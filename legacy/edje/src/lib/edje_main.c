#include "Edje.h"
#include "edje_private.h"

/* FIXME: clip_to needs to work */
/* FIXME: free stuff - no more leaks */
/* FIXME: dragables have to work */
/* FIXME: drag start/top signals etc. */
/* FIXME: drag needs to have signals with relative pos */
/* FIXME: drag vals 0.0 -> 1.0. "rest" pos == 0.0 */
/* FIXME: text parts need to work */
/* FIXME: reduce linked list walking and list_nth calls */
/* FIXME: named parts need to be able to be "replaced" with new evas objects */
/* FIXME: need to be able to calculate min & max size of a whole edje */
/* FIXME: on load don't segv on errors */
/* FIXME: add code to list collections in an eet */
/* FIXME: part replacement with objec t+callbacks */
/* FIXME: part queries for geometry etc. */

/* FIXME: ? somehow handle double click? */
/* FIXME: ? add numeric params to conditions for progs (ranges etc.) */

Edje      *_edje_fetch(Evas_Object *obj);
Edje      *_edje_add(Evas_Object *obj);
void       _edje_del(Edje *ed);
void       _edje_file_add(Edje *ed);
void       _edje_file_del(Edje *ed);
void       _edje_file_free(Edje_File *edf);
void       _edje_collection_free(Edje_Part_Collection *ec);

static void _edje_mouse_in_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static void _edje_mouse_out_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static void _edje_mouse_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static void _edje_mouse_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static void _edje_mouse_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static void _edje_mouse_wheel_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static int  _edje_timer_cb(void *data);
static int  _edje_program_run_iterate(Edje_Running_Program *runp, double tim);
static void _edje_program_end(Edje *ed, Edje_Running_Program *runp);
static void _edje_program_run(Edje *ed, Edje_Program *pr);
static void _edje_emit(Edje *ed, char *sig, char *src);
static int  _edje_glob_match(char *str, char *glob);
static void _edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos);
static void _edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, char  *d1, double v1, char *d2, double v2);
static void _edje_part_recalc_single(Edje *ed, Edje_Real_Part *ep, Edje_Part_Description *desc, Edje_Part_Description *chosen_desc, Edje_Real_Part *rel1_to, Edje_Real_Part *rel2_to, Edje_Real_Part *confine_to, Edje_Calc_Params *params);
static void _edje_part_recalc(Edje *ed, Edje_Real_Part *ep);
static void _edje_recalc(Edje *ed);
static int  _edje_freeze(Edje *ed);
static int  _edje_thaw(Edje *ed);
static void _edje_edd_setup(void);

static void _edje_smart_add(Evas_Object * obj);
static void _edje_smart_del(Evas_Object * obj);
static void _edje_smart_layer_set(Evas_Object * obj, int layer);
static void _edje_smart_raise(Evas_Object * obj);
static void _edje_smart_lower(Evas_Object * obj);
static void _edje_smart_stack_above(Evas_Object * obj, Evas_Object * above);
static void _edje_smart_stack_below(Evas_Object * obj, Evas_Object * below);
static void _edje_smart_move(Evas_Object * obj, double x, double y);
static void _edje_smart_resize(Evas_Object * obj, double w, double h);
static void _edje_smart_show(Evas_Object * obj);
static void _edje_smart_hide(Evas_Object * obj);
static void _edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _edje_smart_clip_unset(Evas_Object * obj);

Eet_Data_Descriptor *_edje_edd_edje_file = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program_target = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_image_id = NULL;

static int          _edje_anim_count = 0;
static double       _edje_frametime = 1.0 / 60.0;
static Ecore_Timer *_edje_timer = NULL;
static Evas_List   *_edje_animators = NULL;
static Evas_Hash   *_edje_file_hash = NULL;

static Evas_Smart *_edje_smart = NULL;

void
edje_init(void)
{
   static int initted = 0;
   
   if (initted) return;
   initted = 1;
   _edje_edd_setup();
}

void
edje_frametime_set(double t)
{
   if (t == _edje_frametime) return;
   _edje_frametime = t;
   if (_edje_timer)
     {
	ecore_timer_del(_edje_timer);
	_edje_timer = ecore_timer_add(_edje_frametime, _edje_timer_cb, NULL);
     }
}

double
edje_frametime_get(void)
{
   return _edje_frametime;
}

Evas_Object *
edje_add(Evas *evas)
{
   if (!_edje_smart)
     {
	edje_init();
	_edje_smart = evas_smart_new("edje",
				     _edje_smart_add,
				     _edje_smart_del,
				     _edje_smart_layer_set,
				     _edje_smart_raise,
				     _edje_smart_lower,
				     _edje_smart_stack_above,
				     _edje_smart_stack_below,
				     _edje_smart_move,
				     _edje_smart_resize,
				     _edje_smart_show,
				     _edje_smart_hide,
				     _edje_smart_color_set,
				     _edje_smart_clip_set, 
				     _edje_smart_clip_unset, 
				     NULL);
     }
   return evas_object_smart_add(evas, _edje_smart);
}

void
edje_file_set(Evas_Object *obj, const char *file, const char *part)
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
	
	for (l = ed->collection->parts; l; l = l->next)
	  {
	     Edje_Part *ep;
	     Edje_Real_Part *rp;
	     char buf[256];
	     
	     ep = l->data;
	     rp = calloc(1, sizeof(Edje_Real_Part));
	     if (!rp) return;
	     ed->parts = evas_list_append(ed->parts, rp);
	     rp->param1.description =  ep->default_desc;
	     if (ep->type == EDJE_PART_TYPE_RECTANGLE)
	       rp->object = evas_object_rectangle_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_IMAGE)
	       rp->object = evas_object_image_add(ed->evas);
	     else if (ep->type == EDJE_PART_TYPE_TEXT)
	       rp->object = evas_object_text_add(ed->evas);
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
	       }
	     evas_object_clip_set(rp->object, ed->clipper);
	     evas_object_show(rp->object);
	     rp->part = ep;
	  }
	for (l = ed->parts; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     
	     rp = l->data;
	     if (rp->param1.description->rel1.id >= 0)
	       rp->param1.rel1_to = evas_list_nth(ed->parts, rp->param1.description->rel1.id);
	     if (rp->param1.description->rel2.id >= 0)
	       rp->param1.rel2_to = evas_list_nth(ed->parts, rp->param1.description->rel2.id);
	  }
	ed->dirty = 1;
	_edje_freeze(ed);
	_edje_recalc(ed);
	_edje_emit(ed, "load", "");
	_edje_thaw(ed);
     }
}

