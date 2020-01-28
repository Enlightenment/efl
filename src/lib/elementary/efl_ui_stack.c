#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_STACK_CLASS

typedef struct {

} Efl_Ui_Stack_Data;

EOLIAN static Efl_Object *
_efl_ui_stack_efl_object_constructor(Eo *obj, Efl_Ui_Stack_Data *sd EINA_UNUSED)
{
   Eo *stack;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   stack = efl_new(EFL_UI_SPOTLIGHT_FADE_MANAGER_CLASS);
   efl_ui_spotlight_manager_set(obj, stack);

   return obj;
}

#include "efl_ui_stack.eo.c"
