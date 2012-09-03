#include "entrance_client.h"
#include "Ecore_X.h"

#define ENTRANCE_GUI_GET(edj, name) edje_object_part_external_object_get(elm_layout_edje_get(edj), name)

typedef struct Entrance_Gui_ Entrance_Gui;
typedef struct Entrance_Screen_ Entrance_Screen;

struct Entrance_Gui_
{
   Eina_List *screens;
   Eina_List *xsessions;
   Eina_List *users;
   Eina_List *actions;
   Eina_List *handlers;
   Entrance_Xsession *selected_session;
   const char *theme;
};

struct Entrance_Screen_
{
   Evas_Object *win;
   Evas_Object *bg;
   Evas_Object *edj;
   Eina_Bool managed:1;
};

static Evas_Object *_entrance_gui_theme_get(Evas_Object *win, const char *group, const char *theme);
static void _entrance_gui_hostname_activated_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_password_activated_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_shutdown(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_focus(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_session_update(Entrance_Xsession *xsession);

static void _entrance_gui_actions_populate();

static void _entrance_gui_user_sel_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_user_sel(Entrance_User *ou);
static char *_entrance_gui_user_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_gui_user_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _entrance_gui_user_state_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_gui_user_del(void *data, Evas_Object *obj);

static Eina_Bool _entrance_gui_auth_enable(void *data);

static Eina_Bool _entrance_gui_cb_window_property(void *data, int type, void *event_info);

static void _entrance_gui_action_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static Entrance_Gui *_gui;
static Ecore_Timer *_gui_login_timeout;

static Evas_Object *
_entrance_gui_theme_get (Evas_Object *win, const char *group, const char *theme)
{
   Evas_Object *edje = NULL;

   edje = elm_layout_add(win);
   if (theme)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), PACKAGE_DATA_DIR"/themes/%s.edj", theme);
        if (!elm_layout_file_set(edje, buf, group))
          {
             fprintf(stderr, PACKAGE": can't load %s theme fallback to default\n", theme);
             elm_layout_file_set(edje, PACKAGE_DATA_DIR"/themes/default.edj", group);
          }
     }
   else
     elm_layout_file_set(edje, PACKAGE_DATA_DIR"/themes/default.edj", group);
   return edje;
}

static void
_entrance_gui_hostname_activated_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   char *txt;
   Eina_List *l, *ll;
   Entrance_Xsession *xsess;
   Entrance_User *eu = NULL;
   Entrance_Screen *screen;

   txt = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
   if (!txt || !strcmp(txt, ""))
     {
        free(txt);
        return;
     }
   EINA_LIST_FOREACH(_gui->users, ll, eu)
      if (!strcmp(txt, eu->login)) break;
   free(txt);

   if (eu && eu->lsess)
     {
        EINA_LIST_FOREACH(_gui->xsessions, l, xsess)
          {
             if (!strcmp(xsess->name, eu->lsess))
               {
                  _entrance_gui_session_update(xsess);
                  break;
               }
          }
     }
   else if (_gui->xsessions)
     _entrance_gui_session_update(_gui->xsessions->data);
   elm_object_focus_set(data, EINA_TRUE);
   EINA_LIST_FOREACH(_gui->screens, l, screen)
      edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                              "entrance.auth.enable", "");
}

static void
_entrance_gui_shutdown(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_exit();
   printf("shutdown cb\n");
}

static Eina_Bool
_entrance_gui_cb_window_property(void *data, int type __UNUSED__, void *event_info)
{
   Entrance_Screen *screen;
   Ecore_X_Event_Window_Property *ev;
   Eina_List *l;
   ev = event_info;
   screen = data;
   if (ev->atom == ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK)
     screen->managed = EINA_TRUE;
   EINA_LIST_FOREACH(_gui->screens, l, screen)
      if (!screen->managed)
        return ECORE_CALLBACK_PASS_ON;
   elm_exit();
   return ECORE_CALLBACK_PASS_ON;
}

static void
_entrance_gui_focus(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
      elm_object_focus_set(ENTRANCE_GUI_GET(screen->edj, "hostname"), EINA_TRUE);
}

static void
_entrance_gui_login_cancel_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *sig __UNUSED__, const char *src __UNUSED__)
{
   Evas_Object *o;
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "hostname");
        elm_entry_entry_set(o, "");
        elm_object_focus_set(o, EINA_TRUE);
        o = ENTRANCE_GUI_GET(screen->edj, "password");
        elm_entry_entry_set(o, "");
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.auth.disable", "");
     }
}

