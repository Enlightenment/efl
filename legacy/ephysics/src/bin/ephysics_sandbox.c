#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include<stdio.h>
#include<string.h>
#include<Eina.h>
#include<Elementary.h>
#include<EPhysics.h>

#define SANDBOX_THEME "ephysics_sandbox"
#define WIDTH (1280)
#define HEIGHT (720)
#define DEPTH (100)
#define INITIAL_MASS (15)

static const char *materials[] =
{
   "Custom", "Concrete", "Iron", "Plastic", "Polystyrene", "Rubber", "Wood"
};

typedef struct _World_Data World_Data;
typedef struct _Body_Data Body_Data;

struct _World_Data {
   EPhysics_Body *top, *bottom, *left, *right;
   Evas_Object *win, *tb, *nf;
   EPhysics_World *world;
   Body_Data *bd1, *bd2;
};

struct _Body_Data {
   World_Data *wd;
   EPhysics_Body *body;
   Evas_Object *sl_hardness;
};

/*** Sandbox Callbacks ***/
static void
_body_del(void *data __UNUSED__, EPhysics_Body *body __UNUSED__,
          void *event_info)
{
   if (event_info) evas_object_del(event_info);
}

static void
_world_gravity_x_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   double gravity_y, gravity_z;

   ephysics_world_gravity_get(world, NULL, &gravity_y, &gravity_z);
   ephysics_world_gravity_set(world, elm_slider_value_get(obj), gravity_y,
                              gravity_z);
}

static void
_world_gravity_y_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   double gravity_x, gravity_z;

   ephysics_world_gravity_get(world, &gravity_x, NULL, &gravity_z);
   ephysics_world_gravity_set(world, gravity_x, elm_slider_value_get(obj),
                              gravity_z);
}

static void
_world_rate_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   ephysics_world_rate_set(world, elm_slider_value_get(obj));
}

static void
_world_max_sleeping_time_cb(void *data, Evas_Object *obj,
                            void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   ephysics_world_max_sleeping_time_set(world, elm_slider_value_get(obj));
}

static void
_world_friction_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   World_Data *wd = data;
   ephysics_body_friction_set(wd->top, elm_slider_value_get(obj));
   ephysics_body_friction_set(wd->bottom, elm_slider_value_get(obj));
   ephysics_body_friction_set(wd->left, elm_slider_value_get(obj));
   ephysics_body_friction_set(wd->right, elm_slider_value_get(obj));
}

static void
_world_restitution_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   World_Data *wd = data;
   ephysics_body_restitution_set(wd->top, elm_slider_value_get(obj));
   ephysics_body_restitution_set(wd->bottom, elm_slider_value_get(obj));
   ephysics_body_restitution_set(wd->left, elm_slider_value_get(obj));
   ephysics_body_restitution_set(wd->right, elm_slider_value_get(obj));
}

static void
_type_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   double mass, rotation, friction, restitution, lin_damping, ang_damping;
   double lin_sleeping, ang_sleeping;
   EPhysics_Body_Material material;
   Evas_Object *body_image;
   EPhysics_World *world;
   Body_Data *bd = data;
   EPhysics_Body *body = bd->body;

   mass = ephysics_body_mass_get(body);
   ephysics_body_rotation_get(body, 0, 0, &rotation);
   friction = ephysics_body_friction_get(body);
   restitution = ephysics_body_restitution_get(body);
   ephysics_body_damping_get(body, &lin_damping, &ang_damping);
   ephysics_body_sleeping_threshold_get(body, &lin_sleeping, &ang_sleeping);
   material = ephysics_body_material_get(body);

   world = ephysics_body_world_get(body);
   body_image = ephysics_body_evas_object_unset(body);

   ephysics_body_del(body);

   if (elm_radio_value_get(obj))
     {
        body = ephysics_body_soft_circle_add(world);
        ephysics_body_soft_body_hardness_set(
           body, elm_slider_value_get(bd->sl_hardness));
        elm_object_disabled_set(bd->sl_hardness, EINA_FALSE);
     }
   else
     {
        body = ephysics_body_circle_add(world);
        elm_object_disabled_set(bd->sl_hardness, EINA_TRUE);
     }

   ephysics_body_evas_object_set(body, body_image, EINA_TRUE);
   ephysics_body_mass_set(body, mass);
   ephysics_body_rotation_set(body, 0, 0, rotation);
   ephysics_body_friction_set(body, friction);
   ephysics_body_restitution_set(body, restitution);
   ephysics_body_damping_set(body, lin_damping, ang_damping);
   ephysics_body_sleeping_threshold_set(body, lin_sleeping, ang_sleeping);
   ephysics_body_material_set(body, material);

   bd->body = body;
}

