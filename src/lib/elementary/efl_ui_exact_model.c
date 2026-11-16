#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

// This class store the size information in a compressed array and unpack it
// when necessary. It does maintain a cache of up to 3 uncompressed slot.
// That cache could get dropped when the application is entering the 'pause'
// state.

#define EFL_UI_EXACT_MODEL_CONTENT 1024
#define EFL_UI_EXACT_MODEL_CONTENT_LENGTH (EFL_UI_EXACT_MODEL_CONTENT * sizeof (unsigned int))

// A size that was set is stored with this bit on. A slot of zeros (what the
// storage starts as, and what a slot that was never written expands to) is
// therefore a row nobody measured, which is not a row of size 0.
#define EFL_UI_EXACT_MODEL_MEASURED 0x80000000u

// For now only vertical logic is implemented. Horizontal list and grid are not supported.

typedef struct _Efl_Ui_Exact_Model_Data Efl_Ui_Exact_Model_Data;
struct _Efl_Ui_Exact_Model_Data
{
   Efl_Ui_Exact_Model_Data *parent;

   struct {
      Eina_List *width;
      Eina_List *height;
   } compressed;

   struct {
      unsigned int width;
      unsigned int height;
   } total_size;

   struct {
      unsigned int *width;
      unsigned int *height;
      unsigned int start_offset;
      unsigned short usage;
      Eina_Bool defined : 1;
      struct {
         Eina_Bool width : 1;
         Eina_Bool height : 1;
      } decompressed;
   } slot[3];
};

static Efl_Object *
_efl_ui_exact_model_efl_object_constructor(Eo *obj, Efl_Ui_Exact_Model_Data *pd)
{
   Eo *parent = efl_parent_get(obj);

   if (parent && efl_isa(parent, EFL_UI_EXACT_MODEL_CLASS))
     pd->parent = efl_data_scope_get(efl_parent_get(obj), EFL_UI_EXACT_MODEL_CLASS);

   return efl_constructor(efl_super(obj, EFL_UI_EXACT_MODEL_CLASS));
}

// The compressed block of zeros that fills the gaps is shared by every model,
// and is let go of with the last list that holds it. The count of the lists
// holding it is not atomic and not locked: it is only ever touched from the
// main loop, as the models themselves are.
static Eina_Binbuf *_zero_block = NULL;
static unsigned int _zero_block_users = 0;

static void
_efl_ui_exact_model_zero_unref(void)
{
   if (--_zero_block_users) return;
   eina_binbuf_free(_zero_block);
   _zero_block = NULL;
}

static Eina_List *
_efl_ui_exact_model_slot_compress(unsigned int list_index, Eina_List *compressed, unsigned int *buffer)
{
   Eina_Binbuf *z;
   Eina_Binbuf *cbuf;
   Eina_Binbuf *tbuf;
   Eina_List *l = NULL;

   z = _zero_block;
   l = eina_list_nth_list(compressed, list_index);

   tbuf = eina_binbuf_manage_new((unsigned char *) buffer, EFL_UI_EXACT_MODEL_CONTENT_LENGTH, EINA_TRUE);
   if (!tbuf) return compressed;

   cbuf = emile_compress(tbuf, EMILE_LZ4, EMILE_COMPRESSOR_FAST);
   eina_binbuf_free(tbuf);
   if (!cbuf) return compressed;

   if (l)
     {
        // Replace older buffer by newer buffer
        tbuf = eina_list_data_get(l);
        eina_list_data_set(l, cbuf);
        if (tbuf != z) eina_binbuf_free(tbuf);
        else _efl_ui_exact_model_zero_unref();
        return compressed;
     }

   // Make sure the list has all the buffer up to the needed one filled with valid data
   if (eina_list_count(compressed) < list_index)
     {
        // Create the compressed zero buffer once.
        if (!z)
          {
             unsigned char *zmem;

             zmem = calloc(EFL_UI_EXACT_MODEL_CONTENT, sizeof (unsigned int));
             if (!zmem)
               {
                  eina_binbuf_free(cbuf);
                  return compressed;
               }

             tbuf = eina_binbuf_manage_new(zmem, EFL_UI_EXACT_MODEL_CONTENT_LENGTH, EINA_TRUE);
             if (!tbuf)
               {
                  eina_binbuf_free(cbuf);
                  free(zmem);
                  return compressed;
               }

             z = _zero_block = emile_compress(tbuf, EMILE_LZ4, EMILE_COMPRESSOR_FAST);

             eina_binbuf_free(tbuf);
             free(zmem);
             if (!z)
               {
                  eina_binbuf_free(cbuf);
                  return compressed;
               }
          }

        // Fill the list all the way to the needed index with buffer full of zero
        while (eina_list_count(compressed) < list_index)
          {
             compressed = eina_list_append(compressed, z);
             _zero_block_users++;
          }
     }

   return eina_list_append(compressed, cbuf);
}

