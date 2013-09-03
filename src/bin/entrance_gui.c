#include "entrance_client.h"
#include "Ecore_X.h"


typedef struct Entrance_Gui_ Entrance_Gui;
typedef struct Entrance_Screen_ Entrance_Screen;

static Eina_Bool _entrance_gui_cb_window_property(void *data, int type, void *event_info);
static void _entrance_gui_user_sel_cb(void *data, Evas_Object *obj, void *event_info);
static char *_entrance_gui_user_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_gui_user_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _entrance_gui_user_state_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_gui_user_del(void *data, Evas_Object *obj);
static void _entrance_gui_actions_populate();
static void _entrance_gui_conf_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_update(void);


/*
static void _entrance_gui_session_update(Entrance_Xsession *xsession);
static void _entrance_gui_login_activated_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_password_activated_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_shutdown(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_focus(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_session_update(Entrance_Xsession *xsession);
static Eina_Bool _entrance_gui_auth_enable(void *data);
static void _entrance_gui_action_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static Ecore_Timer *_gui_login_timeout;
*/
static Entrance_Gui *_gui;

enum {
     ENTRANCE_CONF_NONE = 0,
     ENTRANCE_CONF_STATE = (1 << 0),
     ENTRANCE_CONF_WALLPAPER = (1 << 1),
     ENTRANCE_CONF_VKBD = (1 << 2)
};


struct Entrance_Gui_
{
   Evas_Object *win;
   Eina_List *screens;
   Eina_List *xsessions;
   Eina_List *users;
   Eina_List *actions;
   Eina_List *handlers;
   Entrance_Xsession *selected_session;
   const char *theme;
   struct
     {
        const char *path;
        const char *group;
     } bg;
   unsigned char changed;
   Eina_Bool conf_enabled : 1;
   Eina_Bool vkbd_enabled : 1;
};

struct Entrance_Screen_
{
   Evas_Object *edj;
   Evas_Object *background;
   Evas_Object *transition;
   Evas_Object *login;
   Eina_Bool focused:1;
   Eina_Bool managed:1;
};

int
entrance_gui_init(const char *theme)
{

   Ecore_X_Window xw;
   Entrance_Screen *screen;
   int i, j;
   int x, y, w, h;
   int ww = 0, hh = 0;

   PT("Gui init: ");
   fprintf(stderr, "%s\n", theme);
   _gui = calloc(1, sizeof(Entrance_Gui));
   if (!_gui)
     {
        PT("Not Enough memory\n");
        return 1;
     }
   _gui->theme = eina_stringshare_add(theme);

#ifdef XNEST_DEBUG
   char *tmp = getenv("DISPLAY");
   if (tmp && *tmp)
     {
        PT("client Using display name");
        fprintf(stderr, " %s\n", tmp);
     }
#endif

   i = ecore_x_xinerama_screen_count_get();
   if (i < 1) i = 1;
   _gui->win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_fullscreen_set(_gui->win, EINA_TRUE);
   elm_win_title_set(_gui->win, PACKAGE);
   for(j = 0; j < i; ++j)
     {
        Evas_Object *o, *ol;
        screen = calloc(1, sizeof(Entrance_Screen));
        if (!screen) return 1;

        /* layout */
         o = entrance_gui_theme_get(_gui->win, "entrance/wallpaper/default");
         screen->transition = o;
         ol = entrance_gui_theme_get(_gui->win, "entrance");
         screen->edj = ol;
         if (!ol)
           {
              PT("Tut Tut Tut no theme");
              fprintf(stderr, "%s\n", "entrance");
              return j;
           }
         elm_object_part_content_set(o, "entrance.login", ol);
         o = entrance_login_add(ol);
         entrance_login_open_session_set(o, EINA_TRUE);
         screen->login = o;
         elm_object_part_content_set(ol, "entrance.login", o);
         evas_object_smart_callback_add(
            ENTRANCE_GUI_GET(ol, "entrance.conf"),
            "clicked",
            _entrance_gui_conf_clicked_cb,
            screen->transition);
         evas_object_show(screen->transition);
         evas_object_show(screen->edj);
         evas_object_show(screen->login);

         _gui->screens = eina_list_append(_gui->screens, screen);
         ecore_x_xinerama_screen_geometry_get(j, &x, &y, &w, &h);
         evas_object_move(screen->transition, x, y);
         evas_object_resize(screen->transition, w, h);
         if ((x + w) > ww) ww = x + w;
         if ((y + h) > hh) hh = y + h;
     }
   _entrance_gui_update();
   _gui->handlers =
      eina_list_append(_gui->handlers,
                       ecore_event_handler_add(
                          ECORE_X_EVENT_WINDOW_PROPERTY,
                          _entrance_gui_cb_window_property,
                          NULL));
   xw = elm_win_xwindow_get(_gui->win);
   ecore_x_window_move(xw, 0, 0);
   evas_object_resize(_gui->win, ww, hh);
   evas_object_show(_gui->win);
     {
        /* tricky situation. we are not normally running with a wm and thus
         * have to set focus to our window so things work right */
        screen = _gui->screens->data;
        ecore_evas_focus_set
           (ecore_evas_ecore_evas_get(evas_object_evas_get(_gui->win)), 1);
        /* need to hide and show the cursor */
        ecore_x_window_cursor_show(elm_win_xwindow_get(_gui->win),
                                   EINA_FALSE);
        ecore_x_window_cursor_show(elm_win_xwindow_get(_gui->win),
                                   EINA_TRUE);
     }
   return j;
}

