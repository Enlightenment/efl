#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>
#include <stdint.h>

/* TODO: mempool of EDBus_Message and EDBus_Message_Iter */

#define EDBUS_MESSAGE_CHECK(msg)                        \
  do                                                    \
    {                                                   \
       EINA_SAFETY_ON_NULL_RETURN(msg);                 \
       if (!EINA_MAGIC_CHECK(msg, EDBUS_MESSAGE_MAGIC)) \
         {                                              \
            EINA_MAGIC_FAIL(msg, EDBUS_MESSAGE_MAGIC);  \
            return;                                     \
         }                                              \
       EINA_SAFETY_ON_TRUE_RETURN(msg->refcount <= 0);  \
    }                                                   \
  while (0)

#define EDBUS_MESSAGE_CHECK_RETVAL(msg, retval)                    \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN_VAL(msg, retval);                \
       if (!EINA_MAGIC_CHECK(msg, EDBUS_MESSAGE_MAGIC))            \
         {                                                         \
            EINA_MAGIC_FAIL(msg, EDBUS_MESSAGE_MAGIC);             \
            return retval;                                         \
         }                                                         \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(msg->refcount <= 0, retval); \
    }                                                              \
  while (0)

#define EDBUS_MESSAGE_ITERATOR_CHECK(iter)                        \
  do                                                             \
    {                                                            \
       EINA_SAFETY_ON_NULL_RETURN(iter);                          \
       if (!EINA_MAGIC_CHECK(iter, EDBUS_MESSAGE_ITERATOR_MAGIC)) \
         {                                                       \
            EINA_MAGIC_FAIL(iter, EDBUS_MESSAGE_ITERATOR_MAGIC);  \
            return;                                              \
         }                                                       \
    }                                                            \
  while (0)

#define EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, retval)          \
  do                                                              \
    {                                                             \
       EINA_SAFETY_ON_NULL_RETURN_VAL(iter, retval);               \
       if (!EINA_MAGIC_CHECK(iter, EDBUS_MESSAGE_ITERATOR_MAGIC))  \
         {                                                        \
            EINA_MAGIC_FAIL(iter, EDBUS_MESSAGE_ITERATOR_MAGIC);   \
            return retval;                                        \
         }                                                        \
    }                                                             \
  while (0)

static Eina_Bool append_basic(char type, va_list *vl, DBusMessageIter *iter);

Eina_Bool
edbus_message_init(void)
{
   return EINA_TRUE;
}

void
edbus_message_shutdown(void)
{
}

static EDBus_Message_Iter *
_message_iterator_new(Eina_Bool writable)
{
   EDBus_Message_Iter *iter;

   iter = calloc(1, sizeof(EDBus_Message_Iter));
   EINA_SAFETY_ON_NULL_RETURN_VAL(iter, NULL);
   EINA_MAGIC_SET(iter, EDBUS_MESSAGE_ITERATOR_MAGIC);
   iter->writable = writable;

   return iter;
}

EDBus_Message *edbus_message_new(Eina_Bool writable)
{
   EDBus_Message *msg = calloc(1, sizeof(EDBus_Message));
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);
   EINA_MAGIC_SET(msg, EDBUS_MESSAGE_MAGIC);
   msg->refcount = 1;

   msg->iterator = _message_iterator_new(writable);
   EINA_SAFETY_ON_NULL_GOTO(msg->iterator, fail);

   return msg;

fail:
   edbus_message_unref(msg);
   return NULL;
}

EAPI EDBus_Message *
edbus_message_method_call_new(const char *dest, const char *path, const char *iface, const char *method)
{
   EDBus_Message *msg;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method, NULL);

   msg = edbus_message_new(EINA_TRUE);
   EINA_SAFETY_ON_NULL_GOTO(msg, fail);

   msg->dbus_msg = dbus_message_new_method_call(dest, path, iface, method);
   dbus_message_iter_init_append(msg->dbus_msg, &msg->iterator->dbus_iterator);

   return msg;

