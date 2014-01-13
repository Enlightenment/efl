#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#include "atspi/atspi-constants.h"

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
    "table column header"
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

#define ARG_GET(valist, type) va_arg(*valist, type)

static Elm_Atspi_Object * _elm_atspi_factory_construct(Evas_Object *obj, Elm_Atspi_Object *);

extern Eina_List *_elm_win_list;

typedef struct _Atspi_Object_Data Atspi_Object_Data;

struct _Atspi_Object_Data
{
   const char *name;
   const char *description;
   AtspiRole role;
};

EAPI Eo_Op ELM_ATSPI_OBJ_BASE_ID = EO_NOOP;

static void
_description_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = ARG_GET(list, const char **);
   Atspi_Object_Data *ad = _pd;

   *ret = ad->description;
}

static void
_description_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *desc = ARG_GET(list, const char *);
   Atspi_Object_Data *ad = _pd;

   if (ad->description)
     eina_stringshare_del(ad->description);

   ad->description = eina_stringshare_add(desc);
}

static void
_parent_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Atspi_Object **ret = ARG_GET(list, Elm_Atspi_Object**);
   eo_do(obj, eo_parent_get(ret));
}

static void
_name_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   const char **ret = ARG_GET(list, const char **);
   Atspi_Object_Data *ad = _pd;

   *ret = ad->name;
}

static void
_name_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   const char *name = ARG_GET(list, const char *);
   Atspi_Object_Data *ad = _pd;

   if (ad->name)
     eina_stringshare_del(ad->name);

   ad->name = eina_stringshare_add(name);
}

static void
_role_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   AtspiRole *ret = ARG_GET(list, int*);
   Atspi_Object_Data *ad = _pd;

   *ret = ad->role;
}

static void
_role_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   AtspiRole role = ARG_GET(list, int);
   Atspi_Object_Data *ad = _pd;

   ad->role = role;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, ELM_ATSPI_CLASS, eo_constructor());
   Atspi_Object_Data *ad = _pd;
   ad->name = ad->description = NULL;
}

static void
_destructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Atspi_Object_Data *ad = _pd;

   if (ad->name) eina_stringshare_del(ad->name);
   if (ad->description) eina_stringshare_del(ad->description);

   eo_do_super(obj, ELM_ATSPI_CLASS, eo_destructor());
}

static void
_role_name_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   const char **ret = ARG_GET(list, const char **);
   Atspi_Object_Data *ad = _pd;

   *ret = Atspi_Name[ad->role];
}

static void
_localized_role_name_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   const char **ret = ARG_GET(list, const char **);
   Atspi_Object_Data *ad = _pd;

#ifdef ENABLE_NLS
   *ret = gettext(Atspi_Name[ad->role]);
#else
   *ret = Atspi_Name[ad->role];
#endif
}

static void
_child_at_index_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   int idx = ARG_GET(list, int);
   Elm_Atspi_Object **ao = ARG_GET(list, Elm_Atspi_Object**);
   Eina_List *children = NULL;
   eo_do(obj, elm_atspi_obj_children_get(&children));

   if (children)
     *ao = eina_list_nth(children, idx);
   else
     *ao = NULL;

   eina_list_free(children);
}

static void
_index_in_parent_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   unsigned int *idx = ARG_GET(list, unsigned int*);
   Elm_Atspi_Object *chld, *parent = NULL;
   Eina_List *l, *children = NULL;
   unsigned int tmp = 0;
   eo_do(obj, elm_atspi_obj_parent_get(&parent));

   if (parent)
     {
        eo_do(parent, elm_atspi_obj_children_get(&children));
        EINA_LIST_FOREACH(children, l, chld)
          {
             if (chld == obj)
               {
                  *idx = tmp;
                  break;
               }
             tmp++;
          }
        if (tmp == eina_list_count(children))
             ERR("Access object not present in parent's children list!");
        EINA_LIST_FREE(children, chld)
           eo_unref(chld);

        eo_unref(parent);
     }
   else
     DBG("Access Object has no parent.");
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_NAME_GET), _name_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_NAME_SET), _name_set),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_DESCRIPTION_GET), _description_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_DESCRIPTION_SET), _description_set),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_PARENT_GET), _parent_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_CHILD_AT_INDEX_GET), _child_at_index_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_INDEX_IN_PARENT_GET), _index_in_parent_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_GET), _role_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_SET), _role_set),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_NAME_GET), _role_name_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_LOCALIZED_ROLE_NAME_GET), _localized_role_name_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_NAME_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_NAME_SET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_DESCRIPTION_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_DESCRIPTION_SET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_PARENT_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_CHILD_AT_INDEX_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_CHILDREN_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_INDEX_IN_PARENT_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_RELATION_SET_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_RELATION_SET_SET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_ROLE_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_ROLE_SET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_ROLE_NAME_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_LOCALIZED_ROLE_NAME_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_STATE_GET, ""),
     EO_OP_DESCRIPTION(ELM_ATSPI_OBJ_SUB_ID_ATTRIBUTES_GET, ""),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Access_Object",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_ATSPI_OBJ_BASE_ID, op_desc, ELM_ATSPI_OBJ_SUB_ID_LAST),
     NULL,
     sizeof(Atspi_Object_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_obj_class_get, &class_desc, EO_BASE_CLASS, NULL);

