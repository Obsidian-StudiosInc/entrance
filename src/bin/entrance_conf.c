#include "entrance_client.h"

typedef struct Entrance_Conf_Background_
{
   const char *path;
   const char *group;
   const char *name;
} Entrance_Conf_Background;

typedef struct Entrance_Int_Conf_
{
   struct
     {
        Evas_Object *preview;
        const char *path;
        const char *group;
     } bg;
   const char *theme;
   const char *elm_profile;
   Eina_Bool vkbd_enabled : 1;
   double scale;
   Eina_Bool update : 1;

   struct
     {
        Entrance_Login *orig;
        struct
          {
             Evas_Object *preview;
             const char *path;
             const char *group;
          } bg;
        struct
          {
             const char *path;
             const char *group;
          } image;
        const char *lsess;
        Eina_Bool remember_session : 1;
        Eina_Bool update : 1;
     } user;

   struct
     {
        Evas_Object *btn_ok;
        Evas_Object *btn_apply;
     } gui;
} Entrance_Int_Conf;


static void _entrance_conf_init(void);
static void _entrance_conf_shutdown(void);
static char *_entrance_conf_bg_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_conf_bg_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _entrance_conf_bg_state_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_conf_bg_sel(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _entrance_conf_bg_fill_cb(void *data, Elm_Object_Item *it);
static Eina_Bool _entrance_conf_user_bg_fill_cb(void *data, Elm_Object_Item *it);
static void _entrance_conf_user_bg_sel(void *data, Evas_Object *obj, void *event_info);
static void _entrance_conf_changed(void);
static void _entrance_conf_apply(void);
static Evas_Object *_entrance_conf_user_build(Evas_Object *obj);
static void _entrance_conf_user_build_cb(Evas_Object *t, Entrance_Login *eu);

static Entrance_Fill *_entrance_background_fill = NULL;
static Entrance_Fill *_entrance_session_fill = NULL;
static Entrance_Int_Conf *_entrance_int_conf = NULL;

static char *
_entrance_conf_bg_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Entrance_Conf_Background *cbg;
   cbg = data;
   return strdup(cbg->name);
}

static Evas_Object *
_entrance_conf_bg_content_get(void *data, Evas_Object *obj, const char *part)
{
   Entrance_Conf_Background *cbg;
   Evas_Object *o = NULL;
   cbg = data;
   if (part && !strcmp("elm.swallow.icon", part))
     {
        o = elm_image_add(obj);
        elm_image_file_set(o, cbg->path, cbg->group);
        elm_image_smooth_set(o, EINA_FALSE);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        evas_object_show(o);
     }
   return o;
}

static Eina_Bool
_entrance_conf_bg_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_entrance_conf_bg_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   Entrance_Conf_Background *cbg;
   cbg = data;
   eina_stringshare_del(cbg->name);
   eina_stringshare_del(cbg->path);
   eina_stringshare_del(cbg->group);
   free(cbg);
}

static void
_entrance_conf_bg_sel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Entrance_Conf_Background *cbg;
   cbg = elm_object_item_data_get(event_info);
   if (!elm_layout_file_set(_entrance_int_conf->bg.preview,
                           cbg->path, cbg->group))
     {
        PT("Error on loading ");
        fprintf(stderr, "%s %s\n", cbg->path, cbg->group);
     }
   _entrance_int_conf->bg.path = cbg->path;
   _entrance_int_conf->bg.group = cbg->group;
   _entrance_conf_changed();
}

