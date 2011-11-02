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
    \@endblock

    \@property
        property name
    \@parameters
        property's parameter list
    \@effect
        the property description (lol)
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

#include "edje_cc.h"

/**
 * @page edcref Edje Data Collection reference
 * An Edje Data Collection, it's a plain text file (normally identified with the
 * .edc extension),consisting of instructions for the Edje Compiler.
 *
 * The syntax for the edje data collection files follows a simple structure of
 * "blocks { .. }" that can contain "properties: ..", more blocks, or both.
 *
 * @anchor sec_quickaccess Quick access to block descriptions:
 * <ul>
 *    <li>@ref sec_toplevel "Top-Level"</li>
 *    <li>@ref sec_group "Group"</li>
 *    <li>@ref sec_description "State description"</li>
 *    <ul>
 *      <li>@ref sec_description_image "Image"</li>
 *      <li>@ref sec_description_text "Text"</li>
 *      <li>@ref sec_description_box "Box"</li>
 *      <li>@ref sec_description_table "Table"</li>
 *      <li>@ref sec_description_map "Map (3d/transformations)"</li>
 *    </ul>
 *    <li>@ref sec_program "Program block"</li>
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

static void st_externals_external(void);

static void st_images_image(void);
static void ob_images_set(void);
static void st_images_set_name(void);
static void ob_images_set_image(void);
static void st_images_set_image_image(void);
static void st_images_set_image_size(void);

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

static void ob_collections_group(void);
static void st_collections_group_name(void);
static void st_collections_group_inherit(void);
static void st_collections_group_script_only(void);
static void st_collections_group_alias(void);
static void st_collections_group_min(void);
static void st_collections_group_max(void);
static void st_collections_group_data_item(void);
static void st_collections_group_orientation(void);

static void st_collections_group_limits_vertical(void);
static void st_collections_group_limits_horizontal(void);

static void ob_collections_group_script(void);
static void ob_collections_group_lua_script(void);

static void st_collections_group_parts_alias(void);

static void ob_collections_group_parts_part(void);
static void st_collections_group_parts_part_name(void);
static void st_collections_group_parts_part_type(void);
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
static void st_collections_group_parts_part_dragable_x(void);
static void st_collections_group_parts_part_dragable_y(void);
static void st_collections_group_parts_part_dragable_confine(void);
static void st_collections_group_parts_part_dragable_events(void);

/* box and table items share these */
static void ob_collections_group_parts_part_box_items_item(void);
static void st_collections_group_parts_part_box_items_item_type(void);
static void st_collections_group_parts_part_box_items_item_name(void);
static void st_collections_group_parts_part_box_items_item_source(void);
static void st_collections_group_parts_part_box_items_item_min(void);
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
static void st_collections_group_parts_part_description_inherit(void);
static void st_collections_group_parts_part_description_source(void);
static void st_collections_group_parts_part_description_state(void);
static void st_collections_group_parts_part_description_visible(void);
static void st_collections_group_parts_part_description_align(void);
static void st_collections_group_parts_part_description_fixed(void);
static void st_collections_group_parts_part_description_min(void);
static void st_collections_group_parts_part_description_max(void);
static void st_collections_group_parts_part_description_step(void);
static void st_collections_group_parts_part_description_aspect(void);
static void st_collections_group_parts_part_description_aspect_preference(void);
static void st_collections_group_parts_part_description_rel1_relative(void);
static void st_collections_group_parts_part_description_rel1_offset(void);
static void st_collections_group_parts_part_description_rel1_to(void);
static void st_collections_group_parts_part_description_rel1_to_x(void);
static void st_collections_group_parts_part_description_rel1_to_y(void);
static void st_collections_group_parts_part_description_rel2_relative(void);
static void st_collections_group_parts_part_description_rel2_offset(void);
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
static void st_collections_group_parts_part_description_text_elipsis(void);
static void st_collections_group_parts_part_description_box_layout(void);
static void st_collections_group_parts_part_description_box_align(void);
static void st_collections_group_parts_part_description_box_padding(void);
static void st_collections_group_parts_part_description_box_min(void);
static void st_collections_group_parts_part_description_table_homogeneous(void);
static void st_collections_group_parts_part_description_table_align(void);
static void st_collections_group_parts_part_description_table_padding(void);
static void st_collections_group_parts_part_description_table_min(void);
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
static void st_collections_group_programs_program_after(void);
static void st_collections_group_programs_program_api(void);

static void ob_collections_group_programs_program_script(void);

/*****/