static void
_material_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = evas_object_data_get(obj, "bd");
   Evas_Object *win = bd->wd->win;
   EPhysics_Body *body = bd->body;
   EPhysics_Body_Material material = (EPhysics_Body_Material) data;
   Evas_Object *body_image;
   int x, y, w, h;

   elm_object_text_set(obj, materials[material]);
   ephysics_body_material_set(body, material);

   elm_slider_value_set(evas_object_data_get(obj, "mass"),
                         ephysics_body_mass_get(body));
   elm_slider_value_set(evas_object_data_get(obj, "density"),
                         ephysics_body_density_get(body));
   elm_slider_value_set(evas_object_data_get(obj, "friction"),
                         ephysics_body_friction_get(body));
   elm_slider_value_set(evas_object_data_get(obj, "restitution"),
                         ephysics_body_restitution_get(body));

   body_image = ephysics_body_evas_object_unset(body);
   evas_object_geometry_get(body_image, &x, &y, &w, &h);
   evas_object_del(body_image);

   body_image = elm_image_add(win);
   elm_image_file_set(
      body_image, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj",
      materials[material]);
   evas_object_resize(body_image, w, h);
   evas_object_move(body_image, x, y);
   evas_object_show(body_image);
   ephysics_body_evas_object_set(body, body_image, EINA_FALSE);
}

static void
_mass_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_mass_set(bd->body, elm_slider_value_get(obj));
   elm_slider_value_set(evas_object_data_get(obj, "density"), 0);
}

static void
_density_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_density_set(bd->body, elm_slider_value_get(obj));
   elm_slider_value_set(evas_object_data_get(obj, "mass"),
                         ephysics_body_mass_get(bd->body));
}

static void
_rotation_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_rotation_set(bd->body, 0, 0, elm_slider_value_get(obj));
}

static void
_friction_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_friction_set(bd->body, elm_slider_value_get(obj));
}

static void
_restitution_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_restitution_set(bd->body, elm_slider_value_get(obj));
}

static void
_linear_damping_set_cb(void *data, Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   double ang_damping;
   ephysics_body_damping_get(bd->body, NULL, &ang_damping);
   ephysics_body_damping_set(bd->body, elm_slider_value_get(obj), ang_damping);
}

static void
_angular_damping_set_cb(void *data, Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   double lin_damping;
   ephysics_body_damping_get(bd->body, &lin_damping, NULL);
   ephysics_body_damping_set(bd->body, lin_damping, elm_slider_value_get(obj));
}

static void
_lin_sleeping_threshold_set_cb(void *data, Evas_Object *obj,
                               void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   double ang_sleeping;
   ephysics_body_sleeping_threshold_get(bd->body, NULL, &ang_sleeping);
   ephysics_body_sleeping_threshold_set(bd->body, elm_slider_value_get(obj),
                                        ang_sleeping);
}

static void
_ang_sleeping_threshold_set_cb(void *data, Evas_Object *obj,
                               void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   double lin_sleeping;
   ephysics_body_sleeping_threshold_get(bd->body, &lin_sleeping, NULL);
   ephysics_body_sleeping_threshold_set(bd->body, lin_sleeping,
                                        elm_slider_value_get(obj));
}

static void
_torque_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_torque_apply(bd->body, 0, 0, elm_slider_value_get(obj));
}

static void
_impulse_x_x_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord rel_x, rel_y;
   Body_Data *bd = data;
   double y;

   y = elm_slider_value_get(evas_object_data_get(obj, "y"));
   rel_x = elm_slider_value_get(evas_object_data_get(obj, "relx"));
   rel_y = elm_slider_value_get(evas_object_data_get(obj, "rely"));
   ephysics_body_impulse_apply(bd->body, elm_slider_value_get(obj), y, 0,
                               rel_x, rel_y, 0);
}

