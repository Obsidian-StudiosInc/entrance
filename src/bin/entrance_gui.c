#include "entrance_client.h"
#include "Ecore_X.h"
#include "time.h"


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
static Eina_List* _entrance_gui_theme_icons_cache_fill(Evas_Object *obj, const char *themename);
static Eina_List* _entrance_gui_theme_background_cache_fill(Evas_Object *obj, const char *themename);
static void _entrance_gui_users_populate(void);

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
   Eina_List *background_pool;
   Eina_List *icon_pool;
   Eina_List *user_pools;
   Eina_List *theme_background_pool;
   Eina_List *theme_icon_pool;
   Eina_List *themes;
   Entrance_Xsession *selected_session;
   Ecore_Event_Handler *handler;
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

   PT("Gui init: %s", theme);
   _gui = calloc(1, sizeof(Entrance_Gui));
   if (!_gui)
     {
        PT("Not Enough memory");
        return 1;
     }
   _gui->theme = eina_stringshare_add(theme);

#ifdef XNEST_DEBUG
   char *tmp = getenv("DISPLAY");
   if (tmp && *tmp)
     {
        PT("client Using display name %s", tmp);
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
         if(j<1)
           {
             ol = entrance_gui_theme_get(_gui->win, "entrance");
             screen->edj = ol;
             if (!ol)
               {
                  PT("Tut Tut Tut no theme for entrance");
                  return j;
               }
             elm_object_part_content_set(o, "entrance.screen", ol);

             /* date */
             /* FIXME: EDJE should be handling color and size */
             o = elm_object_part_content_get(ol, "entrance.date");
             evas_object_color_set(o, 51, 153, 255, 255);
             time_t t = time(0);
             struct tm tm = *localtime(&t);
             char date[64];
             strftime(date,64,"%B %d, %Y",&tm);
             char markup[128];
             snprintf(markup,128,"<font_size=36>%s</font_size>",date);
             elm_object_text_set(o,markup);

             /* clock */
             o = elm_clock_add(ol);
             elm_clock_show_am_pm_set(o, EINA_TRUE);
             elm_object_part_content_set(ol, "entrance.clock", o);

             o = entrance_login_add(ol, _entrance_gui_auth_cb, screen);
             entrance_login_open_session_set(o, EINA_TRUE);
             screen->login = o;
             elm_object_part_content_set(ol, "entrance.login", o);
             evas_object_smart_callback_add(
                ENTRANCE_GUI_GET(ol, "entrance.conf"),
                "clicked",
                _entrance_gui_conf_clicked_cb,
                screen->transition);
             evas_object_show(screen->login);
             evas_object_show(screen->edj);
           }
         evas_object_show(screen->transition);

         _gui->screens = eina_list_append(_gui->screens, screen);
         ecore_x_xinerama_screen_geometry_get(j, &x, &y, &w, &h);
         evas_object_move(screen->transition, x, y);
         evas_object_resize(screen->transition, w, h);
         if ((x + w) > ww) ww = x + w;
         if ((y + h) > hh) hh = y + h;
     }
   _gui->theme_icon_pool =
      _entrance_gui_theme_icons_cache_fill(_gui->win, _gui->theme);
   _gui->theme_background_pool =
      _entrance_gui_theme_background_cache_fill(_gui->win, _gui->theme);
   _entrance_gui_update();
   xw = elm_win_xwindow_get(_gui->win);
   ecore_x_window_move(xw, 0, 0);
   ecore_x_event_mask_set(ecore_x_window_root_get(xw),
                          ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
   _gui->handler =
      ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY,
                              _entrance_gui_cb_window_property, NULL);
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