void
entrance_gui_shutdown()
{
   Entrance_Screen *screen;
   Entrance_Xsession *xsession;
   Ecore_Event_Handler *h;
   PT("Gui shutdown\n");
   evas_object_del(_gui->win);
   EINA_LIST_FREE(_gui->screens, screen)
     {
        free(screen);
     }
   eina_stringshare_del(_gui->theme);
   EINA_LIST_FREE(_gui->xsessions, xsession)
     {
        eina_stringshare_del(xsession->name);
        eina_stringshare_del(xsession->command);
        eina_stringshare_del(xsession->icon);
     }
   EINA_LIST_FREE(_gui->handlers, h)
      ecore_event_handler_del(h);
   if (_gui) free(_gui);
}

Evas_Object *
entrance_gui_theme_get (Evas_Object *win, const char *group)
{
   Evas_Object *edje;

   edje = elm_layout_add(win);
   if (_gui->theme)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf),
                 PACKAGE_DATA_DIR"/themes/%s.edj", _gui->theme);
        if (!elm_layout_file_set(edje, buf, group))
          {
             PT("Can't load %s theme fallback to default\n", _gui->theme);
             elm_layout_file_set(edje, PACKAGE_DATA_DIR"/themes/default.edj",
                                 group);
          }
     }
   else
     elm_layout_file_set(edje, PACKAGE_DATA_DIR"/themes/default.edj", group);
   return edje;
}

Eina_List *
entrance_gui_stringlist_get(const char *str)
{
   Eina_List *list = NULL;
   const char *s, *b;
   if (!str) return NULL;
   for (b = s = str; 1; s++)
     {
        if ((*s == ' ') || (!*s))
          {
             char *t = malloc(s - b + 1);
             if (t)
               {
                  strncpy(t, b, s - b);
                  t[s - b] = 0;
                  list = eina_list_append(list, eina_stringshare_add(t));
                  free(t);
               }
             b = s + 1;
          }
        if (!*s) break;
     }
   return list;
}

void
entrance_gui_stringlist_free(Eina_List *list)
{
   const char *s;
   EINA_LIST_FREE(list, s)
      eina_stringshare_del(s);
}

void
entrance_gui_auth_valid()
{
   Eina_List *l;
   Entrance_Screen *screen;
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,auth,valid", "");
     }
   /*
   _gui_login_timeout = ecore_timer_add(10.0,
                                        _entrance_gui_login_timeout,
                                        screen);
                                        */
}

void
entrance_gui_auth_error()
{
   /*
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.password");
        elm_entry_entry_set(o, "");
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,auth,error", "");
     }
     */
}

void
entrance_gui_actions_set(Eina_List *actions)
{
   if (!actions) return;
   PT("Actions set\n");
   _gui->actions = actions;
   _entrance_gui_actions_populate();
}

void
entrance_gui_auth_max_tries(void)
{
   /*
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.login");
        elm_entry_entry_set(o, "");
        elm_object_disabled_set(o, EINA_TRUE);
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.password");
        elm_entry_entry_set(o, "");
        elm_object_disabled_set(o, EINA_TRUE);
     }
   ecore_timer_add(5.0, _entrance_gui_auth_enable, NULL);
   */
}

void
entrance_gui_users_set(Eina_List *users)
{
   Evas_Object *ol;
   Entrance_Screen *screen;
   Eina_List *l;
   Entrance_Fill *ef;

   PT("Add users list\n");
   ef = entrance_fill_new("default",
                          _entrance_gui_user_text_get,
                          _entrance_gui_user_content_get,
                          _entrance_gui_user_state_get,
                          _entrance_gui_user_del);
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        ol = ENTRANCE_GUI_GET(screen->edj, "entrance.users");
        if (!ol) continue;
        entrance_fill(ol, ef, users, _entrance_gui_user_sel_cb, screen->login);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,users,enabled", "");
     }
   _gui->users = users;
}

