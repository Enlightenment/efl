#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"
#include "assert.h"

#include "atspi/atspi-constants.h"

static Elm_Atspi_Object *_app;
static Eina_List *_global_callbacks;

typedef struct _Elm_Atspi_Global_Callback_Info Elm_Atspi_Global_Callback_Info;

struct _Elm_Atspi_Global_Callback_Info
{
   Eo_Event_Cb cb;
   void *user_data;
};

const char* Atspi_Name[] = {
    "invalid",
    "accelerator label",
    "alert",
    "animation",
    "arrow",
    "calendar",
    "canvas",
    "check box",
    "check menu item",
    "color chooser",
    "column header",
    "combo box",
    "dateeditor",
    "desktop icon",
    "desktop frame",
    "dial",
    "dialog",
    "directory pane",
    "drawing area",
    "file chooser",
    "filler",
    "focus traversable",
    "font chooser",
    "frame",
    "glass pane",
    "html container",
    "icon",
    "image",
    "internal frame",
    "label",
    "layered pane",
    "list",
    "list item",
    "menu",
    "menu bar",
    "menu item",
    "option pane",
    "page tab",
    "page tab list",
    "panel",
    "password text",
    "popup menu",
    "progress bar",
    "push button",
    "radio button",
    "radio menu item",
    "root pane",
    "row header",
    "scroll bar",
    "scroll pane",
    "separator",
    "slider",
    "spin button",
    "split pane",
    "status bar",
    "table",
    "table cell",
    "table column header",
    "table row header",
    "tearoff menu item",
    "terminal",
    "text",
    "toggle button",
    "tool bar",
    "tool tip",
    "tree",
    "tree table",
    "unknown",
    "viewport",
    "window",
    "extended",
    "header",
    "footer",
    "paragraph",
    "ruler",
    "application",
    "autocomplete",
    "editbar",
    "embedded",
    "entry",
    "chart",
    "caption",
    "document frame",
    "heading",
    "page",
    "section",
    "redundant object",
    "form",
    "link",
    "input method window",
    "table row",
    "tree item",
    "document spreadsheet",
    "document presentation",
    "document text",
    "document web",
    "document email",
    "comment",
    "list box",
    "grouping",
    "image map",
    "notification",
    "info bar",
    "last defined"
};

extern Eina_List *_elm_win_list;

static void
_eo_emit_state_changed_event(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Object *ao = data;
   int event_data[2] = {ATSPI_STATE_VISIBLE, 1};
   eo_do(ao, eo_event_callback_call(ELM_ATSPI_OBJECT_EVENT_STATE_CHANGED, &event_data[0], NULL));
}

EOLIAN static void
_elm_atspi_object_eo_base_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *internal_obj = NULL;
   eo_do_super(obj, ELM_ATSPI_OBJ_CLASS, eo_constructor());

   eo_do(obj, eo_parent_get(&internal_obj));

   /* Evas_Object can only hold refs to atspi-object */
   assert(eo_isa(internal_obj, EVAS_OBJ_SMART_CLASS));

   evas_object_data_set(internal_obj, "_atspi_object", obj);
   evas_object_event_callback_add(internal_obj, EVAS_CALLBACK_SHOW, _eo_emit_state_changed_event, obj);
}

EOLIAN static Elm_Atspi_Object *
_elm_atspi_object_child_at_index_get(Eo *obj, void *_pd EINA_UNUSED, int idx)
{
   Elm_Atspi_Object *ao = NULL;
   Eina_List *children = NULL;

   eo_do(obj, elm_atspi_obj_children_get(&children));
   if (!children) return NULL;

   ao = eina_list_nth(children, idx);
   eina_list_free(children);
   return ao;
}

EOLIAN static Evas_Object *
_elm_atspi_object_object_get(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *ret = NULL;
   eo_do(obj, eo_parent_get(&ret));
   return ret;
}

EOLIAN static unsigned int
_elm_atspi_object_index_in_parent_get(Eo *obj, void *_pd EINA_UNUSED)
{
   unsigned int idx;
   Elm_Atspi_Object *chld, *parent = NULL;
   Eina_List *l, *children = NULL;
   unsigned int tmp = 0;

   eo_do(obj, elm_atspi_obj_parent_get(&parent));
   if (!parent) return 0;
   eo_do(parent, elm_atspi_obj_children_get(&children));

   EINA_LIST_FOREACH(children, l, chld)
     {
        if (chld == obj)
          {
             idx = tmp;
             break;
          }
        tmp++;
     }
   if (tmp == eina_list_count(children))
      ERR("Access object not present in parent's children list!");

   eina_list_free(children);

   return idx;
}

