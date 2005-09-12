/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
#include "edje_private.h"

static Evas_Hash   *_edje_file_hash = NULL;
static int          _edje_file_cache_size = 16;
static Evas_List   *_edje_file_cache = NULL;
static int          _edje_collection_cache_size = 16;

static Edje_File *_edje_file_cache_find(char *path);
static void _edje_file_cache_clean(void);
static void _edje_file_cache_add(Edje_File *edf);
static void _edje_file_unref(Edje_File *edf);
static void _edje_file_ref(Edje_File *edf);
static void _edje_file_cleanup(void);
static Edje_Part_Collection *_edje_collection_find(Edje_File *edf, char *part);
static void _edje_collection_cache_clean(Edje_File *edf);
static void _edje_collection_cache_add(Edje_File *edf, Edje_Part_Collection *coll);
static void _edje_collection_unref(Edje_File *edf, Edje_Part_Collection *coll);
static void _edje_collection_cleanup(Edje_File *edf);

static void _edje_collection_free_part_description_free(Edje_Part_Description *desc);
static Evas_Bool _edje_file_collection_hash_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata);
#ifdef EDJE_PROGRAM_CACHE
static int  _edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
#endif

/************************** API Routines **************************/

/* FIXDOC: Verify/expand doc */
/** Sets the EET file to be used
 * @param obj A valid Evas_Object handle
 * @param file The path to the EET file
 * @param part The group name in the Edje
 * @return 0 on Error\n
 * 1 on Success and sets EDJE_LOAD_ERROR_NONE
 *
 * This loads the EET file and sets up the Edje.
 */
