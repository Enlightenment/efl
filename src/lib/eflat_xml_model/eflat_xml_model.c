#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eflat_xml_model_private.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS EFLAT_XML_MODEL_CLASS
#define MY_CLASS_NAME "Eflat_Xml_Model"

typedef enum
{
   EFLAT_XML_MODEL_PATH_TYPE_DATA,
   EFLAT_XML_MODEL_PATH_TYPE_ATTRIBUTE,
   EFLAT_XML_MODEL_PATH_TYPE_PROCESSING,
   EFLAT_XML_MODEL_PATH_TYPE_RAW_XML
} Eflat_Xml_Model_Path_Type;

typedef struct
{
  Eina_Simple_XML_Node_Tag           *node;
  Eflat_Xml_Model_Path_Type           type;
  union
  {
     Eina_Simple_XML_Attribute       *attribute;
     Eina_Simple_XML_Node_Processing *processing;
  };
} Eflat_Xml_Model_Path_Data;

static void _properties_load(Eflat_Xml_Model_Data *);
static void _children_load(Eflat_Xml_Model_Data *);
static bool _init(Eflat_Xml_Model_Data *);
static void _clear(Eflat_Xml_Model_Data *);
static void _properties_setup(Eflat_Xml_Model_Data *);
static Eina_Simple_XML_Node_Tag *_tag_find(Eina_Simple_XML_Node_Tag *, const char *);
static Eina_Simple_XML_Attribute *_attribute_find(Eina_Simple_XML_Node_Tag *, const char *);
static Eina_Simple_XML_Node_Tag *_node_from_path_new(Eina_Simple_XML_Node_Root *, const char *, Eflat_Xml_Model_Path_Data *);
static Eina_Simple_XML_Node_Tag *_node_from_path_get(Eina_Simple_XML_Node_Root *, const char *, Eflat_Xml_Model_Path_Data *);
static void _node_children_del(Eina_Simple_XML_Node_Tag *);
static char *_path_from_node_get(Eina_Simple_XML_Node_Tag *);
static char *_concatenate_path(const char *, const char *, char);
static void _node_children_properties_del(Eflat_Xml_Model_Data *, Eina_Simple_XML_Node_Tag *, const char *);
static void _node_attributes_properties_del(Eflat_Xml_Model_Data *, Eina_Simple_XML_Node_Tag *, const char *);
static void _property_del(Eflat_Xml_Model_Data *, const char *);
static void _property_add(Eflat_Xml_Model_Data *, const char *, const char *);
static void _node_clear(Eflat_Xml_Model_Data *, Eina_Simple_XML_Node_Tag *);
static void _generate_properties_from_node(Eflat_Xml_Model_Data *, Eina_Simple_XML_Node_Tag *);
static Eina_Simple_XML_Node_Data *_data_value_set(Eina_Simple_XML_Node_Tag *, Eina_Simple_XML_Node_Data *, const char *);
static char *_data_name_get(const Eina_Simple_XML_Node_Data *);
static void _stream_load(Eflat_Xml_Model_Data *, Eina_Simple_XML_Node_Tag *, const char *);

static int _eflat_xml_model_init_count = 0;
int _eflat_xml_model_log_dom = -1;

EAPI int
eflat_xml_model_init(void)
{
   if (_eflat_xml_model_init_count++ > 0)
     return _eflat_xml_model_init_count;

   if (!eina_init())
     {
        fputs("Eflat_Xml_Model: Unable to initialize eina\n", stderr);
        return 0;
     }

   _eflat_xml_model_log_dom = eina_log_domain_register("eflat_xml_model",
                                                       EINA_COLOR_CYAN);
   if (_eflat_xml_model_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'eflat_xml_model' log domain");
        _eflat_xml_model_log_dom = -1;
        eina_shutdown();
        return 0;
     }

   if (!ecore_init())
     {
        ERR("Unable to initialize ecore");
        eina_log_domain_unregister(_eflat_xml_model_log_dom);
        _eflat_xml_model_log_dom = -1;
        eina_shutdown();
        return 0;
     }

   // ...

   return _eflat_xml_model_init_count;
}

