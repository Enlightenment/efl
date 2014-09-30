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
 *    </ul>
 *    <li>@ref sec_collections "Collections"</li>
 *    <ul>
 *      <li>@ref sec_collections_sounds "Sounds"</li>
 *      <ul>
 *        <li>@ref sec_collections_sounds_sample "Sample"</li>
 *      </ul>
 *      <li>@ref sec_collections_vibrations "Vibrations"</li>
 *      <ul>
 *        <li>@ref sec_collections_vibrations_sample "Sample"</li>
 *      </ul>
 *      <li>@ref sec_collections_group "Group"</li>
 *      <ul>
 *        <li>@ref sec_collections_group_script "Script"</li>
 *        <li>@ref sec_collections_group_limits "Limits"</li>
 *        <li>@ref sec_toplevel_data "Data"</li>
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
 *              <ul>
 *                <li>@ref sec_collections_group_parts_description_image_fill "Fill"</li>
 *                <ul>
 *                  <li>@ref sec_collections_group_parts_description_image_fill_origin "Origin"</li>
 *                  <li>@ref sec_collections_group_parts_description_image_fill_size "Size"</li>
 *                </ul>
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
 *            </ul>
 *          </ul>
 *        </ul>
 *        <li>@ref sec_collections_group_programs "Programs"</li>
 *        <ul>
 *          <li>@ref sec_collections_group_script "Script"</li>
 *          <li>@ref sec_collections_group_program_sequence "Sequence"</li>
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
static Edje_Program *sequencing = NULL;
static Eina_List *sequencing_lookups = NULL;

struct _Edje_Cc_Handlers_Hierarchy_Info
{  /* Struct that keeps globals value to impl hierarchy */
   Edje_Part_Collection_Directory_Entry *current_de;
   Edje_Part *current_part;
   Edje_Pack_Element *current_item;
   Edje_Part_Description_Common *current_desc;
   Edje_Part_Description_Common *parent_desc;
   Edje_Program *current_program;
   Edje_Part *ep;
};
typedef struct _Edje_Cc_Handlers_Hierarchy_Info Edje_Cc_Handlers_Hierarchy_Info;

static Eina_Array *part_hierarchy = NULL; /* stack parts,support nested parts */
static void edje_cc_handlers_hierarchy_set(Edje_Part *src);
static Edje_Part *edje_cc_handlers_hierarchy_parent_get(void);
static void edje_cc_handlers_hierarchy_push(Edje_Part *ep, Edje_Part *cp);
static void edje_cc_handlers_hierarchy_rename(Edje_Part *old, Edje_Part *new);
static void edje_cc_handlers_hierarchy_pop(void);

static void _program_target_add(char *name);
static void _program_after(const char *name);
static void _program_free(Edje_Program *pr);

static void st_externals_external(void);

static void st_images_image(void);
static void ob_images_set(void);
static void st_images_set_name(void);
static void ob_images_set_image(void);
static void st_images_set_image_image(void);
static void st_images_set_image_size(void);
static void st_images_set_image_border(void);
static void st_images_set_image_border_scale_by(void);

static void st_fonts_font(void);

static void st_data_item(void);
static void st_data_file(void);

static void ob_styles_style(void);
static void st_styles_style_name(void);
static void st_styles_style_base(void);
static void st_styles_style_tag(void);

static void ob_color_class(void);
static void st_color_class_name(void);
static void st_color_class_color(void);
static void st_color_class_color2(void);
static void st_color_class_color3(void);

static void ob_collections(void);
static void st_collections_base_scale(void);

static void ob_collections_group(void);
static void st_collections_group_name(void);
static void st_collections_group_inherit_only(void);
static void st_collections_group_inherit(void);
static void st_collections_group_program_source(void);
static void st_collections_group_part_remove(void);
static void st_collections_group_program_remove(void);
static void st_collections_group_script_only(void);
static void st_collections_group_script_recursion(void);
static void st_collections_group_alias(void);
static void st_collections_group_min(void);
static void st_collections_group_max(void);
static void st_collections_group_broadcast_signal(void);
static void st_collections_group_data_item(void);
static void st_collections_group_orientation(void);
static void st_collections_group_mouse_events(void);

static void st_collections_group_limits_vertical(void);
static void st_collections_group_limits_horizontal(void);

static void ob_collections_group_script(void);
static void ob_collections_group_lua_script(void);

static void st_collections_group_parts_alias(void);

static Edje_Part *edje_cc_handlers_part_make(int);
static void ob_collections_group_parts_part(void);
static void st_collections_group_parts_part_name(void);
static void st_collections_group_parts_part_inherit(void);
static void st_collections_group_parts_part_type(void);
#ifdef HAVE_EPHYSICS
static void st_collections_group_parts_part_physics_body(void);
#endif
static void st_collections_group_parts_part_insert_before(void);
static void st_collections_group_parts_part_insert_after(void);
static void st_collections_group_parts_part_effect(void);
static void st_collections_group_parts_part_mouse_events(void);
static void st_collections_group_parts_part_repeat_events(void);
static void st_collections_group_parts_part_ignore_flags(void);
static void st_collections_group_parts_part_scale(void);
static void st_collections_group_parts_part_pointer_mode(void);
static void st_collections_group_parts_part_precise_is_inside(void);
static void st_collections_group_parts_part_use_alternate_font_metrics(void);
static void st_collections_group_parts_part_clip_to_id(void);
static void st_collections_group_parts_part_source(void);
static void st_collections_group_parts_part_source2(void);
static void st_collections_group_parts_part_source3(void);
static void st_collections_group_parts_part_source4(void);
static void st_collections_group_parts_part_source5(void);
static void st_collections_group_parts_part_source6(void);
static void st_collections_group_parts_part_entry_mode(void);
static void st_collections_group_parts_part_select_mode(void);
static void st_collections_group_parts_part_cursor_mode(void);
static void st_collections_group_parts_part_multiline(void);
static void st_collections_group_parts_part_access(void);
static void st_collections_group_parts_part_dragable_x(void);
static void st_collections_group_parts_part_dragable_y(void);
static void st_collections_group_parts_part_dragable_confine(void);
static void st_collections_group_parts_part_dragable_threshold(void);
static void st_collections_group_parts_part_dragable_events(void);

/* box and table items share these */
static void ob_collections_group_parts_part_box_items_item(void);
static void st_collections_group_parts_part_box_items_item_type(void);
static void st_collections_group_parts_part_box_items_item_name(void);
static void st_collections_group_parts_part_box_items_item_source(void);
static void st_collections_group_parts_part_box_items_item_min(void);
static void st_collections_group_parts_part_box_items_item_spread(void);
static void st_collections_group_parts_part_box_items_item_prefer(void);
static void st_collections_group_parts_part_box_items_item_max(void);
static void st_collections_group_parts_part_box_items_item_padding(void);
static void st_collections_group_parts_part_box_items_item_align(void);
static void st_collections_group_parts_part_box_items_item_weight(void);
static void st_collections_group_parts_part_box_items_item_aspect(void);
static void st_collections_group_parts_part_box_items_item_aspect_mode(void);
static void st_collections_group_parts_part_box_items_item_options(void);
/* but these are only for table */
static void st_collections_group_parts_part_table_items_item_position(void);
static void st_collections_group_parts_part_table_items_item_span(void);

static void ob_collections_group_parts_part_description(void);
static void ob_collections_group_parts_part_desc(void);
static void st_collections_group_parts_part_description_inherit(void);
static void ob_collections_group_parts_part_description_link(void);
static void st_collections_group_parts_part_description_link_base(void);
static void st_collections_group_parts_part_description_source(void);
static void st_collections_group_parts_part_description_state(void);
static void st_collections_group_parts_part_description_visible(void);
static void st_collections_group_parts_part_description_limit(void);
static void st_collections_group_parts_part_description_align(void);
static void st_collections_group_parts_part_description_fixed(void);
static void st_collections_group_parts_part_description_min(void);
static void st_collections_group_parts_part_description_minmul(void);
static void st_collections_group_parts_part_description_max(void);
static void st_collections_group_parts_part_description_step(void);
static void st_collections_group_parts_part_description_aspect(void);
static void st_collections_group_parts_part_description_aspect_preference(void);
static void st_collections_group_parts_part_description_rel1_relative(void);
static void st_collections_group_parts_part_description_rel1_offset(void);
static void st_collections_group_parts_part_description_rel1_to_set(const char *name);
static void st_collections_group_parts_part_description_rel1_to(void);
static void st_collections_group_parts_part_description_rel1_to_x(void);
static void st_collections_group_parts_part_description_rel1_to_y(void);
static void st_collections_group_parts_part_description_rel2_relative(void);
static void st_collections_group_parts_part_description_rel2_offset(void);
static void st_collections_group_parts_part_description_rel2_to_set(const char *name);
static void st_collections_group_parts_part_description_rel2_to(void);
static void st_collections_group_parts_part_description_rel2_to_x(void);
static void st_collections_group_parts_part_description_rel2_to_y(void);
static void st_collections_group_parts_part_description_image_normal(void);
static void st_collections_group_parts_part_description_image_tween(void);
static void st_collections_group_parts_part_description_image_border(void);
static void st_collections_group_parts_part_description_image_middle(void);
static void st_collections_group_parts_part_description_image_border_scale(void);
static void st_collections_group_parts_part_description_image_border_scale_by(void);
static void st_collections_group_parts_part_description_image_scale_hint(void);
static void st_collections_group_parts_part_description_fill_smooth(void);
static void st_collections_group_parts_part_description_fill_origin_relative(void);
static void st_collections_group_parts_part_description_fill_origin_offset(void);
static void st_collections_group_parts_part_description_fill_size_relative(void);
static void st_collections_group_parts_part_description_fill_size_offset(void);
static void st_collections_group_parts_part_description_fill_spread(void);
static void st_collections_group_parts_part_description_fill_type(void);
static void st_collections_group_parts_part_description_color_class(void);
static void st_collections_group_parts_part_description_color(void);
static void st_collections_group_parts_part_description_color2(void);
static void st_collections_group_parts_part_description_color3(void);
static void st_collections_group_parts_part_description_text_text(void);
static void st_collections_group_parts_part_description_text_text_class(void);
static void st_collections_group_parts_part_description_text_font(void);
static void st_collections_group_parts_part_description_text_style(void);
static void st_collections_group_parts_part_description_text_repch(void);
static void st_collections_group_parts_part_description_text_size(void);
static void st_collections_group_parts_part_description_text_size_range(void);
static void st_collections_group_parts_part_description_text_fit(void);
static void st_collections_group_parts_part_description_text_min(void);
static void st_collections_group_parts_part_description_text_max(void);
static void st_collections_group_parts_part_description_text_align(void);
static void st_collections_group_parts_part_description_text_source(void);
static void st_collections_group_parts_part_description_text_text_source(void);
static void st_collections_group_parts_part_description_text_ellipsis(void);
static void st_collections_group_parts_part_description_text_filter(void);
static void st_collections_group_parts_part_description_box_layout(void);
static void st_collections_group_parts_part_description_box_align(void);
static void st_collections_group_parts_part_description_box_padding(void);
static void st_collections_group_parts_part_description_box_min(void);
static void st_collections_group_parts_part_description_table_homogeneous(void);
static void st_collections_group_parts_part_description_table_align(void);
static void st_collections_group_parts_part_description_table_padding(void);
static void st_collections_group_parts_part_description_table_min(void);
static void st_collections_group_parts_part_description_proxy_source_visible(void);
static void st_collections_group_parts_part_description_proxy_source_clip(void);