void
edje_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data)
{
   Edje *ed;
   Edje_Signal_Callback *escb;
   
   if ((!emission) || (!source) || (!func)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   escb = calloc(1, sizeof(Edje_Signal_Callback));
   escb->signal = strdup(emission);
   escb->source = strdup(source);
   escb->func = func;
   escb->data = data;
   ed->callbacks = evas_list_append(ed->callbacks, escb);
}

void *
edje_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source))
{
   Edje *ed;
   Evas_List *l;
   
   if ((!emission) || (!source) || (!func)) return NULL;
   ed = _edje_fetch(obj);
   if (!ed) return;
   for (l = ed->callbacks; l; l = l->next)
     {
	Edje_Signal_Callback *escb;
	
	escb = l->data;
	if ((escb->func == func) && 
	    (!strcmp(escb->signal, emission)) &&
	    (!strcmp(escb->source, source)))
	  {
	     void *data;
	     
	     data = escb->data;
	     free(escb->signal);
	     free(escb->source);
	     free(escb);
	     return data;
	  }
     }
   return NULL;
}

void
edje_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   Edje *ed;

   if ((!emission) || (!source)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   _edje_emit(ed, emission, source);
}

/*** internal calls ***/

/* utility functions we will use a lot */

Edje *
_edje_fetch(Evas_Object *obj)
{
   Edje *ed;
   char *type;
   
   type = (char *)evas_object_type_get(obj);
   if (!type) return NULL;
   if (strcmp(type, "edje")) return NULL;
   ed = evas_object_smart_data_get(obj);
   return ed;
}

Edje *
_edje_add(Evas_Object *obj)
{
   Edje *ed;
   
   ed = calloc(1, sizeof(Edje));
   if (!ed) return NULL;
   ed->evas = evas_object_evas_get(obj);
   ed->clipper = evas_object_rectangle_add(ed->evas);
   evas_object_smart_member_add(ed->clipper, obj);
   evas_object_color_set(ed->clipper, 255, 255, 255, 255);
   evas_object_move(ed->clipper, 0, 0);
   evas_object_resize(ed->clipper, 0, 0);
   return ed;
}

void
_edje_del(Edje *ed)
{
   _edje_file_del(ed);
   if (ed->path) free(ed->path);
   if (ed->part) free(ed->part);
   evas_object_del(ed->clipper);
   printf("FIXME: leak: ed->callbacks\n");
   free(ed);
}

void
_edje_file_add(Edje *ed)
{
   Eet_File *ef = NULL;
   Edje_Part_Collection *pc = NULL;
   Evas_List *l;
   int id = -1;

   /* FIXME: look in hash table first */   
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
	     /* FIXME: free up ed->file */
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
	     evas_object_del(rp->object);
	     if (rp->text.text) free(rp->text.text);
	     if (rp->text.font) free(rp->text.font);
	     free(rp);
	     ed->parts = evas_list_remove(ed->parts, ed->parts->data);
	  }
	ed->parts = NULL;
     }
   if (ed->actions)
     {
//	printf("FIXME: leak!\n");
	ed->actions = NULL;
     }
}

void
_edje_file_free(Edje_File *edf)
{
   printf("FIXME: leak!\n");
}

void
_edje_collection_free(Edje_Part_Collection *ec)
{
   printf("FIXME: leak!\n");
}

/* manipulation calls */

static void
_edje_mouse_in_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   _edje_emit(ed, "mouse,in", rp->part->name);
}

static void
_edje_mouse_out_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   _edje_emit(ed, "mouse,out", rp->part->name);
}

static void
_edje_mouse_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
   if (rp->clicked_button == 0)
     {
	rp->clicked_button = ev->button;
	rp->still_in = 1;
     }
   _edje_emit(ed, buf, rp->part->name);
}

static void
_edje_mouse_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,up,%i", ev->button);
   _edje_emit(ed, buf, rp->part->name);
   if ((rp->still_in) && (rp->clicked_button == ev->button))
     {
	rp->clicked_button = 0;
	rp->still_in = 0;
	snprintf(buf, sizeof(buf), "mouse,clicked,%i", ev->button);
	_edje_emit(ed, buf, rp->part->name);
     }
}

static void
_edje_mouse_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   if (rp->still_in)
     {
	double x, y, w, h;
	
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	if ((ev->cur.canvas.x < x) || (ev->cur.canvas.y < y) || 
	    (ev->cur.canvas.x >= (x + w)) || (ev->cur.canvas.y >= (y + h)))
	  rp->still_in = 0;
     }
   _edje_emit(ed, "mouse,move", rp->part->name);
}

static void
_edje_mouse_wheel_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,wheel,%i,%i", ev->direction, (ev->z < 0) ? (-1) : (1));
   _edje_emit(ed, buf, rp->part->name);
}

static int
_edje_timer_cb(void *data)
{
   double t;
   Evas_List *l;
   Evas_List *animl = NULL;
   
   t = ecore_time_get();
   for (l = _edje_animators; l; l = l->next)
     animl = evas_list_append(animl, l->data);
   while (animl)
     {
	Edje *ed;
	Evas_List *newl = NULL;
	
	ed = animl->data;
	_edje_freeze(ed);
	animl = evas_list_remove(animl, animl->data);
	for (l = ed->actions; l; l = l->next)
	  newl = evas_list_append(newl, l->data);
	while (newl)
	  {
	     Edje_Running_Program *runp;
	     
	     runp = newl->data;
	     newl = evas_list_remove(newl, newl->data);
	     _edje_program_run_iterate(runp, t);
	  }
	_edje_thaw(ed);
     }
   if (_edje_anim_count > 0) return 1;
   _edje_timer = NULL;
   return 0;
}

