#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pagescroller.h"
#include "efl_ui_widget_pagecontrol.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGESCROLLER_CLASS


EOLIAN static void
_efl_ui_pagescroller_efl_canvas_group_group_add(Eo *obj,
                                                Efl_Ui_Pagescroller_Data *pd EINA_UNUSED)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_pagescroller_update(Eo *obj,
                            Efl_Ui_Pagescroller_Data *pd)
{
   EFL_UI_PAGECONTROL_DATA_GET(obj, pcd);
   //FIXME need or need not update when an item is appended

   Eina_List *l;
   Evas_Object *subobj, *o1 = NULL, *o2 = NULL;
   int p1 = -1, p2 = -1;

   // All the pages go to background - backclip is essential!
   // in order to hide pages regardless of their visibility
   // geometry isn't enough - think about cube effect
   EINA_LIST_FOREACH(pcd->content_list, l, subobj)
     {
        efl_gfx_size_set(subobj, pcd->w, pcd->h);
        evas_object_clip_set(subobj, pcd->hidden_clip);
     }

   // Only the current and the next page go into the viewport
   p1 = pcd->page;
   o1 = eina_list_nth(pcd->content_list, p1);
   evas_object_clip_set(o1, pcd->viewport.foreclip);

   if (pcd->ratio != 0)
     {
        p2 = (p1 + 1 + pcd->cnt) % pcd->cnt;
        o2 = eina_list_nth(pcd->content_list, p2);
        evas_object_clip_set(o2, pcd->viewport.backclip);
     }

   // Transition effect
   efl_page_transition_update(pd->transition, o1, (-1 * pcd->ratio));
   efl_page_transition_update(pd->transition, o2, pcd->ratio);

   efl_ui_pagecontrol_update(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Page_Transition *
_efl_ui_pagescroller_transition_get(Eo *obj EINA_UNUSED,
                                    Efl_Ui_Pagescroller_Data *sd)
{
   return sd->transition;
}

EOLIAN static void
_efl_ui_pagescroller_transition_set(Eo *obj EINA_UNUSED,
                                    Efl_Ui_Pagescroller_Data *sd,
                                    Efl_Page_Transition *transition)
{
   sd->transition = transition;
}

EOLIAN static void
_efl_ui_pagescroller_efl_gfx_size_set(Eo *obj,
                                      Efl_Ui_Pagescroller_Data *sd EINA_UNUSED,
                                      Evas_Coord w,
                                      Evas_Coord h)
{
   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}


#define EFL_UI_PAGESCROLLER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_pagescroller), \
   EFL_OBJECT_OP_FUNC(efl_ui_pagecontrol_update, _efl_ui_pagescroller_update)

#include "efl_ui_pagescroller.eo.c"