int
edje_object_file_set(Evas_Object *obj, const char *file, const char *part)
{
   Edje *ed;
   int n;
   
   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (!file) file = "";
   if (!part) part = "";
   if (((ed->path) && (!strcmp(file, ed->path))) &&
	(ed->part) && (!strcmp(part, ed->part)))
     return 1;
   
   _edje_file_del(ed);
   
   if (ed->path) free(ed->path);
   ed->path = strdup(file);
   if (ed->part) free(ed->part);
   ed->part = strdup(part);
   
   ed->load_error = EDJE_LOAD_ERROR_NONE;
   _edje_file_add(ed);

   if (ed->collection)
     {
	Evas_List *l;
	int errors = 0;
	
	/* check for invalid loops */
	for (l = ed->collection->parts; (l && ! errors); l = l->next)
	  {
	     Edje_Part *ep;
	     Evas_List *hist = NULL;

	     /* Register any color classes in this parts descriptions. */
	     ep = l->data;
	     if ((ep->default_desc) && (ep->default_desc->color_class)) _edje_color_class_member_add(ed, ep->default_desc->color_class);
	     for (hist = ep->other_desc; hist; hist = hist->next)
	       {
		  Edje_Part_Description *desc;

		  desc = hist->data;
		  if (desc->color_class) _edje_color_class_member_add(ed, desc->color_class);
	       }
	     hist = NULL;
	     hist = evas_list_append(hist, ep);
	     while (ep->dragable.confine_id >= 0)
	       {
		  ep = evas_list_nth(ed->collection->parts,
				     ep->dragable.confine_id);
		  if (evas_list_find(hist, ep))
		    {
		       printf("EDJE ERROR: confine_to loops. invalidating loop.\n");
		       ep->dragable.confine_id = -1;
		       break;
		    }
		  hist = evas_list_append(hist, ep);
	       }
	     evas_list_free(hist);
	     hist = NULL;
	     hist = evas_list_append(hist, ep);
	     while (ep->dragable.events_id >= 0)
	       {
		  Edje_Part* prev;

		  prev = ep;

		  ep = evas_list_nth(ed->collection->parts,
				     ep->dragable.events_id);
		  
		  if (!ep->dragable.x && !ep->dragable.y)
		    {
		       prev->dragable.events_id = -1;
		       break;
		    }

		  if (evas_list_find(hist, ep))
		    {
		       printf("EDJE ERROR: events_to loops. invalidating loop.\n");
		       ep->dragable.events_id = -1;
		       break;
		    }
		  hist = evas_list_append(hist, ep);
	       }
	     evas_list_free(hist);
	     hist = NULL;
	     hist = evas_list_append(hist, ep);
	     while (ep->clip_to_id >= 0)
	       {
		  ep = evas_list_nth(ed->collection->parts,
				     ep->clip_to_id);
		  if (evas_list_find(hist, ep))
		    {
		       printf("EDJE ERROR: clip_to loops. invalidating loop.\n");
		       ep->clip_to_id = -1;
		       break;
		    }
		  hist = evas_list_append(hist, ep);
	       }
	     evas_list_free(hist);
	     hist = NULL;
	  }
	/* build real parts */
	for (n = 0, l = ed->collection->parts; l; l = l->next, n++)
	  {
	     Edje_Part *ep;
	     Edje_Real_Part *rp;
	     
	     ep = l->data;
	     rp = calloc(1, sizeof(Edje_Real_Part));
	     if (!rp) return 0;
	     rp->part = ep;
	     ed->parts = evas_list_append(ed->parts, rp);
	     rp->param1.description = ep->default_desc;
	     rp->chosen_description = rp->param1.description;
	     if (!rp->param1.description)
	       {
		  printf("EDJE ERROR: no default part description!\n");
	       }
	     _edje_text_part_on_add(ed, rp);
	     if (ep->type == EDJE_PART_TYPE_RECTANGLE)
	       rp->object = evas_object_rectangle_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_IMAGE)
	       rp->object = evas_object_image_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_TEXT)
	       {
		  rp->object = evas_object_text_add(ed->evas);
		  evas_object_text_font_source_set(rp->object, ed->path);
	       }
	     else if (ep->type == EDJE_PART_TYPE_SWALLOW)
	       {
		  rp->object = evas_object_rectangle_add(ed->evas);
		  evas_object_color_set(rp->object, 0, 0, 0, 0);
		  evas_object_pass_events_set(rp->object, 1);
	       }
	     else if (ep->type == EDJE_PART_TYPE_TEXTBLOCK)
	       rp->object = evas_object_textblock2_add(ed->evas);
	     else
	       {
		  printf("EDJE ERROR: wrong part type %i!\n", ep->type);
	       }
	     if (rp->object)
	       {
		  evas_object_smart_member_add(rp->object, ed->obj);
		  evas_object_layer_set(rp->object, evas_object_layer_get(ed->obj));
		  if (ep->type != EDJE_PART_TYPE_SWALLOW)
		    {
		       if (ep->mouse_events)
			 {
			    evas_object_event_callback_add(rp->object, 
							   EVAS_CALLBACK_MOUSE_IN,
							   _edje_mouse_in_cb,
							   ed);
			    evas_object_event_callback_add(rp->object, 
							   EVAS_CALLBACK_MOUSE_OUT,
							   _edje_mouse_out_cb,
							   ed);
			    evas_object_event_callback_add(rp->object, 
							   EVAS_CALLBACK_MOUSE_DOWN,
							   _edje_mouse_down_cb,
							   ed);
			    evas_object_event_callback_add(rp->object, 
							   EVAS_CALLBACK_MOUSE_UP,
							   _edje_mouse_up_cb,
							   ed);
			    evas_object_event_callback_add(rp->object, 
							   EVAS_CALLBACK_MOUSE_MOVE,
							   _edje_mouse_move_cb,
							   ed);
			    evas_object_event_callback_add(rp->object, 
							   EVAS_CALLBACK_MOUSE_WHEEL,
							   _edje_mouse_wheel_cb,
							   ed);
			    evas_object_data_set(rp->object, "real_part", rp);
			    if (ep->repeat_events)
			      evas_object_repeat_events_set(rp->object, 1);
			 }
		       else
			 evas_object_pass_events_set(rp->object, 1);
		    }
		  if (rp->part->clip_to_id < 0)
		    evas_object_clip_set(rp->object, ed->clipper);
	       }
	     rp->drag.step.x = ep->dragable.step_x;
	     rp->drag.step.y = ep->dragable.step_y;
	  }
	if (n > 0)
	  {
	     ed->table_parts = malloc(sizeof(Edje_Real_Part *) * n);
	     ed->table_parts_size = n;
	     /* FIXME: check malloc return */
	     n = 0;
	     for (l = ed->parts; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  
		  rp = l->data;
		  ed->table_parts[n] = rp;
		  n++;
		  if (rp->param1.description->rel1.id_x >= 0)
		    rp->param1.rel1_to_x = evas_list_nth(ed->parts, rp->param1.description->rel1.id_x);
		  if (rp->param1.description->rel1.id_y >= 0)
		    rp->param1.rel1_to_y = evas_list_nth(ed->parts, rp->param1.description->rel1.id_y);
		  if (rp->param1.description->rel2.id_x >= 0)
		    rp->param1.rel2_to_x = evas_list_nth(ed->parts, rp->param1.description->rel2.id_x);
		  if (rp->param1.description->rel2.id_y >= 0)
		    rp->param1.rel2_to_y = evas_list_nth(ed->parts, rp->param1.description->rel2.id_y);
		  _edje_text_part_on_add_clippers(ed, rp);
		  if (rp->part->clip_to_id >= 0)
		    {
		       rp->clip_to = evas_list_nth(ed->parts, rp->part->clip_to_id);
		       if (rp->clip_to)
			 {
			    evas_object_pass_events_set(rp->clip_to->object, 1);
			    evas_object_clip_set(rp->object, rp->clip_to->object);
			 }
		    }
		  if (rp->part->dragable.confine_id >= 0)
		    rp->confine_to = evas_list_nth(ed->parts, rp->part->dragable.confine_id);
		  
		  /* replay events for dragable */
		  if (rp->part->dragable.events_id >= 0)
		    {
		       rp->events_to = 
			  evas_list_nth(ed->parts,
				rp->part->dragable.events_id);
		       /* events_to may be used only with dragable */
		       if (!rp->events_to->part->dragable.x &&
			   !rp->events_to->part->dragable.y)
			 rp->events_to = NULL;
		    }

		  rp->swallow_params.min.w = 0;
		  rp->swallow_params.min.w = 0;
		  rp->swallow_params.max.w = -1;
		  rp->swallow_params.max.h = -1;
		  
		  if (ed->file->feature_ver < 1)
		    {
		       rp->param1.description->text.id_source = -1;
		       rp->param1.description->text.id_text_source = -1;
		    }
		  if (rp->param1.description->text.id_source >= 0)
		    rp->text.source = evas_list_nth(ed->parts, rp->param1.description->text.id_source);
		  if (rp->param1.description->text.id_text_source >= 0)
		    rp->text.text_source = evas_list_nth(ed->parts, rp->param1.description->text.id_text_source);
	       }
	  }
	n = evas_list_count(ed->collection->programs);
	if (n > 0)
	  {
	     ed->table_programs = malloc(sizeof(Edje_Program *) * n);
	     ed->table_programs_size = n;
	     /* FIXME: check malloc return */
	     n = 0;
	     for (l = ed->collection->programs; l; l = l->next)
	       {
		  Edje_Program *pr;
		  
		  pr = l->data;
		  ed->table_programs[n] = pr;
		  n++;
	       }
	  }
	_edje_ref(ed);
	_edje_block(ed);
	_edje_freeze(ed);
	if (ed->collection->script) _edje_embryo_script_init(ed);
	_edje_var_init(ed);
	for (l = ed->parts; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     
	     rp = l->data;
	     evas_object_show(rp->object);
	     if (_edje_block_break(ed)) break;
	     if (rp->part->dragable.x < 0) rp->drag.val.x = 1.0;
	     if (rp->part->dragable.y < 0) rp->drag.val.x = 1.0;
	     _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
	  }
	ed->dirty = 1;
	if ((evas_object_clipees_get(ed->clipper)) && 
	    (evas_object_visible_get(obj)))
	  evas_object_show(ed->clipper);
	_edje_recalc(ed);
	_edje_thaw(ed);
	_edje_unblock(ed);
	_edje_unref(ed);
	ed->load_error = EDJE_LOAD_ERROR_NONE;
	_edje_emit(ed, "load", "");
	return 1;
     }
   else
     {
	return 0;
     }
   ed->load_error = EDJE_LOAD_ERROR_NONE;
   return 1;
}