static Eina_Bool
_entrance_conf_bg_fill_cb(void *data, Elm_Object_Item *it)
{
   Entrance_Conf_Background *cbg;
   const char *bg_path, *bg_group;
   cbg = data;

   entrance_gui_background_get(&bg_path, &bg_group);
   if ((cbg->path)
       && (cbg->group)
       && (bg_path)
       && (bg_group)
       && (!strcmp(cbg->path, bg_path))
       && (!strcmp(cbg->group, bg_group)))
     {
        elm_genlist_item_selected_set(it, EINA_TRUE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_entrance_conf_user_bg_sel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Entrance_Conf_Background *cbg;
   cbg = elm_object_item_data_get(event_info);
   if (!elm_layout_file_set(_entrance_int_conf->user.bg.preview,
                           cbg->path, cbg->group))
     {
        PT("Error on loading ");
        fprintf(stderr, "%s %s\n", cbg->path, cbg->group);
     }
   _entrance_int_conf->user.bg.path = cbg->path;
   _entrance_int_conf->user.bg.group = cbg->group;
   _entrance_conf_changed();
}

static Eina_Bool
_entrance_conf_user_bg_fill_cb(void *data, Elm_Object_Item *it)
{
   Entrance_Conf_Background *cbg;
   const char *bg_path, *bg_group;
   cbg = data;

   bg_path = _entrance_int_conf->user.orig->bg.path;
   bg_group = _entrance_int_conf->user.orig->bg.group;
   if ((cbg->path)
       && (cbg->group)
       && (bg_path)
       && (bg_group)
       && (!strcmp(cbg->path, bg_path))
       && (!strcmp(cbg->group, bg_group)))
     {
        elm_genlist_item_selected_set(it, EINA_TRUE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}
static char *
_entrance_conf_session_text_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return NULL;
}

static Evas_Object *
_entrance_conf_session_content_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return NULL;
}

static Eina_Bool
_entrance_conf_session_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_entrance_conf_session_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
}

/* vkbd */
static void
_entrance_conf_vkbd_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Elm_Actionslider_Pos p;

   PT("User change vkbd state\n");

   p = elm_actionslider_indicator_pos_get(obj);
   _entrance_int_conf->vkbd_enabled = !!(p == ELM_ACTIONSLIDER_RIGHT);
   _entrance_conf_changed();
}

/* scale */
static void
_entrance_conf_scale_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   _entrance_int_conf->scale = elm_spinner_value_get(obj);
   _entrance_conf_changed();
}

/* Buttons Cb */
static void
_entrance_conf_ok_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _entrance_conf_apply();
   evas_object_del(data);
   _entrance_conf_shutdown();
}

static void
_entrance_conf_apply_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _entrance_conf_apply();
   _entrance_conf_changed();
}

static void
_entrance_conf_close_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   evas_object_del(data);
   _entrance_conf_shutdown();
}


static Eina_List *
_entrance_conf_backgrounds_get(Evas_Object *obj, const char *user)
{
   Evas_Object *o, *edj;
   Eina_List *list, *l, *nl = NULL;
   Entrance_Conf_Background *cbg;
   const char *str;
   const char *path;
   char buf[PATH_MAX];
   Eina_Iterator *it;

   o = entrance_gui_theme_get(obj, "entrance/background");
   edj = elm_layout_edje_get(o);
   edje_object_file_get(edj, &path, NULL);
   list = entrance_gui_stringlist_get(edje_object_data_get(edj, "items"));
   EINA_LIST_FOREACH(list, l, str)
     {
        cbg = calloc(1, sizeof(Entrance_Conf_Background));
        snprintf(buf, sizeof(buf),
                 "entrance/background/%s", str);

        cbg->path = eina_stringshare_add(path);
        cbg->group = eina_stringshare_add(buf);
        cbg->name = eina_stringshare_add(str);
        nl = eina_list_append(nl, cbg);
     }
   entrance_gui_stringlist_free(list);
   evas_object_del(o);

   it = eina_file_ls(PACKAGE_DATA_DIR"/backgrounds");
   EINA_ITERATOR_FOREACH(it, str)
     {
        int len;
        len = strlen(str);
        if (len < 4) continue;
        if (!strcmp(&str[len-4], ".edj"))
          {
             cbg = calloc(1, sizeof(Entrance_Conf_Background));
             snprintf(buf, sizeof(buf),
                      "entrance/background/%s", str);
             cbg->path = str;
             /* TODO use entrance/desktop/background or e/desktop/background */
             cbg->group = eina_stringshare_add("e/desktop/background");
               {
                  char *name, *p;
                  name = strrchr(str, '/');
                  if (name)
                    {
                       name++;
                       name = strdupa(name);
                       p = strrchr(name, '.');
                       if (p) *p = '\0';
                    }
                  cbg->name = eina_stringshare_add(name);
               }
             nl = eina_list_append(nl, cbg);
          }
        else
          eina_stringshare_del(str);
     }
   eina_iterator_free(it);

   return nl;
}

