#include "Edje.h"
#include "edje_private.h"

void
_edje_var_init(Edje *ed)
{
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   ed->var_pool = calloc(1, sizeof(Edje_Var_Pool));
   if (!ed->var_pool) return;
   ed->var_pool->size = embryo_program_variable_count_get(ed->collection->script);
   ed->var_pool->vars = calloc(1, sizeof(Edje_Var) * ed->var_pool->size);
}

void
_edje_var_shutdown(Edje *ed)
{
   if (!ed->var_pool) return;
   if (ed->var_pool->vars)
     {
	int i;
	
	for (i = 0; i < ed->var_pool->size; i++)
	  {
	     if (ed->var_pool->vars[i].type == EDJE_VAR_STRING)
	       {
		  if (ed->var_pool->vars[i].data.s.v)
		    {
		       free(ed->var_pool->vars[i].data.s.v);
		       ed->var_pool->vars[i].data.s.v = NULL;
		    }
	       }
	  }
	free(ed->var_pool->vars);
     }
   free(ed->var_pool);
   ed->var_pool = NULL;
}

int
_edje_var_string_id_get(Edje *ed, char *string)
{
   Embryo_Cell cell, *cptr;
   
   if (!ed) return 0;
   if (!ed->collection) return 0;
   if (!ed->collection->script) return 0;
   if (!string) return;
   cell = embryo_program_variable_find(ed->collection->script, string);
   if (cell == EMBRYO_CELL_NONE) return  0;
   cptr = embryo_data_address_get(ed->collection->script, cell);
   if (!cptr) return 0;
   return (int)(*cptr);
}

int
_edje_var_int_get(Edje *ed, int id)
{
   if (!ed) return 0;
   if (!ed->var_pool) return 0;
   id -= EDJE_VAR_MAGIC_BASE;
   if ((id < 0) || (id >= ed->var_pool->size)) return 0;
   /* auto-cast */
   if (ed->var_pool->vars[id].type == EDJE_VAR_STRING)
     {
	if (ed->var_pool->vars[id].data.s.v)
	  {
	     double f;
	
	     f = atof(ed->var_pool->vars[id].data.s.v);
	     free(ed->var_pool->vars[id].data.s.v);
	     ed->var_pool->vars[id].data.s.v = NULL;
	     ed->var_pool->vars[id].data.i.v = (int)f;
	  }
	ed->var_pool->vars[id].type = EDJE_VAR_INT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_FLOAT)
     {
	int v;
	
	v = (int)(ed->var_pool->vars[id].data.f.v);
	ed->var_pool->vars[id].type = EDJE_VAR_INT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_NONE)
     {
	ed->var_pool->vars[id].type = EDJE_VAR_INT;
     }
   return ed->var_pool->vars[id].data.i.v;
}

void
_edje_var_int_set(Edje *ed, int id, int v)
{
   if (!ed) return;
   if (!ed->var_pool) return;
   id -= EDJE_VAR_MAGIC_BASE;
   if ((id < 0) || (id >= ed->var_pool->size)) return;
   /* auto-cast */
   if (ed->var_pool->vars[id].type == EDJE_VAR_STRING)
     {
	if (ed->var_pool->vars[id].data.s.v)
	  {
	     free(ed->var_pool->vars[id].data.s.v);
	     ed->var_pool->vars[id].data.s.v = NULL;
	  }
	ed->var_pool->vars[id].type = EDJE_VAR_INT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_FLOAT)
     {
	ed->var_pool->vars[id].data.f.v = 0;
	ed->var_pool->vars[id].type = EDJE_VAR_INT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_NONE)
     {
	ed->var_pool->vars[id].type = EDJE_VAR_INT;
     }
   ed->var_pool->vars[id].data.i.v = v;
}

