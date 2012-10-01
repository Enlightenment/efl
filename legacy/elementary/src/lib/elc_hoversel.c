#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_hoversel.h"

EAPI const char ELM_HOVERSEL_SMART_NAME[] = "elm_hoversel";

static const char SIG_SELECTED[] = "selected";
static const char SIG_DISMISSED[] = "dismissed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_SELECTED, ""},
   {SIG_DISMISSED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_HOVERSEL_SMART_NAME, _elm_hoversel, Elm_Hoversel_Smart_Class,
  Elm_Button_Smart_Class, elm_button_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_hoversel_smart_theme(Evas_Object *obj)
{
   char buf[4096];
   const char *style;

   ELM_HOVERSEL_DATA_GET(obj, sd);

   style = eina_stringshare_add(elm_widget_style_get(obj));

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s", style);
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s", style);

   /* hoversel's style has an extra bit: orientation */
   eina_stringshare_replace(&(ELM_WIDGET_DATA(sd)->style), buf);

   if (!ELM_WIDGET_CLASS(_elm_hoversel_parent_sc)->theme(obj))
     return EINA_FALSE;

   eina_stringshare_replace(&(ELM_WIDGET_DATA(sd)->style), style);

   eina_stringshare_del(style);

   if (sd->hover)
     elm_widget_mirrored_set(sd->hover, elm_widget_mirrored_get(obj));

   elm_hoversel_hover_end(obj);

   return EINA_TRUE;
}

static void
_on_hover_clicked(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   elm_hoversel_hover_end(data);
}

static void
_on_item_clicked(void *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   Elm_Hoversel_Item *item = data;
   Evas_Object *obj2 = WIDGET(item);

   if (item->func) item->func((void *)item->base.data, obj2, item);
   evas_object_smart_callback_call(obj2, SIG_SELECTED, item);
   elm_hoversel_hover_end(obj2);
}

static void
_activate(Evas_Object *obj)
{
   const Elm_Hoversel_Item *item;
   Evas_Object *bt, *bx, *ic;
   const Eina_List *l;
   char buf[4096];

   ELM_HOVERSEL_DATA_GET(obj, sd);

   if (sd->expanded)
     {
        elm_hoversel_hover_end(obj);
        return;
     }
   sd->expanded = EINA_TRUE;

   if (elm_widget_disabled_get(obj)) return;

   sd->hover = elm_hover_add(sd->hover_parent);
   elm_widget_mirrored_automatic_set(sd->hover, EINA_FALSE);

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s",
              elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s",
              elm_widget_style_get(obj));

   elm_object_style_set(sd->hover, buf);

   evas_object_smart_callback_add
     (sd->hover, "clicked", _on_hover_clicked, obj);
   elm_hover_target_set(sd->hover, obj);

   /* hover's content */
   bx = elm_box_add(sd->hover);
   elm_widget_mirrored_automatic_set(bx, EINA_FALSE);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   elm_box_horizontal_set(bx, sd->horizontal);

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal_entry/%s",
              elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical_entry/%s",
              elm_widget_style_get(obj));

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        bt = elm_button_add(bx);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_widget_mirrored_set(bt, elm_widget_mirrored_get(obj));
        elm_object_style_set(bt, buf);
        elm_object_text_set(bt, item->label);

        if (item->icon_file)
          {
             ic = elm_icon_add(obj);
             elm_image_resizable_set(ic, EINA_FALSE, EINA_TRUE);
             if (item->icon_type == ELM_ICON_FILE)
               elm_image_file_set(ic, item->icon_file, item->icon_group);
             else if (item->icon_type == ELM_ICON_STANDARD)
               elm_icon_standard_set(ic, item->icon_file);
             elm_object_part_content_set(bt, "icon", ic);
          }

        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", _on_item_clicked, item);
        evas_object_show(bt);
     }

   if (sd->horizontal)
     elm_object_part_content_set(sd->hover, elm_hover_best_content_location_get
                                   (sd->hover, ELM_HOVER_AXIS_HORIZONTAL), bx);
   else
     elm_object_part_content_set(sd->hover, elm_hover_best_content_location_get
                                   (sd->hover, ELM_HOVER_AXIS_VERTICAL), bx);

   evas_object_show(sd->hover);
}

static void
_on_clicked(void *data,
            Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   _activate(data);
}

static void
_on_parent_del(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   elm_hoversel_hover_parent_set(data, NULL);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;
   return ((Elm_Hoversel_Item *)it)->label;
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Hoversel_Item *item = (Elm_Hoversel_Item *)it;

   ELM_HOVERSEL_DATA_GET_OR_RETURN_VAL(WIDGET(item), sd, EINA_FALSE);

   elm_hoversel_hover_end(WIDGET(item));
   sd->items = eina_list_remove(sd->items, item);
   eina_stringshare_del(item->label);
   eina_stringshare_del(item->icon_file);
   eina_stringshare_del(item->icon_group);

   return EINA_TRUE;
}

static void
_elm_hoversel_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Hoversel_Smart_Data);

   ELM_WIDGET_CLASS(_elm_hoversel_parent_sc)->base.add(obj);

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   priv->expanded = EINA_FALSE;

   evas_object_smart_callback_add(obj, "clicked", _on_clicked, obj);

   _elm_hoversel_smart_theme(obj);
}

