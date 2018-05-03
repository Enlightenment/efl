#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_CANVAS_SCENE_BETA
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_SCROLLBAR_BETA
#define EFL_PART_PROTECTED

#include "eo_internal.h"

#include <Efl.h>

#include "interfaces/efl_config.eo.c"
#include "interfaces/efl_control.eo.c"
#include "interfaces/efl_duplicate.eo.c"
#include "interfaces/efl_gfx_image.eo.c"
#include "interfaces/efl_gfx_image_animation_controller.eo.c"
#include "interfaces/efl_gfx_image_load_controller.eo.c"
#include "interfaces/efl_part.eo.c"
#include "interfaces/efl_playable.eo.c"
#include "interfaces/efl_player.eo.c"
#include "interfaces/efl_text.eo.c"
#include "interfaces/efl_text_font.eo.c"
#include "interfaces/efl_text_style.eo.c"
#include "interfaces/efl_text_format.eo.c"
#include "interfaces/efl_text_cursor.eo.c"
#include "interfaces/efl_text_annotate.eo.c"
#include "interfaces/efl_text_markup.eo.c"

#include "interfaces/efl_gfx_entity.eo.c"
#include "interfaces/efl_gfx_buffer.eo.c"
#include "interfaces/efl_gfx_stack.eo.c"
#include "interfaces/efl_gfx_fill.eo.c"
#include "interfaces/efl_gfx_view.eo.c"

#include "interfaces/efl_gfx_color_class.eo.c"
#include "interfaces/efl_gfx_text_class.eo.c"
#include "interfaces/efl_gfx_size_class.eo.c"

#include "interfaces/efl_gfx_gradient.eo.c"
#include "interfaces/efl_gfx_gradient_linear.eo.c"
#include "interfaces/efl_gfx_gradient_radial.eo.c"

#include "interfaces/efl_gfx_filter.eo.c"
#include "interfaces/efl_gfx_blur.eo.c"

#include "interfaces/efl_gfx_size_hint.eo.c"
#include "interfaces/efl_canvas_scene.eo.c"
#include "interfaces/efl_canvas_pointer.eo.c"

#include "interfaces/efl_screen.eo.c"

/* Packing & containers */
#include "interfaces/efl_container.eo.c"
#include "interfaces/efl_content.eo.c"
#include "interfaces/efl_pack.eo.c"
#include "interfaces/efl_pack_layout.eo.c"
#include "interfaces/efl_pack_linear.eo.c"
#include "interfaces/efl_pack_table.eo.c"

#include "interfaces/efl_model.eo.c"
#include "interfaces/efl_animator.eo.c"
#include "interfaces/efl_orientation.eo.c"
#include "interfaces/efl_ui_base.eo.c"
#include "interfaces/efl_ui_direction.eo.c"
#include "interfaces/efl_ui_drag.eo.c"
#include "interfaces/efl_ui_range.eo.c"
#include "interfaces/efl_ui_autorepeat.eo.c"
#include "interfaces/efl_ui_view.eo.c"
#include "interfaces/efl_ui_model_connect.eo.c"
#include "interfaces/efl_ui_factory.eo.c"

#include "interfaces/efl_ui_draggable.eo.c"
#include "interfaces/efl_ui_clickable.eo.c"
#include "interfaces/efl_ui_scrollable.eo.c"
#include "interfaces/efl_ui_scrollable_interactive.eo.c"
#include "interfaces/efl_ui_scrollbar.eo.c"
#include "interfaces/efl_ui_selectable.eo.c"
#include "interfaces/efl_ui_multi_selectable.eo.c"
#include "interfaces/efl_ui_zoom.eo.c"

static void
_noref_death(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_event_callback_del(event->object, EFL_EVENT_NOREF, _noref_death, NULL);
   efl_del(event->object);
}

EAPI Efl_Object *
efl_part(const Eo *obj, const char *name)
{
   Efl_Object *r;

   r = efl_part_get(obj, name);
   if (!r) return NULL;

   efl_event_callback_add(r, EFL_EVENT_NOREF, _noref_death, NULL);
   ___efl_auto_unref_set(r, EINA_TRUE);

   return efl_ref(r);
}

EAPI void
__efl_internal_init(void)
{
   efl_model_init();
}
