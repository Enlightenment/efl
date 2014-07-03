#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eldbus_private.h"
#include "eldbus_private_types.h"

#define DBUS_ANNOTATION(name, value) \
        "<annotation" \
        " name=\"org.freedesktop.DBus." name "\"" \
        " value=\"" value "\"" \
        "/>"

#define DBUS_ANNOTATION_DEPRECATED  DBUS_ANNOTATION("Deprecated", "true")
#define DBUS_ANNOTATION_NOREPLY     DBUS_ANNOTATION("Method.NoReply", "true")

#ifndef DBUS_ERROR_UNKNOWN_INTERFACE
# define DBUS_ERROR_UNKNOWN_INTERFACE          "org.freedesktop.DBus.Error.UnknownInterface"
#endif

#ifndef DBUS_ERROR_UNKNOWN_PROPERTY
# define DBUS_ERROR_UNKNOWN_PROPERTY           "org.freedesktop.DBus.Error.UnknownProperty"
#endif

#ifndef DBUS_ERROR_PROPERTY_READ_ONLY
# define DBUS_ERROR_PROPERTY_READ_ONLY         "org.freedesktop.DBus.Error.PropertyReadOnly"
#endif

#define ELDBUS_SERVICE_INTERFACE_CHECK(obj)                         \
  do                                                            \
    {                                                           \
       EINA_SAFETY_ON_NULL_RETURN(obj);                         \
       if (!EINA_MAGIC_CHECK(obj, ELDBUS_SERVICE_INTERFACE_MAGIC))  \
         {                                                      \
            EINA_MAGIC_FAIL(obj, ELDBUS_SERVICE_INTERFACE_MAGIC);   \
            return;                                             \
         }                                                      \
    }                                                           \
  while (0)

#define ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(obj, retval)                  \
  do                                                                    \
    {                                                                   \
       EINA_SAFETY_ON_NULL_RETURN_VAL(obj, retval);                     \
       if (!EINA_MAGIC_CHECK(obj, ELDBUS_SERVICE_INTERFACE_MAGIC))          \
         {                                                              \
            EINA_MAGIC_FAIL(obj, ELDBUS_SERVICE_INTERFACE_MAGIC);           \
            return retval;                                              \
         }                                                              \
    }                                                                   \
  while (0)


static void _object_unregister(DBusConnection *conn, void *user_data);
static DBusHandlerResult _object_handler(DBusConnection *conn, DBusMessage *message, void *user_data);
static void _object_free(Eldbus_Service_Object *obj);
static void _interface_free(Eldbus_Service_Interface *interface);
static void _on_connection_free(void *data, const void *dead_pointer);

static DBusObjectPathVTable vtable = {
  _object_unregister,
  _object_handler,
  NULL,
  NULL,
  NULL,
  NULL
};

static Eldbus_Service_Interface *introspectable;
static Eldbus_Service_Interface *properties_iface;
static Eldbus_Service_Interface *objmanager;

static inline void
_introspect_arguments_append(Eina_Strbuf *buf, const Eldbus_Arg_Info *args,
                             const char *direction)
{
   for (; args && args->signature; args++)
     {
        if (args->name && args->name[0])
          eina_strbuf_append_printf(buf, "<arg type=\"%s\" name=\"%s\"",
                                    args->signature, args->name);
        else
          eina_strbuf_append_printf(buf, "<arg type=\"%s\"", args->signature);

        if (direction)
          eina_strbuf_append_printf(buf, " direction=\"%s\" />", direction);
        else
          eina_strbuf_append(buf, " />");
     }
}

static inline void
_introspect_append_signal(Eina_Strbuf *buf, const Eldbus_Signal *sig)
{
   eina_strbuf_append_printf(buf, "<signal name=\"%s\"", sig->name);

   if (!sig->flags && !(sig->args && sig->args->signature))
     {
        eina_strbuf_append(buf, " />");
        return;
     }

   eina_strbuf_append(buf, ">");

   if (sig->flags & ELDBUS_SIGNAL_FLAG_DEPRECATED)
     eina_strbuf_append(buf, DBUS_ANNOTATION_DEPRECATED);

   _introspect_arguments_append(buf, sig->args, NULL);

   eina_strbuf_append(buf, "</signal>");
}

static inline void
_instrospect_append_property(Eina_Strbuf *buf, const Eldbus_Property *prop, const Eldbus_Service_Interface *iface)
{
   eina_strbuf_append_printf(buf, "<property name=\"%s\" type=\"%s\" access=\"",
                             prop->name, prop->type);

   if (iface->get_func || prop->get_func)
     eina_strbuf_append(buf, "read");

   if (iface->set_func || prop->set_func)
     eina_strbuf_append(buf, "write");

   if (!prop->flags)
     {
        eina_strbuf_append(buf, "\" />");
        return;
     }

   eina_strbuf_append(buf, "\">");

   if (prop->flags & ELDBUS_PROPERTY_FLAG_DEPRECATED)
     eina_strbuf_append(buf, DBUS_ANNOTATION_DEPRECATED);

   eina_strbuf_append(buf, "</property>");
}

static inline void
_introspect_append_method(Eina_Strbuf *buf, const Eldbus_Method *method)
{
   eina_strbuf_append_printf(buf, "<method name=\"%s\">", method->member);

   if (method->flags & ELDBUS_METHOD_FLAG_DEPRECATED)
     eina_strbuf_append(buf, DBUS_ANNOTATION_DEPRECATED);

   if (method->flags & ELDBUS_METHOD_FLAG_NOREPLY)
     eina_strbuf_append(buf, DBUS_ANNOTATION_NOREPLY);

   _introspect_arguments_append(buf, method->in, "in");
   _introspect_arguments_append(buf, method->out, "out");
   eina_strbuf_append(buf, "</method>");
}

typedef struct _Property
{
   const Eldbus_Property *property;
   Eina_Bool is_invalidate:1;
} Property;

static void
_introspect_append_interface(Eina_Strbuf *buf, Eldbus_Service_Interface *iface)
{
   const Eldbus_Method *method;
   Property *prop;
   Eina_Iterator *iterator;
   unsigned short i;

   eina_strbuf_append_printf(buf, "<interface name=\"%s\">", iface->name);

   iterator = eina_hash_iterator_data_new(iface->methods);
   EINA_ITERATOR_FOREACH(iterator, method)
     _introspect_append_method(buf, method);
   eina_iterator_free(iterator);

   for (i = 0; i < eina_array_count(iface->sign_of_signals); i++)
     _introspect_append_signal(buf, &iface->signals[i]);

   iterator = eina_hash_iterator_data_new(iface->properties);
   EINA_ITERATOR_FOREACH(iterator, prop)
     _instrospect_append_property(buf, prop->property, iface);
   eina_iterator_free(iterator);

   eina_strbuf_append(buf, "</interface>");
}

