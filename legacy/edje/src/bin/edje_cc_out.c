#include "edje_cc.h"

typedef struct _Part_Lookup Part_Lookup;
typedef struct _Program_Lookup Program_Lookup;
typedef struct _Image_Lookup Image_Lookup;

struct _Part_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
};

struct _Program_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
};

struct _Image_Lookup
{
   char *name;
   int *dest;
};

Edje_File *edje_file = NULL;
Evas_List *edje_collections = NULL;

static Eet_Data_Descriptor *edd_edje_file = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_program = NULL;
static Eet_Data_Descriptor *edd_edje_program_target = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection_directory = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection = NULL;
static Eet_Data_Descriptor *edd_edje_part = NULL;
static Eet_Data_Descriptor *edd_edje_part_description = NULL;
static Eet_Data_Descriptor *edd_edje_part_image_id = NULL;

static Evas_List *part_lookups = NULL;
static Evas_List *program_lookups = NULL;
static Evas_List *image_lookups = NULL;

void
data_setup(void)
{
   /* image directory */
   edd_edje_image_directory_entry = eet_data_descriptor_new("Edje_Image_Directory_Entry",
							    sizeof(Edje_Image_Directory_Entry),
							    evas_list_next,
							    evas_list_append,
							    evas_list_data,
							    evas_hash_foreach,
							    evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);
   
   edd_edje_image_directory = eet_data_descriptor_new("Edje_Image_Directory", 
						      sizeof(Edje_Image_Directory),
						      evas_list_next,
						      evas_list_append,
						      evas_list_data,
						      evas_hash_foreach,
						      evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_image_directory, Edje_Image_Directory, "entries", entries, edd_edje_image_directory_entry);

   /* collection directory */
   edd_edje_part_collection_directory_entry = eet_data_descriptor_new("Edje_Part_Collection_Directory_Entry",
								      sizeof(Edje_Part_Collection_Directory_Entry),
								      evas_list_next,
								      evas_list_append,
								      evas_list_data,
								      evas_hash_foreach,
								      evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "id", id, EET_T_INT);
   
   edd_edje_part_collection_directory = eet_data_descriptor_new("Edje_Part_Collection_Directory",
								sizeof(Edje_Part_Collection_Directory),
								evas_list_next,
								evas_list_append,
								evas_list_data,
								evas_hash_foreach,
								evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_part_collection_directory, Edje_Part_Collection_Directory, "entries", entries, edd_edje_part_collection_directory_entry);
   
   /* the main file directory */
   edd_edje_file = eet_data_descriptor_new("Edje_File", 
					   sizeof(Edje_File),
					   evas_list_next,
					   evas_list_append,
					   evas_list_data,
					   evas_hash_foreach,
					   evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_SUB(edd_edje_file, Edje_File, "image_dir", image_dir, edd_edje_image_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(edd_edje_file, Edje_File, "collection_dir", collection_dir, edd_edje_part_collection_directory);   

   /* parts & programs - loaded induvidually */
   edd_edje_program_target = eet_data_descriptor_new("Edje_Program_Target",
						     sizeof(Edje_Program_Target),
						     evas_list_next,
						     evas_list_append,
						     evas_list_data,
						     evas_hash_foreach,
						     evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program_target, Edje_Program_Target, "id", id, EET_T_INT);
   
   edd_edje_program = eet_data_descriptor_new("Edje_Program",
					      sizeof(Edje_Program),
					      evas_list_next,
					      evas_list_append,
					      evas_list_data,
					      evas_hash_foreach,
					      evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "signal", signal, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "action", action, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "state", state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "value", value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "tween.mode", tween.mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "tween.time", tween.time, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_program, Edje_Program, "targets", targets, edd_edje_program_target);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_program, Edje_Program, "after", after, EET_T_INT);
   
   edd_edje_part_image_id = eet_data_descriptor_new("Edje_Part_Image_Id",
						    sizeof(Edje_Part_Image_Id),
						    evas_list_next,
						    evas_list_append,
						    evas_list_data,
						    evas_hash_foreach,
						    evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_image_id, Edje_Part_Image_Id, "id", id, EET_T_INT);
   
   edd_edje_part_description = eet_data_descriptor_new("Edje_Part_Description",
						       sizeof(Edje_Part_Description),
						       evas_list_next,
						       evas_list_append,
						       evas_list_data,
						       evas_hash_foreach,
						       evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "state.name", state.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "state.value", state.value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "visible", visible, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.x", dragable.x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.step_x", dragable.step_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.count_x", dragable.count_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.y", dragable.y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.step_y", dragable.step_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.count_y", dragable.count_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "dragable.counfine_id", dragable.confine_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "clip_to_id", clip_to_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "align.x", align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "align.y", align.y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "min.w", min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "min.h", min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "max.w", max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "max.h", max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "step.x", step.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "step.y", step.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "aspect.min", aspect.min, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "aspect.max", aspect.max, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel1.relative_x", rel1.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel1.relative_y", rel1.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel1.offset_x", rel1.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel1.offset_y", rel1.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel1.id", rel1.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel2.relative_x", rel2.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel2.relative_y", rel2.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel2.offset_x", rel2.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel2.offset_y", rel2.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "rel2.id", rel2.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "image.id", image.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_part_description, Edje_Part_Description, "image.tween_list", image.tween_list, edd_edje_part_image_id);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "border.l", border.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "border.r", border.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "border.t", border.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "border.b", border.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.pos_rel_x", fill.pos_rel_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.pos_abs_x", fill.pos_abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.rel_x", fill.rel_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.abs_x", fill.abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.pos_rel_y", fill.pos_rel_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.pos_abs_y", fill.pos_abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.rel_y", fill.rel_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "fill.abs_y", fill.abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color.r", color.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color.g", color.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color.b", color.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color.a", color.a, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color2.r", color2.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color2.g", color2.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color2.b", color2.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color2.a", color2.a, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color3.r", color3.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color3.g", color3.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color3.b", color3.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "color3.a", color3.a, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.text", text.text, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.font", text.font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.size", text.size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.effect", text.effect, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.fit_x", text.fit_x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.fit_y", text.fit_y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.min_x", text.min_x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.min_y", text.min_y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.align.x", text.align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_description, Edje_Part_Description, "text.align.y", text.align.y, EET_T_DOUBLE);
   
   edd_edje_part = eet_data_descriptor_new("Edje_Part",
					   sizeof(Edje_Part),
					   evas_list_next,
					   evas_list_append,
					   evas_list_data,
					   evas_hash_foreach,
					   evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part, Edje_Part, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part, Edje_Part, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part, Edje_Part, "type", type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part, Edje_Part, "mouse_events", mouse_events, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part, Edje_Part, "color_class", color_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part, Edje_Part, "text_class", text_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_SUB(edd_edje_part, Edje_Part, "default_desc", default_desc, edd_edje_part_description);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_part, Edje_Part, "other_desc", other_desc, edd_edje_part_description);
   
   edd_edje_part_collection  = eet_data_descriptor_new("Edje_Part_Collection", 
						       sizeof(Edje_Part_Collection),
						       evas_list_next,
						       evas_list_append,
						       evas_list_data,
						       evas_hash_foreach,
						       evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_part_collection, Edje_Part_Collection, "programs", programs, edd_edje_program);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd_edje_part_collection, Edje_Part_Collection, "parts", parts, edd_edje_part);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_edje_part_collection, Edje_Part_Collection, "id", id, EET_T_INT);
}

