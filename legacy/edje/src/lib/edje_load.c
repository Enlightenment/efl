#include "Edje.h"
#include "edje_private.h"

static Evas_Hash   *_edje_file_hash = NULL;

static void _edje_collection_free_part_description_free(Edje_Part_Description *desc);
static int  _edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);

/* API Routines */
void
edje_object_file_set(Evas_Object *obj, const char *file, const char *part)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
   if (!file) file = "";
   if (!part) part = "";
   if (((ed->path) && (!strcmp(file, ed->path))) &&
	(ed->part) && (!strcmp(part, ed->part)))
     return;
   _edje_file_del(ed);
   if (ed->path) free(ed->path);
   ed->path = strdup(file);
   if (ed->part) free(ed->part);
   ed->part = strdup(part);
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
	     
	     ep = l->data;
	     hist = evas_list_append(hist, ep);
	     while (ep->dragable.confine_id >= 0)
	       {
		  ep = evas_list_nth(ed->collection->parts,
				     ep->dragable.confine_id);
		  if (evas_list_find(hist, ep))
		    {
		       printf("EDJE FIXME: ERROR! confine_to loops. invalidating loop.\n");
		       ep->dragable.confine_id = -1;
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
		       printf("EDJE FIXME: ERROR! clip_to loops. invalidating loop.\n");
		       ep->clip_to_id = -1;
		       break;
		    }
		  hist = evas_list_append(hist, ep);
	       }
	     evas_list_free(hist);
	     hist = NULL;
	  }
	/* build real parts */
	for (l = ed->collection->parts; l; l = l->next)
	  {
	     Edje_Part *ep;
	     Edje_Real_Part *rp;
	     char buf[256];
	     
	     ep = l->data;
	     rp = calloc(1, sizeof(Edje_Real_Part));
	     if (!rp) return;
	     rp->part = ep;
	     ed->parts = evas_list_append(ed->parts, rp);
	     rp->param1.description =  ep->default_desc;
	     if (!rp->param1.description)
	       {
		  printf("EDJE FIXME: ERROR! no default part description!\n");
	       }
	     _edje_text_part_on_add(ed, rp);
	     if (ep->type == EDJE_PART_TYPE_RECTANGLE)
	       rp->object = evas_object_rectangle_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_IMAGE)
	       rp->object = evas_object_image_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_TEXT)
	       rp->object = evas_object_text_add(ed->evas);
	     else
	       {
		  printf("EDJE FIXME: ERROR! wrong part type %i!\n", ep->type);
	       }
	     evas_object_smart_member_add(rp->object, ed->obj);
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
	     evas_object_clip_set(rp->object, ed->clipper);
	     evas_object_show(rp->object);
	  }
	for (l = ed->parts; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     
	     rp = l->data;
	     if (rp->param1.description->rel1.id >= 0)
	       rp->param1.rel1_to = evas_list_nth(ed->parts, rp->param1.description->rel1.id);
	     if (rp->param1.description->rel2.id >= 0)
	       rp->param1.rel2_to = evas_list_nth(ed->parts, rp->param1.description->rel2.id);
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
	  }
	ed->dirty = 1;
	_edje_freeze(ed);
	_edje_recalc(ed);
	_edje_emit(ed, "load", "");
	_edje_thaw(ed);
	if ((ed->parts) && (evas_object_visible_get(obj)))
	  evas_object_show(ed->clipper);
     }
}

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

Evas_List *
edje_file_collection_list(const char *file)
{
   Eet_File *ef = NULL;
   Evas_List *lst = NULL;
   Edje_File *ed_file;
   
   ed_file = evas_hash_find(_edje_file_hash, file);
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
     }
   else
     ed_file->references++;
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
   ed_file->references--;   
   if (ed_file->references <= 0) _edje_file_free(ed_file);
   return lst;
}

void
edje_file_collection_list_free(Evas_List *lst)
{
   while (lst)
     {
	if (lst->data) free(lst->data);
	lst = evas_list_remove(lst, lst->data);
     }
}