static Eldbus_Message *
_cb_property_get(const Eldbus_Service_Interface *piface, const Eldbus_Message *msg)
{
   const char *propname, *iface_name;
   Eldbus_Service_Object *obj = piface->obj;
   Eldbus_Service_Interface *iface;
   Property *prop;
   Eldbus_Message *reply, *error_reply = NULL;
   Eldbus_Message_Iter *main_iter, *variant;
   Eina_Bool ret;
   Eldbus_Property_Get_Cb getter = NULL;

   if (!eldbus_message_arguments_get(msg, "ss", &iface_name, &propname))
     return NULL;

   iface = eina_hash_find(obj->interfaces, iface_name);
   if (!iface)
     return eldbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_INTERFACE,
                                    "Interface not found.");

   prop = eina_hash_find(iface->properties, propname);
   if (!prop || prop->is_invalidate) goto not_found;

   if (prop->property->get_func)
     getter = prop->property->get_func;
   else if (iface->get_func)
     getter = iface->get_func;

   if (!getter) goto not_found;

   reply = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);

   main_iter = eldbus_message_iter_get(reply);
   variant = eldbus_message_iter_container_new(main_iter, 'v',
                                              prop->property->type);

   ret = getter(iface, propname, variant, msg, &error_reply);

   if (ret)
     {
        eldbus_message_iter_container_close(main_iter, variant);
        return reply;
     }

   eldbus_message_unref(reply);
   return error_reply;

not_found:
   return eldbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_PROPERTY,
                                  "Property not found.");
}

static Eina_Bool
_props_getall(Eldbus_Service_Interface *iface, Eina_Iterator *iterator, Eldbus_Message_Iter *dict, const Eldbus_Message *input_msg, Eldbus_Message **error_reply)
{
   Property *prop;
   EINA_ITERATOR_FOREACH(iterator, prop)
     {
        Eldbus_Message_Iter *entry, *var;
        Eina_Bool ret;
        Eldbus_Property_Get_Cb getter = NULL;

        if (prop->property->get_func)
          getter = prop->property->get_func;
        else if (iface->get_func)
          getter = iface->get_func;

        if (!getter || prop->is_invalidate)
          continue;

        if (!eldbus_message_iter_arguments_append(dict, "{sv}", &entry))
          continue;

        eldbus_message_iter_basic_append(entry, 's', prop->property->name);
        var = eldbus_message_iter_container_new(entry, 'v',
                                               prop->property->type);

        ret = getter(iface, prop->property->name, var, input_msg, error_reply);
        if (!ret)
          return EINA_FALSE;

        eldbus_message_iter_container_close(entry, var);
        eldbus_message_iter_container_close(dict, entry);
     }
   return EINA_TRUE;
}

static Eldbus_Message *
_cb_property_getall(const Eldbus_Service_Interface *piface, const Eldbus_Message *msg)
{
   const char *iface_name;
   Eldbus_Service_Object *obj = piface->obj;
   Eldbus_Service_Interface *iface;
   Eina_Iterator *iterator;
   Eldbus_Message *reply, *error_reply;
   Eldbus_Message_Iter *main_iter, *dict;

   if (!eldbus_message_arguments_get(msg, "s", &iface_name))
     return NULL;

   iface = eina_hash_find(obj->interfaces, iface_name);
   if (!iface)
     return eldbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_INTERFACE,
                                    "Interface not found.");

   reply = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);
   main_iter = eldbus_message_iter_get(reply);
   if (!eldbus_message_iter_arguments_append(main_iter, "a{sv}", &dict))
     {
        eldbus_message_unref(reply);
        return NULL;
     }

   iterator = eina_hash_iterator_data_new(iface->properties);
   if (!_props_getall(iface, iterator, dict, msg, &error_reply))
     {
        eldbus_message_unref(reply);
        eina_iterator_free(iterator);
        return error_reply;
     }
   eldbus_message_iter_container_close(main_iter, dict);

   eina_iterator_free(iterator);
   return reply;
}

static Eldbus_Message *
_cb_property_set(const Eldbus_Service_Interface *piface, const Eldbus_Message *msg)
{
   const char *propname, *iface_name;
   Eldbus_Service_Object *obj = piface->obj;
   Eldbus_Service_Interface *iface;
   Property *prop;
   Eldbus_Message *reply;
   Eldbus_Message_Iter *variant;
   Eldbus_Property_Set_Cb setter = NULL;

   if (!eldbus_message_arguments_get(msg, "ssv", &iface_name, &propname, &variant))
     return NULL;

   iface = eina_hash_find(obj->interfaces, iface_name);
   if (!iface)
     return eldbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_INTERFACE,
                                    "Interface not found.");

   prop = eina_hash_find(iface->properties, propname);
   if (!prop || prop->is_invalidate)
     return eldbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_PROPERTY,
                                    "Property not found.");

   if (prop->property->set_func)
     setter = prop->property->set_func;
   else if (iface->set_func)
     setter = iface->set_func;

   if (!setter)
     return eldbus_message_error_new(msg, DBUS_ERROR_PROPERTY_READ_ONLY,
                                    "This property is read only");

   reply = setter(iface, propname, variant, msg);
   return reply;
}