void
data_write(void)
{
   Eet_File *ef;
   Evas_List *l;
   int bytes;
   
   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	fprintf(stderr, "%s: Error. unable to open \"%s\" for writing output\n",
		progname, file_out);
	exit(-1);
     }
   bytes = eet_data_write(ef, edd_edje_file, "edje_file", edje_file, 1);
   if (bytes <= 0)
     {
	fprintf(stderr, "%s: Error. unable to write \"edje_file\" entry to \"%s\" \n",
		progname, file_out);	
	exit(-1);	
     }
   if (verbose)
     {
	printf("%s: Wrote %9i bytes (%4iKb) for \"edje_file\" header\n",
	       progname, bytes, (bytes + 512) / 1024);
     }
   for (l = edje_file->image_dir->entries; l; l = l->next)
     {
	Edje_Image_Directory_Entry *img;
	
	img = l->data;	
	if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
	  {
	     Imlib_Image im;
	     Evas_List *l;

	     im = NULL;
	     imlib_set_cache_size(0);	     
	     for (l = img_dirs; l; l = l->next)
	       {
		  char buf[4096];
		  
		  snprintf(buf, sizeof(buf), "%s/%s", l->data, img->entry);
		  im = imlib_load_image(buf);
		  if (im) break;
	       }
	     if (!im) im = imlib_load_image(img->entry);
	     if (im)
	       {
		  DATA32 *im_data;
		  int  im_w, im_h;
		  int  im_alpha;
		  char buf[256];
		  
		  imlib_context_set_image(im);
		  im_w = imlib_image_get_width();
		  im_h = imlib_image_get_height();
		  im_alpha = imlib_image_has_alpha();
		  im_data = imlib_image_get_data_for_reading_only();
		  snprintf(buf, sizeof(buf), "images/%i", img->id);
		  if (img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT)
		    bytes = eet_data_image_write(ef, buf, 
						 im_data, im_w, im_h,
						 im_alpha, 
						 img->source_param, 0, 0);
		  else
		    bytes = eet_data_image_write(ef, buf, 
						 im_data, im_w, im_h,
						 im_alpha,
						 0, img->source_param, 1);
		  if (bytes <= 0)
		    {
		       fprintf(stderr, "%s: Error. unable to write image part \"%s\" as \"%s\" part entry to %s \n",
			       progname, img->entry, buf, file_out);	
		       exit(-1);
		    }
		  if (verbose)
		    {
		       printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" image entry \"%s\"\n",
			      progname, bytes, (bytes + 512) / 1024, buf, img->entry);
		    }
		  imlib_image_put_back_data(im_data);
		  imlib_free_image();
	       }
	     else
	       {
		  fprintf(stderr, "%s: Warning. unable to open image \"%s\" for inclusion in output\n",
			  progname, img->entry);			  
	       }
	  }
     }
   for (l = edje_collections; l; l = l->next)
     {
	Edje_Part_Collection *pc;
	char buf[456];
	
	pc = l->data;
	
	snprintf(buf, sizeof(buf), "collections/%i", pc->id);
	bytes = eet_data_write(ef, edd_edje_part_collection, buf, pc, 1);
	if (bytes <= 0)
	  {
	     fprintf(stderr, "%s: Error. unable to write \"%s\" part entry to %s \n",
		     progname, buf, file_out);	
	     exit(-1);
	  }
	if (verbose)
	  {
	     printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" collection entry\n",
		    progname, bytes, (bytes + 512) / 1024, buf);
	  }
     }
   eet_close(ef);
}