static void
_impulse_x_rel_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   Evas_Object *aux;
   Evas_Coord rel_y;
   double x, y;

   aux = evas_object_data_get(obj, "x");
   x = elm_slider_value_get(aux);
   y = elm_slider_value_get(evas_object_data_get(aux, "y"));
   rel_y = elm_slider_value_get(evas_object_data_get(aux, "rely"));
   ephysics_body_impulse_apply(bd->body, x, y, 0,
                               elm_slider_value_get(obj), rel_y, 0);
}

static void
_impulse_y_y_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   Evas_Object *aux;
   Evas_Coord rel_x, rel_y;
   double x;

   aux = evas_object_data_get(obj, "x");
   x = elm_slider_value_get(aux);
   rel_x = elm_slider_value_get(evas_object_data_get(aux, "relx"));
   rel_y = elm_slider_value_get(evas_object_data_get(aux, "rely"));
   ephysics_body_impulse_apply(bd->body, x, elm_slider_value_get(obj), 0,
                               rel_x, rel_y, 0);
}

static void
_impulse_y_rel_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   Evas_Object *aux;
   Evas_Coord rel_x;
   double x, y;

   aux = evas_object_data_get(obj, "x");
   x = elm_slider_value_get(aux);
   y = elm_slider_value_get(evas_object_data_get(aux, "y"));
   rel_x = elm_slider_value_get(evas_object_data_get(aux, "relx"));
   ephysics_body_impulse_apply(bd->body, x, y, 0,
                               rel_x, 0, elm_slider_value_get(obj));
}

static void
_force_x_x_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord rel_x, rel_y;
   Body_Data *bd = data;
   double y;

   y = elm_slider_value_get(evas_object_data_get(obj, "y"));
   rel_x = elm_slider_value_get(evas_object_data_get(obj, "relx"));
   rel_y = elm_slider_value_get(evas_object_data_get(obj, "rely"));
   ephysics_body_force_apply(bd->body, elm_slider_value_get(obj), y, 0,
                             rel_x, rel_y, 0);
}

static void
_force_x_rel_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   Evas_Object *aux;
   Evas_Coord rel_y;
   double x, y;

   aux = evas_object_data_get(obj, "x");
   x = elm_slider_value_get(aux);
   y = elm_slider_value_get(evas_object_data_get(aux, "y"));
   rel_y = elm_slider_value_get(evas_object_data_get(aux, "rely"));
   ephysics_body_force_apply(bd->body, x, y, 0,
                             elm_slider_value_get(obj), rel_y, 0);
}

static void
_force_y_y_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord rel_x, rel_y;
   Body_Data *bd = data;
   Evas_Object *aux;
   double x;

   aux = evas_object_data_get(obj, "x");
   x = elm_slider_value_get(aux);
   rel_x = elm_slider_value_get(evas_object_data_get(aux, "relx"));
   rel_y = elm_slider_value_get(evas_object_data_get(aux, "rely"));
   ephysics_body_force_apply(bd->body, x, elm_slider_value_get(obj), 0,
                             rel_x, rel_y, 0);
}

static void
_force_y_rel_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   Evas_Object *aux;
   Evas_Coord rel_x;
   double x, y;

   aux = evas_object_data_get(obj, "x");
   x = elm_slider_value_get(aux);
   y = elm_slider_value_get(evas_object_data_get(aux, "y"));
   rel_x = elm_slider_value_get(evas_object_data_get(aux, "relx"));
   ephysics_body_force_apply(bd->body, x, y, 0,
                             rel_x, elm_slider_value_get(obj), 0);
}

static void
_linear_velocity_x_set_cb(void *data, Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   double ylinear;
   ylinear = elm_slider_value_get(evas_object_data_get(obj, "y"));
   ephysics_body_linear_velocity_set(bd->body, elm_slider_value_get(obj),
                                     ylinear, 0);
}

static void
_linear_velocity_y_set_cb(void *data, Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   double xlinear;
   xlinear = elm_slider_value_get(evas_object_data_get(obj, "x"));
   ephysics_body_linear_velocity_set(bd->body, xlinear,
                                     elm_slider_value_get(obj), 0);
}

static void
_angular_velocity_set_cb(void *data, Evas_Object *obj,
                         void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_angular_velocity_set(bd->body, 0, 0,
                                      elm_slider_value_get(obj));
}

static void
_hardness_set_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Body_Data *bd = data;
   ephysics_body_soft_body_hardness_set(bd->body, elm_slider_value_get(obj));
}

