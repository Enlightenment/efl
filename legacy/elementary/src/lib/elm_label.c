#include <Elementary.h>
#include "elm_priv.h"

static void _elm_label_text_set(Elm_Label *lb, const char *text);

Elm_Label_Class _elm_label_class =
{
   &_elm_widget_class,
     ELM_OBJ_LABEL,
     _elm_label_text_set
};

static void
_elm_label_text_set(Elm_Label *lb, const char *text)
{
   Evas_Coord mw, mh;
   
   if (lb->text) evas_stringshare_del(lb->text);
   if (text) lb->text = evas_stringshare_add(text);
   else lb->text = NULL;
   edje_object_part_text_set(lb->base, "elm.text", text);
   edje_object_size_min_calc(lb->base, &mw, &mh);
   if ((lb->minw != mw) || (lb->minh != mh))
     {
	lb->minw = mw;
	lb->minh = mh;
	((Elm_Widget *)(lb->parent))->size_req(lb->parent, lb, lb->minw, lb->minh);
	lb->geom_set(lb, lb->x, lb->y, lb->minw, lb->minh);
     }
}

static void
_elm_label_size_alloc(Elm_Label *lb, int w, int h)
{
   lb->req.w = lb->minw;
   lb->req.h = lb->minh;
}

static void
_elm_label_del(Elm_Label *lb)
{
   if (lb->text) evas_stringshare_del(lb->text);
   ((Elm_Obj_Class *)(((Elm_Label_Class *)(lb->clas))->parent))->del(ELM_OBJ(lb));
}

EAPI Elm_Label *
elm_label_new(Elm_Win *win)
{
   Elm_Label *lb;
   
   lb = ELM_NEW(Elm_Label);
   
   _elm_widget_init(lb);
   lb->clas = &_elm_label_class;
   lb->type = ELM_OBJ_LABEL;
   
   lb->del = _elm_label_del;

   lb->size_alloc = _elm_label_size_alloc;
   
   lb->text_set = _elm_label_text_set;

   lb->base = edje_object_add(win->evas);
   _elm_theme_set(lb->base, "label", "label");
   _elm_widget_post_init(lb);
   win->child_add(win, lb);
   return lb;
}
