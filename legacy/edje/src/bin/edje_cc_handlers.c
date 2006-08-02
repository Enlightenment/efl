/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_cc.h"

static void st_images_image(void);

static void st_fonts_font(void);

static void st_data_item(void);

static void ob_styles_style(void);
static void st_styles_style_name(void);
static void st_styles_style_base(void);
static void st_styles_style_tag(void);

static void ob_color_class(void);
static void st_color_class_name(void);
static void st_color_class_color(void);
static void st_color_class_color2(void);
static void st_color_class_color3(void);

static void ob_spectrum(void);
/*static void st_spectrum(void);*/
static void st_spectrum_name(void);
static void st_spectrum_color(void);

static void ob_collections(void);

static void ob_collections_group(void);
static void st_collections_group_name(void);
static void st_collections_group_min(void);
static void st_collections_group_max(void);
static void st_collections_group_data_item(void);

static void ob_collections_group_script(void);

static void ob_collections_group_parts_part(void);
static void st_collections_group_parts_part_name(void);
static void st_collections_group_parts_part_type(void);
static void st_collections_group_parts_part_effect(void);
static void st_collections_group_parts_part_mouse_events(void);
static void st_collections_group_parts_part_repeat_events(void);
static void st_collections_group_parts_part_clip_to_id(void);
static void st_collections_group_parts_part_dragable_x(void);
static void st_collections_group_parts_part_dragable_y(void);
static void st_collections_group_parts_part_dragable_confine(void);
static void st_collections_group_parts_part_dragable_events(void);

static void ob_collections_group_parts_part_description(void);
static void st_collections_group_parts_part_description_inherit(void);
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
static void st_collections_group_parts_part_description_fill_smooth(void);
static void st_collections_group_parts_part_description_fill_origin_relative(void);
static void st_collections_group_parts_part_description_fill_origin_offset(void);
static void st_collections_group_parts_part_description_fill_size_relative(void);
static void st_collections_group_parts_part_description_fill_size_offset(void);
static void st_collections_group_parts_part_description_fill_angle(void);
static void st_collections_group_parts_part_description_fill_spread(void);
static void st_collections_group_parts_part_description_color_class(void);
static void st_collections_group_parts_part_description_color(void);
static void st_collections_group_parts_part_description_color2(void);
static void st_collections_group_parts_part_description_color3(void);
static void st_collections_group_parts_part_description_text_text(void);
static void st_collections_group_parts_part_description_text_text_class(void);
static void st_collections_group_parts_part_description_text_font(void);
static void st_collections_group_parts_part_description_text_style(void);
static void st_collections_group_parts_part_description_text_size(void);
static void st_collections_group_parts_part_description_text_fit(void);
static void st_collections_group_parts_part_description_text_min(void);
static void st_collections_group_parts_part_description_text_align(void);
static void st_collections_group_parts_part_description_text_source(void);
static void st_collections_group_parts_part_description_text_text_source(void);
static void st_collections_group_parts_part_description_text_elipsis(void);
static void st_collections_group_parts_part_description_gradient_type(void);
static void st_collections_group_parts_part_description_gradient_spectrum(void);

static void ob_collections_group_programs_program(void);
static void st_collections_group_programs_program_name(void);
static void st_collections_group_programs_program_signal(void);
static void st_collections_group_programs_program_source(void);
static void st_collections_group_programs_program_in(void);
static void st_collections_group_programs_program_action(void);
static void st_collections_group_programs_program_transition(void);
static void st_collections_group_programs_program_target(void);
static void st_collections_group_programs_program_after(void);

static void ob_collections_group_programs_program_script(void);

/*****/

