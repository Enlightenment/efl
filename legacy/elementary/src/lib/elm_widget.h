#ifndef ELM_WIDGET_H
#define ELM_WIDGET_H

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 *
 * How to make your own widget? like this (where wname is your widget
 * name (space) and wparentname is you widget's parent widget name
 * (the base widget class if its a 'root' one).
 *
 * #include <Elementary.h>
 * #include "elm_priv.h"
 *
 * static const char ELM_WNAME_SMART_NAME[] = "elm_wname";
 *
 * #define ELM_WNAME_DATA_GET(o, sd) \
 *   Elm_WName_Smart_Data * sd = evas_object_smart_data_get(o)
 *
 * #define ELM_WNAME_CHECK(obj)                                      \
 *   if (!obj || !elm_widget_type_check((obj), ELM_WNAME_SMART_NAME, \
 *                                      __func__))                   \
 *     return
 *
 * typedef struct _Elm_WName_Smart_Class
 * {
 *    Elm_WParentName_Smart_Class base;
 * } Elm_WName_Smart_Class;
 *
 * typedef struct _Elm_WName_Smart_Data Elm_WName_Smart_Data;
 * struct _Elm_WName_Smart_Data
 * {
 *   Elm_WParentName_Smart_Data base;
 *   Evas_Object *sub; // or any private data needed for an instance
 *   // add any other instance data here too
 * };
 *
 * static const char SIG_CLICKED[] = "clicked";
 * static const Evas_Smart_Cb_Description _smart_callbacks[] = {
 *   {SIG_CLICKED, ""},
 *   {NULL, NULL}
 * };
 *
 * EVAS_SMART_SUBCLASS_NEW
 *   (ELM_WNAME_SMART_NAME, _elm_wname, Elm_WName_Smart_Class,
 *   Elm_WParentName_Smart_Class, elm_wparentname_smart_class_get,
 *   _smart_callbacks);
 *
 * static Eina_Bool
 * _elm_wname_smart_on_focus(Evas_Object *obj)
 * {
 *    ELM_WNAME_DATA_GET(obj, sd);
 *
 *    // handle focus going in and out - optional, but if you want to,
 *    // define this virtual function to handle it (e.g. to emit a
 *    // signal to an edje object)
 *
 *    if (elm_widget_focus_get(obj))
 *      {
 *         edje_object_signal_emit(sd->sub, "elm,action,focus", "elm");
 *         evas_object_focus_set(sd->sub, EINA_TRUE);
 *      }
 *    else
 *      {
 *         edje_object_signal_emit(sd->sub, "elm,action,unfocus", "elm");
 *         evas_object_focus_set(sd->sub, EINA_FALSE);
 *      }
 *
 *    return EINA_TRUE;
 * }
 *
 * static Eina_Bool
 * _elm_wname_smart_theme(Evas_Object *obj)
 * {
 *    ELM_WNAME_DATA_GET(obj, sd);
 *
 *   if (!ELM_WIDGET_CLASS(_elm_wname_parent_sc)->theme(obj))
 *     return EINA_FALSE;
 *
 *    // handle changes in theme/scale etc here. always call the
 *    // parent class's version, as even the base class implements it.
 *
 *    elm_widget_theme_object_set(obj, sd->sub, "wname", "base",
 *                                elm_widget_style_get(obj));
 *
 *    return EINA_TRUE;
 * }
 *
 * static Eina_Bool
 * _elm_widget_smart_disable(Evas_Object *obj)
 * {
 *    ELM_WNAME_DATA_GET(obj, sd);
 *
 *    // optional, but handle if the widget gets disabled or not
 *    if (elm_widget_disabled_get(obj))
 *      edje_object_signal_emit(sd->sub, "elm,state,disabled", "elm");
 *    else
 *      edje_object_signal_emit(sd->sub, "elm,state,enabled", "elm");
 *
 *    return EINA_TRUE;
 * }
 *
 * static void
 * _elm_wname_smart_add(Evas_Object *obj)
 * {
 *    EVAS_SMART_DATA_ALLOC(obj, Elm_WName_Smart_Data);
 *
 *    ELM_WIDGET_CLASS(_elm_wname_parent_sc)->base.add(obj);
 *
 *    priv->sub = edje_object_add(evas_object_evas_get(obj));
 *    // just an example having an Edje object here. if it's really the case
 *    // you have a sub edje object as a resize object, consider inheriting
 *    // from @ref elm-layout-class.
 *    elm_widget_can_focus_set(obj, EINA_TRUE);
 *
 *    // for this widget we will add 1 sub object that is an edje object
 *    priv->sub = edje_object_add(e);
 *    // set the theme. this follows a scheme for group name like this:
 *    //   "elm/WIDGETNAME/ELEMENT/STYLE"
 *    // so here it will be:
 *    //   "elm/wname/base/default"
 *    // changing style changes style name from default (all widgets start
 *    // with the default style) and element is for your widget internal
 *    // structure as you see fit
 *    elm_widget_theme_object_set
 *      (obj, priv->sub, "wname", "base", "default");
 *    // listen to a signal from the edje object to produce widget smart
 *    // callback (like click)
 *    edje_object_signal_callback_add
 *      (priv->sub, "elm,action,click", "", _clicked_signal_cb, obj);
 *    // set this sub object as the "resize object". widgets get 1 resize
 *    // object that is resized along with the object wrapper.
 *    elm_widget_resize_object_set(obj, priv->sub, EINA_TRUE);
 * }
 *
 * static void
 * _elm_wname_smart_del(Evas_Object *obj)
 * {
 *    ELM_WNAME_DATA_GET(obj, sd);
 *
 *    // deleting 'virtual' function implementation - on deletion of
 *    // object delete object struct, etc.
 *
 *    ELM_WIDGET_CLASS(_elm_wname_parent_sc)->base.del(obj);
 * }
 *
 * static void
 * _elm_wname_smart_set_user(Elm_WName_Smart_Class *sc)
 * {
 *    ELM_WIDGET_CLASS(sc)->base.add = _elm_wname_smart_add;
 *    ELM_WIDGET_CLASS(sc)->base.del = _elm_wname_smart_del;
 *
 *    ELM_WIDGET_CLASS(sc)->theme = _elm_wname_smart_theme;
 *    ELM_WIDGET_CLASS(sc)->disable = _elm_wname_smart_disable;
 *    ELM_WIDGET_CLASS(sc)->on_focus = _elm_wname_smart_on_focus;
 * }
 *
 * // actual API to create your widget. add more to manipulate it as
 * // needed mark your calls with EAPI to make them "external api"
 * // calls.
 *
 * EAPI Evas_Object *
 * elm_wname_add(Evas_Object *parent)
 * {
 *    Evas_Object *obj;
 *
 *    EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
 *
 *    obj = elm_widget_add(_elm_check_smart_class_new(), parent);
 *    if (!obj) return NULL;
 *
 *    if (!elm_widget_sub_object_add(parent, obj))
 *      ERR("could not add %p as sub object of %p", obj, parent);
 *
 *    return obj;
 * }
 *
 * // example - do "whatever" to the widget (here just emit a signal)
 * EAPI void
 * elm_wname_whatever(Evas_Object *obj)
 * {
 *    // check if type is correct - check will return if it fails
 *    ELM_WNAME_CHECK(obj);
 *    // get widget data - type is correct and sane by this point, so this
 *    // should never fail
 *    ELM_WNAME_DATA_GET(obj, sd);
 *    // do whatever you like
 *    edje_object_signal_emit(sd->sub, "elm,state,action,whatever", "elm");
 * }
 *
 * // you can add more - you need to see elementary's code to know how
 * // to handle all cases. remember this api is not stable and may
 * change. it's internal
 */

#ifndef ELM_INTERNAL_API_ARGESFSDFEFC
#warning "You are using an internal elementary API. This API is not stable"
#warning "and is subject to change. You use this at your own risk."
#warning "Remember to call elm_widget_api_check(ELM_INTERNAL_API_VERSION);"
#warning "in your widgets before you call any other elm_widget calls to do"
#warning "a correct runtime version check. Also remember - you don't NEED"
#warning "to make an Elementary widget is almost ALL cases. You can easily"
#warning "make a smart object with Evas's API and do everything you need"
#warning "there. You only need a widget if you want to seamlessly be part"
#warning "of the focus tree and want to transparently become a container"
#warning "for any number of child Elementary widgets"
#error "ERROR. Compile aborted."
#endif
#define ELM_INTERNAL_API_VERSION 7000

/**
 * @defgroup Widget Widgets Extension Infrastructure
 *
 * This section is intended for people willing to create @b custom
 * Elementary widgets or to contribute new (useful, unique) widgets
 * upstream. If neither is your case, this text won't be of any use
 * for you.
 *
 * Elementary widgets are built in a @b hierarchical fashion. The idea
 * is to factorize as much code as possible between widgets with
 * behavioral similarities, as long as to facilitate the creation of
 * @b custom, new widgets, may the user need them.
 *
 * It all starts with a base class, which aggregates behaviour
 * @b every Elementary widget is supposed to have:
 * #Elm_Widget_Smart_Class. Every Elementary widget will be of that
 * type, be it directly or by means of @b inheriting from it.
 *
 * #Elm_Widget_Smart_Class happens to be an @c Evas_Smart_Class. If
 * you check out Evas' documentation on it, you'll see it's how one is
 * supposed to create custom Evas objects, what Elementary widgets
 * are.
 *
 * Once one instantiates an Elementary widget, since it inherits from
 * #Elm_Widget_Smart_Class, the system will raise a class instance of
 * that type for you. But that happens only @b once: the first time
 * you ask for an Elementary widget (of a given type). All subsequent
 * ones will only point to the very same class instance. Since it's
 * the class which points to the functions implementing the behavior
 * of objects of that type, all of the live instances of Elementary
 * widgets (of that type) will share the same blob of code loaded in
 * memory to execute their routines.
 *
 * Now go and take a look at #Elm_Widget_Smart_Class's fields. Because
 * it inherits from Evas' base smart class, we got a field of that
 * type as the first member, so that Evas can handle Elementary
 * objects internally as if they were 'normal' Evas objects. Evas has
 * the Evas-only behavior function pointers in there, so it's all it
 * needs.
 *
 * Then, comes a version field, so that whenever we got to update or
 * change the fields on our base smart class, there'll be a runtime
 * check of the version expected by Elementary and the one provided by
 * any code linking with it. A mismatch will show the developer of
 * that code he/she needs to recompile and link its code to a newer
 * version of Elementary.
 *
 * The next fields are the class functions themselves. We call them
 * 'virtual' because, as in object-oriented languages, one is supposed
 * here to override them on inheriting classes. On most of
 * inheritances you'll probably want to call the parent's version of
 * the class function too: you must analyse each case to tell.
 *
 * Take a look at #Elm_Widget_Smart_Data. That's private data bound to
 * each Elementary object @b instance. It aggregates data needed for
 * all widgets, since it's meant for the #Elm_Widget_Smart_Class-typed
 * ones.
 *
 * When inheriting from that base type, instance data for this new
 * class has to have, as the first member, a field of type
 * #Elm_Widget_Smart_Data. This has to be respected recursively -- if
 * a third class is to be created inheriting from the one that is a
 * direct 'child' of #Elm_Widget_Smart_Class, then the private data on
 * this third class has to have, as its first field, a variable of the
 * type of the private data of the second class (its parent), direct
 * child of #Elm_Widget_Smart_Class.
 *
 * It is from the base private data, #Elm_Widget_Smart_Data, that we
 * reach an object's class functions, by the given object
 * instance. This is the reason of the first field of that struct: a
 * pointer set to point to its class when the object is instantiated.
 *
 * The following figure illustrates the widget inheritance schema.
 *
 * @image html elm-widget-hierarchy.png
 * @image rtf elm-widget-hierarchy.png
 * @image latex elm-widget-hierarchy.eps
 *
 * @section elm-hierarchy-tree Elementary Widgets Hierarchy Tree
 *
 * The following figure illustrates the Elementary widget inheritance
 * tree.
 *
 * @image html elm-widget-tree.png
 * @image rtf elm-widget-tree.png
 * @image latex elm-widget-tree.eps
 */

/**
 * @addtogroup Widget
 * @{
 */

/* Elm_Activate is used in 'Virtual' function Eina_Bool (*activate)
 * (Evas_Object *obj, Elm_Activate act); of Elm_Widget_Smart_Class */
typedef enum
{
   ELM_ACTIVATE_DEFAULT = 0,
   ELM_ACTIVATE_UP,
   ELM_ACTIVATE_DOWN,
   ELM_ACTIVATE_RIGHT,
   ELM_ACTIVATE_LEFT,
   ELM_ACTIVATE_BACK
} Elm_Activate;

/* Please, ALWAYS update the ELM_WIDGET_SMART_CLASS_INIT macro
 * whenever you change the following struct! */

/**
 * Base widget smart class. It has the 'virtual' functions for all
 * general, common actions on Elementary widgets.
 */
typedef struct _Elm_Widget_Smart_Class
{
   Evas_Smart_Class base; /**< Base smart class struct, needed for all smart objects */
   int              version; /**< Version of this smart class definition */

   void             (*parent_set)(Evas_Object *obj,
                                  Evas_Object *parent); /**< 'Virtual' function handling parent widget attachment to new object */
   Eina_Bool        (*on_focus)(Evas_Object *obj); /**< 'Virtual' function handling focus in/out events on the widget */
   Eina_Bool        (*disable)(Evas_Object *obj); /**< 'Virtual' function on the widget being disabled */
   Eina_Bool        (*theme)(Evas_Object *obj); /**< 'Virtual' function on the widget being re-themed */
   Eina_Bool        (*translate)(Evas_Object *obj); /**< 'Virtual' function handling language changes on Elementary */
   Eina_Bool        (*event)(Evas_Object *obj,
                             Evas_Object *source,
                             Evas_Callback_Type type,
                             void *event_info); /**< 'Virtual' function handling input events on the widget */
   Eina_Bool        (*on_focus_region)(const Evas_Object *obj,
                                       Evas_Coord *x,
                                       Evas_Coord *y,
                                       Evas_Coord *w,
                                       Evas_Coord *h); /**< 'Virtual' function returning an inner area of a widget that should be brought into the visible area of a broader viewport, may this context arise. On the base Elementary widget class, it defaults to the object's total area, so only override it if you have to. */
   Eina_Bool        (*focus_next)(const Evas_Object *obj,
                                  Elm_Focus_Direction dir,
                                  Evas_Object **next); /**< 'Virtual' function handling passing focus to sub-objects */
   Eina_Bool        (*focus_direction)(const Evas_Object *obj,
                                       const Evas_Object *base,
                                       double degree,
                                       Evas_Object **target,
                                       double *weight); /**< 'Virtual' function handling passing focus to sub-objects <b>given a direction, in degrees</b> */

   Eina_Bool        (*sub_object_add)(Evas_Object *obj,
                                      Evas_Object *sobj); /**< 'Virtual' function handling sub objects being added */

   Eina_Bool        (*sub_object_del)(Evas_Object *obj,
                                      Evas_Object *sobj); /**< 'Virtual' function handling sub objects being removed */
   void             (*access)(Evas_Object *obj,
                              Eina_Bool is_access); /**< 'Virtual' function on the widget being set access */
   Eina_Bool        (*activate)(Evas_Object *obj,
                                Elm_Activate act); /**< 'Virtual' function to activate widget  */
} Elm_Widget_Smart_Class;

/**
 * Base widget smart data. This is data bound to an Elementary object
 * @b instance, so its particular to that specific object and not
 * shared between all objects in its class. It is here, though, that
 * we got a pointer to the object's class, the first field -- @c
 * 'api'.
 */
typedef struct _Elm_Widget_Smart_Data
{
   Evas_Object                  *obj;
   Evas_Object                  *parent_obj;
   Evas_Object                  *parent2;
   Evas_Coord                    x, y, w, h;
   Eina_List                    *subobjs;
   Evas_Object                  *resize_obj;
   Evas_Object                  *hover_obj;
   Eina_List                    *tooltips, *cursors;
   Evas_Object                  *focus_previous, *focus_next;
   Evas_Object                  *focus_up, *focus_down, *focus_right, *focus_left;

   /* "show region" coordinates. all widgets got those because this
    * info may be set and queried recursively through the widget
    * parenting tree */
   Evas_Coord                    rx, ry, rw, rh;

   /* scrolling hold/freeze hints. all widgets got those because this
    * info may be set and queried recursively through the widget
    * parenting tree */
   int                           scroll_hold;
   int                           scroll_freeze;

   double                        scale;
   Elm_Theme                    *theme;
   const char                   *style;
   const char                   *focus_highlight_style;  /**< custom focus style for a widget */
   const char                   *access_info;
   unsigned int                  focus_order;
   Eina_Bool                     focus_order_on_calc;

   int                           child_drag_x_locked;
   int                           child_drag_y_locked;

   Eina_Inlist                  *translate_strings;
   Eina_List                    *focus_chain;
   Eina_List                    *event_cb;

   /* this is a hook to be set on-the-fly on widgets. this is code
    * handling the request of showing a specific region from an inner
    * widget (mainly issued by entries, on cursor moving) */
   void                         *on_show_region_data;
   void                        (*on_show_region)(void *data,
                                                 Evas_Object *obj);

   int                           orient_mode; /* -1 is disabled */

   Eina_Bool                     drag_x_locked : 1;
   Eina_Bool                     drag_y_locked : 1;

   Eina_Bool                     can_focus : 1;
   Eina_Bool                     child_can_focus : 1;
   Eina_Bool                     focused : 1;
   Eina_Bool                     top_win_focused : 1;
   Eina_Bool                     tree_unfocusable : 1;
   Eina_Bool                     highlight_ignore : 1;
   Eina_Bool                     highlight_in_theme : 1;
   Eina_Bool                     access_highlight_in_theme : 1;
   Eina_Bool                     disabled : 1;
   Eina_Bool                     is_mirrored : 1;
   Eina_Bool                     mirrored_auto_mode : 1; /* This is
                                                          * TRUE by
                                                          * default */
   Eina_Bool                     still_in : 1;
   Eina_Bool                     highlighted : 1;
   Eina_Bool                     highlight_root : 1;
   Eina_Bool                     on_translate : 1; /**< This is true when any types of elm translate function is being called. */
   Eina_Bool                     on_create : 1; /**< This is true when the widget is on creation(general widget constructor). */
} Elm_Widget_Smart_Data;

/**
 * @}
 */

typedef struct _Elm_Tooltip     Elm_Tooltip;
typedef struct _Elm_Cursor      Elm_Cursor;

/**< base structure for all widget items that are not Elm_Widget themselves */
typedef struct _Elm_Widget_Item Elm_Widget_Item;
typedef struct _Elm_Widget_Item_Signal_Data Elm_Widget_Item_Signal_Data;

/**< accessibility information to be able to set and get from the access API */
typedef struct _Elm_Access_Info Elm_Access_Info;

/**< accessibility info item */
typedef struct _Elm_Access_Item Elm_Access_Item;

typedef struct _Elm_Action Elm_Action;

typedef void                  (*Elm_Widget_Text_Set_Cb)(void *data, const char *part, const char *text);
typedef void                  (*Elm_Widget_Content_Set_Cb)(void *data, const char *part, Evas_Object *content);
typedef const char           *(*Elm_Widget_Text_Get_Cb)(const void *data, const char *part);
typedef Evas_Object          *(*Elm_Widget_Content_Get_Cb)(const void *data, const char *part);
typedef Evas_Object          *(*Elm_Widget_Content_Unset_Cb)(const void *data, const char *part);
typedef void                  (*Elm_Widget_Signal_Emit_Cb)(void *data, const char *emission, const char *source);
typedef void                  (*Elm_Widget_Disable_Cb)(void *data);
typedef Eina_Bool             (*Elm_Widget_Del_Pre_Cb)(void *data);
typedef void                  (*Elm_Widget_Item_Signal_Cb)(void *data, Elm_Widget_Item *item, const char *emission, const char *source);
typedef void                  (*Elm_Widget_Style_Set_Cb)(void *data, const char *style);
typedef const char           *(*Elm_Widget_Style_Get_Cb)(const void *data);
typedef void                  (*Elm_Widget_Focus_Set_Cb)(void *data, Eina_Bool focused);
typedef Eina_Bool             (*Elm_Widget_Focus_Get_Cb)(const void *data);

#define ELM_ACCESS_DONE          -1   /* sentence done - send done event here */
#define ELM_ACCESS_CANCEL        -2   /* stop reading immediately */

typedef void (*Elm_Access_On_Highlight_Cb)(void *data);

struct _Elm_Access_Item
{
   int                   type;
   const void           *data;
   Elm_Access_Info_Cb    func;
};

struct _Elm_Access_Info
{
   Evas_Object               *hoverobj;
   Eina_List                 *items;
   Ecore_Timer               *delay_timer;
   void                      *on_highlight_data;
   Elm_Access_On_Highlight_Cb on_highlight;

   void                      *activate_data;
   Elm_Access_Activate_Cb    activate;

   /* the owner widget item that owns this access info */
   Elm_Widget_Item           *widget_item;

   /* the owner part object that owns this access info */
   Evas_Object               *part_object;

   Evas_Object               *next;
   Evas_Object               *prev;
};

struct _Elm_Action
{
   const char *name;
   Eina_Bool (*func)(Evas_Object *obj, const char *params);
};

void                  _elm_access_shutdown();
void                  _elm_access_mouse_event_enabled_set(Eina_Bool enabled);

/* if auto_higlight is EINA_TRUE, it  does not steal a focus, it just moves a highlight */
void                  _elm_access_auto_highlight_set(Eina_Bool enabled);
Eina_Bool             _elm_access_auto_highlight_get(void);
void                  _elm_access_widget_item_access_order_set(Elm_Widget_Item *item, Eina_List *objs);
const Eina_List      *_elm_access_widget_item_access_order_get(const Elm_Widget_Item *item);
void                  _elm_access_widget_item_access_order_unset(Elm_Widget_Item *item);
void                  _elm_widget_focus_highlight_start(const Evas_Object *obj);
void                  _elm_widget_highlight_in_theme_update(Eo *obj);
void                  _elm_win_focus_highlight_start(Evas_Object *obj);
void                   _elm_win_focus_highlight_in_theme_update(Evas_Object *obj, Eina_Bool in_theme);

EAPI void             _elm_access_clear(Elm_Access_Info *ac);
EAPI void             _elm_access_text_set(Elm_Access_Info *ac, int type, const char *text);
EAPI void             _elm_access_callback_set(Elm_Access_Info *ac, int type, Elm_Access_Info_Cb func, const void *data);
EAPI char            *_elm_access_text_get(const Elm_Access_Info *ac, int type, const Evas_Object *obj); /* this is ok it actually returns a strduped string - it's meant to! */
EAPI void             _elm_access_read(Elm_Access_Info *ac, int type, const Evas_Object *obj);
EAPI void             _elm_access_say(const char *txt);
EAPI Elm_Access_Info *_elm_access_info_get(const Evas_Object *obj);
EAPI void             _elm_access_object_highlight(Evas_Object *obj);
EAPI void             _elm_access_object_unhighlight(Evas_Object *obj);
EAPI void             _elm_access_object_highlight_disable(Evas *e);
EAPI void             _elm_access_object_register(Evas_Object *obj, Evas_Object *hoverobj);
EAPI void             _elm_access_object_unregister(Evas_Object *obj, Evas_Object *hoverobj);
EAPI Eina_Bool        _elm_access_2nd_click_timeout(Evas_Object *obj);
EAPI void             _elm_access_highlight_set(Evas_Object* obj);
EAPI Evas_Object *    _elm_access_edje_object_part_object_register(Evas_Object *obj, const Evas_Object *partobj, const char* part);
EAPI void             _elm_access_edje_object_part_object_unregister(Evas_Object* obj, const Evas_Object *eobj, const char* part);
EAPI void             _elm_access_widget_item_register(Elm_Widget_Item *item);
EAPI void             _elm_access_widget_item_unregister(Elm_Widget_Item *item);
EAPI void             _elm_access_on_highlight_hook_set(Elm_Access_Info *ac, Elm_Access_On_Highlight_Cb func, void *data);
EAPI void             _elm_access_activate_callback_set(Elm_Access_Info *ac, Elm_Access_Activate_Cb func, void *data);
EAPI void             _elm_access_highlight_object_activate(Evas_Object *obj, Elm_Activate act);
EAPI void             _elm_access_highlight_cycle(Evas_Object *obj, Elm_Focus_Direction dir);

EINA_DEPRECATED EAPI Elm_Access_Info *_elm_access_object_get(const Evas_Object *obj);

#define ELM_PREFS_DATA_MAGIC 0xe1f5da7a

/**< put this as the first member in your widget item struct */
#define ELM_WIDGET_ITEM       Elm_Widget_Item base

struct _Elm_Widget_Item_Signal_Data
{
   Elm_Widget_Item *item;
   Elm_Widget_Item_Signal_Cb func;
   const char *emission;
   const char *source;
   void *data;
};

struct _Elm_Widget_Item
{
/* ef1 ~~ efl, el3 ~~ elm */
#define ELM_WIDGET_ITEM_MAGIC 0xef1e1301
   EINA_MAGIC;
/* simple accessor macros */
#define VIEW(X)   X->base.view
#define WIDGET(X) X->base.widget
   /**< the owner widget that owns this item */
   Evas_Object                   *widget;
   /**< the base view object */
   Evas_Object                   *view;
   /**< item specific data. used for del callback */
   const void                    *data;
   /**< user delete callback function */
   Evas_Smart_Cb                  del_func;
   /**< widget delete callback function. don't expose this callback call */
   Elm_Widget_Del_Pre_Cb          del_pre_func;

   Elm_Widget_Content_Set_Cb      content_set_func;
   Elm_Widget_Content_Get_Cb      content_get_func;
   Elm_Widget_Content_Unset_Cb    content_unset_func;
   Elm_Widget_Text_Set_Cb         text_set_func;
   Elm_Widget_Text_Get_Cb         text_get_func;
   Elm_Widget_Signal_Emit_Cb      signal_emit_func;
   Elm_Widget_Disable_Cb          disable_func;
   Elm_Widget_Style_Set_Cb        style_set_func;
   Elm_Widget_Style_Get_Cb        style_get_func;
   Elm_Widget_Focus_Set_Cb        focus_set_func;
   Elm_Widget_Focus_Get_Cb        focus_get_func;
   Evas_Object                   *access_obj;
   const char                    *access_info;
   Eina_List                     *access_order;
   Eina_Inlist                   *translate_strings;
   Eina_List                     *signals;
   Eina_Hash                     *labels;
   Evas_Object                   *track_obj;

   Eina_Bool                      disabled : 1;
   Eina_Bool                      on_deletion : 1;
   Eina_Bool                      on_translate : 1;
};

struct _Elm_Object_Item
{
   ELM_WIDGET_ITEM;
};

#define ELM_NEW(t) calloc(1, sizeof(t))

EAPI Evas_Object     *elm_widget_add(Evas_Smart *, Evas_Object *);
EAPI void             elm_widget_parent_set(Evas_Object *, Evas_Object *);
EAPI Eina_Bool        elm_widget_api_check(int ver);
EAPI Eina_Bool        elm_widget_access(Evas_Object *obj, Eina_Bool is_access);
EAPI Eina_Bool        elm_widget_theme(Evas_Object *obj);
EAPI void             elm_widget_theme_specific(Evas_Object *obj, Elm_Theme *th, Eina_Bool force);
EAPI void             elm_widget_translate(Evas_Object *obj);
EAPI void             elm_widget_on_show_region_hook_set(Evas_Object *obj, void (*func)(void *data, Evas_Object *obj), void *data);
EAPI Eina_Bool        elm_widget_sub_object_parent_add(Evas_Object *sobj);
EAPI Eina_Bool        elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj);
EAPI Eina_Bool        elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj);
EAPI void             elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj, Eina_Bool sub_obj);
EAPI void             elm_widget_hover_object_set(Evas_Object *obj, Evas_Object *sobj);
EAPI void             elm_widget_signal_emit(Evas_Object *obj, const char *emission, const char *source);
EAPI void             elm_widget_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);
EAPI void            *elm_widget_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);
EAPI void             elm_widget_can_focus_set(Evas_Object *obj, Eina_Bool can_focus);
EAPI Eina_Bool        elm_widget_can_focus_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_child_can_focus_get(const Evas_Object *obj);
EAPI Eina_List       *elm_widget_can_focus_child_list_get(const Evas_Object *obj);
EAPI void             elm_widget_tree_unfocusable_set(Evas_Object *obj, Eina_Bool tree_unfocusable);
EAPI Eina_Bool        elm_widget_tree_unfocusable_get(const Evas_Object *obj);
EAPI void             elm_widget_highlight_ignore_set(Evas_Object *obj, Eina_Bool ignore);
EAPI Eina_Bool        elm_widget_highlight_ignore_get(const Evas_Object *obj);
EAPI void             elm_widget_highlight_in_theme_set(Evas_Object *obj, Eina_Bool highlight);
EAPI Eina_Bool        elm_widget_highlight_in_theme_get(const Evas_Object *obj);
EAPI void             elm_widget_access_highlight_in_theme_set(Evas_Object *obj, Eina_Bool highlight);
EAPI Eina_Bool        elm_widget_access_highlight_in_theme_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_focus_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_highlight_get(const Evas_Object *obj);
EAPI Evas_Object     *elm_widget_focused_object_get(const Evas_Object *obj);
EAPI Evas_Object     *elm_widget_top_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_is(const Evas_Object *obj);
EAPI Evas_Object     *elm_widget_parent_widget_get(const Evas_Object *obj);
EAPI void             elm_widget_event_callback_add(Evas_Object *obj, Elm_Event_Cb func, const void *data);
EAPI void            *elm_widget_event_callback_del(Evas_Object *obj, Elm_Event_Cb func, const void *data);
EAPI Eina_Bool        elm_widget_event_propagate(Evas_Object *obj, Evas_Callback_Type type, void *event_info, Evas_Event_Flags *event_flags);
EAPI void             elm_widget_focus_custom_chain_set(Evas_Object *obj, Eina_List *objs);
EAPI void             elm_widget_focus_custom_chain_unset(Evas_Object *obj);
EAPI const Eina_List *elm_widget_focus_custom_chain_get(const Evas_Object *obj);
EAPI void             elm_widget_focus_custom_chain_append(Evas_Object *obj, Evas_Object *child, Evas_Object *relative_child);
EAPI void             elm_widget_focus_custom_chain_prepend(Evas_Object *obj, Evas_Object *child, Evas_Object *relative_child);
EAPI void             elm_widget_focus_cycle(Evas_Object *obj, Elm_Focus_Direction dir);
EAPI Eina_Bool        elm_widget_focus_direction_go(Evas_Object *obj, double degree);
EAPI Eina_Bool        elm_widget_focus_direction_get(const Evas_Object *obj, const Evas_Object *base, double degree, Evas_Object **direction, double *weight);
EAPI Eina_Bool        elm_widget_focus_next_get(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next);
EAPI Eina_Bool        elm_widget_focus_list_direction_get(const Evas_Object  *obj, const Evas_Object *base, const Eina_List *items, void *(*list_data_get)(const Eina_List *list), double degree, Evas_Object **direction, double *weight);
EAPI Eina_Bool        elm_widget_focus_list_next_get(const Evas_Object *obj, const Eina_List *items, void *(*list_data_get)(const Eina_List *list), Elm_Focus_Direction dir, Evas_Object **next);
EAPI Evas_Object     *elm_widget_focus_next_object_get(const Evas_Object *obj, Elm_Focus_Direction dir);
EAPI void             elm_widget_focus_next_object_set(Evas_Object *obj, Evas_Object *next, Elm_Focus_Direction dir);
EAPI Eina_Bool        elm_widget_focus_highlight_style_set(Evas_Object *obj, const char *style);
EAPI const char      *elm_widget_focus_highlight_style_get(const Evas_Object *obj);
EAPI void             elm_widget_parent_highlight_set(Evas_Object *obj, Eina_Bool highlighted);
EAPI void             elm_widget_focus_set(Evas_Object *obj, Eina_Bool focus);
EAPI void             elm_widget_focused_object_clear(Evas_Object *obj);
EAPI Evas_Object     *elm_widget_parent_get(const Evas_Object *obj);
EAPI Evas_Object     *elm_widget_parent2_get(const Evas_Object *obj);
EAPI void             elm_widget_parent2_set(Evas_Object *obj, Evas_Object *parent);
EAPI void             elm_widget_focus_steal(Evas_Object *obj);
EAPI Evas_Object     *elm_widget_newest_focus_order_get(const Evas_Object *obj, unsigned int *newest_focus_order, Eina_Bool can_focus_only);
EAPI void             elm_widget_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode);
EAPI Eina_Bool        elm_widget_focus_highlight_enabled_get(const Evas_Object *obj);
EAPI void             elm_widget_focus_highlight_focus_part_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI const Elm_Widget_Smart_Class *elm_widget_smart_class_get(void);