EAPI int
eflat_xml_model_shutdown(void)
{
   if (_eflat_xml_model_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _eflat_xml_model_init_count = 0;
        return 0;
     }

   if (--_eflat_xml_model_init_count)
     return _eflat_xml_model_init_count;

   // ...

   ecore_shutdown();
   eina_log_domain_unregister(_eflat_xml_model_log_dom);
   _eflat_xml_model_log_dom = -1;
   eina_shutdown();
   return 0;
}

static void
_eflat_xml_model_hash_free(Eina_Value *value)
{
   eina_value_free(value);
}

static Eo_Base *
_eflat_xml_model_eo_base_constructor(Eo *obj, Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   pd->properties_array = NULL;
   pd->properties_hash = eina_hash_string_superfast_new(EINA_FREE_CB(_eflat_xml_model_hash_free));
   pd->root = NULL;
   pd->xml = NULL;
   eina_value_setup(&pd->xml_value, EINA_VALUE_TYPE_STRING);

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_eflat_xml_model_constructor(Eo *obj EINA_UNUSED,
                             Eflat_Xml_Model_Data *pd EINA_UNUSED,
                             const char *xml)
{
   DBG("(%p)", obj);
   pd->xml = strdup(xml ? xml : "");
}

static void
_eflat_xml_model_eo_base_destructor(Eo *obj, Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);
   eina_hash_free(pd->properties_hash);
   free(pd->xml);
   eina_value_flush(&pd->xml_value);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                               Eflat_Xml_Model_Data *pd,
                                               Eina_Array * const* properties_array)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, EFL_MODEL_LOAD_STATUS_ERROR);

   *(Eina_Array**)properties_array = pd->properties_array;
   return pd->load.status;
}

static void
_eflat_xml_model_efl_model_base_properties_load(Eo *obj, Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   if (!_init(pd))
     return;

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);

   _properties_load(pd);

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}

static void
_properties_load(Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", pd->obj);

   _properties_setup(pd);

   if (!pd->xml)
     return;

   _generate_properties_from_node(pd, pd->root);
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_property_set(Eo *obj,
                                             Eflat_Xml_Model_Data *pd,
                                             const char *property,
                                             const Eina_Value *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   Eflat_Xml_Model_Path_Data data;
   Eina_Simple_XML_Node_Tag *node = _node_from_path_new(pd->root,
                                                        property,
                                                        &data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(node, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p)", obj);

   char *value_str = eina_value_to_string(value);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value_str, EFL_MODEL_LOAD_STATUS_ERROR);

   switch (data.type)
     {
      case EFLAT_XML_MODEL_PATH_TYPE_ATTRIBUTE:
        {
           eina_stringshare_del(data.attribute->value);
           data.attribute->value = eina_stringshare_add(value_str);
           break;
        }
      case EFLAT_XML_MODEL_PATH_TYPE_RAW_XML:
        {
           _stream_load(pd, node, value_str);
           break;
        }
      case EFLAT_XML_MODEL_PATH_TYPE_DATA:
        {
           _node_clear(pd, node);

           // checks if it's a empty tag
           size_t len = strlen(value_str);
           if (len)
             eina_simple_xml_node_data_new(node, value_str, len);
           break;
        }
      case EFLAT_XML_MODEL_PATH_TYPE_PROCESSING:
        {
           Eina_Simple_XML_Node_Data *processing = _data_value_set
             (node, data.processing, value_str);
           EINA_SAFETY_ON_NULL_GOTO(processing, on_error);
           break;
        }
     }

   if (data.type != EFLAT_XML_MODEL_PATH_TYPE_RAW_XML)
     {
        // TODO: Check if the property exists instead of deleting it
        _property_del(pd, property);
        _property_add(pd, property, value_str);
     }

   free(value_str);

   efl_model_property_changed_notify(obj, property);

   return pd->load.status;

on_error:
   free(value_str);
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_property_get(Eo *obj EINA_UNUSED,
                                             Eflat_Xml_Model_Data *pd,
                                             const char *property,
                                             const Eina_Value **value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   Eflat_Xml_Model_Path_Data data;
   Eina_Simple_XML_Node_Tag *node = _node_from_path_get(pd->root,
                                                        property,
                                                        &data);
   if (!node)
     {
        ERR("Property doesn't exists: %s", property);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     };

   if (data.type == EFLAT_XML_MODEL_PATH_TYPE_RAW_XML)
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        EINA_SAFETY_ON_NULL_RETURN_VAL(buf, EFL_MODEL_LOAD_STATUS_ERROR);

        Eina_Simple_XML_Node *child;
        EINA_INLIST_FOREACH(node->children, child)
          {
             char *dump = eina_simple_xml_node_dump(child, "  ");
             Eina_Bool ret = eina_strbuf_append(buf, dump);
             free(dump);
             EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EFL_MODEL_LOAD_STATUS_ERROR);
          }

        Eina_Bool ret = eina_value_set(&pd->xml_value, eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EFL_MODEL_LOAD_STATUS_ERROR);

        *value = &pd->xml_value;
        return pd->load.status;
     }

   *value = eina_hash_find(pd->properties_hash, property);
   if (!(*value))
     {
        ERR("Property doesn't exist: %s", property);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   return pd->load.status;
}

static void
_eflat_xml_model_efl_model_base_load(Eo *obj, Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);

   if ((pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED) == EFL_MODEL_LOAD_STATUS_LOADED)
     return;

   if (!_init(pd))
     return;

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     {
        efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);
        _properties_load(pd);
     }

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     {
        efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);
        _children_load(pd);
     }

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED);
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_load_status_get(Eo *obj EINA_UNUSED,
                                                Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);
   return pd->load.status;
}