New_Statement_Handler statement_handlers[] =
{
     {"images.image", st_images_image},
     {"fonts.font", st_fonts_font},
     {"data.item", st_data_item},
     {"styles.style.name", st_styles_style_name},
     {"styles.style.base", st_styles_style_base},
     {"styles.style.tag", st_styles_style_tag},
     {"color_classes.color_class.name", st_color_class_name},
     {"color_classes.color_class.color", st_color_class_color},
     {"color_classes.color_class.color2", st_color_class_color2},
     {"color_classes.color_class.color3", st_color_class_color3},
     /*{"spectra.spectrum", st_spectrum},*/
     {"spectra.spectrum.name", st_spectrum_name},
     {"spectra.spectrum.color", st_spectrum_color},
     {"collections.image", st_images_image}, /* dup */
     {"collections.images.image", st_images_image}, /* dup */
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
     {"collections.group.min", st_collections_group_min},
     {"collections.group.max", st_collections_group_max},
     {"collections.group.data.item", st_collections_group_data_item},
     {"collections.group.image", st_images_image}, /* dup */
     {"collections.group.images.image", st_images_image}, /* dup */
     {"collections.group.font", st_fonts_font}, /* dup */
     {"collections.group.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.color_classes.color_class.name", st_color_class_name}, /* dup */ 
     {"collections.group.color_classes.color_class.color", st_color_class_color}, /* dup */ 
     {"collections.group.color_classes.color_class.color2", st_color_class_color2}, /* dup */ 
     {"collections.group.color_classes.color_class.color3", st_color_class_color3}, /* dup */ 
     {"collections.group.parts.image", st_images_image}, /* dup */
     {"collections.group.parts.images.image", st_images_image}, /* dup */
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
     {"collections.group.parts.part.type", st_collections_group_parts_part_type},
     {"collections.group.parts.part.effect", st_collections_group_parts_part_effect},
     {"collections.group.parts.part.mouse_events", st_collections_group_parts_part_mouse_events},
     {"collections.group.parts.part.repeat_events", st_collections_group_parts_part_repeat_events},
     {"collections.group.parts.part.clip_to", st_collections_group_parts_part_clip_to_id},
     {"collections.group.parts.part.dragable.x", st_collections_group_parts_part_dragable_x},
     {"collections.group.parts.part.dragable.y", st_collections_group_parts_part_dragable_y},
     {"collections.group.parts.part.dragable.confine", st_collections_group_parts_part_dragable_confine},
     {"collections.group.parts.part.dragable.events", st_collections_group_parts_part_dragable_events},
     {"collections.group.parts.part.image", st_images_image}, /* dup */
     {"collections.group.parts.part.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.part.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.part.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.name", st_color_class_name}, /* dup */ 
     {"collections.group.parts.part.color_classes.color_class.color", st_color_class_color}, /* dup */ 
     {"collections.group.parts.part.color_classes.color_class.color2", st_color_class_color2}, /* dup */ 
     {"collections.group.parts.part.color_classes.color_class.color3", st_color_class_color3}, /* dup */ 
     {"collections.group.parts.part.description.inherit", st_collections_group_parts_part_description_inherit},
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
     {"collections.group.parts.part.description.image.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.image.border", st_collections_group_parts_part_description_image_border},
     {"collections.group.parts.part.description.image.middle", st_collections_group_parts_part_description_image_middle},
     {"collections.group.parts.part.description.fill.smooth", st_collections_group_parts_part_description_fill_smooth},
     {"collections.group.parts.part.description.fill.origin.relative", st_collections_group_parts_part_description_fill_origin_relative},
     {"collections.group.parts.part.description.fill.origin.offset", st_collections_group_parts_part_description_fill_origin_offset},
     {"collections.group.parts.part.description.fill.size.relative", st_collections_group_parts_part_description_fill_size_relative},
     {"collections.group.parts.part.description.fill.size.offset", st_collections_group_parts_part_description_fill_size_offset},
     {"collections.group.parts.part.description.fill.angle", st_collections_group_parts_part_description_fill_angle},
     {"collections.group.parts.part.description.fill.spread", st_collections_group_parts_part_description_fill_spread},
     {"collections.group.parts.part.description.color_class", st_collections_group_parts_part_description_color_class},
     {"collections.group.parts.part.description.color", st_collections_group_parts_part_description_color},
     {"collections.group.parts.part.description.color2", st_collections_group_parts_part_description_color2},
     {"collections.group.parts.part.description.color3", st_collections_group_parts_part_description_color3},
     {"collections.group.parts.part.description.text.text", st_collections_group_parts_part_description_text_text},
     {"collections.group.parts.part.description.text.text_class", st_collections_group_parts_part_description_text_text_class},
     {"collections.group.parts.part.description.text.font", st_collections_group_parts_part_description_text_font},
     {"collections.group.parts.part.description.text.style", st_collections_group_parts_part_description_text_style},
     {"collections.group.parts.part.description.text.size", st_collections_group_parts_part_description_text_size},
     {"collections.group.parts.part.description.text.fit", st_collections_group_parts_part_description_text_fit},
     {"collections.group.parts.part.description.text.min", st_collections_group_parts_part_description_text_min},
     {"collections.group.parts.part.description.text.align", st_collections_group_parts_part_description_text_align},
     {"collections.group.parts.part.description.text.source", st_collections_group_parts_part_description_text_source},
     {"collections.group.parts.part.description.text.text_source", st_collections_group_parts_part_description_text_text_source},
     {"collections.group.parts.part.description.text.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.text.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.text.elipsis", st_collections_group_parts_part_description_text_elipsis},
     {"collections.group.parts.part.description.gradient.type", st_collections_group_parts_part_description_gradient_type},
     {"collections.group.parts.part.description.gradient.spectrum", st_collections_group_parts_part_description_gradient_spectrum},
     {"collections.group.parts.part.description.images.image", st_images_image}, /* dup */
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
     {"collections.group.parts.part.description.programs.images.image", st_images_image}, /* dup */
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
     {"collections.group.parts.part.description.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.description.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.description.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.description.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.description.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.description.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.description.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.description.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.part.programs.images.image", st_images_image}, /* dup */
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
     {"collections.group.parts.part.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.programs.program.after", st_collections_group_programs_program_after},
     {"collections.group.parts.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.programs.program.name", st_collections_group_programs_program_name},
     {"collections.group.programs.program.signal", st_collections_group_programs_program_signal},
     {"collections.group.programs.program.source", st_collections_group_programs_program_source},
     {"collections.group.programs.program.in", st_collections_group_programs_program_in},
     {"collections.group.programs.program.action", st_collections_group_programs_program_action},
     {"collections.group.programs.program.transition", st_collections_group_programs_program_transition},
     {"collections.group.programs.program.target", st_collections_group_programs_program_target},
     {"collections.group.programs.program.after", st_collections_group_programs_program_after},
     {"collections.group.programs.image", st_images_image}, /* dup */
     {"collections.group.programs.images.image", st_images_image}, /* dup */
     {"collections.group.programs.font", st_fonts_font}, /* dup */
     {"collections.group.programs.fonts.font", st_fonts_font} /* dup */
};