static Eldbus_Message *
cb_introspect(const Eldbus_Service_Interface *_iface, const Eldbus_Message *message)
{
   Eldbus_Service_Object *obj = _iface->obj;
   Eldbus_Message *reply = eldbus_message_method_return_new(message);
   if (obj->introspection_dirty || !obj->introspection_data)
     {
        Eina_Iterator *iterator;
        Eldbus_Service_Interface *iface;
        Eldbus_Service_Object *child;
        size_t baselen;

        if (obj->introspection_data)
          eina_strbuf_reset(obj->introspection_data);
        else
          obj->introspection_data = eina_strbuf_new();
        EINA_SAFETY_ON_NULL_RETURN_VAL(obj->introspection_data, NULL);

        eina_strbuf_append(obj->introspection_data, "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">");
        eina_strbuf_append_printf(obj->introspection_data,
                                  "<node name=\"%s\">", obj->path);

        iterator = eina_hash_iterator_data_new(obj->interfaces);
        EINA_ITERATOR_FOREACH(iterator, iface)
          _introspect_append_interface(obj->introspection_data, iface);
        eina_iterator_free(iterator);

        baselen = strlen(obj->path);
        /* account for the last '/' */
        if (baselen != 1)
          baselen++;

        EINA_INLIST_FOREACH(obj->children, child)
           eina_strbuf_append_printf(obj->introspection_data,
                                     "<node name=\"%s\" />",
                                     child->path + baselen);

        eina_strbuf_append(obj->introspection_data, "</node>");
        obj->introspection_dirty = EINA_FALSE;
     }

   eldbus_message_arguments_append(reply, "s", eina_strbuf_string_get(obj->introspection_data));
   return reply;
}

static const Eldbus_Method introspect = {
   "Introspect", NULL, ELDBUS_ARGS({ "s", "xml" }), cb_introspect, 0, 0
};

static void
_introspectable_create(void)
{
   introspectable = calloc(1, sizeof(Eldbus_Service_Interface));
   EINA_SAFETY_ON_NULL_RETURN(introspectable);

   EINA_MAGIC_SET(introspectable, ELDBUS_SERVICE_INTERFACE_MAGIC);
   introspectable->sign_of_signals = eina_array_new(1);
   introspectable->properties = eina_hash_string_small_new(NULL);
   introspectable->name = ELDBUS_FDO_INTERFACE_INTROSPECTABLE;
   introspectable->methods = eina_hash_string_small_new(NULL);

   eina_hash_add(introspectable->methods, introspect.member, &introspect);
}

static void
_default_interfaces_free(void)
{
   eina_hash_free(introspectable->methods);
   eina_hash_free(introspectable->properties);
   eina_array_free(introspectable->sign_of_signals);
   free(introspectable);

   eina_hash_free(properties_iface->methods);
   eina_hash_free(properties_iface->properties);
   eina_array_free(properties_iface->sign_of_signals);
   free(properties_iface);

   eina_hash_free(objmanager->methods);
   eina_hash_free(objmanager->properties);
   eina_array_free(objmanager->sign_of_signals);
   free(objmanager);
}

static const Eldbus_Method _property_methods[] = {
   {
    "Get", ELDBUS_ARGS({"s", "interface"}, {"s", "property"}),
    ELDBUS_ARGS({"v", "value"}), _cb_property_get, 0, NULL
   },
   {
    "Set", ELDBUS_ARGS({"s", "interface"}, {"s", "property"}, {"v", "value"}),
    NULL, _cb_property_set, 0, NULL
   },
   {
    "GetAll", ELDBUS_ARGS({"s", "interface"}), ELDBUS_ARGS({"a{sv}", "props"}),
    _cb_property_getall, 0, NULL
   }
};

static const Eldbus_Signal _properties_signals[] = {
   {
    "PropertiesChanged",
    ELDBUS_ARGS({"s", "interface"}, {"a{sv}", "changed_properties"}, {"as", "invalidated_properties"}), 0
   }
};

static void
_properties_create(void)
{
   properties_iface = calloc(1, sizeof(Eldbus_Service_Interface));
   if (!properties_iface) return;

   properties_iface->sign_of_signals = eina_array_new(1);
   properties_iface->properties =  eina_hash_string_small_new(NULL);
   properties_iface->name = ELDBUS_FDO_INTERFACE_PROPERTIES;
   properties_iface->methods = eina_hash_string_small_new(NULL);
   EINA_MAGIC_SET(properties_iface, ELDBUS_SERVICE_INTERFACE_MAGIC);

   eina_hash_add(properties_iface->methods, _property_methods[0].member,
                 &_property_methods[0]);
   eina_hash_add(properties_iface->methods, _property_methods[1].member,
                 &_property_methods[1]);
   eina_hash_add(properties_iface->methods, _property_methods[2].member,
                 &_property_methods[2]);

   properties_iface->signals = _properties_signals;
   eina_array_push(properties_iface->sign_of_signals, "sa{sv}as");
}

static Eina_Bool
_propmgr_iface_props_append(Eldbus_Service_Interface *iface, Eldbus_Message_Iter *array)
{
   Eldbus_Message_Iter *iface_entry, *props_array;
   Eina_Iterator *iterator;
   Eldbus_Message *error_msg;

   eldbus_message_iter_arguments_append(array, "{sa{sv}}", &iface_entry);

   eldbus_message_iter_arguments_append(iface_entry, "sa{sv}", iface->name, &props_array);
   iterator = eina_hash_iterator_data_new(iface->properties);
   if (!_props_getall(iface, iterator, props_array, NULL, &error_msg))
     {
        ERR("Error reply was set without pass any input message.");
        eldbus_message_unref(error_msg);
        eina_iterator_free(iterator);
        return EINA_FALSE;
     }
   eina_iterator_free(iterator);
   eldbus_message_iter_container_close(iface_entry, props_array);
   eldbus_message_iter_container_close(array, iface_entry);
   return EINA_TRUE;
}

static Eina_Bool
_managed_obj_append(Eldbus_Service_Object *obj, Eldbus_Message_Iter *array, Eina_Bool first)
{
   Eldbus_Message_Iter *obj_entry, *array_interface;
   Eina_Iterator *iface_iter;
   Eldbus_Service_Interface *iface;
   Eldbus_Service_Object *children;

   if (first) goto foreach;
   if (obj->objmanager) return EINA_TRUE;

   eldbus_message_iter_arguments_append(array, "{oa{sa{sv}}}", &obj_entry);
   eldbus_message_iter_arguments_append(obj_entry, "oa{sa{sv}}", obj->path,
                                       &array_interface);
   iface_iter = eina_hash_iterator_data_new(obj->interfaces);

   EINA_ITERATOR_FOREACH(iface_iter, iface)
     {
        Eina_Bool ret;

        if (eina_list_data_find(obj->iface_added, iface))
          continue;

        ret = _propmgr_iface_props_append(iface, array_interface);
        if (ret)
          continue;

        eina_iterator_free(iface_iter);
        return EINA_FALSE;
     }
   eina_iterator_free(iface_iter);
   eldbus_message_iter_container_close(obj_entry, array_interface);
   eldbus_message_iter_container_close(array, obj_entry);

foreach:
   EINA_INLIST_FOREACH(obj->children, children)
     {
        Eina_Bool ret;
        ret = _managed_obj_append(children, array, EINA_FALSE);
        if (!ret) return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eldbus_Message *
_cb_managed_objects(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *array_path, *main_iter;
   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);
   main_iter =  eldbus_message_iter_get(reply);
   eldbus_message_iter_arguments_append(main_iter, "a{oa{sa{sv}}}", &array_path);

   ret = _managed_obj_append(iface->obj, array_path, EINA_TRUE);
   if (!ret)
     {
        eldbus_message_unref(reply);
        return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error",
                                       "Irrecoverable error happen");
     }

   eldbus_message_iter_container_close(main_iter, array_path);
   return reply;
}

static Eldbus_Method get_managed_objects = {
   "GetManagedObjects", NULL, ELDBUS_ARGS({"a{oa{sa{sv}}}", "objects"}),
   _cb_managed_objects, 0, NULL
};

static const Eldbus_Signal _object_manager_signals[] = {
   {
    "InterfacesAdded", ELDBUS_ARGS({"o", "object"}, {"a{sa{sv}}", "interfaces"}), 0
   },
   {
    "InterfacesRemoved", ELDBUS_ARGS({"o", "object"}, {"as", "interfaces"}), 0
   }
};

static void
_object_manager_create(void)
{
   objmanager = calloc(1, sizeof(Eldbus_Service_Interface));
   if (!objmanager) return;

   EINA_MAGIC_SET(objmanager, ELDBUS_SERVICE_INTERFACE_MAGIC);
   objmanager->sign_of_signals = eina_array_new(1);
   objmanager->properties = eina_hash_string_small_new(NULL);
   objmanager->name = ELDBUS_FDO_INTERFACE_OBJECT_MANAGER;
   objmanager->methods = eina_hash_string_small_new(NULL);

   eina_hash_add(objmanager->methods, get_managed_objects.member,
                 &get_managed_objects);

   objmanager->signals = _object_manager_signals;
   eina_array_push(objmanager->sign_of_signals, "oa{sa{sv}}");
   eina_array_push(objmanager->sign_of_signals, "oas");
}

Eina_Bool
eldbus_service_init(void)
{
   _introspectable_create();
   EINA_SAFETY_ON_NULL_RETURN_VAL(introspectable, EINA_FALSE);
   _properties_create();
   EINA_SAFETY_ON_NULL_RETURN_VAL(properties_iface, EINA_FALSE);
   _object_manager_create();
   EINA_SAFETY_ON_NULL_RETURN_VAL(objmanager, EINA_FALSE);

   return EINA_TRUE;
}

void
eldbus_service_shutdown(void)
{
   _default_interfaces_free();
}

static Eldbus_Service_Object *
_eldbus_service_object_parent_find(Eldbus_Service_Object *obj)
{
   Eldbus_Service_Object *parent = NULL;
   size_t len = strlen(obj->path);
   char *path = strdup(obj->path);
   char *slash;

   for (slash = path[len] != '/' ? &path[len - 1] : &path[len - 2];
        slash > path; slash--)
     {
        if (*slash != '/')
          continue;

        *slash = '\0';

        if (dbus_connection_get_object_path_data(obj->conn->dbus_conn,
                                     path, (void **) &parent) && parent != NULL)
          break;
     }

   free(path);
   return parent;
}

static Eldbus_Service_Object *
_eldbus_service_object_add(Eldbus_Connection *conn, const char *path, Eina_Bool fallback)
{
   Eldbus_Service_Object *obj, *rootobj;
   Eina_Inlist *safe;
   size_t pathlen;

   obj = calloc(1, sizeof(Eldbus_Service_Object));
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   obj->fallback = fallback;
   if ((fallback && !dbus_connection_register_fallback(conn->dbus_conn, path, &vtable, obj)) ||
      (!fallback && !dbus_connection_register_object_path(conn->dbus_conn, path, &vtable, obj)))
     {
        free(obj);
        return NULL;
     }

   obj->conn = conn;
   obj->path = eina_stringshare_add(path);
   obj->interfaces = eina_hash_string_superfast_new(NULL);
   eldbus_connection_free_cb_add(conn, _on_connection_free, obj);

   if (obj->fallback) return obj;

   eina_hash_add(obj->interfaces, introspectable->name, introspectable);
   eina_hash_add(obj->interfaces, properties_iface->name, properties_iface);

   obj->parent = _eldbus_service_object_parent_find(obj);
   if (obj->parent)
     {
        obj->parent->children = eina_inlist_append(obj->parent->children,
                                                   EINA_INLIST_GET(obj));
        obj->parent->introspection_dirty = EINA_TRUE;
        return obj;
     }

   /*
    * If there wasn't any object above us, check if anyone in conn->root_obj
    * should become our child and append ourselves there.
    */
   pathlen = strlen(obj->path);
   EINA_INLIST_FOREACH_SAFE(conn->root_objs, safe, rootobj)
     {
        if (strncmp(obj->path,  rootobj->path, pathlen) != 0)
          continue;

        if (rootobj->path[pathlen] != '/' && pathlen > 1)
          continue;

        conn->root_objs = eina_inlist_remove(conn->root_objs,
                                             EINA_INLIST_GET(rootobj));
        obj->children = eina_inlist_append(obj->children,
                                           EINA_INLIST_GET(rootobj));
        rootobj->parent = obj;
     }
   conn->root_objs = eina_inlist_append(conn->root_objs, EINA_INLIST_GET(obj));

   return obj;
}

static void
_props_free(void *data)
{
   Property *p = data;
   free(p);
}

static void
_object_manager_iface_added_emit(Eldbus_Service_Object *obj,
                                 Eldbus_Service_Object *parent)
{
   Eldbus_Service_Interface *iface;
   Eldbus_Message_Iter *iter, *array;
   Eldbus_Message *sig = eldbus_message_signal_new(parent->path,
                                                 ELDBUS_FDO_INTERFACE_OBJECT_MANAGER,
                                                 "InterfacesAdded");
   EINA_SAFETY_ON_NULL_RETURN(sig);
   iter = eldbus_message_iter_get(sig);
   eldbus_message_iter_arguments_append(iter, "oa{sa{sv}}", obj->path,
                                       &array);

   EINA_LIST_FREE(obj->iface_added, iface)
     {
        if (!_propmgr_iface_props_append(iface, array))
          {
             ERR("Could not append properties to InterfacesAdded signal");
             eldbus_message_unref(sig);
             goto done;
          }
     }
   eldbus_message_iter_container_close(iter, array);
   eldbus_connection_send(parent->conn, sig, NULL, NULL, -1);
   return;

done:
   obj->iface_added = eina_list_free(obj->iface_added);
}

static void
_object_manager_iface_removed_emit(Eldbus_Service_Object *obj,
                                   Eldbus_Service_Object *parent)
{
   Eina_List *l;
   const char *name;
   Eldbus_Message_Iter *iter, *array;
   Eldbus_Message *sig = eldbus_message_signal_new(parent->path,
                                                   ELDBUS_FDO_INTERFACE_OBJECT_MANAGER,
                                                   "InterfacesRemoved");
   EINA_SAFETY_ON_NULL_RETURN(sig);

   iter = eldbus_message_iter_get(sig);
   eldbus_message_iter_arguments_append(iter, "oas", obj->path, &array);

   EINA_LIST_FOREACH(obj->iface_removed, l, name)
     {
        eldbus_message_iter_arguments_append(array, "s", name);
        eina_stringshare_del(name);
     }
   eldbus_message_iter_container_close(iter, array);
   eldbus_connection_send(parent->conn, sig, NULL, NULL, -1);
   obj->iface_removed = eina_list_free(obj->iface_removed);
}

static Eldbus_Service_Object *
_object_manager_parent_find(Eldbus_Service_Object *obj)
{
   if (!obj->parent)
     return NULL;
   if (obj->parent->objmanager)
     return obj->parent;
   return _object_manager_parent_find(obj->parent);
}

static Eina_Bool
_object_manager_changes_process(void *data)
{
   Eldbus_Service_Object *obj = data;
   Eldbus_Service_Object *parent = _object_manager_parent_find(obj);

   obj->idler_iface_changed = NULL;

   if (!parent)
     {
        const char *name;

        obj->iface_added = eina_list_free(obj->iface_added);
        EINA_LIST_FREE(obj->iface_removed, name)
          eina_stringshare_del(name);

        return EINA_FALSE;
     }

   if (obj->iface_added)
     _object_manager_iface_added_emit(obj, parent);
   if (obj->iface_removed)
     _object_manager_iface_removed_emit(obj, parent);

   return EINA_FALSE;
}

static Eldbus_Service_Interface *
_eldbus_service_interface_add(Eldbus_Service_Object *obj, const char *interface)
{
   Eldbus_Service_Interface *iface;

   iface = eina_hash_find(obj->interfaces, interface);
   if (iface) return iface;

   iface = calloc(1, sizeof(Eldbus_Service_Interface));
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);

   EINA_MAGIC_SET(iface, ELDBUS_SERVICE_INTERFACE_MAGIC);
   iface->name = eina_stringshare_add(interface);
   iface->methods = eina_hash_string_superfast_new(NULL);
   iface->properties = eina_hash_string_superfast_new(_props_free);
   iface->obj = obj;
   eina_hash_add(obj->interfaces, iface->name, iface);

   if (!obj->idler_iface_changed)
     obj->idler_iface_changed = ecore_idler_add(_object_manager_changes_process,
                                                obj);
   obj->iface_added = eina_list_append(obj->iface_added, iface);

   return iface;
}

