#include "edje_cc.h"

Edje_File *edje_file = NULL;

static Eet_Data_Descriptor *edd_edje_file;
static Eet_Data_Descriptor *edd_edje_image_directory;
static Eet_Data_Descriptor *edd_edje_image_directory_entry;

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