#ifdef HAVE_EPHYSICS
static void st_collections_group_parts_part_description_physics_mass(void);
static void st_collections_group_parts_part_description_physics_restitution(void);
static void st_collections_group_parts_part_description_physics_friction(void);
static void st_collections_group_parts_part_description_physics_damping(void);
static void st_collections_group_parts_part_description_physics_sleep(void);
static void st_collections_group_parts_part_description_physics_material(void);
static void st_collections_group_parts_part_description_physics_density(void);
static void st_collections_group_parts_part_description_physics_hardness(void);
static void st_collections_group_parts_part_description_physics_ignore_part_pos(void);
static void st_collections_group_parts_part_description_physics_light_on(void);
static void st_collections_group_parts_part_description_physics_z(void);
static void st_collections_group_parts_part_description_physics_depth(void);
static void st_collections_group_parts_part_description_physics_movement_freedom_linear(void);
static void st_collections_group_parts_part_description_physics_movement_freedom_angular(void);
static void st_collections_group_parts_part_description_physics_backface_cull(void);
static void st_collections_group_parts_part_description_physics_face(void);
static void st_collections_group_parts_part_description_physics_face_type(void);
static void st_collections_group_parts_part_description_physics_face_source(void);
#endif
static void st_collections_group_parts_part_description_map_perspective(void);
static void st_collections_group_parts_part_description_map_light(void);
static void st_collections_group_parts_part_description_map_rotation_center(void);
static void st_collections_group_parts_part_description_map_rotation_x(void);
static void st_collections_group_parts_part_description_map_rotation_y(void);
static void st_collections_group_parts_part_description_map_rotation_z(void);
static void st_collections_group_parts_part_description_map_on(void);
static void st_collections_group_parts_part_description_map_smooth(void);
static void st_collections_group_parts_part_description_map_alpha(void);
static void st_collections_group_parts_part_description_map_backface_cull(void);
static void st_collections_group_parts_part_description_map_perspective_on(void);
static void st_collections_group_parts_part_description_map_color(void);
static void st_collections_group_parts_part_description_perspective_zplane(void);
static void st_collections_group_parts_part_description_perspective_focal(void);
static void st_collections_group_parts_part_api(void);

/* external part parameters */
static void st_collections_group_parts_part_description_params_int(void);
static void ob_collections_group_programs_program(void);
static void st_collections_group_parts_part_description_params_double(void);

static void st_collections_group_programs_program_name(void);
static void st_collections_group_parts_part_description_params_string(void);
static void st_collections_group_parts_part_description_params_bool(void);
static void st_collections_group_parts_part_description_params_choice(void);
static void st_collections_group_programs_program_signal(void);
static void st_collections_group_programs_program_source(void);
static void st_collections_group_programs_program_filter(void);
static void st_collections_group_programs_program_in(void);
static void st_collections_group_programs_program_action(void);
static void st_collections_group_programs_program_transition(void);
static void st_collections_group_programs_program_target(void);
static void st_collections_group_programs_program_targets(void);
static void st_collections_group_programs_program_target_groups(void);
static void st_collections_group_programs_program_after(void);
static void st_collections_group_programs_program_api(void);
static void st_collections_group_target_group(void);

static void ob_collections_group_programs_program_sequence(void);

static void ob_collections_group_programs_program_script(void);
static void st_collections_group_sound_sample_name(void);
static void st_collections_group_sound_sample_source(void);
static void st_collections_group_sound_tone(void);
static void st_collections_group_vibration_sample_name(void);
static void st_collections_group_vibration_sample_source(void);

#ifdef HAVE_EPHYSICS
static void st_collections_group_physics_world_gravity(void);
static void st_collections_group_physics_world_rate(void);
static void st_collections_group_physics_world_z(void);
static void st_collections_group_physics_world_depth(void);
#endif

/* short */
static void st_collections_group_parts_part_noscale(void);
static void st_collections_group_parts_part_precise(void);
static void st_collections_group_parts_part_noprecise(void);
static void st_collections_group_parts_part_mouse(void);
static void st_collections_group_parts_part_nomouse(void);
static void st_collections_group_parts_part_repeat(void);
static void st_collections_group_parts_part_norepeat(void);
static void st_collections_group_parts_part_description_vis(void);
static void st_collections_group_parts_part_description_hid(void);
static void ob_collections_group_parts_part_short(void);

static void st_collections_group_mouse(void);
static void st_collections_group_nomouse(void);
static void st_collections_group_broadcast(void);
static void st_collections_group_nobroadcast(void);
/*****/


#define IMAGE_STATEMENTS(PREFIX) \
     {PREFIX"images.image", st_images_image}, \
     {PREFIX"images.set.name", st_images_set_name}, \
     {PREFIX"images.set.image.image", st_images_set_image_image}, \
     {PREFIX"images.set.image.size", st_images_set_image_size}, \
     {PREFIX"images.set.image.border", st_images_set_image_border}, \
     {PREFIX"images.set.image.scale_by", st_images_set_image_border_scale_by},

#define IMAGE_SET_STATEMENTS(PREFIX) \
     {PREFIX".image", st_images_image}, /* dup */ \
     {PREFIX".set.name", st_images_set_name}, /* dup */ \
     {PREFIX".set.image.image", st_images_set_image_image}, /* dup */ \
     {PREFIX".set.image.size", st_images_set_image_size}, /* dup */ \
     {PREFIX".set.image.border", st_images_set_image_border}, /* dup */ \
     {PREFIX".set.image.scale_by", st_images_set_image_border_scale_by}, /* dup */

#define FONT_STYLE_CC_STATEMENTS(PREFIX) \
     {PREFIX"fonts.font", st_fonts_font}, /* dup */ \
     {PREFIX"styles.style.name", st_styles_style_name}, /* dup */ \
     {PREFIX"styles.style.base", st_styles_style_base}, /* dup */ \
     {PREFIX"styles.style.tag", st_styles_style_tag}, /* dup */ \
     {PREFIX"color_classes.color_class.name", st_color_class_name}, /* dup */ \
     {PREFIX"color_classes.color_class.color", st_color_class_color}, /* dup */ \
     {PREFIX"color_classes.color_class.color2", st_color_class_color2}, /* dup */ \
     {PREFIX"color_classes.color_class.color3", st_color_class_color3}, /* dup */

#define PROGRAM_SEQUENCE(PREFIX, NAME, FN) \
     {PREFIX".program."NAME, FN}, /* dup */ \
     {PREFIX".program.sequence."NAME, FN}, /* dup */


#define PROGRAM_BASE(PREFIX) \
     PROGRAM_SEQUENCE(PREFIX, "name", st_collections_group_programs_program_name) \
     PROGRAM_SEQUENCE(PREFIX, "signal", st_collections_group_programs_program_signal) \
     PROGRAM_SEQUENCE(PREFIX, "source", st_collections_group_programs_program_source) \
     PROGRAM_SEQUENCE(PREFIX, "in", st_collections_group_programs_program_in) \
     PROGRAM_SEQUENCE(PREFIX, "action", st_collections_group_programs_program_action) \
     PROGRAM_SEQUENCE(PREFIX, "transition", st_collections_group_programs_program_transition) \
     PROGRAM_SEQUENCE(PREFIX, "target", st_collections_group_programs_program_target) \
     PROGRAM_SEQUENCE(PREFIX, "target_groups", st_collections_group_programs_program_target_groups) \
     PROGRAM_SEQUENCE(PREFIX, "groups", st_collections_group_programs_program_target_groups) \
     PROGRAM_SEQUENCE(PREFIX, "targets", st_collections_group_programs_program_targets) \
     PROGRAM_SEQUENCE(PREFIX, "after", st_collections_group_programs_program_after) \
     PROGRAM_SEQUENCE(PREFIX, "api", st_collections_group_programs_program_api) \
     PROGRAM_SEQUENCE(PREFIX, "filter", st_collections_group_programs_program_filter)

#define PROGRAM_STATEMENTS(PREFIX) \
     IMAGE_SET_STATEMENTS(PREFIX".programs") \
     IMAGE_STATEMENTS(PREFIX".programs.") \
     IMAGE_SET_STATEMENTS(PREFIX".programs") \
     {PREFIX".programs.font", st_fonts_font}, /* dup */ \
     {PREFIX".programs.fonts.font", st_fonts_font}, /* dup */ \
     PROGRAM_BASE(PREFIX) \
     PROGRAM_BASE(PREFIX".programs")


