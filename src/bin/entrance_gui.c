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
static void _entrance_gui_auth_cb(void *data, const char *user, Eina_Bool granted);
static void _entrance_gui_user_bg_cb(void *data, Evas_Object *obj, const char *sig, const char *src);
static void _entrance_gui_check_wm_loaded(Ecore_X_Window *win);

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
         elm_object_signal_callback_add(o, "entrance,wallpaper,end", "",
                                        _entrance_gui_user_bg_cb, screen);
         ol = entrance_gui_theme_get(_gui->win, "entrance");
         screen->edj = ol;
         if (!ol)
           {
              PT("Tut Tut Tut no theme");
              fprintf(stderr, "%s\n", "entrance");
              return j;
           }
         elm_object_part_content_set(o, "entrance.screen", ol);
         o = entrance_login_add(ol, _entrance_gui_auth_cb, screen);
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
entrance_gui_shutdown(void)
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
   char *style = "double_label";

   screen = eina_list_data_get(_gui->screens);
   style = edje_object_data_get(elm_layout_edje_get(screen->edj), "item_style"); 
   PT("Add users list, using item style: %s\n", style);
   ef = entrance_fill_new(style,
                          _entrance_gui_user_text_get,
                          _entrance_gui_user_content_get,
                          _entrance_gui_user_state_get,
                          _entrance_gui_user_del);
   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        ol = ENTRANCE_GUI_GET(screen->edj, "entrance.users");
        if (!ol) continue;
        entrance_fill(ol, ef, users, NULL,
                      _entrance_gui_user_sel_cb, screen->login);
        elm_object_signal_emit(screen->edj,
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

void
entrance_gui_theme_name_set(const char *theme)
{
   /* TODO */
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

void
entrance_gui_user_bg_set(const char *path, const char *group)
{
   Eina_List *l;
   Entrance_Screen *screen;
   Evas_Object *o;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        if (path && group)
          {
             o = elm_layout_add(screen->background);
             elm_layout_file_set(o, path, group);
             elm_object_part_content_set(screen->transition,
                                         "entrance.wallpaper.user.start", o);
             evas_object_show(o);
             elm_object_signal_emit(screen->transition,
                                    "entrance,wallpaper,user", "");
          }
        else
          elm_object_signal_emit(screen->transition,
                                 "entrance,wallpaper,default", "");
     }
}

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
                  const char *path;
                  const char *group;
                  if (_gui->bg.group)
                    bg = entrance_gui_theme_get(screen->transition,
                                                _gui->bg.group);
                  else
                    bg = entrance_gui_theme_get(screen->transition,
                                                "entrance/background/default");
                  edje_object_file_get(elm_layout_edje_get(bg), &path, &group);
                  eina_stringshare_replace(&_gui->bg.path, path);
                  eina_stringshare_replace(&_gui->bg.group, group);
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
_entrance_gui_user_sel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Entrance_Login *eu;
   eu = elm_object_item_data_get(event_info);
   entrance_login_login_set(data, eu->login);
}


static char *
_entrance_gui_user_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   Entrance_Login *eu;
   eu = data;
   if ((part) && (!strcmp(part, "elm.text.sub")))
     if (eu->lsess)
       return strdup(eu->lsess);
     else
       return NULL;
   else
     return strdup(eu->login);
}

static Evas_Object *
_entrance_gui_user_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = NULL;
   Entrance_Login *eu;
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
                  edje_object_file_get(
                     elm_layout_edje_get(
                        elm_object_part_content_get(ic, "entrance.icon")),
                     &path, &group);
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
_entrance_gui_user_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_entrance_gui_user_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
}

///////////////////////////////////////////////////
///////////////// ACTION //////////////////////////
///////////////////////////////////////////////////

static char *
_entrance_gui_action_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Entrance_Action *ea;
   ea = data;
   if ((part) && (!strcmp(part, "icon")))
     return NULL;
   return strdup(ea->label);
}

static void
_entrance_gui_action_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Entrance_Action *ea;
   ea = elm_object_item_data_get(event_info);
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
        entrance_fill(o, ef, _gui->actions, NULL,
                      _entrance_gui_action_clicked_cb, screen);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                "entrance,action,enabled", "");
     }
}

////////////////////////////////////////////////////////////////////////////////
static void
_entrance_gui_auth_cb(void *data EINA_UNUSED, const char *user EINA_UNUSED, Eina_Bool granted)
{
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        if (granted)
          {
             elm_object_signal_emit(screen->edj,
                                    "entrance,auth,valid", "");

             _entrance_gui_check_wm_loaded(_gui->win);
          }
        else
          {
             elm_object_signal_emit(screen->edj,
                                    "entrance,auth,error", "");
          }
     }
   /*
      if (granted)
      _gui_login_timeout = ecore_timer_add(10.0,
      _entrance_gui_login_timeout,
      data);
    */
}

static void
_entrance_gui_user_bg_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   Evas_Object *o;
   Entrance_Screen *screen;
   screen = data;
   o = elm_object_part_content_get(screen->transition,
                                   "entrance.wallpaper.user");
   evas_object_del(o);
   o = elm_object_part_content_get(screen->transition,
                                   "entrance.wallpaper.user.start");
   if (o)
     elm_object_part_content_set(screen->transition,
                                 "entrance.wallpaper.user", o);
}

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

   _entrance_gui_check_wm_loaded(ev->win);

   return ECORE_CALLBACK_DONE;
}

static void _entrance_gui_check_wm_loaded(Ecore_X_Window *win)
{
   Ecore_X_Window val;
   char *name;

   /* In case we missed the event let's first check if a SUPPORTING_WM is registered */
   ecore_x_window_prop_window_get(ecore_x_window_root_get(win),
                                         ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &val, 1);
   if (val)
     {
        PT("Found a SUPPORTING_WM set\n");
        // TODO we should check the child window exists

        elm_exit();
     }

   /* Adding this avoid us to launch entrance_client with a wm anymore ... */
   name = ecore_x_window_prop_string_get(ecore_x_window_root_get(win),
                                         ECORE_X_ATOM_NET_WM_NAME);
   if (name)
     {
        PT("screen managed by %s though not compliant\n", name);
        elm_exit();
     }

   name = ecore_x_window_prop_string_get(ecore_x_window_root_get(win),
                                         ECORE_X_ATOM_WM_NAME);
   if (name)
     {
        PT("screen managed by legacy %s\n", name);
        elm_exit();
     }
}

