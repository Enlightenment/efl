#include "tools_private.h"

Eina_Bool _alloc_memory(Axis_Key **ckey, Panel_Struct **pobj, Axis_Key **lkey)
{
   *ckey = calloc(1, sizeof(Axis_Key));
   *pobj = calloc(1, sizeof(Panel_Struct));
   *lkey = calloc(1, sizeof(Axis_Key));

   if (!(*ckey) || !(*pobj) || !(*lkey))
     {
        printf("Not enough memory - at %s line %d\n", __FILE__, __LINE__);
        free(*ckey);
        free(*pobj);
        free(*lkey);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Evas_Object *
init_panel_camera_light(Evas_Object *win, Eo *camera_node, Eo *light_node, double posX, double posY)
{
   Evas_Object *nnavig = NULL, *bx_navigall = NULL;
   Evas_Object *bx_navigcamup = NULL, *bx_navigcamlow = NULL, *bx_navigligthup = NULL, *bx_navigligthlow = NULL;
   Evas_Object *spX = NULL, *spY = NULL, *spZ = NULL;
   Evas_Object *spangle = NULL, *spaX = NULL, *spaY = NULL, *spaZ = NULL;
   Evas_Object *splX = NULL, *splY = NULL, *splZ = NULL;
   Evas_Object *splangle = NULL, *splaX = NULL, *splaY = NULL, *splaZ = NULL;
   Evas_Object *separator = NULL;
   Axis_Key *camera_xyz = NULL, *light_xyz = NULL;
   Panel_Struct *pobject = NULL;
   Evas_Real px = 0, py = 0, pz = 0;

   if (ELM_WIN_UNKNOWN == elm_win_type_get(win))
     {
        printf("Not appropriate parent object - at %s line %d\n", __FILE__, __LINE__);
        return NULL;
     }

   if (!_alloc_memory(&camera_xyz, &pobject, &light_xyz)) return NULL;

   /*Navigation panel layout*/
   bx_navigall = elm_box_add(win);
   elm_box_homogeneous_set(bx_navigall, EINA_TRUE);
   evas_object_show(bx_navigall);

   bx_navigcamup = elm_box_add(win);
   elm_box_horizontal_set(bx_navigcamup, EINA_TRUE);
   elm_box_homogeneous_set(bx_navigcamup, EINA_TRUE);
   elm_box_pack_end(bx_navigall, bx_navigcamup);
   evas_object_show(bx_navigcamup);
   bx_navigcamlow = elm_box_add(win);
   elm_box_horizontal_set(bx_navigcamlow, EINA_TRUE);
   elm_box_homogeneous_set(bx_navigcamlow, EINA_TRUE);
   elm_box_pack_end(bx_navigall, bx_navigcamlow);
   evas_object_show(bx_navigcamlow);

   separator = elm_separator_add(win);
   elm_separator_horizontal_set(separator, EINA_TRUE);
   elm_box_pack_end(bx_navigall, separator);
   evas_object_show(separator);

   bx_navigligthup = elm_box_add(win);
   elm_box_horizontal_set(bx_navigligthup, EINA_TRUE);
   elm_box_homogeneous_set(bx_navigligthup, EINA_TRUE);
   elm_box_pack_end(bx_navigall, bx_navigligthup);
   evas_object_show(bx_navigligthup);
   bx_navigligthlow = elm_box_add(win);
   elm_box_horizontal_set(bx_navigligthlow, EINA_TRUE);
   elm_box_homogeneous_set(bx_navigligthlow, EINA_TRUE);
   elm_box_pack_end(bx_navigall, bx_navigligthlow);
   evas_object_show(bx_navigligthlow);

   /*Set UI panel for changing camera and light*/
   /*Init sp camera by default*/
   spX = elm_spinner_add(win);
   elm_spinner_editable_set(spX, EINA_TRUE);
   elm_spinner_min_max_set(spX, -COORDRANGE, COORDRANGE);
   elm_spinner_label_format_set(spX, "camera X: %1.0f");
   elm_spinner_step_set(spX, 1.0);
   pobject->cspX = spX;
   elm_box_pack_end(bx_navigcamup, spX);
   evas_object_show(spX);
   evas_object_smart_callback_add(spX, "changed", _camera_light_changeX_cb, camera_node);

   spY = elm_spinner_add(win);
   elm_spinner_editable_set(spY, EINA_TRUE);
   elm_spinner_min_max_set(spY, -COORDRANGE, COORDRANGE);
   elm_spinner_label_format_set(spY, "camera Y: %1.0f");
   elm_spinner_step_set(spY, 1.0);
   pobject->cspY = spY;
   elm_box_pack_end(bx_navigcamup, spY);
   evas_object_show(spY);
   evas_object_smart_callback_add(spY, "changed", _camera_light_changeY_cb, camera_node);

   spZ = elm_spinner_add(win);
   elm_spinner_editable_set(spZ, EINA_TRUE);
   elm_spinner_min_max_set(spZ, -COORDRANGE, COORDRANGE);
   elm_spinner_label_format_set(spZ, "camera Z: %1.0f");
   elm_spinner_step_set(spY, 1.0);
   pobject->cspZ = spZ;
   elm_box_pack_end(bx_navigcamup, spZ);
   evas_object_show(spZ);
   evas_object_smart_callback_add(spZ, "changed", _camera_light_changeZ_cb, camera_node);

   evas_canvas3d_node_position_get(camera_node, EVAS_CANVAS3D_SPACE_PARENT, &px, &py, &pz);
   elm_spinner_value_set(spX, px);
   elm_spinner_value_set(spY, py);
   elm_spinner_value_set(spZ, pz);

   spangle = elm_spinner_add(win);
   elm_spinner_editable_set(spangle, EINA_TRUE);
   elm_spinner_min_max_set(spangle, 0.0, ANGLERANGE);
   elm_spinner_label_format_set(spangle, "angle: %1.0f");
   elm_spinner_step_set(spangle, 1.0);
   pobject->cspangle = spangle;
   evas_object_data_set(spangle, axiskeys, camera_xyz);
   elm_box_pack_end(bx_navigcamlow, spangle);
   elm_spinner_value_set(spangle, 0.0);
   evas_object_show(spangle);
   evas_object_smart_callback_add(spangle, "changed", _camera_light_angle_change_cb, camera_node);

   spaX = elm_spinner_add(win);
   elm_spinner_min_max_set(spaX, 0.0, 1.0);
   elm_spinner_label_format_set(spaX, "axisX: %1.0f");
   elm_spinner_step_set(spaX, 1.0);
   elm_box_pack_end(bx_navigcamlow, spaX);
   evas_object_show(spaX);
   evas_object_smart_callback_add(spaX, "changed", _camera_light_axisX_change_cb, spangle);

   spaY = elm_spinner_add(win);
   elm_spinner_min_max_set(spaY, 0.0, 1.0);
   elm_spinner_label_format_set(spaY, "axisY: %1.0f");
   elm_spinner_step_set(spaY, 1.0);
   elm_box_pack_end(bx_navigcamlow, spaY);
   evas_object_show(spaY);
   evas_object_smart_callback_add(spaY, "changed", _camera_light_axisY_change_cb, spangle);

   spaZ = elm_spinner_add(win);
   elm_spinner_min_max_set(spaZ, 0.0, 1.0);
   elm_spinner_label_format_set(spaZ, "axisZ: %1.0f");
   elm_spinner_step_set(spaZ, 1.0);
   elm_box_pack_end(bx_navigcamlow, spaZ);
   evas_object_show(spaZ);
   evas_object_smart_callback_add(spaZ, "changed", _camera_light_axisZ_change_cb, spangle);

   /*Init spl light by default*/
   splX = elm_spinner_add(win);
   elm_spinner_editable_set(splX, EINA_TRUE);
   elm_spinner_min_max_set(splX, -COORDRANGE, COORDRANGE);
   elm_spinner_label_format_set(splX, "light X: %1.0f");
   elm_spinner_step_set(splX, 1.0);
   pobject->lspX = splX;
   elm_box_pack_end(bx_navigligthup, splX);
   evas_object_show(splX);
   evas_object_smart_callback_add(splX, "changed", _camera_light_changeX_cb, light_node);

   splY = elm_spinner_add(win);
   elm_spinner_editable_set(splY, EINA_TRUE);
   elm_spinner_min_max_set(splY, -COORDRANGE, COORDRANGE);
   elm_spinner_label_format_set(splY, "light Y: %1.0f");
   elm_spinner_step_set(splY, 1.0);
   pobject->lspY = splY;
   elm_box_pack_end(bx_navigligthup, splY);
   evas_object_show(splY);
   evas_object_smart_callback_add(splY, "changed", _camera_light_changeY_cb, light_node);

   splZ = elm_spinner_add(win);
   elm_spinner_editable_set(splZ, EINA_TRUE);
   elm_spinner_min_max_set(splZ, -COORDRANGE, COORDRANGE);
   elm_spinner_label_format_set(splZ, "light Z: %1.0f");
   elm_spinner_step_set(splY, 1.0);
   pobject->lspZ = splZ;
   elm_box_pack_end(bx_navigligthup, splZ);
   evas_object_show(splZ);
   evas_object_smart_callback_add(splZ, "changed", _camera_light_changeZ_cb, light_node);

   evas_canvas3d_node_position_get(light_node, EVAS_CANVAS3D_SPACE_PARENT, &px, &py, &pz);
   elm_spinner_value_set(splX, px);
   elm_spinner_value_set(splY, py);
   elm_spinner_value_set(splZ, pz);

   splangle = elm_spinner_add(win);
   elm_spinner_editable_set(splangle, EINA_TRUE);
   elm_spinner_min_max_set(splangle, 0.0, ANGLERANGE);
   elm_spinner_label_format_set(splangle, "angle: %1.0f");
   elm_spinner_step_set(splangle, 1.0);
   pobject->lspangle = splangle;
   evas_object_data_set(splangle, axiskeys, light_xyz);
   elm_box_pack_end(bx_navigligthlow, splangle);
   elm_spinner_value_set(splangle, 0.0);
   evas_object_show(splangle);
   evas_object_smart_callback_add(splangle, "changed", _camera_light_angle_change_cb, light_node);

   splaX = elm_spinner_add(win);
   elm_spinner_min_max_set(splaX, 0.0, 1.0);
   elm_spinner_label_format_set(splaX, "axisX: %1.0f");
   elm_spinner_step_set(splaX, 1.0);
   elm_box_pack_end(bx_navigligthlow, splaX);
   evas_object_show(splaX);
   evas_object_smart_callback_add(splaX, "changed", _camera_light_axisX_change_cb, splangle);

   splaY = elm_spinner_add(win);
   elm_spinner_min_max_set(splaY, 0.0, 1.0);
   elm_spinner_label_format_set(splaY, "axisY: %1.0f");
   elm_spinner_step_set(splaY, 1.0);
   elm_box_pack_end(bx_navigligthlow, splaY);
   evas_object_show(splaY);
   evas_object_smart_callback_add(splaY, "changed", _camera_light_axisY_change_cb, splangle);

   splaZ = elm_spinner_add(win);
   elm_spinner_min_max_set(splaZ, 0.0, 1.0);
   elm_spinner_label_format_set(splaZ, "axisZ: %1.0f");
   elm_spinner_step_set(splaZ, 1.0);
   elm_box_pack_end(bx_navigligthlow, splaZ);
   evas_object_show(splaZ);
   evas_object_smart_callback_add(spaZ, "changed", _camera_light_axisZ_change_cb, splangle);

   nnavig = elm_notify_add(win);
   elm_object_content_set(nnavig, bx_navigall);
   elm_notify_align_set(nnavig, posX, posY);
   evas_object_data_set(nnavig, pb, pobject);

   return nnavig;
}

static void
_camera_light_changeX_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Real x, y, z;

   if ((Eo*)data)
     {
        evas_canvas3d_node_position_get((Eo*)data, EVAS_CANVAS3D_SPACE_PARENT, NULL, &y, &z);
        x = elm_spinner_value_get(obj);
        evas_canvas3d_node_position_set((Eo*)data, x, y, z);
     }
}
static void
_camera_light_changeY_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Real x, y, z;

   if ((Eo*)data)
     {
        evas_canvas3d_node_position_get((Eo*)data, EVAS_CANVAS3D_SPACE_PARENT, &x, NULL, &z);
        y = elm_spinner_value_get(obj);
        evas_canvas3d_node_position_set((Eo*)data, x, y, z);
     }
}
static void
_camera_light_changeZ_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Real x, y, z;

   if ((Eo*)data)
     {
        evas_canvas3d_node_position_get((Eo*)data, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, NULL);
        z = elm_spinner_value_get(obj);
        evas_canvas3d_node_position_set((Eo*)data, x, y, z);
     }
}
static void
_camera_light_angle_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Axis_Key *key = evas_object_data_get(obj, axiskeys);
   Evas_Real aw = 0;
   if ((Eo*)data)
     {
       aw = elm_spinner_value_get(obj);
       aw = cos(aw * M_PI / 360.0);
       evas_canvas3d_node_orientation_set(((Eo*)data), key->x, key->y, key->z, aw);
     }
}
static void
_camera_light_axisX_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *fsa = (Evas_Object*)data;
   Axis_Key *key =  evas_object_data_get(fsa, axiskeys);
   if (key)
     key->x = elm_spinner_value_get(obj);
}
static void
_camera_light_axisY_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *fsa = (Evas_Object*)data;
   Axis_Key *key =  evas_object_data_get(fsa, axiskeys);
   if (key)
     key->y = elm_spinner_value_get(obj);
}
static void
_camera_light_axisZ_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *fsa = (Evas_Object*)data;
   Axis_Key *key =  evas_object_data_get(fsa, axiskeys);
   if (key)
     key->z = elm_spinner_value_get(obj);
}

