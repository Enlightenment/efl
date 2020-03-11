#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Spotlight_Fade_Manager_Data;

EOLIAN static Efl_Object*
_efl_ui_spotlight_fade_manager_efl_object_constructor(Eo *obj, Efl_Ui_Spotlight_Fade_Manager_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Animation *in_animation, *out_animation;

   obj = efl_constructor(efl_super(obj, EFL_UI_SPOTLIGHT_FADE_MANAGER_CLASS));

   in_animation = efl_add(EFL_CANVAS_ALPHA_ANIMATION_CLASS, obj);
   efl_animation_alpha_set(in_animation, 0.0, 1.0);
   efl_animation_duration_set(in_animation, 0.5);

   out_animation = efl_add(EFL_CANVAS_ALPHA_ANIMATION_CLASS, obj);
   efl_animation_alpha_set(out_animation, 0.0, 1.0);
   efl_animation_duration_set(out_animation, 0.5);

   efl_ui_spotlight_manager_animation_forward_animation_set(obj, in_animation, out_animation);
   efl_ui_spotlight_manager_animation_backward_animation_set(obj, in_animation, out_animation);
   efl_unref(in_animation);
   efl_unref(out_animation);

   return obj;
}


#include "efl_ui_spotlight_fade_manager.eo.c"