EOLIAN static const char*
_elm_atspi_object_role_name_get(Eo *obj, void *_pd EINA_UNUSED)
{
   AtspiRole role = ATSPI_ROLE_INVALID;

   eo_do(obj, elm_atspi_obj_role_get(&role));

   if (role >= ATSPI_ROLE_LAST_DEFINED)
     {
        ERR("Invalid role enum for atspi-object: %d.", role);
        return NULL;
     }

   return Atspi_Name[role];
}

EOLIAN static const char*
_elm_atspi_object_description_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static const char*
_elm_atspi_object_localized_role_name_get(Eo *obj, void *_pd EINA_UNUSED)
{
   const char *name = NULL;

   eo_do(obj, elm_atspi_obj_role_name_get(&name));
   if (!name) return NULL;
#ifdef ENABLE_NLS
   return gettext(name);
#else
   return name;
#endif
}

EOLIAN static Elm_Atspi_State
_elm_atspi_object_state_get(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *evobj = NULL;
   Elm_Atspi_State states = 0;
   eo_do(obj, elm_atspi_obj_object_get(&evobj));

   if (evas_object_visible_get(evobj))
     BIT_FLAG_SET(states, ATSPI_STATE_VISIBLE);

   return states;
}

EOLIAN static Evas_Object *
_elm_atspi_object_elm_interface_atspi_component_accessible_at_point_get(Eo *obj, void *_pd EINA_UNUSED, int x, int y, AtspiCoordType type)
{
   int ee_x, ee_y;
   Eina_List *l, *objs;
   Evas_Object *evobj = NULL;
   Evas_Object *ret;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));

   if (!evobj) return NULL;
   if (type == ATSPI_COORD_TYPE_SCREEN)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(evobj));
		if (!ee) return NULL;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        x -= ee_x;
        y -= ee_y;
     }
     objs = evas_objects_at_xy_get(evas_object_evas_get(evobj), x, y, EINA_TRUE, EINA_TRUE);
     EINA_LIST_FOREACH(objs, l, evobj)
       {
          // return first only, test if there is atspi interface for eo
          Elm_Atspi_Object *acc = _elm_atspi_factory_construct(evobj);
          if (acc)
            {
               ret = evobj;
               break;
            }
       }
     eina_list_free(objs);
     return ret;
}

EOLIAN static void
_elm_atspi_object_elm_interface_atspi_component_extents_get(Eo *obj, void *_pd EINA_UNUSED, int *x, int *y, int *w, int *h, AtspiCoordType type)
{
   int ee_x, ee_y;
   Evas_Object *evobj = NULL;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));
   if (!evobj) return;

   evas_object_geometry_get(evobj, x, y, w, h);
   if (type == ATSPI_COORD_TYPE_SCREEN)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(evobj));
		if (!ee) return;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        if (x) *x += ee_x;
        if (y) *y += ee_y;
     }
}

EOLIAN static Eina_Bool
_elm_atspi_object_elm_interface_atspi_component_extents_set(Eo *obj, void *_pd EINA_UNUSED, int x, int y, int w, int h, AtspiCoordType type)
{
   int wx, wy;
   Evas_Object *evobj = NULL;

   if ((x < 0) || (y < 0) || (w < 0) || (h < 0)) return EINA_FALSE;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));
   if (!evobj) return EINA_FALSE;

   if (type == ATSPI_COORD_TYPE_SCREEN)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(evobj));
		if (!ee) return EINA_FALSE;
        evas_object_geometry_get(evobj, &wx, &wy, NULL, NULL);
        ecore_evas_move(ee, x - wx, y - wy);
     }
   else
     evas_object_move(evobj, x, y);

   evas_object_resize(evobj, w, h);
   return EINA_TRUE;
}