static void
_simulate_body(Body_Data *bd, Evas_Coord x, Evas_Coord y)
{
   ephysics_body_stop(bd->body);
   ephysics_body_forces_clear(bd->body);
   ephysics_body_geometry_set(bd->body, x, y, -15, 70, 70, 30);

   /*FIXME TODO apply impulses, velocities, forces ... */
}

static void
_simulate(World_Data *wd)
{
   _simulate_body(wd->bd1, (int) WIDTH / 5, (int) HEIGHT / 5);
   _simulate_body(wd->bd2, (int) WIDTH / 2, (int) HEIGHT / 5);
}

static void
_simulate_cb(void *data, Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   _simulate(data);
}

static void
_promote(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_naviframe_item_promote(data);
}

/*** Sandbox Widgets ***/
static void
_label_add(Evas_Object *bxparent, const char *subcategory)
{
   Evas_Object *label;

   label = elm_label_add(bxparent);
   elm_object_text_set(label, subcategory);
   evas_object_size_hint_weight_set(label, 0.0, 0.0);
   evas_object_size_hint_align_set(label, 0.5, 0.5);
   elm_box_pack_end(bxparent, label);
   evas_object_show(label);
}

static Evas_Object *
_material_selector_add(Body_Data *bd, Evas_Object *bxparent)
{
   Evas_Object *box, *hv;

   box = elm_box_add(bxparent);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
   evas_object_show(box);
   elm_box_pack_end(bxparent, box);

   _label_add(box, "Body Material");

   hv = elm_hoversel_add(bxparent);
   elm_hoversel_hover_parent_set(hv, bd->wd->nf);
   elm_object_text_set(hv, materials[EPHYSICS_BODY_MATERIAL_CUSTOM]);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_CUSTOM],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_CUSTOM);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_CONCRETE],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_CONCRETE);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_IRON],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_IRON);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_PLASTIC],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_PLASTIC);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_POLYSTYRENE],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_POLYSTYRENE);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_RUBBER],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_RUBBER);
   elm_hoversel_item_add(hv, materials[EPHYSICS_BODY_MATERIAL_WOOD],
                         NULL, ELM_ICON_NONE, _material_set_cb,
                         (void *) EPHYSICS_BODY_MATERIAL_WOOD);

   evas_object_size_hint_align_set(hv, 1, 0.5);
   elm_box_pack_end(box, hv);
   evas_object_show(hv);
   evas_object_data_set(hv, "bd", bd);

   return hv;
}