/**
 * @internal
 *
 * Restore the focus state of the sub-tree.
 *
 * This API will restore the focus state of the sub-tree to the latest
 * state. If a sub-tree is unfocused and wants to get back to the latest
 * focus state, this API will be helpful.
 *
 * @param obj The widget root of sub-tree
 *
 * @ingroup Widget
 */
EAPI void             elm_widget_focus_restore(Evas_Object *obj);

EAPI void             elm_widget_disabled_set(Evas_Object *obj, Eina_Bool disabled);
EAPI Eina_Bool        elm_widget_disabled_get(const Evas_Object *obj);
EAPI void             elm_widget_show_region_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool forceshow);
EAPI void             elm_widget_show_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI Eina_Bool        elm_widget_focus_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI void             elm_widget_focus_region_show(const Evas_Object *obj);
EAPI void             elm_widget_parents_bounce_get(const Evas_Object *obj, Eina_Bool *horiz, Eina_Bool *vert);
EAPI void             elm_widget_scroll_hold_push(Evas_Object *obj);
EAPI void             elm_widget_scroll_hold_pop(Evas_Object *obj);
EAPI int              elm_widget_scroll_hold_get(const Evas_Object *obj);
EAPI void             elm_widget_scroll_freeze_push(Evas_Object *obj);
EAPI void             elm_widget_scroll_freeze_pop(Evas_Object *obj);
EAPI int              elm_widget_scroll_freeze_get(const Evas_Object *obj);
EAPI void             elm_widget_scale_set(Evas_Object *obj, double scale);
EAPI double           elm_widget_scale_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_mirrored_get(const Evas_Object *obj);
EAPI void             elm_widget_mirrored_set(Evas_Object *obj, Eina_Bool mirrored);
EAPI Eina_Bool        elm_widget_mirrored_automatic_get(const Evas_Object *obj);
EAPI void             elm_widget_mirrored_automatic_set(Evas_Object *obj, Eina_Bool automatic);
EAPI void             elm_widget_theme_set(Evas_Object *obj, Elm_Theme *th);
EAPI Elm_Theme       *elm_widget_theme_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_style_set(Evas_Object *obj, const char *style);
EAPI const char      *elm_widget_style_get(const Evas_Object *obj);
EAPI void             elm_widget_type_set(Evas_Object *obj, const char *type);
EAPI const char      *elm_widget_type_get(const Evas_Object *obj);
EAPI void             elm_widget_tooltip_add(Evas_Object *obj, Elm_Tooltip *tt);
EAPI void             elm_widget_tooltip_del(Evas_Object *obj, Elm_Tooltip *tt);
EAPI void             elm_widget_cursor_add(Evas_Object *obj, Elm_Cursor *cur);
EAPI void             elm_widget_cursor_del(Evas_Object *obj, Elm_Cursor *cur);
EAPI void             elm_widget_drag_lock_x_set(Evas_Object *obj, Eina_Bool lock);
EAPI void             elm_widget_drag_lock_y_set(Evas_Object *obj, Eina_Bool lock);
EAPI Eina_Bool        elm_widget_drag_lock_x_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_drag_lock_y_get(const Evas_Object *obj);
EAPI int              elm_widget_drag_child_locked_x_get(const Evas_Object *obj);
EAPI int              elm_widget_drag_child_locked_y_get(const Evas_Object *obj);
EAPI void             elm_widget_item_loop_enabled_set(Evas_Object *obj, Eina_Bool enable);
EAPI Eina_Bool        elm_widget_item_loop_enabled_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_theme_object_set(Evas_Object *obj, Evas_Object *edj, const char *wname, const char *welement, const char *wstyle);
EAPI Eina_Bool        elm_widget_type_check(const Evas_Object *obj, const char *type, const char *func);
EAPI Evas_Object     *elm_widget_name_find(const Evas_Object *obj, const char *name, int recurse);
EAPI Eina_List       *elm_widget_stringlist_get(const char *str);
EAPI void             elm_widget_stringlist_free(Eina_List *list);
EAPI void             elm_widget_focus_hide_handle(Evas_Object *obj);
EAPI void             elm_widget_focus_mouse_up_handle(Evas_Object *obj);
EAPI void             elm_widget_focus_tree_unfocusable_handle(Evas_Object *obj);
EAPI void             elm_widget_focus_disabled_handle(Evas_Object *obj);
EAPI unsigned int     elm_widget_focus_order_get(const Evas_Object *obj);
EAPI void             elm_widget_activate(Evas_Object *obj, Elm_Activate act);
EAPI void             elm_widget_part_text_set(Evas_Object *obj, const char *part, const char *label);
EAPI const char      *elm_widget_part_text_get(const Evas_Object *obj, const char *part);
EAPI void             elm_widget_domain_translatable_part_text_set(Evas_Object *obj, const char *part, const char *domain, const char *text);
EAPI const char      *elm_widget_translatable_part_text_get(const Evas_Object *obj, const char *part);
EAPI void             elm_widget_domain_part_text_translatable_set(Evas_Object *obj, const char *part, const char *domain, Eina_Bool translatable);
EAPI const char *     elm_widget_part_text_translate(Evas_Object *obj, const char *part, const char *text);
EAPI void             elm_widget_content_part_set(Evas_Object *obj, const char *part, Evas_Object *content);
EAPI Evas_Object     *elm_widget_content_part_get(const Evas_Object *obj, const char *part);
EAPI Evas_Object     *elm_widget_content_part_unset(Evas_Object *obj, const char *part);
EAPI void             elm_widget_access_info_set(Evas_Object *obj, const char *txt);
EAPI const char      *elm_widget_access_info_get(const Evas_Object *obj);
EAPI void             elm_widget_orientation_set(Evas_Object *obj, int rotation);
EAPI Elm_Object_Item *elm_widget_focused_item_get(const Evas_Object *obj);
EAPI void             elm_widget_orientation_mode_disabled_set(Evas_Object *obj, Eina_Bool disabled);
EAPI Eina_Bool        elm_widget_orientation_mode_disabled_get(const Evas_Object *obj);
EAPI void             elm_widget_focus_highlight_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI Elm_Widget_Item *_elm_widget_item_new(Evas_Object *parent, size_t alloc_size);
EAPI void             _elm_widget_item_free(Elm_Widget_Item *item);
EAPI Evas_Object     *_elm_widget_item_widget_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_del(Elm_Widget_Item *item);
EAPI void             _elm_widget_item_pre_notify_del(Elm_Widget_Item *item);
EAPI void             _elm_widget_item_del_cb_set(Elm_Widget_Item *item, Evas_Smart_Cb del_cb);
EAPI void             _elm_widget_item_data_set(Elm_Widget_Item *item, const void *data);
EAPI void            *_elm_widget_item_data_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_tooltip_text_set(Elm_Widget_Item *item, const char *text);
EAPI void             _elm_widget_item_tooltip_translatable_text_set(Elm_Widget_Item *item, const char *text);
EAPI void             _elm_widget_item_tooltip_content_cb_set(Elm_Widget_Item *item, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);
EAPI void             _elm_widget_item_tooltip_unset(Elm_Widget_Item *item);
EAPI void             _elm_widget_item_tooltip_style_set(Elm_Widget_Item *item, const char *style);
EAPI Eina_Bool        _elm_widget_item_tooltip_window_mode_set(Elm_Widget_Item *item, Eina_Bool disable);
EAPI Eina_Bool        _elm_widget_item_tooltip_window_mode_get(const Elm_Widget_Item *item);
EAPI const char      *_elm_widget_item_tooltip_style_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_cursor_set(Elm_Widget_Item *item, const char *cursor);
EAPI const char      *_elm_widget_item_cursor_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_cursor_unset(Elm_Widget_Item *item);
EAPI void             _elm_widget_item_cursor_style_set(Elm_Widget_Item *item, const char *style);
EAPI const char      *_elm_widget_item_cursor_style_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_cursor_engine_only_set(Elm_Widget_Item *item, Eina_Bool engine_only);
EAPI Eina_Bool        _elm_widget_item_cursor_engine_only_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_part_content_set(Elm_Widget_Item *item, const char *part, Evas_Object *content);
EAPI Evas_Object     *_elm_widget_item_part_content_get(const Elm_Widget_Item *item, const char *part);
EAPI Evas_Object     *_elm_widget_item_part_content_unset(Elm_Widget_Item *item, const char *part);
EAPI void             _elm_widget_item_part_text_set(Elm_Widget_Item *item, const char *part, const char *label);
EAPI const char      *_elm_widget_item_part_text_get(const Elm_Widget_Item *item, const char *part);
EAPI void             _elm_widget_item_part_text_custom_set(Elm_Widget_Item *item, const char *part, const char *label);
EAPI const char      *_elm_widget_item_part_text_custom_get(Elm_Widget_Item *item, const char *part);
EAPI void             _elm_widget_item_part_text_custom_update(Elm_Widget_Item *item);
EAPI void            _elm_widget_item_focus_set(Elm_Widget_Item *item, Eina_Bool focused);
EAPI Eina_Bool       _elm_widget_item_focus_get(const Elm_Widget_Item *item);
EAPI void            _elm_widget_item_style_set(Elm_Widget_Item *item, const char *style);
EAPI const char      *_elm_widget_item_style_get(Elm_Widget_Item *item);