EOLIAN static int
_elm_atspi_object_elm_interface_atspi_component_layer_get(Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Object_Layer layer;
   Evas_Object *evobj = NULL;
   AtspiComponentLayer spi_layer;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));
   if (!evobj) return 0;

   layer = evas_object_layer_get(evobj);
   switch (layer) {
    case ELM_OBJECT_LAYER_BACKGROUND:
       spi_layer = ATSPI_LAYER_BACKGROUND;
       break;
    case ELM_OBJECT_LAYER_FOCUS:
    case ELM_OBJECT_LAYER_TOOLTIP:
    case ELM_OBJECT_LAYER_CURSOR:
       spi_layer = ATSPI_LAYER_OVERLAY;
       break;
    default:
       spi_layer = ATSPI_LAYER_WIDGET;
   }
   return spi_layer;
}

EOLIAN static int
_elm_atspi_object_elm_interface_atspi_component_z_order_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   // FIXME
   return 0;
}

EOLIAN static Eina_Bool
_elm_atspi_object_eo_base_event_callback_call(Eo *obj, void *_pd EINA_UNUSED,
      const Eo_Event_Description *desc, const void *event_info)
{
   Elm_Atspi_Global_Callback_Info *info;
   Eina_List *l;
   Eina_Bool ret;

   EINA_LIST_FOREACH(_global_callbacks, l, info)
     {
        if (info->cb) info->cb(info->user_data, obj, desc, (void *)event_info);
     }

   eo_do_super(obj, ELM_ATSPI_OBJ_CLASS, eo_event_callback_call(desc, event_info, &ret));
   return ret;
}

EOLIAN static double
_elm_atspi_object_elm_interface_atspi_component_alpha_get(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *evobj = NULL;
   int alpha;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));
   if (!evobj) return 0.0;

   evas_object_color_get(evobj, NULL, NULL, NULL, &alpha);
   return (double)alpha / 255.0;
}


#include "elm_interface_atspi_window.eo.c"

static void
_emit_atspi_state_changed_focused_event(void *data, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Object *ao = data;
   int evdata[2] = {ATSPI_STATE_FOCUSED, 1};
   eo_do(ao, eo_event_callback_call(ELM_ATSPI_OBJECT_EVENT_STATE_CHANGED, &evdata[0], NULL));
}

static void
_emit_atspi_state_changed_unfocused_event(void *data, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Object *ao = data;
   int evdata[2] = {ATSPI_STATE_FOCUSED, 0};
   eo_do(ao, eo_event_callback_call(ELM_ATSPI_OBJECT_EVENT_STATE_CHANGED, &evdata[0], NULL));
}

static void
_widget_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *internal_obj = NULL;
   eo_do_super(obj, ELM_ATSPI_WIDGET_CLASS, eo_constructor());

   eo_do(obj, eo_parent_get(&internal_obj));

   /* Evas_Object can only hold refs to atspi-object */
   assert(eo_isa(internal_obj, ELM_OBJ_WIDGET_CLASS));

   evas_object_smart_callback_add(internal_obj, "focused", _emit_atspi_state_changed_focused_event, obj);
   evas_object_smart_callback_add(internal_obj, "unfocused", _emit_atspi_state_changed_unfocused_event, obj);
}

static void
_widget_name_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(const char **, ret, list);
   Evas_Object *widget = NULL;
   const char *name = NULL;

   eo_do(obj, elm_atspi_obj_object_get(&widget));
   name = elm_object_text_get(widget);
   if (ret) *ret = name;
}

static void
_widget_role_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(AtspiRole*, ret, list);
   const char *type;
   AtspiRole role;
   Evas_Object *widget = NULL;
   eo_do(obj, elm_atspi_obj_object_get(&widget));
   if (!widget) return;
   type = evas_object_type_get(widget);

   // FIXME make it hash or cast some first bytes to int.
   if (!strcmp(type, "elm_win"))
     role = ATSPI_ROLE_WINDOW;
   else if (!strcmp(type, "elm_button"))
     role = ATSPI_ROLE_PUSH_BUTTON;
   else
     role = ATSPI_ROLE_UNKNOWN;

   if (ret) *ret = role;
}

static void
_widget_parent_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(Elm_Atspi_Object **, ret, list);
   Evas_Object *widget = NULL;
   Elm_Atspi_Object *parent;

   eo_do(obj, elm_atspi_obj_object_get(&widget));
   widget = elm_object_parent_widget_get(widget);

   if (widget)
     parent = _elm_atspi_factory_construct(widget);
   else // if parent is not found, attach it to atspi root object.
     parent = _elm_atspi_root_object_get();

   if (ret) *ret = parent;
}

