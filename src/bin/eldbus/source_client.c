#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "codegen.h"

static const char *code_prefix = NULL;
static char buffer[4028];

static const char *
null_or_zero(const char *type)
{
   if (type[0] == 's' || type[0] == 'o' || type[0] == 'v' || type[1])
     return "NULL";
   return "0";
}

static const char *
prefix_append(const char *text)
{
   if (code_prefix)
     {
        sprintf(buffer, "%s_%s", code_prefix, text);
        return buffer;
     }
   return text;
}

static const char *
dbus_type2c_type2(const char *dbus_type, Eina_Bool with_const)
{
   switch (dbus_type[0])
     {
      case 's'://string
      case 'o'://object path
      case 'g'://signature
        {
           if (with_const)
             return "const char *";
           else
             return "char *";
        }
      case 'h'://file descriptor
      case 'i'://int
        return "int ";
      case 'y'://byte
        return "unsigned char ";
      case 'b'://bool
        return "Eina_Bool ";
      case 'n'://int16
        return "short int ";
      case 'q'://uint16
        return "unsigned short int ";
      case 'u'://uint32
        return "unsigned int ";
      case 'x'://int64
        return "int64_t ";
      case 't'://uint64
        return "uint64_t ";
      case 'd'://double
        return "double ";
      case 'a'://array
      case 'v'://variant
      case '{'://dict
      case '('://struct
        return "Eldbus_Message_Iter *";
      default:
        {
           printf("Error type not handled: %c\n", dbus_type[0]);
           return NULL;
        }
     }
}

static const char *
dbus_type2c_type(const char *dbus_type)
{
   return dbus_type2c_type2(dbus_type, EINA_TRUE);
}

static void
source_client_complex_method_call_generate(const DBus_Method *method, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   DBus_Arg *arg;
   Eina_Strbuf *full_signature = eina_strbuf_new();

   EINA_INLIST_FOREACH(method->args, arg)
     {
        if (arg->direction == 'o')
          continue;
        eina_strbuf_append(full_signature, arg->type);
     }

   if (method->no_reply)
     {
        eina_strbuf_append_printf(h, "void %s_call(Eldbus_Proxy *proxy, Eina_Value *args);\n", prefix_append(method->c_name));

        eina_strbuf_append_printf(c_code, "\nvoid \n%s_call(Eldbus_Proxy *proxy, Eina_Value *args)\n{\n", prefix_append(method->c_name));
        eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN(proxy);\n");
        eina_strbuf_append_printf(c_code, "   Eldbus_Message *msg = eldbus_proxy_method_call_new(proxy, \"%s\");\n", method->name);
        eina_strbuf_append_printf(c_code, "   if (!eldbus_message_from_eina_value(\"%s\", msg, args))\n", eina_strbuf_string_get(full_signature));
        eina_strbuf_append_printf(c_code, "     {\n");
        eina_strbuf_append_printf(c_code, "        ERR(\"Error: Filling message from eina value.\");\n");
        eina_strbuf_append_printf(c_code, "        return;\n");
        eina_strbuf_append_printf(c_code, "     }\n");
        eina_strbuf_append_printf(c_code, "   eldbus_proxy_send(proxy, msg, %s, NULL, NULL, -1);\n", method->cb_name);
        eina_strbuf_append_printf(c_code, "}\n");
        goto end;
     }

   eina_strbuf_append_printf(h, "Eldbus_Pending *%s_call", prefix_append(method->c_name));
   eina_strbuf_append_printf(h, "(Eldbus_Proxy *proxy, %s cb, const void *data, Eina_Value *args);\n", prefix_append(method->function_cb));

   eina_strbuf_append_printf(c_code, "\nEldbus_Pending *\n%s_call(", prefix_append(method->c_name));
   eina_strbuf_append_printf(c_code, "Eldbus_Proxy *proxy, %s cb, const void *data, Eina_Value *args)\n{\n", prefix_append(method->function_cb));
   eina_strbuf_append_printf(c_code, "   Eldbus_Message *msg;\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Pending *p;\n");
   eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);\n");
   eina_strbuf_append_printf(c_code, "   msg = eldbus_proxy_method_call_new(proxy, \"%s\");\n", method->name);
   eina_strbuf_append_printf(c_code, "   if (!eldbus_message_from_eina_value(\"%s\", msg, args))\n", eina_strbuf_string_get(full_signature));
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        ERR(\"Error: Filling message from eina value.\");\n");
   eina_strbuf_append_printf(c_code, "        return NULL;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append_printf(c_code, "   p = eldbus_proxy_send(proxy, msg, %s, cb, -1);\n", method->cb_name);
   eina_strbuf_append_printf(c_code, "   if (data)\n");
   eina_strbuf_append_printf(c_code, "     eldbus_pending_data_set(p, \"__user_data\", data);\n");
   eina_strbuf_append_printf(c_code, "   eldbus_pending_data_set(p, \"__proxy\", proxy);\n");
   eina_strbuf_append_printf(c_code, "   return p;\n");
   eina_strbuf_append_printf(c_code, "}\n");

end:
   eina_strbuf_free(full_signature);
}

