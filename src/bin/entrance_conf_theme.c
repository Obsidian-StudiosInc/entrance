#include "entrance_client.h"

typedef struct Entrance_Int_Conf_Theme_
{
   const char *theme;
   Evas_Object *preview;
   Evas_Object *panel;
} Entrance_Int_Conf_Theme;

typedef struct Entrance_Theme_Preview_
{
   Evas_Object *transition;
   Evas_Object *background;
   Evas_Object *edj;
   Evas_Object *login;
   Evas_Object *user_bg;
   Ecore_Timer *active_timer;
} Entrance_Theme_Preview;

typedef struct String_Animation_
{
   const char *text;
   unsigned int position;
   Entrance_Theme_Preview *root;
} String_Animation;

static void _entrance_conf_theme_begin(void);
static void _entrance_conf_theme_update_preview(const char *name);
static void _entrance_conf_theme_end(void);
static Eina_Bool _entrance_conf_themes_fill_cb(void *data, Elm_Object_Item *it);
static void _entrance_conf_themes_sel(void *data, Evas_Object *obj, void *event_info);
static char* _entrance_conf_theme_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_conf_theme_build(Evas_Object *obj);
static Eina_Bool _entrance_conf_theme_check(void);
static void _entrance_conf_theme_apply(void);
static Entrance_Int_Conf_Theme *_entrance_int_conf_theme = NULL;

/* theme preview */
static char* _entrance_tp_user_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object* _entrance_tp_user_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _entrance_tp_user_state_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_tp_user_del(void *data, Evas_Object *obj);
static Eina_Bool _entrance_tp_login_simulation_reset(void *data);
static Eina_Bool _entrance_tp_login_feedback_timer(void *data);
static Eina_Bool _entrance_tp_string_animation(void *data);
static Eina_Bool _entrance_tp_login_timer(void *data);
static void _entrance_tp_animation_reset(void *data);
static void _entrance_tp_animation_init(Entrance_Theme_Preview *l);
static Evas_Object* _entrance_tp_add(Evas_Object *par, const char *name);
static void _entrance_tp_del(Evas_Object *par);


static void
_entrance_conf_theme_begin(void)
{
   _entrance_int_conf_theme = calloc(1, sizeof(Entrance_Int_Conf_Theme));
   _entrance_int_conf_theme->theme = entrance_gui_theme_name_get();
}



static void
_entrance_conf_theme_update_preview(const char *name)
{
   Evas_Object *o;

   o = elm_object_part_content_get(_entrance_int_conf_theme->preview, "default");
   if (o)
     _entrance_tp_del(o);

   o = _entrance_tp_add(_entrance_int_conf_theme->preview, name);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_object_part_content_set(_entrance_int_conf_theme->preview, "default", o);
   evas_object_show(o);
}

static void
_entrance_conf_theme_end(void)
{
   evas_object_del(_entrance_int_conf_theme->preview);
   free(_entrance_int_conf_theme);
}

static Eina_Bool
_entrance_conf_themes_fill_cb(void *data, Elm_Object_Item *it)
{
   char *theme = data;
   if (!entrance_gui_theme_name_get() && !strcmp(theme, "default"))
     elm_list_item_selected_set(it, EINA_TRUE);
   else if(entrance_gui_theme_name_get())
     if (!strcmp(theme, entrance_gui_theme_name_get()))
       elm_list_item_selected_set(it, EINA_TRUE);
   return EINA_FALSE;
}

static void
_entrance_conf_themes_sel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char *name = data;
   _entrance_int_conf_theme->theme = name;
   _entrance_conf_theme_update_preview(name);
   elm_panel_hidden_set(_entrance_int_conf_theme->panel, EINA_TRUE);
   entrance_conf_changed();
}

static char *
_entrance_conf_theme_text_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup((char*)data);
}

static Evas_Object *
_entrance_conf_theme_build(Evas_Object *obj)
{
   Evas_Object *o, *p, *tb, *bx;
   Entrance_Fill *list_fill = entrance_fill_new("",
                                                _entrance_conf_theme_text_get,
                                                NULL,
                                                NULL,
                                                NULL);

   tb = elm_table_add(obj);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(tb);

   bx = o = elm_box_add(obj);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, o, 0 , 0 , 5, 5);
   evas_object_show(o);

   _entrance_int_conf_theme->preview = o = elm_frame_add(obj);
   elm_object_style_set(o, "pad_huge");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_freeze_events_set(o, EINA_TRUE);
   elm_box_pack_end(bx, o);
   evas_object_show(o);
   _entrance_conf_theme_update_preview(entrance_gui_theme_name_get());

   _entrance_int_conf_theme->panel = p = elm_panel_add(obj);
   elm_panel_orient_set(p, ELM_PANEL_ORIENT_LEFT);
   evas_object_size_hint_weight_set(p, 0.15, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(p, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, p, 0, 0, 1, 5);
   evas_object_show(p);

   o = elm_list_add(obj);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(p, o);

   entrance_fill(o, list_fill, entrance_gui_themes_get(), _entrance_conf_themes_fill_cb,
                 _entrance_conf_themes_sel, NULL);
   entrance_fill_del(list_fill);
   return tb;
}