static void
_type_radio_add(Body_Data *bd, Evas_Object *bxparent)
{
   Evas_Object *dbx, *rd, *rdg;

   dbx = elm_box_add(bxparent);
   elm_box_horizontal_set(dbx, EINA_TRUE);
   evas_object_size_hint_weight_set(dbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(dbx, EVAS_HINT_FILL, 0.0);
   elm_box_padding_set(dbx, 46, 0);
   elm_box_align_set(dbx, 1, 0.5);
   elm_box_pack_end(bxparent, dbx);
   evas_object_show(dbx);

   _label_add(dbx, "Body Type");

   rd = elm_radio_add(bxparent);
   elm_radio_state_value_set(rd, 0);
   elm_object_text_set(rd, "Solid");
   evas_object_size_hint_align_set(rd, 1, 0.5);
   evas_object_size_hint_weight_set(rd, 0, EVAS_HINT_EXPAND);
   elm_box_pack_end(dbx, rd);
   evas_object_show(rd);
   rdg = rd;
   evas_object_smart_callback_add(rd, "changed", _type_set_cb, bd);

   rd = elm_radio_add(bxparent);
   elm_radio_state_value_set(rd, 1);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Soft");
   evas_object_size_hint_align_set(rd, 1, 0.5);
   evas_object_size_hint_weight_set(rd, 0, EVAS_HINT_EXPAND);
   elm_box_pack_end(dbx, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _type_set_cb, bd);
}

static Evas_Object *
_slider_add(Evas_Object *bxparent, const char *subcategory, const char *itemlb,
            float min, float max, float initial)
{
   Evas_Object *sl;

   sl = elm_slider_add(bxparent);
   elm_object_text_set(sl, subcategory);
   elm_slider_unit_format_set(sl, itemlb);
   elm_slider_min_max_set(sl, min, max);
   elm_slider_value_set(sl, initial);
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_weight_set(sl, 0.0, 0.0);
   evas_object_size_hint_align_set(sl, 1, 0.5);
   elm_box_pack_end(bxparent, sl);
   evas_object_show(sl);

   return sl;
}

static Evas_Object *
_category_add(Evas_Object *bxparent, const char *label, Eina_Bool collapse)
{
   Evas_Object *cfr, *cbx;

   cfr = elm_frame_add(bxparent);
   elm_frame_autocollapse_set(cfr, collapse);
   elm_frame_collapse_set(cfr, collapse);
   elm_object_text_set(cfr, label);
   evas_object_size_hint_weight_set(cfr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(cfr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxparent, cfr);
   evas_object_show(cfr);

   cbx = elm_box_add(bxparent);
   elm_box_horizontal_set(cbx, EINA_FALSE);
   evas_object_size_hint_weight_set(cbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(cbx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(cfr, cbx);
   evas_object_show(cbx);

   return cbx;
}

/*** Sandbox Pages ***/
static void
_menu_page_add(Evas_Object *win, Evas_Object **scroller_box,
               Evas_Object **content_box)
{
   Evas_Object *scbx, *bx, *sc;

   scbx = elm_box_add(win);
   evas_object_size_hint_weight_set(scbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(scbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(scbx);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF,
                           ELM_SCROLLER_POLICY_AUTO);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(scbx, sc);
   evas_object_show(sc);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0);
   evas_object_show(bx);
   elm_object_content_set(sc, bx);

   *scroller_box = scbx;
   *content_box = bx;
}

static void
_menu_body_page_add(World_Data *wd, Body_Data *bd, const char *pg_label)
{
   Evas_Object *bx, *bxbody, *scbxbody, *widget, *aux_widget, *material_widget;
   Elm_Object_Item *it;

   _menu_page_add(wd->win, &scbxbody, &bxbody);
   bx = _category_add(bxbody, "", EINA_FALSE);
   _type_radio_add(bd, bx);
   material_widget = _material_selector_add(bd, bx);

   bx = _category_add(bxbody, "Properties", EINA_TRUE);
   aux_widget = _slider_add(bx, "Mass (kg)", "%1.3f", 0, 99999, INITIAL_MASS);
   evas_object_data_set(material_widget, "mass", aux_widget);
   widget = _slider_add(bx, "Density (kg/m³)", "%1.3f", 0, 9999, 0);
   evas_object_data_set(aux_widget, "density", widget);
   evas_object_data_set(material_widget, "density", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed", _mass_set_cb,
                                  bd);
   evas_object_data_set(widget, "mass", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed", _density_set_cb,
                                  bd);
   widget = _slider_add(bx, "Rotation (º)", "%1.0f", 0, 360, 0);
   evas_object_smart_callback_add(widget, "delay,changed", _rotation_set_cb,
                                  bd);
   widget = _slider_add(bx, "Friction", "%1.3f", 0, 1, 0.5);
   evas_object_data_set(material_widget, "friction", widget);
   evas_object_smart_callback_add(widget, "delay,changed", _friction_set_cb,
                                  bd);
   widget = _slider_add(bx, "Restitution", "%1.3f", 0, 1, 0);
   evas_object_data_set(material_widget, "restitution", widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _restitution_set_cb, bd);
   _label_add(bx, "Damping");
   widget = _slider_add(bx, "Linear", "%1.3f", 0, 1, 0);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _linear_damping_set_cb, bd);
   widget = _slider_add(bx, "Angular", "%1.3f", 0, 1, 0);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _angular_damping_set_cb, bd);
   _label_add(bx, "Sleeping Threshold");
   widget = _slider_add(bx, "Linear (p/s)", "%1.2f",
                               0, 250, 24);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _lin_sleeping_threshold_set_cb, bd);
   widget = _slider_add(bx, "Angular (º/s)", "%1.2f",
                               0, 360, 57.29);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _ang_sleeping_threshold_set_cb, bd);

   bx = _category_add(bxbody, "Actions", EINA_TRUE);
   _label_add(bx, "Impulse X");
   aux_widget = _slider_add(bx, "X (kg * p/s)", "%1.3f",
                                   -9999, 9999, 0);
   widget = _slider_add(bx, "Rel Position X", "%1.2f",
                               -360, 360, 0);
   evas_object_data_set(aux_widget, "relx", widget);
   _label_add(bx, "Impulse Y");
   widget = _slider_add(bx, "Y (kg * p/s)", "%1.3f", -9999, 9999, 0);
   evas_object_data_set(aux_widget, "y", widget);
   widget = _slider_add(bx, "Rel Position Y", "%1.2f", -360, 360, 0);
   evas_object_data_set(aux_widget, "rely", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed",
                                  _impulse_x_x_set_cb, bd);
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _impulse_x_rel_set_cb, bd);
   widget = evas_object_data_get(aux_widget, "y");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _impulse_y_y_set_cb, bd);
   widget = evas_object_data_get(aux_widget, "relx");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _impulse_y_rel_set_cb, bd);

   _label_add(bx, "Force X");
   aux_widget = _slider_add(bx, "X (kg * p/s/s)", "%1.3f", -1999, 1999, 0);
   widget = _slider_add(bx, "Rel Position X", "%1.2f", -360, 360, 0);
   evas_object_data_set(aux_widget, "relx", widget);
   _label_add(bx, "Force Y");
   widget = _slider_add(bx, "Y (kg * p/s/s)", "%1.3f", -1999, 1999, 0);
   evas_object_data_set(aux_widget, "y", widget);
   widget = _slider_add(bx, "Rel Position Y", "%1.2f", -360, 360, 0);
   evas_object_data_set(aux_widget, "rely", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed",
                                  _force_x_x_set_cb, bd);
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _force_x_rel_set_cb, bd);
   widget = evas_object_data_get(aux_widget, "y");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _force_y_y_set_cb, bd);
   widget = evas_object_data_get(aux_widget, "relx");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _force_y_rel_set_cb, bd);
   widget = _slider_add(bx, "Torque", "%1.3f", -100, 100, 0);
   evas_object_smart_callback_add(widget, "delay,changed", _torque_set_cb, bd);

   _label_add(bx, "Linear Velocity");
   aux_widget = _slider_add(bx, "X (p/s)", "%1.2f", -1499, 1499, 0);
   widget = _slider_add(bx, "Y (p/s)", "%1.2f", -1499, 1499, 0);
   evas_object_data_set(aux_widget, "y", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed",
                                  _linear_velocity_x_set_cb, bd);
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _linear_velocity_y_set_cb, bd);

   widget = _slider_add(bx, "Angular Velocity (º/s)", "%1.2f", -360, 360, 0);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _angular_velocity_set_cb, bd);

   bx = _category_add(bxbody, "Soft Body", EINA_TRUE);
   widget = _slider_add(bx, "Hardness (%)", "%1.2f%", 0, 100, 100);
   elm_object_disabled_set(widget, EINA_TRUE);
   evas_object_smart_callback_add(widget, "delay,changed", _hardness_set_cb,
                                  bd);
   bd->sl_hardness = widget;

   it = elm_naviframe_item_insert_before(wd->nf,
                                         evas_object_data_get(wd->nf, "world"),
                                         NULL, NULL, NULL, scbxbody, NULL);
   elm_naviframe_item_title_visible_set(it, EINA_FALSE);
   it = elm_toolbar_item_append(wd->tb, NULL, pg_label, _promote, it);
}

