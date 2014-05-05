#ifndef ENTRANCE_CONFIG_H
#define ENTRANCE_CONFIG_H

typedef struct Entrance_Conf_Background_
{
   const char *path;
   const char *group;
   const char *name;
} Entrance_Conf_Background;

#define IMG_LIST_FORK(l_src, l_dest) \
  do { \
  Entrance_Image *ptr; \
  Entrance_Conf_Background *tmp_ptr; \
  Eina_List *img_list; \
  EINA_LIST_FOREACH(l_src, img_list, ptr) \
   { \
     tmp_ptr = malloc(sizeof(Entrance_Conf_Background)); \
     tmp_ptr->path = eina_stringshare_add(ptr->path); \
     tmp_ptr->group = eina_stringshare_add(ptr->group); \
     entrance_conf_background_title_gen(tmp_ptr);\
     l_dest = eina_list_append(l_dest, tmp_ptr); \
   } \
  }while(0);

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
void entrance_conf_background_title_gen(Entrance_Conf_Background *ptr);
Entrance_Fill *entrance_conf_background_fill_get(void);



#endif /* ENTRANCE_CONFIG_H */
