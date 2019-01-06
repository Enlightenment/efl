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
 *    if (efl_ui_focus_object_focus_get(obj))
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
 *    elm_widget_resize_object_set(obj, priv->sub);
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

#include "elm_object_item.h"

typedef Eina_Bool             (*Elm_Widget_Del_Pre_Cb)(void *data);
typedef void                  (*Elm_Widget_Item_Signal_Cb)(void *data, Elm_Object_Item *item, const char *emission, const char *source);

typedef void (*Elm_Access_On_Highlight_Cb)(void *data);

#include "efl_ui_widget.eo.h"

/**
 * @addtogroup Widget
 * @{
 */

/**
 * Base widget smart data. This is data bound to an Elementary object
 * @b instance, so its particular to that specific object and not
 * shared between all objects in its class. It is here, though, that
 * we got a pointer to the object's class, the first field -- @c
 * 'api'.
 */
typedef struct _Elm_Widget_Smart_Data
{
   Evas_Object                  *obj; /**< object pointer for this widget smart data */
   Evas_Object                  *parent_obj; /**< parent object of a widget in the elementary tree */
   Evas_Coord                    x, y, w, h;
   Eina_List                    *subobjs; /**< list of widgets' sub objects in the elementary tree */
   Evas_Object                  *resize_obj; /**< an unique object for each widget that shows the look of a widget. Resize object's geometry is same as the widget. This resize object is different from that of window's resize object. */
   Evas_Object                  *hover_obj;
   Evas_Object                  *bg;
   Eina_List                    *tooltips, *cursors;

   /* "show region" coordinates. all widgets got those because this
    * info may be set and queried recursively through the widget
    * parenting tree */
   Eina_Rect                     show_region;

   /* scrolling hold/freeze hints. all widgets got those because this
    * info may be set and queried recursively through the widget
    * parenting tree */
   int                           scroll_hold;
   int                           scroll_freeze;

   double                        scale;
   Elm_Theme                    *theme;
   const char                   *klass; /**< 1st identifier of an edje object group which is used in theme_set. klass and group are used together. */
   const char                   *group; /**< 2nd identifier of an edje object group which is used in theme_set. klass and group are used together. */
   const char                   *style;
   const char                   *access_info;
   const char                   *accessible_name;
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
   Efl_Ui_Scrollable_On_Show_Region on_show_region;
   Eina_Free_Cb                  on_show_region_data_free;

   int                           orient_mode; /* -1 is disabled */
   Elm_Focus_Move_Policy         focus_move_policy;
   Elm_Focus_Region_Show_Mode    focus_region_show_mode;

   Efl_Ui_Widget_Focus_State focus;
   struct {
      int child_count;
      Efl_Ui_Focus_Object *parent;
   } logical;
   struct {
      Efl_Ui_Focus_Manager *manager;
      Efl_Ui_Focus_Object *provider;
   } manager;
   struct {
     Eina_Bool listen_to_manager;
     Eina_List *custom_chain;
     Evas_Object *prev, *next, *up, *down, *right, *left;
     Elm_Object_Item *item_prev, *item_next, *item_up, *item_down, *item_right, *item_left;
   } legacy_focus;

   Eo                            *access_object;
   Eina_Bool                     scroll_x_locked : 1;
   Eina_Bool                     scroll_y_locked : 1;

   Eina_Bool                     can_focus : 1;
   Eina_Bool                     child_can_focus : 1;
   Eina_Bool                     focused : 1;
   Eina_Bool                     top_win_focused : 1;
   Eina_Bool                     tree_unfocusable : 1;
   Eina_Bool                     focus_move_policy_auto_mode : 1; /* This is TRUE by default */
   Eina_Bool                     highlight_ignore : 1;
   Eina_Bool                     highlight_in_theme : 1;
   Eina_Bool                     access_highlight_in_theme : 1;
   Eina_Bool                     disabled : 1;
   Eina_Bool                     is_mirrored : 1;
   Eina_Bool                     mirrored_auto_mode : 1; /* This is TRUE by default */
   Eina_Bool                     still_in : 1;
   Eina_Bool                     highlighted : 1;
   Eina_Bool                     highlight_root : 1;
   Eina_Bool                     on_translate : 1; /**< This is true when any types of elm translate function is being called. */
   Eina_Bool                     on_create : 1; /**< This is true when the widget is on creation(general widget constructor). */
   Eina_Bool                     on_destroy: 1; /**< This is true when the widget is on destruction(general widget destructor). */
   Eina_Bool                     provider_lookup : 1; /**< This is true when efl_provider_find is currently walking the tree */
   Eina_Bool                     has_shadow : 1;
} Elm_Widget_Smart_Data;