New_Object_Handler object_handlers[] =
{
     {"images", NULL},
     {"fonts", NULL},
     {"data", NULL},
     {"styles", NULL},
     {"styles.style", ob_styles_style},
     {"color_classes", NULL},
     {"color_classes.color_class", ob_color_class},
     {"spectra", NULL},
     {"spectra.spectrum", ob_spectrum},
     {"collections", ob_collections},
     {"collections.images", NULL}, /* dup */
     {"collections.fonts", NULL}, /* dup */
     {"collections.styles", NULL}, /* dup */
     {"collections.styles.style", ob_styles_style}, /* dup */
     {"collections.color_classes", NULL}, /* dup */
     {"collections.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group", ob_collections_group},
     {"collections.group.data", NULL},
     {"collections.group.script", ob_collections_group_script},
     {"collections.group.images", NULL}, /* dup */
     {"collections.group.fonts", NULL}, /* dup */
     {"collections.group.styles", NULL}, /* dup */
     {"collections.group.styles.style", ob_styles_style}, /* dup */
     {"collections.group.color_classes", NULL}, /* dup */
     {"collections.group.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts", NULL},
     {"collections.group.parts.images", NULL}, /* dup */
     {"collections.group.parts.fonts", NULL}, /* dup */
     {"collections.group.parts.styles", NULL}, /* dup */
     {"collections.group.parts.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.color_classes", NULL}, /* dup */
     {"collections.group.parts.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part", ob_collections_group_parts_part},
     {"collections.group.parts.part.dragable", NULL},
     {"collections.group.parts.part.images", NULL}, /* dup */
     {"collections.group.parts.part.fonts", NULL}, /* dup */
     {"collections.group.parts.part.styles", NULL}, /* dup */
     {"collections.group.parts.part.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.part.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part.description", ob_collections_group_parts_part_description},
     {"collections.group.parts.part.description.rel1", NULL},
     {"collections.group.parts.part.description.rel2", NULL},
     {"collections.group.parts.part.description.image", NULL}, /* dup */
     {"collections.group.parts.part.description.image.images", NULL}, /* dup */
     {"collections.group.parts.part.description.fill", NULL},
     {"collections.group.parts.part.description.fill.origin", NULL},
     {"collections.group.parts.part.description.fill.size", NULL},
     {"collections.group.parts.part.description.text", NULL},
     {"collections.group.parts.part.description.text.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.images", NULL}, /* dup */
     {"collections.group.parts.part.description.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.styles", NULL}, /* dup */
     {"collections.group.parts.part.description.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.part.description.gradient", NULL},
     {"collections.group.parts.part.description.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part.description.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.description.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.description.programs", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.images", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.description.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.description.script", ob_collections_group_script}, /* dup */
     {"collections.group.parts.part.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.programs", NULL}, /* dup */
     {"collections.group.parts.part.programs.images", NULL}, /* dup */
     {"collections.group.parts.part.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.part.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.script", ob_collections_group_script}, /* dup */
     {"collections.group.parts.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.programs", NULL}, /* dup */
     {"collections.group.parts.programs.images", NULL}, /* dup */
     {"collections.group.parts.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.script", ob_collections_group_script}, /* dup */
     {"collections.group.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.programs", NULL},
     {"collections.group.programs.images", NULL}, /* dup */
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

/*****/

static void
st_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   int v;
   
   if (!edje_file->image_dir)
     edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));
   img = mem_alloc(SZ(Edje_Image_Directory_Entry));
   img->entry = parse_str(0);
     {
	Evas_List *l;
	
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *limg;
	     
	     limg = l->data;
	     if (!strcmp(limg->entry, img->entry))
	       {
		  free(img->entry);
		  free(img);
		  return;
	       }
	  }
     }
   edje_file->image_dir->entries = evas_list_append(edje_file->image_dir->entries, img);
   img->id = evas_list_count(edje_file->image_dir->entries) - 1;
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

static void
st_fonts_font(void)
{
   Font *fn;
   Edje_Font_Directory_Entry *fnt;

   check_arg_count(2);
   
   if (!edje_file->font_dir)
     edje_file->font_dir = mem_alloc(SZ(Edje_Font_Directory));
   
   fn = mem_alloc(SZ(Font));
   fn->file = parse_str(0);
   fn->name = parse_str(1);
     {
	Evas_List *l;
	
	for (l = fonts; l; l = l->next)
	  {
	     Font *lfn;
	     
	     lfn = l->data;
	     if (!strcmp(lfn->name, fn->name))
	       {
		  free(fn->file);
		  free(fn->name);
		  free(fn);
		  return;
	       }
	  }
     }
   fonts = evas_list_append(fonts, fn);
  
   if (edje_file->font_dir)
     {
	fnt = mem_alloc(SZ(Edje_Font_Directory_Entry));
	fnt->entry = mem_strdup(fn->name);
	edje_file->font_dir->entries = evas_list_append(edje_file->font_dir->entries, fnt);
     }
}

static void
st_data_item(void)
{
   Edje_Data *di;

   check_arg_count(2);
   
   di = mem_alloc(SZ(Edje_Data));
   di->key = parse_str(0);
   di->value = parse_str(1);
   edje_file->data = evas_list_append(edje_file->data, di);
}