static int
_edje_program_run_iterate(Edje_Running_Program *runp, double tim)
{
   double t, total;
   Evas_List *l;

   _edje_freeze(runp->edje);
   t = tim - runp->start_time;
   total = runp->program->tween.time;
   t /= total;
   if (t > 1.0) t = 1.0;
   for (l = runp->program->targets; l; l = l->next)
     {
	Edje_Real_Part *rp;
	Edje_Program_Target *pt;
	
	pt = l->data;
	rp = evas_list_nth(runp->edje->parts, pt->id);
	if (rp) _edje_part_pos_set(runp->edje, rp, 
				   runp->program->tween.mode, t);
     }
   if (t >= 1.0)
     {
	for (l = runp->program->targets; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     Edje_Program_Target *pt;
	     
	     pt = l->data;
	     rp = evas_list_nth(runp->edje->parts, pt->id);
	     if (rp)
	       {
		  _edje_part_description_apply(runp->edje, rp, 
					       runp->program->state, 
					       runp->program->value,
					       NULL,
					       0.0);
		  _edje_part_pos_set(runp->edje, rp, 
				     runp->program->tween.mode, 0.0);
		  rp->program = NULL;
	       }
	  }
	_edje_recalc(runp->edje);
	_edje_anim_count--;
	runp->edje->actions = evas_list_remove(runp->edje->actions, runp);
	if (!runp->edje->actions)
	  _edje_animators = evas_list_remove(_edje_animators, runp->edje);
	_edje_emit(runp->edje, "anim,stop", runp->program->name);
	if (runp->program->after >= 0)
	  {
	     Edje_Program *pr;
	     
	     pr = evas_list_nth(runp->edje->collection->programs, 
				runp->program->after);
	     if (pr) _edje_program_run(runp->edje, pr);
	  }
	_edje_thaw(runp->edje);
	free(runp);
	return  0;
     }
   _edje_recalc(runp->edje);
   _edje_thaw(runp->edje);
   return 1;
}

static void
_edje_program_end(Edje *ed, Edje_Running_Program *runp)
{
   Evas_List *l;

   _edje_freeze(runp->edje);
   for (l = runp->program->targets; l; l = l->next)
     {
	Edje_Real_Part *rp;
	Edje_Program_Target *pt;
	
	pt = l->data;
	rp = evas_list_nth(runp->edje->parts, pt->id);
	if (rp)
	  {
	     _edje_part_description_apply(runp->edje, rp, 
					  runp->program->state, 
					  runp->program->value,
					  NULL,
					  0.0);
	     _edje_part_pos_set(runp->edje, rp, 
				runp->program->tween.mode, 0.0);
	     rp->program = NULL;
	  }
     }
   _edje_recalc(runp->edje);
   _edje_anim_count--;
   runp->edje->actions = evas_list_remove(runp->edje->actions, runp);
   if (!runp->edje->actions)
     _edje_animators = evas_list_remove(_edje_animators, runp->edje);
   _edje_emit(runp->edje, "anim,stop", runp->program->name);
   _edje_thaw(runp->edje);
   free(runp);
}
   
static void
_edje_program_run(Edje *ed, Edje_Program *pr)
{
   Evas_List *l;

   _edje_freeze(ed);
   _edje_emit(ed, "anim,start", pr->name);
   if (pr->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	if (pr->tween.time > 0.0)
	  {
	     Edje_Running_Program *runp;
	     
	     runp = calloc(1, sizeof(Edje_Running_Program));
	     for (l = pr->targets; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  Edje_Program_Target *pt;
		  
		  pt = l->data;
		  rp = evas_list_nth(ed->parts, pt->id);
		  if (rp)
		    {
		       if (rp->program)
			 _edje_program_end(ed, rp->program);
		       _edje_part_description_apply(ed, rp, 
						    rp->param1.description->state.name,
						    rp->param1.description->state.value, 
						    pr->state, 
						    pr->value);
		       _edje_part_pos_set(ed, rp, pr->tween.mode, 0.0);
		       rp->program = runp;
		    }
	       }
	     
	     if (!ed->actions)
	       _edje_animators = evas_list_append(_edje_animators, ed);
	     ed->actions = evas_list_append(ed->actions, runp);
	     runp->start_time = ecore_time_get();
	     runp->edje = ed;
	     runp->program = pr;
	     if (!_edje_timer)
	       _edje_timer = ecore_timer_add(_edje_frametime, _edje_timer_cb, NULL);
	     _edje_anim_count++;
	  }
	else
	  {
	     for (l = pr->targets; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  Edje_Program_Target *pt;
		  
		  pt = l->data;
		  rp = evas_list_nth(ed->parts, pt->id);
		  if (rp)
		    {
		       if (rp->program)
			 _edje_program_end(ed, rp->program);
		       _edje_part_description_apply(ed, rp, 
						    pr->state, 
						    pr->value,
						    NULL,
						    0.0);
		       _edje_part_pos_set(ed, rp, pr->tween.mode, 0.0);
		    }
	       }
	     _edje_recalc(ed);
	  }
     }
   else if (pr->action == EDJE_ACTION_TYPE_ACTION_STOP)
     {
	for (l = pr->targets; l; l = l->next)
	  {
	     Edje_Program_Target *pt;
	     Evas_List *ll;
	     
	     pt = l->data;
	     for (ll = ed->actions; ll; ll = ll->next)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = ll->data;
		  if (pt->id == runp->program->id)
		    {
		       _edje_program_end(ed, runp);
		       break;
		    }
	       }
	  }
     }
   else if (pr->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
	_edje_emit(ed, pr->state, pr->state2);
     }
   _edje_thaw(ed);
}

