#include "eldbus_private.h"
#include "eldbus_private_types.h"
#include <dbus/dbus.h>
#include <stdint.h>

/* TODO: mempool of Eldbus_Message and Eldbus_Message_Iter */

#define ELDBUS_MESSAGE_CHECK(msg)                        \
  do                                                    \
    {                                                   \
       EINA_SAFETY_ON_NULL_RETURN(msg);                 \
       if (!EINA_MAGIC_CHECK(msg, ELDBUS_MESSAGE_MAGIC)) \
         {                                              \
            EINA_MAGIC_FAIL(msg, ELDBUS_MESSAGE_MAGIC);  \
            return;                                     \
         }                                              \
       EINA_SAFETY_ON_TRUE_RETURN(msg->refcount <= 0);  \
    }                                                   \
  while (0)

#define ELDBUS_MESSAGE_CHECK_RETVAL(msg, retval)                    \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN_VAL(msg, retval);                \
       if (!EINA_MAGIC_CHECK(msg, ELDBUS_MESSAGE_MAGIC))            \
         {                                                         \
            EINA_MAGIC_FAIL(msg, ELDBUS_MESSAGE_MAGIC);             \
            return retval;                                         \
         }                                                         \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(msg->refcount <= 0, retval); \
    }                                                              \
  while (0)

#define ELDBUS_MESSAGE_ITERATOR_CHECK(iter)                        \
  do                                                             \
    {                                                            \
       EINA_SAFETY_ON_NULL_RETURN(iter);                          \
       if (!EINA_MAGIC_CHECK(iter, ELDBUS_MESSAGE_ITERATOR_MAGIC)) \
         {                                                       \
            EINA_MAGIC_FAIL(iter, ELDBUS_MESSAGE_ITERATOR_MAGIC);  \
            return;                                              \
         }                                                       \
    }                                                            \
  while (0)

#define ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, retval)          \
  do                                                              \
    {                                                             \
       EINA_SAFETY_ON_NULL_RETURN_VAL(iter, retval);               \
       if (!EINA_MAGIC_CHECK(iter, ELDBUS_MESSAGE_ITERATOR_MAGIC))  \
         {                                                        \
            EINA_MAGIC_FAIL(iter, ELDBUS_MESSAGE_ITERATOR_MAGIC);   \
            return retval;                                        \
         }                                                        \
    }                                                             \
  while (0)

static Eina_Bool append_basic(char type, va_list *vl, DBusMessageIter *iter);

Eina_Bool
eldbus_message_init(void)
{
   return EINA_TRUE;
}

void
eldbus_message_shutdown(void)
{
}

static Eldbus_Message_Iter *
_message_iterator_new(Eina_Bool writable)
{
   Eldbus_Message_Iter *iter;

   iter = calloc(1, sizeof(Eldbus_Message_Iter));
   EINA_SAFETY_ON_NULL_RETURN_VAL(iter, NULL);
   EINA_MAGIC_SET(iter, ELDBUS_MESSAGE_ITERATOR_MAGIC);
   iter->writable = writable;

   return iter;
}

Eldbus_Message *eldbus_message_new(Eina_Bool writable)
{
   Eldbus_Message *msg = calloc(1, sizeof(Eldbus_Message));
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);
   EINA_MAGIC_SET(msg, ELDBUS_MESSAGE_MAGIC);
   msg->refcount = 1;

   msg->iterator = _message_iterator_new(writable);
   EINA_SAFETY_ON_NULL_GOTO(msg->iterator, fail);

   return msg;

fail:
   eldbus_message_unref(msg);
   return NULL;
}

EAPI Eldbus_Message *
eldbus_message_method_call_new(const char *dest, const char *path, const char *iface, const char *method)
{
   Eldbus_Message *msg;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(method, NULL);

   msg = eldbus_message_new(EINA_TRUE);
   EINA_SAFETY_ON_NULL_GOTO(msg, fail);

   msg->dbus_msg = dbus_message_new_method_call(dest, path, iface, method);
   dbus_message_iter_init_append(msg->dbus_msg, &msg->iterator->dbus_iterator);

   return msg;

fail:
   eldbus_message_unref(msg);
   return NULL;
}

EAPI Eldbus_Message *
eldbus_message_ref(Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   DBG("msg=%p, pre-refcount=%d", msg, msg->refcount);
   msg->refcount++;
   return msg;
}

static void
_message_iterator_free(Eldbus_Message_Iter *iter)
{
   Eina_Inlist *lst, *next;
   Eldbus_Message_Iter *sub;
   if (!iter) return;

   lst = iter->iterators;
   while (lst)
     {
        next = lst->next;
        sub = EINA_INLIST_CONTAINER_GET(lst, Eldbus_Message_Iter);
        _message_iterator_free(sub);
        lst = next;
     }
   free(iter);
}