static void
ob_color_class(void)
{
   Edje_Color_Class *cc;

   cc = mem_alloc(SZ(Edje_Color_Class));
   edje_file->color_classes = evas_list_append(edje_file->color_classes, cc);

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
st_color_class_name(void)
{
   Edje_Color_Class *cc, *tcc;
   Evas_List *l;
   
   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->name = parse_str(0);
   for (l = edje_file->color_classes; l; l = l->next)
     {
	tcc = l->data;
	if ((cc != tcc) && (!strcmp(cc->name, tcc->name)))
	  {
	     fprintf(stderr, "%s: Error. parse error %s:%i. There is already a color class named \"%s\"\n",
		     progname, file_in, line - 1, cc->name);
	     exit(-1);
	  }
     }
}

static void
st_color_class_color(void)
{
   Edje_Color_Class *cc;
   
   check_arg_count(4);

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->r = parse_int_range(0, 0, 255);
   cc->g = parse_int_range(1, 0, 255);
   cc->b = parse_int_range(2, 0, 255);
   cc->a = parse_int_range(3, 0, 255);
}

static void
st_color_class_color2(void)
{
   Edje_Color_Class *cc;
   
   check_arg_count(4);

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->r2 = parse_int_range(0, 0, 255);
   cc->g2 = parse_int_range(1, 0, 255);
   cc->b2 = parse_int_range(2, 0, 255);
   cc->a2 = parse_int_range(3, 0, 255);
}


static void
st_color_class_color3(void)
{
   Edje_Color_Class *cc;
   
   check_arg_count(4);

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->r3 = parse_int_range(0, 0, 255);
   cc->g3 = parse_int_range(1, 0, 255);
   cc->b3 = parse_int_range(2, 0, 255);
   cc->a3 = parse_int_range(3, 0, 255);
}

static void
ob_spectrum(void)
{
   Edje_Spectrum_Directory_Entry *se;

   if (!edje_file->spectrum_dir)
     edje_file->spectrum_dir = mem_alloc(SZ(Edje_Spectrum_Directory));
   se = mem_alloc(SZ(Edje_Spectrum_Directory_Entry));
   edje_file->spectrum_dir->entries = evas_list_append(edje_file->spectrum_dir->entries, se);
   se->id = evas_list_count(edje_file->spectrum_dir->entries) - 1;
   se->entry = NULL;
   se->filename = NULL;
   se->color_list = NULL;
}

static void
st_spectrum_name(void)
{
   Edje_Spectrum_Directory_Entry *se;
   Edje_Spectrum_Color *sc;

   se = evas_list_data(evas_list_last(edje_file->spectrum_dir->entries));
   se->entry = parse_str(0);
}

static void
st_spectrum_color(void)
{
   Edje_Spectrum_Directory_Entry *se;
   Edje_Spectrum_Color *sc;

   se = evas_list_data(evas_list_last(edje_file->spectrum_dir->entries));

   sc = mem_alloc(SZ(Edje_Spectrum_Color));
   se->color_list = evas_list_append(se->color_list, sc);
   sc->r = parse_int_range(0, 0, 255);
   sc->g = parse_int_range(1, 0, 255);
   sc->b = parse_int_range(2, 0, 255);
   sc->a = parse_int_range(3, 0, 255);
   sc->d = parse_int(4);
}

static void
ob_styles_style(void)
{
   Edje_Style *stl;
   
   stl = mem_alloc(SZ(Edje_Style));
   edje_file->styles = evas_list_append(edje_file->styles, stl);
}

static void
st_styles_style_name(void)
{
   Edje_Style *stl, *tstl;
   Evas_List *l;
   
   stl = evas_list_data(evas_list_last(edje_file->styles));
   stl->name = parse_str(0);
   for (l = edje_file->styles; l; l = l->next)
     {
	tstl = l->data;
	if ((stl != tstl) && (!strcmp(stl->name, tstl->name)))
	  {
	     fprintf(stderr, "%s: Error. parse error %s:%i. There is already a style named \"%s\"\n",
		     progname, file_in, line - 1, stl->name);
	     exit(-1);
	  }
     }
}


static void
st_styles_style_base(void)
{
   Edje_Style *stl;
   Edje_Style_Tag *tag;
   
   stl = evas_list_data(evas_list_last(edje_file->styles));
   if (stl->tags)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. There is already a basic format for the style\n",
		progname, file_in, line - 1);
	exit(-1);
     }
   tag = mem_alloc(SZ(Edje_Style_Tag));
   tag->key = mem_strdup("DEFAULT");
   tag->value = parse_str(0);
   stl->tags = evas_list_append(stl->tags, tag);
}

static void
st_styles_style_tag(void)
{
   Edje_Style *stl;
   Edje_Style_Tag *tag;
   
   stl = evas_list_data(evas_list_last(edje_file->styles));
   tag = mem_alloc(SZ(Edje_Style_Tag));
   tag->key = parse_str(0);
   tag->value = parse_str(1);
   stl->tags = evas_list_append(stl->tags, tag);
}

static void
ob_collections(void)
{
   if (!edje_file->collection_dir)
     edje_file->collection_dir = mem_alloc(SZ(Edje_Part_Collection_Directory));
}

static void
ob_collections_group(void)
{
   Edje_Part_Collection_Directory_Entry *de;
   Edje_Part_Collection *pc;
   Code *cd;
   
   de = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   edje_file->collection_dir->entries = evas_list_append(edje_file->collection_dir->entries, de);
   de->id = evas_list_count(edje_file->collection_dir->entries) - 1;
   
   pc = mem_alloc(SZ(Edje_Part_Collection));
   edje_collections = evas_list_append(edje_collections, pc);
   pc->id = evas_list_count(edje_collections) - 1;
   
   cd = mem_alloc(SZ(Code));
   codes = evas_list_append(codes, cd);
}