// Component interface
EAPI Eo_Op ELM_ATSPI_COMPONENT_INTERFACE_BASE_ID = EO_NOOP;

static void
_comp_interface_position_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   int *x = ARG_GET(list, int*);
   int *y = ARG_GET(list, int*);
   AtspiCoordType type = ARG_GET(list, AtspiCoordType);

   eo_do(obj, elm_atspi_component_interface_extents_get(x, y, NULL, NULL, type));
}

static void
_comp_interface_position_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   int x = ARG_GET(list, int);
   int y = ARG_GET(list, int);
   AtspiCoordType type = ARG_GET(list, AtspiCoordType);
   Eina_Bool *ret = ARG_GET(list, Eina_Bool*);
   int c_w, c_h;

   eo_do(obj, elm_atspi_component_interface_extents_get(NULL, NULL, &c_w, &c_h, type));
   eo_do(obj, elm_atspi_component_interface_extents_set(x, y, c_w, c_h, type, ret));
}

static void
_comp_interface_size_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   int w = ARG_GET(list, int);
   int h = ARG_GET(list, int);
   Eina_Bool *ret = ARG_GET(list, Eina_Bool*);
   int c_x, c_y;

   eo_do(obj, elm_atspi_component_interface_extents_get(&c_x, &c_y, NULL, NULL, ATSPI_COORD_TYPE_WINDOW));
   eo_do(obj, elm_atspi_component_interface_extents_set(c_x, c_y, w, h, ATSPI_COORD_TYPE_WINDOW, ret));
}

static void
_comp_interface_size_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   int *w = ARG_GET(list, int*);
   int *h = ARG_GET(list, int*);

   eo_do(obj, elm_atspi_component_interface_extents_get(NULL,  NULL, w, h, ATSPI_COORD_TYPE_WINDOW));
}

static void
_comp_interface_contains(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   int x = ARG_GET(list, int);
   int y = ARG_GET(list, int);
   AtspiCoordType type = ARG_GET(list, AtspiCoordType);
   Eina_Bool *ret = ARG_GET(list, Eina_Bool*);
   int w_x, w_y, w_w, w_h;

   *ret = EINA_FALSE;

   if (!eo_do(obj, elm_atspi_component_interface_extents_get(&w_x, &w_y, &w_w, &w_h, type)))
      return;

   if ((x >= w_x) && (x <= w_x + w_w) && (y >= w_y) && (y <= w_y + w_h))
     *ret = EINA_TRUE;
}

static void
_component_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, ELM_ATSPI_COMPONENT_INTERFACE, eo_constructor());
}

static void
_component_interface_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _component_constructor),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_GET), _comp_interface_position_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_SET), _comp_interface_position_set),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_CONTAINS), _comp_interface_contains),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_GET), _comp_interface_size_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_SET), _comp_interface_size_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}


static const Eo_Op_Description component_op_desc[] = {
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_CONTAINS, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ACCESSIBLE_AT_POINT_GET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_GET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_SET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_GET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_SET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_GET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_SET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_LAYER_GET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_Z_ORDER_GET, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_FOCUS_GRAB, ""),
   EO_OP_DESCRIPTION(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ALPHA_GET, ""),
   EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description component_interface_desc = {
     EO_VERSION,
     "Elm_Atspi_Component_Interface",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(&ELM_ATSPI_COMPONENT_INTERFACE_BASE_ID, component_op_desc, ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_LAST),
     NULL,
     0,
     _component_interface_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_component_interface_get, &component_interface_desc, NULL, NULL);

/// Elm_Atspi_Widget base class
#define ELM_ATSPI_WIDGET_CLASS elm_atspi_widget_obj_class_get()

const Eo_Class *elm_atspi_widget_obj_class_get(void) EINA_CONST;

typedef struct _Access_Widget_Object_Data Access_Widget_Object_Data;

struct _Access_Widget_Object_Data
{
     Evas_Object *obj;
};

