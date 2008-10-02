#include <Elementary.h>
#include "elm_priv.h"

static void _elm_scroller_del(Elm_Scroller *sc);
    
Elm_Scroller_Class _elm_scroller_class =
{
   &_elm_widget_class,
     ELM_OBJ_SCROLLER
};

static void
_elm_scroller_del(Elm_Scroller *sc)
{
   evas_object_del(sc->scroller_pan);
   ((Elm_Obj_Class *)(((Elm_Scroller_Class *)(sc->clas))->parent))->del(ELM_OBJ(sc));
}

static void
_elm_scroller_geom_set(Elm_Scroller *sc, int x, int y, int w, int h)
{
   Evas_Coord vw, vh;

   printf("sc geom set %ix%i (am %ix%i)\n", w, h, sc->w, sc->h);
   if ((sc->w != w) || (sc->h != h) || (sc->x != x) || (sc->y != y))
     {
	Evas_List *l;
	int tries = 0;
	
	((Elm_Widget_Class *)(((Elm_Scroller_Class *)(sc->clas))->parent))->geom_set(sc, x, y, w, h);
	again:
	tries++;
	elm_smart_scroller_child_viewport_size_get(sc->base, &vw, &vh);
	for (l = sc->children; l; l = l->next)
	  {
	     if (((Elm_Obj *)(l->data))->hastype(l->data, ELM_OBJ_WIDGET))
	       {
		  ((Elm_Widget *)(l->data))->size_alloc(l->data, vw, vh);
		  ((Elm_Widget *)(l->data))->geom_set(l->data, 
						      ((Elm_Widget *)(l->data))->x,
						      ((Elm_Widget *)(l->data))->y,
						      ((Elm_Widget *)(l->data))->req.w,
						      ((Elm_Widget *)(l->data))->req.h);
// FIXME: if scrollbars affect viewport size then we get an on/off effect of
// resizing child up and down. we need to find a way to avoid this. this tries
// this is a hack - but works.
		  if ((tries == 1) &&
		      (((vw == ((Elm_Widget *)(l->data))->req.w) ||
			(vh == ((Elm_Widget *)(l->data))->req.h)))) goto again;
	       }
	  }
     }
}

static void
_elm_on_child_add(void *data, Elm_Scroller *sc, Elm_Cb_Type type, Elm_Obj *obj)
{
   Evas_Coord vw, vh;
   
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   elm_smart_scroller_child_set(sc->base, ((Elm_Widget *)(obj))->base);
   elm_smart_scroller_child_viewport_size_get(sc->base, &vw, &vh);
   ((Elm_Widget *)(obj))->size_alloc(obj, vw, vh);
   ((Elm_Widget *)(obj))->geom_set(obj, 
				   ((Elm_Widget *)(obj))->x,
				   ((Elm_Widget *)(obj))->y,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
}

static void
_elm_on_child_del(void *data, Elm_Scroller *sc, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   elm_smart_scroller_child_set(sc->base, NULL);
}

EAPI Elm_Scroller *
elm_scroller_new(Elm_Win *win)
{
   Elm_Scroller *sc;
   
   sc = ELM_NEW(Elm_Scroller);
   _elm_widget_init(sc);
   
   sc->clas = &_elm_scroller_class;
   sc->type = ELM_OBJ_SCROLLER;
   
   sc->del = _elm_scroller_del;
   
   sc->geom_set = _elm_scroller_geom_set;

   sc->base = elm_smart_scroller_add(win->evas);
   
   sc->cb_add(sc, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   sc->cb_add(sc, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(sc);
   win->child_add(win, sc);
   return sc;
}