static void
source_client_simple_method_call_no_reply_generate(const DBus_Method *method, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   DBus_Arg *arg;
   Eina_Strbuf *full_signature = eina_strbuf_new();
   Eina_Strbuf *args_call = eina_strbuf_new();

   eina_strbuf_append_printf(h, "void %s_call(Eldbus_Proxy *proxy", prefix_append(method->c_name));
   eina_strbuf_append_printf(c_code, "\nvoid\n%s_call(Eldbus_Proxy *proxy", prefix_append(method->c_name));

   EINA_INLIST_FOREACH(method->args, arg)
     {
        if (arg->direction == 'o')
          continue;
        eina_strbuf_append(full_signature, arg->type);
        eina_strbuf_append_printf(h, ", %s%s", dbus_type2c_type(arg->type), arg->c_name);
        eina_strbuf_append_printf(c_code, ", %s%s", dbus_type2c_type(arg->type), arg->c_name);
        eina_strbuf_append_printf(args_call, ", %s", arg->c_name);
     }
   eina_strbuf_append_printf(h, ");\n");
   eina_strbuf_append_printf(c_code, ")\n{\n");

   eina_strbuf_append_printf(c_code, "   Eldbus_Message *msg;\n");
   eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN(proxy);\n");
   eina_strbuf_append_printf(c_code, "   msg = eldbus_proxy_method_call_new(proxy, \"%s\");\n", method->name);
   eina_strbuf_append_printf(c_code, "   if (!eldbus_message_arguments_append(msg, \"%s\"%s))\n", eina_strbuf_string_get(full_signature), eina_strbuf_string_get(args_call));
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        ERR(\"Error: Filling message.\");\n");
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append_printf(c_code, "   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);\n");
   eina_strbuf_append_printf(c_code, "}\n");

   eina_strbuf_free(full_signature);
   eina_strbuf_free(args_call);
}

