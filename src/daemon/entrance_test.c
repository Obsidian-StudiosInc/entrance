#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

int elm_main (int argc, char **argv);

static Evas_Object *
_theme_get(Evas_Object *win, const char *group)
{
   char buffer[PATH_MAX];
   Evas_Object *edje = NULL;

   edje = elm_layout_add(win);
   snprintf(buffer, sizeof(buffer), "%s/themes/default.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(edje, buffer, group);
   return edje;
}

static void
_signal(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *sig, const char *src)
{
   printf("Event: %s - %s \n", sig, src);
}


static void
_shutdown(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("Quit\n");
   elm_exit();
}


int
elm_main (int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *o, *win;
   win = elm_win_add(NULL, "theme_test", ELM_WIN_BASIC);
   elm_win_title_set(win, PACKAGE);
   evas_object_smart_callback_add(win, "delete,request",
                                  _shutdown, NULL);
   o = _theme_get(win, "entrance");
   evas_object_size_hint_weight_set(o,
                                    EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   edje_object_signal_callback_add(elm_layout_edje_get(o),
                                   "*", "*",
                                   _signal, NULL);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);
   evas_object_resize(win, 640, 480);
   evas_object_show(win);
   elm_run();
   elm_shutdown();
   return 0;
}

ELM_MAIN()
