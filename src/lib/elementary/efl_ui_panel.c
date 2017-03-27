#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_panel.h"

#define MY_CLASS EFL_UI_PANEL_CLASS

#define MY_CLASS_NAME "Efl_Ui_Panel"



EOLIAN static void
_efl_ui_panel_toggle(Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd)
{
   if (sd->state == EFL_UI_PANEL_STATE_CLOSED)
     {
        switch (sd->orient)
          {
           case EFL_ORIENT_LEFT:
              elm_interface_scrollable_region_bring_in(sd->scroller,
                                                       0, 0, sd->w, sd->h);
              break;
           case EFL_ORIENT_RIGHT:
           case EFL_ORIENT_UP:
           case EFL_ORIENT_DOWN:
              break;
          }
        sd->state = EFL_UI_PANEL_STATE_OPEN;
     }
   else if (sd->state == EFL_UI_PANEL_STATE_OPEN)
     {
        switch (sd->orient)
          {
           case EFL_ORIENT_LEFT:
              elm_interface_scrollable_region_bring_in(sd->scroller,
                                                       (sd->w * 0.8), 0, sd->w, sd->h);
              break;
           case EFL_ORIENT_RIGHT:
           case EFL_ORIENT_UP:
           case EFL_ORIENT_DOWN:
              break;
          }
        sd->state = EFL_UI_PANEL_STATE_CLOSED;
     }
}

EOLIAN static void
_efl_ui_panel_efl_orientation_orientation_set(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Panel_Data *sd,
                                              Efl_Orient orient)
{
   if (sd->orient == orient) return;

   sd->orient = orient;
   //TODO change orientation

   switch (orient)
     {
      case EFL_ORIENT_LEFT:
      case EFL_ORIENT_RIGHT:
      case EFL_ORIENT_UP:
      case EFL_ORIENT_DOWN:
         break;
     }
}

EOLIAN static void
_efl_ui_panel_efl_gfx_visible_set(Eo *obj, Efl_Ui_Panel_Data *sd, Eina_Bool vis)
{
   efl_gfx_visible_set(efl_super(obj, MY_CLASS), vis);

   efl_gfx_visible_set(sd->scroller, vis);
   efl_gfx_visible_set(sd->box, vis);
   efl_gfx_visible_set(sd->content_table, vis);
   efl_gfx_visible_set(sd->content_spacer, vis);
   efl_gfx_visible_set(sd->event_spacer, vis);

   efl_gfx_visible_set(sd->event_block, vis);
}

EOLIAN static void
_efl_ui_panel_efl_gfx_position_set(Eo *obj, Efl_Ui_Panel_Data *sd,
                                   Evas_Coord x, Evas_Coord y)
{
   sd->x = x;
   sd->y = y;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   efl_gfx_position_set(sd->scroller, x, y);
   efl_gfx_position_set(sd->event_block, x, y);
}

EOLIAN static void
_efl_ui_panel_efl_gfx_size_set(Eo *obj, Efl_Ui_Panel_Data *sd,
                               Evas_Coord w, Evas_Coord h)
{
   Evas_Coord content_w = w, content_h = h;

   sd->w = w;
   sd->h = h;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   switch (sd->orient)
     {
      case EFL_ORIENT_LEFT:
      case EFL_ORIENT_RIGHT:
         content_w = w * 0.8;
         break;
      case EFL_ORIENT_UP:
      case EFL_ORIENT_DOWN:
         content_h = h * 0.8;
         break;
     }

   efl_gfx_size_set(sd->scroller, w, h);
   efl_gfx_size_set(sd->event_block, w, h);

   evas_object_size_hint_min_set(sd->content_spacer, content_w, content_h);
   evas_object_size_hint_min_set(sd->event_spacer, w, h);

   if (sd->state == EFL_UI_PANEL_STATE_CLOSED)
     {
        switch (sd->orient)
          {
           case EFL_ORIENT_LEFT:
              elm_interface_scrollable_content_region_show(sd->scroller,
                                                           content_w, 0, w, h);
              break;
           case EFL_ORIENT_RIGHT:
           case EFL_ORIENT_UP:
           case EFL_ORIENT_DOWN:
              break;
          }
     }
}

EOLIAN static Eina_Bool
_efl_ui_panel_efl_container_content_set(Eo *obj EINA_UNUSED, Efl_Ui_Panel_Data *sd,
                                        Evas_Object *content)
{
   if (sd->content == content) return EINA_TRUE;

   if (sd->content)
     elm_table_unpack(sd->content_table, sd->content);

   sd->content = content;
   elm_table_pack(sd->content_table, content, 0, 0, 1, 1);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_panel_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Panel_Data *sd)
{
   /** in case of a left panel
     *
     * -------- scroller ---------
     * | --------- box --------- |
     * | | - table - - rect -- | |
     * | | | rect  | |       | | |
     * | | | / obj | |       | | |
     * | | --------- --------- | |
     * | ----------------------- |
     * ---------------------------
     */

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   sd->orient = EFL_ORIENT_LEFT; // left panel by default
   sd->state = EFL_UI_PANEL_STATE_CLOSED; // closed by default

   sd->scroller = efl_add(ELM_SCROLLER_CLASS, obj);
   //TODO set "panel" style to the scroller
   elm_widget_sub_object_add(obj, sd->scroller);
   elm_interface_scrollable_bounce_allow_set(sd->scroller, EINA_FALSE, EINA_FALSE);
   elm_interface_scrollable_policy_set(sd->scroller, ELM_SCROLLER_POLICY_OFF,
                                       ELM_SCROLLER_POLICY_OFF);

   sd->box = efl_add(EFL_UI_BOX_CLASS, sd->scroller);
   efl_orientation_set(sd->box, EFL_ORIENT_RIGHT);
   elm_widget_sub_object_add(obj, sd->box);
   elm_object_content_set(sd->scroller, sd->box);

   sd->content_table = elm_table_add(sd->box);
   elm_widget_sub_object_add(obj, sd->content_table);
   efl_pack_end(sd->box, sd->content_table);

   sd->content_spacer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(sd->content_spacer, 0, 0, 0, 0);
   elm_table_pack(sd->content_table, sd->content_spacer, 0, 0, 1, 1);

   sd->event_spacer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(sd->event_spacer, 0, 0, 0, 0);
   efl_pack_end(sd->box, sd->event_spacer);

   sd->event_block = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(sd->event_block, 0, 0, 0, 0);
}

EOLIAN static Eo *
_efl_ui_panel_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Panel_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

#include "efl_ui_panel.eo.c"
