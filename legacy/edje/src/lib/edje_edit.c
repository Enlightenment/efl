/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * TODO
 * -----------------------------------------------------------------
 * Modify edje so that also ebryo source is included in the eet file
 * Remove images/fonts
 *
 */

#include "edje_private.h"
#include "edje_cc.h"

/* Get ed(Edje*) from obj(Evas_Object*) */
#define GET_ED_OR_RETURN(RET) \
   Edje *ed; \
   ed = _edje_fetch(obj); \
   if (!ed) return RET;

/* Get rp(Edje_Real_Part*) from obj(Evas_Object*) and part(char*) */
#define GET_RP_OR_RETURN(RET) \
   Edje *ed; \
   Edje_Real_Part *rp; \
   ed = _edje_fetch(obj); \
   if (!ed) return RET; \
   rp = _edje_real_part_get(ed, part); \
   if (!rp) return RET;

/* Get pd(Edje_Part_Description*) from obj(Evas_Object*), part(char*) and state (char*) */
#define GET_PD_OR_RETURN(RET) \
   Edje *ed; \
   Edje_Part_Description *pd; \
   ed = _edje_fetch(obj); \
   if (!ed) return RET; \
   pd = _edje_part_description_find_byname(ed, part, state); \
   if (!pd) return RET;

/* Get epr(Edje_Program*) from obj(Evas_Object*) and prog(char*)*/
#define GET_EPR_OR_RETURN(RET) \
   Edje_Program *epr; \
   epr = _edje_program_get_byname(obj, prog); \
   if (!epr) return RET;

void *
mem_alloc(size_t size)
{
   void *mem;

   mem = calloc(1, size);
   if (mem) return mem;
   fprintf(stderr, "Edje_Edit: Error. memory allocation of %i bytes failed. %s\n",
           size, strerror(errno));
   exit(-1);
   return NULL;
}

char *
mem_strdup(const char *s)
{
   void *str;

   str = strdup(s);
   if (str) return str;
   fprintf(stderr, "Edje_Edit: Error. memory allocation of %i bytes failed. %s."
           "string being duplicated: \"%s\"\n", strlen(s) + 1, strerror(errno), s);
   exit(-1);
   return NULL;
}


/*************/
/* INTERNALS */
/*************/

static Edje_Part_Description *
_edje_part_description_find_byname(Edje *ed, const char *part, const char *state) //state include the value in the string (ex. "default 0.00")
{
   Edje_Real_Part *rp;
   Edje_Part_Description *pd;
   char *delim;
   double value;
   char *name;

   if (!ed || !part || !state) return NULL;

   rp = _edje_real_part_get(ed, part);
   if (!rp) return NULL;

   name = strdup(state);
   delim = strrchr(name, (int)' ');
   if (!delim)
     {
	free(name);
	return NULL;
     }

   if (sscanf(delim, "%lf", &value) != 1)
     {
	free(name);
	return NULL;
     }

   delim[0] = '\0';
   //printf("SEARCH DESC(%s): %s %f\n", state, state, value);
   pd = _edje_part_description_find(ed, rp, name, value);

   free(name);

   if (!pd) return NULL;
   return pd;
}

static int
_edje_image_id_find(Evas_Object *obj, const char *image_name)
{
   Edje_Image_Directory_Entry *i;
   Evas_List *l;

   GET_ED_OR_RETURN(-1);

   if (!ed->file) return -1;
   if (!ed->file->image_dir) return -1;

   //printf("SEARCH IMAGE %s\n", image_name);

   for (l = ed->file->image_dir->entries; l; l = l->next)
     {
	i = l->data;
	if (strcmp(image_name, i->entry) == 0)
	  {
	     //printf("   Found id: %d \n", i->id);
	     return i->id;
	  }
     }

   return -1;
}

static const char *
_edje_image_name_find(Evas_Object *obj, int image_id)
{
   Edje_Image_Directory_Entry *i;
   Evas_List *l;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->image_dir) return NULL;

   //printf("SEARCH IMAGE ID %d\n", image_id);

   for (l = ed->file->image_dir->entries; l; l = l->next)
     {
	i = l->data;
	if (image_id == i->id)
	  return i->entry;
     }

   return NULL;
}

static void
_edje_real_part_free(Edje_Real_Part *rp)
{
   Evas_List *l;

   if (!rp) return;

   if (rp->object)
     {
	_edje_text_real_part_on_del(rp->edje, rp);
	_edje_callbacks_del(rp->object);
	evas_object_del(rp->object);
     }

   l = rp->extra_objects;
   while (rp->extra_objects)
     {
	evas_object_del(rp->extra_objects->data);
	rp->extra_objects = evas_list_remove_list(rp->extra_objects,
						  rp->extra_objects);
     }

   if (rp->swallowed_object)
     {
	evas_object_smart_member_del(rp->swallowed_object);
	evas_object_event_callback_del(rp->swallowed_object,
				       EVAS_CALLBACK_FREE,
				       _edje_object_part_swallow_free_cb);
	evas_object_clip_unset(rp->swallowed_object);
	evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");
	if (rp->part->mouse_events)
	  _edje_callbacks_del(rp->swallowed_object);

	if (rp->part->type == EDJE_PART_TYPE_GROUP)
	  evas_object_del(rp->swallowed_object);

	rp->swallowed_object = NULL;
     }

   if (rp->text.text) evas_stringshare_del(rp->text.text);
   if (rp->text.font) evas_stringshare_del(rp->text.font);
   if (rp->text.cache.in_str) evas_stringshare_del(rp->text.cache.in_str);
   if (rp->text.cache.out_str) evas_stringshare_del(rp->text.cache.out_str);

   if (rp->custom.description)
     _edje_collection_free_part_description_free(rp->custom.description, 0);

   _edje_unref(rp->edje);
   free(rp);
}

static unsigned char
_edje_import_image_file(Edje *ed, const char *path, int id)
{
   char buf[256];
   Evas_Object *im;
   Eet_File *eetf;
   void *im_data;
   int  im_w, im_h;
   int  im_alpha;
   int bytes;

   /* Try to load the file */
   im = evas_object_image_add(ed->evas);
   if (!im) return 0;

   evas_object_image_file_set(im, path, NULL);
   if (evas_object_image_load_error_get(im) != EVAS_LOAD_ERROR_NONE)
     {
	fprintf(stderr, "Edje_Edit: Error. unable to load image \"%s\"."
		"Missing PNG or JPEG loader modules for Evas or "
		"file does not exist, or is not readable.\n", path);
	evas_object_del(im);
	im = NULL;
	return 0;
     }

   if (!im) return 0;

   /* Write the loaded image to the edje file */

   evas_object_image_size_get(im, &im_w, &im_h);
   im_alpha = evas_object_image_alpha_get(im);
   im_data = evas_object_image_data_get(im, 0);
   if ((!im_data) || !(im_w > 0) || !(im_h > 0))
     {
	evas_object_del(im);
	return 0;
     }

   /* open the eet file */
   eetf = eet_open(ed->path, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     {
	fprintf(stderr,
		"Edje_Edit: Error. unable to open \"%s\" for writing output\n",
		ed->path);
	evas_object_del(im);
	return 0;
     }

   snprintf(buf, sizeof(buf), "images/%i", id);

   /* write the image data */
   printf("***********  Writing images/%i to edj ******************\n", id);
   bytes = eet_data_image_write(eetf, buf,
				im_data, im_w, im_h,
				im_alpha,
				0, 100, 1);
   if (bytes <= 0)
     {
	fprintf(stderr, "Edje_Edit: Error. unable to write image part \"%s\" "
	"part entry to %s\n", buf, ed->path);
	evas_object_del(im);
	return 0;
     }

   /* Rewrite Edje_File to edj */
   evas_object_del(im);

   printf("***********  Writing Edje_File* ed->file ******************\n");
   bytes = eet_data_write(eetf, _edje_edd_edje_file, "edje_file", ed->file, 1);
   if (bytes <= 0)
     {
	fprintf(stderr, "Edje_Edit: Error. unable to write \"edje_file\" "
	        "entry to \"%s\" \n", ed->path);
	eet_close(eetf);
	return 0;
     }

   eet_close(eetf);
   return 1;
}

static int
_edje_part_id_find(Edje *ed, const char *part)
{
   int id;

   for (id = 0; id < ed->table_parts_size; id++)
     {
	Edje_Real_Part *rp = ed->table_parts[id];

	if (!strcmp(rp->part->name, part))
	  return id;
     }
   return -1;
}

static void
_edje_part_id_set(Edje *ed, Edje_Real_Part *rp, int new_id)
{
   /* This function change the id of a given real_part.
    * All the depedency will be updated too.
    * Also the table_parts is updated, and the current *rp in the table
    * is lost.
    * If new Id = -1 then all the depencies will be deleted
    */
   int old_id;
   Edje_Part *part;
   Evas_List *l, *ll;

   part = rp->part;

   if (!part) return;
   printf("CHANGE ID OF PART %s TO %d\n", part->name, new_id);

   if (!ed || !part || new_id < -1) return;

   if (part->id == new_id) return;

   old_id = part->id;
   part->id = new_id;

   /* Fix all the dependecies in all parts... */
   for (l = ed->collection->parts; l; l = l->next)
     {
	Edje_Part *p;
	Edje_Part_Description *d;

	p = l->data;
	//printf("   search id: %d in %s\n", old_id, p->name);
	if (p->clip_to_id == old_id) p->clip_to_id = new_id;
	if (p->dragable.confine_id == old_id) p->dragable.confine_id = new_id;

	/* ...in default description */
	d = p->default_desc;
	//printf("      search in %s (%s)\n", p->name, d->state.name);
	if (d->rel1.id_x == old_id) d->rel1.id_x = new_id;
	if (d->rel1.id_y == old_id) d->rel1.id_y = new_id;
	if (d->rel2.id_x == old_id) d->rel2.id_x = new_id;
	if (d->rel2.id_y == old_id) d->rel2.id_y = new_id;
	if (d->text.id_source == old_id) d->text.id_source = new_id;
	if (d->text.id_text_source == old_id) d->text.id_text_source = new_id;
	/* ...and in all other descriptions */
	for (ll = p->other_desc; ll; ll = ll->next)
	  {
	     d = ll->data;
	     //printf("      search in %s (%s)\n", p->name, d->state.name);
	     if (d->rel1.id_x == old_id) d->rel1.id_x = new_id;
	     if (d->rel1.id_y == old_id) d->rel1.id_y = new_id;
	     if (d->rel2.id_x == old_id) d->rel2.id_x = new_id;
	     if (d->rel2.id_y == old_id) d->rel2.id_y = new_id;
	     if (d->text.id_source == old_id) d->text.id_source = new_id;
	     if (d->text.id_text_source == old_id) d->text.id_text_source = new_id;
	  }
     }

   /*...and also in programs targets */
   for (l = ed->collection->programs; l; l = l->next)
     {
	Edje_Program *epr;

	epr = l->data;
	if (epr->action != EDJE_ACTION_TYPE_STATE_SET)
	  continue;

	for (ll = epr->targets; ll; ll = ll->next)
	  {
	     Edje_Program_Target *pt;

	     pt = ll->data;
	     if (pt->id == old_id)
	       {
		  if (new_id == -1)
		    epr->targets = evas_list_remove(epr->targets, pt);
		  else
		    pt->id = new_id;
	       }
	  }
     }

   /* Adjust table_parts */
   if (new_id >= 0)
     ed->table_parts[new_id] = rp;
}

static void
_edje_parts_id_switch(Edje *ed, Edje_Real_Part *rp1, Edje_Real_Part *rp2)
{
   /* This function switch the id of two parts.
    * All the depedency will be updated too.
    * Also the table_parts is updated,
    * The parts list isn't touched
    */
   int id1;
   int id2;
   Evas_List *l, *ll;

   printf("SWITCH ID OF PART %d AND %d\n", rp1->part->id, rp2->part->id);

   if (!ed || !rp1 || !rp2) return;
   if (rp1 == rp2) return;

   id1 = rp1->part->id;
   id2 = rp2->part->id;

   /* Switch ids */
   rp1->part->id = id2;
   rp2->part->id = id1;

   /* adjust table_parts */
   ed->table_parts[id1] = rp2;
   ed->table_parts[id2] = rp1;

   // Fix all the dependecies in all parts...
   for (l = ed->collection->parts; l; l = l->next)
     {
	Edje_Part *p;
	Evas_List *ll;
	Edje_Part_Description *d;

	p = l->data;
	//printf("   search id: %d in %s\n", old_id, p->name);
	if (p->clip_to_id == id1) p->clip_to_id = id2;
	else if (p->clip_to_id == id2) p->clip_to_id = id1;
	if (p->dragable.confine_id == id1) p->dragable.confine_id = id2;
	else if (p->dragable.confine_id == id2) p->dragable.confine_id = id1;

	// ...in default description
	d = p->default_desc;
	// printf("      search in %s (%s)\n", p->name, d->state.name);
	if (d->rel1.id_x == id1) d->rel1.id_x = id2;
	else if (d->rel1.id_x == id2) d->rel1.id_x = id1;
	if (d->rel1.id_y == id1) d->rel1.id_y = id2;
	else if (d->rel1.id_y == id2) d->rel1.id_y = id1;
	if (d->rel2.id_x == id1) d->rel2.id_x = id2;
	else if (d->rel2.id_x == id2) d->rel2.id_x = id1;
	if (d->rel2.id_y == id1) d->rel2.id_y = id2;
	else if (d->rel2.id_y == id2) d->rel2.id_y = id1;
	if (d->text.id_source == id1) d->text.id_source = id2;
	else if (d->text.id_source == id2) d->text.id_source = id1;
	if (d->text.id_text_source == id1) d->text.id_text_source = id2;
	else if (d->text.id_text_source == id2) d->text.id_text_source = id2;
	// ...and in all other descriptions
	for (ll = p->other_desc; ll; ll = ll->next)
	  {
	     d = ll->data;
	     //printf("      search in %s (%s)\n", p->name, d->state.name);
	     if (d->rel1.id_x == id1) d->rel1.id_x = id2;
	     else if (d->rel1.id_x == id2) d->rel1.id_x = id1;
	     if (d->rel1.id_y == id1) d->rel1.id_y = id2;
	     else if (d->rel1.id_y == id2) d->rel1.id_y = id1;
	     if (d->rel2.id_x == id1) d->rel2.id_x = id2;
	     else if (d->rel2.id_x == id2) d->rel2.id_x = id1;
	     if (d->rel2.id_y == id1) d->rel2.id_y = id2;
	     else if (d->rel2.id_y == id2) d->rel2.id_y = id1;
	     if (d->text.id_source == id1) d->text.id_source = id2;
	     else if (d->text.id_source == id2) d->text.id_source = id1;
	     if (d->text.id_text_source == id1) d->text.id_text_source = id2;
	     else if (d->text.id_text_source == id2) d->text.id_text_source = id2;
	  }
     }
   //...and also in programs targets
   for (l = ed->collection->programs; l; l = l->next)
     {
	Edje_Program *epr;

	epr = l->data;
	if (epr->action != EDJE_ACTION_TYPE_STATE_SET)
	  continue;

	for (ll = epr->targets; ll; ll = ll->next)
	  {
	     Edje_Program_Target *pt;

	     pt = ll->data;
	     if (pt->id == id1) pt->id = id2;
	     else if (pt->id == id2) pt->id = id1;
	  }
     }
   //TODO Real part dependencies are ok?
}

static void
_edje_fix_parts_id(Edje *ed)
{
   /* We use this to clear the id hole leaved when a part is removed.
    * After the execution of this function all parts will have a right
    * (uniqe & ordered) id. The table_parts is also updated.
    */
   Evas_List *l;
   int correct_id;
   int count;

   printf("FIXING PARTS ID \n");

   //TODO order the list first to be more robust

   /* Give a correct id to all the parts */
   correct_id = 0;
   for (l = ed->collection->parts; l; l = l->next)
     {
	Edje_Part *p;

	p = l->data;
	//printf(" [%d]Checking part: %s id: %d\n", correct_id, p->name, p->id);
	if (p->id != correct_id)
	  _edje_part_id_set(ed, ed->table_parts[p->id], correct_id);

	correct_id++;
     }

   /* If we have removed some parts realloc table_parts */
   count = evas_list_count(ed->collection->parts);
   if (count != ed->table_parts_size)
     {
	ed->table_parts = realloc(ed->table_parts, SZ(Edje_Real_Part *) * count);
	ed->table_parts_size = count;
     }

   //printf("\n");
}

static void
_edje_if_string_free(Edje *ed, const char *str)
{
   Eet_Dictionary *dict;

   if (!ed || !str) return;

   dict = eet_dictionary_get(ed->file->ef);
   if (eet_dictionary_string_check(dict, str)) return;
   evas_stringshare_del(str);
   str = NULL;
}

Edje_Spectrum_Directory_Entry *
_edje_edit_spectrum_entry_get(Edje *ed, const char* spectra)
{
   Edje_Spectrum_Directory_Entry *s;
   Evas_List *l;

   if (!ed->file || !spectra || !ed->file->spectrum_dir)
      return NULL;

   for (l = ed->file->spectrum_dir->entries; l; l = l->next)
     {
	Edje_Spectrum_Directory_Entry *s;

	s = l->data;
	if (!strcmp(s->entry, spectra))
	   return s;
     }

   return NULL;
}

Edje_Spectrum_Directory_Entry *
_edje_edit_spectrum_entry_get_by_id(Edje *ed, int spectra_id)
{
   Evas_List *l;

   if (!ed->file || !ed->file->spectrum_dir)
      return NULL;

   for (l = ed->file->spectrum_dir->entries; l; l = l->next)
     {
	Edje_Spectrum_Directory_Entry *s;

	s = l->data;
	if (s->id == spectra_id)
	   return s;
     }

   return NULL;
}

/*****************/
/*  GENERAL API  */
/*****************/

EAPI void
edje_edit_string_list_free(Evas_List *lst)
{
   //printf("FREE LIST: \n");
   while (lst)
     {
	if (lst->data) evas_stringshare_del(lst->data);
	//printf("FREE: %s\n", lst->data);
	lst = evas_list_remove(lst, lst->data);
     }
}

EAPI void
edje_edit_string_free(const char *str)
{
   if (str) evas_stringshare_del(str);
}

EAPI const char*
edje_edit_compiler_get(Evas_Object *obj)
{
   GET_ED_OR_RETURN(0);
   return evas_stringshare_add(ed->file->compiler);
}

/****************/
/*  GROUPS API  */
/****************/

EAPI unsigned char
edje_edit_group_add(Evas_Object *obj, const char *name)
{
   Edje_Part_Collection_Directory_Entry *de;
   Edje_Part_Collection *pc;
   Evas_List *l;
   int id;
   int search;
   //Code *cd;

   GET_ED_OR_RETURN(0);

   printf("ADD GROUP: %s \n", name);

   /* check if a group with the same name already exists */
   for (l = ed->file->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *d = l->data;
	if (!strcmp(d->entry, name))
	  return 0;
     }

   /* Create structs */
   de = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   if (!de) return 0;

   pc = mem_alloc(SZ(Edje_Part_Collection));
   if (!pc)
     {
	free(de);
	return 0;
     }

   /* Search first free id */
   id = 0;
   search = 0;
   while (!id)
     {
	unsigned char found = 0;

	for (l = ed->file->collection_dir->entries; l; l = l->next)
	  {
	     Edje_Part_Collection_Directory_Entry *d = l->data;

	     // printf("search if %d is free [id %d]\n", search, d->id);
	     if (search == d->id)
	       {
		  found = 1;
		  break;
	       }
	  }
	if (!found)
	  id = search;
	else
	  search++;
     }

   /* Init Edje_Part_Collection_Directory_Entry */
   printf(" new id: %d\n", id);
   de->id = id;
   de->entry = mem_strdup(name);
   ed->file->collection_dir->entries = evas_list_append(ed->file->collection_dir->entries, de);

   /* Init Edje_Part_Collection */
   pc->id = id;
   pc->references = 1;  //TODO i'm not shure about this (maybe set to 0 ?)
   pc->programs = NULL;
   pc->parts = NULL;
   pc->data = NULL;
   pc->script = NULL;
   pc->part = evas_stringshare_add(name);

   //cd = mem_alloc(SZ(Code));
   //codes = evas_list_append(codes, cd);

   ed->file->collection_hash = evas_hash_add(ed->file->collection_hash, name, pc);

   return 1;
}


EAPI unsigned char
edje_edit_group_del(Evas_Object *obj)
{
   char buf[32];
   Evas_List *l;
   int i;
   Edje_Part_Collection *g;
   Eet_File *eetf;

   GET_ED_OR_RETURN(0);

   g = ed->collection;
   printf("REMOVE GROUP: %s [id: %d]\n", g->part, g->id);

   /* Don't remove the last group */
   if (evas_list_count(ed->file->collection_dir->entries) < 2)
     return 0;

   /* Remove collection/id from eet file */
   eetf = eet_open(ed->file->path, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     {
	fprintf(stderr, "Edje_Edit: Error. unable to open \"%s\" "
	        "for writing output\n", ed->file->path);
	return 0;
     }
   snprintf(buf, SZ(buf), "collections/%d", g->id);
   eet_delete(eetf, buf);
   eet_close(eetf);

   /* Remove all Edje_Real_Parts */
   for (i = 0; i < ed->table_parts_size; i++)
     _edje_real_part_free(ed->table_parts[i]);
   ed->table_parts_size = 0;
   free(ed->table_parts);
   ed->table_parts = NULL;

   /* Free Group */
   _edje_collection_free(ed->file, g);

   /* Update collection_dir */
   for (l = ed->file->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *e;

	e = l->data;
	printf("  id: %d  entry: %s\n", e->id, e->entry);
	if (e->id == g->id)
	  {
	     ed->file->collection_dir->entries =
		evas_list_remove_list(ed->file->collection_dir->entries, l);
	     //         free(e->entry);  This should be right but cause a segv
	     free(e);
	     e = NULL;
	     break;
	  }
     }

   ed->collection = NULL;

   if (ed->table_programs_size > 0)
     {
	free(ed->table_programs);
	ed->table_programs = NULL;
	ed->table_programs_size = 0;
     }

   return 1;
}

EAPI unsigned char
edje_edit_group_exist(Evas_Object *obj, const char *group)
{
   Evas_List *l;

   GET_ED_OR_RETURN(0);

   for (l = ed->file->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *e;

	e = l->data;
	if (e->entry && !strcmp(e->entry, group))
	  return 1;
     }
   return 0;
}

EAPI unsigned char
edje_edit_group_name_set(Evas_Object *obj, const char *new_name)
{
   Evas_List *l;
   Edje_Part_Collection *pc;

   GET_ED_OR_RETURN(0);

   if (!new_name) return 0;
   if (edje_edit_group_exist(obj, new_name)) return 0;

   pc = ed->collection;

   printf("Set name of current group: %s [id: %d][new name: %s]\n",
	 pc->part, pc->id, new_name);

   //if (pc->part && ed->file->free_strings) evas_stringshare_del(pc->part); TODO FIXME
   pc->part = evas_stringshare_add(new_name);

   for (l = ed->file->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *pce = l->data;

	if (pc->id == pce->id)
	  {
	     ed->file->collection_hash = evas_hash_del(ed->file->collection_hash,
						       pce->entry, NULL);
	     ed->file->collection_hash = evas_hash_add(ed->file->collection_hash,
						       new_name, pc);

	     //if (pce->entry &&  //TODO Also this cause segv
	     //    !eet_dictionary_string_check(eet_dictionary_get(ed->file->ef), pce->entry))
	     //   evas_stringshare_del(pce->entry);
	     pce->entry = evas_stringshare_add(new_name);

	     return 1;
	  }
     }
   return 0;
}

EAPI int
edje_edit_group_min_w_get(Evas_Object *obj)
{
   printf("Get min_w of group\n");
   GET_ED_OR_RETURN(-1);
   if (!ed->collection) return -1;
   return ed->collection->prop.min.w;
}

EAPI void
edje_edit_group_min_w_set(Evas_Object *obj, int w)
{
   printf("Set min_w of group [new w: %d]\n", w);
   GET_ED_OR_RETURN();
   ed->collection->prop.min.w = w;
}

EAPI int
edje_edit_group_min_h_get(Evas_Object *obj)
{
   printf("Get min_h of group\n");
   GET_ED_OR_RETURN(-1);
   if (!ed->collection) return -1;
   return ed->collection->prop.min.h;
}

EAPI void
edje_edit_group_min_h_set(Evas_Object *obj, int h)
{
   printf("Set min_h of group [new h: %d]\n", h);
   GET_ED_OR_RETURN();
   ed->collection->prop.min.h = h;
}

EAPI int
edje_edit_group_max_w_get(Evas_Object *obj)
{
   printf("Get max_w of group\n");
   GET_ED_OR_RETURN(-1);
   if (!ed->collection) return -1;
   return ed->collection->prop.max.w;
}

EAPI void
edje_edit_group_max_w_set(Evas_Object *obj, int w)
{
   printf("Set max_w of group: [new w: %d]\n", w);
   GET_ED_OR_RETURN();
   ed->collection->prop.max.w = w;
}

EAPI int
edje_edit_group_max_h_get(Evas_Object *obj)
{
   printf("Get max_h of group\n");
   GET_ED_OR_RETURN(-1);
   if (!ed->collection) return -1;
   return ed->collection->prop.max.h;
}

EAPI void
edje_edit_group_max_h_set(Evas_Object *obj, int h)
{
   printf("Set max_h of group: [new h: %d]\n", h);
   GET_ED_OR_RETURN();
   ed->collection->prop.max.h = h;
}

/***************/
/*  DATA API   */
/***************/

EAPI Evas_List *
edje_edit_data_list_get(Evas_Object * obj)
{
   Evas_List *datas, *l;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->data)
     return NULL;

   datas = NULL;
   for (l = ed->file->data; l; l = l->next)
     {
	Edje_Data *d = l->data;
	datas = evas_list_append(datas, evas_stringshare_add(d->key));
     }

   return datas;
}

EAPI unsigned char
edje_edit_data_add(Evas_Object *obj, const char *itemname, const char *value)
{
   Evas_List *l;
   Edje_Data *d;

   GET_ED_OR_RETURN(0);

   if (!itemname || !ed->file)
     return 0;

   for (l = ed->file->data; l; l = l->next)
     {
	Edje_Data *dd = l->data;
	if (strcmp(dd->key, itemname) == 0)
	  return 0;
     }

   d = mem_alloc(sizeof(Edje_Data));
   if (!d) return 0;

   d->key = (char*)evas_stringshare_add(itemname);
   if (value) d->value = (char*)evas_stringshare_add(value);
   else d->value = NULL;

   ed->file->data = evas_list_append(ed->file->data, d);

   return 1;
}

EAPI unsigned char
edje_edit_data_del(Evas_Object *obj, const char *itemname)
{
   Evas_List *l;

   GET_ED_OR_RETURN(0);

   if (!itemname || !ed->file || !ed->file->data)
     return 0;

   for (l = ed->file->data; l; l = l->next)
     {
	Edje_Data *d = l->data;
	if (strcmp(d->key, itemname) == 0)
          {
             _edje_if_string_free(ed, d->key);
             _edje_if_string_free(ed, d->value);
             ed->file->data = evas_list_remove(ed->file->data, d);
             free(d);
             return 1;
          }
     }
   return 0;
}

EAPI const char *
edje_edit_data_value_get(Evas_Object * obj, char *itemname)
{
   Evas_List *l;

   GET_ED_OR_RETURN(NULL);

   if (!itemname || !ed->file || !ed->file->data)
     return NULL;

   for (l = ed->file->data; l; l = l->next)
     {
	Edje_Data *d = l->data;
	if (strcmp(d->key, itemname) == 0)
	  return evas_stringshare_add(d->value);
     }

   return NULL;
}

EAPI unsigned char
edje_edit_data_value_set(Evas_Object *obj, const char *itemname, const char *value)
{
   Evas_List *l;

   GET_ED_OR_RETURN(0);

   if (!itemname || !value || !ed->file || !ed->file->data)
     return 0;

   for (l = ed->file->data; l; l = l->next)
     {
	Edje_Data *d = l->data;
	if (strcmp(d->key, itemname) == 0)
	  {
		_edje_if_string_free(ed, d->value);
		d->value = (char*)evas_stringshare_add(value);
		return 1;
	  }
     }

   return 0;
}

EAPI unsigned char
edje_edit_data_name_set(Evas_Object *obj, const char *itemname,  const char *newname)
{
   Evas_List *l;

   GET_ED_OR_RETURN(0);

   if (!itemname || !newname || !ed->file || !ed->file->data)
     return 0;

   for (l = ed->file->data; l; l = l->next)
     {
	Edje_Data *d = l->data;
	if (strcmp(d->key, itemname) == 0)
	  {
		_edje_if_string_free(ed, d->key);
		d->key = (char*)evas_stringshare_add(newname);
		return 1;
	  }
     }

   return 0;
}

/***********************/
/*  COLOR CLASSES API  */
/***********************/

EAPI Evas_List *
edje_edit_color_classes_list_get(Evas_Object * obj)
{
   Evas_List *classes = NULL;
   Evas_List *l;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->color_classes)
      return NULL;
printf("GET CLASSES LIST %d %d\n", evas_list_count(ed->color_classes), evas_list_count(ed->file->color_classes));
   for (l = ed->file->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc;

	cc = l->data;
	classes = evas_list_append(classes, evas_stringshare_add(cc->name));
     }

   return classes;
}

