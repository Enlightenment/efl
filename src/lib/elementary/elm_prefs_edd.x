#ifndef ELM_PREFS_EDD_H
#define ELM_PREFS_EDD_H

#include "elm_prefs.h"

static struct
{
   Elm_Prefs_Item_Type t;
   const char         *name;
} eet_mapping[] = {
   { ELM_PREFS_TYPE_ACTION, "action" },
   { ELM_PREFS_TYPE_BOOL, "boolean" },
   { ELM_PREFS_TYPE_INT, "integer" },
   { ELM_PREFS_TYPE_FLOAT, "float" },
   { ELM_PREFS_TYPE_LABEL, "label" },
   { ELM_PREFS_TYPE_DATE, "date" },
   { ELM_PREFS_TYPE_PAGE, "page" },
   { ELM_PREFS_TYPE_TEXT, "text" },
   { ELM_PREFS_TYPE_TEXTAREA, "textarea" },
   { ELM_PREFS_TYPE_RESET, "reset" },
   { ELM_PREFS_TYPE_SAVE, "save" },
   { ELM_PREFS_TYPE_SEPARATOR, "separator" },
   { ELM_PREFS_TYPE_SWALLOW, "swallow" },
   { ELM_PREFS_TYPE_UNKNOWN, NULL }
};

static Eet_Data_Descriptor *_bool_spec_edd; /* ELM_PREFS_TYPE_BOOL */
static Eet_Data_Descriptor *_int_spec_edd; /* ELM_PREFS_TYPE_INT */
static Eet_Data_Descriptor *_float_spec_edd; /* ELM_PREFS_TYPE_FLOAT */
static Eet_Data_Descriptor *_page_spec_edd; /* ELM_PREFS_TYPE_PAGE */
static Eet_Data_Descriptor *_str_spec_edd; /* ELM_PREFS_TYPE_TEXT and
                                            * ELM_PREFS_TYPE_TEXTAREA, */
static Eet_Data_Descriptor *_date_spec_edd; /* ELM_PREFS_TYPE_DATE */
static Eet_Data_Descriptor *_void_spec_edd; /* ELM_PREFS_TYPE_ACTION,
                                             * ELM_PREFS_TYPE_LABEL,
                                             * ELM_PREFS_TYPE_RESET,
                                             * ELM_PREFS_TYPE_SAVE and
                                             * ELM_PREFS_TYPE_SEPARATOR
                                             * ELM_PREFS_TYPE_SWALLOW */

static Eet_Data_Descriptor *_item_edd;
static Eet_Data_Descriptor *_item_unified_edd;

static Eet_Data_Descriptor *_page_edd;


static const char *
_union_type_get(const void *data,
                Eina_Bool *unknow)
{
   const Elm_Prefs_Item_Type *t = data;
   int i;

   if (unknow)
     *unknow = EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (*t == eet_mapping[i].t)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_TRUE;

   return NULL;
}

static Eina_Bool
_union_type_set(const char *type,
                void *data,
                Eina_Bool unknow)
{
   Elm_Prefs_Item_Type *t = data;
   int i;

   if (unknow)
     return EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(eet_mapping[i].name, type) == 0)
       {
          *t = eet_mapping[i].t;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

#define DESC_NEW(_type, _desc)                             \
  EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, _type); \
  _desc = eet_data_descriptor_stream_new(&eddc)

static Eet_Data_Descriptor *
_bool_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_Bool, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Bool, "def", def, EET_T_UCHAR);

   return ret;
}

static Eet_Data_Descriptor *
_int_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_Int, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Int, "def", def, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Int, "min", min, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Int, "max", max, EET_T_INT);

   return ret;
}

static Eet_Data_Descriptor *
_float_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_Float, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Float, "def", def, EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Float, "min", min, EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Float, "max", max, EET_T_FLOAT);

   return ret;
}

static Eet_Data_Descriptor *
_page_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_Page, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Page, "source", source, EET_T_STRING);

   return ret;
}

static Eet_Data_Descriptor *
_str_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_String, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_String, "def", def, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_String, "placeholder", placeholder,
     EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_String, "accept", accept, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_String, "deny", deny, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_String, "min", length.min, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_String, "max", length.max, EET_T_INT);

   return ret;
}

static Eet_Data_Descriptor *
_date_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_Date, ret);

   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "def.d", def.d, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "def.m", def.m, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "def.y", def.y, EET_T_USHORT);

   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "min.d", min.d, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "min.m", min.m, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "min.y", min.y, EET_T_USHORT);

   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "max.d", max.d, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "max.m", max.m, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (ret, Elm_Prefs_Item_Spec_Date, "max.y", max.y, EET_T_USHORT);

   return ret;
}

static Eet_Data_Descriptor *
_void_spec_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Elm_Prefs_Item_Spec_Void, ret);

   return ret;
}

static void
_elm_prefs_descriptors_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   DESC_NEW(Elm_Prefs_Page_Node, _page_edd);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "version", version, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "title", title, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "sub_title", sub_title, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "widget", widget, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "style", style, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "icon", icon, EET_T_STRING);

   DESC_NEW(Elm_Prefs_Item_Node, _item_edd);

   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "label", label, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "icon", icon, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "style", style, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "widget", widget, EET_T_STRING);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _union_type_get;
   eddc.func.type_set = _union_type_set;
   _item_unified_edd = eet_data_descriptor_stream_new(&eddc);

   _bool_spec_edd = _bool_spec_desc_new();
   _int_spec_edd = _int_spec_desc_new();
   _float_spec_edd = _float_spec_desc_new();
   _page_spec_edd = _page_spec_desc_new();
   _str_spec_edd = _str_spec_desc_new();
   _date_spec_edd = _date_spec_desc_new();
   _void_spec_edd = _void_spec_desc_new();

   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "action", _void_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "boolean", _bool_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "integer", _int_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "float", _float_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "label", _void_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "page", _page_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "text", _str_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "textarea", _str_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "date", _date_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "reset", _void_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "save", _void_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "separator", _void_spec_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "swallow", _void_spec_edd);

   EET_DATA_DESCRIPTOR_ADD_UNION(
     _item_edd, Elm_Prefs_Item_Node, "spec", spec, type, _item_unified_edd);

   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "persistent", persistent, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "editable", editable, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Elm_Prefs_Item_Node, "visible", visible, EET_T_UCHAR);

   EET_DATA_DESCRIPTOR_ADD_LIST
     (_page_edd, Elm_Prefs_Page_Node, "items", items, _item_edd);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_page_edd, Elm_Prefs_Page_Node, "autosave", autosave, EET_T_UCHAR);
}

#undef DESC_NEW

static void
_elm_prefs_descriptors_shutdown(void)
{
   eet_data_descriptor_free(_bool_spec_edd);
   eet_data_descriptor_free(_int_spec_edd);
   eet_data_descriptor_free(_float_spec_edd);
   eet_data_descriptor_free(_page_spec_edd);
   eet_data_descriptor_free(_str_spec_edd);
   eet_data_descriptor_free(_date_spec_edd);
   eet_data_descriptor_free(_void_spec_edd);
   eet_data_descriptor_free(_page_edd);
   eet_data_descriptor_free(_item_edd);
   eet_data_descriptor_free(_item_unified_edd);
}

#endif