EAPI void             _elm_widget_item_signal_callback_add(Elm_Widget_Item *item, const char *emission, const char *source, Elm_Widget_Item_Signal_Cb func, void *data);
EAPI void            *_elm_widget_item_signal_callback_del(Elm_Widget_Item *it, const char *emission, const char *source, Elm_Widget_Item_Signal_Cb func);
EAPI void             _elm_widget_item_signal_emit(Elm_Widget_Item *item, const char *emission, const char *source);
EAPI void             _elm_widget_item_content_set_hook_set(Elm_Widget_Item *item, Elm_Widget_Content_Set_Cb func);
EAPI void             _elm_widget_item_content_get_hook_set(Elm_Widget_Item *item, Elm_Widget_Content_Get_Cb func);
EAPI void             _elm_widget_item_content_unset_hook_set(Elm_Widget_Item *item, Elm_Widget_Content_Unset_Cb func);
EAPI void             _elm_widget_item_text_set_hook_set(Elm_Widget_Item *item, Elm_Widget_Text_Set_Cb func);
EAPI void             _elm_widget_item_text_get_hook_set(Elm_Widget_Item *item, Elm_Widget_Text_Get_Cb func);
EAPI void             _elm_widget_item_signal_emit_hook_set(Elm_Widget_Item *it, Elm_Widget_Signal_Emit_Cb func);
EAPI void             _elm_widget_item_access_info_set(Elm_Widget_Item *item, const char *txt);
EAPI void             _elm_widget_item_disabled_set(Elm_Widget_Item *item, Eina_Bool disabled);
EAPI Eina_Bool        _elm_widget_item_disabled_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_disable_hook_set(Elm_Widget_Item *item, Elm_Widget_Disable_Cb func);
EAPI void             _elm_widget_item_del_pre_hook_set(Elm_Widget_Item *item, Elm_Widget_Del_Pre_Cb func);
EAPI void             _elm_widget_item_style_set_hook_set(Elm_Widget_Item *item, Elm_Widget_Style_Set_Cb func);
EAPI void             _elm_widget_item_style_get_hook_set(Elm_Widget_Item *item, Elm_Widget_Style_Get_Cb func);
EAPI void             _elm_widget_item_focus_get_hook_set(Elm_Widget_Item *item, Elm_Widget_Focus_Get_Cb func);
EAPI void             _elm_widget_item_focus_set_hook_set(Elm_Widget_Item *item, Elm_Widget_Focus_Set_Cb func);
EAPI void             _elm_widget_item_domain_translatable_part_text_set(Elm_Widget_Item *item, const char *part, const char *domain, const char *label);
EAPI const char *     _elm_widget_item_translatable_part_text_get(const Elm_Widget_Item *item, const char *part);
EAPI void             _elm_widget_item_translate(Elm_Widget_Item *item);
EAPI void             _elm_widget_item_domain_part_text_translatable_set(Elm_Widget_Item *item, const char *part, const char *domain, Eina_Bool translatable);

