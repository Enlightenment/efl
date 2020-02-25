#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efl_Ui.h>
#include "elm_priv.h"

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#ifdef BUILD_VG_LOADER_JSON

typedef struct _App_Data
{
   Eo *label;
   Eo *slider;
} App_Data;

Evas_Object *values[4], *anim_view;
Evas_Object *path_entry, *type_hoversel;

Eina_Bool
add_value_provider(char* new_path, char* new_type, char* new_values)
{
   const char* type = elm_object_text_get(type_hoversel);
   if (!type) return EINA_FALSE;
   const char* path = efl_text_get(path_entry);
   if (!path) return EINA_FALSE;

   if (strstr(type, "Color") != NULL)
     {
        int color[4];
        Eo *vp = efl_add(EFL_GFX_VG_VALUE_PROVIDER_CLASS, anim_view);
        efl_gfx_vg_value_provider_keypath_set(vp, (char*)path);
        for (int i = 0; i < 4; i++)
          {
             char* v = (char*)efl_text_get(values[i]);
             if (v) color[i] = atoi(v);
          }

        sprintf(new_path, "%s", path);
        sprintf(new_values, "%d %d %d %d",color[0], color[1], color[2], color[3]);
        if (!strcmp(type, "FillColor"))
          {
             efl_gfx_vg_value_provider_fill_color_set(vp, color[0], color[1], color[2], color[3]);
             sprintf(new_type, "FillColor");
          }
        if (!strcmp(type, "StrokeColor")) 
          {
             efl_gfx_vg_value_provider_stroke_color_set(vp, color[0], color[1], color[2], color[3]);
             sprintf(new_type, "StrokeColor");
          }
        efl_ui_vg_animation_value_provider_override(anim_view, vp);
     }
   if (!strcmp(type, "StrokeWidth"))
     {
        double width;
        Eo *vp = efl_add(EFL_GFX_VG_VALUE_PROVIDER_CLASS, anim_view);
        efl_gfx_vg_value_provider_keypath_set(vp, (char*)path);
        char* v = (char*)efl_text_get(values[0]);
        if (v) width = strtod(v, NULL);
        efl_gfx_vg_value_provider_stroke_width_set(vp, width);
        efl_ui_vg_animation_value_provider_override(anim_view, vp);
        sprintf(new_path, "%s", path);
        sprintf(new_type, "StrokeWidth");
        sprintf(new_values, "%f", width);
     }
   if  (strstr(type, "Tr"))
     {
        double value[2], value_cnt;
        Eina_Matrix4 m;
        Eo *vp = efl_add(EFL_GFX_VG_VALUE_PROVIDER_CLASS, anim_view);

        efl_gfx_vg_value_provider_keypath_set(vp, (char*)path);

        value_cnt = strstr(type, "Rotation") ? 1 : 2;
        for( int i = 0; i < value_cnt; i++)
          {
             char* v = (char*)efl_text_get(values[i]);
             if (v) value[i] = atof(v);
          }

        eina_matrix4_identity(&m);
        if (!strcmp(type, "TrPosition"))
          {
             // Z projection
             eina_matrix4_translate(&m, value[0], value[1], 0);
             sprintf(new_type, "TrPosition");
             sprintf(new_values, "%f %f",value[0], value[1]);

          }
        else if (!strcmp(type, "TrScale"))
          {
             // Z projection
             eina_matrix4_scale(&m, value[0], value[1], 1);
             sprintf(new_type, "TrScale");
             sprintf(new_values, "%f %f",value[0], value[1]);
          }
        else if (!strcmp(type, "TrRotation"))
          {
             // Z projection
             eina_matrix4_rotate(&m, value[0] * (M_PI / 180), EINA_MATRIX_AXIS_Z); //degree to radian
             sprintf(new_values, "%f",value[0]);
             sprintf(new_type, "TrRotation");
          }

        sprintf(new_path, "%s", path);
        efl_gfx_vg_value_provider_transform_set(vp, &m);
        efl_ui_vg_animation_value_provider_override(anim_view, vp);
     }
   return EINA_TRUE;
}

