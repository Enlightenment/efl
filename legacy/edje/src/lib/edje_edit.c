#include "edje_private.h"

int edje_edit_init()
{
  edje_init();
  return 1;
}

void edje_edit_shutdown()
{
  edje_shutdown();
}

Edje_Edit_File *
edje_edit_file_load(char *file)
{
  Edje_Edit_File *edf;
  Eet_File *ef;
  char **collections, **images;
  int count = 0, i;
  
  
  edf = calloc(1, sizeof(Edje_Edit_File));

  ef = eet_open(file, EET_FILE_MODE_READ);
  if (!ef) return NULL; 
  
  edf->file = (Edje_File *)eet_data_read(ef, _edje_edd_edje_file, "edje_file");
 
  /* load collections */
  collections = eet_list(ef, "collections*", &count);
  for (i=0; i < count; i++)
  {
    Edje_Part_Collection *coll;

    /* we can load these up in any order, as the structs contain their id */
    coll = (Edje_Part_Collection *)eet_data_read(ef, _edje_edd_edje_part_collection, collections[i]);
    edf->collections = evas_list_append(edf->collections, coll);
  }
  count = 0;

  /* load images */
  images = eet_list(ef, "images*", &count);
  for (i=0; i < count; i++)
  {
    Edje_Edit_Image *im = calloc(1, sizeof(Edje_Edit_Image));
    char buf[1024];

    im->id = i;
    /* make sure we match up images and ids (images[] isn't in order) */
    snprintf(buf, 1024, "images/%i", im->id);

    im->data = eet_data_image_read(ef, buf, &(im->w), &(im->h),
                                   &(im->alpha), &(im->compress),
				   &(im->quality), &(im->lossy));
    edf->images = evas_list_append(edf->images, im);
  }

  eet_close(ef);
  return edf;
}

int
edje_edit_file_save(Edje_Edit_File *edf, char *path)
{
  Eet_File *ef;
  Evas_List *l = NULL;
  int i;

  if (!path) path = edf->file->path;
  
  if (edf->file->path && strcmp(path, edf->file->path))
  {
    free(edf->file->path);
    edf->file->path = strdup(path);
  }

  ef = eet_open(path, EET_FILE_MODE_WRITE);
  if (!ef) return 0; 

  for (l = edf->collections, i = 0; l; l = l->next, i++)
  {
    char buf[256];
    Edje_Part_Collection *coll = l->data;

    snprintf(buf, sizeof(buf), "collections/%d", i);
    if (!eet_data_write(ef, _edje_edd_edje_part_collection, buf, coll, 1))
    {
      fprintf(stderr, "Error (edje_edit_file_save): can't write collection %d.\n", i);
    }
  }
  
  for (l = edf->images, i = 0; l; l = l->next, i++)
  {
    char buf[256];
    Edje_Edit_Image *im = l->data;

    snprintf(buf, sizeof(buf), "images/%i", i);
    eet_data_image_write(ef, buf, im->data, im->w, im->h, im->alpha,
                         im->compress, im->quality, im->lossy);
  }

  eet_data_write(ef, _edje_edd_edje_file, "edje_file", edf->file, 1);

  eet_close(ef);

  return 1;
}

void
edje_edit_file_free(Edje_Edit_File *edf)
{
  Evas_List *l;

  for (l = edf->collections; l; l = l->next)
  {
    /* FIXME: do this right */
    free(l->data);
  }
  for (l = edf->images; l; l = l->next)
  {
    free(l->data);
  }

  evas_list_free(edf->images);
  evas_list_free(edf->collections);
  _edje_file_free(edf->file);

  free(edf);
}

Edje_Part_Collection *
edje_edit_file_part_collection_get(Edje_Edit_File *edf, char *collection_name)
{
  Evas_List *l;
  int id=-1;

  if (!edf || !edf->file || !edf->file->collection_dir) return NULL;

  for (l = edf->file->collection_dir->entries; l; l = l->next)
  {
    Edje_Part_Collection_Directory_Entry *entry = l->data;
    if (!strcmp(entry->entry, "icon"))
    {
      id = entry->id;
      break;
    }
  }
  if (id == -1) return NULL;

  return evas_list_nth(edf->collections, id);
}

Edje_Part_Collection *
edje_edit_part_collection_add(Edje_Edit_File *edf, char *collection_name)
{
}

Edje_Part_Collection *
edje_edit_part_collection_del(Edje_Edit_File *edf, char *collection_name)
{
}

Edje_Part *
edje_edit_part_add(Edje_Part_Collection *coll, char *part_name, int part_type)
{
}

Edje_Part *
edje_edit_part_del(Edje_Part_Collection *coll, char *part_name, int part_type)
{
}

Edje_Program *
edje_edit_program_add(Edje_Part_Collection *coll, char *prog_name)
{
}

Edje_Program *
edje_edit_program_del(Edje_Part_Collection *coll, char *prog_name)
{
}


Edje_Edit_Image *
edje_edit_image_add(Edje_Edit_File *edf, char *filename)
{
}

void
edje_edit_image_del(Edje_Edit_Image *im)
{
}


Edje_Edit_Image *
edje_edit_iamge_get_by_id(int id)
{
}



