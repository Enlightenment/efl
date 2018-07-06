/*
    Concerning the EDC reference:

    The formatting for blocks and properties has been implemented as a table
    which is filled using ALIASES.
    For maximum flexibility I implemented them in the \@code/\@encode style,
    this means that missing one or changing the order most certainly cause
    formatting errors.

   \@block
        block name
   \@context
        code sample of the block
   \@description
        the block's description
   \@since X.X
   \@endblock

   \@property
        property name
   \@parameters
        property's parameter list
   \@effect
        the property description (lol)
   \@since X.X
   \@endproperty
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ctype.h>

#include "edje_cc.h"

/**
 * @page edcref Edje Data Collection reference
 *
 * An Edje Data Collection, it's a plain text file (normally identified with the
 * .edc extension), consisting of instructions for the Edje Compiler.
 *
 * The syntax for the edje data collection files follows a simple structure of
 * "blocks { .. }" that can contain "properties: ..", more blocks, or both.
 *
 * @anchor sec_quickaccess Quick access to block descriptions:
 * <ul>
 *    <li>@ref sec_toplevel "Top-Level"</li>
 *    <ul>
 *      <li>@ref sec_toplevel_externals "Externals"</li>
 *      <li>@ref sec_toplevel_images "Images"</li>
 *      <ul>
 *        <li>@ref sec_toplevel_images_set "Set"</li>
 *        <ul>
 *          <li>@ref sec_toplevel_images_set_image "Image"</li>
 *        </ul>
 *      </ul>
 *      <li>@ref sec_toplevel_fonts "Fonts"</li>
 *      <li>@ref sec_toplevel_data "Data"</li>
 *      <li>@ref sec_toplevel_color_classes "Color Classes"</li>
 *      <li>@ref sec_toplevel_styles "Styles"</li>
 *      <li>@ref sec_collections_group_filters "Filters"</li>
 *    </ul>
 *    <li>@ref sec_collections "Collections"</li>
 *    <ul>
 *      <li>@ref sec_collections_sounds "Sounds"</li>
 *      <ul>
 *        <li>@ref sec_collections_sounds_sample "Sample"</li>
 *      </ul>
 *      <li>@ref sec_collections_group_filters "Filters"</li>
 *      <li>@ref sec_collections_vibrations "Vibrations"</li>
 *      <ul>
 *        <li>@ref sec_collections_vibrations_sample "Sample"</li>
 *      </ul>
 *      <li>@ref sec_collections_group "Group"</li>
 *      <ul>
 *        <li>@ref sec_collections_group_script "Script"</li>
 *        <li>@ref sec_collections_group_limits "Limits"</li>
 *        <li>@ref sec_collections_group_data "Data"</li>
 *        <li>@ref sec_collections_group_filters "Filters"</li>
 *        <li>@ref sec_collections_group_parts "Parts"</li>
 *        <ul>
 *          <li>@ref sec_collections_group_parts_part "Part"</li>
 *          <ul>
 *            <li>@ref sec_collections_group_parts_dragable "Dragable"</li>
 *            <li>@ref sec_collections_group_parts_items "Items"</li>
 *            <li>@ref sec_collections_group_parts_description "Description"</li>
 *            <ul>
 *              <li>@ref sec_collections_group_parts_description_relatives "Relatives (rel1/rel2)"</li>
 *              <li>@ref sec_collections_group_parts_description_image "Image"</li>
 *              <li>@ref sec_collections_group_parts_description_proxy "Proxy"</li>
 *              <li>@ref sec_collections_group_parts_description_fill "Fill"</li>
 *              <ul>
 *                <li>@ref sec_collections_group_parts_description_fill_origin "Origin"</li>
 *                <li>@ref sec_collections_group_parts_description_fill_size "Size"</li>
 *              </ul>
 *              <li>@ref sec_collections_group_parts_description_text "Text"</li>
 *              <li>@ref sec_collections_group_parts_description_box "Box"</li>
 *              <ul>
 *                <li>@ref sec_collections_group_parts_items "Items"</li>
 *              </ul>
 *              <li>@ref sec_collections_group_parts_description_table "Table"</li>
 *              <li>@ref sec_collections_group_parts_description_physics "Physics"</li>
 *              <ul>
 *                  <li>@ref sec_collections_group_parts_description_physics_movement_freedom "Movement Freedom"</li>
 *                  <li>@ref sec_collections_group_parts_description_physics_faces "Faces"</li>
 *              </ul>
 *              <li>@ref sec_collections_group_parts_description_map "Map (3d/transformations)"</li>
 *              <ul>
 *                <li>@ref sec_collections_group_parts_description_map_rotation "Rotation"</li>
 *              </ul>
 *              <li>@ref sec_collections_group_parts_description_perspective "Perspective"</li>
 *              <li>@ref sec_collections_group_parts_descriptions_params "Params"</li>
 *              <li>@ref sec_collections_group_parts_description_links "Links"</li>
 *              <li>@ref sec_collections_group_parts_description_filter "Filter"</li>
 *            </ul>
 *          </ul>
 *        </ul>
 *        <li>@ref sec_collections_group_programs "Programs"</li>
 *        <ul>
 *          <li>@ref sec_collections_group_script "Script"</li>
 *          <li>@ref sec_collections_group_programs_program "Program"</li>
 *          <ul>
 *            <li>@ref sec_collections_group_script "Script"</li>
 *            <li>@ref sec_collections_group_program_sequence "Sequence"</li>
 *            <ul>
 *              <li>@ref sec_collections_group_script "Script"</li>
 *            </ul>
 *          </ul>
 *          <li>@ref sec_collections_group_script "Script"</li>
 *          <li>@ref sec_toplevel_fonts "Fonts"</li>
 *        </ul>
 *        <li>@ref sec_collections_group_physics "Physics"</li>
 *        <ul>
 *          <li>@ref sec_collections_group_physics_world "World"</li>
 *        </ul>
 *      </ul>
 *    </ul>
 *    <li>@ref sec_lazedc "LazEDC"</li>
 *    <ul>
 *       <li>@ref sec_lazedc_intro "Intro"</li>
 *       <li>@ref sec_lazedc_synonyms "Synonyms"</li>
 *       <li>@ref sec_lazedc_shorthand "Shorthand"</li>
 *    </ul>
 * </ul>
 *
 * @author Andres Blanc (dresb) andresblanc@gmail.com
 *
 * <table class="edcref" border="0">
 */

static Edje_Part_Collection_Directory_Entry *current_de = NULL;
static Edje_Part *current_part = NULL;
static Edje_Pack_Element *current_item = NULL;
static Edje_Part_Description_Common *current_desc = NULL;
static Edje_Part_Description_Common *parent_desc = NULL;
static Edje_Program *current_program = NULL;
static Eina_List *current_program_lookups = NULL;
Eina_Bool current_group_inherit = EINA_FALSE;
Eina_Bool script_is_replaceable = EINA_FALSE;
static Edje_Program *sequencing = NULL;
static Eina_List *sequencing_lookups = NULL;
static int *anonymous_delete = NULL;
static Edje_Part_Description_Anchors *current_anchors = NULL;
static Eina_Bool has_relatives = EINA_FALSE;

Eina_List *po_files;

static Eina_Hash *desc_hash = NULL;

struct _Edje_Cc_Handlers_Hierarchy_Info /* Struct that keeps globals value to impl hierarchy */
{
   Edje_Part_Collection_Directory_Entry *current_de;
   Edje_Part                            *current_part;
   Edje_Pack_Element                    *current_item;
   Edje_Part_Description_Common         *current_desc;
   Edje_Part_Description_Common         *parent_desc;
   Edje_Program                         *current_program;
   Edje_Part                            *ep;
};
typedef struct _Edje_Cc_Handlers_Hierarchy_Info Edje_Cc_Handlers_Hierarchy_Info;

static Eina_Array *part_hierarchy = NULL; /* stack parts,support nested parts */
static void       edje_cc_handlers_hierarchy_set(Edje_Part *src);
static Edje_Part *edje_cc_handlers_hierarchy_parent_get(void);
static void       edje_cc_handlers_hierarchy_push(Edje_Part *ep, Edje_Part *cp);
static void       edje_cc_handlers_hierarchy_rename(Edje_Part *old, Edje_Part *new);
static void       edje_cc_handlers_hierarchy_pop(void);

static void       _program_target_add(char *name);
static void       _program_after(const char *name);
static void       _program_free(Edje_Program *pr);
static Eina_Bool  _program_remove(const char *name, Edje_Program **pgrms, unsigned int count);

static void      *_part_free(Edje_Part_Collection *pc, Edje_Part *ep);

static void       check_has_anchors(void);

static void       st_id(void);
static void       st_requires(void);
static void       st_efl_version(void);
static void       st_externals_external(void);

static void       st_images_image(void);
static void       ob_images_set(void);
static void       st_images_set_name(void);
static void       ob_images_set_image(void);
static void       st_images_set_image_image(void);
static void       st_images_set_image_size(void);
static void       st_images_set_image_border(void);
static void       st_images_set_image_border_scale_by(void);

static void       st_models_model(void);

static void       st_fonts_font(void);

static void       st_data_item(void);
static void       st_data_file(void);

static void       ob_styles_style(void);
static void       st_styles_style_name(void);
static void       st_styles_style_base(void);
static void       st_styles_style_tag(void);

static void       ob_color_tree(void);

static void       ob_color_class(void);
static void       st_color_class_name(void);
static void       st_color_class_color(void);
static void       st_color_class_color2(void);
static void       st_color_class_color3(void);
static void       st_color_class_desc(void);

static void       ob_text_class(void);
static void       st_text_class_name(void);
static void       st_text_class_font(void);
static void       st_text_class_size(void);

static void       ob_size_class(void);
static void       st_size_class_name(void);
static void       st_size_class_min(void);
static void       st_size_class_max(void);

static void       ob_filters_filter(void);
static void       ob_filters_filter_script(void);
static void       st_filters_filter_file(void);
static void       st_filters_filter_name(void);

static void       ob_collections(void);
static void       st_collections_base_scale(void);

static void       ob_collections_group(void);
static void       st_collections_group_name(void);
static void       st_collections_group_skip_namespace_validation(void);
static void       st_collections_group_inherit_only(void);
static void       st_collections_group_inherit(void);
static void       st_collections_group_program_source(void);
static void       st_collections_group_part_remove(void);
static void       st_collections_group_program_remove(void);
static void       st_collections_group_lua_script_only(void);
static void       st_collections_group_script_recursion(void);
static void       st_collections_group_alias(void);
static void       st_collections_group_min(void);
static void       st_collections_group_max(void);
static void       st_collections_group_scene_size(void);
static void       st_collections_group_broadcast_signal(void);
static void       st_collections_group_data_item(void);
static void       st_collections_group_orientation(void);
static void       st_collections_group_mouse_events(void);
static void       st_collections_group_use_custom_seat_names(void);
static void       st_collections_group_inherit_script(void);

static void       st_collections_group_limits_vertical(void);
static void       st_collections_group_limits_horizontal(void);

static void       ob_collections_group_script(void);
static void       ob_collections_group_lua_script(void);

static void       st_collections_group_parts_alias(void);

static Edje_Part *edje_cc_handlers_part_make(int);
static void       ob_collections_group_parts_part(void);
static void       st_collections_group_parts_part_name(void);
static void       st_collections_group_parts_part_inherit(void);
static void       st_collections_group_parts_part_type(void);
#ifdef HAVE_EPHYSICS
static void       st_collections_group_parts_part_physics_body(void);
#endif
static void       st_collections_group_parts_part_insert_before(void);
static void       st_collections_group_parts_part_insert_after(void);
static void       st_collections_group_parts_part_effect(void);
static void       st_collections_group_parts_part_mouse_events(void);
static void       st_collections_group_parts_part_anti_alias(void);
static void       st_collections_group_parts_part_repeat_events(void);
static void       st_collections_group_parts_part_ignore_flags(void);
static void       st_collections_group_parts_part_mask_flags(void);
static void       st_collections_group_parts_part_scale(void);
static void       st_collections_group_parts_part_pointer_mode(void);
static void       st_collections_group_parts_part_precise_is_inside(void);
static void       st_collections_group_parts_part_use_alternate_font_metrics(void);
static void       st_collections_group_parts_part_clip_to_id(void);
static void       st_collections_group_parts_part_render(void);
static void       st_collections_group_parts_part_no_render(void);
static void       st_collections_group_parts_part_required(void);
static void       st_collections_group_parts_part_norequired(void);
static void       st_collections_group_parts_part_source(void);
static void       st_collections_group_parts_part_source2(void);
static void       st_collections_group_parts_part_source3(void);
static void       st_collections_group_parts_part_source4(void);
static void       st_collections_group_parts_part_source5(void);
static void       st_collections_group_parts_part_source6(void);
static void       st_collections_group_parts_part_entry_mode(void);
static void       st_collections_group_parts_part_select_mode(void);
static void       st_collections_group_parts_part_cursor_mode(void);
static void       st_collections_group_parts_part_multiline(void);
static void       st_collections_group_parts_part_access(void);
static void       st_collections_group_parts_part_dragable_x(void);
static void       st_collections_group_parts_part_dragable_y(void);
static void       st_collections_group_parts_part_dragable_confine(void);
static void       st_collections_group_parts_part_dragable_threshold(void);
static void       st_collections_group_parts_part_dragable_events(void);
static void       st_collections_group_parts_part_allowed_seats(void);

/* box and table items share these */
static void       ob_collections_group_parts_part_box_items_item(void);
static void       st_collections_group_parts_part_box_items_item_type(void);
static void       st_collections_group_parts_part_box_items_item_name(void);
static void       st_collections_group_parts_part_box_items_item_source(void);
static void       st_collections_group_parts_part_box_items_item_min(void);
static void       st_collections_group_parts_part_box_items_item_spread(void);
static void       st_collections_group_parts_part_box_items_item_prefer(void);
static void       st_collections_group_parts_part_box_items_item_max(void);
static void       st_collections_group_parts_part_box_items_item_padding(void);
static void       st_collections_group_parts_part_box_items_item_align(void);
static void       st_collections_group_parts_part_box_items_item_weight(void);
static void       st_collections_group_parts_part_box_items_item_aspect(void);
static void       st_collections_group_parts_part_box_items_item_aspect_mode(void);
static void       st_collections_group_parts_part_box_items_item_options(void);
/* but these are only for table */
static void       st_collections_group_parts_part_table_items_item_position(void);
static void       st_collections_group_parts_part_table_items_item_span(void);

static void       ob_collections_group_parts_part_description(void);
static void       ob_collections_group_parts_part_desc(void);
static void       st_collections_group_parts_part_description_inherit(void);
static void       ob_collections_group_parts_part_description_link(void);
static void       st_collections_group_parts_part_description_link_base(void);
static void       st_collections_group_parts_part_description_source(void);
static void       st_collections_group_parts_part_description_state(void);
static void       st_collections_group_parts_part_description_visible(void);
static void       st_collections_group_parts_part_description_no_render(void);
static void       st_collections_group_parts_part_description_limit(void);
static void       st_collections_group_parts_part_description_align(void);
static void       st_collections_group_parts_part_description_fixed(void);
static void       st_collections_group_parts_part_description_min(void);
static void       st_collections_group_parts_part_description_minmul(void);
static void       st_collections_group_parts_part_description_max(void);
static void       st_collections_group_parts_part_description_step(void);
static void       st_collections_group_parts_part_description_aspect(void);
static void       st_collections_group_parts_part_description_aspect_preference(void);
static void       st_collections_group_parts_part_description_rel_to(void);
static void       st_collections_group_parts_part_description_rel_to_x(void);
static void       st_collections_group_parts_part_description_rel_to_y(void);
static void       st_collections_group_parts_part_description_rel1_relative(void);
static void       st_collections_group_parts_part_description_rel1_offset(void);
static void       st_collections_group_parts_part_description_rel1_to_set(const char *name);
static void       st_collections_group_parts_part_description_rel1_to(void);
static void       st_collections_group_parts_part_description_rel1_to_x(void);
static void       st_collections_group_parts_part_description_rel1_to_y(void);
static void       st_collections_group_parts_part_description_rel2_relative(void);
static void       st_collections_group_parts_part_description_rel2_offset(void);
static void       st_collections_group_parts_part_description_rel2_to_set(const char *name);
static void       st_collections_group_parts_part_description_rel2_to(void);
static void       st_collections_group_parts_part_description_rel2_to_x(void);
static void       st_collections_group_parts_part_description_rel2_to_y(void);
static void       st_collections_group_parts_part_description_anchors_top(void);
static void       st_collections_group_parts_part_description_anchors_bottom(void);
static void       st_collections_group_parts_part_description_anchors_left(void);
static void       st_collections_group_parts_part_description_anchors_right(void);
static void       st_collections_group_parts_part_description_anchors_vertical_center(void);
static void       st_collections_group_parts_part_description_anchors_horizontal_center(void);
static void       st_collections_group_parts_part_description_anchors_fill(void);
static void       st_collections_group_parts_part_description_anchors_margin(void);
static void       st_collections_group_parts_part_description_clip_to_id(void);
static void       st_collections_group_parts_part_description_size_class(void);
static void       st_collections_group_parts_part_description_image_normal(void);
static void       st_collections_group_parts_part_description_image_tween(void);
static void       st_collections_group_parts_part_description_image_border(void);
static void       st_collections_group_parts_part_description_image_middle(void);
static void       st_collections_group_parts_part_description_image_border_scale(void);
static void       st_collections_group_parts_part_description_image_border_scale_by(void);
static void       st_collections_group_parts_part_description_image_scale_hint(void);
static void       st_collections_group_parts_part_description_fill_smooth(void);
static void       st_collections_group_parts_part_description_fill_origin_relative(void);
static void       st_collections_group_parts_part_description_fill_origin_offset(void);
static void       st_collections_group_parts_part_description_fill_size_relative(void);
static void       st_collections_group_parts_part_description_fill_size_offset(void);
static void       st_collections_group_parts_part_description_fill_type(void);
static void       st_collections_group_parts_part_description_color_class(void);
static void       st_collections_group_parts_part_description_color(void);
static void       st_collections_group_parts_part_description_color2(void);
static void       st_collections_group_parts_part_description_color3(void);
static void       st_collections_group_parts_part_description_text_text(void);
static void       st_collections_group_parts_part_description_text_domain(void);
static void       st_collections_group_parts_part_description_text_text_class(void);
static void       st_collections_group_parts_part_description_text_font(void);
static void       st_collections_group_parts_part_description_text_style(void);
static void       st_collections_group_parts_part_description_text_repch(void);
static void       st_collections_group_parts_part_description_text_size(void);
static void       st_collections_group_parts_part_description_text_size_range(void);
static void       st_collections_group_parts_part_description_text_fit(void);
static void       st_collections_group_parts_part_description_text_min(void);
static void       st_collections_group_parts_part_description_text_max(void);
static void       st_collections_group_parts_part_description_text_align(void);
static void       st_collections_group_parts_part_description_text_source(void);
static void       st_collections_group_parts_part_description_text_text_source(void);
static void       st_collections_group_parts_part_description_text_ellipsis(void);
static void       st_collections_group_parts_part_description_box_layout(void);
static void       st_collections_group_parts_part_description_box_align(void);
static void       st_collections_group_parts_part_description_box_padding(void);
static void       st_collections_group_parts_part_description_box_min(void);
static void       st_collections_group_parts_part_description_table_homogeneous(void);
static void       st_collections_group_parts_part_description_table_align(void);
static void       st_collections_group_parts_part_description_table_padding(void);
static void       st_collections_group_parts_part_description_table_min(void);
static void       st_collections_group_parts_part_description_proxy_source_visible(void);
static void       st_collections_group_parts_part_description_proxy_source_clip(void);
static void       st_collections_group_parts_part_description_position_point(void);
static void       st_collections_group_parts_part_description_position_space(void);
static void       st_collections_group_parts_part_description_camera_properties(void);
static void       st_collections_group_parts_part_description_properties_ambient(void);
static void       st_collections_group_parts_part_description_properties_diffuse(void);
static void       st_collections_group_parts_part_description_properties_specular(void);
static void       st_collections_group_parts_part_description_properties_material(void);
static void       st_collections_group_parts_part_description_properties_normal(void);
static void       st_collections_group_parts_part_description_properties_shininess(void);
static void       st_collections_group_parts_part_description_properties_shade(void);
static void       st_collections_group_parts_part_description_orientation_angle_axis(void);
static void       st_collections_group_parts_part_description_orientation_look1(void);
static void       st_collections_group_parts_part_description_orientation_look2(void);
static void       st_collections_group_parts_part_description_orientation_look_to(void);
static void       st_collections_group_parts_part_description_orientation_angle_axis(void);
static void       st_collections_group_parts_part_description_orientation_quaternion(void);
static void       st_collections_group_parts_part_description_scale(void);
static void       st_collections_group_parts_part_description_offset_scale(void);
static void       st_collections_group_parts_part_description_mesh_primitive(void);
static void       ob_collections_group_parts_part_description_texture(void);
static void       st_collections_group_parts_part_description_texture_image(void);
static void       st_collections_group_parts_part_description_texture_wrap1(void);
static void       st_collections_group_parts_part_description_texture_wrap2(void);
static void       st_collections_group_parts_part_description_texture_filter1(void);
static void       st_collections_group_parts_part_description_texture_filter2(void);
static void       st_collections_group_parts_part_description_mesh_assembly(void);
static void       st_collections_group_parts_part_description_mesh_geometry(void);
static void       st_collections_group_parts_part_description_mesh_frame(void);
static void       st_collections_group_parts_part_description_filter_code(void);
static void       st_collections_group_parts_part_description_filter_source(void);
static void       st_collections_group_parts_part_description_filter_data(void);

#ifdef HAVE_EPHYSICS
static void       st_collections_group_parts_part_description_physics_mass(void);
static void       st_collections_group_parts_part_description_physics_restitution(void);
static void       st_collections_group_parts_part_description_physics_friction(void);
static void       st_collections_group_parts_part_description_physics_damping(void);
static void       st_collections_group_parts_part_description_physics_sleep(void);
static void       st_collections_group_parts_part_description_physics_material(void);
static void       st_collections_group_parts_part_description_physics_density(void);
static void       st_collections_group_parts_part_description_physics_hardness(void);
static void       st_collections_group_parts_part_description_physics_ignore_part_pos(void);
static void       st_collections_group_parts_part_description_physics_light_on(void);
static void       st_collections_group_parts_part_description_physics_z(void);
static void       st_collections_group_parts_part_description_physics_depth(void);
static void       st_collections_group_parts_part_description_physics_movement_freedom_linear(void);
static void       st_collections_group_parts_part_description_physics_movement_freedom_angular(void);
static void       st_collections_group_parts_part_description_physics_backface_cull(void);
static void       st_collections_group_parts_part_description_physics_face(void);
static void       st_collections_group_parts_part_description_physics_face_type(void);
static void       st_collections_group_parts_part_description_physics_face_source(void);
#endif
static void       st_collections_group_parts_part_description_map_perspective(void);
static void       st_collections_group_parts_part_description_map_light(void);
static void       st_collections_group_parts_part_description_map_rotation_center(void);
static void       st_collections_group_parts_part_description_map_rotation_x(void);
static void       st_collections_group_parts_part_description_map_rotation_y(void);
static void       st_collections_group_parts_part_description_map_rotation_z(void);
static void       st_collections_group_parts_part_description_map_on(void);
static void       st_collections_group_parts_part_description_map_smooth(void);
static void       st_collections_group_parts_part_description_map_alpha(void);
static void       st_collections_group_parts_part_description_map_backface_cull(void);
static void       st_collections_group_parts_part_description_map_perspective_on(void);
static void       st_collections_group_parts_part_description_map_color(void);
static void       st_collections_group_parts_part_description_map_zoom_x(void);
static void       st_collections_group_parts_part_description_map_zoom_y(void);
static void       st_collections_group_parts_part_description_perspective_zplane(void);
static void       st_collections_group_parts_part_description_perspective_focal(void);
static void       st_collections_group_parts_part_api(void);

/* external part parameters */
static void       st_collections_group_parts_part_description_params_int(void);
static void       st_collections_group_parts_part_description_params_double(void);
static void       st_collections_group_parts_part_description_params_string(void);
static void       st_collections_group_parts_part_description_params_bool(void);
static void       st_collections_group_parts_part_description_params_choice(void);
static void       st_collections_group_parts_part_description_params_smart(void);

static void       ob_collections_group_programs_program(void);
static void       st_collections_group_programs_program_name(void);
static void       st_collections_group_programs_program_signal(void);
static void       st_collections_group_programs_program_source(void);
static void       st_collections_group_programs_program_filter(void);
static void       st_collections_group_programs_program_in(void);
static void       st_collections_group_programs_program_action(void);
static void       st_collections_group_programs_program_transition(void);
static void       st_collections_group_programs_program_target(void);
static void       st_collections_group_programs_program_targets(void);
static void       st_collections_group_programs_program_target_groups(void);
static void       st_collections_group_programs_program_after(void);
static void       st_collections_group_programs_program_api(void);
static void       st_collections_group_target_group(void);

static void       ob_collections_group_programs_program_sequence(void);

static void       ob_collections_group_programs_program_script(void);
static void       st_collections_group_sound_sample_name(void);
static void       st_collections_group_sound_sample_source(void);
static void       st_collections_group_sound_tone(void);
static void       st_collections_group_vibration_sample_name(void);
static void       st_collections_group_vibration_sample_source(void);

static void       st_collections_group_translation_file_locale(void);
static void       st_collections_group_translation_file_source(void);
#ifdef HAVE_EPHYSICS
static void       st_collections_group_physics_world_gravity(void);
static void       st_collections_group_physics_world_rate(void);
static void       st_collections_group_physics_world_z(void);
static void       st_collections_group_physics_world_depth(void);
#endif

/* short */
static void       st_collections_group_parts_part_noscale(void);
static void       st_collections_group_parts_part_precise(void);
static void       st_collections_group_parts_part_noprecise(void);
static void       st_collections_group_parts_part_mouse(void);
static void       st_collections_group_parts_part_nomouse(void);
static void       st_collections_group_parts_part_repeat(void);
static void       st_collections_group_parts_part_norepeat(void);
static void       st_collections_group_parts_part_description_vis(void);
static void       st_collections_group_parts_part_description_hid(void);
static void       ob_collections_group_parts_part_short(void);

static void       st_collections_group_mouse(void);
static void       st_collections_group_nomouse(void);
static void       st_collections_group_broadcast(void);
static void       st_collections_group_nobroadcast(void);
static void       st_collections_group_noinherit_script(void);

static void       st_images_vector(void);
static void       _handle_vector_image(void);

/*****/

#define STRDUP(x) eina_strdup(x)

#define IMAGE_STATEMENTS(PREFIX)                                  \
  {PREFIX "images.image", st_images_image},                       \
  {PREFIX "images.vector", st_images_vector},                     \
  {PREFIX "images.set.name", st_images_set_name},                 \
  {PREFIX "images.set.image.image", st_images_set_image_image},   \
  {PREFIX "images.set.image.size", st_images_set_image_size},     \
  {PREFIX "images.set.image.border", st_images_set_image_border}, \
  {PREFIX "images.set.image.scale_by", st_images_set_image_border_scale_by},

#define IMAGE_SET_STATEMENTS(PREFIX)                                    \
  {PREFIX ".image", st_images_image},   /* dup */                       \
  {PREFIX ".set.name", st_images_set_name},   /* dup */                 \
  {PREFIX ".set.image.image", st_images_set_image_image},   /* dup */   \
  {PREFIX ".set.image.size", st_images_set_image_size},   /* dup */     \
  {PREFIX ".set.image.border", st_images_set_image_border},   /* dup */ \
  {PREFIX ".set.image.scale_by", st_images_set_image_border_scale_by},   /* dup */

#define FONT_STYLE_CC_STATEMENTS(PREFIX)                                             \
  {PREFIX "fonts.font", st_fonts_font},   /* dup */                                  \
  {PREFIX "styles.style.name", st_styles_style_name},   /* dup */                    \
  {PREFIX "styles.style.base", st_styles_style_base},   /* dup */                    \
  {PREFIX "styles.style.tag", st_styles_style_tag},   /* dup */                      \
  {PREFIX "color_classes.color_class.name", st_color_class_name},   /* dup */        \
  {PREFIX "color_classes.color_class.color", st_color_class_color},   /* dup */      \
  {PREFIX "color_classes.color_class.color2", st_color_class_color2},   /* dup */    \
  {PREFIX "color_classes.color_class.color3", st_color_class_color3},   /* dup */    \
  {PREFIX "color_classes.color_class.description", st_color_class_desc},   /* dup */ \
  {PREFIX "color_classes.color_class.desc", st_color_class_desc},   /* dup */

#define TEXT_CLASS_STATEMENTS(PREFIX)                                      \
  {PREFIX "text_classes.text_class.name", st_text_class_name},   /* dup */ \
  {PREFIX "text_classes.text_class.font", st_text_class_font},   /* dup */ \
  {PREFIX "text_classes.text_class.size", st_text_class_size},   /* dup */

#define SIZE_CLASS_STATEMENTS(PREFIX)                                      \
  {PREFIX "size_classes.size_class.name", st_size_class_name},   /* dup */ \
  {PREFIX "size_classes.size_class.min", st_size_class_min},   /* dup */   \
  {PREFIX "size_classes.size_class.max", st_size_class_max},   /* dup */

#define PROGRAM_SEQUENCE(PREFIX, NAME, FN)  \
  {PREFIX ".program."NAME, FN},   /* dup */ \
  {PREFIX ".program.sequence."NAME, FN},   /* dup */

#define PROGRAM_BASE(PREFIX)                                                                     \
  PROGRAM_SEQUENCE(PREFIX, "name", st_collections_group_programs_program_name)                   \
  PROGRAM_SEQUENCE(PREFIX, "signal", st_collections_group_programs_program_signal)               \
  PROGRAM_SEQUENCE(PREFIX, "source", st_collections_group_programs_program_source)               \
  PROGRAM_SEQUENCE(PREFIX, "in", st_collections_group_programs_program_in)                       \
  PROGRAM_SEQUENCE(PREFIX, "action", st_collections_group_programs_program_action)               \
  PROGRAM_SEQUENCE(PREFIX, "transition", st_collections_group_programs_program_transition)       \
  PROGRAM_SEQUENCE(PREFIX, "target", st_collections_group_programs_program_target)               \
  PROGRAM_SEQUENCE(PREFIX, "target_groups", st_collections_group_programs_program_target_groups) \
  PROGRAM_SEQUENCE(PREFIX, "groups", st_collections_group_programs_program_target_groups)        \
  PROGRAM_SEQUENCE(PREFIX, "targets", st_collections_group_programs_program_targets)             \
  PROGRAM_SEQUENCE(PREFIX, "after", st_collections_group_programs_program_after)                 \
  PROGRAM_SEQUENCE(PREFIX, "api", st_collections_group_programs_program_api)                     \
  PROGRAM_SEQUENCE(PREFIX, "filter", st_collections_group_programs_program_filter)

#define PROGRAM_STATEMENTS(PREFIX)                            \
  IMAGE_SET_STATEMENTS(PREFIX ".programs")                    \
  IMAGE_STATEMENTS(PREFIX ".programs.")                       \
  IMAGE_SET_STATEMENTS(PREFIX ".programs")                    \
  {PREFIX ".programs.font", st_fonts_font},   /* dup */       \
  {PREFIX ".programs.fonts.font", st_fonts_font},   /* dup */ \
  PROGRAM_BASE(PREFIX)                                        \
  PROGRAM_BASE(PREFIX ".programs")

#define SET_LOOK1(Type, type_node)                                                   \
  Edje_Part_Description_##Type * ed;                                                 \
  ed = (Edje_Part_Description_##Type *)current_desc;                                 \
                                                                                     \
  if (ed->type_node.orientation.type <= EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT) \
    {                                                                                \
       ed->type_node.orientation.data[0] = FROM_DOUBLE(parse_float(0));              \
       ed->type_node.orientation.data[1] = FROM_DOUBLE(parse_float(1));              \
       ed->type_node.orientation.data[2] = FROM_DOUBLE(parse_float(2));              \
       ed->type_node.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT; \
    }

#define SET_LOOK2(Type, type_node)                                                   \
  Edje_Part_Description_##Type * ed;                                                 \
  ed = (Edje_Part_Description_##Type *)current_desc;                                 \
                                                                                     \
  if (ed->type_node.orientation.type <= EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT) \
    {                                                                                \
       ed->type_node.orientation.data[3] = FROM_DOUBLE(parse_float(0));              \
       ed->type_node.orientation.data[4] = FROM_DOUBLE(parse_float(1));              \
       ed->type_node.orientation.data[5] = FROM_DOUBLE(parse_float(2));              \
       ed->type_node.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT; \
    }

#define SET_LOOK_TO(list, Type, type_node)                                           \
  Edje_Part_Description_##Type * ed;                                                 \
  char *name;                                                                        \
                                                                                     \
  ed = (Edje_Part_Description_##Type *)current_desc;                                 \
                                                                                     \
  if (ed->type_node.orientation.type <= EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_TO) \
    {                                                                                \
       name = parse_str(0);                                                          \
       data_queue_part_lookup(list, name, &(ed->type_node.orientation.look_to));     \
       free(name);                                                                   \
       ed->type_node.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_TO; \
    }

#define SET_ANGLE_AXIS(Type, type_node)                                                 \
  Edje_Part_Description_##Type * ed;                                                    \
  ed = (Edje_Part_Description_##Type *)current_desc;                                    \
                                                                                        \
  if (ed->type_node.orientation.type <= EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_ANGLE_AXIS) \
    {                                                                                   \
       ed->type_node.orientation.data[0] = FROM_DOUBLE(parse_float(0));                 \
       ed->type_node.orientation.data[1] = FROM_DOUBLE(parse_float(1));                 \
       ed->type_node.orientation.data[2] = FROM_DOUBLE(parse_float(2));                 \
       ed->type_node.orientation.data[3] = FROM_DOUBLE(parse_float(3));                 \
       ed->type_node.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_ANGLE_AXIS; \
    }

#define SET_QUATERNION(Type, type_node)                                                 \
  Edje_Part_Description_##Type * ed;                                                    \
  ed = (Edje_Part_Description_##Type *)current_desc;                                    \
                                                                                        \
  if (ed->type_node.orientation.type <= EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_QUATERNION) \
    {                                                                                   \
       ed->type_node.orientation.data[1] = FROM_DOUBLE(parse_float(0));                 \
       ed->type_node.orientation.data[2] = FROM_DOUBLE(parse_float(1));                 \
       ed->type_node.orientation.data[3] = FROM_DOUBLE(parse_float(2));                 \
       ed->type_node.orientation.data[0] = FROM_DOUBLE(parse_float(3));                 \
       ed->type_node.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_QUATERNION; \
    }

#define FILTERS_STATEMENTS(PREFIX)                        \
  {PREFIX "filters.filter.file", st_filters_filter_file}, \
  {PREFIX "filters.filter.name", st_filters_filter_name},

New_Statement_Handler statement_handlers[] =
{
   {"id", st_id},
   {"requires", st_requires},
   {"efl_version", st_efl_version},
   {"externals.external", st_externals_external},
   IMAGE_STATEMENTS("")
   FONT_STYLE_CC_STATEMENTS("")
   TEXT_CLASS_STATEMENTS("")
   SIZE_CLASS_STATEMENTS("")
   {
      "data.item", st_data_item
   },
   {"data.file", st_data_file},
   FILTERS_STATEMENTS("")
   {
      "collections.externals.external", st_externals_external
   },                                                           /* dup */
   IMAGE_STATEMENTS("collections.")
   IMAGE_SET_STATEMENTS("collections")
   {
      "collections.font", st_fonts_font
   },                                     /* dup */
   FONT_STYLE_CC_STATEMENTS("collections.")
   TEXT_CLASS_STATEMENTS("collections.")
   SIZE_CLASS_STATEMENTS("collections.")
   {
      "collections.base_scale", st_collections_base_scale
   },
   {"collections.translation.file.locale", st_collections_group_translation_file_locale},
   {"collections.translation.file.source", st_collections_group_translation_file_source},
   {"collections.group.translation.file.locale", st_collections_group_translation_file_locale},
   {"collections.group.translation.file.source", st_collections_group_translation_file_source},

   {"collections.sounds.sample.name", st_collections_group_sound_sample_name},
   {"collections.sounds.sample.source", st_collections_group_sound_sample_source},
   {"collections.group.sounds.sample.name", st_collections_group_sound_sample_name},   /* dup */
   {"collections.group.sounds.sample.source", st_collections_group_sound_sample_source},   /* dup */
   {"collections.sounds.tone", st_collections_group_sound_tone},
   {"collections.group.sounds.tone", st_collections_group_sound_tone},   /* dup */
   {"collections.vibrations.sample.name", st_collections_group_vibration_sample_name},
   {"collections.vibrations.sample.source", st_collections_group_vibration_sample_source},
   FILTERS_STATEMENTS("collections.")   /* dup */
   {
      "collections.group.vibrations.sample.name", st_collections_group_vibration_sample_name
   },                                                                                          /* dup */
   {"collections.group.vibrations.sample.source", st_collections_group_vibration_sample_source},   /* dup */
   {"collections.group.name", st_collections_group_name},
   {"collections.group.skip_namespace_validation", st_collections_group_skip_namespace_validation},
   {"collections.group.program_source", st_collections_group_program_source},
   {"collections.group.inherit", st_collections_group_inherit},
   {"collections.group.inherit_only", st_collections_group_inherit_only},
   {"collections.group.use_custom_seat_names", st_collections_group_use_custom_seat_names},
   {"collections.group.target_group", st_collections_group_target_group},   /* dup */
   {"collections.group.part_remove", st_collections_group_part_remove},
   {"collections.group.program_remove", st_collections_group_program_remove},
   {"collections.group.lua_script_only", st_collections_group_lua_script_only},
   {"collections.group.script_recursion", st_collections_group_script_recursion},
   {"collections.group.alias", st_collections_group_alias},
   {"collections.group.min", st_collections_group_min},
   {"collections.group.max", st_collections_group_max},
   {"collections.group.scene_size", st_collections_group_scene_size},
   {"collections.group.broadcast_signal", st_collections_group_broadcast_signal},
   {"collections.group.orientation", st_collections_group_orientation},
   {"collections.group.mouse_events", st_collections_group_mouse_events},
   {"collections.group.inherit_script", st_collections_group_inherit_script},
   {"collections.group.data.item", st_collections_group_data_item},
   {"collections.group.limits.horizontal", st_collections_group_limits_horizontal},
   {"collections.group.limits.vertical", st_collections_group_limits_vertical},
   {"collections.group.externals.external", st_externals_external},   /* dup */
   {"collections.group.programs.target_group", st_collections_group_target_group},   /* dup */
   IMAGE_SET_STATEMENTS("collections.group")
   IMAGE_STATEMENTS("collections.group.")
   {
      "collections.group.models.model", st_models_model
   },
   {"collections.group.font", st_fonts_font},   /* dup */
   FONT_STYLE_CC_STATEMENTS("collections.group.")
   TEXT_CLASS_STATEMENTS("collections.group.")
   SIZE_CLASS_STATEMENTS("collections.group.")
   {
      "collections.group.parts.alias", st_collections_group_parts_alias
   },
   IMAGE_SET_STATEMENTS("collections.group.parts")
   IMAGE_STATEMENTS("collections.group.parts.")
   {
      "collections.group.parts.font", st_fonts_font
   },                                                 /* dup */
   FONT_STYLE_CC_STATEMENTS("collections.group.parts.")
   TEXT_CLASS_STATEMENTS("collections.group.parts.")
   SIZE_CLASS_STATEMENTS("collections.group.parts.")
   {
      "collections.group.parts.target_group", st_collections_group_target_group
   },                                                                             /* dup */
   {"collections.group.parts.part.name", st_collections_group_parts_part_name},
   {"collections.group.parts.part.target_group", st_collections_group_target_group},   /* dup */
   {"collections.group.parts.part.inherit", st_collections_group_parts_part_inherit},
   {"collections.group.parts.part.api", st_collections_group_parts_part_api},
   {"collections.group.parts.part.type", st_collections_group_parts_part_type},
#ifdef HAVE_EPHYSICS
   {"collections.group.parts.part.physics_body", st_collections_group_parts_part_physics_body},
#endif
   {"collections.group.parts.part.insert_before", st_collections_group_parts_part_insert_before},
   {"collections.group.parts.part.insert_after", st_collections_group_parts_part_insert_after},
   {"collections.group.parts.part.effect", st_collections_group_parts_part_effect},
   {"collections.group.parts.part.mouse_events", st_collections_group_parts_part_mouse_events},
   {"collections.group.parts.part.anti_alias", st_collections_group_parts_part_anti_alias},
   {"collections.group.parts.part.repeat_events", st_collections_group_parts_part_repeat_events},
   {"collections.group.parts.part.ignore_flags", st_collections_group_parts_part_ignore_flags},
   {"collections.group.parts.part.mask_flags", st_collections_group_parts_part_mask_flags},
   {"collections.group.parts.part.scale", st_collections_group_parts_part_scale},
   {"collections.group.parts.part.pointer_mode", st_collections_group_parts_part_pointer_mode},
   {"collections.group.parts.part.precise_is_inside", st_collections_group_parts_part_precise_is_inside},
   {"collections.group.parts.part.use_alternate_font_metrics", st_collections_group_parts_part_use_alternate_font_metrics},
   {"collections.group.parts.part.clip_to", st_collections_group_parts_part_clip_to_id},
   {"collections.group.parts.part.no_render", st_collections_group_parts_part_no_render},
   {"collections.group.parts.part.required", st_collections_group_parts_part_required},
   {"collections.group.parts.part.source", st_collections_group_parts_part_source},
   {"collections.group.parts.part.source2", st_collections_group_parts_part_source2},
   {"collections.group.parts.part.source3", st_collections_group_parts_part_source3},
   {"collections.group.parts.part.source4", st_collections_group_parts_part_source4},
   {"collections.group.parts.part.source5", st_collections_group_parts_part_source5},
   {"collections.group.parts.part.source6", st_collections_group_parts_part_source6},
   {"collections.group.parts.part.dragable.x", st_collections_group_parts_part_dragable_x},
   {"collections.group.parts.part.dragable.y", st_collections_group_parts_part_dragable_y},
   {"collections.group.parts.part.dragable.confine", st_collections_group_parts_part_dragable_confine},
   {"collections.group.parts.part.dragable.threshold", st_collections_group_parts_part_dragable_threshold},
   {"collections.group.parts.part.dragable.events", st_collections_group_parts_part_dragable_events},
   {"collections.group.parts.part.entry_mode", st_collections_group_parts_part_entry_mode},
   {"collections.group.parts.part.select_mode", st_collections_group_parts_part_select_mode},
   {"collections.group.parts.part.cursor_mode", st_collections_group_parts_part_cursor_mode},
   {"collections.group.parts.part.multiline", st_collections_group_parts_part_multiline},
   {"collections.group.parts.part.access", st_collections_group_parts_part_access},
   {"collections.group.parts.part.allowed_seats", st_collections_group_parts_part_allowed_seats},
   IMAGE_SET_STATEMENTS("collections.group.parts.part")
   IMAGE_STATEMENTS("collections.group.parts.part.")
   {
      "collections.group.parts.part.font", st_fonts_font
   },                                                      /* dup */
   FONT_STYLE_CC_STATEMENTS("collections.group.parts.part.")
   TEXT_CLASS_STATEMENTS("collections.group.parts.part.")
   SIZE_CLASS_STATEMENTS("collections.group.parts.part.")
   {
      "collections.group.parts.part.box.items.item.type", st_collections_group_parts_part_box_items_item_type
   },
   {"collections.group.parts.part.box.items.item.name", st_collections_group_parts_part_box_items_item_name},
   {"collections.group.parts.part.box.items.item.source", st_collections_group_parts_part_box_items_item_source},
   {"collections.group.parts.part.box.items.item.min", st_collections_group_parts_part_box_items_item_min},
   {"collections.group.parts.part.box.items.item.spread", st_collections_group_parts_part_box_items_item_spread},
   {"collections.group.parts.part.box.items.item.prefer", st_collections_group_parts_part_box_items_item_prefer},
   {"collections.group.parts.part.box.items.item.max", st_collections_group_parts_part_box_items_item_max},
   {"collections.group.parts.part.box.items.item.padding", st_collections_group_parts_part_box_items_item_padding},
   {"collections.group.parts.part.box.items.item.align", st_collections_group_parts_part_box_items_item_align},
   {"collections.group.parts.part.box.items.item.weight", st_collections_group_parts_part_box_items_item_weight},
   {"collections.group.parts.part.box.items.item.aspect", st_collections_group_parts_part_box_items_item_aspect},
   {"collections.group.parts.part.box.items.item.aspect_mode", st_collections_group_parts_part_box_items_item_aspect_mode},
   {"collections.group.parts.part.box.items.item.options", st_collections_group_parts_part_box_items_item_options},
   {"collections.group.parts.part.table.items.item.type", st_collections_group_parts_part_box_items_item_type},   /* dup */
   {"collections.group.parts.part.table.items.item.name", st_collections_group_parts_part_box_items_item_name},   /* dup */
   {"collections.group.parts.part.table.items.item.source", st_collections_group_parts_part_box_items_item_source},   /* dup */
   {"collections.group.parts.part.table.items.item.min", st_collections_group_parts_part_box_items_item_min},   /* dup */
   {"collections.group.parts.part.table.items.item.spread", st_collections_group_parts_part_box_items_item_spread},   /* dup */
   {"collections.group.parts.part.table.items.item.prefer", st_collections_group_parts_part_box_items_item_prefer},   /* dup */
   {"collections.group.parts.part.table.items.item.max", st_collections_group_parts_part_box_items_item_max},   /* dup */
   {"collections.group.parts.part.table.items.item.padding", st_collections_group_parts_part_box_items_item_padding},   /* dup */
   {"collections.group.parts.part.table.items.item.align", st_collections_group_parts_part_box_items_item_align},   /* dup */
   {"collections.group.parts.part.table.items.item.weight", st_collections_group_parts_part_box_items_item_weight},   /* dup */
   {"collections.group.parts.part.table.items.item.aspect", st_collections_group_parts_part_box_items_item_aspect},   /* dup */
   {"collections.group.parts.part.table.items.item.aspect_mode", st_collections_group_parts_part_box_items_item_aspect_mode},   /* dup */
   {"collections.group.parts.part.table.items.item.options", st_collections_group_parts_part_box_items_item_options},   /* dup */
   {"collections.group.parts.part.table.items.item.position", st_collections_group_parts_part_table_items_item_position},
   {"collections.group.parts.part.table.items.item.span", st_collections_group_parts_part_table_items_item_span},
   {"collections.group.parts.part.description.target_group", st_collections_group_target_group},   /* dup */
   {"collections.group.parts.part.description.inherit", st_collections_group_parts_part_description_inherit},
   {"collections.group.parts.part.description.link.base", st_collections_group_parts_part_description_link_base},
   {"collections.group.parts.part.description.link.transition", st_collections_group_programs_program_transition},
   {"collections.group.parts.part.description.link.after", st_collections_group_programs_program_after},
   {"collections.group.parts.part.description.link.in", st_collections_group_programs_program_in},
   {"collections.group.parts.part.description.source", st_collections_group_parts_part_description_source},
   {"collections.group.parts.part.description.state", st_collections_group_parts_part_description_state},
   {"collections.group.parts.part.description.visible", st_collections_group_parts_part_description_visible},
   {"collections.group.parts.part.description.limit", st_collections_group_parts_part_description_limit},
   {"collections.group.parts.part.description.no_render", st_collections_group_parts_part_description_no_render},
   {"collections.group.parts.part.description.align", st_collections_group_parts_part_description_align},
   {"collections.group.parts.part.description.fixed", st_collections_group_parts_part_description_fixed},
   {"collections.group.parts.part.description.min", st_collections_group_parts_part_description_min},
   {"collections.group.parts.part.description.minmul", st_collections_group_parts_part_description_minmul},
   {"collections.group.parts.part.description.max", st_collections_group_parts_part_description_max},
   {"collections.group.parts.part.description.step", st_collections_group_parts_part_description_step},
   {"collections.group.parts.part.description.aspect", st_collections_group_parts_part_description_aspect},
   {"collections.group.parts.part.description.aspect_preference", st_collections_group_parts_part_description_aspect_preference},
   {"collections.group.parts.part.description.rel.to", st_collections_group_parts_part_description_rel_to},
   {"collections.group.parts.part.description.rel.to_x", st_collections_group_parts_part_description_rel_to_x},
   {"collections.group.parts.part.description.rel.to_y", st_collections_group_parts_part_description_rel_to_y},
   {"collections.group.parts.part.description.rel1.relative", st_collections_group_parts_part_description_rel1_relative},
   {"collections.group.parts.part.description.rel1.offset", st_collections_group_parts_part_description_rel1_offset},
   {"collections.group.parts.part.description.rel1.to", st_collections_group_parts_part_description_rel1_to},
   {"collections.group.parts.part.description.rel1.to_x", st_collections_group_parts_part_description_rel1_to_x},
   {"collections.group.parts.part.description.rel1.to_y", st_collections_group_parts_part_description_rel1_to_y},
   {"collections.group.parts.part.description.rel2.relative", st_collections_group_parts_part_description_rel2_relative},
   {"collections.group.parts.part.description.rel2.offset", st_collections_group_parts_part_description_rel2_offset},
   {"collections.group.parts.part.description.rel2.to", st_collections_group_parts_part_description_rel2_to},
   {"collections.group.parts.part.description.rel2.to_x", st_collections_group_parts_part_description_rel2_to_x},
   {"collections.group.parts.part.description.rel2.to_y", st_collections_group_parts_part_description_rel2_to_y},
   {"collections.group.parts.part.description.offset_scale", st_collections_group_parts_part_description_offset_scale},
   {"collections.group.parts.part.description.anchors.top", st_collections_group_parts_part_description_anchors_top},
   {"collections.group.parts.part.description.anchors.bottom", st_collections_group_parts_part_description_anchors_bottom},
   {"collections.group.parts.part.description.anchors.left", st_collections_group_parts_part_description_anchors_left},
   {"collections.group.parts.part.description.anchors.right", st_collections_group_parts_part_description_anchors_right},
   {"collections.group.parts.part.description.anchors.vertical_center", st_collections_group_parts_part_description_anchors_vertical_center},
   {"collections.group.parts.part.description.anchors.horizontal_center", st_collections_group_parts_part_description_anchors_horizontal_center},
   {"collections.group.parts.part.description.anchors.fill", st_collections_group_parts_part_description_anchors_fill},
   {"collections.group.parts.part.description.anchors.margin", st_collections_group_parts_part_description_anchors_margin},
   {"collections.group.parts.part.description.clip_to", st_collections_group_parts_part_description_clip_to_id},
   {"collections.group.parts.part.description.size_class", st_collections_group_parts_part_description_size_class},
   {"collections.group.parts.part.description.image.normal", st_collections_group_parts_part_description_image_normal},
   {"collections.group.parts.part.description.image.tween", st_collections_group_parts_part_description_image_tween},
   IMAGE_SET_STATEMENTS("collections.group.parts.part.description.image")
   IMAGE_STATEMENTS("collections.group.parts.part.description.image.")
   {
      "collections.group.parts.part.description.image.border", st_collections_group_parts_part_description_image_border
   },
   {"collections.group.parts.part.description.image.middle", st_collections_group_parts_part_description_image_middle},
   {"collections.group.parts.part.description.image.border_scale", st_collections_group_parts_part_description_image_border_scale},
   {"collections.group.parts.part.description.image.border_scale_by", st_collections_group_parts_part_description_image_border_scale_by},
   {"collections.group.parts.part.description.image.scale_hint", st_collections_group_parts_part_description_image_scale_hint},
   {"collections.group.parts.part.description.fill.smooth", st_collections_group_parts_part_description_fill_smooth},
   {"collections.group.parts.part.description.fill.origin.relative", st_collections_group_parts_part_description_fill_origin_relative},
   {"collections.group.parts.part.description.fill.origin.offset", st_collections_group_parts_part_description_fill_origin_offset},
   {"collections.group.parts.part.description.fill.size.relative", st_collections_group_parts_part_description_fill_size_relative},
   {"collections.group.parts.part.description.fill.size.offset", st_collections_group_parts_part_description_fill_size_offset},
   {"collections.group.parts.part.description.fill.type", st_collections_group_parts_part_description_fill_type},
   {"collections.group.parts.part.description.color_class", st_collections_group_parts_part_description_color_class},
   {"collections.group.parts.part.description.color", st_collections_group_parts_part_description_color},
   {"collections.group.parts.part.description.color2", st_collections_group_parts_part_description_color2},
   {"collections.group.parts.part.description.color3", st_collections_group_parts_part_description_color3},
   {"collections.group.parts.part.description.text.text", st_collections_group_parts_part_description_text_text},
   {"collections.group.parts.part.description.text.domain", st_collections_group_parts_part_description_text_domain},
   {"collections.group.parts.part.description.text.text_class", st_collections_group_parts_part_description_text_text_class},
   {"collections.group.parts.part.description.text.font", st_collections_group_parts_part_description_text_font},
   {"collections.group.parts.part.description.text.style", st_collections_group_parts_part_description_text_style},
   {"collections.group.parts.part.description.text.repch", st_collections_group_parts_part_description_text_repch},
   {"collections.group.parts.part.description.text.size", st_collections_group_parts_part_description_text_size},
   {"collections.group.parts.part.description.text.size_range", st_collections_group_parts_part_description_text_size_range},
   {"collections.group.parts.part.description.text.fit", st_collections_group_parts_part_description_text_fit},
   {"collections.group.parts.part.description.text.min", st_collections_group_parts_part_description_text_min},
   {"collections.group.parts.part.description.text.max", st_collections_group_parts_part_description_text_max},
   {"collections.group.parts.part.description.text.align", st_collections_group_parts_part_description_text_align},
   {"collections.group.parts.part.description.text.source", st_collections_group_parts_part_description_text_source},
   {"collections.group.parts.part.description.text.text_source", st_collections_group_parts_part_description_text_text_source},
   {"collections.group.parts.part.description.text.font", st_fonts_font},   /* dup */
   {"collections.group.parts.part.description.text.fonts.font", st_fonts_font},   /* dup */
   {"collections.group.parts.part.description.text.elipsis", st_collections_group_parts_part_description_text_ellipsis},
   {"collections.group.parts.part.description.text.ellipsis", st_collections_group_parts_part_description_text_ellipsis},
   {"collections.group.parts.part.description.text.filter", st_collections_group_parts_part_description_filter_code},   /* dup */
   {"collections.group.parts.part.description.box.layout", st_collections_group_parts_part_description_box_layout},
   {"collections.group.parts.part.description.box.align", st_collections_group_parts_part_description_box_align},
   {"collections.group.parts.part.description.box.padding", st_collections_group_parts_part_description_box_padding},
   {"collections.group.parts.part.description.box.min", st_collections_group_parts_part_description_box_min},
   {"collections.group.parts.part.description.table.homogeneous", st_collections_group_parts_part_description_table_homogeneous},
   {"collections.group.parts.part.description.table.align", st_collections_group_parts_part_description_table_align},
   {"collections.group.parts.part.description.table.padding", st_collections_group_parts_part_description_table_padding},
   {"collections.group.parts.part.description.table.min", st_collections_group_parts_part_description_table_min},
   {"collections.group.parts.part.description.proxy.source_visible", st_collections_group_parts_part_description_proxy_source_visible},
   {"collections.group.parts.part.description.proxy.source_clip", st_collections_group_parts_part_description_proxy_source_clip},
   {"collections.group.parts.part.description.position.point", st_collections_group_parts_part_description_position_point},
   {"collections.group.parts.part.description.position.space", st_collections_group_parts_part_description_position_space},
   {"collections.group.parts.part.description.properties.perspective", st_collections_group_parts_part_description_camera_properties},
   {"collections.group.parts.part.description.properties.ambient", st_collections_group_parts_part_description_properties_ambient},
   {"collections.group.parts.part.description.properties.diffuse", st_collections_group_parts_part_description_properties_diffuse},
   {"collections.group.parts.part.description.properties.specular", st_collections_group_parts_part_description_properties_specular},
   {"collections.group.parts.part.description.properties.material", st_collections_group_parts_part_description_properties_material},
   {"collections.group.parts.part.description.properties.normal", st_collections_group_parts_part_description_properties_normal},
   {"collections.group.parts.part.description.properties.shininess", st_collections_group_parts_part_description_properties_shininess},
   {"collections.group.parts.part.description.properties.shade", st_collections_group_parts_part_description_properties_shade},
   {"collections.group.parts.part.description.mesh.primitive", st_collections_group_parts_part_description_mesh_primitive},
   {"collections.group.parts.part.description.orientation.look1", st_collections_group_parts_part_description_orientation_look1},
   {"collections.group.parts.part.description.orientation.look2", st_collections_group_parts_part_description_orientation_look2},
   {"collections.group.parts.part.description.orientation.look_to", st_collections_group_parts_part_description_orientation_look_to},
   {"collections.group.parts.part.description.orientation.angle_axis", st_collections_group_parts_part_description_orientation_angle_axis},
   {"collections.group.parts.part.description.orientation.quaternion", st_collections_group_parts_part_description_orientation_quaternion},
   {"collections.group.parts.part.description.scale", st_collections_group_parts_part_description_scale},
   {"collections.group.parts.part.description.texture.image", st_collections_group_parts_part_description_texture_image},
   {"collections.group.parts.part.description.texture.wrap1", st_collections_group_parts_part_description_texture_wrap1},
   {"collections.group.parts.part.description.texture.wrap2", st_collections_group_parts_part_description_texture_wrap2},
   {"collections.group.parts.part.description.texture.filter1", st_collections_group_parts_part_description_texture_filter1},
   {"collections.group.parts.part.description.texture.filter2", st_collections_group_parts_part_description_texture_filter2},
   {"collections.group.parts.part.description.mesh.assembly", st_collections_group_parts_part_description_mesh_assembly},
   {"collections.group.parts.part.description.mesh.geometry", st_collections_group_parts_part_description_mesh_geometry},
   {"collections.group.parts.part.description.mesh.frame", st_collections_group_parts_part_description_mesh_frame},
   {"collections.group.parts.part.description.filter.code", st_collections_group_parts_part_description_filter_code},
   {"collections.group.parts.part.description.filter.source", st_collections_group_parts_part_description_filter_source},
   {"collections.group.parts.part.description.filter.data", st_collections_group_parts_part_description_filter_data},

#ifdef HAVE_EPHYSICS
   {"collections.group.parts.part.description.physics.mass", st_collections_group_parts_part_description_physics_mass},
   {"collections.group.parts.part.description.physics.restitution", st_collections_group_parts_part_description_physics_restitution},
   {"collections.group.parts.part.description.physics.friction", st_collections_group_parts_part_description_physics_friction},
   {"collections.group.parts.part.description.physics.damping", st_collections_group_parts_part_description_physics_damping},
   {"collections.group.parts.part.description.physics.sleep", st_collections_group_parts_part_description_physics_sleep},
   {"collections.group.parts.part.description.physics.material", st_collections_group_parts_part_description_physics_material},
   {"collections.group.parts.part.description.physics.density", st_collections_group_parts_part_description_physics_density},
   {"collections.group.parts.part.description.physics.hardness", st_collections_group_parts_part_description_physics_hardness},
   {"collections.group.parts.part.description.physics.movement_freedom.linear", st_collections_group_parts_part_description_physics_movement_freedom_linear},
   {"collections.group.parts.part.description.physics.movement_freedom.angular", st_collections_group_parts_part_description_physics_movement_freedom_angular},
   {"collections.group.parts.part.description.physics.ignore_part_pos", st_collections_group_parts_part_description_physics_ignore_part_pos},
   {"collections.group.parts.part.description.physics.light_on", st_collections_group_parts_part_description_physics_light_on},
   {"collections.group.parts.part.description.physics.z", st_collections_group_parts_part_description_physics_z},
   {"collections.group.parts.part.description.physics.depth", st_collections_group_parts_part_description_physics_depth},
   {"collections.group.parts.part.description.physics.backface_cull", st_collections_group_parts_part_description_physics_backface_cull},
   {"collections.group.parts.part.description.physics.faces.face.type", st_collections_group_parts_part_description_physics_face_type},
   {"collections.group.parts.part.description.physics.faces.face.source", st_collections_group_parts_part_description_physics_face_source},
#endif
   {"collections.group.parts.part.description.map.perspective", st_collections_group_parts_part_description_map_perspective},
   {"collections.group.parts.part.description.map.light", st_collections_group_parts_part_description_map_light},
   {"collections.group.parts.part.description.map.rotation.center", st_collections_group_parts_part_description_map_rotation_center},
   {"collections.group.parts.part.description.map.rotation.x", st_collections_group_parts_part_description_map_rotation_x},
   {"collections.group.parts.part.description.map.rotation.y", st_collections_group_parts_part_description_map_rotation_y},
   {"collections.group.parts.part.description.map.rotation.z", st_collections_group_parts_part_description_map_rotation_z},
   {"collections.group.parts.part.description.map.on", st_collections_group_parts_part_description_map_on},
   {"collections.group.parts.part.description.map.smooth", st_collections_group_parts_part_description_map_smooth},
   {"collections.group.parts.part.description.map.alpha", st_collections_group_parts_part_description_map_alpha},
   {"collections.group.parts.part.description.map.backface_cull", st_collections_group_parts_part_description_map_backface_cull},
   {"collections.group.parts.part.description.map.perspective_on", st_collections_group_parts_part_description_map_perspective_on},
   {"collections.group.parts.part.description.map.color", st_collections_group_parts_part_description_map_color},
   {"collections.group.parts.part.description.map.zoom.x", st_collections_group_parts_part_description_map_zoom_x},
   {"collections.group.parts.part.description.map.zoom.y", st_collections_group_parts_part_description_map_zoom_y},
   {"collections.group.parts.part.description.perspective.zplane", st_collections_group_parts_part_description_perspective_zplane},
   {"collections.group.parts.part.description.perspective.focal", st_collections_group_parts_part_description_perspective_focal},
   {"collections.group.parts.part.description.params.int", st_collections_group_parts_part_description_params_int},
   {"collections.group.parts.part.description.params.double", st_collections_group_parts_part_description_params_double},
   {"collections.group.parts.part.description.params.string", st_collections_group_parts_part_description_params_string},
   {"collections.group.parts.part.description.params.bool", st_collections_group_parts_part_description_params_bool},
   {"collections.group.parts.part.description.params.choice", st_collections_group_parts_part_description_params_choice},
   {"collections.group.parts.part.description.params.*", st_collections_group_parts_part_description_params_smart},
   IMAGE_STATEMENTS("collections.group.parts.part.description.")
   {
      "collections.group.parts.part.description.font", st_fonts_font
   },                                                                  /* dup */
   FONT_STYLE_CC_STATEMENTS("collections.group.parts.part.description.")
   TEXT_CLASS_STATEMENTS("collections.group.parts.part.description.")
   SIZE_CLASS_STATEMENTS("collections.group.parts.part.description.")
#ifdef HAVE_EPHYSICS
   {
      "collections.group.physics.world.gravity", st_collections_group_physics_world_gravity
   },
   {"collections.group.physics.world.rate", st_collections_group_physics_world_rate},
   {"collections.group.physics.world.z", st_collections_group_physics_world_z},
   {"collections.group.physics.world.depth", st_collections_group_physics_world_depth},
#endif
   FILTERS_STATEMENTS("collections.group.")   /* dup */
   PROGRAM_STATEMENTS("collections.group.parts.part.description")
   PROGRAM_STATEMENTS("collections.group.parts.part")
   PROGRAM_STATEMENTS("collections.group.parts")
   PROGRAM_STATEMENTS("collections.group")
};

/** @edcsection{lazedc,
 *              LazEDC} */

/** @edcsubsection{lazedc_intro,
 *                 LazEDC Intro} */

/**
    @page edcref
    @block
        LazEDC
    @context
        ..
        collections.group { "test";
           parts {
              rect { "clip"; }
              rect { "test"; nomouse; repeat; precise;
                 clip: "clip";
                 desc { "default";
                    color: 255 0 0 255;
                    rel2.relative: 0.5 1;
                 }
              }
              rect { "test2"; inherit: "test";
                 clip: "clip";
                 desc { "default";
                    rel1.relative: 0.5 0;
                    rel2.relative: 1 1;
                 }
                 desc { "t2";
                    inherit: "default";
                    color: 0 255 0 255;
                 }
                 desc { "t3";
                    inherit; // "default" can be omitted.
                    color: 0 0 255 255;
                 }
              }
              program { signal: "load"; name: "start";
                 sequence {
                    action: STATE_SET "t2";
                    target: "test2";
                    transition: LINEAR 0.6;
                    in: 0.5 0;
                    action: STATE_SET "t3";
                    target: "test2";
                    transition: LINEAR 0.3;
                    name: "del";
                 }
              }
           }
        }
        ..
    @description
        LazEDC is an advanced form of EDC which allows the developer to
        leave out or shorten various forms. Parts can be created by using
        their type names, and the "name" and "state" keywords can be omitted entirely.
        Additionally, default description blocks will be automatically created with default
        values even if the description isn't explicitly specified.
        @note Failing to use quotes for block names will trigger syntax errors
        if a block name is the same as an existing EDC keyword.
    @since 1.10
    @endblock
 */

/** @edcsubsection{lazedc_synonyms,
 *                 LazEDC Synonyms} */

/**
    @page edcref
    @block
        Synonyms
    @context
    group {
       parts {
          part {
             before -> insert_before
             after -> insert_after
             ignore -> ignore_flags
             mask -> mask_flags
             pointer -> pointer_mode
             alt_font -> use_alternate_font_metrics
             clip -> clip_to
             desc {
                clip -> clip_to
             }
          }
       }
    }
    color_class {
       desc -> description
       @since 1.14
    }

    @description
        These statements on the left are identical to their original keywords on the right.
    @since 1.10
    @endblock
 */

New_Statement_Handler statement_handlers_short[] =
{
   {"collections.group.parts.part.before", st_collections_group_parts_part_insert_before},
   {"collections.group.parts.part.after", st_collections_group_parts_part_insert_after},
   {"collections.group.parts.part.ignore", st_collections_group_parts_part_ignore_flags},
   {"collections.group.parts.part.mask", st_collections_group_parts_part_mask_flags},
   {"collections.group.parts.part.pointer", st_collections_group_parts_part_pointer_mode},
   {"collections.group.parts.part.alt_font", st_collections_group_parts_part_use_alternate_font_metrics},
   {"collections.group.parts.part.clip", st_collections_group_parts_part_clip_to_id},
   {"collections.group.parts.part.description.clip", st_collections_group_parts_part_description_clip_to_id},
};

/** @edcsubsection{lazedc_shorthand,
 *                 LazEDC Shorthand} */

/**
    @page edcref
    @block
        Shorthand
    @context
    group {
       broadcast; -> broadcast_signal: 1;
       nobroadcast; -> broadcast_signal: 0;
       mouse; -> mouse_events: 1;
       nomouse; -> mouse_events: 0;
       inherit_script; -> inherit_script: 1;
       noinherit_script; -> inherit_script: 0;
       parts {
          part {
             mouse; -> mouse_events: 1;
             nomouse; -> mouse_events: 0;
             repeat; -> repeat_events: 1;
             norepeat; -> repeat_events: 0;
             precise; -> precise_is_inside: 1;
             noprecise; -> precise_is_inside: 0;
             render; -> no_render: 0;
             norender; -> no_render: 1;
             required; -> required: 1;
             norequired; -> required: 0;
             scale; -> scale: 1;
             noscale; -> scale: 0;
             desc {
                vis; -> visible: 1;
                hid; -> visible: 0;
                offscale; -> offset_scale: 1;
             }
          }
       }
    }

    @description
        These statements on the left have the same meaning as statements on the right,
        but they are shorter.
    @since 1.10
    @endblock
 */
New_Statement_Handler statement_handlers_short_single[] =
{
   {"collections.group.parts.part.mouse", st_collections_group_parts_part_mouse},
   {"collections.group.parts.part.nomouse", st_collections_group_parts_part_nomouse},
   {"collections.group.parts.part.repeat", st_collections_group_parts_part_repeat},
   {"collections.group.parts.part.norepeat", st_collections_group_parts_part_norepeat},
   {"collections.group.parts.part.precise", st_collections_group_parts_part_precise},
   {"collections.group.parts.part.noprecise", st_collections_group_parts_part_noprecise},
   {"collections.group.parts.part.scale", st_collections_group_parts_part_scale},
   {"collections.group.parts.part.noscale", st_collections_group_parts_part_noscale},
   {"collections.group.parts.part.render", st_collections_group_parts_part_render},
   {"collections.group.parts.part.norender", st_collections_group_parts_part_no_render},
   {"collections.group.parts.part.required", st_collections_group_parts_part_required},
   {"collections.group.parts.part.norequired", st_collections_group_parts_part_norequired},
   {"collections.group.parts.part.description.vis", st_collections_group_parts_part_description_vis},
   {"collections.group.parts.part.description.hid", st_collections_group_parts_part_description_hid},
   {"collections.group.parts.part.description.offscale", st_collections_group_parts_part_description_offset_scale},
   {"collections.group.mouse", st_collections_group_mouse},
   {"collections.group.nomouse", st_collections_group_nomouse},
   {"collections.group.broadcast", st_collections_group_broadcast},
   {"collections.group.nobroadcast", st_collections_group_nobroadcast},
   {"collections.group.inherit_script", st_collections_group_inherit_script},
   {"collections.group.noinherit_script", st_collections_group_noinherit_script},
   {"collections.group.parts.part.description.inherit", st_collections_group_parts_part_description_inherit},
};

/** @edcsubsection{lazedc_external_params,
 *                 LazEDC Group.Parts.External.Desc.Params} */

/**
    @page edcref
    @block
       params
    @context
       ...
       external {
          desc { "default";
             params {
                number: 10;       -> int: "number" 10;
                number2: 1.1;     -> double: "number2" 1.1;
                label: "OK";      -> string: "label" "OK";
                check: true;      -> bool: "check" 1;
                check2: false;    -> bool: "check2" 0;
                text_wrap: mixed; -> choice: "text_wrap" "mixed";
             }
          }
       }
       ...
    @description
       The name of parameter can be used as a statement keyword in params block.
       The type of parameter is determined automatically by the value,
       so it should follow the next rules.
       Number without decimal point is considered as an integer.
       Number with decimal point is considered as a double.
       Double-quoted string is considered as a string.
       'true' or 'false' without quotes is considred as a boolean.
       String without quotes except for 'true' or 'false' is considered as a choice.
    @since 1.18
    @endblock
 */
static Edje_External_Param_Type
_parse_external_param_type(char *token)
{
   Eina_Bool num, point;
   char *s;

   if (param_had_quote(0))
     return EDJE_EXTERNAL_PARAM_TYPE_STRING;

   num = EINA_TRUE;
   point = EINA_FALSE;
   s = token;

   while (*s)
     {
        if ((*s < '0') || (*s > '9'))
          {
             if ((!point) && (*s == '.'))
               {
                  point = EINA_TRUE;
               }
             else
               {
                  num = EINA_FALSE;
                  break;
               }
          }
        s++;
     }

   if (num)
     {
        if (!point)
          return EDJE_EXTERNAL_PARAM_TYPE_INT;
        else
          return EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
     }
   else
     {
        if (!strcmp(token, "true") || !strcmp(token, "false"))
          return EDJE_EXTERNAL_PARAM_TYPE_BOOL;
        else
          return EDJE_EXTERNAL_PARAM_TYPE_CHOICE;
     }
}

static void
st_collections_group_parts_part_description_params_smart(void)
{
   Edje_Part_Description_External *ed;
   Edje_External_Param *param;
   Eina_List *l;
   char *last, *name, *token;
   int found = 0;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        ERR("parse error %s:%i. params in non-EXTERNAL part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_External *)current_desc;

   last = eina_list_last_data_get(stack);
   if (!strncmp(last, "params.", strlen("params.")))
     name = strdup(last + strlen("params."));
   else
     name = strdup(last);

   /* if a param with this name already exists, overwrite it */
   EINA_LIST_FOREACH(ed->external_params, l, param)
     {
        if (!strcmp(param->name, name))
          {
             found = 1;
             free(name);
             break;
          }
     }

   if (!found)
     {
        param = mem_alloc(SZ(Edje_External_Param));
        param->name = name;
     }

   token = parse_str(0);

   param->type = _parse_external_param_type(token);
   param->i = 0;
   param->d = 0;
   param->s = NULL;

   switch (param->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        if (!strcmp(token, "true"))
          param->i = 1;
        else if (!strcmp(token, "false"))
          param->i = 0;
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_INT:
        param->i = parse_int(0);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        param->d = parse_float(0);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        param->s = parse_str(0);
        break;

      default:
        ERR("parse error %s:%i. Invalid param type.",
            file_in, line - 1);
        break;
     }

   if (!found)
     ed->external_params = eina_list_append(ed->external_params, param);

   free(token);
}

#define PROGRAM_OBJECTS(PREFIX)                                                                           \
  {PREFIX ".program", ob_collections_group_programs_program},   /* dup */                                 \
  {PREFIX ".program.script", ob_collections_group_programs_program_script},   /* dup */                   \
  {PREFIX ".program.sequence.script", ob_collections_group_programs_program_script},   /* dup */          \
  {PREFIX ".program.sequence", ob_collections_group_programs_program_sequence},   /* dup */               \
  {PREFIX ".programs", NULL},   /* dup */                                                                 \
  {PREFIX ".programs.set", ob_images_set},   /* dup */                                                    \
  {PREFIX ".programs.set.image", ob_images_set_image},   /* dup */                                        \
  {PREFIX ".programs.images", NULL},   /* dup */                                                          \
  {PREFIX ".programs.images.set", ob_images_set},   /* dup */                                             \
  {PREFIX ".programs.images.set.image", ob_images_set_image},   /* dup */                                 \
  {PREFIX ".programs.fonts", NULL},   /* dup */                                                           \
  {PREFIX ".programs.program", ob_collections_group_programs_program},   /* dup */                        \
  {PREFIX ".programs.program.script", ob_collections_group_programs_program_script},   /* dup */          \
  {PREFIX ".programs.program.sequence", ob_collections_group_programs_program_sequence},   /* dup */      \
  {PREFIX ".programs.program.sequence.script", ob_collections_group_programs_program_script},   /* dup */ \
  {PREFIX ".programs.script", ob_collections_group_script},   /* dup */                                   \
  {PREFIX ".script", ob_collections_group_script},   /* dup */

New_Object_Handler object_handlers[] =
{
   {"externals", NULL},
   {"images", NULL},
   {"images.set", ob_images_set},
   {"images.set.image", ob_images_set_image},
   {"fonts", NULL},
   {"data", NULL},
   {"styles", NULL},
   {"styles.style", ob_styles_style},
   {"color_tree", ob_color_tree},
   {"color_classes", NULL},
   {"color_classes.color_class", ob_color_class},
   {"text_classes", NULL},
   {"text_classes.text_class", ob_text_class},
   {"size_classes", NULL},
   {"size_classes.size_class", ob_size_class},
   {"spectra", NULL},
   {"filters", NULL},
   {"filters.filter", ob_filters_filter},
   {"filters.filter.script", ob_filters_filter_script},
   {"collections", ob_collections},
   {"collections.externals", NULL},   /* dup */
   {"collections.set", ob_images_set},   /* dup */
   {"collections.set.image", ob_images_set_image},   /* dup */
   {"collections.images", NULL},   /* dup */
   {"collections.images.set", ob_images_set},   /* dup */
   {"collections.images.set.image", ob_images_set_image},   /* dup */
   {"collections.fonts", NULL},   /* dup */
   {"collections.styles", NULL},   /* dup */
   {"collections.styles.style", ob_styles_style},   /* dup */
   {"collections.color_tree", ob_color_tree},   /* dup */
   {"collections.color_classes", NULL},   /* dup */
   {"collections.color_classes.color_class", ob_color_class},   /* dup */
   {"collections.text_classes", NULL},
   {"collections.text_classes.text_class", ob_text_class},   /* dup */
   {"collections.size_classes", NULL},   /* dup */
   {"collections.size_classes.size_class", ob_size_class},   /* dup */
   {"collections.sounds", NULL},
   {"collections.group.sounds", NULL},   /* dup */
   {"collections.sounds.sample", NULL},
   {"collections.translation", NULL},
   {"collections.translation.file", NULL},
   {"collections.group.translation", NULL},  /*dup*/
   {"collections.group.translation.file", NULL},  /*dup*/
   {"collections.group.sounds.sample", NULL},   /* dup */
   {"collections.vibrations", NULL},
   {"collections.group.vibrations", NULL},   /* dup */
   {"collections.vibrations.sample", NULL},
   {"collections.filters", NULL},
   {"collections.filters.filter", ob_filters_filter},   /* dup */
   {"collections.filters.filter.script", ob_filters_filter_script},   /* dup */
   {"collections.group.vibrations.sample", NULL},   /* dup */
   {"collections.group", ob_collections_group},
   {"collections.group.data", NULL},
   {"collections.group.limits", NULL},
   {"collections.group.script", ob_collections_group_script},
   {"collections.group.lua_script", ob_collections_group_lua_script},
   {"collections.group.externals", NULL},   /* dup */
   {"collections.group.set", ob_images_set},   /* dup */
   {"collections.group.set.image", ob_images_set_image},   /* dup */
   {"collections.group.images", NULL},   /* dup */
   {"collections.group.models", NULL},   /* dup */
   {"collections.group.images.set", ob_images_set},   /* dup */
   {"collections.group.images.set.image", ob_images_set_image},   /* dup */
   {"collections.group.fonts", NULL},   /* dup */
   {"collections.group.styles", NULL},   /* dup */
   {"collections.group.styles.style", ob_styles_style},   /* dup */
   {"collections.group.color_tree", ob_color_tree},   /* dup */
   {"collections.group.color_classes", NULL},   /* dup */
   {"collections.group.color_classes.color_class", ob_color_class},   /* dup */
   {"collections.group.text_classes", NULL},
   {"collections.group.text_classes.text_class", ob_text_class},   /* dup */
   {"collections.group.size_classes", NULL},   /* dup */
   {"collections.group.size_classes.size_class", ob_size_class},   /* dup */
   {"collections.group.filters", NULL},
   {"collections.group.filters.filter", ob_filters_filter},   /* dup */
   {"collections.group.filters.filter.script", ob_filters_filter_script},   /* dup */
   {"collections.group.parts", NULL},
   {"collections.group.parts.set", ob_images_set},   /* dup */
   {"collections.group.parts.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.images", NULL},   /* dup */
   {"collections.group.parts.images.set", ob_images_set},   /* dup */
   {"collections.group.parts.images.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.fonts", NULL},   /* dup */
   {"collections.group.parts.styles", NULL},   /* dup */
   {"collections.group.parts.styles.style", ob_styles_style},   /* dup */
   {"collections.group.parts.color_classes", NULL},   /* dup */
   {"collections.group.parts.color_classes.color_class", ob_color_class},   /* dup */
   {"collections.group.parts.text_classes", NULL},
   {"collections.group.parts.text_classes.text_class", ob_text_class},   /* dup */
   {"collections.group.parts.size_classes", NULL},   /* dup */
   {"collections.group.parts.size_classes.size_class", ob_size_class},   /* dup */
   {"collections.group.parts.part", ob_collections_group_parts_part},
   {"collections.group.parts.part.dragable", NULL},
   {"collections.group.parts.part.set", ob_images_set},   /* dup */
   {"collections.group.parts.part.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.part.images", NULL},   /* dup */
   {"collections.group.parts.part.images.set", ob_images_set},   /* dup */
   {"collections.group.parts.part.images.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.part.fonts", NULL},   /* dup */
   {"collections.group.parts.part.styles", NULL},   /* dup */
   {"collections.group.parts.part.styles.style", ob_styles_style},   /* dup */
   {"collections.group.parts.part.color_classes", NULL},   /* dup */
   {"collections.group.parts.part.color_classes.color_class", ob_color_class},   /* dup */
   {"collections.group.parts.part.text_classes", NULL},
   {"collections.group.parts.part.text_classes.text_class", ob_text_class},   /* dup */
   {"collections.group.parts.part.size_classes", NULL},   /* dup */
   {"collections.group.parts.part.size_classes.size_class", ob_size_class},   /* dup */
   {"collections.group.parts.part.box", NULL},
   {"collections.group.parts.part.box.items", NULL},
   {"collections.group.parts.part.box.items.item", ob_collections_group_parts_part_box_items_item},
   {"collections.group.parts.part.table", NULL},
   {"collections.group.parts.part.table.items", NULL},
   {"collections.group.parts.part.table.items.item", ob_collections_group_parts_part_box_items_item},   /* dup */
   {"collections.group.parts.part.description", ob_collections_group_parts_part_description},
   {"collections.group.parts.part.description.link", ob_collections_group_parts_part_description_link},
   {"collections.group.parts.part.description.rel1", NULL},
   {"collections.group.parts.part.description.rel2", NULL},
   {"collections.group.parts.part.description.anchors", NULL},
   {"collections.group.parts.part.description.image", NULL},   /* dup */
   {"collections.group.parts.part.description.image.set", ob_images_set},   /* dup */
   {"collections.group.parts.part.description.image.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.part.description.image.images", NULL},   /* dup */
   {"collections.group.parts.part.description.image.images.set", ob_images_set},   /* dup */
   {"collections.group.parts.part.description.image.images.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.part.description.fill", NULL},
   {"collections.group.parts.part.description.fill.origin", NULL},
   {"collections.group.parts.part.description.fill.size", NULL},
   {"collections.group.parts.part.description.text", NULL},
   {"collections.group.parts.part.description.text.fonts", NULL},   /* dup */
   {"collections.group.parts.part.description.images", NULL},   /* dup */
   {"collections.group.parts.part.description.images.set", ob_images_set},   /* dup */
   {"collections.group.parts.part.description.images.set.image", ob_images_set_image},   /* dup */
   {"collections.group.parts.part.description.fonts", NULL},   /* dup */
   {"collections.group.parts.part.description.styles", NULL},   /* dup */
   {"collections.group.parts.part.description.styles.style", ob_styles_style},   /* dup */
   {"collections.group.parts.part.description.box", NULL},
   {"collections.group.parts.part.description.table", NULL},
   {"collections.group.parts.part.description.position", NULL},
   {"collections.group.parts.part.description.properties", NULL},
   {"collections.group.parts.part.description.orientation", NULL},
   {"collections.group.parts.part.description.texture", ob_collections_group_parts_part_description_texture},
   {"collections.group.parts.part.description.mesh", NULL},
   {"collections.group.parts.part.description.filter", NULL},
   {"collections.group.parts.part.description.proxy", NULL},
#ifdef HAVE_EPHYSICS
   {"collections.group.parts.part.description.physics", NULL},
   {"collections.group.parts.part.description.physics.movement_freedom", NULL},
   {"collections.group.parts.part.description.physics.faces", NULL},
   {"collections.group.parts.part.description.physics.faces.face", st_collections_group_parts_part_description_physics_face},
#endif
   {"collections.group.parts.part.description.map", NULL},
   {"collections.group.parts.part.description.map.rotation", NULL},
   {"collections.group.parts.part.description.map.zoom", NULL},
   {"collections.group.parts.part.description.perspective", NULL},
   {"collections.group.parts.part.description.params", NULL},
   {"collections.group.parts.part.description.color_classes", NULL},   /* dup */
   {"collections.group.parts.part.description.color_classes.color_class", ob_color_class},   /* dup */
   {"collections.group.parts.part.description.text_classes", NULL},   /* dup */
   {"collections.group.parts.part.description.text_classes.text_class", ob_text_class},   /* dup */
   {"collections.group.parts.part.description.size_classes", NULL},   /* dup */
   {"collections.group.parts.part.description.size_classes.size_class", ob_size_class},   /* dup */
#ifdef HAVE_EPHYSICS
   {"collections.group.physics", NULL},
   {"collections.group.physics.world", NULL},
#endif
   PROGRAM_OBJECTS("collections.group.parts.part.description")
   PROGRAM_OBJECTS("collections.group.parts.part")
   PROGRAM_OBJECTS("collections.group.parts")
   PROGRAM_OBJECTS("collections.group")
};

/** @edcsubsection{lazedc_blocks,
 *                 LazEDC Blocks} */

/**
    @page edcref
    @block
        Blocks
    @context
    parts {
       rect{}
       text{}
       image{}
       swallow{}
       textblock{}
       group{}
       box{}
       table{}
       external{}
       proxy{}
       spacer{}
       snapshot{}
       vector{}
       part {
          desc {
          }
       }
    }

    @description
        Lowercase part types can be specified as blocks with the same effect as part { type: TYPE; }
        The "description" block can also be shortened to "desc".

    @since 1.10
    @endblock
 */
New_Object_Handler object_handlers_short[] =
{
   {"collections.group.parts.rect", ob_collections_group_parts_part_short},
   {"collections.group.parts.text", ob_collections_group_parts_part_short},
   {"collections.group.parts.image", ob_collections_group_parts_part_short},
   {"collections.group.parts.swallow", ob_collections_group_parts_part_short},
   {"collections.group.parts.textblock", ob_collections_group_parts_part_short},
   {"collections.group.parts.group", ob_collections_group_parts_part_short},
   {"collections.group.parts.box", ob_collections_group_parts_part_short},
   {"collections.group.parts.table", ob_collections_group_parts_part_short},
   {"collections.group.parts.external", ob_collections_group_parts_part_short},
   {"collections.group.parts.proxy", ob_collections_group_parts_part_short},
   {"collections.group.parts.spacer", ob_collections_group_parts_part_short},
   {"collections.group.parts.snapshot", ob_collections_group_parts_part_short},
   {"collections.group.parts.part.desc", ob_collections_group_parts_part_desc},
   {"collections.group.parts.vector", ob_collections_group_parts_part_short},
};

New_Nested_Handler nested_handlers[] = {
   {"collections.group.parts", "part", NULL, edje_cc_handlers_hierarchy_pop }
};

New_Nested_Handler nested_handlers_short[] = {
   {"collections.group.parts", "rect", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "text", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "image", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "swallow", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "textblock", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "group", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "box", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "table", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "external", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "proxy", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "spacer", NULL, edje_cc_handlers_hierarchy_pop },
   {"collections.group.parts", "vector", NULL, edje_cc_handlers_hierarchy_pop },
};

/*****/

int
object_handler_num(void)
{
   return sizeof(object_handlers) / sizeof (New_Object_Handler);
}

int
object_handler_short_num(void)
{
   return sizeof(object_handlers_short) / sizeof (New_Object_Handler);
}

int
statement_handler_num(void)
{
   return sizeof(statement_handlers) / sizeof (New_Object_Handler);
}

int
statement_handler_short_num(void)
{
   return sizeof(statement_handlers_short) / sizeof (New_Object_Handler);
}

int
statement_handler_short_single_num(void)
{
   return sizeof(statement_handlers_short_single) / sizeof (New_Object_Handler);
}

int
nested_handler_num(void)
{
   return sizeof(nested_handlers) / sizeof (New_Nested_Handler);
}

int
nested_handler_short_num(void)
{
   return sizeof(nested_handlers_short) / sizeof (New_Nested_Handler);
}

static void
_edje_part_description_fill(Edje_Part_Description_Spec_Fill *fill)
{
   fill->smooth = 1;
   fill->pos_rel_x = FROM_DOUBLE(0.0);
   fill->pos_abs_x = 0;
   fill->rel_x = FROM_DOUBLE(1.0);
   fill->abs_x = 0;
   fill->pos_rel_y = FROM_DOUBLE(0.0);
   fill->pos_abs_y = 0;
   fill->rel_y = FROM_DOUBLE(1.0);
   fill->abs_y = 0;
   fill->type = EDJE_FILL_TYPE_SCALE;
}

static void
_edje_part_description_image_remove(Edje_Part_Description_Image *ed)
{
   unsigned int j;

   if (!ed) return;

   data_queue_image_remove(&(ed->image.id), &(ed->image.set));

   for (j = 0; j < ed->image.tweens_count; ++j)
     data_queue_image_remove(&(ed->image.tweens[j]->id),
                             &(ed->image.tweens[j]->set));
}

void
part_description_image_cleanup(Edje_Part *ep)
{
   Edje_Part_Description_Image *ed;
   unsigned int j;

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     return;

   ed = (Edje_Part_Description_Image *)ep->default_desc;
   _edje_part_description_image_remove(ed);

   for (j = 0; j < ep->other.desc_count; j++)
     {
        ed = (Edje_Part_Description_Image *)ep->other.desc[j];
        _edje_part_description_image_remove(ed);
     }
}

static Edje_Part_Description_Common *
_edje_part_description_alloc(unsigned char type, const char *collection, const char *part)
{
   Edje_Part_Description_Common *result = NULL;

   switch (type)
     {
      case EDJE_PART_TYPE_SPACER:
      case EDJE_PART_TYPE_RECTANGLE:
      case EDJE_PART_TYPE_SWALLOW:
      case EDJE_PART_TYPE_GROUP:
        result = mem_alloc(SZ(Edje_Part_Description_Common));
        break;

      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
      {
         Edje_Part_Description_Text *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Text));

         ed->text.color3.r = 0;
         ed->text.color3.g = 0;
         ed->text.color3.b = 0;
         ed->text.color3.a = 128;
         ed->text.align.x = FROM_DOUBLE(0.5);
         ed->text.align.y = FROM_DOUBLE(0.5);
         ed->text.id_source = -1;
         ed->text.id_text_source = -1;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Image));

         ed->image.id = -1;

         _edje_part_description_fill(&ed->image.fill);

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_SNAPSHOT:
      {
         Edje_Part_Description_Snapshot *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Snapshot));

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Proxy));

         ed->proxy.id = -1;
         ed->proxy.source_visible = EINA_TRUE;
         ed->proxy.source_clip = EINA_TRUE;
         _edje_part_description_fill(&ed->proxy.fill);

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Box));

         ed->box.layout = NULL;
         ed->box.alt_layout = NULL;
         ed->box.align.x = FROM_DOUBLE(0.5);
         ed->box.align.y = FROM_DOUBLE(0.5);
         ed->box.padding.x = 0;
         ed->box.padding.y = 0;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Table));

         ed->table.homogeneous = EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE;
         ed->table.align.x = FROM_DOUBLE(0.5);
         ed->table.align.y = FROM_DOUBLE(0.5);
         ed->table.padding.x = 0;
         ed->table.padding.y = 0;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_EXTERNAL:
      {
         Edje_Part_Description_External *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_External));

         ed->external_params = NULL;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Mesh_Node));

         ed->mesh_node.mesh.id = -1;
         ed->mesh_node.mesh.primitive = 0;
         ed->mesh_node.mesh.assembly = 1;
         ed->mesh_node.mesh.frame = 0;

         ed->mesh_node.texture.id = -1;
         ed->mesh_node.texture.wrap1 = 0;
         ed->mesh_node.texture.wrap2 = 0;
         ed->mesh_node.texture.filter1 = 0;
         ed->mesh_node.texture.filter2 = 0;

         ed->mesh_node.properties.shade = EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR;
         ed->mesh_node.properties.ambient.r = 50;
         ed->mesh_node.properties.ambient.g = 50;
         ed->mesh_node.properties.ambient.b = 50;
         ed->mesh_node.properties.ambient.a = 255;
         ed->mesh_node.properties.diffuse.r = 255;
         ed->mesh_node.properties.diffuse.g = 255;
         ed->mesh_node.properties.diffuse.b = 255;
         ed->mesh_node.properties.diffuse.a = 255;
         ed->mesh_node.properties.specular.r = 255;
         ed->mesh_node.properties.specular.g = 255;
         ed->mesh_node.properties.specular.b = 255;
         ed->mesh_node.properties.specular.a = 255;

         ed->mesh_node.properties.material_attrib = 1;
         ed->mesh_node.properties.normal = 1;
         ed->mesh_node.properties.shininess = 50;

         ed->mesh_node.aabb1.relative.x = -1.0;
         ed->mesh_node.aabb1.relative.y = -1.0;
         ed->mesh_node.aabb1.relative.z = -1.0;
         ed->mesh_node.aabb1.offset.x = 0;
         ed->mesh_node.aabb1.offset.y = 0;
         ed->mesh_node.aabb1.offset.z = 0;
         ed->mesh_node.aabb1.rel_to = -1;
         ed->mesh_node.aabb2.relative.x = 1.0;
         ed->mesh_node.aabb2.relative.y = 1.0;
         ed->mesh_node.aabb2.relative.z = 1.0;
         ed->mesh_node.aabb2.offset.x = 0;
         ed->mesh_node.aabb2.offset.y = 0;
         ed->mesh_node.aabb2.offset.z = 0;
         ed->mesh_node.aabb2.rel_to = -1;

         ed->mesh_node.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_NONE;
         /* x1 is angle for angle_axis and cosine of half angle for quternion,
            x2, x3, x4 define axis for angle_axis and quaternion,
            x1, x2, x3 are coordinates of point to look at for look_at,
            x4, x5, x6 define a vector that indicates the angle at which
            the subject is looking at the target for look_at and look_to */
         ed->mesh_node.orientation.data[0] = 1.0;
         ed->mesh_node.orientation.data[1] = 0.0;
         ed->mesh_node.orientation.data[2] = 0.0;
         ed->mesh_node.orientation.data[3] = 0.0;
         ed->mesh_node.orientation.data[4] = 1.0;
         ed->mesh_node.orientation.data[5] = 0.0;
         ed->mesh_node.orientation.look_to = -1;

         ed->mesh_node.scale_3d.x = 1.0;
         ed->mesh_node.scale_3d.y = 1.0;
         ed->mesh_node.scale_3d.z = 1.0;

         ed->mesh_node.position.point.x = 0.0;
         ed->mesh_node.position.point.y = 0.0;
         ed->mesh_node.position.point.z = 0.0;
         ed->mesh_node.position.space = EVAS_CANVAS3D_SPACE_PARENT;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Light));

         ed->light.properties.ambient.r = 50;
         ed->light.properties.ambient.g = 50;
         ed->light.properties.ambient.b = 50;
         ed->light.properties.ambient.a = 255;
         ed->light.properties.diffuse.r = 255;
         ed->light.properties.diffuse.g = 255;
         ed->light.properties.diffuse.b = 255;
         ed->light.properties.diffuse.a = 255;
         ed->light.properties.specular.r = 255;
         ed->light.properties.specular.g = 255;
         ed->light.properties.specular.b = 255;
         ed->light.properties.specular.a = 255;

         ed->light.position.point.x = 0.0;
         ed->light.position.point.y = 0.0;
         ed->light.position.point.z = 1.0;
         ed->light.position.space = EVAS_CANVAS3D_SPACE_PARENT;

         ed->light.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_NONE;
         /* x1 is angle for angle_axis and cosine of half angle for quternion,
            x2, x3, x4 define axis for angle_axis and quaternion,
            x1, x2, x3 are coordinates of point to look at for look_at,
            x4, x5, x6 define a vector that indicates the angle at which
            the subject is looking at the target for look_at and look_to */
         ed->light.orientation.data[0] = 1.0;
         ed->light.orientation.data[1] = 0.0;
         ed->light.orientation.data[2] = 0.0;
         ed->light.orientation.data[3] = 0.0;
         ed->light.orientation.data[4] = 1.0;
         ed->light.orientation.data[5] = 0.0;
         ed->light.orientation.look_to = -1;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_CAMERA:
      {
         Edje_Part_Description_Camera *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Camera));

         ed->camera.camera.fovy = 60.0;
         ed->camera.camera.aspect = 1.0;
         ed->camera.camera.frustum_near = 2.0;
         ed->camera.camera.frustum_far = 50.0;

         ed->camera.position.point.x = 0.0;
         ed->camera.position.point.y = 0.0;
         ed->camera.position.point.z = 5.0;

         ed->camera.position.space = EVAS_CANVAS3D_SPACE_PARENT;

         ed->camera.orientation.type = EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_NONE;
         /* x1 is angle for angle_axis and cosine of half angle for quternion,
            x2, x3, x4 define axis for angle_axis and quaternion,
            x1, x2, x3 are coordinates of point to look at for look_at,
            x4, x5, x6 define a vector that indicates the angle at which
            the subject is looking at the target for look_at and look_to */
         ed->camera.orientation.data[0] = 1.0;
         ed->camera.orientation.data[1] = 0.0;
         ed->camera.orientation.data[2] = 0.0;
         ed->camera.orientation.data[3] = 0.0;
         ed->camera.orientation.data[4] = 1.0;
         ed->camera.orientation.data[5] = 0.0;
         ed->camera.orientation.look_to = -1;

         result = &ed->common;
         break;
      }

      case EDJE_PART_TYPE_VECTOR:
      {
         Edje_Part_Description_Vector *ed;

         ed = mem_alloc(SZ(Edje_Part_Description_Vector));

         result = &ed->common;
         break;
      }
     }

   if (!result)
     {
        ERR("Unknown type %i of part %s in collection %s.",
            type, part, collection);
        exit(-1);
     }

#ifdef HAVE_EPHYSICS
   result->physics.mass = FROM_DOUBLE(1.0);
   result->physics.friction = FROM_DOUBLE(0.5);
   result->physics.sleep.linear = FROM_DOUBLE(24);
   result->physics.sleep.angular = FROM_DOUBLE(57.29);
   result->physics.hardness = FROM_DOUBLE(1.0);
   result->physics.ignore_part_pos = 1;
   result->physics.mov_freedom.lin.x = 1;
   result->physics.mov_freedom.lin.y = 1;
   result->physics.mov_freedom.ang.z = 1;
   result->physics.z = -15;
   result->physics.depth = 30;
#endif

   result->clip_to_id = -1;
   return result;
}

static void
_edje_program_check(const char *name, Edje_Program *me, Edje_Program **pgrms, unsigned int count)
{
   Edje_Part_Collection *pc;
   unsigned int i;
   Edje_Program_Parser *epp;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   for (i = 0; i < count; ++i)
     if (pgrms[i]->name)
       if (pgrms[i] != me && (!strcmp(name, pgrms[i]->name)))
         {
            epp = (Edje_Program_Parser *)pgrms[i];
            if (!epp->can_override)
              {
                 ERR("parse error %s:%i. There is already a program of the name %s",
                     file_in, line - 1, name);
                 exit(-1);
              }
            else
              {
                 _edje_program_remove(pc, me);
                 current_program = pgrms[i];
                 if (pgrms[i]->action == EDJE_ACTION_TYPE_SCRIPT)
                   copied_program_anonymous_lookup_delete(pc, &pgrms[i]->id);
                 epp->can_override = EINA_FALSE;
                 return;
              }
         }
}

static void
_edje_program_copy(Edje_Program *ep, Edje_Program *ep2)
{
   Edje_Part_Collection *pc;
   Edje_Program_Target *et, *et2;
   Edje_Program_After *pa, *pa2;
   Edje_Program_Parser *epp;
   Eina_List *l;
   char *name;
   char *copy;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   ep->name = STRDUP(ep2->name);

   _edje_program_remove(pc, current_program);
   ep->signal = STRDUP(ep2->signal);
   ep->source = STRDUP(ep2->source);
   _edje_program_insert(pc, current_program);

   ep->filter.part = STRDUP(ep2->filter.part);
   ep->filter.state = STRDUP(ep2->filter.state);
   ep->in.from = ep2->in.from;
   ep->in.range = ep2->in.range;
   ep->action = ep2->action;
   ep->seat = STRDUP(ep2->seat);
   ep->state = STRDUP(ep2->state);
   ep->state2 = STRDUP(ep2->state2);
   ep->value = ep2->value;
   ep->value2 = ep2->value2;
   ep->tween.mode = ep2->tween.mode;
   ep->tween.time = ep2->tween.time;
   ep->tween.v1 = ep2->tween.v1;
   ep->tween.v2 = ep2->tween.v2;
   ep->tween.v3 = ep2->tween.v3;
   ep->tween.v4 = ep2->tween.v4;
   ep->tween.use_duration_factor = ep2->tween.use_duration_factor;
   ep->sample_name = STRDUP(ep2->sample_name);
   ep->tone_name = STRDUP(ep2->tone_name);
   ep->duration = ep2->duration;
   ep->speed = ep2->speed;
   ep->channel = ep2->channel;

   EINA_LIST_FOREACH(ep2->targets, l, et2)
     {
        name = (char *)(et2 + 1);
        et = mem_alloc(SZ(Edje_Program_Target) + strlen(name) + 1);
        ep->targets = eina_list_append(ep->targets, et);
        copy = (char *)(et + 1);

        memcpy(copy, name, strlen(name) + 1);

        switch (ep2->action)
          {
           case EDJE_ACTION_TYPE_STATE_SET:
           case EDJE_ACTION_TYPE_SIGNAL_EMIT:
           case EDJE_ACTION_TYPE_DRAG_VAL_SET:
           case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
           case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
           case EDJE_ACTION_TYPE_FOCUS_SET:
           case EDJE_ACTION_TYPE_FOCUS_OBJECT:
             if (current_group_inherit)
               data_queue_part_lookup(pc, name, &et->id);
             else
               data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
             break;

           case EDJE_ACTION_TYPE_ACTION_STOP:
           case EDJE_ACTION_TYPE_SCRIPT:
             if (current_group_inherit)
               data_queue_program_lookup(pc, name, &et->id);
             else
               data_queue_copied_program_lookup(pc, &(et2->id), &(et->id));
             break;

           default:
             ERR("parse error %s:%i. target may only be used after action",
                 file_in, line - 1);
             exit(-1);
          }
     }

   EINA_LIST_FOREACH(ep2->after, l, pa2)
     {
        name = (char *)(pa2 + 1);
        pa = mem_alloc(SZ(Edje_Program_After) + strlen(name) + 1);
        ep->after = eina_list_append(ep->after, pa);
        copy = (char *)(pa + 1);
        memcpy(copy, name, strlen(name) + 1);
        if (!data_queue_copied_program_lookup(pc, &(pa2->id), &(pa->id)))
          data_queue_program_lookup(pc, copy, &(pa->id));
     }

   ep->api.name = STRDUP(ep2->api.name);
   ep->api.description = STRDUP(ep2->api.description);
   data_queue_copied_part_lookup(pc, &(ep2->param.src), &(ep->param.src));
   data_queue_copied_part_lookup(pc, &(ep2->param.dst), &(ep->param.dst));

   epp = (Edje_Program_Parser *)ep;
   epp->can_override = EINA_TRUE;
}

/*****/

/** @edcsection{toplevel,Top-Level blocks} */

/** @edcsubsection{toplevel_efl_version,
 *                 Efl_version} */

/**
    @page edcref

    @property
        efl_version
    @parameters
        [major] [minor]
    @effect
        Used to show which version of EFL is used for developing a edje file.
    @endproperty
 */
static void
st_efl_version(void)
{
   check_arg_count(2);

   edje_file->efl_version.major = parse_int(0);
   edje_file->efl_version.minor = parse_int(1);
}

/** @edcsubsection{toplevel_id,
 *                 id} */

/**
    @page edcref

    @property
        id
    @parameters
        [name]
    @effect
        A string which is used to identify the edje file.
    @since 1.21
    @endproperty
 */
static void
st_id(void)
{
   Eina_Array_Iterator it;
   unsigned int i;
   char *str, *id;

   check_arg_count(1);
   id = parse_str(0);

   EINA_ARRAY_ITER_NEXT(requires, i, str, it)
     if (eina_streq(str, id))
       error_and_abort(NULL, "Cannot use same id for file as one of its required files!");
   free((void*)edje_file->id);
   edje_file->id = id;
}

/** @edcsubsection{toplevel_requires,
 *                 requires} */

/**
    @page edcref

    @property
        requires
    @parameters
        [name]
    @effect
        Specifying this property informs edje not to close the
        file with the corresponding id property for as long as this
        file is open. Multiple requires properties can be individually specified.
    @since 1.21
    @endproperty
 */
static void
st_requires(void)
{
   char *str;
   check_arg_count(1);

   str = parse_str(0);
   if (eina_streq(str, edje_file->id))
     error_and_abort(NULL, "Cannot require the current file!");
   eina_array_push(requires, str);
}

/** @edcsubsection{toplevel_externals,
 *                 Externals} */

/**
    @page edcref

    @block
        externals
    @context
        externals {
           external: "name";
        }
    @description
        The "externals" block is used to list each external module file that will be used in others
        programs.
    @endblock

    @property
        external
    @parameters
        [external filename]
    @effect
        Used to add a file to the externals list.
    @endproperty
 */
static void
st_externals_external(void)
{
   External *ex;

   check_arg_count(1);

   if (!edje_file->external_dir)
     edje_file->external_dir = mem_alloc(SZ(Edje_External_Directory));

   ex = mem_alloc(SZ(External));
   ex->name = parse_str(0);
   {
      Eina_List *l;
      External *lex;

      EINA_LIST_FOREACH(externals, l, lex)
        {
           if (!strcmp(lex->name, ex->name))
             {
                free(ex->name);
                free(ex);
                return;
             }
        }
   }
   externals = eina_list_append(externals, ex);

   if (edje_file->external_dir)
     {
        Edje_External_Directory_Entry *entries;

        edje_file->external_dir->entries_count++;
        entries = realloc(edje_file->external_dir->entries,
                          sizeof (Edje_External_Directory_Entry) * edje_file->external_dir->entries_count);
        if (!entries)
          {
             ERR("not enough memory");
             exit(-1);
          }
        edje_file->external_dir->entries = entries;
        memset(edje_file->external_dir->entries + edje_file->external_dir->entries_count - 1,
               0, sizeof (Edje_External_Directory_Entry));

        edje_file->external_dir->entries[edje_file->external_dir->entries_count - 1].entry = mem_strdup(ex->name);
     }
}

/** @edcsubsection{toplevel_images,
 *                 Images} */

/**
    @page edcref

    @block
        images
    @context
        images {
            image: "filename1.ext" COMP;
            image: "filename2.ext" LOSSY 99;
            image: "filename2.ext" LOSSY_ETC1 50;
            set { }
            set { }
            ..
        }
    @description
        The "images" block is used to list each image file that will be used in
        the theme along with its compression method (if any).
        Besides the document's root, additional "images" blocks can be
        included inside other blocks, normally "collections", "group" and
        "part", easing maintenance of the file list when the theme is split
        among multiple files.
        @note if svg file use as image, not vector, it will be converted to bitmap
        and '.png' will be add to file name.
    @endblock

    @property
        image
    @parameters
        [image file] [compression method] (compression level)(edje file id)
    @effect
        Used to include each image file. The full path to the directory holding
        the images can be defined later with edje_cc's "-id" option.
        Compression methods:
        @li RAW: Uncompressed.
        @li COMP: Lossless compression.
        @li LOSSY [0-100]: JPEG lossy compression with quality from 0 to 100.
        @li LOSSY_ETC1 [0-100]: ETC1 lossy texture compression with quality from 0 to 100.
        @li LOSSY_ETC2 [0-100]: ETC2 lossy texture compression with quality from 0 to 100 (supports alpha).
        @li USER: Do not embed the file, refer to the external file instead.
        @li EXTERNAL: The file exists in the edje file with the specified id.

        Defaults: compression level for lossy methods is 90.
    @endproperty
 */
static void
st_images_image(void)
{
   /* NOTE: if you implement any changes with image_id mechanic don't forget
    * to update following functions: edje_edit_image_replace,
    * edje_edit_image_usage_list_get, edje_edit_image_del, _data_image_id_update
    */
   Edje_Image_Directory_Entry *img;
   const char *tmp;
   unsigned int i;
   int v;

   check_min_arg_count(2);

   if (!edje_file->image_dir)
     edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));

   tmp = parse_str(0);

   for (i = 0; i < edje_file->image_dir->entries_count; ++i)
     if (!strcmp(edje_file->image_dir->entries[i].entry, tmp))
       {
          free((char *)tmp);
          return;
       }

   edje_file->image_dir->entries_count++;
   img = realloc(edje_file->image_dir->entries,
                 sizeof (Edje_Image_Directory_Entry) * edje_file->image_dir->entries_count);
   if (!img)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->image_dir->entries = img;
   memset(edje_file->image_dir->entries + edje_file->image_dir->entries_count - 1,
          0, sizeof (Edje_Image_Directory_Entry));

   img = edje_file->image_dir->entries + edje_file->image_dir->entries_count - 1;

   img->entry = tmp;
   img->id = edje_file->image_dir->entries_count - 1;
   v = parse_enum(1,
                  "RAW", 0,
                  "COMP", 1,
                  "LOSSY", 2,
                  "LOSSY_ETC1", 3,
                  "LOSSY_ETC2", 4,
                  "USER", 5,
                  "EXTERNAL", 6,
                  NULL);
   if (v == 0)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
        img->source_param = 0;
     }
   else if (v == 1)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
        img->source_param = 1;
     }
   else if (v == 2)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY;
        img->source_param = 0;
     }
   else if (v == 3)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC1;
        img->source_param = 0;
     }
   else if (v == 4)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC2;
        img->source_param = 0;
     }
   else if (v == 5)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_USER;
        img->source_param = 0;
     }
   else if (v == 6)
     {
        img->source_type = EDJE_IMAGE_SOURCE_TYPE_EXTERNAL;
        img->source_param = 0;
        img->external_id = parse_str(2);
     }
   if ((img->source_type < EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY) ||
       (img->source_type == EDJE_IMAGE_SOURCE_TYPE_USER))
     check_arg_count(2);
   else if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
     {
        if (check_range_arg_count(2, 3) > 2)
          img->source_param = parse_int_range(2, 0, 100);
        else
          img->source_param = 90;
     }
   if (!edje_file->image_id_hash)
     edje_file->image_id_hash = eina_hash_string_superfast_new(free);
   {
      Edje_Image_Hash *eih = mem_alloc(SZ(Edje_Image_Hash));
      eih->id = img->id;
      eina_hash_add(edje_file->image_id_hash, tmp, eih);
   }
}

static void
_handle_vector_image(void)
{
   Edje_Part_Description_Vector *ed;
   unsigned int i = 0;
   char *name;

   ed = (Edje_Part_Description_Vector *)current_desc;

   name = parse_str(0);

   ed->vg.id = -1;

   for (i = 0; i < edje_file->image_dir->vectors_count; ++i)
     {
        if (!strcmp(edje_file->image_dir->vectors[i].entry, name))
          {
             ed->vg.set = EINA_TRUE;
             ed->vg.id = edje_file->image_dir->vectors[i].id;
             break;
          }
     }

   if (ed->vg.id < 0)
     error_and_abort(NULL, "Failed to find the vector resource :%s", name);

   free(name);
}

/** @edcsubsection{toplevel_images,
 *                 Images} */

/**
    @page edcref

    @block
        images
    @context
        images {
            vector: "filename1.svg";
            vector: "filename2.svg";
            vector: "filename3.svg";
            ..
        }
    @description
        The "vector" context in the "images" block is used to list each svg image file that will be used in
        the theme.
    @endblock

    @property
        vector
    @parameters
        [image file]
    @endproperty
 */
static void
st_images_vector(void)
{
   Edje_Vector_Directory_Entry *vector;
   const char *tmp;
   unsigned int i;

   check_min_arg_count(1);

   if (!edje_file->image_dir)
     edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));

   tmp = parse_str(0);

   for (i = 0; i < edje_file->image_dir->vectors_count; ++i)
     if (!strcmp(edje_file->image_dir->vectors[i].entry, tmp))
       {
          free((char *)tmp);
          return;
       }

   edje_file->image_dir->vectors_count++;
   vector = realloc(edje_file->image_dir->vectors,
                    sizeof (Edje_Vector_Directory_Entry) * edje_file->image_dir->vectors_count);
   if (!vector)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->image_dir->vectors = vector;
   memset(edje_file->image_dir->vectors + edje_file->image_dir->vectors_count - 1,
          0, sizeof (Edje_Vector_Directory_Entry));

   vector = edje_file->image_dir->vectors + edje_file->image_dir->vectors_count - 1;

   vector->entry = tmp;
   vector->id = edje_file->image_dir->vectors_count - 1;
}

/**
   @edcsubsection{toplevel_models,model}
 */

/**
    @page edcref

    @block
        models
    @context
        models {
            model: "filename1.ext";
            model: "filename2.ext";
            model: "filename2.ext" 50;
            ..
        }
    @description
        The "models" block is used to list each model file that will be used in
        the theme.
    @endblock

    @property
        model
    @parameters
        [model file]
    @effect
        Used to include each model file.
    @endproperty
 */
static void
st_models_model(void)
{
   Edje_Model_Directory_Entry *mdl;
   const char *tmp;
   unsigned int i;

   check_min_arg_count(1);

   if (!edje_file->model_dir)
     edje_file->model_dir = mem_alloc(SZ(Edje_Model_Directory));

   tmp = parse_str(0);

   for (i = 0; i < edje_file->model_dir->entries_count; ++i)
     if (!strcmp(edje_file->model_dir->entries[i].entry, tmp))
       {
          free((char *)tmp);
          return;
       }

   edje_file->model_dir->entries_count++;
   mdl = realloc(edje_file->model_dir->entries,
                 sizeof (Edje_Model_Directory_Entry) * edje_file->model_dir->entries_count);
   if (!mdl)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->model_dir->entries = mdl;
   memset(edje_file->model_dir->entries + edje_file->model_dir->entries_count - 1,
          0, sizeof (Edje_Model_Directory_Entry));

   mdl = edje_file->model_dir->entries + edje_file->model_dir->entries_count - 1;

   mdl->entry = tmp;
   mdl->id = edje_file->model_dir->entries_count - 1;
}

/** @edcsubsection{toplevel_images_set,
 *                 Images.Set} */

/**
    @page edcref

    @block
        set
    @context
        images {
            ..
            set {
                name: "image_name_used";
                image { }
                image { }
                ...
            }
        }
    @description
        The "set" block is used to define an image with different content
        depending on their size. Besides the document's root, additional
        "set" blocks can be included inside other blocks, normally
        "collections", "group" and "part", easing maintenance of the file
        list when the theme is split among multiple files.
    @endblock
 */
static void
ob_images_set(void)
{
   Edje_Image_Directory_Set *sets;

   if (!edje_file->image_dir)
     edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));

   edje_file->image_dir->sets_count++;
   sets = realloc(edje_file->image_dir->sets,
                  sizeof (Edje_Image_Directory_Set) * edje_file->image_dir->sets_count);
   if (!sets)
     {
        ERR("Not enough memory.");
        exit(-1);
     }
   edje_file->image_dir->sets = sets;
   memset(edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1,
          0, sizeof (Edje_Image_Directory_Set));

   edje_file->image_dir->sets[edje_file->image_dir->sets_count - 1].id = edje_file->image_dir->sets_count - 1;
}

/**
    @page edcref

    @property
        name
    @parameters
        [image name]
    @effect
        Define the name that refer to this image description.
    @endproperty
 */
static void
st_images_set_name(void)
{
   check_arg_count(1);

   edje_file->image_dir->sets[edje_file->image_dir->sets_count - 1].name = parse_str(0);
}

/**  @edcsubsection{toplevel_images_set_image,
 *                  Images.Set.Image} */

/**
    @page edcref

    @block
        image
    @context
        images {
            ..
            set {
                ..
                image {
                   image: "filename4.ext" COMP;
                   size: 51 51 200 200;
                   border: 0 0 0 0;
                   border_scale_by: 0.0;
                }
                ..
            }
        }
    @description
        The "image" block inside a "set" block define the characteristic of an image.
        Every block will describe one image and the size rule to use it.
    @endblock
 **/
static void
ob_images_set_image(void)
{
   Edje_Image_Directory_Set_Entry *entry;
   Edje_Image_Directory_Set *set;

   set = edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1;

   entry = mem_alloc(SZ(Edje_Image_Directory_Set_Entry));

   set->entries = eina_list_append(set->entries, entry);
}

/**
    @page edcref

    @property
        image
    @parameters
        [image file] [compression method] (compression level)(edje file id)
    @effect
        Used to include each image file. The full path to the directory holding
        the images can be defined later with edje_cc's "-id" option.
        Compression methods:
        @li RAW: Uncompressed.
        @li COMP: Lossless compression.
        @li LOSSY [0-100]: JPEG lossy compression with quality from 0 to 100.
        @li LOSSY_ETC1 [0-100]: ETC1 lossy texture compression with quality from 0 to 100.
        @li LOSSY_ETC2 [0-100]: ETC2 lossy texture compression with quality from 0 to 100 (supports alpha).
        @li USER: Do not embed the file, refer to the external file instead.
        @li EXTERNAL: The file exists in the edje file with the specified id.

        Defaults: compression level for lossy methods is 90.
    @endproperty
 **/
static void
st_images_set_image_image(void)
{
   Edje_Image_Directory_Set_Entry *entry;
   Edje_Image_Directory_Set *set;
   unsigned int i;

   set = edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1;
   entry = eina_list_data_get(eina_list_last(set->entries));

   /* Add the image to the global pool with the same syntax. */
   st_images_image();

   entry->name = parse_str(0);

   for (i = 0; i < edje_file->image_dir->entries_count; ++i)
     if (!strcmp(edje_file->image_dir->entries[i].entry, entry->name))
       {
          entry->id = i;
          return;
       }
}

/**
    @page edcref

    @property
        size
    @parameters
        [minw] [minh] [maxw] [maxh]
    @effect
        Define the minimal and maximal size that will select the specified image.

        Defaults: 0 0 0 0
    @endproperty
 */
static void
st_images_set_image_size(void)
{
   Edje_Image_Directory_Set_Entry *entry;
   Edje_Image_Directory_Set *set;

   set = edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1;
   entry = eina_list_data_get(eina_list_last(set->entries));

   entry->size.min.w = parse_int(0);
   entry->size.min.h = parse_int(1);
   entry->size.max.w = parse_int(2);
   entry->size.max.h = parse_int(3);

   if (entry->size.min.w > entry->size.max.w
       || entry->size.min.h > entry->size.max.h)
     {
        ERR("parse error %s:%i. Image min and max size are not in the right order ([%i, %i] < [%i, %i])",
            file_in, line - 1,
            entry->size.min.w, entry->size.min.h,
            entry->size.max.w, entry->size.max.h);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        border
    @parameters
        [left] [right] [top] [bottom]
    @effect
        If set, the area (in pixels) of each side of the image will be
        displayed as a fixed size border, from the side -> inwards, preventing
        the corners from being changed on a resize.

        Defaults: 0 0 0 0
    @since 1.8
    @endproperty
 */
static void
st_images_set_image_border(void)
{
   Edje_Image_Directory_Set_Entry *entry;
   Edje_Image_Directory_Set *set;

   set = edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1;
   entry = eina_list_data_get(eina_list_last(set->entries));

   entry->border.l = parse_int_range(0, 0, 0x7fffffff);
   entry->border.r = parse_int_range(1, 0, 0x7fffffff);
   entry->border.t = parse_int_range(2, 0, 0x7fffffff);
   entry->border.b = parse_int_range(3, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        border_scale_by
    @parameters
        [value]
    @effect
        If border scaling is enabled then normally the OUTPUT border sizes
        (e.g. if 3 pixels on the left edge are set as a border, then normally
        at scale 1.0, those 3 columns will always be the exact 3 columns of
        output, or at scale 2.0 they will be 6 columns, or 0.33 they will merge
        into a single column). This property multiplies the input scale
        factor by this multiplier, allowing the creation of "supersampled"
        borders to make much higher resolution outputs possible by always using
        the highest resolution artwork and then runtime scaling it down.

        Valid values are: 0.0 or bigger (0.0 or 1.0 to turn it off)

        Defaults: 0.0
    @since 1.8
    @endproperty
 */
static void
st_images_set_image_border_scale_by(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   ed->image.border.scale_by = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
}

/** @edcsubsection{toplevel_fonts,
 *                 Fonts} */

/**
    @page edcref

    @block
        fonts
    @context
        fonts {
            font: "filename1.ext" "fontname";
            font: "filename2.ext" "otherfontname";
            ..
        }
    @description
        The "fonts" block is used to list each font file with an alias used later
        in the theme. As with the "images" block, additional "fonts" blocks can
        be included inside other blocks.
    @endblock

    @property
        font
    @parameters
        [font filename] [font alias]
    @effect
        Defines each font "file" and "alias", the full path to the directory
        holding the font files can be defined with edje_cc's "-fd" option.
    @endproperty
 */
static void
st_fonts_font(void)
{
   Edje_Font *fn;

   check_arg_count(2);

   if (!edje_file->fonts)
     edje_file->fonts = eina_hash_string_small_new(free);

   fn = mem_alloc(SZ(Edje_Font));
   fn->file = parse_str(0);
   fn->name = parse_str(1);

   if (eina_hash_find(edje_file->fonts, fn->name))
     {
        free(fn->file);
        free(fn->name);
        free(fn);
        return;
     }

   eina_hash_direct_add(edje_file->fonts, fn->name, fn);
}

/** @edcsubsection{toplevel_data,
 *                 Data} */

/**
    @page edcref
    @block
        data
    @context
        data {
            item: "key" "value";
            file: "otherkey" "filename.ext";
            ..
        }
    @description
        The "data" block is used to pass arbitrary parameters from the theme to
        the application. Unlike the "images" and "fonts" blocks, additional
        "data" blocks can only be included inside the "group" block.
    @endblock

    @property
        item
    @parameters
        [parameter name] [parameter value]
    @effect
        Defines a new parameter, the value will be the string specified next to
        it.
    @endproperty
 */
static void
st_data_item(void)
{
   Edje_String *es;
   char *key;

   check_arg_count(2);

   key = parse_str(0);

   es = mem_alloc(SZ(Edje_String));
   es->str = parse_str(1);

   if (!edje_file->data)
     edje_file->data = eina_hash_string_small_new(free);
   else
     {
        if (eina_hash_find(edje_file->data, key))
          {
             ERR("parse error %s:%i. There is already a data.item of the name %s",
                 file_in, line - 1, key);
             exit(-1);
          }
     }

   eina_hash_direct_add(edje_file->data, key, es);
}

/**
    @page edcref
    @property
        file
    @parameters
        [parameter name] [parameter filename]
    @effect
        Defines a new parameter, the value will be the contents of the
        specified file formatted as a single string of text. This property only
        works with plain text files.
    @endproperty
 */
static void
st_data_file(void)
{
   const char *data;
   const char *over;
   Edje_String *es;
   char *filename;
   char *value;
   char *key;
   int fd;
   int i;
   struct stat buf;

   check_arg_count(2);

   key = parse_str(0);

   es = mem_alloc(SZ(Edje_String));
   filename = parse_str(1);

   fd = open(filename, O_RDONLY | O_BINARY, S_IRUSR | S_IWUSR);
   if (fd < 0)
     {
        char path[PATH_MAX], *dir;
        Eina_List *l;
        EINA_LIST_FOREACH(data_dirs, l, dir)
          {
             snprintf(path, sizeof(path), "%s/%s", dir, filename);
             fd = open(path, O_RDONLY | O_BINARY, S_IRUSR | S_IWUSR);
             if (fd >= 0)
               break;
          }

        if (fd < 0)
          {
             ERR("%s:%i when opening file \"%s\": \"%s\"",
                 file_in, line, filename, strerror(errno));
             exit(-1);
          }
     }

   if (fstat(fd, &buf))
     {
        ERR("%s:%i when stating file \"%s\": \"%s\"",
            file_in, line, filename, strerror(errno));
        exit(-1);
     }

   data = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (data == MAP_FAILED)
     {
        ERR("%s:%i when mapping file \"%s\": \"%s\"",
            file_in, line, filename, strerror(errno));
        exit(-1);
     }

   over = data;
   for (i = 0; i < buf.st_size; ++i, ++over)
     if (*over == '\0')
       {
          ERR("%s:%i file \"%s\" is a binary file.", file_in, line, filename);
          exit(-1);
       }

   value = malloc(sizeof (char) * buf.st_size + 1);
   snprintf(value, buf.st_size + 1, "%s", data);

   munmap((void *)data, buf.st_size);
   close(fd);

   es->str = value;

   if (!edje_file->data)
     edje_file->data = eina_hash_string_small_new(free);

   eina_hash_direct_add(edje_file->data, key, es);

   free(filename);
}

/** @edcsubsection{toplevel_color_tree,
 *                 Color Tree} */

/**
    @page edcref
    @block
        color_tree
    @context
        color_tree {
            "color_class_0" {
                "color_class_3";
                "color_class_4" {
                    "color_class_5";
                    "color_class_6";
                }
            }
            "color_class_1";
            "color_class_2";
            ..
        }
    @description
        The "color_tree" block contains color tree node blocks.
        Each node block begins with the name of color class and enclosed with braces.
        Node block can be placed within another node block.
    @endblock
 */
static void
ob_color_tree(void)
{
   if (!is_verbatim()) track_verbatim(1);
   else
     {
        char *s;

        s = get_verbatim();
        if (s)
          {
             process_color_tree(s, file_in, get_verbatim_line1());
             set_verbatim(NULL, 0, 0);
          }
     }
}

/** @edcsubsection{toplevel_color_classes,
 *                 Color Classes} */

/**
    @page edcref
    @block
        color_classes
    @context
        color_classes {
            color_class {
                name:  "colorclassname";
                color:  255 0 0 255;
                color2: "#0F0F";
                color3: "#0000FFFF";
            }
            ..
        }
    @description
        The "color_classes" block contains a list of one or more "color_class"
        blocks. Each "color_class" allows the designer to name an arbitrary
        group of colors to be used in the theme, the application can use that
        name to alter the color values at runtime.
    @endblock
 */
static void
ob_color_class(void)
{
   Edje_Color_Class *cc;

   cc = mem_alloc(SZ(Edje_Color_Class));
   edje_file->color_classes = eina_list_append(edje_file->color_classes, cc);

   cc->r = 0;
   cc->g = 0;
   cc->b = 0;
   cc->a = 0;
   cc->r2 = 0;
   cc->g2 = 0;
   cc->b2 = 0;
   cc->a2 = 0;
   cc->r3 = 0;
   cc->g3 = 0;
   cc->b3 = 0;
   cc->a3 = 0;
}

static void
_color_class_name(char *name)
{
   Edje_Color_Class *cc, *tcc;
   Eina_List *l;

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));
   cc->name = name;
   EINA_LIST_FOREACH(edje_file->color_classes, l, tcc)
     {
        if ((cc != tcc) && (!strcmp(cc->name, tcc->name)))
          {
             ERR("parse error %s:%i. There is already a color class named \"%s\"",
                 file_in, line - 1, cc->name);
             exit(-1);
          }
     }
}

/**
    @page edcref

    @property
        name
    @parameters
        [color class name]
    @effect
        Sets the name for the color class, used as reference by both the theme
        and the application.
    @endproperty
 */
static void
st_color_class_name(void)
{
   Edje_Color_Class *cc, *tcc;
   Eina_List *l;

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));
   cc->name = parse_str(0);
   EINA_LIST_FOREACH(edje_file->color_classes, l, tcc)
     {
        if ((cc != tcc) && (!strcmp(cc->name, tcc->name)))
          {
             ERR("parse error %s:%i. There is already a color class named \"%s\"",
                 file_in, line - 1, cc->name);
             exit(-1);
          }
     }
}

static void
parse_color(unsigned int first_arg, void *base)
{
   Edje_Color *color = (Edje_Color *)base;
   int r, g, b, a;
   char *str;

   switch (get_arg_count() - first_arg)
     {
      case 1:
        str = parse_str(first_arg);
        convert_color_code(str, &r, &g, &b, &a);
        color->r = r;
        color->g = g;
        color->b = b;
        color->a = a;
        break;

      case 4:
        color->r = parse_int_range(first_arg + 0, 0, 255);
        color->g = parse_int_range(first_arg + 1, 0, 255);
        color->b = parse_int_range(first_arg + 2, 0, 255);
        color->a = parse_int_range(first_arg + 3, 0, 255);
        break;

      default:
        ERR("%s:%i. color code should be a string or a set of 4 integers.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        color
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        The main color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 0 0 0 0
    @endproperty
 */
static void
st_color_class_color(void)
{
   Edje_Color_Class *cc;

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));

   parse_color(0, &(cc->r));
}

/**
    @page edcref
    @property
        color2
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Used as outline in text and textblock parts.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 0 0 0 0
    @endproperty
 */
static void
st_color_class_color2(void)
{
   Edje_Color_Class *cc;

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));

   parse_color(0, &(cc->r2));
}

/**
    @page edcref
    @property
        color3
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Used as shadow in text and textblock parts.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 0 0 0 0
    @endproperty
 */
static void
st_color_class_color3(void)
{
   Edje_Color_Class *cc;

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));

   parse_color(0, &(cc->r3));
}

/**
    @page edcref
    @property
        description
    @parameters
        [color class description]
    @effect
        Provides a descriptive name for the effect of the color class
        @since 1.14
    @endproperty
 */
static void
st_color_class_desc(void)
{
   Edje_Color_Class *cc;

   check_arg_count(1);

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));
   cc->desc = parse_str(0);
}

/** @edcsubsection{toplevel_styles,
 *                 Styles} */

/**
    @page edcref
    @block
        styles
    @context
        styles {
            style {
                name: "stylename";
                base: "..default style properties..";

                tag:  "tagname" "..style properties..";
                ..
            }
            ..
        }
    @description
        The "styles" block contains a list of one or more "style" blocks. A
        "style" block is used to create style \<tags\> for advanced TEXTBLOCK
        formatting.
    @endblock
 */
static void
ob_styles_style(void)
{
   Edje_Style *stl;

   stl = mem_alloc(SZ(Edje_Style));
   edje_file->styles = eina_list_append(edje_file->styles, stl);
}

static void
_style_name(char *name)
{
   Edje_Style *stl, *tstl;
   Eina_List *l;

   stl = eina_list_last_data_get(edje_file->styles);
   free(stl->name);
   stl->name = name;
   EINA_LIST_FOREACH(edje_file->styles, l, tstl)
     {
        if (stl->name && tstl->name && (stl != tstl) && (!strcmp(stl->name, tstl->name)))
          {
             ERR("parse error %s:%i. There is already a style named \"%s\"",
                 file_in, line - 1, stl->name);
             exit(-1);
          }
     }
}

/**
    @page edcref
    @property
        name
    @parameters
        [style name]
    @effect
        The name of  the style to be used as reference later in the theme.
    @endproperty
 */
static void
st_styles_style_name(void)
{
   _style_name(parse_str(0));
}

/**
    @page edcref
    @property
        base
    @parameters
        [style properties string]
    @effect
        The default style properties that will be applied to the complete
        text.
    @endproperty
 */
static void
st_styles_style_base(void)
{
   Edje_Style *stl;
   Edje_Style_Tag *tag;

   stl = eina_list_data_get(eina_list_last(edje_file->styles));
   if (stl->tags)
     {
        ERR("parse error %s:%i. There is already a basic format for the style",
            file_in, line - 1);
        exit(-1);
     }
   tag = mem_alloc(SZ(Edje_Style_Tag));
   tag->key = mem_strdup("DEFAULT");
   tag->value = parse_str(0);
   stl->tags = eina_list_append(stl->tags, tag);
}

/**
    @page edcref
    @property
        tag
    @parameters
        [tag name] [style properties string]
    @effect
        Style to be applied only to text between style \<tags\>..\</tags\>.
        When creating "paired" tags, like \<bold\>\</bold\>, A '+' should be added at the start of the style properties of the first part (\<bold\>).
        If the second part (\</bold\>) is also defined, a '-' should be prepended to it's style properties.
        This only applies to paired tags; Single tags, like \<tab\>, must not include a starting '+'.
    @endproperty
 */
static void
st_styles_style_tag(void)
{
   Edje_Style *stl;
   Edje_Style_Tag *tag;

   stl = eina_list_data_get(eina_list_last(edje_file->styles));
   tag = mem_alloc(SZ(Edje_Style_Tag));
   tag->key = parse_str(0);
   tag->value = parse_str(1);
   stl->tags = eina_list_append(stl->tags, tag);
}

/** @edcsubsection{toplevel_text_classes,
 *                 Text Classes} */

/**
    @page edcref
    @block
        text_classes
    @context
        text_classes {
           text_class {
              name: "text_class name";
              font: "font name";
              size: SIZE";
           }
            ..
        }
    @description
        The "text_classes" block contains a list of one or more "text_class"
        blocks. Each "text_class" allows the designer to name an arbitrary
        group of font and size to be used in the theme, the application can
        use that name to alter the font and its size at runtime.
    @endblock
 */
static void
ob_text_class(void)
{
   Edje_Text_Class *tc;

   tc = mem_alloc(SZ(Edje_Text_Class));
   edje_file->text_classes = eina_list_append(edje_file->text_classes, tc);

   tc->font = NULL;
   tc->size = 0;
}

static void
_text_class_name(char *name)
{
   Edje_Text_Class *tc, *ttc;
   Eina_List *l;

   tc = eina_list_data_get(eina_list_last(edje_file->text_classes));
   tc->name = name;
   EINA_LIST_FOREACH(edje_file->text_classes, l, ttc)
     {
        if ((tc != ttc) && (!strcmp(tc->name, ttc->name)))
          {
             ERR("parse error %s:%i. There is already a text class named \"%s\"",
                 file_in, line - 1, tc->name);
             exit(-1);
          }
     }
}

/**
    @page edcref

    @property
        name
    @parameters
        [text class name]
    @effect
        Sets the name for the text class, used as reference by both the theme
        and the application.
    @endproperty
 */
static void
st_text_class_name(void)
{
   Edje_Text_Class *tc, *ttc;
   Eina_List *l;

   tc = eina_list_data_get(eina_list_last(edje_file->text_classes));
   tc->name = parse_str(0);
   EINA_LIST_FOREACH(edje_file->text_classes, l, ttc)
     {
        if ((tc != ttc) && (!strcmp(tc->name, ttc->name)))
          {
             ERR("parse error %s:%i. There is already a text class named \"%s\"",
                 file_in, line - 1, tc->name);
             exit(-1);
          }
     }
}

/**
    @page edcref

    @property
        font
    @parameters
        [font name]
    @effect
        Sets the font family for the text class.
    @endproperty
 */
static void
st_text_class_font(void)
{
   Edje_Text_Class *tc;

   check_arg_count(1);

   tc = eina_list_data_get(eina_list_last(edje_file->text_classes));
   tc->font = parse_str(0);
}

/**
    @page edcref

    @property
        size
    @parameters
        [font size in points (pt)]
    @effect
        Sets the font size for the text class.

        Defaults: 0
    @endproperty
 */
static void
st_text_class_size(void)
{
   Edje_Text_Class *tc;

   check_arg_count(1);

   tc = eina_list_data_get(eina_list_last(edje_file->text_classes));
   tc->size = parse_int_range(0, 0, 255);
}

/** @edcsubsection{toplevel_size_classes,
 *                 Size Classes} */

/**
    @page edcref
    @block
        size_classes
    @context
        size_classes {
           size_class {
              name:  "sizeclassname";
              min: width height;
              max: width height;
           }
            ..
        }
    @description
        The "size_classes" block contains a list of one or more "size_class"
        blocks. Each "size_class" allows the designer to name an arbitrary
        group of size to be used in the theme, the application can use that
        name to alter the min and max size values at runtime.
    @endblock
 */
static void
ob_size_class(void)
{
   Edje_Size_Class *sc;

   sc = mem_alloc(SZ(Edje_Size_Class));
   edje_file->size_classes = eina_list_append(edje_file->size_classes, sc);

   sc->minw = 0;
   sc->minh = 0;
   sc->maxw = -1;
   sc->maxh = -1;
}

static void
_size_class_name(char *name)
{
   Edje_Size_Class *sc, *tsc;
   Eina_List *l;

   sc = eina_list_data_get(eina_list_last(edje_file->size_classes));
   sc->name = name;
   EINA_LIST_FOREACH(edje_file->size_classes, l, tsc)
     {
        if ((sc != tsc) && (!strcmp(sc->name, tsc->name)))
          {
             ERR("parse error %s:%i. There is already a size class named \"%s\"",
                 file_in, line - 1, sc->name);
             exit(-1);
          }
     }
}

/**
    @page edcref

    @property
        name
    @parameters
        [size class name]
    @effect
        Sets the name for the size class, used as reference by both the theme
        and the application.
    @endproperty
 */
static void
st_size_class_name(void)
{
   Edje_Size_Class *sc, *tsc;
   Eina_List *l;

   sc = eina_list_data_get(eina_list_last(edje_file->size_classes));
   sc->name = parse_str(0);
   EINA_LIST_FOREACH(edje_file->size_classes, l, tsc)
     {
        if ((sc != tsc) && (!strcmp(sc->name, tsc->name)))
          {
             ERR("parse error %s:%i. There is already a size class named \"%s\"",
                 file_in, line - 1, sc->name);
             exit(-1);
          }
     }
}

/**
    @page edcref
    @property
        min
    @parameters
        [width] [height]
    @effect
        The minimum size.

        Defaults: 0 0
    @endproperty
 */
static void
st_size_class_min(void)
{
   Edje_Size_Class *sc;

   check_arg_count(2);

   sc = eina_list_data_get(eina_list_last(edje_file->size_classes));
   sc->minw = parse_int_range(0, 0, 0x7fffffff);
   sc->minh = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height]
    @effect
        The maximum size.

        Defaults: -1 -1
    @endproperty
 */
static void
st_size_class_max(void)
{
   Edje_Size_Class *sc;

   check_arg_count(2);

   sc = eina_list_data_get(eina_list_last(edje_file->size_classes));
   sc->maxw = parse_int_range(0, -1, 0x7fffffff);
   sc->maxh = parse_int_range(1, -1, 0x7fffffff);
}

/** @edcsection{collections,Collections Blocks} */

/** @edcsubsection{collections,
 *                 Collections} */

/**
    @page edcref
    @block
        collections
    @context
        collections {
            base_scale: 1.2;
            sounds { }
            vibrations { }
            group { }
            group { }
            ..
        }
    @description
        The "collections" block is used to list the groups that compose the
        theme. Additional "collections" blocks do not prevent overriding group
        names. The "sounds" block comprises of all sound definitions. The "vibrations"
        block compriese all vibration definitions.
    @endblock
 */
static void
ob_collections(void)
{
   if (!edje_file->collection)
     {
        edje_file->collection = eina_hash_string_small_new(NULL);
        edje_collections_lookup = eina_hash_int32_new(NULL);
        desc_hash = eina_hash_pointer_new(NULL);
     }
}

/**
    @page edcref
    @property
        base_scale
    @parameters
        [scale val]
    @effect
        The base_scale is the standard scale value of the collection.
        The default base_scale is 1.0. It means the collection is made in the environment
        which is same with a desktop(The monitor has 96 dpi).
        If you make a collection in another environment(ex: 115 dpi), you have to
        set the base_scale(ex: 1.2). Then it will be shown same size in the desktop.

        Defaults: 1.0
    @since 1.11
    @endproperty
 */
static void
st_collections_base_scale(void)
{
   check_min_arg_count(1);

   edje_file->base_scale = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
   if (edje_file->base_scale == ZERO)
     {
        ERR("The base_scale is 0.0. The value should be bigger than 0.0.");
        exit(-1);
     }
}

/** @edcsubsection{collections_sounds,
 *                 Sounds} */

/**
    @page edcref
    @block
        sounds
    @context
        sounds {
            tone: "tone-1"  2300;
            tone: "tone-2"  2300;
            sample { }
            sample { }
            ..
        }
    @description
        The "sounds" block contains a list of one or more sound sample and tones items.
    @endblock
 */

/**
    @page edcref
    @property
        tone
    @parameters
        [tone name] [frequency]
    @effect
        sound of specific frequency
    @since 1.1
    @endproperty
 */
static void
st_collections_group_sound_tone(void)
{
   Edje_Sound_Tone *tone;
   const char *tmp;
   unsigned int i;
   int value;

   check_arg_count(2);

   if (!edje_file->sound_dir)
     edje_file->sound_dir = mem_alloc(SZ(Edje_Sound_Directory));

   tmp = parse_str(0);
   /* Audible range 20 to 20KHz */
   value = parse_int_range(1, 20, 20000);

   /* Check for Tone duplication */
   for (i = 0; i < edje_file->sound_dir->tones_count; i++)
     {
        if (!strcmp(edje_file->sound_dir->tones[i].name, tmp))
          {
             ERR("Tone name: %s already exist.", tmp);
             free((char *)tmp);
             exit(-1);
          }
        if (edje_file->sound_dir->tones[i].value == value)
          {
             ERR("Tone name %s with same frequency %d exist.",
                 edje_file->sound_dir->tones[i].name, value);
             exit(-1);
          }
     }
   edje_file->sound_dir->tones_count++;
   tone = realloc(edje_file->sound_dir->tones,
                  sizeof (Edje_Sound_Tone) *
                  edje_file->sound_dir->tones_count);
   if (!tone)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->sound_dir->tones = tone;

   tone = edje_file->sound_dir->tones + edje_file->sound_dir->tones_count - 1;
   memset(tone, 0, sizeof (Edje_Sound_Tone));

   tone->name = tmp;
   tone->value = value;
   tone->id = edje_file->sound_dir->tones_count - 1;
}

/** @edcsubsection{collections_sounds_sample,
 *                 Sounds.Sample} */

/**
    @page edcref
    @block
        sample
    @context
        sounds {
            ..
            sample {
                name: "sound_file1" RAW;
                source: "sound_file1.wav";
            }
            sample {
                name: "sound_file2" LOSSY 0.5;
                source: "sound_file2.wav";
            }
        }
    @description
        The sample block defines the sound sample.
    @endblock
    @property
        name
    @parameters
        [sample name] [compression type] (quality)
    @effect
        Used to include each sound file. The full path to the directory holding
        the sounds can be defined later with edje_cc's "-sd" option.
        Valid types are:
        @li RAW: Uncompressed.
        @li COMP: Lossless compression.
        @li LOSSY [45.0  - 1000.0]: Lossy compression with quality from 45.0 to 1000.0.
        @li AS_IS: Check for re-encoding, no compression/encoding, just write the file information as it is.

    @since 1.1
    @endproperty
 */
static void
st_collections_group_sound_sample_name(void)
{
   Edje_Sound_Sample *sample;
   const char *tmp;
   unsigned int i;

   if (!edje_file->sound_dir)
     edje_file->sound_dir = mem_alloc(SZ(Edje_Sound_Directory));

   tmp = parse_str(0);

   for (i = 0; i < edje_file->sound_dir->samples_count; i++)
     {
        if (!strcmp(edje_file->sound_dir->samples[i].name, tmp))
          {
             free((char *)tmp);
             return;
          }
     }

   edje_file->sound_dir->samples_count++;
   sample = realloc(edje_file->sound_dir->samples,
                    sizeof(Edje_Sound_Sample) *
                    edje_file->sound_dir->samples_count);
   if (!sample)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->sound_dir->samples = sample;

   sample =
     edje_file->sound_dir->samples +
     edje_file->sound_dir->samples_count - 1;
   memset(sample, 0, sizeof (Edje_Sound_Sample));

   sample->name = tmp;
   sample->id = edje_file->sound_dir->samples_count - 1;
   sample->compression = parse_enum(1,
                                    "RAW", EDJE_SOUND_SOURCE_TYPE_INLINE_RAW,
                                    "COMP", EDJE_SOUND_SOURCE_TYPE_INLINE_COMP,
                                    "LOSSY", EDJE_SOUND_SOURCE_TYPE_INLINE_LOSSY,
                                    "AS_IS", EDJE_SOUND_SOURCE_TYPE_INLINE_AS_IS,
                                    NULL);

   if (sample->compression == EDJE_SOUND_SOURCE_TYPE_INLINE_LOSSY)
     {
        sample->quality = parse_float_range(2, 45.0, 1000.0);
        check_arg_count(3);
     }
   else
     check_arg_count(2);
}

/**
    @page edcref
    @property
        source
    @parameters
        [sound file name]
    @effect
        The Sound source file name (Source can be mono/stereo WAV file.
        Only files with 44.1 KHz sample rate supported now)
    @since 1.1
    @endproperty
 */
static void
st_collections_group_sound_sample_source(void)
{
   Edje_Sound_Sample *sample;

   if (!edje_file->sound_dir->samples)
     {
        ERR("Invalid sound sample source definition.");
        exit(-1);
     }

   sample =
     edje_file->sound_dir->samples +
     edje_file->sound_dir->samples_count - 1;
   sample->snd_src = parse_str(0);
   check_arg_count(1);
}

/** @edcsubsection{collections_vibrations,
 *                 Vibrations} */

/**
    @page edcref
    @block
        vibrations
    @context
        vibrations {
           sample { }
           sample { }
           ..
        }

    @description
        The "vibrations" block contains a list of one or more vibration sample.
    @since 1.10
    @endblock
 */

/** @edcsubsection{collections_vibrations_sample,
 *                 Vibrations.Sample} */

/**
    @page edcref
    @block
        sample
    @context
        vibrations {
            sample {
                name: "vibration_file1";
                source: "vibration_file1.xxx";
            }
            ..
        }
    @description
        The sample block defines the vibration sample.
    @endblock
    @property
        name
    @parameters
        [sample name]
    @effect
        Used to include each vibration file. The full path to the directory holding
        the vibrations can be defined later with edje_cc's "-vd" option.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_vibration_sample_name(void)
{
   Edje_Vibration_Sample *sample;
   const char *tmp;
   unsigned int i;

   if (!edje_file->vibration_dir)
     edje_file->vibration_dir = mem_alloc(SZ(Edje_Vibration_Directory));

   tmp = parse_str(0);

   for (i = 0; i < edje_file->vibration_dir->samples_count; i++)
     {
        if (!strcmp(edje_file->vibration_dir->samples[i].name, tmp))
          {
             free((char *)tmp);
             return;
          }
     }

   edje_file->vibration_dir->samples_count++;
   sample = realloc(edje_file->vibration_dir->samples,
                    sizeof(Edje_Vibration_Sample) *
                    edje_file->vibration_dir->samples_count);
   if (!sample)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->vibration_dir->samples = sample;

   sample =
     edje_file->vibration_dir->samples +
     edje_file->vibration_dir->samples_count - 1;
   memset(sample, 0, sizeof(Edje_Vibration_Sample));

   sample->name = tmp;
   sample->id = edje_file->vibration_dir->samples_count - 1;

   check_arg_count(1);
}

/**
    @page edcref
    @property
        source
    @parameters
        [vibration file name]
    @effect
        The Vibration source file name
    @since 1.10
    @endproperty
 */
static void
st_collections_group_vibration_sample_source(void)
{
   Edje_Vibration_Sample *sample;

   if (!edje_file->vibration_dir->samples)
     {
        ERR("Invalid vibration sample source definition.");
        exit(-1);
     }

   sample =
     edje_file->vibration_dir->samples +
     edje_file->vibration_dir->samples_count - 1;
   sample->src = parse_str(0);
   check_arg_count(1);
}

/** @edcsubsection{collections_translation_file,
 *                 translation.file} */

/**
    @page edcref
    @block
        file
    @context
        translation {
            ..
            file {
                locale: "en_IN";
                source: "domain_name.po";
            }
            file {
                locale: "en_US";
                source: "domain_name.po";
            }
        }
    @description
        The file block defines the po or mo file.
    @endblock
    @property
        name
    @parameters
        [locale name]
    @effect
        Used to include each po or mo file. The full path to the directory holding
        the po or mo file can be defined later with edje_cc's "-md" option.

    @since 1.15
    @endproperty
 */
static void
st_collections_group_translation_file_locale(void)
{
   Edje_Mo *mo_entry;
   const char *tmp;
   unsigned int i;

   check_arg_count(1);

   if (!edje_file->mo_dir)
     edje_file->mo_dir = mem_alloc(SZ(Edje_Mo_Directory));

   tmp = parse_str(0);

   for (i = 0; i < edje_file->mo_dir->mo_entries_count; i++)
     {
        if (!strcmp(edje_file->mo_dir->mo_entries[i].locale, tmp))
          {
             free((char *)tmp);
             return;
          }
     }

   edje_file->mo_dir->mo_entries_count++;
   mo_entry = realloc(edje_file->mo_dir->mo_entries, sizeof(Edje_Mo) * edje_file->mo_dir->mo_entries_count);

   if (!mo_entry)
     {
        ERR("No enough memory.");
        exit(-1);
     }
   edje_file->mo_dir->mo_entries = mo_entry;

   mo_entry = edje_file->mo_dir->mo_entries + edje_file->mo_dir->mo_entries_count - 1;
   memset(mo_entry, 0, sizeof (Edje_Mo));

   mo_entry->locale = tmp;
   mo_entry->id = edje_file->mo_dir->mo_entries_count - 1;
}

/**
    @page edcref
    @property
        source
    @parameters
        [po file name or Mo file name]
    @effect
        The po or mo source file name (Source should be a valid po or mo file.
        Only po or mo files are supported now)
    @since 1.15
    @endproperty
 */

static void
st_collections_group_translation_file_source(void)
{
   Edje_Mo *mo_entry;

   check_arg_count(1);

   if (!edje_file->mo_dir->mo_entries)
     {
        ERR("Invalid mo source definition.");
        exit(-1);
     }

   mo_entry = edje_file->mo_dir->mo_entries + edje_file->mo_dir->mo_entries_count - 1;
   mo_entry->mo_src = parse_str(0);
}

static void
_link_combine(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Collection_Parser *pcp;
   Eina_Iterator *it;
   Eina_Hash_Tuple *tup;

   pc = eina_list_last_data_get(edje_collections);
   pcp = eina_list_last_data_get(edje_collections);

   if (!pcp->link_hash) return;
   it = eina_hash_iterator_tuple_new(pcp->link_hash);
   EINA_ITERATOR_FOREACH(it, tup)
     {
        while (tup->data)
          {
             Edje_Part_Description_Link *el, *ell;
             Eina_List *l, *ll, *combine = NULL;

             el = eina_list_data_get(tup->data);
             tup->data = eina_list_remove_list(tup->data, tup->data);
             EINA_LIST_FOREACH_SAFE(tup->data, l, ll, ell)
               {
                  if (ell->pr->tween.mode != el->pr->tween.mode) continue;
                  if (fabs(ell->pr->tween.time - el->pr->tween.time) > DBL_EPSILON) continue;
                  if (fabs(ell->pr->tween.v1 - el->pr->tween.v1) > DBL_EPSILON) continue;
                  if (fabs(ell->pr->tween.v2 - el->pr->tween.v2) > DBL_EPSILON) continue;
                  if (fabs(ell->pr->tween.v3 - el->pr->tween.v3) > DBL_EPSILON) continue;
                  if (fabs(ell->pr->tween.v4 - el->pr->tween.v4) > DBL_EPSILON) continue;
                  if (fabs(ell->ed->state.value - el->ed->state.value) > DBL_EPSILON) continue;
                  if ((!!ell->ed->state.name) != (!!el->ed->state.name))
                    {
                       if (((!!ell->ed->state.name) && strcmp(ell->ed->state.name, "default")) ||
                           ((!!el->ed->state.name) && strcmp(el->ed->state.name, "default")))
                         continue;
                    }
                  else if (ell->ed->state.name && strcmp(ell->ed->state.name, el->ed->state.name))
                    continue;
                  eina_list_move_list(&combine, (Eina_List **)&tup->data, l);
               }
             current_program = el->pr;
             if (!el->epp->common.name)
               {
                  ERR("A part without a name was detected.");
                  exit(-1);
               }
             _program_target_add(strdup(el->epp->common.name));
             EINA_LIST_FREE(combine, ell)
               {
                  char *name;

                  _program_target_add(strdup(ell->epp->common.name));
                  EINA_LIST_FOREACH(ell->pr->after, l, name)
                    _program_after(name);
                  _program_free(ell->pr);
                  free(ell);
               }
             _edje_program_insert(pc, current_program);
          }
     }
   eina_iterator_free(it);
   eina_hash_free(pcp->link_hash);
   pcp->links = eina_list_free(pcp->links);
   current_program = NULL;
}

/** @edcsubsection{collections_group,
 *                 Group} */

/**
    @page edcref
    @block
        group
    @context
        collections {
            ..
            group {
                name: "nameusedbytheapplication";
                alias: "anothername";
                min: width height;
                max: width height;

                script { }
                limits { }
                data { }
                parts { }
                programs { }
            }
            ..
        }
    @description
        A "group" block contains the list of parts and programs that compose a
        given Edje Object.
    @endblock
 */
static void
ob_collections_group(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Collection_Parser *pcp;
   Code *cd;

   if (current_de && !current_de->entry)
     {
        ERR("A collection without a name was detected, that's not allowed.");
        exit(-1);
     }
   current_program = NULL;
   current_part = NULL;
   current_desc = NULL;

   current_group_inherit = EINA_FALSE;
   script_is_replaceable = EINA_FALSE;

   current_de = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   current_de->id = eina_list_count(edje_collections);

   if (!edje_collections_lookup)
     ob_collections();
   eina_hash_add(edje_collections_lookup, &current_de->id, current_de);

   pc = mem_alloc(SZ(Edje_Part_Collection_Parser));
   edje_collections = eina_list_append(edje_collections, pc);
   pc->id = current_de->id;
   pc->broadcast_signal = EINA_TRUE; /* This was the behaviour by default in Edje 1.1 */

   cd = mem_alloc(SZ(Code));
   codes = eina_list_append(codes, cd);

   pcp = (Edje_Part_Collection_Parser *)pc;
   pcp->default_mouse_events = 1;
   pcp->inherit_script = EINA_FALSE;

   pc->scene_size.width = 0;
   pc->scene_size.height = 0;

#ifdef HAVE_EPHYSICS
   pc->physics.world.gravity.x = 0;
   pc->physics.world.gravity.y = 294;
   pc->physics.world.gravity.z = 0;
   pc->physics.world.depth = 100;
   pc->physics.world.z = -50;
   pc->physics.world.rate = FROM_DOUBLE(30);
#endif
}

static void
_group_name(char *name)
{
   Edje_Part_Collection_Directory_Entry *alias;
   Edje_Part_Collection_Directory_Entry *older;
   Edje_Part_Collection *current_pc;
   Eina_List *l = NULL;

   current_pc = eina_list_data_get(eina_list_last(edje_collections));

   if (current_de->entry)
     goto double_named_group;

   current_de->entry = name;
   current_pc->part = current_de->entry;

   older = eina_hash_find(edje_file->collection, current_de->entry);
   if (older) eina_hash_del(edje_file->collection, current_de->entry, older);
   eina_hash_direct_add(edje_file->collection, current_de->entry, current_de);
   if (!older) return;

   EINA_LIST_FOREACH(aliases, l, alias)
     if (strcmp(alias->entry, current_de->entry) == 0)
       {
          Edje_Part_Collection *pc;

          pc = eina_list_nth(edje_collections, older->id);
          INF("overriding alias ('%s' => '%s') by group '%s'",
              alias->entry, pc->part,
              current_de->entry);
          aliases = eina_list_remove_list(aliases, l);
          free(alias);
          break;
       }

double_named_group:
   ERR("Invalid group '%s', only a single name statement is valid for group,"
       "use alias instead.", current_de->entry);
   exit(-1);
}

/**
    @page edcref
    @property
        name
    @parameters
        [group name]
    @effect
        The name that will be used by the application to load the resulting
        Edje object and to identify the group to swallow in a GROUP part. If
        group with the same name exists already, it won't be compiled.
        Only a single name statement is valid for group, use alias instead if
        you want to give additional names.
    @endproperty
 */
static void
st_collections_group_name(void)
{
   check_arg_count(1);
   _group_name(parse_str(0));
}

/**
    @page edcref
    @property
        skip_namespace_validation
    @parameters
        [1 or 0]
    @effect
        This disables namespace validation for the current group if validation has
        been enabled with edje_cc's -N option.
        This property can be inherited.
        Defaults: 0

    @warning Your edc file should always wrap this keyword with #ifdef HAVE_SKIP_NAMESPACE_VALIDATION
    @since 1.21
    @endproperty
 */
static void
st_collections_group_skip_namespace_validation(void)
{
   Edje_Part_Collection_Parser *pcp = eina_list_last_data_get(edje_collections);
   check_arg_count(1);
   pcp->skip_namespace_validation = parse_bool(0);
}

typedef struct _Edje_List_Foreach_Data Edje_List_Foreach_Data;
struct _Edje_List_Foreach_Data
{
   Eina_List *list;
};

static Eina_Bool
_edje_data_item_list_foreach(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data EINA_UNUSED, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, strdup(key));

   return EINA_TRUE;
}

static void
_filter_copy(Edje_Part_Description_Spec_Filter *ed, const Edje_Part_Description_Spec_Filter *parent)
{
   ed->code = STRDUP(parent->code);
   if (ed->code)
     {
        const char *name;
        Eina_List *l;
        unsigned k;

        ed->name = STRDUP(parent->name);
        ed->sources = NULL;
        EINA_LIST_FOREACH(parent->sources, l, name)
          ed->sources = eina_list_append(ed->sources, STRDUP(name));
        ed->data = NULL;
        ed->data_count = 0;
        if (parent->data)
          {
             ed->data = mem_alloc(parent->data_count * sizeof(*parent->data));
             ed->data_count = parent->data_count;
             for (k = 0; k < parent->data_count; k++)
               {
                  ed->data[k].name = STRDUP(parent->data[k].name);
                  ed->data[k].value = STRDUP(parent->data[k].value);
               }
          }
     }
   else memset(ed, 0, sizeof(*ed));
}

static void
_parts_count_update(unsigned int type, int inc)
{
   switch (type)
     {
      case EDJE_PART_TYPE_RECTANGLE:
        current_de->count.RECTANGLE += inc;
        break;

      case EDJE_PART_TYPE_TEXT:
        current_de->count.TEXT += inc;
        break;

      case EDJE_PART_TYPE_IMAGE:
        current_de->count.IMAGE += inc;
        break;

      case EDJE_PART_TYPE_SWALLOW:
        current_de->count.SWALLOW += inc;
        break;

      case EDJE_PART_TYPE_TEXTBLOCK:
        current_de->count.TEXTBLOCK += inc;
        break;

      case EDJE_PART_TYPE_GROUP:
        current_de->count.GROUP += inc;
        break;

      case EDJE_PART_TYPE_BOX:
        current_de->count.BOX += inc;
        break;

      case EDJE_PART_TYPE_TABLE:
        current_de->count.TABLE += inc;
        break;

      case EDJE_PART_TYPE_EXTERNAL:
        current_de->count.EXTERNAL += inc;
        break;

      case EDJE_PART_TYPE_PROXY:
        current_de->count.PROXY += inc;
        break;

      case EDJE_PART_TYPE_MESH_NODE:
        current_de->count.MESH_NODE += inc;
        break;

      case EDJE_PART_TYPE_LIGHT:
        current_de->count.LIGHT += inc;
        break;

      case EDJE_PART_TYPE_CAMERA:
        current_de->count.CAMERA += inc;
        break;

      case EDJE_PART_TYPE_SPACER:
        current_de->count.SPACER += inc;
        break;

      case EDJE_PART_TYPE_SNAPSHOT:
        current_de->count.SNAPSHOT += inc;
        break;

      case EDJE_PART_TYPE_VECTOR:
        current_de->count.VECTOR += inc;
        break;
     }
   current_de->count.part += inc;
}

static void
_part_copy(Edje_Part *ep, Edje_Part *ep2)
{
   Edje_Part_Collection *pc;
   Edje_Part_Parser *epp, *epp2;
   Edje_Pack_Element *item, *item2;
   Edje_Pack_Element_Parser *pitem;
   Edje_Part_Description_Common *ed, *ed2;
   unsigned int j;

   pc = eina_list_last_data_get(edje_collections);

   ep->name = STRDUP(ep2->name);
   ep->source = STRDUP(ep2->source);
   ep->source2 = STRDUP(ep2->source2);
   ep->source3 = STRDUP(ep2->source3);
   ep->source4 = STRDUP(ep2->source4);
   ep->source5 = STRDUP(ep2->source5);
   ep->source6 = STRDUP(ep2->source6);

   data_queue_copied_part_lookup(pc, &(ep2->clip_to_id), &(ep->clip_to_id));

   ep->type = ep2->type;
   ep->mouse_events = ep2->mouse_events;
   ep->anti_alias = ep2->anti_alias;
   ep->repeat_events = ep2->repeat_events;
   ep->ignore_flags = ep2->ignore_flags;
   ep->mask_flags = ep2->mask_flags;
   ep->scale = ep2->scale;
   ep->pointer_mode = ep2->pointer_mode;
   ep->precise_is_inside = ep2->precise_is_inside;
   ep->use_alternate_font_metrics = ep2->use_alternate_font_metrics;
   ep->effect = ep2->effect;
   ep->entry_mode = ep2->entry_mode;
   ep->select_mode = ep2->select_mode;
   ep->cursor_mode = ep2->cursor_mode;
   ep->multiline = ep2->multiline;
   ep->access = ep2->access;
   ep->no_render = ep2->no_render;
   ep->required = ep2->required;
   ep->dragable.x = ep2->dragable.x;
   ep->dragable.step_x = ep2->dragable.step_x;
   ep->dragable.count_x = ep2->dragable.count_x;
   ep->dragable.y = ep2->dragable.y;
   ep->dragable.step_y = ep2->dragable.step_y;
   ep->dragable.count_y = ep2->dragable.count_y;
   ep->nested_children_count = ep2->nested_children_count;

   if (ep2->allowed_seats)
     {
        Edje_Part_Allowed_Seat *seat;
        unsigned int s;

        ep->allowed_seats_count = ep2->allowed_seats_count;
        ep->allowed_seats = calloc(ep2->allowed_seats_count,
                                   sizeof(Edje_Part_Allowed_Seat *));
        if (!ep->allowed_seats)
          {
             ERR("Not enough memory.");
             exit(-1);
          }

        for (s = 0; s < ep->allowed_seats_count; s++)
          {
             seat = mem_alloc(SZ(Edje_Part_Allowed_Seat));
             if (ep2->allowed_seats[s]->name)
               {
                  seat->name = strdup(ep2->allowed_seats[s]->name);
                  if (!seat->name)
                    {
                       ERR("Not enough memory.");
                       exit(-1);
                    }
               }
             ep->allowed_seats[s] = seat;
          }
     }

   data_queue_copied_part_lookup(pc, &(ep2->dragable.confine_id), &(ep->dragable.confine_id));
   data_queue_copied_part_lookup(pc, &(ep2->dragable.threshold_id), &(ep->dragable.threshold_id));
   data_queue_copied_part_lookup(pc, &(ep2->dragable.event_id), &(ep->dragable.event_id));

   epp = (Edje_Part_Parser *)ep;
   epp2 = (Edje_Part_Parser *)ep2;
   epp->reorder.insert_before = STRDUP(epp2->reorder.insert_before);
   epp->reorder.insert_after = STRDUP(epp2->reorder.insert_after);
   epp->can_override = EINA_TRUE;

   for (j = 0; j < ep2->items_count; j++)
     {
        ob_collections_group_parts_part_box_items_item();
        item = ep->items[j];
        item2 = ep2->items[j];
        item->type = item2->type;
        item->name = STRDUP(item2->name);
        item->source = STRDUP(item2->source);
        item->min.w = item2->min.w;
        item->min.h = item2->min.h;
        item->prefer.w = item2->prefer.w;
        item->prefer.h = item2->prefer.h;
        item->max.w = item2->max.w;
        item->max.h = item2->max.h;
        item->padding.l = item2->padding.l;
        item->padding.r = item2->padding.r;
        item->padding.t = item2->padding.t;
        item->padding.b = item2->padding.b;
        item->align.x = item2->align.x;
        item->align.y = item2->align.y;
        item->weight.x = item2->weight.x;
        item->weight.y = item2->weight.y;
        item->aspect.w = item2->aspect.w;
        item->aspect.h = item2->aspect.h;
        item->aspect.mode = item2->aspect.mode;
        item->options = STRDUP(item2->options);
        item->col = item2->col;
        item->row = item2->row;
        item->colspan = item2->colspan;
        item->rowspan = item2->rowspan;
        item->spread.w = item2->spread.w;
        item->spread.h = item2->spread.h;

        pitem = (Edje_Pack_Element_Parser *)item;
        pitem->can_override = EINA_TRUE;

        _parts_count_update(item->type, 1);
     }

   ep->api.name = STRDUP(ep2->api.name);
   if (ep2->api.description) ep->api.description = STRDUP(ep2->api.description);

   // copy default description
   ob_collections_group_parts_part_description();
   ed = ep->default_desc;
   parent_desc = ed2 = ep2->default_desc;
   free((void *)ed->state.name);
   ed->state.name = STRDUP(ed2->state.name);
   ed->state.value = ed2->state.value;
   st_collections_group_parts_part_description_inherit();
   parent_desc = NULL;

   // copy other description
   for (j = 0; j < ep2->other.desc_count; j++)
     {
        ob_collections_group_parts_part_description();
        ed = ep->other.desc[j];
        parent_desc = ed2 = ep2->other.desc[j];
        ed->state.name = STRDUP(ed2->state.name);
        ed->state.value = ed2->state.value;
        st_collections_group_parts_part_description_inherit();
        parent_desc = NULL;
     }
}

/**
    @page edcref
    @property
        inherit_only
    @parameters
        [1 or 0]
    @effect
        This flags a group as being used only for inheriting, which
        will inhibit edje_cc resolving of programs and parts that may
        not exist in this group, but are located in the group which is inheriting
        this group.

        Defaults: 0
    @since 1.10
    @endproperty
 */
static void
st_collections_group_inherit_only(void)
{
   Edje_Part_Collection_Parser *pcp;

   check_arg_count(1);

   pcp = eina_list_data_get(eina_list_last(edje_collections));
   pcp->inherit_only = parse_bool(0);
}

/**
    @page edcref
    @property
        use_custom_seat_names
    @parameters
        [1 or 0]
    @effect
        This flags a group as designed to listen for multiseat signals
        following a custom naming instead of default Edje naming.
        Seats are named on Edje as "seat1", "seat2", etc, in an incremental
        way and never are changed.

        But on Evas, names may be set on different places
        (Evas, Ecore Evas backends, the application itself)
        and name changes are allowed.
        So custom names come from system at first, but can be overriden with
        evas_device_name_set().
        Also Evas seat names don't need to follow any pattern.

        It's useful for cases where there is control of the
        system, as seat names, or when the application
        sets the devices names to guarantee they'll match
        seat names on EDC.

        Defaults: 0
    @since 1.19
    @endproperty
 */
static void
st_collections_group_use_custom_seat_names(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->use_custom_seat_names = parse_bool(0);
}

/**
    @page edcref
    @property
        part_remove
    @parameters
        [part name] (part name) (part name) ...
    @effect
        Removes the listed parts from an inherited group. Removing nonexistent
        parts is not allowed.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_part_remove(void)
{
   unsigned int n, argc, orig_count, part_type;
   Edje_Part_Collection *pc;

   check_min_arg_count(1);

   if (!current_group_inherit)
     {
        ERR("Cannot remove parts from non-inherited group '%s'", current_de->entry);
        exit(-1);
     }

   pc = eina_list_last_data_get(edje_collections);
   orig_count = pc->parts_count;

   for (n = 0, argc = get_arg_count(); n < argc; n++)
     {
        char *name;
        unsigned int j, cur_count = pc->parts_count;

        name = parse_str(n);

        for (j = 0; j < pc->parts_count; j++)
          {
             unsigned int i;

             if (strcmp(pc->parts[j]->name, name)) continue;

             part_type = pc->parts[j]->type;
             pc->parts[j] = _part_free(pc, pc->parts[j]);
             for (i = j; i < pc->parts_count - 1; i++)
               {
                  if (!pc->parts[i + 1]) break;
                  pc->parts[i] = pc->parts[i + 1];
               }
             pc->parts_count--;
             _parts_count_update(part_type, -1);
             break;
          }
        if (cur_count == pc->parts_count)
          {
             ERR("Attempted removal of nonexistent part '%s' in group '%s'.",
                 name, current_de->entry);
             exit(-1);
          }
        free(name);
     }
   if (orig_count == pc->parts_count) return;
   if (pc->parts_count)
     pc->parts = realloc(pc->parts, pc->parts_count * sizeof(Edje_Part *));
   else
     {
        free(pc->parts);
        pc->parts = NULL;
     }
}

/**
    @page edcref
    @property
        program_remove
    @parameters
        [program name] (program name) (program name) ...
    @effect
        Removes the listed programs from an inherited group. Removing nonexistent
        programs is not allowed.
        This will break program sequences if a program in the middle of the sequence is removed.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_program_remove(void)
{
   unsigned int n, argc;
   Edje_Part_Collection *pc;

   check_min_arg_count(1);

   if (!current_group_inherit)
     {
        ERR("Cannot remove programs from non-inherited group '%s'", current_de->entry);
        exit(-1);
     }

   pc = eina_list_last_data_get(edje_collections);

   for (n = 0, argc = get_arg_count(); n < argc; n++)
     {
        char *name;
        Eina_Bool success = EINA_FALSE;

        name = parse_str(n);

        success |= _program_remove(name, pc->programs.fnmatch, pc->programs.fnmatch_count);
        success |= _program_remove(name, pc->programs.strcmp, pc->programs.strcmp_count);
        success |= _program_remove(name, pc->programs.strncmp, pc->programs.strncmp_count);
        success |= _program_remove(name, pc->programs.strrncmp, pc->programs.strrncmp_count);
        success |= _program_remove(name, pc->programs.nocmp, pc->programs.nocmp_count);

        if (anonymous_delete)
          {
             copied_program_anonymous_lookup_delete(pc, anonymous_delete);
             anonymous_delete = NULL;
          }
        if (!success)
          {
             ERR("Attempted removal of nonexistent program '%s' in group '%s'.",
                 name, current_de->entry);
             exit(-1);
          }
        free(name);
     }
}

/**
    @page edcref
    @property
        target_group
    @parameters
        [name] [part or program] (part or program) (part or program) ...
    @effect
        This creates a group of parts/programs which can then be referenced
        by a single 'groups' or 'target_groups' statement inside a program.
        The resulting program will have all of the parts/programs within the specified
        group added as targets.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_target_group(void)
{
   int n, argc;
   Edje_Part_Collection_Parser *pc;
   char *name;
   Eina_List *l;
   Edje_Target_Group *tg;

   check_min_arg_count(2);

   pc = eina_list_last_data_get(edje_collections);
   name = parse_str(0);
   EINA_LIST_FOREACH(pc->target_groups, l, tg)
     if (!strcmp(tg->name, name))
       {
          ERR("parse error %s:%i. There is already a target_group with the name '%s'",
              file_in, line - 1, name);
          exit(-1);
       }
   tg = malloc(sizeof(Edje_Target_Group));
   pc->target_groups = eina_list_append(pc->target_groups, tg);
   tg->name = name;
   argc = get_arg_count();
   tg->targets = calloc(argc, sizeof(char *));

   for (n = 1; n < argc; n++)
     tg->targets[n - 1] = parse_str(n);
}

/**
    @page edcref
    @property
        inherit
    @parameters
        [parent group name]
    @effect
        Parent group name for inheritance.
        Group "inherit" is used to inherit any predefined group and change
        some property which belongs to "part", "description", "items" or "program".
        The child group has the same property as parent group. If you specify the
        type again in an inherited part, it will cause an error (unless you plan
        to fix that).
        @warning When inheriting any parts, descriptions without state names are NOT
        allowed.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_inherit(void)
{
   Edje_Part_Collection_Directory_Entry *alias;
   Edje_Part_Collection *pc, *pc2 = NULL;
   Edje_Part_Collection_Parser *pcp, *pcp2;
   Edje_Part *ep, *ep2;
   Edje_List_Foreach_Data fdata;
   Eina_List *l;
   char *parent_name;
   unsigned int i, j, offset;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   parent_name = parse_str(0);

   EINA_LIST_FOREACH(aliases, l, alias)
     {
        if (alias->group_alias &&
            !strcmp(alias->entry, parent_name))
          {
             free(parent_name);
             pc2 = eina_list_nth(edje_collections, alias->id);
             parent_name = strdup(pc2->part);
             break;
          }
     }

   if (!pc2)
     {
        EINA_LIST_FOREACH(edje_collections, l, pc2)
          {
             if (!strcmp(parent_name, pc2->part))
               break;
          }
     }

   if (!pc2)
     {
        ERR("parse error %s:%i. There isn't a group with the name %s",
            file_in, line - 1, parent_name);
        exit(-1);
     }

   if (pc2 == pc)
     {
        ERR("parse error %s:%i. You are trying to inherit '%s' from itself. That's not possible."
            "If there is another group of the same name, you want to inherit from that group and have the"
            "same name as that group, there is a trick ! Just put the inherit before the directive that set"
            "the name !", file_in, line - 1, parent_name);
        exit(-1);
     }
   current_group_inherit = EINA_TRUE;

   if (pc2->data)
     {
        char *key, *data;

        memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
        eina_hash_foreach(pc2->data,
                          _edje_data_item_list_foreach, &fdata);

        if (!pc->data) pc->data = eina_hash_string_small_new(free);
        EINA_LIST_FREE(fdata.list, key)
          {
             data = eina_hash_find(pc2->data, key);
             eina_hash_direct_add(pc->data, key, data);
          }
     }

   if (pc2->alias)
     {
        char *key;

        memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
        eina_hash_foreach(pc2->alias,
                          _edje_data_item_list_foreach, &fdata);
        if (!pc->alias) pc->alias = eina_hash_string_small_new(free);
        EINA_LIST_FREE(fdata.list, key)
          {
             char *tmp;
             tmp = eina_hash_find(pc2->alias, key);
             eina_hash_direct_add(pc->alias, key, tmp);
          }
     }
   if (pc2->aliased)
     {
        char *key, *aliased;

        memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
        eina_hash_foreach(pc2->aliased,
                          _edje_data_item_list_foreach, &fdata);
        if (!pc->aliased) pc->aliased = eina_hash_string_small_new(free);
        EINA_LIST_FREE(fdata.list, key)
          {
             aliased = eina_hash_find(pc2->aliased, key);
             eina_hash_direct_add(pc->aliased, key, aliased);
          }
     }

#ifdef HAVE_EPHYSICS
   pc->physics.world.gravity.x = pc2->physics.world.gravity.x;
   pc->physics.world.gravity.y = pc2->physics.world.gravity.y;
   pc->physics.world.gravity.z = pc2->physics.world.gravity.z;
   pc->physics.world.depth = pc2->physics.world.depth;
   pc->physics.world.z = pc2->physics.world.z;
   pc->physics.world.rate = pc2->physics.world.rate;
#endif

   pc->prop.min.w = pc2->prop.min.w;
   pc->prop.min.h = pc2->prop.min.h;
   pc->prop.orientation = pc2->prop.orientation;

   pc->lua_script_only = pc2->lua_script_only;
   pc->use_custom_seat_names = pc2->use_custom_seat_names;

   pcp = (Edje_Part_Collection_Parser *)pc;
   pcp2 = (Edje_Part_Collection_Parser *)pc2;
   pcp->default_mouse_events = pcp2->default_mouse_events;
   pcp->skip_namespace_validation = pcp2->skip_namespace_validation;
   if (pcp2->inherit_script)
     pcp->inherit_script = pcp2->inherit_script;

   /* as of 7 April 2014, target groups cannot be modified and are not freed.
    * this code will break if that ever changes.
    *
    * BORKER CERTIFICATION: BRONZE
    */
   if (pcp2->target_groups)
     pcp->target_groups = eina_list_clone(pcp2->target_groups);

   if (pcp2->default_source)
     {
        free(pcp->default_source);
        pcp->default_source = strdup(pcp2->default_source);
     }

   if (pc2->limits.vertical_count || pc2->limits.horizontal_count)
     {
        Edje_Limit **elp;

        if (pc2->limits.vertical_count)
          {
             elp = realloc(pc->limits.vertical,
                           pc->limits.vertical_count + pc2->limits.vertical_count * sizeof(Edje_Limit *));
             if (!elp)
               {
                  ERR("Not enough memory.");
                  exit(-1);
               }
             pc->limits.vertical = elp;
             offset = pc->limits.vertical_count;
             for (i = 0; i < pc2->limits.vertical_count; i++)
               {
                  Edje_Limit *el;

                  el = mem_alloc(SZ(Edje_Limit));
                  if (!el)
                    {
                       ERR("Not enough memory.");
                       exit(-1);
                    }

                  pc->limits.vertical[i + offset] = el;

                  el->name = STRDUP(pc2->limits.vertical[i]->name);
                  el->value = pc2->limits.vertical[i]->value;
                  pc->limits.vertical_count++;
               }
          }

        if (pc2->limits.horizontal_count)
          {
             elp = realloc(pc->limits.horizontal,
                           pc->limits.horizontal_count + pc2->limits.horizontal_count * sizeof(Edje_Limit *));
             if (!elp)
               {
                  ERR("Not enough memory.");
                  exit(-1);
               }
             pc->limits.horizontal = elp;
             offset = pc->limits.horizontal_count;
             for (i = 0; i < pc2->limits.horizontal_count; i++)
               {
                  Edje_Limit *el;

                  el = mem_alloc(SZ(Edje_Limit));
                  if (!el)
                    {
                       ERR("Not enough memory.");
                       exit(-1);
                    }

                  pc->limits.horizontal[i + offset] = el;

                  el->name = STRDUP(pc2->limits.horizontal[i]->name);
                  el->value = pc2->limits.horizontal[i]->value;
                  pc->limits.horizontal_count++;
               }
          }
     }

   offset = pc->parts_count;
   for (i = 0; i < pc2->parts_count; i++)
     {
        // copy the part
        edje_cc_handlers_part_make(-1);
        ep = pc->parts[i + offset];
        ep2 = pc2->parts[i];
        _part_copy(ep, ep2);
     }

   //copy programs
   for (j = 0; j < pc2->programs.fnmatch_count; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.fnmatch[j]);
     }
   for (j = 0; j < pc2->programs.strcmp_count; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.strcmp[j]);
     }
   for (j = 0; j < pc2->programs.strncmp_count; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.strncmp[j]);
     }
   for (j = 0; j < pc2->programs.strrncmp_count; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.strrncmp[j]);
     }
   for (j = 0; j < pc2->programs.nocmp_count; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.nocmp[j]);
     }

   Code *cd, *cd2;
   Code_Program *cp, *cp2;
   Edje_Part_Collection_Directory_Entry *de;

   de = eina_hash_find(edje_file->collection, pc2->part);
   cd2 = eina_list_nth(codes, de->id);
   cd = eina_list_data_get(eina_list_last(codes));

   cd->is_lua = cd2->is_lua;
   if (!cd2->is_lua)
     pcp->base_codes = eina_list_append(pcp->base_codes, cd2);

   if (cd2->shared)
     {
        if (cd->shared)
          {
             WRN("%s:%i. script block in group \"%s\" will be overwritten by inheriting "
                 "from group \"%s\".", file_in, line - 1, pc->part, pc2->part);
             free(cd->shared);
          }
        if (cd->original)
          free(cd->original);

        cd->shared = STRDUP(cd2->shared);
        cd->original = STRDUP(cd2->original);

        script_is_replaceable = EINA_TRUE;
     }

   EINA_LIST_FOREACH(cd2->programs, l, cp2)
     {
        cp = mem_alloc(SZ(Code_Program));

        cp->l1 = cp2->l1;
        cp->l2 = cp2->l2;
        cp->script = STRDUP(cp2->script);
        cp->original = STRDUP(cp2->original);
        cd->programs = eina_list_append(cd->programs, cp);
        data_queue_copied_anonymous_lookup(pc, &(cp2->id), &(cp->id));
     }

   free(parent_name);
}

/**
    @page edcref
    @property
        lua_script_only
    @parameters
        [on/off]
    @effect
        The flag (on/off) as to if this group is defined ONLY by script
        callbacks such as init(), resize() and shutdown()

        Defaults: off
    @endproperty
 */
static void
st_collections_group_lua_script_only(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->lua_script_only = parse_bool(0);
}

/**
    @page edcref
    @property
        script_recursion
    @parameters
        [1/0]
    @effect
        This flag (1/0) determines whether to error on unsafe calls when
        recursively running Embryo programs.
        For example, running an Embryo script which calls EDC which has a
        script{} block is unsafe, and the outer-most (first) Embryo stack is GUARANTEED
        to be corrupted. Only use this flag if you are sure that you know what you are doing.

        Defaults: 0
    @since 1.10
    @endproperty
 */
static void
st_collections_group_script_recursion(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->script_recursion = parse_bool(0);
}

/**
    @page edcref
    @property
        alias
    @parameters
        [aditional group name]
    @effect
        Additional name to serve as identifier. Defining multiple aliases is
        supported.
    @endproperty
 */
static void
st_collections_group_alias(void)
{
   Edje_Part_Collection_Directory_Entry *alias;
   Edje_Part_Collection_Directory_Entry *tmp;
   Eina_List *l;

   check_arg_count(1);

   alias = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   alias->id = current_de->id;
   alias->entry = parse_str(0);
   alias->group_alias = EINA_TRUE;

   EINA_LIST_FOREACH(aliases, l, tmp)
     if (strcmp(alias->entry, tmp->entry) == 0)
       {
          Edje_Part_Collection *pc;

          pc = eina_list_nth(edje_collections, tmp->id);
          INF("overriding alias ('%s' => '%s') to ('%s' => '%s')",
              tmp->entry, pc->part,
              alias->entry, current_de->entry);
          aliases = eina_list_remove_list(aliases, l);
          free(tmp);
          break;
       }

   aliases = eina_list_append(aliases, alias);
}

/**
    @page edcref
    @property
        min
    @parameters
        [width] [height]
    @effect
        The minimum size for the container defined by the composition of the
        parts. It is not enforced.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_min(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->prop.min.w = parse_int_range(0, 0, 0x7fffffff);
   pc->prop.min.h = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height]
    @effect
        The maximum size for the container defined by the totality of the
        parts. It is not enforced.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_max(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->prop.max.w = parse_int_range(0, 0, 0x7fffffff);
   pc->prop.max.h = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        scene_size
    @parameters
        [width] [height]
    @effect
        Size of scene.

        Defaults: 0.0 0.0
    @endproperty
 */
static void
st_collections_group_scene_size(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->scene_size.width = parse_float(0);
   pc->scene_size.height = parse_float(1);
}

/**
   @page edcref
   @property
       broadcast_signal
   @parameters
       [on/off]
   @effect
       Signal got automatically broadcasted to all sub group part.

       Defaults: true
   @since 1.1
   @endproperty
 */
static void
st_collections_group_broadcast_signal(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->broadcast_signal = parse_bool(0);
}

static void
st_collections_group_broadcast(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(0);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->broadcast_signal = 1;
}

static void
st_collections_group_nobroadcast(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(0);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->broadcast_signal = 0;
}

/**
    @page edcref
    @property
        orientation
    @parameters
        [AUTO/LTR/RTL]
    @effect
        This defines GROUP orientation.
        This is useful if you want match interface orientation with language.
        @li AUTO  - Follow system defs.
        @li LTR  - suitable for Left To Right Languages (latin)
        @li RTL - suitable for Right To Left Languages (Hebrew, Arabic interface)

        Defaults: AUTO
    @endproperty
 */
static void
st_collections_group_orientation(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->prop.orientation = parse_enum(0,
                                     "AUTO", EDJE_ORIENTATION_AUTO,
                                     "LTR", EDJE_ORIENTATION_LTR,
                                     "RTL", EDJE_ORIENTATION_RTL,
                                     NULL);
}

/**
    @page edcref
    @property
        mouse_events
    @parameters
        [1 or 0]
    @effect
        Change the default value of mouse_events for every part in this group.

        Defaults: 1 (to maintain compatibility)
    @endproperty
 */
static void
st_collections_group_mouse_events(void)
{
   Edje_Part_Collection_Parser *pcp;

   check_arg_count(1);

   pcp = eina_list_data_get(eina_list_last(edje_collections));
   pcp->default_mouse_events = parse_bool(0);
}

static void
st_collections_group_mouse(void)
{
   Edje_Part_Collection_Parser *pcp;

   check_arg_count(0);

   pcp = eina_list_data_get(eina_list_last(edje_collections));
   pcp->default_mouse_events = 1;
}

static void
st_collections_group_nomouse(void)
{
   Edje_Part_Collection_Parser *pcp;

   check_arg_count(0);

   pcp = eina_list_data_get(eina_list_last(edje_collections));
   pcp->default_mouse_events = 0;
}

/**
    @page edcref
    @property
        inherit_script
    @parameters
        [1 or 0]
    @effect
        Determine whether to inherit script block from parent group.
        If it is set to 0, script from parent group will be replaced with
        new script block.
        Defaults to 0 if not set, to maintain compatibility.
    @endproperty
 */
static void
st_collections_group_inherit_script(void)
{
   Edje_Part_Collection_Parser *pcp;

   pcp = eina_list_last_data_get(edje_collections);

   if (get_arg_count() == 1)
     pcp->inherit_script = parse_bool(0);
   else
     pcp->inherit_script = EINA_TRUE;
}

static void
st_collections_group_noinherit_script(void)
{
   Edje_Part_Collection_Parser *pcp;

   check_arg_count(0);

   pcp = eina_list_last_data_get(edje_collections);
   pcp->inherit_script = EINA_FALSE;
}

static void
_script_flush(void)
{
   Edje_Part_Collection_Parser *pcp;
   Code *code;

   pcp = eina_list_last_data_get(edje_collections);
   code = eina_list_last_data_get(codes);

   if (!pcp->inherit_script || code->is_lua) return;

   // If script is replaceable and overridable, code->shared will be inherited
   // script. Free it to avoid duplication.
   if (script_is_replaceable)
     {
        if (code->shared)
          {
             free(code->shared);
             code->shared = NULL;
          }
        if (code->original)
          {
             free(code->original);
             code->original = NULL;
          }
     }

   script_rewrite(code);

   eina_list_free(pcp->base_codes);
}

/**
    @page edcref
    @property
        program_source
    @parameters
        [source name]
    @effect
        Change the default value of source for every program in the current group
        which is declared after this value is set.

        Defaults: an unset value (to maintain compatibility)
    @since 1.10
    @endproperty
 */
static void
st_collections_group_program_source(void)
{
   Edje_Part_Collection_Parser *pcp;

   check_arg_count(1);

   pcp = eina_list_last_data_get(edje_collections);
   free(pcp->default_source);
   pcp->default_source = parse_str(0);
}

/** @edcsubsection{collections_group_script,
 *                 Group.Script} */

/**
    @page edcref
    @block
        script
    @context
        ..
        group {
            script {
                //embryo script
            }
            ..
            program {
                script {
                    //embryo script
                }
            }
            ..
        }
        ..
    @description
        This block is used to "inject" embryo scripts to a given Edje theme and
        it functions in two modalities. When it's included inside a "program"
        block, the script will be executed every time the program is run, on
        the other hand, when included directly into a "group", "part" or
        "description" block, it will be executed once at load time, in the
        load order.
    @endblock
 */
static void
ob_collections_group_script(void)
{
   Code *cd;

   cd = eina_list_data_get(eina_list_last(codes));

   if (!is_verbatim()) track_verbatim(1);
   else
     {
        char *s;

        s = get_verbatim();
        if (s)
          {
             cd->l1 = get_verbatim_line1();
             cd->l2 = get_verbatim_line2();
             if (cd->shared)
               {
                  if (script_is_replaceable)
                    {
                       free(cd->shared);
                       free(cd->original);
                       script_is_replaceable = EINA_FALSE;
                    }
                  else
                    {
                       ERR("parse error %s:%i. There is already an existing script section for the group",
                           file_in, line - 1);
                       exit(-1);
                    }
               }
             cd->shared = s;
             cd->original = strdup(s);
             cd->is_lua = 0;
             set_verbatim(NULL, 0, 0);
          }
     }
}

static void
ob_collections_group_lua_script(void)
{
   Code *cd;

   cd = eina_list_data_get(eina_list_last(codes));

   if (!is_verbatim()) track_verbatim(1);
   else
     {
        char *s;

        s = get_verbatim();
        if (s)
          {
             cd->l1 = get_verbatim_line1();
             cd->l2 = get_verbatim_line2();
             if (cd->shared)
               {
                  ERR("parse error %s:%i. There is already an existing script section for the group",
                      file_in, line - 1);
                  exit(-1);
               }
             cd->shared = s;
             cd->is_lua = 1;
             set_verbatim(NULL, 0, 0);
          }
     }
}

/** @edcsubsection{collections_group_data,
 *                 Group.Data} */

/**
    @page edcref
    @block
        data
    @context
        data {
            item: "key" "value";
            ..
        }
    @description
        The "data" block is used to pass arbitrary parameters from the theme to
        the application. Unlike the toplevel data block, this block Group.Data
        can only store inline items (not files).
        See also the toplevel @ref sec_toplevel_data "Data" section.
    @endblock

    @property
        item
    @parameters
        [parameter name] [parameter value]
    @effect
        Defines a new parameter, the value will be the string specified next to
        it.
    @endproperty
 */

static void
st_collections_group_data_item(void)
{
   Edje_Part_Collection *pc;
   Edje_String *es;
   char *key;

   check_arg_count(2);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (!pc->data)
     pc->data = eina_hash_string_small_new(free);

   key = parse_str(0);

   es = mem_alloc(SZ(Edje_String));
   es->str = parse_str(1);

   if (eina_hash_find(pc->data, key))
     eina_hash_modify(pc->data, key, es);
   else
     eina_hash_direct_add(pc->data, key, es);
}

/** @edcsubsection{collections_group_filters,
 *                 Group.Filters} */

/**
    @page edcref
    @block
        filters
    @context
        // (toplevel)
        // collections
        // collections.group
        filters {
            // Inline as script block:
            filter {
               name: "myfilter1";
               script {
                  -- Some Lua code here, eg:
                  blend { color = 'red' }
               }
            // Imported from an external file:
            filter {
               name: "myfilter2";
               file: "filename.lua";
            }
        }
    @description
        The "filter" block lets you embed filter scripts into an EDC group,
        that can then be referred to in the
        @ref sec_collections_group_parts_description_filter "Text.Filter"
        or @ref sec_collections_group_parts_description_filter "Image.Filter"
        statements.

        In a similar way to the toplevel @ref sec_toplevel_data "Data" section,
        it is possible to embed filters from a external file inside the final EDJ.

        Note that filters are defined globally, even if they appear inside a
        specific group (as of EFL 1.19).

        Please also refer to @ref evasfiltersref "Evas filters reference".
    @endblock

    @property
        name
    @parameters
        [name]
    @effect
        Creates a new named filter. This filter can then be used in image, text
        or textblock parts by name.
    @endproperty

    @property
        script
    @parameters
        [Lua script]
    @effect
        A block of Lua code contained inside {}. Example: script { blur{5} }
    @endproperty

    @property
        file
    @parameters
        [Path to Lua file]
    @effect
        Includes an external file to define a new Lua script used for filtering.
        The file must be in the data path passed to edje_cc (-dd argument).
    @endproperty
 */

static Edje_Gfx_Filter *current_filter = NULL;

static void
_filters_filter_sort(void)
{
   Edje_Gfx_Filter *array, current;
   int new_pos, i, cur_pos;

   if (!current_filter)
     return;

   if (!current_filter->name)
     {
        ERR("parse error %s:%i. Filter has no name.",
            file_in, line - 1);
        exit(-1);
     }

   array = edje_file->filter_dir->filters;
   cur_pos = (current_filter - array);

   // find position in sorted array
   for (new_pos = 0; new_pos < edje_file->filter_dir->filters_count - 1; new_pos++)
     {
        int cmp;
        if (cur_pos == new_pos)
          continue;
        cmp = strcmp(current_filter->name, array[new_pos].name);
        if (cmp == 0)
          {
             ERR("parse error %s:%i. Another filter named '%s' already exists.",
                 file_in, line - 1, array[new_pos].name);
             exit(-1);
          }
        else if (cmp < 0)
          break;
     }

   if (new_pos > cur_pos)
     new_pos--;

   if (cur_pos == new_pos)
     return;

   current = *current_filter;

   // move up
   for (i = cur_pos - 1; i >= new_pos; i--)
     array[i + 1] = array[i];

   // move down
   for (i = cur_pos; i < new_pos; i++)
     array[i] = array[i + 1];

   array[new_pos] = current;
   current_filter = &array[new_pos];
}

static void
ob_filters_filter(void)
{
   Edje_Gfx_Filter *array;

   _filters_filter_sort();
   if (!edje_file->filter_dir)
     edje_file->filter_dir = mem_alloc(sizeof(Edje_Gfx_Filter_Directory));

   array = realloc(edje_file->filter_dir->filters,
                   sizeof(Edje_Gfx_Filter) * (edje_file->filter_dir->filters_count + 1));
   if (!array)
     {
        ERR("Memory allocation failed (array grow)");
        exit(-1);
     }

   current_filter = &array[edje_file->filter_dir->filters_count];
   memset(current_filter, 0, sizeof(Edje_Gfx_Filter));
   edje_file->filter_dir->filters_count++;
   edje_file->filter_dir->filters = array;
}

static void
ob_filters_filter_script(void)
{
   char *script;

   if (!current_filter)
     ob_filters_filter();

   if (!current_filter->name)
     {
        ERR("parse error %s:%i. Name for inline filter must be specified first.",
            file_in, line - 1);
        exit(-1);
     }

   if (current_filter->script)
     {
        ERR("parse error %s:%i. Script for filter '%s' is already defined.",
            file_in, line - 1, current_filter->name);
        exit(-1);
     }

   if (!is_verbatim())
     track_verbatim(1);
   else
     {
        script = get_verbatim();
        if (script)
          {
             //current_filter->verb_l1 = get_verbatim_line1();
             //current_filter->verb_l2 = get_verbatim_line2();
             current_filter->script = strdup(script);
             set_verbatim(NULL, 0, 0);
             _filters_filter_sort();
             current_filter = NULL;
          }
     }
}

static void
st_filters_filter_file(void)
{
   char *file, *script;
   Eina_File *f;
   size_t sz;

   if (!current_filter)
     ob_filters_filter();

   if (current_filter->script)
     {
        ERR("parse error %s:%i. Script for filter '%s' is already defined.",
            file_in, line - 1, current_filter->name);
        exit(-1);
     }

   check_arg_count(1);

   file = parse_str(0);
   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
        char path[PATH_MAX], *dir;
        Eina_List *l;
        // TODO: Introduce special filters_dir? needs new edje_cc argument :(
        EINA_LIST_FOREACH(data_dirs, l, dir)
          {
             snprintf(path, sizeof(path), "%s/%s", dir, file);
             f = eina_file_open(path, EINA_FALSE);
             if (f) break;
          }
        if (!f)
          {
             ERR("parse error %s:%i. Could not open filter script file '%s'",
                 file_in, line - 1, file);
             exit(-1);
          }
     }

   script = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!script)
     {
        ERR("parse error %s:%i. Could not read filter script file %s",
            file_in, line - 1, file);
        exit(-1);
     }

   sz = eina_file_size_get(f);
   if (sz > (10 * 1024 * 1024))
     {
        ERR("parse error %s:%i. Filter file '%s' is unreasonably large, abort.",
            file_in, line - 1, file);
        exit(-1);
     }

   current_filter->script = (char *)eina_memdup((unsigned char *)script, sz, 1);
   eina_file_map_free(f, script);
   eina_file_close(f);

   if (!current_filter->name)
     {
        current_filter->name = file;
        _filters_filter_sort();
     }
   else
     {
        free(file);
        _filters_filter_sort();
        current_filter = NULL;
     }
}

static void
st_filters_filter_name(void)
{
   if (!current_filter)
     ob_filters_filter();

   check_arg_count(1);

   current_filter->name = parse_str(0);

   _filters_filter_sort();
}

/** @edcsubsection{collections_group_limits,
 *                 Group.Limits} */

/**
    @page edcref
    @block
        limits
    @context
        group {
            limits {
                vertical: "limit_name" height_barrier;
                horizontal: "limit_name" width_barrier;
                ..
            }
            ..
        }
        ..
    @description
        This block is used to trigger some signal when the Edje object is resized.
    @endblock

    @page edcref
    @property
        vertical
    @parameters
        [name] [height barrier]
    @effect
        It will send a signal: "limit,name,over" when the object is resized and pass
        the limit by growing over it. And it will send: "limit,name,below" when
        it pass below that limit.
        This limit will be applied on the y absis and is expressed in pixels.
    @endproperty
 */
static void
st_collections_group_limits_vertical(void)
{
   Edje_Part_Collection *pc;
   Edje_Limit *el, **elp;

   check_arg_count(2);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->limits.vertical_count++;
   elp = realloc(pc->limits.vertical,
                 pc->limits.vertical_count * sizeof (Edje_Limit *));
   if (!elp)
     {
        ERR("Not enough memory.");
        exit(-1);
     }
   pc->limits.vertical = elp;

   el = mem_alloc(SZ(Edje_Limit));
   if (!el)
     {
        ERR("Not enough memory.");
        exit(-1);
     }

   pc->limits.vertical[pc->limits.vertical_count - 1] = el;

   el->name = parse_str(0);
   el->value = parse_int_range(1, 1, 0xffff);
}

/**
    @page edcref
    @property
        horizontal
    @parameters
        [name] [width barrier]
    @effect
        It will send a signal: "limit,name,over" when the object is resized and pass
        the limit by growing over it. And it will send: "limit,name,below" when
        it pass below that limit.
        This limit will be applied on the x axis and is expressed in pixels.
    @endproperty
 */
static void
st_collections_group_limits_horizontal(void)
{
   Edje_Part_Collection *pc;
   Edje_Limit *el;

   check_arg_count(2);

   el = mem_alloc(SZ(Edje_Limit));

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->limits.horizontal_count++;
   pc->limits.horizontal = realloc(pc->limits.horizontal, pc->limits.horizontal_count * sizeof (Edje_Limit *));
   if (!pc->limits.horizontal || !el)
     {
        ERR("Not enough memory.");
        exit(-1);
     }

   pc->limits.horizontal[pc->limits.horizontal_count - 1] = el;

   el->name = parse_str(0);
   el->value = parse_int_range(1, 1, 0xffff);
}

/** @edcsubsection{collections_group_parts,
 *                 Group.Parts} */

/**
    @page edcref
    @block
        parts
    @context
        group {
            parts {
                alias: "theme_part_path" "somegroup:real_part_path";
                part { "theme_part_path"; }
                part {  }
                ..
            }
        }
    @description
        The parts block is the container for all the parts in the group.

    @property
        alias
    @parameters
        [alias name] [other_group:part name]
    @effect
        Allows for a part to be referenced externally as though
        it had the name of the alias.
        In the above example, swallowing an object into part "theme_part_path"
        will result in the object actually being swallowed into the part
        "real_part_path" in the "somegroup" group.
    @endproperty
    @endblock
 */
static void
st_collections_group_parts_alias(void)
{
   Edje_Part_Collection *pc;
   const char *alias;
   const char *aliased;

   check_arg_count(2);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   alias = parse_str(0);
   aliased = parse_str(1);

   if (!pc->alias) pc->alias = eina_hash_string_small_new(NULL);
   eina_hash_add(pc->alias, alias, aliased);

   if (!pc->aliased) pc->aliased = eina_hash_string_small_new(NULL);
   eina_hash_add(pc->aliased, aliased, alias);
}

/** @edcsubsection{collections_group_parts_part,
 *                 Group.Parts.Part} */

/**
    @page edcref
    @block
        part
    @context
        group {
            parts {
                ..
                part {
                    name: "partname";
                    type: IMAGE;
                    mouse_events:  1;
                    repeat_events: 0;
                    ignore_flags: NONE;
                    mask_flags: NONE;
                    clip_to: "anotherpart";
                    source:  "groupname";
                    pointer_mode: AUTOGRAB;
                    use_alternate_font_metrics: 0;
                    no_render: 0;

                    dragable { }
                    items { }
                    description { }
                }
                ..
            }
        }
    @description
        Parts are used to represent the most basic design elements of the
        theme, for example, a part can represent a line in a border or a label
        on a button.
    @endblock
 */
static Edje_Part *
edje_cc_handlers_part_make(int id)
{  /* Doing ob_collections_group_parts_part() job, without hierarchy */
  Edje_Part_Collection *pc;
  Edje_Part_Collection_Parser *pcp;
  Edje_Part *ep;
  Edje_Part_Parser *epp;

  ep = mem_alloc(SZ(Edje_Part_Parser));

  pc = eina_list_data_get(eina_list_last(edje_collections));
  if (id < 0)
    {
       pc->parts_count++;
       pc->parts = realloc(pc->parts, pc->parts_count * sizeof (Edje_Part *));
       if (!pc->parts)
         {
            ERR("Not enough memory.");
            exit(-1);
         }
       id = pc->parts_count - 1;
    }

  current_part = pc->parts[id] = ep;
  pcp = (Edje_Part_Collection_Parser *)pc;

  ep->id = id;
  ep->type = EDJE_PART_TYPE_IMAGE;
  ep->mouse_events = pcp->default_mouse_events;
  ep->anti_alias = 1;
  ep->repeat_events = 0;
  ep->ignore_flags = EVAS_EVENT_FLAG_NONE;
  ep->mask_flags = EVAS_EVENT_FLAG_NONE;
  ep->scale = 0;
  ep->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
  ep->precise_is_inside = 0;
  ep->use_alternate_font_metrics = 0;
  ep->access = 0;
  ep->clip_to_id = -1;
  ep->no_render = 0;
  ep->required = 0;
  ep->dragable.confine_id = -1;
  ep->dragable.threshold_id = -1;
  ep->dragable.event_id = -1;
  ep->items = NULL;
  ep->nested_children_count = 0;

  ep->allowed_seats = NULL;
  ep->allowed_seats_count = 0;

  epp = (Edje_Part_Parser *)ep;
  epp->reorder.insert_before = NULL;
  epp->reorder.insert_after = NULL;
  epp->reorder.before = NULL;
  epp->reorder.after = NULL;
  epp->reorder.linked_prev = 0;
  epp->reorder.linked_next = 0;
  epp->reorder.done = EINA_FALSE;
  epp->can_override = EINA_FALSE;

  return ep;
}

static void *
_part_desc_free(Edje_Part_Collection *pc,
                Edje_Part *ep,
                Edje_Part_Description_Common *ed)
{
   if (!ed) return NULL;

   eina_hash_del_by_key(desc_hash, &ed);

   part_lookup_del(pc, &(ed->rel1.id_x));
   part_lookup_del(pc, &(ed->rel1.id_y));
   part_lookup_del(pc, &(ed->rel2.id_x));
   part_lookup_del(pc, &(ed->rel2.id_y));
   part_lookup_del(pc, &(ed->clip_to_id));
   part_lookup_del(pc, &(ed->map.id_persp));
   part_lookup_del(pc, &(ed->map.id_light));
   part_lookup_del(pc, &(ed->map.rot.id_center));

   switch (ep->type)
     {
      case EDJE_PART_TYPE_SPACER:
      case EDJE_PART_TYPE_RECTANGLE:
      case EDJE_PART_TYPE_SWALLOW:
      case EDJE_PART_TYPE_GROUP:
        /* Nothing todo, this part only have a common description. */
        break;

      case EDJE_PART_TYPE_BOX:
      case EDJE_PART_TYPE_TABLE:
      case EDJE_PART_TYPE_IMAGE:
      case EDJE_PART_TYPE_SNAPSHOT:
      case EDJE_PART_TYPE_VECTOR:
        /* Nothing todo here */
        break;

      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
      {
         Edje_Part_Description_Text *ted = (Edje_Part_Description_Text *)ed;

         part_lookup_del(pc, &(ted->text.id_source));
         part_lookup_del(pc, &(ted->text.id_text_source));
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ped = (Edje_Part_Description_Proxy *)ed;

         part_lookup_del(pc, &(ped->proxy.id));
         break;
      }
     }

   free((void *)ed->state.name);
   free(ed);
   return NULL;
}

static void
_part_type_set(unsigned int type)
{
   /* handle type change of inherited part */
   if (type != current_part->type)
     {
        Edje_Part_Description_Common *new, *previous, *cur;
        Edje_Part_Collection *pc;
        Edje_Part *ep, *dummy;
        unsigned int i;

        /* we don't free old part as we don't remove all reference to them */
        part_description_image_cleanup(current_part);
        current_part->type = type;

        pc = eina_list_data_get(eina_list_last(edje_collections));
        ep = current_part;

        previous = ep->default_desc;
        cur = current_desc;
        dummy = mem_alloc(SZ(Edje_Part));
        /* ensure type is incompatible with new type */
        dummy->type = ep->type + 2;
        if (previous)
          {
             new = _edje_part_description_alloc(type, pc->part, ep->name);
             eina_hash_add(desc_hash, &new, ep);
             eina_hash_set(desc_hash, &previous, dummy);
             parent_desc = previous;
             current_desc = new;
             new->state.name = strdup(previous->state.name);
             new->state.value = previous->state.value;
             st_collections_group_parts_part_description_inherit();
             parent_desc = NULL;
             _part_desc_free(pc, ep, previous);

             ep->default_desc = new;
          }

        for (i = 0; i < ep->other.desc_count; i++)
          {
             previous = ep->other.desc[i];
             new = _edje_part_description_alloc(type, pc->part, ep->name);
             eina_hash_add(desc_hash, &new, ep);
             eina_hash_set(desc_hash, &previous, dummy);
             parent_desc = previous;
             current_desc = new;
             new->state.name = strdup(previous->state.name);
             new->state.value = previous->state.value;
             st_collections_group_parts_part_description_inherit();
             parent_desc = NULL;
             _part_desc_free(pc, ep, previous);
             ep->other.desc[i] = new;
          }
        free(dummy);
        current_desc = cur;
     }

   _parts_count_update(current_part->type, 1);
}

static void
_part_create(void)
{
   Edje_Part *cp = current_part;  /* Save to restore on pop    */
   Edje_Part *ep = edje_cc_handlers_part_make(-1); /* This changes current_part */
   Edje_Part *prnt;

   /* Add this new part to hierarchy stack (removed part finished parse) */
   edje_cc_handlers_hierarchy_push(ep, cp);

   prnt = edje_cc_handlers_hierarchy_parent_get();
   if (prnt) /* This is the child of parent in stack */
     prnt->nested_children_count++;
}

static void
ob_collections_group_parts_part_short(void)
{
   unsigned int type;

   type = parse_enum(-1,
                     "none", EDJE_PART_TYPE_NONE,
                     "rect", EDJE_PART_TYPE_RECTANGLE,
                     "text", EDJE_PART_TYPE_TEXT,
                     "image", EDJE_PART_TYPE_IMAGE,
                     "swallow", EDJE_PART_TYPE_SWALLOW,
                     "textblock", EDJE_PART_TYPE_TEXTBLOCK,
                     "group", EDJE_PART_TYPE_GROUP,
                     "box", EDJE_PART_TYPE_BOX,
                     "table", EDJE_PART_TYPE_TABLE,
                     "external", EDJE_PART_TYPE_EXTERNAL,
                     "proxy", EDJE_PART_TYPE_PROXY,
                     "spacer", EDJE_PART_TYPE_SPACER,
                     "snapshot", EDJE_PART_TYPE_SNAPSHOT,
                     "vector", EDJE_PART_TYPE_VECTOR,
                     NULL);

   stack_replace_quick("part");
   _part_create();
   _part_type_set(type);
}

static void
ob_collections_group_parts_part(void)
{
   _part_create();
}

static void *
_part_free(Edje_Part_Collection *pc, Edje_Part *ep)
{
   Edje_Part_Parser *epp = (Edje_Part_Parser *)ep;
   unsigned int j;

   part_lookup_del(pc, &(ep->clip_to_id));
   part_lookup_del(pc, &(ep->dragable.confine_id));
   part_lookup_del(pc, &(ep->dragable.threshold_id));
   part_lookup_del(pc, &(ep->dragable.event_id));

   _part_desc_free(pc, ep, ep->default_desc);
   for (j = 0; j < ep->other.desc_count; j++)
     _part_desc_free(pc, ep, ep->other.desc[j]);

   for (j = 0; j < ep->items_count; j++)
     free(ep->items[j]);
   free(ep->items);

   for (j = 0; j < ep->allowed_seats_count; j++)
     {
        free((void *)(ep->allowed_seats[j]->name));
        free(ep->allowed_seats[j]);
     }
   free(ep->allowed_seats);

   free((void *)ep->name);
   free((void *)ep->source);
   free((void *)ep->source2);
   free((void *)ep->source3);
   free((void *)ep->source4);
   free((void *)ep->source5);
   free((void *)ep->source6);

   free((void *)epp->reorder.insert_before);
   free((void *)epp->reorder.insert_after);

   free((void *)ep->api.name);
   free((void *)ep->api.description);

   free(ep->other.desc);
   free(ep);
   return NULL;
}

/**
    @page edcref
    @property
        inherit
    @parameters
        [part name]
    @effect
        Copies all attributes except part name from referenced part into current part.
        ALL existing attributes, except part name, are overwritten.
        @warning When inheriting any parts, descriptions without state names are NOT
        allowed.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_parts_part_inherit(void)
{
   char *name;
   Edje_Part_Collection *pc;
   unsigned int i;

   check_arg_count(1);

   name = parse_str(0);
   pc = eina_list_data_get(eina_list_last(edje_collections));
   for (i = 0; i < pc->parts_count; i++)
     {
        int id = current_part->id;
        const char *pname;

        if (strcmp(pc->parts[i]->name, name)) continue;
        if (pc->parts[i] == current_part)
          {
             ERR("Cannot inherit from same part '%s' in group '%s'", name, current_de->entry);
             free(name);
             exit(-1);
          }
        pname = current_part->name;
        current_part->name = NULL;
        current_part = _part_free(pc, current_part);
        edje_cc_handlers_part_make(id);
        _part_copy(current_part, pc->parts[i]);
        free((void *)current_part->name);
        current_part->name = pname;
        free(name);
        return;
     }

   ERR("Cannot inherit non-existing part '%s' in group '%s'", name, current_de->entry);
   free(name);
   exit(-1);
}

static void
_program_free(Edje_Program *pr)
{
   Edje_Part_Collection *pc;
   Edje_Program_Target *prt;
   Edje_Program_After *pa;

   pc = eina_list_last_data_get(edje_collections);

   free((void *)pr->name);
   free((void *)pr->signal);
   free((void *)pr->source);
   free((void *)pr->filter.part);
   free((void *)pr->filter.state);
   free((void *)pr->seat);
   free((void *)pr->state);
   free((void *)pr->state2);
   free((void *)pr->sample_name);
   free((void *)pr->tone_name);
   EINA_LIST_FREE(pr->targets, prt)
     {
        part_lookup_del(pc, &prt->id);
        free(prt);
     }
   EINA_LIST_FREE(pr->after, pa)
     free(pa);
   free(pr);
}

static Eina_Bool
_program_remove(const char *name, Edje_Program **pgrms, unsigned int count)
{
   unsigned int i;
   Edje_Part_Collection *pc;

   pc = eina_list_last_data_get(edje_collections);

   for (i = 0; i < count; ++i)
     if (pgrms[i]->name && (!strcmp(name, pgrms[i]->name)))
       {
          Edje_Program *pr = pgrms[i];

          if (pr->after)
            {
               Eina_List *l;
               Edje_Program_After *pa;

               EINA_LIST_FOREACH(pr->after, l, pa)
                 {
                    copied_program_lookup_delete(pc, (char *)(pa + 1));
                 }
            }

          _edje_program_remove(pc, pr);

          if (pr->action == EDJE_ACTION_TYPE_SCRIPT)
            {
               anonymous_delete = &pr->id;
            }

          _program_free(pr);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

static Eina_Bool
_part_name_check(void)
{
   unsigned int i;
   Edje_Part_Collection *pc;
   Edje_Part *ep = current_part;

   if (!ep->name) return EINA_FALSE;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   for (i = 0; i < (pc->parts_count - 1); i++)
     {  /* Compare name only if did NOT updated ep from hircy pop */
       if ((ep != pc->parts[i]) &&
           (pc->parts[i]->name &&
            (!strcmp(pc->parts[i]->name, ep->name))))
         {
            Edje_Part_Parser *epp;

            epp = (Edje_Part_Parser *)pc->parts[i];
            if (!epp->can_override)
              {
                 ERR("parse error %s:%i. There is already a part of the name %s",
                     file_in, line - 1, ep->name);
                 exit(-1);
              }
            else
              {
                 pc->parts_count--;
                 pc->parts = realloc(pc->parts, pc->parts_count * sizeof (Edje_Part *));
                 current_part = pc->parts[i];
                 edje_cc_handlers_hierarchy_rename(ep, current_part);
                 free(ep);
                 epp->can_override = EINA_FALSE;
                 break;
              }
         }
     }
   return EINA_TRUE;
}

/**
    @page edcref
    @property
        name
    @parameters
        [part name]
    @effect
        The part's name will be used as reference in the theme's relative
        positioning system, by programs and in some cases by the application.
        It must be unique within the group.
    @endproperty
 */
static void
st_collections_group_parts_part_name(void)
{
   Edje_Part *ep;

   check_arg_count(1);

   ep = current_part;
   ep->name = parse_str(0);
   _part_name_check();
}

/**
    @page edcref
    @property
        type
    @parameters
        [TYPE]
    @effect
        Set the type (all caps) from among the available types, it's set to
        IMAGE by default. Valid types:
            @li RECT
            @li TEXT
            @li IMAGE
            @li SWALLOW
            @li TEXTBLOCK
            @li GROUP
            @li BOX
            @li TABLE
            @li EXTERNAL
            @li MESH_NODE
            @li CAMERA
            @li LIGHT
            @li PROXY
            @li SPACER
            @li SNAPSHOT

         Defaults: IMAGE
    @endproperty
 */
static void
st_collections_group_parts_part_type(void)
{
   unsigned int type;
   unsigned int i = 0;

   Edje_Part_Collection *pc;

   check_arg_count(1);

   type = parse_enum(0,
                     "NONE", EDJE_PART_TYPE_NONE,
                     "RECT", EDJE_PART_TYPE_RECTANGLE,
                     "TEXT", EDJE_PART_TYPE_TEXT,
                     "IMAGE", EDJE_PART_TYPE_IMAGE,
                     "SWALLOW", EDJE_PART_TYPE_SWALLOW,
                     "TEXTBLOCK", EDJE_PART_TYPE_TEXTBLOCK,
                     "GROUP", EDJE_PART_TYPE_GROUP,
                     "BOX", EDJE_PART_TYPE_BOX,
                     "TABLE", EDJE_PART_TYPE_TABLE,
                     "EXTERNAL", EDJE_PART_TYPE_EXTERNAL,
                     "PROXY", EDJE_PART_TYPE_PROXY,
                     "MESH_NODE", EDJE_PART_TYPE_MESH_NODE,
                     "LIGHT", EDJE_PART_TYPE_LIGHT,
                     "CAMERA", EDJE_PART_TYPE_CAMERA,
                     "SPACER", EDJE_PART_TYPE_SPACER,
                     "SNAPSHOT", EDJE_PART_TYPE_SNAPSHOT,
                     "VECTOR", EDJE_PART_TYPE_VECTOR,
                     NULL);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (type == EDJE_PART_TYPE_CAMERA)
     {
        for (i = 0; i < (pc->parts_count - 1); i++)
          {
             if (pc->parts[i]->type == EDJE_PART_TYPE_CAMERA)
               {
                  WRN("parse error %s:%i. more then one part of type CAMERA in scene.",
                      file_in, line - 1);
                  exit(-1);
               }
          }
     }

   _part_type_set(type);
}

/**
    @page edcref
    @property
        physics_body
    @parameters
        [TYPE]
    @effect
        Set the type (all caps) from among the available types of physics
        body, it's set to NONE by default. If type is NONE no physics
        will be applied and physics block inside part will be discarded.
        Valid types:
            @li NONE
            @li RIGID_BOX
            @li RIGID_SPHERE
            @li RIGID_CYLINDER
            @li SOFT_BOX
            @li SOFT_SPHERE
            @li SOFT_CYLINDER
            @li CLOTH
            @li BOUNDARY_TOP
            @li BOUNDARY_BOTTOM
            @li BOUNDARY_RIGHT
            @li BOUNDARY_LEFT
            @li BOUNDARY_FRONT
            @li BOUNDARY_BACK

        Defaults: NONE
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_physics_body(void)
{
   unsigned int body;

   check_arg_count(1);

   body = parse_enum(0,
                     "NONE", EDJE_PART_PHYSICS_BODY_NONE,
                     "RIGID_BOX", EDJE_PART_PHYSICS_BODY_RIGID_BOX,
                     "RIGID_SPHERE", EDJE_PART_PHYSICS_BODY_RIGID_SPHERE,
                     "RIGID_CYLINDER", EDJE_PART_PHYSICS_BODY_RIGID_CYLINDER,
                     "SOFT_BOX", EDJE_PART_PHYSICS_BODY_SOFT_BOX,
                     "SOFT_SPHERE", EDJE_PART_PHYSICS_BODY_SOFT_SPHERE,
                     "SOFT_CYLINDER", EDJE_PART_PHYSICS_BODY_SOFT_CYLINDER,
                     "CLOTH", EDJE_PART_PHYSICS_BODY_CLOTH,
                     "BOUNDARY_TOP", EDJE_PART_PHYSICS_BODY_BOUNDARY_TOP,
                     "BOUNDARY_BOTTOM", EDJE_PART_PHYSICS_BODY_BOUNDARY_BOTTOM,
                     "BOUNDARY_RIGHT", EDJE_PART_PHYSICS_BODY_BOUNDARY_RIGHT,
                     "BOUNDARY_LEFT", EDJE_PART_PHYSICS_BODY_BOUNDARY_LEFT,
                     "BOUNDARY_FRONT", EDJE_PART_PHYSICS_BODY_BOUNDARY_FRONT,
                     "BOUNDARY_BACK", EDJE_PART_PHYSICS_BODY_BOUNDARY_BACK,
                     NULL);

   current_part->physics_body = body;

   if (body)
     {
        Edje_Part_Collection *pc;
        pc = eina_list_data_get(eina_list_last(edje_collections));
        pc->physics_enabled = 1;
     }
}

#endif

/**
    @page edcref
    @property
        part
    @parameters
        [part declaration]
    @effect
    @code
        group {
            parts {
                part {
                    name: "parent_rect";
                    type: RECT;
                    description { }
                    part {
                        name: "nested_rect";
                        type: RECT;
                        description { }
                    }
                }
                ..
            }
        }
    @endcode
        Nested parts adds hierarchy to edje.
        Nested part inherits it's location relatively to the parent part.
        To declare a nested part just start a new part within current part decl.
        You must define parent part name before adding nested parts.
    @since 1.7
    @endproperty
 */

/**
    @page edcref
    @property
        insert_before
    @parameters
        [another part's name]
    @effect
        The part's name which this part is inserted before. One part cannot
        have both insert_before and insert_after. One part cannot refer
        more than one by insert_before.
    @since 1.1
    @endproperty
 */
static void
st_collections_group_parts_part_insert_before(void)
{
   Edje_Part_Parser *epp;

   check_arg_count(1);

   epp = (Edje_Part_Parser *)current_part;
   epp->reorder.insert_before = parse_str(0);
}

/**
    @page edcref
    @property
        insert_after
    @parameters
        [another part's name]
    @effect
        The part's name which this part is inserted after. One part cannot
        have both insert_before and insert_after. One part cannot refer
        more than one by insert_after.
    @since 1.1
    @endproperty
 */
static void
st_collections_group_parts_part_insert_after(void)
{
   Edje_Part_Parser *epp;

   check_arg_count(1);

   epp = (Edje_Part_Parser *)current_part;
   epp->reorder.insert_after = parse_str(0);
}

/**
    @page edcref
    @property
        mouse_events
    @parameters
        [1 or 0]
    @effect
        Specifies whether the part will emit signals, although it is named
        "mouse_events", disabling it (0) will prevent the part from emitting
        any type of signal at all.

        Defaults: group.mouse_events
    @endproperty
 */
static void
st_collections_group_parts_part_mouse_events(void)
{
   check_arg_count(1);

   current_part->mouse_events = parse_bool(0);
}

static void
st_collections_group_parts_part_mouse(void)
{
   check_arg_count(0);
   current_part->mouse_events = 1;
}

static void
st_collections_group_parts_part_nomouse(void)
{
   check_arg_count(0);
   current_part->mouse_events = 0;
}

/**
    @page edcref
    @property
        anti_alias
    @parameters
        [1 or 0]
    @effect
        Takes a boolean value specifying whether part is anti_alias (1) or not (0).

        Defaults: 1
    @endproperty
 */
static void
st_collections_group_parts_part_anti_alias(void)
{
   check_arg_count(1);
   current_part->anti_alias = parse_bool(0);
}

/**
    @page edcref
    @property
        repeat_events
    @parameters
        [1 or 0]
    @effect
        Specifies whether a part echoes a mouse event to other parts below the
        pointer (1), or not (0)

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_repeat_events(void)
{
   check_arg_count(1);

   current_part->repeat_events = parse_bool(0);
}

static void
st_collections_group_parts_part_repeat(void)
{
   check_arg_count(0);

   current_part->repeat_events = 1;
}

static void
st_collections_group_parts_part_norepeat(void)
{
   check_arg_count(0);

   current_part->repeat_events = 0;
}

/**
    @page edcref
    @property
        ignore_flags
    @parameters
        [FLAG] ...
    @effect
        Specifies whether events with the given flags should be ignored,
        i.e., will not have the signals emitted to the parts. Multiple flags
        must be separated by spaces, the effect will be ignoring all events
        with one of the flags specified.
        Possible flags:
            @li NONE (no event will be ignored)
            @li ON_HOLD

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_ignore_flags(void)
{
   check_min_arg_count(1);

   current_part->ignore_flags = parse_flags(0,
                                            "NONE", EVAS_EVENT_FLAG_NONE,
                                            "ON_HOLD", EVAS_EVENT_FLAG_ON_HOLD,
                                            NULL);
}

/**
    @page edcref
    @property
        mask_flags
    @parameters
        [FLAG] ...
    @effect
        Masks event flags with the given value, so that the event can be repeated
        to the lower object along with masked event flags.
        Possible flags:
            @li NONE (no event will be masked)
            @li ON_HOLD

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_mask_flags(void)
{
   check_min_arg_count(1);

   current_part->mask_flags = parse_flags(0,
                                          "NONE", EVAS_EVENT_FLAG_NONE,
                                          "ON_HOLD", EVAS_EVENT_FLAG_ON_HOLD,
                                          NULL);
}

/**
    @page edcref
    @property
        scale
    @parameters
        [1 or 0]
    @effect
        Specifies whether the part will scale its size with an edje scaling
        factor. By default scale is off (0) and the default scale factor is
        1.0 - that means no scaling. This would be used to scale properties
        such as font size, min/max size of the part, and possibly can be used
        to scale based on DPI of the target device. The reason to be selective
        is that some things work well being scaled, others do not, so the
        designer gets to choose what works best. For MESH_NODE parts three
        parameters specify how much the part will stretched along each axis.

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_scale(void)
{
   if (get_arg_count() == 1)
     current_part->scale = parse_bool(0);
   else
     current_part->scale = 1;
}

static void
st_collections_group_parts_part_noscale(void)
{
   check_arg_count(0);
   current_part->scale = 0;
}

/**
    @page edcref
    @property
        pointer_mode
    @parameters
        [MODE]
    @effect
        Sets the mouse pointer behavior for a given part. Aviable modes:
            @li AUTOGRAB, when the part is clicked and the button remains
                pressed, the part will be the source of all future mouse
                signals emitted, even outside the object, until the button is
                released.
            @li NOGRAB, the effect will be limited to the part's container.

        Defaults: AUTOGRAB
    @endproperty
 */
static void
st_collections_group_parts_part_pointer_mode(void)
{
   check_arg_count(1);

   current_part->pointer_mode = parse_enum(0,
                                           "AUTOGRAB", EVAS_OBJECT_POINTER_MODE_AUTOGRAB,
                                           "NOGRAB", EVAS_OBJECT_POINTER_MODE_NOGRAB,
                                           NULL);
}

/**
    @page edcref
    @property
        precise_is_inside
    @parameters
        [1 or 0]
    @effect
        Enables precise point collision detection for the part, which is more
        resource intensive.

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_precise_is_inside(void)
{
   check_arg_count(1);

   current_part->precise_is_inside = parse_bool(0);
}

static void
st_collections_group_parts_part_precise(void)
{
   check_arg_count(0);

   current_part->precise_is_inside = 1;
}

static void
st_collections_group_parts_part_noprecise(void)
{
   check_arg_count(0);

   current_part->precise_is_inside = 0;
}

/**
    @page edcref
    @property
        use_alternate_font_metrics
    @parameters
        [1 or 0]
    @effect
        Only affects text and textblock parts, when enabled Edje will use
        different size measurement functions. Disabled by default. (note from
        the author: I don't know what this is exactly useful for?)

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_use_alternate_font_metrics(void)
{
   check_arg_count(1);

   current_part->use_alternate_font_metrics = parse_bool(0);
}

/**
    @page edcref
    @property
        clip_to
    @parameters
        [another part's name]
    @effect
        Only renders the area of part that coincides with another part's
        container. Overflowing content will not be displayed. Note that
        the part being clipped to can only be a rectangle part.
    @endproperty
 */
static void
st_collections_group_parts_part_clip_to_id(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_part->clip_to_id));
      free(name);
   }
}

/**
    @page edcref
    @property
        no_render
    @parameters
        [1 or 0]
    @effect
        Setting the no_render flag on an object will make it never render
        directly on the canvas, regardless of the visible and color properties.
        But the object will still be rendered in a dedicated surface when
        required if it is a proxy source or a mask (clipper).
        Strongly recommended for use with mask objects and proxy sources
        (instead of setting "source_visible" on the proxy itself).

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_no_render(void)
{
   if (check_range_arg_count(0, 1) == 1)
     current_part->no_render = parse_bool(0);
   else /* lazEDC form */
     current_part->no_render = EINA_TRUE;
}

static void
st_collections_group_parts_part_render(void)
{
   current_part->no_render = EINA_FALSE;
}

/**
    @page edcref
    @property
        required
    @parameters
        [1 or 0]
    @effect
        If the required flag is set, this part will be considered
        stable and it is safe to use by any application."

        Defaults: 0
    @since 1.18
    @endproperty
 */
static void
st_collections_group_parts_part_required(void)
{
   if (check_range_arg_count(0, 1) == 1)
     current_part->required = parse_bool(0);
   else /* lazEDC form */
     current_part->required = EINA_TRUE;
}

static void
st_collections_group_parts_part_norequired(void)
{
   current_part->required = EINA_FALSE;
}

/**
    @page edcref
    @property
        source
    @parameters
        [another group's name]
    @effect
        Only available to GROUP or TEXTBLOCK parts. Swallows the specified
        group into the part's container if a GROUP. If TEXTBLOCK it is used
        for the group to be loaded and used for selection display UNDER the
        selected text. source2 is used for on top of the selected text, if
        source2 is specified.
    @endproperty
 */
static void
st_collections_group_parts_part_source(void)
{
   check_arg_count(1);

   //FIXME: validate this somehow (need to decide on the format also)
   current_part->source = parse_str(0);
   data_queue_group_lookup(current_part->source, current_part);
}

/**
    @page edcref
    @property
        source2
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be
        loaded and used for selection display OVER the selected text. source
        is used for under of the selected text, if source is specified.
    @endproperty
 */
static void
st_collections_group_parts_part_source2(void)
{
   check_arg_count(1);

   //FIXME: validate this somehow (need to decide on the format also)
   current_part->source2 = parse_str(0);
   data_queue_group_lookup(current_part->source2, current_part);
}

/**
    @page edcref
    @property
        source3
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be
        loaded and used for cursor display UNDER the cursor position. source4
        is used for over the cursor text, if source4 is specified.
    @endproperty
 */
static void
st_collections_group_parts_part_source3(void)
{
   check_arg_count(1);

   //FIXME: validate this somehow (need to decide on the format also)
   current_part->source3 = parse_str(0);
   data_queue_group_lookup(current_part->source3, current_part);
}

/**
    @page edcref
    @property
        source4
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be
        loaded and used for cursor display OVER the cursor position. source3
        is used for under the cursor text, if source4 is specified.
    @endproperty
 */
static void
st_collections_group_parts_part_source4(void)
{
   check_arg_count(1);

   //FIXME: validate this somehow (need to decide on the format also)
   current_part->source4 = parse_str(0);
   data_queue_group_lookup(current_part->source4, current_part);
}

/**
    @page edcref
    @property
        source5
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be
        loaded and used for anchors display UNDER the anchor position. source6
        is used for over the anchors text, if source6 is specified.
    @endproperty
 */
static void
st_collections_group_parts_part_source5(void)
{
   check_arg_count(1);

   //FIXME: validate this somehow (need to decide on the format also)
   current_part->source5 = parse_str(0);
   data_queue_group_lookup(current_part->source5, current_part);
}

/**
    @page edcref
    @property
        source6
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be
        loaded and used for anchor display OVER the anchor position. source5
        is used for under the anchor text, if source6 is specified.
    @endproperty
 */
static void
st_collections_group_parts_part_source6(void)
{
   check_arg_count(1);

   //FIXME: validate this somehow (need to decide on the format also)
   current_part->source6 = parse_str(0);
   data_queue_group_lookup(current_part->source6, current_part);
}

/**
    @page edcref

    @property
        effect
    @parameters
        [effect]
        (shadow direction)
    @effect
        Causes Edje to draw the selected effect among:
        @li PLAIN
        @li OUTLINE
        @li SOFT_OUTLINE
        @li SHADOW
        @li SOFT_SHADOW
        @li OUTLINE_SHADOW
        @li OUTLINE_SOFT_SHADOW
        @li FAR_SHADOW
        @li FAR_SOFT_SHADOW
        @li GLOW

        Shadow directions (default if not given is BOTTOM_RIGHT):
        @li BOTTOM_RIGHT
        @li BOTTOM
        @li BOTTOM_LEFT
        @li LEFT
        @li TOP_LEFT
        @li TOP
        @li TOP_RIGHT
        @li RIGHT

        Defaults: PLAIN
    @endproperty
 */
static void
st_collections_group_parts_part_effect(void)
{
   check_min_arg_count(1);

   current_part->effect = parse_enum(0,
                                     "NONE", EDJE_TEXT_EFFECT_NONE,
                                     "PLAIN", EDJE_TEXT_EFFECT_PLAIN,
                                     "OUTLINE", EDJE_TEXT_EFFECT_OUTLINE,
                                     "SOFT_OUTLINE", EDJE_TEXT_EFFECT_SOFT_OUTLINE,
                                     "SHADOW", EDJE_TEXT_EFFECT_SHADOW,
                                     "SOFT_SHADOW", EDJE_TEXT_EFFECT_SOFT_SHADOW,
                                     "OUTLINE_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SHADOW,
                                     "OUTLINE_SOFT_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW,
                                     "FAR_SHADOW", EDJE_TEXT_EFFECT_FAR_SHADOW,
                                     "FAR_SOFT_SHADOW", EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW,
                                     "GLOW", EDJE_TEXT_EFFECT_GLOW,
                                     NULL);
   if (get_arg_count() >= 2)
     {
        unsigned char shadow;

        shadow = parse_enum(1,
                            "BOTTOM_RIGHT", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_RIGHT,
                            "BOTTOM", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM,
                            "BOTTOM_LEFT", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_LEFT,
                            "LEFT", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_LEFT,
                            "TOP_LEFT", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_LEFT,
                            "TOP", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP,
                            "TOP_RIGHT", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_RIGHT,
                            "RIGHT", EDJE_TEXT_EFFECT_SHADOW_DIRECTION_RIGHT,
                            NULL);
        EDJE_TEXT_EFFECT_SHADOW_DIRECTION_SET(current_part->effect, shadow);
     }
}

/**
    @page edcref
    @property
        entry_mode
    @parameters
        [mode]
    @effect
        Sets the edit mode for a textblock part to one of:
        @li NONE
        @li PLAIN
        @li EDITABLE
        @li PASSWORD

        It causes the part be editable if the edje object has the keyboard
        focus AND the part has the edje focus (or selectable always
        regardless of focus) and in the event of password mode, not
        selectable and all text chars replaced with *'s but editable and
        pastable.

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_entry_mode(void)
{
   check_arg_count(1);

   current_part->entry_mode = parse_enum(0,
                                         "NONE", EDJE_ENTRY_EDIT_MODE_NONE,
                                         "PLAIN", EDJE_ENTRY_EDIT_MODE_SELECTABLE,
                                         "EDITABLE", EDJE_ENTRY_EDIT_MODE_EDITABLE,
                                         "PASSWORD", EDJE_ENTRY_EDIT_MODE_PASSWORD,
                                         NULL);
}

/**
    @page edcref
    @property
        select_mode
    @parameters
        [mode]
    @effect
        Sets the selection mode for a textblock part to one of:
        @li DEFAULT selection mode is what you would expect on any desktop. Press
        mouse, drag and release to end.
        @li EXPLICIT mode requires the application
        controlling the edje object has to explicitly begin and end selection
        modes, and the selection itself is draggable at both ends.
    @endproperty
 */
static void
st_collections_group_parts_part_select_mode(void)
{
   check_arg_count(1);

   current_part->select_mode = parse_enum(0,
                                          "DEFAULT", EDJE_ENTRY_SELECTION_MODE_DEFAULT,
                                          "EXPLICIT", EDJE_ENTRY_SELECTION_MODE_EXPLICIT,
                                          NULL);
}

/**
    @page edcref
    @property
        cursor_mode
    @parameters
        [mode]
    @effect
        Sets the cursor mode for a textblock part to one of:
        @li UNDER cursor mode means the cursor will draw below the character pointed at.
        @li BEFORE cursor mode means the cursor is drawn as a vertical line before
        the current character, just like many other GUI toolkits handle it.

        Defaults: UNDER
    @endproperty
 */
static void
st_collections_group_parts_part_cursor_mode(void)
{
   check_arg_count(1);

   current_part->cursor_mode = parse_enum(0,
                                          "UNDER", EDJE_ENTRY_CURSOR_MODE_UNDER,
                                          "BEFORE", EDJE_ENTRY_CURSOR_MODE_BEFORE,
                                          NULL);
}

/**
    @page edcref
    @property
        multiline
    @parameters
        [1 or 0]
    @effect
        It causes a textblock that is editable to allow multiple lines for
        editing.

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_multiline(void)
{
   check_arg_count(1);

   current_part->multiline = parse_bool(0);
}

/**
    @page edcref
    @property
        access
    @parameters
        [1 or 0]
    @effect
        Specifies whether the part will use accessibility feature (1), or not (0).

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_access(void)
{
   check_arg_count(1);

   current_part->access = parse_bool(0);
}

/** @edcsubsection{collections_group_parts_dragable,
 *                 Group.Parts.Part.Dragable} */

/**
    @page edcref
    @block
        dragable
    @context
        part {
            ..
            dragable {
                confine: "another part";
                threshold: "another part";
                events:  "another dragable part";
                x: 0 0 0;
                y: 0 0 0;
            }
            ..
        }
    @description
        When this block is used the resulting part can be dragged around the
        interface, do not confuse with external drag & drop. By default Edje
        (and most applications) will attempt to use the minimal size possible
        for a dragable part. If the min property is not set in the description
        the part will be (most likely) set to 0px width and 0px height, thus
        invisible.
    @endblock

    @property
        x
    @parameters
        [enable/disable] [step] [count]
    @effect
        Used to setup dragging events for the X axis. The first parameter is
        used to enable (1 or -1) and disable (0) dragging along the axis. When
        enabled, 1 will set the starting point at 0.0 and -1 at 1.0. The second
        parameter takes any integer and will limit movement to values
        divisible by it, causing the part to jump from position to position.
        If step is set to 0 it is calculated as width of confine part divided by
        count.

        Defaults: 0 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_dragable_x(void)
{
   check_arg_count(3);

   current_part->dragable.x = parse_int_range(0, -1, 1);
   current_part->dragable.step_x = parse_int_range(1, 0, 0x7fffffff);
   current_part->dragable.count_x = parse_int_range(2, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        y
    @parameters
        [enable/disable] [step] [count]
    @effect
        Used to setup dragging events for the Y axis. The first parameter is
        used to enable (1 or -1) and disable (0) dragging along the axis. When
        enabled, 1 will set the starting point at 0.0 and -1 at 1.0. The second
        parameter takes any integer and will limit movement to values
        divisible by it, causing the part to jump from position to position.
        If step is set to 0 it is calculated as height of confine part divided by
        count.

        Defaults: 0 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_dragable_y(void)
{
   check_arg_count(3);

   current_part->dragable.y = parse_int_range(0, -1, 1);
   current_part->dragable.step_y = parse_int_range(1, 0, 0x7fffffff);
   current_part->dragable.count_y = parse_int_range(2, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        confine
    @parameters
        [another part's name]
    @effect
        When set, limits the movement of the dragged part to another part's
        container. When you use confine don't forget to set a min size for the
        part, or the draggie will not show up.
    @endproperty
 */
static void
st_collections_group_parts_part_dragable_confine(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_part->dragable.confine_id));
      free(name);
   }
}

/**
    @page edcref
    @property
        threshold
    @parameters
        [another part's name]
    @effect
        When set, the movement of the dragged part can only start when it get
        moved enough to be outside of the threshold part.
    @endproperty
 */
static void
st_collections_group_parts_part_dragable_threshold(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_part->dragable.threshold_id));
      free(name);
   }
}

/**
    @page edcref
    @property
        events
    @parameters
        [another dragable part's name]
    @effect
        It causes the part to forward the drag events to another part, thus
        ignoring them for itself.
    @endproperty
 */
static void
st_collections_group_parts_part_dragable_events(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_part->dragable.event_id));
      free(name);
   }
}

/**
    @page edcref
    @property
        allowed_seats
    @parameters
        [seat1] [seat2] [seat3] ...
    @effect
        List of seat names allowed to interact with the part.

        If no list is defined all seats are allowed. It's the
        default behaviour.

        If a seat isn't allowed, no signals will be emitted
        related to its actions, as mouse and focus events.
        Also it won't be able to focus this part.
    @since 1.19
    @endproperty
 */
static void
st_collections_group_parts_part_allowed_seats(void)
{
   Edje_Part_Allowed_Seat *seat;
   Edje_Part *ep;
   int n, argc;

   check_min_arg_count(1);

   ep = current_part;
   argc = get_arg_count();

   ep->allowed_seats = calloc(argc, sizeof(Edje_Part_Allowed_Seat *));
   if (!ep->allowed_seats)
     {
        ERR("Not enough memory.");
        exit(-1);
     }

   for (n = 0; n < argc; n++)
     {
        seat = mem_alloc(SZ(Edje_Part_Allowed_Seat));
        seat->name = parse_str(n);
        ep->allowed_seats[n] = seat;
     }

   ep->allowed_seats_count = argc;
}

/** @edcsubsection{collections_group_parts_items,
 *                 Group.Parts.Part.Box/Table.Items} */

/**
    @page edcref
    @block
        items
    @context
        part {
            ..
            box/table {
                items {
                    item {
                        type: TYPE;
                        source: "some source";
                        min: 1 1;
                        max: 100 100;
                        padding: 1 1 2 2;
                    }
                    item {
                        type: TYPE;
                        source: "some other source";
                        name: "some name";
                        align: 1.0 0.5;
                    }
                    ..
                }
            }
            ..
        }
    @description
        On a part of type BOX, this block can be used to set other groups
        as elements of the box. These can be mixed with external objects set
        by the application through the edje_object_part_box_* API.
    @endblock
 */
static void
ob_collections_group_parts_part_box_items_item(void)
{
   Edje_Part *ep;
   Edje_Pack_Element *item;
   Edje_Pack_Element_Parser *pitem;

   ep = current_part;

   if ((ep->type != EDJE_PART_TYPE_BOX) && (ep->type != EDJE_PART_TYPE_TABLE))
     {
        ERR("parse error %s:%i. box attributes in non-BOX or TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ep->items_count++;
   ep->items = realloc(ep->items, sizeof (Edje_Pack_Element *) * ep->items_count);
   if (!ep->items)
     {
        ERR("Not enough memory.");
        exit(-1);
     }

   item = mem_alloc(SZ(Edje_Pack_Element_Parser));
   current_item = ep->items[ep->items_count - 1] = item;
   item->type = EDJE_PART_TYPE_GROUP;
   item->name = NULL;
   item->source = NULL;
   item->min.w = 0;
   item->min.h = 0;
   item->prefer.w = 0;
   item->prefer.h = 0;
   item->max.w = -1;
   item->max.h = -1;
   item->padding.l = 0;
   item->padding.r = 0;
   item->padding.t = 0;
   item->padding.b = 0;
   item->align.x = FROM_DOUBLE(0.5);
   item->align.y = FROM_DOUBLE(0.5);
   item->weight.x = FROM_DOUBLE(0.0);
   item->weight.y = FROM_DOUBLE(0.0);
   item->aspect.w = 0;
   item->aspect.h = 0;
   item->aspect.mode = EDJE_ASPECT_CONTROL_NONE;
   item->options = NULL;
   item->col = -1;
   item->row = -1;
   item->colspan = 1;
   item->rowspan = 1;
   item->spread.w = 1;
   item->spread.h = 1;
   pitem = (Edje_Pack_Element_Parser *)item;
   pitem->can_override = EINA_FALSE;
}

#define CURRENT_ITEM_CHECK                                       \
  if (!current_item)                                             \
    {                                                            \
       ERR("parse error %s:%i. Item not defined at this stage.", \
           file_in, line - 1);                                   \
       exit(-1);                                                 \
    }

/**
    @page edcref
    @property
        type
    @parameters
        [item type]
    @effect
        Sets the type of the object this item will hold.
        Supported types are:
        @li GROUP
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_type(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(1);

   {
      char *s;

      s = parse_str(0);
      if (strcmp(s, "GROUP"))
        {
           ERR("parse error %s:%i. token %s not one of: GROUP.",
               file_in, line - 1, s);
           free(s);
           exit(-1);
        }
      free(s);
      /* FIXME: handle the enum, once everything else is supported */
      current_item->type = EDJE_PART_TYPE_GROUP;
   }
}

/**
    @page edcref
    @property
        name
    @parameters
        [name for the object]
    @effect
        Sets the name of the object via evas_object_name_set().
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_name(void)
{
   Edje_Part *ep;
   Edje_Pack_Element *item;
   Edje_Pack_Element_Parser *pitem;

   CURRENT_ITEM_CHECK;

   check_arg_count(1);

   ep = current_part;
   item = ep->items[ep->items_count - 1];

   item->name = parse_str(0);

   {
      unsigned int i;

      for (i = 0; i < ep->items_count - 1; ++i)
        {
           if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item->name)))
             {
                pitem = (Edje_Pack_Element_Parser *)ep->items[i];
                if (!pitem->can_override)
                  {
                     ERR("parse error %s:%i. There is already a item of the name %s",
                         file_in, line - 1, item->name);
                     exit(-1);
                  }
                else
                  {
                     free(item);
                     ep->items_count--;
                     ep->items = realloc(ep->items, ep->items_count * sizeof (Edje_Pack_Element *));
                     current_item = ep->items[i];
                     pitem->can_override = EINA_FALSE;
                     break;
                  }
             }
        }
   }
}

/**
    @page edcref
    @property
        source
    @parameters
        [another group's name]
    @effect
        Sets the group this object will be made from.
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_source(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(1);

   current_item->source = parse_str(0);
   data_queue_group_lookup(current_item->source, current_part);
}

/**
    @page edcref
    @property
        min
    @parameters
        [width] [height]
    @effect
        Sets the minimum size hints for this object.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_min(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->min.w = parse_int_range(0, 0, 0x7ffffff);
   current_item->min.h = parse_int_range(1, 0, 0x7ffffff);
}

/**
   @page edcref
   @property
       spread
   @parameters
       [width] [height]
   @effect
       Will replicate the item in a rectangle of size width x height
       box starting from the defined position of this item.

       Defaults: 1 1
   @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_spread(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->spread.w = parse_int_range(0, 1, 0x7ffffff);
   current_item->spread.h = parse_int_range(1, 1, 0x7ffffff);
}

/**
    @page edcref
    @property
        prefer
    @parameters
        [width] [height]
    @effect
        Sets the preferred size hints for this object.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_prefer(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->prefer.w = parse_int_range(0, 0, 0x7ffffff);
   current_item->prefer.h = parse_int_range(1, 0, 0x7ffffff);
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height]
    @effect
        Sets the maximum size hints for this object.

        Defaults: -1 -1
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_max(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->max.w = parse_int_range(0, -1, 0x7ffffff);
   current_item->max.h = parse_int_range(1, -1, 0x7ffffff);
}

/**
    @page edcref
    @property
        padding
    @parameters
        [left] [right] [top] [bottom]
    @effect
        Sets the padding hints for this object.

        Defaults: 0 0 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_padding(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(4);

   current_item->padding.l = parse_int_range(0, 0, 0x7ffffff);
   current_item->padding.r = parse_int_range(1, 0, 0x7ffffff);
   current_item->padding.t = parse_int_range(2, 0, 0x7ffffff);
   current_item->padding.b = parse_int_range(3, 0, 0x7ffffff);
}

/**
    @page edcref
    @property
        align
    @parameters
        [x] [y]
    @effect
        Sets the alignment [-1.0 - 1.0] hints for this object.

        Defaults: 0.5
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_align(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->align.x = FROM_DOUBLE(parse_float_range(0, -1.0, 1.0));
   current_item->align.y = FROM_DOUBLE(parse_float_range(1, -1.0, 1.0));
}

/**
    @page edcref
    @property
        weight
    @parameters
        [x] [y]
    @effect
        Sets the weight hints for this object.

        Defaults: 0.0 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_weight(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->weight.x = FROM_DOUBLE(parse_float_range(0, 0.0, 99999.99));
   current_item->weight.y = FROM_DOUBLE(parse_float_range(1, 0.0, 99999.99));
}

/**
    @page edcref
    @property
        aspect
    @parameters
        [w] [h]
    @effect
        Sets the aspect width and height hints for this object.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_aspect(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->aspect.w = parse_int_range(0, 0, 0x7fffffff);
   current_item->aspect.h = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        aspect_mode
    @parameters
        [mode]
    @effect
        Sets the aspect control hints for this object. Mode can be one of:
        @li NONE
        @li NEITHER
        @li HORIZONTAL
        @li VERTICAL
        @li BOTH

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_aspect_mode(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(1);

   current_item->aspect.mode = parse_enum(0,
                                          "NONE", EDJE_ASPECT_CONTROL_NONE,
                                          "NEITHER", EDJE_ASPECT_CONTROL_NEITHER,
                                          "HORIZONTAL", EDJE_ASPECT_CONTROL_HORIZONTAL,
                                          "VERTICAL", EDJE_ASPECT_CONTROL_VERTICAL,
                                          "BOTH", EDJE_ASPECT_CONTROL_BOTH,
                                          NULL);
}

/**
    @page edcref
    @property
        options
    @parameters
        [extra options]
    @effect
        Sets extra options for the object. Unused for now.
    @endproperty
 */
static void
st_collections_group_parts_part_box_items_item_options(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(1);

   current_item->options = parse_str(0);
}

/**
    @page edcref
    @property
        position
    @parameters
        [col] [row]
    @effect
        Sets the position this item will have in the table.
        This is required for parts of type TABLE.
    @endproperty
 */
static void
st_collections_group_parts_part_table_items_item_position(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   current_item->col = parse_int_range(0, 0, 0xffff);
   current_item->row = parse_int_range(1, 0, 0xffff);
}

/**
    @page edcref
    @property
        span
    @parameters
        [col] [row]
    @effect
        Sets how many columns/rows this item will use.

        Defaults: 1 1
    @endproperty
 */
static void
st_collections_group_parts_part_table_items_item_span(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   current_item->colspan = parse_int_range(0, 1, 0xffff);
   current_item->rowspan = parse_int_range(1, 1, 0xffff);
}

static Edje_Map_Color **
_copied_map_colors_get(Edje_Part_Description_Common *parent)
{
   Edje_Map_Color **colors;
   Edje_Map_Color *color;
   int i;

   if (parent->map.colors_count == 0) return NULL;
   colors = mem_alloc(sizeof(Edje_Map_Color *) * parent->map.colors_count);

   for (i = 0; i < (int)parent->map.colors_count; i++)
     {
        color = parent->map.colors[i];

        Edje_Map_Color *c = mem_alloc(SZ(Edje_Map_Color));
        if (!color)
          {
             ERR("Could not find allocated source when copying map colors");
             exit(-1);
             return NULL;
          }
        memcpy(c, color, sizeof(Edje_Map_Color));
        colors[i] = c;
     }
   return colors;
}

/** @edcsubsection{collections_group_parts_description,
 *                 Group.Parts.Part.Description} */

/**
    @page edcref
    @block
        description
    @context
        description {
            inherit: "another_description" INDEX;
            state: "description_name" INDEX;
            visible: 1;
            min: 0 0;
            max: -1 -1;
            align: 0.5 0.5;
            fixed: 0 0;
            step: 0 0;
            aspect: 1 1;
            clip_to: "clip_override_part_name";
            no_render: 0;
            offset_scale: 0;

            rel1 {
                ..
            }

            rel2 {
                ..
            }
        }
    @description
        Every part can have one or more description blocks. Each description is
        used to define style and layout properties of a part in a given
        "state".
    @endblock
 */
static void
ob_collections_group_parts_part_description(void)
{  /* Allocate and set desc, set relative part hierarchy if needed */
  Edje_Part_Collection *pc;
  Edje_Part *ep;
  Edje_Part_Description_Common *ed;

  pc = eina_list_data_get(eina_list_last(edje_collections));
  ep = current_part;

  ed = _edje_part_description_alloc(ep->type, pc->part, ep->name);
  eina_hash_add(desc_hash, &ed, ep);

  ed->rel1.id_x = -1;
  ed->rel1.id_y = -1;
  ed->rel2.id_x = -1;
  ed->rel2.id_y = -1;
  ed->clip_to_id = -1;

  if (!ep->default_desc)
    {
       current_desc = ep->default_desc = ed;
       ed->state.name = strdup("default");

       {     /* Get the ptr of the part above current part in hierarchy */
         Edje_Part *node = edje_cc_handlers_hierarchy_parent_get();
         if (node) /* Make relative according to part hierarchy */
           edje_cc_handlers_hierarchy_set(node);
       }
    }
  else
    {
       ep->other.desc_count++;
       ep->other.desc = realloc(ep->other.desc,
                                sizeof (Edje_Part_Description_Common *) * ep->other.desc_count);
       current_desc = ep->other.desc[ep->other.desc_count - 1] = ed;
    }

  ed->visible = 1;
  ed->limit = 0;
  ed->no_render = 0;
  ed->align.x = FROM_DOUBLE(0.5);
  ed->align.y = FROM_DOUBLE(0.5);
  ed->min.w = 0;
  ed->min.h = 0;
  ed->fixed.w = 0;
  ed->fixed.h = 0;
  ed->max.w = -1;
  ed->max.h = -1;
  ed->size_class = NULL;
  ed->rel1.relative_x = FROM_DOUBLE(0.0);
  ed->rel1.relative_y = FROM_DOUBLE(0.0);
  ed->rel1.offset_x = 0;
  ed->rel1.offset_y = 0;
  ed->rel2.relative_x = FROM_DOUBLE(1.0);
  ed->rel2.relative_y = FROM_DOUBLE(1.0);
  ed->rel2.offset_x = -1;
  ed->rel2.offset_y = -1;
  ed->color_class = NULL;
  ed->color.r = 255;
  ed->color.g = 255;
  ed->color.b = 255;
  ed->color.a = 255;
  ed->color2.r = 0;
  ed->color2.g = 0;
  ed->color2.b = 0;
  ed->color2.a = 255;
  ed->map.id_persp = -1;
  ed->map.id_light = -1;
  ed->map.rot.id_center = -1;
  ed->map.rot.x = FROM_DOUBLE(0.0);
  ed->map.rot.y = FROM_DOUBLE(0.0);
  ed->map.rot.z = FROM_DOUBLE(0.0);
  ed->map.on = 0;
  ed->map.smooth = 1;
  ed->map.alpha = 1;
  ed->map.backcull = 0;
  ed->map.persp_on = 0;
  ed->map.colors = NULL;
  ed->map.zoom.x = FROM_DOUBLE(1.0);
  ed->map.zoom.y = FROM_DOUBLE(1.0);
  ed->persp.zplane = 0;
  ed->persp.focal = 1000;
  ed->minmul.have = 1;
  ed->minmul.w = FROM_DOUBLE(1.0);
  ed->minmul.h = FROM_DOUBLE(1.0);
  ed->align_3d.x = FROM_DOUBLE(0.5);
  ed->align_3d.y = FROM_DOUBLE(0.5);
  ed->align_3d.z = FROM_DOUBLE(0.5);
}

static void
ob_collections_group_parts_part_desc(void)
{
   stack_replace_quick("description");
   ob_collections_group_parts_part_description();
}

/**
    @page edcref
    @property
        inherit
    @parameters
        [another description's name] [another description's index]
    @effect
        When set, the description will inherit all the properties from the
        named description. The properties defined in this part will override
        the inherited properties, reducing the amount of necessary code for
        simple state changes. Note: inheritance in Edje is single level only.
        @since 1.14 omitting both the description name and index will inherit the default 0.0 description.
    @endproperty
 */
static void
st_collections_group_parts_part_description_inherit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep, *parent_ep = NULL;
   Edje_Part_Description_Common *ed, *parent = NULL;
   Edje_Part_Image_Id *iid;
   char *parent_name;
   const char *state_name;
   double parent_val = 0.0, state_val;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_part;
   ed = current_desc;

   parent = parent_desc;
   if (parent)
     parent_ep = eina_hash_find(desc_hash, &parent);
   else
     {
        /* inherit may not be used in the default description */
        if (!ep->other.desc_count)
          {
             ERR("parse error %s:%i. "
                 "inherit may not be used in the default description",
                 file_in, line - 1);
             exit(-1);
          }

        /* find the description that we inherit from */
        switch (get_arg_count())
          {
           case 0:
             parent_name = strdup("default");
             break;

           case 2:
             parent_val = parse_float_range(1, 0.0, 1.0);
             EINA_FALLTHROUGH;

           case 1:
             parent_name = parse_str(0);
             break;

           default:
             ERR("parse error %s:%i. too many parameters",
                 file_in, line - 1);
             exit(-1);
          }

        if (!strcmp(parent_name, "default") && parent_val == 0.0)
          parent = ep->default_desc;
        else
          {
             Edje_Part_Description_Common *d;
             double min_dst = 999.0;
             unsigned int i;

             if (!strcmp(parent_name, "default"))
               {
                  parent = ep->default_desc;
                  min_dst = ABS(ep->default_desc->state.value - parent_val);
               }

             for (i = 0; i < ep->other.desc_count; ++i)
               {
                  d = ep->other.desc[i];

                  if (!strcmp(d->state.name, parent_name))
                    {
                       double dst;

                       dst = ABS(d->state.value - parent_val);
                       if (dst < min_dst)
                         {
                            parent = d;
                            min_dst = dst;
                         }
                    }
               }

             if (min_dst)
               {
                  WRN("%s:%i: couldn't find an exact match in part '%s' when looking for '%s' %lf. Falling back to nearest one '%s' %lf.",
                      file_in, line - 1, ep->name, parent_name, parent_val, parent ? parent->state.name : NULL, parent ? parent->state.value : 0);
               }
          }

        if (!parent)
          {
             ERR("parse error %s:%i. "
                 "cannot find referenced part %s state %s %lf",
                 file_in, line - 1, ep->name, parent_name, parent_val);
             exit(-1);
          }

        free(parent_name);
     }
   /* now do a full copy, only state info will be kept */
   state_name = ed->state.name;
   state_val = ed->state.value;

   *ed = *parent;

   ed->state.name = state_name;
   ed->state.value = state_val;

   data_queue_copied_part_lookup(pc, &parent->rel1.id_x, &ed->rel1.id_x);
   data_queue_copied_part_lookup(pc, &parent->rel1.id_y, &ed->rel1.id_y);
   data_queue_copied_part_lookup(pc, &parent->rel2.id_x, &ed->rel2.id_x);
   data_queue_copied_part_lookup(pc, &parent->rel2.id_y, &ed->rel2.id_y);

   data_queue_copied_part_lookup(pc, &parent->clip_to_id, &ed->clip_to_id);

   data_queue_copied_part_lookup(pc, &parent->map.id_persp, &ed->map.id_persp);
   data_queue_copied_part_lookup(pc, &parent->map.id_light, &ed->map.id_light);
   data_queue_copied_part_lookup(pc, &parent->map.rot.id_center, &ed->map.rot.id_center);

   /* make sure all the allocated memory is getting copied, not just
    * referenced
    */

   ed->size_class = STRDUP(ed->size_class);
   ed->color_class = STRDUP(ed->color_class);
   ed->map.colors = _copied_map_colors_get(parent);

   if (parent_ep && (parent_ep->type != ep->type))
     {
        /* ensure parent's owner is a compatible type of part */
        if (((ep->type != EDJE_PART_TYPE_TEXT) && (ep->type != EDJE_PART_TYPE_TEXTBLOCK)) ||
            ((parent_ep->type != EDJE_PART_TYPE_TEXT) && (parent_ep->type != EDJE_PART_TYPE_TEXTBLOCK)))
          return;
     }

   switch (ep->type)
     {
      case EDJE_PART_TYPE_SPACER:
      case EDJE_PART_TYPE_RECTANGLE:
      case EDJE_PART_TYPE_SWALLOW:
      case EDJE_PART_TYPE_GROUP:
        /* Nothing todo, this part only have a common description. */
        break;

      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
      {
         Edje_Part_Description_Text *ted = (Edje_Part_Description_Text *)ed;
         Edje_Part_Description_Text *tparent = (Edje_Part_Description_Text *)parent;

         ted->text = tparent->text;

         ted->text.text.str = STRDUP(ted->text.text.str);
         ted->text.domain = STRDUP(ted->text.domain);
         ted->text.text_class = STRDUP(ted->text.text_class);
         ted->text.font.str = STRDUP(ted->text.font.str);

         _filter_copy(&ted->filter, &tparent->filter);
         data_queue_copied_part_nest_lookup(pc, &(tparent->text.id_source), &(ted->text.id_source), &ted->text.id_source_part);
         data_queue_copied_part_nest_lookup(pc, &(tparent->text.id_text_source), &(ted->text.id_text_source), &ted->text.id_text_source_part);

         break;
      }

      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ied = (Edje_Part_Description_Image *)ed;
         Edje_Part_Description_Image *iparent = (Edje_Part_Description_Image *)parent;
         unsigned int i;

         ied->image = iparent->image;

         data_queue_image_remove(&ied->image.id, &ied->image.set);
         data_queue_copied_image_lookup(&iparent->image.id, &ied->image.id, &ied->image.set);

         ied->image.tweens = calloc(iparent->image.tweens_count,
                                    sizeof (Edje_Part_Image_Id *));
         for (i = 0; i < iparent->image.tweens_count; i++)
           {
              Edje_Part_Image_Id *iid_new;

              iid = iparent->image.tweens[i];

              iid_new = mem_alloc(SZ(Edje_Part_Image_Id));
              data_queue_image_remove(&ied->image.id, &ied->image.set);
              data_queue_copied_image_lookup(&(iid->id), &(iid_new->id), &(iid_new->set));
              ied->image.tweens[i] = iid_new;
           }

         _filter_copy(&ied->filter, &iparent->filter);

         break;
      }

      case EDJE_PART_TYPE_SNAPSHOT:
      {
         Edje_Part_Description_Snapshot *sed = (Edje_Part_Description_Snapshot *)ed;
         Edje_Part_Description_Snapshot *sparent = (Edje_Part_Description_Snapshot *)parent;

         _filter_copy(&sed->filter, &sparent->filter);

         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ped = (Edje_Part_Description_Proxy *)ed;
         Edje_Part_Description_Proxy *pparent = (Edje_Part_Description_Proxy *)parent;

         data_queue_copied_part_lookup(pc, &(pparent->proxy.id), &(ped->proxy.id));
         ped->proxy.source_clip = pparent->proxy.source_clip;
         ped->proxy.source_visible = pparent->proxy.source_visible;
         _filter_copy(&ped->filter, &pparent->filter);

         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *bed = (Edje_Part_Description_Box *)ed;
         Edje_Part_Description_Box *bparent = (Edje_Part_Description_Box *)parent;

         bed->box = bparent->box;

         break;
      }

      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *ted = (Edje_Part_Description_Table *)ed;
         Edje_Part_Description_Table *tparent = (Edje_Part_Description_Table *)parent;

         ted->table = tparent->table;

         break;
      }

      case EDJE_PART_TYPE_EXTERNAL:
      {
         Edje_Part_Description_External *eed = (Edje_Part_Description_External *)ed;
         Edje_Part_Description_External *eparent = (Edje_Part_Description_External *)parent;

         if (eparent->external_params)
           {
              Eina_List *l;
              Edje_External_Param *param, *new_param;

              eed->external_params = NULL;
              EINA_LIST_FOREACH(eparent->external_params, l, param)
                {
                   new_param = mem_alloc(SZ(Edje_External_Param));
                   *new_param = *param;
                   eed->external_params = eina_list_append(eed->external_params, new_param);
                }
           }
         break;
      }

      case EDJE_PART_TYPE_CAMERA:
      {
         Edje_Part_Description_Camera *ced = (Edje_Part_Description_Camera *)ed;
         Edje_Part_Description_Camera *cparent = (Edje_Part_Description_Camera *)parent;

         ced->camera = cparent->camera;

         data_queue_copied_part_lookup(pc, &(cparent->camera.orientation.look_to), &(ced->camera.orientation.look_to));

         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *led = (Edje_Part_Description_Light *)ed;
         Edje_Part_Description_Light *lparent = (Edje_Part_Description_Light *)parent;

         led->light = lparent->light;

         data_queue_copied_part_lookup(pc, &(lparent->light.orientation.look_to), &(led->light.orientation.look_to));

         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *med = (Edje_Part_Description_Mesh_Node *)ed;
         Edje_Part_Description_Mesh_Node *mparent = (Edje_Part_Description_Mesh_Node *)parent;

         med->mesh_node = mparent->mesh_node;

         data_queue_model_remove(&med->mesh_node.mesh.id, &med->mesh_node.mesh.set);
         data_queue_copied_model_lookup(&mparent->mesh_node.mesh.id, &med->mesh_node.mesh.id, &med->mesh_node.mesh.set);

         data_queue_image_remove(&med->mesh_node.texture.id, &med->mesh_node.texture.set);
         data_queue_copied_model_lookup(&mparent->mesh_node.texture.id, &med->mesh_node.texture.id, &med->mesh_node.texture.set);

         data_queue_copied_part_lookup(pc, &(mparent->mesh_node.orientation.look_to), &(med->mesh_node.orientation.look_to));

         break;
      }

      case EDJE_PART_TYPE_VECTOR:
      {
         Edje_Part_Description_Vector *ied = (Edje_Part_Description_Vector *)ed;
         Edje_Part_Description_Vector *iparent = (Edje_Part_Description_Vector *)parent;
         ied->vg.set = iparent->vg.set;
         ied->vg.id = iparent->vg.id;
         break;
      }
     }
}

/**
    @page edcref

    @property
        source
    @parameters
        [another part's name]
    @effect
        Causes the part to use another part content as the content of this part.
        Only work with PROXY part.
    @endproperty
 */
static void
st_collections_group_parts_part_description_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Description_Proxy *ed;
   char *name;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (current_part->type != EDJE_PART_TYPE_PROXY)
     {
        ERR("parse error %s:%i. source attributes in non-PROXY part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Proxy *)current_desc;

   name = parse_str(0);

   data_queue_part_lookup(pc, name, &(ed->proxy.id));
   free(name);
}

static void
_part_description_state_update(Edje_Part_Description_Common *ed)
{
   Edje_Part *ep = current_part;

   if (ed == ep->default_desc) return;
   if ((ep->default_desc->state.name && !strcmp(ed->state.name, ep->default_desc->state.name) && ed->state.value == ep->default_desc->state.value) ||
       (!ep->default_desc->state.name && !strcmp(ed->state.name, "default") && ed->state.value == ep->default_desc->state.value))
     {
        if (ep->type == EDJE_PART_TYPE_IMAGE)
          _edje_part_description_image_remove((Edje_Part_Description_Image *)ed);

        free(ed);
        ep->other.desc_count--;
        ep->other.desc = realloc(ep->other.desc,
                                 sizeof (Edje_Part_Description_Common *) * ep->other.desc_count);
        current_desc = ep->default_desc;
     }
   else if (ep->other.desc_count)
     {
        unsigned int i;
        for (i = 0; i < ep->other.desc_count - 1; ++i)
          {
             if (!strcmp(ed->state.name, ep->other.desc[i]->state.name) && ed->state.value == ep->other.desc[i]->state.value)
               {
                  if (ep->type == EDJE_PART_TYPE_IMAGE)
                    _edje_part_description_image_remove((Edje_Part_Description_Image *)ed);

                  free(ed);
                  ep->other.desc_count--;
                  ep->other.desc = realloc(ep->other.desc,
                                           sizeof (Edje_Part_Description_Common *) * ep->other.desc_count);
                  current_desc = ep->other.desc[i];
                  break;
               }
          }
     }
}

/**
    @page edcref
    @property
        state
    @parameters
        [name for the description] [index]
    @effect
        Sets a name used to identify a description inside a given part.
        Multiple descriptions are used to declare different states of the same
        part, like "clicked" or "invisible". All states declarations are also
        coupled with an index number between 0.0 and 1.0.
        First description in part must always be "default" 0.0.

        @warning state name "custom" is reserved and can't be used in edc.

        Defaults: "default" 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_state(void)
{
   Edje_Part *ep;
   Edje_Part_Description_Common *ed;
   char *s;
   double val;

   check_min_arg_count(1);

   ep = current_part;

   s = parse_str(0);
   if (!strcmp(s, "custom"))
     {
        ERR("parse error %s:%i. invalid state name: '%s'.",
            file_in, line - 1, s);
        exit(-1);
     }
   if (get_arg_count() == 1)
     val = 0.0;
   else
     val = parse_float_range(1, 0.0, 1.0);

   /* if only default desc exists and current desc is not default, commence paddling */
   if ((!ep->other.desc_count) && (val || (!eina_streq(s, "default"))))
     {
        ERR("parse error %s:%i. invalid state name: '%s'. \"default\" state must always be first.",
            file_in, line - 1, s);
        exit(-1);
     }
   ed = ep->default_desc;
   if (ep->other.desc_count) ed = ep->other.desc[ep->other.desc_count - 1];

   free((void *)ed->state.name);
   ed->state.name = s;
   ed->state.value = val;

   _part_description_state_update(ed);
}

/**
    @page edcref
    @property
        visible
    @parameters
        [0 or 1]
    @effect
        Takes a boolean value specifying whether part is visible (1) or not
        (0). Non-visible parts do not emit signals.

        Defaults: 1
    @endproperty
 */
static void
st_collections_group_parts_part_description_visible(void)
{
   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't have a visibility defined",
            file_in, line - 1);
        exit(-1);
     }

   current_desc->visible = parse_bool(0);
}

static void
st_collections_group_parts_part_description_vis(void)
{
   check_arg_count(0);

   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't have a visibility defined",
            file_in, line - 1);
        exit(-1);
     }

   current_desc->visible = 1;
}

static void
st_collections_group_parts_part_description_hid(void)
{
   check_arg_count(0);

   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't have a visibility defined",
            file_in, line - 1);
        exit(-1);
     }

   current_desc->visible = 0;
}

/**
    @page edcref
    @property
        no_render
    @parameters
        [0 or 1]
    @effect
        Same as setting no_render in part, but can be changed in different states.

        Defaults: 0
    @since 1.19
    @endproperty
 */
static void
st_collections_group_parts_part_description_no_render(void)
{
   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't be marked as no_render",
            file_in, line - 1);
        exit(-1);
     }

   if (check_range_arg_count(0, 1) == 1)
     EDJE_DESC_NO_RENDER_SET(current_desc, parse_bool(0));
   else /* lazEDC form */
     EDJE_DESC_NO_RENDER_SET(current_desc, 1);
}

/**
    @page edcref
    @property
        limit
    @parameters
        [mode]
    @effect
        Emit a signal when the part size change from zero or to a zero size
        ('limit,width,over', 'limit,width,zero'). By default no signal are
        emitted. Valid values are:
        @li NONE
        @li WIDTH
        @li HEIGHT
        @li BOTH

        Defaults: NONE
    @since 1.7
    @endproperty
 */
static void
st_collections_group_parts_part_description_limit(void)
{
   check_arg_count(1);

   current_desc->limit = parse_enum(0,
                                    "NONE", 0,
                                    "WIDTH", 1,
                                    "HEIGHT", 2,
                                    "BOTH", 3);

   if (current_desc->limit)
     {
        Edje_Part_Collection *pc;
        int count;

        pc = eina_list_data_get(eina_list_last(edje_collections));
        count = pc->limits.parts_count++;
        pc->limits.parts = realloc(pc->limits.parts,
                                   pc->limits.parts_count * sizeof (Edje_Part_Limit));
        data_queue_part_reallocated_lookup(pc, current_part->name,
                                           (unsigned char **)&(pc->limits.parts),
                                           (unsigned char *)&pc->limits.parts[count].part - (unsigned char *)pc->limits.parts); //fixme
     }
}

/**
    @page edcref
    @property
        align
    @parameters
        [X axis] [Y axis] ([Z axis])
    @effect
        When the displayed object's size is smaller (or bigger) than
        its container, this property moves it relatively along both
        axis inside its container. @c "0.0" means left/top edges of
        the object touching container's respective ones, while @c
        "1.0" stands for right/bottom edges of the object (on
        horizontal/vertical axis, respectively).
        There is one more parameter for Z axis in case of MESH_NODE.

        Defaults: 0.5 0.5 (0.5)
    @endproperty
 */
static void
st_collections_group_parts_part_description_align(void)
{
   check_has_anchors();

   if (get_arg_count() == 2)
     {
        current_desc->align.x = FROM_DOUBLE(parse_float_range(0, 0.0, 1.0));
        current_desc->align.y = FROM_DOUBLE(parse_float_range(1, 0.0, 1.0));
     }
   else if (get_arg_count() == 3)
     {
        current_desc->align_3d.x = FROM_DOUBLE(parse_float_range(0, 0.0, 1.0));
        current_desc->align_3d.y = FROM_DOUBLE(parse_float_range(1, 0.0, 1.0));
        current_desc->align_3d.z = FROM_DOUBLE(parse_float_range(2, 0.0, 1.0));
     }
}

/**
    @page edcref
    @property
        fixed
    @parameters
        [width, 0 or 1] [height, 0 or 1]
    @effect
        This affects the minimum size calculation. See
        edje_object_size_min_calc() and edje_object_size_min_restricted_calc().
        This tells the min size calculation routine that this part does not
        change size in width or height (1 for it doesn't, 0 for it does), so
        the routine should not try and expand or contract the part.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_fixed(void)
{
   check_has_anchors();
   check_arg_count(2);

   current_desc->fixed.w = parse_bool(0);
   current_desc->fixed.h = parse_bool(1);
}

/**
    @page edcref
    @property
        min
    @parameters
        [width] [height] or SOURCE
    @effect
        The minimum size of the state.

        When min is defined to SOURCE, it will look at the original
        image size and enforce it minimal size to match at least the
        original one. The part must be an IMAGE or a GROUP part.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_min(void)
{
   check_min_arg_count(1);

   if (is_param(1))
     {
        current_desc->min.w = parse_int_range(0, 0, 0x7fffffff);
        current_desc->min.h = parse_int_range(1, 0, 0x7fffffff);
     }
   else
     {
        char *tmp;

        tmp = parse_str(0);
        if ((current_part->type != EDJE_PART_TYPE_IMAGE && current_part->type != EDJE_PART_TYPE_GROUP) ||
            !tmp || strcmp(tmp, "SOURCE") != 0)
          {
             free(tmp);
             ERR("parse error %s:%i. "
                 "Only IMAGE and GROUP part can have a min: SOURCE; defined",
                 file_in, line - 1);
             exit(-1);
          }
        free(tmp);

        current_desc->min.limit = EINA_TRUE;
     }
}

/**
    @page edcref
    @property
        minmul
    @parameters
        [width multiplier] [height multiplier]
    @effect
        A multiplier FORCIBLY applied to whatever minimum size is only during
        minimum size calculation.

        Defaults: 1.0 1.0
    @since 1.2
    @endproperty
 */
static void
st_collections_group_parts_part_description_minmul(void)
{
   check_arg_count(2);

   current_desc->minmul.w = FROM_DOUBLE(parse_float_range(0, 0, 999999));
   current_desc->minmul.h = FROM_DOUBLE(parse_float_range(1, 0, 999999));
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height] or SOURCE
    @effect
        The maximum size of the state. A size of -1 means that it will be ignored in one direction.

        When max is set to SOURCE, edje will enforce the part to be
        not more than the original image size. The part must be an
        IMAGE part.

        Defaults: -1 -1
    @endproperty
 */
static void
st_collections_group_parts_part_description_max(void)
{
   check_min_arg_count(1);

   if (is_param(1))
     {
        current_desc->max.w = parse_int_range(0, -1, 0x7fffffff);
        current_desc->max.h = parse_int_range(1, -1, 0x7fffffff);
     }
   else
     {
        char *tmp;

        tmp = parse_str(0);
        if (current_part->type != EDJE_PART_TYPE_IMAGE ||
            !tmp || strcmp(tmp, "SOURCE") != 0)
          {
             free(tmp);
             ERR("parse error %s:%i. "
                 "Only IMAGE part can have a max: SOURCE; defined",
                 file_in, line - 1);
             exit(-1);
          }
        free(tmp);

        current_desc->max.limit = EINA_TRUE;
     }
}

/**
   @page edcref
   @property
      size_class
   @parameters
      [size class name]
   @effect
      The part will have the min and max size defined in the size class.
      "min" and "max" property in description can be overridden by the size class
      at runtime.
   @endproperty
 */
static void
st_collections_group_parts_part_description_size_class(void)
{
   check_arg_count(1);

   current_desc->size_class = parse_str(0);
}

/**
    @page edcref
    @property
        step
    @parameters
        [width] [height]
    @effect
        Restricts resizing of each dimension to values divisible by its value.
        This causes the part to jump from value to value while resizing. The
        default value is "0 0" disabling stepping.

        Defaults: 0.0 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_step(void)
{
   check_arg_count(2);

   current_desc->step.x = parse_float_range(0, 0, 0x7fffffff);
   current_desc->step.y = parse_float_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        aspect
    @parameters
        [min] [max]
    @effect
        Normally width and height can be resized to any values independently.
        The aspect property forces the width to height ratio to be kept between
        the minimum and maximum set. For example, "1.0 1.0" will increase the
        width a pixel for every pixel added to height. The default value is
        "0.0 0.0" disabling aspect.

        Defaults: 0.0 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_aspect(void)
{
   check_arg_count(2);

   current_desc->aspect.min = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
   current_desc->aspect.max = FROM_DOUBLE(parse_float_range(1, 0.0, 999999999.0));
}

/**
    @page edcref
    @property
        aspect_preference
    @parameters
        [DIMENSION]
    @effect
        Sets the scope of the "aspect" property to a given dimension. Available
        options are BOTH, VERTICAL, HORIZONTAL, SOURCE and NONE

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_description_aspect_preference(void)
{
   check_arg_count(1);

   current_desc->aspect.prefer = parse_enum(0,
                                            "NONE", EDJE_ASPECT_PREFER_NONE,
                                            "VERTICAL", EDJE_ASPECT_PREFER_VERTICAL,
                                            "HORIZONTAL", EDJE_ASPECT_PREFER_HORIZONTAL,
                                            "BOTH", EDJE_ASPECT_PREFER_BOTH,
                                            "SOURCE", EDJE_ASPECT_PREFER_SOURCE,
                                            NULL);
}

/**
    @page edcref
    @property
        color_class
    @parameters
        [color class name]
    @effect
        The part will use the color values of the named color_class, these
        values can be modified by the "color", "color2" and "color3"
        properties set below.
    @endproperty
 */
static void
st_collections_group_parts_part_description_color_class(void)
{
   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't have a color defined",
            file_in, line - 1);
        exit(-1);
     }

   current_desc->color_class = parse_str(0);
}

/**
    @page edcref
    @property
        color
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Sets the main color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        If color_class is set resulting color channel values will be (color * color_class / 255)

        Defaults: 255 255 255 255
    @endproperty
 */
static void
st_collections_group_parts_part_description_color(void)
{
   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't have a color defined",
            file_in, line - 1);
        exit(-1);
     }

   parse_color(0, &(current_desc->color.r));
}

/**
    @page edcref
    @property
        color2
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Sets the text outline color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        If color_class is set resulting color channel values will be (color * color_class / 255)

        Defaults: 0 0 0 255
    @endproperty
 */
static void
st_collections_group_parts_part_description_color2(void)
{
   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
        ERR("parse error %s:%i. SPACER part can't have a color defined",
            file_in, line - 1);
        exit(-1);
     }

   parse_color(0, &(current_desc->color2.r));
}

/**
    @page edcref
    @property
        color3
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Sets the text shadow color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        If color_class is set resulting color channel values will be (color * color_class / 255)

        Defaults: 0 0 0 128
    @endproperty
 */
static void
st_collections_group_parts_part_description_color3(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Description_Text *ed;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (current_part->type != EDJE_PART_TYPE_TEXT
       && current_part->type != EDJE_PART_TYPE_TEXTBLOCK)
     {
        ERR("Setting color3 in part %s from %s not of type TEXT or TEXTBLOCK.",
            current_part->name, pc->part);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   parse_color(0, &(ed->text.color3.r));
}

/**
    @page edcref
    @property
        clip_to
    @parameters
        [another part's name]
    @effect
        Overrides the 'clip_to' property of this part. This allows switching
        clippers (or masks) at runtime by changing this part's state. When
        transitioning between two states, the switch of the clipper shall
        happen at the end of the animation, when the new state is finally set
        (this is similar to the 'visible' flag).
    @endproperty
 */
static void
st_collections_group_parts_part_description_clip_to_id(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->clip_to_id));
      free(name);
   }
}

/**
    @page edcref
    @property
        offset_scale
    @parameters
        [1 or 0]
    @effect
        Makes rel1/2 offset values scale by scale factor like min/max if set
        to 1, otherwise 0 means they will not scale. Note that the part
        as a whole has to be set to scale too like:

            part { name: "partname"; scale: 1;
            ...

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_offset_scale(void)
{
   if (get_arg_count() == 1)
     current_desc->offset_is_scaled = parse_bool(0);
   else
     current_desc->offset_is_scaled = EINA_TRUE;
}

/** @edcsubsection{collections_group_parts_description_relatives,
 *                 Group.Parts.Part.Description.Relatives (rel1/rel2)} */

/**
    @page edcref
    @block
        rel1/rel2/rel
    @context
        description {
            ..
            rel1 {
                relative: 0.0 0.0;
                offset:     0   0;
            }
            ..
            rel2 {
                relative: 1.0 1.0;
                offset:    -1  -1;
            }
            ..
            rel {
                to: "somepart";
            }
            ..
        }
    @description
        The rel1 and rel2 blocks are used to define the position of each corner
        of the part's container. With rel1 being the left-up corner and rel2
        being the right-down corner; rel (no number) is equivalent to setting both
        rel1 AND rel2 since 1.14.
    @endblock

    @property
        relative
    @parameters
        [X axis] [Y axis]
    @effect
        Moves a corner to a relative position inside the container of the
        relative "to" part. Values from 0.0 (0%, beginning) to 1.0 (100%, end)
        of each axis.

        Defaults:
        @li rel1.relative: 0.0 0.0
        @li rel2.relative: 1.0 1.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_rel1_relative(void)
{
   check_has_anchors();
   check_arg_count(2);

   current_desc->rel1.relative_x = FROM_DOUBLE(parse_float(0));
   current_desc->rel1.relative_y = FROM_DOUBLE(parse_float(1));
}

/**
    @page edcref
    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Affects the corner position a fixed number of pixels along each axis.

        Defaults:
        @li rel1.offset: 0 0
        @li rel2.offset: -1 -1
    @endproperty
 */
static void
st_collections_group_parts_part_description_rel1_offset(void)
{
   check_has_anchors();
   check_arg_count(2);

   current_desc->rel1.offset_x = parse_int(0);
   current_desc->rel1.offset_y = parse_int(1);
}

/**
    @page edcref
    @property
        to
    @parameters
        [another part's name]
    @effect
        Causes a corner to be positioned relatively to another part's
        container. Setting to "" will unset this value for inherited
        parts.
    @endproperty
 */
static void
st_collections_group_parts_part_description_rel1_to_set(const char *name)
{
   Edje_Part_Collection *pc;
   pc = eina_list_data_get(eina_list_last(edje_collections));
   data_queue_part_lookup(pc, name, &(current_desc->rel1.id_x));
   data_queue_part_lookup(pc, name, &(current_desc->rel1.id_y));
}

static void
st_collections_group_parts_part_description_rel_to(void)
{
   check_has_anchors();
   check_arg_count(1);

   {
      char *name;
      name = parse_str(0);
      st_collections_group_parts_part_description_rel1_to_set(name);
      st_collections_group_parts_part_description_rel2_to_set(name);
      free(name);
   }
}

static void
st_collections_group_parts_part_description_rel1_to(void)
{
   check_has_anchors();
   check_arg_count(1);

   {
      char *name;
      name = parse_str(0);
      st_collections_group_parts_part_description_rel1_to_set(name);
      free(name);
   }
}

/**
    @page edcref
    @property
        to_x
    @parameters
        [another part's name]
    @effect
        Causes a corner to be positioned relatively to the X axis of another
        part's container. Simply put affects the first parameter of "relative".
        Setting to "" will unset this value for inherited parts.
    @endproperty
 */
static void
st_collections_group_parts_part_description_rel_to_x(void)
{
   Edje_Part_Collection *pc;

   check_has_anchors();
   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel1.id_x));
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_x));
      free(name);
   }
}

static void
st_collections_group_parts_part_description_rel1_to_x(void)
{
   Edje_Part_Collection *pc;

   check_has_anchors();
   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel1.id_x));
      free(name);
   }
}

/**
    @page edcref
    @property
        to_y
    @parameters
        [another part's name]
    @effect
        Causes a corner to be positioned relatively to the Y axis of another
        part's container. Simply put, affects the second parameter of
        "relative". Setting to "" will unset this value for inherited parts.
    @endproperty
 */
static void
st_collections_group_parts_part_description_rel_to_y(void)
{
   Edje_Part_Collection *pc;

   check_has_anchors();
   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel1.id_y));
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
      free(name);
   }
}

static void
st_collections_group_parts_part_description_rel1_to_y(void)
{
   Edje_Part_Collection *pc;

   check_has_anchors();
   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel1.id_y));
      free(name);
   }
}

static void
st_collections_group_parts_part_description_rel2_relative(void)
{
   check_has_anchors();
   check_arg_count(2);

   current_desc->rel2.relative_x = FROM_DOUBLE(parse_float(0));
   current_desc->rel2.relative_y = FROM_DOUBLE(parse_float(1));
}

static void
st_collections_group_parts_part_description_rel2_offset(void)
{
   check_has_anchors();
   check_arg_count(2);

   current_desc->rel2.offset_x = parse_int(0);
   current_desc->rel2.offset_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_rel2_to_set(const char *name)
{
   Edje_Part_Collection *pc;
   pc = eina_list_data_get(eina_list_last(edje_collections));
   data_queue_part_lookup(pc, name, &(current_desc->rel2.id_x));
   data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
}

static void
st_collections_group_parts_part_description_rel2_to(void)
{
   check_has_anchors();
   check_arg_count(1);

   {
      char *name;
      name = parse_str(0);
      st_collections_group_parts_part_description_rel2_to_set(name);
      free(name);
   }
}

static void
st_collections_group_parts_part_description_rel2_to_x(void)
{
   Edje_Part_Collection *pc;

   check_has_anchors();
   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_x));
      free(name);
   }
}

static void
st_collections_group_parts_part_description_rel2_to_y(void)
{
   Edje_Part_Collection *pc;

   check_has_anchors();
   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
      free(name);
   }
}

/** @edcsubsection{collections_group_parts_description_anchors,
 *                 Group.Parts.Part.Description.Anchors} */

/**
    @page edcref
    @block
        anchors
    @context
        // This part will be expanded from the top-left corner of edje group
        part { name : "part1";
            description { state: "default" 0.0;
                anchors {
                    top: GROUP TOP;
                    left: GROUP; // This means 'left: GROUP LEFT;'
                }
                min: 50 50;
            }
        }
        // This part will be expanded from the bottom-right corner of "part1"
        // to the bottom-right
        part { name: "part2";
            description { state: "default" 0.0;
                anchors {
                    top: "part1" BOTTOM;
                    left: "part1"; // This means 'left: "part1" RIGHT;'
                }
                min: 50 50;
            }
        }
        // This part will be expanded from the right edje of "part2" to the right
        part { name: "part3";
            description { state: "default" 0.0;
                anchors {
                    left: "part2";
                    fill: "part2" VERTICAL;
                }
                min: 100 0; // The height will be determined by the height of "part2"
            }
        }
        // This part will be expanded from the center of right edge of "part3"
        // to the bottom-right corner of edje group
        part { name: "part4";
            description { state: "default" 0.0;
                anchors {
                    top: "part3" VERTICAL_CENTER;
                    left: "part3";
                    right: GROUP;
                    bottom: GROUP;
                }
            }
        }
    @description
        The anchors blocks are used to define the position of each edge of
        the part's container. Anchors will change relative, align and fixed
        attributes internally, so setting both of them is not allowed.
        When the second parameter of position enumeration is omitted, anchoring
        a part to the other part will put the part adjacent to the given part.
        However, if the part is anchored to edje group, the part will be contained
        inside the group.
    @endblock

    @property
        anchors
    @parameters
        [partname] [the edge of other part]
    @effect
        Moves an edge of the part to the position of the edge of given part or
        whole edje group. (GROUP means edje group that the part belong to)
    @endproperty
 */

static void
check_has_anchors(void)
{
   if (current_anchors)
     {
        ERR("parse error %s:%i. Anchors and Relatives(rel/align/fixed) cannot be used at the same time.",
            file_in, line - 1);
        exit(-1);
     }

   has_relatives = EINA_TRUE;
}

static void
check_has_relatives(void)
{
   if (!beta)
     error_and_abort(NULL, "Anchors are currently a beta feature, please enable them by running edje_cc with -beta.");

   if (has_relatives)
     {
        ERR("parse error %s:%i. Anchors and Relatives(rel/align/fixed) cannot be used at the same time.",
            file_in, line - 1);
        exit(-1);
     }

   current_desc->offset_is_scaled = EINA_TRUE;
}

static void
parse_anchor_line(Edje_Part_Anchor *anchor, Edje_Part_Anchor_Line undefined)
{
   int nargs;
   char *name;

   nargs = get_arg_count();
   if (!nargs || (nargs > 2))
     {
        ERR("parse error %s:%i. Anchors should have a name of part and base line.",
            file_in, line - 1);
        exit(-1);
     }

   name = parse_str(0);
   anchor->set = EINA_TRUE;

   if (nargs == 2)
     anchor->base.line = parse_enum(1,
                                    "TOP", EDJE_PART_ANCHOR_LINE_TOP,
                                    "BOTTOM", EDJE_PART_ANCHOR_LINE_BOTTOM,
                                    "LEFT", EDJE_PART_ANCHOR_LINE_LEFT,
                                    "RIGHT", EDJE_PART_ANCHOR_LINE_RIGHT,
                                    "VERTICAL_CENTER", EDJE_PART_ANCHOR_LINE_VERTICAL_CENTER,
                                    "HORIZONTAL_CENTER", EDJE_PART_ANCHOR_LINE_HORIZONTAL_CENTER,
                                    "*", EDJE_PART_ANCHOR_LINE_RELATIVE,
                                    NULL);
   else if (strcmp(name, "GROUP") || param_had_quote(0))
     anchor->base.line = undefined;

   free(name);
}

static void
parse_anchor_fill(Edje_Part_Anchor *anchor)
{
   int nargs;

   nargs = get_arg_count();
   if (!nargs || (nargs > 2))
     {
        ERR("parse error %s:%i. Anchors should have a name of part and base line.",
            file_in, line - 1);
        exit(-1);
     }

   anchor->set = EINA_TRUE;

   if (nargs == 2)
     anchor->base.fill = parse_enum(1,
                                    "BOTH", EDJE_PART_ANCHOR_FILL_BOTH,
                                    "HORIZONTAL", EDJE_PART_ANCHOR_FILL_HORIZONTAL,
                                    "VERTICAL", EDJE_PART_ANCHOR_FILL_VERTICAL,
                                    NULL);
   else
     anchor->base.fill = EDJE_PART_ANCHOR_FILL_BOTH;
}

static void
anchor_queue_part_lookup(int *part, int *counterpart, Eina_Bool counterpart_is_set)
{
   Edje_Part_Collection *pc;
   char *name;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   name = parse_str(0);
   if (!strcmp(name, "GROUP") && !param_had_quote(0))
     goto end;

   data_queue_part_lookup(pc, name, part);

   if (!counterpart_is_set)
     data_queue_part_lookup(pc, name, counterpart);

end:
   free(name);
}

static void
anchor_dequeue_part_lookup(int *part, Eina_Bool counterpart_is_set)
{
   Edje_Part_Collection *pc;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (counterpart_is_set && part)
     part_lookup_del(pc, part);
}

static void
anchor_adjust_align(FLOAT_T *align, FLOAT_T val, unsigned char *fixed, Eina_Bool counterpart_is_set)
{
   if (counterpart_is_set)
     {
        *align = 0.5;
        *fixed = 0;
     }
   else
     {
        *align = val;
        *fixed = 1;
     }
}

static void
anchor_adjust_relative(const Edje_Part_Anchor_Line *anchor_lines, FLOAT_T *rel, FLOAT_T *relc, Edje_Part_Anchor_Line anchor_line, Edje_Part_Anchor_Line base, Eina_Bool counterpart_is_set)
{
   if (anchor_line == EDJE_PART_ANCHOR_LINE_NONE)
     anchor_line = base;

   if (anchor_line == anchor_lines[0])
     {
        *rel = FROM_DOUBLE(0.0);
        if (!counterpart_is_set)
          *relc = FROM_DOUBLE(0.0);
     }
   else if (anchor_line == anchor_lines[1])
     {
        *rel = FROM_DOUBLE(1.0);
        if (!counterpart_is_set)
          *relc = FROM_DOUBLE(1.0);
     }
   else if (anchor_line == anchor_lines[2])
     {
        *rel = FROM_DOUBLE(0.5);
        if (!counterpart_is_set)
          *relc = FROM_DOUBLE(0.5);
     }
   else if (anchor_line == EDJE_PART_ANCHOR_LINE_RELATIVE)
     {
        *rel = FROM_DOUBLE(parse_float(1));
        if (!counterpart_is_set)
          *relc = FROM_DOUBLE(parse_float(1));
     }
   else
     {
        ERR("parse error %s:%i. Edje part is anchored to wrong position.",
            file_in, line - 1);
        exit(-1);
     }
}

static void
anchor_adjust_relative_vertical(FLOAT_T *rel, FLOAT_T *relc, Edje_Part_Anchor_Line anchor_line, Edje_Part_Anchor_Line base, Eina_Bool counterpart_is_set)
{
   static const Edje_Part_Anchor_Line anchor_lines[] = {
      EDJE_PART_ANCHOR_LINE_TOP,
      EDJE_PART_ANCHOR_LINE_BOTTOM,
      EDJE_PART_ANCHOR_LINE_VERTICAL_CENTER
   };

   anchor_adjust_relative(anchor_lines, rel, relc, anchor_line, base, counterpart_is_set);
}

static void
anchor_adjust_relative_horizontal(FLOAT_T *rel, FLOAT_T *relc, Edje_Part_Anchor_Line anchor_line, Edje_Part_Anchor_Line base, Eina_Bool counterpart_is_set)
{
   static const Edje_Part_Anchor_Line anchor_lines[] = {
      EDJE_PART_ANCHOR_LINE_LEFT,
      EDJE_PART_ANCHOR_LINE_RIGHT,
      EDJE_PART_ANCHOR_LINE_HORIZONTAL_CENTER
   };

   anchor_adjust_relative(anchor_lines, rel, relc, anchor_line, base, counterpart_is_set);
}

/**
    @page edcref
    @property
        top
    @parameters
        [partname] [TOP/BOTTOM/VERTICAL_CENTER]
    @effect
        Causes top edge to be positioned to the edge of another part's container.
        Setting to GROUP will indicate edje group instead of another part.
        If bottom anchor is not set, edje part will be expanded to the bottom.
        The second parameter of position enumeration can be omitted. (Default
        value is BOTTOM, but TOP when the part is anchored to edje group)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_top(void)
{
   Eina_Bool counterpart_is_set;

   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   counterpart_is_set = current_anchors->bottom.set;

   parse_anchor_line(&(current_anchors->top), EDJE_PART_ANCHOR_LINE_BOTTOM);

   anchor_dequeue_part_lookup(&(current_desc->rel1.id_y), counterpart_is_set);
   anchor_queue_part_lookup(&(current_desc->rel1.id_y), &(current_desc->rel2.id_y), counterpart_is_set);

   anchor_adjust_align(&(current_desc->align.y), 0.0, &(current_desc->fixed.h), counterpart_is_set);
   anchor_adjust_relative_vertical(&(current_desc->rel1.relative_y), &(current_desc->rel2.relative_y), current_anchors->top.base.line, EDJE_PART_ANCHOR_LINE_TOP, counterpart_is_set);
}

/**
    @page edcref
    @property
        bottom
    @parameters
        [partname] [TOP/BOTTOM/VERTICAL_CENTER]
    @effect
        Causes bottom edge to be positioned to the edge of another part's container.
        Setting to GROUP will indicate edje group instead of another part.
        If top anchor is not set, edje part will be expanded to the top.
        The second parameter of position enumeration can be omitted. (Default
        value is TOP, but BOTTOM when the part is anchored to edje group)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_bottom(void)
{
   Eina_Bool counterpart_is_set;

   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   counterpart_is_set = current_anchors->top.set;

   parse_anchor_line(&(current_anchors->bottom), EDJE_PART_ANCHOR_LINE_TOP);

   anchor_dequeue_part_lookup(&(current_desc->rel2.id_y), counterpart_is_set);
   anchor_queue_part_lookup(&(current_desc->rel2.id_y), &(current_desc->rel1.id_y), counterpart_is_set);

   anchor_adjust_align(&(current_desc->align.y), 1.0, &(current_desc->fixed.h), counterpart_is_set);
   anchor_adjust_relative_vertical(&(current_desc->rel2.relative_y), &(current_desc->rel1.relative_y), current_anchors->bottom.base.line, EDJE_PART_ANCHOR_LINE_BOTTOM, counterpart_is_set);
}

/**
    @page edcref
    @property
        left
    @parameters
        [partname] [LEFT/RIGHT/HORIZONTAL_CENTER]
    @effect
        Causes left edge to be positioned to the edge of another part's container.
        Setting to GROUP will indicate edje group instead of another part.
        If right anchor is not set, edje part will be expanded to the right.
        The second parameter of position enumeration can be omitted. (Default
        value is RIGHT, but LEFT when the part is anchored to edje group)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_left(void)
{
   Eina_Bool counterpart_is_set;

   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   counterpart_is_set = current_anchors->right.set;

   parse_anchor_line(&(current_anchors->left), EDJE_PART_ANCHOR_LINE_RIGHT);

   anchor_dequeue_part_lookup(&(current_desc->rel1.id_x), counterpart_is_set);
   anchor_queue_part_lookup(&(current_desc->rel1.id_x), &(current_desc->rel2.id_x), counterpart_is_set);

   anchor_adjust_align(&(current_desc->align.x), 0.0, &(current_desc->fixed.w), counterpart_is_set);
   anchor_adjust_relative_horizontal(&(current_desc->rel1.relative_x), &(current_desc->rel2.relative_x), current_anchors->left.base.line, EDJE_PART_ANCHOR_LINE_LEFT, counterpart_is_set);
}

/**
    @page edcref
    @property
        right
    @parameters
        [partname] [LEFT/RIGHT/HORIZONTAL_CENTER]
    @effect
        Causes right edge to be positioned to the edge of another part's container.
        Setting to GROUP will indicate edje group instead of another part.
        If left anchor is not set, edje part will be expanded to the left.
        The second parameter of position enumeration can be omitted. (Default
        value is LEFT, but RIGHT when the part is anchored to edje group)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_right(void)
{
   Eina_Bool counterpart_is_set;

   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   counterpart_is_set = current_anchors->left.set;

   parse_anchor_line(&(current_anchors->right), EDJE_PART_ANCHOR_LINE_LEFT);

   anchor_dequeue_part_lookup(&(current_desc->rel2.id_x), counterpart_is_set);
   anchor_queue_part_lookup(&(current_desc->rel2.id_x), &(current_desc->rel1.id_x), counterpart_is_set);

   anchor_adjust_align(&(current_desc->align.x), 1.0, &(current_desc->fixed.w), counterpart_is_set);
   anchor_adjust_relative_horizontal(&(current_desc->rel2.relative_x), &(current_desc->rel1.relative_x), current_anchors->right.base.line, EDJE_PART_ANCHOR_LINE_RIGHT, counterpart_is_set);
}

/**
    @page edcref
    @property
        vertical_center
    @parameters
        [partname] [TOP/BOTTOM/VERTICAL_CENTER]
    @effect
        Causes (virtual) vertical center line to be positioned to the edge of
        another part's container. Setting to GROUP will indicate edje group instead
        of another part.
        This part will be expanded vertically in both directions, so do not
        set top or bottom anchor with vertical_center anchor.
        The second parameter of position enumeration can be omitted. (Default
        value is VERTICAL_CENTER)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_vertical_center(void)
{
   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   parse_anchor_line(&(current_anchors->vertical_center), EDJE_PART_ANCHOR_LINE_VERTICAL_CENTER);

   anchor_queue_part_lookup(&(current_desc->rel1.id_y), &(current_desc->rel2.id_y), EINA_FALSE);

   anchor_adjust_align(&(current_desc->align.y), 0.5, &(current_desc->fixed.h), EINA_FALSE);
   anchor_adjust_relative_vertical(&(current_desc->rel1.relative_y), &(current_desc->rel2.relative_y), current_anchors->vertical_center.base.line, EDJE_PART_ANCHOR_LINE_VERTICAL_CENTER, EINA_FALSE);
}

/**
    @page edcref
    @property
        horizontal_center
    @parameters
        [partname] [LEFT/RIGHT/HORIZONTAL_CENTER]
    @effect
        Causes (virtual) horizontal center line to be positioned to the edge of
        another part's container. Setting to GROUP will indicate edje group instead
        of another part.
        This part will be expanded horizontally in both directions, so do not
        set left or right anchor with vertical_center anchor.
        The second parameter of position enumeration can be omitted. (Default
        value is HORIZONTAL_CENTER)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_horizontal_center(void)
{
   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   parse_anchor_line(&(current_anchors->horizontal_center), EDJE_PART_ANCHOR_LINE_HORIZONTAL_CENTER);

   anchor_queue_part_lookup(&(current_desc->rel1.id_x), &(current_desc->rel2.id_x), EINA_FALSE);

   anchor_adjust_align(&(current_desc->align.x), 0.5, &(current_desc->fixed.w), EINA_FALSE);
   anchor_adjust_relative_horizontal(&(current_desc->rel1.relative_x), &(current_desc->rel2.relative_x), current_anchors->horizontal_center.base.line, EDJE_PART_ANCHOR_LINE_HORIZONTAL_CENTER, EINA_FALSE);
}

/**
    @page edcref
    @property
        fill
    @parameters
        [partname] [BOTH/HORIZONTAL/VERTICAL]
    @effect
        Causes the part's container to expand to the width or height of another
        part's container. Setting to GROUP will indicate edje group instead of another part.
        Setting horizontal fill has same effect to setting top and bottom anchors
        to the same part.
        (setting vertical fill means left and right anchors to the same part)
        The second parameter of direction enumeration can be omitted. (Default
        value is BOTH)
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_fill(void)
{
   Edje_Part_Collection *pc;
   char *name;

   pc = eina_list_last_data_get(edje_collections);

   check_has_relatives();

   if (!current_anchors)
     current_anchors = mem_alloc(SZ(Edje_Part_Description_Anchors));

   parse_anchor_fill(&(current_anchors->fill));

   name = parse_str(0);

   switch (current_anchors->fill.base.fill)
     {
      case EDJE_PART_ANCHOR_FILL_BOTH:
        if (strcmp("GROUP", name) || param_had_quote(0))
          {
             data_queue_part_lookup(pc, name, &(current_desc->rel1.id_x));
             data_queue_part_lookup(pc, name, &(current_desc->rel2.id_x));
             data_queue_part_lookup(pc, name, &(current_desc->rel1.id_y));
             data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
          }
        current_desc->align.x = 0.5;
        current_desc->align.y = 0.5;
        current_desc->fixed.w = 0;
        current_desc->fixed.h = 0;
        break;

      case EDJE_PART_ANCHOR_FILL_HORIZONTAL:
        if (strcmp("GROUP", name) || param_had_quote(0))
          {
             data_queue_part_lookup(pc, name, &(current_desc->rel1.id_x));
             data_queue_part_lookup(pc, name, &(current_desc->rel2.id_x));
          }
        current_desc->align.x = 0.5;
        current_desc->fixed.w = 0;
        break;

      case EDJE_PART_ANCHOR_FILL_VERTICAL:
        if (strcmp("GROUP", name) || param_had_quote(0))
          {
             data_queue_part_lookup(pc, name, &(current_desc->rel1.id_y));
             data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
          }
        current_desc->align.y = 0.5;
        current_desc->fixed.h = 0;
        break;
     }

   free(name);
}

/**
    @page edcref
    @property
        margin
    @parameters
        [left] [right] [top] [bottom]
    @effect
        Affects the edge position a fixed number of pixels along each direction.
        Margins will scale its size with an edje scaling factor.
    @endproperty
 */
static void
st_collections_group_parts_part_description_anchors_margin(void)
{
   check_has_relatives();
   check_arg_count(4);

   current_desc->rel1.offset_x = parse_int(0);
   current_desc->rel2.offset_x = -parse_int(1) - 1;
   current_desc->rel1.offset_y = parse_int(2);
   current_desc->rel2.offset_y = -parse_int(3) - 1;
}

static void
free_anchors(void)
{
   has_relatives = EINA_FALSE;

   if (!current_anchors) return;

   free(current_anchors);
   current_anchors = NULL;
}

/** @edcsubsection{collections_group_parts_description_image,
 *                 Group.Parts.Part.Description.Image} */

/**
    @page edcref
    @block
        image
    @context
        description {
            ..
            image {
                normal: "filename.ext";
                normal: "filename.svg";
                tween:  "filename2.ext";
                ..
                tween:  "filenameN.ext";
                border:  left right top bottom;
                middle:  0/1/NONE/DEFAULT/SOLID;
                fill { }
            }
            ..
        }
    @description
    @endblock

    @property
        normal
    @parameters
        [image's filename]
    @effect
        Name of image to be used as previously declared in the  images block.
        In an animation, this is the first and last image displayed. It's
        required in any image part
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_normal(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_VECTOR)
     {
        return _handle_vector_image();
     }

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. "
            "image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   {
      char *name;

      if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
        ed->image.set = EINA_TRUE;

      name = parse_str(0);
      data_queue_image_remove(&(ed->image.id), &(ed->image.set));
      data_queue_image_lookup(name, &(ed->image.id), &(ed->image.set));
      free(name);
   }
}

/**
    @page edcref
    @property
        tween
    @parameters
        [image's filename]
    @effect
        Name of an image to be used in an animation loop, an image block can
        have none, one or multiple tween declarations. Images are displayed in
        the order they are listed, during the transition to the state they are
        declared in; the "normal" image is the final state.
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_tween(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   {
      char *name;
      Edje_Part_Image_Id *iid;

      iid = mem_alloc(SZ(Edje_Part_Image_Id));
      ed->image.tweens_count++;
      ed->image.tweens = realloc(ed->image.tweens,
                                 sizeof (Edje_Part_Image_Id *) * ed->image.tweens_count);
      ed->image.tweens[ed->image.tweens_count - 1] = iid;
      name = parse_str(0);
      data_queue_image_remove(&(iid->id), &(iid->set));
      data_queue_image_lookup(name, &(iid->id), &(iid->set));
      free(name);
   }
}

/**
    @page edcref
    @property
        border
    @parameters
        [left] [right] [top] [bottom]
    @effect
        If set, the area (in pixels) of each side of the image will be
        displayed as a fixed size border, from the side -> inwards, preventing
        the corners from being changed on a resize.

        Defaults: 0 0 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_border(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(4);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   ed->image.border.l = parse_int_range(0, 0, 0x7fffffff);
   ed->image.border.r = parse_int_range(1, 0, 0x7fffffff);
   ed->image.border.t = parse_int_range(2, 0, 0x7fffffff);
   ed->image.border.b = parse_int_range(3, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        middle
    @parameters
        [mode]
    @effect
        If border is set, this value tells Edje if the rest of the
        image (not covered by the defined border) will be displayed or not
        or be assumed to be solid (without alpha). The default is 1/DEFAULT.
        Valid values are:
        @li 0 or NONE
        @li 1 or DEFAULT
        @li SOLID
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_middle(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   ed->image.border.no_fill = parse_enum(0,
                                         "1", 0,
                                         "DEFAULT", 0,
                                         "0", 1,
                                         "NONE", 1,
                                         "SOLID", 2,
                                         NULL);
}

/**
    @page edcref
    @property
        border_scale_by
    @parameters
        [value]
    @effect
        If border scaling is enabled then normally the OUTPUT border sizes
        (e.g. if 3 pixels on the left edge are set as a border, then normally
        at scale 1.0, those 3 columns will always be the exact 3 columns of
        output, or at scale 2.0 they will be 6 columns, or 0.33 they will merge
        into a single column). This property multiplies the input scale
        factor by this multiplier, allowing the creation of "supersampled"
        borders to make much higher resolution outputs possible by always using
        the highest resolution artwork and then runtime scaling it down.

        value can be: 0.0 or bigger (0.0 or 1.0 to turn it off)

        Defaults: 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_border_scale_by(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   ed->image.border.scale_by = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
}

/**
    @page edcref
    @property
        border_scale
    @parameters
        [0/1]
    @effect
        If border is set, this value tells Edje if the border should be scaled
        by the object/global edje scale factors

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_border_scale(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   ed->image.border.scale = parse_bool(0);
}

/**
    @page edcref
    @property
        scale_hint
    @parameters
        [mode]
    @effect
        Sets the evas image scale hint letting the engine more effectively save
        cached copies of the scaled image if it makes sense.
        Valid values are:
        @li 0 or NONE
        @li DYNAMIC
        @li STATIC

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_description_image_scale_hint(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image *)current_desc;

   ed->image.scale_hint = parse_enum(0,
                                     "NONE", EVAS_IMAGE_SCALE_HINT_NONE,
                                     "DYNAMIC", EVAS_IMAGE_SCALE_HINT_DYNAMIC,
                                     "STATIC", EVAS_IMAGE_SCALE_HINT_STATIC,
                                     "0", EVAS_IMAGE_SCALE_HINT_NONE,
                                     NULL);
}

/** @edcsubsection{collections_group_parts_description_fill,
 *                 Group.Parts.Part.Description.Fill} */

/**
    @page edcref
    @block
        fill
    @context
        part { type: [IMAGE or PROXY];
            description {
                ..
                fill {
                    type: SCALE;
                    smooth: 0-1;
                    origin { }
                    size { }
                }
                ..
            }
            ..
        }
    @description
        The fill method is an optional block that defines the way an IMAGE or
        PROXY part is going to be displayed inside its container.
        It can be used for tiling (repeating the image) or displaying only
        part of an image. See @ref evas_object_image_fill_set() documentation
        for more details.
    @endblock

    @property
        smooth
    @parameters
        [0 or 1]
    @effect
        The smooth property takes a boolean value to decide if the image will
        be smoothed on scaling (1) or not (0).

        Defaults: 1
    @endproperty
 */
static void
st_collections_group_parts_part_description_fill_smooth(void)
{
   Edje_Part_Description_Spec_Fill *fill;

   check_arg_count(1);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = (Edje_Part_Description_Image *)current_desc;

         fill = &ed->image.fill;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = (Edje_Part_Description_Proxy *)current_desc;

         fill = &ed->proxy.fill;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. "
             "image and proxy attributes in non-IMAGE, non-PROXY `%s` part (%i).",
             file_in, line - 1, current_part->name, current_part->type);
         exit(-1);
      }
     }

   fill->smooth = parse_bool(0);
}

/**
    @page edcref
    @property
        type
    @parameters
        [fill type]
    @effect
        Sets the image fill type. SCALE - image will be scaled accordingly params
        value 'relative' and 'offset' from 'origin' and 'size' blocks.
        TILE - image will be tiled accordingly params value 'relative' and
        'offset' from 'origin' and 'size' blocks. Important: the part parameter
        'min' must be setted, it's size of tiled image. If parameter 'max' setted
        tiled area will has the size accordingly 'max' values.

        Valid values are:
        @li SCALE
        @li TILE

        Defaults: SCALE
    @endproperty
 */
static void
st_collections_group_parts_part_description_fill_type(void)
{
   Edje_Part_Description_Spec_Fill *fill;

   check_arg_count(1);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = (Edje_Part_Description_Image *)current_desc;

         fill = &ed->image.fill;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = (Edje_Part_Description_Proxy *)current_desc;

         fill = &ed->proxy.fill;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. "
             "image and proxy attributes in non-IMAGE, non-PROXY part.",
             file_in, line - 1);
         exit(-1);
      }
     }

   fill->type = parse_enum(0,
                           "SCALE", EDJE_FILL_TYPE_SCALE,
                           "TILE", EDJE_FILL_TYPE_TILE,
                           NULL);
}

/** @edcsubsection{collections_group_parts_description_fill_origin,
 *                 Group.Parts.Part.Description.Fill.Origin} */

/**
    @page edcref
    @block
        origin
    @context
        image {
            ..
            fill {
                ..
                origin {
                    relative: 0.0 0.0;
                    offset:   0   0;
                }
                ..
            }
            ..
        }
    @description
        The origin block is used to place the starting point, inside the
        displayed element, that will be used to render the tile. By default,
        the origin is set at the element's left-up corner.
    @endblock

    @property
        relative
    @parameters
        [X axis] [Y axis]
    @effect
        Sets the starting point relatively to displayed element's content.

        Defaults: 0.0 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_fill_origin_relative(void)
{
   Edje_Part_Description_Spec_Fill *fill;

   check_arg_count(2);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = (Edje_Part_Description_Image *)current_desc;

         fill = &ed->image.fill;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = (Edje_Part_Description_Proxy *)current_desc;

         fill = &ed->proxy.fill;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. "
             "image and proxy attributes in non-IMAGE, non-PROXY part.",
             file_in, line - 1);
         exit(-1);
      }
     }

   fill->pos_rel_x = FROM_DOUBLE(parse_float_range(0, -999999999.0, 999999999.0));
   fill->pos_rel_y = FROM_DOUBLE(parse_float_range(1, -999999999.0, 999999999.0));
}

/**
    @page edcref
    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Affects the starting point a fixed number of pixels along each axis.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_fill_origin_offset(void)
{
   Edje_Part_Description_Spec_Fill *fill;

   check_arg_count(2);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = (Edje_Part_Description_Image *)current_desc;

         fill = &ed->image.fill;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = (Edje_Part_Description_Proxy *)current_desc;

         fill = &ed->proxy.fill;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. "
             "image and proxy attributes in non-IMAGE, non-PROXY part.",
             file_in, line - 1);
         exit(-1);
      }
     }

   fill->pos_abs_x = parse_int(0);
   fill->pos_abs_y = parse_int(1);
}

/** @edcsubsection{collections_group_parts_description_fill_size,
 *                 Group.Parts.Part.Description.Fill.Size} */

/**
    @page edcref
    @block
        size
    @context
        image {
            ..
            fill {
                ..
                size {
                    relative: 1.0 1.0;
                    offset:  -1  -1;
                }
                ..
            }
            ..
        }
    @description
        The size block defines the tile size of the content that will be
        displayed.
    @endblock

    @property
        relative
    @parameters
        [width] [height]
    @effect
        Takes a pair of decimal values that represent the percentual value
        of the original size of the element. For example, "0.5 0.5" represents
        half the size, while "2.0 2.0" represents the double.

        Defaults: 1.0 1.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_fill_size_relative(void)
{
   Edje_Part_Description_Spec_Fill *fill;

   check_arg_count(2);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = (Edje_Part_Description_Image *)current_desc;

         fill = &ed->image.fill;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = (Edje_Part_Description_Proxy *)current_desc;

         fill = &ed->proxy.fill;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. "
             "image and proxy attributes in non-IMAGE, non-PROXY part.",
             file_in, line - 1);
         exit(-1);
      }
     }

   fill->rel_x = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
   fill->rel_y = FROM_DOUBLE(parse_float_range(1, 0.0, 999999999.0));
}

/**
    @page edcref
    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Affects the size of the tile a fixed number of pixels along each axis.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_fill_size_offset(void)
{
   Edje_Part_Description_Spec_Fill *fill;

   check_arg_count(2);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *ed;

         ed = (Edje_Part_Description_Image *)current_desc;

         fill = &ed->image.fill;
         break;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *ed;

         ed = (Edje_Part_Description_Proxy *)current_desc;

         fill = &ed->proxy.fill;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. "
             "image and proxy attributes in non-IMAGE, non-PROXY part.",
             file_in, line - 1);
         exit(-1);
      }
     }

   fill->abs_x = parse_int(0);
   fill->abs_y = parse_int(1);
}

/** @edcsubsection{collections_group_parts_description_text,
 *                 Group.Parts.Part.Description.Text} */

/**
    @page edcref

    @block
        text
    @context
        part {
            description {
                ..
                text {
                    text:        "some string of text to display";
                    domain:      "domain_name";
                    font:        "font_name";
                    size:         SIZE;
                    text_class:  "class_name";
                    fit:          horizontal vertical;
                    min:          horizontal vertical;
                    max:          horizontal vertical;
                    align:        X-axis     Y-axis;
                    source:      "part_name";
                    text_source: "text_part_name";
                    ellipsis:     -1.0 (since 1.8), 0.0-1.0;
                    style:       "stylename";
                }
                ..
            }
        }
    @description
    @endblock

    @property
        text
    @parameters
        [a string of text, or nothing]
    @effect
        Sets the default content of a text part, normally the application is
        the one changing its value.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_text(void)
{
   Edje_Part_Description_Text *ed;
   char *str = NULL;
   int i;

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   for (i = 0;; i++)
     {
        char *s;

        if (!is_param(i)) break;
        s = parse_str(i);
        if (!str) str = s;
        else
          {
             str = realloc(str, strlen(str) + strlen(s) + 1);
             strcat(str, s);
             free(s);
          }
     }
   ed->text.text.str = str;
}

/**
    @page edcref

    @property
        domain
    @parameters
        [domain name]
    @effect
        This is the domain name of the .mo file which has to be checked
        for translation.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_domain(void)
{
   Edje_Part_Description_Text *ed;

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.domain = parse_str(0);
}

/**
    @page edcref

    @property
        text_class
    @parameters
        [text class name]
    @effect
        Similar to color_class, this is the name used by the application
        to alter the font family and size at runtime.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_text_class(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.text_class = parse_str(0);
}

/**
    @page edcref

    @property
        font
    @parameters
        [font alias]
    @effect
        This sets the font family to one of the aliases set up in the "fonts"
        block. Can be overridden by the application.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_font(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.font.str = parse_str(0);
}

/**
    @page edcref

    @property
        style
    @parameters
        [the style name]
    @effect
        Causes the part to use the default style and tags defined in the
        "style" block with the specified name.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_style(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.style.str = parse_str(0);
}

/**
    @page edcref

    @property
        repch
    @parameters
        [the replacement character string]
    @effect
        If this is a textblock and is in PASSWORD mode this string is used
        to replace every character to hide the details of the entry. Normally
        you would use a "*", but you can use anything you like.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_repch(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.repch.str = parse_str(0);
}

/**
    @page edcref

    @property
        size
    @parameters
        [font size in points (pt) 0 - 255]
    @effect
        Sets the default font size for the text part. Can be overridden by the
        application.

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_size(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.size = parse_int_range(0, 0, 255);
}

/**
    @page edcref

    @property
        size_range
    @parameters
        [font min size in points (pt) 0 - 255] [font max size in points (pt) 0 - 255]
    @effect
        Sets the allowed font size for the text part. Setting min and max to 0
        means we won't restrict the sizing.

        Defaults: 0 0
    @since 1.1
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_size_range(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(2);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.size_range_min = parse_int_range(0, 0, 255);
   ed->text.size_range_max = parse_int_range(1, 0, 255);
   if (ed->text.size_range_min > ed->text.size_range_max)
     {
        ERR("parse error %s:%i. min size is bigger than max size.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref

    @property
        fit
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is set to 1 edje will resize the text for it
        to fit in it's container. Both are disabled by default.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_fit(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(2);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.fit_x = parse_bool(0);
   ed->text.fit_y = parse_bool(1);
}

/**
    @page edcref

    @property
        min
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the minimum size of
        the container to be equal to the minimum size of the text.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_min(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(2);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.min_x = parse_bool(0);
   ed->text.min_y = parse_bool(1);

   if (current_part->type == EDJE_PART_TYPE_TEXTBLOCK)
     edje_file->has_textblock_min_max = EINA_TRUE;
}

/**
    @page edcref

    @property
        max
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the maximum size of
        the container to be equal to the maximum size of the text.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_max(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(2);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.max_x = parse_bool(0);
   ed->text.max_y = parse_bool(1);

   if (current_part->type == EDJE_PART_TYPE_TEXTBLOCK)
     edje_file->has_textblock_min_max = EINA_TRUE;
}

/**
    @page edcref

    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the alignment of the text [0.0(left) - 1.0(right)].
        You can set horizontal alignment to -1.0 to use bidirectional based alignment(
        0.0 for LTR content or 1.0 for RTL)

        Defaults: 0.5 0.5
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_align(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(2);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.align.x = FROM_DOUBLE(parse_float_range(0, -1.0, 1.0));
   ed->text.align.y = FROM_DOUBLE(parse_float_range(1, 0.0, 1.0));
}

/**
    @page edcref

    @property
        source
    @parameters
        [another TEXT part's name]
    @effect
        Causes the part to use the text properties (like font and size) of
        another part and update them as they change.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   {
      char *name;

      name = parse_str(0);
      data_queue_part_nest_lookup(pc, name, &(ed->text.id_source), &ed->text.id_source_part);
      free(name);
   }
}

/**
    @page edcref

    @property
        text_source
    @parameters
        [another TEXT part's name]
    @effect
        Causes the part to display the text content of another part and update
        them as they change.
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_text_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   {
      char *name;

      name = parse_str(0);
      data_queue_part_nest_lookup(pc, name, &(ed->text.id_text_source), &ed->text.id_text_source_part);
      free(name);
   }
}

/**
    @page edcref

    @property
        ellipsis
    @parameters
        [point of balance]
    @effect
        Used to balance the text in a relative point from 0.0 to 1.0, this
        point is the last section of the string to be cut out in case of a
        resize that is smaller than the text itself.
        Setting -1.0 will disable text cutting.

        Defaults: 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_text_ellipsis(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text *)current_desc;

   ed->text.ellipsis = parse_float_range(0, -1.0, 1.0);
}

/** @edcsubsection{collections_group_parts_description_box,
 *                 Group.Parts.Part.Description.Box} */

/**
    @page edcref

    @block
        box
    @context
        part {
            description {
                ..
                box {
                    layout: "vertical";
                    padding: 0 2;
                    align: 0.5 0.5;
                    min: 0 0;
                }
                ..
            }
        }
    @description
        A box block can contain other objects and display them in different
        layouts, any of the predefined set, or a custom one, set by the
        application.
    @endblock

    @property
        layout
    @parameters
        [primary layout] (fallback layout)
    @effect
        Sets the layout for the box:
            @li horizontal
            @li vertical
            @li horizontal_homogeneous
            @li vertical_homogeneous
            @li horizontal_max (homogeneous to the max sized child)
            @li vertical_max
            @li horizontal_flow
            @li vertical_flow
            @li stack
            @li some_other_custom_layout_set_by_the_application
        You could set a custom layout as fallback, it makes very
        very little sense though, and if that one fails, it will
        default to horizontal.

        Defaults: "horizontal"
    @endproperty
 */
static void
st_collections_group_parts_part_description_box_layout(void)
{
   Edje_Part_Description_Box *ed;

   check_min_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box *)current_desc;

   ed->box.layout = parse_str(0);
   if (is_param(1))
     ed->box.alt_layout = parse_str(1);
}

/**
    @page edcref
    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the position of the point of balance inside the box [-1.0 - 1.0].

        Defaults: 0.5 0.5
    @endproperty
 */
static void
st_collections_group_parts_part_description_box_align(void)
{
   Edje_Part_Description_Box *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box *)current_desc;

   ed->box.align.x = FROM_DOUBLE(parse_float_range(0, -1.0, 1.0));
   ed->box.align.y = FROM_DOUBLE(parse_float_range(1, -1.0, 1.0));
}

/**
    @page edcref
    @property
        padding
    @parameters
        [horizontal] [vertical]
    @effect
        Sets the space between box items in pixels.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_box_padding(void)
{
   Edje_Part_Description_Box *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box *)current_desc;

   ed->box.padding.x = parse_int_range(0, 0, 0x7fffffff);
   ed->box.padding.y = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        min
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the minimum size of
        the box to be equal to the minimum size of the items.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_box_min(void)
{
   Edje_Part_Description_Box *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box *)current_desc;

   ed->box.min.h = parse_bool(0);
   ed->box.min.v = parse_bool(1);
}

/** @edcsubsection{collections_group_parts_description_table,
 *                 Group.Parts.Part.Description.Table} */

/**
    @page edcref

    @block
        table
    @context
        part {
            description {
                ..
                table {
                    homogeneous: TABLE;
                    padding: 0 2;
                    align: 0.5 0.5;
                    min: 0 0;
                }
                ..
            }
        }
    @description
        A table block can contain other objects packed in multiple columns
        and rows, and each item can span across more than one column and/or
        row.
    @endblock

    @property
        homogeneous
    @parameters
        [homogeneous mode]
    @effect
        Sets the homogeneous mode for the table:
            @li NONE
            @li TABLE
            @li ITEM

        Defaults: NONE
    @endproperty
 */
static void
st_collections_group_parts_part_description_table_homogeneous(void)
{
   Edje_Part_Description_Table *ed;

   check_min_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table *)current_desc;

   ed->table.homogeneous = parse_enum(0,
                                      "NONE", EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE,
                                      "TABLE", EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE,
                                      "ITEM", EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM,
                                      NULL);
}

/**
    @page edcref
    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the position of the point of balance inside the table [-1.0 - 1.0].

        Defaults: 0.5 0.5
    @endproperty
 */
static void
st_collections_group_parts_part_description_table_align(void)
{
   Edje_Part_Description_Table *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table *)current_desc;

   ed->table.align.x = FROM_DOUBLE(parse_float_range(0, -1.0, 1.0));
   ed->table.align.y = FROM_DOUBLE(parse_float_range(1, -1.0, 1.0));
}

/**
    @page edcref
    @property
        padding
    @parameters
        [horizontal] [vertical]
    @effect
        Sets the space between table cells in pixels.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_table_padding(void)
{
   Edje_Part_Description_Table *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table *)current_desc;

   ed->table.padding.x = parse_int_range(0, 0, 0x7fffffff);
   ed->table.padding.y = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        min
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the minimum size of
        the table to be equal to the minimum size of the items.

        Defaults: 0 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_table_min(void)
{
   Edje_Part_Description_Table *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table *)current_desc;

   ed->table.min.h = parse_bool(0);
   ed->table.min.v = parse_bool(1);
}

/**
   @edcsubsection{collections_group_parts_description_proxy,
                  Group.Parts.Part.Description.Proxy}
 */

/**
    @page edcref

    @block
        proxy
    @context
        part { type: PROXY;
            description {
                ..
                proxy {
                    source_clip:    1;
                    source_visible: 1;
                }
                ..
            }
        }
    @description
        State flags used for proxy objects.
    @endblock

    @property
        source_clip
    @parameters
        [0 or 1]
    @effect
        Sets the 'source_clip' property on this PROXY object. True by default,
        this means the proxy will be clipped by its source clipper. False
        means the source clipper is ignored when rendering the proxy.

        Defaults: 1
    @endproperty

    @property
        source_visible
    @parameters
        [0 or 1]
    @effect
        Sets the 'source_visible' property on this PROXY object. True by
        default, meaning both the proxy and its source object will be visible.
        If false, the source object will not be visible. False is equivalent
        to setting the 'no_render' flag on the source object itself.

        Defaults: 1
    @endproperty
 */
static void
st_collections_group_parts_part_description_proxy_source_clip(void)
{
   Edje_Part_Description_Proxy *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_PROXY)
     {
        ERR("parse error %s:%i. proxy attributes in non-PROXY part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Proxy *)current_desc;
   ed->proxy.source_clip = parse_bool(0);
}

static void
st_collections_group_parts_part_description_proxy_source_visible(void)
{
   Edje_Part_Description_Proxy *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_PROXY)
     {
        ERR("parse error %s:%i. proxy attributes in non-PROXY part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Proxy *)current_desc;
   ed->proxy.source_visible = parse_bool(0);
}

/**
   @edcsubsection{collections_group_parts_description_position,
                  Group.Parts.Part.Description.Position}
 */

/**
    @page edcref

    @block
        position
    @context
        part {
            description {
                ..
                position {
                    point:        x y z;
                    space:        LOCAL/PARENT/WORLD;
                }
                ..
            }
        }
    @description
        A position block defines position of CAMERA,
        LIGHT or MESH_NODE at the scene.
    @endblock

    @property
        point
    @parameters
        [x] [y] [z]
    @effect
        Sets the point of CAMERA, LIGHT or MESH_NODE centre.
    @endproperty
 */
static void
st_collections_group_parts_part_description_position_point(void)
{
   check_arg_count(3);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_CAMERA:
      {
         Edje_Part_Description_Camera *ed;

         ed = (Edje_Part_Description_Camera *)current_desc;

         ed->camera.position.point.x = FROM_DOUBLE(parse_float(0));
         ed->camera.position.point.y = FROM_DOUBLE(parse_float(1));
         ed->camera.position.point.z = FROM_DOUBLE(parse_float(2));
         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *ed;

         ed = (Edje_Part_Description_Light *)current_desc;

         ed->light.position.point.x = FROM_DOUBLE(parse_float(0));
         ed->light.position.point.y = FROM_DOUBLE(parse_float(1));
         ed->light.position.point.z = FROM_DOUBLE(parse_float(2));
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *ed;

         ed = (Edje_Part_Description_Mesh_Node *)current_desc;

         ed->mesh_node.position.point.x = FROM_DOUBLE(parse_float(0));
         ed->mesh_node.position.point.y = FROM_DOUBLE(parse_float(1));
         ed->mesh_node.position.point.z = FROM_DOUBLE(parse_float(2));
         break;
      }

      default:
      {
         ERR("parse error %s:%i. camera and light attributes in non-CAMERA, non-LIGHT, and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref

    @property
        space
    @parameters
        [SPACE]
    @effect
        Explains in which relative coordinates the location of LIGHT
        or CAMERA considers. Valid space types:
            @li LOCAL
            @li PARENT
            @li WORLD
    @endproperty
 */
static void
st_collections_group_parts_part_description_position_space(void)
{
   unsigned int space;

   check_arg_count(1);

   space = parse_enum(0,
                      "LOCAL", EVAS_CANVAS3D_SPACE_LOCAL,
                      "PARENT", EVAS_CANVAS3D_SPACE_PARENT,
                      "WORLD", EVAS_CANVAS3D_SPACE_WORLD,
                      NULL);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_CAMERA:
      {
         Edje_Part_Description_Camera *ed;

         ed = (Edje_Part_Description_Camera *)current_desc;

         ed->camera.position.space = space;
         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *ed;

         ed = (Edje_Part_Description_Light *)current_desc;

         ed->light.position.space = space;
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *ed;

         ed = (Edje_Part_Description_Mesh_Node *)current_desc;

         ed->mesh_node.position.space = space;
         break;
      }

      default:
      {
         ERR("parse error %s:%i. camera and light attributes in non-CAMERA, non-LIGHT, and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
   @edcsubsection{collections_group_parts_description_camera,
                  Group.Parts.Part.Description.Properties}
 */

/**
    @page edcref

    @block
        properties
    @context
        part { type: CAMERA;
            description {
                ..
                properties {
                   perspective: fovy aspect near far;
                }
                ..
            }
        }
    @description
    @endblock

    @property
        properties
    @parameters
        [fovy] [aspect] [near] [far]
    @effect
        Specifies the basic attributes of the camera.
    @endproperty
 */
static void
st_collections_group_parts_part_description_camera_properties(void)
{
   check_arg_count(4);

   if (current_part->type == EDJE_PART_TYPE_CAMERA)
     {
        Edje_Part_Description_Camera *ed;

        ed = (Edje_Part_Description_Camera *)current_desc;

        ed->camera.camera.fovy = FROM_DOUBLE(parse_float(0));
        ed->camera.camera.aspect = FROM_DOUBLE(parse_float(1));
        ed->camera.camera.frustum_near = FROM_DOUBLE(parse_float(2));
        ed->camera.camera.frustum_far = FROM_DOUBLE(parse_float(3));
     }
   else if (current_part->type == EDJE_PART_TYPE_LIGHT)
     {
        Edje_Part_Description_Light *ed;

        ed = (Edje_Part_Description_Light *)current_desc;

        ed->light.light.fovy = FROM_DOUBLE(parse_float(0));
        ed->light.light.aspect = FROM_DOUBLE(parse_float(1));
        ed->light.light.frustum_near = FROM_DOUBLE(parse_float(2));
        ed->light.light.frustum_far = FROM_DOUBLE(parse_float(3));
     }
   else
     {
        ERR("parse error %s:%i. camera attributes in non-CAMERA and non-LIGHT part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
   @edcsubsection{collections_group_parts_description_properties,
                  Group.Parts.Part.Description.Properties}
 */

/**
    @page edcref

    @block
        properties
    @context
        part { type: [LIGHT or MESH_NODE];
            description {
                ..
                properties {
                   ambient:     red green blue alpha;
                   diffuse:     red green blue alpha;
                   specular:    red green blue alpha;
                   shade:       PHONG;
                   material:    AMBIENT;
                   normal:      1;
                   shininess:   50.0;
                }
                ..
            }
        }
    @description
        A properties block defines main lighting attributes of LIGHT and MESH_NODE.
    @endblock

    @property
        ambient
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Sets the components of the ambient color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 50 50 50 255
    @endproperty
 */
static void
st_collections_group_parts_part_description_properties_ambient(void)
{
   check_arg_count(4);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *ed;

         ed = (Edje_Part_Description_Light *)current_desc;

         parse_color(0, &(ed->light.properties.ambient));
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *ed;

         ed = (Edje_Part_Description_Mesh_Node *)current_desc;

         parse_color(0, &(ed->mesh_node.properties.ambient));
         break;
      }

      default:
      {
         ERR("parse error %s:%i. light and mesh_node attributes in non-LIGHT and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref
    @property
        diffuse
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Sets the components of the diffuse color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 255 255 255 255
    @endproperty
 */
static void
st_collections_group_parts_part_description_properties_diffuse(void)
{
   check_arg_count(4);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *ed;

         ed = (Edje_Part_Description_Light *)current_desc;

         parse_color(0, &(ed->light.properties.diffuse));
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *ed;

         ed = (Edje_Part_Description_Mesh_Node *)current_desc;

         parse_color(0, &(ed->mesh_node.properties.diffuse));
         break;
      }

      default:
      {
         ERR("parse error %s:%i. light and mesh_node attributes in non-LIGHT and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref
    @property
        specular
    @parameters
        [red] [green] [blue] [alpha] or "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Sets the components of the specular color.

        Format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 255 255 255 255
    @endproperty
 */
static void
st_collections_group_parts_part_description_properties_specular(void)
{
   check_arg_count(4);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *ed;

         ed = (Edje_Part_Description_Light *)current_desc;

         parse_color(0, &(ed->light.properties.specular));
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *ed;

         ed = (Edje_Part_Description_Mesh_Node *)current_desc;

         parse_color(0, &(ed->mesh_node.properties.specular));
         break;
      }

      default:
      {
         ERR("parse error %s:%i. light and mesh_node attributes in non-LIGHT and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref
    @property
        material
    @parameters
        [MATERIAL]
    @effect
        Sets the color mode for the lighting. Valid color modes:
            @li AMBIENT
            @li DIFFUSE
            @li SPECULAR
            @li EMISSION
            @li NORMAL
    @endproperty
 */
static void
st_collections_group_parts_part_description_properties_material(void)
{
   unsigned int material_attrib;

   check_arg_count(1);

   material_attrib = parse_enum(0,
                                "AMBIENT", EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT,
                                "DIFFUSE", EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE,
                                "SPECULAR", EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR,
                                "EMISSION", EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION,
                                "NORMAL", EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL,
                                NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.properties.material_attrib = material_attrib;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        normal
    @parameters
        [0 or 1]
    @effect
        Sets the material attribute enable flag of the given material.
    @endproperty
 */
static void
st_collections_group_parts_part_description_properties_normal(void)
{
   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.properties.normal = parse_bool(0);
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        shininess
    @parameters
        [shininess]
    @effect
        Sets the shininess of the given material.
    @endproperty
 */
static void
st_collections_group_parts_part_description_properties_shininess(void)
{
   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.properties.shininess = FROM_DOUBLE(parse_float(0));
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        shade
    @parameters
        [SHADE]
    @effect
        Sets the shade mode for MESH_NODE. Valid shade modes:
            @li COLOR
            @li DIFFUSE
            @li FLAT
            @li PHONG
            @li MAP
            @li RENDER
    @endproperty
 */

static void
st_collections_group_parts_part_description_properties_shade(void)
{
   unsigned int shade;

   check_arg_count(1);

   shade = parse_enum(0,
                      "VERTEX_COLOR", EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR,
                      "PARENT", EVAS_CANVAS3D_SHADER_MODE_DIFFUSE,
                      "WORLD", EVAS_CANVAS3D_SHADER_MODE_FLAT,
                      "PHONG", EVAS_CANVAS3D_SHADER_MODE_PHONG,
                      "NORMAL_MAP", EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP,
                      "RENDER", EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER,
                      NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.properties.shade = shade;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
   @edcsubsection{collections_group_parts_description_orientation,
                  Group.Parts.Part.Description.Orientation}
 */

/**
    @page edcref

    @block
        orientation
    @context
        part { type: [CAMERA or MESH_NODE or LIGHT];
            description {
                ..
                orientation {
                   look1:        [x] [y] [z];
                   look2:        [x] [y] [z];
                   look_to:      [another part's name];
                   angle_axis:   [w] [x] [y] [z];
                   quaternion:   [x] [y] [z] [w];
                }
                ..
            }
        }
    @description
        The orientation block defines an orientation of CAMERA, LIGHT or MESH_NODE in the scene.
    @endblock

    @property
        look1
    @parameters
        [x] [y] [z]
    @effect
        Indicates a target point for CAMERA and MESH_NODE or for LIGHT to see or
        to illuminate.
    @endproperty
 */
static void
st_collections_group_parts_part_description_orientation_look1(void)
{
   check_arg_count(3);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_CAMERA:
      {
         SET_LOOK1(Camera, camera);
         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         SET_LOOK1(Light, light);
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         SET_LOOK1(Mesh_Node, mesh_node);
         break;
      }

      default:
      {
         ERR("parse error %s:%i. camera, light and mesh_node  attributes in non-CAMERA, non-LIGHT and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref
    @property
        look2
    @parameters
        [x] [y] [z]
    @effect
        Specifies the angle at which the target point will be caught.
    @endproperty
 */
static void
st_collections_group_parts_part_description_orientation_look2(void)
{
   check_arg_count(3);

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_CAMERA:
      {
         SET_LOOK2(Camera, camera);
         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         SET_LOOK2(Light, light);
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         SET_LOOK2(Mesh_Node, mesh_node);
         break;
      }

      default:
      {
         ERR("parse error %s:%i. camera, light and mesh_node  attributes in non-CAMERA, non-LIGHT and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref
    @property
        look_to
    @parameters
        [another part's name]
    @effect
        Indicates another part to make target of CAMERA, LIGHT or MESH_NODE
        or LIGHT.
    @endproperty
 */
static void
st_collections_group_parts_part_description_orientation_look_to(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   switch (current_part->type)
     {
      case EDJE_PART_TYPE_CAMERA:
      {
         SET_LOOK_TO(pc, Camera, camera);
         break;
      }

      case EDJE_PART_TYPE_LIGHT:
      {
         SET_LOOK_TO(pc, Light, light);
         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         SET_LOOK_TO(pc, Mesh_Node, mesh_node);
         break;
      }

      default:
      {
         ERR("parse error %s:%i. camera, light and mesh_node  attributes in non-CAMERA, non-LIGHT and non-MESH_NODE part.",
             file_in, line - 1);
         exit(-1);
      }
     }
}

/**
    @page edcref
    @property
        angle_axis
    @parameters
        [x] [y] [z] [w]
    @effect
        Specifies the angle and indicates what proportions the MESH_NODE rotates in.
    @endproperty
 */
static void
st_collections_group_parts_part_description_orientation_angle_axis(void)
{
   check_arg_count(4);

   if (current_part->type == EDJE_PART_TYPE_CAMERA)
     {
        SET_ANGLE_AXIS(Camera, camera)
     }
   else if (current_part->type == EDJE_PART_TYPE_LIGHT)
     {
        SET_ANGLE_AXIS(Light, light)
     }
   else if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        SET_ANGLE_AXIS(Mesh_Node, mesh_node)
     }
   else
     {
        ERR("parse error %s:%i. camera, light and mesh_node  attributes in non-CAMERA, non-LIGHT and non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        quaternion
    @parameters
        [x] [y] [z] [w]
    @effect
        Specifies the axis and arccosinus of half angle to rotate on the MESH_NODE, CAMERA or LIGHT.
    @endproperty
 */
static void
st_collections_group_parts_part_description_orientation_quaternion(void)
{
   check_arg_count(4);

   if (current_part->type == EDJE_PART_TYPE_CAMERA)
     {
        SET_QUATERNION(Camera, camera)
     }
   else if (current_part->type == EDJE_PART_TYPE_LIGHT)
     {
        SET_QUATERNION(Light, light)
     }
   else if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        SET_QUATERNION(Mesh_Node, mesh_node)
     }
   else
     {
        ERR("parse error %s:%i. camera, light and mesh_node  attributes in non-CAMERA, non-LIGHT and non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        scale
    @parameters
        [scale_x] [scale_y] [scale_z]
    @effect
        Specifies the scale parameter for MESH_NODE.
    @endproperty
 */
static void
st_collections_group_parts_part_description_scale(void)
{
   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.scale_3d.x = FROM_DOUBLE(parse_float_range(0, 0.0, 1000.0));
        ed->mesh_node.scale_3d.y = FROM_DOUBLE(parse_float_range(1, 0.0, 1000.0));
        ed->mesh_node.scale_3d.z = FROM_DOUBLE(parse_float_range(2, 0.0, 1000.0));
     }
   else
     {
        ERR("parse error %s:%i. mesh_node  attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
   @edcsubsection{collections_group_parts_description_texture,
                  Group.Parts.Part.Description.Texture}
 */

/**
    @page edcref

    @block
        texture
    @context
        part {
            description {
                ..
                texture {
                    image:        "file_name";
                    wrap1:        REPEAT;
                    wrap2:        REPEAT;
                    filter1:      NEAREST;
                    filter2:      NEAREST;
                }
                ..
            }
        }
    @description
        A texture block is used to set texture, this texture will be imposed on
        MESH_NODE model.
    @endblock
 */

static void
ob_collections_group_parts_part_description_texture(void)
{
   Edje_Part_Description_Mesh_Node *ed;

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.texture.need_texture = EINA_TRUE;
     }
   else
     {
        ERR("parse error %s:%i. "
            "mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        shade
        image
    @parameters
        [SHADE]
        [texture's filename]
    @effect
        Sets the shade mode for MESH_NODE. Valid shade modes:
           @li COLOR
           @li DIFFUSE
           @li FLAT
           @li PHONG
           @li MAP
           @li RENDER
        Name of image to be used as previously declared in the image block.
        It's required in any mesh_node part.
    @endproperty
 */
static void
st_collections_group_parts_part_description_texture_image(void)
{
   Edje_Part_Description_Mesh_Node *ed;

   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        char *name;
        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.texture.textured = EINA_TRUE;

        name = parse_str(0);
        data_queue_image_remove(&(ed->mesh_node.texture.id), &(ed->mesh_node.texture.set));
        data_queue_image_lookup(name, &(ed->mesh_node.texture.id), &(ed->mesh_node.texture.set));
        free(name);
     }
   else
     {
        ERR("parse error %s:%i. "
            "mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        wrap1
    @parameters
        [WRAP]
    @effect
        Sets the wrap mode for S-axis. Valid wrap modes:
            @li CLAMP
            @li REPEAT
            @li REFLECT
    @endproperty
 */
static void
st_collections_group_parts_part_description_texture_wrap1(void)
{
   unsigned int wrap1;

   check_arg_count(1);

   wrap1 = parse_enum(0,
                      "CLAMP", EVAS_CANVAS3D_WRAP_MODE_CLAMP,
                      "REPEAT", EVAS_CANVAS3D_WRAP_MODE_REPEAT,
                      "REFLECT", EVAS_CANVAS3D_WRAP_MODE_REFLECT,
                      NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.texture.wrap1 = wrap1;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        wrap2
    @parameters
        [SHADE]
    @effect
        Sets the wrap mode for T-axis. Valid wrap modes:
            @li CLAMP
            @li REPEAT
            @li REFLECT
    @endproperty
 */
static void
st_collections_group_parts_part_description_texture_wrap2(void)
{
   unsigned int wrap2;

   check_arg_count(1);

   wrap2 = parse_enum(0,
                      "CLAMP", EVAS_CANVAS3D_WRAP_MODE_CLAMP,
                      "REPEAT", EVAS_CANVAS3D_WRAP_MODE_REPEAT,
                      "REFLECT", EVAS_CANVAS3D_WRAP_MODE_REFLECT,
                      NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.texture.wrap2 = wrap2;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        filter1
    @parameters
        [FILTER]
    @effect
        Sets the minification filter used when down-scrolling. Valid filter types:
            @li NEAREST
            @li LINEAR
            @li NEAREST_MIPMAP_NEAREST
            @li LINEAR_MIPMAP_NEAREST
            @li NEAREST_MIPMAP_LINEAR
    @endproperty
 */
static void
st_collections_group_parts_part_description_texture_filter1(void)
{
   unsigned int filter1;

   check_arg_count(1);

   filter1 = parse_enum(0,
                        "NEAREST", EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST,
                        "LINEAR", EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR,
                        "NEAREST_NEAREST", EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
                        "LINEAR_NEAREST", EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
                        "NEAREST_LINEAR", EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
                        "LINEAR_LINEAR", EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
                        NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.texture.filter1 = filter1;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        filter2
    @parameters
        [FILTER]
    @effect
        Sets the magnification filter used when down-scrolling. Valid filter types:
            @li NEAREST
            @li LINEAR
            @li NEAREST_MIPMAP_NEAREST
            @li LINEAR_MIPMAP_NEAREST
            @li NEAREST_MIPMAP_LINEAR
    @endproperty
 */
static void
st_collections_group_parts_part_description_texture_filter2(void)
{
   unsigned int filter2;

   check_arg_count(1);

   filter2 = parse_enum(0,
                        "NEAREST", EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST,
                        "LINEAR", EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR,
                        "NEAREST_NEAREST", EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
                        "LINEAR_NEAREST", EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
                        "NEAREST_LINEAR", EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
                        "LINEAR_LINEAR", EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
                        NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.texture.filter2 = filter2;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
   @edcsubsection{collections_group_parts_description_mesh,Mesh}
 */

/**
    @page edcref

    @block
        mesh
    @context
        part {
            description {
                ..
                mesh {
                    geometry:        "file_name";
                    primitive:        CUBE;
                    assembly:         LINEAR;
                }
                ..
            }
        }
    @description
    @endblock

    @property
        primitive
    @parameters
        [PRIMITIVE]
    @effect
        Specifies the type of primitive model to be used.
        Valid primitives:
            @li NONE
            @li CUBE
            @li SPHERE
    @endproperty
 */
static void
st_collections_group_parts_part_description_mesh_primitive(void)
{
   unsigned int primitive;

   check_arg_count(1);

   primitive = parse_enum(0,
                          "NONE", EVAS_CANVAS3D_MESH_PRIMITIVE_NONE,
                          "CUBE", EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE,
                          "SPHERE", EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE,
                          NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.mesh.primitive = primitive;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        assembly
    @parameters
        [ASSEMBLY]
    @effect
        Sets the vertex assembly of the mesh. Valid assemblies:
            @li POINTS
            @li LINES
            @li LINE_STRIP
            @li LINE_LOOP
            @li TRIANGLES
            @li TRIANGLE_STRIP
            @li TRIANGLE_FAN
    @endproperty
 */
static void
st_collections_group_parts_part_description_mesh_assembly(void)
{
   unsigned int assembly;

   check_arg_count(1);

   assembly = parse_enum(0,
                         "POINTS", EVAS_CANVAS3D_VERTEX_ASSEMBLY_POINTS,
                         "LINES", EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES,
                         "LINE_STRIP", EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINE_STRIP,
                         "LINE_LOOP", EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINE_LOOP,
                         "TRIANGLES", EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES,
                         "TRIANGLE_STRIP", EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP,
                         "TRIANGLE_FAN", EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_FAN,
                         NULL);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.mesh.assembly = assembly;
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        geometry
    @parameters
        [model's filename]
    @effect
        Name of model to be used as previously declared in the model block.
        It's required in any mesh_node part.
    @endproperty
 */
static void
st_collections_group_parts_part_description_mesh_geometry(void)
{
   Edje_Part_Description_Mesh_Node *ed;

   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        char *name;
        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        name = parse_str(0);
        if (!ecore_file_exists(name))
          {
             ERR("Unable to load model \"%s\". Check if path to file is correct (both directory and file name).",
                 name);
             exit(-1);
          }
        data_queue_model_remove(&(ed->mesh_node.mesh.id), &(ed->mesh_node.mesh.set));
        data_queue_model_lookup(name, &(ed->mesh_node.mesh.id), &(ed->mesh_node.mesh.set));
        free(name);
     }
   else
     {
        ERR("parse error %s:%i. "
            "image attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

static void
st_collections_group_parts_part_description_mesh_frame(void)
{
   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_MESH_NODE)
     {
        Edje_Part_Description_Mesh_Node *ed;

        ed = (Edje_Part_Description_Mesh_Node *)current_desc;

        ed->mesh_node.mesh.frame = parse_int(0);
     }
   else
     {
        ERR("parse error %s:%i. mesh_node attributes in non-MESH_NODE part.",
            file_in, line - 1);
        exit(-1);
     }
}

/** @edcsubsection{collections_group_parts_description_physics,
 *                 Group.Parts.Part.Description.Physics} */

/**
    @page edcref
    @block
        physics
    @context
    description {
        ..
        physics {
            ignore_part_pos: 1;
            mass: 5.31;
            friction: 0.5;
            restitution: 0.82;
            damping: 0.4 0.24;
            sleep: 32 18.9;
            material: IRON;
            density: 3.2;
            hardness: 0.42;
            light_on: 1;
            z: -15;
            depth: 30;
            movement_freedom { }
            faces { }
        }
        ..
    }

    @description
        Physics block should be used to configure the body associated to the
        part. The part's property physics_body needs to be set to something
        different from NONE, otherwise the properties inside physics block
        won't have any effect.
        It's possible to set body's material, mass, restitution, friction,
        allow / disallow movement in specific axes, etc.
    @endblock

    @property
        mass
    @parameters
        [body's mass in kilograms]
    @effect
        Double value used to set inertial mass of the body.
        It is a quantitative measure of an object's resistance to the change of
        its speed. If mass is set to 0 the body will have infinite mass,
        so it will be immovable, static.
    @since 1.8
    @endproperty
 */

#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_mass(void)
{
   check_arg_count(1);

   current_desc->physics.mass = parse_float(0);
}

#endif

/**
    @page edcref
    @property
        restitution
    @parameters
        [body's restitution]
    @effect
        The coefficient of restitution is proporcion between speed after and
        before a collision. It's 0 by default.

        COR = relative speed after collision / relative speed before collision

        @li elastically collide for COR == 1;
        @li inelastically collide for 0 < COR < 1;
        @li completelly stop (no bouncing at all) for COR == 0.

    @since 1.8
    @endproperty
 */

#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_restitution(void)
{
   check_arg_count(1);

   current_desc->physics.restitution = parse_float(0);
}

#endif

/**
    @page edcref
    @property
        friction
    @parameters
        [body's friction]
    @effect
        Friction is used to make objects slide along each ot

        The friction parameter is usually set between 0 and 1, but can be any
        non-negative value. A friction value of 0 turns off friction and a value
        of 1 makes the friction strong.

        By default friction value is 0.5 and simulation resulsts will be better
        when friction in non-zero.

    @since 1.8
    @endproperty
 */

#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_friction(void)
{
   check_arg_count(1);

   current_desc->physics.friction = parse_float(0);
}

#endif

/**
    @page edcref
    @property
        ignore_part_pos
    @parameters
        [1 or 0]
    @effect
        If enabled, the body won't be positioned following rel1/rel2.
        It will keep its position updated only by physics calculations.
        If disabled, when the state is set, the body will be moved to
        the position described by the blocks rel1/rel2.
        Default is 1 (enabled).
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_ignore_part_pos(void)
{
   check_arg_count(1);

   current_desc->physics.ignore_part_pos = parse_bool(0);
}

#endif

/**
    @page edcref
    @property
        damping
    @parameters
        [linear damping] [angular damping]
    @effect
        Damping(linear and angular) values are applied to body's linear and
        angular velocity.
        By applying a bodies damping factor the user will face a velocity
        reduction, with a force applied to it - "like" air resistance.
        The force is applied to slow it down.
        Values should be between 0.0 and 1.0, and are set to 0 by default.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_damping(void)
{
   check_arg_count(2);

   current_desc->physics.damping.linear = parse_float_range(0, 0, 1.0);
   current_desc->physics.damping.angular = parse_float_range(1, 0, 1.0);
}

#endif

/**
    @page edcref
    @property
        sleep
    @parameters
        [linear sleeping threshold] [angular sleeping threshold]
    @effect
        Sleeping threshold factors are used to determine whenever a rigid body
        is supposed to increment the sleeping time. Linear threshold is
        measured in Evas coordinates per second and angular threshold is
        measured in degrees per second.
        After every tick the sleeping time is incremented, if the body's
        linear and angular speed is less than the respective thresholds
        the sleeping time is incremented by the current time step (delta time).
        Reaching the max sleeping time the body is marked to sleep, that means
        the rigid body is to be deactivated.
        By default linear threshold is 24 pixels / second and angular is
        57.29 degrees / sec (1 rad/sec).
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_sleep(void)
{
   check_arg_count(2);

   current_desc->physics.sleep.linear = parse_float(0);
   current_desc->physics.sleep.angular = parse_float(1);
}

#endif

/**
    @page edcref
    @property
        material
    @parameters
        [body's material]
    @effect
        Set the type (all caps) from among the available material types,
        it's set to CUSTOM by default.
        Each material has specific properties to be
        applied on the body, as density, friction and restitution.
        So if a material different of CUSTOM is set, the properties cited above
        won't be considered.
        Valid types:
          @li CUSTOM
          @li CONCRETE
          @li IRON
          @li PLASTIC
          @li POLYSTYRENE
          @li RUBBER
          @li WOOD

    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_material(void)
{
   check_arg_count(1);

   current_desc->physics.material = parse_enum(0,
                                               "CUSTOM", EPHYSICS_BODY_MATERIAL_CUSTOM,
                                               "CONCRETE", EPHYSICS_BODY_MATERIAL_CONCRETE,
                                               "IRON", EPHYSICS_BODY_MATERIAL_IRON,
                                               "PLASTIC", EPHYSICS_BODY_MATERIAL_PLASTIC,
                                               "POLYSTYRENE", EPHYSICS_BODY_MATERIAL_POLYSTYRENE,
                                               "RUBBER", EPHYSICS_BODY_MATERIAL_RUBBER,
                                               "WOOD", EPHYSICS_BODY_MATERIAL_WOOD,
                                               NULL);
}

#endif

/**
    @page edcref
    @property
        density
    @parameters
        [body's material density]
    @effect
        It will set the body mass considering its volume. While a density is
        set, resizing a body will always recalculate its mass.
        When a mass is explicitely set the density will be unset.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_density(void)
{
   check_arg_count(1);

   current_desc->physics.density = parse_float(0);
}

#endif

/**
    @page edcref
    @property
        hardness
    @parameters
        [soft body or cloth hardness]
    @effect
        The hardness is set with a double value (0.0 - 1.0), defining
        how the soft body is supposed to deform.
        Its default is set to 1.0. The soft body mass will also interfere on
        soft body deformation, so bare in mind that the bodies mass must also
        be changed to have different deformation results.
        Valid values vary from 0.0 to 1.0. Only works on soft bodies and cloths.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_hardness(void)
{
   check_arg_count(1);

   current_desc->physics.hardness = parse_float_range(0, 0, 1.0);
}

#endif

/**
    @page edcref
    @property
        light_on
    @parameters
        [1 or 0]
    @effect
        Set body to be affected by world's light or not.
        It won't be respected if world's property "all_bodies" is enabled.
        Disabled by default (0).
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_light_on(void)
{
   check_arg_count(1);

   current_desc->physics.light_on = parse_bool(0);
}

#endif

/**
    @page edcref
    @property
        z
    @parameters
        [body position in z axis]
    @effect
        Defines body position in z axis. It's set to -15 by default.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_z(void)
{
   check_arg_count(1);

   current_desc->physics.z = parse_int(0);
}

#endif

/**
    @page edcref
    @property
        depth
    @parameters
        [body's depth]
    @effect
        Defines body's depth (z axis). It's set to 30 by default.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_depth(void)
{
   check_arg_count(1);

   current_desc->physics.depth = parse_int(0);
}

#endif

/**
    @page edcref
    @property
        backface_cull
    @parameters
        [1 or 0]
    @effect
        This enables backface culling (when the rotated part that normally faces
        the camera is facing away after being rotated etc.).
        This means that the object will be hidden when "backface culled".
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_backface_cull(void)
{
   check_arg_count(1);

   current_desc->physics.backcull = parse_bool(0);
}

#endif

/** @edcsubsection{collections_group_parts_description_physics_movement_freedom,
 *                 Group.Parts.Part.Description.Physics.Movement_Freedom} */

/**
    @page edcref
    @block
        movement_freedom
    @context
        physics {
            ...
            movement_freedom {
                linear: 1 1 0;
                angular: 0 0 1;
            }
        }
        ..
    @description
        The "movement_freedom" block consists of two blocks to describe all
        the allowed movements for a body.
        It's set by default to allow just 2D movement (linear moves on
        x and y axis and rotations on x-y plane).
    @endblock

    @property
        linear
    @parameters
        [x-axis (1 or 0)] [y-axis (1 or 0)] [z-axis (1 or 0)]
    @effect
        Block "linear" can be used to allow linear movements in the three
        axes. Allowed values are 0 or 1.
        Axes x and y are enabled by default.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_movement_freedom_linear(void)
{
   check_arg_count(3);

   current_desc->physics.mov_freedom.lin.x = parse_bool(0);
   current_desc->physics.mov_freedom.lin.y = parse_bool(1);
   current_desc->physics.mov_freedom.lin.z = parse_bool(2);
}

#endif

/**
    @page edcref
    @property
        angular
    @parameters
        [x-axis (1 or 0)] [y-axis (1 or 0)] [z-axis (1 or 0)]
    @effect
        Block "angular" can be used to allow angular movements around the three
        axes. Allowed values are 0 or 1.
        Z axis is enabled by default.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_movement_freedom_angular(void)
{
   check_arg_count(3);

   current_desc->physics.mov_freedom.ang.x = parse_bool(0);
   current_desc->physics.mov_freedom.ang.y = parse_bool(1);
   current_desc->physics.mov_freedom.ang.z = parse_bool(2);
}

#endif

/** @edcsubsection{collections_group_parts_description_physics_faces,
 *                 Group.Parts.Part.Description.Physics.Faces} */

/**
    @page edcref
    @block
        faces
    @context
        physics {
            ...
            faces {
                face {
                    type: BOX_FRONT;
                    source:  "groupname";
                }
                ..
            }
            ..
        }
    @description
        The "faces" block contains a list of one or more "face" blocks.
        The "faces" block is used to list the faces that compose the body.
        Each face is described by a "face" block, that associates a part
        to a specific face of the body's shape.
        Only the "faces" block declared in the "default" description
        will be considered.
    @endblock
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_face(void)
{
   Edje_Physics_Face *pface;

   pface = mem_alloc(SZ(Edje_Physics_Face));
   current_desc->physics.faces = eina_list_append(current_desc->physics.faces,
                                                  pface);
   pface->type = 0;
   pface->source = NULL;
}

#endif

/**
    @page edcref
    @property
        type
    @parameters
        [FACE]
    @effect
        Set the face (all caps) from among the available body's shape faces.
        Valid faces:
            @li BOX_MIDDLE_FRONT
            @li BOX_MIDDLE_BACK
            @li BOX_FRONT
            @li BOX_BACK
            @li BOX_LEFT
            @li BOX_RIGHT
            @li BOX_TOP
            @li BOX_BOTTOM
            @li CLOTH_FRONT
            @li CLOTH_BACK
            @li CYLINDER_MIDDLE_FRONT
            @li CYLINDER_MIDDLE_BACK
            @li CYLINDER_FRONT
            @li CYLINDER_BACK
            @li CYLINDER_CURVED
            @li SPHERE_FRONT
            @li SPHERE_BACK
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_face_type(void)
{
   Edje_Physics_Face *pface, *pface2;
   Eina_List *l;

   pface = eina_list_data_get(eina_list_last(current_desc->physics.faces));
   pface->type = parse_enum(0,
                            "BOX_MIDDLE_FRONT", EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT,
                            "BOX_MIDDLE_BACK", EPHYSICS_BODY_BOX_FACE_MIDDLE_BACK,
                            "BOX_FRONT", EPHYSICS_BODY_BOX_FACE_FRONT,
                            "BOX_BACK", EPHYSICS_BODY_BOX_FACE_BACK,
                            "BOX_LEFT", EPHYSICS_BODY_BOX_FACE_LEFT,
                            "BOX_RIGHT", EPHYSICS_BODY_BOX_FACE_RIGHT,
                            "BOX_TOP", EPHYSICS_BODY_BOX_FACE_TOP,
                            "BOX_BOTTOM", EPHYSICS_BODY_BOX_FACE_BOTTOM,
                            "CLOTH_FRONT", EPHYSICS_BODY_CLOTH_FACE_FRONT,
                            "CLOTH_BACK", EPHYSICS_BODY_CLOTH_FACE_BACK,
                            "CYLINDER_MIDDLE_FRONT", EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT,
                            "CYLINDER_MIDDLE_BACK", EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_BACK,
                            "CYLINDER_FRONT", EPHYSICS_BODY_CYLINDER_FACE_FRONT,
                            "CYLINDER_BACK", EPHYSICS_BODY_CYLINDER_FACE_BACK,
                            "CYLINDER_CURVED", EPHYSICS_BODY_CYLINDER_FACE_CURVED,
                            "SPHERE_FRONT", EPHYSICS_BODY_SPHERE_FACE_FRONT,
                            "SPHERE_BACK", EPHYSICS_BODY_SPHERE_FACE_BACK,
                            NULL);

   EINA_LIST_FOREACH(current_desc->physics.faces, l, pface2)
     {
        if ((pface != pface2) && (pface->type == pface2->type))
          {
             ERR("parse error %s:%i. There is already a face of type \"%i\"",
                 file_in, line - 1, pface->type);
             exit(-1);
          }
     }
}

#endif

/**
    @page edcref
    @property
        source
    @parameters
        [another group's name]
    @effect
        This sets the group that is used as the object representing the physics
        body face.
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_parts_part_description_physics_face_source(void)
{
   Edje_Physics_Face *pface;

   pface = eina_list_data_get(eina_list_last(current_desc->physics.faces));
   check_arg_count(1);

   pface->source = parse_str(0);
   data_queue_face_group_lookup(pface->source);
}

#endif

/** @edcsubsection{collections_group_parts_description_map,
 *                 Group.Parts.Part.Description.Map} */

/**
    @page edcref
    @block
        map
    @context
    description {
        ..
        map {
            perspective: "name";
            light: "name";
            on: 1;
            smooth: 1;
            perspective_on: 1;
            backface_cull: 1;
            alpha: 1;

            rotation {
                ..
            }
        }
        ..
    }

    @description
    @endblock

    @property
        perspective
    @parameters
        [another part's name]
    @effect
        This sets the part that is used as the "perspective point" for giving
        a part a "3d look". The perspective point should have a perspective
        section that provides zplane and focal properties. The center of this
        part will be used as the focal point, so size, color and visibility
        etc. are not relevant just center point, zplane and focal are used.
        This also implicitly enables perspective transforms (see the on
        parameter for the map section).
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_perspective(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->map.id_persp));
      free(name);
   }

   current_desc->map.persp_on = 1;
}

/**
    @page edcref
    @property
        light
    @parameters
        [another part's name]
    @effect
        This sets the part that is used as the "light" for calculating the
        brightness (based on how directly the part's surface is facing the
        light source point). Like the perspective point part, the center point
        is used and zplane is used for the z position (0 being the zero-plane
        where all 2D objects normally live) and positive values being further
        away into the distance. The light part color is used as the light
        color (alpha not used for light color). The color2 color is used for
        the ambient lighting when calculating brightness (alpha also not
        used).
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_light(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->map.id_light));
      free(name);
   }
}

/**
    @page edcref
    @property
        on
    @parameters
        [1 or 0]
    @effect
        This enables mapping for the part.

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_on(void)
{
   check_arg_count(1);

   current_desc->map.on = parse_bool(0);
}

/**
    @page edcref
    @property
        smooth
    @parameters
        [1 or 0]
    @effect
        This enable smooth map rendering. This may be linear interpolation,
        anisotropic filtering or anything the engine decides is "smooth".
        This is a best-effort hint and may not produce precisely the same
        results in all engines and situations.

        Defaults: 1
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_smooth(void)
{
   check_arg_count(1);

   current_desc->map.smooth = parse_bool(0);
}

/**
    @page edcref
    @property
        alpha
    @parameters
        [1 or 0]
    @effect
        This enable alpha channel when map rendering.

        Defaults: 1
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_alpha(void)
{
   check_arg_count(1);

   current_desc->map.alpha = parse_bool(0);
}

/**
    @page edcref
    @property
        backface_cull
    @parameters
        [1 or 0]
    @effect
        This enables backface culling (when the rotated part that normally
        faces the camera is facing away after being rotated etc.). This means
        that the object will be hidden when "backface culled".

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_backface_cull(void)
{
   check_arg_count(1);

   current_desc->map.backcull = parse_bool(0);
}

/**
    @page edcref
    @property
        perspective_on
    @parameters
       [1 or 0]
    @effect
        Enable perspective when rotating even without a perspective point object.
        This would use perspective set for the object itself or for the
        canvas as a whole as the global perspective with
        edje_perspective_set() and edje_perspective_global_set().

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_perspective_on(void)
{
   check_arg_count(1);

   current_desc->map.persp_on = parse_bool(0);
}

/**
    @page edcref
    @property
        color
    @parameters
        [point] [red] [green] [blue] [alpha] or
        [point] "#[RR][GG][BB](AA)" or "#[R][G][B](A)"
    @effect
        Set the color of a vertex in the map.
        Colors will be linearly interpolated between vertex points through the map.
        The default color of a vertex in a map is white solid (255, 255, 255, 255)
        which means it will have no affect on modifying the part pixels.
        Currently only four points are supported:
        @li 0 - Left-Top point of a part.
        @li 1 - Right-Top point of a part.
        @li 2 - Left-Bottom point of a part.
        @li 3 - Right-Bottom point of a part.

        Color format:
        @li [red] [green] [blue] [alpha]: one integer [0-255] for each
        RGBA channel, i.e. 255 0 0 255
        @li "#[RR][GG][BB](AA)": string with two hex values per RGBA channel,
        i.e "#FF0000FF" or "#FF0000"
        @li "#[R][G][B](A)": string with one hex value per RGBA channel,
        i.e "#F00F" or "#F00".\n
        In string format you can omit alpha channel and it will be set to FF.

        Defaults: 255 255 255 255
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_color(void)
{
   Edje_Map_Color *color;
   Edje_Map_Color tmp;
   int i;

   check_min_arg_count(2);

   tmp.idx = parse_int(0);
   parse_color(1, &tmp.r);

   for (i = 0; i < (int)current_desc->map.colors_count; i++)
     {
        color = current_desc->map.colors[i];
        if (color->idx != tmp.idx) continue;
        color->r = tmp.r;
        color->g = tmp.g;
        color->b = tmp.b;
        color->a = tmp.a;
        return;
     }
   color = mem_alloc(SZ(Edje_Map_Color));
   if (!color)
     {
        ERR("not enough memory");
        exit(-1);
     }

   *color = tmp;
   current_desc->map.colors_count++;
   current_desc->map.colors =
     realloc(current_desc->map.colors,
             sizeof(Edje_Map_Color *) * current_desc->map.colors_count);
   current_desc->map.colors[current_desc->map.colors_count - 1] = color;
}

/**
    @page edcref
    @property
        zoom.x
    @parameters
        [X horizontal zoom to use]
    @effect
        This sets the zoom rate of the horizontal

        Defaults: 1.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_zoom_x(void)
{
   check_arg_count(1);

   current_desc->map.zoom.x = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
}

/**
    @page edcref
    @property
        zoom.y
    @parameters
        [Y vertical zoom to use]
    @effect
        This sets the zoom rate of vertical

        Defaults: 1.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_zoom_y(void)
{
   check_arg_count(1);

   current_desc->map.zoom.y = FROM_DOUBLE(parse_float_range(0, 0.0, 999999999.0));
}

/** @edcsubsection{collections_group_parts_description_map_rotation,
 *                 Group.Parts.Part.Description.Map.Rotation} */

/**
    @page edcref
    @block
        rotation
    @context
    map {
        ..
        rotation {
            center: "name";
            x: 45.0;
            y: 45.0;
            z: 45.0;
        }
        ..
    }
    @description
        Rotates the part, optionally with the center on another part.
    @endblock

    @property
        center
    @parameters
        [another part's name]
    @effect
        This sets the part that is used as the center of rotation when
        rotating the part with this description. The part's center point
        is used as the rotation center when applying rotation around the
        x, y and z axes. If no center is given, the parts original center
        itself is used for the rotation center.
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_rotation_center(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->map.rot.id_center));
      free(name);
   }
}

/**
    @page edcref
    @property
        x
    @parameters
        [X degrees]
    @effect
        This sets the rotation around the x axis of the part considering
        the center set. In degrees.

        Defaults: 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_rotation_x(void)
{
   check_arg_count(1);

   current_desc->map.rot.x = FROM_DOUBLE(parse_float(0));
}

/**
    @page edcref
    @property
        y
    @parameters
        [Y degrees]
    @effect
        This sets the rotation around the y axis of the part considering
        the center set. In degrees.

        Defaults: 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_rotation_y(void)
{
   check_arg_count(1);

   current_desc->map.rot.y = FROM_DOUBLE(parse_float(0));
}

/**
    @page edcref
    @property
        z
    @parameters
        [Z degrees]
    @effect
        This sets the rotation around the z axis of the part considering
        the center set. In degrees.

        Defaults: 0.0
    @endproperty
 */
static void
st_collections_group_parts_part_description_map_rotation_z(void)
{
   check_arg_count(1);

   current_desc->map.rot.z = FROM_DOUBLE(parse_float(0));
}

/** @edcsubsection{collections_group_parts_description_perspective,
 *                 Group.Parts.Part.Description.Perspective} */

/**
    @page edcref
    @block
        perspective
    @context
    description {
        ..
        perspective {
            zplane: 0;
            focal: 1000;
        }
        ..
    }
    @description
        Adds focal and plane perspective to the part. Active if perspective_on is true.
        Must be provided if the part is being used by other part as it's perspective target.
    @endblock

    @property
        zplane
    @parameters
        [unscaled Z value]
    @effect
        This sets the z value that will not be scaled. Normally this is 0 as
        that is the z distance that all objects are at normally.

        Defaults: 0
    @endproperty
 */
static void
st_collections_group_parts_part_description_perspective_zplane(void)
{
   check_arg_count(1);

   current_desc->persp.zplane = parse_int(0);
}

/**
    @page edcref
    @property
        focal
    @parameters
        [distance]
    @effect
        This sets the distance from the focal z plane (zplane) and the
        camera - i.e. very much equating to focal length of the camera

        Defaults: 1000
    @endproperty
 */
static void
st_collections_group_parts_part_description_perspective_focal(void)
{
   check_arg_count(1);

   current_desc->persp.focal = parse_int_range(0, 1, 0x7fffffff);
}

/** @edcsubsection{collections_group_parts_description_filter,
 *                 Group.Parts.Part.Description.Filter} */

/**
    @page edcref
    @block
        filter
    @context
        part {
            type: [IMAGE or TEXT or PROXY or SNAPSHOT];
            ..
            description {
                ..
                filter {
                   code: "blend {}";
                   // or:
                   code: "filter_name";
                   source: "part1" "buf";
                   source: "part2" "otherbuf";
                   source: "part3";
                   ..
                   data: "the_answer" "42";
                   data: "something" "anything";
                   data: "mycc" "color_class('my_color_class')";
                   ..
                }
                // or:
                text.filter: "blend {} -- ...";
                ..
            }
        }
    @description
        Applies a series of image filters to a TEXT, IMAGE, PROXY or SNAPSHOT part.
        For more information, please refer to the page
        @ref evasfiltersref "Evas filters reference".
    @endblock

    @property
        filter.code
    @parameters
        [filter script or filter name]
    @effect
        The argument to this field is the source code of a Lua script as defined
        @ref evasfiltersref "here" or a filter name defined in the
        @ref sec_collections_group_filters "Filters" section.
    @endproperty
 */
static void
st_collections_group_parts_part_description_filter_code(void)
{
   Edje_Part_Description_Spec_Filter *filter;

   check_arg_count(1);

   if (current_part->type == EDJE_PART_TYPE_TEXT)
     filter = &(((Edje_Part_Description_Text *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_IMAGE)
     filter = &(((Edje_Part_Description_Image *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_PROXY)
     filter = &(((Edje_Part_Description_Proxy *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_SNAPSHOT)
     filter = &(((Edje_Part_Description_Snapshot *)current_desc)->filter);
   else
     {
        ERR("parse error %s:%i. filter only supported for: TEXT, IMAGE, PROXY, SNAPSHOT.",
            file_in, line - 1);
        exit(-1);
     }

   free((void *)filter->code);
   filter->code = parse_str(0);
}

/**
    @page edcref

    @property
        filter.source
    @parameters
        [another part's name] [(optional) buffer name for filter program]
    @effect
        Binds another part as an image source (like a proxy source) for a
        text or image filter operation. Optionally, a buffer name may be
        specified, so the same filter code can be used with different sources.
    @endproperty
 */
static void
st_collections_group_parts_part_description_filter_source(void)
{
   Edje_Part_Description_Spec_Filter *filter;
   Edje_Part_Collection *pc;
   char *name, *part, *str;
   size_t sn = 0, sp, k;
   int *part_key;
   int args;

   static const char allowed_name_chars[] =
     "abcdefghijklmnopqrstuvwxyzABCDEFGHJIKLMNOPQRSTUVWXYZ0123456789_";

   if (current_part->type == EDJE_PART_TYPE_TEXT)
     filter = &(((Edje_Part_Description_Text *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_IMAGE)
     filter = &(((Edje_Part_Description_Image *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_PROXY)
     filter = &(((Edje_Part_Description_Proxy *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_SNAPSHOT)
     filter = &(((Edje_Part_Description_Snapshot *)current_desc)->filter);
   else
     {
        ERR("parse error %s:%i. filter only supported for: TEXT, IMAGE, PROXY, SNAPSHOT.",
            file_in, line - 1);
        exit(-1);
     }

   args = check_range_arg_count(1, 2);
   pc = eina_list_data_get(eina_list_last(edje_collections));

   part = parse_str(0);
   sp = strlen(part);

   if (args > 1)
     {
        name = parse_str(1);
        if (name) sn = strlen(name);
        if (!name || (strspn(name, allowed_name_chars) != sn))
          {
             ERR("parse error %s:%i. invalid name for a filter buffer: '%s'",
                 file_in, line - 1, name);
             exit(-1);
          }
     }
   else
     name = NULL;

   if (!name && (strspn(part, allowed_name_chars) == sp))
     str = strdup(part);
   else
     {
        if (!name)
          {
             // name = part so we replace all invalid chars by '_'
             name = strdup(part);
             sn = strlen(name);
             for (k = 0; k < sn; k++)
               {
                  if (!strchr(allowed_name_chars, name[k]))
                    name[k] = '_';
               }
          }
        sn += sp + 1;
        str = malloc(sn + 1);
        if (!str) exit(-1);
        strncpy(str, name, sn);
        strncat(str, ":", sn);
        strncat(str, part, sn);
        str[sn] = '\0';
     }
   filter->sources = eina_list_append(filter->sources, str);

   // note: this is leaked. not a big deal.
   part_key = malloc(sizeof(int));
   *part_key = -1;
   data_queue_part_lookup(pc, part, part_key);

   free(part);
   free(name);
}

/**
    @page edcref

    @property
        filter.data
    @parameters
        [name] [content]
    @effect
        Pass extra data to the Lua filter program. All data passed will
        be strings, except for the special case of color classes:
        @code
        filter.data: "mycc" "color_class('my_color_class')";
        @endcode
        Those will appear to Lua as a table of the following structure:
        @code
        -- Lua code
        mycc = { r = 255, g = 0, b, a, r2, g2, b2, a2, r3, g3, b3, a3 }
        @endcode
    @endproperty
 */
static void
st_collections_group_parts_part_description_filter_data(void)
{
   Edje_Part_Description_Spec_Filter_Data *array;
   Edje_Part_Description_Spec_Filter *filter;
   char *name, *value;
   unsigned k;

   if (current_part->type == EDJE_PART_TYPE_TEXT)
     filter = &(((Edje_Part_Description_Text *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_IMAGE)
     filter = &(((Edje_Part_Description_Image *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_PROXY)
     filter = &(((Edje_Part_Description_Proxy *)current_desc)->filter);
   else if (current_part->type == EDJE_PART_TYPE_SNAPSHOT)
     filter = &(((Edje_Part_Description_Snapshot *)current_desc)->filter);
   else
     {
        ERR("parse error %s:%i. filter only supported for: TEXT, IMAGE, PROXY, SNAPSHOT.",
            file_in, line - 1);
        exit(-1);
     }

   check_arg_count(2);

   name = parse_str(0);
   value = parse_str(1);
   for (k = 0; k < filter->data_count; k++)
     if (!strcmp(filter->data[k].name, name))
       {
          ERR("parse error %s:%i. filter.data '%s' already exists in this context",
              file_in, line - 1, name);
          exit(-1);
       }

   filter->data_count++;
   array = realloc(filter->data, sizeof(Edje_Part_Description_Spec_Filter_Data) * filter->data_count);
   array[filter->data_count - 1].name = name;
   array[filter->data_count - 1].value = value;
   filter->data = array;
}

/** @edcsubsection{collections_group_parts_descriptions_params,
 *                 Group.Parts.Part.Description.Params} */

/**
    @page edcref
    @block
        params
    @context
        description {
            ..
            params {
                int: "name" 0;
                double: "other_name" 0.0;
                string: "another_name" "some text";
                bool: "name" 1;
                choice: "some_name" "value";
            }
            ..
        }
    @description
        Set parameters for EXTERNAL parts. The value overwrites previous
        definitions with the same name.
    @endblock
 */
static void
_st_collections_group_parts_part_description_params(Edje_External_Param_Type type)
{
   Edje_Part_Description_External *ed;
   Edje_External_Param *param;
   Eina_List *l;
   char *name;
   int found = 0;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        ERR("parse error %s:%i. params in non-EXTERNAL part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_External *)current_desc;

   name = parse_str(0);

   /* if a param with this name already exists, overwrite it */
   EINA_LIST_FOREACH(ed->external_params, l, param)
     {
        if (!strcmp(param->name, name))
          {
             found = 1;
             free(name);
             break;
          }
     }

   if (!found)
     {
        param = mem_alloc(SZ(Edje_External_Param));
        param->name = name;
     }

   param->type = type;
   param->i = 0;
   param->d = 0;
   param->s = NULL;

   switch (type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        param->i = parse_bool(1);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_INT:
        param->i = parse_int(1);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        param->d = parse_float(1);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        param->s = parse_str(1);
        break;

      default:
        ERR("parse error %s:%i. Invalid param type.",
            file_in, line - 1);
        break;
     }

   if (!found)
     ed->external_params = eina_list_append(ed->external_params, param);
}

/**
    @page edcref
    @property
        int
    @parameters
        [param_name] [int_value]
    @effect
        Adds an integer parameter for an external object
    @endproperty
 */
static void
st_collections_group_parts_part_description_params_int(void)
{
   _st_collections_group_parts_part_description_params(EDJE_EXTERNAL_PARAM_TYPE_INT);
}

/**
    @page edcref
    @property
        double
    @parameters
        [param_name] [double_value]
    @effect
        Adds a double parameter for an external object
    @endproperty
 */
static void
st_collections_group_parts_part_description_params_double(void)
{
   _st_collections_group_parts_part_description_params(EDJE_EXTERNAL_PARAM_TYPE_DOUBLE);
}

/**
    @page edcref
    @property
        string
    @parameters
        [param_name] [string_value]
    @effect
        Adds a string parameter for an external object
    @endproperty
 */
static void
st_collections_group_parts_part_description_params_string(void)
{
   _st_collections_group_parts_part_description_params(EDJE_EXTERNAL_PARAM_TYPE_STRING);
}

/**
    @page edcref
    @property
        bool
    @parameters
        [param_name] [bool_value]
    @effect
        Adds an boolean parameter for an external object.
    @endproperty
 */
static void
st_collections_group_parts_part_description_params_bool(void)
{
   _st_collections_group_parts_part_description_params(EDJE_EXTERNAL_PARAM_TYPE_BOOL);
}

/**
    @page edcref
    @property
        choice
    @parameters
        [param_name] [choice_string]
    @effect
        Adds a choice parameter for an external object. The possible
        choice values are defined by external type at their register time
        and will be validated at runtime.
    @endproperty
 */
static void
st_collections_group_parts_part_description_params_choice(void)
{
   _st_collections_group_parts_part_description_params(EDJE_EXTERNAL_PARAM_TYPE_CHOICE);
}

/** @edcsubsection{collections_group_parts_description_links,
 *                 Group.Parts.Part.Description.Links} */

/**
    @page edcref
    @block
        link
    @context
        description {
            ..
            link {
                base: "edje,signal" "edje";
                transition: LINEAR 0.2;
                in: 0.5 0.1;
                after: "some_program";
            }
            ..
        }
    @description
        The link block can be used to create transitions to the enclosing part description state.
        The result of the above block is identical to creating a program with
        action: STATE_SET "default";
        signal: "edje,signal"; source: "edje";
    @since 1.10
    @endblock
 */
static void
ob_collections_group_parts_part_description_link(void)
{
   Edje_Part_Collection_Parser *pcp;
   Edje_Part_Parser *epp;
   Edje_Part_Description_Link *el;
   const char *nm;

   pcp = eina_list_last_data_get(edje_collections);
   epp = (Edje_Part_Parser *)current_part;

   ob_collections_group_programs_program();
   _edje_program_remove((Edje_Part_Collection *)pcp, current_program);
   el = mem_alloc(SZ(Edje_Part_Description_Link));
   el->pr = current_program;
   el->ed = current_desc;
   el->epp = epp;
   pcp->links = eina_list_append(pcp->links, el);
   current_program->action = EDJE_ACTION_TYPE_STATE_SET;
   nm = current_desc->state.name;
   current_program->state = strdup(nm ? nm : "default");
   current_program->value = current_desc->state.value;
}

/**
    @page edcref
    @property
        base
    @parameters
        [signal] [source]
    @effect
        Defines the signal and source which will trigger the transition to this state.
        The source parameter is optional here and will be filled with the current group's
        default value if it is not provided.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_parts_part_description_link_base(void)
{
   char *name;
   char buf[4096];
   Edje_Part_Collection_Parser *pcp;
   Edje_Part_Description_Link *el, *ell;
   Eina_List *l;

   pcp = eina_list_last_data_get(edje_collections);
   el = eina_list_last_data_get(pcp->links);

   if ((!el) || (el->pr != current_program) ||
       (el->ed != current_desc) || (el->epp != (Edje_Part_Parser *)current_part) ||
       el->pr->source)
     ob_collections_group_parts_part_description_link();
   el = eina_list_last_data_get(pcp->links);

   check_min_arg_count(1);
   name = parse_str(0);
   if (current_program->signal && pcp->link_hash)
     {
        snprintf(buf, sizeof(buf), "%s\"\"\"%s", current_program->signal,
                 current_program->source ? current_program->source : "");
        eina_hash_list_remove(pcp->link_hash, buf, el);
     }
   if (!pcp->link_hash)
     pcp->link_hash = eina_hash_string_superfast_new((Eina_Free_Cb)eina_list_free);
   free((void *)current_program->signal);
   current_program->signal = name;
   if (get_arg_count() == 2)
     {
        name = parse_str(1);
        free((void *)current_program->source);
        current_program->source = name;
     }
   snprintf(buf, sizeof(buf), "%s\"\"\"%s", current_program->signal,
            current_program->source ? current_program->source : "");
   EINA_LIST_FOREACH(eina_hash_find(pcp->link_hash, buf), l, ell)
     {
        if (ell->epp == el->epp)
          {
             ERR("parse error %s:%i. "
                 "cannot have multiple links with the same signal on the same part",
                 file_in, line - 1);
             exit(-1);
          }
     }
   eina_hash_list_append(pcp->link_hash, buf, el);
}

/** @edcsubsection{collections_group_programs,
 *                 Group.Programs} */

/**
    @page edcref
    @block
        programs
    @context
        group {
            programs {
                ..
                program { }
                program { }
                program { }
                ..
            }
        }
    @description
        The programs group contain one ore more program.
    @endblock
 */
static void
_program_sequence_check(void)
{
   if (sequencing != current_program) return;
   ERR("parse error %s:%i. cannot set sequence parent program attributes within sequence block",
       file_in, line - 1);
   exit(-1);
}

static void
_program_after(const char *name)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;
   Edje_Program_After *pa;
   Edje_Program_After *pa2;
   Eina_List *l;
   char *copy;
   void *pl;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_program;

   EINA_LIST_FOREACH(ep->after, l, pa2)
     {
        if (!strcmp(name, (char *)(pa2 + 1)))
          return;
     }

   pa = mem_alloc(SZ(Edje_Program_After) + strlen(name) + 1);
   pa->id = -1;
   ep->after = eina_list_append(ep->after, pa);
   copy = (char *)(pa + 1);
   memcpy(copy, name, strlen(name) + 1);
   pl = data_queue_program_lookup(pc, name, &(pa->id));
   if (pl)
     current_program_lookups = eina_list_append(current_program_lookups, pl);
}

static Edje_Program *
_program_sequence_new(void)
{
   Edje_Program *ep, *pr = current_program;

   /* sequence a new program after current */
   ob_collections_group_programs_program();
   ep = current_program;
   current_program = pr;
   _program_after(ep->name);
   return current_program = ep;
}

/** @edcsubsection{collections_group_programs_program,
 *                 Group.Programs.Program} */

/**
    @page edcref
    @block
        program
    @context
        programs {
            ..
            program {
                name: "programname";
                signal: "signalname";
                source: "partname";
                filter: "partname" "statename";
                in: 0.3 0.0;
                action: STATE_SET "statename" state_value;
                transition: LINEAR 0.5;
                target: "partname";
                target: "anotherpart";
                after: "programname";
                after: "anotherprogram";
            }
            ..
        }
    @description
        Programs define how your interface reacts to events.
        Programs can change the state of parts, react to events or trigger
        other events.
    @endblock
 */
static void
ob_collections_group_programs_program(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Collection_Parser *pcp;
   Edje_Program *ep;
   Edje_Program_Parser *epp;
   char *def_name;

   current_program_lookups = eina_list_free(current_program_lookups);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pcp = eina_list_data_get(eina_list_last(edje_collections));

   ep = mem_alloc(SZ(Edje_Program_Parser));
   ep->id = -1;
   ep->source_3d_id = -1;
   ep->tween.mode = EDJE_TWEEN_MODE_LINEAR;
   ep->tween.use_duration_factor = EINA_FALSE;
   ep->after = NULL;
   epp = (Edje_Program_Parser *)ep;
   epp->can_override = EINA_FALSE;

   /* generate new name */
   def_name = alloca(strlen("program_") + strlen("FFFFFFFFFFFFFFFF") + 1);
   sprintf(def_name, "program_%X", pc->programs.total_count);
   ep->name = strdup(def_name);
   if (pcp->default_source)
     ep->source = strdup(pcp->default_source);
   _edje_program_insert(pc, ep);

   current_program = ep;
   pc->programs.total_count++;
}

static void
_program_name(char *name)
{
   Edje_Part_Collection *pc;
   Eina_List *l;
   void *pl;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   free((void *)current_program->name);
   current_program->name = name;

   _edje_program_check(current_program->name, current_program, pc->programs.fnmatch, pc->programs.fnmatch_count);
   _edje_program_check(current_program->name, current_program, pc->programs.strcmp, pc->programs.strcmp_count);
   _edje_program_check(current_program->name, current_program, pc->programs.strncmp, pc->programs.strncmp_count);
   _edje_program_check(current_program->name, current_program, pc->programs.strrncmp, pc->programs.strrncmp_count);
   _edje_program_check(current_program->name, current_program, pc->programs.nocmp, pc->programs.nocmp_count);

   EINA_LIST_FOREACH(current_program_lookups, l, pl)
     program_lookup_rename(pl, name);
}

/**
    @page edcref
    @property
        name
    @parameters
        [program name]
    @effect
        Symbolic name of program as a unique identifier.
    @endproperty
 */
static void
st_collections_group_programs_program_name(void)
{
   check_arg_count(1);
   _program_sequence_check();
   _program_name(parse_str(0));
}

/**
    @page edcref
    @property
        signal
    @parameters
        [signal name]
    @effect
        Specifies signal(s) that should cause the program to run. The signal
        received must match the specified source to run.
        Signals may be globbed, but only one signal keyword per program
        may be used. ex: signal: "mouse,clicked,*"; (clicking any mouse button
        that matches source starts program).
        A list of global signal, that edje provide:
          - hold,on;
          - hold,off;
          - mouse,in;
          - mouse,out;
          - mouse,pressed,in;
          - mouse,pressed,out;
          - mouse,down,N: where N - mouse button number;
          - mouse,down,N,double: where N - mouse button number;
          - mouse,down,N,triple: where N - mouse button number;
          - mouse,up,N: where N - mouse button number;
          - mouse,clicked,N: where N - mouse button number;
          - mouse,move;
          - mouse,wheel,N,M: where N - the direction (by default is 0),
            M - 1 if wheel scrolled up and -1 if down;
          - drag,start;
          - drag;
          - drag,stop;
          - focus,part,in;
          - focus,part,out.
    @endproperty
 */
static void
st_collections_group_programs_program_signal(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   _program_sequence_check();

   pc = eina_list_data_get(eina_list_last(edje_collections));

   _edje_program_remove(pc, current_program);
   current_program->signal = parse_str(0);
   _edje_program_insert(pc, current_program);
}

/**
    @page edcref
    @property
        source
    @parameters
        [source name]
    @effect
        Source of accepted signal. Sources may be globbed, but only one source
        keyword per program may be used. ex: source: "button-*"; (Signals from
        any part or program named "button-*" are accepted).
    @endproperty
 */
static void
st_collections_group_programs_program_source(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   _program_sequence_check();

   pc = eina_list_data_get(eina_list_last(edje_collections));

   _edje_program_remove(pc, current_program);
   free((void *)current_program->source);
   current_program->source = parse_str(0);
   _edje_program_insert(pc, current_program);
}

/**
    @page edcref
    @property
        filter
    @parameters
        [part] [state]
    @effect
        Filter signals to be only accepted if the part [part] is in state named [state].
        Only one filter per program can be used. If [part] is not given, the source of
        the event will be used instead.
    @endproperty
 */
static void
st_collections_group_programs_program_filter(void)
{
   check_min_arg_count(1);

   _program_sequence_check();

   if (is_param(1))
     {
        current_program->filter.part = parse_str(0);
        current_program->filter.state = parse_str(1);
     }
   else
     {
        current_program->filter.state = parse_str(0);
     }
}

/**
    @page edcref
    @property
        in
    @parameters
        [from] [range]
    @effect
        Wait 'from' seconds before executing the program. And add a random
        number of seconds (from 0 to 'range') to the total waiting time.

        Defaults: 0.0 0.0
    @endproperty
 */
static void
st_collections_group_programs_program_in(void)
{
   check_arg_count(2);

   _program_sequence_check();

   current_program->in.from = parse_float_range(0, 0.0, 999999999.0);
   current_program->in.range = parse_float_range(1, 0.0, 999999999.0);
}

/**
    @page edcref
    @property
        action
    @parameters
        [type] (param1) (param2) (param3) (param4)
    @effect
        Action to be performed by the program. Valid actions are:
        @li STATE_SET "state name" (state value)
        @li ACTION_STOP
        @li SIGNAL_EMIT "signalname" "emitter"
        @li DRAG_VAL_SET 0.5 0.0
        @li DRAG_VAL_STEP 1.0 0.0
        @li DRAG_VAL_PAGE 0.0 0.0
        @li FOCUS_SET ("seat")
        @li FOCUS_OBJECT ("seat")
        @li PARAM_COPY "src_part" "src_param" "dst_part" "dst_param"
        @li PARAM_SET "part" "param" "value"
        @li PLAY_SAMPLE "sample name" speed (channel)
        @li PLAY_TONE "tone name" duration_in_seconds( Range 0.1 to 10.0 )
        @li PLAY_VIBRATION "sample name" repeat (repeat count)
        @li PHYSICS_IMPULSE 10 -23.4 0
        @li PHYSICS_TORQUE_IMPULSE 0 2.1 0.95
        @li PHYSICS_FORCE -20.8 0 30.85
        @li PHYSICS_TORQUE 0 0 4.8
        @li PHYSICS_FORCES_CLEAR
        @li PHYSICS_VEL_SET 40.9 0 0
        @li PHYSICS_ANG_VEL_SET 12.4 0 0.66
        @li PHYSICS_STOP
        @li PHYSICS_ROT_SET 0.707 0 0 0.707

        Only one action can be specified per program.

        PLAY_SAMPLE (optional) channel can be one of:
        @li EFFECT/FX
        @li BACKGROUND/BG
        @li MUSIC/MUS
        @li FOREGROUND/FG
        @li INTERFACE/UI
        @li INPUT
        @li ALERT

    @endproperty
 */
static void
st_collections_group_programs_program_action(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;
   int i;
   Eina_Bool found = EINA_FALSE;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   if (sequencing)
     ep = _program_sequence_new();
   else
     ep = current_program;
   ep->action = parse_enum(0,
                           "STATE_SET", EDJE_ACTION_TYPE_STATE_SET,
                           "ACTION_STOP", EDJE_ACTION_TYPE_ACTION_STOP,
                           "SIGNAL_EMIT", EDJE_ACTION_TYPE_SIGNAL_EMIT,
                           "DRAG_VAL_SET", EDJE_ACTION_TYPE_DRAG_VAL_SET,
                           "DRAG_VAL_STEP", EDJE_ACTION_TYPE_DRAG_VAL_STEP,
                           "DRAG_VAL_PAGE", EDJE_ACTION_TYPE_DRAG_VAL_PAGE,
                           "SCRIPT", EDJE_ACTION_TYPE_SCRIPT,
                           "FOCUS_SET", EDJE_ACTION_TYPE_FOCUS_SET,
                           "FOCUS_OBJECT", EDJE_ACTION_TYPE_FOCUS_OBJECT,
                           "PARAM_COPY", EDJE_ACTION_TYPE_PARAM_COPY,
                           "PARAM_SET", EDJE_ACTION_TYPE_PARAM_SET,
                           "PLAY_SAMPLE", EDJE_ACTION_TYPE_SOUND_SAMPLE,
                           "PLAY_TONE", EDJE_ACTION_TYPE_SOUND_TONE,
                           "PHYSICS_IMPULSE", EDJE_ACTION_TYPE_PHYSICS_IMPULSE,
                           "PHYSICS_TORQUE_IMPULSE", EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE,
                           "PHYSICS_FORCE", EDJE_ACTION_TYPE_PHYSICS_FORCE,
                           "PHYSICS_TORQUE", EDJE_ACTION_TYPE_PHYSICS_TORQUE,
                           "PHYSICS_FORCES_CLEAR", EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR,
                           "PHYSICS_VEL_SET", EDJE_ACTION_TYPE_PHYSICS_VEL_SET,
                           "PHYSICS_ANG_VEL_SET", EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET,
                           "PHYSICS_STOP", EDJE_ACTION_TYPE_PHYSICS_STOP,
                           "PHYSICS_ROT_SET", EDJE_ACTION_TYPE_PHYSICS_ROT_SET,
                           "PLAY_VIBRATION", EDJE_ACTION_TYPE_VIBRATION_SAMPLE,
                           NULL);
   if (ep->action == EDJE_ACTION_TYPE_STATE_SET)
     {
        ep->state = parse_str(1);
        if (get_arg_count() == 2)
          ep->value = 0.0;
        else
          ep->value = parse_float_range(2, 0.0, 1.0);
     }
   else if ((ep->action == EDJE_ACTION_TYPE_FOCUS_SET) ||
            (ep->action == EDJE_ACTION_TYPE_FOCUS_OBJECT))
     {
        if (get_arg_count() == 1)
          ep->seat = NULL;
        else
          ep->seat = parse_str(1);
     }
   else if (ep->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
        ep->state = parse_str(1);
        ep->state2 = parse_str(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_SOUND_SAMPLE)
     {
        ep->sample_name = parse_str(1);
        if (edje_file->sound_dir)
          {
             for (i = 0; i < (int)edje_file->sound_dir->samples_count; i++)
               {
                  if (!strcmp(edje_file->sound_dir->samples[i].name, ep->sample_name))
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }
          }
        if (!found)
          {
             ERR("No Sample name %s exist.", ep->sample_name);
             exit(-1);
          }
        ep->speed = parse_float_range(2, 0.0, 100.0);
        if (get_arg_count() >= 4)
          ep->channel = parse_enum(3,
                                   "EFFECT", 0, "FX", 0,
                                   "BACKGROUND", 1, "BG", 1,
                                   "MUSIC", 2, "MUS", 2,
                                   "FOREGROUND", 3, "FG", 3,
                                   "INTERFACE", 4, "UI", 4,
                                   "INPUT", 5,
                                   "ALERT", 6,
                                   NULL);
     }
   else if (ep->action == EDJE_ACTION_TYPE_SOUND_TONE)
     {
        ep->tone_name = parse_str(1);
        if (edje_file->sound_dir)
          {
             for (i = 0; i < (int)edje_file->sound_dir->tones_count; i++)
               {
                  if (!strcmp(edje_file->sound_dir->tones[i].name, ep->tone_name))
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }
          }
        if (!found)
          {
             ERR("No Tone name %s exist.", ep->tone_name);
             exit(-1);
          }
        ep->duration = parse_float_range(2, 0.1, 10.0);
     }
   else if (ep->action == EDJE_ACTION_TYPE_VIBRATION_SAMPLE)
     {
        ep->vibration_name = parse_str(1);
        for (i = 0; i < (int)edje_file->vibration_dir->samples_count; i++)
          {
             if (!strcmp(edje_file->vibration_dir->samples[i].name, ep->vibration_name))
               break;
             if (i == (int)(edje_file->vibration_dir->samples_count - 1))
               {
                  ERR("No Vibration Sample name %s exist.", ep->vibration_name);
                  exit(-1);
               }
          }
        ep->vibration_repeat = parse_int(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
     {
        ep->value = parse_float(1);
        ep->value2 = parse_float(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
     {
        ep->value = parse_float(1);
        ep->value2 = parse_float(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
     {
        ep->value = parse_float(1);
        ep->value2 = parse_float(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_PARAM_COPY)
     {
        char *src_part, *dst_part;

        src_part = parse_str(1);
        ep->state = parse_str(2);
        dst_part = parse_str(3);
        ep->state2 = parse_str(4);

        data_queue_part_lookup(pc, src_part, &(ep->param.src));
        data_queue_part_lookup(pc, dst_part, &(ep->param.dst));

        free(src_part);
        free(dst_part);
     }
   else if (ep->action == EDJE_ACTION_TYPE_PARAM_SET)
     {
        char *part;

        part = parse_str(1);
        ep->state = parse_str(2);
        ep->state2 = parse_str(3);

        data_queue_part_lookup(pc, part, &(ep->param.dst));
        free(part);
     }
#ifdef HAVE_EPHYSICS
   else if ((ep->action == EDJE_ACTION_TYPE_PHYSICS_IMPULSE) ||
            (ep->action == EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE) ||
            (ep->action == EDJE_ACTION_TYPE_PHYSICS_FORCE) ||
            (ep->action == EDJE_ACTION_TYPE_PHYSICS_TORQUE) ||
            (ep->action == EDJE_ACTION_TYPE_PHYSICS_VEL_SET) ||
            (ep->action == EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET))
     {
        ep->physics.x = parse_float(1);
        ep->physics.y = parse_float(2);
        ep->physics.z = parse_float(3);
     }
   else if (ep->action == EDJE_ACTION_TYPE_PHYSICS_ROT_SET)
     {
        ep->physics.w = parse_float(1);
        ep->physics.x = parse_float(2);
        ep->physics.y = parse_float(3);
        ep->physics.z = parse_float(4);
     }
#endif

   switch (ep->action)
     {
      case EDJE_ACTION_TYPE_SCRIPT:
        /* this is implicitly set by script {} so this is here just for
         * completeness */
        break;

      case EDJE_ACTION_TYPE_ACTION_STOP:
      case EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR:
      case EDJE_ACTION_TYPE_PHYSICS_STOP:
        check_arg_count(1);
        break;

      case EDJE_ACTION_TYPE_PARAM_SET:
      case EDJE_ACTION_TYPE_PHYSICS_IMPULSE:
      case EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE:
      case EDJE_ACTION_TYPE_PHYSICS_FORCE:
      case EDJE_ACTION_TYPE_PHYSICS_TORQUE:
      case EDJE_ACTION_TYPE_PHYSICS_VEL_SET:
      case EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET:
        check_arg_count(4);
        break;

      case EDJE_ACTION_TYPE_PARAM_COPY:
      case EDJE_ACTION_TYPE_PHYSICS_ROT_SET:
        check_arg_count(5);
        break;

      case EDJE_ACTION_TYPE_SOUND_SAMPLE:
      case EDJE_ACTION_TYPE_VIBRATION_SAMPLE:
        break;

      case EDJE_ACTION_TYPE_STATE_SET:
        check_min_arg_count(2);
        break;

      case EDJE_ACTION_TYPE_FOCUS_SET:
      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
        check_min_arg_count(1);
        break;

      default:
        check_arg_count(3);
     }
}

/**
    @page edcref
    @property
        transition
    @parameters
        [type] [length] (interp val 1) (interp val 2) (option)
    @effect
        Defines how transitions occur using STATE_SET action.\n
        Where 'type' is the style of the transition and 'length' is a double
        specifying the number of seconds in which to preform the transition.\n
        Valid types are:
        @li LIN or LINEAR
        @li SIN or SINUSOIDAL
        @li ACCEL or ACCELERATE
        @li DECEL or DECELERATE
        @li ACCEL_FAC or ACCELERATE_FACTOR
        @li DECEL_FAC or DECELERATE_FACTOR
        @li SIN_FAC or SINUSOIDAL_FACTOR
        @li DIVIS or DIVISOR_INTERP
        @li BOUNCE
        @li SPRING
        @li CUBIC_BEZIER

        ACCEL_FAC, DECEL_FAC and SIN_FAC need the extra optional
        "interp val 1" to determine the "factor" of curviness. 1.0 is the same
        as their non-factor counterparts, where 0.0 is equal to linear.
        Numbers higher than one make the curve angles steeper with a more
        pronounced curve point.

        DIVIS, BOUNCE and SPRING also require "interp val 2" in addition
        to "interp val 1".

        DIVIS uses val 1 as the initial gradient start
        (0.0 is horizontal, 1.0 is diagonal (linear), 2.0 is twice the
        gradient of linear etc.). val 2 is interpreted as an integer factor
        defining how much the value swings "outside" the gradient only to come
        back to the final resting spot at the end. 0.0 for val 2 is equivalent
        to linear interpolation. Note that DIVIS can exceed 1.0

        BOUNCE uses val 2 as the number of bounces (so its rounded down to
        the nearest integer value), with val 1 determining how much the
        bounce decays, with 0.0 giving linear decay per bounce, and higher
        values giving much more decay.

        SPRING is similar to bounce, where val 2 specifies the number of
        spring "swings" and val 1 specifies the decay, but it can exceed 1.0
        on the outer swings.

        Valid options are:
        @li CURRENT causes the object to move from its current position.
        Can be used as the last parameter of any transition type. (since 1.1.0)

    @endproperty
 */
static void
st_collections_group_programs_program_transition(void)
{
   int current = -1, index = -1;
   unsigned int required_args = 0;

   check_min_arg_count(2);

   _program_sequence_check();

   current_program->tween.v1 = current_program->tween.v2 =
       current_program->tween.v3 = current_program->tween.v4 = 0.0;

   current_program->tween.mode = parse_enum(0,
     // short names
                                            "LIN", EDJE_TWEEN_MODE_LINEAR,
                                            "SIN", EDJE_TWEEN_MODE_SINUSOIDAL,
                                            "ACCEL", EDJE_TWEEN_MODE_ACCELERATE,
                                            "DECEL", EDJE_TWEEN_MODE_DECELERATE,
                                            "ACCEL_FAC", EDJE_TWEEN_MODE_ACCELERATE_FACTOR,
                                            "DECEL_FAC", EDJE_TWEEN_MODE_DECELERATE_FACTOR,
                                            "SIN_FAC", EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR,
                                            "DIVIS", EDJE_TWEEN_MODE_DIVISOR_INTERP,

     // long/full names
                                            "LINEAR", EDJE_TWEEN_MODE_LINEAR,
                                            "SINUSOIDAL", EDJE_TWEEN_MODE_SINUSOIDAL,
                                            "CUBIC_BEZIER", EDJE_TWEEN_MODE_CUBIC_BEZIER,
                                            "ACCELERATE", EDJE_TWEEN_MODE_ACCELERATE,
                                            "DECELERATE", EDJE_TWEEN_MODE_DECELERATE,
                                            "ACCELERATE_FACTOR", EDJE_TWEEN_MODE_ACCELERATE_FACTOR,
                                            "DECELERATE_FACTOR", EDJE_TWEEN_MODE_DECELERATE_FACTOR,
                                            "SINUSOIDAL_FACTOR", EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR,
                                            "DIVISOR_INTERP", EDJE_TWEEN_MODE_DIVISOR_INTERP,

     // long/full is short enough
                                            "BOUNCE", EDJE_TWEEN_MODE_BOUNCE,
                                            "SPRING", EDJE_TWEEN_MODE_SPRING,
                                            NULL);
   current_program->tween.time = FROM_DOUBLE(parse_float_range(1, 0.0, 999999999.0));

   //Check the index of params not related to tweenmode's param
   //This index use for count of the tweenmode's param
   if ((index = get_param_index("USE_DURATION_FACTOR")) != -1)
     {
        current_program->tween.use_duration_factor = parse_bool(index + 1);
        required_args += 2;
     }
   if ((current = get_param_index("CURRENT")) != -1)
     {
        if (index == -1 || current < index)
          index = current;
        required_args++;
     }
   switch (current_program->tween.mode)
     {
      case EDJE_TWEEN_MODE_LINEAR:
      case EDJE_TWEEN_MODE_SINUSOIDAL:
      case EDJE_TWEEN_MODE_ACCELERATE:
      case EDJE_TWEEN_MODE_DECELERATE:
      {
         required_args += 2;
         check_arg_count(required_args);
      }
      break;

      // the following need v1
      case EDJE_TWEEN_MODE_ACCELERATE_FACTOR:
      case EDJE_TWEEN_MODE_DECELERATE_FACTOR:
      case EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR:
      {
         required_args += 3;
         check_arg_count(required_args);
         if (index == -1 || index > 2)
           {
              current_program->tween.v1 =
                FROM_DOUBLE(parse_float_range(2, -999999999.0, 999999999.0));
              break;
           }
         else
           {
              ERR("parse error %s:%i. Need 3rd parameter to set factor",
                  file_in, line - 1);
              exit(-1);
           }
      }

      case EDJE_TWEEN_MODE_DIVISOR_INTERP:
      case EDJE_TWEEN_MODE_BOUNCE:
      case EDJE_TWEEN_MODE_SPRING:
      {
         required_args += 4;
         check_arg_count(required_args);
         if (index == -1 || index > 3)
           {
              current_program->tween.v1 =
                FROM_DOUBLE(parse_float_range(2, -999999999.0, 999999999.0));
              current_program->tween.v2 =
                FROM_DOUBLE(parse_float_range(3, -999999999.0, 999999999.0));
              break;
           }
         else
           {
              ERR("parse error %s:%i. "
                  "Need 3rd and 4th parameters to set factor and counts",
                  file_in, line - 1);
              exit(-1);
           }
      }

      case EDJE_TWEEN_MODE_CUBIC_BEZIER:
      {
         required_args += 6;
         check_arg_count(required_args);
         if (index == -1 || index > 5)
           {
              current_program->tween.v1 =
                FROM_DOUBLE(parse_float_range(2, -999999999.0, 999999999.0));
              current_program->tween.v2 =
                FROM_DOUBLE(parse_float_range(3, -999999999.0, 999999999.0));
              current_program->tween.v3 =
                FROM_DOUBLE(parse_float_range(4, -999999999.0, 999999999.0));
              current_program->tween.v4 =
                FROM_DOUBLE(parse_float_range(5, -999999999.0, 999999999.0));
              break;
           }
         else
           {
              ERR("parse error %s:%i. "
                  "Need 3rd, 4th, 5th and 6th parameters to set x1, y1, x2 and y2",
                  file_in, line - 1);
              exit(-1);
           }
      }
     }
   if (current > 0)
     current_program->tween.mode |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
}

static void
_program_target_add(char *name)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;
   Edje_Program_Target *et;
   Edje_Program_Target *etw;
   Eina_List *l;
   char *copy;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_program;

   EINA_LIST_FOREACH(ep->targets, l, etw)
     {
        if (!strcmp(name, (char *)(etw + 1)))
          {
             free(name);
             return;
          }
     }

   et = mem_alloc(SZ(Edje_Program_Target) + strlen(name) + 1);
   ep->targets = eina_list_append(ep->targets, et);
   copy = (char *)(et + 1);
   memcpy(copy, name, strlen(name) + 1);
   switch (ep->action)
     {
      case EDJE_ACTION_TYPE_ACTION_STOP:
        data_queue_program_lookup(pc, name, &(et->id));
        break;

      case EDJE_ACTION_TYPE_STATE_SET:
      case EDJE_ACTION_TYPE_SIGNAL_EMIT:
      case EDJE_ACTION_TYPE_DRAG_VAL_SET:
      case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
      case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
      case EDJE_ACTION_TYPE_FOCUS_SET:
      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
#ifdef HAVE_EPHYSICS
      case EDJE_ACTION_TYPE_PHYSICS_IMPULSE:
      case EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE:
      case EDJE_ACTION_TYPE_PHYSICS_FORCE:
      case EDJE_ACTION_TYPE_PHYSICS_TORQUE:
      case EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR:
      case EDJE_ACTION_TYPE_PHYSICS_VEL_SET:
      case EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET:
      case EDJE_ACTION_TYPE_PHYSICS_STOP:
      case EDJE_ACTION_TYPE_PHYSICS_ROT_SET:
#endif
        data_queue_part_lookup(pc, name, &(et->id));
        break;

      default:
        ERR("parse error %s:%i. target may only be used after action",
            file_in, line - 1);
        exit(-1);
     }
   free(name);
}

/**
    @page edcref
    @property
        target
    @parameters
        [target]
    @effect
        Program or part on which the specified action acts. Multiple target
        keywords may be specified, one per target. SIGNAL_EMITs can have
        targets.
    @endproperty
 */
static void
st_collections_group_programs_program_target(void)
{
   check_arg_count(1);

   _program_sequence_check();

   _program_target_add(parse_str(0));
}

/**
    @page edcref
    @property
        targets
    @parameters
        [target1] [target2] [target3] ...
    @effect
        Programs or parts upon which the specified action will act. Multiple
        target or targets keywords may be specified. SIGNAL_EMITs can have
        targets.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_programs_program_targets(void)
{
   int n, argc;

   check_min_arg_count(1);

   _program_sequence_check();

   for (n = 0, argc = get_arg_count(); n < argc; n++)
     _program_target_add(parse_str(n));
}

/**
    @page edcref
    @property
        target_groups
        groups
    @parameters
        [group1] [group2] [group3] ...
    @effect
        Groups of programs or parts upon which the specified action will act. Multiple 'groups', 'target',
        and 'targets' keywords may be specified. SIGNAL_EMITs can have targets.
    @since 1.10
    @endproperty
 */
static void
st_collections_group_programs_program_target_groups(void)
{
   int n, argc;
   Edje_Part_Collection_Parser *pc;

   check_min_arg_count(1);

   pc = eina_list_last_data_get(edje_collections);
   for (n = 0, argc = get_arg_count(); n < argc; n++)
     {
        Eina_List *l;
        Edje_Target_Group *tg;
        char *name;
        Eina_Bool found = EINA_FALSE;

        name = parse_str(n);
        EINA_LIST_FOREACH(pc->target_groups, l, tg)
          if (!strcmp(tg->name, name))
            {
               char **t;

               for (t = tg->targets; *t; t++)
                 _program_target_add(strdup(*t));
               found = EINA_TRUE;
            }
        if (found)
          {
             free(name);
             continue;
          }
        ERR("parse error %s:%i. There is no target_group with the name '%s'",
            file_in, line - 1, name);
        exit(-1);
     }
}

/**
    @page edcref
    @property
        after
    @parameters
        [after]
    @effect
        Specifies a program to run after the current program completes. The
        source and signal parameters of a program run as an "after" are ignored.
        Multiple "after" statements can be specified per program.
    @endproperty
 */
static void
st_collections_group_programs_program_after(void)
{
   char *name;

   check_arg_count(1);
   _program_sequence_check();
   name = parse_str(0);
   _program_after(name);
   free(name);
}

/**
    @page edcref
    @property
        api
    @parameters
        [name] [description]
    @effect
        Specifies a hint to let applications (or IDE's) know how to bind
        things. The parameter name should contain the name of the function that
        the application should use, and description describes how it should
        be used.
    @endproperty
 */
static void
st_collections_group_programs_program_api(void)
{
   check_min_arg_count(1);

   _program_sequence_check();

   current_program->api.name = parse_str(0);

   if (is_param(1))
     {
        check_arg_count(2);
        current_program->api.description = parse_str(1);
     }
}

/** @edcsubsection{collections_group_program_sequence,
 *                 Group.Programs.Program.Sequence} */

/**
    @page edcref
    @block
        sequence
    @context
       ..
          program {
             name: "programname";
             signal: "signalname";
             source: "partname";
             filter: "partname" "statename";
             in: 0.3 0.0;
             action: STATE_SET "statename" state_value;
             transition: LINEAR 0.5;
             targets: "partname" "anotherpart";
             sequence {
                action: SIGNAL_EMIT "signal" "source";
                action: STATE_SET "somestate";
                transition: SINUSOIDAL 0.4;
                targets: "partname" "anotherpart";
                signal: "also,runs"; source: "on,signals";
                name: "runs_after_programname";

                action: STATE_SET "someotherstate";
                transition: DECELERATE 4.0;
                in: 0.2 0.0;
                targets: "partname" "anotherpart";
                name: "runs_after_runs_after_programname";
                after: "coolprogram1337";
                filter: "partname" "goodstate";
             }
          }
       ..
    @description
        Sequences allow quick, easy chaining of programs.
        Each "action" keyword within the sequence will start a new program definition.
        Programs defined in sequences can be used as regular programs in every way,
        but they will automatically set up sequence chains which run after the originating program.
        After the sequence block is closed, the original program can continue to be
        modified.
        Scripts are allowed within sequences: each script block will be treated as a newly
        sequenced program.
        @since 1.10
    @endblock
 */
static void
ob_collections_group_programs_program_sequence(void)
{
   sequencing = current_program;
   sequencing_lookups = current_program_lookups;
   current_program_lookups = NULL;
   ((Edje_Program_Parser *)sequencing)->can_override = EINA_FALSE;
}

static void
st_collections_group_parts_part_api(void)
{
   check_min_arg_count(1);

   current_part->api.name = parse_str(0);
   if (is_param(1))
     {
        check_arg_count(2);
        current_part->api.description = parse_str(1);
     }
}

static void
ob_collections_group_programs_program_script(void)
{
   Edje_Part_Collection *pc;
   Code *cd;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   cd = eina_list_data_get(eina_list_last(codes));

   if (!is_verbatim()) track_verbatim(1);
   else
     {
        Eina_Bool empty = EINA_TRUE;
        char *s;
        int i, len;

        s = get_verbatim();
        if (s)
          {
             Code_Program *cp;

             /* FIXME: Need to store the script somewhere to be restored when using edje_edit API */
             cp = mem_alloc(SZ(Code_Program));
             cp->l1 = get_verbatim_line1();
             cp->l2 = get_verbatim_line2();
             cp->script = s;
             cp->original = strdup(s);
             if (cd->shared && cd->is_lua)
               {
                  ERR("parse error %s:%i. You're trying to mix Embryo and Lua scripting in the same group",
                      file_in, line - 1);
                  exit(-1);
               }
             cd->is_lua = 0;

             len = strlen(cp->script);
             for (i = 0; empty && i < len; i++)
               {
                  if (((cp->script[i] > 'a') && (cp->script[i] < 'z')) ||
                      ((cp->script[i] > 'A') && (cp->script[i] < 'Z')) ||
                      ((cp->script[i] > '0') && (cp->script[i] < '9')))
                    empty = EINA_FALSE;
               }

             if (sequencing)
               _program_sequence_new();
             if (!empty)
               {
                  cd->programs = eina_list_append(cd->programs, cp);
                  data_queue_anonymous_lookup(pc, current_program, &(cp->id));
                  current_program->action = EDJE_ACTION_TYPE_SCRIPT;
               }
             else
               {
                  data_queue_anonymous_lookup(pc, current_program, NULL);
                  free(cp->original);
                  free(cp);
                  cp = NULL;
               }

             set_verbatim(NULL, 0, 0);
          }
     }
}

/** @edcsubsection{collections_group_physics,
 *                 Group.Physics} */

/**
    @page edcref
    @block
        physics
    @context
       group {
           ..
           physics {
               world {
                   ..
               }
           }
           ..
       }
    @description
        The "physics" block consists of blocks related to physics but
        not to configure a body. By now, it only has a "world" block.
    @endblock
 */

/** @edcsubsection{collections_group_physics_world,
 *                Group.Physics.World} */

/**
    @page edcref
    @block
        world
    @context
        physics {
            world {
               gravity: 0 294 0;
               rate: 30;
               z: -50;
               depth: 100;
            }
        }
    @description
        The "world" block configures the physics world. It's the
        environment where the part's bodies will be simulated.
        It can be used to customize gravity, rate, depth and others.
    @endblock

    @property
        gravity
    @parameters
        [x-axis] [y-axis] [z-axis]
    @effect
        Three double values defining the gravity vector.
        Each one is the component of this same vector over each axis.
        Its unit is Evas Coordinates per second ^ 2.
        The default value is 0, 294, 0, since we've a default rate of
        30 pixels.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_physics_world_gravity(void)
{
   Edje_Part_Collection *pc;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   check_arg_count(3);

   pc->physics.world.gravity.x = parse_int(0);
   pc->physics.world.gravity.y = parse_int(1);
   pc->physics.world.gravity.z = parse_int(2);
}

#endif

/**
    @page edcref
    @property
        rate
    @parameters
        [rate pixels / meter]
    @effect
        Set rate between pixels on evas canvas and meters on physics world.
        It will be used by automatic updates of evas objects associated to
        physics bodies.
        By default rate is 30 pixels per meter.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_physics_world_rate(void)
{
   Edje_Part_Collection *pc;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   check_arg_count(1);

   pc->physics.world.rate = parse_float(0);
}

#endif

/**
    @page edcref
    @property
        depth
    @parameters
        [depth in pixels]
    @effect
        World's depth, in pixels. It's only relevant if boundaries are used,
        since their size depends on this.
        By default world's depth is 100 pixels.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_physics_world_depth(void)
{
   Edje_Part_Collection *pc;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   check_arg_count(1);

   pc->physics.world.depth = parse_int(0);
}

#endif

/**
    @page edcref
    @property
        z
    @parameters
        [world's front border position]
    @effect
        Position in z axis, in pixels.
        It's only relevant if boundaries are used, since their position
        depends on this.
        By default world's z is -50 pixels.
    @since 1.8
    @endproperty
 */
#ifdef HAVE_EPHYSICS
static void
st_collections_group_physics_world_z(void)
{
   Edje_Part_Collection *pc;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   check_arg_count(1);

   pc->physics.world.z = parse_int(0);
}

#endif

/**
    @page edcref
    </table>
 */

void
edje_cc_handlers_pop_notify(const char *token)
{
   if (sequencing && (!strcmp(token, "sequence")))
     {
        current_program = sequencing;
        ((Edje_Program_Parser *)sequencing)->can_override = EINA_TRUE;
        eina_list_free(current_program_lookups);
        current_program_lookups = sequencing_lookups;
        sequencing_lookups = NULL;
        sequencing = NULL;
     }
   else if (current_program && (!strcmp(token, "link")))
     current_program = NULL;
   else if (current_de && (!strcmp(token, "group")))
     {
        _link_combine();
        _script_flush();
     }
   else if (current_desc && (!strcmp(token, "description")))
     free_anchors();
}

static void
edje_cc_handlers_hierarchy_set(Edje_Part *src)
{  /* This funcion makes current part rel_1.id, rel_2.id relative to src */
  if (!src->name)
    {
       ERR("parse error %s:%i. You must set parent name before creating nested part",
           file_in, line - 1);
       exit(-1);
    }
  st_collections_group_parts_part_description_rel1_to_set(src->name);
  st_collections_group_parts_part_description_rel2_to_set(src->name);
}

static Edje_Part *
edje_cc_handlers_hierarchy_parent_get(void)
{  /* Return the parent part pointer */
  int idx = eina_array_count(part_hierarchy) - 2;
  Edje_Cc_Handlers_Hierarchy_Info *info = (idx >= 0) ?
    eina_array_data_get(part_hierarchy, idx) : NULL;

  return (info) ? info->ep : NULL;
}

static void
edje_cc_handlers_hierarchy_push(Edje_Part *ep, Edje_Part *cp)
{  /* Remove part from hierarchy stack when finished parsing it */
  Edje_Cc_Handlers_Hierarchy_Info *info = malloc(sizeof(*info));
  info->current_de = current_de;
  info->current_part = cp;   /* current_part restored on pop */
  info->current_item = current_item;
  info->current_desc = current_desc;
  info->parent_desc = parent_desc;
  info->current_program = current_program;
  info->ep = ep;

  eina_array_push(part_hierarchy, info);
}

static void
edje_cc_handlers_hierarchy_rename(Edje_Part *old, Edje_Part *new)
{
   Edje_Cc_Handlers_Hierarchy_Info *item;
   Eina_Array_Iterator iterator;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(part_hierarchy, i, item, iterator)
   {
      if (item->ep == old) item->ep = new;
      if (item->current_part == old) item->current_part = new;
   }
}

void
edje_cc_handlers_hierarchy_alloc(void)
{
   part_hierarchy = eina_array_new(8);
}

void
edje_cc_handlers_hierarchy_free(void)
{
   eina_array_free(part_hierarchy);
   part_hierarchy = NULL;
}

static void
edje_cc_handlers_hierarchy_pop(void)
{  /* Remove part from hierarchy stack when finished parsing it */
  Edje_Cc_Handlers_Hierarchy_Info *info = eina_array_pop(part_hierarchy);

  if (current_part)
    {
       unsigned int i;

       if (!current_part->name)
         {
            WRN("Parse error near %s:%i. Unnamed part exists in Group \"%s\".",
                file_in, line - 1, current_de->entry);
         }

       for (i = 0; i < current_part->other.desc_count; i++)
         {
            if (!current_part->other.desc[i]->state.name)
              {
                 ERR("syntax error near %s:%i. Non-default or inherited parts are required to have state names for all descriptions (Group '%s', part '%s' has missing description state names)",
                     file_in, line - 1, current_de->entry, current_part->name);
                 exit(-1);
              }
         }

       /* auto-add default desc if it was omitted */
       if (!current_part->default_desc)
         ob_collections_group_parts_part_description();
    }

  if (info)
    {
       current_de = info->current_de;
       current_part = info->current_part;
       current_item = info->current_item;
       current_desc = info->current_desc;
       parent_desc = info->parent_desc;
       current_program = info->current_program;

       free(info);
    }
}

Eina_Bool
edje_cc_handlers_wildcard(void)
{
   char *token, *last;

   token = eina_list_last_data_get(stack);
   last = eina_list_data_get(eina_list_prev(eina_list_last(stack)));
   if (!last) return EINA_FALSE;
   if (last)
     {
        char *end;

        end = strrchr(last, '.');
        if (end) last = end + 1;
     }
   if (current_part)
     {
        if ((!strcmp(last, "part")) && (!current_part->name))
          {
             Eina_Bool ret;

             if (!had_quote) return EINA_FALSE;
             current_part->name = token;
             ret = _part_name_check();
             if (ret)
               stack_pop_quick(EINA_FALSE, EINA_FALSE);
             return ret;
          }
        if (current_desc && ((!strcmp(last, "desc")) || (!strcmp(last, "description"))))
          {
             double st;
             char *end;

             if (had_quote)
               {
                  if ((!current_desc->state.name) || strcmp(current_desc->state.name, token))
                    {
                       free((char *)current_desc->state.name);
                       current_desc->state.name = token;
                       _part_description_state_update(current_desc);
                    }
                  stack_pop_quick(EINA_FALSE, current_desc->state.name != token);
                  return EINA_TRUE;
               }

             if (!isdigit(token[0])) return EINA_FALSE;
             st = strtod(token, &end);
             if ((end && end[0]) || (fabs(current_desc->state.value) > DBL_EPSILON))
               return EINA_FALSE;
             if (current_desc == current_part->default_desc)
               {
                  ob_collections_group_parts_part_description();
                  current_desc->state.name = strdup("default");
               }
             else
               {
                  unsigned int j;

                  for (j = 0; j < current_part->other.desc_count; j++)
                    {
                       /* check for inherited descriptions */
                       Edje_Part_Description_Common *ed = current_part->other.desc[j];

                       if (((!!ed->state.name) != (!!current_desc->state.name)) ||
                           (ed->state.name && strcmp(ed->state.name, current_desc->state.name)) ||
                           (fabs(ed->state.value - st) > DBL_EPSILON)) continue;
                       current_desc = ed;
                       break;
                    }
                  /* not found */
                  if (j == current_part->other.desc_count)
                    {
                       void *name = NULL;

                       if (current_desc->state.name)
                         name = strdup(current_desc->state.name);
                       ob_collections_group_parts_part_description();
                       current_desc->state.name = name;
                    }
               }
             current_desc->state.value = st;
             stack_pop_quick(EINA_FALSE, EINA_TRUE);
             return EINA_TRUE;
          }
     }
   if (current_program && ((!strcmp(last, "program")) || (!strcmp(last, "sequence"))))
     {
        if (!had_quote) return EINA_FALSE;
        _program_sequence_check();
        _program_name(token);
        stack_pop_quick(EINA_FALSE, EINA_FALSE);
        return EINA_TRUE;
     }
   if (current_de && (!strcmp(last, "group")))
     {
        if (!had_quote) return EINA_FALSE;
        _group_name(token);
        stack_pop_quick(EINA_FALSE, EINA_FALSE);
        return EINA_TRUE;
     }
   if (edje_file->styles && (!strcmp(last, "style")))
     {
        if (!had_quote) return EINA_FALSE;
        _style_name(token);
        stack_pop_quick(EINA_FALSE, EINA_FALSE);
        return EINA_TRUE;
     }
   if (edje_file->color_classes && (!strcmp(last, "color_class")))
     {
        if (!had_quote) return EINA_FALSE;
        _color_class_name(token);
        stack_pop_quick(EINA_FALSE, EINA_FALSE);
        return EINA_TRUE;
     }
   if (edje_file->text_classes && (!strcmp(last, "text_class")))
     {
        if (!had_quote) return EINA_FALSE;
        _text_class_name(token);
        stack_pop_quick(EINA_FALSE, EINA_FALSE);
        return EINA_TRUE;
     }
   if (edje_file->size_classes && (!strcmp(last, "size_class")))
     {
        if (!had_quote) return EINA_FALSE;
        _size_class_name(token);
        stack_pop_quick(EINA_FALSE, EINA_FALSE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

