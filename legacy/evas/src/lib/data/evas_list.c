#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* list ops */
Evas_List *
evas_list_append(Evas_List *list, void *data)
{
   Evas_List *l, *new_l;
   
   new_l = malloc(sizeof(Evas_List));
   new_l->next = NULL;
   new_l->prev = NULL;
   new_l->data = data;
   if (!list) 
     {
	new_l->last = new_l;
	new_l->count = 1;
	return new_l;
     }
   if (list->last)
     {
	l = list->last;
	l->next = new_l;
	new_l->prev = l;
	list->last = new_l;
	list->count++;
	return list;	
     }
   else
     {
	for (l = list; l; l = l->next)
	  {
	     if (!l->next)
	       {
		  l->next = new_l;
		  new_l->prev = l;
		  list->last = new_l;
		  list->count++;
		  return list;
	       }
	  }
     }
   return list;
}

Evas_List *
evas_list_prepend(Evas_List *list, void *data)
{
   Evas_List *new_l;
   
   new_l = malloc(sizeof(Evas_List));
   new_l->prev = NULL;
   new_l->data = data;
   if (!list) 
     {
	new_l->next = NULL;
	new_l->last = new_l;
	new_l->count = 1;
	return new_l;
     }
   new_l->next = list;
   list->prev = new_l;
   new_l->last = list->last;
   list->last = NULL;
   new_l->count = list->count + 1;
   return new_l;
}

Evas_List *
evas_list_append_relative(Evas_List *list, void *data, void *relative)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List *new_l;
	     
	     new_l = malloc(sizeof(Evas_List));
	     new_l->data = data;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }
	     else
	       new_l->next = NULL;
	     
	     l->next = new_l;
	     new_l->prev = l;
	     if (!new_l->next) list->last = new_l;
	     list->count++;
	     return list;
	  }
     }
   return evas_list_append(list, data);
}

Evas_List *
evas_list_prepend_relative(Evas_List *list, void *data, void *relative)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List *new_l;
	     
	     new_l = malloc(sizeof(Evas_List));
	     new_l->data = data;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     if (l->prev) l->prev->next = new_l;
	     l->prev = new_l;
	     if (new_l->prev)
	       {
		  if (!new_l->next) list->last = new_l;
		  list->count++;
		  return list;
	       }
	     else
	       {
		  new_l->last = list->last;
		  list->last = NULL;
		  new_l->count = list->count + 1;
		  return new_l;
	       }
	  }
     }
   return evas_list_prepend(list, data);
}

Evas_List *
evas_list_remove(Evas_List *list, void *data)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data)
	  return evas_list_remove_list(list, l);
     }
   return list;
}

Evas_List *
evas_list_remove_list(Evas_List *list, Evas_List *remove_list)
{
   Evas_List *return_l;
   
   if (!remove_list) return list;
   if (remove_list->next) remove_list->next->prev = remove_list->prev;
   if (remove_list->prev)
     {
	remove_list->prev->next = remove_list->next;
	return_l = list;
     }
   else
     {
	if (remove_list->next)
	  remove_list->next->count = remove_list->count;
	return_l = remove_list->next;
	if (return_l) return_l->last = list->last;
     }
   if (remove_list == list->last) list->last = remove_list->prev;
   free(remove_list);
   if (return_l) return_l->count--;
   return return_l;
}

void *
evas_list_find(Evas_List *list, void *data)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data) return data;
     }
   return NULL;
}

Evas_List *
evas_list_free(Evas_List *list)
{
   Evas_List *l, *free_l;
   
   for (l = list; l;)
     {
	free_l = l;
	l = l->next;
	free(free_l);
     }
   return NULL;
}

Evas_List *
evas_list_last(Evas_List *list)
{
   if (!list) return NULL;
   return list->last;
}

Evas_List *
evas_list_next(Evas_List *list)
{
   if (!list) return NULL;
   return list->next;
}

Evas_List *
evas_list_prev(Evas_List *list)
{
   if (!list) return NULL;
   return list->prev;
}

void *
evas_list_data(Evas_List *list)
{
   if (!list) return NULL;
   return list->data;
}

int
evas_list_count(Evas_List *list)
{
   if (!list) return 0;
   return list->count;
}

void *
evas_list_nth(Evas_List *list, int n)
{
   int i;
   Evas_List *l;
   
   if (n < 0) return NULL;
   for (i = 0, l = list; l; l = l->next, i++)
     {
	if (i == n) return l->data;
     }
   return NULL;
}

Evas_List *
evas_list_reverse(Evas_List *list)
{
   Evas_List *new_l = NULL;
   
   while (list)
     {
	void *data;
	
	data = list->data;
	list = evas_list_remove_list(list, list);
	new_l = evas_list_prepend(new_l, data);
     }
   return new_l;
}
