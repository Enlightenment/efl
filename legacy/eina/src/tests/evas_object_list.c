/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "Evas_Data.h"

/* list ops */
void *
evas_object_list_append(void *in_list, void *in_item)
{
   Evas_Object_List *l, *new_l;
   Evas_Object_List *list;

   list = in_list;
   new_l = in_item;
   new_l->next = NULL;
   if (!list)
     {
	new_l->prev = NULL;
	new_l->last = new_l;
	return new_l;
     }
   if (list->last) l = list->last;
   else for (l = list; (l) && (l->next); l = l->next);
   l->next = new_l;
   new_l->prev = l;
   list->last = new_l;
   return list;
}

void *
evas_object_list_prepend(void *in_list, void *in_item)
{
   Evas_Object_List *new_l;
   Evas_Object_List *list;

   list = in_list;
   new_l = in_item;
   new_l->prev = NULL;
   if (!list)
     {
	new_l->next = NULL;
	new_l->last = new_l;
	return new_l;
     }
   new_l->next = list;
   list->prev = new_l;
   new_l->last = list->last;
   list->last = NULL;
   return new_l;
}

void *
evas_object_list_append_relative(void *in_list, void *in_item, void *in_relative)
{
   Evas_Object_List *list, *relative, *new_l;

   list = in_list;
   new_l = in_item;
   relative = in_relative;
   if (relative)
     {
	if (relative->next)
	  {
	     new_l->next = relative->next;
	     relative->next->prev = new_l;
	  }
	else new_l->next = NULL;
	relative->next = new_l;
	new_l->prev = relative;
	if (!new_l->next) list->last = new_l;
	return list;
     }
   return evas_object_list_append(list, new_l);
}

void *
evas_object_list_prepend_relative(void *in_list, void *in_item, void *in_relative)
{
   Evas_Object_List *list, *relative, *new_l;

   list = in_list;
   new_l = in_item;
   relative = in_relative;
   if (relative)
     {
	new_l->prev = relative->prev;
	new_l->next = relative;
	relative->prev = new_l;
	if (new_l->prev)
	  {
	     new_l->prev->next = new_l;
	     if (!new_l->next)
	       list->last = new_l;
	     return list;
	  }
	else
	  {
	     if (!new_l->next)
	       new_l->last = new_l;
	     else
	       {
		  new_l->last = list->last;
		  list->last = NULL;
	       }
	     return new_l;
	  }
     }
   return evas_object_list_prepend(list, new_l);
}

void *
evas_object_list_remove(void *in_list, void *in_item)
{
   Evas_Object_List *return_l;
   Evas_Object_List *list, *item;

   /* checkme */
   if(!in_list)
     return in_list;

   list = in_list;
   item = in_item;
   if (!item) return list;
   if (item->next)
     item->next->prev = item->prev;
   if (item->prev)
     {
	item->prev->next = item->next;
	return_l = list;
     }
   else
     {
	return_l = item->next;
	if (return_l)
	  return_l->last = list->last;
     }
   if (item == list->last)
     list->last = item->prev;
   item->next = NULL;
   item->prev = NULL;
   return return_l;
}

void *
evas_object_list_find(void *in_list, void *in_item)
{
   Evas_Object_List *l;
   Evas_Object_List *list, *item;

   list = in_list;
   item = in_item;
   for (l = list; l; l = l->next)
     {
	if (l == item) return item;
     }
   return NULL;
}
