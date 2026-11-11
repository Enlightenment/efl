#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

// This class rely on the parent class to do all the individual holding of value,
// it only compute the average size of an item and answer for that property alone.
//
// What an average is over (the heights that were told, and how many rows were
// told one) is kept here, and follows a row that is removed: the Exact_Model says
// what the row had, once it has moved its own sizes. It also says when it had
// to give all its sizes up, and then there is nothing left to average.
typedef struct _Efl_Ui_Average_Model_Data Efl_Ui_Average_Model_Data;
struct _Efl_Ui_Average_Model_Data
{
   Efl_Ui_Average_Model_Data *parent;
   Eo *self;

   struct {
      unsigned long long width;
      unsigned long long height;
      unsigned long long wseen;
      unsigned long long hseen;
   } total;

   // The times the Exact_Model gave up its sizes that this has seen
   unsigned int forgotten;

   // The average of the heights the last time it was asked for or changed, and
   // the number of times it changed (what a view that estimates with it keeps)
   unsigned long long average;
   unsigned int revision;
   // What a row was told that has not been added to the totals yet (it is
   // added when the loop gets to it), for the row that leaves before that.
   Eina_List *pending;

   // Whether any row was told when it was: a view gives the rows nobody measured
   // the size it has for them (not the average) until one was, so it changes
   // what they are even when the average is the 0 it was
   Eina_Bool known : 1;
};

typedef struct _Efl_Ui_Average_Model_Update Efl_Ui_Average_Model_Update;
struct _Efl_Ui_Average_Model_Update
{
   unsigned long long *total;
   unsigned long long *seen;
   Efl_Ui_Average_Model_Data *root;
   Eo *row;
   unsigned int previous;
   unsigned int now;
   // The row left, or the sizes were forgotten, before it was added: there is no row to add it to
   Eina_Bool dropped : 1;
   // (root is NULL once the root is gone: the row, which a ref keeps, outlives it)
};

// What an item is estimated to be, from what was told: not a number of its own,
// a change of it is what makes the estimates of the others change. That is a
// change of the average, and the first row that is told (or the last one that
// leaves), as the rows nobody measured are then what the average is and not
// what the view had for them.
static Eina_Bool
_efl_ui_average_model_revise(Efl_Ui_Average_Model_Data *root)
{
   unsigned long long average = root->total.hseen ? root->total.height / root->total.hseen : 0;
   Eina_Bool known = !!root->total.hseen;

   if ((average == root->average) && (known == root->known)) return EINA_FALSE;
   root->average = average;
   root->known = known;
   root->revision++;
   return EINA_TRUE;
}

// Totals of what the Exact_Model says is not kept any more are of no use
static void
_efl_ui_average_model_forgotten_check(const Eo *obj, Efl_Ui_Average_Model_Data *root)
{
   Efl_Ui_Average_Model_Update *update;
   unsigned int forgotten = _efl_ui_exact_model_forgotten_get(obj);
   Eina_List *l;

   if (root->forgotten == forgotten) return;
   root->forgotten = forgotten;
   EINA_LIST_FOREACH(root->pending, l, update)
     {
        update->dropped = EINA_TRUE;
     }
   memset(&root->total, 0, sizeof (root->total));
   _efl_ui_average_model_revise(root);
}

static Eina_Value
_efl_ui_average_model_update(Eo *obj EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Ui_Average_Model_Update *request = data;
   unsigned int now;

   // (a row that left is not in the model: what it was told is not averaged)
   if (request->dropped || !eina_value_uint_convert(&v, &now))
     return v;

   // (a row that shrinks takes from the total: the difference is signed)
   *(request->total) += (long long) now - (long long) request->previous;
   if (request->seen) *(request->seen) += 1;
   // What is estimated for the rows nobody measured is another one: nothing else says it
   if (_efl_ui_average_model_revise(request->root))
     efl_model_properties_changed(request->root->self, _efl_model_property_totalh);

   return v;
}

static void
_efl_ui_average_model_unref(Eo *loop EINA_UNUSED, void *data,
                            const Eina_Future *dead_future EINA_UNUSED)
{
   efl_unref(data);
}

static void
_efl_ui_average_model_clean(Eo *obj, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Average_Model_Update *update = data;

   // (a root that is gone has let go of its list, and says so with no root)
   if (update->root)
     {
        update->root->pending = eina_list_remove(update->root->pending, update);
     }
   free(update);

   // This is called from inside the freeing of the future, which the row
   // being the last ref of is destroyed under: the row cancels the futures it
   // is the object of, this one among them, and it is freed twice. The ref is
   // let go of from a job, where nothing of that future is left to be touched
   // (the job is freed, and so the ref is let go of, even if the loop goes first).
   efl_future_then(efl_loop_get(obj), efl_loop_job(efl_loop_get(obj)),
                   .data = obj, .free = _efl_ui_average_model_unref);
}