static void
_entrance_gui_theme_update(void)
{
   Eina_List *node;
   Entrance_Screen *screen;
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", _gui->theme);
   EINA_LIST_FOREACH(_gui->screens, node, screen)
     {
        elm_layout_file_set(screen->transition, buf, "entrance/wallpaper/default");
        elm_layout_file_set(screen->edj, buf, "entrance");
        elm_layout_file_set(screen->login, buf, "entrance/login");
        evas_object_smart_callback_add(
                ENTRANCE_GUI_GET(screen->edj, "entrance.conf"),
                "clicked",
                _entrance_gui_conf_clicked_cb,
                screen->transition);
     }
   _gui->theme_icon_pool =
          _entrance_gui_theme_icons_cache_fill(_gui->win, _gui->theme);
   _gui->theme_background_pool =
           _entrance_gui_theme_background_cache_fill(_gui->win, _gui->theme);
   _entrance_gui_actions_populate();
   _entrance_gui_users_populate();

}

void
entrance_gui_shutdown(void)
{
   Entrance_Screen *screen;
   Entrance_Xsession *xsession;
   Entrance_Image *img;
   PT("Gui shutdown");
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
   ecore_event_handler_del(_gui->handler);
   EINA_LIST_FREE(_gui->background_pool, img)
     {
       eina_stringshare_del(img->path);
       eina_stringshare_del(img->group);
       free(img);
     }
   EINA_LIST_FREE(_gui->icon_pool, img)
     {
       eina_stringshare_del(img->path);
       eina_stringshare_del(img->group);
       free(img);
     }
   EINA_LIST_FREE(_gui->theme_icon_pool, img)
     {
       eina_stringshare_del(img->path);
       eina_stringshare_del(img->group);
       free(img);
     }
   EINA_LIST_FREE(_gui->theme_background_pool, img)
     {
       eina_stringshare_del(img->path);
       eina_stringshare_del(img->group);
       free(img);
     }

   if (_gui) free(_gui);
}

static Eina_List*
_entrance_gui_string_to_entrance_image(Eina_List *src, char *stdfile, char *mask)
{
   //If srdfile is NULL we will set the src string to file, if not we will set the stdfile. And the src as group.
   Eina_List *result = NULL;
   char *src_str, path[PATH_MAX];
   Entrance_Image *img;
   EINA_LIST_FREE(src, src_str)
     {
        img = calloc(1, sizeof(Entrance_Image));
        if (stdfile)
          {
            if (mask)
              {
                 snprintf(path, PATH_MAX, mask, src_str);
                 img->group = eina_stringshare_add(path);
                 eina_stringshare_del(src_str);
              }
            else
              img->group = src_str;
            img->path = eina_stringshare_add(stdfile);
          }
        else
          img->path = src_str;
        result = eina_list_append(result,img);
     }
   return result;
}

Eina_List*
entrance_gui_theme_icons(void)
{
  return _gui->theme_icon_pool;
}

Eina_List*
entrance_gui_theme_backgrounds(void)
{
  return _gui->theme_background_pool;
}
static Eina_List*
_entrance_gui_theme_icons_cache_fill(Evas_Object *obj, const char *themename)
{
   Evas_Object *edje, *o;
   char buf[PATH_MAX];
   Eina_List *icons = NULL;

   edje = elm_layout_add(obj);
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", themename);
   if (!elm_layout_file_set(edje, buf, "entrance/user"))
     return NULL; //Can we get to this point ??
   o = elm_layout_edje_get(edje);
   if (!o) return NULL;
   icons = entrance_gui_stringlist_get(edje_object_data_get(o, "items"));
   evas_object_del(edje);
   return _entrance_gui_string_to_entrance_image(icons, buf, "entrance/user/%s");
}