static void
st_collections_group_name(void)
{
   Edje_Part_Collection_Directory_Entry *de;

   check_arg_count(1);
   
   de = evas_list_data(evas_list_last(edje_file->collection_dir->entries));
   de->entry = parse_str(0);
}

static void
st_collections_group_min(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   pc->prop.min.w = parse_int_range(0, 0, 0x7fffffff);
   pc->prop.min.h = parse_int_range(1, 0, 0x7fffffff);
}

static void
st_collections_group_max(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   pc->prop.max.w = parse_int_range(0, 0, 0x7fffffff);
   pc->prop.max.h = parse_int_range(1, 0, 0x7fffffff);
}

static void
ob_collections_group_script(void)
{
   Edje_Part_Collection *pc;
   Code *cd;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   cd = evas_list_data(evas_list_last(codes));
   
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
		  fprintf(stderr, "%s: Error. parse error %s:%i. There is already an existing script section for the group\n",
			  progname, file_in, line - 1);
		  exit(-1);
	       }
	     cd->shared = s;
	     set_verbatim(NULL, 0, 0);
	  }
     }
}

static void
st_collections_group_data_item(void)
{
   Edje_Part_Collection *pc;
   Edje_Data *di;

   check_arg_count(2);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   di = mem_alloc(SZ(Edje_Data));
   di->key = parse_str(0);
   di->value = parse_str(1);
   pc->data = evas_list_append(pc->data, di);
}

static void
ob_collections_group_parts_part(void)
{   
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   ep = mem_alloc(SZ(Edje_Part));
   pc = evas_list_data(evas_list_last(edje_collections));
   pc->parts = evas_list_append(pc->parts, ep);
   ep->id = evas_list_count(pc->parts) - 1;
   ep->type = EDJE_PART_TYPE_IMAGE;
   ep->mouse_events = 1;
   ep->repeat_events = 0;
   ep->clip_to_id = -1;
   ep->dragable.confine_id = -1;
   ep->dragable.events_id = -1;
}

static void
st_collections_group_parts_part_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->name = parse_str(0);

   {
	Evas_List *l;
	
	for (l = pc->parts; l; l = l->next)
	  {
	     Edje_Part *lep;
	     
	     lep = l->data;
	     if ((lep != ep) && (lep->name) && (!strcmp(lep->name, ep->name)))
	       {
		  fprintf(stderr, "%s: Error. parse error %s:%i. There is already a part of the name %s\n",
			  progname, file_in, line - 1, ep->name);
		  exit(-1);
	       }
	  }
   }
}

static void
st_collections_group_parts_part_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->type = parse_enum(0, 
			 "NONE", EDJE_PART_TYPE_NONE,
			 "RECT", EDJE_PART_TYPE_RECTANGLE,
			 "TEXT", EDJE_PART_TYPE_TEXT,
			 "IMAGE", EDJE_PART_TYPE_IMAGE,
			 "SWALLOW", EDJE_PART_TYPE_SWALLOW,
			 "TEXTBLOCK", EDJE_PART_TYPE_TEXTBLOCK,
			 "GRADIENT", EDJE_PART_TYPE_GRADIENT,
			 NULL);
}

static void
st_collections_group_parts_part_mouse_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->mouse_events = parse_bool(0);
}

static void
st_collections_group_parts_part_repeat_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->repeat_events = parse_bool(0);
}

static void
st_collections_group_parts_part_clip_to_id(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ep->clip_to_id));
	free(name);
     }
}

static void
st_collections_group_parts_part_dragable_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(3);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->dragable.x = parse_int_range(0, -1, 1);
   ep->dragable.step_x = parse_int_range(1, 0, 0x7fffffff);
   ep->dragable.count_x = parse_int_range(2, 0, 0x7fffffff);
}

static void
st_collections_group_parts_part_dragable_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(3);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->dragable.y = parse_int_range(0, -1, 1);
   ep->dragable.step_y = parse_int_range(1, 0, 0x7fffffff);
   ep->dragable.count_y = parse_int_range(2, 0, 0x7fffffff);
}

static void
st_collections_group_parts_part_dragable_confine(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ep->dragable.confine_id));
	free(name);
     }
}

static void
st_collections_group_parts_part_dragable_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ep->dragable.events_id));
	free(name);
     }
}