double
_edje_var_float_get(Edje *ed, int id)
{
   if (!ed) return 0;
   if (!ed->var_pool) return 0;
   id -= EDJE_VAR_MAGIC_BASE;
   if ((id < 0) || (id >= ed->var_pool->size)) return 0;
   /* auto-cast */
   if (ed->var_pool->vars[id].type == EDJE_VAR_STRING)
     {
	if (ed->var_pool->vars[id].data.s.v)
	  {
	     double f;
	
	     f = atof(ed->var_pool->vars[id].data.s.v);
	     free(ed->var_pool->vars[id].data.s.v);
	     ed->var_pool->vars[id].data.s.v = NULL;
	     ed->var_pool->vars[id].data.f.v = f;
	  }
	ed->var_pool->vars[id].type = EDJE_VAR_FLOAT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_INT)
     {
	int v;
	
	v = (int)(ed->var_pool->vars[id].data.f.v);
	ed->var_pool->vars[id].type = EDJE_VAR_FLOAT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_NONE)
     {
	ed->var_pool->vars[id].type = EDJE_VAR_FLOAT;
     }
   return ed->var_pool->vars[id].data.f.v;
}

void
_edje_var_float_set(Edje *ed, int id, double v)
{
   if (!ed) return;
   if (!ed->var_pool) return;
   id -= EDJE_VAR_MAGIC_BASE;
   if ((id < 0) || (id >= ed->var_pool->size)) return;
   /* auto-cast */
   if (ed->var_pool->vars[id].type == EDJE_VAR_STRING)
     {
	if (ed->var_pool->vars[id].data.s.v)
	  {
	     free(ed->var_pool->vars[id].data.s.v);
	     ed->var_pool->vars[id].data.s.v = NULL;
	  }
	ed->var_pool->vars[id].type = EDJE_VAR_FLOAT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_INT)
     {
	ed->var_pool->vars[id].data.f.v = 0;
	ed->var_pool->vars[id].type = EDJE_VAR_FLOAT;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_NONE)
     {
	ed->var_pool->vars[id].type = EDJE_VAR_FLOAT;
     }
   ed->var_pool->vars[id].data.f.v = v;
}

const char *
_edje_var_str_get(Edje *ed, int id)
{
   if (!ed) return NULL;
   if (!ed->var_pool) return NULL;
   id -= EDJE_VAR_MAGIC_BASE;
   if ((id < 0) || (id >= ed->var_pool->size)) return NULL;
   /* auto-cast */
   if (ed->var_pool->vars[id].type == EDJE_VAR_INT)
     {
	char buf[64];
	
	snprintf(buf, sizeof(buf), "%i", ed->var_pool->vars[id].data.i.v);
	ed->var_pool->vars[id].data.s.v = strdup(buf);
	ed->var_pool->vars[id].type = EDJE_VAR_STRING;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_FLOAT)
     {
	char buf[64];
	
	snprintf(buf, sizeof(buf), "%f", ed->var_pool->vars[id].data.f.v);
	ed->var_pool->vars[id].data.s.v = strdup(buf);
	ed->var_pool->vars[id].type = EDJE_VAR_STRING;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_NONE)
     {
	ed->var_pool->vars[id].data.s.v = strdup("");
	ed->var_pool->vars[id].type = EDJE_VAR_STRING;
     }
   return ed->var_pool->vars[id].data.s.v;
}

void
_edje_var_str_set(Edje *ed, int id, char *str)
{
   if (!ed) return;
   if (!ed->var_pool) return;
   if (!str) return;
   id -= EDJE_VAR_MAGIC_BASE;
   if ((id < 0) || (id >= ed->var_pool->size)) return;
   /* auto-cast */
   if (ed->var_pool->vars[id].type == EDJE_VAR_STRING)
     {
	if (ed->var_pool->vars[id].data.s.v)
	  {
	     free(ed->var_pool->vars[id].data.s.v);
	     ed->var_pool->vars[id].data.s.v = NULL;
	  }
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_INT)
     {
	ed->var_pool->vars[id].data.f.v = 0;
	ed->var_pool->vars[id].type = EDJE_VAR_STRING;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_FLOAT)
     {
	ed->var_pool->vars[id].data.f.v = 0;
	ed->var_pool->vars[id].type = EDJE_VAR_STRING;
     }
   else if (ed->var_pool->vars[id].type == EDJE_VAR_NONE)
     {
	ed->var_pool->vars[id].type = EDJE_VAR_STRING;
     }
   ed->var_pool->vars[id].data.s.v = strdup(str);
}

