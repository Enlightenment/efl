#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_item_priv.h"
#include "efl_ui_toolbar_item.eo.h"

#define MY_CLASS EFL_UI_TOOLBAR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Toolbar"

typedef struct {
   Eo               *self;
   Eo               *bx;
   Efl_Ui_Dir        dir;
} Efl_Ui_Toolbar_Data;

static void
_sizing_calc(Efl_Ui_Toolbar_Data *pd)
{
   Eina_Rect r;

   efl_canvas_group_calculate(pd->bx);
   r.pos = efl_gfx_position_get(pd->self);
   r.size = efl_gfx_size_hint_combined_min_get(pd->bx);
   efl_gfx_geometry_set(pd->bx, r);

   efl_gfx_size_hint_restricted_min_set(pd->self, r.size);
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_toolbar_elm_widget_theme_apply(Eo *obj, Efl_Ui_Toolbar_Data *sd)
{
   Efl_Ui_Theme_Apply ret;

   ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!ret) return EFL_UI_THEME_APPLY_FAILED;

   elm_layout_theme_set(obj, "toolbar", "base", elm_widget_style_get(obj));
   if (!efl_ui_dir_is_horizontal(sd->dir, EINA_TRUE))
     efl_canvas_layout_signal_emit(obj, "elm,orient,vertical", "elm");
   else
     efl_canvas_layout_signal_emit(obj, "elm,orient,horizontal", "elm");

   efl_canvas_group_need_recalculate_set(obj, EINA_TRUE);

   return ret;
}

static void
_box_size_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;

   efl_canvas_group_need_recalculate_set(obj, EINA_TRUE);
}

EOLIAN static Eo *
_efl_ui_toolbar_efl_object_constructor(Eo *obj, Efl_Ui_Toolbar_Data *pd)
{
   pd->self = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   elm_widget_sub_object_parent_add(obj);

   elm_layout_theme_set(obj, "toolbar", "base", NULL);
   if (!efl_ui_widget_theme_apply(obj))
     return NULL;

   pd->dir = EFL_UI_DIR_HORIZONTAL;
   pd->bx = efl_add(EFL_UI_BOX_CLASS, obj,
                    efl_ui_direction_set(efl_added, pd->dir),
                    efl_pack_align_set(efl_added, -1.0, -1.0));
   elm_layout_content_set(obj, "elm.swallow.content", pd->bx);
   efl_event_callback_add(pd->bx, EFL_GFX_EVENT_CHANGE_SIZE_HINTS,
                          _box_size_cb, obj);

   return obj;
}

EOLIAN static void
_efl_ui_toolbar_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Toolbar_Data *sd, Efl_Ui_Dir dir)
{
   // Adjust direction to be either horizontal or vertical.
   if (efl_ui_dir_is_horizontal(dir, EINA_TRUE))
     dir = EFL_UI_DIR_HORIZONTAL;
   else
     dir = EFL_UI_DIR_VERTICAL;

   if (sd->dir == dir) return;

   sd->dir = dir;
   efl_ui_direction_set(sd->bx, sd->dir);

   evas_object_smart_changed(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_toolbar_efl_ui_direction_direction_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return sd->dir;
}

EOLIAN void
_efl_ui_toolbar_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd)
{
   _sizing_calc(pd);
}

EOLIAN static const Efl_Class *
_efl_ui_toolbar_efl_ui_item_list_item_class_get(Eo *obj EINA_UNUSED,
                                                Efl_Ui_Toolbar_Data *pd EINA_UNUSED)
{
   return EFL_UI_TOOLBAR_ITEM_CLASS;
}

EOLIAN static Efl_Ui_Item *
_efl_ui_toolbar_efl_ui_item_list_item_add(Eo *obj, Efl_Ui_Toolbar_Data *pd,
                                          const char *text, const char *icon)
{
   Efl_Ui_Item *item = efl_ui_item_list_item_add(efl_super(obj, MY_CLASS), text, icon);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, NULL);

   efl_pack(pd->bx, efl_ui_item_view_get(item));
   return item;
}


// ===========================================================================
//  Toolbar Item
// ===========================================================================

#undef MY_CLASS
#define MY_CLASS EFL_UI_TOOLBAR_ITEM_CLASS

typedef struct {
   Efl_Ui_Toolbar_Item *self;
   Efl_Ui_Toolbar      *parent;
   Efl_Ui_Layout       *view;
} Efl_Ui_Toolbar_Item_Data;

EOLIAN static Eo *
_efl_ui_toolbar_item_efl_object_constructor(Eo *obj, Efl_Ui_Toolbar_Item_Data *pd)
{
   pd->self = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->parent = efl_parent_get(obj);

   return obj;
}

EOLIAN static Eo *
_efl_ui_toolbar_item_efl_object_finalize(Eo *obj, Efl_Ui_Toolbar_Item_Data *pd)
{
   const char *style;

   obj = efl_finalize(efl_super(obj, MY_CLASS));

   // Always realized.
   style = efl_ui_widget_style_get(pd->parent);
   pd->view = efl_add(EFL_UI_LAYOUT_CLASS, pd->parent,
                      efl_ui_widget_style_set(efl_added, style),
                      efl_ui_layout_theme_set(efl_added, "toolbar", "item", style));

   return obj;
}

EOLIAN static Efl_Ui_Layout *
_efl_ui_toolbar_item_efl_ui_item_item_view_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *pd)
{
   return pd->view;
}

#include "efl_ui_toolbar.eo.c"
#include "efl_ui_toolbar_item.eo.c"