EAPI unsigned char
edje_edit_color_class_colors_get(Evas_Object *obj, const char *class_name, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   Evas_List *l;
   GET_ED_OR_RETURN(0);

   if (!ed->file || !ed->file->color_classes)
      return 0;

   for (l = ed->file->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc;

	cc = l->data;
	if (!strcmp(cc->name, class_name))
	  {
		if (r) *r = cc->r;
		if (g) *g = cc->g;
		if (b) *b = cc->b;
		if (a) *a = cc->a;

		if (r2) *r2 = cc->r2;
		if (g2) *g2 = cc->g2;
		if (b2) *b2 = cc->b2;
		if (a2) *a2 = cc->a2;

		if (r3) *r3 = cc->r3;
		if (g3) *g3 = cc->g3;
		if (b3) *b3 = cc->b3;
		if (a3) *a3 = cc->a3;

		return 1;
	  }
     }
   return 0;
}

EAPI unsigned char
edje_edit_color_class_colors_set(Evas_Object *obj, const char *class_name, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Evas_List *l;
   GET_ED_OR_RETURN(0);

   if (!ed->file || !ed->file->color_classes)
      return 0;

   for (l = ed->file->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc;

	cc = l->data;
	if (!strcmp(cc->name, class_name))
	  {
		if (r > -1) cc->r = r;
		if (g > -1) cc->g = g;
		if (b > -1) cc->b = b;
		if (a > -1) cc->a = a;

		if (r2 > -1) cc->r2 = r2;
		if (g2 > -1) cc->g2 = g2;
		if (b2 > -1) cc->b2 = b2;
		if (a2 > -1) cc->a2 = a2;

		if (r3 > -1) cc->r3 = r3;
		if (g3 > -1) cc->g3 = g3;
		if (b3 > -1) cc->b3 = b3;
		if (a3 > -1) cc->a3 = a3;

		return 1;
	  }
     }
   return 0;
}

EAPI unsigned char
edje_edit_color_class_add(Evas_Object *obj, const char *name)
{
   Evas_List *l;
   Edje_Color_Class *c;

   GET_ED_OR_RETURN(0);

   if (!name || !ed->file)
     return 0;

   for (l = ed->file->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc = l->data;
	if (strcmp(cc->name, name) == 0)
	  return 0;
     }

   c = mem_alloc(sizeof(Edje_Color_Class));
   if (!c) return 0;

   c->name = (char*)evas_stringshare_add(name);
   c->r = c->g = c->b = c->a = 255;
   c->r2 = c->g2 = c->b2 = c->a2 = 255;
   c->r3 = c->g3 = c->b3 = c->a3 = 255;

   ed->file->color_classes = evas_list_append(ed->file->color_classes, c);

   return 1;
}

EAPI unsigned char
edje_edit_color_class_del(Evas_Object *obj, const char *name)
{
   Evas_List *l;

   GET_ED_OR_RETURN(0);

   if (!name || !ed->file || !ed->file->color_classes)
     return 0;

   for (l = ed->file->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc = l->data;
	if (strcmp(cc->name, name) == 0)
          {
             _edje_if_string_free(ed, cc->name);
             ed->file->color_classes = evas_list_remove(ed->file->color_classes, cc);
             free(cc);
             return 1;
          }
     }
   return 0;
}

EAPI unsigned char
edje_edit_color_class_name_set(Evas_Object *obj, const char *name, const char *newname)
{
   Evas_List *l;

   GET_ED_OR_RETURN(0);

   if (!ed->file || !ed->file->color_classes)
      return 0;

   for (l = ed->file->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc;

	cc = l->data;
	if (!strcmp(cc->name, name))
	  {
		_edje_if_string_free(ed, cc->name);
		cc->name = (char*)evas_stringshare_add(newname);
		return 1;
	  }
     }

   return 0;
}


/***************/
/*  PARTS API  */
/***************/

EAPI Evas_List *
edje_edit_parts_list_get(Evas_Object *obj)
{
   Evas_List *parts;
   int i;

   GET_ED_OR_RETURN(NULL);

   //printf("EE: Found %d parts\n", ed->table_parts_size);

   parts = NULL;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	parts = evas_list_append(parts, evas_stringshare_add(rp->part->name));
     }

   return parts;
}

EAPI unsigned char
edje_edit_part_name_set(Evas_Object *obj, const char* part, const char* new_name)
{
   GET_RP_OR_RETURN(0);

   if (!new_name) return 0;
   if (_edje_real_part_get(ed, new_name)) return 0;

   printf("Set name of part: %s [new name: %s]\n", part, new_name);

   _edje_if_string_free(ed, rp->part->name);
   rp->part->name = (char *)evas_stringshare_add(new_name);

   return 1;
}

EAPI unsigned char
edje_edit_part_add(Evas_Object *obj, const char* name, unsigned char type)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Real_Part *rp;

   GET_ED_OR_RETURN(0);

   printf("ADD PART: %s [type: %d]\n", name, type);

   /* Check if part already exists */
   if (_edje_real_part_get(ed, name))
     return FALSE;

   /* Alloc Edje_Part or return */
   ep = mem_alloc(sizeof(Edje_Part));
   if (!ep) return FALSE;

   /* Alloc Edje_Real_Part or return */
   rp = mem_alloc(sizeof(Edje_Real_Part));
   if (!rp)
     {
	free(ep);
	return FALSE;
     }

   /* Init Edje_Part */
   pc = ed->collection;
   pc->parts = evas_list_append(pc->parts, ep);

   ep->id = evas_list_count(pc->parts) - 1;
   ep->type = type;
   ep->name = evas_stringshare_add(name);
   ep->mouse_events = 1;
   ep->repeat_events = 0;
   ep->ignore_flags = EVAS_EVENT_FLAG_NONE;
   ep->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
   ep->precise_is_inside = 0;
   ep->use_alternate_font_metrics = 0;
   ep->clip_to_id = -1;
   ep->dragable.confine_id = -1;
   ep->dragable.events_id = -1;

   ep->default_desc = NULL;
   ep->other_desc = NULL;

   /* Init Edje_Real_Part */
   rp->edje = ed;
   _edje_ref(rp->edje);
   rp->part = ep;

   if (ep->type == EDJE_PART_TYPE_RECTANGLE)
     rp->object = evas_object_rectangle_add(ed->evas);
   else if (ep->type == EDJE_PART_TYPE_IMAGE)
     rp->object = evas_object_image_add(ed->evas);
   else if (ep->type == EDJE_PART_TYPE_TEXT)
     {
	_edje_text_part_on_add(ed, rp);
	rp->object = evas_object_text_add(ed->evas);
	evas_object_text_font_source_set(rp->object, ed->path);
     }
   else if (ep->type == EDJE_PART_TYPE_SWALLOW ||
	    ep->type == EDJE_PART_TYPE_GROUP)
     {
	rp->object = evas_object_rectangle_add(ed->evas);
	evas_object_color_set(rp->object, 0, 0, 0, 0);
	evas_object_pass_events_set(rp->object, 1);
	evas_object_pointer_mode_set(rp->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
     }
   else if (ep->type == EDJE_PART_TYPE_TEXTBLOCK)
     rp->object = evas_object_textblock_add(ed->evas);
   else if (ep->type == EDJE_PART_TYPE_GRADIENT)
     rp->object = evas_object_gradient_add(ed->evas);
   else
     printf("EDJE ERROR: wrong part type %i!\n", ep->type);
   if (rp->object)
     {
	evas_object_smart_member_add(rp->object, ed->obj);
	evas_object_layer_set(rp->object, evas_object_layer_get(ed->obj));
	if (ep->type != EDJE_PART_TYPE_SWALLOW && ep->type != EDJE_PART_TYPE_GROUP)
	  {
	     if (ep->mouse_events)
	       {
		  _edje_callbacks_add(rp->object, ed, rp);
		  if (ep->repeat_events)
		    evas_object_repeat_events_set(rp->object, 1);

		  if (ep->pointer_mode != EVAS_OBJECT_POINTER_MODE_AUTOGRAB)
		    evas_object_pointer_mode_set(rp->object, ep->pointer_mode);
	       }
	     else
	       {
		  evas_object_pass_events_set(rp->object, 1);
		  evas_object_pointer_mode_set(rp->object,
					       EVAS_OBJECT_POINTER_MODE_NOGRAB);
	       }
	     if (ep->precise_is_inside)
	       evas_object_precise_is_inside_set(rp->object, 1);
	  }
	evas_object_clip_set(rp->object, ed->clipper);
     }
   rp->drag.step.x = ep->dragable.step_x;
   rp->drag.step.y = ep->dragable.step_y;
   rp->gradient_id = -1;


   /* Update table_parts */
   ed->table_parts_size++;
   ed->table_parts = realloc(ed->table_parts,
			     sizeof(Edje_Real_Part *) * ed->table_parts_size);

   ed->table_parts[ep->id % ed->table_parts_size] = rp;

   /* Create default description */
   edje_edit_state_add(obj, name, "default");

   rp->param1.description = ep->default_desc;
   rp->chosen_description = rp->param1.description;

   edje_object_calc_force(obj);

   return TRUE;
}