static void
_entrance_conf_apply(void)
{
   Entrance_Conf_Gui_Event conf;

   conf.bg.path = _entrance_int_conf->bg.path;
   conf.bg.group = _entrance_int_conf->bg.group;
   conf.vkbd_enabled = _entrance_int_conf->vkbd_enabled;

   if (_entrance_int_conf->update)
     {
        if (_entrance_int_conf->scale != elm_config_scale_get())
          {
             elm_config_scale_set(_entrance_int_conf->scale);
             elm_config_all_flush();
             elm_config_save();
          }
        if (_entrance_int_conf->theme != entrance_gui_theme_name_get())
          {
             entrance_gui_theme_name_set(_entrance_int_conf->theme);
          }
        entrance_gui_conf_set(&conf);
        entrance_connect_conf_gui_send(&conf);
     }

   if (_entrance_int_conf->user.update)
     {
        Entrance_Login *eu;
        eu = _entrance_int_conf->user.orig;
        if (eu->bg.path != _entrance_int_conf->user.bg.path)
          eina_stringshare_replace(&eu->bg.path,
                                   _entrance_int_conf->user.bg.path);
        if (eu->bg.group != _entrance_int_conf->user.bg.group)
          eina_stringshare_replace(&eu->bg.group,
                                   _entrance_int_conf->user.bg.group);
        if (eu->image.path != _entrance_int_conf->user.image.path)
          eina_stringshare_replace(&eu->image.path,
                                   _entrance_int_conf->user.image.path);
        if (eu->image.group != _entrance_int_conf->user.image.group)
          eina_stringshare_replace(&eu->image.group,
                                   _entrance_int_conf->user.image.group);
        if (eu->remember_session != _entrance_int_conf->user.remember_session)
          eu->remember_session = _entrance_int_conf->user.remember_session;
        if (eu->lsess != _entrance_int_conf->user.lsess)
          eina_stringshare_replace(&eu->lsess, _entrance_int_conf->user.lsess);
        entrance_connect_conf_user_send(eu);
        /*
        printf("%s | %s\n%s | %s\n%s | %s\n%s | %s\n%d | %d\n%s | %s\n",
               eu->bg.path, _entrance_int_conf->user.bg.path,
               eu->bg.group, _entrance_int_conf->user.bg.group,
               eu->image.path, _entrance_int_conf->user.image.path,
               eu->image.group, _entrance_int_conf->user.image.group,
               eu->remember_session, _entrance_int_conf->user.remember_session,
               eu->lsess, _entrance_int_conf->user.lsess);
               */
     }
}

