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
    
static void ob_collections_group_programs(void);

static void ob_collections_group_programs_program(void);

/*****/

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
     {"collections.group.programs", ob_collections_group_programs},
     {"collections.group.programs.program", ob_collections_group_programs_program}
};

New_Statement_Handler statement_handlers[] =
{
     {"images.image", st_images_image},
     {"collections.group.name", st_collections_group_name},
     {"collections.group.parts.part.name", st_collections_group_parts_part_name},
     {"collections.group.parts.part.type", st_collections_group_parts_part_type},
     {"collections.group.parts.part.mouse_events", st_collections_group_parts_part_mouse_events},
     {"collections.group.parts.part.color_class", st_collections_group_parts_part_color_class},
     {"collections.group.parts.part.description.state", st_collections_group_parts_part_description_state},
     {"collections.group.parts.part.description.visible", NULL},
     {"collections.group.parts.part.description.dragable.x", NULL},
     {"collections.group.parts.part.description.dragable.y", NULL},
     {"collections.group.parts.part.description.dragable.confine", NULL}
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
   edje_file->collection_loaded = evas_list_append(edje_file->collection_loaded, pc);
   pc->id = evas_list_count(edje_file->collection_loaded) - 1;
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
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   pc->parts = evas_list_append(pc->parts, ep);
   ep->id = evas_list_count(pc->parts) - 1;
}

static void
st_collections_group_parts_part_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->name = parse_str(0);
}

static void
st_collections_group_parts_part_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->type = parse_enum(0, 
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
   
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->mouse_events = parse_int_range(0, 0, 1);
}

static void
st_collections_group_parts_part_color_class(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->color_class = parse_str(0);
}

static void
ob_collections_group_parts_part_description(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = mem_alloc(SZ(Edje_Part_Description));
   if (!ep->default_desc)
     ep->default_desc = ed;
   else
     ep->other_desc = evas_list_append(ep->other_desc, ed);
}

static void
st_collections_group_parts_part_description_state(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   
   pc = evas_list_data(evas_list_last(edje_file->collection_loaded));
   ep = evas_list_data(evas_list_last(pc->parts));   
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->state.name = parse_str(0);
   ed->state.value = parse_float_range(1, 0.0, 1.0);
}

static void
ob_collections_group_programs(void)
{
}

static void
ob_collections_group_programs_program(void)
{
}
