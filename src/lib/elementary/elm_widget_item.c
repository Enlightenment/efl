#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#define ELM_WIDGET_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define EFL_CANVAS_OBJECT_BETA
#define EFL_INPUT_EVENT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_container.h"

/* FIXME: remove this when we don't rely on evas event structs anymore */
#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define ERR_NOT_SUPPORTED(item, method)  ERR("%s does not support %s API.", elm_widget_type_get(item->widget), method);

typedef struct _Elm_Label_Data            Elm_Label_Data;
typedef struct _Elm_Translate_String_Data Elm_Translate_String_Data;

struct _Elm_Label_Data
{
   const char *part;
   const char *text;
};

struct _Elm_Translate_String_Data
{
   EINA_INLIST;
   Eina_Stringshare *id;
   Eina_Stringshare *domain;
   Eina_Stringshare *string;
   Eina_Bool   preset : 1;
};

// methods

/**
 * @internal
 *
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_widget_item_tooltip_content_cb_set() or
 *       elm_widget_item_tooltip_text_set()
 *
 * @param item widget item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_tooltip_style_set(Eo *eo_item EINA_UNUSED,
                                   Elm_Widget_Item_Data *item,
                                   const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_tooltip_style_set(item->view, style);
}

EOLIAN static Eina_Bool
_elm_widget_item_tooltip_window_mode_set(Eo *eo_item EINA_UNUSED,
                                         Elm_Widget_Item_Data *item,
                                         Eina_Bool disable)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, EINA_FALSE);

   return elm_object_tooltip_window_mode_set(item->view, disable);
}

EOLIAN static Eina_Bool
_elm_widget_item_tooltip_window_mode_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, EINA_FALSE);

   return elm_object_tooltip_window_mode_get(item->view);
}

/**
 * @internal
 *
 * Get the style for this item tooltip.
 *
 * @param item widget item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @ingroup Widget
 */
EOLIAN static const char *
_elm_widget_item_tooltip_style_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);

   return elm_object_tooltip_style_get(item->view);
}

EOLIAN static void
_elm_widget_item_cursor_set(Eo *eo_item EINA_UNUSED,
                            Elm_Widget_Item_Data *item,
                            const char *cursor)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_sub_cursor_set(item->view, item->widget, cursor);
}

EOLIAN static const char *
_elm_widget_item_cursor_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return elm_object_cursor_get(item->view);
}

/**
 * @internal
 *
 * Sets a different style for this item cursor.
 *
 * @note before you set a style you should define a cursor with
 *       elm_widget_item_cursor_set()
 *
 * @param item widget item with cursor already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_cursor_style_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_cursor_style_set(item->view, style);
}

/**
 * @internal
 *
 * Get the style for this item cursor.
 *
 * @param item widget item with cursor already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then NULL is returned.
 *
 * @ingroup Widget
 */
EOLIAN static const char *
_elm_widget_item_cursor_style_get(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return elm_object_cursor_style_get(item->view);
}

/**
 * @internal
 *
 * Set if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a cursor
 * with elm_object_cursor_set(). By default it will only look for cursors
 * provided by the engine.
 *
 * @param item widget item with cursor already set.
 * @param engine_only boolean to define it cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well.
 *
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_cursor_engine_only_set(Eo *eo_item EINA_UNUSED,
                                        Elm_Widget_Item_Data *item,
                                        Eina_Bool engine_only)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_cursor_theme_search_enabled_set(item->view, !engine_only);
}/**
 * @internal
 *
 * Get the cursor engine only usage for this item cursor.
 *
 * @param item widget item with cursor already set.
 * @return engine_only boolean to define it cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well. If
 *         the object does not have a cursor set, then EINA_FALSE is returned.
 *
 * @ingroup Widget
 */
EOLIAN static Eina_Bool
_elm_widget_item_cursor_engine_only_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return !elm_object_cursor_theme_search_enabled_get(item->view);
}


EOLIAN static void
_elm_widget_item_part_content_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  const char *part EINA_UNUSED,
                                  Evas_Object *content EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_content_set()");
}

EOLIAN static Evas_Object *
_elm_widget_item_part_content_get(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  const char *part EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_content_get()");
   return NULL;
}

EOLIAN static void
_elm_widget_item_part_text_set(Eo *eo_item EINA_UNUSED,
                               Elm_Widget_Item_Data *item,
                               const char *part EINA_UNUSED,
                               const char *label EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_text_set()");
}