static void
source_client_simple_method_call_generate(const DBus_Method *method, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   DBus_Arg *arg;
   Eina_Strbuf *full_signature = eina_strbuf_new();
   Eina_Strbuf *args_call = eina_strbuf_new();

   eina_strbuf_append_printf(h, "Eldbus_Pending *%s_call", prefix_append(method->c_name));
   eina_strbuf_append_printf(h, "(Eldbus_Proxy *proxy, %s cb, const void *data", prefix_append(method->function_cb));
   eina_strbuf_append_printf(c_code, "\nEldbus_Pending *\n%s_call", prefix_append(method->c_name));
   eina_strbuf_append_printf(c_code, "(Eldbus_Proxy *proxy, %s cb, const void *data", prefix_append(method->function_cb));

   EINA_INLIST_FOREACH(method->args, arg)
     {
        if (arg->direction == 'o')
          continue;
        eina_strbuf_append(full_signature, arg->type);
        eina_strbuf_append_printf(h, ", %s%s", dbus_type2c_type(arg->type), arg->c_name);
        eina_strbuf_append_printf(c_code, ", %s%s", dbus_type2c_type(arg->type), arg->c_name);
        eina_strbuf_append_printf(args_call, ", %s", arg->c_name);
     }
   eina_strbuf_append_printf(h, ");\n");
   eina_strbuf_append_printf(c_code,")\n{\n");

   eina_strbuf_append_printf(c_code, "   Eldbus_Message *msg;\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Pending *p;\n");
   eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);\n");
   eina_strbuf_append_printf(c_code, "   msg = eldbus_proxy_method_call_new(proxy, \"%s\");\n", method->name);
   eina_strbuf_append_printf(c_code, "   if (!eldbus_message_arguments_append(msg, \"%s\"%s))\n", eina_strbuf_string_get(full_signature), eina_strbuf_string_get(args_call));
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        ERR(\"Error: Filling message.\");\n");
   eina_strbuf_append_printf(c_code, "        return NULL;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append_printf(c_code, "   p = eldbus_proxy_send(proxy, msg, %s, cb, -1);\n", method->cb_name);
   eina_strbuf_append_printf(c_code, "   if (data)\n");
   eina_strbuf_append_printf(c_code, "     eldbus_pending_data_set(p, \"__user_data\", data);\n");
   eina_strbuf_append_printf(c_code, "   eldbus_pending_data_set(p, \"__proxy\", proxy);\n");
   eina_strbuf_append_printf(c_code, "   return p;\n");
   eina_strbuf_append_printf(c_code, "}\n");

   eina_strbuf_free(full_signature);
   eina_strbuf_free(args_call);
}

static void
source_client_complex_method_callback_generate(const DBus_Method *method, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   eina_strbuf_append_printf(h, "typedef void (*%s)(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error, Eina_Value *args);\n", prefix_append(method->function_cb));

   eina_strbuf_append_printf(c_code, "\nstatic void\n%s(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)\n{\n", method->cb_name);
   eina_strbuf_append_printf(c_code, "   void *user_data = eldbus_pending_data_del(pending, \"__user_data\");\n");
   eina_strbuf_append_printf(c_code, "   %s cb = data;\n", prefix_append(method->function_cb));
   eina_strbuf_append_printf(c_code, "   const char *error, *error_msg;\n");
   eina_strbuf_append_printf(c_code, "   Eina_Value *value;\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, \"__proxy\");\n");
   eina_strbuf_append_printf(c_code, "   if (eldbus_message_error_get(msg, &error, &error_msg))\n");
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {error, error_msg};\n");
   eina_strbuf_append_printf(c_code, "        cb(proxy, user_data, pending, &error_info, NULL);\n");
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append_printf(c_code, "   value = eldbus_message_to_eina_value(msg);\n");
   eina_strbuf_append_printf(c_code, "   cb(proxy, user_data, pending, NULL, value);\n");
   eina_strbuf_append_printf(c_code, "   eina_value_free(value);\n");
   eina_strbuf_append_printf(c_code, "   return;\n");
   eina_strbuf_append_printf(c_code, "}\n");
}

