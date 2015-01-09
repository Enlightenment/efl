#include "eldbus_introspection.h"

typedef struct _Eldbus_Introspection_Element_Parse_Table Eldbus_Introspection_Element_Parse_Table;

struct _Eldbus_Introspection_Element_Parse_Table
{
   const char *name;
   void *(*parse)(Eina_Simple_XML_Node_Tag *tag);
   Eina_List **list;
};

static const char *_eldbus_introspection_attribute_value_get(Eina_Inlist *, const char *);
static void *_eldbus_introspection_parse_node(Eina_Simple_XML_Node_Tag *);
static void *_eldbus_introspection_parse_interface(Eina_Simple_XML_Node_Tag *);
static void *_eldbus_introspection_parse_method(Eina_Simple_XML_Node_Tag *);
static void *_eldbus_introspection_parse_signal(Eina_Simple_XML_Node_Tag *);
static void *_eldbus_introspection_parse_argument(Eina_Simple_XML_Node_Tag *);
static void *_eldbus_introspection_parse_property(Eina_Simple_XML_Node_Tag *);
static void *_eldbus_introspection_parse_annotation(Eina_Simple_XML_Node_Tag *);
static void _eldbus_introspection_parse_children(Eina_Inlist *, const Eldbus_Introspection_Element_Parse_Table[]);
static void _eldbus_introspection_interface_free(Eldbus_Introspection_Interface *);
static void _eldbus_introspection_method_free(Eldbus_Introspection_Method *);
static void _eldbus_introspection_signal_free(Eldbus_Introspection_Signal *);
static void _eldbus_introspection_argument_free(Eldbus_Introspection_Argument *);
static void _eldbus_introspection_property_free(Eldbus_Introspection_Property *);
static void _eldbus_introspection_annotation_free(Eldbus_Introspection_Annotation *);

EAPI Eldbus_Introspection_Node *
eldbus_introspection_parse(const char *xml)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(xml, NULL);

   Eldbus_Introspection_Node *node = NULL;

   Eina_Simple_XML_Node_Root *xml_root = eina_simple_xml_node_load(xml, strlen(xml), EINA_TRUE);

   Eina_Simple_XML_Node *xml_node = (Eina_Simple_XML_Node*)eina_inlist_last(xml_root->children);
   EINA_SAFETY_ON_FALSE_GOTO(EINA_SIMPLE_XML_NODE_TAG == xml_node->type, free_root);

   node = (Eldbus_Introspection_Node*)_eldbus_introspection_parse_node((Eina_Simple_XML_Node_Tag*)xml_node);

free_root:
   eina_simple_xml_node_root_free(xml_root);

   return node;
}

