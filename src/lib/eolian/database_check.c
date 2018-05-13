#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"
#include "eolian_priv.h"

static Eina_Bool
_check_cycle(Eina_Hash *chash, const Eolian_Object *obj)
{
   /* need to check this for classes, typedecls, vars (toplevel objects) */
   if (eina_hash_find(chash, &obj))
     return EINA_TRUE;
   eina_hash_add(chash, &obj, obj);
   return EINA_FALSE;
}

static void _check_class(const Eolian_Class *cl, Eina_Hash *depset,
                         Eina_Hash *chash);
static void _check_typedecl(const Eolian_Typedecl *tp, Eina_Hash *depset,
                            Eina_Hash *chash);
static void _check_variable(const Eolian_Variable *v, Eina_Hash *depset,
                            Eina_Hash *chash);

static void
_check_type(const Eolian_Type *tp, Eina_Hash *depset, Eina_Hash *chash)
{
   if (tp->base_type)
      _check_type(tp->base_type, depset, chash);

   if (tp->next_type)
     {
        const Eolian_Type *ntp = tp->next_type;
        for (; ntp; ntp = ntp->next_type)
          _check_type(ntp, depset, chash);
     }

   if (tp->type == EOLIAN_TYPE_CLASS)
     _check_class(tp->klass, depset, chash);
   else if (tp->tdecl)
     _check_typedecl(tp->tdecl, depset, chash);
}

typedef struct _Check_Cb_Data
{
   Eina_Hash *depset;
   Eina_Hash *chash;
} Check_Cb_Data;

static void
_check_expr_cb(const Eolian_Object *obj, void *data)
{
   Check_Cb_Data *d = data;
   switch (obj->type)
     {
      case EOLIAN_OBJECT_TYPEDECL:
        _check_typedecl((const Eolian_Typedecl *)obj, d->depset, d->chash);
        break;
      case EOLIAN_OBJECT_VARIABLE:
        _check_variable((const Eolian_Variable *)obj, d->depset, d->chash);
        break;
      default:
        break;
     }
}

static void
_check_expr(const Eolian_Expression *expr, Eina_Hash *depset, Eina_Hash *chash)
{
   Check_Cb_Data d = { depset, chash };
   database_expr_eval(expr->base.unit, (Eolian_Expression *)expr,
                      EOLIAN_MASK_ALL, _check_expr_cb, &d);
}

static void
_check_param(const Eolian_Function_Parameter *arg, Eina_Hash *depset,
             Eina_Hash *chash)
{
   if (arg->type)
     _check_type(arg->type, depset, chash);
   if (arg->value)
     _check_expr(arg->value, depset, chash);
}

static void
_check_function(const Eolian_Function *f, Eina_Hash *depset, Eina_Hash *chash)
{
   if (f->get_ret_type)
     _check_type(f->get_ret_type, depset, chash);
   if (f->set_ret_type)
     _check_type(f->set_ret_type, depset, chash);

   if (f->get_ret_val)
     _check_expr(f->get_ret_val, depset, chash);
   if (f->set_ret_val)
     _check_expr(f->set_ret_val, depset, chash);

   Eina_List *l;
   const Eolian_Function_Parameter *arg;
   if ((f->type == EOLIAN_METHOD) || (f->type == EOLIAN_FUNCTION_POINTER))
     {
        EINA_LIST_FOREACH(f->params, l, arg)
          _check_param(arg, depset, chash);
     }
   else
     {
        EINA_LIST_FOREACH(f->prop_values, l, arg)
          _check_param(arg, depset, chash);
        EINA_LIST_FOREACH(f->prop_values_get, l, arg)
          _check_param(arg, depset, chash);
        EINA_LIST_FOREACH(f->prop_values_set, l, arg)
          _check_param(arg, depset, chash);
        EINA_LIST_FOREACH(f->prop_keys, l, arg)
          _check_param(arg, depset, chash);
        EINA_LIST_FOREACH(f->prop_keys_get, l, arg)
          _check_param(arg, depset, chash);
        EINA_LIST_FOREACH(f->prop_keys_set, l, arg)
          _check_param(arg, depset, chash);
     }
}

