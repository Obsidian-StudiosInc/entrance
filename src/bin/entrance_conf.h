#ifndef ENTRANCE_CONFIG_H
#define ENTRANCE_CONFIG_H

typedef struct Entrance_Conf_Background_
{
   const char *path;
   const char *group;
   const char *name;
} Entrance_Conf_Background;

typedef void (*Entrance_Conf_Begin) (void);
typedef void (*Entrance_Conf_End) (void);
typedef Evas_Object *(*Entrance_Conf_Build) (Evas_Object *obj);
typedef Eina_Bool (*Entrance_Conf_Check) (void);
typedef void (*Entrance_Conf_Apply) (void);

void entrance_conf_init(void);
void entrance_conf_shutdown(void);
void entrance_conf_module_register(const char *label, Entrance_Conf_Begin begin, Entrance_Conf_End end, Entrance_Conf_Build build, Entrance_Conf_Check check, Entrance_Conf_Apply apply);
void entrance_conf_begin(Evas_Object *obj, Evas_Object *parent);
void entrance_conf_changed(void);
Eina_List *entrance_conf_backgrounds_get(Evas_Object *obj, const char *user);
Entrance_Fill *entrance_conf_background_fill_get(void);



#endif /* ENTRANCE_CONFIG_H */