typedef Elm_Widget_Smart_Data Efl_Ui_Widget_Data;

/**
 * @}
 */

/**< base structure for all widget items that are not Efl_Ui_Widget themselves */
typedef struct _Elm_Widget_Item_Data Elm_Widget_Item_Data;
typedef struct _Elm_Widget_Item_Signal_Data Elm_Widget_Item_Signal_Data;

/**< accessibility information to be able to set and get from the access API */
typedef struct _Elm_Access_Info Elm_Access_Info;

/**< accessibility info item */
typedef struct _Elm_Access_Item Elm_Access_Item;

typedef struct _Elm_Action Elm_Action;

/** Internal type for mouse cursors */
typedef struct _Elm_Cursor Elm_Cursor;

/** Internal type for tooltips */
typedef struct _Elm_Tooltip Elm_Tooltip;

#define ELM_ACCESS_DONE          -1   /* sentence done - send done event here */
#define ELM_ACCESS_CANCEL        -2   /* stop reading immediately */

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
   Elm_Widget_Item_Data      *widget_item;

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

void                  _elm_access_shutdown(void);
void                  _elm_access_mouse_event_enabled_set(Eina_Bool enabled);

/* if auto_higlight is EINA_TRUE, it  does not steal a focus, it just moves a highlight */
void                  _elm_access_auto_highlight_set(Eina_Bool enabled);
Eina_Bool             _elm_access_auto_highlight_get(void);
void                  _elm_access_widget_item_access_order_set(Elm_Widget_Item_Data *item, Eina_List *objs);
const Eina_List      *_elm_access_widget_item_access_order_get(const Elm_Widget_Item_Data *item);
void                  _elm_access_widget_item_access_order_unset(Elm_Widget_Item_Data *item);

// widget focus highlight
void                  _elm_widget_focus_highlight_start(const Evas_Object *obj);
void                  _elm_widget_highlight_in_theme_update(Eo *obj);

// win focus highlight
void                  _elm_win_focus_highlight_start(Evas_Object *obj);
void                  _elm_win_focus_highlight_in_theme_update(Evas_Object *obj, Eina_Bool in_theme);
Evas_Object          *_elm_win_focus_highlight_object_get(Evas_Object *obj);
void                  _elm_win_focus_auto_show(Evas_Object *obj);
void                  _elm_win_focus_auto_hide(Evas_Object *obj);

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
EAPI void             _elm_access_widget_item_register(Elm_Widget_Item_Data *item);
EAPI void             _elm_access_widget_item_unregister(Elm_Widget_Item_Data *item);
EAPI void             _elm_access_on_highlight_hook_set(Elm_Access_Info *ac, Elm_Access_On_Highlight_Cb func, void *data);
EAPI void             _elm_access_activate_callback_set(Elm_Access_Info *ac, Elm_Access_Activate_Cb func, void *data);
EAPI void             _elm_access_highlight_object_activate(Evas_Object *obj, Efl_Ui_Activate act);
EAPI void             _elm_access_highlight_cycle(Evas_Object *obj, Elm_Focus_Direction dir);

EINA_DEPRECATED EAPI Elm_Access_Info *_elm_access_object_get(const Evas_Object *obj);

#define ELM_PREFS_DATA_MAGIC 0xe1f5da7a

/**< put this as the first member in your widget item struct */
#define ELM_WIDGET_ITEM       Elm_Widget_Item_Data base

struct _Elm_Widget_Item_Signal_Data
{
   Elm_Object_Item *item;
   Elm_Widget_Item_Signal_Cb func;
   const char *emission;
   const char *source;
   void *data;
};

#define WIDGET_ITEM_DATA_GET(eo_obj) \
    efl_key_data_get((Eo *) eo_obj, "__elm_widget_item_data")

#define WIDGET_ITEM_DATA_SET(eo_obj, data) \
    efl_key_data_set((Eo *) eo_obj, "__elm_widget_item_data", data)

