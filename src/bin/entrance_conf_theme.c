#include "entrance_client.h"

typedef struct Entrance_Int_Conf_Theme_
{
   const char *theme;
} Entrance_Int_Conf_Theme;

static void _entrance_conf_theme_begin(void);
static void _entrance_conf_theme_end(void);
static Evas_Object *_entrance_conf_theme_build(Evas_Object *obj);
static Eina_Bool _entrance_conf_theme_check(void);
static void _entrance_conf_theme_apply(void);

static Entrance_Int_Conf_Theme *_entrance_int_conf_theme = NULL;


static void
_entrance_conf_theme_begin(void)
{
   _entrance_int_conf_theme = calloc(1, sizeof(Entrance_Int_Conf_Theme));
   _entrance_int_conf_theme->theme = entrance_gui_theme_name_get();
}

static void
_entrance_conf_theme_end(void)
{
   free(_entrance_int_conf_theme);
}


static Evas_Object *
_entrance_conf_theme_build(Evas_Object *obj)
{
   Evas_Object *o;
   /* Graphical Log */
   o = elm_label_add(obj);
   elm_object_text_set(o, "TODO Implement Theme Selector !");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

  return o;
}

static Eina_Bool
_entrance_conf_theme_check(void)
{
   return !!((_entrance_int_conf_theme->theme != entrance_gui_theme_name_get()));
}

static void
_entrance_conf_theme_apply(void)
{
   //TODO save the correct theme
}


void
entrance_conf_theme_init(void)
{
   PT("conf theme init\n");
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
   PT("conf theme shutdown\n");
}