EAPI void
eldbus_message_unref(Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK(msg);
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
eldbus_message_path_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_path(msg->dbus_msg);
}

EAPI const char *
eldbus_message_interface_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_interface(msg->dbus_msg);
}

EAPI const char *
eldbus_message_member_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_member(msg->dbus_msg);
}

EAPI const char *
eldbus_message_destination_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_destination(msg->dbus_msg);
}

EAPI const char *
eldbus_message_sender_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_sender(msg->dbus_msg);
}

EAPI const char *
eldbus_message_signature_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   return dbus_message_get_signature(msg->dbus_msg);
}

EAPI Eina_Bool
eldbus_message_error_get(const Eldbus_Message *msg, const char **name, const char **text)
{
   if (name) *name = NULL;
   if (text) *text = NULL;

   ELDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);

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
_eldbus_message_arguments_vget(Eldbus_Message *msg, const char *signature, va_list ap)
{
   Eldbus_Message_Iter *iter;
   iter = eldbus_message_iter_get(msg);
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);

   return eldbus_message_iter_arguments_vget(iter, signature, ap);
}

EAPI Eldbus_Message_Iter *
eldbus_message_iter_get(const Eldbus_Message *msg)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   /* Something went wrong, msg->iterator should not be NULL */
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg->iterator, NULL);

   return msg->iterator;
}

EAPI Eina_Bool
eldbus_message_arguments_get(const Eldbus_Message *msg, const char *signature, ...)
{
   Eina_Bool ret;
   va_list ap;

   ELDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_start(ap, signature);
   ret = _eldbus_message_arguments_vget((Eldbus_Message *)msg, signature, ap);
   va_end(ap);
   return ret;
}

EAPI Eina_Bool
eldbus_message_arguments_vget(const Eldbus_Message *msg, const char *signature, va_list ap)
{
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);
   return _eldbus_message_arguments_vget((Eldbus_Message *)msg, signature, ap);
}

static Eina_Bool
_eldbus_message_iter_arguments_vappend(Eldbus_Message_Iter *iter, const char *signature, va_list *aq)
{
   DBusSignatureIter signature_iter;
   Eina_Bool r = EINA_TRUE;
   char *type;

   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   dbus_signature_iter_init(&signature_iter, signature);
   while ((type = dbus_signature_iter_get_signature(&signature_iter)))
     {
        if (type[0] != DBUS_TYPE_VARIANT && !type[1])
          r = append_basic(type[0], aq, &iter->dbus_iterator);
        else
          {
             Eldbus_Message_Iter **user_itr;
             Eldbus_Message_Iter *sub;

             user_itr = va_arg(*aq, Eldbus_Message_Iter **);
             sub = _message_iterator_new(EINA_TRUE);
             if (!sub)
               {
                  ERR("Could not create sub iterator");
                  r = EINA_FALSE;
                  goto next;
               }

             iter->iterators = eina_inlist_append(iter->iterators,
                                                  EINA_INLIST_GET(sub));

             if (type[0] == DBUS_TYPE_ARRAY)
               r = dbus_message_iter_open_container(&iter->dbus_iterator,
                                                    type[0], type+1,
                                                    &sub->dbus_iterator);
             else if (type[1] == DBUS_TYPE_VARIANT)
               {
                  ERR("variant not supported by \
                      eldbus_message_iter_arguments_append(), \
                      try eldbus_message_iter_container_new()");
                  r = EINA_FALSE;
                  goto next;
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

next:
        dbus_free(type);
        if (!r || !dbus_signature_iter_next(&signature_iter))
          break;
     }

   return r;

}

EAPI Eina_Bool
eldbus_message_iter_arguments_vappend(Eldbus_Message_Iter *iter, const char *signature, va_list ap)
{
   va_list aq;
   Eina_Bool ret;

   va_copy(aq, ap);
   ret = _eldbus_message_iter_arguments_vappend(iter, signature, &aq);
   va_end(aq);

   return ret;
}

EAPI Eina_Bool
eldbus_message_iter_arguments_append(Eldbus_Message_Iter *iter, const char *signature, ...)
{
   Eina_Bool r;
   va_list ap;
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_start(ap, signature);
   r = eldbus_message_iter_arguments_vappend(iter, signature, ap);
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
#ifdef DBUS_TYPE_UNIX_FD
      case DBUS_TYPE_UNIX_FD:
#endif
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
_eldbus_message_arguments_vappend(Eldbus_Message *msg, const char *signature, va_list *aq)
{
   DBusSignatureIter signature_iter;
   Eldbus_Message_Iter *iter;
   int type;
   Eina_Bool r = EINA_TRUE;

   if (!signature[0]) return EINA_TRUE;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(dbus_signature_validate(signature, NULL),
                                   EINA_FALSE);

   iter = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);

   dbus_signature_iter_init(&signature_iter, signature);
   while ((type = dbus_signature_iter_get_current_type(&signature_iter)))
     {
        if (dbus_type_is_basic(type))
          r = append_basic(type, aq, &iter->dbus_iterator);
        else
          {
             ERR("sig = %s | eldbus_message_arguments_append() and \
                  eldbus_message_arguments_vappend() only support basic types, \
                  to complex types use eldbus_message_iter_* functions",
                  signature);
             r = EINA_FALSE;
          }

        if (!r || !dbus_signature_iter_next(&signature_iter))
          break;
     }

   return r;
}

EAPI Eina_Bool
eldbus_message_arguments_append(Eldbus_Message *msg, const char *signature, ...)
{
   Eina_Bool ret;
   va_list ap;

   ELDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_start(ap, signature);
   ret = _eldbus_message_arguments_vappend(msg, signature, &ap);
   va_end(ap);
   return ret;
}

EAPI Eina_Bool
eldbus_message_arguments_vappend(Eldbus_Message *msg, const char *signature, va_list ap)
{
   va_list aq;
   Eina_Bool ret;

   ELDBUS_MESSAGE_CHECK_RETVAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);

   va_copy(aq, ap);
   ret = _eldbus_message_arguments_vappend(msg, signature, &aq);
   va_end(aq);

   return ret;
}

EAPI Eldbus_Message_Iter *
eldbus_message_iter_container_new(Eldbus_Message_Iter *iter, int type, const char* contained_signature)
{
   Eldbus_Message_Iter *sub;

   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, NULL);
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
eldbus_message_iter_container_close(Eldbus_Message_Iter *iter, Eldbus_Message_Iter *sub)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(sub, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);
   return dbus_message_iter_close_container(&iter->dbus_iterator,
                                            &sub->dbus_iterator);
}

