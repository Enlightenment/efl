#include "edje_cc.h"

static void ob_images(void);

static void ob_images_image(void);
static void st_images_image(void);

static void ob_collections(void);

static void ob_collections_group(void);
static void st_collections_group_name(void);

static void ob_collections_group_parts(void);

static void ob_collections_group_parts_part(void);
static void st_collections_group_parts_part_name(void);
static void st_collections_group_parts_part_type(void);
static void st_collections_group_parts_part_mouse_events(void);
static void st_collections_group_parts_part_color_class(void);

static void ob_collections_group_parts_part_description(void);
static void st_collections_group_parts_part_description_state(void);
static void st_collections_group_parts_part_description_visible(void);
static void st_collections_group_parts_part_description_dragable_x(void);
static void st_collections_group_parts_part_description_dragable_y(void);
static void st_collections_group_parts_part_description_dragable_confine(void);
static void st_collections_group_parts_part_description_align(void);
static void st_collections_group_parts_part_description_min(void);
static void st_collections_group_parts_part_description_max(void);
static void st_collections_group_parts_part_description_step(void);
static void st_collections_group_parts_part_description_aspect(void);
static void st_collections_group_parts_part_description_rel1_relative(void);
static void st_collections_group_parts_part_description_rel1_offset(void);
static void st_collections_group_parts_part_description_rel1_to(void);
static void st_collections_group_parts_part_description_rel2_relative(void);
static void st_collections_group_parts_part_description_rel2_offset(void);
static void st_collections_group_parts_part_description_rel2_to(void);
static void st_collections_group_parts_part_description_image_normal(void);
static void st_collections_group_parts_part_description_image_tween(void);
static void st_collections_group_parts_part_description_border(void);
static void st_collections_group_parts_part_description_fill_origin_relative(void);
static void st_collections_group_parts_part_description_fill_origin_offset(void);
static void st_collections_group_parts_part_description_fill_size_relative(void);
static void st_collections_group_parts_part_description_fill_size_offset(void);
static void st_collections_group_parts_part_description_color(void);
static void st_collections_group_parts_part_description_color2(void);
static void st_collections_group_parts_part_description_color3(void);
static void st_collections_group_parts_part_description_text_text(void);
static void st_collections_group_parts_part_description_text_font(void);
static void st_collections_group_parts_part_description_text_size(void);
static void st_collections_group_parts_part_description_text_effect(void);
static void st_collections_group_parts_part_description_text_fit(void);
static void st_collections_group_parts_part_description_text_min(void);
static void st_collections_group_parts_part_description_text_align(void);

static void ob_collections_group_programs(void);

static void ob_collections_group_programs_program(void);
static void st_collections_group_programs_program_name(void);
static void st_collections_group_programs_program_signal(void);
static void st_collections_group_programs_program_source(void);
static void st_collections_group_programs_program_action(void);
static void st_collections_group_programs_program_transition(void);
static void st_collections_group_programs_program_target(void);
static void st_collections_group_programs_program_after(void);

/*****/

