#include "entrance_client.h"

static void _entrance_conf_log_begin(void);
static void _entrance_conf_log_end(void);
static Evas_Object *_entrance_conf_log_build(Evas_Object *obj);
static Eina_Bool _entrance_conf_log_check(void);
static void _entrance_conf_log_apply(void);

static void
_entrance_conf_log_begin(void)
{

}

static void
_entrance_conf_log_end(void)
{

}

static Evas_Object *
_entrance_conf_log_build(Evas_Object *obj)
{
   Evas_Object *o;
   /* Graphical Log */
   o = elm_label_add(obj);
   elm_object_text_set(o, "TODO Implement graphical Log !");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   return o;
}

static Eina_Bool
_entrance_conf_log_check(void)
{
   return 0;
}

static void
_entrance_conf_log_apply(void)
{
}


void
entrance_conf_log_init(void)
{
   PT("conf grapical log init\n");
   entrance_conf_module_register("Log",
                                 _entrance_conf_log_begin,
                                 _entrance_conf_log_end,
                                 _entrance_conf_log_build,
                                 _entrance_conf_log_check,
                                 _entrance_conf_log_apply);
}

void
entrance_conf_log_shutdown(void)
{
   PT("conf log shutdown\n");
}