static void
btn_clicked_cb(void *data , const Efl_Event *ev )
{
   const char *text = efl_text_get(ev->object);

   if (!text) return;

   if (!strcmp("Play", text))
     {
        double speed = efl_player_playback_speed_get(anim_view);
        efl_player_playback_speed_set(anim_view, speed < 0 ? speed * -1 : speed);
        efl_player_playing_set(anim_view, EINA_TRUE);
     }
   else if (!strcmp("Pause", text))
     efl_player_paused_set((Evas_Object*)data, EINA_TRUE);
   else if (!strcmp("Resume", text))
     efl_player_paused_set((Evas_Object*)data, EINA_FALSE);
   else if (!strcmp("Play Backwards", text))
     {
        double speed = efl_player_playback_speed_get(anim_view);
        efl_player_playback_speed_set(anim_view, speed > 0 ? speed * -1 : speed);
        efl_player_playing_set(anim_view, EINA_TRUE);
     }
   else if (!strcmp("Stop", text))
     efl_player_playing_set((Evas_Object*)data, EINA_FALSE);
   else if (!strcmp("ADD", text))
     {
        Evas_Object *list = (Evas_Object*)data;
        Elm_Object_Item *list_it;
        char new_path[255], new_type[255], new_values[255];
        if (add_value_provider(new_path, new_type, new_values))
          {
             char buf[765];
             //TODO: Even if there is the same path as the existing item, it is added without updating.
             //      In efl_ui_vg_animation, duplicate paths are managed.
             //      However, animator (lottie) does not have an implementation that manages overridden values.
             /*Eina_List *items = (Eina_List*)elm_list_items_get(list);
             Eina_List *l;
             EINA_LIST_FOREACH(items, l, list_it)
               {
                  char item_text[255];
                  strcpy(item_text, elm_object_item_text_get(list_it));
                  if (item_text[0] != '/')
                    {
                       char* path = strtok(item_text, "/");
                       char* type = strtok(NULL, "/");
                       if (!strcmp(new_path, path) && !strcmp(new_type, type))
                          {
                             elm_object_item_del(list_it);
                             break;
                          }
                    }
               }*/
             snprintf(buf, sizeof(buf), "%s/%s/%s", new_path, new_type, new_values);
             list_it = elm_list_item_append(list, buf, NULL, NULL, NULL, NULL);
             elm_list_item_bring_in(list_it);
             elm_list_go(list);
          }
     }
   else if (!strcmp("DEL", text))
     {
        Evas_Object *list = (Evas_Object*)data;
        Elm_Object_Item *list_it = elm_list_selected_item_get(list);
        if (list_it)
          {
             char item_text[255];
             strcpy(item_text, elm_object_item_text_get(list_it));
             if (item_text[0] != '/')
               {
                   /* Need to work */
               }
             //TODO
             printf("Value provider remove is not implemented yet\n");
             /*elm_object_item_del(list_it);
             elm_list_go(list);*/
          }
     }
}

static void
check_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   efl_player_playback_loop_set(anim_view, efl_ui_selectable_selected_get(event->object));
}

static void
speed_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   double speed = 1;
   if (efl_ui_selectable_selected_get(event->object)) speed = 0.25;
   efl_player_playback_speed_set(anim_view, speed);
}

static void
limit_frame_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   int frame_count = efl_ui_vg_animation_frame_count_get(anim_view);
   printf("Total Frame Count : %d\n", frame_count);
   if (efl_ui_selectable_selected_get(event->object))
     {
        efl_ui_vg_animation_min_frame_set(anim_view, 5);
        efl_ui_vg_animation_max_frame_set(anim_view, 10);
        printf("Frames to show 5-10 only\n");
     }
   else
     {
        efl_ui_vg_animation_min_frame_set(anim_view, 0);
        efl_ui_vg_animation_max_frame_set(anim_view, frame_count);
        printf("Showing all frames now\n");
     }
}

static void
_slider_changed_cb(void *data, const Efl_Event *ev)
{
   Evas_Object *anim_view = data;
   efl_player_playback_progress_set(anim_view, efl_ui_range_value_get(ev->object));
}