EAPI unsigned char
edje_edit_part_del(Evas_Object *obj, const char* part)
{
   Edje_Part *ep;
   Edje_Part_Collection *pc;
   int id;

   GET_RP_OR_RETURN(0);

   printf("REMOVE PART: %s\n", part);

   ep = rp->part;
   id = ep->id;

   if (ed->table_parts_size <= 1) return FALSE; //don't remove the last part

   /* Unlik Edje_Real_Parts that link to the removed one */
   int i;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *real;

	if (i == id) continue; //don't check the deleted id
	real = ed->table_parts[i % ed->table_parts_size];

	if (real->text.source == rp) real->text.source = NULL;
	if (real->text.text_source == rp) real->text.text_source = NULL;

	if (real->param1.rel1_to_x == rp) real->param1.rel1_to_x = NULL;
	if (real->param1.rel1_to_y == rp) real->param1.rel1_to_y = NULL;
	if (real->param1.rel2_to_x == rp) real->param1.rel2_to_x = NULL;
	if (real->param1.rel2_to_y == rp) real->param1.rel2_to_y = NULL;

	if (real->param2.rel1_to_x == rp) real->param2.rel1_to_x = NULL;
	if (real->param2.rel1_to_y == rp) real->param2.rel1_to_y = NULL;
	if (real->param2.rel2_to_x == rp) real->param2.rel2_to_x = NULL;
	if (real->param2.rel2_to_y == rp) real->param2.rel2_to_y = NULL;

	if (real->custom.rel1_to_x == rp) real->custom.rel1_to_x = NULL;
	if (real->custom.rel1_to_y == rp) real->custom.rel1_to_y = NULL;
	if (real->custom.rel2_to_x == rp) real->custom.rel2_to_x = NULL;
	if (real->custom.rel2_to_y == rp) real->custom.rel2_to_y = NULL;

	if (real->clip_to == rp)
	  {
	     evas_object_clip_set(real->object, ed->clipper);
	     real->clip_to = NULL;
	  }
	//TODO confine ??
     }

   /* Unlink all the parts and descriptions that refer to id */
   _edje_part_id_set(ed, rp, -1);

   /* Remove part from parts list */
   pc = ed->collection;
   pc->parts = evas_list_remove(pc->parts, ep);
   _edje_fix_parts_id(ed);

   /* Free Edje_Part and all descriptions */
   _edje_if_string_free(ed, ep->name);
   if (ep->default_desc)
     {
	_edje_collection_free_part_description_free(ep->default_desc, 0);
	ep->default_desc = NULL;
     }
   while (ep->other_desc)
     {
	Edje_Part_Description *desc;

	desc = ep->other_desc->data;
	ep->other_desc = evas_list_remove(ep->other_desc, desc);
	_edje_collection_free_part_description_free(desc, 0);
     }
   free(ep);

   /* Free Edje_Real_Part */
   _edje_real_part_free(rp);


   edje_object_calc_force(obj);
   return TRUE;
}

EAPI unsigned char
edje_edit_part_exist(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   return 1;
}

EAPI unsigned char
edje_edit_part_restack_below(Evas_Object *obj, const char* part)
{
   Edje_Part_Collection *group;
   Edje_Real_Part *prev;

   GET_RP_OR_RETURN(0);

   printf("RESTACK PART: %s BELOW\n", part);

   if (rp->part->id < 1) return 0;
   group = ed->collection;

   /* update parts list */
   prev = ed->table_parts[(rp->part->id - 1) % ed->table_parts_size];
   group->parts = evas_list_remove(group->parts, rp->part);
   group->parts = evas_list_prepend_relative(group->parts, rp->part, prev->part);

   _edje_parts_id_switch(ed, rp, prev);

   evas_object_stack_below(rp->object, prev->object);

   return 1;
}

EAPI unsigned char
edje_edit_part_restack_above(Evas_Object *obj, const char* part)
{
   Edje_Part_Collection *group;
   Edje_Real_Part *next;

   GET_RP_OR_RETURN(0);

   printf("RESTACK PART: %s ABOVE\n", part);

   if (rp->part->id >= ed->table_parts_size - 1) return 0;

   group = ed->collection;

   /* update parts list */
   next = ed->table_parts[(rp->part->id + 1) % ed->table_parts_size];
   group->parts = evas_list_remove(group->parts, rp->part);
   group->parts = evas_list_append_relative(group->parts, rp->part, next->part);

   /* update ids */
   _edje_parts_id_switch(ed, rp, next);

   evas_object_stack_above(rp->object, next->object);

   return 1;
}

EAPI unsigned char
edje_edit_part_type_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->type;
}

EAPI const char *
edje_edit_part_selected_state_get(Evas_Object *obj, const char *part)
{
   char name[PATH_MAX];

   GET_RP_OR_RETURN(NULL);

   if (!rp->chosen_description)
     return "default 0.00";

   snprintf(name, PATH_MAX, "%s %.2f",
            rp->chosen_description->state.name,
            rp->chosen_description->state.value);

   return evas_stringshare_add(name);
}

EAPI unsigned char
edje_edit_part_selected_state_set(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Part_Description *pd;

   GET_RP_OR_RETURN(0);

   pd = _edje_part_description_find_byname(ed, part, state);
   if (!pd) return 0;

   printf("EDJE: Set state: %s\n", pd->state.name);
   _edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, NULL, 0); //WHAT IS NULL , 0

   edje_object_calc_force(obj);
   return 1;
}

EAPI const char *
edje_edit_part_clip_to_get(Evas_Object *obj, const char *part)
{
   Edje_Real_Part *clip = NULL;

   GET_RP_OR_RETURN(NULL);

   printf("Get clip_to for part: %s [to_id: %d]\n", part, rp->part->clip_to_id);
   if (rp->part->clip_to_id < 0) return NULL;

   clip = ed->table_parts[rp->part->clip_to_id % ed->table_parts_size];
   if (!clip || !clip->part || !clip->part->name) return NULL;

   return evas_stringshare_add(clip->part->name);
}

EAPI unsigned char
edje_edit_part_clip_to_set(Evas_Object *obj, const char *part, const char *clip_to)
{
   Edje_Real_Part *clip;

   GET_RP_OR_RETURN(0);

   /* unset clipping */
   if (!clip_to)
     {
	printf("UnSet clip_to for part: %s\n", part);

	if (rp->clip_to->object)
	  {
	     evas_object_pointer_mode_set(rp->clip_to->object,
					  EVAS_OBJECT_POINTER_MODE_AUTOGRAB);
	     evas_object_clip_unset(rp->object);
	  }

	evas_object_clip_set(rp->object, ed->clipper);


	rp->part->clip_to_id = -1;
	rp->clip_to = NULL;

	edje_object_calc_force(obj);

	return 1;
     }

   /* set clipping */
   printf("Set clip_to for part: %s [to: %s]\n", part, clip_to);
   clip = _edje_real_part_get(ed, clip_to);
   if (!clip || !clip->part) return 0;
   rp->part->clip_to_id = clip->part->id;
   rp->clip_to = clip;

   evas_object_pass_events_set(rp->clip_to->object, 1);
   evas_object_pointer_mode_set(rp->clip_to->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
   evas_object_clip_set(rp->object, rp->clip_to->object);

   edje_object_calc_force(obj);

   return 1;
}

EAPI unsigned char
edje_edit_part_mouse_events_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get mouse_events for part: %s [%d]\n", part, rp->part->mouse_events);
   return rp->part->mouse_events;
}

EAPI void
edje_edit_part_mouse_events_set(Evas_Object *obj, const char *part, unsigned char mouse_events)
{
   GET_RP_OR_RETURN();

   if (!rp->object) return;

   printf("Set mouse_events for part: %s [%d]\n", part, mouse_events);

   rp->part->mouse_events = mouse_events ? 1 : 0;

   if (mouse_events)
     {
	evas_object_pass_events_set(rp->object, 0);
	_edje_callbacks_add(rp->object, ed, rp);
     }
   else
     {
	evas_object_pass_events_set(rp->object, 1);
	_edje_callbacks_del(rp->object);
     }
}

EAPI unsigned char
edje_edit_part_repeat_events_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   //printf("Get repeat_events for part: %s [%d]\n", part, rp->part->repeat_events);
   return rp->part->repeat_events;
}

EAPI void
edje_edit_part_repeat_events_set(Evas_Object *obj, const char *part, unsigned char repeat_events)
{
   GET_RP_OR_RETURN();

   if (!rp->object) return;

   printf("Set repeat_events for part: %s [%d]\n", part, repeat_events);

   rp->part->repeat_events = repeat_events ? 1 : 0;

   if (repeat_events)
     evas_object_repeat_events_set(rp->object, 1);
   else
     evas_object_repeat_events_set(rp->object, 0);
}

EAPI Evas_Event_Flags
edje_edit_part_ignore_flags_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->ignore_flags;
}

EAPI void
edje_edit_part_ignore_flags_set(Evas_Object *obj, const char *part, Evas_Event_Flags ignore_flags)
{
   GET_RP_OR_RETURN();

   if (!rp->object) return;
   printf("Set ignore_flags for part: %s [%#x]\n", part, ignore_flags);

   rp->part->ignore_flags = ignore_flags;
}

EAPI const char *
edje_edit_part_source_get(Evas_Object *obj, const char *part)
{
   //Edje_Real_Part *clip = NULL;

   GET_RP_OR_RETURN(NULL);

   printf("Get source for part: %s\n", part);
   if (!rp->part->source) return NULL;

   return evas_stringshare_add(rp->part->source);
}

EAPI unsigned char
edje_edit_part_source_set(Evas_Object *obj, const char *part, const char *source)
{
   GET_RP_OR_RETURN(0);

   printf("Set source for part: %s [source: %s]\n", part, source);

   _edje_if_string_free(ed, rp->part->source);

   if (source)
     rp->part->source = evas_stringshare_add(source);

   return 1;
}

EAPI int
edje_edit_part_drag_x_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get dragX for part: %s\n", part);
   return rp->part->dragable.x;
}

EAPI void
edje_edit_part_drag_x_set(Evas_Object *obj, const char *part, int drag)
{
   GET_RP_OR_RETURN();
   printf("Set dragX for part: %s\n", part);
   rp->part->dragable.x = drag;
}

EAPI int
edje_edit_part_drag_y_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get dragY for part: %s\n", part);
   return rp->part->dragable.y;
}

EAPI void
edje_edit_part_drag_y_set(Evas_Object *obj, const char *part, int drag)
{
   GET_RP_OR_RETURN();
   printf("Set dragY for part: %s\n", part);
   rp->part->dragable.y = drag;
}

EAPI int
edje_edit_part_drag_step_x_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get dragX_STEP for part: %s\n", part);
   return rp->part->dragable.step_x;
}

EAPI void
edje_edit_part_drag_step_x_set(Evas_Object *obj, const char *part, int step)
{
   GET_RP_OR_RETURN();
   printf("Set dragX_STEP for part: %s\n", part);
   rp->part->dragable.step_x = step;
}

EAPI int
edje_edit_part_drag_step_y_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get dragY_STEP for part: %s\n", part);
   return rp->part->dragable.step_y;
}

EAPI void
edje_edit_part_drag_step_y_set(Evas_Object *obj, const char *part, int step)
{
   GET_RP_OR_RETURN();
   printf("Set dragY_STEP for part: %s\n", part);
   rp->part->dragable.step_y = step;
}

EAPI int
edje_edit_part_drag_count_x_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get dragX_COUNT for part: %s\n", part);
   return rp->part->dragable.count_x;
}

EAPI void
edje_edit_part_drag_count_x_set(Evas_Object *obj, const char *part, int count)
{
   GET_RP_OR_RETURN();
   printf("Set dragX_COUNT for part: %s\n", part);
   rp->part->dragable.count_x = count;
}

EAPI int
edje_edit_part_drag_count_y_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   //printf("Get dragY_COUNT for part: %s\n", part);
   return rp->part->dragable.count_y;
}

EAPI void
edje_edit_part_drag_count_y_set(Evas_Object *obj, const char *part, int count)
{
   GET_RP_OR_RETURN();
   printf("Set dragY_COUNT for part: %s\n", part);
   rp->part->dragable.count_y = count;
}

EAPI const char*
edje_edit_part_drag_confine_get(Evas_Object *obj, const char *part)
{
   Edje_Real_Part *confine;
   //printf("******Get drag confine\n");
   GET_RP_OR_RETURN(NULL);

   if (rp->part->dragable.confine_id < 0)
      return NULL;

   confine = ed->table_parts[rp->part->dragable.confine_id];
   return evas_stringshare_add(confine->part->name);
}

EAPI void
edje_edit_part_drag_confine_set(Evas_Object *obj, const char *part, const char *confine)
{
   Edje_Real_Part *confine_part;
   printf("******Set drag confine to: %s\n", confine);
   GET_RP_OR_RETURN();

   if (!confine)
     {
      rp->part->dragable.confine_id = -1;
      return;
     }

   confine_part = _edje_real_part_get(ed, confine);
   rp->part->dragable.confine_id = confine_part->part->id;
}

EAPI const char*
edje_edit_part_drag_event_get(Evas_Object *obj, const char *part)
{
   Edje_Real_Part *events;
   //printf("******Get drag event part\n");
   GET_RP_OR_RETURN(NULL);

   if (rp->part->dragable.events_id < 0)
      return NULL;

   events = ed->table_parts[rp->part->dragable.events_id];
   return evas_stringshare_add(events->part->name);
}

EAPI void
edje_edit_part_drag_event_set(Evas_Object *obj, const char *part, const char *event)
{
   Edje_Real_Part *event_part;
   printf("******Set drag event to: %s\n", event);
   GET_RP_OR_RETURN();

   if (!event)
     {
      rp->part->dragable.events_id = -1;
      return;
     }

   event_part = _edje_real_part_get(ed, event);
   rp->part->dragable.events_id = event_part->part->id;
}
/*********************/
/*  PART STATES API  */
/*********************/
EAPI Evas_List *
edje_edit_part_states_list_get(Evas_Object *obj, const char *part)
{
   char state_name[PATH_MAX];
   Evas_List *states;
   Evas_List *l;
   Edje_Part_Description *state;

   GET_RP_OR_RETURN(NULL);

   //Is there a better place to put this? maybe edje_edit_init() ?
   setlocale(LC_NUMERIC, "C");

   states = NULL;

   //append default state
   state = rp->part->default_desc;
   snprintf(state_name, PATH_MAX,
            "%s %.2f", state->state.name, state->state.value);
   states = evas_list_append(states, evas_stringshare_add(state_name));
   //printf("NEW STATE def: %s\n", state->state.name);

   //append other states
   for (l = rp->part->other_desc; l; l = l->next)
     {
	state = l->data;
	snprintf(state_name, sizeof(state_name),
	         "%s %.2f", state->state.name, state->state.value);
	states = evas_list_append(states, evas_stringshare_add(state_name));
	//printf("NEW STATE: %s\n", state_name);
     }
   return states;
}

EAPI int
edje_edit_state_name_set(Evas_Object *obj, const char *part, const char *state, const char *new_name)//state and new_name include the value in the string (ex. "default 0.00")
{
   char *delim;
   double value;
   int part_id;
   int i;

   GET_PD_OR_RETURN(0);
   printf("Set name of state: %s in part: %s [new name: %s]\n",
          part, state, new_name);

   if (!new_name) return 0;

   /* split name from value */
   delim = strrchr(new_name, (int)' ');
   if (!delim) return 0;
   if (sscanf(delim, "%lf", &value) != 1) return 0;
   delim[0] = '\0';

   /* update programs */
   /* update the 'state' field in all programs. update only if program has
      a single target */
   part_id = _edje_part_id_find(ed, part);
   for (i = 0; i < ed->table_programs_size; i++)
     {
	Edje_Program *epr = ed->table_programs[i];

	if (evas_list_count(epr->targets) == 1)
	  {
	     Edje_Program_Target *t = epr->targets->data;

	     if (t->id == part_id &&
		 !strcmp(epr->state, pd->state.name) &&
		 pd->state.value == epr->value)
	       {
		  _edje_if_string_free(ed, epr->state);
		  epr->state = evas_stringshare_add(new_name);
		  epr->value = value;
	       }
	  }
     }

   /* set name */
   _edje_if_string_free(ed, pd->state.name);
   pd->state.name = (char *)evas_stringshare_add(new_name);
   /* set value */
   pd->state.value = value;

   delim[0] = ' ';
   printf("## SET OK %s %.2f\n", pd->state.name, pd->state.value);

   return 1;
}

EAPI void
edje_edit_state_del(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Part_Description *pd;

   GET_RP_OR_RETURN();

   printf("REMOVE STATE: %s IN PART: %s\n", state, part);

   pd = _edje_part_description_find_byname(ed, part, state);
   if (!pd) return;

   rp->part->other_desc = evas_list_remove(rp->part->other_desc, pd);

   _edje_collection_free_part_description_free(pd, 0);
}