fail:
   edbus_message_unref(msg);
   return NULL;
}

EAPI EDBus_Message *
edbus_message_ref(EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   DBG("msg=%p, pre-refcount=%d", msg, msg->refcount);
   msg->refcount++;
   return msg;
}

static void
_message_iterator_free(EDBus_Message_Iter *iter)
{
   Eina_Inlist *lst, *next;
   EDBus_Message_Iter *sub;
   if (!iter) return;

   lst = iter->iterators;
   while (lst)
     {
        next = lst->next;
        sub = EINA_INLIST_CONTAINER_GET(lst, EDBus_Message_Iter);
        _message_iterator_free(sub);
        lst = next;
     }
   free(iter);
}

EAPI void
edbus_message_unref(EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK(msg);
   DBG("msg=%p, pre-refcount=%d", msg, msg->refcount);
   msg->refcount--;
   if (msg->refcount > 0) return;

   DBG("message free %p", msg);

   EINA_MAGIC_SET(msg, EINA_MAGIC_NONE);
   if (msg->dbus_msg)
     dbus_message_unref(msg->dbus_msg);
   msg->dbus_msg = NULL;

   _message_iterator_free(msg->iterator);
   free(msg);
}

EAPI const char *
edbus_message_path_get(const EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_path(msg->dbus_msg);
}

EAPI const char *
edbus_message_interface_get(const EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_interface(msg->dbus_msg);
}

EAPI const char *
edbus_message_member_get(const EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_member(msg->dbus_msg);
}

EAPI const char *
edbus_message_destination_get(const EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_destination(msg->dbus_msg);
}

EAPI const char *
edbus_message_sender_get(const EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_sender(msg->dbus_msg);
}

EAPI const char *
edbus_message_signature_get(const EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_signature(msg->dbus_msg);
}

EAPI Eina_Bool
edbus_message_error_get(const EDBus_Message *msg, const char **name, const char **text)
{
   if (name) *name = NULL;
   if (text) *text = NULL;

   EDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);

   if (dbus_message_get_type(msg->dbus_msg) != DBUS_MESSAGE_TYPE_ERROR)
     return EINA_FALSE;

   if (name)
     *name = dbus_message_get_error_name(msg->dbus_msg);

   if (text)
     dbus_message_get_args(msg->dbus_msg, NULL, DBUS_TYPE_STRING, text,
                           DBUS_TYPE_INVALID);

   return EINA_TRUE;
}

static Eina_Bool
_edbus_message_arguments_vget(EDBus_Message *msg, const char *signature, va_list ap)
{
   EDBus_Message_Iter *iter;
   iter = edbus_message_iter_get(msg);
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);

   return edbus_message_iter_arguments_vget(iter, signature, ap);
}

EAPI EDBus_Message_Iter *
edbus_message_iter_get(EDBus_Message *msg)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   /* Something went wrong, msg->iterator should not be NULL */
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg->iterator, NULL);

   return msg->iterator;
}

EAPI Eina_Bool
edbus_message_arguments_get(const EDBus_Message *msg, const char *signature, ...)
{
   Eina_Bool ret;
   va_list ap;

   EDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_start(ap, signature);
   ret = _edbus_message_arguments_vget((EDBus_Message *)msg, signature, ap);
   va_end(ap);
   return ret;
}

EAPI Eina_Bool
edbus_message_arguments_vget(const EDBus_Message *msg, const char *signature, va_list ap)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);
   return _edbus_message_arguments_vget((EDBus_Message *)msg, signature, ap);
}