EAPI Evas_Object     *elm_widget_item_track(Elm_Widget_Item *item);
EAPI void             elm_widget_item_untrack(Elm_Widget_Item *item);
EAPI int              elm_widget_item_track_get(const Elm_Widget_Item *item);
EAPI void             _elm_widget_item_track_cancel(Elm_Widget_Item *item);
void                  _elm_widget_item_highlight_in_theme(Evas_Object *obj, Elm_Object_Item *it);

/**
 * Function to operate on a given widget's scrollabe children when necessary.
 * @warning free the returned list with eina_list_free().
 */
EAPI Eina_List       *elm_widget_scrollable_children_get(const Evas_Object *obj);

/* debug function. don't use it unless you are tracking parenting issues */
EAPI void             elm_widget_tree_dump(const Evas_Object *top);
EAPI void             elm_widget_tree_dot_dump(const Evas_Object *top, FILE *output);

#define ELM_WIDGET_DATA_GET_OR_RETURN(o, ptr, ...)   \
  Elm_Widget_Smart_Data *ptr;                        \
  ptr = eo_data_scope_get(o, ELM_OBJ_WIDGET_CLASS);  \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("no widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#define ELM_WIDGET_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_WIDGET_CLASS))) \
    return

/**
 * Convenience macro to create new widget item, doing casts for you.
 * @see _elm_widget_item_new()
 * @param parent a valid elm_widget variant.
 * @param type the C type that extends Elm_Widget_Item
 */
#define elm_widget_item_new(parent, type) \
  (type *)_elm_widget_item_new((parent), sizeof(type))
/**
 * Convenience macro to free widget item, doing casts for you.
 * @see _elm_widget_item_free()
 * @param item a valid item.
 */
#define elm_widget_item_free(item) \
  _elm_widget_item_free((Elm_Widget_Item *)item)

/**
 * Convenience macro to delete widget item, doing casts for you.
 * @see _elm_widget_item_del()
 * @param item a valid item.
 */
#define elm_widget_item_del(item) \
  _elm_widget_item_del((Elm_Widget_Item *)item)
/**
 * Convenience macro to notify deletion of widget item, doing casts for you.
 * @see _elm_widget_item_pre_notify_del()
 */
#define elm_widget_item_pre_notify_del(item) \
  _elm_widget_item_pre_notify_del((Elm_Widget_Item *)item)
/**
 * Convenience macro to set deletion callback of widget item, doing casts for you.
 * @see _elm_widget_item_del_cb_set()
 */
#define elm_widget_item_del_cb_set(item, del_cb) \
  _elm_widget_item_del_cb_set((Elm_Widget_Item *)item, del_cb)

/**
 * Get item's owner widget
 * @see _elm_widget_item_widget_get()
 */
#define elm_widget_item_widget_get(item) \
  _elm_widget_item_widget_get((const Elm_Widget_Item *)item)

/**
 * Set item's data
 * @see _elm_widget_item_data_set()
 */
#define elm_widget_item_data_set(item, data) \
  _elm_widget_item_data_set((Elm_Widget_Item *)item, data)
/**
 * Get item's data
 * @see _elm_widget_item_data_get()
 */
#define elm_widget_item_data_get(item) \
  _elm_widget_item_data_get((const Elm_Widget_Item *)item)

/**
 * Convenience function to set widget item tooltip as a text string.
 * @see _elm_widget_item_tooltip_text_set()
 */
#define elm_widget_item_tooltip_text_set(item, text) \
  _elm_widget_item_tooltip_text_set((Elm_Widget_Item *)item, text)
/**
 * Convenience function to set widget item tooltip as a text string.
 * @see _elm_widget_item_tooltip_text_set()
 */
#define elm_widget_item_tooltip_translatable_text_set(item, text) \
  _elm_widget_item_tooltip_translatable_text_set((Elm_Widget_Item *)item, text)
/**
 * Convenience function to set widget item tooltip.
 * @see _elm_widget_item_tooltip_content_cb_set()
 */
#define elm_widget_item_tooltip_content_cb_set(item, func, data, del_cb) \
  _elm_widget_item_tooltip_content_cb_set((Elm_Widget_Item *)item,       \
                                          func, data, del_cb)
/**
 * Convenience function to unset widget item tooltip.
 * @see _elm_widget_item_tooltip_unset()
 */
#define elm_widget_item_tooltip_unset(item) \
  _elm_widget_item_tooltip_unset((Elm_Widget_Item *)item)
/**
 * Convenience function to change item's tooltip style.
 * @see _elm_widget_item_tooltip_style_set()
 */
#define elm_widget_item_tooltip_style_set(item, style) \
  _elm_widget_item_tooltip_style_set((Elm_Widget_Item *)item, style)

#define elm_widget_item_tooltip_window_mode_set(item, disable) \
  _elm_widget_item_tooltip_window_mode_set((Elm_Widget_Item *)item, disable)

#define elm_widget_item_tooltip_window_mode_get(item) \
  _elm_widget_item_tooltip_window_mode_get((Elm_Widget_Item *)item)
/**
 * Convenience function to query item's tooltip style.
 * @see _elm_widget_item_tooltip_style_get()
 */
#define elm_widget_item_tooltip_style_get(item) \
  _elm_widget_item_tooltip_style_get((const Elm_Widget_Item *)item)
/**
 * Convenience function to set widget item cursor.
 * @see _elm_widget_item_cursor_set()
 */
#define elm_widget_item_cursor_set(item, cursor) \
  _elm_widget_item_cursor_set((Elm_Widget_Item *)item, cursor)
/**
 * Convenience function to get widget item cursor.
 * @see _elm_widget_item_cursor_get()
 */
#define elm_widget_item_cursor_get(item) \
  _elm_widget_item_cursor_get((const Elm_Widget_Item *)item)
/**
 * Convenience function to unset widget item cursor.
 * @see _elm_widget_item_cursor_unset()
 */
#define elm_widget_item_cursor_unset(item) \
  _elm_widget_item_cursor_unset((Elm_Widget_Item *)item)
/**
 * Convenience function to change item's cursor style.
 * @see _elm_widget_item_cursor_style_set()
 */
#define elm_widget_item_cursor_style_set(item, style) \
  _elm_widget_item_cursor_style_set((Elm_Widget_Item *)item, style)
/**
 * Convenience function to query item's cursor style.
 * @see _elm_widget_item_cursor_style_get()
 */
#define elm_widget_item_cursor_style_get(item) \
  _elm_widget_item_cursor_style_get((const Elm_Widget_Item *)item)
/**
 * Convenience function to change item's cursor engine_only.
 * @see _elm_widget_item_cursor_engine_only_set()
 */
#define elm_widget_item_cursor_engine_only_set(item, engine_only) \
  _elm_widget_item_cursor_engine_only_set((Elm_Widget_Item *)item, engine_only)
/**
 * Convenience function to query item's cursor engine_only.
 * @see _elm_widget_item_cursor_engine_only_get()
 */
#define elm_widget_item_cursor_engine_only_get(item) \
  _elm_widget_item_cursor_engine_only_get((const Elm_Widget_Item *)item)
/**
 * Convenience function to query item's content set hook.
 * @see _elm_widget_item_content_set_hook_set()
 */
#define elm_widget_item_content_set_hook_set(item, func) \
  _elm_widget_item_content_set_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Content_Set_Cb)func)