EAPI void
edje_edit_state_add(Evas_Object *obj, const char *part, const char *name)
{
   Edje_Part_Description *pd;

   GET_RP_OR_RETURN();

   printf("ADD STATE: %s TO PART: %s\n", name , part);

   pd = mem_alloc(sizeof(Edje_Part_Description));

   if (!rp->part->default_desc)
     rp->part->default_desc = pd;
   else
     rp->part->other_desc = evas_list_append(rp->part->other_desc, pd);

   pd->state.name = evas_stringshare_add(name);
   pd->state.value = 0.0;
   pd->visible = 1;
   pd->align.x = 0.5;
   pd->align.y = 0.5;
   pd->min.w = 0;
   pd->min.h = 0;
   pd->fixed.w = 0;
   pd->fixed.h = 0;
   pd->max.w = -1;
   pd->max.h = -1;
   pd->rel1.relative_x = 0.0;
   pd->rel1.relative_y = 0.0;
   pd->rel1.offset_x = 0;
   pd->rel1.offset_y = 0;
   pd->rel1.id_x = -1;
   pd->rel1.id_y = -1;
   pd->rel2.relative_x = 1.0;
   pd->rel2.relative_y = 1.0;
   pd->rel2.offset_x = -1;
   pd->rel2.offset_y = -1;
   pd->rel2.id_x = -1;
   pd->rel2.id_y = -1;
   pd->image.id = -1;
   pd->fill.smooth = 1;
   pd->fill.pos_rel_x = 0.0;
   pd->fill.pos_abs_x = 0;
   pd->fill.rel_x = 1.0;
   pd->fill.abs_x = 0;
   pd->fill.pos_rel_y = 0.0;
   pd->fill.pos_abs_y = 0;
   pd->fill.rel_y = 1.0;
   pd->fill.abs_y = 0;
   pd->fill.angle = 0;
   pd->fill.spread = 0;
   pd->fill.type = EDJE_FILL_TYPE_SCALE;
   pd->color_class = NULL;
   pd->color.r = 255;
   pd->color.g = 255;
   pd->color.b = 255;
   pd->color.a = 255;
   pd->color2.r = 0;
   pd->color2.g = 0;
   pd->color2.b = 0;
   pd->color2.a = 255;
   pd->color3.r = 0;
   pd->color3.g = 0;
   pd->color3.b = 0;
   pd->color3.a = 128;
   pd->text.align.x = 0.5;
   pd->text.align.y = 0.5;
   pd->text.id_source = -1;
   pd->text.id_text_source = -1;
   pd->gradient.rel1.relative_x = 0;
   pd->gradient.rel1.relative_y = 0;
   pd->gradient.rel1.offset_x = 0;
   pd->gradient.rel1.offset_y = 0;
   pd->gradient.rel2.relative_x = 1;
   pd->gradient.rel2.relative_y = 1;
   pd->gradient.rel2.offset_x = -1;
   pd->gradient.rel2.offset_y = -1;
   pd->gradient.use_rel = 1;
}

EAPI unsigned char
edje_edit_state_exist(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   return 1;
}

//relative
EAPI double
edje_edit_state_rel1_relative_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel1 rel of part: %s state: %s [%f]\n", part, state, pd->rel1.relative_x);
   return pd->rel1.relative_x;
}

EAPI double
edje_edit_state_rel1_relative_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel1 rel of part: %s state: %s\n", part, state);
   return pd->rel1.relative_y;
}

EAPI double
edje_edit_state_rel2_relative_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel2 rel of part: %s state: %s\n", part, state);
   return pd->rel2.relative_x;
}

EAPI double
edje_edit_state_rel2_relative_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel2 rel of part: %s state: %s\n", part, state);
   return pd->rel2.relative_y;
}

EAPI void
edje_edit_state_rel1_relative_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   //printf("Set rel1x of part: %s state: %s to: %f\n", part, state, x);
   //TODO check boudaries
   pd->rel1.relative_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_rel1_relative_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   //printf("Set rel1y of part: %s state: %s to: %f\n", part, state, y);
   //TODO check boudaries
   pd->rel1.relative_y = y;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_rel2_relative_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   //printf("Set rel2x of part: %s state: %s to: %f\n", part, state, x);
   //TODO check boudaries
   pd->rel2.relative_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_rel2_relative_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   //printf("Set rel2y of part: %s state: %s to: %f\n", part, state, y);
   pd = _edje_part_description_find_byname(ed, part, state);
   //TODO check boudaries
   pd->rel2.relative_y = y;
   edje_object_calc_force(obj);
}

//offset
EAPI int
edje_edit_state_rel1_offset_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel1 offset of part: %s state: %s\n", part, state);
   return pd->rel1.offset_x;
}

EAPI int
edje_edit_state_rel1_offset_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel1 offset of part: %s state: %s\n", part, state);
   return pd->rel1.offset_y;
}

EAPI int
edje_edit_state_rel2_offset_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel2 offset of part: %s state: %s\n", part, state);
   return pd->rel2.offset_x;
}

EAPI int
edje_edit_state_rel2_offset_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get rel2 offset of part: %s state: %s\n", part, state);
   return pd->rel2.offset_y;
}

EAPI void
edje_edit_state_rel1_offset_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   printf("Set rel1x offset of part: %s state: %s to: %f\n", part, state, x);
   //TODO check boudaries
   pd->rel1.offset_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_rel1_offset_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   printf("Set rel1y offset of part: %s state: %s to: %f\n", part, state, y);
   //TODO check boudaries
   pd->rel1.offset_y = y;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_rel2_offset_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   printf("Set rel2x offset of part: %s state: %s to: %f\n", part, state, x);
   //TODO check boudaries
   pd->rel2.offset_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_rel2_offset_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   printf("Set rel2y offset of part: %s state: %s to: %f\n", part, state, y);
   //TODO check boudaries
   pd->rel2.offset_y = y;
   edje_object_calc_force(obj);
}

//relative to
EAPI const char *
edje_edit_state_rel1_to_x_get(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Real_Part *rel;

   GET_PD_OR_RETURN(NULL);

   //printf("Get rel1x TO of part: %s state: %s\n", part, state);

   if (pd->rel1.id_x == -1) return NULL;

   rel = ed->table_parts[pd->rel1.id_x % ed->table_parts_size];

   if (rel->part->name)
     return evas_stringshare_add(rel->part->name);
   else
     return NULL;
}

EAPI const char *
edje_edit_state_rel1_to_y_get(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Real_Part *rel;

   GET_PD_OR_RETURN(NULL);

   //printf("Get rel1y TO of part: %s state: %s\n", part, state);

   if (pd->rel1.id_y == -1) return NULL;

   rel = ed->table_parts[pd->rel1.id_y % ed->table_parts_size];

   if (rel->part->name)
     return evas_stringshare_add(rel->part->name);
   else
     return NULL;
}

EAPI const char *
edje_edit_state_rel2_to_x_get(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Real_Part *rel;

   GET_PD_OR_RETURN(NULL);

   //printf("Get rel2x TO of part: %s state: %s\n", part, state);

   if (pd->rel2.id_x == -1) return NULL;

   rel = ed->table_parts[pd->rel2.id_x % ed->table_parts_size];

   if (rel->part->name)
     return evas_stringshare_add(rel->part->name);
   else
     return NULL;
}

EAPI const char *
edje_edit_state_rel2_to_y_get(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Real_Part *rel;

   GET_PD_OR_RETURN(NULL);

   //printf("Get rel2y TO of part: %s state: %s\n", part, state);

   if (pd->rel2.id_y == -1) return NULL;

   rel = ed->table_parts[pd->rel2.id_y % ed->table_parts_size];

   if (rel->part->name)
     return evas_stringshare_add(rel->part->name);
   else
     return NULL;
}

EAPI void
//note after this call edje_edit_part_selected_state_set() to update !! need to fix this
edje_edit_state_rel1_to_x_set(Evas_Object *obj, const char *part, const char *state, const char *rel_to)
{
   Edje_Real_Part *relp;

   GET_PD_OR_RETURN();

   printf("Set rel1 to x on state: %s (to part: )\n", state);

   if (rel_to)
     {
	relp = _edje_real_part_get(ed, rel_to);
	if (!relp) return;
	pd->rel1.id_x = relp->part->id;
     }
   else
     pd->rel1.id_x = -1;

   //_edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, "state", 0.1); //Why segfault??
   // edje_object_calc_force(obj);//don't work for redraw
}

EAPI void
//note after this call edje_edit_part_selected_state_set() to update !! need to fix this
edje_edit_state_rel1_to_y_set(Evas_Object *obj, const char *part, const char *state, const char *rel_to)
{
   Edje_Real_Part *relp;

   GET_PD_OR_RETURN();

   //printf("Set rel1 to y on state: %s (to part: %s)\n", state, rel_to);

   if (rel_to)
     {
	relp = _edje_real_part_get(ed, rel_to);
	if (!relp) return;
	pd->rel1.id_y = relp->part->id;
     }
   else
     pd->rel1.id_y = -1;

   //_edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, "state", 0.1); //Why segfault??
   // edje_object_calc_force(obj);//don't work for redraw
}

EAPI void
//note after this call edje_edit_part_selected_state_set() to update !! need to fix this
edje_edit_state_rel2_to_x_set(Evas_Object *obj, const char *part, const char *state, const char *rel_to)
{
   Edje_Real_Part *relp;

   GET_PD_OR_RETURN();

   printf("Set rel2 to x on state: %s (to part: )\n", state);

   if (rel_to)
     {
	relp = _edje_real_part_get(ed, rel_to);
	if (!relp) return;
	pd->rel2.id_x = relp->part->id;
     }
   else
     pd->rel2.id_x = -1;

   //_edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, "state", 0.1); //Why segfault??
   // edje_object_calc_force(obj);//don't work for redraw
}

EAPI void
//note after this call edje_edit_part_selected_state_set() to update !! need to fix this
edje_edit_state_rel2_to_y_set(Evas_Object *obj, const char *part, const char *state, const char *rel_to)
{
   Edje_Real_Part *relp;

   GET_PD_OR_RETURN();

   //printf("Set rel2 to y on state: %s (to part: %s)\n", state, rel_to);

   if (rel_to)
     {
	relp = _edje_real_part_get(ed, rel_to);
	if (!relp) return;
	pd->rel2.id_y = relp->part->id;
     }
   else
      pd->rel2.id_y = -1;

   //_edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, "state", 0.1); //Why segfault??
   // edje_object_calc_force(obj);//don't work for redraw
}

//colors
EAPI void
edje_edit_state_color_get(Evas_Object *obj, const char *part, const char *state, int *r, int *g, int *b, int *a)
{
   GET_PD_OR_RETURN();

   //printf("GET COLOR of state '%s'\n", state);

   if (r) *r = pd->color.r;
   if (g) *g = pd->color.g;
   if (b) *b = pd->color.b;
   if (a) *a = pd->color.a;
}

EAPI void
edje_edit_state_color2_get(Evas_Object *obj, const char *part, const char *state, int *r, int *g, int *b, int *a)
{
   GET_PD_OR_RETURN();

   //printf("GET COLOR2 of state '%s'\n", state);

   if (r) *r = pd->color2.r;
   if (g) *g = pd->color2.g;
   if (b) *b = pd->color2.b;
   if (a) *a = pd->color2.a;
}

EAPI void
edje_edit_state_color3_get(Evas_Object *obj, const char *part, const char *state, int *r, int *g, int *b, int *a)
{
   GET_PD_OR_RETURN();

   //printf("GET COLOR3 of state '%s'\n", state);

   if (r) *r = pd->color3.r;
   if (g) *g = pd->color3.g;
   if (b) *b = pd->color3.b;
   if (a) *a = pd->color3.a;
}

EAPI void
edje_edit_state_color_set(Evas_Object *obj, const char *part, const char *state, int r, int g, int b, int a)
{
   GET_PD_OR_RETURN();

   //printf("SET COLOR of state '%s'\n", state);

   if (r > -1 && r < 256) pd->color.r = r;
   if (g > -1 && g < 256) pd->color.g = g;
   if (b > -1 && b < 256) pd->color.b = b;
   if (a > -1 && a < 256) pd->color.a = a;

   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_color2_set(Evas_Object *obj, const char *part, const char *state, int r, int g, int b, int a)
{
   GET_PD_OR_RETURN();

   //printf("SET COLOR2 of state '%s'\n", state);

   if (r > -1 && r < 256) pd->color2.r = r;
   if (g > -1 && g < 256) pd->color2.g = g;
   if (b > -1 && b < 256) pd->color2.b = b;
   if (a > -1 && a < 256) pd->color2.a = a;

   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_color3_set(Evas_Object *obj, const char *part, const char *state, int r, int g, int b, int a)
{
   GET_PD_OR_RETURN();

   //printf("SET COLOR3 of state '%s'\n", state);

   if (r > -1 && r < 256) pd->color3.r = r;
   if (g > -1 && g < 256) pd->color3.g = g;
   if (b > -1 && b < 256) pd->color3.b = b;
   if (a > -1 && a < 256) pd->color3.a = a;

   edje_object_calc_force(obj);
}

//align
EAPI double
edje_edit_state_align_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   //printf("GET ALIGN_X of state '%s' [%f]\n", state, pd->align.x);

   return pd->align.x;
}

EAPI double
edje_edit_state_align_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   //printf("GET ALIGN_Y of state '%s' [%f]\n", state, pd->align.y);

   return pd->align.y;
}

EAPI void
edje_edit_state_align_x_set(Evas_Object *obj, const char *part, const char *state, double align)
{
   GET_PD_OR_RETURN();
   printf("SET ALIGN_X of state '%s' [to: %f]\n", state, align);
   pd->align.x = align;
}

EAPI void
edje_edit_state_align_y_set(Evas_Object *obj, const char *part, const char *state, double align)
{
   GET_PD_OR_RETURN();

   printf("SET ALIGN_Y of state '%s' [to: %f]\n", state, align);
   pd->align.y = align;
}

//min & max
EAPI int
edje_edit_state_min_w_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET MIN_W of state '%s' [%d]\n", state, pd->min.w);
   return pd->min.w;
}

EAPI void
edje_edit_state_min_w_set(Evas_Object *obj, const char *part, const char *state, int min_w)
{
   GET_PD_OR_RETURN();

   printf("SET MIN_W of state '%s' [to: %d]\n", state, min_w);
   pd->min.w = min_w;
}

EAPI int
edje_edit_state_min_h_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET MIN_H of state '%s' [%d]\n", state, pd->min.h);
   return pd->min.h;
}

EAPI void
edje_edit_state_min_h_set(Evas_Object *obj, const char *part, const char *state, int min_h)
{
   GET_PD_OR_RETURN();

   printf("SET MIN_H of state '%s' [to: %d]\n", state, min_h);
   pd->min.h = min_h;
}

EAPI int
edje_edit_state_max_w_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET MAX_W of state '%s' [%d]\n", state, pd->max.w);
   return pd->max.w;
}

EAPI void
edje_edit_state_max_w_set(Evas_Object *obj, const char *part, const char *state, int max_w)
{
   GET_PD_OR_RETURN();

   printf("SET MAX_W of state '%s' [to: %d]\n", state, max_w);
   pd->max.w = max_w;
}

EAPI int
edje_edit_state_max_h_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET MAX_H of state '%s' [%d]\n", state, pd->max.h);
   return pd->max.h;
}

EAPI void
edje_edit_state_max_h_set(Evas_Object *obj, const char *part, const char *state, int max_h)
{
   GET_PD_OR_RETURN();

   printf("SET MAX_H of state '%s' [to: %d]\n", state, max_h);
   pd->max.h = max_h;
}

//aspect
EAPI double
edje_edit_state_aspect_min_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET ASPECT_MIN of state '%s' [%f]\n", state, pd->aspect.min);
   return pd->aspect.min;
}

EAPI double
edje_edit_state_aspect_max_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET ASPECT_MAX of state '%s' [%f]\n", state, pd->aspect.max);
   return pd->aspect.max;
}

EAPI void
edje_edit_state_aspect_min_set(Evas_Object *obj, const char *part, const char *state, double aspect)
{
   GET_PD_OR_RETURN();

   printf("SET ASPECT_MIN of state '%s' [to: %f]\n", state, aspect);
   pd->aspect.min = aspect;
}

EAPI void
edje_edit_state_aspect_max_set(Evas_Object *obj, const char *part, const char *state, double aspect)
{
   GET_PD_OR_RETURN();

   printf("SET ASPECT_MAX of state '%s' [to: %f]\n", state, aspect);
   pd->aspect.max = aspect;
}

EAPI unsigned char
edje_edit_state_aspect_pref_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   printf("GET ASPECT_PREF of state '%s' [%d]\n", state, pd->aspect.prefer);
   return pd->aspect.prefer;
}

EAPI void
edje_edit_state_aspect_pref_set(Evas_Object *obj, const char *part, const char *state, unsigned char pref)
{
   GET_PD_OR_RETURN();

   printf("SET ASPECT_PREF of state '%s' [to: %d]\n", state, pref);
   pd->aspect.prefer = pref;
}

//fill
EAPI double
edje_edit_state_fill_origin_relative_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state fill origin of part: %s state: %s\n", part, state);
   return pd->fill.pos_rel_x;
}

EAPI double
edje_edit_state_fill_origin_relative_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state fill origin of part: %s state: %s\n", part, state);
   return pd->fill.pos_rel_y;
}

EAPI int
edje_edit_state_fill_origin_offset_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state fill origin offset of part: %s state: %s\n", part, state);
   return pd->fill.pos_abs_x;
}

EAPI int
edje_edit_state_fill_origin_offset_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state fill origin offset of part: %s state: %s\n", part, state);
   return pd->fill.pos_abs_y;
}


EAPI void
edje_edit_state_fill_origin_relative_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   printf("Set state fill origin of part: %s state: %s to: %f\n", part, state, x);
   pd->fill.pos_rel_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_fill_origin_relative_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   printf("Set state fill origin of part: %s state: %s to: %f\n", part, state, y);
   pd->fill.pos_rel_y = y;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_fill_origin_offset_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   printf("Set state fill origin offset x of part: %s state: %s to: %f\n", part, state, x);
   pd->fill.pos_abs_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_fill_origin_offset_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   printf("Set state fill origin offset y of part: %s state: %s to: %f\n", part, state, y);
   pd->fill.pos_abs_y = y;
   edje_object_calc_force(obj);
}

EAPI double
edje_edit_state_fill_size_relative_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0.0);
   //printf("Get state fill size of part: %s state: %s\n", part, state);
   return pd->fill.rel_x;
}

EAPI double
edje_edit_state_fill_size_relative_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0.0);
   //printf("Get state fill size of part: %s state: %s\n", part, state);
   return pd->fill.rel_y;
}

EAPI int
edje_edit_state_fill_size_offset_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state fill size offset of part: %s state: %s\n", part, state);
   return pd->fill.abs_x;
}

EAPI int
edje_edit_state_fill_size_offset_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state fill size offset of part: %s state: %s\n", part, state);
   return pd->fill.abs_y;
}

EAPI void
edje_edit_state_fill_size_relative_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   printf("Set state fill size of part: %s state: %s to: %f\n", part, state, x);
   pd->fill.rel_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_fill_size_relative_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   printf("Set state fill size of part: %s state: %s to: %f\n", part, state, y);
   pd->fill.rel_y = y;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_fill_size_offset_x_set(Evas_Object *obj, const char *part, const char *state, double x)
{
   GET_PD_OR_RETURN();
   printf("Set state fill size offset x of part: %s state: %s to: %f\n", part, state, x);
   pd->fill.abs_x = x;
   edje_object_calc_force(obj);
}

EAPI void
edje_edit_state_fill_size_offset_y_set(Evas_Object *obj, const char *part, const char *state, double y)
{
   GET_PD_OR_RETURN();
   printf("Set state fill size offset y of part: %s state: %s to: %f\n", part, state, y);
   pd->fill.abs_y = y;
   edje_object_calc_force(obj);
}