New_Statement_Handler statement_handlers[] =
{
     {"externals.external", st_externals_external},
     {"images.image", st_images_image},
     {"images.set.name", st_images_set_name},
     {"images.set.image.image", st_images_set_image_image},
     {"images.set.image.size", st_images_set_image_size},
     {"fonts.font", st_fonts_font},
     {"data.item", st_data_item},
     {"data.file", st_data_file},
     {"styles.style.name", st_styles_style_name},
     {"styles.style.base", st_styles_style_base},
     {"styles.style.tag", st_styles_style_tag},
     {"color_classes.color_class.name", st_color_class_name},
     {"color_classes.color_class.color", st_color_class_color},
     {"color_classes.color_class.color2", st_color_class_color2},
     {"color_classes.color_class.color3", st_color_class_color3},
     {"collections.externals.external", st_externals_external}, /* dup */
     {"collections.image", st_images_image}, /* dup */
     {"collections.set.name", st_images_set_name}, /* dup */
     {"collections.set.image.image", st_images_set_image_image}, /* dup */
     {"collections.set.image.size", st_images_set_image_size}, /* dup */
     {"collections.images.image", st_images_image}, /* dup */
     {"collections.images.set.name", st_images_set_name}, /* dup */
     {"collections.images.set.image.image", st_images_set_image_image}, /* dup */
     {"collections.images.set.image.size", st_images_set_image_size}, /* dup */
     {"collections.font", st_fonts_font}, /* dup */
     {"collections.fonts.font", st_fonts_font}, /* dup */
     {"collections.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.name", st_collections_group_name},
     {"collections.group.inherit", st_collections_group_inherit},
     {"collections.group.script_only", st_collections_group_script_only},
     {"collections.group.lua_script_only", st_collections_group_script_only},
     {"collections.group.alias", st_collections_group_alias},
     {"collections.group.min", st_collections_group_min},
     {"collections.group.max", st_collections_group_max},
     {"collections.group.orientation", st_collections_group_orientation},
     {"collections.group.data.item", st_collections_group_data_item},
     {"collections.group.limits.horizontal", st_collections_group_limits_horizontal},
     {"collections.group.limits.vertical", st_collections_group_limits_vertical},
     {"collections.group.externals.external", st_externals_external}, /* dup */
     {"collections.group.image", st_images_image}, /* dup */
     {"collections.group.set.name", st_images_set_name},
     {"collections.group.set.image.image", st_images_set_image_image},
     {"collections.group.set.image.size", st_images_set_image_size},
     {"collections.group.images.image", st_images_image}, /* dup */
     {"collections.group.images.set.name", st_images_set_name},
     {"collections.group.images.set.image.image", st_images_set_image_image},
     {"collections.group.images.set.image.size", st_images_set_image_size},
     {"collections.group.font", st_fonts_font}, /* dup */
     {"collections.group.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.alias", st_collections_group_parts_alias },
     {"collections.group.parts.image", st_images_image}, /* dup */
     {"collections.group.parts.set.name", st_images_set_name},
     {"collections.group.parts.set.image.image", st_images_set_image_image},
     {"collections.group.parts.set.image.size", st_images_set_image_size},
     {"collections.group.parts.images.image", st_images_image}, /* dup */
     {"collections.group.parts.images.set.name", st_images_set_name},
     {"collections.group.parts.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.images.set.image.size", st_images_set_image_size},
     {"collections.group.parts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.parts.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.parts.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.parts.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.part.name", st_collections_group_parts_part_name},
     {"collections.group.parts.part.api", st_collections_group_parts_part_api},
     {"collections.group.parts.part.type", st_collections_group_parts_part_type},
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
     {"collections.group.parts.part.dragable.events", st_collections_group_parts_part_dragable_events},
     {"collections.group.parts.part.entry_mode", st_collections_group_parts_part_entry_mode},
     {"collections.group.parts.part.select_mode", st_collections_group_parts_part_select_mode},
     {"collections.group.parts.part.cursor_mode", st_collections_group_parts_part_cursor_mode},
     {"collections.group.parts.part.multiline", st_collections_group_parts_part_multiline},
     {"collections.group.parts.part.image", st_images_image}, /* dup */
     {"collections.group.parts.part.set.name", st_images_set_name},
     {"collections.group.parts.part.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.images.set.name", st_images_set_name},
     {"collections.group.parts.part.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.images.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.part.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.part.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.part.box.items.item.type", st_collections_group_parts_part_box_items_item_type},
     {"collections.group.parts.part.box.items.item.name", st_collections_group_parts_part_box_items_item_name},
     {"collections.group.parts.part.box.items.item.source", st_collections_group_parts_part_box_items_item_source},
     {"collections.group.parts.part.box.items.item.min", st_collections_group_parts_part_box_items_item_min},
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
     {"collections.group.parts.part.description.inherit", st_collections_group_parts_part_description_inherit},
     {"collections.group.parts.part.description.source", st_collections_group_parts_part_description_source},
     {"collections.group.parts.part.description.state", st_collections_group_parts_part_description_state},
     {"collections.group.parts.part.description.visible", st_collections_group_parts_part_description_visible},
     {"collections.group.parts.part.description.align", st_collections_group_parts_part_description_align},
     {"collections.group.parts.part.description.fixed", st_collections_group_parts_part_description_fixed},
     {"collections.group.parts.part.description.min", st_collections_group_parts_part_description_min},
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
     {"collections.group.parts.part.description.image.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.image.set.name", st_images_set_name},
     {"collections.group.parts.part.description.image.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.description.image.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.description.image.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.image.images.set.name", st_images_set_name},
     {"collections.group.parts.part.description.image.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.description.image.images.set.image.size", st_images_set_image_size},
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
     {"collections.group.parts.part.description.text.elipsis", st_collections_group_parts_part_description_text_elipsis},
     {"collections.group.parts.part.description.text.ellipsis", st_collections_group_parts_part_description_text_elipsis},
     {"collections.group.parts.part.description.box.layout", st_collections_group_parts_part_description_box_layout},
     {"collections.group.parts.part.description.box.align", st_collections_group_parts_part_description_box_align},
     {"collections.group.parts.part.description.box.padding", st_collections_group_parts_part_description_box_padding},
     {"collections.group.parts.part.description.box.min", st_collections_group_parts_part_description_box_min},
     {"collections.group.parts.part.description.table.homogeneous", st_collections_group_parts_part_description_table_homogeneous},
     {"collections.group.parts.part.description.table.align", st_collections_group_parts_part_description_table_align},
     {"collections.group.parts.part.description.table.padding", st_collections_group_parts_part_description_table_padding},
     {"collections.group.parts.part.description.table.min", st_collections_group_parts_part_description_table_min},
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
     {"collections.group.parts.part.description.perspective.zplane", st_collections_group_parts_part_description_perspective_zplane},
     {"collections.group.parts.part.description.perspective.focal", st_collections_group_parts_part_description_perspective_focal},
     {"collections.group.parts.part.description.params.int", st_collections_group_parts_part_description_params_int},
     {"collections.group.parts.part.description.params.double", st_collections_group_parts_part_description_params_double},
     {"collections.group.parts.part.description.params.string", st_collections_group_parts_part_description_params_string},
     {"collections.group.parts.part.description.params.bool", st_collections_group_parts_part_description_params_bool},
     {"collections.group.parts.part.description.params.choice", st_collections_group_parts_part_description_params_choice},
     {"collections.group.parts.part.description.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.images.set.name", st_images_set_name},
     {"collections.group.parts.part.description.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.description.images.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.description.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.part.description.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.part.description.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.part.description.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.programs.set.name", st_images_set_name},
     {"collections.group.parts.part.description.programs.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.description.programs.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.description.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.programs.images.set.name", st_images_set_name},
     {"collections.group.parts.part.description.programs.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.description.programs.images.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.description.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.description.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.description.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.description.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.description.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.description.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.description.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.description.programs.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.description.programs.program.api", st_collections_group_programs_program_api}, /* dup */
     {"collections.group.parts.part.description.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.description.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.description.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.description.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.description.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.description.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.description.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.description.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.description.program.api", st_collections_group_programs_program_api}, /* dup */
     {"collections.group.parts.part.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.part.programs.set.name", st_images_set_name},
     {"collections.group.parts.part.programs.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.programs.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.programs.images.set.name", st_images_set_name},
     {"collections.group.parts.part.programs.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.part.programs.images.set.image.size", st_images_set_image_size},
     {"collections.group.parts.part.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.programs.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.programs.program.api", st_collections_group_programs_program_api}, /* dup */
     {"collections.group.parts.part.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.program.api", st_collections_group_programs_program_api}, /* dup */
     {"collections.group.parts.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.programs.set.name", st_images_set_name},
     {"collections.group.parts.programs.set.image.image", st_images_set_image_image},
     {"collections.group.parts.programs.set.image.size", st_images_set_image_size},
     {"collections.group.parts.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.programs.images.set.name", st_images_set_name},
     {"collections.group.parts.programs.images.set.image.image", st_images_set_image_image},
     {"collections.group.parts.programs.images.set.image.size", st_images_set_image_size},
     {"collections.group.parts.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.programs.program.filter", st_collections_group_programs_program_filter}, /* dup */
     {"collections.group.parts.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.programs.program.after", st_collections_group_programs_program_after},
     {"collections.group.parts.programs.program.api", st_collections_group_programs_program_api},
     {"collections.group.parts.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.program.filter", st_collections_group_programs_program_filter}, /* dup */
     {"collections.group.parts.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.program.api", st_collections_group_programs_program_api}, /* dup */
     {"collections.group.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.program.filter", st_collections_group_programs_program_filter}, /* dup */
     {"collections.group.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.program.api", st_collections_group_programs_program_api}, /* dup */
     {"collections.group.programs.program.name", st_collections_group_programs_program_name},
     {"collections.group.programs.program.signal", st_collections_group_programs_program_signal},
     {"collections.group.programs.program.source", st_collections_group_programs_program_source},
     {"collections.group.programs.program.filter", st_collections_group_programs_program_filter}, /* dup */
     {"collections.group.programs.program.in", st_collections_group_programs_program_in},
     {"collections.group.programs.program.action", st_collections_group_programs_program_action},
     {"collections.group.programs.program.transition", st_collections_group_programs_program_transition},
     {"collections.group.programs.program.target", st_collections_group_programs_program_target},
     {"collections.group.programs.program.after", st_collections_group_programs_program_after},
     {"collections.group.programs.program.api", st_collections_group_programs_program_api},
     {"collections.group.programs.image", st_images_image}, /* dup */
     {"collections.group.programs.set.name", st_images_set_name},
     {"collections.group.programs.set.image.image", st_images_set_image_image},
     {"collections.group.programs.set.image.size", st_images_set_image_size},
     {"collections.group.programs.images.image", st_images_image}, /* dup */
     {"collections.group.programs.images.set.name", st_images_set_name},
     {"collections.group.programs.images.set.image.image", st_images_set_image_image},
     {"collections.group.programs.images.set.image.size", st_images_set_image_size},
     {"collections.group.programs.font", st_fonts_font}, /* dup */
     {"collections.group.programs.fonts.font", st_fonts_font} /* dup */
};

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
     {"collections.group", ob_collections_group},
     {"collections.group.data", NULL},
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
     {"collections.group.parts.part.description.map", NULL},
     {"collections.group.parts.part.description.map.rotation", NULL},
     {"collections.group.parts.part.description.perspective", NULL},
     {"collections.group.parts.part.description.params", NULL},
     {"collections.group.parts.part.description.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part.description.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.description.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.description.programs", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.description.programs.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.description.programs.images", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.images.set", ob_images_set},
     {"collections.group.parts.part.description.programs.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.description.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.description.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.description.script", ob_collections_group_script}, /* dup */
     {"collections.group.parts.part.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.programs", NULL}, /* dup */
     {"collections.group.parts.part.programs.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.programs.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.programs.images", NULL}, /* dup */
     {"collections.group.parts.part.programs.images.set", ob_images_set}, /* dup */
     {"collections.group.parts.part.programs.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.part.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.part.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.script", ob_collections_group_script}, /* dup */
     {"collections.group.parts.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.programs", NULL}, /* dup */
     {"collections.group.parts.programs.set", ob_images_set}, /* dup */
     {"collections.group.parts.programs.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.programs.images", NULL}, /* dup */
     {"collections.group.parts.programs.images.set", ob_images_set}, /* dup */
     {"collections.group.parts.programs.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.parts.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.script", ob_collections_group_script}, /* dup */
     {"collections.group.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.programs", NULL},
     {"collections.group.programs.set", ob_images_set}, /* dup */
     {"collections.group.programs.set.image", ob_images_set_image}, /* dup */
     {"collections.group.programs.images", NULL}, /* dup */
     {"collections.group.programs.images.set", ob_images_set}, /* dup */
     {"collections.group.programs.images.set.image", ob_images_set_image}, /* dup */
     {"collections.group.programs.fonts", NULL}, /* dup */
     {"collections.group.programs.program", ob_collections_group_programs_program},
     {"collections.group.programs.program.script", ob_collections_group_programs_program_script},
     {"collections.group.programs.script", ob_collections_group_script} /* dup */
};