New_Statement_Handler statement_handlers[] =
{
     {"externals.external", st_externals_external},
     IMAGE_STATEMENTS("")
     FONT_STYLE_CC_STATEMENTS("")
     {"data.item", st_data_item},
     {"data.file", st_data_file},
     {"collections.externals.external", st_externals_external}, /* dup */
     IMAGE_STATEMENTS("collections.")
     IMAGE_SET_STATEMENTS("collections")
     {"collections.font", st_fonts_font}, /* dup */
     FONT_STYLE_CC_STATEMENTS("collections.")
     {"collections.base_scale", st_collections_base_scale},

     {"collections.sounds.sample.name", st_collections_group_sound_sample_name},
     {"collections.sounds.sample.source", st_collections_group_sound_sample_source},
     {"collections.group.sounds.sample.name", st_collections_group_sound_sample_name}, /* dup */
     {"collections.group.sounds.sample.source", st_collections_group_sound_sample_source}, /* dup */
     {"collections.sounds.tone", st_collections_group_sound_tone},
     {"collections.group.sounds.tone", st_collections_group_sound_tone}, /* dup */
     {"collections.vibrations.sample.name", st_collections_group_vibration_sample_name},
     {"collections.vibrations.sample.source", st_collections_group_vibration_sample_source},
     {"collections.group.vibrations.sample.name", st_collections_group_vibration_sample_name}, /* dup */
     {"collections.group.vibrations.sample.source", st_collections_group_vibration_sample_source}, /* dup */
     {"collections.group.name", st_collections_group_name},
     {"collections.group.program_source", st_collections_group_program_source},
     {"collections.group.inherit", st_collections_group_inherit},
     {"collections.group.inherit_only", st_collections_group_inherit_only},
     {"collections.group.target_group", st_collections_group_target_group}, /* dup */
     {"collections.group.part_remove", st_collections_group_part_remove},
     {"collections.group.program_remove", st_collections_group_program_remove},
     {"collections.group.script_only", st_collections_group_script_only},
     {"collections.group.script_recursion", st_collections_group_script_recursion},
     {"collections.group.lua_script_only", st_collections_group_script_only},
     {"collections.group.alias", st_collections_group_alias},
     {"collections.group.min", st_collections_group_min},
     {"collections.group.max", st_collections_group_max},
     {"collections.group.broadcast_signal", st_collections_group_broadcast_signal},
     {"collections.group.orientation", st_collections_group_orientation},
     {"collections.group.mouse_events", st_collections_group_mouse_events},
     {"collections.group.data.item", st_collections_group_data_item},
     {"collections.group.limits.horizontal", st_collections_group_limits_horizontal},
     {"collections.group.limits.vertical", st_collections_group_limits_vertical},
     {"collections.group.externals.external", st_externals_external}, /* dup */
     {"collections.group.programs.target_group", st_collections_group_target_group}, /* dup */
     IMAGE_SET_STATEMENTS("collections.group")
     IMAGE_STATEMENTS("collections.group.")
     {"collections.group.font", st_fonts_font}, /* dup */
     FONT_STYLE_CC_STATEMENTS("collections.group.")
     {"collections.group.parts.alias", st_collections_group_parts_alias },
     IMAGE_SET_STATEMENTS("collections.group.parts")
     IMAGE_STATEMENTS("collections.group.parts.")
     {"collections.group.parts.font", st_fonts_font}, /* dup */
     FONT_STYLE_CC_STATEMENTS("collections.group.parts.")
     {"collections.group.parts.target_group", st_collections_group_target_group}, /* dup */
     {"collections.group.parts.part.name", st_collections_group_parts_part_name},
     {"collections.group.parts.part.target_group", st_collections_group_target_group}, /* dup */
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
     {"collections.group.parts.part.repeat_events", st_collections_group_parts_part_repeat_events},
     {"collections.group.parts.part.ignore_flags", st_collections_group_parts_part_ignore_flags},
     {"collections.group.parts.part.scale", st_collections_group_parts_part_scale},
     {"collections.group.parts.part.pointer_mode", st_collections_group_parts_part_pointer_mode},
     {"collections.group.parts.part.precise_is_inside", st_collections_group_parts_part_precise_is_inside},
     {"collections.group.parts.part.use_alternate_font_metrics", st_collections_group_parts_part_use_alternate_font_metrics},
     {"collections.group.parts.part.clip_to", st_collections_group_parts_part_clip_to_id},
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
     IMAGE_SET_STATEMENTS("collections.group.parts.part")
     IMAGE_STATEMENTS("collections.group.parts.part.")
     {"collections.group.parts.part.font", st_fonts_font}, /* dup */
     FONT_STYLE_CC_STATEMENTS("collections.group.parts.part.")
     {"collections.group.parts.part.box.items.item.type", st_collections_group_parts_part_box_items_item_type},
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
     {"collections.group.parts.part.table.items.item.type", st_collections_group_parts_part_box_items_item_type}, /* dup */
     {"collections.group.parts.part.table.items.item.name", st_collections_group_parts_part_box_items_item_name}, /* dup */
     {"collections.group.parts.part.table.items.item.source", st_collections_group_parts_part_box_items_item_source}, /* dup */
     {"collections.group.parts.part.table.items.item.min", st_collections_group_parts_part_box_items_item_min}, /* dup */
     {"collections.group.parts.part.table.items.item.spread", st_collections_group_parts_part_box_items_item_spread}, /* dup */
     {"collections.group.parts.part.table.items.item.prefer", st_collections_group_parts_part_box_items_item_prefer}, /* dup */
     {"collections.group.parts.part.table.items.item.max", st_collections_group_parts_part_box_items_item_max}, /* dup */
     {"collections.group.parts.part.table.items.item.padding", st_collections_group_parts_part_box_items_item_padding}, /* dup */
     {"collections.group.parts.part.table.items.item.align", st_collections_group_parts_part_box_items_item_align}, /* dup */
     {"collections.group.parts.part.table.items.item.weight", st_collections_group_parts_part_box_items_item_weight}, /* dup */
     {"collections.group.parts.part.table.items.item.aspect", st_collections_group_parts_part_box_items_item_aspect}, /* dup */
     {"collections.group.parts.part.table.items.item.aspect_mode", st_collections_group_parts_part_box_items_item_aspect_mode}, /* dup */
     {"collections.group.parts.part.table.items.item.options", st_collections_group_parts_part_box_items_item_options}, /* dup */
     {"collections.group.parts.part.table.items.item.position", st_collections_group_parts_part_table_items_item_position},
     {"collections.group.parts.part.table.items.item.span", st_collections_group_parts_part_table_items_item_span},
     {"collections.group.parts.part.description.target_group", st_collections_group_target_group}, /* dup */
     {"collections.group.parts.part.description.inherit", st_collections_group_parts_part_description_inherit},
     {"collections.group.parts.part.description.link.base", st_collections_group_parts_part_description_link_base},
     {"collections.group.parts.part.description.link.transition", st_collections_group_programs_program_transition},
     {"collections.group.parts.part.description.link.after", st_collections_group_programs_program_after},
     {"collections.group.parts.part.description.link.in", st_collections_group_programs_program_in},
     {"collections.group.parts.part.description.source", st_collections_group_parts_part_description_source},
     {"collections.group.parts.part.description.state", st_collections_group_parts_part_description_state},
     {"collections.group.parts.part.description.visible", st_collections_group_parts_part_description_visible},
     {"collections.group.parts.part.description.limit", st_collections_group_parts_part_description_limit},
     {"collections.group.parts.part.description.align", st_collections_group_parts_part_description_align},
     {"collections.group.parts.part.description.fixed", st_collections_group_parts_part_description_fixed},
     {"collections.group.parts.part.description.min", st_collections_group_parts_part_description_min},
     {"collections.group.parts.part.description.minmul", st_collections_group_parts_part_description_minmul},
     {"collections.group.parts.part.description.max", st_collections_group_parts_part_description_max},
     {"collections.group.parts.part.description.step", st_collections_group_parts_part_description_step},
     {"collections.group.parts.part.description.aspect", st_collections_group_parts_part_description_aspect},
     {"collections.group.parts.part.description.aspect_preference", st_collections_group_parts_part_description_aspect_preference},
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
     {"collections.group.parts.part.description.image.normal", st_collections_group_parts_part_description_image_normal},
     {"collections.group.parts.part.description.image.tween", st_collections_group_parts_part_description_image_tween},
     IMAGE_SET_STATEMENTS("collections.group.parts.part.description.image")
     IMAGE_STATEMENTS("collections.group.parts.part.description.image.")
     {"collections.group.parts.part.description.image.border", st_collections_group_parts_part_description_image_border},
     {"collections.group.parts.part.description.image.middle", st_collections_group_parts_part_description_image_middle},
     {"collections.group.parts.part.description.image.border_scale", st_collections_group_parts_part_description_image_border_scale},
     {"collections.group.parts.part.description.image.border_scale_by", st_collections_group_parts_part_description_image_border_scale_by},
     {"collections.group.parts.part.description.image.scale_hint", st_collections_group_parts_part_description_image_scale_hint},
     {"collections.group.parts.part.description.fill.smooth", st_collections_group_parts_part_description_fill_smooth},
     {"collections.group.parts.part.description.fill.origin.relative", st_collections_group_parts_part_description_fill_origin_relative},
     {"collections.group.parts.part.description.fill.origin.offset", st_collections_group_parts_part_description_fill_origin_offset},
     {"collections.group.parts.part.description.fill.size.relative", st_collections_group_parts_part_description_fill_size_relative},
     {"collections.group.parts.part.description.fill.size.offset", st_collections_group_parts_part_description_fill_size_offset},
     {"collections.group.parts.part.description.fill.spread", st_collections_group_parts_part_description_fill_spread},
     {"collections.group.parts.part.description.fill.type", st_collections_group_parts_part_description_fill_type},
     {"collections.group.parts.part.description.color_class", st_collections_group_parts_part_description_color_class},
     {"collections.group.parts.part.description.color", st_collections_group_parts_part_description_color},
     {"collections.group.parts.part.description.color2", st_collections_group_parts_part_description_color2},
     {"collections.group.parts.part.description.color3", st_collections_group_parts_part_description_color3},
     {"collections.group.parts.part.description.text.text", st_collections_group_parts_part_description_text_text},
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
     {"collections.group.parts.part.description.text.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.text.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.text.elipsis", st_collections_group_parts_part_description_text_ellipsis},
     {"collections.group.parts.part.description.text.ellipsis", st_collections_group_parts_part_description_text_ellipsis},
     {"collections.group.parts.part.description.text.filter", st_collections_group_parts_part_description_text_filter},
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
     {"collections.group.parts.part.description.perspective.zplane", st_collections_group_parts_part_description_perspective_zplane},
     {"collections.group.parts.part.description.perspective.focal", st_collections_group_parts_part_description_perspective_focal},
     {"collections.group.parts.part.description.params.int", st_collections_group_parts_part_description_params_int},
     {"collections.group.parts.part.description.params.double", st_collections_group_parts_part_description_params_double},
     {"collections.group.parts.part.description.params.string", st_collections_group_parts_part_description_params_string},
     {"collections.group.parts.part.description.params.bool", st_collections_group_parts_part_description_params_bool},
     {"collections.group.parts.part.description.params.choice", st_collections_group_parts_part_description_params_choice},
     IMAGE_STATEMENTS("collections.group.parts.part.description.")
     {"collections.group.parts.part.description.font", st_fonts_font}, /* dup */
     FONT_STYLE_CC_STATEMENTS("collections.group.parts.part.description.")
#ifdef HAVE_EPHYSICS
     {"collections.group.physics.world.gravity", st_collections_group_physics_world_gravity},
     {"collections.group.physics.world.rate", st_collections_group_physics_world_rate},
     {"collections.group.physics.world.z", st_collections_group_physics_world_z},
     {"collections.group.physics.world.depth", st_collections_group_physics_world_depth},
#endif
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
                 desc { "t2"; inherit: "default";
                    color: 0 255 0 255;
                 }
                 desc { "t3"; inherit: "default";
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
             pointer -> pointer_mode
             alt_font -> use_alternate_font_metrics
             clip -> clip_to
          }
       }
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
     {"collections.group.parts.part.pointer", st_collections_group_parts_part_pointer_mode},
     {"collections.group.parts.part.alt_font", st_collections_group_parts_part_use_alternate_font_metrics},
     {"collections.group.parts.part.clip", st_collections_group_parts_part_clip_to_id},
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
       parts {
          part {
             mouse; -> mouse_events: 1;
             nomouse; -> mouse_events: 0;
             repeat; -> repeat_events: 1;
             norepeat; -> repeat_events: 0;
             precise; -> precise_is_inside: 1;
             noprecise; -> precise_is_inside: 0;
             scale; -> scale: 1;
             noscale; -> scale: 0;
             desc {
                vis; -> visible: 1;
                hid; -> visible: 0;
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
     {"collections.group.parts.part.description.vis", st_collections_group_parts_part_description_vis},
     {"collections.group.parts.part.description.hid", st_collections_group_parts_part_description_hid},
     {"collections.group.mouse", st_collections_group_mouse},
     {"collections.group.nomouse", st_collections_group_nomouse},
     {"collections.group.broadcast", st_collections_group_broadcast},
     {"collections.group.nobroadcast", st_collections_group_nobroadcast},
};

#define PROGRAM_OBJECTS(PREFIX) \
     {PREFIX".program", ob_collections_group_programs_program}, /* dup */ \
     {PREFIX".program.script", ob_collections_group_programs_program_script}, /* dup */ \
     {PREFIX".program.sequence.script", ob_collections_group_programs_program_script}, /* dup */ \
     {PREFIX".program.sequence", ob_collections_group_programs_program_sequence}, /* dup */ \
     {PREFIX".programs", NULL}, /* dup */ \
     {PREFIX".programs.set", ob_images_set}, /* dup */ \
     {PREFIX".programs.set.image", ob_images_set_image}, /* dup */ \
     {PREFIX".programs.images", NULL}, /* dup */ \
     {PREFIX".programs.images.set", ob_images_set}, /* dup */ \
     {PREFIX".programs.images.set.image", ob_images_set_image}, /* dup */ \
     {PREFIX".programs.fonts", NULL}, /* dup */ \
     {PREFIX".programs.program", ob_collections_group_programs_program}, /* dup */ \
     {PREFIX".programs.program.script", ob_collections_group_programs_program_script}, /* dup */ \
     {PREFIX".programs.program.sequence", ob_collections_group_programs_program_sequence}, /* dup */ \
     {PREFIX".programs.program.sequence.script", ob_collections_group_programs_program_script}, /* dup */ \
     {PREFIX".programs.script", ob_collections_group_script}, /* dup */ \
     {PREFIX".script", ob_collections_group_script}, /* dup */

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
     {"color_classes", NULL},
     {"color_classes.color_class", ob_color_class},
     {"spectra", NULL},
     {"collections", ob_collections},
     {"collections.externals", NULL}, /* dup */
     {"collections.set", ob_images_set}, /* dup */
     {"collections.set.image", ob_images_set_image}, /* dup */
     {"collections.images", NULL}, /* dup */
     {"collections.images.set", ob_images_set}, /* dup */
     {"collections.images.set.image", ob_images_set_image}, /* dup */
     {"collections.fonts", NULL}, /* dup */
     {"collections.styles", NULL}, /* dup */
     {"collections.styles.style", ob_styles_style}, /* dup */
     {"collections.color_classes", NULL}, /* dup */
     {"collections.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.sounds", NULL},
     {"collections.group.sounds", NULL}, /* dup */
     {"collections.sounds.sample", NULL},
     {"collections.group.sounds.sample", NULL}, /* dup */
     {"collections.vibrations", NULL},
     {"collections.group.vibrations", NULL}, /* dup */
     {"collections.vibrations.sample", NULL},
     {"collections.group.vibrations.sample", NULL}, /* dup */
     {"collections.group", ob_collections_group},
     {"collections.group.data", NULL},
     {"collections.group.limits", NULL},
     {"collections.group.script", ob_collections_group_script},
     {"collections.group.lua_script", ob_collections_group_lua_script},
     {"collections.group.externals", NULL}, /* dup */
     {"collections.group.set", ob_images_set}, /* dup */
     {"collections.group.set.image", ob_images_set_image}, /* dup */
     {"collections.group.images", NULL}, /* dup */
     {"collections.group.images.set", ob_images_set}, /* dup */
     {"collections.group.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.fonts", NULL}, /* dup */
     {"collections.group.styles", NULL}, /* dup */
     {"collections.group.styles.style", ob_styles_style}, /* dup */
     {"collections.group.color_classes", NULL}, /* dup */
     {"collections.group.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts", NULL},
     {"collections.group.parts.set", ob_images_set}, /* dup */
     {"collections.group.parts.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.images", NULL}, /* dup */
     {"collections.group.parts.images.set", ob_images_set}, /* dup */
     {"collections.group.parts.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.fonts", NULL}, /* dup */
     {"collections.group.parts.styles", NULL}, /* dup */
     {"collections.group.parts.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.color_classes", NULL}, /* dup */
     {"collections.group.parts.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part", ob_collections_group_parts_part},
     {"collections.group.parts.part.dragable", NULL},
     {"collections.group.parts.part.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.images", NULL}, /* dup */
     {"collections.group.parts.part.images.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.fonts", NULL}, /* dup */
     {"collections.group.parts.part.styles", NULL}, /* dup */
     {"collections.group.parts.part.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.part.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part.box", NULL},
     {"collections.group.parts.part.box.items", NULL},
     {"collections.group.parts.part.box.items.item", ob_collections_group_parts_part_box_items_item},
     {"collections.group.parts.part.table", NULL},
     {"collections.group.parts.part.table.items", NULL},
     {"collections.group.parts.part.table.items.item", ob_collections_group_parts_part_box_items_item}, /* dup */
     {"collections.group.parts.part.description", ob_collections_group_parts_part_description},
     {"collections.group.parts.part.description.link", ob_collections_group_parts_part_description_link},
     {"collections.group.parts.part.description.rel1", NULL},
     {"collections.group.parts.part.description.rel2", NULL},
     {"collections.group.parts.part.description.image", NULL}, /* dup */
     {"collections.group.parts.part.description.image.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.description.image.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.description.image.images", NULL}, /* dup */
     {"collections.group.parts.part.description.image.images.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.description.image.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.description.fill", NULL},
     {"collections.group.parts.part.description.fill.origin", NULL},
     {"collections.group.parts.part.description.fill.size", NULL},
     {"collections.group.parts.part.description.text", NULL},
     {"collections.group.parts.part.description.text.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.images", NULL}, /* dup */
     {"collections.group.parts.part.description.images.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.description.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.description.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.styles", NULL}, /* dup */
     {"collections.group.parts.part.description.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.part.description.box", NULL},
     {"collections.group.parts.part.description.table", NULL},
#ifdef HAVE_EPHYSICS
     {"collections.group.parts.part.description.physics", NULL},
     {"collections.group.parts.part.description.physics.movement_freedom", NULL},
     {"collections.group.parts.part.description.physics.faces", NULL},
     {"collections.group.parts.part.description.physics.faces.face", st_collections_group_parts_part_description_physics_face},
#endif
     {"collections.group.parts.part.description.map", NULL},
     {"collections.group.parts.part.description.map.rotation", NULL},
     {"collections.group.parts.part.description.perspective", NULL},
     {"collections.group.parts.part.description.params", NULL},
     {"collections.group.parts.part.description.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class", ob_color_class}, /* dup */
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
     {"collections.group.parts.part.desc", ob_collections_group_parts_part_desc},
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
   fill->angle = 0;
   fill->spread = 0;
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

   ed = (Edje_Part_Description_Image*) ep->default_desc;
   _edje_part_description_image_remove(ed);

   for (j = 0; j < ep->other.desc_count; j++)
     {
        ed = (Edje_Part_Description_Image*) ep->other.desc[j];
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

   #define STRDUP(x) x ? strdup(x) : NULL
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
   ep->sample_name = STRDUP(ep2->sample_name);
   ep->tone_name = STRDUP(ep2->tone_name);
   ep->duration = ep2->duration;
   ep->speed = ep2->speed;

   EINA_LIST_FOREACH(ep2->targets, l, et2)
     {
        name = (char*) (et2 + 1);
        et = mem_alloc(SZ(Edje_Program_Target) + strlen(name) + 1);
        ep->targets = eina_list_append(ep->targets, et);
        copy = (char*) (et + 1);

        memcpy(copy, name, strlen(name) + 1);

        switch (ep2->action)
          {
           case EDJE_ACTION_TYPE_STATE_SET:
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
        name = (char*) (pa2 + 1);
        pa = mem_alloc(SZ(Edje_Program_After) + strlen(name) + 1);
        ep->after = eina_list_append(ep->after, pa);
        copy = (char*) (pa + 1);
        memcpy(copy, name, strlen(name) + 1);
        data_queue_copied_program_lookup(pc, &(pa2->id), &(pa->id));
     }

   ep->api.name = STRDUP(ep2->api.name);
   ep->api.description = STRDUP(ep2->api.description);
   data_queue_copied_part_lookup(pc, &(ep2->param.src), &(ep->param.src));
   data_queue_copied_part_lookup(pc, &(ep2->param.dst), &(ep->param.dst));

   epp = (Edje_Program_Parser *)ep;
   epp->can_override = EINA_TRUE;

   #undef STRDUP
}

/*****/

/** @edcsection{toplevel,Top-Level blocks} */

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
    @endblock

    @property
        image
    @parameters
        [image file] [compression method] (compression level)
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
	  free((char*) tmp);
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
	img->source_type = EDJE_IMAGE_SOURCE_TYPE_EXTERNAL;
	img->source_param = 0;
     }
   if ((img->source_type < EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY) ||
       (img->source_type > EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC2))
	check_arg_count(2);
   else
     {
	img->source_param = parse_int_range(2, 0, 100);
	check_arg_count(3);
     }
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
        [image file] [compression method] (compression level)
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

   ed = (Edje_Part_Description_Image*) current_desc;

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
        specified file formated as a single string of text. This property only
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

   munmap((void*)data, buf.st_size);
   close(fd);

   es->str = value;

   if (!edje_file->data)
     edje_file->data = eina_hash_string_small_new(free);

   eina_hash_direct_add(edje_file->data, key, es);

   free(filename);
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
                color:  [0-255] [0-255] [0-255] [0-255];
                color2: [0-255] [0-255] [0-255] [0-255];
                color3: [0-255] [0-255] [0-255] [0-255]
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

/**
    @page edcref
    @property
        color
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        The main color.
    @endproperty
*/
static void
st_color_class_color(void)
{
   Edje_Color_Class *cc;

   check_arg_count(4);

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));
   cc->r = parse_int_range(0, 0, 255);
   cc->g = parse_int_range(1, 0, 255);
   cc->b = parse_int_range(2, 0, 255);
   cc->a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color2
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Used as outline in text and textblock parts.
    @endproperty
*/
static void
st_color_class_color2(void)
{
   Edje_Color_Class *cc;

   check_arg_count(4);

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));
   cc->r2 = parse_int_range(0, 0, 255);
   cc->g2 = parse_int_range(1, 0, 255);
   cc->b2 = parse_int_range(2, 0, 255);
   cc->a2 = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color3
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Used as shadow in text and textblock parts.
    @endproperty
