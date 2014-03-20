#include "entrance_client.h"

typedef struct Entrance_Int_Conf_User_
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
} Entrance_Int_Conf_User;


static void _entrance_conf_user_bg_sel(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _entrance_conf_user_bg_fill_cb(void *data, Elm_Object_Item *it);
static char *_entrance_conf_session_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_conf_session_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _entrance_conf_session_state_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_conf_session_del(void *data, Evas_Object *obj);
static Evas_Object *_entrance_conf_user_build(Evas_Object *obj);
static void _entrance_conf_user_build_cb(Evas_Object *t, Entrance_Login *eu);
static Eina_Bool _entrance_conf_user_check(void);
static void _entrance_conf_user_apply(void);

static Entrance_Fill *_entrance_session_fill = NULL;
static Entrance_Int_Conf_User *_entrance_int_conf_user = NULL;

static void
_entrance_conf_user_bg_sel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Entrance_Conf_Background *cbg;
   cbg = elm_object_item_data_get(event_info);
   if (!elm_layout_file_set(_entrance_int_conf_user->bg.preview,
                           cbg->path, cbg->group))
     {
        PT("Error on loading ");
        fprintf(stderr, "%s %s\n", cbg->path, cbg->group);
     }
   _entrance_int_conf_user->bg.path = cbg->path;
   _entrance_int_conf_user->bg.group = cbg->group;
   entrance_conf_changed();
}

static Eina_Bool
_entrance_conf_user_bg_fill_cb(void *data, Elm_Object_Item *it)
{
   Entrance_Conf_Background *cbg;
   const char *bg_path, *bg_group;
   cbg = data;

   bg_path = _entrance_int_conf_user->orig->bg.path;
   bg_group = _entrance_int_conf_user->orig->bg.group;
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
                  _entrance_int_conf_user->orig = eu;
                  _entrance_int_conf_user->bg.path = eu->bg.path;
                  _entrance_int_conf_user->bg.group = eu->bg.group;
                  _entrance_int_conf_user->image.path = eu->image.path;
                  _entrance_int_conf_user->image.group = eu->image.group;
                  _entrance_int_conf_user->lsess = eu->lsess;
                  _entrance_int_conf_user->remember_session =
                     eu->remember_session;
                  /*
                  printf("init %s %s | %s %s | %s | %d\n",
                         _entrance_int_conf_user->bg.path,
                         _entrance_int_conf_user->bg.group,
                         _entrance_int_conf_user->image.path,
                         _entrance_int_conf_user->image.group,
                         _entrance_int_conf_user->lsess,
                         _entrance_int_conf_user->remember_session);
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
_entrance_conf_user_begin(void)
{
   _entrance_int_conf_user = calloc(1, sizeof(Entrance_Int_Conf_User));

}

static void
_entrance_conf_user_end(void)
{
   free(_entrance_int_conf_user);

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
   _entrance_int_conf_user->bg.preview = o;
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
   l = entrance_conf_backgrounds_get(o, eu->login);
   entrance_fill(gl, entrance_conf_background_fill_get(),
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


static Eina_Bool
_entrance_conf_user_check(void)
{
   Entrance_Login *eu = _entrance_int_conf_user->orig;
   return !!((eu->bg.path != _entrance_int_conf_user->bg.path)
             || (eu->bg.group != _entrance_int_conf_user->bg.group)
             || (eu->image.path != _entrance_int_conf_user->image.path)
             || (eu->image.group != _entrance_int_conf_user->image.group)
             || (eu->remember_session != _entrance_int_conf_user->remember_session)
             || (eu->lsess != _entrance_int_conf_user->lsess));
}

static void
_entrance_conf_user_apply(void)
{
   Entrance_Login *eu;
   eu = _entrance_int_conf_user->orig;
   if (eu->bg.path != _entrance_int_conf_user->bg.path)
     eina_stringshare_replace(&eu->bg.path,
                              _entrance_int_conf_user->bg.path);
   if (eu->bg.group != _entrance_int_conf_user->bg.group)
     eina_stringshare_replace(&eu->bg.group,
                              _entrance_int_conf_user->bg.group);
   if (eu->image.path != _entrance_int_conf_user->image.path)
     eina_stringshare_replace(&eu->image.path,
                              _entrance_int_conf_user->image.path);
   if (eu->image.group != _entrance_int_conf_user->image.group)
     eina_stringshare_replace(&eu->image.group,
                              _entrance_int_conf_user->image.group);
   if (eu->remember_session != _entrance_int_conf_user->remember_session)
     eu->remember_session = _entrance_int_conf_user->remember_session;
   if (eu->lsess != _entrance_int_conf_user->lsess)
     eina_stringshare_replace(&eu->lsess, _entrance_int_conf_user->lsess);
   entrance_connect_conf_user_send(eu);
   /*
      printf("%s | %s\n%s | %s\n%s | %s\n%s | %s\n%d | %d\n%s | %s\n",
      eu->bg.path, _entrance_int_conf->bg.path,
      eu->bg.group, _entrance_int_conf->bg.group,
      eu->image.path, _entrance_int_conf->image.path,
      eu->image.group, _entrance_int_conf->image.group,
      eu->remember_session, _entrance_int_conf->remember_session,
      eu->lsess, _entrance_int_conf->lsess);
    */
}

void
entrance_conf_user_init(void)
{
   PT("conf user init\n");
   _entrance_session_fill =
      entrance_fill_new("default",
                        _entrance_conf_session_text_get,
                        _entrance_conf_session_content_get,
                        _entrance_conf_session_state_get,
                        _entrance_conf_session_del);
   entrance_conf_module_register("User",
                                 _entrance_conf_user_begin,
                                 _entrance_conf_user_end,
                                 _entrance_conf_user_build,
                                 _entrance_conf_user_check,
                                 _entrance_conf_user_apply);

}

void
entrance_conf_user_shutdown(void)
{
   PT("conf user shutdown\n");
   entrance_fill_del(_entrance_session_fill);
}

