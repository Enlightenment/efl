#include "edbus_private_types.h"
#include "edbus_private.h"

#define DBUS_ANNOTATION(name, value) \
        "<annotation" \
        " name=\"org.freedesktop.DBus." name "\"" \
        " value=\"" value "\"" \
        "/>"

#define DBUS_ANNOTATION_DEPRECATED  DBUS_ANNOTATION("Deprecated", "true")
#define DBUS_ANNOTATION_NOREPLY     DBUS_ANNOTATION("Method.NoReply", "true")

#define EDBUS_SERVICE_INTERFACE_CHECK(obj)                         \
  do                                                            \
    {                                                           \
       EINA_SAFETY_ON_NULL_RETURN(obj);                         \
       if (!EINA_MAGIC_CHECK(obj, EDBUS_SERVICE_INTERFACE_MAGIC))  \
         {                                                      \
            EINA_MAGIC_FAIL(obj, EDBUS_SERVICE_INTERFACE_MAGIC);   \
            return;                                             \
         }                                                      \
    }                                                           \
  while (0)

#define EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(obj, retval)                  \
  do                                                                    \
    {                                                                   \
       EINA_SAFETY_ON_NULL_RETURN_VAL(obj, retval);                     \
       if (!EINA_MAGIC_CHECK(obj, EDBUS_SERVICE_INTERFACE_MAGIC))          \
         {                                                              \
            EINA_MAGIC_FAIL(obj, EDBUS_SERVICE_INTERFACE_MAGIC);           \
            return retval;                                              \
         }                                                              \
    }                                                                   \
  while (0)


static void _object_unregister(DBusConnection *conn, void *user_data);
static DBusHandlerResult _object_handler(DBusConnection *conn, DBusMessage *message, void *user_data);
static void _object_free(EDBus_Service_Object *obj);
static void _interface_free(EDBus_Service_Interface *interface);
static void _on_connection_free(void *data, const void *dead_pointer);

static DBusObjectPathVTable vtable = {
  _object_unregister,
  _object_handler,
  NULL,
  NULL,
  NULL,
  NULL
};

EDBus_Service_Interface *introspectable;
EDBus_Service_Interface *properties_iface;

static void
_introspect_append_signal(Eina_Strbuf *buf, const EDBus_Signal *sig)
{
   int i;
   const char *part, *name;

   eina_strbuf_append_printf(buf, "<signal name=\"%s\">", sig->name);
   if (sig->flags & EDBUS_SIGNAL_FLAG_DEPRECATED)
     eina_strbuf_append(buf, DBUS_ANNOTATION_DEPRECATED);

   for (i = 0; &sig->args[i] && sig->args[i].signature; i++)
     {
        part = sig->args[i].signature;
        name = sig->args[i].name;

        if (name && name[0])
          eina_strbuf_append_printf(buf, "<arg type=\"%s\" name=\"%s\"/>",
                                    part, name);
        else
          eina_strbuf_append_printf(buf, "<arg type=\"%s\"/>", part);
     }
   eina_strbuf_append(buf, "</signal>");
}

static void
_instrospect_append_property(Eina_Strbuf *buf, const EDBus_Property *prop, const EDBus_Service_Interface *iface)
{
   eina_strbuf_append_printf(buf, "<property name=\"%s\" type=\"%s\" access=\"",
                             prop->name, prop->type);
   if (iface->get_func || prop->get_func)
     eina_strbuf_append(buf, "read");
   if (iface->set_func || prop->set_func)
     eina_strbuf_append(buf, "write");
   eina_strbuf_append(buf, "\">");

   if (prop->flags & EDBUS_PROPERTY_FLAG_DEPRECATED)
     eina_strbuf_append(buf, DBUS_ANNOTATION_DEPRECATED);

   eina_strbuf_append(buf, "</property>");
}

