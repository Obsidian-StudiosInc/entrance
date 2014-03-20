#include "entrance_client.h"

typedef struct Entrance_Int_Conf_Main_
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
} Entrance_Int_Conf_Main;

static void _entrance_conf_main_begin(void);
static void _entrance_conf_main_end(void);
static Eina_Bool _entrance_conf_bg_fill_cb(void *data, Elm_Object_Item *it);
static void _entrance_conf_bg_sel(void *data, Evas_Object *obj, void *event_info);
static void _entrance_conf_vkbd_changed(void *data, Evas_Object *obj, void *event);
static void _entrance_conf_scale_changed(void *data, Evas_Object *obj, void *event);
static Evas_Object *_entrance_conf_main_build(Evas_Object *obj);
static Eina_Bool _entrance_conf_main_check(void);
static void _entrance_conf_main_apply(void);

static Entrance_Int_Conf_Main *_entrance_int_conf_main = NULL;


static void
_entrance_conf_main_begin(void)
{
   _entrance_int_conf_main = calloc(1, sizeof(Entrance_Int_Conf_Main));
   _entrance_int_conf_main->theme = entrance_gui_theme_name_get();
   entrance_gui_background_get(&(_entrance_int_conf_main->bg.path),
                               &(_entrance_int_conf_main->bg.group));
   _entrance_int_conf_main->vkbd_enabled = entrance_gui_vkbd_enabled_get();
   _entrance_int_conf_main->scale = elm_config_scale_get();
   _entrance_int_conf_main->elm_profile = elm_config_profile_get();

}

static void
_entrance_conf_main_end(void)
{
   free(_entrance_int_conf_main);
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
_entrance_conf_bg_sel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Entrance_Conf_Background *cbg;
   cbg = elm_object_item_data_get(event_info);
   if (!elm_layout_file_set(_entrance_int_conf_main->bg.preview,
                           cbg->path, cbg->group))
     {
        PT("Error on loading ");
        fprintf(stderr, "%s %s\n", cbg->path, cbg->group);
     }
   _entrance_int_conf_main->bg.path = cbg->path;
   _entrance_int_conf_main->bg.group = cbg->group;
   entrance_conf_changed();
}

/* vkbd */
static void
_entrance_conf_vkbd_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Elm_Actionslider_Pos p;

   PT("User change vkbd state\n");

   p = elm_actionslider_indicator_pos_get(obj);
   _entrance_int_conf_main->vkbd_enabled = !!(p == ELM_ACTIONSLIDER_RIGHT);
   entrance_conf_changed();
}

/* scale */
static void
_entrance_conf_scale_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   _entrance_int_conf_main->scale = elm_spinner_value_get(obj);
   entrance_conf_changed();
}


static Evas_Object *
_entrance_conf_main_build(Evas_Object *obj)
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
   _entrance_int_conf_main->bg.preview = o;
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
   l = entrance_conf_backgrounds_get(gl, NULL);
   entrance_fill(gl, entrance_conf_background_fill_get(),
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
   if (_entrance_int_conf_main->vkbd_enabled)
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
   elm_object_text_set(o, _entrance_int_conf_main->elm_profile);
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
   elm_spinner_value_set(o, _entrance_int_conf_main->scale);
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

static Eina_Bool
_entrance_conf_main_check(void)
{
   const char *bg_path;
   const char *bg_group;

   entrance_gui_background_get(&bg_path, &bg_group);
   return !!((_entrance_int_conf_main->theme != entrance_gui_theme_name_get())
             || (_entrance_int_conf_main->bg.path != bg_path)
             || (_entrance_int_conf_main->bg.group != bg_group)
             || (_entrance_int_conf_main->scale != elm_config_scale_get())
             || (_entrance_int_conf_main->elm_profile != elm_config_profile_get())
             || (_entrance_int_conf_main->vkbd_enabled != entrance_gui_vkbd_enabled_get()));
}

static void
_entrance_conf_main_apply(void)
{
   Entrance_Conf_Gui_Event conf;

   conf.bg.path = _entrance_int_conf_main->bg.path;
   conf.bg.group = _entrance_int_conf_main->bg.group;
   conf.vkbd_enabled = _entrance_int_conf_main->vkbd_enabled;

   if (_entrance_int_conf_main->scale != elm_config_scale_get())
     {
        elm_config_scale_set(_entrance_int_conf_main->scale);
        elm_config_all_flush();
        elm_config_save();
     }
   if (_entrance_int_conf_main->theme != entrance_gui_theme_name_get())
     {
        entrance_gui_theme_name_set(_entrance_int_conf_main->theme);
     }
   entrance_gui_conf_set(&conf);
   entrance_connect_conf_gui_send(&conf);
}


void
entrance_conf_main_init(void)
{
   PT("conf main init\n");
   entrance_conf_module_register("Main",
                                 _entrance_conf_main_begin,
                                 _entrance_conf_main_end,
                                 _entrance_conf_main_build,
                                 _entrance_conf_main_check,
                                 _entrance_conf_main_apply);
}

void
entrance_conf_main_shutdown(void)
{
   PT("conf main shutdown\n");
}