EAPI Eina_Bool
edbus_message_iter_arguments_vset(EDBus_Message_Iter *iter, const char *signature, va_list ap)
{
   DBusSignatureIter signature_iter;
   Eina_Bool r = EINA_TRUE;
   char *type;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);

   dbus_signature_iter_init(&signature_iter, signature);
   while ((type = dbus_signature_iter_get_signature(&signature_iter)) && r)
     {
        if (type[0] != DBUS_TYPE_VARIANT && !type[1])
          r = append_basic(type[0], MAKE_PTR_FROM_VA_LIST(ap),
                           &iter->dbus_iterator);
        else
          {
             EDBus_Message_Iter **user_itr;
             EDBus_Message_Iter *sub;

             user_itr = va_arg(ap, EDBus_Message_Iter **);
             sub = _message_iterator_new(EINA_TRUE);
             EINA_SAFETY_ON_NULL_GOTO(sub, error);
             iter->iterators = eina_inlist_append(iter->iterators,
                                                  EINA_INLIST_GET(sub));

             if (type[0] == DBUS_TYPE_ARRAY)
               r = dbus_message_iter_open_container(&iter->dbus_iterator,
                                                    type[0], type+1,
                                                    &sub->dbus_iterator);
             else if(type[1] == DBUS_TYPE_VARIANT)
               {
                  ERR("variant not supported by \
                      edbus_message_iter_arguments_set(), \
                      try edbus_message_iter_container_new()");
                  goto error;
               }
             else
               {
                  char real_type;

                  if (type[0] == DBUS_STRUCT_BEGIN_CHAR)
                    real_type = DBUS_TYPE_STRUCT;
                  else real_type = DBUS_TYPE_DICT_ENTRY;
                  r = dbus_message_iter_open_container(&iter->dbus_iterator,
                                                       real_type, NULL,
                                                       &sub->dbus_iterator);
               }
             *user_itr = sub;
          }

        dbus_free(type);
        if (!dbus_signature_iter_next(&signature_iter)) break;
        continue;
error:
       r = EINA_FALSE;
       dbus_free(type);
       break;
     }
   return r;
}

EAPI Eina_Bool
edbus_message_iter_arguments_set(EDBus_Message_Iter *iter, const char *signature, ...)
{
   Eina_Bool r;
   va_list ap;
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_start(ap, signature);
   r = edbus_message_iter_arguments_vset(iter, signature, ap);
   va_end(ap);
   return r;
}

static Eina_Bool
append_basic(char type, va_list *vl, DBusMessageIter *iter)
{
   switch (type)
     {
      case DBUS_TYPE_BYTE:
        {
           uint32_t read_byte = va_arg(*vl, uint32_t);
           uint8_t byte = read_byte;
           return dbus_message_iter_append_basic(iter, type, &byte);
        }
      case DBUS_TYPE_INT16:
        {
           int32_t read_int16 = va_arg(*vl, int32_t);
           int16_t int16 = read_int16;
           return dbus_message_iter_append_basic(iter, type, &int16);
        }
      case DBUS_TYPE_UINT16:
        {
           uint32_t read_uint16 = va_arg(*vl, uint32_t);
           uint16_t uint16 = read_uint16;
           return dbus_message_iter_append_basic(iter, type, &uint16);
        }
      case DBUS_TYPE_BOOLEAN:
      case DBUS_TYPE_INT32:
      case DBUS_TYPE_UNIX_FD:
        {
           int32_t int32 = va_arg(*vl, int32_t);
           return dbus_message_iter_append_basic(iter, type, &int32);
        }
      case DBUS_TYPE_UINT32:
        {
           uint32_t uint32 = va_arg(*vl, uint32_t);
           return dbus_message_iter_append_basic(iter, type, &uint32);
        }
      case DBUS_TYPE_INT64:
        {
           int64_t int64 = va_arg(*vl, int64_t);
           return dbus_message_iter_append_basic(iter, type, &int64);
        }
      case DBUS_TYPE_UINT64:
        {
           uint64_t uint64 = va_arg(*vl, uint64_t);
           return dbus_message_iter_append_basic(iter, type, &uint64);
        }
      case DBUS_TYPE_DOUBLE:
        {
           double double_ieee = va_arg(*vl, double);
           return dbus_message_iter_append_basic(iter, type, &double_ieee);
        }
      case DBUS_TYPE_STRING:
      case DBUS_TYPE_OBJECT_PATH:
      case DBUS_TYPE_SIGNATURE:
        {
           char *string = va_arg(*vl, char*);
           return dbus_message_iter_append_basic(iter, type, &string);
        }
      default:
         ERR("Type not handled %c", type);
         return EINA_FALSE;
     }
}