New_Statement_Handler statement_handlers[] =
{
     {"images.image", st_images_image},
     {"collections.group.name", st_collections_group_name},
     {"collections.group.parts.part.name", st_collections_group_parts_part_name},
     {"collections.group.parts.part.type", st_collections_group_parts_part_type},
     {"collections.group.parts.part.mouse_events", st_collections_group_parts_part_mouse_events},
     {"collections.group.parts.part.color_class", st_collections_group_parts_part_color_class},
     {"collections.group.parts.part.description.state", st_collections_group_parts_part_description_state},
     {"collections.group.parts.part.description.visible", st_collections_group_parts_part_description_visible},
     {"collections.group.parts.part.description.dragable.x", st_collections_group_parts_part_description_dragable_x},
     {"collections.group.parts.part.description.dragable.y", st_collections_group_parts_part_description_dragable_y},
     {"collections.group.parts.part.description.dragable.confine", st_collections_group_parts_part_description_dragable_confine},
     {"collections.group.parts.part.description.align", st_collections_group_parts_part_description_align},
     {"collections.group.parts.part.description.min", st_collections_group_parts_part_description_min},
     {"collections.group.parts.part.description.max", st_collections_group_parts_part_description_max},
     {"collections.group.parts.part.description.step", st_collections_group_parts_part_description_step},
     {"collections.group.parts.part.description.aspect", st_collections_group_parts_part_description_aspect},
     {"collections.group.parts.part.description.rel1.relative", st_collections_group_parts_part_description_rel1_relative},
     {"collections.group.parts.part.description.rel1.offset", st_collections_group_parts_part_description_rel1_offset},
     {"collections.group.parts.part.description.rel1.to", st_collections_group_parts_part_description_rel1_to},
     {"collections.group.parts.part.description.rel2.relative", st_collections_group_parts_part_description_rel2_relative},
     {"collections.group.parts.part.description.rel2.offset", st_collections_group_parts_part_description_rel2_offset},
     {"collections.group.parts.part.description.rel2.to", st_collections_group_parts_part_description_rel2_to},
     {"collections.group.parts.part.description.image.normal", st_collections_group_parts_part_description_image_normal},
     {"collections.group.parts.part.description.image.tween", st_collections_group_parts_part_description_image_tween},
     {"collections.group.parts.part.description.border", st_collections_group_parts_part_description_border},
     {"collections.group.parts.part.description.fill.origin.relative", st_collections_group_parts_part_description_fill_origin_relative},
     {"collections.group.parts.part.description.fill.origin.offset", st_collections_group_parts_part_description_fill_origin_offset},
     {"collections.group.parts.part.description.fill.size.relative", st_collections_group_parts_part_description_fill_size_relative},
     {"collections.group.parts.part.description.fill.size.offset", st_collections_group_parts_part_description_fill_size_offset},
     {"collections.group.parts.part.description.color", st_collections_group_parts_part_description_color},
     {"collections.group.parts.part.description.color2", st_collections_group_parts_part_description_color2},
     {"collections.group.parts.part.description.color3", st_collections_group_parts_part_description_color3},
     {"collections.group.parts.part.description.text.text", st_collections_group_parts_part_description_text_text},
     {"collections.group.parts.part.description.text.font", st_collections_group_parts_part_description_text_font},
     {"collections.group.parts.part.description.text.size", st_collections_group_parts_part_description_text_size},
     {"collections.group.parts.part.description.text.effect", st_collections_group_parts_part_description_text_effect},
     {"collections.group.parts.part.description.text.fit", st_collections_group_parts_part_description_text_fit},
     {"collections.group.parts.part.description.text.min", st_collections_group_parts_part_description_text_min},
     {"collections.group.parts.part.description.text.align", st_collections_group_parts_part_description_text_align},
     {"collections.group.programs.program.name", st_collections_group_programs_program_name},
     {"collections.group.programs.program.signal", st_collections_group_programs_program_signal},
     {"collections.group.programs.program.source", st_collections_group_programs_program_source},
     {"collections.group.programs.program.action", st_collections_group_programs_program_action},
     {"collections.group.programs.program.transition", st_collections_group_programs_program_transition},
     {"collections.group.programs.program.target", st_collections_group_programs_program_target},
     {"collections.group.programs.program.after", st_collections_group_programs_program_after}
};

New_Object_Handler object_handlers[] =
{
     {"images", ob_images},
     {"images.image", ob_images_image},
     {"collections", ob_collections},
     {"collections.group", ob_collections_group},
     {"collections.group.name", NULL},
     {"collections.group.parts", ob_collections_group_parts},
     {"collections.group.parts.part", ob_collections_group_parts_part},
     {"collections.group.parts.part.name", NULL},
     {"collections.group.parts.part.type", NULL},
     {"collections.group.parts.part.mouse_events", NULL},
     {"collections.group.parts.part.color_class", NULL},
     {"collections.group.parts.part.description", ob_collections_group_parts_part_description},
     {"collections.group.parts.part.description.state", NULL},
     {"collections.group.parts.part.description.visible", NULL},
     {"collections.group.parts.part.description.dragable", NULL},
     {"collections.group.parts.part.description.dragable.x", NULL},
     {"collections.group.parts.part.description.dragable.y", NULL},
     {"collections.group.parts.part.description.dragable.confine", NULL},
     {"collections.group.parts.part.description.align", NULL},
     {"collections.group.parts.part.description.min", NULL},
     {"collections.group.parts.part.description.max", NULL},
     {"collections.group.parts.part.description.step", NULL},
     {"collections.group.parts.part.description.aspect", NULL},
     {"collections.group.parts.part.description.rel1", NULL},
     {"collections.group.parts.part.description.rel1.relative", NULL},
     {"collections.group.parts.part.description.rel1.offset", NULL},
     {"collections.group.parts.part.description.rel1.to", NULL},
     {"collections.group.parts.part.description.rel2", NULL},
     {"collections.group.parts.part.description.rel2.relative", NULL},
     {"collections.group.parts.part.description.rel2.offset", NULL},
     {"collections.group.parts.part.description.rel2.to", NULL},
     {"collections.group.parts.part.description.image", NULL},
     {"collections.group.parts.part.description.image.normal", NULL},
     {"collections.group.parts.part.description.image.tween", NULL},
     {"collections.group.parts.part.description.border", NULL},
     {"collections.group.parts.part.description.fill", NULL},
     {"collections.group.parts.part.description.fill.origin", NULL},
     {"collections.group.parts.part.description.fill.origin.relative", NULL},
     {"collections.group.parts.part.description.fill.origin.offset", NULL},
     {"collections.group.parts.part.description.fill.size", NULL},
     {"collections.group.parts.part.description.fill.size.relative", NULL},
     {"collections.group.parts.part.description.fill.size.offset", NULL},
     {"collections.group.parts.part.description.color", NULL},
     {"collections.group.parts.part.description.color2", NULL},
     {"collections.group.parts.part.description.color3", NULL},
     {"collections.group.parts.part.description.text", NULL},
     {"collections.group.parts.part.description.text.text", NULL},
     {"collections.group.parts.part.description.text.font", NULL},
     {"collections.group.parts.part.description.text.size", NULL},
     {"collections.group.parts.part.description.text.effect", NULL},
     {"collections.group.parts.part.description.text.fit", NULL},
     {"collections.group.parts.part.description.text.min", NULL},
     {"collections.group.parts.part.description.text.align", NULL},
     {"collections.group.programs", ob_collections_group_programs},
     {"collections.group.programs.program", ob_collections_group_programs_program},
     {"collections.group.programs.program.name", NULL},
     {"collections.group.programs.program.signal", NULL},
     {"collections.group.programs.program.source", NULL},
     {"collections.group.programs.program.action", NULL},
     {"collections.group.programs.program.transition", NULL},
     {"collections.group.programs.program.target", NULL},
     {"collections.group.programs.program.after", NULL}
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
ob_images(void)
{
   edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));
}