struct _Elm_Widget_Item_Data
{
/* ef1 ~~ efl, el3 ~~ elm */
#define ELM_WIDGET_ITEM_MAGIC 0xef1e1301
   EINA_MAGIC;
/* simple accessor macros */
#define VIEW(X)   X->base->view
#define VIEW_SET(X, V) efl_wref_add(V, &X->base->view)
#define WIDGET(X) X->base->widget
#define EO_OBJ(X) ((X)?X->base->eo_obj:NULL)
   /**< the owner widget that owns this item */
   Evas_Object                   *widget;
   /**< The Eo item, useful to invoke eo_do when only the item data is available */
   Eo                            *eo_obj;
   /**< the base view object */
   Evas_Object                   *view;
   /**< user delete callback function */
   Evas_Smart_Cb                  del_func;
   /**< widget delete callback function. don't expose this callback call */
   Elm_Widget_Del_Pre_Cb          del_pre_func;

   Evas_Object                   *focus_previous, *focus_next;
   Evas_Object                   *focus_up, *focus_down, *focus_right, *focus_left;
   Elm_Object_Item               *item_focus_previous, *item_focus_next;
   Elm_Object_Item               *item_focus_up, *item_focus_down, *item_focus_right, *item_focus_left;
   Eina_Stringshare              *style;

   Evas_Object                   *access_obj;
   const char                    *access_info;
   const char                    *accessible_name;
   Eina_List                     *access_order;
   Eina_Inlist                   *translate_strings;
   Eina_List                     *signals;
   Eina_Hash                     *labels;
   Evas_Object                   *track_obj;
   Eo                            *access_object;

   Eina_Bool                      disabled : 1;
   Eina_Bool                      on_deletion : 1;
   Eina_Bool                      on_translate : 1;
   Eina_Bool                      still_in : 1;
};

#define ELM_NEW(t) calloc(1, sizeof(t))

EAPI Eina_Bool        elm_widget_api_check(int ver);
EAPI Eina_Bool        elm_widget_access(Evas_Object *obj, Eina_Bool is_access);
EAPI Efl_Ui_Theme_Apply_Result  elm_widget_theme(Evas_Object *obj);
EAPI void             elm_widget_theme_specific(Evas_Object *obj, Elm_Theme *th, Eina_Bool force);
EAPI void             elm_widget_on_show_region_hook_set(Evas_Object *obj, void *data, Efl_Ui_Scrollable_On_Show_Region func, Eina_Free_Cb data_free);
EAPI Eina_Bool        elm_widget_sub_object_parent_add(Evas_Object *sobj);
EAPI Eina_Bool        elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj);
EAPI Eina_Bool        elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj);
EAPI void             elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj);
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
EAPI Eina_Bool        elm_widget_highlight_get(const Evas_Object *obj);
EAPI Evas_Object     *elm_widget_top_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_is(const Evas_Object *obj);
EAPI Evas_Object     *elm_widget_parent_widget_get(const Evas_Object *obj);
EAPI void             elm_widget_event_callback_add(Evas_Object *obj, Elm_Event_Cb func, const void *data);
EAPI void            *elm_widget_event_callback_del(Evas_Object *obj, Elm_Event_Cb func, const void *data);
EAPI Eina_Bool        elm_widget_focus_highlight_style_set(Evas_Object *obj, const char *style);
EAPI const char      *elm_widget_focus_highlight_style_get(const Evas_Object *obj);
EAPI void             elm_widget_parent_highlight_set(Evas_Object *obj, Eina_Bool highlighted);
EAPI void             elm_widget_focus_set(Evas_Object *obj, Eina_Bool focus);
EAPI Evas_Object     *elm_widget_parent_get(const Evas_Object *obj);
EAPI void             elm_widget_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode);
EAPI Eina_Bool        elm_widget_focus_highlight_enabled_get(const Evas_Object *obj);
EAPI void             elm_widget_focus_highlight_focus_part_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
Evas_Object          *_elm_widget_focus_highlight_object_get(const Evas_Object *obj);

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