static void
_del_ao_obj(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Object *obj = data;
   Access_Widget_Object_Data *ad = eo_data_scope_get(obj, ELM_ATSPI_WIDGET_CLASS);
   ad->obj = NULL;
   // below will cause Accessibility object destruction while evas object is destroyed and access object has none extra refs
   eo_unref(obj);
}

static void
_widget_constructor(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *widget = ARG_GET(list, Evas_Object*);
   Access_Widget_Object_Data *ad = _pd;
   const char *name;

   eo_do_super(obj, ELM_ATSPI_WIDGET_CLASS, eo_constructor());
   name = evas_object_type_get(widget);

   evas_object_event_callback_add(widget, EVAS_CALLBACK_DEL, _del_ao_obj, obj);

   ad->obj = widget;
   eo_do(obj, elm_atspi_obj_name_set(name));
   eo_do(obj, elm_atspi_obj_role_set(ATSPI_ROLE_UNKNOWN));
}

static void
_widget_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Access_Widget_Object_Data *ad = _pd;
   if (ad->obj)
     evas_object_event_callback_del(ad->obj, EVAS_CALLBACK_DEL, _del_ao_obj);

   eo_do_super(obj, ELM_ATSPI_WIDGET_CLASS, eo_destructor());
}

static void
_widget_children_get(Eo *obj, void *_pd, va_list *list)
{
   Eina_List **ret = ARG_GET(list, Eina_List**);
   Access_Widget_Object_Data *ad = _pd;
   Eina_List *l, *al = NULL;
   Evas_Object *sub;
   Elm_Atspi_Object *ao;
   Elm_Widget_Smart_Data *sd;

   EINA_SAFETY_ON_NULL_GOTO(ad->obj, fail);

   sd = eo_data_scope_get(ad->obj, ELM_OBJ_WIDGET_CLASS);
   EINA_SAFETY_ON_NULL_GOTO(sd, fail);

   EINA_LIST_FOREACH(sd->subobjs, l, sub) {
      if (!sub) continue;
      ao = _elm_atspi_factory_construct(sub, obj);
      if (ao)
         al = eina_list_append(al, ao);
   }

   *ret = al;
   return;

fail:
  *ret = NULL;
  return;
}


static void
_widget_state_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // FIXME
}


static void
_widget_comp_access_at_point_get(Eo *obj, void *_pd, va_list *list)
{
   int x = ARG_GET(list, int);
   int y = ARG_GET(list, int);
   AtspiCoordType type = ARG_GET(list, AtspiCoordType);
   Elm_Atspi_Object **ret = ARG_GET(list, Elm_Atspi_Object**);
   int ee_x, ee_y;
   Access_Widget_Object_Data *ad = _pd;
   Eina_List *l, *objs;
   Evas_Object *wid;

   if (ad->obj)
     {
      if (type == ATSPI_COORD_TYPE_SCREEN)
        {
           Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(ad->obj));
           ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
           x -= ee_x;
           y -= ee_y;
        }
        objs = evas_objects_at_xy_get(evas_object_evas_get(ad->obj), x, y, EINA_TRUE, EINA_TRUE);
        EINA_LIST_FOREACH(objs, l, wid)
          {
             // return first only
             if (elm_object_widget_check(wid))
               {
                  *ret = _elm_atspi_factory_construct(wid, obj);
                  break;
               }
          }
        eina_list_free(objs);
     }
}

static void
_widget_comp_extents_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Access_Widget_Object_Data *ad = _pd;
   int *x, *y, *w, *h;
   int ee_x, ee_y;
   x = ARG_GET(list, int*);
   y = ARG_GET(list, int*);
   w = ARG_GET(list, int*);
   h = ARG_GET(list, int*);
   AtspiCoordType type = ARG_GET(list, AtspiCoordType);

   if (ad->obj)
     {
       evas_object_geometry_get(ad->obj, x, y, w, h);
       if (type == ATSPI_COORD_TYPE_SCREEN)
         {
            Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(ad->obj));
            ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
            if (x) *x += ee_x;
            if (y) *y += ee_y;
         }
     }
}

static void
_widget_comp_extents_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Access_Widget_Object_Data *ad = _pd;
   int x, y, w, h;
   int wx, wy;
   x = ARG_GET(list, int);
   y = ARG_GET(list, int);
   w = ARG_GET(list, int);
   h = ARG_GET(list, int);
   AtspiCoordType type = ARG_GET(list, AtspiCoordType);
   Eina_Bool *ret = ARG_GET(list, Eina_Bool*);

   *ret = EINA_FALSE;

   if ((x < 0) || (y < 0) || (w < 0) || (h < 0)) return;

   if (ad->obj)
     {
        if (type == ATSPI_COORD_TYPE_SCREEN)
          {
             Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(ad->obj));
             evas_object_geometry_get(ad->obj, &wx, &wy, NULL, NULL);
             ecore_evas_move(ee, x - wx, y - wy);
          }
        else
          evas_object_move(ad->obj, x, y);

        evas_object_resize(ad->obj, w, h);
        *ret = EINA_TRUE;
     }
}


