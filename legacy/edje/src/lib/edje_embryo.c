#include "Edje.h"
#include "edje_private.h"

#define CHKPARAM(n) if (params[0] != (sizeof(Embryo_Cell) * (n))) return 0;

/**** All the api exported to edje scripts ****/
/* tst() */
static Embryo_Cell
_edje_embryo_fn_tst(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   
   /* params[0] = number of bytes of params passed */
   ed = embryo_program_data_get(ep);
   printf("EDJE DEBUG: Embryo code detected for \"%s\":\"%s\"\n",
	  ed->path, ed->part);
   return 7;
}

/* emit(sig[], src[]) */
static Embryo_Cell
_edje_embryo_fn_emit(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Embryo_Cell *cptr;
   char *sig, *src;
   int l;
   
   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   
   cptr = embryo_data_address_get(ep, params[1]);
   l = embryo_data_string_length_get(ep, cptr);
   sig = alloca(l + 1);
   embryo_data_string_get(ep, cptr, sig);
   
   cptr = embryo_data_address_get(ep, params[2]);
   l = embryo_data_string_length_get(ep, cptr);
   src = alloca(l + 1);
   embryo_data_string_get(ep, cptr, src);
   
   _edje_emit(ed, sig, src);
   return 0;
}

void
_edje_embryo_script_init(Edje *ed)
{
   Embryo_Program *ep;
  
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   ep = ed->collection->script;
   embryo_program_data_set(ep, ed);
   /* first advertise all the edje "script" calls */
   embryo_program_native_call_add(ep, "tst", _edje_embryo_fn_tst);
   embryo_program_native_call_add(ep, "emit", _edje_embryo_fn_emit);
   
   embryo_program_vm_push(ep); /* neew a new vm to run in */
   /* by default always call main() to init stuff */
   if (embryo_program_run(ep, EMBRYO_FUNCTION_MAIN) != EMBRYO_PROGRAM_OK)
     /* FIXME: debugging hack!!!! */
     {
	printf("EDJE DEBUG: Run of main() failed. Reason:\n");
	printf("%s\n", embryo_error_string_get(embryo_program_error_get(ep)));
     }
   else
     {
	printf("EDJE DEBUG: main() returned %i\n",
	       embryo_program_return_value_get(ep));
     }
}

void
_edje_embryo_script_shutdown(Edje *ed)
{
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   if (embryo_program_recursion_get(ed->collection->script) > 0) return;
   embryo_program_vm_pop(ed->collection->script);
   embryo_program_free(ed->collection->script);
   ed->collection->script = NULL;
}

void
_edje_embryo_script_reset(Edje *ed)
{
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   if (embryo_program_recursion_get(ed->collection->script) > 0) return;
   embryo_program_vm_reset(ed->collection->script);
}