static Eina_List*
_entrance_gui_theme_background_cache_fill(Evas_Object *obj, const char *themename)
{
   Evas_Object *edje, *o;
   char buf[PATH_MAX];
   Eina_List *icons = NULL;

   edje = elm_layout_add(obj);
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", themename);
   if (!elm_layout_file_set(edje, buf, "entrance/background"))
     return NULL;
   o = elm_layout_edje_get(edje);
   if (!o) return NULL;
   icons = entrance_gui_stringlist_get(edje_object_data_get(o, "items"));
   if (!icons) return NULL;
   evas_object_del(edje);
   return _entrance_gui_string_to_entrance_image(icons, buf, "entrance/background/%s");
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
             PT("Can't load %s theme fallback to default", _gui->theme);
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
   PT("Actions set");
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

static void
_entrance_gui_users_populate(void)
{
   Entrance_Screen *screen;
   Eina_List *l;
   Evas_Object *ol;
   Entrance_Fill *ef;
   const char *style;

   screen = eina_list_data_get(_gui->screens);

   style = edje_object_data_get(elm_layout_edje_get(screen->edj), "item_style_users");

   if (!style)
     style = "default"; //theme has not settet a style

   PT("Add users list, using item style: %s", style);
   ef = entrance_fill_new(style,
                          _entrance_gui_user_text_get,
                          _entrance_gui_user_content_get,
                          _entrance_gui_user_state_get,
                          _entrance_gui_user_del);

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        ol = ENTRANCE_GUI_GET(screen->edj, "entrance.users");
        if (!ol) continue;
        entrance_fill(ol, ef, _gui->users, NULL,
                      _entrance_gui_user_sel_cb, screen->login);
        elm_object_signal_emit(screen->edj,
                                "entrance,users,enabled", "");
     }
   entrance_fill_del(ef);
}

void
entrance_gui_users_set(Eina_List *users)
{
   _gui->users = users;
   _entrance_gui_users_populate();
}

const Eina_List *
entrance_gui_users_get(void)
{
   return _gui->users;
}

