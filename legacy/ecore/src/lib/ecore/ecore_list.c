#include "ecore_private.h"
#include "Ecore.h"

void *
_ecore_list_append(void *in_list, void *in_item)
{
   Ecore_List *l, *new_l;
   Ecore_List *list, *item;
   
   list = in_list;
   item = in_item;
   new_l = item;
   new_l->next = NULL;
   if (!list) 
     {
	new_l->prev = NULL;
	new_l->last = new_l;
	return new_l;
     }
   if (list->last) l = list->last;
   else for (l = list; l; l = l->next);
   l->next = new_l;
   new_l->prev = l;
   list->last = new_l;
   return list;
}

void *
_ecore_list_prepend(void *in_list, void *in_item)
{
   Ecore_List *new_l;
   Ecore_List *list, *item;
   
   list = in_list;
   item = in_item;   
   new_l = item;
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
_ecore_list_append_relative(void *in_list, void *in_item, void *in_relative)
{
   Ecore_List *l;
   Ecore_List *list, *item, *relative;
   
   list = in_list;
   item = in_item;
   relative = in_relative;
   for (l = list; l; l = l->next)
     {
	if (l == relative)
	  {
	     Ecore_List *new_l;
	     
	     new_l = item;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }

	     else new_l->next = NULL;
	     l->next = new_l;
	     new_l->prev = l;
	     if (!new_l->next)
	       list->last = new_l;
	     return list;
	  }
     }
   return _ecore_list_append(list, item);
}

void *
_ecore_list_prepend_relative(void *in_list, void *in_item, void *in_relative)
{
   Ecore_List *l;
   Ecore_List *list, *item, *relative;
   
   list = in_list;
   item = in_item;
   relative = in_relative;
   for (l = list; l; l = l->next)
     {
	if (l == relative)
	  {
	     Ecore_List *new_l;
	     
	     new_l = item;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     l->prev = new_l;
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
     }
   return _ecore_list_prepend(list, item);
}

void *
_ecore_list_remove(void *in_list, void *in_item)
{
   Ecore_List *return_l;
   Ecore_List *list, *item;

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
_ecore_list_find(void *in_list, void *in_item)
{
   Ecore_List *l;
   Ecore_List *list, *item;
   
   list = in_list;
   item = in_item;   
   for (l = list; l; l = l->next)
     {
	if (l == item) return item;
     }
   return NULL;
}