static Eina_Bool
_have_signature(const Eldbus_Arg_Info *args, Eldbus_Message *msg)
{
   const char *sig = dbus_message_get_signature(msg->dbus_msg);
   const char *p = NULL;

   for (; args && args->signature && *sig; args++)
     {
        p = args->signature;
        for (; *sig && *p; sig++, p++)
          {
             if (*p != *sig)
               return EINA_FALSE;
          }
     }

   if (*sig || (p && *p) || (args && args->signature))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_eldbus_service_method_add(Eldbus_Service_Interface *interface,
                          const Eldbus_Method *method)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_hash_find(interface->methods,
                                  method->member), EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method->member, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method->cb, EINA_FALSE);

   eina_hash_add(interface->methods, method->member, method);
   return EINA_TRUE;
}

static Eina_Bool
_eldbus_service_property_add(Eldbus_Service_Interface *interface,
                            const Eldbus_Property *property)
{
   Property *p;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_hash_find(interface->properties,
                                  property->name), EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(property->type, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(
      dbus_signature_validate_single(property->type, NULL), EINA_FALSE);

   p = calloc(1, sizeof(Property));
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, EINA_FALSE);
   p->property = property;

   return eina_hash_add(interface->properties, property->name, p);
}

/* Check if all signals in desc have valid signatures and return an Eina_Array
 * with each of them. Return NULL if any of the signatures is invalid */