static void
source_client_simple_method_callback_generate(const DBus_Method *method, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   Eina_Strbuf *full_signature = eina_strbuf_new();
   DBus_Arg *arg;
   Eina_Strbuf *end_cb = eina_strbuf_new();
   Eina_Strbuf *arguments_get = eina_strbuf_new();

   eina_strbuf_append_printf(h, "typedef void (*%s)(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error", prefix_append(method->function_cb));

   eina_strbuf_append_printf(c_code, "\nstatic void\n%s(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)\n{\n", method->cb_name);
   eina_strbuf_append_printf(c_code, "   void *user_data = eldbus_pending_data_del(pending, \"__user_data\");\n");
   eina_strbuf_append_printf(c_code, "   %s cb = data;\n", prefix_append(method->function_cb));
   eina_strbuf_append_printf(c_code, "   const char *error, *error_msg;\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, \"__proxy\");\n");

   EINA_INLIST_FOREACH(method->args, arg)
     {
        if (arg->direction != 'o')
          continue;
        eina_strbuf_append(full_signature, arg->type);
        eina_strbuf_append_printf(h, ", %s%s", dbus_type2c_type(arg->type), arg->c_name);
        eina_strbuf_append_printf(c_code, "   %s%s = %s;\n", dbus_type2c_type(arg->type), arg->c_name, null_or_zero(arg->type));
        eina_strbuf_append_printf(end_cb, ", %s", arg->c_name);
        eina_strbuf_append_printf(arguments_get, ", &%s", arg->c_name);
     }
   eina_strbuf_append_printf(h, ");\n");

   eina_strbuf_append_printf(c_code, "   if (eldbus_message_error_get(msg, &error, &error_msg))\n");
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {error, error_msg};\n");
   eina_strbuf_append_printf(c_code, "        cb(proxy, user_data, pending, &error_info%s);\n", eina_strbuf_string_get(end_cb));
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");

   eina_strbuf_append_printf(c_code, "   if (!eldbus_message_arguments_get(msg, \"%s\"%s))\n", eina_strbuf_string_get(full_signature), eina_strbuf_string_get(arguments_get));
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {\"\", \"\"};\n");
   eina_strbuf_append_printf(c_code, "        ERR(\"Error: Getting arguments from message.\");\n");
   eina_strbuf_append_printf(c_code, "        cb(proxy, user_data, pending, &error_info%s);\n", eina_strbuf_string_get(end_cb));
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append_printf(c_code, "   cb(proxy, user_data, pending, NULL%s);\n", eina_strbuf_string_get(end_cb));
   eina_strbuf_append_printf(c_code, "   return;\n");
   eina_strbuf_append_printf(c_code, "}\n");

   eina_strbuf_free(full_signature);
   eina_strbuf_free(end_cb);
   eina_strbuf_free(arguments_get);
}

static void
source_client_method_generate(const DBus_Method *method, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   if (!method->no_reply)
     {
        if (method->out_complex)
          source_client_complex_method_callback_generate(method, c_code, h);
        else
          source_client_simple_method_callback_generate(method, c_code, h);
     }

   if (method->in_complex)
     source_client_complex_method_call_generate(method, c_code, h);
   else
     {
        if (method->no_reply)
          source_client_simple_method_call_no_reply_generate(method, c_code, h);
        else
          source_client_simple_method_call_generate(method, c_code, h);
     }
}