/**
 * Convenience function to query item's content get hook.
 * @see _elm_widget_item_content_get_hook_set()
 */
#define elm_widget_item_content_get_hook_set(item, func) \
  _elm_widget_item_content_get_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Content_Get_Cb)func)
/**
 * Convenience function to query item's content unset hook.
 * @see _elm_widget_item_content_unset_hook_set()
 */
#define elm_widget_item_content_unset_hook_set(item, func) \
  _elm_widget_item_content_unset_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Content_Unset_Cb)func)
/**
 * Convenience function to query item's text set hook.
 * @see _elm_widget_item_text_set_hook_set()
 */
#define elm_widget_item_text_set_hook_set(item, func) \
  _elm_widget_item_text_set_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Text_Set_Cb)func)
/**
 * Convenience function to query item's text get hook.
 * @see _elm_widget_item_text_get_hook_set()
 */
#define elm_widget_item_text_get_hook_set(item, func) \
  _elm_widget_item_text_get_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Text_Get_Cb)func)
/**
 * Convenience function to query item's signal emit hook.
 * @see _elm_widget_item_signal_emit_hook_set()
 */
#define elm_widget_item_signal_emit_hook_set(item, func) \
  _elm_widget_item_signal_emit_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Signal_Emit_Cb)func)
/**
 * Convenience function to query disable get hook.
 * @see _elm_widget_item_disabled_get()
 */