const Entrance_Login*
entrance_gui_user_get(const char* name)
{
   Entrance_Login *el;
   Eina_List *l;
   EINA_LIST_FOREACH(_gui->users, l, el)
     {
       if(!strcmp(name, el->login))
         {
            return el;
         }
     }
   return NULL;
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
   if (_gui->bg.path != conf->bg.path)
     {
        if ((conf->bg.path) && (*conf->bg.path))
          eina_stringshare_replace(&_gui->bg.path, conf->bg.path);
        else
          {
             eina_stringshare_del(_gui->bg.path);
             _gui->bg.path = NULL;
          }
        _gui->changed &= ENTRANCE_CONF_WALLPAPER;
     }
   if (_gui->bg.group != conf->bg.group)
     {
        if ((conf->bg.group) && (*conf->bg.group))
          eina_stringshare_replace(&_gui->bg.group, conf->bg.group);
        else
          {
             eina_stringshare_del(_gui->bg.group);
             _gui->bg.group = NULL;
          }
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
entrance_gui_pools_set(const Entrance_Pools *pool)
{
   Entrance_Image *img;
   EINA_LIST_FREE(_gui->background_pool, img)
     {
        eina_stringshare_del(img->path);
        eina_stringshare_del(img->group);
        free(img);
     }
   _gui->background_pool = pool->background_pool;

   EINA_LIST_FREE(_gui->icon_pool, img)
     {
        eina_stringshare_del(img->path);
        eina_stringshare_del(img->group);
        free(img);
     }
   _gui->icon_pool = pool->icon_pool;
}

void
entrance_gui_themes_set(Eina_List *list)
{
   _gui->themes = list;
}

Eina_List*
entrance_gui_themes_get(void)
{
   return _gui->themes;
}

void
entrance_gui_theme_name_set(const char *theme)
{
   if (!_gui->theme)
     _gui->theme = eina_stringshare_add(theme);
   else
     eina_stringshare_replace(&_gui->theme, theme);
   _entrance_gui_theme_update();
}

const char *
entrance_gui_theme_name_get(void)
{
   return _gui->theme;
}

Eina_List*
entrance_gui_background_pool_get(void)
{
  return _gui->background_pool;
}

Eina_List*
entrance_gui_icon_pool_get(void)
{
  return _gui->icon_pool;
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

/* FIXME: Use or drop path/group variables restore ability to set background? */
static Evas_Object *
_entrance_gui_background_obj_get(Evas_Object *par,
                                 const char *path EINA_UNUSED,
                                 const char *group EINA_UNUSED)
{
/* FIXME: Fix path to elementary theme and restore ability to set background */
  Evas_Object *bg = NULL;
  bg = elm_layout_add(par);
  if (!elm_layout_file_set(bg,
                           PACKAGE_DATA_DIR"/../elementary/themes/default.edj",
                           "e/desktop/background"))
    {
      evas_object_del(bg);
      return NULL;
    }
  return bg;
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
             PT("Set background %s - %s", _gui->bg.path, _gui->bg.group);
             bg = _entrance_gui_background_obj_get(screen->transition, _gui->bg.path, _gui->bg.group);
             if (!bg)
               {
                  const char *path;
                  const char *group;
                  if ((_gui->bg.group) || (_gui->bg.path))
                    PT("Failed to load new background, fallback on the theme default! ");
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


static Evas_Object*
_entrance_gui_user_icon_random_get(Evas_Object *obj, const char *username)
{
   unsigned int rnd = 0;
   Evas_Object *o = NULL;
   Entrance_Image *img;
   const Entrance_Login *el;
   Eina_List *user_icons = NULL, *sys_icons = NULL, *theme_icons = NULL;

   el = entrance_gui_user_get(username);
   if (el)
       user_icons = el->icon_pool;
   sys_icons = entrance_gui_icon_pool_get();
   theme_icons = entrance_gui_theme_icons();

   srand(time(NULL));
   rnd = (((eina_list_count(user_icons) + eina_list_count(sys_icons) + eina_list_count(theme_icons))
         * (double)rand()) / (RAND_MAX + 1.0));
   if ((el) && (rnd < eina_list_count(user_icons)))
     {
        o = elm_icon_add(obj);
        img = eina_list_nth(user_icons, rnd);
        elm_image_file_set(o, img->path, NULL);

     }
   else if((rnd >= eina_list_count(user_icons)) && (rnd < (eina_list_count(user_icons)
            +eina_list_count(sys_icons))))
     {
        o = elm_icon_add(obj);
        img = eina_list_nth(sys_icons, (rnd - eina_list_count(user_icons)));
        elm_image_file_set(o, img->path, NULL);
     }
   else
     {
        img = eina_list_nth(theme_icons, (rnd - (eina_list_count(user_icons)
                                        + eina_list_count(sys_icons))));
        o = elm_icon_add(obj);
        elm_image_file_set(o, img->path, img->group);

     }
   return o;
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
   Evas_Object *ic = NULL, *o;
   Entrance_Login *eu;
   eu = data;
   if (eu && !strcmp(part, "elm.swallow.icon"))
     {
        ic = entrance_gui_theme_get(obj, "entrance/user");
        if ((!eu->image.path) && (!eu->image.group))
          {
             o = _entrance_gui_user_icon_random_get(obj, eu->login);
             if(eu->image.path || eu->image.group)
               {
                 elm_image_file_get(o,&(eu->image.path),&(eu->image.group));
                 eu->tmp_icon = EINA_TRUE;
               }
          }
        else if(eu->image.path && (!eu->image.group))
          {
             o = elm_icon_add(obj);
             elm_image_file_set(o, eu->image.path, NULL);
          }
        else
          {
             o = entrance_gui_theme_get(obj, eu->image.group);
          }
        //TODO if this fails we maybe should wipe those fields in the config and use a random one
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(o);
        elm_object_part_content_set(ic ,"entrance.icon", o);
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
   const char *style;

   screen = eina_list_data_get(_gui->screens);

   style = edje_object_data_get(elm_layout_edje_get(screen->edj), "item_style_actions");

   if (!style)
     style = "default"; //theme has not settet a style

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        Entrance_Fill *ef;
        ef = entrance_fill_new(style, _entrance_gui_action_text_get,
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

static Eina_Bool
_entrance_gui_cb_window_property(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
   Ecore_X_Event_Window_Property *ev;

   ev = event_info;
   if (ev->atom == ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK)
     {
        PT("screen managed");
        elm_exit();
     }

   return ECORE_CALLBACK_DONE;
}