const Eina_List *
entrance_gui_users_get(void)
{
   return _gui->users;
}

void
entrance_gui_xsessions_set(Eina_List *xsessions)
{
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        entrance_login_xsessions_populate(screen->login, xsessions);
     }
   _gui->xsessions = xsessions;
}

const Eina_List *
entrance_gui_xsessions_get(void)
{
   return _gui->xsessions;
}

void
entrance_gui_conf_set(const Entrance_Conf_Gui_Event *conf)
{
   if ((conf->bg.path) && (*conf->bg.path)
       && (_gui->bg.path != conf->bg.path))
     {
        eina_stringshare_replace(&_gui->bg.path, conf->bg.path);
        _gui->changed &= ENTRANCE_CONF_WALLPAPER;
     }
   if ((conf->bg.group) && (*conf->bg.group)
       && (_gui->bg.group != conf->bg.group))
     {
        eina_stringshare_replace(&_gui->bg.group, conf->bg.group);
        _gui->changed &= ENTRANCE_CONF_WALLPAPER;
     }

   if (_gui->vkbd_enabled != conf->vkbd_enabled)
     {
        _gui->vkbd_enabled = conf->vkbd_enabled;
        _gui->changed &= ENTRANCE_CONF_VKBD;
     }

   _gui->changed = ~(ENTRANCE_CONF_NONE);
   _entrance_gui_update();
}

const char *
entrance_gui_theme_name_get(void)
{
   return _gui->theme;
}

const char *
entrance_gui_theme_path_get(void)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", _gui->theme);
   return eina_stringshare_add(buf);
}

void
entrance_gui_background_get(const char **path, const char **group)
{
   if (path)
     *path = _gui->bg.path;
   if (group)
     *group = _gui->bg.group;
}

Eina_Bool
entrance_gui_vkbd_enabled_get(void)
{
   return _gui->vkbd_enabled;
}
/*
static void
_entrance_gui_login_activated_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   char *txt;
   Eina_List *l, *ll;
   Entrance_Xsession *xsess;
   Entrance_User_Event *eu = NULL;
   Entrance_Screen *screen;

   PT("login activated\n");
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
                              "entrance,auth,enable", "");
}

static void
_entrance_gui_shutdown(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_exit();
   PT("shutdown cb\n");
}


static void
_entrance_gui_focus(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
      elm_object_focus_set(ENTRANCE_GUI_GET(screen->edj, "entrance.login"),
                           EINA_TRUE);
}

static void
_entrance_gui_login_cancel_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *sig __UNUSED__, const char *src __UNUSED__)
{
   Evas_Object *o;
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.login");
        elm_entry_entry_set(o, "");
        elm_object_focus_set(o, EINA_TRUE);
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.password");
        elm_entry_entry_set(o, "");
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,auth,disable", "");
     }
}

static Eina_Bool
_entrance_gui_login_timeout(void *data __UNUSED__)
{
   Evas_Object *popup, *o, *bx;
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        popup = elm_popup_add(screen->win);
        evas_object_size_hint_weight_set(popup,
                                         EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_object_text_set(popup, "Something wrong happened ... "
                            "No window manager detected after a lapse of time. "
                            "See your debug below.");

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
        elm_object_focus_set(ENTRANCE_GUI_GET(screen->edj, "entrance.password"), EINA_TRUE);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,auth,enable", "");
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
   PT("password activated\n");
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
_entrance_gui_callback_add(Entrance_Screen *screen)
{
   Evas_Object *host, *pwd;
   Evas_Object *edj;

   evas_object_smart_callback_add(screen->win, "delete,request",
                                  _entrance_gui_shutdown, NULL);
   evas_object_smart_callback_add(screen->win, "focus,in",
                                  _entrance_gui_focus, NULL);

   edj = elm_layout_edje_get(screen->edj);
   host = ENTRANCE_GUI_GET(screen->edj, "entrance.login");
   pwd = ENTRANCE_GUI_GET(screen->edj, "entrance.password");
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
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.xsessions");
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
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.xsessions");

        EINA_LIST_FOREACH(_gui->xsessions, l, xsession)
          {
             elm_hoversel_item_add(o, xsession->name, xsession->icon,
                                   ELM_ICON_FILE,
                                   _entrance_gui_xsessions_clicked_cb, xsession);
          }
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,xsession,enabled", "");
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
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.login");
        elm_object_disabled_set(o, EINA_FALSE);
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.password");
        elm_object_disabled_set(o, EINA_FALSE);
     }
   return ECORE_CALLBACK_CANCEL;
}

*/
static void
_entrance_gui_update(void)
{
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        Evas_Object *bg = NULL;
        if (_gui->changed & ENTRANCE_CONF_WALLPAPER)
          {
             PT("Set background %s - %s\n", _gui->bg.path, _gui->bg.group);
             if (_gui->bg.path)
               {
                  if (_gui->bg.group)
                    {
                       bg = elm_layout_add(screen->transition);
                       elm_layout_file_set(bg, _gui->bg.path, _gui->bg.group);
                    }
                  else
                    {
                       bg = elm_layout_add(screen->transition);
                       elm_layout_file_set(bg, _gui->bg.path,
                                           "entrance/background/default");
                    }
               }
             if (!bg)
               {
                  if (_gui->bg.group)
                    bg = entrance_gui_theme_get(screen->transition,
                                                _gui->bg.group);
                  else
                    bg = entrance_gui_theme_get(screen->transition,
                                                "entrance/background/default");
               }
             elm_object_part_content_set(screen->transition,
                                         "entrance.wallpaper.default", bg);
             evas_object_del(screen->background);
             screen->background = bg;
          }
        if (_gui->conf_enabled)
          {
             elm_object_signal_emit(screen->edj,
                                    "entrance,custom_config.enabled", "");
          }
        else
          elm_object_signal_emit(screen->edj,
                                 "entrance,custom_config.disabled", "");
     }
   _gui->changed = 0;
}