static void
_introspect_append_method(Eina_Strbuf *buf, const EDBus_Method *method)
{
   int i;
   const char *part, *name;

   eina_strbuf_append_printf(buf, "<method name=\"%s\">", method->member);
   if (method->flags & EDBUS_METHOD_FLAG_DEPRECATED)
     eina_strbuf_append(buf, DBUS_ANNOTATION_DEPRECATED);
   if (method->flags & EDBUS_METHOD_FLAG_NOREPLY)
     eina_strbuf_append(buf, DBUS_ANNOTATION_NOREPLY);

   for (i = 0; &method->in[i] && method->in[i].signature; i++)
     {
        part = method->in[i].signature;
        name = method->in[i].name;

        if (name && name[0])
          eina_strbuf_append_printf(buf,
                                    "<arg type=\"%s\" name=\"%s\" direction=\"in\"/>",
                                    part, name);
        else
          eina_strbuf_append_printf(buf, "<arg type=\"%s\" direction=\"in\"/>",
                                    part);
     }

   for (i = 0; &method->out[i] && method->out[i].signature; i++)
     {
        part = method->out[i].signature;
        name = method->out[i].name;

        if (name && name[0])
          eina_strbuf_append_printf(buf,
                                    "<arg type=\"%s\" name=\"%s\" direction=\"out\"/>",
                                    part, name);
        else
          eina_strbuf_append_printf(buf, "<arg type=\"%s\" direction=\"out\"/>",
                                    part);
     }
   eina_strbuf_append(buf, "</method>");
}

static void
_introspect_append_interface(Eina_Strbuf *buf, EDBus_Service_Interface *iface)
{
   EDBus_Method *method;
   EDBus_Property *prop;
   Eina_Iterator *iterator;
   unsigned short i;
   unsigned int size;

   eina_strbuf_append_printf(buf, "<interface name=\"%s\">", iface->name);
   iterator = eina_hash_iterator_data_new(iface->methods);
   EINA_ITERATOR_FOREACH(iterator, method)
     _introspect_append_method(buf, method);
   eina_iterator_free(iterator);

   size = eina_array_count(iface->sign_of_signals);
   for (i = 0; i < size; i++)
     _introspect_append_signal(buf, &iface->signals[i]);

   iterator = eina_hash_iterator_data_new(iface->properties);
   EINA_ITERATOR_FOREACH(iterator, prop)
     _instrospect_append_property(buf, prop, iface);
   eina_iterator_free(iterator);

   eina_strbuf_append(buf, "</interface>");
}

static EDBus_Message *
_cb_property_get(const EDBus_Service_Interface *piface, const EDBus_Message *msg)
{
   const char *propname, *iface_name;
   EDBus_Service_Object *obj = piface->obj;
   EDBus_Service_Interface *iface;
   EDBus_Property *prop;
   EDBus_Message *reply, *error_reply = NULL;
   EDBus_Message_Iter *main_iter, *variant;
   Eina_Bool ret;
   EDBus_Property_Get_Cb getter = NULL;

   if (!edbus_message_arguments_get(msg, "ss", &iface_name, &propname))
     return NULL;

   iface = eina_hash_find(obj->interfaces, iface_name);
   if (!iface)
     return edbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_INTERFACE,
                                    "Interface not found.");

   prop = eina_hash_find(iface->properties, propname);
   if (!prop) goto not_found;

   if (prop->get_func)
     getter = prop->get_func;
   else if (iface->get_func)
     getter = iface->get_func;

   if (!getter) goto not_found;

   reply = edbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);

   main_iter = edbus_message_iter_get(reply);
   variant = edbus_message_iter_container_new(main_iter, 'v', prop->type);

   ret = getter(iface, propname, variant, &error_reply);

   if (ret)
     {
        edbus_message_iter_container_close(main_iter, variant);
        return reply;
     }

   edbus_message_unref(reply);
   return error_reply;

not_found:
   return edbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_PROPERTY,
                                  "Property not found.");
}

