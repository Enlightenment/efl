#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Anchorblock Anchorblock
 *
 * Anchorblock is for displaying text that contains markup with anchors like:
 * \<a href=1234\>something\</\> in it. These will be drawn differently and will
 * be able to be clicked on by the user to display a popup. This popup then
 * is intended to contain extra options such as "call", "add to contacts",
 * "open web page" etc.
 *
 * Signals that you can add callbacks for are:
 *
 * "anchor,clicked" - anchor called was clicked. event_info is anchor info -
 *                    Elm_Entry_Anchorview_Info
 */
typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Anchorblock_Item_Provider Elm_Anchorblock_Item_Provider;

struct _Widget_Data
{
   Evas_Object *entry;
   Evas_Object *hover_parent;
   Evas_Object *pop, *hover;
   Eina_List *item_providers;
   const char *hover_style;
};

struct _Elm_Anchorblock_Item_Provider
{
   Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item);
   void *data;
};

static const char *widtype = NULL;

static const char SIG_ANCHOR_CLICKED[] = "anchor,clicked";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_ANCHOR_CLICKED, ""}, /* TODO: declare the type properly, as data is
                                  * being passed
                                  */
       {NULL, NULL}
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_event_callback_del_full(wd->entry, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
   elm_anchorblock_hover_end(obj);
   elm_anchorblock_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Anchorblock_Item_Provider *ip;
   if (!wd) return;
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   EINA_LIST_FREE(wd->item_providers, ip)
     {
        free(ip);
     }
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_mirrored_set(wd->hover, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->entry, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_anchorblock_hover_end(data);
}

static void
_anchor_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info *info = event_info;
   Evas_Object *hover_parent;
   Elm_Entry_Anchorblock_Info ei;
   Evas_Coord x, w, y, h, px, py;
   if (!wd) return;
   wd->pop = elm_icon_add(obj);
   evas_object_move(wd->pop, info->x, info->y);
   evas_object_resize(wd->pop, info->w, info->h);
   wd->hover = elm_hover_add(obj);
   elm_widget_mirrored_set(wd->hover, elm_widget_mirrored_get((Evas_Object *) data));
   if (wd->hover_style)
     elm_object_style_set(wd->hover, wd->hover_style);
   hover_parent = wd->hover_parent;
   if (!hover_parent) hover_parent = obj;
   elm_hover_parent_set(wd->hover, hover_parent);
   elm_hover_target_set(wd->hover, wd->pop);
   ei.name = info->name;
   ei.button = info->button;
   ei.hover = wd->hover;
   ei.anchor.x = info->x;
   ei.anchor.y = info->y;
   ei.anchor.w = info->w;
   ei.anchor.h = info->h;
   evas_object_geometry_get(hover_parent, &x, &y, &w, &h);
   ei.hover_parent.x = x;
   ei.hover_parent.y = y;
   ei.hover_parent.w = w;
   ei.hover_parent.h = h;
   px = info->x + (info->w / 2);
   py = info->y + (info->h / 2);
   ei.hover_left = 1;
   if (px < (x + (w / 3))) ei.hover_left = 0;
   ei.hover_right = 1;
   if (px > (x + ((w * 2) / 3))) ei.hover_right = 0;
   ei.hover_top = 1;
   if (py < (y + (h / 3))) ei.hover_top = 0;
   ei.hover_bottom = 1;
   if (py > (y + ((h * 2) / 3))) ei.hover_bottom = 0;

   if (elm_widget_mirrored_get(wd->hover))
     {  /* Swap right and left because they switch sides in RTL */
        Eina_Bool tmp = ei.hover_left;
        ei.hover_left = ei.hover_right;
        ei.hover_right = tmp;
     }

   evas_object_smart_callback_call(data, SIG_ANCHOR_CLICKED, &ei);
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, data);
   evas_object_show(wd->hover);
}

static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->hover_parent = NULL;
}

static Evas_Object *
_item_provider(void *data, Evas_Object *entry __UNUSED__, const char *item)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   Elm_Anchorblock_Item_Provider *ip;

   EINA_LIST_FOREACH(wd->item_providers, l, ip)
     {
        Evas_Object *o;

        o = ip->func(ip->data, data, item);
        if (o) return o;
     }
   return NULL;
}

/**
 * Add a new Anchorblock object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Anchorblock
 */