static void
_eflat_xml_model_efl_model_base_unload(Eo *obj, Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

Eo *
_eflat_xml_model_efl_model_base_child_add(Eo *obj EINA_UNUSED,
                                          Eflat_Xml_Model_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
   return NULL;
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_child_del(Eo *obj EINA_UNUSED,
                                          Eflat_Xml_Model_Data *pd EINA_UNUSED,
                                          Eo *child EINA_UNUSED)
{
   DBG("(%p)", obj);
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                                   Eflat_Xml_Model_Data *pd,
                                                   unsigned start EINA_UNUSED,
                                                   unsigned count EINA_UNUSED,
                                                   Eina_Accessor **children_accessor)
{
   DBG("(%p)", obj);
   *children_accessor = NULL;
   return pd->load.status;
}

static Efl_Model_Load_Status
_eflat_xml_model_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                                   Eflat_Xml_Model_Data *pd,
                                                   unsigned *children_count)
{
   DBG("(%p)", obj);
   *children_count = 0;
   return pd->load.status;
}

static void
_eflat_xml_model_efl_model_base_children_load(Eo *obj, Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   if (!_init(pd))
     return;

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);

   _children_load(pd);

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);
}

static void
_children_load(Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", pd->obj);
}

static bool
_init(Eflat_Xml_Model_Data *pd)
{
   if (!pd->root)
     pd->root = eina_simple_xml_node_load(pd->xml, strlen(pd->xml), EINA_TRUE);
   return true;
}