/*****/

int
object_handler_num(void)
{
   return sizeof(object_handlers) / sizeof (New_Object_Handler);
}

int
statement_handler_num(void)
{
   return sizeof(statement_handlers) / sizeof (New_Object_Handler);
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

static Edje_Part_Description_Common *
_edje_part_description_alloc(unsigned char type, const char *collection, const char *part)
{
   Edje_Part_Description_Common *result = NULL;

   switch (type)
     {
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
	ERR("%s: Error. Unknown type %i of part %s in collection %s.", progname, type, part, collection);
	exit(-1);
     }

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
                 ERR("%s: Error. parse error %s:%i. There is already a program of the name %s\n",
                     progname, file_in, line - 1, name);
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

   EINA_LIST_FOREACH(ep2->targets, l, et2)
     {
        name = (char*) (et2 + 1);
        et = mem_alloc(SZ(Edje_Program_Target) + strlen(name) + 1);
        ep->targets = eina_list_append(ep->targets, et);
        copy = (char*) (et + 1);

        memcpy(copy, name, strlen(name) + 1);

        if (ep2->action == EDJE_ACTION_TYPE_STATE_SET)
          data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
        else if (ep2->action == EDJE_ACTION_TYPE_ACTION_STOP)
          data_queue_copied_program_lookup(pc, &(et2->id), &(et->id));
        else if (ep2->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
          data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
        else if (ep2->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
          data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
        else if (ep2->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
          data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
        else if (ep2->action == EDJE_ACTION_TYPE_FOCUS_SET)

          data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
        else if (ep2->action == EDJE_ACTION_TYPE_FOCUS_OBJECT)
          data_queue_copied_part_lookup(pc, &(et2->id), &(et->id));
        else
          {
             ERR("%s: Error. parse error %s:%i. "
                 "target may only be used after action",
                 progname, file_in, line - 1);
             exit(-1);
          }
     }

   EINA_LIST_FOREACH(ep2->after, l, pa2)
     {
        pa = mem_alloc(SZ(Edje_Program_After));
        ep->after = eina_list_append(ep->after, pa);

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

/**
   @edcsection{toplevel,Top-Level blocks}
 */

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
	edje_file->external_dir->entries_count++;
	edje_file->external_dir->entries = realloc(edje_file->external_dir->entries,
						   sizeof (Edje_External_Directory) * edje_file->external_dir->entries_count);
	memset(edje_file->external_dir->entries + edje_file->external_dir->entries_count - 1,
	       0, sizeof (Edje_External_Directory));
	if (!edje_file->external_dir->entries)
	  {
	     ERR("%s: Error. not enough memory", progname);
	     exit(-1);
	  }

	edje_file->external_dir->entries[edje_file->external_dir->entries_count - 1].entry = mem_strdup(ex->name);
     }
}

/**
    @page edcref

    @block
        images
    @context
        images {
            image: "filename1.ext" COMP;
            image: "filename2.ext" LOSSY 99;
	    set {
	       name: "image_name_used";
               image {
                  image: "filename3.ext" LOSSY 90;
                  size: 201 201 500 500;
               }
               image {
                  image: "filename4.ext" COMP;
                  size: 51 51 200 200;
               }
               image {
                  image: "filename5.ext" COMP;
                  size: 11 11 50 50;
               }
               image {
                  image: "filename6.ext" RAW;
                  size: 0 0 10 10;
               }
            }
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
        @li LOSSY [0-100]: Lossy comression with quality from 0 to 100.
        @li USER: Do not embed the file, refer to the external file instead.
    @endproperty
 */
static void
st_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   const char *tmp;
   unsigned int i;
   int v;

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
   edje_file->image_dir->entries = realloc(edje_file->image_dir->entries,
					   sizeof (Edje_Image_Directory_Entry) * edje_file->image_dir->entries_count);
   memset(edje_file->image_dir->entries + edje_file->image_dir->entries_count - 1,
	  0, sizeof (Edje_Image_Directory_Entry));
   if (!edje_file->image_dir->entries)
     {
	ERR("%s: Error. No enough memory.", progname);
	exit(-1);
     }

   img = edje_file->image_dir->entries + edje_file->image_dir->entries_count - 1;

   img->entry = tmp;
   img->id = edje_file->image_dir->entries_count - 1;
   v = parse_enum(1,
		  "RAW", 0,
		  "COMP", 1,
		  "LOSSY", 2,
		  "USER", 3,
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
	img->source_type = EDJE_IMAGE_SOURCE_TYPE_EXTERNAL;
	img->source_param = 0;
     }
   if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY)
	check_arg_count(2);
   else
     {
	img->source_param = parse_int_range(2, 0, 100);
	check_arg_count(3);
     }
}

/**
    @page edcref

    @block
        set
    @context
    set {
       name: "image_name_used";
       image {
          image: "filename3.ext" LOSSY 90;
          size: 201 201 500 500;
       }
       image {
          image: "filename4.ext" COMP;
          size: 51 51 200 200;
       }
       image {
          image: "filename5.ext" COMP;
          size: 11 11 50 50;
       }
       image {
          image: "filename6.ext" RAW;
          size: 0 0 10 10;
       }
    }
    @description
        The "set" block is used to define an image with different content depending on their size.
        Besides the document's root, additional "set" blocks can be
        included inside other blocks, normally "collections", "group" and
        "part", easing maintenance of the file list when the theme is split
        among multiple files.
    @endblock
 */
static void
ob_images_set(void)
{
   if (!edje_file->image_dir)
     edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));

   edje_file->image_dir->sets_count++;
   edje_file->image_dir->sets = realloc(edje_file->image_dir->sets,
					sizeof (Edje_Image_Directory_Set) * edje_file->image_dir->sets_count);
   memset(edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1,
	  0, sizeof (Edje_Image_Directory_Set));
   if (!edje_file->image_dir->sets)
     {
	ERR("%s: Error. Not enough memory.", progname);
	exit(-1);
     }
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

static void
ob_images_set_image(void)
{
   Edje_Image_Directory_Set_Entry *entry;
   Edje_Image_Directory_Set *set;

   set = edje_file->image_dir->sets + edje_file->image_dir->sets_count - 1;

   entry = mem_alloc(SZ(Edje_Image_Directory_Set_Entry));

   set->entries = eina_list_append(set->entries, entry);
}

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
        [minw minh maxw mawh]
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
       ERR("%s: Error. parse error %s:%i. Image min and max size are not in the right order ([%i, %i] < [%i, %i])",
	   progname, file_in, line - 1,
	   entry->size.min.w, entry->size.min.h,
	   entry->size.max.w, entry->size.max.h);
       exit(-1);
     }
}

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
   Font *fn;

   check_arg_count(2);

   if (!edje_file->fonts)
     edje_file->fonts = eina_hash_string_small_new(free);

   fn = mem_alloc(SZ(Font));
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

   /* FIXME: check if data already exist */
   eina_hash_direct_add(edje_file->data, key, es);
}