static Eina_Future *
_efl_ui_average_model_prepare(Eo *obj, Efl_Ui_Average_Model_Data *root,
                              const char *property, Eina_Value *value)
{
   unsigned long long *total = &root->total.height;
   unsigned long long *seen = &root->total.hseen;
   Efl_Ui_Average_Model_Update *update;
   Eina_Value *previous;
   Eina_Future *f;
   unsigned int now, forgotten;

   // What the Exact_Model refuses is not one that will be averaged
   if (!eina_value_uint_convert(value, &now) || (now & EFL_UI_EXACT_MODEL_MEASURED) ||
       (efl_composite_model_index_get(obj) == 0xFFFFFFFF))
     {
        return efl_model_property_set(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS),
                                      property, value);
     }

   update = calloc(1, sizeof (Efl_Ui_Average_Model_Update));
   if (!update) return efl_loop_future_rejected(obj, ENOMEM);

   forgotten = _efl_ui_exact_model_forgotten_get(efl_parent_get(obj));
   previous = efl_model_property_get(obj, property);
   if (eina_value_type_get(previous) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        // Check the case when that property hasn't been set before
        if (!eina_value_error_convert(previous, &err))
          goto on_error;
        if (err != EAGAIN) goto on_error;
        // (a row that was not measured is one more that was seen once it is)
        update->seen = seen;
     }
   else if (!eina_value_uint_convert(previous, &update->previous))
     goto on_error;
   eina_value_free(previous);

   update->total = total;
   update->now = now;
   update->root = root;
   update->row = obj;

   // As we are operating asynchronously and we want to make sure that the object
   // survive until the transaction is commited, we will ref the object here
   // and unref on clean. This is necessary so that a nested call of property_set
   // on a model returned by children_slice_get, the user doesn't have to keep a
   // reference around to do the same. It shouldn't create any problem as this
   // future would be cancelled automatically when the parent object get destroyed.
   efl_ref(obj);

   // We have to make the change after we fetch the old value, otherwise, well, no old value left
   f = efl_model_property_set(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS), property, value);
   // Making room for the size, to read it or to write it, can give up all of them
   // (a block that cannot be stored): what was told before is not kept, and this row is
   // the first that is, so that it is added to nothing.
   if (_efl_ui_exact_model_forgotten_get(efl_parent_get(obj)) != forgotten)
     {
        _efl_ui_average_model_forgotten_check(efl_parent_get(obj), root);
        update->previous = 0;
        update->seen = seen;
     }

   root->pending = eina_list_append(root->pending, update);
   return efl_future_then(obj, f,
                          .success = _efl_ui_average_model_update,
                          .free = _efl_ui_average_model_clean,
                          .data = update);
 on_error:
   eina_value_free(previous);
   free(update);
   return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
}

static void
_efl_ui_average_model_child_removed(void *data, const Efl_Event *event)
{
   Efl_Ui_Average_Model_Data *pd = data;
   const Efl_Model_Children_Event *ev = event->info;
   Efl_Ui_Average_Model_Update *update;
   Eina_List *l;
   long long pending = 0, height;
   unsigned int removed;
   Eina_Bool seen = EINA_TRUE;

   _efl_ui_average_model_forgotten_check(event->object, pd);
   // What the row was told and the loop did not get to yet is in what the
   // Exact_Model took out, but not yet in what is here
   EINA_LIST_FOREACH(pd->pending, l, update)
     {
        if ((update->row != ev->child) || update->dropped) continue;
        pending += (long long) update->now - (long long) update->previous;
        if (update->seen) seen = EINA_FALSE;
        update->dropped = EINA_TRUE;
     }
   if (!_efl_ui_exact_model_removed_height_get(event->object, ev, &removed)) return;
   height = (long long) removed - pending;
   // What the Exact_Model took out was added here, so there is enough to take out.
   // If there is not, the totals are wrong and are of no use any more.
   if ((height < 0) || (pd->total.height < (unsigned long long) height) ||
       (seen && !pd->total.hseen))
     {
        ERR("The average of the heights does not hold the row that was removed: "
            "starting it again.");
        memset(&pd->total, 0, sizeof (pd->total));
        _efl_ui_average_model_revise(pd);
        return;
     }
   pd->total.height -= height;
   if (seen) pd->total.hseen--;
   _efl_ui_average_model_revise(pd);
}