/* FIXDOC: Verify/expand doc. */
/** Get the EET location and group for the Evas Object. ?! Assuming eet file
 * @param obj A valid Evas_Object handle
 * @param file The EET file location pointer
 * @param part The EET part pointer
 *
 * This gets the EET file location and group for the given Evas_Object.
 */
void
edje_object_file_get(Evas_Object *obj, const char **file, const char **part)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed)
     {
	if (file) *file = NULL;
	if (part) *part = NULL;
	return;
     }
   if (file) *file = ed->path;
   if (part) *part = ed->part;
}

/* FIXDOC: Verify. return error? */
/** Gets the Edje load error
 * @param obj A valid Evas_Object handle
 *
 * @return The Edje load error:\n
 * 0: No Error\n
 * 1: Generic Error\n
 * 2: Does not Exist\n
 * 3: Permission Denied\n
 * 4: Resource Allocation Failed\n
 * 5: Corrupt File\n
 * 6: Unknown Format\n
 * 7: Incompatible File
 */
int
edje_object_load_error_get(Evas_Object *obj)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return EDJE_LOAD_ERROR_NONE;
   return ed->load_error;
}

/* FIXDOC: Verify/expand */
/** Get the collection list from the edje file ?
 * @param file The file path?
 *
 * @return The Evas_List of files
 */   