/**
    @page edcref
    @property
        file
    @parameters
        [parameter name] [parameter filename]
    @effect
        Defines a new parameter , the value will be the contents of the
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
        ERR("%s: Error. %s:%i when opening file \"%s\": \"%s\"",
	    progname, file_in, line, filename, strerror(errno));
        exit(-1);
     }

   if (fstat(fd, &buf))
     {
        ERR("%s: Error. %s:%i when stating file \"%s\": \"%s\"",
	    progname, file_in, line, filename, strerror(errno));
        exit(-1);
     }

   data = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (data == MAP_FAILED)
     {
        ERR("%s: Error. %s:%i when mapping file \"%s\": \"%s\"",
	    progname, file_in, line, filename, strerror(errno));
        exit(-1);
     }

   over = data;
   for (i = 0; i < buf.st_size; ++i, ++over)
     if (*over == '\0')
       {
          ERR("%s: Error. %s:%i file \"%s\" is a binary file.",
                  progname, file_in, line, filename);
          exit(-1);
       }

   value = malloc(sizeof (char) * buf.st_size + 1);
   snprintf(value, buf.st_size + 1, "%s", data);

   munmap((void*)data, buf.st_size);
   close(fd);

   es->str = value;

   eina_hash_direct_add(edje_file->data, key, es);

   free(filename);
}

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
	     fprintf(stderr, "%s: Error. parse error %s:%i. There is already a color class named \"%s\"\n",
		     progname, file_in, line - 1, cc->name);
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
        Used as shadow in text and textblock parts.
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
        Used as outline in text and textblock parts.
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
   Edje_Style *stl, *tstl;
   Eina_List *l;

   stl = eina_list_data_get(eina_list_last(edje_file->styles));
   stl->name = parse_str(0);
   EINA_LIST_FOREACH(edje_file->styles, l, tstl)
     {
	if (stl->name && tstl->name && (stl != tstl) && (!strcmp(stl->name, tstl->name)))
	  {
	     ERR("%s: Error. parse error %s:%i. There is already a style named \"%s\"",
		 progname, file_in, line - 1, stl->name);
	     exit(-1);
	  }
     }
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
	ERR("%s: Error. parse error %s:%i. There is already a basic format for the style",
	    progname, file_in, line - 1);
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

/**
    @page edcref
    @block
        collections
    @context
        collections {
            ..
            group { }
            group { }
            ..
        }
    @description
        The "collections" block is used to list the groups that compose the
        theme. Additional "collections" blocks do not prevent overriding group
        names.
    @endblock
*/
static void
ob_collections(void)
{
   if (!edje_file->collection)
     edje_file->collection = eina_hash_string_small_new(NULL);
}

/**
   @edcsection{group,Group sub blocks}
 */

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

                data { }
                script { }
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
   Code *cd;

   if (current_de && !current_de->entry)
     {
	ERR("%p: Error. A collection without a name was detected, that's not allowed.", progname);
	exit(-1);
     }

   current_de = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   current_de->id = eina_list_count(edje_collections);

   pc = mem_alloc(SZ(Edje_Part_Collection));
   edje_collections = eina_list_append(edje_collections, pc);
   pc->id = current_de->id;

   cd = mem_alloc(SZ(Code));
   codes = eina_list_append(codes, cd);
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
   Edje_Part_Collection_Directory_Entry *older;
   Edje_Part_Collection *current_pc;

   check_arg_count(1);

   current_pc = eina_list_data_get(eina_list_last(edje_collections));

   current_de->entry = parse_str(0);
   current_pc->part = current_de->entry;

   older = eina_hash_find(edje_file->collection, current_de->entry);

   if (older)
     {
	Edje_Part_Collection *pc;
	Eina_List *l;
	Code *cd;
	int i = 0;

	pc = eina_list_nth(edje_collections, older->id);
	cd = eina_list_nth(codes, older->id);

	eina_hash_del(edje_file->collection, current_de->entry, older);
	edje_collections = eina_list_remove(edje_collections, pc);
	codes = eina_list_remove(codes, cd);

	EINA_LIST_FOREACH(edje_collections, l, pc)
	  {
	     older = eina_hash_find(edje_file->collection, pc->part);

	     pc->id = i++;
	     if (older) older->id = pc->id;
	  }
     }

   eina_hash_direct_add(edje_file->collection, current_de->entry, current_de);
}

typedef struct _Edje_List_Foreach_Data Edje_List_Foreach_Data;
struct _Edje_List_Foreach_Data
{
   Eina_List *list;
};

static Eina_Bool
_edje_data_item_list_foreach(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, strdup(key));

   return EINA_TRUE;
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
    @endproperty
    @since 1.1.0
