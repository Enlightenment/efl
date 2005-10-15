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
#include <alloca.h>

/* debugging native calls */
static int process_format_char(Embryo_Program *ep, char ch, Embryo_Cell param);
static int process_escape_char(Embryo_Program *ep, char ch);
static int custom_printf(Embryo_Program *ep, Embryo_Cell *cstr, Embryo_Cell *params, int num);

static Embryo_Cell exported_printf(Embryo_Program *ep, Embryo_Cell *params);

static int
process_format_char(Embryo_Program * ep, char ch, Embryo_Cell param)
{
   Embryo_Cell *cptr;
                                   
   switch (ch)
     {
      case '%':
	putchar(ch);
	return 0;
      case 'c':
	cptr = embryo_data_address_get(ep, param);
	if (cptr) putchar((int) *cptr);
	return 1;
      case 'i':
      case 'd':
	cptr = embryo_data_address_get(ep, param);
	if (cptr) printf("%i", (int) *cptr);
	return 1;
      case 'x':
	cptr = embryo_data_address_get(ep, param);
	if (cptr) printf("%x", (unsigned int) *cptr);
	return 1;
      case 'f':
	cptr = embryo_data_address_get(ep, param);
	if (cptr) printf("%f", (float)(*(float *)cptr));
	return 1;
      case 'X':
	cptr = embryo_data_address_get(ep, param);
	if (cptr) printf("%08x", (unsigned int) *cptr);
	return 1;
      case 's':
	cptr = embryo_data_address_get(ep, param);
	if (cptr) custom_printf(ep, cptr, NULL, 0);
	return 1;
     }
   putchar(ch);
   return 0;
}