static void
_efl_ui_exact_model_compressed_free(Eina_List *list)
{
   Eina_Binbuf *b;

   EINA_LIST_FREE(list, b)
     {
        if (b == _zero_block) _efl_ui_exact_model_zero_unref();
        else eina_binbuf_free(b);
     }
}

static void
_efl_ui_exact_model_efl_object_destructor(Eo *obj, Efl_Ui_Exact_Model_Data *pd)
{
   unsigned int i;

   // (only the root keeps sizes, its rows have none)
   _efl_ui_exact_model_compressed_free(pd->compressed.width);
   _efl_ui_exact_model_compressed_free(pd->compressed.height);
   pd->compressed.width = NULL;
   pd->compressed.height = NULL;
   for (i = 0; i < EINA_C_ARRAY_LENGTH(pd->slot); i++)
     {
        free(pd->slot[i].width);
        free(pd->slot[i].height);
        pd->slot[i].width = NULL;
        pd->slot[i].height = NULL;
     }

   efl_destructor(efl_super(obj, EFL_UI_EXACT_MODEL_CLASS));
}

static unsigned int *
_efl_ui_exact_model_buffer_expand(unsigned int list_index, unsigned int *buffer, Eina_List *list)
{
   Eina_Binbuf *tmp;
   Eina_List *l = NULL;

   if (!buffer) buffer = malloc(EFL_UI_EXACT_MODEL_CONTENT_LENGTH);

   l = eina_list_nth_list(list, list_index);

   // Check if the data is in the list
   if (!l)
     {
        // Not found -> everything is assumed to be zero
        memset(buffer, 0, EFL_UI_EXACT_MODEL_CONTENT_LENGTH);
        return buffer;
     }

   // Found -> expand in buffer
   tmp = eina_binbuf_manage_new((unsigned char*) buffer, EFL_UI_EXACT_MODEL_CONTENT_LENGTH, EINA_TRUE);
   emile_expand(eina_list_data_get(l), tmp, EMILE_LZ4);
   eina_binbuf_free(tmp);

   return buffer;
}