Eina_Bool
panel_camera_coord_step_set(Evas_Object *navigation, int step)
{
   Panel_Struct *pobject = NULL;

   pobject = evas_object_data_get(navigation, pb);
   elm_spinner_step_set(pobject->cspX, step);
   elm_spinner_step_set(pobject->cspY, step);
   elm_spinner_step_set(pobject->cspZ, step);
   return EINA_TRUE;
}

Eina_Bool
panel_camera_angle_step_set(Evas_Object *navigation, int step)
{
   Panel_Struct *pobject = NULL;

   pobject = evas_object_data_get(navigation, pb);
   elm_spinner_step_set(pobject->cspangle, step);
   return EINA_TRUE;
}

Eina_Bool
panel_light_coord_step_set(Evas_Object *navigation, int step)
{
   Panel_Struct *pobject = NULL;

   pobject = evas_object_data_get(navigation, pb);
   elm_spinner_step_set(pobject->lspX, step);
   elm_spinner_step_set(pobject->lspY, step);
   elm_spinner_step_set(pobject->lspZ, step);
   return EINA_TRUE;
}

Eina_Bool
panel_light_angle_step_set(Evas_Object *navigation, int step)
{
   Panel_Struct *pobject = NULL;

   pobject = evas_object_data_get(navigation, pb);
   elm_spinner_step_set(pobject->lspangle, step);
   return EINA_TRUE;
}

void panel_camera_light_fini(Evas_Object *navigation)
{
   Panel_Struct *pobject = evas_object_data_get(navigation, pb);
   Axis_Key *ckey = evas_object_data_get(pobject->cspangle, axiskeys);
   Axis_Key *lkey = evas_object_data_get(pobject->lspangle, axiskeys);
   free(pobject);
   free(ckey);
   free(lkey);
}

