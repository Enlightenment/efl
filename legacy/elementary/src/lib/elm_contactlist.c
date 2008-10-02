#include <Elementary.h>
#include "elm_priv.h"

static void _elm_contactlist_del(Elm_Contactlist *cl);
    
Elm_Contactlist_Class _elm_contactlist_class =
{
   &_elm_widget_class,
     ELM_OBJ_CONTACTLIST
};

static void
_elm_contactlist_del(Elm_Contactlist *cl)
{
   // custom here
   ((Elm_Obj_Class *)(((Elm_Contactlist_Class *)(cl->clas))->parent))->del(ELM_OBJ(cl));
}

static void
_elm_contactlist_geom_set(Elm_Contactlist *cl, int x, int y, int w, int h)
{
   cl->box->geom_set(cl->box, x, y, w, h);
}

static void
_elm_on_child_add(void *data, Elm_Contactlist *cl, Elm_Cb_Type type, Elm_Obj *obj)
{
   Evas_Coord vw, vh;
   
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   // custom here (eg pack to box)
}

static void
_elm_on_child_del(void *data, Elm_Contactlist *cl, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   // dunno
}

static void
_elm_contactlist_show(Elm_Contactlist *cl)
{
   cl->box->show(cl->box);
}

static void
_elm_contactlist_hide(Elm_Contactlist *cl)
{
   cl->box->hide(cl->box);
}

static void
on_button_activate(Elm_Contactlist *cl, Elm_Button *bt, Elm_Cb_Type type, void *info)
{
   printf("Add contact\n");
}


EAPI Elm_Contactlist *
elm_contactlist_new(Elm_Win *win)
{
   Elm_Contactlist *cl;
   
   cl = ELM_NEW(Elm_Contactlist);
   _elm_widget_init(cl);
   
   cl->clas = &_elm_contactlist_class;
   cl->type = ELM_OBJ_CONTACTLIST;
   
   cl->del = _elm_contactlist_del;
   
   cl->geom_set = _elm_contactlist_geom_set;
   cl->show = _elm_contactlist_show;
   cl->hide = _elm_contactlist_hide;

   cl->box = elm_box_new(win);
   cl->child_add(cl, cl->box);
   
   cl->listbox = elm_box_new(win);
   printf("--listbox = %p\n", cl->listbox);
   
   cl->scroller = elm_scroller_new(win);
   cl->box->pack_end(cl->box, cl->scroller);
   
   cl->button = elm_button_new(win);
   cl->button->text_set(cl->button, "Add Contact");
   cl->button->cb_add(cl->button, ELM_CB_ACTIVATED, on_button_activate, cl);
   cl->button->expand_y = 0;
   cl->box->pack_end(cl->box, cl->button);
   
   cl->button->show(cl->button);
   
   int i;
   for (i = 0; i < 5; i++)
     {
	Elm_Button *bt;
	
	bt = elm_button_new(win);
	bt->text_set(bt, "Contact 1");
	bt->expand_y = 0;
	cl->listbox->pack_end(cl->listbox, bt);
	bt->show(bt);
     }

   cl->scroller->child_add(cl->scroller, cl->listbox);
   elm_widget_sizing_update(cl->listbox);
   cl->scroller->show(cl->scroller);
   cl->listbox->show(cl->listbox);
   
   elm_widget_sizing_update(cl->scroller);
   elm_widget_sizing_update(cl->button);
   elm_widget_sizing_update(cl->box);
   
   cl->base = cl->box->base;
   
   cl->cb_add(cl, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   cl->cb_add(cl, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(cl);
   win->child_add(win, cl);
   return cl;
}