EAPI unsigned char
edje_edit_state_visible_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("Get state visible flag of part: %s state: %s\n", part, state);
   return pd->visible;
}

EAPI void
edje_edit_state_visible_set(Evas_Object *obj, const char *part, const char *state, unsigned char visible)
{
   GET_PD_OR_RETURN();
   printf("Set state visible flag of part: %s state: %s to: %d\n", part, state, visible);
   if (visible) pd->visible = 1;
   else         pd->visible = 0;
   edje_object_calc_force(obj);
}

EAPI const char*
edje_edit_state_color_class_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(NULL);
   printf("Get ColorClass of part: %s state: %s\n", part, state);
   return evas_stringshare_add(pd->color_class);
}

EAPI void
edje_edit_state_color_class_set(Evas_Object *obj, const char *part, const char *state, const char *color_class)
{
   GET_PD_OR_RETURN();
   printf("Set ColorClass of part: %s state: %s [to: %s]\n", part, state, color_class);
   _edje_if_string_free(ed, pd->color_class);
   pd->color_class = (char*)evas_stringshare_add(color_class);
}

/**************/
/*  TEXT API */
/**************/

EAPI const char *
edje_edit_state_text_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(NULL);

   //printf("GET TEXT of state: %s\n", state);

   if (pd->text.text)
     return evas_stringshare_add(pd->text.text);

   return NULL;
}

EAPI void
edje_edit_state_text_set(Evas_Object *obj, const char *part, const char *state, const char *text)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT of state: %s\n", state);

   if (!text) return;

   _edje_if_string_free(ed, pd->text.text);
   pd->text.text = (char *)evas_stringshare_add(text);

   edje_object_calc_force(obj);
}

EAPI int
edje_edit_state_text_size_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(-1);

   //printf("GET TEXT_SIZE of state: %s [%d]\n", state, pd->text.size);
   return pd->text.size;
}

EAPI void
edje_edit_state_text_size_set(Evas_Object *obj, const char *part, const char *state, int size)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT_SIZE of state: %s [%d]\n", state, size);

   if (size < 0) return;

   pd->text.size = size;

   edje_object_calc_force(obj);
}

EAPI double
edje_edit_state_text_align_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);

   //printf("GET TEXT_ALIGN_X of state: %s [%f]\n", state, pd->text.align.x);
   return pd->text.align.x;
}

EAPI void
edje_edit_state_text_align_x_set(Evas_Object *obj, const char *part, const char *state, double align)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT_ALIGN_X of state: %s [%f]\n", state, align);

   pd->text.align.x = align;
   edje_object_calc_force(obj);
}

EAPI double
edje_edit_state_text_align_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0.0);

   //printf("GET TEXT_ALIGN_Y of state: %s [%f]\n", state, pd->text.align.x);
   return pd->text.align.y;
}

EAPI void
edje_edit_state_text_align_y_set(Evas_Object *obj, const char *part, const char *state, double align)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT_ALIGN_Y of state: %s [%f]\n", state, align);

   pd->text.align.y = align;
   edje_object_calc_force(obj);
}

EAPI double
edje_edit_state_text_elipsis_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0.0);

   //printf("GET TEXT_ELIPSIS of state: %s [%f]\n", state, pd->text.elipsis);
   return pd->text.elipsis;
}

EAPI void
edje_edit_state_text_elipsis_set(Evas_Object *obj, const char *part, const char *state, double balance)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT_ELIPSIS of state: %s [%f]\n", state, balance);

   pd->text.elipsis = balance;
   edje_object_calc_force(obj);
}

EAPI unsigned char
edje_edit_state_text_fit_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN();
   //printf("GET TEXT_FIT_VERT of state: %s \n", state);
   return pd->text.fit_x;
}

EAPI void
edje_edit_state_text_fit_x_set(Evas_Object *obj, const char *part, const char *state, unsigned char fit)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT_FIT_VERT of state: %s\n", state);

   pd->text.fit_x = fit ? 1 : 0;
   edje_object_calc_force(obj);
}

EAPI unsigned char
edje_edit_state_text_fit_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN();
   //printf("GET TEXT_FIT_VERT of state: %s \n", state);
   return pd->text.fit_y;
}

EAPI void
edje_edit_state_text_fit_y_set(Evas_Object *obj, const char *part, const char *state, unsigned char fit)
{
   GET_PD_OR_RETURN();

   //printf("SET TEXT_FIT_VERT of state: %s\n", state);

   pd->text.fit_y = fit ? 1 : 0;
   edje_object_calc_force(obj);
}

EAPI Evas_List *
edje_edit_fonts_list_get(Evas_Object *obj)
{
   Edje_Font_Directory_Entry *f;
   Evas_List *fonts = NULL;
   Evas_List *l;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->font_dir) return NULL;

   printf("GET FONT LIST for %s\n", ed->file->path);

   for (l = ed->file->font_dir->entries; l; l = l->next)
     {
	f = l->data;
	fonts = evas_list_append(fonts, evas_stringshare_add(f->entry));
	printf("   Font: %s (%s) \n", f->entry, f->path);
     }

   return fonts;
}

EAPI unsigned char
edje_edit_font_add(Evas_Object *obj, const char* path)
{
   char buf[PATH_MAX];
   Font *fn;
   Edje_Font_Directory_Entry *fnt;
   Eet_File *eetf;
   struct stat st;
   char *name;
   FILE *f;
   void *fdata = NULL;
   int fsize = 0;

   GET_ED_OR_RETURN(0);

   printf("ADD FONT: %s\n", path);

   if (!path) return 0;
   if (stat(path, &st) || !S_ISREG(st.st_mode)) return 0;
   if (!ed->file) return 0;
   if (!ed->path) return 0;


   /* Create Font_Directory if not exist */
   if (!ed->file->font_dir)
     ed->file->font_dir = mem_alloc(sizeof(Edje_Font_Directory));

   /* Create Font */
   fn = mem_alloc(sizeof(Font));
   if (!fn) return 0;

   if ((name = strrchr(path, '/'))) name ++;
   else name = (char *)path;

   fn->file = mem_strdup(name);
   fn->name = mem_strdup(name);

   /*{
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
   */

   /* Read font data from file */
   f = fopen(path, "rb");
   if (f)
     {
	long pos;

	fseek(f, 0, SEEK_END);
	pos = ftell(f);
	rewind(f);
	fdata = malloc(pos);
	if (fdata)
	  {
	     if (fread(fdata, pos, 1, f) != 1)
	       {
		  fprintf(stderr, "Edje_Edit: Error. unable to read all of font file \"%s\"\n",
			  path);
		  return 0;
	       }
	     fsize = pos;
	  }
	fclose(f);
     }
   /* Write font to edje file */
   snprintf(buf, sizeof(buf), "fonts/%s", fn->name);

   if (fdata)
     {
	/* open the eet file */
	eetf = eet_open(ed->path, EET_FILE_MODE_READ_WRITE);
	if (!eetf)
	  {
	     fprintf(stderr,
		     "Edje_Edit: Error. unable to open \"%s\" for writing output\n",
		     ed->path);
	     return 0;
	  }

	if (eet_write(eetf, buf, fdata, fsize, 1) <= 0)
	  {
	     fprintf(stderr, "Edje_Edit: Error. unable to write font part \"%s\" as \"%s\" part entry\n",
		     path, buf);
	     eet_close(eetf);
	     free(fdata);
	     return 0;
	  }

	eet_close(eetf);
	free(fdata);
     }

   /* Create Edje_Font_Directory_Entry */
   if (ed->file->font_dir)
     {
	fnt = mem_alloc(sizeof(Edje_Font_Directory_Entry));
	fnt->entry = mem_strdup(fn->name);
	fnt->path = mem_strdup(buf);

	ed->file->font_dir->entries = evas_list_append(ed->file->font_dir->entries, fnt);
	ed->file->font_hash = evas_hash_direct_add(ed->file->font_hash, fnt->entry, fnt);
     }

   return 1;
}

EAPI const char *
edje_edit_state_font_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(NULL);

   printf("GET FONT of state: %s [%s]\n", state, pd->text.font);
   if (!pd->text.font) return NULL;
   return evas_stringshare_add(pd->text.font);
}

EAPI void
edje_edit_state_font_set(Evas_Object *obj, const char *part, const char *state, const char *font)
{
   GET_PD_OR_RETURN();

   printf("SET FONT of state: %s [%s]\n", state, font);

   _edje_if_string_free(ed, pd->text.font);
   pd->text.font = (char *)evas_stringshare_add(font);

   edje_object_calc_force(obj);
}

EAPI unsigned char
edje_edit_part_effect_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   printf("GET EFFECT of part: %s\n", part);
   return rp->part->effect;
}

EAPI void
edje_edit_part_effect_set(Evas_Object *obj, const char *part, unsigned char effect)
{
   GET_RP_OR_RETURN();

   printf("SET EFFECT of part: %s [%d]\n", part, effect);
   rp->part->effect = effect;

   edje_object_calc_force(obj);
}

/****************/
/*  IMAGES API  */
/****************/

EAPI Evas_List *
edje_edit_images_list_get(Evas_Object *obj)
{
   Edje_Image_Directory_Entry *i;
   Evas_List *images = NULL;
   Evas_List *l;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->image_dir) return NULL;

   printf("GET IMAGES LIST for %s\n", ed->file->path);

   for (l = ed->file->image_dir->entries; l; l = l->next)
     {
	i = l->data;
	images = evas_list_append(images, evas_stringshare_add(i->entry));
	//printf("   Image: %s (type: %d param: %d id: %d) \n",
	//       i->entry, i->source_type, i->source_param, i->id);
     }

   return images;
}

EAPI unsigned char
edje_edit_image_add(Evas_Object *obj, const char* path)
{
   Evas_List *l;
   Edje_Image_Directory_Entry *de;
   int free_id = 0;
   char *name;

   GET_ED_OR_RETURN(0);

   if (!path) return 0;
   if (!ed->file) return 0;
   if (!ed->path) return 0;

   /* Create Image_Directory if not exist */
   if (!ed->file->image_dir)
     ed->file->image_dir = mem_alloc(sizeof(Edje_Image_Directory));

   /* Loop trough image directory to find if image exist */
   printf("Add Image '%s' (total %d)\n", path,
          evas_list_count(ed->file->image_dir->entries));
   for (l = ed->file->image_dir->entries; l; l = l->next)
     {
	Edje_Image_Directory_Entry *i;

	i = l->data;
	if (!i) return 0;
	if (i->id >= free_id) free_id = i->id + 1; /*TODO search for free (hole) id*/
	printf("IMG: %s [%d]\n", i->entry, i->id);
     }
   printf("FREE ID: %d\n", free_id);

   /* Import image */
   if (!_edje_import_image_file(ed, path, free_id))
      return 0;

   /* Create Image Entry */
   de = mem_alloc(sizeof(Edje_Image_Directory_Entry));
   if ((name = strrchr(path, '/'))) name++;
   else name = (char *)path;
   de->entry = mem_strdup(name);
   de->id = free_id;
   de->source_type = 1;
   de->source_param = 1;

   /* Add image to Image Directory */
   ed->file->image_dir->entries =
        evas_list_append(ed->file->image_dir->entries, de);

   return 1;
}

EAPI int
edje_edit_image_id_get(Evas_Object *obj, const char *image_name)
{
   return _edje_image_id_find(obj, image_name);
}

EAPI int 
edje_edit_image_compression_type_get(Evas_Object *obj, const char *image)
{
   Edje_Image_Directory_Entry *i = NULL;
   Evas_List *l;

   GET_ED_OR_RETURN(-1);

   if (!ed->file) return -1;
   if (!ed->file->image_dir) return -1;

   for (l = ed->file->image_dir->entries; l; l = l->next)
     {
	i = l->data;
	if (strcmp(i->entry, image) == 0)
	  break;
	i = NULL;
     }

   if (!i) return -1;

   switch(i->source_type)
     {
	case EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT:
		if (i->source_param == 0) // RAW
		  return EDJE_EDIT_IMAGE_COMP_RAW;
		else // COMP
		  return EDJE_EDIT_IMAGE_COMP_COMP;
		break;
	case EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY: // LOSSY
		return EDJE_EDIT_IMAGE_COMP_LOSSY;
		break;
	case EDJE_IMAGE_SOURCE_TYPE_EXTERNAL: // USER
		return EDJE_EDIT_IMAGE_COMP_USER;
		break;
     }

   return -1;
}

EAPI int
edje_edit_image_compression_rate_get(Evas_Object *obj, const char *image)
{
   Evas_List *l;
   Edje_Image_Directory_Entry *i;

   GET_ED_OR_RETURN(-1);

   // Gets the Image Entry
   for (l = ed->file->image_dir->entries; l; l = l->next)
     {
	i = l->data;
	if (strcmp(i->entry, image) == 0) break;
	i = NULL;
     }

   if (!i) return -1;
   if (i->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY) return -2;

   return i->source_param;
}

EAPI const char *
edje_edit_state_image_get(Evas_Object *obj, const char *part, const char *state)
{
   char *image;

   GET_PD_OR_RETURN(NULL);

   image = (char *)_edje_image_name_find(obj, pd->image.id);
   if (!image) return NULL;

   //printf("GET IMAGE for %s [%s]\n", state, image);
   return evas_stringshare_add(image);
}

EAPI void
edje_edit_state_image_set(Evas_Object *obj, const char *part, const char *state, const char *image)
{
   int id;

   GET_PD_OR_RETURN();

   if (!image) return;

   id = _edje_image_id_find(obj, image);
   printf("SET IMAGE for %s [%s]\n", state, image);

   if (id > -1) pd->image.id = id;

   edje_object_calc_force(obj);
}

EAPI Evas_List *
edje_edit_state_tweens_list_get(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Part_Image_Id *i;
   Evas_List *tweens = NULL, *l;
   const char *name;

   GET_PD_OR_RETURN(NULL);

   //printf("GET TWEEN LIST for %s\n", state);

   for (l = pd->image.tween_list; l; l = l->next)
     {
	i = l->data;
	name = _edje_image_name_find(obj, i->id);
	//printf("   t: %s\n", name);
	tweens = evas_list_append(tweens, evas_stringshare_add(name));
     }

   return tweens;
}

EAPI unsigned char
edje_edit_state_tween_add(Evas_Object *obj, const char *part, const char *state, const char *tween)
{
   Edje_Part_Image_Id *i;
   int id;

   GET_PD_OR_RETURN(0);

   id = _edje_image_id_find(obj, tween);
   if (id < 0) return 0;

   /* alloc Edje_Part_Image_Id */
   i = mem_alloc(SZ(Edje_Part_Image_Id));
   if (!i) return 0;
   i->id = id;

   /* add to tween list */
   pd->image.tween_list = evas_list_append(pd->image.tween_list, i);

   return 1;
}

EAPI unsigned char
edje_edit_state_tween_del(Evas_Object *obj, const char *part, const char *state, const char *tween)
{
   Evas_List *l;
   int id;

   GET_PD_OR_RETURN(0);

   if (!pd->image.tween_list) return 0;

   id = _edje_image_id_find(obj, tween);
   if (id < 0) return 0;

   for (l = pd->image.tween_list; l; l = l->next)
     {
	Edje_Part_Image_Id *i;

	i = l->data;
	if (i->id == id)
	  {
	     pd->image.tween_list = evas_list_remove_list(pd->image.tween_list, l);
	     return 1;
	  }
     }
   return 0;
}

EAPI void
edje_edit_state_image_border_get(Evas_Object *obj, const char *part, const char *state, int *l, int *r, int *t, int *b)
{
   GET_PD_OR_RETURN();

   //printf("GET IMAGE_BORDER of state '%s'\n", state);

   if (l) *l = pd->border.l;
   if (r) *r = pd->border.r;
   if (t) *t = pd->border.t;
   if (b) *b = pd->border.b;
}

EAPI void
edje_edit_state_image_border_set(Evas_Object *obj, const char *part, const char *state, int l, int r, int t, int b)
{
   GET_PD_OR_RETURN();

   //printf("SET IMAGE_BORDER of state '%s'\n", state);

   if (l > -1) pd->border.l = l;
   if (r > -1) pd->border.r = r;
   if (t > -1) pd->border.t = t;
   if (b > -1) pd->border.b = b;

   edje_object_calc_force(obj);
}

EAPI unsigned char
edje_edit_state_image_border_fill_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN();
   return pd->border.no_fill ? 0 : 1;
}

EAPI void
edje_edit_state_image_border_fill_set(Evas_Object *obj, const char *part, const char *state, unsigned char fill)
{
   GET_PD_OR_RETURN();
   pd->border.no_fill = fill ? 0 : 1;

   edje_object_calc_force(obj);
}

/******************/
/*  SPECTRUM API  */
/******************/

EAPI Evas_List *
edje_edit_spectrum_list_get(Evas_Object *obj)
{
   Edje_Spectrum_Directory_Entry *s;
   Evas_List *spectrum = NULL;
   Evas_List *l;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->spectrum_dir) return NULL;

   //printf("GET SPECTRUM LIST for %s\n", ed->file->path);

   for (l = ed->file->spectrum_dir->entries; l; l = l->next)
     {
	s = l->data;
	//printf("SPECTRUM: %s [id: %d]\n", s->entry, s->id);
	spectrum = evas_list_append(spectrum, evas_stringshare_add(s->entry));
     }

   return spectrum;
}


EAPI unsigned char
edje_edit_spectra_add(Evas_Object *obj, const char* name)
{
   int id;
   GET_ED_OR_RETURN(0);

   printf("SPECTRA ADD [new name:%s]\n", name);

   Edje_Spectrum_Directory_Entry *s;
   Evas_List *l;

   if (!ed->file) return 0;

   if (_edje_edit_spectrum_entry_get(ed, name)) return 0;

   if (!ed->file->spectrum_dir)
     ed->file->spectrum_dir = mem_alloc(SZ(Edje_Spectrum_Directory));

   s = mem_alloc(SZ(Edje_Spectrum_Directory_Entry));
   ed->file->spectrum_dir->entries = evas_list_append(ed->file->spectrum_dir->entries, s);
   s->id = evas_list_count(ed->file->spectrum_dir->entries) - 1; //TODO Search for id holes
   s->entry = (char*)evas_stringshare_add(name);
   s->filename = NULL;
   s->color_list = NULL;

   return 1;
}

EAPI unsigned char
edje_edit_spectra_del(Evas_Object *obj, const char* spectra)
{
   Edje_Spectrum_Directory_Entry *s;

   GET_ED_OR_RETURN(0);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;

   printf("SPECTRA DEL %s\n", spectra);

   ed->file->spectrum_dir->entries = evas_list_remove(ed->file->spectrum_dir->entries, s);
   _edje_if_string_free(ed, s->entry);
   _edje_if_string_free(ed, s->filename);
   while (s->color_list)
     {
        Edje_Spectrum_Color *color;
        color = s->color_list->data;
        free(color);
        s->color_list = evas_list_remove_list(s->color_list, s->color_list);
     }
   free(s);

   return 1;
}