EOLIAN static const char *
_elm_widget_item_part_text_get(Eo *eo_item EINA_UNUSED,
                               Elm_Widget_Item_Data *item,
                               const char *part EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_text_get()");
   return NULL;
}

static void
_elm_widget_item_part_text_custom_free(void *data)
{
   Elm_Label_Data *label;
   label = data;
   eina_stringshare_del(label->part);
   eina_stringshare_del(label->text);
   free(label);
}

EOLIAN static void
_elm_widget_item_part_text_custom_set(Eo *eo_item EINA_UNUSED,
                                      Elm_Widget_Item_Data *item,
                                      const char *part,
                                      const char *text)
{
   Elm_Label_Data *label;
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->labels)
     item->labels =
        eina_hash_stringshared_new(_elm_widget_item_part_text_custom_free);
   label = eina_hash_find(item->labels, part);
   if (!label)
     {
        label = malloc(sizeof(Elm_Label_Data));
        if (!label)
          {
             ERR("Failed to allocate memory");
             return;
          }
        label->part = eina_stringshare_add(part);
        label->text = eina_stringshare_add(text);
        eina_hash_add(item->labels, part, label);
     }
   else
     eina_stringshare_replace(&label->text, text);
}

EOLIAN static const char *
_elm_widget_item_part_text_custom_get(Eo *eo_item EINA_UNUSED,
                                      Elm_Widget_Item_Data *item,
                                      const char *part)
{
   Elm_Label_Data *label;
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   label = eina_hash_find(item->labels, part);
   return label ? label->text : NULL;
}

EOLIAN static void
_elm_widget_item_focus_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Eina_Bool focused EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_item_focus_set");
}

EOLIAN static Eina_Bool
_elm_widget_item_focus_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ERR_NOT_SUPPORTED(item, "elm_object_item_focus_get");
   return EINA_FALSE;
}

EOLIAN static void
_elm_widget_item_style_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, const char *style EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_style_set()");
}

EOLIAN static const char *
_elm_widget_item_style_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ERR_NOT_SUPPORTED(item, "elm_object_style_get()");
   return NULL;
}

EOLIAN static void
_elm_widget_item_disable(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item EINA_UNUSED)
{
}

EAPI void
elm_object_item_data_set(Elm_Object_Item *it, void *data)
{
   WIDGET_ITEM_DATA_SET(it, data);
}

EAPI void *
elm_object_item_data_get(const Elm_Object_Item *it)
{
   return (void *) WIDGET_ITEM_DATA_GET(it);
}

EOLIAN static void
_elm_widget_item_disabled_set(Eo *eo_item EINA_UNUSED,
                              Elm_Widget_Item_Data *item,
                              Eina_Bool disabled)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (item->disabled == disabled) return;
   item->disabled = !!disabled;
   elm_wdg_item_disable(item->eo_obj);
}

EOLIAN static Eina_Bool
_elm_widget_item_disabled_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return item->disabled;
}

EOLIAN static void
_elm_widget_item_access_order_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  Eina_List *objs)
{
   _elm_access_widget_item_access_order_set(item, objs);
}

EOLIAN static const Eina_List *
_elm_widget_item_access_order_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   return _elm_access_widget_item_access_order_get(item);
}

/**
 * @internal
 *
 * Get owner widget of this item.
 *
 * @param item a valid #Elm_Widget_Item to get data from.
 * @return owner widget of this item.
 * @ingroup Widget
 */
EOLIAN static Evas_Object *
_elm_widget_item_widget_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   return item->widget;
}