static EDBus_Message *
_cb_property_getall(const EDBus_Service_Interface *piface, const EDBus_Message *msg)
{
   const char *iface_name;
   EDBus_Service_Object *obj = piface->obj;
   EDBus_Service_Interface *iface;
   Eina_Iterator *iterator;
   EDBus_Property *prop;
   EDBus_Message *reply, *error_reply;
   EDBus_Message_Iter *main_iter, *dict;

   if (!edbus_message_arguments_get(msg, "s", &iface_name))
     return NULL;

   iface = eina_hash_find(obj->interfaces, iface_name);
   if (!iface)
     return edbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_INTERFACE,
                                    "Interface not found.");

   reply = edbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);
   main_iter = edbus_message_iter_get(reply);
   if (!edbus_message_iter_arguments_set(main_iter, "a{sv}", &dict))
     {
        edbus_message_unref(reply);
        return NULL;
     }

   iterator = eina_hash_iterator_data_new(iface->properties);
   EINA_ITERATOR_FOREACH(iterator, prop)
     {
        EDBus_Message_Iter *entry, *var;
        Eina_Bool ret;
        EDBus_Property_Get_Cb getter = NULL;

        if (prop->get_func)
          getter = prop->get_func;
        else if (iface->get_func)
          getter = iface->get_func;

        if (!getter)
          continue;

        if (!edbus_message_iter_arguments_set(dict, "{sv}", &entry))
          continue;

        edbus_message_iter_basic_append(entry, 's', prop->name);
        var = edbus_message_iter_container_new(entry, 'v', prop->type);

        ret = getter(iface, prop->name, var, &error_reply);

        if (!ret)
          {
             edbus_message_unref(reply);
             reply = error_reply;
             goto end;
          }

        edbus_message_iter_container_close(entry, var);
        edbus_message_iter_container_close(dict, entry);
     }
   edbus_message_iter_container_close(main_iter, dict);

end:
   eina_iterator_free(iterator);
   return reply;
}

static EDBus_Message *
_cb_property_set(const EDBus_Service_Interface *piface, const EDBus_Message *msg)
{
   const char *propname, *iface_name;
   EDBus_Service_Object *obj = piface->obj;
   EDBus_Service_Interface *iface;
   EDBus_Property *prop;
   EDBus_Message *reply;
   EDBus_Message_Iter *main_iter;
   EDBus_Property_Set_Cb setter = NULL;

   main_iter = edbus_message_iter_get(msg);
   if (!edbus_message_iter_get_and_next(main_iter, 's', &iface_name) ||
       !edbus_message_iter_get_and_next(main_iter, 's', &propname))
     return NULL;

   dbus_message_iter_init(msg->dbus_msg,
                          &main_iter->dbus_iterator);

   iface = eina_hash_find(obj->interfaces, iface_name);
   if (!iface)
     return edbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_INTERFACE,
                                    "Interface not found.");

   prop = eina_hash_find(iface->properties, propname);
   if (!prop)
     return edbus_message_error_new(msg, DBUS_ERROR_UNKNOWN_PROPERTY,
                                    "Property not found.");

   if (prop->set_func)
     setter = prop->set_func;
   else if (iface->set_func)
     setter = iface->set_func;

   if (!setter)
     return edbus_message_error_new(msg, DBUS_ERROR_PROPERTY_READ_ONLY,
                                    "This property is read only");
   reply = setter(iface, propname, msg);
   return reply;
}

static EDBus_Message *
cb_introspect(const EDBus_Service_Interface *_iface, const EDBus_Message *message)
{
   EDBus_Service_Object *obj = _iface->obj;
   EDBus_Message *reply = edbus_message_method_return_new(message);
   if (obj->introspection_dirty || !obj->introspection_data)
     {
        Eina_Iterator *iterator;
        EDBus_Service_Interface *iface;

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
        eina_strbuf_append(obj->introspection_data, "</node>");

        obj->introspection_dirty = EINA_FALSE;
     }

   edbus_message_arguments_set(reply, "s", eina_strbuf_string_get(obj->introspection_data));
   return reply;
}

static const EDBus_Method introspect = {
   "Introspect", NULL, EDBUS_ARGS({ "s", "xml" }), cb_introspect
};

static void
_introspectable_create(void)
{
   introspectable = calloc(1, sizeof(EDBus_Service_Interface));
   EINA_SAFETY_ON_NULL_RETURN(introspectable);

   EINA_MAGIC_SET(introspectable, EDBUS_SERVICE_INTERFACE_MAGIC);
   introspectable->sign_of_signals = eina_array_new(1);
   introspectable->properties = eina_hash_string_small_new(NULL);
   introspectable->name = eina_stringshare_add("org.freedesktop.DBus.Introspectable");
   introspectable->methods = eina_hash_string_small_new(NULL);

   eina_hash_add(introspectable->methods, introspect.member, &introspect);
}