void values_input(Eo* box, const char* type)
{
   for (int i = 0; i < 4; i++)
     {
        if (values[i])
          {
             //efl_pack_unpack(box, values[i]);
             efl_del(values[i]);
             values[i] = NULL;
          }
     }
   if (strstr(type, "Color") != NULL)
     {
        char color_text[4][2] = { "R", "G", "B", "A" };
        for (int i = 0; i < 4; i++)
          {
             values[i] =  efl_add(EFL_UI_TEXTBOX_CLASS, box,
                                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                                  efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                                  efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                                  efl_text_interactive_editable_set(efl_added, EINA_TRUE),
                                  efl_pack(box, efl_added));
             efl_gfx_hint_size_min_set(values[i], EINA_SIZE2D(50, 10));
             efl_text_set(efl_part(values[i], "efl.text_guide"), color_text[i]);
          }
     }
   else if (!strcmp(type, "StrokeWidth"))
     {
        values[0] =  efl_add(EFL_UI_TEXTBOX_CLASS, box,
                             efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                             efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                             efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                             efl_text_interactive_editable_set(efl_added, EINA_TRUE),
                             efl_pack(box, efl_added));
        efl_gfx_hint_size_min_set(values[0], EINA_SIZE2D(50, 10));
        efl_text_set(efl_part(values[0], "efl.text_guide"), "Width(double type)");
     }
   else if (strstr(type, "Tr"))
     {
        char text[2][2];
        if (!strcmp(type, "TrPosition"))
          {
             snprintf(text[0], sizeof(text[0]), "X");
             snprintf(text[1], sizeof(text[1]), "Y");
          }
        else if (!strcmp(type, "TrScale"))
          {
             snprintf(text[0], sizeof(text[0]), "W");
             snprintf(text[1], sizeof(text[1]), "H");
          }
        else if (!strcmp(type, "TrRotation"))
          {
             snprintf(text[0], sizeof(text[0]), "R");
          }

        int value_cnt = strstr(type, "Rotation") ? 1 : 2;
        for( int i = 0; i < value_cnt; i++)
          {
             values[i] =  efl_add(EFL_UI_TEXTBOX_CLASS, box,
                                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                                  efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                                  efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                                  efl_text_interactive_editable_set(efl_added, EINA_TRUE),
                                  efl_pack(box, efl_added));
             efl_gfx_hint_size_min_set(values[i], EINA_SIZE2D(50, 10));
             efl_text_set(efl_part(values[i], "efl.text_guide"), text[i]);
          }
     }
}

static void
_hover_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object* box = (Evas_Object*)data;
   const char *selected = elm_object_item_text_get(event_info);

   elm_object_text_set(obj, selected);
   values_input(box, selected);
}

static void
update_anim_view_state(Evas_Object *anim_view, Evas_Object *label)
{
   Efl_Ui_Vg_Animation_State state = efl_ui_vg_animation_state_get(anim_view);

   switch (state)
     {
      case EFL_UI_VG_ANIMATION_STATE_NOT_READY:
         efl_text_set(label, "State = Not Ready");
         break;
      case EFL_UI_VG_ANIMATION_STATE_PLAYING:
         efl_text_set(label, "State = Playing");
         break;
      case EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS:
         efl_text_set(label, "State = Playing Backwards");

         break;
      case EFL_UI_VG_ANIMATION_STATE_PAUSED:
         efl_text_set(label, "State = Paused");
         break;
      case EFL_UI_VG_ANIMATION_STATE_STOPPED:
         efl_text_set(label, "State = Stopped");
         break;
     }
}

static void
_animation_playing_changed_cb(void *data, const Efl_Event *event)
{
   Eina_Bool playing = *(Eina_Bool*)event->info;
   App_Data *ad = data;
   update_anim_view_state(event->object, ad->label);
   //Stopped
   if (!playing)
     efl_ui_range_value_set(ad->slider, 0);
}

static void
_animation_paused_changed_cb(void *data, const Efl_Event *event)
{
   App_Data *ad = data;
   update_anim_view_state(event->object, ad->label);
}

static void
_animation_playback_progress_changed_cb(void *data, const Efl_Event *event)
{
   double progress = *(double*)event->info;
   App_Data *ad = data;
   efl_ui_range_value_set(ad->slider, progress);
}

static void
_animation_playback_repeated_changed_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   int repeated_times = *(int*)event->info;
   printf("repeated! (times: %d)\n", repeated_times);
}

