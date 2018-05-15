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

static void
_add_dep(Eina_Hash *depset, const Eolian_Unit *dep)
{
   if (!eina_hash_find(depset, &dep))
     eina_hash_add(depset, &dep, dep);
}

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

   /* also covers EOLIAN_TYPE_CLASS */
   if (tp->tdecl)
     _add_dep(depset, ((const Eolian_Object *)tp->tdecl)->unit);
}

static void
_check_expr_cb(const Eolian_Object *obj, void *data)
{
   Eina_Hash *depset = data;
   switch (obj->type)
     {
      case EOLIAN_OBJECT_TYPEDECL:
      case EOLIAN_OBJECT_VARIABLE:
        _add_dep(depset, obj->unit);
      default:
        break;
     }
}

static void
_check_expr(const Eolian_Expression *expr, Eina_Hash *depset)
{
   database_expr_eval(expr->base.unit, (Eolian_Expression *)expr,
                      EOLIAN_MASK_ALL, _check_expr_cb, depset);
}

static void
_check_param(const Eolian_Function_Parameter *arg, Eina_Hash *depset,
             Eina_Hash *chash)
{
   if (arg->type)
     _check_type(arg->type, depset, chash);
   if (arg->value)
     _check_expr(arg->value, depset);
}

static void
_check_function(const Eolian_Function *f, Eina_Hash *depset, Eina_Hash *chash)
{
   if (f->get_ret_type)
     _check_type(f->get_ret_type, depset, chash);
   if (f->set_ret_type)
     _check_type(f->set_ret_type, depset, chash);

   if (f->get_ret_val)
     _check_expr(f->get_ret_val, depset);
   if (f->set_ret_val)
     _check_expr(f->set_ret_val, depset);

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

   _add_dep(depset, cl->base.unit);

   Eina_Iterator *itr = eina_list_iterator_new(cl->inherits);
   const Eolian_Class *icl;
   EINA_ITERATOR_FOREACH(itr, icl)
     _add_dep(depset, icl->base.unit);
   eina_iterator_free(itr);

   const Eolian_Function *fid;
   itr = eina_list_iterator_new(cl->properties);
   EINA_ITERATOR_FOREACH(itr, fid)
     _check_function(fid, depset, chash);
   eina_iterator_free(itr);

   itr = eina_list_iterator_new(cl->methods);
   EINA_ITERATOR_FOREACH(itr, fid)
     _check_function(fid, depset, chash);
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
     _add_dep(depset, part->klass->base.unit);
   eina_iterator_free(itr);
}

static void
_check_typedecl(const Eolian_Typedecl *tp, Eina_Hash *depset, Eina_Hash *chash)
{
   if (_check_cycle(chash, &tp->base))
     return;

   _add_dep(depset, tp->base.unit);

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
                              depset);
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

   _add_dep(depset, v->base.unit);

   _check_type(v->base_type, depset, chash);
   if (v->value)
     _check_expr(v->value, depset);
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

static Eina_Bool
_check_namespaces(const Eolian_Unit *src)
{
   Eina_Bool ret = EINA_TRUE;
   Eina_Iterator *itr = eina_hash_iterator_data_new(src->objects);
   const Eolian_Object *obj;
   EINA_ITERATOR_FOREACH(itr, obj)
     {
        char const *dot = strrchr(obj->name, '.');
        if (!dot)
          continue;
        Eina_Stringshare *ssr = eina_stringshare_add_length(obj->name,
          dot - obj->name);
        const Eolian_Object *cobj = eina_hash_find(src->objects, ssr);
        eina_stringshare_del(ssr);
        if (cobj)
          {
             eolian_state_log_obj(src->state, obj,
               "the namespace of object '%s' conflicts with %s:%d:%d",
               obj->name, cobj->file, cobj->line, cobj->column);
             ret = EINA_FALSE;
          }
     }
   eina_iterator_free(itr);
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

   /* namespace checks */
   if (!_check_namespaces(&state->main.unit))
     ret = EINA_FALSE;

   return ret;
}