static void
_default_interfaces_free(void)
{
   eina_hash_free(introspectable->methods);
   eina_hash_free(introspectable->properties);
   eina_stringshare_del(introspectable->name);
   eina_array_free(introspectable->sign_of_signals);
   free(introspectable);

   eina_hash_free(properties_iface->methods);
   eina_hash_free(properties_iface->properties);
   eina_array_free(properties_iface->sign_of_signals);
   free(properties_iface);
}

static const EDBus_Method _property_methods[] = {
   {
    "Get", EDBUS_ARGS({"s", "interface"}, {"s", "property"}),
    EDBUS_ARGS({"v", "value"}), _cb_property_get
   },
   {
    "Set", EDBUS_ARGS({"s", "interface"}, {"s", "property"}, {"v", "value"}),
    NULL, _cb_property_set
   },
   {
    "GetAll", EDBUS_ARGS({"s", "interface"}), EDBUS_ARGS({"a{sv}", "props"}),
    _cb_property_getall
   }
};

static const EDBus_Signal _properties_signals[] = {
   {
    "PropertiesChanged",
    EDBUS_ARGS({"s", "interface"}, {"a{sv}", "changed_properties"}, {"as", "invalidated_properties"})
   }
};

static void
_properties_create(void)
{
   properties_iface = calloc(1, sizeof(EDBus_Service_Interface));
   if (!properties_iface) return;

   properties_iface->sign_of_signals = eina_array_new(1);
   properties_iface->properties =  eina_hash_string_small_new(NULL);
   properties_iface->name = EDBUS_FDO_INTERFACE_PROPERTIES;
   properties_iface->methods = eina_hash_string_small_new(NULL);
   EINA_MAGIC_SET(properties_iface, EDBUS_SERVICE_INTERFACE_MAGIC);

   eina_hash_add(properties_iface->methods, _property_methods[0].member,
                 &_property_methods[0]);
   eina_hash_add(properties_iface->methods, _property_methods[1].member,
                 &_property_methods[1]);
   eina_hash_add(properties_iface->methods, _property_methods[2].member,
                 &_property_methods[2]);

   properties_iface->signals = _properties_signals;
   eina_array_push(properties_iface->sign_of_signals, "sa{sv}as");
}

Eina_Bool
edbus_service_init(void)
{
   _introspectable_create();
   EINA_SAFETY_ON_NULL_RETURN_VAL(introspectable, EINA_FALSE);
   _properties_create();
   EINA_SAFETY_ON_NULL_RETURN_VAL(properties_iface, EINA_FALSE);

   return EINA_TRUE;
}

void
edbus_service_shutdown(void)
{
   _default_interfaces_free();
}

static EDBus_Service_Object *
_edbus_service_object_add(EDBus_Connection *conn, const char *path)
{
   EDBus_Service_Object *obj;

   obj = calloc(1, sizeof(EDBus_Service_Object));
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   if (!dbus_connection_register_object_path(conn->dbus_conn, path, &vtable,
                                             obj))
     {
        free(obj);
        return NULL;
     }

   obj->conn = conn;
   obj->path = eina_stringshare_add(path);
   obj->interfaces = eina_hash_string_superfast_new(NULL);
   edbus_connection_cb_free_add(conn, _on_connection_free, obj);

   eina_hash_add(obj->interfaces, introspectable->name, introspectable);
   eina_hash_add(obj->interfaces, properties_iface->name, properties_iface);

   return obj;
}

static EDBus_Service_Interface *
_edbus_service_interface_add(EDBus_Service_Object *obj, const char *interface)
{
   EDBus_Service_Interface *iface;

   iface = eina_hash_find(obj->interfaces, interface);
   if (iface) return iface;

   iface = calloc(1, sizeof(EDBus_Service_Interface));
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);

   EINA_MAGIC_SET(iface, EDBUS_SERVICE_INTERFACE_MAGIC);
   iface->name = eina_stringshare_add(interface);
   iface->methods = eina_hash_string_superfast_new(NULL);
   iface->properties = eina_hash_string_superfast_new(NULL);
   iface->obj = obj;
   eina_hash_add(obj->interfaces, iface->name, iface);
   return iface;
}

