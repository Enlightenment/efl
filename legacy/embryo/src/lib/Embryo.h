#ifndef _EMBRYO_H
#define _EMBRYO_H

#ifdef  __cplusplus
extern "C" {
#endif

   /* potentioal error values */
   enum
     {
	EMBRYO_ERROR_NONE,
	  /* reserve the first 15 error codes for exit codes of the abstract machine */
	  EMBRYO_ERROR_EXIT,         /* forced exit */
	  EMBRYO_ERROR_ASSERT,       /* assertion failed */
	  EMBRYO_ERROR_STACKERR,     /* stack/heap collision */
	  EMBRYO_ERROR_BOUNDS,       /* index out of bounds */
	  EMBRYO_ERROR_MEMACCESS,    /* invalid memory access */
	  EMBRYO_ERROR_INVINSTR,     /* invalid instruction */
	  EMBRYO_ERROR_STACKLOW,     /* stack underflow */
	  EMBRYO_ERROR_HEAPLOW,      /* heap underflow */
	  EMBRYO_ERROR_CALLBACK,     /* no callback, or invalid callback */
	  EMBRYO_ERROR_NATIVE,       /* native function failed */
	  EMBRYO_ERROR_DIVIDE,       /* divide by zero */
	  EMBRYO_ERROR_SLEEP,        /* go into sleepmode - code can be restarted */
	  
	  EMBRYO_ERROR_MEMORY = 16,  /* out of memory */
	  EMBRYO_ERROR_FORMAT,       /* invalid file format */
	  EMBRYO_ERROR_VERSION,      /* file is for a newer version of the Embryo_Program */
	  EMBRYO_ERROR_NOTFOUND,     /* function not found */
	  EMBRYO_ERROR_INDEX,        /* invalid index parameter (bad entry point) */
	  EMBRYO_ERROR_DEBUG,        /* debugger cannot run */
	  EMBRYO_ERROR_INIT,         /* Embryo_Program not initialized (or doubly initialized) */
	  EMBRYO_ERROR_USERDATA,     /* unable to set user data field (table full) */
	  EMBRYO_ERROR_INIT_JIT,     /* cannot initialize the JIT */
	  EMBRYO_ERROR_PARAMS,       /* parameter error */
	  EMBRYO_ERROR_DOMAIN,       /* domain error, expression result does not fit in range */
     };

   /* possible function type values that are enumerated */
#define EMBRYO_FUNCTION_NONE 0x7fffffff /* an invalid/non existant function */
#define EMBRYO_FUNCTION_MAIN -1         /* start at program entry point */
#define EMBRYO_FUNCTION_CONT -2         /* continue from last address */
   /* an invalid cell reference */
#define EMBRYO_CELL_NONE     0x7fffffff /* an invalid cell reference */
   /* program run return values */
#define EMBRYO_PROGRAM_OK    1
#define EMBRYO_PROGRAM_SLEEP 2
#define EMBRYO_PROGRAM_BUSY  3
#define EMBRYO_PROGRAM_FAIL  0

#define EMBRYO_FLOAT_TO_CELL(f) ( *((Embryo_Cell*)&f)) /* float to Embryo_Cell */
#define EMBRYO_CELL_TO_FLOAT(c) ( *((float*)&c))       /* Embryo_Cell to float */

   typedef unsigned int                Embryo_UCell;
   typedef int                         Embryo_Cell;
   typedef struct _Embryo_Program      Embryo_Program;
   typedef int                         Embryo_Function;
   
   int              embryo_init(void);
   int              embryo_shutdown(void);
   
   Embryo_Program  *embryo_program_new(void *data, int size);
   Embryo_Program  *embryo_program_const_new(void *data, int size);
   Embryo_Program  *embryo_program_load(char *file);
   void             embryo_program_free(Embryo_Program *ep);
   void             embryo_program_native_call_add(Embryo_Program *ep, char *name, Embryo_Cell (*func) (Embryo_Program *ep, Embryo_Cell *params));
   void             embryo_program_vm_reset(Embryo_Program *ep);
   void             embryo_program_vm_push(Embryo_Program *ep);
   void             embryo_program_vm_pop(Embryo_Program *ep);
   void             embryo_swap_16(unsigned short *v);
   void             embryo_swap_32(unsigned int *v);
   Embryo_Function  embryo_program_function_find(Embryo_Program *ep, char *name);
   Embryo_Cell      embryo_program_variable_find(Embryo_Program *ep, char *name);
   void             embryo_program_error_set(Embryo_Program *ep, int error);
   int              embryo_program_error_get(Embryo_Program *ep);
   const char      *embryo_error_string_get(int error);
   int              embryo_data_string_length_get(Embryo_Program *ep, Embryo_Cell *str_cell);
   void             embryo_data_string_get(Embryo_Program *ep, Embryo_Cell *str_cell, char *dst);
   void             embryo_data_string_set(Embryo_Program *ep, char *src, Embryo_Cell *str_cell);
   Embryo_Cell     *embryo_data_address_get(Embryo_Program *ep, Embryo_Cell addr);
   Embryo_Cell      embryo_data_heap_push(Embryo_Program *ep, int cells);
   void             embryo_data_heap_pop(Embryo_Program *ep, Embryo_Cell down_to);
   int              embryo_program_run(Embryo_Program *ep, Embryo_Function func);
   Embryo_Cell      embryo_program_return_value_get(Embryo_Program *ep);
   int              embryo_parameter_cell_push(Embryo_Program *ep, Embryo_Cell cell);
   int              embryo_parameter_string_push(Embryo_Program *ep, char *str);
   int              embryo_parameter_cell_array_push(Embryo_Program *ep, Embryo_Cell *cells, int num);
   
#ifdef  __cplusplus
}         
#endif

#endif