static void
ob_collections_group_parts_part_description(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = mem_alloc(SZ(Edje_Part_Description));
   if (!ep->default_desc)
     ep->default_desc = ed;
   else
     ep->other_desc = evas_list_append(ep->other_desc, ed);
   ed->visible = 1;
   ed->align.x = 0.5;
   ed->align.y = 0.5;
   ed->min.w = 0;
   ed->min.h = 0;
   ed->fixed.w = 0;
   ed->fixed.h = 0;
   ed->max.w = -1;
   ed->max.h = -1;
   ed->rel1.relative_x = 0.0;
   ed->rel1.relative_y = 0.0;
   ed->rel1.offset_x = 0;
   ed->rel1.offset_y = 0;
   ed->rel1.id_x = -1;
   ed->rel1.id_y = -1;
   ed->rel2.relative_x = 1.0;
   ed->rel2.relative_y = 1.0;
   ed->rel2.offset_x = -1;
   ed->rel2.offset_y = -1;
   ed->rel2.id_x = -1;
   ed->rel2.id_y = -1;
   ed->image.id = -1;
   ed->fill.smooth = 1;
   ed->fill.pos_rel_x = 0.0;
   ed->fill.pos_abs_x = 0;
   ed->fill.rel_x = 1.0;
   ed->fill.abs_x = 0;
   ed->fill.pos_rel_y = 0.0;
   ed->fill.pos_abs_y = 0;
   ed->fill.rel_y = 1.0;
   ed->fill.abs_y = 0;
   ed->fill.angle = 0;
   ed->fill.spread = 0;
   ed->color_class = NULL;
   ed->color.r = 255;
   ed->color.g = 255;
   ed->color.b = 255;
   ed->color.a = 255;
   ed->color2.r = 0;
   ed->color2.g = 0;
   ed->color2.b = 0;
   ed->color2.a = 255;
   ed->color3.r = 0;
   ed->color3.g = 0;
   ed->color3.b = 0;
   ed->color3.a = 128;
   ed->text.align.x = 0.5;
   ed->text.align.y = 0.5;
   ed->text.id_source = -1;
   ed->text.id_text_source = -1;
}

static void
st_collections_group_parts_part_description_inherit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed, *parent = NULL;
   Evas_List *l;
   char *parent_name, *state_name;
   double parent_val, state_val;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   /* inherit may not be used in the default description */
   if (!ep->other_desc)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"inherit may not be used in the default description\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = evas_list_data(evas_list_last(ep->other_desc));

   if (!ed->state.name)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"inherit may only be used after state\n",
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
	double min_dst = 999.0;

	if (!strcmp(parent_name, "default"))
	  {
	     parent = ep->default_desc;
	     min_dst = ABS(ep->default_desc->state.value - parent_val);
	  }

	for (l = ep->other_desc; l; l = l->next)
	  {
	     Edje_Part_Description *d = l->data;

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
	fprintf (stderr, "%s: Error. parse error %s:%i. "
	      "cannot find referenced part state %s %lf\n",
	      ep->name, file_in, line - 1, parent_name, parent_val);
	exit(-1);
     }

   free (parent_name);

   /* now do a full copy, only state info will be kept */
   state_name = ed->state.name;
   state_val = ed->state.value;

   *ed = *parent;

   ed->state.name = state_name;
   ed->state.value = state_val;

   data_queue_part_slave_lookup(&parent->rel1.id_x, &ed->rel1.id_x);
   data_queue_part_slave_lookup(&parent->rel1.id_y, &ed->rel1.id_y);
   data_queue_part_slave_lookup(&parent->rel2.id_x, &ed->rel2.id_x);
   data_queue_part_slave_lookup(&parent->rel2.id_y, &ed->rel2.id_y);
   data_queue_image_slave_lookup(&parent->image.id, &ed->image.id);
   data_queue_spectrum_slave_lookup(&parent->gradient.id, &ed->gradient.id);

   /* make sure all the allocated memory is getting copied, not just
    * referenced
    */
   ed->image.tween_list = NULL;

   for (l = parent->image.tween_list; l; l = l->next)
     {
	Edje_Part_Image_Id *iid, *iid_new;
	
	iid = l->data;
	iid_new = mem_alloc(SZ(Edje_Part_Image_Id));
	data_queue_image_slave_lookup(&(iid->id), &(iid_new->id));
	ed->image.tween_list = evas_list_append(ed->image.tween_list, iid_new);
     }

#define STRDUP(x) x ? strdup(x) : NULL

   ed->color_class = STRDUP(ed->color_class);
   ed->text.text = STRDUP(ed->text.text);
   ed->text.text_class = STRDUP(ed->text.text_class);
   ed->text.font = STRDUP(ed->text.font);
#undef STRDUP

   data_queue_part_slave_lookup(&(parent->text.id_source), &(ed->text.id_source));
   data_queue_part_slave_lookup(&(parent->text.id_text_source), &(ed->text.id_text_source));
}

static void
st_collections_group_parts_part_description_state(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   char *s;

   check_arg_count(2);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   s = parse_str(0);
   if (!strcmp (s, "custom"))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"invalid state name: '%s'.\n",
		progname, file_in, line - 1, s);
	exit(-1);
     }

   ed->state.name = s;
   ed->state.value = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_visible(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->visible = parse_bool(0);
}

static void
st_collections_group_parts_part_description_align(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->align.x = parse_float_range(0, 0.0, 1.0);
   ed->align.y = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_fixed(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fixed.w = parse_float_range(0, 0, 1);
   ed->fixed.h = parse_float_range(1, 0, 1);
}

static void
st_collections_group_parts_part_description_min(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->min.w = parse_float_range(0, 0, 0x7fffffff);
   ed->min.h = parse_float_range(1, 0, 0x7fffffff);
}

static void
st_collections_group_parts_part_description_max(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->max.w = parse_float_range(0, 0, 0x7fffffff);
   ed->max.h = parse_float_range(1, 0, 0x7fffffff);
}

static void
st_collections_group_parts_part_description_step(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->step.x = parse_float_range(0, 0, 0x7fffffff);
   ed->step.y = parse_float_range(1, 0, 0x7fffffff);
}

static void
st_collections_group_parts_part_description_aspect(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->aspect.min = parse_float_range(0, 0.0, 999999999.0);
   ed->aspect.max = parse_float_range(1, 0.0, 999999999.0);
}

static void
st_collections_group_parts_part_description_aspect_preference(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->aspect.prefer =  parse_enum(0, 
				   "NONE", EDJE_ASPECT_PREFER_NONE,
				   "VERTICAL", EDJE_ASPECT_PREFER_VERTICAL,
				   "HORIZONTAL", EDJE_ASPECT_PREFER_HORIZONTAL,
				   "BOTH", EDJE_ASPECT_PREFER_BOTH,
				   NULL);
}

static void
st_collections_group_parts_part_description_rel1_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel1.relative_x = parse_float(0);
   ed->rel1.relative_y = parse_float(1);
}