static Eina_Bool
_have_signature(const EDBus_Arg_Info *args, EDBus_Message *msg)
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
_edbus_service_method_add(EDBus_Service_Interface *interface, EDBus_Method *method)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_hash_find(interface->methods,
                                  method->member), EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method->member, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method->cb, EINA_FALSE);

   eina_hash_add(interface->methods, method->member, method);
   return EINA_TRUE;
}

static Eina_Bool
_edbus_service_property_add(EDBus_Service_Interface *interface, EDBus_Property *property)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_hash_find(interface->properties,
                                  property->name), EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(property->type, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(
            dbus_signature_validate_single(property->type, NULL), EINA_FALSE);

   return eina_hash_add(interface->properties, property->name, property);
}

EAPI EDBus_Service_Interface *
edbus_service_interface_register(EDBus_Connection *conn, const char *path, const EDBus_Service_Interface_Desc *desc)
{
   EDBus_Service_Object *obj;
   EDBus_Service_Interface *iface;
   EDBus_Method *method;
   EDBus_Property *property;
   unsigned short i, z;
   Eina_Strbuf *buf = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->interface, EINA_FALSE);

   if (!dbus_connection_get_object_path_data(conn->dbus_conn, path,
                                             (void*)&obj))
     {
        ERR("Invalid object path");
        return NULL;
     }

   if (obj == NULL)
     obj = _edbus_service_object_add(conn, path);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   iface = _edbus_service_interface_add(obj, desc->interface);
   if (!iface)
     {
        if (eina_hash_population(obj->interfaces) < 2)
           _object_free(obj);
        return NULL;
     }

   for (method = (EDBus_Method *)desc->methods; method && method->member; method++)
     _edbus_service_method_add(iface, method);

   if (!iface->sign_of_signals)
     iface->sign_of_signals = eina_array_new(1);
   for (i = 0; &desc->signals[i] && desc->signals[i].name; i++)
     {
        buf = eina_strbuf_new();
        for (z = 0;
             &desc->signals[i].args[z] && desc->signals[i].args[z].signature;
             z++)
          eina_strbuf_append(buf, desc->signals[i].args[z].signature);

        if (!dbus_signature_validate(eina_strbuf_string_get(buf), NULL))
          {
             ERR("Signal with invalid signature: interface=%s signal=%s",
                 iface->name, desc->signals[i].name);
             eina_strbuf_free(buf);
             continue;
          }

	eina_array_push(iface->sign_of_signals,
			eina_stringshare_add(eina_strbuf_string_get(buf)));
	eina_strbuf_free(buf);
     }
   iface->signals = desc->signals;

   for (property = (EDBus_Property *)desc->properties;
        property && property->name; property++)
     _edbus_service_property_add(iface, property);

   iface->get_func = desc->default_get;
   iface->set_func = desc->default_set;

   return iface;
}

static void
_interface_free(EDBus_Service_Interface *interface)
{
   unsigned size, i;
   if (interface == introspectable || interface == properties_iface) return;

   eina_hash_free(interface->methods);
   eina_stringshare_del(interface->name);
   size = eina_array_count(interface->sign_of_signals);
   for (i = 0; i < size; i++)
     eina_stringshare_del(eina_array_data_get(interface->sign_of_signals, i));
   eina_array_free(interface->sign_of_signals);
   eina_hash_free(interface->properties);
   if (interface->props_changed)
     eina_array_free(interface->props_changed);
   if (interface->idler_propschanged)
     ecore_idler_del(interface->idler_propschanged);
   free(interface);
}

static void
_object_free(EDBus_Service_Object *obj)
{
   Eina_Iterator *iterator;
   EDBus_Service_Interface *iface;

   iterator = eina_hash_iterator_data_new(obj->interfaces);
   EINA_ITERATOR_FOREACH(iterator, iface)
     _interface_free(iface);

   edbus_data_del_all(&obj->data);

   eina_hash_free(obj->interfaces);
   eina_iterator_free(iterator);
   if (obj->introspection_data)
     eina_strbuf_free(obj->introspection_data);
   eina_stringshare_del(obj->path);
   free(obj);
}

static void
_on_connection_free(void *data, const void *dead_pointer)
{
   EDBus_Service_Object *obj = data;
   dbus_connection_unregister_object_path(obj->conn->dbus_conn, obj->path);
}