#define elm_widget_item_disabled_get(item) \
  _elm_widget_item_disabled_get((Elm_Widget_Item *)item)
/**
 * Convenience function to query disable set hook.
 * @see _elm_widget_item_disable_hook_set()
 */
#define elm_widget_item_disable_hook_set(item, func) \
  _elm_widget_item_disable_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Disable_Cb)func)
/**
 * Convenience function to query del pre hook.
 * @see _elm_widget_item_del_pre_hook_set()
 */
#define elm_widget_item_del_pre_hook_set(item, func) \
  _elm_widget_item_del_pre_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Del_Pre_Cb)func)
/**
 * Convenience function to query style set hook.
 * @see _elm_widget_item_style_set_hook_set()
 */
#define elm_widget_item_style_set_hook_set(item, func) \
  _elm_widget_item_style_set_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Style_Set_Cb)func)
/**
 * Convenience function to query style get hook.
 * @see _elm_widget_item_style_get_hook_set()
 */
#define elm_widget_item_style_get_hook_set(item, func) \
  _elm_widget_item_style_get_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Style_Get_Cb)func)

/**
 * Convenience function to set style .
 * @see _elm_widget_item_style_set()
 */
#define elm_widget_item_style_set(item, style) \
  _elm_widget_item_style_set((Elm_Widget_Item *)item, style)
