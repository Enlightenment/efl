#include "edje_cc.h"

static void ob_images(void);
static void ob_images_image(void);
static void st_images_image(void);

static void ob_collections(void);

/*****/

New_Object_Handler object_handlers[] =
{
     {"images", ob_images},
     {"images.image", ob_images_image},
     {"collections", ob_collections},
     {"collections.group", NULL},
     {"collections.group.name", NULL},
     {"collections.group.parts", NULL},
     {"collections.group.parts.part", NULL},
     {"collections.group.parts.part.name", NULL},
     {"collections.group.parts.part.type", NULL},
     {"collections.group.parts.part.mouse_events", NULL},
     {"collections.group.parts.part.color_class", NULL},
     {"collections.group.parts.part.description", NULL},
     {"collections.group.parts.part.description.state", NULL},
     {"collections.group.parts.part.description.visible", NULL},
     {"collections.group.parts.part.description.dragable", NULL},
     {"collections.group.parts.part.description.dragable.x", NULL},
     {"collections.group.parts.part.description.dragable.y", NULL},
     {"collections.group.parts.part.description.dragable.confine", NULL},
     {"collections.group.programs", NULL},
     {"collections.group.programs.program", NULL}
};

New_Statement_Handler statement_handlers[] =
{
     {"images.image", st_images_image},
     {"collections.group.name", NULL},
     {"collections.group.parts.part.name", NULL},
     {"collections.group.parts.part.type", NULL},
     {"collections.group.parts.part.mouse_events", NULL},
     {"collections.group.parts.part.color_class", NULL},
     {"collections.group.parts.part.description.state", NULL},
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
   edje_file->image_dir = calloc(1, sizeof(Edje_Image_Directory));
   if (!edje_file->image_dir)
     {
	fprintf(stderr, "%s: Error. memory allocation of %i bytes failed. %s\n",
		progname, sizeof(Edje_Image_Directory), strerror(errno));
	exit(-1);
     }
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
}