EAPI void
edbus_service_interface_unregister(EDBus_Service_Interface *iface)
{
   EDBUS_SERVICE_INTERFACE_CHECK(iface);
   eina_hash_del(iface->obj->interfaces, NULL, iface);
   if (eina_hash_population(iface->obj->interfaces) < 3)
     edbus_service_object_unregister(iface);
   _interface_free(iface);
}

EAPI void
edbus_service_object_unregister(EDBus_Service_Interface *iface)
{
   EDBUS_SERVICE_INTERFACE_CHECK(iface);
   /*
    * It will be freed when _object_unregister() is called
    * by libdbus.
    */
   edbus_connection_cb_free_del(iface->obj->conn, _on_connection_free, iface->obj);
   dbus_connection_unregister_object_path(iface->obj->conn->dbus_conn, iface->obj->path);
}

static void
_object_unregister(DBusConnection *conn, void *user_data)
{
   EDBus_Service_Object *obj = user_data;
   _object_free(obj);
}

static DBusHandlerResult
_object_handler(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
   EDBus_Service_Object *obj;
   EDBus_Service_Interface *iface;
   EDBus_Method *method;
   EDBus_Message *edbus_msg;
   EDBus_Message *reply;

   obj = user_data;
   if (!obj) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

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

   edbus_msg = edbus_message_new(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(edbus_msg, DBUS_HANDLER_RESULT_NEED_MEMORY);
   edbus_msg->dbus_msg = msg;
   dbus_message_iter_init(edbus_msg->dbus_msg, &edbus_msg->iterator->dbus_iterator);

   if (!_have_signature(method->in, edbus_msg))
     {
        reply = edbus_message_error_new(edbus_msg,
                                        DBUS_ERROR_INVALID_SIGNATURE,
                                        "See introspectable to know the expected signature");
     }
   else
     {
        if (iface->obj)
          reply = method->cb(iface, edbus_msg);
        else
          {
             //if iface does have obj it is some of FreeDesktop interfaces:
             //Introspectable, Properties...
             EDBus_Service_Interface *cpy;
             cpy = calloc(1, sizeof(EDBus_Service_Interface));
             if (!cpy)
	       {
	          dbus_message_ref(edbus_msg->dbus_msg);
                  edbus_message_unref(edbus_msg);
                  return DBUS_HANDLER_RESULT_NEED_MEMORY;
               }
             cpy->obj = obj;
             reply = method->cb(cpy, edbus_msg);
             free(cpy);
          }
     }

   dbus_message_ref(edbus_msg->dbus_msg);
   edbus_message_unref(edbus_msg);
   if (!reply) return DBUS_HANDLER_RESULT_HANDLED;

   _edbus_connection_send(obj->conn, reply, NULL, NULL, -1);
   edbus_message_unref(reply);

   return DBUS_HANDLER_RESULT_HANDLED;
}

EAPI EDBus_Connection *
edbus_service_connection_get(const EDBus_Service_Interface *iface)
{
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   return iface->obj->conn;
}

EAPI const char *
edbus_service_object_path_get(const EDBus_Service_Interface *iface)
{
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   return iface->obj->path;
}

EAPI EDBus_Message *
edbus_service_signal_new(EDBus_Service_Interface *iface, unsigned int signal_id)
{
   unsigned size;
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   size = eina_array_count(iface->sign_of_signals);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(signal_id < size, EINA_FALSE);

   return edbus_message_signal_new(iface->obj->path, iface->name,
                                   iface->signals[signal_id].name);
}

EAPI Eina_Bool
edbus_service_signal_emit(EDBus_Service_Interface *iface, unsigned int signal_id, ...)
{
   EDBus_Message *sig;
   va_list ap;
   Eina_Bool r;
   const char *signature;
   unsigned size;

   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   size = eina_array_count(iface->sign_of_signals);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(signal_id < size, EINA_FALSE);

   sig = edbus_service_signal_new(iface, signal_id);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sig, EINA_FALSE);

   signature = eina_array_data_get(iface->sign_of_signals, signal_id);
   va_start(ap, signal_id);
   r = edbus_message_arguments_vset(sig, signature, ap);
   va_end(ap);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(r, EINA_FALSE);

   edbus_service_signal_send(iface, sig);
   edbus_message_unref(sig);
   return EINA_TRUE;
}