/**
 * @internal
 *
 * Releases widget item memory, calling back item_del_pre_hook() and
 * item_del_cb() if they exist.
 *
 * @param item a valid #Elm_Widget_Item to be deleted.
 *
 * If there is an Elm_Widget_Item::del_cb, then it will be called prior
 * to memory release. Note that elm_widget_item_pre_notify_del() calls
 * this function and then unset it, thus being useful for 2 step
 * cleanup whenever the del_cb may use any of the data that must be
 * deleted from item.
 *
 * The Elm_Widget_Item::view will be deleted (evas_object_del()) if it
 * is presented!
 *
 * Note that if item_del_pre_hook() returns @c EINA_TRUE, item free will be
 * deferred, or item will be freed here if it returns @c EINA_FALSE.
 *
 * @see elm_widget_item_del() convenience macro.
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_del(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->on_deletion = EINA_TRUE;

   //Widget item delete callback
   Eina_Bool del_ok;
   del_ok = elm_wdg_item_del_pre(item->eo_obj);
   if (del_ok)
      efl_del(item->eo_obj);
   return;
}

/**
 * @internal
 *
 * Notify object will be deleted without actually deleting it.
 *
 * This function will callback Elm_Widget_Item::del_cb if it is set
 * and then unset it so it is not called twice (ie: from
 * elm_widget_item_del()).
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_pre_notify_del() convenience macro.
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_pre_notify_del(Eo *eo_item, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if (!item->del_func) return;
   item->del_func((void *)WIDGET_ITEM_DATA_GET(eo_item), item->widget, item->eo_obj);
   item->del_func = NULL;
}

static Evas_Object *
_elm_widget_item_tooltip_label_create(void *data,
                                      Evas_Object *obj EINA_UNUSED,
                                      Evas_Object *tooltip,
                                      void *item EINA_UNUSED)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static void
_elm_widget_item_tooltip_label_del_cb(void *data,
                                      Evas_Object *obj EINA_UNUSED,
                                      void *event_info EINA_UNUSED)
{
   eina_stringshare_del(data);
}

/**
 * @internal
 *
 * Set the text to be shown in the widget item.
 *
 * @param item Target item
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data is removed.
 *
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_tooltip_text_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item EINA_UNUSED,
                                  const char *text)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   elm_wdg_item_tooltip_content_cb_set(item->eo_obj, _elm_widget_item_tooltip_label_create, text, _elm_widget_item_tooltip_label_del_cb);
}

typedef struct _Elm_Widget_Item_Tooltip Elm_Widget_Item_Tooltip;

struct _Elm_Widget_Item_Tooltip
{
   Elm_Widget_Item_Data       *item;
   Elm_Tooltip_Item_Content_Cb func;
   Evas_Smart_Cb               del_cb;
   const void                 *data;
};

static Evas_Object *
_elm_widget_item_tooltip_trans_label_create(void *data,
                                            Evas_Object *obj EINA_UNUSED,
                                            Evas_Object *tooltip,
                                            void *item EINA_UNUSED)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_translatable_text_set(label, data);
   return label;
}

static Evas_Object *
_elm_widget_item_tooltip_create(void *data,
                                Evas_Object *obj,
                                Evas_Object *tooltip)
{
   Elm_Widget_Item_Tooltip *wit = data;
   return wit->func((void *)wit->data, obj, tooltip, wit->item->eo_obj);
}

static void
_elm_widget_item_tooltip_del_cb(void *data,
                                Evas_Object *obj,
                                void *event_info EINA_UNUSED)
{
   Elm_Widget_Item_Tooltip *wit = data;
   if (wit->del_cb) wit->del_cb((void *)wit->data, obj, wit->item->eo_obj);
   free(wit);
}

EOLIAN static void
_elm_widget_item_tooltip_translatable_text_set(Eo *eo_item EINA_UNUSED,
                                               Elm_Widget_Item_Data *item EINA_UNUSED,
                                               const char *text)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   elm_wdg_item_tooltip_content_cb_set(item->eo_obj, _elm_widget_item_tooltip_trans_label_create, text, _elm_widget_item_tooltip_label_del_cb);
}

/**
 * @internal
 *
 * Unset tooltip from item
 *
 * @param item widget item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_widget_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @see elm_widget_item_tooltip_content_cb_set()
 *
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_tooltip_unset(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_tooltip_unset(item->view);
}

EOLIAN static void
_elm_widget_item_cursor_unset(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_cursor_unset(item->view);
}

EOLIAN static Evas_Object *
_elm_widget_item_part_content_unset(Eo *eo_item EINA_UNUSED,
                                    Elm_Widget_Item_Data *item,
                                    const char *part EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_content_unset()");
   return NULL;
}

static Eina_Bool
_elm_widget_item_part_text_custom_foreach(const Eina_Hash *labels EINA_UNUSED,
                                          const void *key EINA_UNUSED,
                                          void *data,
                                          void *func_data)
{
   Elm_Label_Data *label;
   Elm_Widget_Item_Data *item;
   label = data;
   item = func_data;

   elm_wdg_item_part_text_set(item->eo_obj, label->part, label->text);

   return EINA_TRUE;
}

EOLIAN static void
_elm_widget_item_part_text_custom_update(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   if (item->labels)
     eina_hash_foreach(item->labels,
                       _elm_widget_item_part_text_custom_foreach, item);
}

static void
_elm_widget_item_signal_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *emission,
                           const char *source)
{
   Elm_Widget_Item_Signal_Data *wisd = data;
   wisd->func(wisd->data, wisd->item, emission, source);
}

EOLIAN static void
_elm_widget_item_signal_callback_add(Eo *eo_item,
                                     Elm_Widget_Item_Data *item,
                                     const char *emission,
                                     const char *source,
                                     Elm_Object_Item_Signal_Cb func,
                                     void *data)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(func);

   Elm_Widget_Item_Signal_Data *wisd;

   wisd = malloc(sizeof(Elm_Widget_Item_Signal_Data));
   if (!wisd) return;

   wisd->item = eo_item;
   wisd->func = (Elm_Widget_Item_Signal_Cb)func;
   wisd->data = data;
   wisd->emission = eina_stringshare_add(emission);
   wisd->source = eina_stringshare_add(source);

   if (elm_widget_is(item->view))
     elm_object_signal_callback_add(item->view, emission, source, _elm_widget_item_signal_cb, wisd);
   else if (efl_isa(item->view, EDJE_OBJECT_CLASS))
     edje_object_signal_callback_add(item->view, emission, source, _elm_widget_item_signal_cb, wisd);
   else
     {
        WRN("The %s widget item doesn't support signal callback add!",
            efl_class_name_get(efl_class_get(item->widget)));
        free(wisd);
        return;
     }

   item->signals = eina_list_append(item->signals, wisd);
}

static void *
_elm_widget_item_signal_callback_list_get(Elm_Widget_Item_Data *item, Eina_List *position)
{
   Elm_Widget_Item_Signal_Data *wisd = eina_list_data_get(position);
   void *data;

   item->signals = eina_list_remove_list(item->signals, position);
   data = wisd->data;

   if (elm_widget_is(item->view))
     elm_object_signal_callback_del(item->view,
                                    wisd->emission, wisd->source,
                                    _elm_widget_item_signal_cb);
   else if (efl_isa(item->view, EDJE_OBJECT_CLASS))
     edje_object_signal_callback_del_full(item->view,
                                          wisd->emission, wisd->source,
                                          _elm_widget_item_signal_cb, wisd);

   eina_stringshare_del(wisd->emission);
   eina_stringshare_del(wisd->source);
   free(wisd);

   return data;
}


EOLIAN static void *
_elm_widget_item_signal_callback_del(Eo *eo_item EINA_UNUSED,
                                     Elm_Widget_Item_Data *item,
                                     const char *emission,
                                     const char *source,
                                     Elm_Object_Item_Signal_Cb func)
{
   Elm_Widget_Item_Signal_Data *wisd;
   Eina_List *l;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   EINA_LIST_FOREACH(item->signals, l, wisd)
     {
        if ((wisd->func == (Elm_Widget_Item_Signal_Cb)func) &&
            !strcmp(wisd->emission, emission) &&
            !strcmp(wisd->source, source))
          return _elm_widget_item_signal_callback_list_get(item, l);
     }

   return NULL;
}

EOLIAN static void
_elm_widget_item_signal_emit(Eo *eo_item EINA_UNUSED,
                             Elm_Widget_Item_Data *item EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{

}

EOLIAN static void
_elm_widget_item_access_info_set(Eo *eo_item EINA_UNUSED,
                                 Elm_Widget_Item_Data *item,
                                 const char *txt)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   eina_stringshare_del(item->access_info);
   if (!txt) item->access_info = NULL;
   else item->access_info = eina_stringshare_add(txt);
}

EOLIAN static Evas_Object*
_elm_widget_item_access_object_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   return item->access_obj;
}

static Elm_Translate_String_Data *
_translate_string_data_get(Eina_Inlist *translate_strings, const char *part)
{
   Elm_Translate_String_Data *ts;
   Eina_Stringshare *str;

   if (!translate_strings) return NULL;

   str = eina_stringshare_add(part);
   EINA_INLIST_FOREACH(translate_strings, ts)
     {
        if (ts->id == str) break;
     }

   eina_stringshare_del(str);

   return ts;
}

static Elm_Translate_String_Data *
_part_text_translatable_set(Eina_Inlist **translate_strings, const char *part, Eina_Bool translatable, Eina_Bool preset)
{
   Eina_Inlist *t;
   Elm_Translate_String_Data *ts;
   t = *translate_strings;
   ts = _translate_string_data_get(t, part);

   if (translatable)
     {
        if (!ts)
          {
             ts = ELM_NEW(Elm_Translate_String_Data);
             if (!ts) return NULL;

             ts->id = eina_stringshare_add(part);
             t = eina_inlist_append(t, (Eina_Inlist*) ts);
          }
        if (preset) ts->preset = EINA_TRUE;
     }
   //Delete this exist one if this part has been not preset.
   //see elm_widget_part_text_translatable_set()
   else if (ts && ((preset) || (!ts->preset)))
     {
        t = eina_inlist_remove(t, EINA_INLIST_GET(ts));
        eina_stringshare_del(ts->id);
        eina_stringshare_del(ts->domain);
        eina_stringshare_del(ts->string);
        ELM_SAFE_FREE(ts, free);
     }

   *translate_strings = t;

   return ts;
}

EOLIAN static void
_elm_widget_item_domain_translatable_part_text_set(Eo *eo_item EINA_UNUSED,
                                                   Elm_Widget_Item_Data *item,
                                                   const char *part,
                                                   const char *domain,
                                                   const char *label)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   Elm_Translate_String_Data *ts;

   if (!label)
     {
        _part_text_translatable_set(&item->translate_strings, part, EINA_FALSE,
                                    EINA_FALSE);
     }
   else
     {
        ts = _part_text_translatable_set(&item->translate_strings, part,
                                         EINA_TRUE, EINA_FALSE);
        if (!ts) return;
        if (!ts->string) ts->string = eina_stringshare_add(label);
        else eina_stringshare_replace(&ts->string, label);
        if (!ts->domain) ts->domain = eina_stringshare_add(domain);
        else eina_stringshare_replace(&ts->domain, domain);
#ifdef HAVE_GETTEXT
        if (label[0]) label = dgettext(domain, label);
#endif
     }
   item->on_translate = EINA_TRUE;
   elm_wdg_item_part_text_set(item->eo_obj, part, label);
   item->on_translate = EINA_FALSE;
}

EOLIAN static const char *
_elm_widget_item_translatable_part_text_get(const Eo *eo_item EINA_UNUSED,
                                            Elm_Widget_Item_Data *item,
                                            const char *part)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   Elm_Translate_String_Data *ts;
   ts = _translate_string_data_get(item->translate_strings, part);
   if (ts) return ts->string;
   return NULL;
}

EOLIAN static void
_elm_widget_item_translate(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

#ifdef HAVE_GETTEXT
   Elm_Translate_String_Data *ts;
   EINA_INLIST_FOREACH(item->translate_strings, ts)
     {
        if (!ts->string) continue;
        const char *s = dgettext(ts->domain, ts->string);
        item->on_translate = EINA_TRUE;
        elm_wdg_item_part_text_set(item->eo_obj, ts->id, s);
        item->on_translate = EINA_FALSE;
     }
#endif
}

EOLIAN static void
_elm_widget_item_domain_part_text_translatable_set(Eo *eo_item EINA_UNUSED,
                                                   Elm_Widget_Item_Data *item,
                                                   const char *part,
                                                   const char *domain,
                                                   Eina_Bool translatable)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   Elm_Translate_String_Data *ts;
   const char *text;

   ts = _part_text_translatable_set(&item->translate_strings, part,
                                    translatable, EINA_TRUE);
   if (!ts) return;
   if (!ts->domain) ts->domain = eina_stringshare_add(domain);
   else eina_stringshare_replace(&ts->domain, domain);

   text = elm_wdg_item_part_text_get(item->eo_obj, part);

   if (!text || !text[0]) return;

   if (!ts->string) ts->string = eina_stringshare_add(text);

//Try to translate text since we don't know the text is already translated.
#ifdef HAVE_GETTEXT
   text = dgettext(domain, text);
#endif
   item->on_translate = EINA_TRUE;
   elm_wdg_item_part_text_set(item->eo_obj, part, text);
   item->on_translate = EINA_FALSE;
}

EOLIAN static void
_elm_widget_item_track_cancel(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->track_obj) return;

   while (evas_object_ref_get(item->track_obj) > 0)
     evas_object_unref(item->track_obj);

   evas_object_del(item->track_obj);
}

static void
_track_obj_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_track_obj_update(Evas_Object *track, Evas_Object *obj)
{
   //Geometry
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(track, x, y);
   evas_object_resize(track, w, h);

   //Visibility
   if (evas_object_visible_get(obj)) evas_object_show(track);
   else evas_object_hide(track);
}

static void
_track_obj_view_update(void *data, const Efl_Event *event)
{
   Elm_Widget_Item_Data *item = data;
   _track_obj_update(item->track_obj, event->object);
}

static void
_track_obj_view_del(void *data, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(tracker_callbacks,
                          { EFL_GFX_EVENT_RESIZE, _track_obj_view_update },
                          { EFL_GFX_EVENT_MOVE, _track_obj_view_update },
                          { EFL_GFX_EVENT_SHOW, _track_obj_view_update },
                          { EFL_GFX_EVENT_HIDE, _track_obj_view_update },
                          { EFL_EVENT_DEL, _track_obj_view_del });

static void
_track_obj_view_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Widget_Item_Data *item = data;

   while (evas_object_ref_get(item->track_obj) > 0)
     evas_object_unref(item->track_obj);

   evas_object_event_callback_del(item->track_obj, EVAS_CALLBACK_DEL,
                                  _track_obj_del);
   evas_object_del(item->track_obj);
   item->track_obj = NULL;
}

static void
_track_obj_del(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Widget_Item_Data *item = data;
   item->track_obj = NULL;

   if (!item->view) return;

   efl_event_callback_array_del(item->view, tracker_callbacks(), item);
}

EOLIAN static Evas_Object *
_elm_widget_item_track(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   if (item->track_obj)
     {
        evas_object_ref(item->track_obj);
        return item->track_obj;
     }

   if (!item->view)
     {
        WRN("view obj of the item(%p) is invalid. Please make sure the view obj is created!", item);
        return NULL;
     }

   Evas_Object *track =
      evas_object_rectangle_add(evas_object_evas_get(item->widget));
   evas_object_color_set(track, 0, 0, 0, 0);
   evas_object_pass_events_set(track, EINA_TRUE);
   _track_obj_update(track, item->view);
   evas_object_event_callback_add(track, EVAS_CALLBACK_DEL, _track_obj_del,
                                  item);

   efl_event_callback_array_add(item->view, tracker_callbacks(), item);

   evas_object_ref(track);

   item->track_obj = track;

   return track;
}

EOLIAN static void
_elm_widget_item_untrack(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->track_obj) return;
   evas_object_unref(item->track_obj);

   if (evas_object_ref_get(item->track_obj) == 0)
     evas_object_del(item->track_obj);
}

EOLIAN static int
_elm_widget_item_track_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, 0);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, 0);

   if (!item->track_obj) return 0;
   return evas_object_ref_get(item->track_obj);
}

/**
 * @internal
 *
 * Set the function to notify when item is being deleted.
 *
 * This function will complain if there was a callback set already,
 * however it will set the new one.
 *
 * The callback will be called from elm_widget_item_pre_notify_del()
 * or elm_widget_item_del() will be called with:
 *   - data: the Elm_Widget_Item::data value.
 *   - obj: the Elm_Widget_Item::widget evas object.
 *   - event_info: the item being deleted.
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_del_cb_set() convenience macro.
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_del_cb_set(Eo *eo_item EINA_UNUSED,
                            Elm_Widget_Item_Data *item,
                            Evas_Smart_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if ((item->del_func) && (item->del_func != func))
     WRN("You're replacing a previously set del_cb %p of item %p with %p",
         item->del_func, item->eo_obj, func);

   item->del_func = func;
}

/**
 * @internal
 *
 * Set the content to be shown in the tooltip item
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param item the widget item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @func, the tooltip is unset with
 *        elm_widget_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_tooltip_content_cb_set(Eo *eo_item EINA_UNUSED,
                                        Elm_Widget_Item_Data *item,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void *data,
                                        Evas_Smart_Cb del_cb)
{
   Elm_Widget_Item_Tooltip *wit;

   ELM_WIDGET_ITEM_CHECK_OR_GOTO(item, error_noitem);
   //ELM_WIDGET_ITEM_RETURN_IF_GOTO(item, error_noitem);

   if (!func)
     {
        elm_wdg_item_tooltip_unset(item->eo_obj);
        return;
     }

   wit = ELM_NEW(Elm_Widget_Item_Tooltip);
   if (!wit) goto error;
   wit->item = item;
   wit->func = func;
   wit->data = data;
   wit->del_cb = del_cb;

   elm_object_sub_tooltip_content_cb_set
     (item->view, item->widget, _elm_widget_item_tooltip_create, wit,
     _elm_widget_item_tooltip_del_cb);

   return;

error_noitem:
   if (del_cb) del_cb((void *)data, NULL, item);
   return;
error:
   if (del_cb) del_cb((void *)data, item->widget, item);
}

EOLIAN static void
_elm_widget_item_access_order_unset(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   _elm_access_widget_item_access_order_unset(item);
}

EOLIAN static Evas_Object*
_elm_widget_item_access_register(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   _elm_access_widget_item_register(item);
   return item->access_obj;
}

EOLIAN static void
_elm_widget_item_access_unregister(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   _elm_access_widget_item_unregister(item);
}

EOLIAN static Eina_Bool
_elm_widget_item_del_pre(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Evas_Object *
_elm_widget_item_focus_next_object_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Elm_Focus_Direction dir)
{
   Evas_Object *ret = NULL;

   if (dir == ELM_FOCUS_PREVIOUS)
     ret = item->focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     ret = item->focus_next;
   else if (dir == ELM_FOCUS_UP)
     ret = item->focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     ret = item->focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     ret = item->focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     ret = item->focus_left;

   return ret;
}

EOLIAN static void
_elm_widget_item_focus_next_object_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Evas_Object *next, Elm_Focus_Direction dir)
{
   if (dir == ELM_FOCUS_PREVIOUS)
     item->focus_previous = next;
   else if (dir == ELM_FOCUS_NEXT)
     item->focus_next = next;
   else if (dir == ELM_FOCUS_UP)
     item->focus_up = next;
   else if (dir == ELM_FOCUS_DOWN)
     item->focus_down = next;
   else if (dir == ELM_FOCUS_RIGHT)
     item->focus_right = next;
   else if (dir == ELM_FOCUS_LEFT)
     item->focus_left = next;
}

EOLIAN static Elm_Object_Item*
_elm_widget_item_focus_next_item_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Elm_Focus_Direction dir)
{
   Elm_Object_Item *ret = NULL;

   if (dir == ELM_FOCUS_PREVIOUS)
     ret = item->item_focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     ret = item->item_focus_next;
   else if (dir == ELM_FOCUS_UP)
     ret = item->item_focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     ret = item->item_focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     ret = item->item_focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     ret = item->item_focus_left;

   return ret;
}

EOLIAN static void
_elm_widget_item_focus_next_item_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Elm_Object_Item *next_item, Elm_Focus_Direction dir)
{
   if (dir == ELM_FOCUS_PREVIOUS)
     item->item_focus_previous = next_item;
   else if (dir == ELM_FOCUS_NEXT)
     item->item_focus_next = next_item;
   else if (dir == ELM_FOCUS_UP)
     item->item_focus_up = next_item;
   else if (dir == ELM_FOCUS_DOWN)
     item->item_focus_down = next_item;
   else if (dir == ELM_FOCUS_RIGHT)
     item->item_focus_right = next_item;
   else if (dir == ELM_FOCUS_LEFT)
     item->item_focus_left = next_item;
}

// implements
static void
_efl_del_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Widget_Item_Data *item = efl_data_scope_get(event->object, ELM_WIDGET_ITEM_CLASS);
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if (item->del_func)
      item->del_func((void *) WIDGET_ITEM_DATA_GET(event->object), item->widget, item->eo_obj);
}

/**
 * @internal
 *
 * Allocate a new Elm_Widget_Item-derived structure.
 *
 * The goal of this structure is to provide common ground for actions
 * that a widget item have, such as the owner widget, callback to
 * notify deletion, data pointer and maybe more.
 *
 * @param widget the owner widget that holds this item, must be an elm_widget!
 * @param alloc_size any number greater than sizeof(Elm_Widget_Item) that will
 *        be used to allocate memory.
 *
 * @return allocated memory that is already zeroed out, or NULL on errors.
 *
 * @see elm_widget_item_new() convenience macro.
 * @see elm_widget_item_del() to release memory.
 * @ingroup Widget
 */