static void
source_client_signal_generate(const DBus_Signal *sig, Eina_Strbuf *c_code, Eina_Strbuf * h, Eina_Strbuf *c_init_function, Eina_Strbuf *c_header)
{
   DBus_Arg *arg;
   Eina_Strbuf *full_signature = eina_strbuf_new();
   Eina_Strbuf *parameters = eina_strbuf_new();
   Eina_Strbuf *string_copy = eina_strbuf_new();
   Eina_Strbuf *string_free = eina_strbuf_new();

   eina_strbuf_append_printf(c_init_function, "   eldbus_proxy_signal_handler_add(proxy, \"%s\", %s, proxy);\n", sig->name, sig->cb_name);
   eina_strbuf_append_printf(c_header, "int %s = 0;\n", sig->signal_event);
   eina_strbuf_append_printf(h, "extern int %s;\n", sig->signal_event);
   eina_strbuf_append_printf(c_init_function, "   if (!%s)\n", sig->signal_event);
   eina_strbuf_append_printf(c_init_function, "     %s = ecore_event_type_new();\n", sig->signal_event);

   eina_strbuf_append_printf(h, "typedef struct _%s\n", sig->struct_name);
   eina_strbuf_append_printf(h, "{\n");
   eina_strbuf_append_printf(h, "   Eldbus_Proxy *proxy;\n");

   if (sig->complex)
     {
        eina_strbuf_append_printf(h, "   Eina_Value *value;\n");
        goto jump_simple_stuff;
     }

   EINA_INLIST_FOREACH(sig->args, arg)
     {
        eina_strbuf_append(full_signature, arg->type);
        eina_strbuf_append_printf(parameters, ", &s_data->%s", arg->c_name);
        eina_strbuf_append_printf(h, "   %s%s;\n", dbus_type2c_type2(arg->type, EINA_FALSE), arg->c_name);

        if (!strcmp(arg->type, "s") || !strcmp(arg->type, "o"))
          {
             eina_strbuf_append_printf(string_copy, "   s_data->%s = strdup(s_data->%s);\n", arg->c_name, arg->c_name);
             eina_strbuf_append_printf(string_free, "   free(s_data->%s);\n", arg->c_name);
          }
     }

jump_simple_stuff:
   eina_strbuf_append_printf(h, "} %s;\n", sig->struct_name);

   //free function
   eina_strbuf_append_printf(c_code, "\nstatic void\n%s(void *user_data EINA_UNUSED, void *func_data)\n{\n", sig->free_function);
   eina_strbuf_append_printf(c_code, "   %s *s_data = func_data;\n", sig->struct_name);
   if (sig->complex)
     eina_strbuf_append(c_code, "   eina_value_free(s_data->value);\n");
   else
     eina_strbuf_append(c_code, eina_strbuf_string_get(string_free));
   eina_strbuf_append_printf(c_code, "   free(s_data);\n");
   eina_strbuf_append_printf(c_code, "}\n");

   //cb function
   eina_strbuf_append_printf(c_code, "\nstatic void\n%s(void *data, const Eldbus_Message *msg)\n{\n", sig->cb_name);
   eina_strbuf_append_printf(c_code, "   Eldbus_Proxy *proxy = data;\n");
   eina_strbuf_append_printf(c_code, "   %s *s_data = calloc(1, sizeof(%s));\n", sig->struct_name, sig->struct_name);
   eina_strbuf_append_printf(c_code, "   s_data->proxy = proxy;\n");
   if (sig->complex)
     {
        eina_strbuf_append_printf(c_code, "   s_data->value = eldbus_message_to_eina_value(msg);\n");
        goto end_signal;
     }
   eina_strbuf_append_printf(c_code, "   if (!eldbus_message_arguments_get(msg, \"%s\"%s))\n", eina_strbuf_string_get(full_signature), eina_strbuf_string_get(parameters));
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        ERR(\"Error: Getting arguments from message.\");\n");
   eina_strbuf_append_printf(c_code, "        free(s_data);\n");
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append(c_code, eina_strbuf_string_get(string_copy));

end_signal:
   eina_strbuf_append_printf(c_code, "   ecore_event_add(%s, s_data, %s, NULL);\n", sig->signal_event, sig->free_function);
   eina_strbuf_append_printf(c_code, "}\n");

   eina_strbuf_free(full_signature);
   eina_strbuf_free(parameters);
   eina_strbuf_free(string_copy);
   eina_strbuf_free(string_free);
}

static const char *
prop_cb_get(const DBus_Property *prop)
{
   if (prop->complex)
     return "Eldbus_Codegen_Property_Complex_Get_Cb";
   switch (prop->type[0])
     {
      case 's':
      case 'o':
        return "Eldbus_Codegen_Property_String_Get_Cb";
      case 'i':
      case 'h':
        return "Eldbus_Codegen_Property_Int32_Get_Cb";
      case 'y':
         return "Eldbus_Codegen_Property_Byte_Get_Cb";
      case 'b':
         return "Eldbus_Codegen_Property_Bool_Get_Cb";
      case 'n':
         return "Eldbus_Codegen_Property_Int16_Get_Cb";
      case 'q':
         return "Eldbus_Codegen_Property_Uint16_Get_Cb";
      case 'u':
         return "Eldbus_Codegen_Property_Uint32_Get_Cb";
      case 'd':
         return "Eldbus_Codegen_Property_Double_Get_Cb";
      case 'x':
         return "Eldbus_Codegen_Property_Int64_Get_Cb";
      case 't':
         return "Eldbus_Codegen_Property_Uint64_Get_Cb";
      default:
         return "Unexpected_type";
     }
}