/**
 * Convenience function to get style .
 * @see _elm_widget_item_style_get()
 */
#define elm_widget_item_style_get(item) \
  _elm_widget_item_style_get((Elm_Widget_Item *)item)

/**
 * Convenience function to query focus set hook.
 * @see _elm_widget_item_focus_set_hook_set()
 */
#define elm_widget_item_focus_set_hook_set(item, func) \
  _elm_widget_item_focus_set_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Focus_Set_Cb)func)

/**
 * Convenience function to query focus get hook.
 * @see _elm_widget_item_focus_get_hook_set()
 */
#define elm_widget_item_focus_get_hook_set(item, func) \
  _elm_widget_item_focus_get_hook_set((Elm_Widget_Item *)item, (Elm_Widget_Focus_Get_Cb)func)

/**
 * Convenience function to query track_cancel.
 * @see _elm_widget_item_del_pre_hook_set()
 */
#define elm_widget_item_track_cancel(item) \
  _elm_widget_item_track_cancel((Elm_Widget_Item *)item)

/**
 * Convenience function to query translate hook.
 * @see _elm_widget_item_track_cancel()
 */
#define elm_widget_item_translate(item) \
  _elm_widget_item_translate((Elm_Widget_Item *)item)

/**
 * Convenience function to save additional text part content.
 * @see _elm_widget_item_part_text_custom_set()
 */