Evas_List *
edje_file_collection_list(const char *file)
{
   Eet_File *ef = NULL;
   Evas_List *lst = NULL;
   Edje_File *ed_file;

   if (!file || !*file) return NULL;

   ed_file = evas_hash_find(_edje_file_hash, file);
   if (!ed_file) ed_file = _edje_file_cache_find((char *)file);
   if (!ed_file)
     {
	ef = eet_open((char *)file, EET_FILE_MODE_READ);
	if (!ef) return NULL;
	ed_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	if (!ed_file)
	  {
	     eet_close(ef);
	     return NULL;
	  }
	eet_close(ef);
	ed_file->path = strdup(file);
	ed_file->collection_hash = NULL;
	ed_file->references = 1;
	_edje_file_hash = evas_hash_add(_edje_file_hash, ed_file->path, ed_file);
     }
   if (ed_file->collection_dir)
     {
	Evas_List *l;
	
	for (l = ed_file->collection_dir->entries; l; l = l->next)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;
	     
	     ce = l->data;
	     lst = evas_list_append(lst, strdup(ce->entry));
	  }
     }
   if (ed_file->references > 0) _edje_file_unref(ed_file);
   else _edje_file_free(ed_file);
   return lst;
}

/* FIXDOC: Verify/Expand */
/** Free file collection
 * @param lst The Evas_List of files
 *
 * Frees the file collection.
 */
void
edje_file_collection_list_free(Evas_List *lst)
{
   while (lst)
     {
	if (lst->data) free(lst->data);
	lst = evas_list_remove(lst, lst->data);
     }
}

/* FIXDOC: Verify/Expand */
/** Get edje file data
 * @param file The file
 * @param key The data key
 * @return The file data string
 */
char *
edje_file_data_get(const char *file, const char *key)
{
   Eet_File *ef = NULL;
   Edje_File *ed_file;
   Evas_List *l;
   char *str = NULL;
   
   ed_file = evas_hash_find(_edje_file_hash, file);
   if (!ed_file) ed_file = _edje_file_cache_find((char *)file);
   if (!ed_file)
     {
	ef = eet_open((char *)file, EET_FILE_MODE_READ);
	if (!ef) return NULL;
	ed_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	if (!ed_file)
	  {
	     eet_close(ef);
	     return NULL;
	  }
	eet_close(ef);
	ed_file->path = strdup(file);
	ed_file->collection_hash = NULL;
	ed_file->references = 1;
	_edje_file_hash = evas_hash_add(_edje_file_hash, ed_file->path, ed_file);
     }
   for (l = ed_file->data; l; l = l->next)
     {
	Edje_Data *di;
	
	di = l->data;
	if (!strcmp(di->key, key))
	  {
	     str = strdup(di->value);
	     break;
	  }
     }
   if (ed_file->references > 0) _edje_file_unref(ed_file);
   else _edje_file_free(ed_file);
   return str;
}

void
edje_file_cache_set(int count)
{
   if (count < 0) count = 0;
   _edje_file_cache_size = count;
   _edje_file_cache_clean();
}

int
edje_file_cache_get(void)
{
   return _edje_file_cache_size;
}

void
edje_file_cache_flush(void)
{
   int ps;
   
   ps = _edje_file_cache_size;
   _edje_file_cache_size = 0;
   _edje_file_cache_clean();
   _edje_file_cache_size = ps;
}

void
edje_collection_cache_set(int count)
{
   Evas_List *l;
   
   if (count < 0) count = 0;
   _edje_collection_cache_size = count;
   for (l = _edje_file_cache; l; l = l->next)
     {
	Edje_File *edf;
	
	edf = l->data;
	_edje_collection_cache_clean(edf);
     }
}

int
edje_collection_cache_get(void)
{
   return _edje_collection_cache_size;
}

void
edje_collection_cache_flush(void)
{
   int ps;
   Evas_List *l;
   
   ps = _edje_collection_cache_size;
   _edje_collection_cache_size = 0;
   for (l = _edje_file_cache; l; l = l->next)
     {
	Edje_File *edf;
	
	edf = l->data;
	_edje_collection_cache_clean(edf);
     }
   _edje_collection_cache_size = ps;
}
    

static Edje_File *
_edje_file_cache_find(char *path)
{
   Evas_List *l;
   
   for (l = _edje_file_cache; l; l = l->next)
     {
	Edje_File *edf;
	
	edf = l->data;
	if (!strcmp(edf->path, path))
	  {
	     edf->references = 1;
	     _edje_file_cache = evas_list_remove_list(_edje_file_cache, l);
	     _edje_file_hash = evas_hash_add(_edje_file_hash, path, edf);
	     return edf;
	  }
     }
   return NULL;
}

static void
_edje_file_cache_clean(void)
{
   int count;
   
   count = evas_list_count(_edje_file_cache);
   while ((_edje_file_cache) && (count > _edje_file_cache_size))
     {
	Edje_File *edf;
	
	edf = evas_list_last(_edje_file_cache)->data;
	_edje_file_cache = evas_list_remove_list(_edje_file_cache, evas_list_last(_edje_file_cache));
	_edje_file_free(edf);
	count = evas_list_count(_edje_file_cache);
     }
}