static void
_animation_playback_finished_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("done!\n");
}

EFL_CALLBACKS_ARRAY_DEFINE(animation_stats_cb,
  {EFL_PLAYER_EVENT_PLAYING_CHANGED, _animation_playing_changed_cb },
  {EFL_PLAYER_EVENT_PAUSED_CHANGED, _animation_paused_changed_cb },
  {EFL_PLAYER_EVENT_PLAYBACK_PROGRESS_CHANGED, _animation_playback_progress_changed_cb },
  {EFL_PLAYER_EVENT_PLAYBACK_REPEATED, _animation_playback_repeated_changed_cb },
  {EFL_PLAYER_EVENT_PLAYBACK_FINISHED, _animation_playback_finished_changed_cb },
)

static void
_win_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_gfx_vg_value_provider(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *box_sub, *label, *check, *slider, *list;
   char buf[255];
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   // This line must to need.
   setenv("ELM_ACCEL", "gl", 1);

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl_Ui_Vg_Animation demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, _win_del_cb, ad));

   // Create a box in Canvas
   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                 efl_content_set(win, efl_added));

   //Create a label to show state of animation
   label = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_pack(box, efl_added));

   //Create Animation View to play animation directly from JSON file
   snprintf(buf, sizeof(buf), "%s/images/three_box.json", elm_app_data_dir_get());
   anim_view = efl_add(EFL_UI_VG_ANIMATION_CLASS, win,
                       efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                       efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                       efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(600, 600)),
                       efl_file_set(efl_added, buf),
                       efl_pack(box, efl_added));

//-----------------------------------------------------------------------------------
   box_sub = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                  efl_pack(box, efl_added));
   //Path
   efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
           efl_text_set(efl_added, "Path examples : three_box.json\n"
                                   "**\n"
                                   "layer.*.Fill 1\n"
                                   "layer.box1.*\n"
                                   "layer.box1.Fill 1\n"
                                   "layer.box1.Stroke 1\n"
                                   "layer.box_sub.Fill 1\n"
                                   "layer.box_sub.Stroke 1\n"
                                   "layer.box_sub.Fill 1\n"
                                   "layer.box_sub.Stroke 1\n"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_text_multiline_set(efl_added, EINA_TRUE),
           efl_pack(box_sub, efl_added));

   efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, 0, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
           efl_text_set(efl_added, "[Value Provider]"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_text_multiline_set(efl_added, EINA_TRUE),
           efl_pack(box_sub, efl_added));
//-----------------------------------------------------------------------------------
   // Controller Set : 0
   box_sub = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Path
   efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, 0, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "PATH : "),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(box_sub, efl_added));

   path_entry = efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
                   efl_gfx_hint_weight_set(efl_added, 0.1, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_text_interactive_editable_set(efl_added, EINA_TRUE),
                   efl_pack(box_sub, efl_added));
   efl_gfx_hint_size_min_set(path_entry, EINA_SIZE2D(200, 10));
   efl_text_set(efl_part(path_entry, "efl.text_guide"), "Enter path(ex: ** or Layer.Shape.Fill) ");

   efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, 0, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "TYPE : "),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(box_sub, efl_added));
   type_hoversel = elm_hoversel_add(box_sub);
   elm_hoversel_hover_parent_set(type_hoversel, win);
   evas_object_size_hint_weight_set(type_hoversel, 0, 0);
   evas_object_size_hint_align_set(type_hoversel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_hint_size_min_set(type_hoversel, EINA_SIZE2D(100, 10)),
   elm_object_text_set(type_hoversel, "FillColor");
   elm_hoversel_item_add(type_hoversel, "FillColor", NULL, ELM_ICON_NONE, _hover_item_selected_cb, box_sub);
   elm_hoversel_item_add(type_hoversel, "StrokeColor", NULL, ELM_ICON_NONE, _hover_item_selected_cb, box_sub);
   elm_hoversel_item_add(type_hoversel, "StrokeWidth", NULL, ELM_ICON_NONE, _hover_item_selected_cb, box_sub);
   elm_hoversel_item_add(type_hoversel, "TrPosition", NULL, ELM_ICON_NONE, _hover_item_selected_cb, box_sub);
   elm_hoversel_item_add(type_hoversel, "TrScale", NULL, ELM_ICON_NONE, _hover_item_selected_cb, box_sub);
   elm_hoversel_item_add(type_hoversel, "TrRotation", NULL, ELM_ICON_NONE, _hover_item_selected_cb, box_sub);
   evas_object_show(type_hoversel);
   elm_object_focus_set(type_hoversel, EINA_TRUE);
   efl_pack(box_sub, type_hoversel);

   efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, 0, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "VALUES : "),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(box_sub, efl_added));
   values_input(box_sub, elm_object_text_get(type_hoversel));

