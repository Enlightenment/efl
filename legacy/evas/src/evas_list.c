#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* list ops */
Evas_List
evas_list_append(Evas_List list, void *data)
{
   Evas_List l, new_l;
   
   new_l = malloc(sizeof(struct _Evas_List));
   new_l->next = NULL;
   new_l->prev = NULL;
   new_l->data = data;
   if (!list) return new_l;
   for (l = list; l; l = l->next)
     {
	if (!l->next)
	  {
	     l->next = new_l;
	     new_l->prev = l;
	     return list;
	  }
     }
   return list;
}

Evas_List
evas_list_prepend(Evas_List list, void *data)
{
   Evas_List new_l;
   
   new_l = malloc(sizeof(struct _Evas_List));
   new_l->next = NULL;
   new_l->prev = NULL;
   new_l->data = data;
   if (!list) return new_l;
   new_l->next = list;
   list->prev = new_l;
   return new_l;
}

Evas_List
evas_list_append_relative(Evas_List list, void *data, void *relative)
{
   Evas_List l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List new_l;
	     
	     new_l = malloc(sizeof(struct _Evas_List));
	     new_l->next = NULL;
	     new_l->prev = NULL;
	     new_l->data = data;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }
	     l->next = new_l;
	     new_l->prev = l;
	     return list;
	  }
     }
   return evas_list_append(list, data);
}

Evas_List
evas_list_prepend_relative(Evas_List list, void *data, void *relative)
{
   Evas_List l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List new_l;
	     
	     new_l = malloc(sizeof(struct _Evas_List));
	     new_l->next = NULL;
	     new_l->prev = NULL;
	     new_l->data = data;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     if (l->prev)
		l->prev->next = new_l;
	     l->prev = new_l;
	     if (new_l->prev)
		return list;
	     else
		return new_l;
	  }
     }
   return evas_list_prepend(list, data);
}

Evas_List
evas_list_remove(Evas_List list, void *data)
{
   Evas_List l, return_l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data)
	  {
	     if (l->next)
		l->next->prev = l->prev;
	     if (l->prev)
	       {
		  l->prev->next = l->next;
		  return_l = l->prev;
	       }
	     else
		return_l = l->next;
	     free(l);
	     return return_l;
	  }
     }
   return list;
}

void *
evas_list_find(Evas_List list, void *data)
{
   Evas_List l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data) return data;
     }
   return NULL;
}

Evas_List
evas_list_free(Evas_List list)
{
   Evas_List l, free_l;
   
   for (l = list; l;)
     {
	free_l = l;
	l = l->next;
	free(free_l);
     }
   return NULL;
}