static void
_edje_emit(Edje *ed, char *sig, char *src)
{
   Evas_List *l;
   static Evas_List *emissions = NULL;
   Edje_Emission *ee;

   _edje_freeze(ed);
   printf("EMIT \"%s\" \"%s\"\n", sig, src);
   ee = calloc(1, sizeof(Edje_Emission));
   if (!ee) return;
   ee->signal = strdup(sig);
   ee->source = strdup(src);
   if (emissions)
     {
	emissions = evas_list_append(emissions, ee);
	_edje_thaw(ed);
	return;
     }
   else
     emissions = evas_list_append(emissions, ee);
   while (emissions)
     {
	ee = emissions->data;
	emissions = evas_list_remove(emissions, ee);
	for (l = ed->collection->programs; l; l = l->next)
	  {
	     Edje_Program *pr;
	     
	     pr = l->data;
	     if ((_edje_glob_match(ee->signal, pr->signal)) &&
		 (_edje_glob_match(ee->source, pr->source)))
	       _edje_program_run(ed, pr);
	  }
	for (l = ed->callbacks; l; l = l->next)
	  {
	     Edje_Signal_Callback *escb;
	     
	     escb = l->data;
	     if ((_edje_glob_match(ee->signal, escb->signal)) &&
		 (_edje_glob_match(ee->source, escb->source)))
	       escb->func(escb->data, ed->obj, ee->signal, ee->source);
	  }
	free(ee->signal);
	free(ee->source);
	free(ee);
     }
   _edje_thaw(ed);
}

static int
_edje_glob_match(char *str, char *glob)
{
   if (!fnmatch(glob, str, 0)) return 1;
   return 0;
}

static void
_edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos)
{
   double npos;
   
   if (pos > 1.0) pos = 1.0;
   else if (pos < 0.0) pos = 0.0;
   npos = 0.0;
   /* take linear pos along timescale and use interpolation method */
   switch (mode)
     {
      case EDJE_TWEEN_MODE_SINUSOIDAL:
	npos = (1.0 - cos(pos * PI)) / 2.0;
	break;
      case EDJE_TWEEN_MODE_ACCELERATE:
	npos = 1.0 - sin((PI / 2.0) + (pos * PI / 2.0));
	break;
      case EDJE_TWEEN_MODE_DECELERATE:
	npos = sin(pos * PI / 2.0);
	break;
      case EDJE_TWEEN_MODE_LINEAR:
	npos = pos;
	break;
      default:
	break;
     }
   if (npos == ep->description_pos) return;

   ep->description_pos = npos;
   
   ed->dirty = 1;
   ep->dirty = 1;
}

static void
_edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, char  *d1, double v1, char *d2, double v2)
{
   char *cd1 = "default", *cd2 = "default";
   double cv1 = 0.0, cv2 = 0.0;
   
   if (!d1) d1 = "default";
   if (!d2) d2 = "default";

   if (!strcmp(d1, "default") && (v1 == 0.0))
     ep->param1.description = ep->part->default_desc;
   else
     {
	Evas_List *l;
	double min_dst;
	Edje_Part_Description *desc_found;

	desc_found = NULL;
	min_dst = 999.0;
	if (!strcmp("default", d1))
	  {
	     desc_found = ep->part->default_desc;
	     min_dst = ep->part->default_desc->state.value - v1;
	     if (min_dst < 0) min_dst = -min_dst;
	  }
	for (l = ep->part->other_desc; l; l = l->next)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = l->data;
	     if (!strcmp(desc->state.name, d1))
	       {
		  double dst;
		  
		  dst = desc->state.value - v1;
		  if (dst == 0.0)
		    {
		       desc_found = desc;
		       break;
		    }
		  if (dst < 0.0) dst = -dst;
		  if (dst < min_dst)
		    {
		       desc_found = desc;
		       min_dst = dst;
		    }
	       }
	  }
	ep->param1.description = desc_found;
     }
   ep->param1.rel1_to = NULL;
   ep->param1.rel2_to = NULL;
   if (ep->param1.description)
     {
	if (ep->param1.description->rel1.id >= 0)
	  ep->param1.rel1_to = evas_list_nth(ed->parts, ep->param1.description->rel1.id);
	if (ep->param1.description->rel2.id >= 0)
	  ep->param1.rel2_to = evas_list_nth(ed->parts, ep->param1.description->rel2.id);
     }
   
   if (!strcmp(d2, "default") && (v2 == 0.0))
     ep->param2.description = ep->part->default_desc;
   else
     {
	Evas_List *l;
	double min_dst;
	Edje_Part_Description *desc_found;
	
	desc_found = NULL;
	min_dst = 999.0;
	if (!strcmp("default", d2))
	  {
	     desc_found = ep->part->default_desc;
	     min_dst = ep->part->default_desc->state.value - v2;
	     if (min_dst < 0) min_dst = -min_dst;
	  }
	for (l = ep->part->other_desc; l; l = l->next)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = l->data;
	     if (!strcmp(desc->state.name, d2))
	       {
		  double dst;
		  
		  dst = desc->state.value - v2;
		  if (dst == 0.0)
		    {
		       desc_found = desc;
		       break;
		    }
		  if (dst < 0.0) dst = -dst;
		  if (dst < min_dst)
		    {
		       desc_found = desc;
		       min_dst = dst;
		    }
	       }
	  }
	ep->param2.description = desc_found;
     }
   if (!ep->param1.description)
     ep->param1.description = ep->part->default_desc;
   ep->param1.rel1_to = NULL;
   ep->param1.rel2_to = NULL;
   if (ep->param1.description)
     {
	if (ep->param1.description->rel1.id >= 0)
	  ep->param1.rel1_to = evas_list_nth(ed->parts, ep->param1.description->rel1.id);
	if (ep->param1.description->rel2.id >= 0)
	  ep->param1.rel2_to = evas_list_nth(ed->parts, ep->param1.description->rel2.id);
     }
   ep->param2.rel1_to = NULL;
   ep->param2.rel2_to = NULL;
   if (ep->param2.description)
     {
	if (ep->param2.description->rel1.id >= 0)
	  ep->param2.rel1_to = evas_list_nth(ed->parts, ep->param2.description->rel1.id);
	if (ep->param2.description->rel2.id >= 0)
	  ep->param2.rel2_to = evas_list_nth(ed->parts, ep->param2.description->rel2.id);
     }
   
   ed->dirty = 1;
   ep->dirty = 1;
}

/* calculation functions */