static void
_widget_comp_layer_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *l = ARG_GET(list, int *);
   Access_Widget_Object_Data *ad = _pd;

   if (ad->obj)
     *l = evas_object_layer_get(ad->obj);
}

static void
_widget_comp_z_order_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // FIXME
}

static void
_widget_comp_focus_grab(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = ARG_GET(list, Eina_Bool*);
   Access_Widget_Object_Data *ad = _pd;

   *ret = EINA_FALSE;

   if (ad->obj && elm_object_focus_allow_get(ad->obj))
     {
       Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(ad->obj));

       ecore_evas_activate(ee);
       elm_object_focus_set(ad->obj, EINA_TRUE);
       *ret = EINA_TRUE;
     }
}

static void
_widget_comp_alpha_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = ARG_GET(list, double*);
   int alpha;
   Access_Widget_Object_Data *ad = _pd;

   if (ad->obj)
     {
        evas_object_color_get(ad->obj, NULL, NULL, NULL, &alpha);
        *ret = (double)alpha/255.0;
     }
}

static void
_widget_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _widget_constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _widget_destructor),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_CHILDREN_GET), _widget_children_get),
        EO_OP_FUNC(ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_STATE_GET), _widget_state_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ACCESSIBLE_AT_POINT_GET), _widget_comp_access_at_point_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_GET), _widget_comp_extents_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_SET), _widget_comp_extents_set),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_LAYER_GET), _widget_comp_layer_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_Z_ORDER_GET), _widget_comp_z_order_get),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_FOCUS_GRAB), _widget_comp_focus_grab),
        EO_OP_FUNC(ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ALPHA_GET), _widget_comp_alpha_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description widget_class_desc = {
     EO_VERSION,
     "Elm_Widget Access_Object",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Access_Widget_Object_Data),
     _widget_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_widget_obj_class_get, &widget_class_desc, ELM_ATSPI_CLASS, ELM_ATSPI_COMPONENT_INTERFACE, NULL);

static Elm_Atspi_Object *
_elm_atspi_factory_construct(Evas_Object *obj, Elm_Atspi_Object *parent)
{
   Elm_Atspi_Object *ret;
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   if (!elm_object_widget_check(obj))
      return NULL;

   if (!strcmp(evas_object_type_get(obj), "elm_access"))
     return NULL;

   ret = evas_object_data_get(obj, "_atspi_object");
   if (!ret)
     {
        ret = eo_add(ELM_ATSPI_WIDGET_CLASS, parent, obj);
        evas_object_data_set(obj, "_atspi_object", ret);
        eo_unref(ret); // only parent should hold reference to atspi object
     }
   else
     {
        Elm_Atspi_Object *tmp;
        eo_do(ret, eo_parent_get(&tmp));
        if (!tmp)
          eo_do(ret, eo_parent_set(parent));
     }

   return ret;
}

/// Elm_Atspi_App base class
const Eo_Class *elm_atspi_app_obj_class_get(void) EINA_CONST;
#define ELM_ATSPI_APP_CLASS elm_atspi_app_obj_class_get()

static void
_app_children_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_List **ret = ARG_GET(list, Eina_List**);
   Eina_List *l, *objs = NULL;
   Evas_Object *win;
   Elm_Atspi_Object *o = NULL;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
       if (!win) continue;
       o = _elm_atspi_factory_construct(win, obj);
       if (o)
         objs = eina_list_append(objs, o);
     }
   *ret = objs;
}

static void
_app_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, ELM_ATSPI_APP_CLASS, eo_constructor());

   eo_do(obj, elm_atspi_obj_name_set(elm_app_name_get()));
   eo_do(obj, elm_atspi_obj_role_set(ATSPI_ROLE_APPLICATION));
}

static void
_app_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _app_constructor),
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
     "App Access_Object",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _app_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_atspi_app_obj_class_get, &app_class_desc, ELM_ATSPI_CLASS, NULL);

Elm_Atspi_Object * elm_atspi_root_object_get(void)
{
   static Elm_Atspi_Object *app;

   if (!app)
     app = eo_add(ELM_ATSPI_APP_CLASS, NULL);
   else
     eo_ref(app);

   return app;
}