static inline Eina_Array *
_eldbus_service_interface_desc_signals_signatures_get(
   const Eldbus_Service_Interface_Desc *desc)
{
   const Eldbus_Signal *sig;
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Array *signatures = eina_array_new(1);

   EINA_SAFETY_ON_NULL_GOTO(buf, fail_signature);
   EINA_SAFETY_ON_NULL_GOTO(signatures, fail_signature);

   for (sig = desc->signals; sig && sig->name; sig++)
     {
        const Eldbus_Arg_Info *arg;

        eina_strbuf_reset(buf);
        for (arg = sig->args; arg && arg->signature; arg++)
          eina_strbuf_append(buf, arg->signature);

        if (!dbus_signature_validate(eina_strbuf_string_get(buf), NULL))
          {
             ERR("Signal with invalid signature: interface=%s signal=%s",
                 desc->interface, sig->name);
             goto fail_signature;
          }

	eina_array_push(signatures,
                        eina_stringshare_add(eina_strbuf_string_get(buf)));
     }
   eina_strbuf_free(buf);

   return signatures;

fail_signature:
   if (buf) eina_strbuf_free(buf);
   if (signatures) eina_array_free(signatures);
   return NULL;
}

static Eldbus_Service_Interface *
_eldbus_service_interface_register(Eldbus_Connection *conn, const char *path, const Eldbus_Service_Interface_Desc *desc, Eina_Bool fallback)
{
   Eldbus_Service_Object *obj;
   Eldbus_Service_Interface *iface;
   const Eldbus_Method *method;
   const Eldbus_Property *property;
   Eina_Array *signatures;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->interface, NULL);

   if (!dbus_connection_get_object_path_data(conn->dbus_conn, path,
                                             (void*)&obj))
     {
        ERR("Invalid object path");
        return NULL;
     }

   signatures = _eldbus_service_interface_desc_signals_signatures_get(desc);
   if (!signatures)
     return NULL;

   if (!obj)
     obj = _eldbus_service_object_add(conn, path, fallback);
   else
     obj->introspection_dirty = EINA_TRUE;
   EINA_SAFETY_ON_NULL_GOTO(obj, fail);

   iface = _eldbus_service_interface_add(obj, desc->interface);
   if (!iface)
     goto fail;

   for (method = desc->methods; method && method->member; method++)
     _eldbus_service_method_add(iface, method);

   iface->signals = desc->signals;
   iface->sign_of_signals = signatures;

   for (property = desc->properties; property && property->name; property++)
     _eldbus_service_property_add(iface, property);

   iface->get_func = desc->default_get;
   iface->set_func = desc->default_set;

   return iface;

