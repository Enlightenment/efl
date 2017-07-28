#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "codegen.h"

#define OBJECT_TAG "node"
#define OBJECT_TAG_LENGTH sizeof(OBJECT_TAG) - 1
#define INTERFACE_TAG "interface"
#define INTERFACE_TAG_LENGTH sizeof(INTERFACE_TAG) - 1
#define SIGNAL_TAG "signal"
#define SIGNAL_TAG_LENGTH sizeof(SIGNAL_TAG) - 1
#define METHOD_TAG "method"
#define METHOD_TAG_LENGTH sizeof(METHOD_TAG) - 1
#define PROPERTY_TAG "property"
#define PROPERTY_TAG_LENGTH sizeof(PROPERTY_TAG) - 1
#define ARG_TAG "arg"
#define ARG_TAG_LENGTH sizeof(ARG_TAG) - 1
#define ANNOTATION_TAG "annotation"
#define ANNOTATION_TAG_LENGTH sizeof(ANNOTATION_TAG) - 1

//attributes
#define NAME_ATTR "name"
#define TYPE_ATTR "type"
#define DIRECTION_ATTR "direction"
#define ACCESS_ATTR "access"
#define VALUE_ATTR "value"

#define ACCESS_ATTR_VALUE_WRITE "write"
#define ACCESS_ATTR_VALUE_READ "read"

#define DBUS_INTERFACE "org.freedesktop.DBus."

static DBus_Interface *iface;
static DBus_Signal *d_signal;
static DBus_Method *method;
static DBus_Property *property;

static Eina_Bool attributes_parse(const char *content, unsigned length, Eina_Simple_XML_Attribute_Cb func, const void *data);

static Eina_Bool
obj_attributes_parser(void *data, const char *key, const char *value)
{
   DBus_Object *obj = data;

   if (!strcmp(key, NAME_ATTR))
     obj->name = strdup(value);

   return EINA_TRUE;
}

static Eina_Bool
iface_attributes_parser(void *data EINA_UNUSED, const char *key, const char *value)
{
   if (!strcmp(key, NAME_ATTR))
     iface->name = strdup(value);

   return EINA_TRUE;
}

static Eina_Bool
signal_attributes_parser(void *data EINA_UNUSED, const char *key, const char *value)
{
   if (!strcmp(key, NAME_ATTR))
     d_signal->name = strdup(value);

   return EINA_TRUE;
}

static Eina_Bool
arg_attributes_parser(void *data EINA_UNUSED, const char *key, const char *value)
{
   DBus_Arg *arg = data;
   if (!strcmp(key, NAME_ATTR))
     arg->name = strdup(value);
   else if (!strcmp(key, TYPE_ATTR))
     arg->type = strdup(value);
   else if (!strcmp(key, DIRECTION_ATTR))
     arg->direction = value[0];

   return EINA_TRUE;
}

static Eina_Bool
method_attributes_parser(void *data EINA_UNUSED, const char *key, const char *value)
{
   if (!strcmp(key, NAME_ATTR))
     method->name = strdup(value);

   return EINA_TRUE;
}

static Eina_Bool
property_attributes_parser(void *data EINA_UNUSED, const char *key, const char *value)
{
   if (!strcmp(key, NAME_ATTR))
     property->name = strdup(value);
   else if (!strcmp(key, TYPE_ATTR))
     {
        property->type = strdup(value);
        if (value[1] || value[0] == 'v')
          property->complex = EINA_TRUE;
     }
   else if (!strcmp(key, ACCESS_ATTR))
     {
        if (!strcmp(value, ACCESS_ATTR_VALUE_READ))
          property->access = ACCESS_READ;
        else if (!strcmp(value, ACCESS_ATTR_VALUE_WRITE))
          property->access = ACCESS_WRITE;
        else
          property->access = (ACCESS_WRITE | ACCESS_READ);
     }

   return EINA_TRUE;
}

static Eina_Bool
open_object(const char *content, unsigned length, Eina_Bool is_open_empty, DBus_Object **ptr_obj)
{
   Eina_Bool r;
   DBus_Object *obj = *ptr_obj;

   if (is_open_empty) return EINA_TRUE;

   if (obj)
     {
        printf("Only one object is supported per file.");
        return EINA_FALSE;
     }
   obj = calloc(1, sizeof(DBus_Object));
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   r = attributes_parse(content, length, obj_attributes_parser, obj);
   if (!obj->name) obj->name = strdup("/");

   obj->c_name = dbus_name_to_c(obj->name);

   *ptr_obj = obj;
   return r;
}

static void
interface_close(void)
{
   //its not necessary generate code to FreeDesktop interfaces
   if (!strncmp(iface->name, DBUS_INTERFACE, strlen(DBUS_INTERFACE)))
     {
        printf("Refusing to generate code for FDO interface; see eldbus_freedesktop.h\n");
        interface_free(iface);
     }
   iface = NULL;
}