static void
source_client_property_generate_get(const DBus_Property *prop, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   //callback
   eina_strbuf_append_printf(c_code, "\nstatic void\n%s(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)\n{\n", prop->cb_name);
   eina_strbuf_append_printf(c_code, "   void *user_data = eldbus_pending_data_del(pending, \"__user_data\");\n");
   eina_strbuf_append_printf(c_code, "   const char *error, *error_msg;\n");
   eina_strbuf_append_printf(c_code, "   %s cb = data;\n", prop_cb_get(prop));
   eina_strbuf_append_printf(c_code, "   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, \"__proxy\");\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Message_Iter *variant;\n");
   if (prop->complex)
     eina_strbuf_append_printf(c_code, "   Eina_Value *v, stack_value;\n");
   else
     eina_strbuf_append_printf(c_code, "   %sv;\n", dbus_type2c_type(prop->type));
   eina_strbuf_append_printf(c_code, "   if (eldbus_message_error_get(msg, &error, &error_msg))\n");
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {error, error_msg};\n");
   eina_strbuf_append_printf(c_code, "        cb(user_data, pending, \"%s\", proxy, &error_info, %s);\n", prop->name, null_or_zero(prop->type));
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");

   eina_strbuf_append_printf(c_code, "   if (!eldbus_message_arguments_get(msg, \"v\", &variant))\n");
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {\"\", \"\"};\n");
   eina_strbuf_append_printf(c_code, "        cb(user_data, pending, \"%s\", proxy, &error_info, %s);\n", prop->name, null_or_zero(prop->type));
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");

   if (prop->complex)
     {
        eina_strbuf_append_printf(c_code, "   v = eldbus_message_iter_struct_like_to_eina_value(variant);\n");
        eina_strbuf_append_printf(c_code, "   eina_value_struct_value_get(v, \"arg0\", &stack_value);\n");
        eina_strbuf_append_printf(c_code, "   cb(user_data, pending, \"%s\", proxy, NULL, &stack_value);\n", prop->name);
        eina_strbuf_append_printf(c_code, "   eina_value_flush(&stack_value);\n");
        eina_strbuf_append_printf(c_code, "   eina_value_free(v);\n");
     }
   else
     {
        eina_strbuf_append_printf(c_code, "   if (!eldbus_message_iter_arguments_get(variant, \"%s\", &v))\n", prop->type);
        eina_strbuf_append_printf(c_code, "     {\n");
        eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {\"\", \"\"};\n");
        eina_strbuf_append_printf(c_code, "        cb(user_data, pending, \"%s\", proxy, &error_info, %s);\n", prop->name, null_or_zero(prop->type));
        eina_strbuf_append_printf(c_code, "        return;\n");
        eina_strbuf_append_printf(c_code, "     }\n");
        eina_strbuf_append_printf(c_code, "   cb(user_data, pending, \"%s\", proxy, NULL, v);\n", prop->name);
     }
   eina_strbuf_append_printf(c_code, "}\n");

   //call
   eina_strbuf_append_printf(h, "Eldbus_Pending *%s_propget(Eldbus_Proxy *proxy, %s cb, const void *data);\n", prefix_append(prop->c_name), prop_cb_get(prop));

   eina_strbuf_append_printf(c_code, "\nEldbus_Pending *\n%s_propget(Eldbus_Proxy *proxy, %s cb, const void *data)\n{\n", prefix_append(prop->c_name), prop_cb_get(prop));
   eina_strbuf_append_printf(c_code, "   Eldbus_Pending *p;\n");
   eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);\n");
   eina_strbuf_append_printf(c_code, "   p = eldbus_proxy_property_get(proxy, \"%s\", %s, cb);\n", prop->name, prop->cb_name);
   eina_strbuf_append_printf(c_code, "   if (data)\n");
   eina_strbuf_append_printf(c_code, "     eldbus_pending_data_set(p, \"__user_data\", data);\n");
   eina_strbuf_append_printf(c_code, "   eldbus_pending_data_set(p, \"__proxy\", proxy);\n");
   eina_strbuf_append_printf(c_code, "   return p;\n");
   eina_strbuf_append_printf(c_code, "}\n");
}