*/
static void
st_collections_group_inherit(void)
{
   Edje_Part_Collection *pc, *pc2;
   Edje_Part *ep, *ep2;
   Edje_Part_Parser *epp, *epp2;
   Edje_Pack_Element *item, *item2;
   Edje_Pack_Element_Parser *pitem;
   Edje_Part_Description_Common *ed, *ed2;
   Edje_List_Foreach_Data fdata;
   Edje_String *es;
   Eina_List *l;
   char *parent_name;
   unsigned int i, j;

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
        ERR("%s: Error. parse error %s:%i. There isn't a part of the name %s",
            progname, file_in, line - 1, parent_name);
        exit(-1);
     }

   if (pc2->data)
     {
        char *key;

        memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
        eina_hash_foreach(pc2->data,
                     _edje_data_item_list_foreach, &fdata);

        if (!pc->data)
          pc->data = eina_hash_string_small_new(free);


        EINA_LIST_FREE(fdata.list, key)
          {
             es = mem_alloc(SZ(Edje_String));
             es = (Edje_String *)eina_hash_find(pc2->data, key);
             eina_hash_direct_add(pc->data, key, es);
          }
     }

   pc->prop.min.w = pc2->prop.min.w;
   pc->prop.min.h = pc2->prop.min.h;
   pc->prop.orientation = pc2->prop.orientation;

   pc->lua_script_only = pc2->lua_script_only;

   #define STRDUP(x) x ? strdup(x) : NULL
   for (i = 0 ; i < pc2->parts_count ; i++)
     {
        // copy the part
        ob_collections_group_parts_part();
        ep = pc->parts[i];
        ep2 = pc2->parts[i];
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
        ep->dragable.x = ep2->dragable.x;
        ep->dragable.step_x = ep2->dragable.step_x;
        ep->dragable.count_x = ep2->dragable.count_x;
        ep->dragable.y = ep2->dragable.y;
        ep->dragable.step_y = ep2->dragable.step_y;
        ep->dragable.count_y = ep2->dragable.count_y;

        data_queue_copied_part_lookup(pc, &(ep2->dragable.confine_id), &(ep->dragable.confine_id));
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
        ed->state.name = STRDUP(ed2->state.name);
        st_collections_group_parts_part_description_inherit();
        parent_desc = NULL;

        // copy other description
        for (j = 0 ; j < ep2->other.desc_count ; j++)
          {
             ob_collections_group_parts_part_description();
             ed = ep->other.desc[j];
             parent_desc = ed2 = ep2->other.desc[j];
             ed->state.name = STRDUP(ed2->state.name);
             st_collections_group_parts_part_description_inherit();
             parent_desc = NULL;
          }
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
        cd->is_lua = cd2->is_lua;
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

   check_arg_count(1);

   alias = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   alias->id = current_de->id;
   alias->entry = parse_str(0);

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
		  ERR("%s: Error. parse error %s:%i. There is already an existing script section for the group",
		      progname, file_in, line - 1);
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
		  ERR("%s: Error. parse error %s:%i. There is already an existing script section for the group",
		      progname, file_in, line - 1);
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

/**
    @page edcref
    @property
        orientation
    @parameters
    enum AUTO, LTR, RTL
    @effect
        This defines GROUP orientation.
        This is useful if you want match interface orientation with language.
        AUTO  - Follow system defs.
        LTR  - suitable for Left To Right Languages (latin)
        RTL - suitable for Right To Left Languages (Hebrew, Arabic interface)
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
	group {
            limits {
	        vertical: "limit_name" height_barrier;
		..
	    }
	}
    @parameters
       [name] [height barrier]
    @effect
        This defines when to trigger some even when the Edje object is resized.
	It will send a signal: "limit,name,over" when the object is resized and pass
	the limit by growing over it. And it will send: "limit,name,below" when
	it pass below that limit.
    @endproperty
*/
static void
st_collections_group_limits_vertical(void)
{
   Edje_Part_Collection *pc;
   Edje_Limit *el;

   check_arg_count(2);

   el = mem_alloc(SZ(Edje_Limit));

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->limits.vertical_count++;
   pc->limits.vertical = realloc(pc->limits.vertical, pc->limits.vertical_count * sizeof (Edje_Limit *));
   if (!pc->limits.vertical || el)
     {
        ERR("%s: Error. Not enough memory.", progname);
        exit(-1);
     }

   pc->limits.vertical[pc->limits.vertical_count - 1] = el;

   el->name = parse_str(0);
   el->value = parse_int_range(1, 1, 0xffff);
}

/**
    @page edcref
    @property
	group {
            limits {
	        horizontal: "limit_name" width_barrier;
		..
	    }
	}
    @parameters
       [name] [width barrier]
    @effect
        This defines when to trigger some signal when the Edje object is resized.
	It will send a signal: "limit,name,over" when the object is resized and pass
	the limit by growing over it. And it will send: "limit,name,below" when
	it pass below that limit.
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
   if (!pc->limits.horizontal || el)
     {
        ERR("%s: Error. Not enough memory.", progname);
        exit(-1);
     }

   pc->limits.horizontal[pc->limits.horizontal_count - 1] = el;

   el->name = parse_str(0);
   el->value = parse_int_range(1, 1, 0xffff);
}

/**
    @page edcref
    @block
        parts
    @context
        group {
            parts {
                alias: "theme_part_path" "real_part_path";
                ..
            }
        }
    @description
        Alias of part give a chance to let the designer put the real one
	in a box or reuse one from a GROUP or inside a BOX.
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

                    description { }
                    dragable { }
                    items { }
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
static void
ob_collections_group_parts_part(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Parser *epp;

   ep = mem_alloc(SZ(Edje_Part_Parser));

   pc = eina_list_data_get(eina_list_last(edje_collections));
   pc->parts_count++;
   pc->parts = realloc(pc->parts, pc->parts_count * sizeof (Edje_Part *));
   if (!pc->parts)
     {
	ERR("%s: Error. Not enough memory.", progname);
	exit(-1);
     }
   current_part = pc->parts[pc->parts_count - 1] = ep;

   ep->id = pc->parts_count - 1;
   ep->type = EDJE_PART_TYPE_IMAGE;
   ep->mouse_events = 1;
   ep->repeat_events = 0;
   ep->ignore_flags = EVAS_EVENT_FLAG_NONE;
   ep->scale = 0;
   ep->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
   ep->precise_is_inside = 0;
   ep->use_alternate_font_metrics = 0;
   ep->clip_to_id = -1;
   ep->dragable.confine_id = -1;
   ep->dragable.event_id = -1;
   ep->items = NULL;

   epp = (Edje_Part_Parser *)ep;
   epp->reorder.insert_before = NULL;
   epp->reorder.insert_after = NULL;
   epp->reorder.before = NULL;
   epp->reorder.after = NULL;
   epp->reorder.linked_prev = 0;
   epp->reorder.linked_next = 0;
   epp->reorder.done = EINA_FALSE;
   epp->can_override = EINA_FALSE;
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
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Parser *epp;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_part;
   ep->name = parse_str(0);

   if (ep->name)
     {
        unsigned int i;

        for (i = 0; i < (pc->parts_count - 1); i++)
          {
             if (pc->parts[i]->name && (!strcmp(pc->parts[i]->name, ep->name)))
               {
                  epp = (Edje_Part_Parser *)pc->parts[i];
                  if (!epp->can_override)
                    {
                       ERR("%s: Error. parse error %s:%i. There is already a part of the name %s",
                           progname, file_in, line - 1, ep->name);
                       exit(-1);
                    }
                  else
                    {
                       free(ep);
                       pc->parts_count--;
                       pc->parts = realloc(pc->parts, pc->parts_count * sizeof (Edje_Part *));
                       ep = current_part = pc->parts[i];
                       epp->can_override = EINA_FALSE;
                    }
               }
          }
     }
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
    @endproperty
*/
static void
st_collections_group_parts_part_type(void)
{
   check_arg_count(1);

   current_part->type = parse_enum(0,
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
                                   NULL);

   if (current_part->default_desc || current_part->other.desc_count > 0)
     {
        ERR("%s: Error. parse error %s:%i. You can't change the part type in inherited group. (part: %s)",
            progname, file_in, line - 1, current_part->name);
        exit(-1);
     }
}

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
    @endproperty
    @since 1.1.0
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
    @endproperty
    @since 1.1.0
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
        any type of signal at all. It's set to 1 by default.
    @endproperty
*/
static void
st_collections_group_parts_part_mouse_events(void)
{
   check_arg_count(1);

   current_part->mouse_events = parse_bool(0);
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
   check_arg_count(1);

   current_part->scale = parse_bool(0);
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

/**
    @page edcref
    @property
        use_alternate_font_metrics
    @parameters
        [1 or 0]
    @effect
        Only affects text and textblock parts, when enabled Edje will use
        different size measurement functions. Disabled by default. (note from
        the author: I don't know what this is exactlu useful for?)
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
        container. Overflowing content will not be displayed.
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
}

/**
    @page edcref

    @property
        effect
    @parameters
        [EFFECT]
        (optional) [SHADOW DIRECTION]
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
        [MODE]
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
        [MODE]
    @effect
        Sets the selection mode for a textblock part to one of:
        @li DEFAULT
        @li EXPLICIT
        DEFAULT selection mode is what you would expect on any desktop. Press
        mouse, drag and release to end. EXPLICIT mode requires the application
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
        [MODE]
    @effect
        Sets the cursor mode for a textblock part to one of:
        @li UNDER
        @li BEFORE
        UNDER cursor mode means the cursor will draw below the character pointed
        at. That's the default.
        BEFORE cursor mode means the cursor is drawn as a vertical line before
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
    @block
        dragable
    @context
        part {
            ..
            dragable {
                confine: "another part";
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
        The third parameter, (question from the author: What is count for?).
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
        The third parameter, (question from the author: What is count for?).
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
    @block
        items
    @context
        part {
            ..
	    box {
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
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Pack_Element *item;
   Edje_Pack_Element_Parser *pitem;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_part;

   if ((ep->type != EDJE_PART_TYPE_BOX) && (ep->type != EDJE_PART_TYPE_TABLE))
     {
	ERR("%s: Error. parse error %s:%i. "
	    "box attributes in non-BOX or TABLE part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ep->items_count++;
   ep->items = realloc(ep->items, sizeof (Edje_Pack_Element*) * ep->items_count);
   if (!ep->items)
     {
	ERR("%s: Error. Not enough memory.", progname);
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
   pitem = (Edje_Pack_Element_Parser *)item;
   pitem->can_override = EINA_FALSE;
}

/**
    @page edcref
    @property
        type
    @parameters
        Only GROUP for now (defaults to it)
    @effect
        Sets the type of the object this item will hold.
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_type(void)
{
   check_arg_count(1);

     {
	char *s;

	s = parse_str(0);
	if (strcmp(s, "GROUP"))
	  {
	     ERR("%s: Error. parse error %s:%i. "
		 "token %s not one of: GROUP.",
		 progname, file_in, line - 1, s);
	     exit(-1);
	  }
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
                       ERR("%s: Error. parse error %s:%i. There is already a item of the name %s",
                           progname, file_in, line - 1, item->name);
                       exit(-1);
                    }
                  else
                    {
                       free(item);
                       ep->items_count--;
                       ep->items = realloc(ep->items, ep->items_count * sizeof (Edje_Pack_Element *));
                       current_item = ep->items[i];
                       pitem->can_override = EINA_FALSE;
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
   check_arg_count(1);

   current_item->source = parse_str(0);
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
   check_arg_count(2);

   current_item->min.w = parse_int_range(0, 0, 0x7ffffff);
   current_item->min.h = parse_int_range(1, 0, 0x7ffffff);
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
   check_arg_count(2);

   current_item->aspect.w = parse_int_range(0, 0, 0x7fffffff);
   current_item->aspect.h = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        aspect_mode
    @parameters
        NONE, NEITHER, HORIZONTAL, VERTICAL, BOTH
    @effect
        Sets the aspect control hints for this object.
    @endproperty
*/
static void st_collections_group_parts_part_box_items_item_aspect_mode(void)
{
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
   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "table attributes in non-TABLE part.",
	    progname, file_in, line - 1);
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
   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "table attributes in non-TABLE part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   current_item->colspan = parse_int_range(0, 1, 0xffff);
   current_item->rowspan = parse_int_range(1, 1, 0xffff);
}

/**
   @edcsection{description,State description sub blocks}
 */

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
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description_Common *ed;

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_part;

   ed = _edje_part_description_alloc(ep->type, pc->part, ep->name);

   if (!ep->default_desc)
     {
	current_desc = ep->default_desc = ed;
     }
   else
     {
	ep->other.desc_count++;
	ep->other.desc = realloc(ep->other.desc,
				 sizeof (Edje_Part_Description_Common*) * ep->other.desc_count);
	current_desc = ep->other.desc[ep->other.desc_count - 1] = ed;
     }

   ed->visible = 1;
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
   ed->rel1.id_x = -1;
   ed->rel1.id_y = -1;
   ed->rel2.relative_x = FROM_DOUBLE(1.0);
   ed->rel2.relative_y = FROM_DOUBLE(1.0);
   ed->rel2.offset_x = -1;
   ed->rel2.offset_y = -1;
   ed->rel2.id_x = -1;
   ed->rel2.id_y = -1;
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
   ed->persp.zplane = 0;
   ed->persp.focal = 1000;
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

   if (!ed->state.name)
     {
        ERR("%s: Error. parse error %s:%i. "
            "inherit may only be used after state",
            progname, file_in, line - 1);
        exit(-1);
     }

   parent = parent_desc;
   if (!parent)
     {
        check_arg_count(2);

        /* inherit may not be used in the default description */
        if (!ep->other.desc_count)
          {
             ERR("%s: Error. parse error %s:%i. "
                 "inherit may not be used in the default description",
                 progname, file_in, line - 1);
             exit(-1);
          }

        /* find the description that we inherit from */
        parent_name = parse_str(0);
        parent_val = parse_float_range(1, 0.0, 1.0);

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
          }

        if (!parent)
          {
             ERR("%s: Error. parse error %s:%i. "
                 "cannot find referenced part state %s %lf",
                 ep->name, file_in, line - 1, parent_name, parent_val);
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
   switch (ep->type)
     {
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

              data_queue_copied_part_lookup(pc, &(tparent->text.id_source), &(ted->text.id_source));
              data_queue_copied_part_lookup(pc, &(tparent->text.id_text_source), &(ted->text.id_text_source));

              break;
           }
      case EDJE_PART_TYPE_IMAGE:
           {
              Edje_Part_Description_Image *ied = (Edje_Part_Description_Image *) ed;
              Edje_Part_Description_Image *iparent = (Edje_Part_Description_Image *) parent;
              unsigned int i;

              ied->image = iparent->image;

              data_queue_copied_image_lookup(&iparent->image.id, &ied->image.id, &ied->image.set);

              ied->image.tweens = calloc(iparent->image.tweens_count,
                                         sizeof (Edje_Part_Image_Id*));
              for (i = 0; i < iparent->image.tweens_count; i++)
                {
                   Edje_Part_Image_Id *iid_new;

                   iid = iparent->image.tweens[i];

                   iid_new = mem_alloc(SZ(Edje_Part_Image_Id));
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
        ERR("%s: Error. parse error %s:%i. "
            "source attributes in non-PROXY part.",
            progname, file_in, line - 1);
        exit(-1);
     }

   ed = (Edje_Part_Description_Proxy*) current_desc;

   name = parse_str(0);

   data_queue_part_lookup(pc, name, &(ed->proxy.id));
   free(name);
}

/**
    @page edcref
    @property
        state
    @parameters
        [a name for the description] [an index]
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

   check_arg_count(2);

   ep = current_part;

   ed = ep->default_desc;
   if (ep->other.desc_count) ed = ep->other.desc[ep->other.desc_count - 1];

   s = parse_str(0);
   if (!strcmp (s, "custom"))
     {
        ERR("%s: Error. parse error %s:%i. "
            "invalid state name: '%s'.",
            progname, file_in, line - 1, s);
        exit(-1);
     }

   ed->state.name = s;
   ed->state.value = parse_float_range(1, 0.0, 1.0);

   if (ed != ep->default_desc)
     {
        if ((ep->default_desc->state.name && !strcmp(s, ep->default_desc->state.name) && ed->state.value == ep->default_desc->state.value) ||
            (!ep->default_desc->state.name && !strcmp(s, "default") && ed->state.value == ep->default_desc->state.value))
          {
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
                  if (!strcmp(s, ep->other.desc[i]->state.name) && ed->state.value == ep->other.desc[i]->state.value)
                    {
                       free(ed);
                       ep->other.desc_count--;
                       ep->other.desc = realloc(ep->other.desc,
                                                sizeof (Edje_Part_Description_Common*) * ep->other.desc_count);
                       current_desc = ep->other.desc[i];
                    }
               }
          }
     }
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

   current_desc->visible = parse_bool(0);
}

/**
    @page edcref
    @property
        align
    @parameters
        [X axis] [Y axis]
    @effect
        When the displayed object's size is smaller than its container, this
        property moves it relatively along both axis inside its container. The
        default value is "0.5 0.5".
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
        [width] [height]
    @effect
        The minimum size of the state.
    @endproperty
*/
static void
st_collections_group_parts_part_description_min(void)
{
   check_arg_count(2);

   current_desc->min.w = parse_float_range(0, 0, 0x7fffffff);
   current_desc->min.h = parse_float_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height]
    @effect
        The maximum size of the state. A size of -1.0 means that it will be ignored in one direction.
    @endproperty
*/
static void
st_collections_group_parts_part_description_max(void)
{
   check_arg_count(2);

   current_desc->max.w = parse_float_range(0, -1.0, 0x7fffffff);
   current_desc->max.h = parse_float_range(1, -1.0, 0x7fffffff);
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
        width a pixel for every pixel added to heigh. The default value is
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
        options are BOTH, VERTICAL, HORIZONTAL and NONE
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
	ERR("%s: Error. Setting color3 in part %s from %s not of type TEXT or TEXTBLOCK.", progname, current_part->name, pc->part);
	exit(-1);
     }

   ed = (Edje_Part_Description_Text*)current_desc;

   ed->text.color3.r = parse_int_range(0, 0, 255);
   ed->text.color3.g = parse_int_range(1, 0, 255);
   ed->text.color3.b = parse_int_range(2, 0, 255);
   ed->text.color3.a = parse_int_range(3, 0, 255);
}

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
st_collections_group_parts_part_description_rel1_to(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel1.id_x));
      data_queue_part_lookup(pc, name, &(current_desc->rel1.id_y));
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
st_collections_group_parts_part_description_rel2_to(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_x));
      data_queue_part_lookup(pc, name, &(current_desc->rel2.id_y));
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

