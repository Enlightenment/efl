/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

static void _edje_smart_add(Evas_Object * obj);
static void _edje_smart_del(Evas_Object * obj);
static void _edje_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _edje_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _edje_smart_show(Evas_Object * obj);
static void _edje_smart_hide(Evas_Object * obj);
static void _edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _edje_smart_clip_unset(Evas_Object * obj);

static Evas_Smart *_edje_smart = NULL;

Evas_List *_edje_edjes = NULL;

/************************** API Routines **************************/

/* FIXDOC: Verify/Expand */
/** Constructs the Edje object
 * @param evas A valid Evas handle
 * @return The Evas_Object pointer.
 *
 * Creates the Edje smart object, returning the Evas_Object handle.
 */
EAPI Evas_Object *
edje_object_add(Evas *evas)
{
   if (!_edje_smart)
     {
        static const Evas_Smart_Class sc =
	  {
	     "edje",
	       EVAS_SMART_CLASS_VERSION,
	       _edje_smart_add,
	       _edje_smart_del,
	       _edje_smart_move,
	       _edje_smart_resize,
	       _edje_smart_show,
	       _edje_smart_hide,
	       _edje_smart_color_set,
	       _edje_smart_clip_set,
	       _edje_smart_clip_unset,
	       NULL,
	       NULL
	  };
	_edje_smart = evas_smart_class_new(&sc);
     }
   return evas_object_smart_add(evas, _edje_smart);
}

/* Private Routines */
static void
_edje_smart_add(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_add(obj);
   if (!ed) return;
   evas_object_smart_data_set(obj, ed);
   evas_object_geometry_get(obj, &(ed->x), &(ed->y), &(ed->w), &(ed->h));
   ed->obj = obj;
   _edje_edjes = evas_list_append(_edje_edjes, obj);
   _edje_entry_init(ed);
/*
     {
	Evas_List *l;

	printf("--- EDJE DUMP [%i]\n", evas_list_count(_edje_edjes));
	for (l = _edje_edjes; l; l = l->next)
	  {
	     ed = _edje_fetch(l->data);
	     printf("EDJE: %80s | %80s\n", ed->path, ed->part);
	  }
	printf("--- EDJE DUMP [%i]\n", evas_list_count(_edje_edjes));
     }
 */
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   _edje_block_violate(ed);
   _edje_entry_shutdown(ed);
   ed->delete_me = 1;
   _edje_clean_objects(ed);
   _edje_edjes = evas_list_remove(_edje_edjes, obj);
   evas_object_smart_data_set(obj, NULL);
   if (_edje_script_only(ed)) _edje_script_only_shutdown(ed);
   _edje_file_del(ed);
   _edje_unref(ed);
}

static void
_edje_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   Edje *ed;
   int i;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
//   evas_object_move(ed->clipper, ed->x, ed->y);

   if (_edje_script_only(ed))
     {
	_edje_script_only_move(ed);
	return;
     }
   
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *ep;
	Evas_Coord ox, oy;

	ep = ed->table_parts[i];
	evas_object_geometry_get(ep->object, &ox, &oy, NULL, NULL);
	evas_object_move(ep->object, ed->x + ep->x + ep->offset.x, ed->y + ep->y +ep->offset.y);
	if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
	  _edje_entry_real_part_configure(ep);
	if (ep->swallowed_object)
	  {
	     evas_object_geometry_get(ep->swallowed_object, &ox, &oy, NULL, NULL);
	     evas_object_move(ep->swallowed_object, ed->x + ep->x + ep->offset.x, ed->y + ep->y +ep->offset.y);
	  }
	if (ep->extra_objects)
	  {
	     Evas_List *el;

	     for (el = ep->extra_objects; el; el = el->next)
	       {
		  Evas_Object *o;
		  Evas_Coord oox, ooy;

		  o = el->data;
		  evas_object_geometry_get(o, &oox, &ooy, NULL, NULL);
		  evas_object_move(o, ed->x + ep->x + ep->offset.x + (oox - ox), ed->y + ep->y + ep->offset.y + (ooy - oy));
	       }
	  }
     }
//   _edje_emit(ed, "move", NULL);
}

static void
_edje_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((w == ed->w) && (h == ed->h)) return;
   ed->w = w;
   ed->h = h;
   if (_edje_script_only(ed))
     {
	_edje_script_only_resize(ed);
	return;
     }
//   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = 1;
   _edje_recalc(ed);
   _edje_emit(ed, "resize", NULL);
}

static void
_edje_smart_show(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (evas_object_visible_get(ed->clipper)) return;
   if ((ed->collection) && (evas_object_clipees_get(ed->clipper)))
     evas_object_show(ed->clipper);
   if (_edje_script_only(ed))
     {  
	_edje_script_only_show(ed);
	return;
     }
   _edje_emit(ed, "show", NULL);
}

static void
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (!evas_object_visible_get(ed->clipper)) return;
   if ((ed->collection) && (evas_object_clipees_get(ed->clipper)))
     evas_object_hide(ed->clipper);
   if (_edje_script_only(ed))
     {  
	_edje_script_only_hide(ed);
	return;
     }
   _edje_emit(ed, "hide", NULL);
}

static void
_edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_color_set(ed->clipper, r, g, b, a);
//   _edje_emit(ed, "color_set", NULL);
}

static void
_edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (evas_object_clip_get(obj) == clip) return;
   evas_object_clip_set(ed->clipper, clip);
//   _edje_emit(ed, "clip_set", NULL);
}

static void
_edje_smart_clip_unset(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (!evas_object_clip_get(obj)) return;
   evas_object_clip_unset(ed->clipper);
//   _edje_emit(ed, "clip_unset", NULL);
}
