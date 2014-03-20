#include "entrance_client.h"


typedef struct Entrance_Conf_Module_
{
   const char *label;
   Entrance_Conf_Begin begin;
   Entrance_Conf_End end;
   Entrance_Conf_Build build;
   Entrance_Conf_Check check;
   Entrance_Conf_Apply apply;
   Elm_Object_Item *item;
} Entrance_Conf_Module;

typedef struct Entrance_Int_Conf_
{
   Evas_Object *btn_ok;
   Evas_Object *btn_apply;
   Eina_List *modules;
   Entrance_Conf_Module *current;
   Entrance_Fill *background_fill;
} Entrance_Int_Conf;

static void _entrance_conf_build(Evas_Object *naviframe, Evas_Object *segment);
static void _entrance_conf_apply(void);
static void _entrance_conf_end(Evas_Object *win);
static void _entrance_conf_promote(Entrance_Conf_Module *conf);
static void _entrance_conf_control_changed(void *data, Evas_Object *obj, void *event);
static void _entrance_conf_ok_clicked(void *data, Evas_Object *obj, void *event);
static void _entrance_conf_apply_clicked(void *data, Evas_Object *obj, void *event);
static void _entrance_conf_close_clicked(void *data, Evas_Object *obj, void *event);
static char *_entrance_conf_bg_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_conf_bg_content_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_conf_bg_del(void *data, Evas_Object *obj);



static Entrance_Int_Conf *_entrance_conf = NULL;

static void
_entrance_conf_build(Evas_Object *naviframe, Evas_Object *segment)
{
   Entrance_Conf_Module *conf;
   Eina_List *l;
   Evas_Object *o;
   Elm_Object_Item *it, *itc;

   EINA_LIST_FOREACH(_entrance_conf->modules, l, conf)
     {
        o = conf->build(naviframe);
        it = elm_naviframe_item_simple_push(naviframe, o);
        itc = elm_segment_control_item_add(segment, NULL, conf->label);
        conf->item = it;
        elm_object_item_data_set(itc, conf);
        if (!_entrance_conf->current)
          _entrance_conf_promote(conf);
     }
}

static void
_entrance_conf_apply(void)
{
   _entrance_conf->current->apply();
}

static void
_entrance_conf_end(Evas_Object *win)
{
   Entrance_Conf_Module *conf;
   Eina_List *l;

   PT("Delete config panel\n");
   evas_object_del(win);
   EINA_LIST_FOREACH(_entrance_conf->modules, l, conf)
      conf->end();
   _entrance_conf->current = NULL;
}

static void
_entrance_conf_promote(Entrance_Conf_Module *conf)
{
   elm_naviframe_item_promote(conf->item);
   _entrance_conf->current = conf;
}
static void
_entrance_conf_control_changed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Elm_Object_Item *it;
   it = event;
   _entrance_conf_promote(elm_object_item_data_get(it));
}

static void
_entrance_conf_ok_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _entrance_conf_apply();
   _entrance_conf_end(data);
}

static void
_entrance_conf_apply_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _entrance_conf_apply();
   entrance_conf_changed();
}

static void
_entrance_conf_close_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _entrance_conf_end(data);
}

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


void
entrance_conf_init(void)
{
   PT("conf init\n");
   _entrance_conf = calloc(1, sizeof(Entrance_Int_Conf));
   _entrance_conf->background_fill =
      entrance_fill_new("default",
                        _entrance_conf_bg_text_get,
                        _entrance_conf_bg_content_get,
                        _entrance_conf_bg_state_get,
                        _entrance_conf_bg_del);

   entrance_conf_main_init();
   entrance_conf_user_init();
}

void
entrance_conf_shutdown(void)
{
   Entrance_Conf_Module *conf;

   PT("conf shutdown\n");
   entrance_conf_user_shutdown();
   entrance_conf_main_shutdown();
   EINA_LIST_FREE(_entrance_conf->modules, conf)
     {
        eina_stringshare_del(conf->label);
        free(conf);
     }
   free(_entrance_conf);
}

void
entrance_conf_module_register(const char *label, Entrance_Conf_Begin begin, Entrance_Conf_End end, Entrance_Conf_Build build, Entrance_Conf_Check check, Entrance_Conf_Apply apply)
{
   Entrance_Conf_Module *conf;
   conf = calloc(1, sizeof(Entrance_Conf_Module));
   conf->label = eina_stringshare_add(label);
   conf->begin = begin;
   conf->end = end;
   conf->build = build;
   conf->check = check;
   conf->apply = apply;
   _entrance_conf->modules = eina_list_append(_entrance_conf->modules, conf);
}

void
entrance_conf_begin(Evas_Object *obj, Evas_Object *parent)
{
   PT("Build config panel\n");
   Evas_Object *win, *bg, *bx, *hbx, *sc, *nf, *o;
   Entrance_Conf_Module *conf;
   Eina_List *l;

   EINA_LIST_FOREACH(_entrance_conf->modules, l, conf)
      conf->begin();
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
   _entrance_conf->btn_ok = o;
   evas_object_show(o);

   o = elm_button_add(hbx);
   elm_object_text_set(o, "Apply");
   elm_object_disabled_set(o, EINA_TRUE);
   evas_object_smart_callback_add(o, "clicked",
                                  _entrance_conf_apply_clicked, NULL);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, o);
   _entrance_conf->btn_apply = o;
   evas_object_show(o);

   o = elm_button_add(hbx);
   elm_object_text_set(o, "Close");
   evas_object_smart_callback_add(o, "clicked",
                                  _entrance_conf_close_clicked, win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(hbx, o);
   evas_object_show(o);

   /* Build configuration panel */
   _entrance_conf_build(nf, sc);

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

void
entrance_conf_changed(void)
{
   if (_entrance_conf->current && _entrance_conf->current->check())
     {
        elm_object_disabled_set(_entrance_conf->btn_ok, EINA_FALSE);
        elm_object_disabled_set(_entrance_conf->btn_apply, EINA_FALSE);
     }
   else
     {
        elm_object_disabled_set(_entrance_conf->btn_ok, EINA_TRUE);
        elm_object_disabled_set(_entrance_conf->btn_apply, EINA_TRUE);
     }
}

Eina_List *
entrance_conf_backgrounds_get(Evas_Object *obj, const char *user)
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

Entrance_Fill *
entrance_conf_background_fill_get(void)
{
   return _entrance_conf->background_fill;
}