/* Build it */
static Evas_Object *
_entrance_conf_build(Evas_Object *obj)
{
   Evas_Object *t, *bx, *hbx, *o, *gl;
   Eina_List *l;
   int j = 0;

   t = elm_table_add(obj);
   elm_table_padding_set(t, 5 , 5);
   evas_object_size_hint_weight_set(t, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   /* Background */
   o = elm_label_add(t);
   elm_object_text_set(o, "Background");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   ++j;

   hbx = elm_box_add(t);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_table_pack(t, hbx, 0, j, 2, 3);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   gl = elm_genlist_add(hbx);
   elm_scroller_bounce_set(gl, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, gl);
   evas_object_show(gl);
   bx = elm_box_add(hbx);
   elm_box_pack_end(hbx, bx);
   evas_object_show(bx);
   o = elm_layout_add(hbx);
   _entrance_int_conf->bg.preview = o;
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);
   o = evas_object_rectangle_add(hbx);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_size_hint_min_set(o, 256, 0);
   elm_box_pack_end(bx, o);
   evas_object_show(o);
   evas_object_show(hbx);
   j += 3;
   l = _entrance_conf_backgrounds_get(gl, NULL);
   entrance_fill(gl, _entrance_background_fill,
                 l, _entrance_conf_bg_fill_cb, _entrance_conf_bg_sel, o);
   eina_list_free(l);

   /* Touch Screen */
   o = elm_label_add(t);
   elm_object_text_set(o, "Use a virtual keyboard");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   o = elm_actionslider_add(t);
   elm_object_style_set(o, "bar");
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_text_set(o, "left", "Disabled");
   elm_object_part_text_set(o, "right", "Enabled");

   elm_actionslider_magnet_pos_set(o, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_enabled_pos_set(o, ELM_ACTIONSLIDER_LEFT |
                                    ELM_ACTIONSLIDER_RIGHT);
   evas_object_smart_callback_add(o, "selected",
                                  _entrance_conf_vkbd_changed, NULL);
   if (_entrance_int_conf->vkbd_enabled)
     elm_actionslider_indicator_pos_set(o, ELM_ACTIONSLIDER_RIGHT);
   else
     elm_actionslider_indicator_pos_set(o, ELM_ACTIONSLIDER_LEFT);
   elm_table_pack(t, o, 1, j, 1, 1);
   evas_object_show(o);
   ++j;

   /* Elementary Profile */
   o = elm_label_add(t);
   elm_object_text_set(o, "elementary profile");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   o = elm_hoversel_add(t);
   elm_object_text_set(o, _entrance_int_conf->elm_profile);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 1, j, 1, 1);
   evas_object_show(o);
   ++j;

   /* Scaling */
   o = elm_label_add(t);
   elm_object_text_set(o, "Scaling");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   o = elm_spinner_add(t);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "changed",
                                  _entrance_conf_scale_changed, NULL);
   elm_spinner_value_set(o, _entrance_int_conf->scale);
   elm_spinner_min_max_set(o, 0.25, 5.0);
   elm_spinner_step_set(o, 0.15);
   elm_spinner_label_format_set(o, "%.2f");

   elm_table_pack(t, o, 1, j, 1, 1);
   evas_object_show(o);
   ++j;

   o = evas_object_rectangle_add(o);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_size_hint_min_set(o, 128, 0);
   elm_table_pack(t, o, 1, j, 1, 1);

   evas_object_show(t);
   return t;
}

static void
_entrance_conf_user_auth(void *data, const char *user, Eina_Bool granted)
{
   Evas_Object *t;
   const Eina_List *users, *l;
   Entrance_Login *eu;

   if (granted)
     {
        t = elm_object_part_content_get(data, "entrance.conf");
        users = entrance_gui_users_get();
        EINA_LIST_FOREACH(users, l, eu)
          {
             if (!strcmp(eu->login, user))
               {
                  _entrance_int_conf->user.orig = eu;
                  _entrance_int_conf->user.bg.path = eu->bg.path;
                  _entrance_int_conf->user.bg.group = eu->bg.group;
                  _entrance_int_conf->user.image.path = eu->image.path;
                  _entrance_int_conf->user.image.group = eu->image.group;
                  _entrance_int_conf->user.lsess = eu->lsess;
                  _entrance_int_conf->user.remember_session
                     = eu->remember_session;
                  /*
                  printf("init %s %s | %s %s | %s | %d\n",
                         _entrance_int_conf->user.bg.path,
                         _entrance_int_conf->user.bg.group,
                         _entrance_int_conf->user.image.path,
                         _entrance_int_conf->user.image.group,
                         _entrance_int_conf->user.lsess,
                         _entrance_int_conf->user.remember_session);
                         */
                  break;
               }
          }
        if (eu)
          {
             _entrance_conf_user_build_cb(t, eu);
             elm_object_signal_emit(data, "entrance,conf_user,enabled", "");
          }
     }
}


