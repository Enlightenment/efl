#include "edje_cc.h"

typedef struct _Part_Lookup Part_Lookup;
typedef struct _Image_Lookup Image_Lookup;

struct _Part_Lookup
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

static Eet_Data_Descriptor *edd_edje_file = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory_entry = NULL;

static Evas_List *part_lookups = NULL;
static Evas_List *image_lookups = NULL;

void
data_setup(void)
{
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
   edd_edje_file = eet_data_descriptor_new("Edje_File", 
					   sizeof(Edje_File),
					   evas_list_next,
					   evas_list_append,
					   evas_list_data,
					   evas_hash_foreach,
					   evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_SUB(edd_edje_file, Edje_File, "image_dir", image_dir, edd_edje_image_directory);
}

void
data_write(void)
{
   Eet_File *ef;
   Evas_List *l;
   
   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	fprintf(stderr, "%s: Error. unable to open %s for writing output\n",
		progname, file_out);
	exit(-1);
     }
   eet_data_write(ef, edd_edje_file, "data", edje_file, 1);
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
		  int im_w, im_h;
		  int im_alpha;
		  char buf[256];
		  
		  imlib_context_set_image(im);
		  im_w = imlib_image_get_width();
		  im_h = imlib_image_get_height();
		  im_alpha = imlib_image_has_alpha();
		  im_data = imlib_image_get_data_for_reading_only();
		  snprintf(buf, sizeof(buf), "/images/%i", img->id);
		  if (img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT)
		    eet_data_image_write(ef, buf, 
					 im_data, im_w, im_h, im_alpha, 
					 img->source_param, 0, 0);
		  else
		    eet_data_image_write(ef, buf, 
					 im_data, im_w, im_h, im_alpha, 
					 0, img->source_param, 1);
		  imlib_image_put_back_data(im_data);
		  imlib_free_image();
	       }
	     else
	       {
		  fprintf(stderr, "%s: Warning. unable to open image %s for inclusion in output\n",
			  progname, img->entry);			  
	       }
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