EAPI unsigned char
edje_edit_spectra_name_set(Evas_Object *obj, const char* spectra, const char* name)
{
   Edje_Spectrum_Directory_Entry *s;

   GET_ED_OR_RETURN(0);

   printf("SET SPECTRA NAME for spectra: %s [new name:%s]\n", spectra, name);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;

   _edje_if_string_free(ed, s->entry);
   s->entry = (char*)evas_stringshare_add(name);

   return 1;
}

EAPI int
edje_edit_spectra_stop_num_get(Evas_Object *obj, const char* spectra)
{
   Edje_Spectrum_Directory_Entry *s;

   GET_ED_OR_RETURN(0);

   //printf("GET SPECTRA STOP NUM for spectra: %s\n", spectra);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;

   return evas_list_count(s->color_list);
}

EAPI unsigned char
edje_edit_spectra_stop_num_set(Evas_Object *obj, const char* spectra, int num)
{
   Edje_Spectrum_Directory_Entry *s;
   Edje_Spectrum_Color *color;
   GET_ED_OR_RETURN(0);

   printf("SET SPECTRA STOP NUM for spectra: %s\n", spectra);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;

   if (num == evas_list_count(s->color_list)) return 1;

   //destroy all colors
   while (s->color_list)
     {
        color = s->color_list->data;
        free(color);
        s->color_list = evas_list_remove_list(s->color_list, s->color_list);
     }

   //... and recreate (TODO we should optimize this function)
   while (num)
     {
        color = mem_alloc(SZ(Edje_Spectrum_Color));
        s->color_list = evas_list_append(s->color_list, color);
        color->r = 255;
        color->g = 255;
        color->b = 255;
        color->a = 255;
        color->d = 10;
        num--;
     }

   return 1;
}

EAPI unsigned char
edje_edit_spectra_stop_color_get(Evas_Object *obj, const char* spectra, int stop_number, int *r, int *g, int *b, int *a, int *d)
{
   Edje_Spectrum_Directory_Entry *s;
   Edje_Spectrum_Color *color;
   GET_ED_OR_RETURN(0);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;
   //printf("GET SPECTRA STOP COLOR for spectra: %s stopn: %d\n", spectra, stop_number);

   color = evas_list_nth(s->color_list, stop_number);
   if (!color) return 0;
   if (r) *r = color->r;
   if (g) *g = color->g;
   if (b) *b = color->b;
   if (a) *a = color->a;
   if (d) *d = color->d;

   return 1;
}

EAPI unsigned char
edje_edit_spectra_stop_color_set(Evas_Object *obj, const char* spectra, int stop_number, int r, int g, int b, int a, int d)
{
   Edje_Spectrum_Directory_Entry *s;
   Edje_Spectrum_Color *color;
   GET_ED_OR_RETURN(0);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;
   printf("SET SPECTRA STOP COLOR for spectra: %s stopn: %d\n", spectra, stop_number);

   color = evas_list_nth(s->color_list, stop_number);
   if (!color) return 0;
   color->r = r;
   color->g = g;
   color->b = b;
   color->a = a;
   color->d = d;

   edje_object_calc_force(obj);

   return 1;
}


/******************/
/*  GRADIENT API  */
/******************/

EAPI const char *
edje_edit_state_gradient_type_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(NULL);

   if (!pd->gradient.type)
      return NULL;

//   printf("GET GRADIENT TYPE for part: %s state: %s [%s]\n", part, state, pd->gradient.type);

   return evas_stringshare_add(pd->gradient.type);
}

EAPI unsigned char
edje_edit_state_gradient_type_set(Evas_Object *obj, const char *part, const char *state, const char *type)
{
   GET_PD_OR_RETURN(0);
   if (!type) return 0;

//   printf("SET GRADIENT TYPE for part: %s state: %s TO: %s\n", part, state, type);

   _edje_if_string_free(ed, pd->gradient.type);
   pd->gradient.type = (char *)evas_stringshare_add(type);
   edje_object_calc_force(obj);
   return 1;
}


EAPI unsigned char
edje_edit_state_gradient_use_fill_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(-1);

   if (!pd->gradient.type)
      return -1;

   //~ if (!strcmp(pd->gradient.type, "linear"))
      //~ return 0;
   return 1;
}

EAPI const char *
edje_edit_state_gradient_spectra_get(Evas_Object *obj, const char *part, const char *state)
{
   Edje_Spectrum_Directory_Entry *s;

   GET_PD_OR_RETURN(0);

   //printf("GET GRADIENT SPECTRA for part: %s state: %s\n", part, state);
   s = _edje_edit_spectrum_entry_get_by_id(ed, pd->gradient.id);
   if (!s) return 0;

   return evas_stringshare_add(s->entry);
}

EAPI unsigned char
edje_edit_state_gradient_spectra_set(Evas_Object *obj, const char *part, const char *state, const char* spectra)
{
   Edje_Spectrum_Directory_Entry *s;

   GET_PD_OR_RETURN(0);

   printf("SET GRADIENT SPECTRA for part: %s state: %s [%s]\n", part, state, spectra);

   s = _edje_edit_spectrum_entry_get(ed, spectra);
   if (!s) return 0;

   pd->gradient.id = s->id;
   edje_object_calc_force(obj);

   return 1;
}

EAPI int
edje_edit_state_gradient_angle_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   return pd->fill.angle;
}

EAPI void
edje_edit_state_gradient_angle_set(Evas_Object *obj, const char *part, const char *state, int angle)
{
   GET_PD_OR_RETURN();
   pd->fill.angle = angle;
   edje_object_calc_force(obj);
}

EAPI double
edje_edit_state_gradient_rel1_relative_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL1 RELX for part: %s state: %s [%f]\n", part, state, pd->gradient.rel1.relative_x);

   return pd->gradient.rel1.relative_x;
}

EAPI double
edje_edit_state_gradient_rel1_relative_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL1 RELY for part: %s state: %s [%f]\n", part, state, pd->gradient.rel1.relative_y);

   return pd->gradient.rel1.relative_y;
}

EAPI double
edje_edit_state_gradient_rel2_relative_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL2 RELX for part: %s state: %s [%f]\n", part, state, pd->gradient.rel2.relative_x);

   return pd->gradient.rel2.relative_x;
}

EAPI double
edje_edit_state_gradient_rel2_relative_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL2 RELY for part: %s state: %s [%f]\n", part, state, pd->gradient.rel2.relative_y);

   return pd->gradient.rel2.relative_y;
}

EAPI unsigned char
edje_edit_state_gradient_rel1_relative_x_set(Evas_Object *obj, const char *part, const char *state, double val)
{
   GET_PD_OR_RETURN(0);
   //printf("SET GRADIENT REL1 RELX for part: %s state: %s [TO %f]\n", part, state, val);

   pd->gradient.rel1.relative_x = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI unsigned char
edje_edit_state_gradient_rel1_relative_y_set(Evas_Object *obj, const char *part, const char *state, double val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL1 RELY for part: %s state: %s [TO %f]\n", part, state, val);

   pd->gradient.rel1.relative_y = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI unsigned char
edje_edit_state_gradient_rel2_relative_x_set(Evas_Object *obj, const char *part, const char *state, double val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL2 RELX for part: %s state: %s [TO %f]\n", part, state, val);

   pd->gradient.rel2.relative_x = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI unsigned char
edje_edit_state_gradient_rel2_relative_y_set(Evas_Object *obj, const char *part, const char *state, double val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL2 RELY for part: %s state: %s [TO %f]\n", part, state, val);

   pd->gradient.rel2.relative_y = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI int
edje_edit_state_gradient_rel1_offset_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL1 OFFSETX for part: %s state: %s [%f]\n", part, state, pd->gradient.rel1.offset_x);
   return pd->gradient.rel1.offset_x;
}

EAPI int
edje_edit_state_gradient_rel1_offset_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL1 OFFSETY for part: %s state: %s [%f]\n", part, state, pd->gradient.rel1.offset_y);
   return pd->gradient.rel1.offset_y;
}

EAPI int
edje_edit_state_gradient_rel2_offset_x_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL2 OFFSETX for part: %s state: %s [%f]\n", part, state, pd->gradient.rel2.offset_x);
   return pd->gradient.rel2.offset_x;
}

EAPI int
edje_edit_state_gradient_rel2_offset_y_get(Evas_Object *obj, const char *part, const char *state)
{
   GET_PD_OR_RETURN(0);
   //printf("GET GRADIENT REL2 OFFSETY for part: %s state: %s [%f]\n", part, state, pd->gradient.rel2.offset_y);
   return pd->gradient.rel2.offset_y;
}

EAPI unsigned char
edje_edit_state_gradient_rel1_offset_x_set(Evas_Object *obj, const char *part, const char *state, int val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL1 OFFSETX for part: %s state: %s [TO %d]\n", part, state, val);

   pd->gradient.rel1.offset_x = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI unsigned char
edje_edit_state_gradient_rel1_offset_y_set(Evas_Object *obj, const char *part, const char *state, int val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL1 OFFSETY for part: %s state: %s [TO %d]\n", part, state, val);

   pd->gradient.rel1.offset_y = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI unsigned char
edje_edit_state_gradient_rel2_offset_x_set(Evas_Object *obj, const char *part, const char *state, int val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL2 OFFSETX for part: %s state: %s [TO %d]\n", part, state, val);

   pd->gradient.rel2.offset_x = val;
   edje_object_calc_force(obj);
   return 1;
}

EAPI unsigned char
edje_edit_state_gradient_rel2_offset_y_set(Evas_Object *obj, const char *part, const char *state, int val)
{
   GET_PD_OR_RETURN(0);
   printf("SET GRADIENT REL2 OFFSETY for part: %s state: %s [TO %d]\n", part, state, val);

   pd->gradient.rel2.offset_y = val;
   edje_object_calc_force(obj);
   return 1;
}

/******************/
/*  PROGRAMS API  */
/******************/
Edje_Program *
_edje_program_get_byname(Evas_Object *obj, const char *prog_name)
{
   Edje_Program *epr;
   int i;

   GET_ED_OR_RETURN(NULL);

   if (!prog_name) return NULL;

   for (i = 0; i < ed->table_programs_size; i++)
     {
	epr = ed->table_programs[i];
	if (strcmp(epr->name, prog_name) == 0)
	  return epr;
     }
   return NULL;
}

EAPI Evas_List *
edje_edit_programs_list_get(Evas_Object *obj)
{
   Evas_List *progs = NULL;
   int i;

   GET_ED_OR_RETURN(NULL);

   //printf("EE: Found %d programs\n", ed->table_programs_size);

   for (i = 0; i < ed->table_programs_size; i++)
     {
	Edje_Program *epr;

	epr = ed->table_programs[i];
	progs = evas_list_append(progs, evas_stringshare_add(epr->name));
     }

   return progs;
}

EAPI unsigned char
edje_edit_program_add(Evas_Object *obj, const char *name)
{
   Edje_Program *epr;
   Edje_Part_Collection *pc;

   GET_ED_OR_RETURN(0);

   printf("ADD PROGRAM [new name: %s]\n", name);

   //Check if program already exists
   if (_edje_program_get_byname(obj, name))
     return 0;

   //Alloc Edje_Program or return
   epr = mem_alloc(SZ(Edje_Program));
   if (!epr) return 0;

   //Add program to group
   pc = ed->collection;
   pc->programs = evas_list_append(pc->programs, epr);

   //Init Edje_Program
   epr->id = evas_list_count(pc->programs) - 1;
   epr->name = evas_stringshare_add(name);
   epr->signal = NULL;
   epr->source = NULL;
   epr->in.from = 0.0;
   epr->in.range = 0.0;
   epr->action = 0;
   epr->state = NULL;
   epr->value = 0.0;
   epr->state2 = NULL;
   epr->value2 = 0.0;
   epr->tween.mode = 1;
   epr->tween.time = 0.0;
   epr->targets = NULL;
   epr->after = NULL;


   //Update table_programs
   ed->table_programs_size++;
   ed->table_programs = realloc(ed->table_programs,
                                sizeof(Edje_Program *) * ed->table_programs_size);
   ed->table_programs[epr->id % ed->table_programs_size] = epr;

   //Update patterns
   if (ed->patterns.programs.signals_patterns)
     {
	edje_match_patterns_free(ed->patterns.programs.signals_patterns);
	edje_match_patterns_free(ed->patterns.programs.sources_patterns);
     }
   ed->patterns.programs.signals_patterns = edje_match_programs_signal_init(ed->collection->programs);
   ed->patterns.programs.sources_patterns = edje_match_programs_source_init(ed->collection->programs);

   return 1;
}

EAPI unsigned char
edje_edit_program_del(Evas_Object *obj, const char *prog)
{
   Evas_List *l;
   Edje_Part_Collection *pc;
   int id, i;
   int old_id;

   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   printf("DEL PROGRAM: %s\n", prog);

   //Remove program from programs list
   id = epr->id;
   pc = ed->collection;
   pc->programs = evas_list_remove(pc->programs, epr);

   //Free Edje_Program
   _edje_if_string_free(ed, epr->name);
   _edje_if_string_free(ed, epr->signal);
   _edje_if_string_free(ed, epr->source);
   _edje_if_string_free(ed, epr->state);
   _edje_if_string_free(ed, epr->state2);

   while (epr->targets)
     {
	Edje_Program_Target *prt;

	prt = epr->targets->data;
	epr->targets = evas_list_remove_list(epr->targets, epr->targets);
	free(prt);
     }
   while (epr->after)
     {
	Edje_Program_After *pa;

	pa = epr->after->data;
	epr->after = evas_list_remove_list(epr->after, epr->after);
	free(pa);
     }
   free(epr);


   //Update programs table
   //We move the last program in place of the deleted one
   //and realloc the table without the last element.
   ed->table_programs[id % ed->table_programs_size] = ed->table_programs[ed->table_programs_size-1];
   ed->table_programs_size--;
   ed->table_programs = realloc(ed->table_programs,
                             sizeof(Edje_Program *) * ed->table_programs_size);

   //Update the id of the moved program
   if (id < ed->table_programs_size)
     {
	Edje_Program *p;

	p = ed->table_programs[id % ed->table_programs_size];
	printf("UPDATE: %s(id:%d) with new id: %d\n",
	       p->name, p->id, id);
	old_id = p->id;
	p->id = id;
     }
   else
     old_id = -1;

   //We also update all other programs that point to old_id and id
   for (i = 0; i < ed->table_programs_size; i++)
     {
	Edje_Program *p;

	p = ed->table_programs[i];
	// printf("Check dependencies on %s\n", p->name);
	/* check in afters */
	l = p->after;
	while (l)
	  {
	     Edje_Program_After *pa;

	     pa = l->data;
	     if (pa->id == old_id)
	       {
		  //    printf("   dep on after old_id\n");
		  pa->id = id;
	       }
	     else if (pa->id == id)
	       {
		  //  printf("   dep on after id\n");
		  p->after = evas_list_remove(p->after, pa);
	       }

	     if (l) l = l->next;
	  }
	/* check in targets */
	if (p->action == EDJE_ACTION_TYPE_ACTION_STOP)
	  {
	     l = p->targets;
	     while (l)
	       {
		  Edje_Program_Target *pt;

		  pt = l->data;
		  if (pt->id == old_id)
		    {
		       // printf("   dep on target old_id\n");
		       pt->id = id;
		    }
		  else if (pt->id == id)
		    {
		       // printf("   dep on target id\n");
		       p->targets = evas_list_remove(p->targets, pt);
		    }
		  if (l) l = l->next;
	       }
	  }
     }

   return 1;
}

EAPI unsigned char
edje_edit_program_exist(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   return 1;
}

EAPI unsigned char
edje_edit_program_run(Evas_Object *obj, const char *prog)
{
   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   _edje_program_run(ed, epr, 0, "", "");
   return 1;
}

EAPI unsigned char
edje_edit_program_name_set(Evas_Object *obj, const char *prog, const char* new_name)
{
   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   if (!new_name) return 0;

   if (_edje_program_get_byname(obj, new_name)) return 0;

   printf("SET NAME for program: %s [new name: %s]\n", prog, new_name);

   _edje_if_string_free(ed, epr->name);
   epr->name = evas_stringshare_add(new_name);

   return 1;
}

EAPI const char *
edje_edit_program_source_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->source) return NULL;
   //printf("GET SOURCE for program: %s [%s]\n", prog, epr->source);
   return evas_stringshare_add(epr->source);
}

EAPI unsigned char
edje_edit_program_source_set(Evas_Object *obj, const char *prog, const char *source)
{
   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   if (!source) return 0;

   printf("SET SOURCE for program: %s [%s]\n", prog, source);

   _edje_if_string_free(ed, epr->source);
   epr->source = evas_stringshare_add(source);

   //Update patterns
   if (ed->patterns.programs.sources_patterns)
      edje_match_patterns_free(ed->patterns.programs.sources_patterns);
   ed->patterns.programs.sources_patterns = edje_match_programs_source_init(ed->collection->programs);

   return 1;
}

EAPI const char *
edje_edit_program_signal_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->signal) return NULL;
   //printf("GET SIGNAL for program: %s [%s]\n", prog, epr->signal);
   return evas_stringshare_add(epr->signal);
}

EAPI unsigned char
edje_edit_program_signal_set(Evas_Object *obj, const char *prog, const char *signal)
{
   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   if (!signal) return 0;

   printf("SET SIGNAL for program: %s [%s]\n", prog, signal);

   _edje_if_string_free(ed, epr->signal);
   epr->signal = evas_stringshare_add(signal);

   //Update patterns
   if (ed->patterns.programs.signals_patterns)
      edje_match_patterns_free(ed->patterns.programs.signals_patterns);
   ed->patterns.programs.signals_patterns = edje_match_programs_signal_init(ed->collection->programs);

   return 1;
}

EAPI const char *
edje_edit_program_state_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->state) return NULL;
   //printf("GET STATE for program: %s [%s %.2f]\n", prog, epr->state, epr->value);
   return evas_stringshare_add(epr->state);
}

EAPI unsigned char
edje_edit_program_state_set(Evas_Object *obj, const char *prog, const char *state)
{
   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   printf("SET STATE for program: %s\n", prog);

   _edje_if_string_free(ed, epr->state);
   epr->state = evas_stringshare_add(state);

   return 1;
}

EAPI const char *
edje_edit_program_state2_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->state2) return NULL;
   //printf("GET STATE2 for program: %s [%s %.2f]\n", prog, epr->state2, epr->value2);
   return evas_stringshare_add(epr->state2);
}