static void
_edje_file_cache_add(Edje_File *edf)
{
   _edje_file_hash = evas_hash_del(_edje_file_hash, edf->path, edf);
   _edje_file_cache  = evas_list_prepend(_edje_file_cache, edf);
   _edje_file_cache_clean();
}

static void
_edje_file_unref(Edje_File *edf)
{
   edf->references--;
   if (edf->references == 0) _edje_file_cache_add(edf);
}

static void
_edje_file_ref(Edje_File *edf)
{
   if (edf->references == 0)
     {
	_edje_file_cache = evas_list_remove(_edje_file_cache, edf);
	_edje_file_hash = evas_hash_add(_edje_file_hash, edf->path, edf);
     }
   edf->references++;
}

static void
_edje_file_cleanup(void)
{
   int ps;
   
   ps = _edje_file_cache_size;
   _edje_file_cache_size = 0;
   _edje_file_cache_clean();
   _edje_file_cache_size = ps;
}




static Edje_Part_Collection *
_edje_collection_find(Edje_File *edf, char *part)
{
   Evas_List *l;
   Edje_Part_Collection *coll = NULL;
   
   coll = evas_hash_find(edf->collection_hash, part);
   if (coll)
     {
	coll->references++;
	return coll;
     }
   for (l = edf->collection_cache; l; l = l->next)
     {
	coll = l->data;

	if (!strcmp(coll->part, part))
	  {
	     coll->references = 1;
	     edf->collection_cache = evas_list_remove_list(edf->collection_cache, l);
	     edf->collection_hash = evas_hash_add(edf->collection_hash, part, coll);
	     return coll;
	  }
     }
   return NULL;
}

static void
_edje_collection_cache_clean(Edje_File *edf)
{
   int count;
   
   count = evas_list_count(edf->collection_cache);
   while ((edf->collection_cache) && (count > _edje_collection_cache_size))
     {
	Edje_Part_Collection *coll;
	
	coll = evas_list_last(edf->collection_cache)->data;
	edf->collection_cache = evas_list_remove_list(edf->collection_cache, evas_list_last(edf->collection_cache));
	_edje_collection_free(edf, coll);
	count = evas_list_count(edf->collection_cache);
     }
}

static void
_edje_collection_cache_add(Edje_File *edf, Edje_Part_Collection *coll)
{
   edf->collection_hash = evas_hash_del(edf->collection_hash, coll->part, coll);
   edf->collection_cache = evas_list_prepend(edf->collection_cache, coll);
   _edje_collection_cache_clean(edf);
}

static void
_edje_collection_unref(Edje_File *edf, Edje_Part_Collection *coll)
{
   coll->references--;
   if (coll->references == 0) _edje_collection_cache_add(edf, coll);
}

static void
_edje_collection_cleanup(Edje_File *edf)
{
   int ps;
   
   ps = _edje_collection_cache_size;
   _edje_collection_cache_size = 0;
   _edje_collection_cache_clean(edf);
   _edje_collection_cache_size = ps;
}

