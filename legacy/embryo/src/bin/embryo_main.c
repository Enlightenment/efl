#include "Embryo.h"

/* This is ugly code! don't look at it please! i am embarrassed! i need to */
/* cleanit up! */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>

/* debugging native calls */
static int dochar(Embryo_Program *ep, char ch, Embryo_Cell param);
static int doesc(Embryo_Program *ep, char ch, Embryo_Cell param);
static int printstring(Embryo_Program *ep, Embryo_Cell *cstr, Embryo_Cell *params, int num);

static int
dochar(Embryo_Program * ep, char ch, Embryo_Cell param)
{
   Embryo_Cell *cptr;
                                   
   switch (ch)
     {
      case '%':
	putchar(ch);
	return 0;
      case 'c':
	cptr = embryo_data_address_get(ep, param);
	if (cptr)
	  putchar((int) *cptr);
	return 1;
      case 'i':
      case 'd':
	cptr = embryo_data_address_get(ep, param);
	if (cptr)
	  printf("%i", (int) *cptr);
	return 1;
      case 'x':
	cptr = embryo_data_address_get(ep, param);
	if (cptr)
	  printf("%x", (unsigned int) *cptr);
	return 1;
      case 'f':
	cptr = embryo_data_address_get(ep, param);
	if (cptr)
	  printf("%f", (float)(*(float *)cptr));
	return 1;
      case 'X':
	cptr = embryo_data_address_get(ep, param);
	if (cptr)
	  printf("%08x", (unsigned int) *cptr);
	return 1;
      case 's':
	cptr = embryo_data_address_get(ep, param);
	if (cptr)
	  printstring(ep, cptr, NULL, 0);
	return 1;
     }   
   putchar(ch);
   return 0;
}

static int
doesc(Embryo_Program * ep, char ch, Embryo_Cell param)
{
   Embryo_Cell *cptr;
                                   
   switch (ch)
     {
      case 'n':
	putchar('\n');
	return 1;
      case 't':
	putchar('\t');
	return 1;
     }   
   putchar(ch);
   return 0;
}

static int
printstring(Embryo_Program * ep, Embryo_Cell *cstr, Embryo_Cell *params, int num)
{
   int i;
   int informat = 0, paramidx = 0, inesc = 0, len = 0;
   int j = sizeof(Embryo_Cell) - sizeof(char);
   char c;
   char *str;
      
   /* the string is packed */
   i = 0;
   len = embryo_data_string_length_get(ep, cstr);
   str = alloca(len + 1);
   embryo_data_string_get(ep, cstr, str);
   for (i = 0;; i++) {
      c = (char) (str[i]);
      if (c == 0)
	break;
      if (informat) {
	 paramidx += dochar(ep, c, params[paramidx]);
	 informat = 0;
      } else if (inesc) {
	 doesc(ep, c, params[paramidx]);
	 inesc = 0;
      } else if (params != NULL && c == '%') {
	 informat = 1;
      } else if (params != NULL && c == '\\') {
	 inesc = 1;
      } else {
	 putchar(c);
      }                                       /* if */
   }                                               /* for */
   return EMBRYO_ERROR_NONE;
}

static Embryo_Cell
exported_printf(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Cell *cptr;

   // params[0] = number of bytes params passed 
   cptr = embryo_data_address_get(ep, params[1]);
   printstring(ep, cptr, params + 2, (int) (params[0] / sizeof(Embryo_Cell)) - 1);
   fflush(stdout);
   return EMBRYO_ERROR_NONE;
}

static Embryo_Cell
exported_call(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Cell *cptr;
   
   // params[0] = number of bytes of params passed 
   cptr = embryo_data_address_get(ep, params[1]);
   if (cptr)
     {
	char buf[4096];
	
	buf[0] = 0;
	printf("..");
	embryo_data_string_get(ep, cptr, buf);
	printf("OUT: \"%s\"", buf);
     }
   cptr = embryo_data_address_get(ep, params[3]);
   if (cptr)
     {
	char buf[4096];
	
	buf[0] = 0;
	embryo_data_string_get(ep, cptr, buf);
	printf(" \"%s\"", buf);
     }
     {
	int v;
	
	v = params[3];
	printf(" %i", v);
     }
   printf("\n");
   return 10;
}

void
exit_error(Embryo_Program *ep, int errorcode)
{
  printf("Run time error %d: \"%s\"\n", 
	 errorcode, 
	 embryo_error_string_get(errorcode));
  exit(-1);
}

void PrintUsage(char *program)
{
  printf("Usage: %s <filename>\n", program);
  exit(1);
}

int
main(int argc,char *argv[])
{
   Embryo_Program *ep;
   Embryo_Cell val;
   Embryo_Function fn;
   int r;
   int err;

   if (argc != 2)
     {
	printf("Usage: %s <filename>\n", argv[0]);
	exit(-1);
     }
   ep = embryo_program_load(argv[1]);
   if (!ep)
     {
	printf("Cannot load %s\n", argv[1]);
	exit(-1);
     }
   embryo_program_native_call_add(ep, "call", exported_call);
   embryo_program_native_call_add(ep, "printf", exported_printf);

   embryo_program_vm_push(ep);
   val = embryo_program_variable_find(ep, "global1");
   if (val != EMBRYO_CELL_NONE)
     {
	Embryo_Cell *addr;
	
	addr = embryo_data_address_get(ep, val);
	if (addr) printf("Global variable value = %i\n", (int)*addr);
     }
   else
     printf("Cannot find variable\n");
	
   fn = embryo_program_function_find(ep, "testfn");
   if (fn != EMBRYO_FUNCTION_NONE)
     {
	printf("Found testfn()\n");
	embryo_parameter_cell_push(ep, 9876);
	embryo_parameter_string_push(ep, "K is a dirty fish");
	embryo_parameter_cell_push(ep, 127);
	while ((r = embryo_program_run(ep, fn)) == EMBRYO_PROGRAM_SLEEP);
	if (r == EMBRYO_PROGRAM_FAIL) printf("Run failed!\n");
     }
   else
     {
	printf("Runing main()\n");
	while ((r = embryo_program_run(ep, EMBRYO_FUNCTION_MAIN)) == EMBRYO_PROGRAM_SLEEP);
	if (r == EMBRYO_PROGRAM_FAIL) printf("Run failed!\n");
     }
   embryo_program_vm_pop(ep);
   
   err = embryo_program_error_get(ep);
   if (err != EMBRYO_ERROR_NONE) exit_error(ep, err);
   printf("Program %s returns %i\n", argv[1], embryo_program_return_value_get(ep));
   embryo_program_free(ep);
   return 0;
}