static void
_check_class(const Eolian_Class *cl, Eina_Hash *depset, Eina_Hash *chash)
{
   if (_check_cycle(chash, &cl->base))
     return;

   if (!eina_hash_find(depset, &cl->base.unit))
     eina_hash_add(depset, &cl->base.unit, cl->base.unit);

   Eina_Iterator *itr = eina_list_iterator_new(cl->inherits);
   const Eolian_Class *icl;
   EINA_ITERATOR_FOREACH(itr, icl)
     _check_class(icl, depset, chash);
   eina_iterator_free(itr);

   const Eolian_Implement *impl;
   itr = eina_list_iterator_new(cl->implements);
   EINA_ITERATOR_FOREACH(itr, impl)
     {
        _check_function(impl->foo_id, depset, chash);
        _check_class(impl->klass, depset, chash);
     }
   eina_iterator_free(itr);

   const Eolian_Event *ev;
   itr = eina_list_iterator_new(cl->events);
   EINA_ITERATOR_FOREACH(itr, ev)
     {
        if (ev->type)
          _check_type(ev->type, depset, chash);
     }
   eina_iterator_free(itr);

   const Eolian_Part *part;
   itr = eina_list_iterator_new(cl->parts);
   EINA_ITERATOR_FOREACH(itr, part)
     _check_class(part->klass, depset, chash);
   eina_iterator_free(itr);
}

static void
_check_typedecl(const Eolian_Typedecl *tp, Eina_Hash *depset, Eina_Hash *chash)
{
   if (_check_cycle(chash, &tp->base))
     return;

   if (!eina_hash_find(depset, &tp->base.unit))
     eina_hash_add(depset, &tp->base.unit, tp->base.unit);

   if (tp->base_type)
     _check_type(tp->base_type, depset, chash);

   if (tp->field_list)
     {
        Eina_List *l;
        void *data;
        EINA_LIST_FOREACH(tp->field_list, l, data)
          {
             switch (tp->type)
               {
                case EOLIAN_TYPEDECL_STRUCT:
                  _check_type(((const Eolian_Struct_Type_Field *)data)->type,
                              depset, chash);
                  break;
                case EOLIAN_TYPEDECL_ENUM:
                  _check_expr(((const Eolian_Enum_Type_Field *)data)->value,
                              depset, chash);
                  break;
                default:
                  break;
               }
          }
     }

   if (tp->function_pointer)
     _check_function(tp->function_pointer, depset, chash);
}

static void
_check_variable(const Eolian_Variable *v, Eina_Hash *depset, Eina_Hash *chash)
{
   if (_check_cycle(chash, &v->base))
     return;

   if (!eina_hash_find(depset, &v->base.unit))
     eina_hash_add(depset, &v->base.unit, v->base.unit);

   _check_type(v->base_type, depset, chash);
   if (v->value)
     _check_expr(v->value, depset, chash);
}

static Eina_Bool
_check_unit(const Eolian_Unit *unit)
{
   Eina_Bool ret = EINA_TRUE;
   Eina_Hash *depset = eina_hash_pointer_new(NULL);

   /* collect all real dependencies of the unit */
   Eina_Hash *chash = eina_hash_pointer_new(NULL);
   Eina_Iterator *itr = eolian_unit_objects_get(unit);
   const Eolian_Object *obj;
   EINA_ITERATOR_FOREACH(itr, obj)
     {
        /* skip stuff merged in from children */
        if (obj->unit != unit)
          continue;
        switch (obj->type)
          {
           case EOLIAN_OBJECT_CLASS:
             _check_class((const Eolian_Class *)obj, depset, chash);
             break;
           case EOLIAN_OBJECT_TYPEDECL:
             _check_typedecl((const Eolian_Typedecl *)obj, depset, chash);
             break;
           case EOLIAN_OBJECT_VARIABLE:
             _check_variable((const Eolian_Variable *)obj, depset, chash);
             break;
           default:
             continue;
          }
     }
   eina_hash_free(chash);

   /* check collected deps against children units */
   Eina_Iterator *citr = eolian_unit_children_get(unit);
   const Eolian_Unit *cunit;
   EINA_ITERATOR_FOREACH(citr, cunit)
     {
        if (!eina_hash_find(depset, &cunit))
          {
             eolian_state_log(unit->state, "%s: unused dependency %s",
                              unit->file, cunit->file);
             ret = EINA_FALSE;
          }
     }
   eina_iterator_free(citr);

   eina_hash_free(depset);
   return ret;
}

Eina_Bool
database_check(const Eolian_State *state)
{
   Eina_Bool ret = EINA_TRUE;

   /* check for extra dependencies */
   Eina_Iterator *itr = eolian_state_units_get(state);
   const Eolian_Unit *unit;
   EINA_ITERATOR_FOREACH(itr, unit)
     {
        if (!_check_unit(unit))
          ret = EINA_FALSE;
     }
   eina_iterator_free(itr);

   return ret;
}