EAPI void             elm_widget_disabled_set(Evas_Object *obj, Eina_Bool disabled);
EAPI Eina_Bool        elm_widget_disabled_get(const Evas_Object *obj);
EAPI void             elm_widget_show_region_set(Evas_Object *obj, Eina_Rect sr, Eina_Bool forceshow);
EAPI Eina_Rect        elm_widget_show_region_get(const Evas_Object *obj);
EAPI Eina_Rect        elm_widget_focus_region_get(const Evas_Object *obj);
EAPI void             elm_widget_focus_region_show(Evas_Object *obj);
EAPI void             elm_widget_scroll_hold_push(Evas_Object *obj);
EAPI void             elm_widget_scroll_hold_pop(Evas_Object *obj);
EAPI int              elm_widget_scroll_hold_get(const Evas_Object *obj);
EAPI void             elm_widget_scroll_freeze_push(Evas_Object *obj);
EAPI void             elm_widget_scroll_freeze_pop(Evas_Object *obj);
EAPI int              elm_widget_scroll_freeze_get(const Evas_Object *obj);
EAPI void             elm_widget_theme_set(Evas_Object *obj, Elm_Theme *th);
EAPI Elm_Theme       *elm_widget_theme_get(const Evas_Object *obj);
EAPI Efl_Ui_Theme_Apply_Result  elm_widget_style_set(Evas_Object *obj, const char *style);
EAPI const char      *elm_widget_style_get(const Evas_Object *obj);
EAPI void             elm_widget_type_set(Evas_Object *obj, const char *type);
EAPI const char      *elm_widget_type_get(const Evas_Object *obj);
EAPI void             elm_widget_tooltip_add(Evas_Object *obj, Elm_Tooltip *tt);
EAPI void             elm_widget_tooltip_del(Evas_Object *obj, Elm_Tooltip *tt);
EAPI void             elm_widget_cursor_add(Evas_Object *obj, Elm_Cursor *cur);
EAPI void             elm_widget_cursor_del(Evas_Object *obj, Elm_Cursor *cur);
EAPI void             elm_widget_scroll_lock_set(Evas_Object *obj, Efl_Ui_Scroll_Block block);
EAPI Efl_Ui_Scroll_Block elm_widget_scroll_lock_get(const Evas_Object *obj);
EAPI int              elm_widget_scroll_child_locked_x_get(const Evas_Object *obj);
EAPI int              elm_widget_scroll_child_locked_y_get(const Evas_Object *obj);
EAPI Efl_Ui_Theme_Apply_Result  elm_widget_theme_object_set(Evas_Object *obj, Evas_Object *edj, const char *wname, const char *welement, const char *wstyle);
EAPI Eina_Bool        elm_widget_type_check(const Evas_Object *obj, const char *type, const char *func);
EAPI Evas_Object     *elm_widget_name_find(const Evas_Object *obj, const char *name, int recurse);
EAPI Eina_List       *elm_widget_stringlist_get(const char *str);
EAPI void             elm_widget_stringlist_free(Eina_List *list);
EAPI void             elm_widget_focus_mouse_up_handle(Evas_Object *obj);
EAPI void             elm_widget_activate(Evas_Object *obj, Efl_Ui_Activate act);
EAPI void             elm_widget_part_text_set(Evas_Object *obj, const char *part, const char *label);
EAPI const char      *elm_widget_part_text_get(const Evas_Object *obj, const char *part);
EAPI const char      *elm_widget_translatable_part_text_get(const Evas_Object *obj, const char *part);
EAPI void             elm_widget_domain_part_text_translatable_set(Evas_Object *obj, const char *part, const char *domain, Eina_Bool translatable);
EAPI void             elm_widget_content_part_set(Evas_Object *obj, const char *part, Evas_Object *content);
EAPI Evas_Object     *elm_widget_content_part_get(const Evas_Object *obj, const char *part);
EAPI Evas_Object     *elm_widget_content_part_unset(Evas_Object *obj, const char *part);
EAPI void             elm_widget_access_info_set(Evas_Object *obj, const char *txt);
EAPI const char      *elm_widget_access_info_get(const Evas_Object *obj);
EAPI Eina_Rect        elm_widget_focus_highlight_geometry_get(const Evas_Object *obj);
void                  _elm_widget_item_highlight_in_theme(Evas_Object *obj, Elm_Object_Item *it);
EAPI void             elm_widget_focus_region_show_mode_set(Evas_Object *obj, Elm_Focus_Region_Show_Mode mode);
EAPI Elm_Focus_Region_Show_Mode elm_widget_focus_region_show_mode_get(const Evas_Object *obj);
const char           *elm_widget_part_translatable_text_get(const Eo *obj, const char *part, const char **domain);
void                  elm_widget_part_translatable_text_set(Eo *obj, const char *part, const char *label, const char *domain);