EOLIAN static Eo *
_elm_widget_item_efl_object_constructor(Eo *eo_item, Elm_Widget_Item_Data *item)
{
   Evas_Object *widget;
   widget = efl_parent_get(eo_item);

   if (!elm_widget_is(widget))
     {
        ERR("Failed");
        return NULL;
     }

   eo_item = efl_constructor(efl_super(eo_item, ELM_WIDGET_ITEM_CLASS));

   EINA_MAGIC_SET(item, ELM_WIDGET_ITEM_MAGIC);

   item->widget = widget;
   item->eo_obj = eo_item;
   efl_event_callback_add(eo_item, EFL_EVENT_DEL, _efl_del_cb, NULL);

   return eo_item;
}

EOLIAN static void
_elm_widget_item_efl_object_destructor(Eo *eo_item, Elm_Widget_Item_Data *item)
{
   Elm_Translate_String_Data *ts;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   evas_object_del(item->view);

   eina_stringshare_del(item->access_info);
   eina_stringshare_del(item->accessible_name);

   while (item->signals)
     _elm_widget_item_signal_callback_list_get(item, item->signals);

   while (item->translate_strings)
     {
        ts = EINA_INLIST_CONTAINER_GET(item->translate_strings,
                                       Elm_Translate_String_Data);
        eina_stringshare_del(ts->id);
        eina_stringshare_del(ts->domain);
        eina_stringshare_del(ts->string);
        item->translate_strings = eina_inlist_remove(item->translate_strings,
                                                     item->translate_strings);
        free(ts);
     }
   eina_hash_free(item->labels);

   elm_interface_atspi_accessible_removed(eo_item);

   EINA_MAGIC_SET(item, EINA_MAGIC_NONE);

   efl_destructor(efl_super(eo_item, ELM_WIDGET_ITEM_CLASS));
}