static void
_edje_part_recalc_single(Edje *ed,
			 Edje_Real_Part *ep, 
			 Edje_Part_Description *desc, 
			 Edje_Part_Description *chosen_desc,
			 Edje_Real_Part *rel1_to, 
			 Edje_Real_Part *rel2_to, 
			 Edje_Real_Part *confine_to,
			 Edje_Calc_Params *params)
{
   int minw, minh;

   /* relative coords of top left & bottom right */
   if (rel1_to)
     {
	params->x = desc->rel1.offset_x +
	  rel1_to->x + (desc->rel1.relative_x * rel1_to->w);
	params->y = desc->rel1.offset_y +
	  rel1_to->y + (desc->rel1.relative_y * rel1_to->h);
     }
   else
     {
	params->x = desc->rel1.offset_x +
	  (desc->rel1.relative_x * ed->w);
	params->y = desc->rel1.offset_y +
	  (desc->rel1.relative_y * ed->h);
     }
   if (rel2_to)
     {
	params->w = desc->rel2.offset_x +
	  rel2_to->x + (desc->rel2.relative_x * rel2_to->w) -
	  params->x;
	params->h = desc->rel2.offset_y +
	  rel2_to->y + (desc->rel2.relative_y * rel2_to->h) -
	  params->y;
     }
   else
     {
	params->w = (double)desc->rel2.offset_x +
	  (desc->rel2.relative_x * (double)ed->w) -
	  params->x + 1;
	params->h = (double)desc->rel2.offset_y +
	  (desc->rel2.relative_y * (double)ed->h) -
	  params->y + 1;
     }   

   /* aspect */
   if (params->h > 0)
     {
	double aspect;
	double new_w, new_h;
   
	new_h = params->h;
	new_w = params->w;
	aspect = (double)params->w / (double)params->h;
	/* adjust for max aspect (width / height) */
	if ((desc->aspect.max > 0.0) && (aspect > desc->aspect.max))
	  {
	     new_h = (params->w / desc->aspect.max);
	     new_w = (params->h * desc->aspect.max);
	  }
	/* adjust for min aspect (width / height) */
	if ((desc->aspect.min > 0.0) && (aspect < desc->aspect.min))
	  {
	     new_h = (params->w / desc->aspect.min);
	     new_w = (params->w * desc->aspect.min);
	  }
	/* do real adjustment */
	if ((params->h - new_h) > (params->w - new_w))
	  {
	     if (params->h < new_h)
	       {
		  params->y = params->y +
		    ((params->h - new_h) * (1.0 - desc->align.y));
		  params->h = new_h;
	       }
	     else if (params->h > new_h)
	       {
		  params->y = params->y +
		    ((params->h - new_h) * desc->align.y);
		  params->h = new_h;
	       }
	  }
	else
	  {
	     if (params->w < new_w)
	       {
		  params->x = params->x +
		    ((params->w - new_w) * (1.0 - desc->align.x));
		  params->w = new_w;
	       }
	     else if (params->w > new_w)
	       {
		  params->x = params->x +
		    ((params->w - new_w) * desc->align.x);
		  params->w = new_w;
	       }
	  }
     }

   /* size step */
   if (desc->step.x > 0)
     {
	int steps;
	int new_w;
	
	steps = params->w / desc->step.x;
	new_w = desc->step.x * steps;
	if (params->w > new_w)
	  {
	     params->x = params->x +
	       ((params->w - new_w) * desc->align.x);
	     params->w = new_w;
	  }	
     }
   if (desc->step.y > 0)
     {
	int steps;
	int new_h;
	
	steps = params->h / desc->step.y;
	new_h = desc->step.y * steps;
	if (params->h > new_h)
	  {
	     params->y = params->y +
	       ((params->h - new_h) * desc->align.y);
	     params->h = new_h;
	  }	
     }
   minw = desc->min.w;
   minh = desc->min.h;
   /* if we have text that wants to make the min size the text size... */
   if ((chosen_desc) && (ep->part->type == EDJE_PART_TYPE_TEXT))
     {
	char   *text;
	char   *font;
	int     size;
	double  tw, th;
	
	text = chosen_desc->text.text;
	font = chosen_desc->text.font;
	size = chosen_desc->text.size;
	if (ep->text.text) text = ep->text.text;
	if (ep->text.font) font = ep->text.font;
	if (ep->text.size) size = ep->text.size;
	evas_object_text_font_set(ep->object, font, size);
	if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y))
	  {
	     evas_object_text_text_set(ep->object, text);
	     evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	     if (chosen_desc->text.min_x)
	       {
		  minw = tw;
		  /* FIXME: account for effect */
		  /* for now just add 2 */
		  minw += 2;
	       }
	     if (chosen_desc->text.min_y)
	       {
		  minh = th;
		  /* FIXME: account for effect */
		  /* for now just add 2 */
		  minw += 2;
	       }
	  }
     }
   /* adjust for min size */
   if (minw >= 0)
     {
	if (params->w < minw)
	  {
	     params->x = params->x + 
	       ((params->w - minw) * (1.0 - desc->align.x));
	     params->w = minw;
	  }
     }
   if (minh >= 0)
     {
	if (params->h < minh)
	  {
	     params->y = params->y + 
	       ((params->h - minh) * (1.0 - desc->align.y));
	     params->h = minh;
	  }
     }
   /* adjust for max size */
   if (desc->max.w >= 0)
     {
	if (params->w > desc->max.w)
	  {
	     params->x = params->x + 
	       ((params->w - desc->max.w) * desc->align.x);
	     params->w = desc->max.w;
	  }
     }
   if (desc->max.h >= 0)
     {
	if (params->h > desc->max.h)
	  {
	     params->y = params->y + 
	       ((params->h - desc->max.h) * desc->align.y);
	     params->h = desc->max.h;
	  }
     }
   /* confine */
   if (confine_to)
     {
	int offset;
	int step;
	
	/* complex dragable params */
	offset = params->x + ep->drag.x - confine_to->x;
	if (desc->dragable.step_x > 0)
	  {
	     params->x = confine_to->x + 
	       ((offset / desc->dragable.step_x) * desc->dragable.step_x);
	  }
	else if (desc->dragable.count_x > 0)
	  {
	     step = (confine_to->w - params->w) / desc->dragable.count_x;
	     params->x = confine_to->x +
	       ((offset / step) * step);	       
	  }
	offset = params->y + ep->drag.y - confine_to->y;
	if (desc->dragable.step_y > 0)
	  {
	     params->y = confine_to->y + 
	       ((offset / desc->dragable.step_y) * desc->dragable.step_y);
	  }
	else if (desc->dragable.count_y > 0)
	  {
	     step = (confine_to->h - params->h) / desc->dragable.count_y;
	     params->y = confine_to->y +
	       ((offset / step) * step);	       
	  }
	/* limit to confine */
	if (params->x < confine_to->x)
	  {
	     params->x = confine_to->x;
	  }
	if ((params->x + params->w) > (confine_to->x + confine_to->w))
	  {
	     params->x = confine_to->w - params->w;
	  }
	if (params->y < confine_to->y)
	  {
	     params->y = confine_to->y;
	  }
	if ((params->y + params->h) > (confine_to->y + confine_to->h))
	  {
	     params->y = confine_to->h - params->y;
	  }
     }
   else
     {
	/* simple dragable params */
	params->x += ep->drag.x;
	params->y += ep->drag.y;
     }
   /* fill */
   params->fill.x = desc->fill.pos_abs_x + (params->w * desc->fill.pos_rel_x);
   params->fill.w = desc->fill.abs_x + (params->w * desc->fill.rel_x);
   params->fill.y = desc->fill.pos_abs_y + (params->h * desc->fill.pos_rel_y);
   params->fill.h = desc->fill.abs_y + (params->h * desc->fill.rel_y);
   /* colors */
   params->color.r = desc->color.r;
   params->color.g = desc->color.g;
   params->color.b = desc->color.b;
   params->color.a = desc->color.a;
   params->color2.r = desc->color2.r;
   params->color2.g = desc->color2.g;
   params->color2.b = desc->color2.b;
   params->color2.a = desc->color2.a;
   params->color3.r = desc->color3.r;
   params->color3.g = desc->color3.g;
   params->color3.b = desc->color3.b;
   params->color3.a = desc->color3.a;
   /* visible */
   params->visible = desc->visible;
   /* border */
   params->border.l = desc->border.l;
   params->border.r = desc->border.r;
   params->border.t = desc->border.t;
   params->border.b = desc->border.b;
}