void
data_queue_part_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   Part_Lookup *pl;
   
   pl = mem_alloc(SZ(Part_Lookup));
   part_lookups = evas_list_append(part_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_program_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   Program_Lookup *pl;
   
   pl = mem_alloc(SZ(Program_Lookup));
   program_lookups = evas_list_append(program_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_image_lookup(char *name, int *dest)
{
   Image_Lookup *il;
   
   il = mem_alloc(SZ(Image_Lookup));
   image_lookups = evas_list_append(image_lookups, il);
   il->name = mem_strdup(name);
   il->dest = dest;
}

void
data_process_lookups(void)
{
   Evas_List *l;
   
   while (part_lookups)
     {
	Part_Lookup *pl;
	
	pl = part_lookups->data;
	
	for (l = pl->pc->parts; l; l = l->next)
	  {
	     Edje_Part *ep;
	     
	     ep = l->data;
	     if ((ep->name) && (!strcmp(ep->name, pl->name)))
	       {
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable find part name %s\n",
		     progname, pl->name);
	     exit(-1);
	  }
	part_lookups = evas_list_remove(part_lookups, pl);
	free(pl->name);
	free(pl);
     }

   while (program_lookups)
     {
	Program_Lookup *pl;
	
	pl = program_lookups->data;
	
	for (l = pl->pc->programs; l; l = l->next)
	  {
	     Edje_Program *ep;
	     
	     ep = l->data;
	     if ((ep->name) && (!strcmp(ep->name, pl->name)))
	       {
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable find program name %s\n",
		     progname, pl->name);
	     exit(-1);
	  }
	program_lookups = evas_list_remove(program_lookups, pl);
	free(pl->name);
	free(pl);
     }
   
   while (image_lookups)
     {
	Image_Lookup *il;
	
	il = image_lookups->data;
	
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *de;
	     
	     de = l->data;
	     if ((de->entry) && (!strcmp(de->entry, il->name)))
	       {
		  *(il->dest) = de->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable find image name %s\n",
		     progname, il->name);
	     exit(-1);
	  }
	image_lookups = evas_list_remove(image_lookups, il);
	free(il->name);
	free(il);
     }
}
