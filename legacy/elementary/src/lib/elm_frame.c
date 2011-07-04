#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Frame Frame
 *
 * This holds some content and has a title. Looks like a frame, but
 * supports styles so multple frames are avaible
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *content;
   const char *label;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->frm, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->frm, "frame", "base", elm_widget_style_get(obj));
   edje_object_part_text_set(wd->frm, "elm.text", wd->label);
   if (wd->content)
     edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->frm, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static Eina_Bool
_elm_frame_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *cur;

   if ((!wd) || (!wd->content))
     return EINA_FALSE;

   cur = wd->content;

   /* Try Focus cycle in subitem */
   return elm_widget_focus_next_get(cur, dir, next);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   edje_object_size_min_calc(wd->frm, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   // FIXME: why is this needed? how does edje get this unswallowed or
   // lose its callbacks to edje
   if (!wd) return;
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->content)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->content = NULL;
        _sizing_eval(obj);
     }
}

static void
_elm_frame_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return;
   if (!wd) return;
   eina_stringshare_replace(&(wd->label), label);
   edje_object_part_text_set(wd->frm, "elm.text", wd->label);
   _sizing_eval(obj);
}

static const char *
_elm_frame_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Add a new frame to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Frame
 */
EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "frame");
   elm_widget_type_set(obj, "frame");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_frame_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_text_set_hook_set(obj, _elm_frame_label_set);
   elm_widget_text_get_hook_set(obj, _elm_frame_label_get);

   wd->frm = edje_object_add(e);
   _elm_theme_object_set(obj, wd->frm, "frame", "base", "default");
   elm_widget_resize_object_set(obj, wd->frm);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the frame label
 *
 * @param obj The frame object
 * @param label The label of this frame object
 *
 * @ingroup Frame
 * @deprecate use elm_object_text_* instead.
 */
EAPI void
elm_frame_label_set(Evas_Object *obj, const char *label)
{
   _elm_frame_label_set(obj, NULL, label);
}

/**
 * Get the frame label
 *
 * @param obj The frame object
 *
 * @return The label of this frame objet or NULL if unable to get frame
 *
 * @ingroup Frame
 * @deprecate use elm_object_text_* instead.
 */
EAPI const char *
elm_frame_label_get(const Evas_Object *obj)
{
   return _elm_frame_label_get(obj, NULL);
}

/**
 * Set the content of the frame widget
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_frame_content_unset() function.
 *
 * @param obj The frame object
 * @param content The content will be filled in this frame object
 *
 * @ingroup Frame
 */
EAPI void
elm_frame_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
     }
   _sizing_eval(obj);
}

/**
 * Get the content of the frame widget
 *
 * Return the content object which is set for this widget
 *
 * @param obj The frame object
 * @return The content that is being used
 *
 * @ingroup Frame
 */
EAPI Evas_Object *
elm_frame_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

/**
 * Unset the content of the frame widget
 *
 * Unparent and return the content object which was set for this widget
 *
 * @param obj The frame object
 * @return The content that was being used
 *
 * @ingroup Frame
 */
EAPI Evas_Object *
elm_frame_content_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, wd->content);
   edje_object_part_unswallow(wd->frm, wd->content);
   wd->content = NULL;
   return content;
}