static Eina_Bool
_entrance_gui_login_timeout(void *data)
{
   Evas_Object *popup, *o, *vbx, *bx;
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        popup = elm_popup_add(screen->win);
        evas_object_size_hint_weight_set(popup,
                                         EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_object_text_set(popup, "something wrong happened... No window manager detected after a lapse of time. See your debug below.");

        bx = elm_box_add(popup);
        elm_object_content_set(popup, bx);
        evas_object_show(bx);

        o = elm_entry_add(popup);
        elm_entry_scrollable_set(o, EINA_TRUE);
        elm_object_text_set(o, "Test !!!!!");
        elm_box_pack_end(bx, o);
        evas_object_show(o);

        o = evas_object_rectangle_add(evas_object_evas_get(popup));
        evas_object_size_hint_min_set(o, 0, 260);
        elm_box_pack_end(bx, o);


        o = elm_button_add(popup);
        elm_object_text_set(o, "Close");
        elm_object_part_content_set(popup, "button1", o);
        evas_object_smart_callback_add(o, "clicked",
                                       _entrance_gui_shutdown, NULL);
        evas_object_show(popup);
     }
   _gui_login_timeout = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_entrance_gui_login(Entrance_Screen *screen)
{
   Eina_List *l;
   char *h, *s;
   h = entrance_gui_user_get(screen);
   s = entrance_gui_password_get(screen);
   if (h && s)
     {
        if (strcmp(h, "") && strcmp(s, ""))
          {
             if (_gui->selected_session)
                  entrance_connect_auth_send(h, s, _gui->selected_session->name);
             else
                  entrance_connect_auth_send(h, s, NULL);
          }
     }
   free(h);
   free(s);
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        elm_object_focus_set(ENTRANCE_GUI_GET(screen->edj, "password"), EINA_TRUE);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.auth.enable", "");
     }
}

static void
_entrance_gui_login_request_cb(void *data, Evas_Object *obj __UNUSED__, const char *sig __UNUSED__, const char *src __UNUSED__)
{
   _entrance_gui_login(data);
}

static void
_entrance_gui_password_activated_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _entrance_gui_login(data);
}

static void
_entrance_gui_xsessions_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *icon;
   Eina_List *l;
   Entrance_Screen *screen;

   _gui->selected_session = data;
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        elm_object_text_set(obj, _gui->selected_session->name);
        icon = elm_icon_add(screen->win);
        elm_image_file_set(icon, _gui->selected_session->icon, NULL);
        elm_object_content_set(obj, icon);
        evas_object_show(icon);
     }
}

static void
_entrance_gui_action_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Entrance_Action *ea;
   ea = data;
   if (ea) entrance_connect_action_send(ea->id);
}



static void
_entrance_gui_callback_add(Entrance_Screen *screen)
{
   Evas_Object *host, *pwd;
   Evas_Object *edj;

   evas_object_smart_callback_add(screen->win, "delete,request",
                                  _entrance_gui_shutdown, NULL);
   evas_object_smart_callback_add(screen->win, "focus,in",
                                  _entrance_gui_focus, NULL);

   edj = elm_layout_edje_get(screen->edj);
   host = ENTRANCE_GUI_GET(screen->edj, "hostname");
   pwd = ENTRANCE_GUI_GET(screen->edj, "password");
   evas_object_smart_callback_add(host, "activated",
                                  _entrance_gui_hostname_activated_cb, pwd);
   evas_object_smart_callback_add(pwd, "activated",
                                  _entrance_gui_password_activated_cb, screen);
   edje_object_signal_callback_add(edj, "entrance.auth.cancel", "",
                                   _entrance_gui_login_cancel_cb, NULL);
   edje_object_signal_callback_add(edj, "entrance.auth.request", "",
                                   _entrance_gui_login_request_cb, screen);
   elm_entry_single_line_set(host, EINA_TRUE);
   elm_entry_single_line_set(pwd, EINA_TRUE);
}

static void
_entrance_gui_session_update(Entrance_Xsession *xsession)
{
   Evas_Object *o, *icon;
   Eina_List *l;
   Entrance_Screen *screen;

   if (!xsession) return;
   _gui->selected_session = xsession;
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "xsessions");
        elm_object_text_set(o, _gui->selected_session->name);
        icon = elm_icon_add(screen->win);
        elm_image_file_set(icon, _gui->selected_session->icon, NULL);
        elm_object_content_set(o, icon);
     }
}