EAPI unsigned char
edje_edit_program_state2_set(Evas_Object *obj, const char *prog, const char *state2)
{
   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   printf("SET STATE2 for program: %s\n", prog);

   _edje_if_string_free(ed, epr->state2);
   epr->state2 = evas_stringshare_add(state2);

   return 1;
}

EAPI double
edje_edit_program_value_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET VALUE for program: %s [%s %.2f]\n", prog, epr->state, epr->value);
   return epr->value;
}

EAPI unsigned char
edje_edit_program_value_set(Evas_Object *obj, const char *prog, double value)
{
   GET_EPR_OR_RETURN(0);

   //printf("SET VALUE for program: %s [%.2f]\n", prog, value);
   epr->value = value;
   return 1;
}

EAPI double
edje_edit_program_value2_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET VALUE2 for program: %s [%s %.2f]\n", prog, epr->state2, epr->value2);
   return epr->value2;
}

EAPI unsigned char
edje_edit_program_value2_set(Evas_Object *obj, const char *prog, double value)
{
   GET_EPR_OR_RETURN(0);

   //printf("SET VALUE for program: %s [%.2f]\n", prog, value);
   epr->value2 = value;
   return 1;
}

EAPI double
edje_edit_program_in_from_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   //printf("GET IN.FROM for program: %s [%f]\n", prog, epr->in.from);
   return epr->in.from;
}

EAPI unsigned char
edje_edit_program_in_from_set(Evas_Object *obj, const char *prog, double seconds)
{
   GET_EPR_OR_RETURN(0);

   //printf("SET IN.FROM for program: %s [%f]\n", prog, epr->in.from);
   epr->in.from = seconds;
   return 1;
}

EAPI double
edje_edit_program_in_range_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   //printf("GET IN.RANGE for program: %s [%f]\n", prog, epr->in.range);
   return epr->in.range;
}

EAPI unsigned char
edje_edit_program_in_range_set(Evas_Object *obj, const char *prog, double seconds)
{
   GET_EPR_OR_RETURN(0);

   //printf("SET IN.RANGE for program: %s [%f]\n", prog, epr->in.range);
   epr->in.range = seconds;
   return 1;
}

EAPI int
edje_edit_program_transition_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET TRANSITION for program: %s [%d]\n", prog, epr->tween.mode);
   return epr->tween.mode;
}

EAPI unsigned char
edje_edit_program_transition_set(Evas_Object *obj, const char *prog, int transition)
{
   GET_EPR_OR_RETURN(0);

   //printf("GET TRANSITION for program: %s [%d]\n", prog, epr->tween.mode);
   epr->tween.mode = transition;
   return 1;
}

EAPI double
edje_edit_program_transition_time_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET TRANSITION_TIME for program: %s [%.4f]\n", prog, epr->tween.time);
   return epr->tween.time;
}

EAPI unsigned char
edje_edit_program_transition_time_set(Evas_Object *obj, const char *prog, double seconds)
{
   GET_EPR_OR_RETURN(0);

   //printf("GET TRANSITION_TIME for program: %s [%.4f]\n", prog, epr->tween.time);
   epr->tween.time = seconds;
   return 1;
}

EAPI int
edje_edit_program_action_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET ACTION for program: %s [%d]\n", prog, epr->action);
   return epr->action;
}

EAPI unsigned char
edje_edit_program_action_set(Evas_Object *obj, const char *prog, int action)
{
   GET_EPR_OR_RETURN(0);

   //printf("SET ACTION for program: %s [%d]\n", prog, action);
   if (action >= EDJE_ACTION_TYPE_LAST) return 0;

   epr->action = action;
   return 1;
}

EAPI Evas_List *
edje_edit_program_targets_get(Evas_Object *obj, const char *prog)
{
   Evas_List *l, *targets = NULL;

   GET_ED_OR_RETURN(NULL);
   GET_EPR_OR_RETURN(NULL);

   //printf("GET TARGETS for program: %s [count: %d]\n", prog, evas_list_count(epr->targets));

   for (l = epr->targets; l; l = l->next)
     {
	Edje_Program_Target *t;

	t = l->data;

	if (epr->action == EDJE_ACTION_TYPE_STATE_SET)
	  {
	     /* the target is a part */
	     Edje_Real_Part *p = NULL;

	     p = ed->table_parts[t->id % ed->table_parts_size];
	     if (p && p->part && p->part->name)
	       targets = evas_list_append(targets,
		     evas_stringshare_add(p->part->name));
	  }
	else if (epr->action == EDJE_ACTION_TYPE_ACTION_STOP)
	  {
	     /* the target is a program */
	     Edje_Program *p;

	     p = ed->table_programs[t->id % ed->table_programs_size];
	     if (p && p->name)
	       targets = evas_list_append(targets,
		     evas_stringshare_add(p->name));
	  }
     }
   return targets;
}

EAPI unsigned char
edje_edit_program_targets_clear(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   while (epr->targets)
     {
	Edje_Program_Target *prt;

	prt = epr->targets->data;
	epr->targets = evas_list_remove_list(epr->targets, epr->targets);
	free(prt);
     }

   return 1;
}

EAPI unsigned char
edje_edit_program_target_add(Evas_Object *obj, const char *prog, const char *target)
{
   int id;
   Edje_Program_Target *t;

   GET_ED_OR_RETURN(0);
   GET_EPR_OR_RETURN(0);

   if (epr->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	/* the target is a part */
	Edje_Real_Part *rp;

	rp = _edje_real_part_get(ed, target);
	if (!rp) return 0;
	id = rp->part->id;
     }
   else if (epr->action == EDJE_ACTION_TYPE_ACTION_STOP)
     {
	/* the target is a program */
	Edje_Program *tar;

	tar = _edje_program_get_byname(obj, target);
	if (!tar) return 0;
	id = tar->id;
     }
   else
     return 0;

   t = mem_alloc(SZ(Edje_Program_Target));
   if (!t) return 0;

   t->id = id;
   epr->targets = evas_list_append(epr->targets, t);

   return 1;
}

EAPI Evas_List *
edje_edit_program_afters_get(Evas_Object *obj, const char *prog)
{
   Evas_List *l, *afters = NULL;

   GET_ED_OR_RETURN(NULL);
   GET_EPR_OR_RETURN(NULL);

  // printf("GET AFTERS for program: %s [count: %d]\n", prog, evas_list_count(epr->after));

   for (l = epr->after; l; l = l->next)
     {
	Edje_Program_After *a;
	Edje_Program *p = NULL;

	a = l->data;
	p = ed->table_programs[a->id % ed->table_programs_size];
	if (p && p->name)
	  {
	     printf("   a: %d name: %s\n", a->id, p->name);
	     afters = evas_list_append(afters, evas_stringshare_add(p->name));
	  }
     }
   return afters;
}

EAPI unsigned char
edje_edit_program_afters_clear(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   while (epr->after)
     {
	Edje_Program_After *pa;

	pa = epr->after->data;
	epr->after = evas_list_remove_list(epr->after, epr->after);
	free(pa);
     }

   return 1;
}

EAPI unsigned char
edje_edit_program_after_add(Evas_Object *obj, const char *prog, const char *after)
{
   Edje_Program *af;
   Edje_Program_After *a;

   GET_EPR_OR_RETURN(0);

   af = _edje_program_get_byname(obj, after);
   if (!af) return 0;

   a = mem_alloc(SZ(Edje_Program_After));
   if (!a) return 0;

   a->id = af->id;

   epr->after = evas_list_append(epr->after, a);

   return 1;
}

/*************************/
/*  EMBRYO SCRIPTS  API  */
/*************************/
EAPI const char *
edje_edit_script_get(Evas_Object *obj)
{
   Embryo_Program   *script = NULL;

   GET_ED_OR_RETURN(NULL);

   if (!ed->collection) return NULL;
   if (!ed->collection->script) return NULL;

   script = ed->collection->script;

   printf("Get Script [%p] %d\n", script, embryo_program_recursion_get(script));

   return "Not yet complete...";
}


/***************************/
/*  EDC SOURCE GENERATION  */
/***************************/
#define I0 ""
#define I1 "   "
#define I2 "      "
#define I3 "         "
#define I4 "            "
#define I5 "               "
#define I6 "                  "
#define I7 "                     "

static char *types[] = {"NONE", "RECT", "TEXT", "IMAGE", "SWALLOW", "TEXTBLOCK", "GRADIENT", "GROUP"};
static char *effects[] = {"NONE", "PLAIN", "OUTLINE", "SOFT_OUTLINE", "SHADOW", "SOFT_SHADOW", "OUTLINE_SHADOW", "OUTLINE_SOFT_SHADOW ", "FAR_SHADOW ", "FAR_SOFT_SHADOW", "GLOW"};
static char *prefers[] = {"NONE", "VERTICAL", "HORIZONTAL", "BOTH"};
static void
_edje_generate_source_of_spectra(Edje * ed, const char *name, FILE * f)
{
   Edje_Spectrum_Directory_Entry *d;
   Edje_Spectrum_Color *color = NULL;
   Evas_List *l;

   if (!ed || !name || !f) return;

   if (d = _edje_edit_spectrum_entry_get(ed, name))
     {
	fprintf(f, I1 "spectrum {\n");
	fprintf(f, I2 "name: \"%s\";\n", d->entry);

	for (l = d->color_list; l; l = l->next)
	  {
		color = l->data;
		if (color)
		  fprintf(f, I2 "color: %d %d %d %d %d;\n", color->r, color->g,
			  color->b, color->a, color->d);
	  }

	fprintf(f, I1 "}\n");
     }
}

 static void
_edje_generate_source_of_colorclass(Edje * ed, const char *name, FILE * f)
{
   Evas_List *l;
   Edje_Color_Class *cc;

   for (l = ed->file->color_classes; l; l = l->next)
     {
	cc = l->data;
	if (!strcmp(cc->name, name))
	  {
		fprintf(f, I1 "color_class {\n");
		fprintf(f, I2 "name: \"%s\";\n", cc->name);
		fprintf(f, I2 "color: %d %d %d %d;\n", cc->r, cc->g, cc->b, cc->a);
		fprintf(f, I2 "color2: %d %d %d %d;\n", cc->r2, cc->g2, cc->b2, cc->a2);
		fprintf(f, I2 "color3: %d %d %d %d;\n", cc->r3, cc->g3, cc->b3, cc->a3);
		fprintf(f, I1 "}\n");
	  }
     }
}

static void
_edje_generate_source_of_program(Evas_Object *obj, const char *program, FILE *f)
{
   Evas_List *l, *ll;
   const char *s, *s2;
   double db, db2;

   GET_ED_OR_RETURN();

   fprintf(f, I3"program {\n");
   fprintf(f, I4"name: \"%s\";\n", program);

   /* Signal */
   if (s = edje_edit_program_signal_get(obj, program))
     {
	fprintf(f, I4"signal: \"%s\";\n", s);
	edje_edit_string_free(s);
     }

   /* Source */
   if (s = edje_edit_program_source_get(obj, program))
     {
	fprintf(f, I4"source: \"%s\";\n", s);
	edje_edit_string_free(s);
     }

   /* Action */
   switch (edje_edit_program_action_get(obj, program))
     {
     case EDJE_ACTION_TYPE_ACTION_STOP:
	fprintf(f, I4"action: ACTION_STOP;\n");
	break;
     case EDJE_ACTION_TYPE_STATE_SET:
	if (s = edje_edit_program_state_get(obj, program))
	  {
		fprintf(f, I4"action: STATE_SET \"%s\" %.2f;\n", s,
			edje_edit_program_value_get(obj, program));
		edje_edit_string_free(s);
	  }
	break;
     case EDJE_ACTION_TYPE_SIGNAL_EMIT:
	s = edje_edit_program_state_get(obj, program);
	s2 = edje_edit_program_state2_get(obj, program);
	if (s && s2)
	  {
		fprintf(f, I4"action: SIGNAL_EMIT \"%s\" \"%s\";\n", s, s2);
		edje_edit_string_free(s);
		edje_edit_string_free(s2);
	  }
	break;
     //TODO Support Drag
     //~ case EDJE_ACTION_TYPE_DRAG_VAL_SET:
	//~ fprintf(f, I4"action: DRAG_VAL_SET TODO;\n");
	//~ break;
     //~ case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
	//~ fprintf(f, I4"action: DRAG_VAL_STEP TODO;\n");
	//~ break;
     //~ case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
	//~ fprintf(f, I4"action: DRAG_VAL_PAGE TODO;\n");
	//~ break;
     }

   /* Transition */
   db = edje_edit_program_transition_time_get(obj, program);
   switch (edje_edit_program_transition_get(obj, program))
     {
     case EDJE_TWEEN_MODE_LINEAR:
	fprintf(f, I4"transition: LINEAR %.5f;\n", db);
	break;
     case EDJE_TWEEN_MODE_ACCELERATE:
	fprintf(f, I4"transition: ACCELERATE %.5f;\n", db);
	break;
     case EDJE_TWEEN_MODE_DECELERATE:
	fprintf(f, I4"transition: DECELERATE %.5f;\n", db);
	break;
     case EDJE_TWEEN_MODE_SINUSOIDAL:
	fprintf(f, I4"transition: SINUSOIDAL %.5f;\n", db);
	break;
     }

   /* In */
   db = edje_edit_program_in_from_get(obj, program);
   db2 = edje_edit_program_in_range_get(obj, program);
   if (db || db2)
     fprintf(f, I4"in: %.5f %.5f;\n", db, db2);

   /* Targets */
   if (ll = edje_edit_program_targets_get(obj, program))
     {
	for (l = ll; l; l = l->next)
	  fprintf(f, I4"target: \"%s\";\n", (char *)l->data);
	edje_edit_string_list_free(ll);
     }

   /* Afters */
   if (ll = edje_edit_program_afters_get(obj, program))
     {
	for (l = ll; l; l = l->next)
	  fprintf(f, I4"after: \"%s\";\n", (char *)l->data);
	edje_edit_string_list_free(ll);
     }

   // TODO Support script {}

   fprintf(f, I3 "}\n");
}


static void
_edje_generate_source_of_state(Evas_Object *obj, const char *part, const char *state, FILE *f)
{
   Evas_List *l, *ll;
   Edje_Real_Part *rp;
   const char *str;
   
   GET_PD_OR_RETURN();
   
   rp = _edje_real_part_get(ed, part);
   if (!rp) return;
   
   fprintf(f, I4"description {\n");
   //TODO Support inherit
   fprintf(f, I5"state: \"%s\" %g;\n", pd->state.name, pd->state.value);
   
   if (!pd->visible)
     fprintf(f, I5"visible: 0;\n");
   
   if (pd->align.x != 0.5 || pd->align.y != 0.5)
     fprintf(f, I5"align: %g %g;\n", pd->align.x, pd->align.y);
   
   //TODO Support fixed
   
   if (pd->min.w || pd->min.h)
     fprintf(f, I5"min: %d %d;\n", pd->min.w, pd->min.h);
   if (pd->max.w != -1 || pd->max.h != -1)
     fprintf(f, I5"max: %d %d;\n", pd->max.w, pd->max.h);
   
   //TODO Support step
   
   if (pd->aspect.min || pd->aspect.max)
      fprintf(f, I5"aspect: %g %g;\n", pd->aspect.min, pd->aspect.max);
   if (pd->aspect.prefer)
      fprintf(f, I5"aspect_preference: %s;\n", prefers[pd->aspect.prefer]);
   
   if (pd->color_class)
      fprintf(f, I5"color_class: \"%s\";\n", pd->color_class);
   
   if (pd->color.r != 255 || pd->color.g != 255 ||
       pd->color.b != 255 ||  pd->color.a != 255)
      fprintf(f, I5"color: %d %d %d %d;\n",
              pd->color.r, pd->color.g, pd->color.b, pd->color.a);
   if (pd->color2.r != 0 || pd->color2.g != 0 ||
        pd->color2.b != 0 ||  pd->color2.a != 255)
      fprintf(f, I5"color2: %d %d %d %d;\n",
              pd->color2.r, pd->color2.g, pd->color2.b, pd->color2.a);
   if (pd->color3.r != 0 || pd->color3.g != 0 ||
        pd->color3.b != 0 ||  pd->color3.a != 128)
      fprintf(f, I5"color3: %d %d %d %d;\n",
              pd->color3.r, pd->color3.g, pd->color3.b, pd->color3.a);
   
   //Rel1
   if (pd->rel1.relative_x || pd->rel1.relative_y || pd->rel1.offset_x ||
       pd->rel1.offset_y || pd->rel1.id_x != -1 || pd->rel1.id_y != -1)
     {
	fprintf(f, I5"rel1 {\n");
	if (pd->rel1.relative_x || pd->rel1.relative_y)
	  fprintf(f, I6"relative: %g %g;\n", pd->rel1.relative_x, pd->rel1.relative_y);
	if (pd->rel1.offset_x || pd->rel1.offset_y)
	  fprintf(f, I6"offset: %d %d;\n", pd->rel1.offset_x, pd->rel1.offset_y);
	if (pd->rel1.id_x != -1 && pd->rel1.id_x == pd->rel1.id_y)
	  fprintf(f, I6"to: \"%s\";\n", ed->table_parts[pd->rel1.id_x]->part->name);
	else
	  {
		if (pd->rel1.id_x != -1)
		  fprintf(f, I6"to_x: \"%s\";\n", ed->table_parts[pd->rel1.id_x]->part->name);
		if (pd->rel1.id_y != -1)
		  fprintf(f, I6"to_y: \"%s\";\n", ed->table_parts[pd->rel1.id_y]->part->name);
	  }
	fprintf(f, I5"}\n");//rel1
     }
   
   //Rel2
   if (pd->rel2.relative_x != 1.0 || pd->rel2.relative_y != 1.0 ||
       pd->rel2.offset_x != -1 || pd->rel2.offset_y != -1 ||
       pd->rel2.id_x != -1 || pd->rel2.id_y != -1)
     {
	fprintf(f, I5"rel2 {\n");
	if (pd->rel2.relative_x != 1.0 || pd->rel2.relative_y != 1.0)
	  fprintf(f, I6"relative: %g %g;\n", pd->rel2.relative_x, pd->rel2.relative_y);
	if (pd->rel2.offset_x != -1 || pd->rel2.offset_y != -1)
	  fprintf(f, I6"offset: %d %d;\n", pd->rel2.offset_x, pd->rel2.offset_y);
	if (pd->rel2.id_x != -1 && pd->rel2.id_x == pd->rel2.id_y)
	  fprintf(f, I6"to: \"%s\";\n", ed->table_parts[pd->rel2.id_x]->part->name);
	else
	  {
		if (pd->rel2.id_x != -1)
		  fprintf(f, I6"to_x: \"%s\";\n", ed->table_parts[pd->rel2.id_x]->part->name);
		if (pd->rel2.id_y != -1)
		  fprintf(f, I6"to_y: \"%s\";\n", ed->table_parts[pd->rel2.id_y]->part->name);
	  }
	fprintf(f, I5"}\n");//rel2
     }
   
   //Image
   if (rp->part->type == EDJE_PART_TYPE_IMAGE)
     {
	fprintf(f, I5"image {\n");
	fprintf(f, I6"normal: \"%s\";\n", _edje_image_name_find(obj, pd->image.id));

	ll = edje_edit_state_tweens_list_get(obj, part, state);
	for (l = ll; l; l = l->next)
	  fprintf(f, I6"tween: \"%s\";\n", (char *)l->data);
	edje_edit_string_list_free(ll);
      
	if (pd->border.l || pd->border.r || pd->border.t || pd->border.b)
	  fprintf(f, I6"border: %d %d %d %d;\n", pd->border.l, pd->border.r, pd->border.t, pd->border.b);
	if (pd->border.no_fill)
	  fprintf(f, I6"middle: 0;\n");

	fprintf(f, I5"}\n");//image
     }
   
   //Fill
   if (rp->part->type == EDJE_PART_TYPE_IMAGE ||
       rp->part->type == EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(f, I5"fill {\n");
	if (rp->part->type == EDJE_PART_TYPE_IMAGE && !pd->fill.smooth)
	  fprintf(f, I6"smooth: 0;\n");
        //TODO Support spread
	if (rp->part->type == EDJE_PART_TYPE_GRADIENT && pd->fill.angle)
	  fprintf(f, I6"angle: %d;\n", pd->fill.angle);
        //TODO Support type
        
	if (pd->fill.pos_rel_x || pd->fill.pos_rel_y ||
            pd->fill.pos_abs_x || pd->fill.pos_abs_y)
	  {
		fprintf(f, I6"origin {\n");
		if (pd->fill.pos_rel_x || pd->fill.pos_rel_y)
		  fprintf(f, I7"relative: %g %g;\n", pd->fill.pos_rel_x, pd->fill.pos_rel_y);
		if (pd->fill.pos_abs_x || pd->fill.pos_abs_y)
		  fprintf(f, I7"offset: %d %d;\n", pd->fill.pos_abs_x, pd->fill.pos_abs_y);
		fprintf(f, I6"}\n");
          }

	if (pd->fill.rel_x != 1.0 || pd->fill.rel_y != 1.0 ||
            pd->fill.abs_x || pd->fill.abs_y)
	  {
		fprintf(f, I6"size {\n");
		if (pd->fill.rel_x != 1.0 || pd->fill.rel_y != 1.0)
		  fprintf(f, I7"relative: %g %g;\n", pd->fill.rel_x, pd->fill.rel_y);
		if (pd->fill.abs_x || pd->fill.abs_y)
		  fprintf(f, I7"offset: %d %d;\n", pd->fill.abs_x, pd->fill.abs_y);
		fprintf(f, I6"}\n");
          }
        
	fprintf(f, I5"}\n");
     }
      
   //Text
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     {
	fprintf(f, I5"text {\n");
	if (pd->text.text)
	  fprintf(f, I6"text: \"%s\";\n", pd->text.text);
	fprintf(f, I6"font: \"%s\";\n", pd->text.font);
	fprintf(f, I6"size: %d;\n", pd->text.size);
	if (pd->text.text_class)
	  fprintf(f, I6"text_class: \"%s\";\n", pd->text.text_class);
	if (pd->text.fit_x || pd->text.fit_y)
	  fprintf(f, I6"fit: %d %d;\n", pd->text.fit_x, pd->text.fit_y);
        //TODO Support min & max
	if (pd->text.align.x != 0.5 || pd->text.align.y != 0.5)
	  fprintf(f, I6"align: %g %g;\n", pd->text.align.x, pd->text.align.y);
        //TODO Support source
        //TODO Support text_source
	if (pd->text.elipsis)
	  fprintf(f, I6"elipsis: %g;\n", pd->text.elipsis);
	fprintf(f, I5"}\n");
     }

   //Gradient
   if (rp->part->type == EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(f, I5"gradient {\n");
	fprintf(f, I6"type: \"%s\";\n", pd->gradient.type);
	str = edje_edit_state_gradient_spectra_get(obj, part, state);
	if (str)
	  {
		fprintf(f, I6"spectrum: \"%s\";\n", str);
		edje_edit_string_free(str);
	  }
        //TODO rel1 and 2 seems unused
	fprintf(f, I5"}\n");
     }
   
   fprintf(f, I4"}\n");//description
}