static void
_clear(Eflat_Xml_Model_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   eina_hash_free_buckets(pd->properties_hash);

   if (pd->properties_array)
     {
        size_t i;
        char *property;
        Eina_Array_Iterator it;
        EINA_ARRAY_ITER_NEXT(pd->properties_array, i, property, it)
          free(property);
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   eina_simple_xml_node_root_free(pd->root);
   pd->root = NULL;

   free(pd->xml);
   pd->xml = strdup("");
   eina_value_set(&pd->xml_value, "");
}

static void
_properties_setup(Eflat_Xml_Model_Data *pd)
{
   DBG("(%p)", pd->obj);

   pd->properties_array = eina_array_new(1);
}

static Eina_Simple_XML_Node_Tag *
_tag_find(Eina_Simple_XML_Node_Tag *node, const char *name)
{
   if (!node) return NULL;

   Eina_Simple_XML_Node *child;
   EINA_INLIST_FOREACH(node->children, child)
     {
        if (EINA_SIMPLE_XML_NODE_TAG == child->type)
          {
             Eina_Simple_XML_Node_Tag *tag = (Eina_Simple_XML_Node_Tag*)child;
             if (strcmp(name, tag->name) == 0)
               return tag;
          }
     }

   return NULL;
}

static bool
_space_is(char c)
{
   return '\x20' == c || '\x9' == c || '\xD' == c || '\xA' == c;
}

static char *
_data_name_get(const Eina_Simple_XML_Node_Data *data)
{
   if (!data) return NULL;

   size_t name_length = data->length;

   const char *separator = NULL;
   const char *pos = data->data;
   const char *end = pos + name_length;
   while (pos < end)
     {
        if (_space_is(*pos))
          {
             separator = pos;
             break;
          }
        ++pos;
     }
   if (separator)
     name_length = (size_t)(ptrdiff_t)(separator - data->data);

   char *name = malloc(name_length + 1);
   strncpy(name, data->data, name_length);
   name[name_length] = '\0';
   return name;
}

static Eina_Simple_XML_Node_Data *
_eflat_xml_model_data_find(Eina_Simple_XML_Node_Tag *node,
                           const char *name,
                           Eina_Simple_XML_Node_Type type)
{
   if (!node) return NULL;

   const size_t name_length = strlen(name);

   Eina_Simple_XML_Node *child;
   EINA_INLIST_FOREACH(node->children, child)
     {
        if (type != child->type)
          continue;

        Eina_Simple_XML_Node_Data *data = (Eina_Simple_XML_Node_Data*)child;
        if (name_length > data->length)
          continue;

        if (strncmp(data->data, name, name_length) != 0)
          continue;

        if ((data->length > name_length) && !_space_is(data->data[name_length]))
          continue;

        return data;
     }

   return NULL;
}

static Eina_Simple_XML_Attribute *
_attribute_find(Eina_Simple_XML_Node_Tag *node, const char *key)
{
   if (!node) return NULL;

   Eina_Simple_XML_Attribute *attr;
   EINA_INLIST_FOREACH(node->attributes, attr)
     {
        if (strcmp(key, attr->key) == 0)
          return attr;
     }

   return NULL;
}

static Eina_Simple_XML_Node_Tag *
_node_new(Eina_Simple_XML_Node_Tag *parent, const char *name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Eina_Simple_XML_Node_Tag *child = _tag_find(parent, name);
   if (!child)
     child = eina_simple_xml_node_tag_new(parent, name);

   return child;
}

static Eina_Simple_XML_Attribute *
_attr_new(Eina_Simple_XML_Node_Tag *parent, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Eina_Simple_XML_Attribute *attr = _attribute_find(parent, key);
   if (!attr)
     attr = eina_simple_xml_attribute_new(parent, key, "");

   return attr;
}

static Eina_Simple_XML_Node_Data *
_data_new(Eina_Simple_XML_Node_Tag *parent,
          const char *name,
          Eina_Simple_XML_Node_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Eina_Simple_XML_Node_Data *data = _eflat_xml_model_data_find(parent, name, type);
   if (data)
     return data;

   switch (type)
     {
      case EINA_SIMPLE_XML_NODE_PROCESSING:
        return eina_simple_xml_node_processing_new(parent, name, strlen(name));
      default:
        ERR("Node type not supported: %d", type);
        return NULL;
     }
}

static const char *
_next_node_name(char **it, char c)
{
   char *separator = strchr(*it, c);
   if (!separator) return NULL;

   *separator = '\0';
   char *node_name = *it;
   *it = separator + 1;
   return node_name;
}

typedef struct
{
   Eina_Simple_XML_Node_Tag   *(*node_find)(Eina_Simple_XML_Node_Tag *, const char *);
   Eina_Simple_XML_Attribute  *(*attr_find)(Eina_Simple_XML_Node_Tag *, const char *);
   Eina_Simple_XML_Node_Data  *(*data_find)(Eina_Simple_XML_Node_Tag *, const char *, Eina_Simple_XML_Node_Type);
} Eflat_Xml_Model_Path_Operators;

static const Eflat_Xml_Model_Path_Operators
_const_path_operators = {
   .node_find = _tag_find,
   .attr_find = _attribute_find,
   .data_find = _eflat_xml_model_data_find,
};

static const Eflat_Xml_Model_Path_Operators
_new_path_operators = {
   .node_find = _node_new,
   .attr_find = _attr_new,
   .data_find = _data_new,
};

static Eina_Simple_XML_Node_Tag *
_node_path(Eina_Simple_XML_Node_Root *root,
           const char *path,
           Eflat_Xml_Model_Path_Data *data,
           Eflat_Xml_Model_Path_Operators ops)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(root, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   *data = (Eflat_Xml_Model_Path_Data){0};
   Eina_Simple_XML_Node_Tag *node = (Eina_Simple_XML_Node_Tag*)root;

   char *path_tmp = strdup(path);
   char *it = path_tmp;

   const char *node_name;
   while ((node_name = _next_node_name(&it, '/')))
     {
        if (strlen(node_name))
          {
             node = ops.node_find(node, node_name);
             if (!node) goto on_error;
          }
     }

   if ((node_name = _next_node_name(&it, '@')))
     {
        if (!strlen(node_name)) goto on_error; // attribute cannot be assigned to root

        node = ops.node_find(node, node_name);
        if (!node) goto on_error;

        char *attr_key = it;
        if (strlen(attr_key) == 0)
          {
             ERR("Attributes cannot have null key: %s", path);
             goto on_error;
          }

        data->attribute = ops.attr_find(node, attr_key);
        if (!data->attribute) goto on_error;
        data->type = EFLAT_XML_MODEL_PATH_TYPE_ATTRIBUTE;
     }
   else
   if ((node_name = _next_node_name(&it, '?')))
     {
        if (strlen(node_name))
          {
             node = ops.node_find(node, node_name);
             if (!node) goto on_error;
          }

        char *processing_key = it;
        if (strlen(processing_key) == 0)
          {
             ERR("Processing tags cannot have null name: %s", path);
             goto on_error;
          }

        data->processing = (Eina_Simple_XML_Node_Processing*)
          ops.data_find(node, processing_key, EINA_SIMPLE_XML_NODE_PROCESSING);
        if (!data->processing) goto on_error;
        data->type = EFLAT_XML_MODEL_PATH_TYPE_PROCESSING;
     }
   else
     {
        node_name = it;
        if (strlen(node_name) == 0) // terminating with '/'
          data->type = EFLAT_XML_MODEL_PATH_TYPE_RAW_XML;
        else
          {
             node = ops.node_find(node, node_name);
             if (!node) goto on_error;
             data->type = EFLAT_XML_MODEL_PATH_TYPE_DATA;
          }
     }

   free(path_tmp);
   return node;

on_error:
   free(path_tmp);
   return NULL;
}

static Eina_Simple_XML_Node_Tag *
_node_from_path_new(Eina_Simple_XML_Node_Root *root,
                    const char *path,
                    Eflat_Xml_Model_Path_Data *data)
{
   return _node_path(root, path, data, _new_path_operators);
}

static Eina_Simple_XML_Node_Tag *
_node_from_path_get(Eina_Simple_XML_Node_Root *root,
                    const char *path,
                    Eflat_Xml_Model_Path_Data *data)
{
   return _node_path(root, path, data, _const_path_operators);
}

static void
_node_children_del(Eina_Simple_XML_Node_Tag *tag)
{
   while (tag->children)
     {
        Eina_Simple_XML_Node *node = EINA_INLIST_CONTAINER_GET(tag->children,
                                                               Eina_Simple_XML_Node);
        if (EINA_SIMPLE_XML_NODE_TAG == node->type)
          eina_simple_xml_node_tag_free((Eina_Simple_XML_Node_Tag*)node);
        else
          eina_simple_xml_node_data_free((Eina_Simple_XML_Node_Data*)node);
     }
}

static char *
_path_from_node_get(Eina_Simple_XML_Node_Tag *tag)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tag, NULL);

   Eina_Strbuf *buf = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, NULL);

   Eina_Bool ret = eina_strbuf_append(buf, tag->name ? tag->name : "");
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   Eina_Simple_XML_Node_Tag *parent = tag;
   while ((parent = parent->base.parent) && parent->name)
     {
        ret = eina_strbuf_prepend(buf, "/")
           && eina_strbuf_prepend(buf, parent->name);
        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
     }

   char *result = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return result;