static Eina_Bool
open_interface(const char *content, unsigned length, Eina_Bool is_open_empty, DBus_Object *obj)
{
   Eina_Bool r;
   char *tmp_name;

   iface = interface_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   r = attributes_parse(content, length, iface_attributes_parser, NULL);
   if (!iface->name)
     {
        interface_free(iface);
        printf("Error interface without name.\n");
        return EINA_FALSE;
     }

   tmp_name = get_pieces(iface->name, '.', 2);
   iface->c_name = dbus_name_to_c(tmp_name);
   free(tmp_name);

   if (is_open_empty)
     interface_close();

   return r;
}

static void
signal_close(void)
{
   DBus_Arg *arg;
   EINA_INLIST_FOREACH(d_signal->args, arg)
     {
        if ((arg->type[1]) || (arg->type[0] == 'v'))
          {
             d_signal->complex = EINA_TRUE;
             break;
          }
     }
   d_signal = NULL;
}

static Eina_Bool
open_signal(const char *content, unsigned length, Eina_Bool is_open_empty)
{
   Eina_Bool r;
   char *tmp;
   int i;
   Eina_Strbuf *buf;

   d_signal = signal_new(iface);
   EINA_SAFETY_ON_NULL_RETURN_VAL(d_signal, EINA_FALSE);

   r = attributes_parse(content, length, signal_attributes_parser, NULL);
   if (!d_signal->name)
     {
        signal_free(d_signal);
        d_signal = NULL;
        printf("Error signal without name.\n");
        return EINA_FALSE;
     }

   buf = eina_strbuf_new();
   tmp = dbus_name_to_c(d_signal->name);
   d_signal->c_name = string_build("%s_%s", iface->c_name, tmp);
   free(tmp);
   d_signal->cb_name = string_build("on_%s", d_signal->c_name);
   d_signal->free_function = string_build("%s_data_free", d_signal->c_name);
   d_signal->struct_name = string_build("%s_%s_Data", iface->c_name, d_signal->name);
   d_signal->struct_name[0] = toupper(d_signal->struct_name[0]);
   for (i = 0; d_signal->struct_name[i]; i++)
     {
        if (d_signal->struct_name[i] == '_' && d_signal->struct_name[i+1])
          d_signal->struct_name[i+1] = toupper(d_signal->struct_name[i+1]);
     }
   for (i = 0; iface->c_name[i]; i++)
     eina_strbuf_append_char(buf, toupper(iface->c_name[i]));
   eina_strbuf_append_char(buf, '_');
   for (i = 0; d_signal->name[i]; i++)
     {
        if (i && isupper(d_signal->name[i]) && !isupper(d_signal->name[i-1]))
          eina_strbuf_append_char(buf, '_');
        eina_strbuf_append_char(buf, toupper(d_signal->name[i]));
     }
   eina_strbuf_append(buf, "_EVENT");
   d_signal->signal_event = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   if (is_open_empty)
     signal_close();

   return r;
}

#define ANNOTATION_NO_REPLY "org.freedesktop.DBus.Method.NoReply"

static Eina_Bool
annotation_attributes_parser(void *data, const char *key, const char *value)
{
   DBus_Annotation *annotation = data;
   if (!strcmp(key, NAME_ATTR))
     {
        if (!strcmp(value, ANNOTATION_NO_REPLY))
          annotation->type = NO_REPLY;
     }
   else if (!strcmp(key, VALUE_ATTR))
     {
        unsigned i;
        annotation->value = strdup(value);
        for (i = 0; annotation->value[i]; i++)
          annotation->value[i] = tolower(annotation->value[i]);
     }

   return EINA_TRUE;
}

static Eina_Bool
open_annotation(const char *content, unsigned length)
{
   DBus_Annotation annotation;
   Eina_Bool r;

   annotation.type = INVALID;
   r = attributes_parse(content, length, annotation_attributes_parser, &annotation);

   if (annotation.type == NO_REPLY)
     {
        Eina_Bool value = EINA_FALSE;
        if (!strcmp(annotation.value, "true"))
          value = EINA_TRUE;
        free(annotation.value);

        if (method)
          method->no_reply = value;
     }

   return r;
}

static Eina_Bool
open_arg(const char *content, unsigned length)
{
   Eina_Bool r;
   unsigned int *without_name;
   DBus_Arg *arg = calloc(1, sizeof(DBus_Arg));
   EINA_SAFETY_ON_NULL_RETURN_VAL(arg, EINA_FALSE);

   r = attributes_parse(content, length, arg_attributes_parser, arg);
   if (d_signal)
     {
        d_signal->args = eina_inlist_append(d_signal->args, EINA_INLIST_GET(arg));
        without_name = &d_signal->arg_without_name;
     }
   else if (method)
     {
        method->args = eina_inlist_append(method->args, EINA_INLIST_GET(arg));
        without_name = &method->arg_without_name;
     }
   else
     {
        printf("Error find an argument without any valid parent.\n");
        return EINA_FALSE;
     }

   if (!arg->name)
     {
        arg->c_name = string_build("arg%d", *without_name);
        (*without_name)++;
     }
   else
     arg->c_name = dbus_name_to_c(arg->name);

   return r;
}