void
_edje_file_add(Edje *ed)
{
   Eet_File *ef = NULL;
   Evas_List *l, *ll;
   int id = -1;

   if (_edje_edd_edje_file == NULL)
     return;

   ed->file = evas_hash_find(_edje_file_hash, ed->path);
   if (ed->file)
     _edje_file_ref(ed->file);
   else
     {
	ed->file = _edje_file_cache_find(ed->path);
	if (!ed->file)
	  {
	     ef = eet_open(ed->path, EET_FILE_MODE_READ);
	     if (!ef)
	       {
		  ed->load_error = EDJE_LOAD_ERROR_UNKNOWN_FORMAT;
		  return;
	       }
	     
	     ed->file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	     if (!ed->file)
	       {
		  ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
		  goto out;
	       }
	     
	     if (ed->file->version != EDJE_FILE_VERSION)
	       {
		  _edje_file_free(ed->file);
		  ed->file = NULL;
		  ed->load_error = EDJE_LOAD_ERROR_INCOMPATIBLE_FILE;
		  goto out;
	       }
	     
	     ed->file->references = 1;   
	     ed->file->path = strdup(ed->path);
	     if (!ed->file->collection_dir)
	       {
		  _edje_file_free(ed->file);
		  ed->file = NULL;
		  ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
		  goto out;
	       }
	     _edje_file_hash = evas_hash_add(_edje_file_hash, ed->file->path, ed->file);
	  }
     }

   for (l = ed->file->styles; l; l = l->next)
     {
	Edje_Style *stl;
	Edje_Style_Tag *tag;
	char *buf = NULL;
	int len = 0;
	int def_done;
	
	stl = l->data;
	if (stl->style) break;
	stl->style = evas_textblock2_style_new();
	evas_textblock2_style_set(stl->style, (const char *)buf);
	for (ll = stl->tags; ll; ll = ll->next)
	  {
	     tag = ll->data;
	     len += strlen(tag->key);
	     len += 1;
	     len += 1;
	     len += strlen(tag->value);
	     len += 1;
	  }
	if (_edje_fontset_append)
	  {
	     len += 1 + strlen("font_fallbacks=") + strlen(_edje_fontset_append);
	  }
	len += 1 + strlen("font_source=") + strlen(ed->path);
	buf = malloc(len + 1);
	buf[0] = 0;
	for (ll = stl->tags; ll; ll = ll->next)
	  {
	     tag = ll->data;
	     strcat(buf, tag->key);
	     strcat(buf, "='");
	     strcat(buf, tag->value);
	     if ((!def_done) && (!strcmp(tag->key, "DEFAULT")))
	       {
		  if (_edje_fontset_append)
		    {
		       strcat(buf, " ");
		       strcat(buf, "font_fallbacks=");
		       strcat(buf, _edje_fontset_append);
		    }
		  strcat(buf, " ");
		  strcat(buf, "font_source=");
		  strcat(buf, ed->path);
		  def_done = 1;
	       }
	     strcat(buf, "'");
	  }
	evas_textblock2_style_set(stl->style, buf);
	free(buf);
     }
   
   ed->collection = _edje_collection_find(ed->file, ed->part);
   if (!ed->collection)
     {
	for (l = ed->file->collection_dir->entries; l; l = l->next)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;
	     
	     ce = l->data;
	     if ((ce->entry) && (!strcmp(ce->entry, ed->part)))
	       {
		  id = ce->id;
		  break;
	       }
	  }
	if (id >= 0)
	  {
	     char buf[256];
	     int  size;
	     void *data;
	     
	     snprintf(buf, sizeof(buf), "collections/%i", id);
	     if (!ef) ef = eet_open(ed->path, EET_FILE_MODE_READ);
	     if (!ef)
	       {
		  _edje_file_unref(ed->file);
		  ed->file = NULL;
		  ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
		  goto out;
	       }
	     ed->collection = eet_data_read(ef, 
					    _edje_edd_edje_part_collection, 
					    buf);
	     if (!ed->collection)
	       {
		  _edje_file_unref(ed->file);
		  ed->file = NULL;
		  ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;
		  goto out;
	       }
//	     printf("OPEN COLL %s\n", ed->part);
	     
	     snprintf(buf, sizeof(buf), "scripts/%i", id);
	     data = eet_read(ef, buf, &size);
	     if (data)
	       {
		  ed->collection->script = embryo_program_new(data, size);
		  free(data);
	       }
	     
	     ed->collection->part = strdup(ed->part);
	     ed->collection->references = 1;
	     ed->file->collection_hash = evas_hash_add(ed->file->collection_hash, ed->part, ed->collection);
	  }
	else
	  {
	     _edje_file_unref(ed->file);
	     ed->file = NULL;
	     ed->load_error = EDJE_LOAD_ERROR_CORRUPT_FILE;	     
	  }
     }
   out:
   if (ef) eet_close(ef);
}