/**
   @edcsection{description_image,Image state description sub blocks}
 */

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
   Edje_Part_Collection *pc;
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Image*) current_desc;

   {
      char *name;

      name = parse_str(0);
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
        the order they are listed.
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_tween(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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
        0, 1, NONE, DEFAULT, SOLID
    @effect
        If border is set, this value tells Edje if the rest of the
        image (not covered by the defined border) will be displayed or not
        or be assumed to be solid (without alpha). The default is 1/DEFAULT.
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_middle(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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
        0.0 or bigger (0.0 or 1.0 to turn it off)
    @effect
        If border scaling is enabled then normally the OUTPUT border sizes
        (e.g. if 3 pixels on the left edge are set as a border, then normally
        at scale 1.0, those 3 columns will always be the exact 3 columns of
        output, or at scale 2.0 they will be 6 columns, or 0.33 they will merge
        into a single column). This property multiplies the input scale
        factor by this multiplier, allowing the creation of "supersampled"
        borders to make much higher resolution outputs possible by always using
        the highest resolution artwork and then runtime scaling it down.
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_border_scale_by(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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
        0, 1
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
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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
        0, NONE, DYNAMIC, STATIC
    @effect
        Sets the evas image scale hint letting the engine more effectively save
        cached copies of the scaled image if it makes sense
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_scale_hint(void)
{
   Edje_Part_Description_Image *ed;

   check_arg_count(1);

   if (current_part->type != EDJE_PART_TYPE_IMAGE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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

/**
    @page edcref
    @block
        fill
    @context
        description {
            ..
            fill {
                smooth: 0-1;
                origin {
                    relative: X-axis Y-axis;
                    offset:   X-axis Y-axis;
                }
                size {
                    relative: width  height;
                    offset:   width  height;
                }
            }
            ..
        }
    @description
        The fill method is an optional block that defines the way an IMAGE part
	is going to be displayed inside its container.
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
           ERR("%s: Error. parse error %s:%i. "
               "image and proxy attributes in non-IMAGE, non-PROXY `%s` part (%i).",
               progname, file_in, line - 1, current_part->name, current_part->type);
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
      ERR("%s: Error. parse error %s:%i. "
	  "fill.spread not supported yet.",
	  progname, file_in, line - 1);
      exit(-1);
   }

#if 0
   pc = eina_list_data_get(eina_list_last(edje_collections));

   ep = pc->parts[pc->parts_count - 1];

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "image attributes in non-IMAGE part.",
	    progname, file_in, line - 1);
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
        TODO
    @effect
        TODO
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
           ERR("%s: Error. parse error %s:%i. "
               "image and proxy attributes in non-IMAGE, non-PROXY part.",
               progname, file_in, line - 1);
           exit(-1);
        }
     }

   fill->type = parse_enum(0,
                           "SCALE", EDJE_FILL_TYPE_SCALE,
                           "TILE", EDJE_FILL_TYPE_TILE,
                           NULL);
}

/**
    @page edcref
    @block
        origin
    @context
        description {
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
           ERR("%s: Error. parse error %s:%i. "
               "image and proxy attributes in non-IMAGE, non-PROXY part.",
               progname, file_in, line - 1);
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
           ERR("%s: Error. parse error %s:%i. "
               "image and proxy attributes in non-IMAGE, non-PROXY part.",
               progname, file_in, line - 1);
           exit(-1);
        }
     }

   fill->pos_abs_x = parse_int(0);
   fill->pos_abs_y = parse_int(1);
}

/**
    @page edcref
    @block
        size
    @context
        description {
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
        Takes a pair of decimal values that represent the a percentual value
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
           ERR("%s: Error. parse error %s:%i. "
               "image and proxy attributes in non-IMAGE, non-PROXY part.",
               progname, file_in, line - 1);
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
           ERR("%s: Error. parse error %s:%i. "
               "image and proxy attributes in non-IMAGE, non-PROXY part.",
               progname, file_in, line - 1);
           exit(-1);
        }
     }

   fill->abs_x = parse_int(0);
   fill->abs_y = parse_int(1);
}


/**
   @edcsection{description_text,Text state description sub blocks}
 */

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
                    ellipsis:     0.0-1.0;
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
    @endproperty
    @since 1.1.0
*/
static void
st_collections_group_parts_part_description_text_size_range(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(2);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Text*) current_desc;

   ed->text.size_range_min = parse_int_range(0, 0, 255);
   ed->text.size_range_max = parse_int_range(1, 0, 255);
   if (ed->text.size_range_min > ed->text.size_range_max)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "min size is bigger than max size.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Text*) current_desc;

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(ed->text.id_source));
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
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Text*) current_desc;

   {
      char *name;

      name = parse_str(0);
      data_queue_part_lookup(pc, name, &(ed->text.id_text_source));
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
st_collections_group_parts_part_description_text_elipsis(void)
{
   Edje_Part_Description_Text *ed;

   check_arg_count(1);

   if ((current_part->type != EDJE_PART_TYPE_TEXT) &&
       (current_part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	ERR("%s: Error. parse error %s:%i. "
	    "text attributes in non-TEXT part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Text*) current_desc;

   ed->text.elipsis = parse_float_range(0, 0.0, 1.0);
}


/**
   @edcsection{description_box,Box state description sub blocks}
 */

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
        [primary layout] [fallback layout]
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
	ERR("%s: Error. parse error %s:%i. "
	    "box attributes in non-BOX part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "box attributes in non-BOX part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "box attributes in non-BOX part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "box attributes in non-BOX part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Box*) current_desc;

   ed->box.min.h = parse_bool(0);
   ed->box.min.v = parse_bool(1);
}


/**
   @edcsection{description_table,Table state description sub blocks}
 */

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
	ERR("%s: Error. parse error %s:%i. "
	    "table attributes in non-TABLE part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "table attributes in non-TABLE part.",
	    progname, file_in, line - 1);
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
	ERR("%s: Error. parse error %s:%i. "
	    "table attributes in non-TABLE part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Table*) current_desc;

   ed->table.padding.x = parse_int_range(0, 0, 0x7fffffff);
   ed->table.padding.y = parse_int_range(1, 0, 0x7fffffff);
}

static void
st_collections_group_parts_part_description_table_min(void)
{
   Edje_Part_Description_Table *ed;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_TABLE)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "box attributes in non-TABLE part.",
	    progname, file_in, line - 1);
	exit(-1);
     }

   ed = (Edje_Part_Description_Table*) current_desc;

   ed->table.min.h = parse_bool(0);
   ed->table.min.v = parse_bool(1);
}

/**
   @edcsection{description_map,Map state description sub blocks}
 */

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
        This sets the rotation around the u axis of the part considering
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


/**
   @edcsection{description_params,Params state description sub blocks}
 */

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
   const char *name;
   int found = 0;

   check_arg_count(2);

   if (current_part->type != EDJE_PART_TYPE_EXTERNAL)
     {
	ERR("%s: Error. parse error %s:%i. "
	    "params in non-EXTERNAL part.",
	    progname, file_in, line - 1);
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
	 ERR("%s: Error. parse error %s:%i. Invalid param type.\n",
	     progname, file_in, line - 1);
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


/**
   @edcsection{program, Program block}
 */

/**
    @page edcref
    @block
        program
    @context
        group {
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
   Edje_Program *ep;
   Edje_Program_Parser *epp;

   pc = eina_list_data_get(eina_list_last(edje_collections));

   ep = mem_alloc(SZ(Edje_Program_Parser));
   ep->id = -1;
   ep->tween.mode = EDJE_TWEEN_MODE_LINEAR;
   ep->after = NULL;
   epp = (Edje_Program_Parser *)ep;
   epp->can_override = EINA_FALSE;

   _edje_program_insert(pc, ep);

   current_program = ep;
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
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   current_program->name = parse_str(0);

   _edje_program_check(current_program->name, current_program, pc->programs.fnmatch, pc->programs.fnmatch_count);
   _edje_program_check(current_program->name, current_program, pc->programs.strcmp, pc->programs.strcmp_count);
   _edje_program_check(current_program->name, current_program, pc->programs.strncmp, pc->programs.strncmp_count);
   _edje_program_check(current_program->name, current_program, pc->programs.strrncmp, pc->programs.strrncmp_count);
   _edje_program_check(current_program->name, current_program, pc->programs.nocmp, pc->programs.nocmp_count);
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

   pc = eina_list_data_get(eina_list_last(edje_collections));

   _edje_program_remove(pc, current_program);
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

   current_program->in.from = parse_float_range(0, 0.0, 999999999.0);
   current_program->in.range = parse_float_range(1, 0.0, 999999999.0);
}

/**
    @page edcref
    @property
        action
    @parameters
        [type] [param1] [param2]
    @effect
        Action to be performed by the program. Valid actions are: STATE_SET,
        ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, DRAG_VAL_STEP, DRAG_VAL_PAGE,
        FOCUS_SET, PARAM_COPY, PARAM_SET
        Only one action can be specified per program. Examples:\n
           action: STATE_SET "statename" 0.5;\n
           action: ACTION_STOP;\n
           action: SIGNAL_EMIT "signalname" "emitter";\n
           action: DRAG_VAL_SET 0.5 0.0;\n
           action: DRAG_VAL_STEP 1.0 0.0;\n
           action: DRAG_VAL_PAGE 0.0 0.0;\n
           action: FOCUS_SET;\n
           action: FOCUS_OBJECT;\n
           action: PARAM_COPY "src_part" "src_param" "dst_part" "dst_param";\n
	   action: PARAM_SET "part" "param" "value";\n
    @endproperty
*/
static void
st_collections_group_programs_program_action(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = eina_list_data_get(eina_list_last(edje_collections));
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
			   NULL);
   if (ep->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	ep->state = parse_str(1);
	ep->value = parse_float_range(2, 0.0, 1.0);
     }
   else if (ep->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
	ep->state = parse_str(1);
	ep->state2 = parse_str(2);
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

   switch (ep->action)
     {
      case EDJE_ACTION_TYPE_ACTION_STOP:
	check_arg_count(1);
	break;
      case EDJE_ACTION_TYPE_SCRIPT:
	/* this is implicitly set by script {} so this is here just for
	 * completeness */
	break;
      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
      case EDJE_ACTION_TYPE_FOCUS_SET:
	check_arg_count(1);
	break;
      case EDJE_ACTION_TYPE_PARAM_COPY:
	 check_arg_count(5);
	 break;
      case EDJE_ACTION_TYPE_PARAM_SET:
	 check_arg_count(4);
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
        [type] [length] [[interp val 1]] [[interp val 2]]
    @effect
        Defines how transitions occur using STATE_SET action.\n
        Where 'type' is the style of the transition and 'length' is a double
        specifying the number of seconds in which to preform the transition.\n
        Valid types are: LIN or LINEAR, SIN or SINUSOIDAL, 
        ACCEL or ACCELERATE, DECEL or DECELERATE, 
        ACCEL_FAC or ACCELERATE_FACTOR, DECEL_FAC or DECELERATE_FACTOR,
        SIN_FAC or SINUSOIDAL_FACTOR, DIVIS or DIVISOR_INTERP,
        BOUNCE, SPRING.
        
        ACCEL_FAC, DECEL_FAC and SIN_FAC need the extra optional
        "interp val 1" to determine the "factor" of curviness. 1.0 is the same
        as their non-factor counterparts, where 0.0 is equal to linear.
        numbers higher than one make the curve angles steeper with a more
        prnounced curve point.
        
        DIVIS, BOUNCE and SPRING also require "interp val 2" in addition
        to "interp val 1".
        
        DIVIS uses val 1 as the initial graident start
        (0.0 is horizontal, 1.0 is diagonal (linear), 2.0 is twice the
        gradient of linear etc.). val 2 is interpreted as an integer factor
        defining how much the value swings "outside" the gradient only to come
        back to the final resting spot at the end. 0.0 for val 2 is equivalent
        to linear interpolation. Note that DIVIS can exceed 1.0
        
        BOUNCE uses val 2 as the number of bounces (so its rounded down to
        the nearest integer value), with val 2 determining how much the
        bounce decays, with 0.0 giving linear decay per bounce, and higher
        values giving much more decay.

        SPRING is similar to bounce, where val 2 specifies the number of
        spring "swings" and val 1 specifies the decay, but it can exceed 1.0
        on the outer swings.

    @endproperty
*/
static void
st_collections_group_programs_program_transition(void)
{
   check_min_arg_count(2);

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
   // the following need v1
   // EDJE_TWEEN_MODE_ACCELERATE_FACTOR
   // EDJE_TWEEN_MODE_DECELERATE_FACTOR
   // EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR
   // current_program->tween.v1
   if ((current_program->tween.mode >= EDJE_TWEEN_MODE_ACCELERATE_FACTOR) &&
       (current_program->tween.mode <= EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR))
     {
        if (get_arg_count() != 3)
          {
	     ERR("%s: Error. parse error %s:%i. "
		 "Need 3rd parameter to set factor",
		 progname, file_in, line - 1);
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
        if (get_arg_count() != 4)
          {
	     ERR("%s: Error. parse error %s:%i. "
		 "Need 3rd and 4th parameters to set factor and counts",
		 progname, file_in, line - 1);
	     exit(-1);
          }
        current_program->tween.v1 = FROM_DOUBLE(parse_float_range(2, 0.0, 999999999.0));
        current_program->tween.v2 = FROM_DOUBLE(parse_float_range(3, 0.0, 999999999.0));
     }
}

/**
    @page edcref
    @property
        target
    @parameters
        [target]
    @effect
        Program or part on which the specified action acts. Multiple target
        keywords may be specified, one per target. SIGNAL_EMITs do not have
        targets.
    @endproperty
*/
static void
st_collections_group_programs_program_target(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_program;
     {
	Edje_Program_Target *et;
	Edje_Program_Target *etw;
	Eina_List *l;
	char *name;
	char *copy;

	name = parse_str(0);

	et = mem_alloc(SZ(Edje_Program_Target) + strlen(name) + 1);
	ep->targets = eina_list_append(ep->targets, et);
	copy = (char*) (et + 1);

	memcpy(copy, name, strlen(name) + 1);

	if (ep->action == EDJE_ACTION_TYPE_STATE_SET)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_ACTION_STOP)
	  data_queue_program_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_FOCUS_SET)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_FOCUS_OBJECT)
	  data_queue_part_lookup(pc, name, &(et->id));
	else
	  {
	     ERR("%s: Error. parse error %s:%i. "
		 "target may only be used after action",
		 progname, file_in, line - 1);
	     exit(-1);
	  }
	EINA_LIST_FOREACH(ep->targets, l, etw)
	  if (et != etw && strcmp(name, (char*) (etw + 1)) == 0)
	    {
	      ERR("%s: Error. parse error %s:%i. "
		  "target is targetted twice",
		  progname, file_in, line - 1);
	      exit(-1);
	    }
	free(name);
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
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = eina_list_data_get(eina_list_last(edje_collections));
   ep = current_program;
     {
	Edje_Program_After *pa;
	char *name;

	name = parse_str(0);

	pa = mem_alloc(SZ(Edje_Program_After));
	pa->id = -1;
	ep->after = eina_list_append(ep->after, pa);

	data_queue_program_lookup(pc, name, &(pa->id));
	free(name);
     }
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

   current_program->api.name = parse_str(0);

   if (is_param(1))
     {
       check_arg_count(2);
       current_program->api.description = parse_str(1);
     }
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
                  ERR("%s: Error. parse error %s:%i. You're trying to mix Embryo and Lua scripting in the same group",
                      progname, file_in, line - 1);
                  exit(-1);
               }
             cd->is_lua = 0;

             len = strlen(cp->script);
             for (i = 0; i < len; i++)
               {
                  if (((cp->script[i] > 'a') && (cp->script[i] < 'z')) ||
                      ((cp->script[i] > 'A') && (cp->script[i] < 'Z')) ||
                      ((cp->script[i] > '0') && (cp->script[i] < '9')))
                    empty = EINA_FALSE;
               }

             if (!empty)
               {
                  cd->programs = eina_list_append(cd->programs, cp);
                  data_queue_anonymous_lookup(pc, current_program, &(cp->id));
                  current_program->action = EDJE_ACTION_TYPE_SCRIPT;
               }
             else
               {
                  data_queue_anonymous_lookup(pc, current_program, NULL);
                  free(cp);
                  cp = NULL;
               }

             set_verbatim(NULL, 0, 0);
          }
     }
}
/**
    @page edcref
    </table>
*/