static void
_edje_generate_source_of_part(Evas_Object *obj, const char *part, FILE *f)
{
   const char *str;
   Evas_List *l, *ll;
   
   fprintf(f, I3"part {\n");
   fprintf(f, I4"name: \"%s\";\n", part);
   fprintf(f, I4"type: %s;\n", types[edje_edit_part_type_get(obj, part)]);
   if (!edje_edit_part_mouse_events_get(obj, part))
      fprintf(f, I4"mouse_events: 0;\n");
   if (edje_edit_part_repeat_events_get(obj, part))
      fprintf(f, I4"repeat_events: 1;\n");
   //TODO Support ignore_flags
   //TODO Support scale
   //TODO Support pointer_mode
   //TODO Support precise_is_inside
   //TODO Support use_alternate_font_metrics
   if ((str = edje_edit_part_clip_to_get(obj, part)))
     {
        fprintf(f, I4"clip_to: \"%s\";\n", str);
        edje_edit_string_free(str);
     }
   if ((str = edje_edit_part_source_get(obj, part)))
     {
        fprintf(f, I4"source: \"%s\";\n", str);
        edje_edit_string_free(str);
     }
   if (edje_edit_part_effect_get(obj, part))
     fprintf(f, I4"effect: %s;\n", effects[edje_edit_part_effect_get(obj, part)]);

   //Dragable
   if (edje_edit_part_drag_x_get(obj, part) ||
       edje_edit_part_drag_x_get(obj, part))
     {
	fprintf(f, I4"dragable {\n");
	fprintf(f, I5"x: %d %d %d;\n", edje_edit_part_drag_x_get(obj, part),
	                               edje_edit_part_drag_step_x_get(obj, part),
	                               edje_edit_part_drag_count_x_get(obj, part));
	fprintf(f, I5"y: %d %d %d;\n", edje_edit_part_drag_y_get(obj, part),
	                               edje_edit_part_drag_step_y_get(obj, part),
	                               edje_edit_part_drag_count_y_get(obj, part));
	if (str = edje_edit_part_drag_confine_get(obj, part))
	  {
		fprintf(f, I5"confine: \"%s\";\n", str);
		edje_edit_string_free(str);
	  }
	if (str = edje_edit_part_drag_event_get(obj, part))
	  {
		fprintf(f, I5"events: \"%s\";\n", str);
		edje_edit_string_free(str);
	  }
	fprintf(f, I4"}\n");
     }

   //Descriptions
   ll = edje_edit_part_states_list_get(obj, part);
   for (l = ll; l; l = l->next)
     _edje_generate_source_of_state(obj, part, (char*)l->data, f);
   edje_edit_string_list_free(ll);
   
   fprintf(f, I3"}\n");//part
}

static void
_edje_generate_source_of_group(Edje *ed, const char *group, FILE *f)
{
   Evas_Object *obj;
   Evas_List *l, *ll;
   int w, h;
   
   obj = edje_object_add(ed->evas);
   if (!edje_object_file_set(obj, ed->file->path, group)) return;
      
   fprintf(f, I1"group {\n");
   fprintf(f, I2"name: \"%s\";\n", group);
   //TODO Support alias:
   if ((w = edje_edit_group_min_w_get(obj)) || (h = edje_edit_group_min_h_get(obj)))
      fprintf(f, I2"min: %d %d;\n", w, h);
   if ((w = edje_edit_group_max_w_get(obj)) || (h = edje_edit_group_max_h_get(obj)))
      fprintf(f, I2"max: %d %d;\n", w, h);
   //TODO Support data
   //TODO Support script

   /* Parts */
   fprintf(f, I2"parts {\n");
   ll = edje_edit_parts_list_get(obj);
   for (l = ll; l; l = l->next)
     {
        _edje_generate_source_of_part(obj, (char*)l->data, f);
     }
   edje_edit_string_list_free(ll);
   fprintf(f, I2"}\n");//parts

   /* Programs */
   if (ll = edje_edit_programs_list_get(obj))
     {
	fprintf(f, I2 "programs {\n");
	for (l = ll; l; l = l->next)
	  _edje_generate_source_of_program(obj, (char *)l->data, f);
	fprintf(f, I2 "}\n");
	edje_edit_string_list_free(ll);
     }
   
   
   fprintf(f, "   }\n");//group
   
   
   //TODO Free the Evas_Object *obj
}

static const char* //return the name of the temp file containing the edc
_edje_generate_source(Evas_Object *obj)
{
   printf("\n****** GENERATE EDC SOURCE *********\n");
   char tmpn[PATH_MAX];
   int fd;
   FILE *f;


   Evas_List *l, *ll;

   GET_ED_OR_RETURN();
   
   /* Open a temp file */
   //TODO this will not work on windows
   strcpy(tmpn, "/tmp/edje_edit.edc-tmp-XXXXXX");
   if (!(fd = mkstemp(tmpn))) return NULL;
   printf("*** tmp file: %s\n", tmpn);
   if (!(f = fopen(tmpn, "w"))) return NULL;

   /* Write edc into file */
   //TODO Probably we need to save the file before generation
   
   /* Images */
   if (ll = edje_edit_images_list_get(obj))
     {
	fprintf(f, I0"images {\n");
	for (l = ll; l; l = l->next)
	  {
		char *entry = l->data;  // Name
		int comp = edje_edit_image_compression_type_get(obj, entry);
		if (comp < 0) continue;

		fprintf(f, I1"image: \"%s\" ", entry);

		if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY)
		  fprintf(f, "LOSSY %d;\n",
		          edje_edit_image_compression_rate_get(obj, entry));
		else if (comp == EDJE_EDIT_IMAGE_COMP_RAW)
		  fprintf(f, "RAW;\n");
		else if (comp == EDJE_EDIT_IMAGE_COMP_USER)
		  fprintf(f, "USER;\n");
		else fprintf(f, "COMP;\n");
	  }
	fprintf(f, I0"}\n\n");
	edje_edit_string_list_free(ll);
     }

   /* Fonts */
   if (ll = edje_edit_fonts_list_get(obj))
     {
	fprintf(f, I0"fonts {\n");

	for (l = ll; l; l = l->next)
	  {
		char *entry = l->data;
		// TODO Fixme the filename is wrong
		fprintf(f, I1"font: \"%s.ttf\" \"%s\";\n", entry, entry);
	  }
	fprintf(f, I0"}\n\n");
	edje_edit_string_list_free(ll);
     }

   /* Data */
   if (ll = edje_edit_data_list_get(obj))
     {
	fprintf(f, I0 "data {\n");

	for (l = ll; l; l = l->next)
	  {
		fprintf(f, I1 "item: \"%s\" \"%s\";\n", (char *)l->data,
			edje_edit_data_value_get(obj, (char *)l->data));
	  }

	fprintf(f, I0 "}\n\n");
	edje_edit_string_list_free(ll);
     }

   /* Color Classes */
   if (ll = edje_edit_color_classes_list_get(obj))
     {
	fprintf(f, I0 "color_classes {\n");
	for (l = ll; l; l = l->next)
	  _edje_generate_source_of_colorclass(ed, (char *)l->data, f);
	fprintf(f, I0 "}\n\n");
	edje_edit_string_list_free(ll);
     }
   
   /* Spectrum */
   if (ll = edje_edit_spectrum_list_get(obj))
     {
	fprintf(f, I0 "spectra {\n");

	for (l = ll; l; l = l->next)
	  _edje_generate_source_of_spectra(ed, (char *)l->data, f);

	fprintf(f, I0 "}\n\n");
	edje_edit_string_list_free(ll);
     }

   /* Styles */
   //TODO Support styles

   /* Collections */
   fprintf(f, "collections {\n");
   ll = edje_file_collection_list(ed->file->path);
   for (l = ll; l; l = l->next)
     _edje_generate_source_of_group(ed, (char*)l->data, f);
   fprintf(f, "}\n\n");
   edje_file_collection_list_free(ll);

   fclose(f);

   return evas_stringshare_add(tmpn);
}



/*********************/
/*  SAVING ROUTINES  */
/*********************/
////////////////////////////////////////
static char *
_edje_edit_str_direct_alloc(const char *str)
{
   return (char *)str;
}

static void
_edje_edit_str_direct_free(const char *str)
{
}

static Eet_Data_Descriptor *_srcfile_edd = NULL;
static Eet_Data_Descriptor *_srcfile_list_edd = NULL;

void
source_edd(void)
{
   Eet_Data_Descriptor_Class eddc;

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.mem_alloc = NULL;
   eddc.func.mem_free = NULL;
   eddc.func.str_alloc = evas_stringshare_add;
   eddc.func.str_free = evas_stringshare_del;
   eddc.func.list_next = evas_list_next;
   eddc.func.list_append = evas_list_append;
   eddc.func.list_data = evas_list_data;
   eddc.func.list_free = evas_list_free;
   eddc.func.hash_foreach = evas_hash_foreach;
   eddc.func.hash_add = evas_hash_add;
   eddc.func.hash_free = evas_hash_free;
   eddc.func.str_direct_alloc = _edje_edit_str_direct_alloc;
   eddc.func.str_direct_free = _edje_edit_str_direct_free;

   eddc.name = "srcfile"; 
   eddc.size = sizeof(SrcFile);
   _srcfile_edd = eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "name", name, EET_T_INLINED_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "file", file, EET_T_INLINED_STRING);

   eddc.name = "srcfile_list"; 
   eddc.size = sizeof(SrcFile_List);
   _srcfile_list_edd = eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_srcfile_list_edd, SrcFile_List, "list", list, _srcfile_edd);
}
/////////////////////////////////////////

EAPI int
edje_edit_save(Evas_Object *obj)
{
   char buf[256];
   Edje_File *ef;
   Eet_File *eetf;
   int bytes;

   GET_ED_OR_RETURN(0);

   ef = ed->file;
   if (!ef) return 0;

   printf("***********  Saving file ******************\n");
   printf("** path: %s\n", ef->path);
   
   /* Set compiler name */
   if (strcmp(ef->compiler, "edje_edit"))
     {
	_edje_if_string_free(ed, ef->compiler);
	ef->compiler = strdup("edje_edit");
     }

   /* Open the eet file */
   eetf = eet_open(ef->path, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     {
	fprintf(stderr, "Error. unable to open \"%s\" for writing output\n",
	        ef->path);
	return 0;
     }

   /* Write Edje_File structure */
   printf("** Writing Edje_File* ed->file\n");
   bytes = eet_data_write(eetf, _edje_edd_edje_file, "edje_file", ef, 1);
   if (bytes <= 0)
     {
	fprintf(stderr, "Error. unable to write \"edje_file\" "
	        "entry to \"%s\" \n", ef->path);
	eet_close(eetf);
	return 0;
     }

   /* Write all the collections */
   if (ed->collection)
     {
	printf("** Writing Edje_Part_Collection* ed->collection "
	       "[id: %d]\n", ed->collection->id);

	snprintf(buf, sizeof(buf), "collections/%i", ed->collection->id);

	bytes = eet_data_write(eetf, _edje_edd_edje_part_collection,
	                       buf, ed->collection, 1);
	if (bytes <= 0)
	  {
		fprintf(stderr, "Error. unable to write \"%s\" part entry to %s \n",
		        buf, ef->path);
		eet_close(eetf);
		return 0;
	  }
     }

   /* Write the new edc source */
   SrcFile *sf;
   SrcFile_List *sfl;
   const char *source_file;
   FILE *f;
   long sz;

   source_file = _edje_generate_source(obj);
   if (!source_file)
     {
	fprintf(stderr, "Error: can't create edc source\n");
	eet_close(eetf);
	return 0;
     }
   printf("** Writing EDC Source [from: %s]\n", source_file);

   //open the temp file and put the contents in SrcFile
   sf = mem_alloc(SZ(SrcFile));
   sf->name = mem_strdup("generated_source.edc");

   f = fopen(source_file, "rb");
   if (!f)
     {
	fprintf(stderr, "Error. unable to read the created edc source [%s]\n",
	        source_file);
	eet_close(eetf);
	return 0;
     }

   fseek(f, 0, SEEK_END);
   sz = ftell(f);
   fseek(f, 0, SEEK_SET);

   sf->file = mem_alloc(sz + 1);
   fread(sf->file, sz, 1, f);
   sf->file[sz] = '\0';
   fseek(f, 0, SEEK_SET);
   fclose(f);

   //create the needed list of source files (only one)
   sfl = mem_alloc(SZ(SrcFile_List));
   sfl->list = NULL;
   sfl->list = evas_list_append(sfl->list, sf);

   // write the sources list to the eet file
   source_edd();
   bytes = eet_data_write(eetf, _srcfile_list_edd, "edje_sources", sfl, 1);
   if (bytes <= 0)
    {
	fprintf(stderr, "Error. unable to write edc source\n");
	eet_close(eetf);
	return 0;
    }

   /* Clear stuff */
   unlink(source_file);
   evas_stringshare_del(source_file);
   eet_close(eetf);
   printf("***********  Saving DONE ******************\n");
   return 1;
}

EAPI void
edje_edit_print_internal_status(Evas_Object *obj)
{
   Evas_List *l;

   GET_ED_OR_RETURN();

   _edje_generate_source(obj);
   return;
   
   printf("\n****** CHECKIN' INTERNAL STRUCTS STATUS *********\n");

   printf("*** Edje\n");
   printf("    path: '%s'\n", ed->path);
   printf("    group: '%s'\n", ed->group);
   printf("    parent: '%s'\n", ed->parent);

   printf("\n*** Parts [table:%d list:%d]\n", ed->table_parts_size,
          evas_list_count(ed->collection->parts));
   for (l = ed->collection->parts; l; l = l->next)
     {
	Edje_Part *p;
	Edje_Real_Part *rp;

	p = l->data;
	rp = ed->table_parts[p->id % ed->table_parts_size];
	printf("    [%d]%s ", p->id, p->name);
	if (p == rp->part)
	  printf(" OK!\n");
	else
	  printf(" WRONG (table[%id]->name = '%s')\n", p->id, rp->part->name);
     }

   printf("\n*** Programs [table:%d list:%d]\n", ed->table_programs_size,
          evas_list_count(ed->collection->programs));
   for (l = ed->collection->programs; l; l = l->next)
     {
	Edje_Program *epr;
	Edje_Program *epr2;

	epr = l->data;
	epr2 = ed->table_programs[epr->id % ed->table_programs_size];
	printf("     [%d]%s ", epr->id, epr->name);
	if (epr == epr2)
	  printf(" OK!\n");
	else
	  printf(" WRONG (table[%id]->name = '%s')\n", epr->id, epr2->name);
     }

   printf("\n");

   printf("******************  END  ************************\n\n");
}