static void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep)
{
   Edje_Calc_Params p1, p2, p3;
   Edje_Part_Description *chosen_desc;
   double pos = 0.0;
   
   if (ep->calculated) return;
   if (ep->param1.rel1_to)    _edje_part_recalc(ed, ep->param1.rel1_to);
   if (ep->param1.rel2_to)    _edje_part_recalc(ed, ep->param1.rel2_to);
   if (ep->param1.confine_to) _edje_part_recalc(ed, ep->param1.confine_to);
   if (ep->param2.rel1_to)    _edje_part_recalc(ed, ep->param2.rel1_to);
   if (ep->param2.rel2_to)    _edje_part_recalc(ed, ep->param2.rel2_to);
   if (ep->param2.confine_to) _edje_part_recalc(ed, ep->param2.confine_to);
   
   /* actually calculate now */
   if (ep->description_pos == 0.0)
     chosen_desc = ep->param1.description;
   else
     chosen_desc = ep->param2.description;
   
   if (ep->param1.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, chosen_desc, ep->param1.rel1_to, ep->param1.rel2_to, ep->param1.confine_to, &p1);
   if (ep->param2.description)
     {
	_edje_part_recalc_single(ed, ep, ep->param2.description, chosen_desc, ep->param2.rel1_to, ep->param2.rel2_to, ep->param2.confine_to, &p2);

	/* FIXME: pos isnt just linear - depends on tween method */
	pos = ep->description_pos;
	
	/* visible is special */
	if ((p1.visible) && (!p2.visible))
	  {
	     if (pos == 1.0)
	       p3.visible = 0;
	     else
	       p3.visible = 1;
	  }
	else if ((!p1.visible) && (p2.visible))
	  {
	     if (pos == 0.0)
	       p3.visible = 0;
	     else
	       p3.visible = 1;
	  }
	else
	  p3.visible = p1.visible;
	
	p3.x = (p1.x * (1.0 - pos)) + (p2.x * (pos));
	p3.y = (p1.y * (1.0 - pos)) + (p2.y * (pos));
	p3.w = (p1.w * (1.0 - pos)) + (p2.w * (pos));
	p3.h = (p1.h * (1.0 - pos)) + (p2.h * (pos));
	
	p3.fill.x = (p1.fill.x * (1.0 - pos)) + (p2.fill.x * (pos));
	p3.fill.y = (p1.fill.y * (1.0 - pos)) + (p2.fill.y * (pos));
	p3.fill.w = (p1.fill.w * (1.0 - pos)) + (p2.fill.w * (pos));
	p3.fill.h = (p1.fill.h * (1.0 - pos)) + (p2.fill.h * (pos));
	
	p3.color.r = (p1.color.r * (1.0 - pos)) + (p2.color.r * (pos));
	p3.color.g = (p1.color.g * (1.0 - pos)) + (p2.color.g * (pos));
	p3.color.b = (p1.color.b * (1.0 - pos)) + (p2.color.b * (pos));
	p3.color.a = (p1.color.a * (1.0 - pos)) + (p2.color.a * (pos));
	
	p3.color2.r = (p1.color2.r * (1.0 - pos)) + (p2.color2.r * (pos));
	p3.color2.g = (p1.color2.g * (1.0 - pos)) + (p2.color2.g * (pos));
	p3.color2.b = (p1.color2.b * (1.0 - pos)) + (p2.color2.b * (pos));
	p3.color2.a = (p1.color2.a * (1.0 - pos)) + (p2.color2.a * (pos));
	
	p3.color3.r = (p1.color3.r * (1.0 - pos)) + (p2.color3.r * (pos));
	p3.color3.g = (p1.color3.g * (1.0 - pos)) + (p2.color3.g * (pos));
	p3.color3.b = (p1.color3.b * (1.0 - pos)) + (p2.color3.b * (pos));
	p3.color3.a = (p1.color3.a * (1.0 - pos)) + (p2.color3.a * (pos));
	
	p3.border.l = (p1.border.l * (1.0 - pos)) + (p2.border.l * (pos));
	p3.border.r = (p1.border.r * (1.0 - pos)) + (p2.border.r * (pos));
	p3.border.t = (p1.border.t * (1.0 - pos)) + (p2.border.t * (pos));
	p3.border.b = (p1.border.b * (1.0 - pos)) + (p2.border.b * (pos));
     }
   else
     p3 = p1;
   if (ep->part->type == EDJE_PART_TYPE_RECTANGLE)
     {
	evas_object_move(ep->object, ed->x + p3.x, ed->y + p3.y);
	evas_object_resize(ep->object, p3.w, p3.h);
     }
   else if (ep->part->type == EDJE_PART_TYPE_TEXT)
     {
	/* FIXME: if text object calculate text now */
	/* FIXME: set other colors */
     }
   else if (ep->part->type == EDJE_PART_TYPE_IMAGE)
     {
	char buf[4096];
	int image_id;
	int image_count, image_num;

//	printf("loc %3.3f %3.3f %3.3fx%3.3f\n", p3.x, p3.y, p3.w, p3.h);
	evas_object_move(ep->object, ed->x + p3.x, ed->y + p3.y);
	evas_object_resize(ep->object, p3.w, p3.h);
	evas_object_image_fill_set(ep->object, p3.fill.x, p3.fill.y, p3.fill.w, p3.fill.h);
//	printf("fill %3.3f %3.3f %3.3fx%3.3f\n", p3.fill.x, p3.fill.y, p3.fill.w, p3.fill.h);

	evas_object_image_border_set(ep->object, p3.border.l, p3.border.r, p3.border.t, p3.border.b);
	image_id = ep->param1.description->image.id;
	image_count = 2;
	if (ep->param2.description)
	  image_count += evas_list_count(ep->param2.description->image.tween_list);
	image_num = (pos * ((double)image_count - 0.5));
	if (image_num > (image_count - 1))
	  image_num = image_count - 1;
	if (image_num == 0)
	  image_id = ep->param1.description->image.id;
	else if (image_num == (image_count - 1))
	  image_id = ep->param2.description->image.id;
	else
	  {
	     Edje_Part_Image_Id *imid;
	     
	     imid = evas_list_nth(ep->param2.description->image.tween_list, image_num - 1);
	     if (imid) image_id = imid->id;
	  }

	snprintf(buf, sizeof(buf), "images/%i", image_id);
	evas_object_image_file_set(ep->object, ed->file->path, buf);
     }
   if (p3.visible) evas_object_show(ep->object);
   else evas_object_hide(ep->object);
   evas_object_color_set(ep->object, p3.color.r, p3.color.g, p3.color.b, p3.color.a);
   
   ep->x = p3.x;
   ep->y = p3.y;
   ep->w = p3.w;
   ep->h = p3.h;
   
   ep->calculated = 1;
   ep->dirty = 0;
}