on_error:
   eina_strbuf_free(buf);
   return NULL;
}

static char *
_concatenate_path(const char *path, const char *child, char separator)
{
   int len = strlen(path) + sizeof(separator) + strlen(child) + 1;

   char *child_path = malloc(len);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_path, NULL);

   int written = snprintf(child_path, len, "%s%c%s", path, separator, child);
   EINA_SAFETY_ON_FALSE_GOTO((written > 0) && (written < len), on_error);

   return child_path;

on_error:
   free(child_path);
   return NULL;
}

static void
_node_children_properties_del(Eflat_Xml_Model_Data *pd,
                              Eina_Simple_XML_Node_Tag *tag,
                              const char *path)
{
   DBG("(%p)", pd->obj);
   Eina_Simple_XML_Node *child;
   EINA_INLIST_FOREACH(tag->children, child)
     {
        switch (child->type)
          {
           case EINA_SIMPLE_XML_NODE_TAG:
             {
                Eina_Simple_XML_Node_Tag *child_tag = (Eina_Simple_XML_Node_Tag*)child;

                char *child_path = _concatenate_path(path, child_tag->name, '/');
                EINA_SAFETY_ON_NULL_RETURN(child_path);

                _property_del(pd, child_path);
                _node_children_properties_del(pd, child_tag, child_path);
                _node_attributes_properties_del(pd, child_tag, child_path);

                free(child_path);
                break;
             }
           default:
             break;
          }
     }
}