static Eina_Bool
_item_onscreen_is(Elm_Object_Item *item)
{
   Eina_Rectangle r1, r2;
   Elm_Widget_Item_Data *id = efl_data_scope_get(item, ELM_WIDGET_ITEM_CLASS);
   if (!id || !id->view) return EINA_FALSE;

   if (!evas_object_visible_get(id->view))
     return EINA_FALSE;

   if (!_elm_widget_onscreen_is(id->widget))
     return EINA_FALSE;

   evas_object_geometry_get(id->view, &r1.x, &r1.y, &r1.w, &r1.h);
   if (eina_rectangle_is_empty(&r1))
     return EINA_FALSE;

   evas_object_geometry_get(id->widget, &r2.x, &r2.y, &r2.w, &r2.h);
   if (!eina_rectangles_intersect(&r1, &r2))
     return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Elm_Atspi_State_Set
_elm_widget_item_elm_interface_atspi_accessible_state_set_get(Eo *eo_item,
                                                              Elm_Widget_Item_Data *item EINA_UNUSED)
{
   Elm_Atspi_State_Set states = 0;

   STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSABLE);

   if (elm_object_item_focus_get(eo_item))
     STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSED);
   if (!elm_object_item_disabled_get(eo_item))
     {
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_ENABLED);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_SENSITIVE);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_VISIBLE);
     }
   if (_item_onscreen_is(eo_item))
     STATE_TYPE_SET(states, ELM_ATSPI_STATE_SHOWING);

   return states;
}