static void
_menu_world_page_add(World_Data *wd)
{
   Evas_Object *bx, *scbx, *bxparent, *widget;
   Elm_Object_Item *it;

   _menu_page_add(wd->win, &scbx, &bxparent);

   bx = _category_add(bxparent, "World", EINA_FALSE);
   _label_add(bx, "Gravity (px/s²)");
   widget = _slider_add(bx, "X:", "%1.2f", -1000, 1000, 0);
   evas_object_smart_callback_add(widget, "delay,changed", _world_gravity_x_cb,
                                  wd->world);
   widget = _slider_add(bx, "Y:", "%1.2f", -1000, 1000, 294);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_gravity_y_cb, wd->world);
   widget = _slider_add(bx, "Rate (px/m)", "%1.0f", 1, 100, 30);
   evas_object_smart_callback_add(widget, "delay,changed", _world_rate_cb,
                                  wd->world);
   widget = _slider_add(bx, "Max Sleeping Time (s)", "%1.0f", 0, 50, 2);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_max_sleeping_time_cb, wd->world);

   bx = _category_add(bxparent, "Boundaries", EINA_FALSE);
   widget = _slider_add(bx, "Friction", "%1.3f", 0, 1, 0.5);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_friction_cb, wd);
   widget = _slider_add(bx, "Restitution", "%1.3f", 0, 1, 0);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_restitution_cb, wd);

   it = elm_naviframe_item_push(wd->nf, NULL, NULL, NULL, scbx, NULL);
   evas_object_data_set(wd->nf, "world", it);
   elm_naviframe_item_title_visible_set(it, EINA_FALSE);
   it = elm_toolbar_item_append(wd->tb, NULL, "World", _promote, it);
}