static void *
_eldbus_introspection_parse_node(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Node *node = calloc(1, sizeof(Eldbus_Introspection_Node));
   EINA_SAFETY_ON_NULL_RETURN_VAL(node, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   node->name = eina_stringshare_add(name);

   Eldbus_Introspection_Element_Parse_Table table[] = {
      {"node", _eldbus_introspection_parse_node, &node->nodes},
      {"interface", _eldbus_introspection_parse_interface, &node->interfaces},
      {NULL, NULL, NULL}
   };
   _eldbus_introspection_parse_children(tag->children, table);

   return node;
}

EAPI void
eldbus_introspection_node_free(Eldbus_Introspection_Node *node)
{
   EINA_SAFETY_ON_NULL_RETURN(node);

   Eldbus_Introspection_Node *child_node;
   EINA_LIST_FREE(node->nodes, child_node)
     eldbus_introspection_node_free(child_node);

   Eldbus_Introspection_Interface *interface;
   EINA_LIST_FREE(node->interfaces, interface)
     _eldbus_introspection_interface_free(interface);

   eina_stringshare_del(node->name);
   free(node);
}

static const char *
_eldbus_introspection_attribute_value_get(Eina_Inlist *attributes, const char *key)
{
   Eina_Simple_XML_Attribute *attribute;
   EINA_INLIST_FOREACH(attributes, attribute)
     {
        if (strcmp(attribute->key, key) == 0)
          return attribute->value;
     }

  return NULL;
}

static void *
_eldbus_introspection_parse_interface(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Interface *interface = calloc(1, sizeof(Eldbus_Introspection_Interface));
   EINA_SAFETY_ON_NULL_RETURN_VAL(interface, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   EINA_SAFETY_ON_NULL_GOTO(name, error); // required
   interface->name = eina_stringshare_add(name);

   Eldbus_Introspection_Element_Parse_Table table[] = {
      {"method", _eldbus_introspection_parse_method, &interface->methods},
      {"signal", _eldbus_introspection_parse_signal, &interface->signals},
      {"property", _eldbus_introspection_parse_property, &interface->properties},
      {"annotation", _eldbus_introspection_parse_annotation, &interface->annotations},
      {NULL, NULL, NULL}
   };
   _eldbus_introspection_parse_children(tag->children, table);

   return interface;

error:
   _eldbus_introspection_interface_free(interface);
   return NULL;
}

static void
_eldbus_introspection_interface_free(Eldbus_Introspection_Interface *interface)
{
   EINA_SAFETY_ON_NULL_RETURN(interface);

   Eldbus_Introspection_Method *method;
   EINA_LIST_FREE(interface->methods, method)
     _eldbus_introspection_method_free(method);

   Eldbus_Introspection_Signal *signal;
   EINA_LIST_FREE(interface->signals, signal)
     _eldbus_introspection_signal_free(signal);

   Eldbus_Introspection_Property *property;
   EINA_LIST_FREE(interface->properties, property)
     _eldbus_introspection_property_free(property);

   Eldbus_Introspection_Annotation *annotation;
   EINA_LIST_FREE(interface->annotations, annotation)
     _eldbus_introspection_annotation_free(annotation);

   eina_stringshare_del(interface->name);
   free(interface);
}

static void *
_eldbus_introspection_parse_method(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Method *method = calloc(1, sizeof(Eldbus_Introspection_Method));
   EINA_SAFETY_ON_NULL_RETURN_VAL(method, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   EINA_SAFETY_ON_NULL_GOTO(name, error); // required
   method->name = eina_stringshare_add(name);

   Eldbus_Introspection_Element_Parse_Table table[] = {
      {"arg", _eldbus_introspection_parse_argument, &method->arguments},
      {"annotation", _eldbus_introspection_parse_annotation, &method->annotations},
      {NULL, NULL, NULL}
   };
   _eldbus_introspection_parse_children(tag->children, table);

   return method;

error:
   _eldbus_introspection_method_free(method);
   return NULL;
}

static void
_eldbus_introspection_method_free(Eldbus_Introspection_Method *method)
{
   EINA_SAFETY_ON_NULL_RETURN(method);

   Eldbus_Introspection_Argument *argument;
   EINA_LIST_FREE(method->arguments, argument)
     _eldbus_introspection_argument_free(argument);

   Eldbus_Introspection_Annotation *annotation;
   EINA_LIST_FREE(method->annotations, annotation)
     _eldbus_introspection_annotation_free(annotation);

   eina_stringshare_del(method->name);
   free(method);
}

static void *
_eldbus_introspection_parse_signal(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Signal *signal = calloc(1, sizeof(Eldbus_Introspection_Signal));
   EINA_SAFETY_ON_NULL_RETURN_VAL(signal, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   EINA_SAFETY_ON_NULL_GOTO(name, error); // required
   signal->name = eina_stringshare_add(name);

   Eldbus_Introspection_Element_Parse_Table table[] = {
      {"arg", _eldbus_introspection_parse_argument, &signal->arguments},
      {"annotation", _eldbus_introspection_parse_annotation, &signal->annotations},
      {NULL, NULL, NULL}
   };
   _eldbus_introspection_parse_children(tag->children, table);

   return signal;

error:
   _eldbus_introspection_signal_free(signal);
   return NULL;
}

static void
_eldbus_introspection_signal_free(Eldbus_Introspection_Signal *signal)
{
   EINA_SAFETY_ON_NULL_RETURN(signal);

   Eldbus_Introspection_Argument *argument;
   EINA_LIST_FREE(signal->arguments, argument)
     _eldbus_introspection_argument_free(argument);

   Eldbus_Introspection_Annotation *annotation;
   EINA_LIST_FREE(signal->annotations, annotation)
     _eldbus_introspection_annotation_free(annotation);

   eina_stringshare_del(signal->name);
   free(signal);
}

static void *
_eldbus_introspection_parse_argument(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Argument *argument = calloc(1, sizeof(Eldbus_Introspection_Argument));
   EINA_SAFETY_ON_NULL_RETURN_VAL(argument, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   argument->name = eina_stringshare_add(name);

   const char *type = _eldbus_introspection_attribute_value_get(tag->attributes, "type");
   EINA_SAFETY_ON_NULL_GOTO(type, error); // required
   argument->type = eina_stringshare_add(type);

   const char *direction = _eldbus_introspection_attribute_value_get(tag->attributes, "direction");
   if (direction)
     {
        //EINA_SAFETY_ON_NULL_GOTO(direction, error); // required for methods only
        if (strcmp(direction, "in") == 0)
          argument->direction = ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN;
        else
        if (strcmp(direction, "out") == 0)
          argument->direction = ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_OUT;
        else
          argument->direction = ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_NONE;
     }

   return argument;

error:
   _eldbus_introspection_argument_free(argument);
   return NULL;
}

static void
_eldbus_introspection_argument_free(Eldbus_Introspection_Argument *argument)
{
   EINA_SAFETY_ON_NULL_RETURN(argument);
   eina_stringshare_del(argument->name);
   eina_stringshare_del(argument->type);
   free(argument);
}

static void *
_eldbus_introspection_parse_property(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Property *property = calloc(1, sizeof(Eldbus_Introspection_Property));
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   EINA_SAFETY_ON_NULL_GOTO(name, error); // required
   property->name = eina_stringshare_add(name);

   const char *type = _eldbus_introspection_attribute_value_get(tag->attributes, "type");
   EINA_SAFETY_ON_NULL_GOTO(type, error); // required
   property->type = eina_stringshare_add(type);

   const char *access = _eldbus_introspection_attribute_value_get(tag->attributes, "access");
   EINA_SAFETY_ON_NULL_GOTO(access, error); // required
   if (strcmp(access, "read") == 0)
     property->access = ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READ;
   else
   if (strcmp(access, "write") == 0)
     property->access = ELDBUS_INTROSPECTION_PROPERTY_ACCESS_WRITE;
   else
   if (strcmp(access, "readwrite") == 0)
     property->access = ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READWRITE;
   else
     EINA_SAFETY_ON_TRUE_GOTO(!!"Unknown property access", error);

   Eldbus_Introspection_Element_Parse_Table table[] = {
      {"annotation", _eldbus_introspection_parse_annotation, &property->annotations},
      {NULL, NULL, NULL}
   };
   _eldbus_introspection_parse_children(tag->children, table);

   return property;

error:
   _eldbus_introspection_property_free(property);
   return NULL;
}

static void
_eldbus_introspection_property_free(Eldbus_Introspection_Property *property)
{
   EINA_SAFETY_ON_NULL_RETURN(property);

   Eldbus_Introspection_Annotation *annotation;
   EINA_LIST_FREE(property->annotations, annotation)
     _eldbus_introspection_annotation_free(annotation);

   eina_stringshare_del(property->name);
   eina_stringshare_del(property->type);
   free(property);
}

static void *
_eldbus_introspection_parse_annotation(Eina_Simple_XML_Node_Tag *tag)
{
   Eldbus_Introspection_Annotation *annotation = calloc(1, sizeof(Eldbus_Introspection_Annotation));
   EINA_SAFETY_ON_NULL_RETURN_VAL(annotation, NULL);

   const char *name = _eldbus_introspection_attribute_value_get(tag->attributes, "name");
   EINA_SAFETY_ON_NULL_GOTO(name, error); // required
   annotation->name = eina_stringshare_add(name);

   const char *value = _eldbus_introspection_attribute_value_get(tag->attributes, "value");
   EINA_SAFETY_ON_NULL_GOTO(value, error); // required
   annotation->value = eina_stringshare_add(value);

   return annotation;

error:
   _eldbus_introspection_annotation_free(annotation);
   return NULL;
}

static void
_eldbus_introspection_annotation_free(Eldbus_Introspection_Annotation *annotation)
{
   EINA_SAFETY_ON_NULL_RETURN(annotation);
   eina_stringshare_del(annotation->name);
   eina_stringshare_del(annotation->value);
   free(annotation);
}

static void
_eldbus_introspection_parse_children(Eina_Inlist *children, const Eldbus_Introspection_Element_Parse_Table table[])
{
   Eina_Simple_XML_Node *child;
   EINA_INLIST_FOREACH(children, child)
     {
        if (EINA_SIMPLE_XML_NODE_TAG != child->type)
          continue;

        Eina_Simple_XML_Node_Tag *tag = (Eina_Simple_XML_Node_Tag*)child;

        for (const Eldbus_Introspection_Element_Parse_Table *it = table; it->name; ++it)
          {
             if (strcmp(tag->name, it->name) != 0)
               continue;

             void *item = it->parse(tag);
             if (item)
               *it->list = eina_list_append(*it->list, item);
          }
     }
}

EAPI Eldbus_Introspection_Interface *
eldbus_introspection_interface_find(Eina_List *interfaces, const char *name)
{
   Eina_List *it;
   Eldbus_Introspection_Interface *interface;
   EINA_LIST_FOREACH(interfaces, it, interface)
     {
        if (strcmp(interface->name, name) == 0)
          return interface;
     }

   return NULL;
}

EAPI Eldbus_Introspection_Property *
eldbus_introspection_property_find(Eina_List *properties, const char *name)
{
   Eina_List *it;
   Eldbus_Introspection_Property *property;
   EINA_LIST_FOREACH(properties, it, property)
     {
        if (strcmp(property->name, name) == 0)
          return property;
     }

   return NULL;
}

EAPI Eldbus_Introspection_Argument *
eldbus_introspection_argument_find(Eina_List *arguments, const char *name)
{
   Eina_List *it;
   Eldbus_Introspection_Argument *argument;
   EINA_LIST_FOREACH(arguments, it, argument)
     {
        if (strcmp(argument->name, name) == 0)
          return argument;
     }

   return NULL;
}