static void
_entrance_gui_sessions_populate()
{
   Evas_Object *o;

   Entrance_Xsession *xsession;
   Eina_List *l, *ll;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, ll, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "xsessions");

        EINA_LIST_FOREACH(_gui->xsessions, l, xsession)
          {
             elm_hoversel_item_add(o, xsession->name, xsession->icon,
                                   ELM_ICON_FILE,
                                   _entrance_gui_xsessions_clicked_cb, xsession);
          }
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.xsession.enabled", "");
     }
   if (_gui->xsessions)
     _entrance_gui_session_update(_gui->xsessions->data);
}

static Eina_Bool
_entrance_gui_auth_enable(void *data __UNUSED__)
{
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "hostname");
        elm_object_disabled_set(o, EINA_FALSE);
        o = ENTRANCE_GUI_GET(screen->edj, "password");
        elm_object_disabled_set(o, EINA_FALSE);
     }
   return ECORE_CALLBACK_CANCEL;
}


void
entrance_gui_xsession_set(Eina_List *xsessions)
{
   if (!xsessions) return;
   _gui->xsessions = xsessions;
   _entrance_gui_sessions_populate();
}

void
entrance_gui_actions_set(Eina_List *actions)
{
   if (!actions) return;
   fprintf(stderr, PACKAGE": Action set\n");
   _gui->actions = actions;
   _entrance_gui_actions_populate();
}

int
entrance_gui_init(const char *theme)
{

   Ecore_X_Window xw;
   Entrance_Screen *screen;
   int ii, i;
   int x, y, w, h;

   fprintf(stderr, PACKAGE": client Gui init\n");
   _gui = calloc(1, sizeof(Entrance_Gui));
   if (!_gui)
     {
        fprintf(stderr, PACKAGE": client Not Enough memory\n");
        return 1;
     }

#ifdef XNEST_DEBUG
   char *tmp = getenv("DISPLAY");
   if (tmp && *tmp)
     {
        fprintf(stderr, PACKAGE": client Using display name %s", tmp);
     }
#endif


   i = ecore_x_xinerama_screen_count_get();
   if (i < 1) i = 1;
   for(ii = 0; ii < i; ++ii)
     {
        screen = calloc(1, sizeof(Entrance_Screen));
        if (!screen) return 1;

        _gui->screens = eina_list_append(_gui->screens, screen);
        ecore_x_xinerama_screen_geometry_get(ii, &x, &y, &w, &h);
        screen->win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
        elm_win_fullscreen_set(screen->win, EINA_TRUE);
        elm_win_title_set(screen->win, PACKAGE);

        _gui->theme = eina_stringshare_add(theme);
        screen->edj = _entrance_gui_theme_get(screen->win, "entrance", theme);

        if (!screen->edj)
          {
             fprintf(stderr, PACKAGE": client Tut Tut Tut no theme\n");
             return 2;
          }
        evas_object_size_hint_weight_set(screen->edj,
                                         EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_win_resize_object_add(screen->win, screen->edj);
        _entrance_gui_callback_add(screen);
        evas_object_show(screen->edj);

        xw = elm_win_xwindow_get(screen->win);
        evas_object_resize(screen->win, w, h);
        ecore_x_window_move(xw, x, y);
        evas_object_show(screen->win);
        _gui->handlers =
           eina_list_append(_gui->handlers,
                            ecore_event_handler_add(
                               ECORE_X_EVENT_WINDOW_PROPERTY,
                               _entrance_gui_cb_window_property,
                               screen));
     }
   if (_gui->screens)
     {
        /* tricky situation. we are not normally running with a wm and thus
         * have to set focus to our window so things work right */
        screen = _gui->screens->data;
        ecore_evas_focus_set
           (ecore_evas_ecore_evas_get(evas_object_evas_get(screen->win)), 1);
     }
   return 0;
}

void
entrance_gui_shutdown()
{
   Entrance_Xsession *xsession;
   Entrance_Screen *screen;
   Ecore_Event_Handler *h;
   fprintf(stderr, PACKAGE": Gui shutdown\n");
   EINA_LIST_FREE(_gui->screens, screen)
     {
        evas_object_del(screen->win);
        free(screen);
     }
   eina_stringshare_del(_gui->theme);
   EINA_LIST_FREE(_gui->xsessions, xsession)
     {
        eina_stringshare_del(xsession->name);
        eina_stringshare_del(xsession->command);
        if (xsession->icon) eina_stringshare_del(xsession->icon);
     }
   EINA_LIST_FREE(_gui->handlers, h)
      ecore_event_handler_del(h);
   if (_gui) free(_gui);
}

char *
entrance_gui_user_get(Entrance_Screen *screen)
{
   Evas_Object *o;
   o = ENTRANCE_GUI_GET(screen->edj, "hostname");
   if (o) return elm_entry_markup_to_utf8(elm_entry_entry_get(o));
   return NULL;
}

char *
entrance_gui_password_get(Entrance_Screen *screen)
{
   Evas_Object *o;
   o = ENTRANCE_GUI_GET(screen->edj, "password");
   if (o) return elm_entry_markup_to_utf8(elm_entry_entry_get(o));
   return NULL;
}

void
entrance_gui_auth_error()
{
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "password");
        elm_entry_entry_set(o, "");
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.auth.error", "");
     }
}

