#include "embryo_private.h"

/* exported args api */

static Embryo_Cell
_embryo_args_numargs(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Header *hdr;
   unsigned char *data;
   Embryo_Cell bytes;
   
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   bytes = *(Embryo_Cell *)(data + (int)ep->frm + 
			    (2 * sizeof(Embryo_Cell)));
   return bytes / sizeof(Embryo_Cell);
}

static Embryo_Cell
_embryo_args_getarg(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Header *hdr;
   unsigned char *data;
   Embryo_Cell val;
   
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   val = *(Embryo_Cell *)(data + (int)ep->frm + 
			  (((int)params[1] + 3) * sizeof(Embryo_Cell)));
   val += params[2] * sizeof(Embryo_Cell);
   val = *(Embryo_Cell *)(data + (int)val);
   return val;
}

static Embryo_Cell
_embryo_args_setarg(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Header *hdr;
   unsigned char *data;
   Embryo_Cell val;
   
   if (params[0] != (3 * sizeof(Embryo_Cell))) return 0;
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   val = *(Embryo_Cell *)(data + (int)ep->frm + 
			  (((int)params[1] + 3) * sizeof(Embryo_Cell)));
   val += params[2] * sizeof(Embryo_Cell);
   if ((val < 0) || (val >= ep->hea) && (val < ep->stk)) return 0;
   *(Embryo_Cell *)(data + (int)val) = params[3];
   return 1;
}

/* functions used by the rest of embryo */

void
_embryo_args_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "numargs",  _embryo_args_numargs);
   embryo_program_native_call_add(ep, "getarg", _embryo_args_getarg);
   embryo_program_native_call_add(ep, "setarg", _embryo_args_setarg);
}
