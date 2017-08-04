#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_transition_scroll.h"
#include "efl_ui_widget_pagecontrol.h"


EOLIAN static void
_efl_page_transition_scroll_efl_page_transition_update(Eo *obj EINA_UNUSED,
                                                       Efl_Page_Transition_Scroll_Data *_pd EINA_UNUSED,
                                                       Efl_Canvas_Object *target,
                                                       double position)
{
   if (!target) return;

   Evas_Coord x, y, w, h;
   Evas_Object *parent = efl_parent_get(target);

   //FIXME do this somewhere else
   EFL_UI_PAGECONTROL_DATA_GET(parent, sd);
   efl_gfx_visible_set(sd->viewport.foreclip, EINA_TRUE);
   efl_gfx_visible_set(sd->viewport.backclip, EINA_TRUE);

   efl_gfx_geometry_get(parent, &x, &y, &w, &h);

   if (position <= 0) efl_gfx_position_set(target, x + (w * position), y);
   else efl_gfx_position_set(target, x + w - (w * position), y);
}

#include "efl_page_transition_scroll.eo.c"
