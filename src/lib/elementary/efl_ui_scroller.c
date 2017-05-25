#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroller.h"
#include "efl_ui_widget_scroll_manager.h"
#include "elm_widget_layout.h"

#define MY_CLASS EFL_UI_SCROLLER_CLASS
#define MY_CLASS_NAME "Efl_Ui_Scroller"


EOLIAN static void
_efl_ui_scroller_efl_gfx_size_set(Eo *obj,
                                  Efl_Ui_Scroller_Data *sd,
                                  Evas_Coord w,
                                  Evas_Coord h)
{
   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   efl_gfx_size_set(sd->smanager, w, h);
}

EOLIAN static void
_efl_ui_scroller_efl_gfx_position_set(Eo *obj,
                                      Efl_Ui_Scroller_Data *sd,
                                      Evas_Coord x,
                                      Evas_Coord y)
{
   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   efl_gfx_position_set(sd->smanager, x, y);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_container_content_set(Eo *obj EINA_UNUSED,
                                           Efl_Ui_Scroller_Data *sd,
                                           Evas_Object *content)
{
   efl_ui_scroll_manager_content_set(sd->smanager, content);
}

EOLIAN static void
_efl_ui_scroller_efl_canvas_group_group_add(Eo *obj,
                                            Efl_Ui_Scroller_Data *sd)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_layout_theme_set(obj, "scroller", "base", elm_widget_style_get(obj));

   sd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_constructor(Eo *obj,
                                        Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}


#include "efl_ui_scroller.eo.c"