fail:
   eina_array_free(signatures);

   if (obj && (eina_hash_population(obj->interfaces) < 2))
     _object_free(obj);

   return NULL;
}

EAPI Eldbus_Service_Interface *
eldbus_service_interface_register(Eldbus_Connection *conn, const char *path, const Eldbus_Service_Interface_Desc *desc)
{
   return _eldbus_service_interface_register(conn, path, desc, EINA_FALSE);
}

EAPI Eldbus_Service_Interface *
eldbus_service_interface_fallback_register(Eldbus_Connection *conn, const char *path, const Eldbus_Service_Interface_Desc *desc)
{
   return _eldbus_service_interface_register(conn, path, desc, EINA_TRUE);
}

static Eina_Bool
_idler_propschanged(void *data)
{
   Eldbus_Service_Interface *iface = data;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *main_iter, *dict, *array_invalidate;
   Eina_Hash *added = NULL;
   Property *prop;

   iface->idler_propschanged = NULL;

   added = eina_hash_string_small_new(NULL);
   msg = eldbus_message_signal_new(iface->obj->path, properties_iface->name,
                                  properties_iface->signals[0].name);
   EINA_SAFETY_ON_NULL_GOTO(msg, error);

   main_iter = eldbus_message_iter_get(msg);
   if (!eldbus_message_iter_arguments_append(main_iter, "sa{sv}", iface->name, &dict))
     {
        eldbus_message_unref(msg);
        goto error;
     }

   if (!iface->props_changed)
     goto invalidate;
   while ((prop = eina_array_pop(iface->props_changed)))
     {
        Eldbus_Message_Iter *entry, *var;
        Eldbus_Message *error_reply = NULL;
        Eina_Bool ret;
        Eldbus_Property_Get_Cb getter = NULL;

        if (eina_hash_find(added, prop->property->name))
          continue;
        eina_hash_add(added, prop->property->name, prop);

        if (prop->property->get_func)
          getter = prop->property->get_func;
        else if (iface->get_func)
          getter = iface->get_func;

        if (!getter || prop->is_invalidate)
          continue;

        if (!eldbus_message_iter_arguments_append(dict, "{sv}", &entry))
          {
             eldbus_message_unref(msg);
             goto error;
          }


        eldbus_message_iter_basic_append(entry, 's', prop->property->name);
        var = eldbus_message_iter_container_new(entry, 'v',
                                               prop->property->type);

        ret = getter(iface, prop->property->name, var, NULL, &error_reply);
        if (!ret)
          {
             eldbus_message_unref(msg);
             if (error_reply)
               {
                  ERR("Error reply was set without pass any input message.");
                  eldbus_message_unref(error_reply);
               }
             ERR("Getter of property %s returned error.", prop->property->name);
             goto error;
          }

        eldbus_message_iter_container_close(entry, var);
        eldbus_message_iter_container_close(dict, entry);
     }
invalidate:
   eldbus_message_iter_container_close(main_iter, dict);

   eldbus_message_iter_arguments_append(main_iter, "as", &array_invalidate);

   if (!iface->prop_invalidated)
     goto end;
   while ((prop = eina_array_pop(iface->prop_invalidated)))
     {
        if (!prop->is_invalidate)
          continue;
        eldbus_message_iter_basic_append(array_invalidate, 's',
                                        prop->property->name);
     }
end:
   eldbus_message_iter_container_close(main_iter, array_invalidate);

   eldbus_service_signal_send(iface, msg);
error:
   if (added)
     eina_hash_free(added);
   if (iface->props_changed)
     eina_array_flush(iface->props_changed);
   if (iface->prop_invalidated)
     eina_array_flush(iface->prop_invalidated);
   return ECORE_CALLBACK_CANCEL;
}

static void
_interface_free(Eldbus_Service_Interface *interface)
{
   const char *sig;
   Eldbus_Service_Object *obj;
   Eina_List *l;

   if (interface == introspectable || interface == properties_iface ||
       interface == objmanager)
     return;

   /**
    * flush props changes before remove interface
    */
   if (interface->idler_propschanged)
     {
        ecore_idler_del(interface->idler_propschanged);
        _idler_propschanged(interface);
     }

   eina_hash_free(interface->methods);
   while ((sig = eina_array_pop(interface->sign_of_signals)))
     eina_stringshare_del(sig);
   eina_array_free(interface->sign_of_signals);
   eina_hash_free(interface->properties);
   if (interface->props_changed)
     eina_array_free(interface->props_changed);
   if (interface->prop_invalidated)
     eina_array_free(interface->prop_invalidated);

   obj = interface->obj;
   l = eina_list_data_find_list(obj->iface_added, interface);
   if (l)
     {
        /* Adding and removing the interface in the same main loop iteration.
         * Let's not send any signal */
        obj->iface_added = eina_list_remove_list(obj->iface_added, l);
        if (!obj->iface_added && !obj->iface_removed && obj->idler_iface_changed)
          {
             ecore_idler_del(obj->idler_iface_changed);
             obj->idler_iface_changed = NULL;
          }
     }
   else
     {
        if (!obj->idler_iface_changed)
          {
             obj->idler_iface_changed = ecore_idler_add(
                _object_manager_changes_process, obj);
          }

        obj->iface_removed = eina_list_append(
           obj->iface_removed, eina_stringshare_ref(interface->name));
     }

   eina_stringshare_del(interface->name);
   free(interface);
}