static Eina_Bool
_edbus_message_arguments_vset(EDBus_Message *msg, const char *signature, va_list ap)
{
   DBusSignatureIter signature_iter;
   EDBus_Message_Iter *iter;
   char *type;
   Eina_Bool r = EINA_TRUE;

   if (!signature || !signature[0]) return EINA_TRUE;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(dbus_signature_validate(signature, NULL), EINA_FALSE);

   iter = edbus_message_iter_get(msg);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);

   dbus_signature_iter_init(&signature_iter, signature);
   while ((type = dbus_signature_iter_get_signature(&signature_iter)) && r)
     {
        if (dbus_type_is_basic(type[0]))
          r = append_basic(type[0], MAKE_PTR_FROM_VA_LIST(ap),
                           &iter->dbus_iterator);
        else
          {
             ERR("edbus_message_arguments_set() and \
                  edbus_message_arguments_vset() only support basic types, \
                  to complex types use edbus_message_iter_* functions");
             r = EINA_FALSE;
          }

        dbus_free(type);
        if (!dbus_signature_iter_next(&signature_iter)) break;
     }

   return r;
}

EAPI Eina_Bool
edbus_message_arguments_set(EDBus_Message *msg, const char *signature, ...)
{
   Eina_Bool ret;
   va_list ap;

   EDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_start(ap, signature);
   ret = _edbus_message_arguments_vset(msg, signature, ap);
   va_end(ap);
   return ret;
}

EAPI Eina_Bool
edbus_message_arguments_vset(EDBus_Message *msg, const char *signature, va_list ap)
{
   EDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);
   return _edbus_message_arguments_vset(msg, signature, ap);
}

EAPI EDBus_Message_Iter *
edbus_message_iter_container_new(EDBus_Message_Iter *iter, int type, const char* contained_signature)
{
   EDBus_Message_Iter *sub;

   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, NULL);
   sub = _message_iterator_new(EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sub, NULL);

   if (!dbus_message_iter_open_container(&iter->dbus_iterator, type,
                                         contained_signature,
                                         &sub->dbus_iterator))
     goto cleanup;

   iter->iterators = eina_inlist_append(iter->iterators, EINA_INLIST_GET(sub));
   return sub;

cleanup:
   free(sub);
   return NULL;
}

EAPI Eina_Bool
edbus_message_iter_container_close(EDBus_Message_Iter *iter, EDBus_Message_Iter *sub)
{
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(sub, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);
   return dbus_message_iter_close_container(&iter->dbus_iterator,
                                            &sub->dbus_iterator);
}

EAPI Eina_Bool
edbus_message_iter_append_basic(EDBus_Message_Iter *iter, int type, ...)
{
   Eina_Bool r;
   va_list vl;
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);

   va_start(vl, type);
   r = append_basic(type, &vl, &iter->dbus_iterator);
   va_end(vl);

   return r;
}

EAPI void
edbus_message_iter_get_basic(EDBus_Message_Iter *iter, void *value)
{
   EDBUS_MESSAGE_ITERATOR_CHECK(iter);
   EINA_SAFETY_ON_TRUE_RETURN(iter->writable);
   dbus_message_iter_get_basic(&iter->dbus_iterator, value);
}

EAPI char*
edbus_message_iter_signature_get(EDBus_Message_Iter *iter)
{
   return dbus_message_iter_get_signature(&iter->dbus_iterator);
}

EAPI Eina_Bool
edbus_message_iter_next(EDBus_Message_Iter *iter)
{
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);
   return dbus_message_iter_next(&iter->dbus_iterator);
}