static void
_edje_recalc(Edje *ed)
{
   Evas_List *l;
   
   if (!ed->dirty) return;
   if (ed->freeze)
     {
	ed->recalc = 1;
	return;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	ep->calculated = 0;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	if (!ep->calculated) _edje_part_recalc(ed, ep);
     }
   ed->dirty = 0;
   ed->recalc = 0;
}

static int
_edje_freeze(Edje *ed)
{
   ed->freeze++;
   return ed->freeze;
}

static int
_edje_thaw(Edje *ed)
{
   ed->freeze--;
   if ((ed->freeze <= 0) && (ed->recalc))
     _edje_recalc(ed);
   return ed->freeze;
}

static void
_edje_edd_setup(void)
{
   /* image directory */
   _edje_edd_edje_image_directory_entry = eet_data_descriptor_new("Edje_Image_Directory_Entry",
								  sizeof(Edje_Image_Directory_Entry),
								  evas_list_next,
								  evas_list_append,
								  evas_list_data,
								  evas_hash_foreach,
								  evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);
   
   _edje_edd_edje_image_directory = eet_data_descriptor_new("Edje_Image_Directory", 
							    sizeof(Edje_Image_Directory),
							    evas_list_next,
							    evas_list_append,
							    evas_list_data,
							    evas_hash_foreach,
							    evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_image_directory, Edje_Image_Directory, "entries", entries, _edje_edd_edje_image_directory_entry);

   /* collection directory */
   _edje_edd_edje_part_collection_directory_entry = eet_data_descriptor_new("Edje_Part_Collection_Directory_Entry",
									    sizeof(Edje_Part_Collection_Directory_Entry),
									    evas_list_next,
									    evas_list_append,
									    evas_list_data,
									    evas_hash_foreach,
									    evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "id", id, EET_T_INT);
   
   _edje_edd_edje_part_collection_directory = eet_data_descriptor_new("Edje_Part_Collection_Directory",
								      sizeof(Edje_Part_Collection_Directory),
								      evas_list_next,
								      evas_list_append,
								      evas_list_data,
								      evas_hash_foreach,
								      evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection_directory, Edje_Part_Collection_Directory, "entries", entries, _edje_edd_edje_part_collection_directory_entry);
   
   /* the main file directory */
   _edje_edd_edje_file = eet_data_descriptor_new("Edje_File", 
						 sizeof(Edje_File),
						 evas_list_next,
						 evas_list_append,
						 evas_list_data,
						 evas_hash_foreach,
						 evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "image_dir", image_dir, _edje_edd_edje_image_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "collection_dir", collection_dir, _edje_edd_edje_part_collection_directory);   

   /* parts & programs - loaded induvidually */
   _edje_edd_edje_program_target = eet_data_descriptor_new("Edje_Program_Target",
							   sizeof(Edje_Program_Target),
							   evas_list_next,
							   evas_list_append,
							   evas_list_data,
							   evas_hash_foreach,
							   evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_target, Edje_Program_Target, "id", id, EET_T_INT);
   
   _edje_edd_edje_program = eet_data_descriptor_new("Edje_Program",
						    sizeof(Edje_Program),
						    evas_list_next,
						    evas_list_append,
						    evas_list_data,
						    evas_hash_foreach,
						    evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "signal", signal, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "action", action, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "state", state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "state2", state2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "value", value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tween.mode", tween.mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tween.time", tween.time, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_program, Edje_Program, "targets", targets, _edje_edd_edje_program_target);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "after", after, EET_T_INT);
   
   _edje_edd_edje_part_image_id = eet_data_descriptor_new("Edje_Part_Image_Id",
							  sizeof(Edje_Part_Image_Id),
							  evas_list_next,
							  evas_list_append,
							  evas_list_data,
							  evas_hash_foreach,
							  evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_image_id, Edje_Part_Image_Id, "id", id, EET_T_INT);
   
   _edje_edd_edje_part_description = eet_data_descriptor_new("Edje_Part_Description",
							     sizeof(Edje_Part_Description),
							     evas_list_next,
							     evas_list_append,
							     evas_list_data,
							     evas_hash_foreach,
							     evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "state.name", state.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "state.value", state.value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "visible", visible, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.x", dragable.x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.step_x", dragable.step_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.count_x", dragable.count_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.y", dragable.y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.step_y", dragable.step_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.count_y", dragable.count_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "dragable.counfine_id", dragable.confine_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "clip_to_id", clip_to_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "align.x", align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "align.y", align.y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "min.w", min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "min.h", min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "max.w", max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "max.h", max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "step.x", step.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "step.y", step.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "aspect.min", aspect.min, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "aspect.max", aspect.max, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.relative_x", rel1.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.relative_y", rel1.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.offset_x", rel1.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.offset_y", rel1.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.id", rel1.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.relative_x", rel2.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.relative_y", rel2.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.offset_x", rel2.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.offset_y", rel2.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.id", rel2.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "image.id", image.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_description, Edje_Part_Description, "image.tween_list", image.tween_list, _edje_edd_edje_part_image_id);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.l", border.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.r", border.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.t", border.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.b", border.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_rel_x", fill.pos_rel_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_abs_x", fill.pos_abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.rel_x", fill.rel_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.abs_x", fill.abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_rel_y", fill.pos_rel_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_abs_y", fill.pos_abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.rel_y", fill.rel_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.abs_y", fill.abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.r", color.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.g", color.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.b", color.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.a", color.a, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.r", color2.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.g", color2.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.b", color2.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.a", color2.a, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.r", color3.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.g", color3.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.b", color3.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.a", color3.a, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.text", text.text, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.font", text.font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.size", text.size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.effect", text.effect, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.fit_x", text.fit_x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.fit_y", text.fit_y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.min_x", text.min_x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.min_y", text.min_y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.align.x", text.align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.align.y", text.align.y, EET_T_DOUBLE);
   
   _edje_edd_edje_part = eet_data_descriptor_new("Edje_Part",
						 sizeof(Edje_Part),
						 evas_list_next,
						 evas_list_append,
						 evas_list_data,
						 evas_hash_foreach,
						 evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "type", type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "mouse_events", mouse_events, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "color_class", color_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "text_class", text_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_part, Edje_Part, "default_desc", default_desc, _edje_edd_edje_part_description);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part, Edje_Part, "other_desc", other_desc, _edje_edd_edje_part_description);
   
   _edje_edd_edje_part_collection  = eet_data_descriptor_new("Edje_Part_Collection", 
							     sizeof(Edje_Part_Collection),
							     evas_list_next,
							     evas_list_append,
							     evas_list_data,
							     evas_hash_foreach,
							     evas_hash_add);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs", programs, _edje_edd_edje_program);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "parts", parts, _edje_edd_edje_part);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "id", id, EET_T_INT);
}