static void
_node_attributes_properties_del(Eflat_Xml_Model_Data *pd,
                                Eina_Simple_XML_Node_Tag *tag,
                                const char *path)
{
   Eina_Simple_XML_Attribute *attr;
   EINA_INLIST_FOREACH(tag->attributes, attr)
     {
        char *attr_path = _concatenate_path(path, attr->key, '@');
        _property_del(pd, attr_path);
        free(attr_path);
     }
}

static Eina_Bool
_property_array_keep_cb(void *data, void *gdata)
{
   char *a = (char *)data;
   const char *b = (char *)gdata;

   if (strcmp(a, b) != 0) return EINA_TRUE;

   free(a);
   return EINA_FALSE;
}

static void
_property_del(Eflat_Xml_Model_Data *pd, const char *property)
{
   DBG("(%p) deleting property %s", pd->obj, property);
   eina_hash_del(pd->properties_hash, property, NULL);

   eina_array_remove(pd->properties_array,
                     _property_array_keep_cb,
                     (void*)property);
}

static void
_property_add(Eflat_Xml_Model_Data *pd, const char *property, const char *value)
{
   EINA_SAFETY_ON_NULL_RETURN(property);
   EINA_SAFETY_ON_NULL_RETURN(value);

   DBG("(%p) adding property %s=%s", pd->obj, property, value);

   char *property_copy = strdup(property);
   EINA_SAFETY_ON_NULL_RETURN(property_copy);

   Eina_Bool ret = eina_array_push(pd->properties_array, property_copy);
   EINA_SAFETY_ON_FALSE_RETURN(ret);

   Eina_Value *prop_value = eina_hash_find(pd->properties_hash, property);
   if (!prop_value)
     {
        prop_value = eina_value_new(EINA_VALUE_TYPE_STRING);
        EINA_SAFETY_ON_NULL_RETURN(prop_value);

        ret = eina_hash_add(pd->properties_hash, property, prop_value);
        EINA_SAFETY_ON_FALSE_RETURN(ret);
     }

   ret = eina_value_set(prop_value, value);
   EINA_SAFETY_ON_FALSE_RETURN(ret);
}