static unsigned char
_efl_ui_exact_model_slot_find(Efl_Ui_Exact_Model_Data *pd, unsigned int index,
                              Eina_Bool width_get, Eina_Bool height_get)
{
   unsigned char lookup;
   unsigned char found = EINA_C_ARRAY_LENGTH(pd->parent->slot);

   for (lookup = 0; lookup < EINA_C_ARRAY_LENGTH(pd->parent->slot); lookup++)
     {
        // Check if the slot has valid content
        if (!pd->parent->slot[lookup].defined)
          continue;
        // (start_offset is a block number, not a row index)
        if (pd->parent->slot[lookup].start_offset == index / EFL_UI_EXACT_MODEL_CONTENT)
          found = lookup;
        // Reduce usage to find unused slot.
        if (pd->parent->slot[lookup].usage > 0)
          pd->parent->slot[lookup].usage--;
     }

   // Do we need to find a new slot?
   if (found == EINA_C_ARRAY_LENGTH(pd->parent->slot))
     {
        found = 0;
        for (lookup = 0; lookup < EINA_C_ARRAY_LENGTH(pd->parent->slot); lookup++)
          {
             if (!pd->parent->slot[lookup].defined)
               {
                  // Found an empty slot, let's use that.
                  found = lookup;
                  break;
               }
             if (pd->parent->slot[lookup].usage < pd->parent->slot[found].usage)
               found = lookup;
          }

        // Commit change back to the stored buffer list
        if (pd->parent->slot[found].defined &&
            (pd->parent->slot[found].width ||
             pd->parent->slot[found].height))
          {
             unsigned int block = pd->parent->slot[found].start_offset;

             if (pd->parent->slot[found].width &&
                 pd->parent->slot[found].decompressed.width)
               pd->parent->compressed.width = _efl_ui_exact_model_slot_compress(block,
                                                                                pd->parent->compressed.width,
                                                                                pd->parent->slot[found].width);
             if (pd->parent->slot[found].height &&
                 pd->parent->slot[found].decompressed.height)
               pd->parent->compressed.height = _efl_ui_exact_model_slot_compress(block,
                                                                                 pd->parent->compressed.height,
                                                                                 pd->parent->slot[found].height);
          }

        pd->parent->slot[found].defined = EINA_TRUE;
        pd->parent->slot[found].decompressed.width = EINA_FALSE;
        pd->parent->slot[found].decompressed.height = EINA_FALSE;
        pd->parent->slot[found].start_offset = index / EFL_UI_EXACT_MODEL_CONTENT;
     }

   // Increase usage of the returned slot for now
   pd->parent->slot[found].usage++;

   // Unpack the data if requested
   if (width_get && !pd->parent->slot[found].decompressed.width)
     {
        pd->parent->slot[found].width = _efl_ui_exact_model_buffer_expand(pd->parent->slot[found].start_offset,
                                                                          pd->parent->slot[found].width,
                                                                          pd->parent->compressed.width);
        pd->parent->slot[found].decompressed.width = EINA_TRUE;
     }
   if (height_get && !pd->parent->slot[found].decompressed.height)
     {
        pd->parent->slot[found].height = _efl_ui_exact_model_buffer_expand(pd->parent->slot[found].start_offset,
                                                                          pd->parent->slot[found].height,
                                                                          pd->parent->compressed.height);
        pd->parent->slot[found].decompressed.height = EINA_TRUE;
     }

   return found;
}

static Eina_Future *
_efl_ui_exact_model_efl_model_property_set(Eo *obj, Efl_Ui_Exact_Model_Data *pd,
                                           const char *property, Eina_Value *value)
{
   if (pd->parent)
    {
       if (eina_streq(property, _efl_model_property_selfw))
         {
            unsigned int index, size;
            unsigned char found;

            index = efl_composite_model_index_get(obj);
            // (no index: the row left while a slice held it, and has no slot)
            if (index == 0xFFFFFFFF)
              return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
            found = _efl_ui_exact_model_slot_find(pd, index, EINA_TRUE, EINA_FALSE);
            if (!eina_value_uint_convert(value, &size) || (size & EFL_UI_EXACT_MODEL_MEASURED))
              return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
            pd->parent->slot[found].width[index % EFL_UI_EXACT_MODEL_CONTENT] =
              size | EFL_UI_EXACT_MODEL_MEASURED;
            // We succeeded so let's update the max total size width (As we only handle vertical list case at the moment)
            if (pd->parent->total_size.width < size)
              pd->parent->total_size.width = size;
            return efl_loop_future_resolved(obj, eina_value_uint_init(size));
         }
       if (eina_streq(property, _efl_model_property_selfh))
         {
            unsigned int old_value;
            unsigned int index, size;
            unsigned char found;
            unsigned int *cell;

            index = efl_composite_model_index_get(obj);
            if (index == 0xFFFFFFFF)
              return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
            found = _efl_ui_exact_model_slot_find(pd, index, EINA_FALSE, EINA_TRUE);
            cell = &pd->parent->slot[found].height[index % EFL_UI_EXACT_MODEL_CONTENT];
            old_value = *cell & ~EFL_UI_EXACT_MODEL_MEASURED;
            if (!eina_value_uint_convert(value, &size) || (size & EFL_UI_EXACT_MODEL_MEASURED))
              return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
            *cell = size | EFL_UI_EXACT_MODEL_MEASURED;
            // We succeeded so let's update the total size
            pd->parent->total_size.height += size - old_value;
            return efl_loop_future_resolved(obj, eina_value_uint_init(size));
         }
       // The following property are calculated by the model and so READ_ONLY
       if (eina_streq(property, _efl_model_property_totalh))
         {
            return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
         }
       if (eina_streq(property, _efl_model_property_totalw))
         {
            return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
         }
    }

   if (eina_streq(property, _efl_model_property_itemw))
     {
        // The exact model can not guess a general item size if asked
        // and should refuse to remember anything like that.
        return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
     }
   if (eina_streq(property, _efl_model_property_itemh))
     {
        // The exact model can not guess a general item size if asked
        // and should refuse to remember anything like that.
        return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
     }

   return efl_model_property_set(efl_super(obj, EFL_UI_EXACT_MODEL_CLASS), property, value);
}