static void _children_ifaces_add_removed_flush(Eldbus_Service_Object *obj)
{
   Eldbus_Service_Object *children;

   EINA_INLIST_FOREACH(obj->children, children)
     {
        /**
         * if there a object manager in some child
         * that object manager is responsible for they
         * children objects
         */
        if (!obj->objmanager)
          _children_ifaces_add_removed_flush(children);
     }

   if (obj->idler_iface_changed)
     {
        ecore_idler_del(obj->idler_iface_changed);
        _object_manager_changes_process(obj);
     }
}

static void
_object_free(Eldbus_Service_Object *obj)
{
   Eina_Iterator *iterator;
   Eldbus_Service_Interface *iface;

   if (obj->objmanager)
     {
        Eldbus_Service_Object *children;

        /**
         * Flush the iface_add/removed of all children objects
         * that this object is the ObjectManager
         */
        EINA_INLIST_FOREACH(obj->children, children)
          _children_ifaces_add_removed_flush(children);
     }

   iterator = eina_hash_iterator_data_new(obj->interfaces);
   EINA_ITERATOR_FOREACH(iterator, iface)
     _interface_free(iface);

   /**
    * Flush our iface_add/removed if this object are
    * children of some other path with ObjectManager
    */
   if (obj->idler_iface_changed)
     {
        ecore_idler_del(obj->idler_iface_changed);
        _object_manager_changes_process(obj);
     }

   while (obj->children)
     {
        Eldbus_Service_Object *child;
        child = EINA_INLIST_CONTAINER_GET(obj->children, Eldbus_Service_Object);
        obj->children = eina_inlist_remove(obj->children, obj->children);
        if (obj->parent)
          {
             obj->parent->children = eina_inlist_append(obj->parent->children,
							EINA_INLIST_GET(child));
             child->parent = obj->parent;
          }
        else
          {
             obj->conn->root_objs = eina_inlist_append(obj->conn->root_objs,
                                                       EINA_INLIST_GET(child));
             child->parent = NULL;
          }
     }

   if (obj->parent)
     obj->parent->children = eina_inlist_remove(obj->parent->children,
						EINA_INLIST_GET(obj));
   else
     obj->conn->root_objs = eina_inlist_remove(obj->conn->root_objs,
                                               EINA_INLIST_GET(obj));

   eldbus_data_del_all(&obj->data);

   eina_hash_free(obj->interfaces);
   eina_iterator_free(iterator);
   if (obj->introspection_data)
     eina_strbuf_free(obj->introspection_data);
   eina_stringshare_del(obj->path);
   free(obj);
}

static void
_on_connection_free(void *data, const void *dead_pointer EINA_UNUSED)
{
   Eldbus_Service_Object *obj = data;
   dbus_connection_unregister_object_path(obj->conn->dbus_conn, obj->path);
}

EAPI void
eldbus_service_interface_unregister(Eldbus_Service_Interface *iface)
{
   Eldbus_Service_Object *obj;
   ELDBUS_SERVICE_INTERFACE_CHECK(iface);
   obj = iface->obj;
   eina_hash_del(obj->interfaces, NULL, iface);
   _interface_free(iface);
   obj->introspection_dirty = EINA_TRUE;
}

EAPI void
eldbus_service_object_unregister(Eldbus_Service_Interface *iface)
{
   ELDBUS_SERVICE_INTERFACE_CHECK(iface);
   /*
    * It will be freed when _object_unregister() is called
    * by libdbus.
    */
   eldbus_connection_free_cb_del(iface->obj->conn, _on_connection_free, iface->obj);
   dbus_connection_unregister_object_path(iface->obj->conn->dbus_conn, iface->obj->path);
}

static void
_object_unregister(DBusConnection *conn EINA_UNUSED, void *user_data)
{
   Eldbus_Service_Object *obj = user_data;
   _object_free(obj);
}

static Eldbus_Message*
_eldbus_method_call(Eldbus_Method const* method, Eldbus_Service_Interface* iface, Eldbus_Message* msg)
{
  if(method->flags & ELDBUS_METHOD_FLAG_HAS_DATA)
    {
      Eldbus_Method_Data_Cb cb = (Eldbus_Method_Data_Cb)method->cb;
      return cb(method->data, iface, msg);
    }
  else
    {
      return method->cb(iface, msg);
    }
}