static Evas_Object *
_entrance_conf_user_build(Evas_Object *obj)
{
   Evas_Object *t, *o, *ly;

   ly = entrance_gui_theme_get(obj, "entrance/conf/login");
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(ly);

   o = entrance_login_add(ly, _entrance_conf_user_auth, ly);
   entrance_login_open_session_set(o, EINA_FALSE);
   elm_object_part_content_set(ly, "entrance.login", o);
   evas_object_show(o);
   t = elm_table_add(obj);
   elm_object_part_content_set(ly, "entrance.conf", t);
   evas_object_size_hint_weight_set(t, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_table_padding_set(t, 5 , 5);

   return ly;
}

static void
_entrance_conf_user_build_cb(Evas_Object *t, Entrance_Login *eu)
{
   Evas_Object *o, *gl, *bx, *hbx;
   Eina_List *l;
   int j = 0;

   /* Background */
   o = elm_label_add(t);
   elm_object_text_set(o, "Background");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   ++j;
   hbx = elm_box_add(t);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_table_pack(t, hbx, 0, j, 2, 1);
   ++j;
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   gl = elm_genlist_add(hbx);
   elm_scroller_bounce_set(gl, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, gl);
   evas_object_show(gl);
   bx = elm_box_add(hbx);
   elm_box_pack_end(hbx, bx);
   evas_object_show(bx);
   o = elm_layout_add(hbx);
   _entrance_int_conf->user.bg.preview = o;
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);
   o = evas_object_rectangle_add(hbx);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_size_hint_min_set(o, 256, 0);
   elm_box_pack_end(bx, o);
   evas_object_show(o);
   evas_object_show(hbx);
   l = _entrance_conf_backgrounds_get(o, eu->login);
   entrance_fill(gl, _entrance_background_fill,
                 l, _entrance_conf_user_bg_fill_cb,
                 _entrance_conf_user_bg_sel, o);
   eina_list_free(l);

   /* Icon */

   /* Session to autoselect */
   o = elm_label_add(t);
   elm_object_text_set(o, "Session to use");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   o = elm_hoversel_add(t);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Session");
   elm_table_pack(t, o, 1, j, 1, 1);
   evas_object_show(o);
   ++j;

   /* Remember last session */
   o = elm_label_add(t);
   elm_object_text_set(o, "Remember last session");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(t, o, 0, j, 1, 1);
   evas_object_show(o);
   o = elm_actionslider_add(t);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_text_set(o, "left", "Enabled");
   elm_object_part_text_set(o, "right", "Disabled");
   elm_table_pack(t, o, 1, j, 1, 1);
   evas_object_show(o);
   ++j;

   evas_object_show(t);
}


static void
_entrance_conf_control_changed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Elm_Object_Item *it;
   it = event;
   elm_naviframe_item_promote(elm_object_item_data_get(it));
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void
_entrance_conf_changed(void)
{
   const char *bg_path;
   const char *bg_group;

   entrance_gui_background_get(&bg_path, &bg_group);
   _entrance_int_conf->update =
      !!((_entrance_int_conf->theme != entrance_gui_theme_name_get())
         || (_entrance_int_conf->bg.path != bg_path)
         || (_entrance_int_conf->bg.group != bg_group)
         || (_entrance_int_conf->scale != elm_config_scale_get())
         || (_entrance_int_conf->elm_profile != elm_config_profile_get())
         || (_entrance_int_conf->vkbd_enabled != entrance_gui_vkbd_enabled_get()));

   if (_entrance_int_conf->user.orig)
     _entrance_int_conf->user.update =
        !!((_entrance_int_conf->user.orig->bg.path != _entrance_int_conf->user.bg.path)
           || (_entrance_int_conf->user.orig->bg.group != _entrance_int_conf->user.bg.group)
           || (_entrance_int_conf->user.orig->image.path != _entrance_int_conf->user.image.path)
           || (_entrance_int_conf->user.orig->image.group != _entrance_int_conf->user.image.group)
           || (_entrance_int_conf->user.orig->remember_session != _entrance_int_conf->user.remember_session)
           || (_entrance_int_conf->user.orig->lsess != _entrance_int_conf->user.lsess));
   if (_entrance_int_conf->update || _entrance_int_conf->user.update)
     {
        elm_object_disabled_set(_entrance_int_conf->gui.btn_ok, EINA_FALSE);
        elm_object_disabled_set(_entrance_int_conf->gui.btn_apply, EINA_FALSE);
     }
   else
     {
        elm_object_disabled_set(_entrance_int_conf->gui.btn_ok, EINA_TRUE);
        elm_object_disabled_set(_entrance_int_conf->gui.btn_apply, EINA_TRUE);
     }
}