static void
source_client_property_generate_set(const DBus_Property *prop, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   //callback
   eina_strbuf_append_printf(c_code, "\nstatic void\n%s_set(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)\n{\n", prop->cb_name);
   eina_strbuf_append_printf(c_code, "   const char *error, *error_msg;\n");
   eina_strbuf_append_printf(c_code, "   void *user_data = eldbus_pending_data_del(pending, \"__user_data\");\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, \"__proxy\");\n");
   eina_strbuf_append_printf(c_code, "   Eldbus_Codegen_Property_Set_Cb cb = data;\n");
   eina_strbuf_append_printf(c_code, "   if (eldbus_message_error_get(msg, &error, &error_msg))");
   eina_strbuf_append_printf(c_code, "     {\n");
   eina_strbuf_append_printf(c_code, "        Eldbus_Error_Info error_info = {error, error_msg};\n\n");
   eina_strbuf_append_printf(c_code, "        cb(user_data, \"%s\", proxy, pending, &error_info);\n", prop->name);
   eina_strbuf_append_printf(c_code, "        return;\n");
   eina_strbuf_append_printf(c_code, "     }\n");
   eina_strbuf_append_printf(c_code, "   cb(user_data, \"%s\", proxy, pending, NULL);\n", prop->name);
   eina_strbuf_append_printf(c_code, "}\n");

    //call
   eina_strbuf_append_printf(h, "Eldbus_Pending *%s_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value);\n", prefix_append(prop->c_name));

   eina_strbuf_append_printf(c_code, "\nEldbus_Pending *\n%s_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value)\n{\n", prop->c_name);
   eina_strbuf_append_printf(c_code, "   Eldbus_Pending *p;\n");
   eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);\n");
   eina_strbuf_append_printf(c_code, "   EINA_SAFETY_ON_NULL_RETURN_VAL(value, NULL);\n");
   eina_strbuf_append_printf(c_code, "   p = eldbus_proxy_property_set(proxy, \"%s\", \"%s\", value, %s_set, data);\n", prop->name, prop->type, prop->cb_name);
   eina_strbuf_append_printf(c_code, "   eldbus_pending_data_set(p, \"__user_data\", data);\n");
   eina_strbuf_append_printf(c_code, "   eldbus_pending_data_set(p, \"__proxy\", proxy);\n");
   eina_strbuf_append_printf(c_code, "   return p;\n");
   eina_strbuf_append_printf(c_code, "}\n");
}

static void
source_client_property_generate(const DBus_Property *prop, Eina_Strbuf *c_code, Eina_Strbuf *h)
{
   if ((prop->access & ACCESS_READ) == ACCESS_READ)
     source_client_property_generate_get(prop, c_code, h);
   if ((prop->access & ACCESS_WRITE) == ACCESS_WRITE)
     source_client_property_generate_set(prop, c_code, h);
}