EOLIAN static void
_elm_widget_item_elm_interface_atspi_component_extents_get(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED, Eina_Bool screen_coords, int *x, int *y, int *w, int *h)
{
   int ee_x, ee_y;

   if (!sd->view)
     {
        if (x) *x = -1;
        if (y) *y = -1;
        if (w) *w = -1;
        if (h) *h = -1;
        return;
     }

   evas_object_geometry_get(sd->view, x, y, w, h);
   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(sd->view));
        if (!ee) return;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        if (x) *x += ee_x;
        if (y) *y += ee_y;
     }
}

EOLIAN static Eina_Bool
_elm_widget_item_elm_interface_atspi_component_extents_set(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED, Eina_Bool screen_coords EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static int
_elm_widget_item_elm_interface_atspi_component_layer_get(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED)
{
   if (!sd->view)
     return -1;
   return evas_object_layer_get(sd->view);
}

EOLIAN static Eina_Bool
_elm_widget_item_elm_interface_atspi_component_focus_grab(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *_pd EINA_UNUSED)
{
   elm_object_item_focus_set(obj, EINA_TRUE);
   return elm_object_item_focus_get(obj);
}

EOLIAN static double
_elm_widget_item_elm_interface_atspi_component_alpha_get(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED)
{
   int alpha;

   if (!sd->view) return -1.0;
   evas_object_color_get(sd->view, NULL, NULL, NULL, &alpha);
   return (double)alpha / 255.0;
}

#include "elm_widget_item.eo.c"