static void
get_basic(char type, DBusMessageIter *iter, va_list *vl)
{
   switch (type)
     {
      case DBUS_TYPE_BYTE:
        {
           uint8_t *byte = va_arg(*vl, uint8_t *);
           dbus_message_iter_get_basic(iter, byte);
           break;
        }
      case DBUS_TYPE_BOOLEAN:
        {
           Eina_Bool *boolean = va_arg(*vl, Eina_Bool *);
           dbus_bool_t val;
           dbus_message_iter_get_basic(iter, &val);
           *boolean = val;
           break;
        }
      case DBUS_TYPE_INT16:
        {
           int16_t *int16 = va_arg(*vl, int16_t *);
           dbus_message_iter_get_basic(iter, int16);
           break;
        }
      case DBUS_TYPE_UINT16:
        {
           uint16_t *uint16 = va_arg(*vl, uint16_t *);
           dbus_message_iter_get_basic(iter, uint16);
           break;
        }
      case DBUS_TYPE_INT32:
      case DBUS_TYPE_UNIX_FD:
        {
           int32_t *int32 = va_arg(*vl, int32_t *);
           dbus_message_iter_get_basic(iter, int32);
           break;
        }
      case DBUS_TYPE_UINT32:
        {
           uint32_t *uint32 = va_arg(*vl, uint32_t *);
           dbus_message_iter_get_basic(iter, uint32);
           break;
        }
      case DBUS_TYPE_INT64:
        {
           int64_t *int64 = va_arg(*vl, int64_t *);
           dbus_message_iter_get_basic(iter, int64);
           break;
        }
      case DBUS_TYPE_UINT64:
        {
           uint64_t *uint64 = va_arg(*vl, uint64_t *);
           dbus_message_iter_get_basic(iter, uint64);
           break;
        }
      case DBUS_TYPE_DOUBLE:
        {
           double *double_ieee = va_arg(*vl, double *);
           dbus_message_iter_get_basic(iter, double_ieee);
           break;
        }
      case DBUS_TYPE_STRING:
      case DBUS_TYPE_OBJECT_PATH:
      case DBUS_TYPE_SIGNATURE:
        {
           char **string = va_arg(*vl, char**);
           dbus_message_iter_get_basic(iter, string);
           break;
        }
      default:
        {
           ERR("Type not handled %c", type);
        }
     }
}

EAPI Eina_Bool
edbus_message_iter_fixed_array_get(EDBus_Message_Iter *iter, int signature, void *value, int *n_elements)
{
   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(
        (dbus_message_iter_get_element_type(&iter->dbus_iterator) == signature),
        EINA_FALSE);
   dbus_message_iter_get_fixed_array(&iter->dbus_iterator, value, n_elements);
   return EINA_TRUE;
}

/**
 * Useful when iterating over arrays
 */
EAPI Eina_Bool
edbus_message_iter_get_and_next(EDBus_Message_Iter *iter, char signature, ...)
{
   char type;
   va_list vl;

   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);
   va_start(vl, signature);

   type = dbus_message_iter_get_arg_type(&iter->dbus_iterator);
   if (type == DBUS_TYPE_INVALID) return EINA_FALSE;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(type == signature, EINA_FALSE);

   if (dbus_type_is_basic(type))
     get_basic(type, &iter->dbus_iterator, &vl);
   else
     {
        EDBus_Message_Iter *sub;
        EDBus_Message_Iter **iter_var = va_arg(vl, EDBus_Message_Iter**);

        sub = _message_iterator_new(EINA_FALSE);
        EINA_SAFETY_ON_NULL_RETURN_VAL(sub, EINA_FALSE);
        dbus_message_iter_recurse(&iter->dbus_iterator,
                                  &sub->dbus_iterator);
        iter->iterators = eina_inlist_append(iter->iterators,
                                             EINA_INLIST_GET(sub));

        *iter_var = sub;
     }
   va_end(vl);

   dbus_message_iter_next(&iter->dbus_iterator);
   return EINA_TRUE;
}