static void
ob_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   
   img = calloc(1, sizeof(Edje_Image_Directory_Entry));
   edje_file->image_dir->entries = evas_list_append(edje_file->image_dir->entries, img);
   img->id = evas_list_count(edje_file->image_dir->entries) - 1;
}

static void
st_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   int v;
   
   img = evas_list_data(evas_list_last(edje_file->image_dir->entries));
   img->entry = parse_str(0);
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
   if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY) return;
   img->source_param = parse_int_range(2, 0, 100);
}

static void
ob_collections(void)
{
   edje_file->collection_dir = mem_alloc(SZ(Edje_Part_Collection_Directory));
}

static void
ob_collections_group(void)
{
   Edje_Part_Collection_Directory_Entry *de;
   Edje_Part_Collection *pc;
   
   de = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   edje_file->collection_dir->entries = evas_list_append(edje_file->collection_dir->entries, de);
   de->id = evas_list_count(edje_file->collection_dir->entries) - 1;
   
   pc = mem_alloc(SZ(Edje_Part_Collection));
   edje_collections = evas_list_append(edje_collections, pc);
   pc->id = evas_list_count(edje_collections) - 1;
}

static void
st_collections_group_name(void)
{
   Edje_Part_Collection_Directory_Entry *de;
   
   de = evas_list_data(evas_list_last(edje_file->collection_dir->entries));
   de->entry = parse_str(0);
}

static void
ob_collections_group_parts(void)
{
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
}

static void
st_collections_group_parts_part_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->name = parse_str(0);
}

static void
st_collections_group_parts_part_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->type = parse_enum(0, 
			 "NONE", EDJE_PART_TYPE_NONE,
			 "RECT", EDJE_PART_TYPE_RECTANGLE,
			 "TEXT", EDJE_PART_TYPE_TEXT,
			 "IMAGE", EDJE_PART_TYPE_IMAGE,
			 NULL);
}

static void
st_collections_group_parts_part_mouse_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->mouse_events = parse_int_range(0, 0, 1);
}

static void
st_collections_group_parts_part_color_class(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->color_class = parse_str(0);
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
   ed->dragable.confine_id = -1;
   ed->clip_to_id = -1;
   ed->align.x = 0.5;
   ed->align.y = 0.5;
   ed->min.w = 0;
   ed->min.h = 0;
   ed->max.w = -1;
   ed->max.h = -1;
   ed->rel1.relative_x = 0.0;
   ed->rel1.relative_y = 0.0;
   ed->rel1.offset_x = 0;
   ed->rel1.offset_x = 0;
   ed->rel1.id = -1;
   ed->rel2.relative_x = 1.0;
   ed->rel2.relative_y = 1.0;
   ed->rel2.offset_x = -1;
   ed->rel2.offset_x = -1;
   ed->rel2.id = -1;
   ed->image.id = -1;
   ed->fill.pos_rel_x = 0.0;
   ed->fill.pos_abs_x = 0;
   ed->fill.rel_x = 1.0;
   ed->fill.abs_x = 0;
   ed->fill.pos_rel_y = 0.0;
   ed->fill.pos_abs_y = 0;
   ed->fill.rel_y = 1.0;
   ed->fill.abs_y = 0;
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
}