EAPI Eina_Bool
eldbus_message_iter_basic_append(Eldbus_Message_Iter *iter, int type, ...)
{
   Eina_Bool r;
   va_list vl;
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);

   va_start(vl, type);
   r = append_basic(type, &vl, &iter->dbus_iterator);
   va_end(vl);

   return r;
}

EAPI Eina_Bool
eldbus_message_iter_fixed_array_append(Eldbus_Message_Iter *iter, int type, const void *array, unsigned int size)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(array, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(dbus_type_is_basic(type), EINA_FALSE);
   return dbus_message_iter_append_fixed_array(&iter->dbus_iterator, type, &array, (int)size);
}

EAPI void
eldbus_message_iter_basic_get(Eldbus_Message_Iter *iter, void *value)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK(iter);
   EINA_SAFETY_ON_TRUE_RETURN(iter->writable);
   dbus_message_iter_get_basic(&iter->dbus_iterator, value);
}

Eldbus_Message_Iter *
eldbus_message_iter_sub_iter_get(Eldbus_Message_Iter *iter)
{
   Eldbus_Message_Iter *sub;
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, NULL);

   sub = _message_iterator_new(EINA_FALSE);
   dbus_message_iter_recurse(&iter->dbus_iterator, &sub->dbus_iterator);
   iter->iterators = eina_inlist_append(iter->iterators,
                                        EINA_INLIST_GET(sub));
   return sub;
}

EAPI char *
eldbus_message_iter_signature_get(Eldbus_Message_Iter *iter)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, NULL);
   return dbus_message_iter_get_signature(&iter->dbus_iterator);
}

EAPI Eina_Bool
eldbus_message_iter_next(Eldbus_Message_Iter *iter)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
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
#ifdef DBUS_TYPE_UNIX_FD
      case DBUS_TYPE_UNIX_FD:
#endif
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
eldbus_message_iter_fixed_array_get(Eldbus_Message_Iter *iter, int signature, void *value, int *n_elements)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(
        (dbus_message_iter_get_arg_type(&iter->dbus_iterator) == signature),
        EINA_FALSE);
   dbus_message_iter_get_fixed_array(&iter->dbus_iterator, value, n_elements);
   return EINA_TRUE;
}

/**
 * Useful when iterating over arrays
 */