void
source_client_generate(DBus_Object *path, const char *prefix, const char *interface_name, const char *output_name)
{
   DBus_Interface *iface;
   Eina_Bool found = EINA_FALSE;
   code_prefix = prefix;
   EINA_INLIST_FOREACH(path->ifaces, iface)
     {
        Eina_Strbuf *h, *c_init_function, *c_header, *c_code;
        DBus_Method *method;
        DBus_Signal *sig;
        DBus_Property *prop;
        char *file_name, *aux;
        int i;

        if (interface_name && strcmp(interface_name, iface->name))
          continue;
        found = EINA_TRUE;
        h = eina_strbuf_new();//.h file
        c_init_function = eina_strbuf_new();
        c_header = eina_strbuf_new();
        c_code = eina_strbuf_new();

        aux = string_build("ELDBUS_%s_H", iface->c_name);
        for (i = 0; aux[i]; i++)
          aux[i] = toupper(aux[i]);
        eina_strbuf_append_printf(h, "#ifndef %s\n", aux);
        eina_strbuf_append_printf(h, "#define %s\n\n", aux);
        free(aux);

        eina_strbuf_append_printf(h, "#include <Eina.h>\n");
        eina_strbuf_append_printf(h, "#include <Ecore.h>\n");
        eina_strbuf_append_printf(h, "#include <Eldbus.h>\n");
        eina_strbuf_append_printf(h, "#include \"eldbus_utils.h\"\n\n");
        eina_strbuf_append_printf(h, "Eldbus_Proxy *%s_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path);\n", prefix_append(iface->c_name));
        eina_strbuf_append_printf(h, "void %s_proxy_unref(Eldbus_Proxy *proxy);\n", prefix_append(iface->c_name));
        eina_strbuf_append_printf(h, "void %s_log_domain_set(int id);\n", prefix_append(iface->c_name));

        if (interface_name && output_name)
          eina_strbuf_append_printf(c_header, "#include \"%s.h\"\n\n", output_name);
        else
          eina_strbuf_append_printf(c_header, "#include \"eldbus_%s.h\"\n\n", iface->c_name);

        eina_strbuf_append_printf(c_header, "static int _log_main = -1;\n");
        eina_strbuf_append_printf(c_header, "#undef ERR\n");
        eina_strbuf_append_printf(c_header, "#define ERR(...) EINA_LOG_DOM_ERR(_log_main, __VA_ARGS__);\n");

        eina_strbuf_append_printf(c_init_function, "void\n%s_log_domain_set(int id)\n{\n", prefix_append(iface->c_name));
        eina_strbuf_append_printf(c_init_function, "   _log_main = id;\n");
        eina_strbuf_append_printf(c_init_function, "}\n");

        eina_strbuf_append_printf(c_init_function, "\nvoid\n%s_proxy_unref(Eldbus_Proxy *proxy)\n{\n", prefix_append(iface->c_name));
        eina_strbuf_append_printf(c_init_function, "   Eldbus_Object *obj = eldbus_proxy_object_get(proxy);\n");
        eina_strbuf_append_printf(c_init_function, "   eldbus_proxy_unref(proxy);\n");
        eina_strbuf_append_printf(c_init_function, "   eldbus_object_unref(obj);\n");
        eina_strbuf_append_printf(c_init_function, "}\n");

        eina_strbuf_append_printf(c_init_function, "\nEldbus_Proxy *\n%s_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path)\n{\n", prefix_append(iface->c_name));
        eina_strbuf_append_printf(c_init_function, "   Eldbus_Object *obj;\n");
        eina_strbuf_append_printf(c_init_function, "   Eldbus_Proxy *proxy;\n");
        eina_strbuf_append_printf(c_init_function, "   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);\n");
        eina_strbuf_append_printf(c_init_function, "   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, NULL);\n");
        eina_strbuf_append_printf(c_init_function, "   if (!path) path = \"%s\";\n", path->name);
        eina_strbuf_append_printf(c_init_function, "   obj = eldbus_object_get(conn, bus, path);\n");
        eina_strbuf_append_printf(c_init_function, "   proxy = eldbus_proxy_get(obj, \"%s\");\n", iface->name);

        EINA_INLIST_FOREACH(iface->methods, method)
          source_client_method_generate(method, c_code, h);

        EINA_INLIST_FOREACH(iface->signals, sig)
          source_client_signal_generate(sig, c_code, h, c_init_function, c_header);

        EINA_INLIST_FOREACH(iface->properties, prop)
          source_client_property_generate(prop, c_code, h);

        eina_strbuf_append_printf(c_init_function, "   return proxy;\n");
        eina_strbuf_append_printf(c_init_function, "}\n");

        eina_strbuf_append(h, "\n#endif");

        if (interface_name && output_name)
          file_name = string_build("%s.h", output_name);
        else
          file_name = string_build("eldbus_%s.h", iface->c_name);
        file_write(file_name, eina_strbuf_string_get(h));
        eina_strbuf_free(h);
        free(file_name);

        eina_strbuf_append(c_header, eina_strbuf_string_get(c_code));
        eina_strbuf_free(c_code);
        eina_strbuf_append(c_header, "\n");
        eina_strbuf_append(c_header, eina_strbuf_string_get(c_init_function));
        eina_strbuf_free(c_init_function);
        if (interface_name && output_name)
          file_name = string_build("%s.c", output_name);
        else
          file_name = string_build("eldbus_%s.c", iface->c_name);
        file_write(file_name, eina_strbuf_string_get(c_header));
        eina_strbuf_free(c_header);
        free(file_name);
     }

   if (interface_name && !found)
     printf("Error: Interface %s not found.\n", interface_name);
}