static Eina_Bool
_entrance_conf_theme_check(void)
{
   return !!strcmp(_entrance_int_conf_theme->theme,entrance_gui_theme_name_get());
}

static void
_entrance_conf_theme_apply(void)
{
   Entrance_Conf_Gui_Event conf;

   conf.vkbd_enabled = entrance_gui_vkbd_enabled_get();
   conf.theme = eina_stringshare_add(_entrance_int_conf_theme->theme);
   entrance_gui_background_get(&conf.bg.path, &conf.bg.group);

   entrance_connect_conf_gui_send(&conf);
   entrance_gui_theme_name_set(conf.theme);
   entrance_conf_changed();
}


void
entrance_conf_theme_init(void)
{
   PT("conf theme init");
   entrance_conf_module_register("Theme Selector",
                                 _entrance_conf_theme_begin,
                                 _entrance_conf_theme_end,
                                 _entrance_conf_theme_build,
                                 _entrance_conf_theme_check,
                                 _entrance_conf_theme_apply);
}

void
entrance_conf_theme_shutdown(void)
{
   PT("conf theme shutdown");
}


/* THEME PREVIEW WIDGET FUNCTIONS */

static char *
_entrance_tp_user_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
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
_entrance_tp_user_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = NULL, *o;
   Entrance_Login *eu;
   eu = data;
   if (eu && !strcmp(part, "elm.swallow.icon"))
     {
        ic = entrance_gui_theme_get(obj, "entrance/user");
        if(eu->image.path && (!eu->image.group))
          {
             o = elm_icon_add(obj);
             elm_image_file_set(o, eu->image.path, NULL);
          }
        else
          {
             o = entrance_gui_theme_get(obj, eu->image.group);
          }
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(o);
        elm_object_part_content_set(ic ,"entrance.icon", o);
     }
   return ic;
}