EAPI Eina_Bool
eldbus_message_iter_get_and_next(Eldbus_Message_Iter *iter, char signature, ...)
{
   char type;
   va_list vl;

   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);
   va_start(vl, signature);

   type = dbus_message_iter_get_arg_type(&iter->dbus_iterator);
   if (type == DBUS_TYPE_INVALID)
     {
        va_end(vl);
        return EINA_FALSE;
     }
   if (type != signature)
     {
        if (signature == '(') signature = 'r';
        else if (signature == '{') signature = 'e';
        if (type != signature)
          {
             va_end(vl);
             return EINA_FALSE;
          }
     }

   if (dbus_type_is_basic(type))
     get_basic(type, &iter->dbus_iterator, &vl);
   else
     {
        Eldbus_Message_Iter *sub;
        Eldbus_Message_Iter **iter_var = va_arg(vl, Eldbus_Message_Iter**);

        sub = _message_iterator_new(EINA_FALSE);
        if (!sub)
          {
             va_end(vl);
             return EINA_FALSE;
          }
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

static Eina_Bool
_eldbus_message_iter_arguments_vget(Eldbus_Message_Iter *iter, const char *signature, va_list *aq)
{
   int iter_type;
   DBusSignatureIter sig_iter;

   ELDBUS_MESSAGE_ITERATOR_CHECK_RETVAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(iter->writable, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(dbus_signature_validate(signature, NULL), EINA_FALSE);

   dbus_signature_iter_init(&sig_iter, signature);
   iter_type = dbus_message_iter_get_arg_type(&iter->dbus_iterator);

   while (iter_type != DBUS_TYPE_INVALID)
     {
        int sig_type = dbus_signature_iter_get_current_type(&sig_iter);

        if (sig_type != iter_type)
          {
             ERR("Type in iterator different of signature");
             return EINA_FALSE;
          }

        if (dbus_type_is_basic(iter_type))
          get_basic(iter_type, &iter->dbus_iterator, aq);
        else
          {
             Eldbus_Message_Iter **user_itr = va_arg(*aq, Eldbus_Message_Iter **);
             Eldbus_Message_Iter *sub_itr;

             sub_itr = _message_iterator_new(EINA_FALSE);
             EINA_SAFETY_ON_NULL_RETURN_VAL(sub_itr, EINA_FALSE);
             dbus_message_iter_recurse(&iter->dbus_iterator,
                                       &sub_itr->dbus_iterator);
             iter->iterators = eina_inlist_append(iter->iterators,
                                                  EINA_INLIST_GET(sub_itr));
             *user_itr = sub_itr;
          }

        dbus_message_iter_next(&iter->dbus_iterator);
        dbus_signature_iter_next(&sig_iter);
        iter_type = dbus_message_iter_get_arg_type(&iter->dbus_iterator);
     }

   return dbus_signature_iter_get_current_type(&sig_iter) == DBUS_TYPE_INVALID;

}

EAPI Eina_Bool
eldbus_message_iter_arguments_get(Eldbus_Message_Iter *iter, const char *signature, ...)
{
   va_list ap;
   Eina_Bool ret;

   va_start(ap, signature);
   ret = _eldbus_message_iter_arguments_vget(iter, signature, &ap);
   va_end(ap);

   return ret;
}

EAPI Eina_Bool
eldbus_message_iter_arguments_vget(Eldbus_Message_Iter *iter, const char *signature, va_list ap)
{
   va_list aq;
   Eina_Bool ret;

   va_copy(aq, ap);
   ret = _eldbus_message_iter_arguments_vget(iter, signature, &aq);
   va_end(aq);

   return ret;
}

EAPI void
eldbus_message_iter_del(Eldbus_Message_Iter *iter)
{
   ELDBUS_MESSAGE_ITERATOR_CHECK(iter);
   _message_iterator_free(iter);
}

EAPI Eldbus_Message *
eldbus_message_error_new(const Eldbus_Message *msg, const char *error_name, const char *error_msg)
{
   Eldbus_Message *reply;

   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_msg, NULL);

   reply = eldbus_message_new(EINA_FALSE);
   reply->dbus_msg = dbus_message_new_error(msg->dbus_msg,
                                            error_name, error_msg);

   dbus_message_iter_init(reply->dbus_msg, &reply->iterator->dbus_iterator);

   return reply;
}

EAPI Eldbus_Message *
eldbus_message_method_return_new(const Eldbus_Message *msg)
{
   Eldbus_Message *reply;
   ELDBUS_MESSAGE_CHECK_RETVAL(msg, NULL);

   reply = eldbus_message_new(EINA_TRUE);
   reply->dbus_msg = dbus_message_new_method_return(msg->dbus_msg);

   dbus_message_iter_init_append(reply->dbus_msg,
                                 &reply->iterator->dbus_iterator);

   return reply;
}

EAPI Eldbus_Message *
eldbus_message_signal_new(const char *path, const char *interface, const char *name)
{
   Eldbus_Message *msg;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(interface, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(name, NULL);

   msg = eldbus_message_new(EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   msg->dbus_msg = dbus_message_new_signal(path, interface, name);
   dbus_message_iter_init_append(msg->dbus_msg,
                                 &msg->iterator->dbus_iterator);

   return msg;
}