static Eina_Future *
_efl_ui_average_model_efl_model_property_set(Eo *obj, Efl_Ui_Average_Model_Data *pd, const char *property, Eina_Value *value)
{
   Eina_Future *f = NULL;

   if (!pd->parent) goto end;

   // In vertical list mode we do not need to compute the average width size
   /* if (eina_streq(property, _efl_model_property_selfw)) */
   /*   { */
   /*      f = _efl_ui_average_model_prepare(obj, &pd->parent->total.width, */
   /*                                        pd->wseen ? NULL : &pd->parent->total.wseen, */
   /*                                        property, value, EINA_TRUE); */
   /*      pd->wseen = EINA_TRUE; */
   /*   } */
   if (eina_streq(property, _efl_model_property_selfh))
     {
        _efl_ui_average_model_forgotten_check(efl_parent_get(obj), pd->parent);
        f = _efl_ui_average_model_prepare(obj, pd->parent, property, value);
     }

 end:
   if (!f)
     f = efl_model_property_set(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS), property, value);

   return f;
}

static inline Eina_Value *
_efl_ui_average_model_compute(const Eo *obj, Eina_Value *r, unsigned long long total, unsigned long long seen)
{
   unsigned int count;

   eina_value_free(r);

   // Protection against divide by zero
   if (!seen) return eina_value_uint_new(0);

   count = efl_model_children_count_get(obj);
   // We are doing the multiply first in an attempt to not reduce the precision to early on.
   return eina_value_uint_new((total * count) / seen);
}

static Eina_Value *
_efl_ui_average_model_efl_model_property_get(const Eo *obj, Efl_Ui_Average_Model_Data *pd, const char *property)
{
   const Eina_Value_Type *t;
   Eina_Value *r;

   r = efl_model_property_get(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS), property);
   if (!r) return r;

   // We are checking that the parent class was able to provide an answer to the request for property "Total.Width"
   // or "Total.Height" which means that we are an object that should compute its size. This avoid computing the
   // pointer to the parent object.
   t = eina_value_type_get(r);
   if (t == EINA_VALUE_TYPE_UINT)
     {
        if (eina_streq(property, _efl_model_property_totalh))
          {
             if (!pd->parent) _efl_ui_average_model_forgotten_check(obj, pd);
             r = _efl_ui_average_model_compute(obj, r, pd->total.height, pd->total.hseen);
          }
        // We do not need to average the width in vertical list mode as this is done by the parent class
        /* if (eina_streq(property, _efl_model_property_totalw)) */
        /*   r = _efl_ui_average_model_compute(obj, r, pd->total.width, pd->total.wseen); */
     }

   return r;
}

// The rows that were told something that was not added yet outlive the root
// (the update holds a ref on its row): what they point at is gone with it.
static void
_efl_ui_average_model_efl_object_destructor(Eo *obj, Efl_Ui_Average_Model_Data *pd)
{
   Efl_Ui_Average_Model_Update *update;

   EINA_LIST_FREE(pd->pending, update)
     {
        update->dropped = EINA_TRUE;
        update->root = NULL;
        update->total = NULL;
        update->seen = NULL;
     }

   efl_destructor(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS));
}

static Efl_Object *
_efl_ui_average_model_efl_object_constructor(Eo *obj, Efl_Ui_Average_Model_Data *pd)
{
   Eo *parent = efl_parent_get(obj);

   pd->self = obj;
   if (parent && efl_isa(parent, EFL_UI_AVERAGE_MODEL_CLASS))
     pd->parent = efl_data_scope_get(efl_parent_get(obj), EFL_UI_AVERAGE_MODEL_CLASS);

   obj = efl_constructor(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS));
   // (only the root keeps totals) After the Exact_Model's own listener, which
   // moves the sizes and is the first, and before the ones that are told.
   if (obj && !pd->parent)
     efl_event_callback_priority_add(obj, EFL_MODEL_EVENT_CHILD_REMOVED,
                                     EFL_CALLBACK_PRIORITY_BEFORE,
                                     _efl_ui_average_model_child_removed, pd);
   return obj;
}

/* What a view that estimates the rows nobody measured of an Average_Model gives
   them: true if it is one (the rows nobody measured are then an estimate), with
   the height of the average of those that were if any was (known), and a number
   that is another every time that height changes. */
Eina_Bool
_efl_ui_average_model_estimate_get(const Efl_Model *exact, unsigned int *height,
                                   Eina_Bool *known, unsigned int *revision)
{
   Efl_Ui_Average_Model_Data *pd;

   if (!efl_isa(exact, EFL_UI_AVERAGE_MODEL_CLASS)) return EINA_FALSE;
   pd = efl_data_scope_get(exact, EFL_UI_AVERAGE_MODEL_CLASS);
   if (!pd || pd->parent) return EINA_FALSE;
   _efl_ui_average_model_forgotten_check(exact, pd);
   *revision = pd->revision;
   *known = !!pd->total.hseen;
   *height = pd->average;
   return EINA_TRUE;
}

#include "efl_ui_average_model.eo.c"