*/
static void
st_color_class_color3(void)
{
   Edje_Color_Class *cc;

   check_arg_count(4);

   cc = eina_list_data_get(eina_list_last(edje_file->color_classes));
   cc->r3 = parse_int_range(0, 0, 255);
   cc->g3 = parse_int_range(1, 0, 255);
   cc->b3 = parse_int_range(2, 0, 255);
   cc->a3 = parse_int_range(3, 0, 255);
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
        @li LOSSY [-0.1  - 1.0]: Lossy compression with quality from 0.0 to 1.0.
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
                      if (((!!ell->ed->state.name) && strcmp(el->ed->state.name, "default")) ||
                         ((!!el->ed->state.name) && strcmp(ell->ed->state.name, "default")))
                           continue;
                    }
                  else if (ell->ed->state.name && strcmp(ell->ed->state.name, el->ed->state.name))
                    continue;
                  eina_list_move_list(&combine, (Eina_List**)&tup->data, l);
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
        Edje object and to identify the group to swallow in a GROUP part. If a
        group with the same name exists already it will be completely overriden
        by the new group.
    @endproperty
*/
static void
st_collections_group_name(void)
{
   check_arg_count(1);
   _group_name(parse_str(0));
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

#define STRDUP(x) x ? strdup(x) : NULL
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
   ep->repeat_events = ep2->repeat_events;
   ep->ignore_flags = ep2->ignore_flags;
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
   ep->dragable.x = ep2->dragable.x;
   ep->dragable.step_x = ep2->dragable.step_x;
   ep->dragable.count_x = ep2->dragable.count_x;
   ep->dragable.y = ep2->dragable.y;
   ep->dragable.step_y = ep2->dragable.step_y;
   ep->dragable.count_y = ep2->dragable.count_y;
   ep->nested_children_count = ep2->nested_children_count;

   data_queue_copied_part_lookup(pc, &(ep2->dragable.confine_id), &(ep->dragable.confine_id));
   data_queue_copied_part_lookup(pc, &(ep2->dragable.threshold_id), &(ep->dragable.threshold_id));
   data_queue_copied_part_lookup(pc, &(ep2->dragable.event_id), &(ep->dragable.event_id));

   epp = (Edje_Part_Parser *)ep;
   epp2 = (Edje_Part_Parser *)ep2;
   epp->reorder.insert_before = STRDUP(epp2->reorder.insert_before);
   epp->reorder.insert_after = STRDUP(epp2->reorder.insert_after);
   epp->can_override = EINA_TRUE;

   for (j = 0 ; j < ep2->items_count ; j++)
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

        pitem = (Edje_Pack_Element_Parser *)item;
        pitem->can_override = EINA_TRUE;
     }

   ep->api.name = STRDUP(ep2->api.name);
   if (ep2->api.description) ep->api.description = STRDUP(ep2->api.description);

   // copy default description
   ob_collections_group_parts_part_description();
   ed = ep->default_desc;
   parent_desc = ed2 = ep2->default_desc;
   free((void*)ed->state.name);
   ed->state.name = STRDUP(ed2->state.name);
   ed->state.value = ed2->state.value;
   st_collections_group_parts_part_description_inherit();
   parent_desc = NULL;

   // copy other description
   for (j = 0 ; j < ep2->other.desc_count ; j++)
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
   tg->targets = calloc(argc, sizeof(char*));

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
   Edje_Part_Collection *pc, *pc2;
   Edje_Part_Collection_Parser *pcp, *pcp2;
   Edje_Part *ep, *ep2;
   Edje_List_Foreach_Data fdata;
   Eina_List *l;
   char *parent_name;
   unsigned int i, j, offset;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   parent_name = parse_str(0);

   EINA_LIST_FOREACH(edje_collections, l, pc2)
     {
        if (!strcmp(parent_name, pc2->part))
          break;
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
        char *key, *alias;

        memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
        eina_hash_foreach(pc2->alias,
                     _edje_data_item_list_foreach, &fdata);
        if (!pc->alias) pc->alias = eina_hash_string_small_new(free);
        EINA_LIST_FREE(fdata.list, key)
          {
             alias = eina_hash_find(pc2->alias, key);
             eina_hash_direct_add(pc->alias, key, alias);
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

   pcp = (Edje_Part_Collection_Parser *)pc;
   pcp2 = (Edje_Part_Collection_Parser *)pc2;
   pcp->default_mouse_events = pcp2->default_mouse_events;

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
   for (i = 0 ; i < pc2->parts_count ; i++)
     {
        // copy the part
        edje_cc_handlers_part_make(-1);
        ep = pc->parts[i + offset];
        ep2 = pc2->parts[i];
        _part_copy(ep, ep2);
     }

   //copy programs
   for (j = 0 ; j < pc2->programs.fnmatch_count ; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.fnmatch[j]);
     }
   for (j = 0 ; j < pc2->programs.strcmp_count ; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.strcmp[j]);
     }
   for (j = 0 ; j < pc2->programs.strncmp_count ; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.strncmp[j]);
     }
   for (j = 0 ; j < pc2->programs.strrncmp_count ; j++)
     {
        ob_collections_group_programs_program();
        _edje_program_copy(current_program, pc2->programs.strrncmp[j]);
     }
   for (j = 0 ; j < pc2->programs.nocmp_count ; j++)
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

   EINA_LIST_FOREACH(cd2->programs, l, cp2)
     {
        cp = mem_alloc(SZ(Code_Program));

        cp->l1 = cp2->l1;
        cp->l2 = cp2->l2;
        cp->script = STRDUP(cp2->script);
        cp->original = STRDUP(cp2->original);
        cd->is_lua = cd2->is_lua;
        cd->shared = cd2->shared;
        cd->original = cd2->original;
        cd->programs = eina_list_append(cd->programs, cp);
        data_queue_copied_anonymous_lookup(pc, &(cp2->id), &(cp->id));
     }

   free(parent_name);
   #undef STRDUP
}

