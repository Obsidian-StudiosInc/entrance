#include "entrance_client.h"

struct Entrance_Fill_
{
   const char *item_style; //maybee need to be provided by theme ?
   struct
     {
        Entrance_Fill_Text_Get_Func text_get;
        Entrance_Fill_Content_Get_Func content_get;
        Entrance_Fill_State_Get_Func state_get;
        Entrance_Fill_Del_Func del;
        Evas_Smart_Cb sel;
        void *data;
     } func;
   Elm_Genlist_Item_Class *glc;
   Elm_Gengrid_Item_Class *ggc;
};

///////////////// LIST ///////////////////////////////
static void
_entrance_fill_list(Evas_Object *obj, Entrance_Fill *ef,const  Eina_List *contents, Entrance_Fill_Cb_Func fill_cb, Evas_Smart_Cb func, void *data)
{
   const Eina_List *l;
   void *content;

   EINA_LIST_FOREACH(contents, l, content)
     {
        Elm_Object_Item *it = NULL;
        if (ef->func.text_get)
          it = elm_list_item_append(obj, ef->func.text_get(content, NULL, NULL), NULL,
                                    NULL, func, data);
        if (it)
          elm_object_item_data_set(it, content);
        if (fill_cb)
          fill_cb(content, it);
     }
   elm_list_go(obj);
}

///////////////// GENLIST /////////////////////////////
static void
_entrance_fill_genlist(Evas_Object *obj, Entrance_Fill *ef, const Eina_List *contents, Entrance_Fill_Cb_Func fill_cb, Evas_Smart_Cb func, void *data)
{
   const Eina_List *l;
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
     {
        Elm_Object_Item *it;
        it = elm_genlist_item_append(obj, glc,
                                     content, NULL, ELM_GENLIST_ITEM_NONE,
                                     func, data);
        elm_object_item_data_set(it, content);
        if (fill_cb)
          fill_cb(content, it);
     }
}

///////////////// GENGRID /////////////////////////////
static void
_entrance_fill_gengrid(Evas_Object *obj, Entrance_Fill *ef, const Eina_List *contents, Entrance_Fill_Cb_Func fill_cb, Evas_Smart_Cb func, void *data)
{
   const Eina_List *l;
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
     {
        Elm_Object_Item *it;
        it = elm_gengrid_item_append(obj, ggc,
                                     content, func, data);
        elm_object_item_data_set(it, content);
        if (fill_cb)
          fill_cb(content, it);
     }
}

///////////////// HOVERSEL /////////////////////////////
static void
_entrance_fill_hoversell_func_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Entrance_Fill *ef;

   ef = evas_object_data_get(obj, "fill_data");
   if (!ef) return;
   if (ef->func.sel)
     ef->func.sel(ef->func.data, obj, event_info);
}

static void
_entrance_fill_hoversell(Evas_Object *obj, Entrance_Fill *ef, const Eina_List *contents, Entrance_Fill_Cb_Func fill_cb, Evas_Smart_Cb func, void *data)
{
   const Eina_List *l;
   void *content;
   char *str = NULL;
   char *ic = NULL;

   if (!ef->func.text_get) return;
   ef->func.sel = func;
   ef->func.data = data;
   EINA_LIST_FOREACH(contents, l, content)
     {
        Elm_Object_Item *it;
        str = ef->func.text_get(content, obj, NULL);
        ic = ef->func.text_get(content, obj, "icon");
        it = elm_hoversel_item_add(obj, str, ic,
                                   ELM_ICON_FILE,
                                   _entrance_fill_hoversell_func_cb, NULL);
        elm_object_item_data_set(it, content);
        evas_object_data_set(elm_object_item_widget_get(it), "fill_data", ef);
        if (fill_cb)
          fill_cb(content, it);
        if (ic)
          {
             free(ic);
             ic = NULL;
          }
        if (str)
          {
             free(str);
             str = NULL;
          }
     }
}

///////////////// MAIN /////////////////////////////
Entrance_Fill *
entrance_fill_new(const char *item_style, Entrance_Fill_Text_Get_Func text_get, Entrance_Fill_Content_Get_Func content_get, Entrance_Fill_State_Get_Func state_get, Entrance_Fill_Del_Func del_func)
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
entrance_fill(Evas_Object *obj, Entrance_Fill *ef, const Eina_List *contents, Entrance_Fill_Cb_Func fill_cb, Evas_Smart_Cb func, void *data)
{
   const char *type;
   if (!obj) return;
   if ((type = elm_object_widget_type_get(obj)))
     {
        if (!strcasecmp(type, "elm_list"))
          _entrance_fill_list(obj, ef, contents, fill_cb, func, data);
        else if (!strcasecmp(type, "elm_genlist"))
          _entrance_fill_genlist(obj, ef, contents, fill_cb, func, data);
        else if (!strcasecmp(type, "elm_gengrid"))
          _entrance_fill_gengrid(obj, ef, contents, fill_cb, func, data);
        else if (!strcasecmp(type, "elm_hoversel"))
          _entrance_fill_hoversell(obj, ef, contents, fill_cb, func, data);
        else
          {
             PT("Unknow object type to fill ");
             fprintf(stderr, "%s\n", type);
          }
     }
}