static void
_entrance_conf_init(void)
{
   PT("conf init\n");
   _entrance_background_fill =
      entrance_fill_new("default",
                        _entrance_conf_bg_text_get,
                        _entrance_conf_bg_content_get,
                        _entrance_conf_bg_state_get,
                        _entrance_conf_bg_del);
   _entrance_session_fill =
      entrance_fill_new("default",
                        _entrance_conf_session_text_get,
                        _entrance_conf_session_content_get,
                        _entrance_conf_session_state_get,
                        _entrance_conf_session_del);

   _entrance_int_conf = calloc(1, sizeof(Entrance_Int_Conf));
   _entrance_int_conf->theme = entrance_gui_theme_name_get();
   entrance_gui_background_get(&(_entrance_int_conf->bg.path),
                               &(_entrance_int_conf->bg.group));
   _entrance_int_conf->vkbd_enabled = entrance_gui_vkbd_enabled_get();
   _entrance_int_conf->scale = elm_config_scale_get();
   _entrance_int_conf->elm_profile = elm_config_profile_get();
}

static void
_entrance_conf_shutdown(void)
{
   PT("conf shutdown\n");
   entrance_fill_del(_entrance_background_fill);
   entrance_fill_del(_entrance_session_fill);
   free(_entrance_int_conf);
}


void
entrance_conf_begin(Evas_Object *obj, Evas_Object *parent)
{
   PT("Build config panel\n");
   Evas_Object *win, *bg, *bx, *hbx, *sc, *nf, *o;
   Elm_Object_Item *it, *itc;
   Elm_Object_Item *itu, *ituc;

   _entrance_conf_init();

   win = elm_hover_add(obj);
   elm_hover_parent_set(win, parent);
//   win = elm_popup_add(obj);
   bg = elm_bg_add(win);
   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(win, bx);

   sc = elm_segment_control_add(bx);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, sc);
   evas_object_smart_callback_add(sc, "changed",
                                  _entrance_conf_control_changed, NULL);
   evas_object_show(sc);

   nf = elm_naviframe_add(bx);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, nf);
   evas_object_show(nf);

   /* Ok Apply Close */
   hbx = elm_box_add(bx);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_box_homogeneous_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   o = elm_button_add(hbx);
   elm_object_text_set(o, "Ok");
   elm_object_disabled_set(o, EINA_TRUE);
   evas_object_smart_callback_add(o, "clicked",
                                  _entrance_conf_ok_clicked, win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, o);
   _entrance_int_conf->gui.btn_ok = o;
   evas_object_show(o);

   o = elm_button_add(hbx);
   elm_object_text_set(o, "Apply");
   elm_object_disabled_set(o, EINA_TRUE);
   evas_object_smart_callback_add(o, "clicked",
                                  _entrance_conf_apply_clicked, NULL);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, o);
   _entrance_int_conf->gui.btn_apply = o;
   evas_object_show(o);

   o = elm_button_add(hbx);
   elm_object_text_set(o, "Close");
   evas_object_smart_callback_add(o, "clicked",
                                  _entrance_conf_close_clicked, win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, o);
   evas_object_show(o);

   /* Build configuration panel */
   /* main */
   o = _entrance_conf_build(nf);
   it = elm_naviframe_item_simple_push(nf, o);
   itc = elm_segment_control_item_add(sc, NULL, "General");
   elm_object_item_data_set(itc, it);

   /* user */
   o = _entrance_conf_user_build(nf);
   itu = elm_naviframe_item_simple_push(nf, o);
   ituc = elm_segment_control_item_add(sc, NULL, "User");
   elm_object_item_data_set(ituc, itu);


   elm_segment_control_item_selected_set(itc, EINA_TRUE);

     {
        Evas_Coord xx, yy, ww, hh, x, y, w, h;
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_object_geometry_get(bx, NULL, NULL, &ww, &hh);
        xx = 50;//x + (w / 2) - (ww / 2);
        yy = 50;//y + (y / 2) - (hh / 2);
        evas_object_move(bg, xx, yy);
        evas_object_resize(bg, w - 100, h - 100);
        evas_object_move(bx, xx, yy);
        evas_object_resize(bx, w - 100, h - 100);
     }
   evas_object_show(bg);
   evas_object_show(bx);
   evas_object_show(win);
   evas_object_freeze_events_set(win, EINA_TRUE);
}