static int
process_escape_char(Embryo_Program * ep, char ch)
{
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
custom_printf(Embryo_Program * ep, Embryo_Cell *cstr, Embryo_Cell *params, int num)
{
   int i;
   int informat = 0, paramidx = 0, inesc = 0, len = 0;
   char c;
   char *str;
      
   i = 0;
   len = embryo_data_string_length_get(ep, cstr);
   str = alloca(len + 1);
   embryo_data_string_get(ep, cstr, str);
   for (i = 0;; i++)
     {
	c = (char) (str[i]);
	if (c == 0)
	  break;
	if (informat)
	  {
	     paramidx += process_format_char(ep, c, params[paramidx]);
	     informat = 0;
	  }
	else if (inesc)
	  {
	     process_escape_char(ep, c);
	     inesc = 0;
	  }
	else if ((params != NULL) && (c == '%'))
	  {
	     informat = 1;
	  }
	else if ((params != NULL) && (c == '\\'))
	  {
	     inesc = 1;
	  }
	else
	  {
	     putchar(c);
	  }
     }
   return EMBRYO_ERROR_NONE;
}

static Embryo_Cell
exported_printf(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Cell *cptr;

   cptr = embryo_data_address_get(ep, params[1]);
   custom_printf(ep, cptr, params + 2, (int) (params[0] / sizeof(Embryo_Cell)) - 1);
   return EMBRYO_ERROR_NONE;
}

static Embryo_Cell
exported_rec(Embryo_Program *ep, Embryo_Cell *params)
{
   Embryo_Function fn;
   Embryo_Cell ret = 0, arg1, arg2;
   
   // params[0] = number of bytes of params passed 
   if (params[0] != 3 * sizeof(Embryo_Cell)) return -1;
   arg1 = params[1];
   arg2 = params[3];
   arg1++;

   if (arg1 > 7)
     {
	printf("arg1 == %i\n", arg1);
	return arg1;
     }
   
   // call the rec1 again
   fn = embryo_program_function_find(ep, "rec1");
   if (fn != EMBRYO_FUNCTION_NONE)
     {
	char buf[128];
	
	printf("... recurse!\n");
	snprintf(buf, sizeof(buf), "SMELLY %i", arg1);
	embryo_parameter_cell_push(ep, arg1);
	embryo_parameter_string_push(ep, buf);
	embryo_parameter_cell_push(ep, arg2);
	while (embryo_program_run(ep, fn) == EMBRYO_PROGRAM_SLEEP);
	ret = embryo_program_return_value_get(ep);
     }
   return ret + 1;
}

/* another example native call */
/*
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
*/

int
main(int argc,char *argv[])
{
   Embryo_Program *ep;
   Embryo_Function fn;
   int i;
   int r = EMBRYO_PROGRAM_OK;
   int err;
   int args = 0;
   int instruct = 0;
   char *file = NULL;
   char *func = NULL;

   embryo_init();
   for (i = 1; i < argc; i++)
     {
	if (argv[i][0] != '-')
	  {
	     file = argv[i];
	     if (i < (argc - 1)) args = i + 1;
	     break;
	  }
	else if (!strcmp(argv[i], "-func"))
	  {
	     if (i < (argc - 1))
	       {
		  i++;
		  func = argv[i];
	       }
	  }
     }
   if ((argc < 2) || (!file))
     {
	printf("Usage: %s [options] <filename> [parameters]\n"
	       "Where options can be any of:\n"
	       "\t-func name         execute function \"name\" to start\n"
	       , argv[0]);
	exit(-1);
     }
   ep = embryo_program_load(file);
   if (!ep)
     {
	printf("Cannot load %s\n", file);
	exit(-1);
     }
   embryo_program_native_call_add(ep, "printf", exported_printf);
   embryo_program_native_call_add(ep, "native_rec", exported_rec);
   embryo_program_vm_push(ep);

   if (args > 0)
     {
	for (i = args; i < argc; i++)
	  {
	     if (argv[i][0] == 's')
		  embryo_parameter_string_push(ep, argv[i] + 1);
	     else if (argv[i][0] == 'i')
	       {
		  Embryo_Cell n;
		  
		  n = atoi(argv[i] + 1);
		  embryo_parameter_cell_push(ep, n);
	       }
	     else if (argv[i][0] == 'f')
	       {
		  float n;
		  Embryo_Cell c;
		  
		  n = atof(argv[i] + 1);
		  c = EMBRYO_FLOAT_TO_CELL(n);
		  embryo_parameter_cell_push(ep, c);
	       }
	  }
     }
   r = EMBRYO_PROGRAM_OK;
   fn = EMBRYO_FUNCTION_MAIN;
   if (func)
     {
	fn = embryo_program_function_find(ep, func);
	if (fn == EMBRYO_FUNCTION_NONE)
	  {
	     printf("Unable to find public function %s()\n"
		    "Executing main() instead\n", func);
	     fn = EMBRYO_FUNCTION_MAIN;
	  }
     }
   if (!getenv("NOMAX"))
     embryo_program_max_cycle_run_set(ep, 100000000);
   for (;;)
     {
	r = embryo_program_run(ep, fn);
	if (r == EMBRYO_PROGRAM_SLEEP)
	  {
	     fn = EMBRYO_FUNCTION_CONT;
	     printf("SLEEP INSTRUCTION!\n");
	     continue;
	  }
	else if (r == EMBRYO_PROGRAM_TOOLONG)
	  {
	     fn = EMBRYO_FUNCTION_CONT;
	     instruct++;
	     printf("Executed %i00 million instructions!\n", instruct);
	     continue;
	  }
	else
	  break;
     }
   embryo_program_vm_pop(ep);
   if (r == EMBRYO_PROGRAM_FAIL)
     {
	printf("Run failed!\n");
	err = embryo_program_error_get(ep);
	if (err != EMBRYO_ERROR_NONE)
	  {
	     printf("Run time error %d: \"%s\"\n", err, embryo_error_string_get(err));
	     exit(-1);
	  }
     }
   printf("Program returned %i\n", (int)embryo_program_return_value_get(ep));
   embryo_program_free(ep);
   embryo_shutdown();
   return 0;
}