static Eina_Bool
_entrance_tp_user_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_entrance_tp_user_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{

}
static Eina_Bool
_entrance_tp_login_simulation_reset(void *data)
{
   String_Animation *ani = data;
   ani->root->active_timer = NULL;
   _entrance_tp_animation_reset(data);
   elm_panel_hidden_set(_entrance_int_conf_theme->panel, EINA_FALSE);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_entrance_tp_login_feedback_timer(void *data)
{
   String_Animation *ani = data;
   Evas_Object *w = ani->root->login;
   elm_object_signal_emit(w,
                          "entrance,auth,error", "");
   ani->root->active_timer = ecore_timer_add(2, _entrance_tp_login_simulation_reset, data);
   return ECORE_CALLBACK_CANCEL;
}

static void
_entrance_conf_theme_user_bg_cb(void *data , Evas_Object *obj EINA_UNUSED,
                                const char *sig EINA_UNUSED,
                                const char *src EINA_UNUSED)
{
   Evas_Object *o;
   Entrance_Theme_Preview *pre = data;
   o = elm_object_part_content_get(pre->transition,
                                   "entrance.wallpaper.user");
   evas_object_del(o);
   o = elm_object_part_content_get(pre->transition,
                                   "entrance.wallpaper.user.start");

   if (o)
       elm_object_part_content_set(pre->transition,
                                   "entrance.wallpaper.user", o);
}


static Eina_Bool
_entrance_tp_string_animation(void *data)
{
   Evas_Object *l;
   String_Animation *ani = data;
   char txt[PATH_MAX];

   l = elm_object_part_content_get(ani->root->login, "entrance.login");

   snprintf(txt, sizeof(txt), "%*.*s", ani->position, ani->position, ani->text);
   elm_object_text_set(l, txt);
   elm_entry_cursor_end_set(l);
   elm_object_focus_set(l, EINA_TRUE);
   if (ani->position >= strlen(ani->text))
     {
       elm_object_part_content_set(ani->root->transition,
                                   "entrance.wallpaper.user.start",
                                   ani->root->user_bg);
       elm_object_signal_emit(ani->root->transition,
                                  "entrance,wallpaper,user", "");
       elm_object_signal_callback_add(ani->root->transition, "entrance,wallpaper,end", "",
                                      _entrance_conf_theme_user_bg_cb, ani->root);

       ani->root->active_timer = ecore_timer_add(1, _entrance_tp_login_timer, data);
       return ECORE_CALLBACK_CANCEL;
     }
   ani->position ++;
   ani->root->active_timer = ecore_timer_add(0.25, _entrance_tp_string_animation, data);

   return ECORE_CALLBACK_CANCEL;
}
static Eina_Bool
_entrance_tp_login_timer(void *data)
{
   String_Animation *ani = data;
   Evas_Object *w = ani->root->login;
   elm_object_signal_emit(w,
                          "entrance,auth,checking", "");
   ani->root->active_timer = ecore_timer_add(4, _entrance_tp_login_feedback_timer, data);
   return ECORE_CALLBACK_CANCEL;
}

static void
_entrance_tp_animation_reset(void *data)
{
   String_Animation *ani = data;
   free(ani);
}
static void
_entrance_tp_animation_init(Entrance_Theme_Preview *p)
{
   String_Animation *ani;
   Entrance_Login *el;

   el = eina_list_data_get(entrance_gui_users_get());

   ani = calloc(1, sizeof(String_Animation));
   ani->text = eina_stringshare_add(el->login);
   ani->root = p;
   ani->position = 1;
   ani->root->active_timer = ecore_timer_add(1, _entrance_tp_string_animation, ani);
}

static Evas_Object*
_entrance_tp_random_bg(Evas_Object *par, char *theme_path)
{
   Evas_Object *o, *res;
   const char *icons, *ic;
   char **user_bg, buf[PATH_MAX];
   int i = 0, rnd = 0;
   o = elm_layout_add(par);
   elm_layout_file_set(o, theme_path, "entrance/background");

   icons = edje_object_data_get(elm_layout_edje_get(o), "items");
   if (!icons)
     goto bad_end;

   user_bg = eina_str_split(icons, " ", 0);
   if (!user_bg)
     goto bad_end;

   while(user_bg[i])
      i++;

   srand(time(NULL));
   rnd = i * (double) rand()/ (RAND_MAX + 1.0);
   ic = user_bg[rnd];

   snprintf(buf, sizeof(buf), "entrance/background/%s",ic);

   res = elm_layout_add(par);
   elm_layout_file_set(res, theme_path, buf);
   evas_object_del(o);
//   for (i = 0; user_bg[i]; i++)
      //free(user_bg[i]);
   //free(user_bg);
   return res;
bad_end:
   evas_object_del(o);
   return NULL;
}

static Evas_Object*
_entrance_tp_add(Evas_Object *par, const char *name)
{
   Evas_Object *o, *oo;
   Eina_List *l = NULL;
   char buf[PATH_MAX];
   const char *style;
   Entrance_Fill *ef;
   Entrance_Theme_Preview *pre;

   pre = calloc(1, sizeof(Entrance_Theme_Preview));
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", name);

   /*root layout*/
   o = elm_layout_add(par);
   elm_layout_file_set(o, buf, "entrance/wallpaper/default");
   pre->transition = o;

   o = elm_layout_add(pre->transition);
   elm_layout_file_set(o, buf, "entrance/background/default");
   elm_object_part_content_set(pre->transition, "entrance.wallpaper.default", o);
   pre->background = o;

   /*screen setup*/
   o = elm_layout_add(pre->transition);
   elm_layout_file_set(o, buf, "entrance");
   elm_object_part_content_set(pre->transition, "entrance.screen", o);
   edje_object_signal_emit(elm_layout_edje_get(o),
                           "entrance,action,enabled", "");
   pre->edj = o;

   /*the login*/
   o = elm_layout_add(pre->transition);
   elm_layout_file_set(o, buf, "entrance/login");
   pre->login = o;
   elm_object_part_content_set(pre->edj, "entrance.login", o);

   oo = elm_entry_add(o);
   elm_entry_single_line_set(oo, EINA_TRUE);
   elm_entry_scrollable_set(oo, EINA_TRUE);
   evas_object_show(oo);
   elm_object_part_content_set(pre->login, "entrance.login", oo);

   oo = elm_entry_add(o);
   elm_entry_single_line_set(oo, EINA_TRUE);
   elm_entry_scrollable_set(oo, EINA_TRUE);
   evas_object_show(oo);
   elm_object_part_content_set(pre->login, "entrance.password", oo);

   oo = elm_hoversel_add(o);
   elm_hoversel_hover_parent_set(oo, pre->transition);
   elm_object_text_set(oo, "Xsession");
   elm_object_part_content_set(pre->login, "entrance.xsessions", oo);


   /*sample user background*/
   pre->user_bg = _entrance_tp_random_bg(o, buf);

   style = edje_object_data_get(elm_layout_edje_get(pre->edj), "item_style_users");

   ef = entrance_fill_new(style,
                       _entrance_tp_user_text_get,
                       _entrance_tp_user_content_get,
                       _entrance_tp_user_state_get,
                       _entrance_tp_user_del);
   o = ENTRANCE_GUI_GET(pre->edj, "entrance.users");
   l = entrance_gui_users_get();
   if(l)
     entrance_fill(o, ef, l, NULL, NULL, NULL);
   entrance_fill_del(ef);
   eina_list_free(l);

   _entrance_tp_animation_init(pre);
   evas_object_smart_data_set(pre->transition, pre);
   return pre->transition;
}

static void
_entrance_tp_del(Evas_Object *obj)
{
   Entrance_Theme_Preview *pre = evas_object_smart_data_get(obj);
   String_Animation *ani;

  if ((pre) && pre->active_timer)
    {
       ani = ecore_timer_del(pre->active_timer);
       free(ani);
    }

  evas_object_del(obj);
}
