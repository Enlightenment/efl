#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_mi_state_private.h"
//#include "efl_ui_mi_controller_part.eo.h"
//#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_MI_STATE_CLASS

#define MY_CLASS_NAME "Efl_Ui_Mi_State"


EOLIAN static void
_efl_ui_mi_state_sector_set(Eo *eo_obj, Efl_Ui_Mi_State_Data *pd,
                                const char *start, const char *end)
{
   if (!start) return ;

   if (pd->start) free(pd->start);
   pd->start = strdup(start);

   if (!end) return ;

   if (pd->end) free(pd->end);
   pd->end = strdup(end);

}

EOLIAN void
_efl_ui_mi_state_sector_get(const Eo *obj, Efl_Ui_Mi_State_Data *pd, const char **start, const char **end)
{
   if (start)
     *start = pd->start;
   if (end)
     *end = pd->end;
}

EOLIAN void
_efl_ui_mi_state_rule_set(Eo *obj, Efl_Ui_Mi_State_Data *pd, const char *keypath, Efl_Ui_Mi_Rule *rule)
{

}

EOLIAN Efl_Ui_Mi_Rule*
_efl_ui_mi_state_rule_get(const Eo *obj, Efl_Ui_Mi_State_Data *pd, const char *keypath)
{
   return 0;
}


EOLIAN static void
_efl_ui_mi_state_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Mi_State_Data *pd)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);


   pd->start = NULL;
   pd->end = NULL;
}

EOLIAN static void
_efl_ui_mi_state_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Mi_State_Data *pd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));

   if (pd->start) free(pd->start);
   if (pd->end) free(pd->end);
}

EOLIAN static void
_efl_ui_mi_state_efl_object_destructor(Eo *obj,
                                          Efl_Ui_Mi_State_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_mi_state_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Mi_State_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   //evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

/* Efl.Part begin */
/*ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_State_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_State_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_State_Data)
#include "efl_ui_mi_controller_part.eo.c"*/
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_MI_STATE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_mi_state)

#include "efl_ui_mi_state.eo.c"