void
_edje_file_add(Edje *ed)
{
   Eet_File *ef = NULL;
   Edje_Part_Collection *pc = NULL;
   Evas_List *l;
   int id = -1;

   ed->file = evas_hash_find(_edje_file_hash, ed->path);
   if (ed->file)
     {
	ed->file->references++;
     }
   else
     {
	ef = eet_open(ed->path, EET_FILE_MODE_READ);
	if (!ef) return;
   
	ed->file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
	if (!ed->file) goto out;

	ed->file->references = 1;   
	ed->file->path = strdup(ed->path);
	if (!ed->file->collection_dir)
	  {
	     _edje_file_free(ed->file);
	     ed->file = NULL;
	     goto out;
	  }
	if (!ed->file->collection_dir)
	  {
	     _edje_file_free(ed->file);
	     ed->file = NULL;
	     goto out;	     
	  }
	_edje_file_hash = evas_hash_add(_edje_file_hash, ed->path, ed->file);
     }
   
   ed->collection = evas_hash_find(ed->file->collection_hash, ed->part);
   if (ed->collection)
     {
	ed->collection->references++;
     }
   else
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
	     
	     snprintf(buf, sizeof(buf), "collections/%i", id);
	     if (!ef) eet_open(ed->path, EET_FILE_MODE_READ);
	     if (!ef) goto out;
	     ed->collection = eet_data_read(ef, 
					    _edje_edd_edje_part_collection, 
					    buf);
	     if (!ed->collection) goto out;
	     ed->collection->references = 1;
	     ed->file->collection_hash = evas_hash_add(ed->file->collection_hash, ed->part, ed->collection);
	  }
	else
	  {
	     ed->file->references--;
	     if (ed->file->references <= 0)
	       _edje_file_free(ed->file);
	     ed->file = NULL;
	  }
     }
   out:
   if (ef) eet_close(ef);
}

void
_edje_file_del(Edje *ed)
{
   if (ed->collection)
     {
	ed->collection->references--;
	if (ed->collection->references <= 0)
	  _edje_collection_free(ed->collection);
	ed->collection = NULL;
     }
   if (ed->file)
     {
	ed->file->references--;
	if (ed->file->references <= 0)
	  _edje_file_free(ed->file);
	ed->file = NULL;
     }
   if (ed->parts)
     {
	while (ed->parts)
	  {
	     Edje_Real_Part *rp;
	     
	     rp = ed->parts->data;
	     _edje_text_part_on_del(ed, rp);
	     evas_object_del(rp->object);
	     if (rp->swallowed_object)
	       {
		  evas_object_event_callback_del(rp->swallowed_object,
						 EVAS_CALLBACK_FREE,
						 _edje_object_part_swallow_free_cb);
		  evas_object_del(rp->swallowed_object);
	       }
	     if (rp->text.text) free(rp->text.text);
	     if (rp->text.font) free(rp->text.font);
	     if (rp->text.cache.in_str) free(rp->text.cache.in_str);
	     if (rp->text.cache.out_str) free(rp->text.cache.out_str);	     
	     free(rp);
	     ed->parts = evas_list_remove(ed->parts, ed->parts->data);
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
	_edje_animators = evas_list_remove(_edje_animators, ed);
     }
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
}

void
_edje_file_free(Edje_File *edf)
{
   if (edf->path) free(edf->path);
   if (edf->image_dir)
     {
	while (edf->image_dir->entries)
	  {
	     Edje_Image_Directory_Entry *ie;
	     
	     ie = edf->image_dir->entries->data;
	     edf->image_dir->entries = evas_list_remove(edf->image_dir->entries, ie);
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
	     edf->collection_dir->entries = evas_list_remove(edf->collection_dir->entries, ce);
	     if (ce->entry) free(ce->entry);
	     free(ce);
	  }
	free(edf->collection_dir);
     }
   if (edf->collection_hash) evas_hash_free(edf->collection_hash);
   free(edf);
}

void
_edje_collection_free(Edje_Part_Collection *ec)
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
	free(pr);
     }
   while (ec->parts)
     {
	Edje_Part *ep;
	
	ep = ec->parts->data;
	ec->parts = evas_list_remove(ec->parts, ep);
	if (ep->name) free(ep->name);
	if (ep->color_class) free(ep->color_class);
	if (ep->text_class) free(ep->text_class);
	if (ep->default_desc) _edje_collection_free_part_description_free(ep->default_desc);
	while (ep->other_desc)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = ep->other_desc->data;
	     ep->other_desc = evas_list_remove(ep->other_desc, desc);
	     _edje_collection_free_part_description_free(desc);
	  }
     }
   if (ec->prog_cache.no_matches) evas_hash_free(ec->prog_cache.no_matches);
   if (ec->prog_cache.matches)
     {
	evas_hash_foreach(ec->prog_cache.matches, _edje_collection_free_prog_cache_matches_free_cb, NULL);
	evas_hash_free(ec->prog_cache.matches);
     }
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
   if (desc->text.text) free(desc->text.text);
   if (desc->text.font) free(desc->text.font);
   free(desc);
}

static int
_edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_list_free((Evas_List *)data);
   return 1;
}