EAPI Eina_Bool
edbus_service_signal_send(EDBus_Service_Interface *iface, EDBus_Message *signal_msg)
{
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signal_msg, EINA_FALSE);
   _edbus_connection_send(iface->obj->conn, signal_msg, NULL, NULL, -1);
   return EINA_TRUE;
}

EAPI void
edbus_service_object_data_set(EDBus_Service_Interface *iface, const char *key, const void *data)
{
   EDBUS_SERVICE_INTERFACE_CHECK(iface);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   edbus_data_set(&(iface->obj->data), key, data);
}

EAPI void *
edbus_service_object_data_get(const EDBus_Service_Interface *iface, const char *key)
{
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return edbus_data_get(&(((EDBus_Service_Object *)iface->obj)->data), key);
}

EAPI void *
edbus_service_object_data_del(EDBus_Service_Interface *iface, const char *key)
{
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return edbus_data_del(&(((EDBus_Service_Object *)iface->obj)->data), key);
}

static Eina_Bool
_idler_propschanged(void *data)
{
   EDBus_Service_Interface *iface = data;
   EDBus_Message *msg;
   EDBus_Message_Iter *main_iter, *dict, *array_invalidate;
   Eina_Hash *added = NULL;
   EDBus_Property *prop;

   iface->idler_propschanged = NULL;

   added = eina_hash_string_small_new(NULL);
   msg = edbus_message_signal_new(iface->obj->path, properties_iface->name,
                                  properties_iface->signals[0].name);
   EINA_SAFETY_ON_NULL_GOTO(msg, error);

   main_iter = edbus_message_iter_get(msg);
   if (!edbus_message_iter_arguments_set(main_iter, "sa{sv}", iface->name, &dict))
     {
        edbus_message_unref(msg);
        goto error;
     }

   if (!iface->props_changed)
     goto invalidate;
   while ((prop = eina_array_pop(iface->props_changed)))
     {
        EDBus_Message_Iter *entry, *var;
        EDBus_Message *error_reply;
        Eina_Bool ret;
        EDBus_Property_Get_Cb getter = NULL;

        if (eina_hash_find(added, prop->name))
          continue;
        eina_hash_add(added, prop->name, prop);

        if (prop->get_func)
          getter = prop->get_func;
        else if (iface->get_func)
          getter = iface->get_func;

        if (!getter)
          continue;

        EINA_SAFETY_ON_FALSE_GOTO(
                edbus_message_iter_arguments_set(dict, "{sv}", &entry), error);

        edbus_message_iter_basic_append(entry, 's', prop->name);
        var = edbus_message_iter_container_new(entry, 'v', prop->type);

        ret = getter(iface, prop->name, var, &error_reply);

        if (!ret)
          {
             const char *errorname, *errormsg;
             if (error_reply &&
                 edbus_message_error_get(error_reply, &errorname, &errormsg))
               ERR("%s %s", errorname, errormsg);

             edbus_message_unref(msg);
             if (error_reply) edbus_message_unref(error_reply);
             goto error;
          }

        edbus_message_iter_container_close(entry, var);
        edbus_message_iter_container_close(dict, entry);
     }
invalidate:
   edbus_message_iter_container_close(main_iter, dict);

   edbus_message_iter_arguments_set(main_iter, "as", &array_invalidate);
   edbus_message_iter_container_close(main_iter, array_invalidate);

   edbus_service_signal_send(iface, msg);
   edbus_message_unref(msg);
error:
   if (added)
     eina_hash_free(added);
   if (iface->props_changed)
     eina_array_flush(iface->props_changed);
   return ECORE_CALLBACK_CANCEL;
}

EAPI Eina_Bool
edbus_service_property_changed(EDBus_Service_Interface *iface, const char *name)
{
   EDBus_Property *prop;
   EDBUS_SERVICE_INTERFACE_CHECK_RETVAL(iface, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);

   prop = eina_hash_find(iface->properties, name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop, EINA_FALSE);

   if (!iface->idler_propschanged)
     {
        iface->idler_propschanged = ecore_idler_add(_idler_propschanged, iface);
        if (!iface->props_changed)
          iface->props_changed = eina_array_new(1);
     }

   return eina_array_push(iface->props_changed, prop);
}