void
entrance_gui_auth_wait()
{
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "hostname");
        elm_entry_entry_set(o, "");
        elm_object_disabled_set(o, EINA_TRUE);
        o = ENTRANCE_GUI_GET(screen->edj, "password");
        elm_entry_entry_set(o, "");
        elm_object_disabled_set(o, EINA_TRUE);
     }
   ecore_timer_add(5.0, _entrance_gui_auth_enable, NULL);
}

void
entrance_gui_auth_valid()
{
   Eina_List *l;
   Entrance_Screen *screen;
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.auth.valid", "");
     }
   _gui_login_timeout = ecore_timer_add(10.0,
                                        _entrance_gui_login_timeout,
                                        screen);
}

///////////////////////////////////////////////////
///////////////// USER ////////////////////////////
///////////////////////////////////////////////////
void
entrance_gui_users_set(Eina_List *users)
{
   Evas_Object *ol;
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        Entrance_Fill *ef;
        ol = ENTRANCE_GUI_GET(screen->edj, "entrance_users");
        ef = entrance_fill_new("default",
                               _entrance_gui_user_text_get,
                               _entrance_gui_user_content_get,
                               _entrance_gui_user_state_get,
                               _entrance_gui_user_del);
        entrance_fill(ol, ef, users, _entrance_gui_user_sel_cb);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.users.enabled", "");
        _gui->users = users;
     }
}

static void
_entrance_gui_user_sel(Entrance_User *eu)
{
   Evas_Object *o;
   Entrance_Xsession *xsess;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "hostname");
        elm_entry_entry_set(o, eu->login);
        elm_object_focus_set(ENTRANCE_GUI_GET(screen->edj, "password"), EINA_TRUE);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.auth.enable", "");
     }
   if (eu->lsess)
     {
        EINA_LIST_FOREACH(_gui->xsessions, l, xsess)
          {
             if (!strcmp(xsess->name, eu->lsess))
               {
                  _entrance_gui_session_update(xsess);
                  break;
               }
          }
     }
   else if (_gui->xsessions)
     _entrance_gui_session_update(_gui->xsessions->data);
}

static void
_entrance_gui_user_sel_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _entrance_gui_user_sel(data);
}

static char *
_entrance_gui_user_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   Entrance_User *eu;
   eu = data;
   return strdup(eu->login);
}

static Evas_Object *
_entrance_gui_user_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = NULL;
   Entrance_User *eu;
   eu = data;

   if (eu && !strcmp(part, "elm.swallow.icon"))
     {
        if (eu->image)
          {
             ic = elm_icon_add(obj);
             elm_image_file_set(ic, eu->image, "entrance/user/icon");
          }
        else
          {
             ic = _entrance_gui_theme_get(obj, "entrance/user/default",
                                      _gui->theme);
          }
        evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(ic);
     }
   return ic;
}

static Eina_Bool
_entrance_gui_user_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}

static void
_entrance_gui_user_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

///////////////////////////////////////////////////
///////////////// ACTION //////////////////////////
///////////////////////////////////////////////////

static char *
_entrance_gui_action_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   Entrance_Action *ea;
   ea = data;
   return strdup(ea->label);
}

static void
_entrance_gui_actions_populate()
{
   Evas_Object *o;

   Entrance_Action *action;
   Eina_List *l, *ll;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, ll, screen)
     {
        Entrance_Fill *ef;
        ef = entrance_fill_new(NULL, _entrance_gui_action_text_get,
                               NULL, NULL, NULL);
        o = ENTRANCE_GUI_GET(screen->edj, "actions");
        entrance_fill(o, ef, _gui->actions, _entrance_gui_action_clicked_cb);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance.action.enabled", "");
     }
}

