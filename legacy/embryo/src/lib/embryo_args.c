/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "embryo_private.h"

#define STRSET(ep, par, str) { \
   Embryo_Cell *___cptr; \
   if ((___cptr = embryo_data_address_get(ep, par))) { \
      embryo_data_string_set(ep, str, ___cptr); \
   } }

/* exported args api */

static Embryo_Cell
_embryo_args_numargs(Embryo_Program *ep, Embryo_Cell *params __UNUSED__)
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
   if ((val < 0) || ((val >= ep->hea) && (val < ep->stk))) return 0;
   *(Embryo_Cell *)(data + (int)val) = params[3];
   return 1;
}

static Embryo_Cell
_embryo_args_getsarg(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Header *hdr;
   unsigned char *data;
   Embryo_Cell base_cell;
   char *s;
   int i = 0;

   /* params[1] = arg_no */
   /* params[2] = buf */
   /* params[3] = buflen */
   if (params[0] != (3 * sizeof(Embryo_Cell))) return 0;
   if (params[3] <= 0) return 0; /* buflen must be > 0 */
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   base_cell = *(Embryo_Cell *)(data + (int)ep->frm +
			  (((int)params[1] + 3) * sizeof(Embryo_Cell)));

   s = alloca(params[3]);

   while (i < params[3])
     {
	int offset = base_cell + (i * sizeof(Embryo_Cell));

	s[i] = *(Embryo_Cell *)(data + offset);
	if (!s[i++]) break;
     }

   s[i - 1] = 0;
   STRSET(ep, params[2], s);

   return i - 1; /* characters written minus terminator */
}

/* functions used by the rest of embryo */

void
_embryo_args_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "numargs",  _embryo_args_numargs);
   embryo_program_native_call_add(ep, "getarg", _embryo_args_getarg);
   embryo_program_native_call_add(ep, "setarg", _embryo_args_setarg);
   embryo_program_native_call_add(ep, "getfarg", _embryo_args_getarg);
   embryo_program_native_call_add(ep, "setfarg", _embryo_args_setarg);
   embryo_program_native_call_add(ep, "getsarg", _embryo_args_getsarg);
}