static Eina_Value *
_efl_ui_exact_model_efl_model_property_get(const Eo *obj, Efl_Ui_Exact_Model_Data *pd,
                                           const char *property)
{
   if (pd->parent)
     {
        if (eina_streq(property, _efl_model_property_selfw))
          {
             unsigned int index, size;
             unsigned char found;

             index = efl_composite_model_index_get(obj);
             // (no index: the row left while a slice held it, and has no size)
             if (index == 0xFFFFFFFF) return eina_value_error_new(EAGAIN);
             found = _efl_ui_exact_model_slot_find(pd, index, EINA_TRUE, EINA_FALSE);
             size = pd->parent->slot[found].width[index % EFL_UI_EXACT_MODEL_CONTENT];
             // Not measured yet: not ready, as for a Homogeneous_Model, and not a size of 0.
             if (!(size & EFL_UI_EXACT_MODEL_MEASURED)) return eina_value_error_new(EAGAIN);
             return eina_value_uint_new(size & ~EFL_UI_EXACT_MODEL_MEASURED);
          }
        if (eina_streq(property, _efl_model_property_selfh))
          {
             unsigned int index, size;
             unsigned char found;

             index = efl_composite_model_index_get(obj);
             if (index == 0xFFFFFFFF) return eina_value_error_new(EAGAIN);
             found = _efl_ui_exact_model_slot_find(pd, index, EINA_FALSE, EINA_TRUE);
             size = pd->parent->slot[found].height[index % EFL_UI_EXACT_MODEL_CONTENT];
             if (!(size & EFL_UI_EXACT_MODEL_MEASURED)) return eina_value_error_new(EAGAIN);
             return eina_value_uint_new(size & ~EFL_UI_EXACT_MODEL_MEASURED);
          }
     }
   if (eina_streq(property, _efl_model_property_totalh))
     {
        return eina_value_uint_new(pd->total_size.height);
     }
   if (eina_streq(property, _efl_model_property_totalw))
     {
        return eina_value_uint_new(pd->total_size.width);
     }
   if (eina_streq(property, _efl_model_property_itemw))
     {
        // The exact model can not guess a general item size if asked.
        return eina_value_error_new(EAGAIN);
     }
   if (eina_streq(property, _efl_model_property_itemh))
     {
        // The exact model can not guess a general item size if asked.
        return eina_value_error_new(EAGAIN);
     }
   return efl_model_property_get(efl_super(obj, EFL_UI_EXACT_MODEL_CLASS), property);
}

#include "efl_ui_exact_model.eo.c"