/**
    @page edcref
    @property
        script_only
    @parameters
        [on/off]
    @effect
        The flag (on/off) as to if this group is defined ONLY by script
        callbacks such as init(), resize() and shutdown()
    @endproperty
*/
static void
st_collections_group_script_only(void)
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
       broadcast_signal
   @parameters
       [on/off]
   @effect
       Signal got automatically broadcasted to all sub group part. Default to
       true since 1.1.
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
        Defaults to 1 if not set, to maintain compatibility.
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
        program_source
    @parameters
        [source name]
    @effect
        Change the default value of source for every program in the current group
        which is declared after this value is set.
        Defaults to an unset value to maintain compatibility.
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
		  ERR("parse error %s:%i. There is already an existing script section for the group",
		      file_in, line - 1);
		  exit(-1);
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
        This limit will be applied on the x absis and is expressed in pixels.
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
                    clip_to: "anotherpart";
                    source:  "groupname";
                    pointer_mode: AUTOGRAB;
                    use_alternate_font_metrics: 0;

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
   ep->repeat_events = 0;
   ep->ignore_flags = EVAS_EVENT_FLAG_NONE;
   ep->scale = 0;
   ep->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
   ep->precise_is_inside = 0;
   ep->use_alternate_font_metrics = 0;
   ep->access = 0;
   ep->clip_to_id = -1;
   ep->dragable.confine_id = -1;
   ep->dragable.threshold_id = -1;
   ep->dragable.event_id = -1;
   ep->items = NULL;
   ep->nested_children_count = 0;

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

static void
_part_type_set(unsigned int type)
{
   /* handle type change of inherited part */
   if (type != current_part->type)
     {
        Edje_Part_Description_Common *new, *previous;
        Edje_Part_Collection *pc;
        Edje_Part *ep;
        unsigned int i;

        /* we don't free old part as we don't remove all reference to them */
        part_description_image_cleanup(current_part);

        pc = eina_list_data_get(eina_list_last(edje_collections));
        ep = current_part;

        previous = ep->default_desc;
        if (previous)
          {
             new = _edje_part_description_alloc(type, pc->part, ep->name);
             memcpy(new, previous, sizeof (Edje_Part_Description_Common));

             ep->default_desc = new;
          }

        for (i = 0; i < ep->other.desc_count; i++)
          {
             previous = ep->other.desc[i];
             new = _edje_part_description_alloc(type, pc->part, ep->name);
             memcpy(new, previous, sizeof (Edje_Part_Description_Common));
             ep->other.desc[i] = new;
          }
     }

   current_part->type = type;
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
   if (prnt)  /* This is the child of parent in stack */
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
                  NULL);

   stack_pop_quick(EINA_TRUE, EINA_TRUE);
   stack_push_quick("part");
   _part_create();
   _part_type_set(type);
}

static void
ob_collections_group_parts_part(void)
{
   _part_create();
}

static void *
_part_desc_free(Edje_Part_Description_Common *ed)
{
   if (!ed) return NULL;
   free((void*)ed->state.name);
   free(ed);
   return NULL;
}

static void *
_part_free(Edje_Part *ep)
{
   Edje_Part_Parser *epp = (Edje_Part_Parser*)ep;
   unsigned int j;

   free((void*)ep->name);
   free((void*)ep->source);
   free((void*)ep->source2);
   free((void*)ep->source3);
   free((void*)ep->source4);
   free((void*)ep->source5);
   free((void*)ep->source6);

   free((void*)epp->reorder.insert_before);
   free((void*)epp->reorder.insert_after);

   for (j = 0 ; j < ep->items_count ; j++)
     free(ep->items[j]);
   free(ep->items);

   free((void*)ep->api.name);
   free((void*)ep->api.description);

   _part_desc_free(ep->default_desc);
   for (j = 0 ; j < ep->other.desc_count ; j++)
     _part_desc_free(ep->other.desc[j]);
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
             exit(-1);
          }
        pname = current_part->name;
        current_part->name = NULL;
        current_part = _part_free(current_part);
        edje_cc_handlers_part_make(id);
        _part_copy(current_part, pc->parts[i]);
        free((void*)current_part->name);
        current_part->name = pname;
        free(name);
        return;
     }

   ERR("Cannot inherit non-existing part '%s' in group '%s'", name, current_de->entry);
   exit(-1);

   free(name);
}

static void
_program_free(Edje_Program *pr)
{
   Edje_Program_Target *prt;
   Edje_Program_After *pa;

   free((void*)pr->name);
   free((void*)pr->signal);
   free((void*)pr->source);
   free((void*)pr->filter.part);
   free((void*)pr->filter.state);
   free((void*)pr->state);
   free((void*)pr->state2);
   free((void*)pr->sample_name);
   free((void*)pr->tone_name);
   EINA_LIST_FREE(pr->targets, prt);
      free(prt);
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

          _edje_program_remove(pc, pr);

          _program_free(pr);
          return EINA_TRUE;
       }
   return EINA_FALSE;
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

        copied_program_lookup_delete(pc, name);
        if (!success)
          {
             ERR("Attempted removal of nonexistent program '%s' in group '%s'.",
                 name, current_de->entry);
             exit(-1);
          }
        free(name);
     }
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
   unsigned int n, argc, orig_count;
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

             pc->parts[j] = _part_free(pc->parts[j]);
             for (i = j; i < pc->parts_count - 1; i++)
               {
                  if (!pc->parts[i + 1]) break;
                  pc->parts[i] = pc->parts[i + 1];
               }
             pc->parts_count--;
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
            @li PROXY
            @li SPACER
    @endproperty
*/
static void
st_collections_group_parts_part_type(void)
{
   unsigned int type;

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
		     "SPACER", EDJE_PART_TYPE_SPACER,
                     NULL);

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
        any type of signal at all. It's set to 1 by default, or to the value
        set to "mouse_events" at the group level, if any.
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
        repeat_events
    @parameters
        [1 or 0]
    @effect
        Specifies whether a part echoes a mouse event to other parts below the
        pointer (1), or not (0). It's set to 0 by default.
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
        with one of the flags specified. Possible flags:
            @li NONE (default value, no event will be ignored)
            @li ON_HOLD
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
        designer gets to choose what works best.
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
        Sets the mouse pointer behavior for a given part. The default value is
        AUTOGRAB. Aviable modes:
            @li AUTOGRAB, when the part is clicked and the button remains
                pressed, the part will be the source of all future mouse
                signals emitted, even outside the object, until the button is
                released.
            @li NOGRAB, the effect will be limited to the part's container.
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
        resource intensive. Disabled by default.
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
        modes, and the selection itself is dragable at both ends.
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
        @li UNDER cursor mode means the cursor will draw below the character pointed
        at. That's the default.
        @li BEFORE cursor mode means the cursor is drawn as a vertical line before
        the current character, just like many other GUI toolkits handle it.
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
        Specifies whether the part will use accessibility feature (1),
        or not (0). It's set to 0 by default.
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
        divisibles by it, causing the part to jump from position to position.
        If step is set to 0 it is calculated as height of confine part divided by
        count.
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
static void ob_collections_group_parts_part_box_items_item(void)
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
   ep->items = realloc(ep->items, sizeof (Edje_Pack_Element*) * ep->items_count);
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

