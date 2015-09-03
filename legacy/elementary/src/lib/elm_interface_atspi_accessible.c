#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

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

struct _Elm_Atspi_Event_Handler
{
   Eo_Event_Cb cb;
   void *data;
};

static Eina_List *global_callbacks;

EOLIAN static int
_elm_interface_atspi_accessible_index_in_parent_get(Eo *obj, void *pd EINA_UNUSED)
{
   Eina_List *l, *children = NULL;
   Eo *chld, *parent = NULL;
   int ret = 0;

   eo_do(obj, parent = elm_interface_atspi_accessible_parent_get());
   if (!parent) return -1;

   eo_do(parent, children = elm_interface_atspi_accessible_children_get());
   if (!children) return -1;

   EINA_LIST_FOREACH(children, l, chld)
     {
       if (obj == chld)
         break;
       ret++;
     }
   if (ret == (int)eina_list_count(children))
     {
        ERR("Object %s not present in its AT-SPI parents (%s) children list! This should never happen.", eo_class_name_get(eo_class_get(obj)), eo_class_name_get(eo_class_get(parent)));
        ret = -1;
     }
   eina_list_free(children);
   return ret;
}

EOLIAN static Eo *
_elm_interface_atspi_accessible_parent_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Eo *parent = NULL;

   /* By default using Eo_Base object hierarchy */
   eo_do(obj, parent = eo_parent_get());
   if (!parent) return NULL;

   return eo_isa(parent, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN) ? parent : NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_parent_set(Eo *obj, void *priv EINA_UNUSED, Eo *new_parent EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_parent_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN Eina_List*
_elm_interface_atspi_accessible_attributes_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_attributes_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static Elm_Atspi_Role
_elm_interface_atspi_accessible_role_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return ELM_ATSPI_ROLE_UNKNOWN;
}

EOLIAN static void
_elm_interface_atspi_accessible_role_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Elm_Atspi_Role role EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_role_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN const char *
_elm_interface_atspi_accessible_role_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Elm_Atspi_Role role;

   eo_do(obj, role = elm_interface_atspi_accessible_role_get());

   return role > ELM_ATSPI_ROLE_LAST_DEFINED ? "" : Atspi_Name[role];
}

EOLIAN char *
_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_name_get\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_name_set(Eo *obj, void *pd EINA_UNUSED, char *val EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_name_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

const char * _elm_interface_atspi_accessible_description_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_description_get\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_description_set(Eo *obj, void *pd EINA_UNUSED, const char *val EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_description_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN static const char *
_elm_interface_atspi_accessible_localized_role_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   const char *ret = NULL;
   eo_do(obj, ret = elm_interface_atspi_accessible_role_name_get());
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

EOLIAN static Eina_List *
_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Eina_List *children = NULL;
   Eina_Iterator *iter = NULL;
   Eo *chld;

   // By default use Eo_Base object hierarchy
   eo_do(obj, iter = eo_children_iterator_new());
   if (!iter) return NULL;

   EINA_ITERATOR_FOREACH(iter, chld)
     {
        if (eo_isa(chld, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
          children = eina_list_append(children, chld);
     }
   eina_iterator_free(iter);

   return children;
}

EOLIAN static Elm_Atspi_State_Set
_elm_interface_atspi_accessible_state_set_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Elm_Atspi_State_Set ret = 0;
   return ret;
}

EOLIAN Eina_List*
_elm_interface_atspi_accessible_relation_set_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_relation_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EAPI void elm_atspi_attributes_list_free(Eina_List *list)
{
   Elm_Atspi_Attribute *attr;
   EINA_LIST_FREE(list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}

EOLIAN void
_elm_interface_atspi_accessible_event_emit(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Eo *accessible, const Eo_Event_Description *event, void *event_info)
{
   Eina_List *l;
   Elm_Atspi_Event_Handler *hdl;

   if (!accessible || !event || !eo_isa(accessible, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
     {
        CRI("Invalid accessibility event emit parameters");
        return;
     }

   EINA_LIST_FOREACH(global_callbacks, l, hdl)
     {
        if (hdl->cb)
          hdl->cb(hdl->data, accessible, event, event_info);
     }
}

EOLIAN Elm_Atspi_Event_Handler *
_elm_interface_atspi_accessible_event_handler_add(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Eo_Event_Cb cb, void *data)
{
   Elm_Atspi_Event_Handler *ret = calloc(sizeof(Elm_Atspi_Event_Handler), 1);

   ret->cb = cb;
   ret->data = data;

   global_callbacks = eina_list_append(global_callbacks, ret);

   return ret;
}

EOLIAN void 
_elm_interface_atspi_accessible_event_handler_del(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Elm_Atspi_Event_Handler *handler)
{
   Eina_List *l, *l2;
   Elm_Atspi_Event_Handler *hdl;
   EINA_LIST_FOREACH_SAFE(global_callbacks, l, l2, hdl)
     {
        if (hdl == handler)
          {
             global_callbacks = eina_list_remove_list(global_callbacks, l);
             free(hdl);
             break;
          }
     }
}

#include "elm_interface_atspi_accessible.eo.c"