static void
st_collections_group_parts_part_description_rel1_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel1.offset_x = parse_int(0);
   ed->rel1.offset_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_rel1_to(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id_x));
	data_queue_part_lookup(pc, name, &(ed->rel1.id_y));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel1_to_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id_x));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel1_to_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id_y));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel2.relative_x = parse_float(0);
   ed->rel2.relative_y = parse_float(1);
}

static void
st_collections_group_parts_part_description_rel2_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel2.offset_x = parse_int(0);
   ed->rel2.offset_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_rel2_to(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id_x));
	data_queue_part_lookup(pc, name, &(ed->rel2.id_y));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_to_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id_x));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_to_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id_y));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_image_normal(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_image_lookup(name, &(ed->image.id));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_image_tween(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	Edje_Part_Image_Id *iid;
	
	iid = mem_alloc(SZ(Edje_Part_Image_Id));
	ed->image.tween_list = evas_list_append(ed->image.tween_list, iid);
	name = parse_str(0);
	data_queue_image_lookup(name, &(iid->id));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_image_border(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->border.l = parse_int_range(0, 0, 0x7fffffff);
   ed->border.r = parse_int_range(1, 0, 0x7fffffff);
   ed->border.t = parse_int_range(2, 0, 0x7fffffff);
   ed->border.b = parse_int_range(3, 0, 0x7fffffff);   
}

static void
st_collections_group_parts_part_description_image_middle(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->border.no_fill = !parse_bool(0);
}

static void
st_collections_group_parts_part_description_fill_smooth(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.smooth = parse_bool(0);
}

static void
st_collections_group_parts_part_description_fill_origin_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.pos_rel_x = parse_float_range(0, -999999999.0, 999999999.0);   
   ed->fill.pos_rel_y = parse_float_range(1, -999999999.0, 999999999.0);   
}

static void
st_collections_group_parts_part_description_fill_origin_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.pos_abs_x = parse_int(0);
   ed->fill.pos_abs_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_fill_size_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed->fill.rel_x = parse_float_range(0, 0.0, 999999999.0);   
   ed->fill.rel_y = parse_float_range(1, 0.0, 999999999.0);   
}

static void
st_collections_group_parts_part_description_fill_size_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed->fill.abs_x = parse_int(0);
   ed->fill.abs_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_color_class(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color_class = parse_str(0);
}

static void
st_collections_group_parts_part_description_color(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color.r = parse_int_range(0, 0, 255);
   ed->color.g = parse_int_range(1, 0, 255);
   ed->color.b = parse_int_range(2, 0, 255);
   ed->color.a = parse_int_range(3, 0, 255);
}

static void
st_collections_group_parts_part_description_color2(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color2.r = parse_int_range(0, 0, 255);
   ed->color2.g = parse_int_range(1, 0, 255);
   ed->color2.b = parse_int_range(2, 0, 255);
   ed->color2.a = parse_int_range(3, 0, 255);
}

static void
st_collections_group_parts_part_description_color3(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color3.r = parse_int_range(0, 0, 255);
   ed->color3.g = parse_int_range(1, 0, 255);
   ed->color3.b = parse_int_range(2, 0, 255);
   ed->color3.a = parse_int_range(3, 0, 255);
}

static void
st_collections_group_parts_part_description_text_text(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   char *str = NULL;
   int i;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
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
   ed->text.text = str;
}

static void
st_collections_group_parts_part_description_text_text_class(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.text_class = parse_str(0);
}

static void
st_collections_group_parts_part_description_text_font(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.font = parse_str(0);
}

static void
st_collections_group_parts_part_description_text_style(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXTBLOCK)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXTBLOCK part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.style = parse_str(0);
}

static void
st_collections_group_parts_part_description_text_size(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.size = parse_int_range(0, 0, 255);
}

static void
st_collections_group_parts_part_effect(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->effect = parse_enum(0, 
			   "NONE", EDJE_TEXT_EFFECT_NONE,
			   "PLAIN", EDJE_TEXT_EFFECT_PLAIN,
			   "OUTLINE", EDJE_TEXT_EFFECT_OUTLINE,
			   "SOFT_OUTLINE", EDJE_TEXT_EFFECT_SOFT_OUTLINE,
			   "SHADOW", EDJE_TEXT_EFFECT_SHADOW,
			   "SOFT_SHADOW", EDJE_TEXT_EFFECT_SOFT_SHADOW,
			   "OUTLINE_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SHADOW,
			   "OUTLINE_SOFT_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW,
			   NULL);
}

static void
st_collections_group_parts_part_description_text_fit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.fit_x = parse_bool(0);
   ed->text.fit_y = parse_bool(1);
}

static void
st_collections_group_parts_part_description_text_min(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.min_x = parse_bool(0);
   ed->text.min_y = parse_bool(1);
}