static void
_widget_children_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(Eina_List **, ret, list);
   Evas_Object *widget = NULL;
   Eina_List *l, *accs = NULL;
   Elm_Widget_Smart_Data *sd;
   Elm_Atspi_Object *aobj;

   eo_do(obj, elm_atspi_obj_object_get(&widget));

   sd = eo_data_scope_get(widget, ELM_OBJ_WIDGET_CLASS);
   if (!sd) return;

   EINA_LIST_FOREACH(sd->subobjs, l, widget)
     {
        if (!elm_object_widget_check(widget)) continue;
        aobj = _elm_atspi_factory_construct(widget);
        if (aobj)
          accs = eina_list_append(accs, aobj);
     }
   if (ret)
     *ret = accs;
   else
     eina_list_free(accs);
}

static void
_widget_state_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(Elm_Atspi_State *, ret, list);
   Evas_Object *widget;
   Elm_Atspi_State states;
   eo_do(obj, elm_atspi_obj_object_get(&widget));

   eo_do_super(obj, ELM_ATSPI_WIDGET_CLASS, elm_atspi_obj_state_get(&states));

   if (elm_object_focus_get(widget))
     BIT_FLAG_SET(states, ATSPI_STATE_FOCUSED);
   if (elm_object_focus_allow_get(widget))
     BIT_FLAG_SET(states, ATSPI_STATE_FOCUSABLE);
   if (!elm_object_disabled_get(widget))
     BIT_FLAG_SET(states, ATSPI_STATE_ENABLED);

   if (ret) *ret = states;
}

static void
_widget_comp_focus_grab(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(Eina_Bool*, ret, list);
   Evas_Object *evobj = NULL;
   if (ret) *ret = EINA_FALSE;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));
   EINA_SAFETY_ON_NULL_RETURN(evobj);
   if (elm_object_focus_allow_get(evobj))
     {
       Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(evobj));
	   if (!ee) return;
       ecore_evas_activate(ee);
       elm_object_focus_set(evobj, EINA_TRUE);
       if (ret) *ret = EINA_TRUE;
     }
}

static void
_widget_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _widget_constructor),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_NAME_GET), _widget_name_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_GET), _widget_role_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_PARENT_GET), _widget_parent_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_CHILDREN_GET), _widget_children_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_STATE_GET), _widget_state_get),
        EO_OP_FUNC(ELM_INTERFACE_ATSPI_COMPONENT_ID(ELM_INTERFACE_ATSPI_COMPONENT_SUB_ID_FOCUS_GRAB), _widget_comp_focus_grab),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description widget_class_desc = {
     EO_VERSION,
     "Elm_Widget_Access_Object",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _widget_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_widget_obj_class_get, &widget_class_desc, ELM_ATSPI_OBJ_CLASS, NULL);


/// Elm_Atspi_App base class
#define ELM_ATSPI_APP_CLASS elm_atspi_app_obj_class_get()

static void
_app_children_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(Eina_List **, ret, list);
   Eina_List *l, *accs = NULL;
   Elm_Atspi_Object *aobj;
   Evas_Object *win;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        if (!win) continue;
        aobj = _elm_atspi_factory_construct(win);
        if (aobj)
          accs = eina_list_append(accs, aobj);
     }

   if (ret) *ret = accs;
}

static void
_app_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, ELM_ATSPI_OBJ_CLASS, eo_constructor());
}

static void
_app_name_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   EO_PARAMETER_GET(const char **, name, list);
   if (name) *name = elm_app_name_get();
}

static void
_app_role_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   EO_PARAMETER_GET(AtspiRole *, ret, list);
   if (ret) *ret = ATSPI_ROLE_APPLICATION;
}

static void
_app_parent_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   EO_PARAMETER_GET(Elm_Atspi_Object **, ret, list);
   if (ret) *ret = NULL;
}