static void
_elm_hoversel_smart_del(Evas_Object *obj)
{
   Elm_Hoversel_Item *item;

   ELM_HOVERSEL_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->items, item)
     {
        eina_stringshare_del(item->label);
        eina_stringshare_del(item->icon_file);
        eina_stringshare_del(item->icon_group);
        elm_widget_item_free(item);
     }
   elm_hoversel_hover_parent_set(obj, NULL);

   ELM_WIDGET_CLASS(_elm_hoversel_parent_sc)->base.del(obj);
}

static void
_elm_hoversel_smart_parent_set(Evas_Object *obj,
                               Evas_Object *parent)
{
   elm_hoversel_hover_parent_set(obj, parent);
}

static void
_elm_hoversel_smart_set_user(Elm_Hoversel_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_hoversel_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_hoversel_smart_del;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_hoversel_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->theme = _elm_hoversel_smart_theme;

   ELM_BUTTON_CLASS(sc)->admits_autorepeat = EINA_FALSE;
}

EAPI const Elm_Hoversel_Smart_Class *
elm_hoversel_smart_class_get(void)
{
   static Elm_Hoversel_Smart_Class _sc =
     ELM_HOVERSEL_SMART_CLASS_INIT_NAME_VERSION(ELM_HOVERSEL_SMART_NAME);
   static const Elm_Hoversel_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_hoversel_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_hoversel_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj,
                              Evas_Object *parent)
{
   ELM_HOVERSEL_CHECK(obj);
   ELM_HOVERSEL_DATA_GET(obj, sd);

   if (sd->hover_parent)
     evas_object_event_callback_del_full
       (sd->hover_parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);

   sd->hover_parent = parent;
   if (sd->hover_parent)
     evas_object_event_callback_add
       (sd->hover_parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
}

EAPI Evas_Object *
elm_hoversel_hover_parent_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) NULL;
   ELM_HOVERSEL_DATA_GET(obj, sd);

   return sd->hover_parent;
}

EAPI void
elm_hoversel_horizontal_set(Evas_Object *obj,
                            Eina_Bool horizontal)
{
   ELM_HOVERSEL_CHECK(obj);
   ELM_HOVERSEL_DATA_GET(obj, sd);

   sd->horizontal = !!horizontal;

   _elm_hoversel_smart_theme(obj);
}

EAPI Eina_Bool
elm_hoversel_horizontal_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) EINA_FALSE;
   ELM_HOVERSEL_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI void
elm_hoversel_hover_begin(Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj);
   ELM_HOVERSEL_DATA_GET(obj, sd);

   if (sd->hover) return;

   _activate(obj);
}

EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj);
   ELM_HOVERSEL_DATA_GET(obj, sd);

   if (!sd->hover) return;

   sd->expanded = EINA_FALSE;

   evas_object_del(sd->hover);
   sd->hover = NULL;

   evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
}

EAPI Eina_Bool
elm_hoversel_expanded_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) EINA_FALSE;
   ELM_HOVERSEL_DATA_GET(obj, sd);

   return (sd->hover) ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_hoversel_clear(Evas_Object *obj)
{
   Elm_Object_Item *it;
   Eina_List *l, *ll;

   ELM_HOVERSEL_CHECK(obj);
   ELM_HOVERSEL_DATA_GET(obj, sd);

   EINA_LIST_FOREACH_SAFE(sd->items, l, ll, it)
     {
        elm_widget_item_del(it);
     }
}

EAPI const Eina_List *
elm_hoversel_items_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) NULL;
   ELM_HOVERSEL_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Elm_Object_Item *
elm_hoversel_item_add(Evas_Object *obj,
                      const char *label,
                      const char *icon_file,
                      Elm_Icon_Type icon_type,
                      Evas_Smart_Cb func,
                      const void *data)
{
   ELM_HOVERSEL_CHECK(obj) NULL;
   ELM_HOVERSEL_DATA_GET(obj, sd);

   Elm_Hoversel_Item *item = elm_widget_item_new(obj, Elm_Hoversel_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);

   item->label = eina_stringshare_add(label);
   item->icon_file = eina_stringshare_add(icon_file);
   item->icon_type = icon_type;
   item->func = func;
   item->base.data = data;

   sd->items = eina_list_append(sd->items, item);

   return (Elm_Object_Item *)item;
}

EAPI void
elm_hoversel_item_icon_set(Elm_Object_Item *it,
                           const char *icon_file,
                           const char *icon_group,
                           Elm_Icon_Type icon_type)
{
   ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it);

   Elm_Hoversel_Item *item = (Elm_Hoversel_Item *)it;

   eina_stringshare_replace(&item->icon_file, icon_file);
   eina_stringshare_replace(&item->icon_group, icon_group);

   item->icon_type = icon_type;
}

EAPI void
elm_hoversel_item_icon_get(const Elm_Object_Item *it,
                           const char **icon_file,
                           const char **icon_group,
                           Elm_Icon_Type *icon_type)
{
   ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it);

   Elm_Hoversel_Item *item = (Elm_Hoversel_Item *)it;

   if (icon_file) *icon_file = item->icon_file;
   if (icon_group) *icon_group = item->icon_group;
   if (icon_type) *icon_type = item->icon_type;
}