static void
st_collections_group_parts_part_description_text_align(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.align.x = parse_float_range(0, 0.0, 1.0);
   ed->text.align.y = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_text_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->text.id_source));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_text_text_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->text.id_text_source));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_fill_angle(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   /* XXX this will need to include IMAGES when angle support is added to evas images */
   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.angle = parse_int_range(0, 0, 360);
}

static void
st_collections_group_parts_part_description_fill_spread(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   /* XXX this will need to include IMAGES when spread support is added to evas images */
   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.spread = parse_int_range(0, 0, 1);
}

static void
st_collections_group_parts_part_description_text_elipsis(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.elipsis = parse_float_range(0, 0.0, 1.0);
}

static void 
st_collections_group_parts_part_description_gradient_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->gradient.type  = parse_str(0);
}

static void 
st_collections_group_parts_part_description_gradient_spectrum(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   char *spectrum;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

     {
	char *name;
	
	name = parse_str(0);
	data_queue_spectrum_lookup(name, &(ed->gradient.id));
	free(name);
     }
}


static void
ob_collections_group_programs_program(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = mem_alloc(SZ(Edje_Program));
   pc->programs = evas_list_append(pc->programs, ep);
   ep->id = evas_list_count(pc->programs) - 1;
   ep->tween.mode = EDJE_TWEEN_MODE_LINEAR;
   ep->after = NULL;
}

static void
st_collections_group_programs_program_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->name = parse_str(0);
     {
	Evas_List *l;
	
	for (l = pc->programs; l; l = l->next)
	  {
	     Edje_Program *lep;
	     
	     lep = l->data;
	     if ((lep != ep) && (lep->name) && (!strcmp(lep->name, ep->name)))
	       {
		  fprintf(stderr, "%s: Error. parse error %s:%i. There is already a program of the name %s\n",
			  progname, file_in, line - 1, ep->name);
		  exit(-1);
	       }
	  }
     }
}

static void
st_collections_group_programs_program_signal(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->signal = parse_str(0);
}

static void
st_collections_group_programs_program_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->source = parse_str(0);
}

static void
st_collections_group_programs_program_in(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->in.from = parse_float_range(0, 0.0, 999999999.0);
   ep->in.range = parse_float_range(1, 0.0, 999999999.0);
}

static void
st_collections_group_programs_program_action(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->action = parse_enum(0,
			   "STATE_SET", EDJE_ACTION_TYPE_STATE_SET,
			   "ACTION_STOP", EDJE_ACTION_TYPE_ACTION_STOP,
			   "SIGNAL_EMIT", EDJE_ACTION_TYPE_SIGNAL_EMIT,
			   "DRAG_VAL_SET", EDJE_ACTION_TYPE_DRAG_VAL_SET,
			   "DRAG_VAL_STEP", EDJE_ACTION_TYPE_DRAG_VAL_STEP,
			   "DRAG_VAL_PAGE", EDJE_ACTION_TYPE_DRAG_VAL_PAGE,
			   "SCRIPT", EDJE_ACTION_TYPE_SCRIPT,
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

   switch (ep->action) {
      case EDJE_ACTION_TYPE_ACTION_STOP:
	 check_arg_count(1);
	 break;
      case EDJE_ACTION_TYPE_SCRIPT:
	 /* FIXME: what's this? people usually just use script{}, no? */
	 break;
      default:
	 check_arg_count(3);
   }
}

static void
st_collections_group_programs_program_transition(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->tween.mode = parse_enum(0,
			       "LINEAR", EDJE_TWEEN_MODE_LINEAR,
			       "SINUSOIDAL", EDJE_TWEEN_MODE_SINUSOIDAL,
			       "ACCELERATE", EDJE_TWEEN_MODE_ACCELERATE,
			       "DECELERATE", EDJE_TWEEN_MODE_DECELERATE,
			       NULL);
   ep->tween.time = parse_float_range(1, 0.0, 999999999.0);
}

static void
st_collections_group_programs_program_target(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
     {
	Edje_Program_Target *et;
	char *name;
	
	et = mem_alloc(SZ(Edje_Program_Target));
	ep->targets = evas_list_append(ep->targets, et);
	
	name = parse_str(0);
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
	else
	  {
	     fprintf(stderr, "%s: Error. parse error %s:%i. "
		   "target may only be used after action\n",
		   progname, file_in, line - 1);
	     exit(-1);
	  }
	free(name);
     }
}

static void
st_collections_group_programs_program_after(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
     {
	Edje_Program_After *pa;
	char *name;
	
	name = parse_str(0);
	
	pa = mem_alloc(SZ(Edje_Program_After));
	pa->id = -1;
	ep->after = evas_list_append(ep->after, pa);
	
	data_queue_program_lookup(pc, name, &(pa->id));
	free(name);
     }
}

static void
ob_collections_group_programs_program_script(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;
   Code *cd;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   cd = evas_list_data(evas_list_last(codes));
   
   if (!is_verbatim()) track_verbatim(1);
   else
     {
	char *s;
	
	s = get_verbatim();
	if (s)
	  {
	     Code_Program *cp;
	     
	     cp = mem_alloc(SZ(Code_Program));
	     cp->l1 = get_verbatim_line1();
	     cp->l2 = get_verbatim_line2();
	     cp->id = ep->id;
	     cp->script = s;
	     cd->programs = evas_list_append(cd->programs, cp);
	     set_verbatim(NULL, 0, 0);
	     ep->action = EDJE_ACTION_TYPE_SCRIPT;
	  }
     }
}