static DBusHandlerResult
_object_handler(DBusConnection *dbus_conn EINA_UNUSED, DBusMessage *msg, void *user_data)
{
   Eldbus_Service_Object *obj;
   Eldbus_Service_Interface *iface;
   const Eldbus_Method *method;
   Eldbus_Message *eldbus_msg, *reply;
   Eldbus_Connection *conn;
   const char* fallback_path = NULL;

   obj = user_data;
   if (!obj) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
   conn = obj->conn;

   if (obj->fallback)
     {
       fallback_path = eina_stringshare_add(obj->path);
       eina_stringshare_replace(&obj->path, dbus_message_get_path(msg));
     }

   DBG("Connection@%p Got message:\n"
          "  Type: %s\n"
          "  Path: %s\n"
          "  Interface: %s\n"
          "  Member: %s\n"
          "  Sender: %s", obj->conn,
          dbus_message_type_to_string(dbus_message_get_type(msg)),
          dbus_message_get_path(msg),
          dbus_message_get_interface(msg),
          dbus_message_get_member(msg),
          dbus_message_get_sender(msg));

   iface = eina_hash_find(obj->interfaces, dbus_message_get_interface(msg));
   if (!iface) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

   method = eina_hash_find(iface->methods, dbus_message_get_member(msg));
   if (!method) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

   eldbus_msg = eldbus_message_new(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(eldbus_msg, DBUS_HANDLER_RESULT_NEED_MEMORY);
   eldbus_msg->dbus_msg = msg;
   dbus_message_ref(eldbus_msg->dbus_msg);
   dbus_message_iter_init(eldbus_msg->dbus_msg, &eldbus_msg->iterator->dbus_iterator);

   eldbus_init();
   eldbus_connection_ref(conn);

   if (!_have_signature(method->in, eldbus_msg))
     reply = eldbus_message_error_new(eldbus_msg, DBUS_ERROR_INVALID_SIGNATURE,
                                          "See introspectable to know the expected signature");
   else
     {
       if (iface->obj)
         reply = _eldbus_method_call(method, iface, eldbus_msg);
       else
         {
           /* if iface does have obj it is some of FreeDesktop interfaces:
              Introspectable, Properties or ObjectManager */
           iface->obj = obj;
           reply = _eldbus_method_call(method, iface, eldbus_msg);
           iface->obj = NULL;
         }
     }

   eldbus_message_unref(eldbus_msg);
   if (reply)
     _eldbus_connection_send(conn, reply, NULL, NULL, -1);

   eldbus_connection_unref(conn);
   eldbus_shutdown();

   if (obj->fallback)
     {
        eina_stringshare_replace(&obj->path, fallback_path);
        eina_stringshare_del(fallback_path);
     }

   return DBUS_HANDLER_RESULT_HANDLED;
}

EAPI Eldbus_Connection *
eldbus_service_connection_get(const Eldbus_Service_Interface *iface)
{
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   return iface->obj->conn;
}

EAPI const char *
eldbus_service_object_path_get(const Eldbus_Service_Interface *iface)
{
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   return iface->obj->path;
}

EAPI Eldbus_Message *
eldbus_service_signal_new(const Eldbus_Service_Interface *iface, unsigned int signal_id)
{
   unsigned size;
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   size = eina_array_count(iface->sign_of_signals);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(signal_id < size, NULL);

   return eldbus_message_signal_new(iface->obj->path, iface->name,
                                   iface->signals[signal_id].name);
}

EAPI Eina_Bool
eldbus_service_signal_emit(const Eldbus_Service_Interface *iface, unsigned int signal_id, ...)
{
   Eldbus_Message *sig;
   va_list ap;
   Eina_Bool r;
   const char *signature;
   unsigned size;

   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   size = eina_array_count(iface->sign_of_signals);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(signal_id < size, EINA_FALSE);

   sig = eldbus_service_signal_new(iface, signal_id);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sig, EINA_FALSE);

   signature = eina_array_data_get(iface->sign_of_signals, signal_id);
   va_start(ap, signal_id);
   r = eldbus_message_arguments_vappend(sig, signature, ap);
   va_end(ap);
   if (!r)
     {
        eldbus_message_unref(sig);
        return EINA_FALSE;
     }

   eldbus_service_signal_send(iface, sig);
   return EINA_TRUE;
}

EAPI Eina_Bool
eldbus_service_signal_send(const Eldbus_Service_Interface *iface, Eldbus_Message *signal_msg)
{
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signal_msg, EINA_FALSE);
   _eldbus_connection_send(iface->obj->conn, signal_msg, NULL, NULL, -1);
   return EINA_TRUE;
}

EAPI void
eldbus_service_object_data_set(Eldbus_Service_Interface *iface, const char *key, const void *data)
{
   ELDBUS_SERVICE_INTERFACE_CHECK(iface);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   eldbus_data_set(&(iface->obj->data), key, data);
}

EAPI void *
eldbus_service_object_data_get(const Eldbus_Service_Interface *iface, const char *key)
{
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_get(&(((Eldbus_Service_Object *)iface->obj)->data), key);
}

EAPI void *
eldbus_service_object_data_del(Eldbus_Service_Interface *iface, const char *key)
{
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_del(&(((Eldbus_Service_Object *)iface->obj)->data), key);
}

EAPI Eina_Bool
eldbus_service_property_changed(const Eldbus_Service_Interface *interface, const char *name)
{
   Property *prop;
   Eldbus_Service_Interface *iface = (Eldbus_Service_Interface *)interface;
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);

   prop = eina_hash_find(iface->properties, name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop, EINA_FALSE);

   if (!iface->idler_propschanged)
     iface->idler_propschanged = ecore_idler_add(_idler_propschanged, iface);
   if (!iface->props_changed)
     iface->props_changed = eina_array_new(1);

   return eina_array_push(iface->props_changed, prop);
}

EAPI Eina_Bool
eldbus_service_property_invalidate_set(const Eldbus_Service_Interface *interface, const char *name, Eina_Bool is_invalidate)
{
   Property *prop;
   Eldbus_Service_Interface *iface = (Eldbus_Service_Interface *) interface;

   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);

   prop = eina_hash_find(iface->properties, name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop, EINA_FALSE);

   if (prop->is_invalidate == is_invalidate)
     return EINA_TRUE;

   prop->is_invalidate = is_invalidate;

   if (!iface->idler_propschanged)
     iface->idler_propschanged = ecore_idler_add(_idler_propschanged, iface);

   if (is_invalidate)
     {
        if (!iface->props_changed)
          iface->props_changed = eina_array_new(1);
        return eina_array_push(iface->props_changed, prop);
     }

   if (!iface->prop_invalidated)
     iface->prop_invalidated = eina_array_new(1);
   return eina_array_push(iface->prop_invalidated, prop);
}

EAPI Eina_Bool
eldbus_service_object_manager_attach(Eldbus_Service_Interface *iface)
{
   Eldbus_Service_Object *obj;
   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);

   obj = iface->obj;

   /* We already have it and we registered it ourselves */
   if (obj->objmanager)
     return EINA_TRUE;

   /* Ugh. User already registered the ObjectManager interface himself? */
   if (eina_hash_find(obj->interfaces, objmanager->name))
     return EINA_FALSE;

   if (!eina_hash_add(obj->interfaces, objmanager->name, objmanager))
     return EINA_FALSE;

   /*
    * Flush the iface_added and iface_removed, otherwise it could be sent
    * with path equal to our path rather than from the previous
    * ObjectManager
    */
   if (obj->idler_iface_changed)
     ecore_idler_del(obj->idler_iface_changed);
   _object_manager_changes_process(obj);

   obj->objmanager = objmanager;
   obj->introspection_dirty = EINA_TRUE;
   return EINA_TRUE;
}

EAPI Eina_Bool
eldbus_service_object_manager_detach(Eldbus_Service_Interface *iface)
{
   Eldbus_Service_Object *obj, *children;
   Eina_Bool ret;

   ELDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   obj = iface->obj;
   if (!obj->objmanager)
     return EINA_TRUE;

   /**
    * Flush the iface_add/removed of all children objects
    * that this object is the ObjectManager
    */
   EINA_INLIST_FOREACH(obj->children, children)
     _children_ifaces_add_removed_flush(children);

   ret = eina_hash_del(obj->interfaces, objmanager->name, NULL);
   obj->objmanager = NULL;
   obj->introspection_dirty = EINA_TRUE;
   return ret;
}