void
_edje_file_del(Edje *ed)
{
   _edje_message_del(ed);
   _edje_block_violate(ed);
   _edje_var_shutdown(ed);
   if (!((ed->file) && (ed->collection))) return;
   if ((ed->file) && (ed->collection))
     {
	Evas_List *l;
	
	for (l = ed->collection->parts; l; l = l->next)
	  {
	     Edje_Part *ep;
	     
	     ep = l->data;
	     _edje_text_part_on_del(ed, ep);
	     _edje_color_class_on_del(ed, ep);
	  }
	_edje_collection_unref(ed->file, ed->collection);
	ed->collection = NULL;
     }
   if (ed->file)
     {
	_edje_file_unref(ed->file);
	ed->file = NULL;
     }
   if (ed->parts)
     {
	while (ed->parts)
	  {
	     Edje_Real_Part *rp;

	     rp = ed->parts->data;
	     ed->parts = evas_list_remove(ed->parts, rp);
	     if (rp->object)
	       {
		  evas_object_event_callback_del(rp->object, 
						 EVAS_CALLBACK_MOUSE_IN,
						 _edje_mouse_in_cb);
		  evas_object_event_callback_del(rp->object, 
						 EVAS_CALLBACK_MOUSE_OUT,
						 _edje_mouse_out_cb);
		  evas_object_event_callback_del(rp->object, 
						 EVAS_CALLBACK_MOUSE_DOWN,
					    _edje_mouse_down_cb);
		  evas_object_event_callback_del(rp->object, 
						 EVAS_CALLBACK_MOUSE_UP,
						 _edje_mouse_up_cb);
		  evas_object_event_callback_del(rp->object, 
						 EVAS_CALLBACK_MOUSE_MOVE,
						 _edje_mouse_move_cb);
		  evas_object_event_callback_del(rp->object, 
						 EVAS_CALLBACK_MOUSE_WHEEL,
						 _edje_mouse_wheel_cb);
		  _edje_text_real_part_on_del(ed, rp);
		  evas_object_del(rp->object);
	       }
	     if (rp->swallowed_object)
	       {
		  evas_object_smart_member_del(rp->swallowed_object);
		  evas_object_event_callback_del(rp->swallowed_object,
						 EVAS_CALLBACK_FREE,
						 _edje_object_part_swallow_free_cb);
		  evas_object_clip_unset(rp->swallowed_object);
		  rp->swallowed_object = NULL;
/* I think it would be better swallowed objects dont get deleted */
/*		  evas_object_del(rp->swallowed_object);*/
	       }
	     if (rp->text.text) free(rp->text.text);
	     if (rp->text.font) free(rp->text.font);
	     if (rp->text.cache.in_str) free(rp->text.cache.in_str);
	     if (rp->text.cache.out_str) free(rp->text.cache.out_str);	     

	     if (rp->custom.description)
	       {
		  _edje_collection_free_part_description_free(rp->custom.description);
	       }

	     free(rp);
	  }
	ed->parts = NULL;
     }
   if (ed->actions)
     {
	while (ed->actions)
	  {
	     Edje_Running_Program *runp;
	     
	     _edje_anim_count--;
	     runp = ed->actions->data;
	     ed->actions = evas_list_remove(ed->actions, runp);
	     free(runp);
	  }
     }
   _edje_animators = evas_list_remove(_edje_animators, ed);
   if (ed->pending_actions)
     {
	while (ed->pending_actions)
	  {
	     Edje_Pending_Program *pp;
	     
	     pp = ed->pending_actions->data;
	     ed->pending_actions = evas_list_remove(ed->pending_actions, pp);
	     ecore_timer_del(pp->timer);
	     free(pp);
	  }
     }
   if (ed->table_parts) free(ed->table_parts);
   ed->table_parts = NULL;
   ed->table_parts_size = 0;
   if (ed->table_programs) free(ed->table_programs);
   ed->table_programs = NULL;
   ed->table_programs_size = 0;
}

void
_edje_file_free(Edje_File *edf)
{
   if (edf->font_dir)
     {
	while (edf->font_dir->entries)
	  {
	     Edje_Font_Directory_Entry *fe;
	     
	     fe = edf->font_dir->entries->data;
	     edf->font_dir->entries = 
	       evas_list_remove(edf->font_dir->entries, fe);
	     if (fe->entry) free(fe->entry);
	     free(fe);
	  }
	free(edf->font_dir);
     }
   if (edf->image_dir)
     {
	while (edf->image_dir->entries)
	  {
	     Edje_Image_Directory_Entry *ie;
	     
	     ie = edf->image_dir->entries->data;
	     edf->image_dir->entries = 
	       evas_list_remove(edf->image_dir->entries, ie);
	     if (ie->entry) free(ie->entry);
	     free(ie);
	  }
	free(edf->image_dir);
     }
   if (edf->collection_dir)
     {
	while (edf->collection_dir->entries)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;
	     
	     ce = edf->collection_dir->entries->data;
	     edf->collection_dir->entries = 
	       evas_list_remove(edf->collection_dir->entries, ce);
	     if (ce->entry) free(ce->entry);
	     free(ce);
	  }
	free(edf->collection_dir);
     }
   if (edf->data)
     {
	while (edf->data)
	  {
	     Edje_Data *edt;
	     
	     edt = edf->data->data;
	     edf->data = evas_list_remove(edf->data, edt);
	     if (edt->key) free(edt->key);
	     if (edt->value) free(edt->value);
	     free(edt);
	  }
     }
   /* FIXME: free collection_hash and collection_cache */
   if (edf->collection_hash)
     {
	printf("EDJE ERROR:\n"
	       "\n"
	       "Naughty Programmer - spank spank!\n"
	       "\n"
	       "This program as probably called edje_shutdown() with active Edje objects\n"
	       "still around.\n This can cause problems as both Evas and Edje retain\n"
	       "references to the objects. you should shut down all canvases and objects\n"
	       "before calling edje_shutdown().\n"
	       "The following errors are the edje object files and parts that are still\n"
	       "hanging around, with their reference counts\n");
	evas_hash_foreach(edf->collection_hash, _edje_file_collection_hash_foreach, edf);
	evas_hash_free(edf->collection_hash);
     }
   if (edf->path) free(edf->path);
   if (edf->compiler) free(edf->compiler);
   if (edf->collection_cache)
     _edje_collection_cleanup(edf);
   while (edf->styles)
     {
	Edje_Style *stl;
	
	stl == edf->styles->data;
	edf->styles = evas_list_remove_list(edf->styles, edf->styles);
	while (stl->tags)
	  {
	     Edje_Style_Tag *tag;
	     
	     tag = stl->tags->data;
	     stl->tags = evas_list_remove_list(stl->tags, stl->tags);
	     if (tag->key) free(tag->key);
	     if (tag->value) free(tag->value);
	     free(tag);
	  }
	if (stl->name) free(stl->name);
	if (stl->style) evas_textblock2_style_free(stl->style);
	free(stl);
     }
   free(edf);
}