#define elm_widget_item_part_text_custom_set(item, part, text) \
  _elm_widget_item_part_text_custom_set((Elm_Widget_Item *)item, part, text)

/**
 * Convenience function to get additional text part content.
 * @see _elm_widget_item_part_text_custom_set()
 */
#define elm_widget_item_part_text_custom_get(item, part) \
  _elm_widget_item_part_text_custom_get((Elm_Widget_Item *)item, part)

/**
 * Convenience function to update additional text part content.
 * @see _elm_widget_item_part_text_custom_set()
 */
#define elm_widget_item_part_text_custom_update(item) \
  _elm_widget_item_part_text_custom_update((Elm_Widget_Item *)item)

/**
 * Convenience function to set the focus on widget item.
 * @see _elm_widget_item_focus_set()
 */
#define elm_widget_item_focus_set(item, focused) \
  _elm_widget_item_focus_set((Elm_Widget_Item *)item, focused)

/**
 * Convenience function to query focus set hook.
 * @see _elm_widget_item_focus_get()
 */
#define elm_widget_item_focus_get(item) \
  _elm_widget_item_focus_get((const Elm_Widget_Item *)item)

#define ELM_WIDGET_CHECK_OR_RETURN(obj, ...)                    \
   do {                                                         \
        if (!obj || !evas_object_smart_data_get(obj))           \
          {                                                     \
             ERR("Object [%p] is NULL or already deleted", obj);\
             return __VA_ARGS__;                                \
          }                                                     \
   } while (0)

#define ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, ...)              \
   do {                                                         \
       if (item && (item)->on_deletion) {                       \
            WRN("Elm_Widget_Item " # item " is deleting");      \
            return __VA_ARGS__;                                 \
        }                                                       \
   } while (0)

#define ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, ...)              \
   do {                                                         \
        if (!item) {                                            \
             CRI("Elm_Widget_Item " # item " is NULL");    \
             return __VA_ARGS__;                                \
        }                                                       \
       if (!EINA_MAGIC_CHECK(item, ELM_WIDGET_ITEM_MAGIC)) {    \
            EINA_MAGIC_FAIL(item, ELM_WIDGET_ITEM_MAGIC);       \
            return __VA_ARGS__;                                 \
         }                                                      \
  } while (0)

#define ELM_WIDGET_ITEM_CHECK_OR_GOTO(item, label)              \
  do {                                                          \
        if (!item) {                                            \
             CRI("Elm_Widget_Item " # item " is NULL");    \
             goto label;                                        \
        }                                                       \
       if (!EINA_MAGIC_CHECK(item, ELM_WIDGET_ITEM_MAGIC)) {    \
            EINA_MAGIC_FAIL(item, ELM_WIDGET_ITEM_MAGIC);       \
            goto label;                                         \
         }                                                      \
  } while (0)

/* to be used by INTERNAL classes on Elementary, so that the widgets
 * parsing script skips it */
#define ELM_INTERNAL_SMART_SUBCLASS_NEW EVAS_SMART_SUBCLASS_NEW

EAPI Eina_Bool elm_selection_selection_has_owner(Evas_Object *obj);

typedef void (*region_hook_func_type)(void *data, Evas_Object *obj);
typedef void * (*list_data_get_func_type)(const Eina_List * l);

#include "elm_widget.eo.h"

#endif
