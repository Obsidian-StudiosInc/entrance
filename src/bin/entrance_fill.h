#ifndef ENTRANCE_FILL_H
#define ENTRANCE_FILL_H

typedef struct Entrance_Fill_ Entrance_Fill;

typedef char *(*EntranceFillTextGetFunc) (void *data, Evas_Object *obj, const char *part);
typedef Evas_Object *(*EntranceFillContentGetFunc) (void *data, Evas_Object *obj, const char *part);
typedef Eina_Bool (*EntranceFillStateGetFunc) (void *data, Evas_Object *obj, const char *part);
typedef void (*EntranceFillDelFunc) (void *data, Evas_Object *obj);

Entrance_Fill *entrance_fill_new(const char *item_style, EntranceFillTextGetFunc label_get, EntranceFillContentGetFunc content_get, EntranceFillStateGetFunc state_get, EntranceFillDelFunc del_func);
void entrance_fill(Evas_Object *obj, Entrance_Fill *egf, Eina_List *contents, Evas_Smart_Cb func, void *data);
void entrance_fill_del(Entrance_Fill *ef);


#endif /* ENTRANCE_FILL_H */