EAPI Eina_Bool
edbus_message_iter_arguments_get(EDBus_Message_Iter *iter, const char *signature, ...)
{
   va_list ap;
   Eina_Bool ret;

   va_start(ap, signature);
   ret = edbus_message_iter_arguments_vget(iter, signature, ap);
   va_end(ap);

   return ret;
}

EAPI Eina_Bool
edbus_message_iter_arguments_vget(EDBus_Message_Iter *iter, const char *signature, va_list ap)
{
   int current_type;
   DBusSignatureIter signature_iter;

   EDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(dbus_signature_validate(signature, NULL), EINA_FALSE);

   dbus_signature_iter_init(&signature_iter, signature);
   current_type = dbus_message_iter_get_arg_type(&iter->dbus_iterator);
   while (current_type != DBUS_TYPE_INVALID)
     {
        char *iter_sig = dbus_signature_iter_get_signature(&signature_iter);
        int c = iter_sig[0];

        dbus_free(iter_sig);
        dbus_signature_iter_next(&signature_iter);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(c == current_type, EINA_FALSE);

        if (dbus_type_is_basic(current_type))
          get_basic(current_type, &iter->dbus_iterator, MAKE_PTR_FROM_VA_LIST(ap));
        else
          {
             EDBus_Message_Iter **user_itr = va_arg(ap, EDBus_Message_Iter **);
             EDBus_Message_Iter *sub_itr;

             sub_itr = _message_iterator_new(EINA_FALSE);
             EINA_SAFETY_ON_NULL_RETURN_VAL(sub_itr, EINA_FALSE);
             dbus_message_iter_recurse(&iter->dbus_iterator,
                                       &sub_itr->dbus_iterator);
             iter->iterators = eina_inlist_append(iter->iterators,
                                                  EINA_INLIST_GET(sub_itr));
             *user_itr = sub_itr;
          }
        dbus_message_iter_next(&iter->dbus_iterator);
        current_type = dbus_message_iter_get_arg_type(&iter->dbus_iterator);
     }
   return EINA_TRUE;
}

EAPI void
edbus_message_iter_del(EDBus_Message_Iter *iter)
{
   EDBUS_MESSAGE_ITERATOR_CHECK(iter);
   _message_iterator_free(iter);
}

/* TODO: proper doc
 * Return the *reply* to @msg, i.e. @msg is the message we are replying to.
 */
EAPI EDBus_Message *
edbus_message_error_new(const EDBus_Message *msg, const char *error_name, const char *error_msg)
{
   EDBus_Message *reply;

   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_msg, NULL);

   reply = edbus_message_new(EINA_FALSE);
   reply->dbus_msg = dbus_message_new_error(msg->dbus_msg,
                                            error_name, error_msg);

   /*
    * Technically user should not append more arguments in an error message but
    * we can't leave its iter as NULL.
    */
   dbus_message_iter_init_append(reply->dbus_msg,
                                 &reply->iterator->dbus_iterator);

   return reply;
}

EAPI EDBus_Message *
edbus_message_method_return_new(const EDBus_Message *msg)
{
   EDBus_Message *reply;
   EDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);

   reply = edbus_message_new(EINA_TRUE);
   reply->dbus_msg = dbus_message_new_method_return(msg->dbus_msg);

   dbus_message_iter_init_append(reply->dbus_msg,
                                 &reply->iterator->dbus_iterator);

   return reply;
}

EAPI EDBus_Message *
edbus_message_signal_new(const char *path, const char *interface, const char *name)
{
   EDBus_Message *msg;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(interface, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(name, NULL);

   msg = edbus_message_new(EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   msg->dbus_msg = dbus_message_new_signal(path, interface, name);
   dbus_message_iter_init_append(msg->dbus_msg,
                                 &msg->iterator->dbus_iterator);

   return msg;
}
