#include "eolian_database.h"

static Eina_Bool
_validate_type(const Eolian_Type *tp)
{
   (void)tp;
   return EINA_TRUE;
}

static Eina_Bool
_validate_expr(const Eolian_Expression *expr, const Eolian_Type *tp)
{
   (void)expr;
   (void)tp;
   return EINA_TRUE;
}

static Eina_Bool
_validate_param(const Eolian_Function_Parameter *param)
{
   if (!_validate_type(param->type))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_function(const Eolian_Function *func)
{
   Eina_List *l;
   const Eolian_Function_Parameter *param;

   if (func->get_ret_type && !_validate_type(func->get_ret_type))
     return EINA_FALSE;

   if (func->set_ret_type && !_validate_type(func->set_ret_type))
     return EINA_FALSE;

   if (func->get_ret_val && !_validate_expr(func->get_ret_val,
                                            func->get_ret_type))
     return EINA_FALSE;

   if (func->set_ret_val && !_validate_expr(func->set_ret_val,
                                            func->set_ret_type))
     return EINA_FALSE;

   EINA_LIST_FOREACH(func->keys, l, param)
     if (!_validate_param(param))
       return EINA_FALSE;

   EINA_LIST_FOREACH(func->params, l, param)
     if (!_validate_param(param))
       return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_event(const Eolian_Event *event)
{
   if (event->type && !_validate_type(event->type))
     return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_validate_class(const Eolian_Class *cl)
{
   Eina_List *l;
   const Eolian_Function *func;
   const Eolian_Event *event;

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->constructors, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(event))
       return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Bool
database_validate(void)
{
   const Eolian_Class *cl;
   Eina_Iterator *iter = eolian_all_classes_get();
   EINA_ITERATOR_FOREACH(iter, cl)
     if (!_validate_class(cl))
       {
          eina_iterator_free(iter);
          return EINA_FALSE;
       }
   eina_iterator_free(iter);
   return EINA_TRUE;
}