static void
_menu_create(World_Data *wd)
{
   Evas_Object *mainbx, *btn, *bg, *layout;

   bg = elm_bg_add(wd->win);
   elm_win_resize_object_add(wd->win, bg);
   evas_object_size_hint_min_set(bg, WIDTH, HEIGHT);
   evas_object_size_hint_max_set(bg, WIDTH, HEIGHT);

   layout = elm_layout_add(wd->win);
   elm_layout_file_set(layout, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj",
                       "sandbox");
   evas_object_resize(layout, WIDTH, HEIGHT);
   evas_object_show(layout);

   mainbx = elm_box_add(wd->win);
   evas_object_size_hint_fill_set(mainbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_content_set(layout, "swallow", mainbx);
   evas_object_show(mainbx);

   btn = elm_button_add(wd->win);
   elm_object_text_set(btn, "Restart");
   elm_box_pack_end(mainbx, btn);
   evas_object_size_hint_min_set(btn, 100, 30);
   evas_object_size_hint_align_set(btn, 1, 0.5);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _simulate_cb, wd);

   wd->tb = elm_toolbar_add(wd->win);
   evas_object_size_hint_weight_set(wd->tb, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(wd->tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_toolbar_select_mode_set(wd->tb, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_box_pack_end(mainbx, wd->tb);
   evas_object_show(wd->tb);

   wd->nf = elm_naviframe_add(wd->win);
   evas_object_size_hint_weight_set(wd->nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(wd->nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(mainbx, wd->nf);
   evas_object_show(wd->nf);
}

static EPhysics_Body *
_body_add(Evas_Object *win, EPhysics_World *world, int x, int y)
{
   Evas_Object *body_image;
   EPhysics_Body *body;

   body_image = elm_image_add(win);
   elm_image_file_set(
      body_image, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj", "Custom");
   evas_object_resize(body_image, 70, 70);
   evas_object_move(body_image, x, y);
   evas_object_show(body_image);

   body = ephysics_body_circle_add(world);
   ephysics_body_evas_object_set(body, body_image, EINA_TRUE);
   ephysics_body_mass_set(body, INITIAL_MASS);
   ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_DEL,
                                    _body_del, NULL);

   return body;
}

EAPI int
elm_main()
{
   EPhysics_World *world;
   Body_Data bd1, bd2;
   Evas_Object *win;
   World_Data wd;

   if (!ephysics_init())
     {
        fprintf(stderr, "Failed to init ephysics\n");
        elm_shutdown();
        return -1;
     }

   elm_theme_extension_add(NULL, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "Sandbox", ELM_WIN_BASIC);
   elm_win_title_set(win, "EPhysics Sandbox");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_show(win);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 80, 80, -50,
                                      (int) WIDTH * 0.7 - 160, HEIGHT - 160,
                                      DEPTH);
   wd.top = ephysics_body_top_boundary_add(world);
   wd.bottom = ephysics_body_bottom_boundary_add(world);
   wd.left = ephysics_body_left_boundary_add(world);
   wd.right = ephysics_body_right_boundary_add(world);

   wd.world = world;
   wd.win = win;
   wd.bd1 = &bd1;
   wd.bd2 = &bd2;
   bd1.wd = &wd;
   bd2.wd = &wd;

   _menu_create(&wd);
   _menu_world_page_add(&wd);
   _menu_body_page_add(&wd, &bd1, "Body 1");
   _menu_body_page_add(&wd, &bd2, "Body 2");

   bd1.body = _body_add(win, world, WIDTH / 5, HEIGHT / 5);
   bd2.body = _body_add(win, world, WIDTH / 2, HEIGHT / 5);

   _simulate(&wd);

   elm_run();

   ephysics_shutdown();
   elm_shutdown();
   return 0;
}
ELM_MAIN()