#define CURRENT_ITEM_CHECK						\
  if (!current_item)							\
    {									\
       ERR("parse error %s:%i. Item not defined at this stage.",        \
           file_in, line - 1);                                          \
      exit(-1);								\
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
static void st_collections_group_parts_part_box_items_item_type(void)
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
static void st_collections_group_parts_part_box_items_item_name(void)
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
static void st_collections_group_parts_part_box_items_item_source(void)
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
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_min(void)
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

       default value will be 1 1;
   @endproperty
*/
static void st_collections_group_parts_part_box_items_item_spread(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->spread.w = parse_int_range(0, 0, 0x7ffffff);
   current_item->spread.h = parse_int_range(1, 0, 0x7ffffff);
}

/**
    @page edcref
    @property
        prefer
    @parameters
        [width] [height]
    @effect
        Sets the preferred size hints for this object.
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_prefer(void)
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
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_max(void)
{
   CURRENT_ITEM_CHECK;

   check_arg_count(2);

   current_item->max.w = parse_int_range(0, 0, 0x7ffffff);
   current_item->max.h = parse_int_range(1, 0, 0x7ffffff);
}

/**
    @page edcref
    @property
        padding
    @parameters
        [left] [right] [top] [bottom]
    @effect
        Sets the padding hints for this object.
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_padding(void)
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
        Sets the alignment hints for this object.
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_align(void)
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
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_weight(void)
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
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_aspect(void)
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
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_aspect_mode(void)
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
static void st_collections_group_parts_part_box_items_item_options(void)
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
static void st_collections_group_parts_part_table_items_item_position(void)
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
        Defaults to 1 1.
    @endproperty
*/
static void st_collections_group_parts_part_table_items_item_span(void)
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
   colors = malloc(sizeof(Edje_Map_Color *) * parent->map.colors_count);

   for (i = 0; i < (int)parent->map.colors_count; i++)
     {
        color = parent->map.colors[i];

        Edje_Map_Color *c = mem_alloc(SZ(Edje_Map_Color));
        if (!color)
          {
             ERR("not enough memory");
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

   ed->rel1.id_x = -1;
   ed->rel1.id_y = -1;
   ed->rel2.id_x = -1;
   ed->rel2.id_y = -1;

   if (!ep->default_desc)
     {
        current_desc = ep->default_desc = ed;
        ed->state.name = strdup("default");

          {  /* Get the ptr of the part above current part in hierarchy */
             Edje_Part *node = edje_cc_handlers_hierarchy_parent_get();
             if (node)  /* Make relative according to part hierarchy */
               edje_cc_handlers_hierarchy_set(node);
          }
     }
   else
     {
        ep->other.desc_count++;
        ep->other.desc = realloc(ep->other.desc,
                                 sizeof (Edje_Part_Description_Common*) * ep->other.desc_count);
        current_desc = ep->other.desc[ep->other.desc_count - 1] = ed;
     }

   ed->visible = 1;
   ed->limit = 0;
   ed->align.x = FROM_DOUBLE(0.5);
   ed->align.y = FROM_DOUBLE(0.5);
   ed->min.w = 0;
   ed->min.h = 0;
   ed->fixed.w = 0;
   ed->fixed.h = 0;
   ed->max.w = -1;
   ed->max.h = -1;
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
   ed->persp.zplane = 0;
   ed->persp.focal = 1000;
   ed->minmul.have = 1;
   ed->minmul.w = FROM_INT(1);
   ed->minmul.h = FROM_INT(1);
}

static void
ob_collections_group_parts_part_desc(void)
{
   stack_pop_quick(EINA_TRUE, EINA_TRUE);
   stack_push_quick("description");
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
    @endproperty
*/
static void
st_collections_group_parts_part_description_inherit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description_Common *ed, *parent = NULL;
   Edje_Part_Image_Id *iid;
   char *parent_name;
   const char *state_name;
   double parent_val, state_val;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_part;
   ed = current_desc;

   parent = parent_desc;
   if (!parent)
     {
        check_min_arg_count(1);

        /* inherit may not be used in the default description */
        if (!ep->other.desc_count)
          {
             ERR("parse error %s:%i. "
                 "inherit may not be used in the default description",
                 file_in, line - 1);
             exit(-1);
          }

        /* find the description that we inherit from */
        parent_name = parse_str(0);
        if (get_arg_count() == 2)
          parent_val = parse_float_range(1, 0.0, 1.0);
        else
          parent_val = 0.0;

        if (!strcmp (parent_name, "default") && parent_val == 0.0)
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

                  if (!strcmp (d->state.name, parent_name))
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

   data_queue_copied_part_lookup(pc, &parent->map.id_persp, &ed->map.id_persp);
   data_queue_copied_part_lookup(pc, &parent->map.id_light, &ed->map.id_light);
   data_queue_copied_part_lookup(pc, &parent->map.rot.id_center, &ed->map.rot.id_center);

   /* make sure all the allocated memory is getting copied, not just
    * referenced
    */
#define STRDUP(x) x ? strdup(x) : NULL

   ed->color_class = STRDUP(ed->color_class);
   ed->map.colors = _copied_map_colors_get(parent);

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
              Edje_Part_Description_Text *ted = (Edje_Part_Description_Text*) ed;
              Edje_Part_Description_Text *tparent = (Edje_Part_Description_Text*) parent;

              ted->text = tparent->text;

              ted->text.text.str = STRDUP(ted->text.text.str);
              ted->text.text_class = STRDUP(ted->text.text_class);
              ted->text.font.str = STRDUP(ted->text.font.str);
              ted->text.filter.str = STRDUP(ted->text.filter.str);
              {
                 Eina_List *l;
                 Eina_Stringshare *name;
                 static int part_key = 0;

                 EINA_LIST_FOREACH(ted->text.filter_sources, l, name)
                   data_queue_part_lookup(pc, name, &part_key);
              }

              data_queue_copied_part_nest_lookup(pc, &(tparent->text.id_source), &(ted->text.id_source), &ted->text.id_source_part);
              data_queue_copied_part_nest_lookup(pc, &(tparent->text.id_text_source), &(ted->text.id_text_source), &ted->text.id_text_source_part);

              break;
           }
      case EDJE_PART_TYPE_IMAGE:
           {
              Edje_Part_Description_Image *ied = (Edje_Part_Description_Image *) ed;
              Edje_Part_Description_Image *iparent = (Edje_Part_Description_Image *) parent;
              unsigned int i;

              ied->image = iparent->image;

              data_queue_image_remove(&ied->image.id, &ied->image.set);
              data_queue_copied_image_lookup(&iparent->image.id, &ied->image.id, &ied->image.set);

              ied->image.tweens = calloc(iparent->image.tweens_count,
                                         sizeof (Edje_Part_Image_Id*));
              for (i = 0; i < iparent->image.tweens_count; i++)
                {
                   Edje_Part_Image_Id *iid_new;

                   iid = iparent->image.tweens[i];

                   iid_new = mem_alloc(SZ(Edje_Part_Image_Id));
                   data_queue_image_remove(&ied->image.id, &ied->image.set);
                   data_queue_copied_image_lookup(&(iid->id), &(iid_new->id), &(iid_new->set));
                   ied->image.tweens[i] = iid_new;
                }

              break;
           }
      case EDJE_PART_TYPE_PROXY:
           {
              Edje_Part_Description_Proxy *ped = (Edje_Part_Description_Proxy*) ed;
              Edje_Part_Description_Proxy *pparent = (Edje_Part_Description_Proxy*) parent;

              data_queue_copied_part_lookup(pc, &(pparent->proxy.id), &(ped->proxy.id));

              break;
           }
      case EDJE_PART_TYPE_BOX:
           {
              Edje_Part_Description_Box *bed = (Edje_Part_Description_Box *) ed;
              Edje_Part_Description_Box *bparent = (Edje_Part_Description_Box *) parent;

              bed->box = bparent->box;

              break;
           }
      case EDJE_PART_TYPE_TABLE:
           {
              Edje_Part_Description_Table *ted = (Edje_Part_Description_Table *) ed;
              Edje_Part_Description_Table *tparent = (Edje_Part_Description_Table *) parent;

              ted->table = tparent->table;

              break;
           }
      case EDJE_PART_TYPE_EXTERNAL:
           {
              Edje_Part_Description_External *eed = (Edje_Part_Description_External *) ed;
              Edje_Part_Description_External *eparent = (Edje_Part_Description_External *) parent;

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
     }

#undef STRDUP
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

   ed = (Edje_Part_Description_Proxy*) current_desc;

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
          _edje_part_description_image_remove((Edje_Part_Description_Image*) ed);

        free(ed);
        ep->other.desc_count--;
        ep->other.desc = realloc(ep->other.desc,
                                 sizeof (Edje_Part_Description_Common*) * ep->other.desc_count);
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
                    _edje_part_description_image_remove((Edje_Part_Description_Image*) ed);

                  free(ed);
                  ep->other.desc_count--;
                  ep->other.desc = realloc(ep->other.desc,
                                           sizeof (Edje_Part_Description_Common*) * ep->other.desc_count);
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
        coupled with an index number between 0.0 and 1.0. All parts must have
        at least one description named "default 0.0".
    @endproperty
*/
static void
st_collections_group_parts_part_description_state(void)
{
   Edje_Part *ep;
   Edje_Part_Description_Common *ed;
   char *s;

   check_min_arg_count(1);

   ep = current_part;

   ed = ep->default_desc;
   if (ep->other.desc_count) ed = ep->other.desc[ep->other.desc_count - 1];

   s = parse_str(0);
   if (!strcmp (s, "custom"))
     {
        ERR("parse error %s:%i. invalid state name: '%s'.",
            file_in, line - 1, s);
        exit(-1);
     }

   free((void *)ed->state.name);
   ed->state.name = s;
   if (get_arg_count() == 1)
     ed->state.value = 0.0;
   else
     ed->state.value = parse_float_range(1, 0.0, 1.0);
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
        (0). Non-visible parts do not emit signals. The default value is 1.
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
					   (unsigned char**) &(pc->limits.parts),
					   (unsigned char*) &pc->limits.parts[count].part - (unsigned char*) pc->limits.parts); //fixme
     }
}

/**
    @page edcref
    @property
        align
    @parameters
        [X axis] [Y axis]
    @effect
        When the displayed object's size is smaller (or bigger) than
        its container, this property moves it relatively along both
        axis inside its container. @c "0.0" means left/top edges of
        the object touching container's respective ones, while @c
        "1.0" stands for right/bottom edges of the object (on
        horizonal/vertical axis, respectively). The default value is
        @c "0.5 0.5".
    @endproperty
*/
static void
st_collections_group_parts_part_description_align(void)
{
   check_arg_count(2);

   current_desc->align.x = FROM_DOUBLE(parse_float_range(0, 0.0, 1.0));
   current_desc->align.y = FROM_DOUBLE(parse_float_range(1, 0.0, 1.0));
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
    @endproperty
*/
static void
st_collections_group_parts_part_description_fixed(void)
{
   check_arg_count(2);

   current_desc->fixed.w = parse_float_range(0, 0, 1);
   current_desc->fixed.h = parse_float_range(1, 0, 1);
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
    @endproperty
*/
static void
st_collections_group_parts_part_description_min(void)
{
   check_min_arg_count(1);

   if (is_param(1)) {
      current_desc->min.w = parse_float_range(0, 0, 0x7fffffff);
      current_desc->min.h = parse_float_range(1, 0, 0x7fffffff);
   } else {
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
        [width multipler] [height multiplier]
    @effect
        A multiplier FORCIBLY applied to whatever minimum size is only during
        minimum size calculation.
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
        The maximum size of the state. A size of -1.0 means that it will be ignored in one direction.

        When max is set to SOURCE, edje will enforce the part to be
        not more than the original image size. The part must be an
        IMAGE part.
    @endproperty
*/
static void
st_collections_group_parts_part_description_max(void)
{
   check_min_arg_count(1);

   if (is_param(1)) {
      current_desc->max.w = parse_float_range(0, -1.0, 0x7fffffff);
      current_desc->max.h = parse_float_range(1, -1.0, 0x7fffffff);
   } else {
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
        step
    @parameters
        [width] [height]
    @effect
        Restricts resizing of each dimension to values divisibles by its value.
        This causes the part to jump from value to value while resizing. The
        default value is "0 0" disabling stepping.
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
    @endproperty
*/
static void
st_collections_group_parts_part_description_aspect_preference(void)
{
   check_arg_count(1);

   current_desc->aspect.prefer =  parse_enum(0,
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
        values can be overrided by the "color", "color2" and "color3"
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
        [red] [green] [blue] [alpha]
    @effect
        Sets the main color to the specified values (between 0 and 255).
    @endproperty
*/
static void
st_collections_group_parts_part_description_color(void)
{
   check_arg_count(4);

   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
       ERR("parse error %s:%i. SPACER part can't have a color defined",
	   file_in, line - 1);
       exit(-1);
     }

   current_desc->color.r = parse_int_range(0, 0, 255);
   current_desc->color.g = parse_int_range(1, 0, 255);
   current_desc->color.b = parse_int_range(2, 0, 255);
   current_desc->color.a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color2
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Sets the text shadow color to the specified values (0 to 255).
    @endproperty
*/
static void
st_collections_group_parts_part_description_color2(void)
{
   check_arg_count(4);

   if (current_part->type == EDJE_PART_TYPE_SPACER)
     {
       ERR("parse error %s:%i. SPACER part can't have a color defined",
	   file_in, line - 1);
       exit(-1);
     }

   current_desc->color2.r = parse_int_range(0, 0, 255);
   current_desc->color2.g = parse_int_range(1, 0, 255);
   current_desc->color2.b = parse_int_range(2, 0, 255);
   current_desc->color2.a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color3
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Sets the text outline color to the specified values (0 to 255).
    @endproperty
*/
static void
st_collections_group_parts_part_description_color3(void)
{
   Edje_Part_Collection *pc;
   Edje_Part_Description_Text *ed;

   check_arg_count(4);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (current_part->type != EDJE_PART_TYPE_TEXT
       && current_part->type != EDJE_PART_TYPE_TEXTBLOCK)
     {
        ERR("Setting color3 in part %s from %s not of type TEXT or TEXTBLOCK.",
            current_part->name, pc->part);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text*)current_desc;

   ed->text.color3.r = parse_int_range(0, 0, 255);
   ed->text.color3.g = parse_int_range(1, 0, 255);
   ed->text.color3.b = parse_int_range(2, 0, 255);
   ed->text.color3.a = parse_int_range(3, 0, 255);
}

/** @edcsubsection{collections_group_parts_description_relatives,
 *                 Group.Parts.Part.Description.Relatives (rel1/rel2)} */

/**
    @page edcref
    @block
        rel1/rel2
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
        }
    @description
        The rel1 and rel2 blocks are used to define the position of each corner
        of the part's container. With rel1 being the left-up corner and rel2
        being the right-down corner.
    @endblock

    @property
        relative
    @parameters
        [X axis] [Y axis]
    @effect
        Moves a corner to a relative position inside the container of the
        relative "to" part. Values from 0.0 (0%, beginning) to 1.0 (100%, end)
        of each axis.
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_relative(void)
{
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
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_offset(void)
{
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
        container. Setting to "" will un-set this value for inherited
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
st_collections_group_parts_part_description_rel1_to(void)
{
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
        Setting to "" will un-set this value for inherited parts.
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_to_x(void)
{
   Edje_Part_Collection *pc;

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
        "relative". Setting to "" will un-set this value for inherited parts.
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_to_y(void)
{
   Edje_Part_Collection *pc;

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
   check_arg_count(2);

   current_desc->rel2.relative_x = FROM_DOUBLE(parse_float(0));
   current_desc->rel2.relative_y = FROM_DOUBLE(parse_float(1));
}

static void
st_collections_group_parts_part_description_rel2_offset(void)
{
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

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
      free(name);
   }
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

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. "
            "image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image*) current_desc;

   {
      char *name;

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

   ed = (Edje_Part_Description_Image*) current_desc;

   {
      char *name;
      Edje_Part_Image_Id *iid;

      iid = mem_alloc(SZ(Edje_Part_Image_Id));
      ed->image.tweens_count++;
      ed->image.tweens = realloc(ed->image.tweens,
				 sizeof (Edje_Part_Image_Id*) * ed->image.tweens_count);
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

   ed = (Edje_Part_Description_Image*) current_desc;

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

   ed = (Edje_Part_Description_Image*) current_desc;

   ed->image.border.no_fill =  parse_enum(0,
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

   ed = (Edje_Part_Description_Image*) current_desc;

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

   ed = (Edje_Part_Description_Image*) current_desc;

   ed->image.border.scale =  parse_enum(0,
					"0", 0,
					"1", 1,
					NULL);
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

   ed = (Edje_Part_Description_Image*) current_desc;

   ed->image.scale_hint =  parse_enum(0,
				      "NONE", EVAS_IMAGE_SCALE_HINT_NONE,
				      "DYNAMIC", EVAS_IMAGE_SCALE_HINT_DYNAMIC,
				      "STATIC", EVAS_IMAGE_SCALE_HINT_STATIC,
				      "0", EVAS_IMAGE_SCALE_HINT_NONE,
				      NULL);
}

/** @edcsubsection{collections_group_parts_description_image_fill,
 *                 Group.Parts.Part.Description.Image.Fill} */

/**
    @page edcref
    @block
        fill
    @context
        image {
            ..
            fill {
                type: SCALE;
                smooth: 0-1;
                origin { }
                size { }
            }
            ..
        }
    @description
        The fill method is an optional block that defines the way an IMAGE part
        is going to be displayed inside its container.
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
        be smoothed on scaling (1) or not (0). The default value is 1.
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

           ed = (Edje_Part_Description_Image*) current_desc;

           fill = &ed->image.fill;
	   break;
        }
      case EDJE_PART_TYPE_PROXY:
        {
           Edje_Part_Description_Proxy *ed;

           ed = (Edje_Part_Description_Proxy*) current_desc;

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
        spread
    @parameters
        TODO
    @effect
        TODO
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_spread(void)
{
#if 0
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description_Image *ed;
#endif

   check_arg_count(1);

   /* XXX this will need to include IMAGES when spread support is added to evas images */
   {
      ERR("parse error %s:%i. fill.spread not supported yet.",
	  file_in, line - 1);
      exit(-1);
   }

#if 0
   pc = eina_list_data_get(eina_list_last(edje_collections));

   ep = pc->parts[pc->parts_count - 1];

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
        ERR("parse error %s:%i. image attributes in non-IMAGE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Image*) ep->default_desc;
   if (ep->other.desc_count) ed = (Edje_Part_Description_Image*)  ep->other.desc[ep->other.desc_count - 1];

   ed->image.fill.spread = parse_int_range(0, 0, 1);
#endif
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
        SCALE is default type.

        Valid values are:
        @li SCALE
        @li TILE
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

           ed = (Edje_Part_Description_Image*) current_desc;

           fill = &ed->image.fill;
	   break;
        }
      case EDJE_PART_TYPE_PROXY:
        {
           Edje_Part_Description_Proxy *ed;

           ed = (Edje_Part_Description_Proxy*) current_desc;

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

/** @edcsubsection{collections_group_parts_description_image_fill_origin,
 *                 Group.Parts.Part.Description.Image.Fill.Origin} */

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

           ed = (Edje_Part_Description_Image*) current_desc;

           fill = &ed->image.fill;
	   break;
        }
      case EDJE_PART_TYPE_PROXY:
        {
           Edje_Part_Description_Proxy *ed;

           ed = (Edje_Part_Description_Proxy*) current_desc;

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

           ed = (Edje_Part_Description_Image*) current_desc;

           fill = &ed->image.fill;
	   break;
        }
      case EDJE_PART_TYPE_PROXY:
        {
           Edje_Part_Description_Proxy *ed;

           ed = (Edje_Part_Description_Proxy*) current_desc;

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

/** @edcsubsection{collections_group_parts_description_image_fill_size,
 *                 Group.Parts.Part.Description.Image.Fill.Size} */

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
        half the size, while "2.0 2.0" represents the double. The default
        value is "1.0 1.0".
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

           ed = (Edje_Part_Description_Image*) current_desc;

           fill = &ed->image.fill;
	   break;
        }
      case EDJE_PART_TYPE_PROXY:
        {
           Edje_Part_Description_Proxy *ed;

           ed = (Edje_Part_Description_Proxy*) current_desc;

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

           ed = (Edje_Part_Description_Image*) current_desc;

           fill = &ed->image.fill;
	   break;
        }
      case EDJE_PART_TYPE_PROXY:
        {
           Edje_Part_Description_Proxy *ed;

           ed = (Edje_Part_Description_Proxy*) current_desc;

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

   ed = (Edje_Part_Description_Text*) current_desc;

   for (i = 0; ;i++)
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

   ed = (Edje_Part_Description_Text*) current_desc;

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
        block. Can be overrided by the application.
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

   ed = (Edje_Part_Description_Text*) current_desc;

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

   ed = (Edje_Part_Description_Text*) current_desc;

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

   ed = (Edje_Part_Description_Text*) current_desc;

   ed->text.repch.str = parse_str(0);
}

/**
    @page edcref

    @property
        size
    @parameters
        [font size in points (pt)]
    @effect
        Sets the default font size for the text part. Can be overrided by the
        application.
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

   ed = (Edje_Part_Description_Text*)current_desc;

   ed->text.size = parse_int_range(0, 0, 255);
}

/**
    @page edcref

    @property
        size_range
    @parameters
        [font min size in points (pt)] [font max size in points (pt)]
    @effect
        Sets the allowed font size for the text part. Setting min and max to 0
        means we won't restrict the sizing (default).
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

   ed = (Edje_Part_Description_Text*) current_desc;

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

   ed = (Edje_Part_Description_Text*) current_desc;

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
        the container to be equal to the minimum size of the text. The default
        value is "0 0".
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

   ed = (Edje_Part_Description_Text*)current_desc;

   ed->text.min_x = parse_bool(0);
   ed->text.min_y = parse_bool(1);
}

/**
    @page edcref

    @property
        max
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the maximum size of
        the container to be equal to the maximum size of the text. The default
        value is "0 0".
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

   ed = (Edje_Part_Description_Text*) current_desc;

   ed->text.max_x = parse_bool(0);
   ed->text.max_y = parse_bool(1);
}

/**
    @page edcref

    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the position of the point of balance inside the container. The
        default value is 0.5 0.5.
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

   ed = (Edje_Part_Description_Text*) current_desc;

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

   ed = (Edje_Part_Description_Text*) current_desc;

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

   ed = (Edje_Part_Description_Text*) current_desc;

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
        resize that is smaller than the text itself. The default value is 0.0.
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

   ed = (Edje_Part_Description_Text*) current_desc;

   ed->text.ellipsis = parse_float_range(0, -1.0, 1.0);
}

/**
    @page edcref

    @property
        filter
    @parameters
        [filter program as a string]
    @effect
        Applies a series of filtering operations to the text.
        EXPERIMENTAL FEATURE. TO BE DOCUMENTED.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_filter(void)
{
   Edje_Part_Description_Text *ed;
   Eina_List *sources = NULL;
   Eina_Stringshare *name;
   char *token, *code;
   Eina_Bool valid = EINA_TRUE;
   Edje_Part_Collection *pc;

   static int part_key = 0;

   static const char *allowed_name_chars =
         "abcdefghijklmnopqrstuvwxyzABCDEFGHJIKLMNOPQRSTUVWXYZ0123456789_";

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_TEXT)
     {
        ERR("parse error %s:%i. text attributes in non-TEXT part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Text*) current_desc;
   pc = eina_list_data_get(eina_list_last(edje_collections));
   if (ed->text.filter.str)
     {
        EINA_LIST_FREE(ed->text.filter_sources, name)
          {
             part_lookup_delete(pc, name, &part_key, NULL);
             eina_stringshare_del(name);
          }
        free((void*)ed->text.filter.str);
     }
   ed->text.filter_sources = NULL;

   ed->text.filter.str = parse_str(0);
   if (!ed->text.filter.str) return;

   // Parse list of buffers that have a source
   // note: does not support comments
   code = strdup(ed->text.filter.str);
   for (token = strtok(code, ";"); token; token = strtok(NULL, ";"))
     {
        size_t len;

        len = strspn(token, " \n\t");
        token += len;

        if (!strncasecmp("buffer", token, 6))
          {
             // note: a valid string won't necessary compile at runtime

             token = strchr(token, ':');
             if (!token)
               {
                  valid = EINA_FALSE;
                  break;
               }
             token = strchr(token, '(');
             if (!token)
               {
                  valid = EINA_FALSE;
                  break;
               }
             token = strcasestr(token, "src");
             if (!token) continue;
             token += 3;
             len = strspn(token, " =\n\t");
             if (!len || !token[len])
               {
                  valid = EINA_FALSE;
                  break;
               }
             token += len;
             len = strspn(token, allowed_name_chars);
             if (!len || !token[len])
               {
                  valid = EINA_FALSE;
                  break;
               }
             token[len] = '\0';
             name = eina_stringshare_add(token);

             sources = eina_list_append(sources, name);
             data_queue_part_lookup(pc, name, &part_key);
          }
     }
   free(code);

   if (valid) ed->text.filter_sources = sources;
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
            @li horizontal (default)
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
    @endproperty

    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the position of the point of balance inside the container. The
        default value is 0.5 0.5.
    @endproperty

    @property
        padding
    @parameters
        [horizontal] [vertical]
    @effect
        Sets the space between cells in pixels. Defaults to 0 0.
    @endproperty

    @property
        min
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the minimum size of
        the box to be equal to the minimum size of the items. The default
        value is "0 0".
    @endproperty
*/
static void st_collections_group_parts_part_description_box_layout(void)
{
   Edje_Part_Description_Box *ed;

   check_min_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box*) current_desc;

   ed->box.layout = parse_str(0);
   if (is_param(1))
     ed->box.alt_layout = parse_str(1);
}

static void st_collections_group_parts_part_description_box_align(void)
{
   Edje_Part_Description_Box *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box*) current_desc;

   ed->box.align.x = FROM_DOUBLE(parse_float_range(0, -1.0, 1.0));
   ed->box.align.y = FROM_DOUBLE(parse_float_range(1, -1.0, 1.0));
}

static void st_collections_group_parts_part_description_box_padding(void)
{
   Edje_Part_Description_Box *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_BOX)
     {
        ERR("parse error %s:%i. box attributes in non-BOX part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Box*) current_desc;

   ed->box.padding.x = parse_int_range(0, 0, 0x7fffffff);
   ed->box.padding.y = parse_int_range(1, 0, 0x7fffffff);
}

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

   ed = (Edje_Part_Description_Box*) current_desc;

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
            @li NONE (default)
            @li TABLE
            @li ITEM
    @endproperty

    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the position of the point of balance inside the container. The
        default value is 0.5 0.5.
    @endproperty

    @property
        padding
    @parameters
        [horizontal] [vertical]
    @effect
        Sets the space between cells in pixels. Defaults to 0 0.
    @endproperty

    @property
        min
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the minimum size of
        the table to be equal to the minimum size of the items. The default
        value is "0 0".
    @endproperty
*/
static void st_collections_group_parts_part_description_table_homogeneous(void)
{
   Edje_Part_Description_Table *ed;

   check_min_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table*) current_desc;

   ed->table.homogeneous = parse_enum(0,
				     "NONE", EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE,
				     "TABLE", EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE,
				     "ITEM", EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM,
				     NULL);
}

static void st_collections_group_parts_part_description_table_align(void)
{
   Edje_Part_Description_Table *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table*) current_desc;

   ed->table.align.x = FROM_DOUBLE(parse_float_range(0, -1.0, 1.0));
   ed->table.align.y = FROM_DOUBLE(parse_float_range(1, -1.0, 1.0));
}

static void st_collections_group_parts_part_description_table_padding(void)
{
   Edje_Part_Description_Table *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
        ERR("parse error %s:%i. table attributes in non-TABLE part.",
            file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Table*) current_desc;

   ed->table.padding.x = parse_int_range(0, 0, 0x7fffffff);
   ed->table.padding.y = parse_int_range(1, 0, 0x7fffffff);
}

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

   ed = (Edje_Part_Description_Proxy*) current_desc;
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

   ed = (Edje_Part_Description_Proxy*) current_desc;
   ed->proxy.source_visible = parse_bool(0);
}

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

   ed = (Edje_Part_Description_Table*) current_desc;

   ed->table.min.h = parse_bool(0);
   ed->table.min.v = parse_bool(1);
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
        [soft bodie or cloth hardness]
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
 *                 Group.Parts.Part.Description.Physics.Movement Freedom} */

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
        This enables mapping for the part. Default is 0.
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
        results in all engines and situations. Default is 1
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
        This enable alpha channel when map rendering. Default is 1.
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
        [point] [red] [green] [blue] [alpha]
    @effect
        Set the color of a vertex in the map.
        Colors will be linearly interpolated between vertex points through the map.
        The default color of a vertex in a map is white solid (255, 255, 255, 255)
        which means it will have no affect on modifying the part pixels.
        Currently only four points are supported:
         0 - Left-Top point of a part.
         1 - Right-Top point of a part.
         2 - Left-Bottom point of a part.
         3 - Right-Bottom point of a part.
    @endproperty
*/
static void
st_collections_group_parts_part_description_map_color(void)
{
   Edje_Map_Color *color;
   Edje_Map_Color tmp;
   int i;

   check_arg_count(5);

   tmp.idx = parse_int(0);
   tmp.r = parse_int_range(1, 0, 255);
   tmp.g = parse_int_range(2, 0, 255);
   tmp.b = parse_int_range(3, 0, 255);
   tmp.a = parse_int_range(4, 0, 255);

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
        return;
     }

   *color = tmp;
   current_desc->map.colors_count++;
   current_desc->map.colors =
      realloc(current_desc->map.colors,
              sizeof(Edje_Map_Color*) * current_desc->map.colors_count);
   current_desc->map.colors[current_desc->map.colors_count - 1] = color;
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
    @endproperty
*/
static void
st_collections_group_parts_part_description_perspective_focal(void)
{
   check_arg_count(1);

   current_desc->persp.focal = parse_int_range(0, 1, 0x7fffffff);
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

   ed = (Edje_Part_Description_External*) current_desc;

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
        Adds an boolean parameter for an external object. Value must be 0 or 1.
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

   pcp = eina_list_last_data_get(edje_collections);
   epp = (Edje_Part_Parser*)current_part;

   ob_collections_group_programs_program();
   _edje_program_remove((Edje_Part_Collection*)pcp, current_program);
   el = mem_alloc(SZ(Edje_Part_Description_Link));
   el->pr = current_program;
   el->ed = current_desc;
   el->epp = epp;
   pcp->links = eina_list_append(pcp->links, el);
   current_program->action = EDJE_ACTION_TYPE_STATE_SET;
   current_program->state = strdup(current_desc->state.name ?: "default");
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
       (el->ed != current_desc) || (el->epp != (Edje_Part_Parser*)current_part) ||
       el->pr->source)
     ob_collections_group_parts_part_description_link();
   el = eina_list_last_data_get(pcp->links);

   check_min_arg_count(1);
   name = parse_str(0);
   if (current_program->signal && pcp->link_hash)
     {
        snprintf(buf, sizeof(buf), "%s\"\"\"%s", current_program->signal, current_program->source ?: "");
        eina_hash_list_remove(pcp->link_hash, buf, el);
     }
   if (!pcp->link_hash)
     pcp->link_hash = eina_hash_string_superfast_new((Eina_Free_Cb)eina_list_free);
   free((void*)current_program->signal);
   current_program->signal = name;
   if (get_arg_count() == 2)
     {
        name = parse_str(1);
        free((void*)current_program->source);
        current_program->source = name;
     }
   snprintf(buf, sizeof(buf), "%s\"\"\"%s", current_program->signal, current_program->source ?: "");
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
        if (!strcmp(name, (char*) (pa2 + 1)))
          return;
     }

   pa = mem_alloc(SZ(Edje_Program_After) + strlen(name) + 1);
   pa->id = -1;
   ep->after = eina_list_append(ep->after, pa);
   copy = (char*)(pa + 1);
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
   ep->tween.mode = EDJE_TWEEN_MODE_LINEAR;
   ep->after = NULL;
   epp = (Edje_Program_Parser *)ep;
   epp->can_override = EINA_FALSE;

   /* generate new name */
   def_name = alloca(strlen("program_") + strlen("0xFFFFFFFFFFFFFFFF") + 1);
   sprintf(def_name, "program_%p", ep);
   ep->name = strdup(def_name);
   if (pcp->default_source)
     ep->source = strdup(pcp->default_source);
   _edje_program_insert(pc, ep);

   current_program = ep;
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
        keyword per program may be used. ex:source: "button-*"; (Signals from
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
   free((void*)current_program->source);
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
        Only one filter per program can be used. If [state] is not given, the source of
        the event will be used instead.
    @endproperty
*/
static void
st_collections_group_programs_program_filter(void)
{
   check_min_arg_count(1);

   _program_sequence_check();

   if(is_param(1)) {
	   current_program->filter.part = parse_str(0);
	   current_program->filter.state = parse_str(1);
   } else {
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
        @li FOCUS_SET
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
   else if (ep->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
	ep->state = parse_str(1);
	ep->state2 = parse_str(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_SOUND_SAMPLE)
     {
        ep->sample_name = parse_str(1);
        for (i = 0; i < (int)edje_file->sound_dir->samples_count; i++)
          {
             if (!strcmp(edje_file->sound_dir->samples[i].name, ep->sample_name))
               break;
             if (i == (int)(edje_file->sound_dir->samples_count - 1))
               {
                  ERR("No Sample name %s exist.", ep->sample_name);
                  exit(-1);
               }
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
        for (i = 0; i < (int)edje_file->sound_dir->tones_count; i++)
          {
             if (!strcmp(edje_file->sound_dir->tones[i].name, ep->tone_name))
               break;
             if (i == (int)(edje_file->sound_dir->tones_count - 1))
               {
                  ERR("No Tone name %s exist.", ep->tone_name);
                  exit(-1);
               }
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
      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
      case EDJE_ACTION_TYPE_FOCUS_SET:
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
   char *tmp = NULL;

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
   if ((current_program->tween.mode >= EDJE_TWEEN_MODE_LINEAR) &&
       (current_program->tween.mode <= EDJE_TWEEN_MODE_DECELERATE))
     {
        tmp = NULL;
        if ((get_arg_count() == 3) && (!strcmp((tmp = parse_str(2)), "CURRENT")))
          {
             free(tmp);
             current_program->tween.mode |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (get_arg_count() != 2)
          {
             free(tmp);
             ERR("parse error %s:%i. Need 2rd parameter to set time",
                 file_in, line - 1);
             exit(-1);
          }
     }
   // the following need v1
   // EDJE_TWEEN_MODE_ACCELERATE_FACTOR
   // EDJE_TWEEN_MODE_DECELERATE_FACTOR
   // EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR
   // current_program->tween.v1
   else if ((current_program->tween.mode >= EDJE_TWEEN_MODE_ACCELERATE_FACTOR) &&
       (current_program->tween.mode <= EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR))
     {
        tmp = NULL;
        if ((get_arg_count() == 4) && (!strcmp((tmp = parse_str(3)), "CURRENT")))
          {
             free(tmp);
             current_program->tween.mode |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (get_arg_count() != 3)
          {
             free(tmp);
	     ERR("parse error %s:%i. Need 3rd parameter to set factor",
		 file_in, line - 1);
	     exit(-1);
          }
        current_program->tween.v1 = FROM_DOUBLE(parse_float_range(2, 0.0, 999999999.0));
     }
   // the followjng also need v2
   // EDJE_TWEEN_MODE_DIVISOR_INTERP
   // EDJE_TWEEN_MODE_BOUNCE
   // EDJE_TWEEN_MODE_SPRING
   // current_program->tween.v2
   else if ((current_program->tween.mode >= EDJE_TWEEN_MODE_DIVISOR_INTERP) &&
            (current_program->tween.mode <= EDJE_TWEEN_MODE_SPRING))
     {
        tmp = NULL;
        if ((get_arg_count() == 5) && (!strcmp((tmp = parse_str(4)), "CURRENT")))
          {
             free(tmp);
             current_program->tween.mode |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (get_arg_count() != 4)
          {
             free(tmp);
	     ERR("parse error %s:%i. "
		 "Need 3rd and 4th parameters to set factor and counts",
		 file_in, line - 1);
	     exit(-1);
          }
        current_program->tween.v1 = FROM_DOUBLE(parse_float_range(2, 0.0, 999999999.0));
        current_program->tween.v2 = FROM_DOUBLE(parse_float_range(3, 0.0, 999999999.0));
     }
   else if (current_program->tween.mode == EDJE_TWEEN_MODE_CUBIC_BEZIER)
     {
        tmp = NULL;
        if ((get_arg_count() == 7) && (!strcmp((tmp = parse_str(4)), "CURRENT")))
          {
             free(tmp);
             current_program->tween.mode |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (get_arg_count() != 6)
          {
             free(tmp);
             ERR("parse error %s:%i. "
             "Need 3rd, 4th, 5th and 6th parameters to set x1, y1, x2 and y2",
             file_in, line - 1);
             exit(-1);
          }
        current_program->tween.v1 = FROM_DOUBLE(parse_float_range(2, 0.0, 999999999.0));
        current_program->tween.v2 = FROM_DOUBLE(parse_float_range(3, 0.0, 999999999.0));
        current_program->tween.v3 = FROM_DOUBLE(parse_float_range(4, 0.0, 999999999.0));
        current_program->tween.v4 = FROM_DOUBLE(parse_float_range(5, 0.0, 999999999.0));
     }
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
        if (!strcmp(name, (char*) (etw + 1)))
          {
             free(name);
             return;
          }
     }

   et = mem_alloc(SZ(Edje_Program_Target) + strlen(name) + 1);
   ep->targets = eina_list_append(ep->targets, et);
   copy = (char*) (et + 1);
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
        target
    @parameters
        [target1] [target2] [target3] ...
    @effect
        Programs or parts upon which the specified action will act. Multiple target
        or targets keywords may be specified. SIGNAL_EMITs can have
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
   ((Edje_Program_Parser*)sequencing)->can_override = EINA_FALSE;
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
        ((Edje_Program_Parser*)sequencing)->can_override = EINA_TRUE;
        current_program_lookups = eina_list_free(current_program_lookups);
        current_program_lookups = sequencing_lookups;
        sequencing_lookups = NULL;
        sequencing = NULL;
     }
   else if (current_program && (!strcmp(token, "link")))
     current_program = NULL;
   else if (current_de && (!strcmp(token, "group")))
     _link_combine();
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
   info->current_part = cp;  /* current_part restored on pop */
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

static Eina_Bool
_part_text_ellipsis_check(Edje_Part *ep, Edje_Part_Description_Common *desc)
{
   Edje_Part_Description_Text *ed;

   if ((ep->type != EDJE_PART_TYPE_TEXT) && (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   ed = (Edje_Part_Description_Text*)desc;

   return ((ed->text.ellipsis != -1) && ed->text.min_x);
}

static void
edje_cc_handlers_hierarchy_pop(void)
{  /* Remove part from hierarchy stack when finished parsing it */
   Edje_Cc_Handlers_Hierarchy_Info *info = eina_array_pop(part_hierarchy);

   if (current_part)
     {
        unsigned int i;

        for (i = 0; i < current_part->other.desc_count; i++)
          {
             if (!current_part->other.desc[i]->state.name)
               {
                  ERR("syntax error near %s:%i. Non-default or inherited parts are required to have state names for all descriptions (Group '%s', part '%s' has missing description state names)",
                        file_in, line - 1, current_de->entry, current_part->name);
                    exit(-1);
               }
             if (_part_text_ellipsis_check(current_part, current_part->other.desc[i]))
               {
                  WRN("Part '%s' in group '%s' contains description '%s:%g' which has text.min: 1 X; but not text.ellipsis: -1;",
                      current_part->name, current_de->entry,
                      current_part->other.desc[i]->state.name, current_part->other.desc[i]->state.value);
                  WRN("This is almost certainly not what you want.");
               }
          }

        /* auto-add default desc if it was omitted */
        if (!current_part->default_desc)
          ob_collections_group_parts_part_description();
        else if (_part_text_ellipsis_check(current_part, current_part->default_desc))
          {
             WRN("Part '%s' in group '%s' contains description '%s:%g' which has text.min: 1 X; but not text.ellipsis: -1;",
                 current_part->name, current_de->entry,
                 current_part->default_desc->state.name, current_part->default_desc->state.value);
             WRN("This is almost certainly not what you want.");
          }
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
   if (!last) return EINA_FALSE;
   if (current_part)
     {
        if ((!strcmp(last, "part")) && (!current_part->name))
          {
             Eina_Bool ret;

             if (!had_quote) return EINA_FALSE;
             free((void*)current_part->name);
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
                       free((char*)current_desc->state.name);
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
   return EINA_FALSE;
}