static void
_entrance_gui_conf_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   entrance_conf_begin(data, _gui->win);
}
///////////////////////////////////////////////////
///////////////// USER ////////////////////////////
///////////////////////////////////////////////////


static Evas_Object *
_entrance_gui_user_icon_random_get(Evas_Object *obj)
{
   Evas_Object *ic, *o, *r;
   Eina_List *icons;
   unsigned char i;
   const char *icon;
   char buf[PATH_MAX];

   ic = entrance_gui_theme_get(obj, "entrance/user");
   if (!ic) return NULL;
   o = elm_layout_edje_get(ic);
   if (!o) return NULL;
   icons = entrance_gui_stringlist_get(edje_object_data_get(o, "items"));
   if (icons)
     {
        srand(time(NULL));
        i = (unsigned char) ((eina_list_count(icons) * (double)rand())
                             / (RAND_MAX + 1.0));
        icon = eina_list_nth(icons, i);
        snprintf(buf, sizeof(buf),
                 "entrance/user/%s", icon);
        entrance_gui_stringlist_free(icons);
        r = entrance_gui_theme_get(obj, buf);
        elm_object_part_content_set(ic, "entrance.icon", r);
        evas_object_show(r);
     }

   return ic;
}

static void
_entrance_gui_user_sel_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Entrance_User_Event *eu;
   eu = elm_object_item_data_get(event_info);
   entrance_login_login_set(data, eu->login);
   entrance_login_session_set(data, eu->lsess);
}

static char *
_entrance_gui_user_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   Entrance_User_Event *eu;
   eu = data;
   return strdup(eu->login);
}

static Evas_Object *
_entrance_gui_user_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = NULL;
   Entrance_User_Event *eu;
   eu = data;

   if (eu && !strcmp(part, "elm.swallow.icon"))
     {
        if ((eu->image.path) && (*eu->image.path == '/') && (!eu->image.group))
          {
             ic = elm_icon_add(obj);
             elm_image_file_set(ic, eu->image.path, "entrance/user/icon");
             eu->image.group = eina_stringshare_add("entrance/user/icon");

          }
        else
          {
             if (eu->image.group)
               {
                  ic = elm_icon_add(obj);
                  elm_image_file_set(ic, eu->image.path, eu->image.group);
               }
             else
               {
                  const char *path, *group;
                  ic = _entrance_gui_user_icon_random_get(obj);
                  edje_object_file_get(elm_layout_edje_get(ic), &path, &group);
                  eu->image.path = eina_stringshare_add(path);
                  eu->image.group = eina_stringshare_add(group);
               }
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
_entrance_gui_action_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Entrance_Action *ea;
   ea = data;
   if (ea) entrance_connect_action_send(ea->id);
}

static void
_entrance_gui_actions_populate()
{
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        Entrance_Fill *ef;
        ef = entrance_fill_new(NULL, _entrance_gui_action_text_get,
                               NULL, NULL, NULL);
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.actions");
        entrance_fill(o, ef, _gui->actions,
                      _entrance_gui_action_clicked_cb, screen);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,action,enabled", "");
     }
}

////////////////////////////////////////////////////////////////////////////////

static Eina_Bool
_entrance_gui_cb_window_property(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
   Ecore_X_Event_Window_Property *ev;

   ev = event_info;
   if (ev->atom == ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK)
     {
        PT("screen managed\n");
        elm_exit();
     }
   return ECORE_CALLBACK_DONE;
}