static void
_node_clear(Eflat_Xml_Model_Data *pd, Eina_Simple_XML_Node_Tag *node)
{
   DBG("(%p)", pd->obj);
   EINA_SAFETY_ON_NULL_RETURN(node);
   char *path = _path_from_node_get(node);
   EINA_SAFETY_ON_NULL_RETURN(path);

   DBG("(%p)", pd->obj);
   _node_children_properties_del(pd, node, path);
   free(path);

   _node_children_del(node);
}

static void
_generate_properties_from_node(Eflat_Xml_Model_Data *pd,
                               Eina_Simple_XML_Node_Tag *node)
{
   char *path = _path_from_node_get(node);

   Eina_Simple_XML_Attribute *attr;
   EINA_INLIST_FOREACH(node->attributes, attr)
     {
        char *attr_path = _concatenate_path(path, attr->key, '@');
        _property_add(pd, attr_path, attr->value);
        free(attr_path);
     }

   size_t lines_count = 0;
   Eina_Strbuf *buf = eina_strbuf_new();

   Eina_Simple_XML_Node *child;
   EINA_INLIST_FOREACH(node->children, child)
     {
        switch (child->type)
          {
           case EINA_SIMPLE_XML_NODE_TAG:
             _generate_properties_from_node(pd, (Eina_Simple_XML_Node_Tag*)child);
             break;
           case EINA_SIMPLE_XML_NODE_DATA:
             {
                Eina_Simple_XML_Node_Data *data = (Eina_Simple_XML_Node_Data*)child;
                if (!data->length) break;
                eina_strbuf_append_length(buf, data->data, data->length);
                if (lines_count > 0)
                  eina_strbuf_append(buf, "\n");
                ++lines_count;
                break;
             }
           case EINA_SIMPLE_XML_NODE_PROCESSING:
             {
                const Eina_Simple_XML_Node_Processing *data = (Eina_Simple_XML_Node_Processing*)child;

                char *name = _data_name_get(data);
                char *processing_path = _concatenate_path(path, name, '?');

                size_t name_length = strlen(name);
                if (data->length == name_length)
                  _property_add(pd, processing_path, "");
                else
                  _property_add(pd, processing_path, data->data + name_length + 1);

                free(processing_path);
                free(name);
                break;
             }
           default:
             ERR("Node type not supported: %d", child->type);
             break;
          }
     }

  if (lines_count)
    _property_add(pd, path, eina_strbuf_string_get(buf));

  eina_strbuf_free(buf);
  free(path);
}

static Eina_Simple_XML_Node_Data *
_data_value_set(Eina_Simple_XML_Node_Tag *node,
                Eina_Simple_XML_Node_Data *data,
                const char *value)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, NULL);

   char *name = _data_name_get(data);
   EINA_SAFETY_ON_NULL_GOTO(name, on_error_1);

   Eina_Bool ret = eina_strbuf_append_printf(buf, "%s %s", name, value);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error_2);

   Eina_Simple_XML_Node_Data *processing = eina_simple_xml_node_processing_new
     (node, eina_strbuf_string_get(buf), eina_strbuf_length_get(buf));

   eina_simple_xml_node_processing_free(data);
   free(name);
   eina_strbuf_free(buf);
   return processing;

on_error_2:
   free(name);
on_error_1:
   eina_strbuf_free(buf);
   return NULL;
}

static void
_stream_load(Eflat_Xml_Model_Data *pd,
             Eina_Simple_XML_Node_Tag *node,
             const char *xml)
{
   _node_clear(pd, node);

   Eina_Simple_XML_Node_Root *root = eina_simple_xml_node_load(xml,
                                                               strlen(xml),
                                                               EINA_TRUE);
   // steals root items
   Eina_Simple_XML_Node *child;
   EINA_INLIST_FOREACH(root->children, child)
     child->parent = node;
   node->children = root->children;
   root->children = NULL;

   Eina_Simple_XML_Attribute *attr;
   EINA_INLIST_FOREACH(root->attributes, attr)
     attr->parent = node;
   node->attributes = root->attributes;
   root->attributes = NULL;

   eina_simple_xml_node_root_free(root);

   _generate_properties_from_node(pd, node);
}

#include "eflat_xml_model.eo.c"