EAPI Eina_Bool        elm_widget_theme_klass_set(Evas_Object *obj, const char *name);
EAPI const char      *elm_widget_theme_klass_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_theme_element_set(Evas_Object *obj, const char *name);
EAPI const char      *elm_widget_theme_element_get(const Evas_Object *obj);
EAPI Eina_Bool        elm_widget_theme_style_set(Evas_Object *obj, const char *name);
EAPI const char      *elm_widget_theme_style_get(const Evas_Object *obj);
EAPI Efl_Ui_Theme_Apply_Result elm_widget_element_update(Evas_Object *obj, Evas_Object *component, const char *name);

/* debug function. don't use it unless you are tracking parenting issues */
EAPI void             elm_widget_tree_dump(const Evas_Object *top);
EAPI void             elm_widget_tree_dot_dump(const Evas_Object *top, FILE *output);
EAPI Eina_Bool        _elm_widget_onscreen_is(const Evas_Object *widget);
EAPI Eina_Bool        _elm_widget_item_onscreen_is(const Elm_Object_Item *item);
const char*           _elm_widget_accessible_plain_name_get(const Evas_Object *obj, const char* name);
const char*           _elm_widget_item_accessible_plain_name_get(const Elm_Object_Item *item, const char* name);

Efl_Canvas_Object *   _efl_ui_widget_bg_get(const Efl_Ui_Widget *obj);

#define ELM_WIDGET_DATA_GET_OR_RETURN(o, ptr, ...)   \
  Elm_Widget_Smart_Data *ptr;                        \
  ptr = efl_data_scope_get(o, EFL_UI_WIDGET_CLASS);  \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#define ELM_WIDGET_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_WIDGET_CLASS))) \
    return

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
       if ((item)->eo_obj &&                                   \
           efl_isa((item)->eo_obj, ELM_WIDGET_ITEM_CLASS)) break; \
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
       if ((item)->eo_obj &&                                    \
           efl_isa((item)->eo_obj, ELM_WIDGET_ITEM_CLASS)) break; \
       if (!EINA_MAGIC_CHECK(item, ELM_WIDGET_ITEM_MAGIC)) {    \
            EINA_MAGIC_FAIL(item, ELM_WIDGET_ITEM_MAGIC);       \
            goto label;                                         \
         }                                                      \
  } while (0)

static inline Eina_Bool
_elm_widget_sub_object_redirect_to_top(Evas_Object *obj, Evas_Object *sobj)
{
   Eina_Bool ret = elm_widget_sub_object_del(obj, sobj);
   if (!ret) return ret;
   if (elm_widget_is(sobj))
     ret = elm_widget_sub_object_add(elm_widget_top_get(obj), sobj);

   return ret;
}

#define elm_legacy_add(k, p, ...) ({ \
       efl_add(k, p ? p : efl_main_loop_get(), efl_canvas_object_legacy_ctor(efl_added), ##__VA_ARGS__); })

static inline Eo *
elm_widget_resize_object_get(const Eo *obj)
{
   Elm_Widget_Smart_Data *wd = efl_data_scope_safe_get(obj, EFL_UI_WIDGET_CLASS);
   return wd ? wd->resize_obj : NULL;
}

static inline Eina_Bool
elm_widget_is_legacy(const Eo *obj)
{
   return efl_isa(obj, EFL_UI_LEGACY_INTERFACE);
}

/** Takes in any canvas object and returns the first smart parent that is a widget */
static inline Efl_Ui_Widget *
evas_object_widget_parent_find(Evas_Object *o)
{
   while (o && !efl_isa(o, EFL_UI_WIDGET_CLASS))
     evas_object_smart_parent_get(o);
   return o;
}

/* to be used by INTERNAL classes on Elementary, so that the widgets
 * parsing script skips it */
#define ELM_INTERNAL_SMART_SUBCLASS_NEW EVAS_SMART_SUBCLASS_NEW

EAPI Eina_Bool elm_selection_selection_has_owner(Evas_Object *obj);

EAPI Eina_Bool _elm_layout_part_aliasing_eval(const Evas_Object *obj,
                                              const char **part,
                                              Eina_Bool is_text);

/* Internal EO APIs */
const char *efl_ui_widget_default_content_part_get(const Eo *obj);
const char *efl_ui_widget_default_text_part_get(const Eo *obj);


#define ELM_WIDGET_ITEM_PROTECTED
#include "elm_widget_item.eo.h"

#endif
