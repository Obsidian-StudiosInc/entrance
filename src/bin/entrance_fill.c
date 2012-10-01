#include "entrance_client.h"

struct Entrance_Fill_
{
   const char *item_style; //maybee need to be provided by theme ?
   struct
     {
        EntranceFillTextGetFunc   text_get;
        EntranceFillContentGetFunc content_get;
        EntranceFillStateGetFunc   state_get;
        EntranceFillDelFunc        del;
     } func;
   Elm_Genlist_Item_Class *glc;
   Elm_Gengrid_Item_Class *ggc;
};

///////////////// LIST ///////////////////////////////
static void
_entrance_fill_list(Evas_Object *obj, Entrance_Fill *ef, Eina_List *contents, Evas_Smart_Cb func)
{
   Eina_List *l;
   void *content;

   EINA_LIST_FOREACH(contents, l, content)
     {
        if (ef->func.text_get)
          elm_list_item_append(obj, ef->func.text_get(content, NULL, NULL), NULL,
                               NULL, func, content);
     }
   elm_list_go(obj);
}

///////////////// GENLIST /////////////////////////////
static void
_entrance_fill_genlist(Evas_Object *obj, Entrance_Fill *ef, Eina_List *contents, Evas_Smart_Cb func)
{
   Eina_List *l;
   Elm_Genlist_Item_Class *glc;
   void *content;

   if (!ef->glc)
     {
        glc = elm_genlist_item_class_new();
        ef->glc = glc;
     }
   else
     glc = ef->glc;
   glc->item_style = ef->item_style;
   glc->func.text_get = ef->func.text_get;
   glc->func.content_get = ef->func.content_get;
   glc->func.state_get = ef->func.state_get;
   glc->func.del = ef->func.del;


   EINA_LIST_FOREACH(contents, l, content)
      elm_genlist_item_append(obj, glc,
                              content, NULL, ELM_GENLIST_ITEM_NONE,
                              func, content);
}

///////////////// GENGRID /////////////////////////////
static void
_entrance_fill_gengrid(Evas_Object *obj, Entrance_Fill *ef, Eina_List *contents, Evas_Smart_Cb func)
{
   Eina_List *l;
   Elm_Gengrid_Item_Class *ggc;
   void *content;

   if (!ef->ggc)
     {
        ggc = elm_gengrid_item_class_new();
        ef->ggc = ggc;
     }
   else
     ggc = ef->ggc;
   ggc->item_style = ef->item_style;
   ggc->func.text_get = ef->func.text_get;
   ggc->func.content_get = ef->func.content_get;
   ggc->func.state_get = ef->func.state_get;
   ggc->func.del = ef->func.del;

   EINA_LIST_FOREACH(contents, l, content)
      elm_gengrid_item_append(obj, ggc,
                              content, func, content);
}

///////////////// HOVERSEL /////////////////////////////
static void
_entrance_fill_hoversell(Evas_Object *obj, Entrance_Fill *ef, Eina_List *contents, Evas_Smart_Cb func)
{
   Eina_List *l;
   char *str = NULL;
   void *content;

   EINA_LIST_FOREACH(contents, l, content)
     {
        if (ef->func.text_get)
          str = ef->func.text_get(content, obj, NULL);
        elm_hoversel_item_add(obj, str, NULL,
                              ELM_ICON_FILE, func, content);
        free(str);
     }
}

Entrance_Fill *
entrance_fill_new(const char *item_style, EntranceFillTextGetFunc text_get, EntranceFillContentGetFunc content_get, EntranceFillStateGetFunc state_get, EntranceFillDelFunc del_func)
{
   Entrance_Fill *ef;
   ef = calloc(1, sizeof(Entrance_Fill));
   ef->item_style = eina_stringshare_add(item_style);
   ef->func.text_get = text_get;
   ef->func.content_get = content_get;
   ef->func.state_get = state_get;
   ef->func.del = del_func;
   return ef;
}

void
entrance_fill_del(Entrance_Fill *ef)
{
   eina_stringshare_del(ef->item_style);
   free(ef);
}

void
entrance_fill(Evas_Object *obj, Entrance_Fill *ef, Eina_List *contents, Evas_Smart_Cb func)
{
   const char *type;
   if (!obj) return;
   if ((type = elm_object_widget_type_get(obj)))
     {
        if (!strcmp(type, "elm_list"))
          _entrance_fill_list(obj, ef, contents, func);
        else if (!strcmp(type, "elm_genlist"))
          _entrance_fill_genlist(obj, ef, contents, func);
        else if (!strcmp(type, "elm_gengrid"))
          _entrance_fill_gengrid(obj, ef, contents, func);
        else if (!strcmp(type, "elm_hoversel"))
          _entrance_fill_hoversell(obj, ef, contents, func);
     }
}


