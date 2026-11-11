#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <limits.h>
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
      // Written to since it was expanded: what the compressed list has is older
      struct {
         Eina_Bool width : 1;
         Eina_Bool height : 1;
      } dirty;
   } slot[3];
};

static void _efl_ui_exact_model_child_added(void *data, const Efl_Event *event);
static void _efl_ui_exact_model_child_removed(void *data, const Efl_Event *event);

// Before anything that is told of the change (the view, the position manager),
// the sizes have to be where the rows now are.
//
// The callbacks of an object are called in the order of their priority value,
// the lowest first, and a priority is a short: the lowest value there is runs
// before any other, including the ones of EFL_CALLBACK_PRIORITY_BEFORE that the
// composite model and others use.
#define EFL_UI_EXACT_MODEL_FIRST SHRT_MIN

EFL_CALLBACKS_ARRAY_DEFINE(exact_model_callbacks,
                           { EFL_MODEL_EVENT_CHILD_ADDED, _efl_ui_exact_model_child_added },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _efl_ui_exact_model_child_removed });

static Efl_Object *
_efl_ui_exact_model_efl_object_constructor(Eo *obj, Efl_Ui_Exact_Model_Data *pd)
{
   Eo *parent = efl_parent_get(obj);

   if (parent && efl_isa(parent, EFL_UI_EXACT_MODEL_CLASS))
     pd->parent = efl_data_scope_get(efl_parent_get(obj), EFL_UI_EXACT_MODEL_CLASS);

   obj = efl_constructor(efl_super(obj, EFL_UI_EXACT_MODEL_CLASS));
   // (only the root keeps sizes)
   if (obj && !pd->parent)
     efl_event_callback_array_priority_add(obj, exact_model_callbacks(),
                                           EFL_UI_EXACT_MODEL_FIRST, pd);
   return obj;
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

// Writes the slot back under the block it holds (not the one that evicts it),
// when it was written to, and leaves it holding nothing.
static void
_efl_ui_exact_model_slot_commit(Efl_Ui_Exact_Model_Data *root, unsigned char found)
{
   unsigned int block = root->slot[found].start_offset;

   if (!root->slot[found].defined) return;

   // (a slot that was only read has nothing to commit)
   if (root->slot[found].width &&
       root->slot[found].decompressed.width &&
       root->slot[found].dirty.width)
     root->compressed.width =
       _efl_ui_exact_model_slot_compress(block, root->compressed.width,
                                         root->slot[found].width);
   if (root->slot[found].height &&
       root->slot[found].decompressed.height &&
       root->slot[found].dirty.height)
     root->compressed.height =
       _efl_ui_exact_model_slot_compress(block, root->compressed.height,
                                         root->slot[found].height);
   root->slot[found].decompressed.width = EINA_FALSE;
   root->slot[found].decompressed.height = EINA_FALSE;
   root->slot[found].dirty.width = EINA_FALSE;
   root->slot[found].dirty.height = EINA_FALSE;
}

static unsigned char
_efl_ui_exact_model_slot_find(Efl_Ui_Exact_Model_Data *root, unsigned int index,
                              Eina_Bool width_get, Eina_Bool height_get)
{
   unsigned char lookup;
   unsigned char found = EINA_C_ARRAY_LENGTH(root->slot);

   for (lookup = 0; lookup < EINA_C_ARRAY_LENGTH(root->slot); lookup++)
     {
        // Check if the slot has valid content
        if (!root->slot[lookup].defined)
          continue;
        // (start_offset is a block number, not a row index)
        if (root->slot[lookup].start_offset == index / EFL_UI_EXACT_MODEL_CONTENT)
          found = lookup;
        // Reduce usage to find unused slot.
        if (root->slot[lookup].usage > 0)
          root->slot[lookup].usage--;
     }

   // Do we need to find a new slot?
   if (found == EINA_C_ARRAY_LENGTH(root->slot))
     {
        found = 0;
        for (lookup = 0; lookup < EINA_C_ARRAY_LENGTH(root->slot); lookup++)
          {
             if (!root->slot[lookup].defined)
               {
                  // Found an empty slot, let's use that.
                  found = lookup;
                  break;
               }
             if (root->slot[lookup].usage < root->slot[found].usage)
               found = lookup;
          }

        _efl_ui_exact_model_slot_commit(root, found);

        root->slot[found].defined = EINA_TRUE;
        root->slot[found].decompressed.width = EINA_FALSE;
        root->slot[found].decompressed.height = EINA_FALSE;
        root->slot[found].dirty.width = EINA_FALSE;
        root->slot[found].dirty.height = EINA_FALSE;
        root->slot[found].start_offset = index / EFL_UI_EXACT_MODEL_CONTENT;
     }

   // Increase usage of the returned slot for now
   root->slot[found].usage++;

   // Unpack the data if requested
   if (width_get && !root->slot[found].decompressed.width)
     {
        root->slot[found].width = _efl_ui_exact_model_buffer_expand(root->slot[found].start_offset,
                                                                    root->slot[found].width,
                                                                    root->compressed.width);
        root->slot[found].decompressed.width = EINA_TRUE;
     }
   if (height_get && !root->slot[found].decompressed.height)
     {
        root->slot[found].height = _efl_ui_exact_model_buffer_expand(root->slot[found].start_offset,
                                                                     root->slot[found].height,
                                                                     root->compressed.height);
        root->slot[found].decompressed.height = EINA_TRUE;
     }

   return found;
}

// Stores the content of a block in the list node that holds it, or at the end
// of the list when it is the one after the last.
static void
_efl_ui_exact_model_block_store(Eina_List **compressed, Eina_List *l,
                                unsigned int block, unsigned int *buffer)
{
   Eina_Binbuf *old;
   Eina_Binbuf *tbuf;
   Eina_Binbuf *cbuf;

   if (!l)
     {
        *compressed = _efl_ui_exact_model_slot_compress(block, *compressed, buffer);
        return;
     }

   tbuf = eina_binbuf_manage_new((unsigned char *) buffer,
                                 EFL_UI_EXACT_MODEL_CONTENT_LENGTH, EINA_TRUE);
   if (!tbuf) return;
   cbuf = emile_compress(tbuf, EMILE_LZ4, EMILE_COMPRESSOR_FAST);
   eina_binbuf_free(tbuf);
   if (!cbuf) return;

   old = eina_list_data_get(l);
   eina_list_data_set(l, cbuf);
   if (old != _zero_block)
     {
        eina_binbuf_free(old);
     }
   else
     {
        _efl_ui_exact_model_zero_unref();
     }
}

static void
_efl_ui_exact_model_block_load(Eina_List *l, unsigned int *buffer)
{
   Eina_Binbuf *tmp;

   if (!l)
     {
        memset(buffer, 0, EFL_UI_EXACT_MODEL_CONTENT_LENGTH);
        return;
     }
   tmp = eina_binbuf_manage_new((unsigned char *) buffer,
                                EFL_UI_EXACT_MODEL_CONTENT_LENGTH, EINA_TRUE);
   if (!tmp)
     {
        memset(buffer, 0, EFL_UI_EXACT_MODEL_CONTENT_LENGTH);
        return;
     }
   emile_expand(eina_list_data_get(l), tmp, EMILE_LZ4);
   eina_binbuf_free(tmp);
}

// A row is inserted at index: every size from there on is one row further, and
// the row that is now at index has none. Only the blocks from the one holding
// index on are touched, and the blocks of zeros that the shift does not reach
// are left alone.
static void
_efl_ui_exact_model_list_insert(Eina_List **compressed, unsigned int index)
{
   unsigned int buffer[EFL_UI_EXACT_MODEL_CONTENT];
   unsigned int block = index / EFL_UI_EXACT_MODEL_CONTENT;
   unsigned int start = index % EFL_UI_EXACT_MODEL_CONTENT;
   unsigned int carry = 0, out;
   Eina_List *l;

   l = eina_list_nth_list(*compressed, block);
   // Everything from here on is zero, and stays so
   if (!l) return;

   for (;; block++)
     {
        Eina_List *next = eina_list_next(l);

        if ((block > (index / EFL_UI_EXACT_MODEL_CONTENT)) && !carry &&
            (eina_list_data_get(l) == _zero_block))
          goto advance;

        _efl_ui_exact_model_block_load(l, buffer);
        out = buffer[EFL_UI_EXACT_MODEL_CONTENT - 1];
        memmove(buffer + start + 1, buffer + start,
                (EFL_UI_EXACT_MODEL_CONTENT - 1 - start) * sizeof (unsigned int));
        buffer[start] = carry;
        carry = out;
        _efl_ui_exact_model_block_store(compressed, l, block, buffer);

     advance:
        start = 0;
        if (!next)
          {
             // What fell out of the last block is a size too
             if (carry)
               {
                  memset(buffer, 0, sizeof (buffer));
                  buffer[0] = carry;
                  _efl_ui_exact_model_block_store(compressed, NULL, block + 1, buffer);
               }
             break;
          }
        l = next;
     }
}

// A row is removed at index: every size after it is one row closer. What the
// size of the row was is returned.
static unsigned int
_efl_ui_exact_model_list_remove(Eina_List *compressed, unsigned int index)
{
   unsigned int bufs[2][EFL_UI_EXACT_MODEL_CONTENT];
   unsigned int block = index / EFL_UI_EXACT_MODEL_CONTENT;
   unsigned int start = index % EFL_UI_EXACT_MODEL_CONTENT;
   unsigned int removed;
   unsigned int cur = 0;
   Eina_List *l;

   l = eina_list_nth_list(compressed, block);
   // Never written, so never measured
   if (!l) return 0;

   _efl_ui_exact_model_block_load(l, bufs[cur]);
   removed = bufs[cur][start];
   for (;; block++)
     {
        Eina_List *next = eina_list_next(l);
        Eina_Bool unchanged;

        _efl_ui_exact_model_block_load(next, bufs[!cur]);
        unchanged = (block > (index / EFL_UI_EXACT_MODEL_CONTENT)) &&
          (eina_list_data_get(l) == _zero_block) &&
          (!next || (eina_list_data_get(next) == _zero_block));
        if (!unchanged)
          {
             memmove(bufs[cur] + start, bufs[cur] + start + 1,
                     (EFL_UI_EXACT_MODEL_CONTENT - 1 - start) * sizeof (unsigned int));
             bufs[cur][EFL_UI_EXACT_MODEL_CONTENT - 1] = bufs[!cur][0];
             _efl_ui_exact_model_block_store(&compressed, l, block, bufs[cur]);
          }
        start = 0;
        if (!next) break;
        l = next;
        cur = !cur;
     }
   return removed;
}

// The sizes follow their rows when one comes or goes. The sizes are kept by
// index, and the rows after the one that came or went have another one now.
static void
_efl_ui_exact_model_shift(Efl_Ui_Exact_Model_Data *root, unsigned int index, Eina_Bool added)
{
   unsigned char i;

   // What is kept expanded in the slots has to be in the list to be moved
   for (i = 0; i < EINA_C_ARRAY_LENGTH(root->slot); i++)
     {
        _efl_ui_exact_model_slot_commit(root, i);
        root->slot[i].defined = EINA_FALSE;
        root->slot[i].usage = 0;
     }

   if (added)
     {
        _efl_ui_exact_model_list_insert(&root->compressed.width, index);
        _efl_ui_exact_model_list_insert(&root->compressed.height, index);
     }
   else
     {
        unsigned int h;

        _efl_ui_exact_model_list_remove(root->compressed.width, index);
        h = _efl_ui_exact_model_list_remove(root->compressed.height, index);
        if (h & EFL_UI_EXACT_MODEL_MEASURED)
          root->total_size.height -= h & ~EFL_UI_EXACT_MODEL_MEASURED;
     }
}

static void
_efl_ui_exact_model_child_added(void *data, const Efl_Event *event)
{
   const Efl_Model_Children_Event *ev = event->info;

   _efl_ui_exact_model_shift(data, ev->index, EINA_TRUE);
}

static void
_efl_ui_exact_model_child_removed(void *data, const Efl_Event *event)
{
   const Efl_Model_Children_Event *ev = event->info;

   _efl_ui_exact_model_shift(data, ev->index, EINA_FALSE);
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
            found = _efl_ui_exact_model_slot_find(pd->parent, index, EINA_TRUE, EINA_FALSE);
            if (!eina_value_uint_convert(value, &size) || (size & EFL_UI_EXACT_MODEL_MEASURED))
              return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
            pd->parent->slot[found].width[index % EFL_UI_EXACT_MODEL_CONTENT] =
              size | EFL_UI_EXACT_MODEL_MEASURED;
            pd->parent->slot[found].dirty.width = EINA_TRUE;
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
            found = _efl_ui_exact_model_slot_find(pd->parent, index, EINA_FALSE, EINA_TRUE);
            cell = &pd->parent->slot[found].height[index % EFL_UI_EXACT_MODEL_CONTENT];
            old_value = *cell & ~EFL_UI_EXACT_MODEL_MEASURED;
            if (!eina_value_uint_convert(value, &size) || (size & EFL_UI_EXACT_MODEL_MEASURED))
              return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
            *cell = size | EFL_UI_EXACT_MODEL_MEASURED;
            pd->parent->slot[found].dirty.height = EINA_TRUE;
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
             found = _efl_ui_exact_model_slot_find(pd->parent, index, EINA_TRUE, EINA_FALSE);
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
             found = _efl_ui_exact_model_slot_find(pd->parent, index, EINA_FALSE, EINA_TRUE);
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

/* The Exact_Model under the models the view wraps its own around, or NULL.

   Only wrappers that keep the index of a row are walked through: a Boolean_Model
   (the Select_Model) and a Size_Model (the Homogeneous_Model and the
   Average_Model). Any other model (a Filter_Model, say) may
   number a row differently than the Exact_Model under it does, and the
   sizes are kept under the index of the Exact_Model. */
Efl_Model *
_efl_ui_exact_model_find(Efl_Model *model)
{
   while (model && !efl_isa(model, EFL_UI_EXACT_MODEL_CLASS))
     {
        if (!efl_isa(model, EFL_BOOLEAN_MODEL_CLASS) &&
            !efl_isa(model, EFL_UI_SIZE_MODEL_CLASS))
          return NULL;
        model = efl_ui_view_model_get(model);
     }
   return model;
}

/* The size of a row as it was told to the model, without the row itself:
   the view asks for rows it has let go of and cannot wait for them. */
Eina_Bool
_efl_ui_exact_model_row_size_get(Efl_Model *exact, unsigned int index, Eina_Size2D *size)
{
   Efl_Ui_Exact_Model_Data *root = efl_data_scope_get(exact, EFL_UI_EXACT_MODEL_CLASS);
   unsigned int w, h;
   unsigned char found;

   if (!root || root->parent) return EINA_FALSE;

   found = _efl_ui_exact_model_slot_find(root, index, EINA_TRUE, EINA_TRUE);
   w = root->slot[found].width[index % EFL_UI_EXACT_MODEL_CONTENT];
   h = root->slot[found].height[index % EFL_UI_EXACT_MODEL_CONTENT];
   if (!(w & EFL_UI_EXACT_MODEL_MEASURED) || !(h & EFL_UI_EXACT_MODEL_MEASURED))
     return EINA_FALSE;
   size->w = w & ~EFL_UI_EXACT_MODEL_MEASURED;
   size->h = h & ~EFL_UI_EXACT_MODEL_MEASURED;
   return EINA_TRUE;
}

#include "efl_ui_exact_model.eo.c"