//-----------------------------------------------------------------------------------

   // Controller Set : 0
   box_sub = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   list = elm_list_add(win);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);
   efl_pack(box_sub, list);

   elm_object_item_disabled_set(elm_list_item_append(list, "Example : ** / FillColor / 255, 255, 255, 255", NULL, NULL, NULL, NULL), EINA_TRUE);
   efl_gfx_hint_size_min_set(list, EINA_SIZE2D(400, 100)),
   elm_list_go(list);



   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, 0, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "ADD"),
           efl_pack(box_sub, efl_added),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(50, 20)),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, list));

   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, 0, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "DEL"),
           efl_pack(box_sub, efl_added),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(50, 20)),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, list));
//-----------------------------------------------------------------------------------
   // Controller Set : 0
   box_sub = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Loop
   check = efl_add(EFL_UI_CHECK_CLASS, box_sub,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_pack(box_sub, efl_added));
   efl_text_set(check, "Loop");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED,  check_changed_cb, anim_view);

   //Speed: 0.5x
   check = efl_add(EFL_UI_CHECK_CLASS, box_sub,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_pack(box_sub, efl_added));
   efl_text_set(check, "Speed: 0.25x");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED,  speed_changed_cb, anim_view);

   //Limit Frames
   check = efl_add(EFL_UI_CHECK_CLASS, box_sub,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_pack(box_sub, efl_added));
   efl_text_set(check, "Limit Frames");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED,  limit_frame_cb, anim_view);


   //Duration Text
   snprintf(buf, sizeof(buf), "Duration(Length): %1.2fs", efl_playable_length_get(anim_view));
   efl_add(EFL_UI_TEXTBOX_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, buf),
           efl_pack(box_sub, efl_added));

   //Slider
   slider = efl_add(EFL_UI_SLIDER_CLASS, box,
                    efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                    efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                    efl_ui_range_limits_set(efl_added, 0, 1),
                    efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _slider_changed_cb, anim_view),
                    efl_pack(box, efl_added));

   //Controller Set: 1
   box_sub = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                  efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, 1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Play Button
   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Play"),
           efl_pack(box_sub, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));


   //Play Back Button
   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Play Backwards"),
           efl_pack(box_sub, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   //Stop Button
   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Stop"),
           efl_pack(box_sub, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   //Controller Set: 2
   box_sub = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                  efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, 1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Pause Button
   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Pause"),
           efl_pack(box_sub, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   //Resume Button
   efl_add(EFL_UI_BUTTON_CLASS, box_sub,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Resume"),
           efl_pack(box_sub, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   efl_event_callback_array_add(anim_view, animation_stats_cb(), ad);

   ad->label = label;
   ad->slider = slider;

   update_anim_view_state(anim_view, label);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(600, 850));
}

#else

void
test_efl_gfx_vg_value_provider(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box;
   char buf[255];

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl_Ui_Vg_Animation demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   // Create a box
   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                 efl_content_set(win, efl_added));

   efl_add(EFL_UI_TEXTBOX_CLASS, win,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_text_set(efl_added, "Evas Vg Json (Lottie) Loader is not supported, Only Static Vector Image is available!"),
           efl_pack(box, efl_added));

   //Create Vector object.
   snprintf(buf, sizeof(buf), "%s/images/tiger.svg", elm_app_data_dir_get());
   efl_add(EFL_CANVAS_VG_OBJECT_CLASS, win,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(600, 600)),
           efl_file_simple_load(efl_added, buf, NULL),
           efl_gfx_entity_visible_set(efl_added, EINA_TRUE),
           efl_pack(box, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(600, 730));
}

#endif