EAPI Evas_Object *
elm_anchorblock_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "anchorblock");
   elm_widget_type_set(obj, "anchorblock");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   wd->entry = elm_entry_add(parent);
   elm_entry_item_provider_prepend(wd->entry, _item_provider, obj);
   elm_widget_resize_object_set(obj, wd->entry);
   elm_entry_editable_set(wd->entry, 0);
   evas_object_size_hint_weight_set(wd->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);

   evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);

   elm_entry_entry_set(wd->entry, "");

   evas_object_smart_callback_add(wd->entry, "anchor,clicked",
                                  _anchor_clicked, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set the text markup of the anchorblock
 *
 * This sets the text of the anchorblock to be the text given as @p text. This
 * text is in markup format with \<a href=XXX\> beginning an achor with the
 * string link of 'XXX', and \</\> or \</a\> ending the link. Other markup can
 * be used dependign on the style support.
 *
 * @param obj The anchorblock object
 * @param text The text to set, or NULL to clear
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_text_set(Evas_Object *obj, const char *text)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_set(wd->entry, text);
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
   _sizing_eval(obj);
}

/**
 * Get the markup text set for the anchorblock
 *
 * This retrieves back the string set by @c elm_anchorblock_text_set().
 *
 * @param obj The anchorblock object
 * @return text The markup text set or @c NULL, either if it was not set
 * or an error occurred
 *
 * @ingroup Anchorblock
 */
EAPI const char*
elm_anchorblock_text_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_entry_get(wd->entry);
}

/**
 * Set the parent of the hover popup
 *
 * This sets the parent of the hover that anchorblock will create. See hover
 * objects for more information on this.
 *
 * @param obj The anchorblock object
 * @param parent The parent the hover should use
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_parent)
     evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

/**
 * Get the parent of the hover popup
 *
 * This sgets the parent of the hover that anchorblock will create. See hover
 * objects for more information on this.
 *
 * @param obj The anchorblock object
 * @return The parent used by the hover
 *
 * @ingroup Anchorblock
 */
EAPI Evas_Object *
elm_anchorblock_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->hover_parent;
}

/**
 * Set the style that the hover should use
 *
 * This sets the style for the hover that anchorblock will create. See hover
 * objects for more information
 *
 * @param obj The anchorblock object
 * @param style The style to use
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_hover_style_set(Evas_Object *obj, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->hover_style, style);
}

/**
 * Get the style that the hover should use
 *
 * This gets the style for the hover that anchorblock will create. See hover
 * objects for more information
 *
 * @param obj The anchorblock object
 * @return The style defined
 *
 * @ingroup Anchorblock
 */
EAPI const char *
elm_anchorblock_hover_style_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->hover_style;
}

/**
 * Stop the hover popup in the anchorblock
 *
 * This will stop the hover popup in the anchorblock if it is currently active.
 *
 * @param obj The anchorblock object
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_hover_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
}

/**
 * This appends a custom item provider to the list for that anchorblock
 *
 * This appends the given callback. The list is walked from beginning to end
 * with each function called given the item href string in the text. If the
 * function returns an object handle other than NULL (it should create an
 * and object to do this), then this object is used to replace that item. If
 * not the next provider is called until one provides an item object, or the
 * default provider in anchorblock does.
 *
 * @param obj The anchorblock object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   Elm_Anchorblock_Item_Provider *ip = calloc(1, sizeof(Elm_Anchorblock_Item_Provider));
   if (!ip) return;
   ip->func = func;
   ip->data = data;
   wd->item_providers = eina_list_append(wd->item_providers, ip);
}

/**
 * This prepends a custom item provider to the list for that anchorblock
 *
 * This prepends the given callback. See elm_anchorblock_item_provider_append() for
 * more information
 *
 * @param obj The anchorblock object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   Elm_Anchorblock_Item_Provider *ip = calloc(1, sizeof(Elm_Anchorblock_Item_Provider));
   if (!ip) return;
   ip->func = func;
   ip->data = data;
   wd->item_providers = eina_list_prepend(wd->item_providers, ip);
}

/**
 * This removes a custom item provider to the list for that anchorblock
 *
 * This removes the given callback. See elm_anchorblock_item_provider_append() for
 * more information
 *
 * @param obj The anchorblock object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Anchorblock
 */
EAPI void
elm_anchorblock_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Anchorblock_Item_Provider *ip;
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   EINA_LIST_FOREACH(wd->item_providers, l, ip)
     {
        if ((ip->func == func) && (ip->data == data))
          {
             wd->item_providers = eina_list_remove_list(wd->item_providers, l);
             free(ip);
             return;
          }
     }
}