void
_edje_file_cache_shutdown(void)
{
   _edje_file_cleanup();
}

void
_edje_collection_free(Edje_File *edf, Edje_Part_Collection *ec)
{
   while (ec->programs)
     {
	Edje_Program *pr;

	pr = ec->programs->data;
	ec->programs = evas_list_remove(ec->programs, pr);
	if (pr->name) free(pr->name);
	if (pr->signal) free(pr->signal);
	if (pr->source) free(pr->source);
	if (pr->state) free(pr->state);
	if (pr->state2) free(pr->state2);
	while (pr->targets)
	  {
	     Edje_Program_Target *prt;
	     
	     prt = pr->targets->data;
	     pr->targets = evas_list_remove(pr->targets, prt);
	     free(prt);
	  }
	while (pr->after)
	  {
	     Edje_Program_After *pa;

	     pa = pr->after->data;
	     pr->after = evas_list_remove(pr->after, pa);
		 free(pa);
	  }
	free(pr);
     }
   while (ec->parts)
     {
	Edje_Part *ep;

	ep = ec->parts->data;
	ec->parts = evas_list_remove(ec->parts, ep);
	if (ep->name) free(ep->name);
	if (ep->default_desc)
	  {
	     _edje_collection_free_part_description_free(ep->default_desc);
	     ep->default_desc = NULL;
	  }
	while (ep->other_desc)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = ep->other_desc->data;
	     ep->other_desc = evas_list_remove(ep->other_desc, desc);
	     _edje_collection_free_part_description_free(desc);
	  }
	free(ep);
     }
   if (ec->data)
     {
	while (ec->data)
	  {
	     Edje_Data *edt;
	     
	     edt = ec->data->data;
	     ec->data = evas_list_remove(ec->data, edt);
	     if (edt->key) free(edt->key);
	     if (edt->value) free(edt->value);
	     free(edt);
	  }
     }
   if (ec->part)
     free(ec->part);
#ifdef EDJE_PROGRAM_CACHE
   if (ec->prog_cache.no_matches) evas_hash_free(ec->prog_cache.no_matches);
   if (ec->prog_cache.matches)
     {
	evas_hash_foreach(ec->prog_cache.matches, 
			  _edje_collection_free_prog_cache_matches_free_cb, 
			  NULL);
	evas_hash_free(ec->prog_cache.matches);
     }
#endif   
   if (ec->script) embryo_program_free(ec->script);
   free(ec);
}

static void
_edje_collection_free_part_description_free(Edje_Part_Description *desc)
{
   if (desc->state.name) free(desc->state.name);
   while (desc->image.tween_list)
     {
	Edje_Part_Image_Id *pi;
	
	pi = desc->image.tween_list->data;
	desc->image.tween_list = evas_list_remove(desc->image.tween_list, pi);
	free(pi);
     }
   if (desc->color_class)     free(desc->color_class);
   if (desc->text.text)       free(desc->text.text);
   if (desc->text.text_class) free(desc->text.text_class);
   if (desc->text.font)       free(desc->text.font);
   free(desc);
}

static Evas_Bool
_edje_file_collection_hash_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Edje_File *edf;
   Edje_Part_Collection *coll;
   
   edf = fdata;
   coll = data;
   printf("EEK: EDJE FILE: \"%s\" ref(%i) PART: \"%s\" ref(%i) \n",
	  edf->path, edf->references,
	  coll->part, coll->references);
   _edje_collection_free(edf, coll);

   return 1;
}

#ifdef EDJE_PROGRAM_CACHE
static int
_edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_list_free((Evas_List *)data);
   return 1;
   hash = NULL;
   fdata = NULL;
}
#endif