static void
method_close(void)
{
   DBus_Arg *arg;
   EINA_INLIST_FOREACH(method->args, arg)
     {
        if ((arg->type[1]) || (arg->type[0] == 'v'))
          {
             if (arg->direction == 'o')
               method->out_complex = EINA_TRUE;
             else
               method->in_complex = EINA_TRUE;
          }
     }
   if (method->no_reply)
     {
        free(method->cb_name);
        method->cb_name = strdup("NULL");
     }
   method = NULL;
}

static Eina_Bool
open_method(const char *content, unsigned lenght, Eina_Bool is_open_empty)
{
   Eina_Bool r;
   char *tmp;
   int i;

   method = method_new(iface);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method, EINA_FALSE);

   r = attributes_parse(content, lenght, method_attributes_parser, NULL);
   if (!method->name)
     {
        method_free(method);
        method = NULL;
        printf("Error method without name.\n");
        return EINA_FALSE;
     }

   tmp = dbus_name_to_c(method->name);
   method->c_name = string_build("%s_%s", iface->c_name, tmp);
   free(tmp);
   method->cb_name = string_build("cb_%s", method->c_name);
   method->function_cb = string_build("%s_Cb", method->c_name);
   method->function_cb[0] = toupper(method->function_cb[0]);
   for (i = 0; method->function_cb[i]; i++)
     {
        if (method->function_cb[i] == '_' && method->function_cb[i+1])
           method->function_cb[i+1] = toupper(method->function_cb[i+1]);
     }

   if (is_open_empty)
     method_close();

   return r;
}

static Eina_Bool
open_property(const char *content, unsigned length)
{
   Eina_Bool r;
   char *tmp;

   property = property_new(iface);
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EINA_FALSE);

   r = attributes_parse(content, length, property_attributes_parser, NULL);
   if (!property->name)
     {
        property_free(property);
        property = NULL;
        printf("Error property without name.\n");
        return EINA_FALSE;
     }

   tmp = dbus_name_to_c(property->name);
   property->c_name = string_build("%s_%s", iface->c_name, tmp);
   free(tmp);
   property->cb_name = string_build("cb_%s", property->c_name);

   return r;
}

static Eina_Bool
open_tag(const char *content, unsigned length, Eina_Bool is_open_empty, DBus_Object **obj)
{
   unsigned int i;
   if (!strncmp(content, OBJECT_TAG, OBJECT_TAG_LENGTH))
     return open_object(content, length, is_open_empty, obj);
   else if (!strncmp(content, INTERFACE_TAG, INTERFACE_TAG_LENGTH) && *obj)
     return open_interface(content, length, is_open_empty, *obj);
   else if (!strncmp(content, SIGNAL_TAG, SIGNAL_TAG_LENGTH) && iface)
     return open_signal(content, length, is_open_empty);
   else if (!strncmp(content, ARG_TAG, ARG_TAG_LENGTH) && iface)
     return open_arg(content, length);
   else if (!strncmp(content, ANNOTATION_TAG, ANNOTATION_TAG_LENGTH) && iface)
     return open_annotation(content, length);
   else if (!strncmp(content, METHOD_TAG, METHOD_TAG_LENGTH) && iface)
     return open_method(content, length, is_open_empty);
   else if (!strncmp(content, PROPERTY_TAG, PROPERTY_TAG_LENGTH) && iface)
     return open_property(content, length);

   printf("Warning: Tag not handled:\n");
   for (i = 0; i < length; i++)
     printf("%c", content[i]);
   printf("\n\n");

   return EINA_TRUE;
}

static Eina_Bool
close_tag(const char *content)
{
   if (!strncmp(content, INTERFACE_TAG, INTERFACE_TAG_LENGTH))
     interface_close();
   if (!strncmp(content, SIGNAL_TAG, SIGNAL_TAG_LENGTH))
     signal_close();
   else if (!strncmp(content, METHOD_TAG, METHOD_TAG_LENGTH))
     method_close();
   else if (!strncmp(content, PROPERTY_TAG, PROPERTY_TAG_LENGTH))
     property = NULL;

   return EINA_TRUE;
}

Eina_Bool
parser(void *data, Eina_Simple_XML_Type type, const char *content, unsigned offset EINA_UNUSED, unsigned length)
{
   Eina_Bool r = EINA_TRUE;
   DBus_Object **obj = data;

   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
      case EINA_SIMPLE_XML_OPEN_EMPTY:
        {
           r = open_tag(content, length, type == EINA_SIMPLE_XML_OPEN_EMPTY,
                        obj);
           break;
        }
      case EINA_SIMPLE_XML_CLOSE:
        {
           r = close_tag(content);
           break;
        }
      default:
        break;
     }
   return r;
}

static Eina_Bool
attributes_parse(const char *content, unsigned length, Eina_Simple_XML_Attribute_Cb func, const void *data)
{
   const char *attrs = eina_simple_xml_tag_attributes_find(content, length);
   unsigned attrslen = 0;
   if (attrs)
     {
        attrslen = length - (attrs - content);
        if (!eina_simple_xml_attributes_parse(attrs, attrslen, func, data))
          {
             printf("Parser error - attrs=%s | content=%s\n", attrs, content);
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}