static void
st_collections_group_parts_part_description_state(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->state.name = parse_str(0);
   ed->state.value = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_visible(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   
   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->visible = parse_int_range(0, 0, 1);
}

static void
st_collections_group_parts_part_description_dragable_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->dragable.x = parse_int_range(0, 0, 1);
   ed->dragable.step_x = parse_int_range(1, 1, 0x7fffffff);
   ed->dragable.count_x = parse_int_range(2, 1, 0x7fffffff);
}

static void
st_collections_group_parts_part_description_dragable_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->dragable.y = parse_int_range(0, 0, 1);
   ed->dragable.step_y = parse_int_range(1, 1, 0x7fffffff);
   ed->dragable.count_y = parse_int_range(2, 1, 0x7fffffff);
}

static void
st_collections_group_parts_part_description_dragable_confine(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->dragable.confine_id));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_align(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->align.x = parse_float_range(0, 0.0, 1.0);
   ed->align.y = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_min(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

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

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->aspect.min = parse_float_range(0, 0.0, 999999999.0);
   ed->aspect.max = parse_float_range(1, 0.0, 999999999.0);
}

static void
st_collections_group_parts_part_description_rel1_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel1.relative_x = parse_float_range(0, 0.0, 1.0);
   ed->rel1.relative_y = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_rel1_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

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

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel2.relative_x = parse_float_range(0, 0.0, 1.0);
   ed->rel2.relative_y = parse_float_range(1, 0.0, 1.0);
}

static void
st_collections_group_parts_part_description_rel2_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

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

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_image_normal(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
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

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
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
st_collections_group_parts_part_description_border(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->border.l = parse_int_range(0, 0, 0x7fffffff);
   ed->border.r = parse_int_range(1, 0, 0x7fffffff);
   ed->border.t = parse_int_range(2, 0, 0x7fffffff);
   ed->border.b = parse_int_range(3, 0, 0x7fffffff);   
}

static void
st_collections_group_parts_part_description_fill_origin_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.pos_rel_x = parse_float_range(0, 0.0, 999999999.0);   
   ed->fill.pos_rel_y = parse_float_range(1, 0.0, 999999999.0);   
}

static void
st_collections_group_parts_part_description_fill_origin_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
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

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.rel_x = parse_float_range(0, 0.0, 999999999.0);   
   ed->fill.rel_y = parse_float_range(1, 0.0, 999999999.0);   
}

static void
st_collections_group_parts_part_description_fill_size_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.abs_x = parse_int(0);
   ed->fill.abs_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_color(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

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

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.text = parse_str(0);
}

static void
st_collections_group_parts_part_description_text_font(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.font = parse_str(0);
}

static void
st_collections_group_parts_part_description_text_size(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.size = parse_int_range(0, 0, 255);
}

static void
st_collections_group_parts_part_description_text_effect(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.effect = parse_enum(0, 
				"NONE", EDJE_TEXT_EFFECT_NONE,
				"PLAIN", EDJE_TEXT_EFFECT_PLAIN,
				"OUTLINE", EDJE_TEXT_EFFECT_OUTLINE,
				"SHADOW", EDJE_TEXT_EFFECT_SHADOW,
				"SHADOW_OUTLINE", EDJE_TEXT_EFFECT_OUTLINE_SHADOW,
				NULL);
}

static void
st_collections_group_parts_part_description_text_fit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.fit_x = parse_int_range(0, 0, 1);
   ed->text.fit_y = parse_int_range(1, 0, 1);
}

static void
st_collections_group_parts_part_description_text_min(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.min_x = parse_int_range(0, 0, 1);
   ed->text.min_y = parse_int_range(1, 0, 1);
}

static void
st_collections_group_parts_part_description_text_align(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.align.x = parse_float_range(0, 0.0, 1.0);
   ed->text.align.y = parse_float_range(1, 0.0, 1.0);
}


static void
ob_collections_group_programs(void)
{
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
   ep->after = -1;
}

static void
st_collections_group_programs_program_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->name = parse_str(0);
}

static void
st_collections_group_programs_program_signal(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->signal = parse_str(0);
}

static void
st_collections_group_programs_program_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->source = parse_str(0);
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
}

static void
st_collections_group_programs_program_transition(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

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
	else
	  {
	     /* FIXME: not type specified. guess */
	     data_queue_part_lookup(pc, name, &(et->id));
	     data_queue_program_lookup(pc, name, &(et->id));
	  }
	free(name);
     }
}

static void
st_collections_group_programs_program_after(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
     {
	char *name;
	
	name = parse_str(0);
	data_queue_program_lookup(pc, name, &(ep->after));
	free(name);
     }
}
