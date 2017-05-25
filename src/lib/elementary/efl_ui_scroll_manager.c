#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroll_manager.h"

#define MY_CLASS EFL_UI_SCROLL_MANAGER_CLASS
#define MY_CLASS_NAME "Efl_Ui_Scroll_Manager"


EOLIAN static void
_efl_ui_scroll_manager_efl_gfx_size_set(Eo *obj EINA_UNUSED,
                                        Efl_Ui_Scroll_Manager_Data *sd,
                                        Evas_Coord w,
                                        Evas_Coord h)
{
   efl_gfx_size_set(sd->hit_rect, w, h);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_gfx_position_set(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Scroll_Manager_Data *sd,
                                            Evas_Coord x,
                                            Evas_Coord y)
{
   efl_gfx_position_set(sd->hit_rect, x, y);
}

static void
_resize_cb(Evas_Object *obj, Evas_Coord w EINA_UNUSED, Evas_Coord h EINA_UNUSED)
{
   Evas_Coord minw, minh;
   Efl_Ui_Scroll_Manager_Data *sd = efl_data_scope_get(obj, EFL_UI_SCROLL_MANAGER_CLASS);

   efl_gfx_size_hint_combined_min_get(sd->content, &minw, &minh);
   efl_gfx_size_set(sd->content, minw, minh);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_canvas_group_group_add(Eo *obj,
                                                  Efl_Ui_Scroll_Manager_Data *sd)
{
   sd->parent = efl_parent_get(obj);
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->parent, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   sd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   efl_gfx_color_set(sd->hit_rect, 0, 0, 0, 0);
   efl_gfx_visible_set(sd->hit_rect, EINA_TRUE);

   elm_interface_scrollable_objects_set(obj, wd->resize_obj, sd->hit_rect);

   elm_interface_scrollable_content_viewport_resize_cb_set(obj, _resize_cb);
}

EOLIAN static void
_efl_ui_scroll_manager_content_set(Eo *obj,
                                   Efl_Ui_Scroll_Manager_Data *sd,
                                   Evas_Object *content)
{
   if (sd->content == content) return;

   sd->content = content;
   efl_canvas_group_member_add(obj, content);
   elm_interface_scrollable_content_set(obj, content);
}

EOLIAN static Eo *
_efl_ui_scroll_manager_efl_object_constructor(Eo *obj,
                                              Efl_Ui_Scroll_Manager_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}


#include "efl_ui_scroll_manager.eo.c"