/* evas smart object methods - required by evas smart objects to do the */
/* dirty work on smrt objects */

static void
_edje_smart_add(Evas_Object * obj)
{
   Edje *ed;

   ed = _edje_add(obj);
   if (!ed) return;
   evas_object_smart_data_set(obj, ed);
   ed->obj = obj;
   evas_object_smart_member_add(ed->clipper, ed->obj);
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   _edje_del(ed);
}

static void
_edje_smart_layer_set(Evas_Object * obj, int layer)
{
   Edje *ed;
   Evas_List *l;
   char buf[256];
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (ed->layer == layer) return;
   ed->layer = layer;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_layer_set(ep->object, ed->layer);
     }
   snprintf(buf, sizeof(buf), "layer,set,%i", layer);
   _edje_emit(ed, buf, "");
}

static void
_edje_smart_raise(Evas_Object * obj)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_raise(ep->object);
     }
   _edje_emit(ed, "raise", "");
}

static void
_edje_smart_lower(Evas_Object * obj)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = evas_list_last(ed->parts); l; l = l->prev)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_lower(ep->object);
     }
   _edje_emit(ed, "lower", "");
}

static void 
_edje_smart_stack_above(Evas_Object * obj, Evas_Object * above)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = evas_list_last(ed->parts); l; l = l->prev)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_stack_above(ep->object, above);
     }
   _edje_emit(ed, "stack_above", "");
}

static void
_edje_smart_stack_below(Evas_Object * obj, Evas_Object * below)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_stack_below(ep->object, below);
     }
   _edje_emit(ed, "stack_below", "");
}

static void 
_edje_smart_move(Evas_Object * obj, double x, double y)
{
   Edje *ed;
   Evas_List *l;
   double xx, yy;
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
   evas_object_move(ed->clipper, ed->x, ed->y);
   
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_move(ep->object, ed->x + ep->x, ed->y + ep->y);
     }
   _edje_emit(ed, "move", "");
}

static void 
_edje_smart_resize(Evas_Object * obj, double w, double h)
{
   Edje *ed;
   int nw, nh;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   nw = ed->w;
   nh = ed->h;
   ed->w = w;
   ed->h = h;
   if ((nw == ed->w) && (nh == ed->h)) return;
   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = 1;
   _edje_recalc(ed);
   _edje_emit(ed, "resize", "");
}

static void 
_edje_smart_show(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_show(ed->clipper);
   _edje_emit(ed, "show", "");
}

static void 
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_hide(ed->clipper);
   _edje_emit(ed, "hide", "");
}

static void 
_edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_color_set(ed->clipper, r, g, b, a);
   _edje_emit(ed, "color_set", "");
}

static void 
_edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (evas_object_clip_get(obj) == clip) return;
   evas_object_clip_set(ed->clipper, clip);
   _edje_emit(ed, "clip_set", "");
}

static void 
_edje_smart_clip_unset(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (!evas_object_clip_get(obj)) return;
   evas_object_clip_unset(ed->clipper);
   _edje_emit(ed, "clip_unset", "");
}