static void
_app_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _app_constructor),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_NAME_GET), _app_name_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_GET), _app_role_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_PARENT_GET), _app_parent_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_CHILDREN_GET), _app_children_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description app_op_desc[] = {
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description app_class_desc = {
     EO_VERSION,
     "Elm_App_Access_Object",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _app_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_app_obj_class_get, &app_class_desc, ELM_ATSPI_OBJ_CLASS, NULL);

// elm_win wrapper

const Eo_Class *elm_atspi_win_obj_class_get(void) EINA_CONST;
#define ELM_ATSPI_WIN_CLASS elm_atspi_win_obj_class_get()

static void
_win_focused(void *data, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Object *ao = data;
   eo_do(ao, eo_event_callback_call(ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_ACTIVATED, NULL, NULL));
}

static void
_win_unfocused(void *data, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Object *ao = data;
   eo_do(ao, eo_event_callback_call(ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DEACTIVATED, NULL, NULL));
}

static void
_win_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, ELM_ATSPI_WIN_CLASS, eo_constructor());
   Evas_Object *evobj = NULL;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));

   evas_object_smart_callback_add(evobj, "focused", _win_focused, obj);
   evas_object_smart_callback_add(evobj, "unfocused", _win_unfocused, obj);
}

static void
_win_destructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_Atspi_Object *root = _elm_atspi_root_object_get();
   eo_do(root, eo_event_callback_call(ELM_ATSPI_OBJECT_EVENT_CHILD_REMOVED, obj, NULL));

   eo_do_super(obj, ELM_ATSPI_WIN_CLASS, eo_destructor());
}

static void
_win_name_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   EO_PARAMETER_GET(const char **, ret, list);
   Evas_Object *evobj = NULL;

   eo_do(obj, elm_atspi_obj_object_get(&evobj));
   EINA_SAFETY_ON_NULL_RETURN(evobj);

   if (ret) *ret = elm_win_title_get(evobj);
}

static void
_win_parent_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   EO_PARAMETER_GET(Elm_Atspi_Object **, ret, list);
   if (ret) *ret = _elm_atspi_root_object_get();
}

static void
_win_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _win_constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _win_destructor),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_NAME_GET), _win_name_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_PARENT_GET), _win_parent_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description win_class_desc = {
     EO_VERSION,
     "Elm_Win_Access_Object",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _win_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_win_obj_class_get, &win_class_desc, ELM_ATSPI_WIDGET_CLASS, ELM_INTERFACE_ATSPI_WINDOW_CLASS, NULL);

Elm_Atspi_Object*
_elm_atspi_root_object_get(void)
{
   if (!_app)
     _app = eo_add(ELM_ATSPI_APP_CLASS, NULL);
   else
     eo_ref(_app);

   return _app;
}

Elm_Atspi_Object *
_elm_atspi_factory_construct(Evas_Object *obj)
{
   Elm_Atspi_Object *ret;
   const char *type;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   ret = evas_object_data_get(obj, "_atspi_object");
   if (ret) return ret;

   type = evas_object_type_get(obj);

   //FIXME add possibility to install new handlers
   if (!strcmp(type, "elm_access"))
     {
        DBG("Unable to create ATSPI object for elm_access widget.");
        return NULL;
     }
   else if (!strcmp(type, "elm_win"))
     ret = eo_add(ELM_ATSPI_WIN_CLASS, obj);
   else if (!strncmp(type, "elm_", 4)) // defaults to implementation for elm_widget class.
     ret = eo_add(ELM_ATSPI_WIDGET_CLASS, obj);

   if (ret) eo_unref(ret); // only evas_object should hold reference to atspi object

   return ret;
}

void _elm_atspi_object_init(void)
{
}

void _elm_atspi_object_shutdown(void)
{
   Elm_Atspi_Global_Callback_Info *info;
   if (_app)
      eo_del(_app);

   EINA_LIST_FREE(_global_callbacks, info)
      free(info);
   _global_callbacks = NULL;
}

void _elm_atspi_object_global_callback_add(Eo_Event_Cb cb, void *user_data)
{
   Elm_Atspi_Global_Callback_Info *info = calloc(1, sizeof(Elm_Atspi_Global_Callback_Info));
   if (!info) return;
   info->user_data = user_data;
   info->cb = cb;
   _global_callbacks = eina_list_append(_global_callbacks, info);
}

void _elm_atspi_object_global_callback_del(Eo_Event_Cb cb)
{
   Elm_Atspi_Global_Callback_Info *info;
   Eina_List *l;
   EINA_LIST_FOREACH(_global_callbacks, l, info)
     {
        if ((info->cb == cb))
          {
             _global_callbacks = eina_list_remove(_global_callbacks, info);
             free(info);
          }
     }
}

#include "elm_atspi_object.eo.c"
