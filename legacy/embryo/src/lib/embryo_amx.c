/*  Abstract Machine for the Small compiler
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *  Portions Copyright (c) Carsten Haitzler, 2004 <raster@rasterman.com>
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Embryo.h"
#include "embryo_private.h"


#define JUMPABS(base, ip)     ((Embryo_Cell *)(code + (*ip)))

#ifdef WORDS_BIGENDIAN
static void _embryo_byte_swap_16 (unsigned short *v);
static void _embryo_byte_swap_32 (unsigned int *v);
#endif
static int  _embryo_native_call  (Embryo_Program *ep, Embryo_Cell index, Embryo_Cell *result, Embryo_Cell *params);
static int  _embryo_func_get     (Embryo_Program *ep, int index, char *funcname);
static int  _embryo_var_get      (Embryo_Program *ep, int index, char *varname, Embryo_Cell *ep_addr);
static int  _embryo_program_init (Embryo_Program *ep, void *code);

#ifdef WORDS_BIGENDIAN
static void
_embryo_byte_swap_16(unsigned short *v)
{
   unsigned char *s, t;

   s = (unsigned char *)v;
   t = s[0]; s[0] = s[1]; s[1] = t;
}

static void
_embryo_byte_swap_32(unsigned int *v)
{
   unsigned char *s, t;

   s = (unsigned char *)v;
   t = s[0]; s[0] = s[3]; s[3] = t;
   t = s[1]; s[1] = s[2]; s[2] = t;
}
#endif

static int
_embryo_native_call(Embryo_Program *ep, Embryo_Cell index, Embryo_Cell *result, Embryo_Cell *params)
{
   Embryo_Header    *hdr;
   Embryo_Func_Stub *func_entry;
   Embryo_Native     f;

   hdr = (Embryo_Header *)ep->base;
   func_entry = GETENTRY(hdr, natives, index);
   if ((func_entry->address <= 0) ||
       (func_entry->address > ep->native_calls_size))
     {
	ep->error = EMBRYO_ERROR_CALLBACK;
	return ep->error;
     }
   f = ep->native_calls[func_entry->address - 1];
   if (!f)
     {
	ep->error = EMBRYO_ERROR_CALLBACK;
	return ep->error;
     }
   ep->error = EMBRYO_ERROR_NONE;
   *result = f(ep, params);
   return ep->error;
}

static int
_embryo_func_get(Embryo_Program *ep, int index, char *funcname)
{
   Embryo_Header    *hdr;
   Embryo_Func_Stub *func;

   hdr = (Embryo_Header *)ep->code;
   if (index >= (Embryo_Cell)NUMENTRIES(hdr, publics, natives))
     return EMBRYO_ERROR_INDEX;

   func = GETENTRY(hdr, publics, index);
   strcpy(funcname, GETENTRYNAME(hdr, func));
   return EMBRYO_ERROR_NONE;
}

static int
_embryo_var_get(Embryo_Program *ep, int index, char *varname, Embryo_Cell *ep_addr)
{

  Embryo_Header    *hdr;
  Embryo_Func_Stub *var;

  hdr=(Embryo_Header *)ep->base;
  if (index >= (Embryo_Cell)NUMENTRIES(hdr, pubvars, tags))
     return EMBRYO_ERROR_INDEX;

  var = GETENTRY(hdr, pubvars, index);
  strcpy(varname, GETENTRYNAME(hdr, var));
  *ep_addr = var->address;
  return EMBRYO_ERROR_NONE;
}

static int
_embryo_program_init(Embryo_Program *ep, void *code)
{
   Embryo_Header    *hdr;

   if ((ep->flags & EMBRYO_FLAG_RELOC)) return 1;
   ep->code = (unsigned char *)code;
   hdr = (Embryo_Header *)ep->code;
#ifdef WORDS_BIGENDIAN
   embryo_swap_32((unsigned int *)&hdr->size);
   embryo_swap_16((unsigned short *)&hdr->magic);
   embryo_swap_16((unsigned short *)&hdr->flags);
   embryo_swap_16((unsigned short *)&hdr->defsize);
   embryo_swap_32((unsigned int *)&hdr->cod);
   embryo_swap_32((unsigned int *)&hdr->dat);
   embryo_swap_32((unsigned int *)&hdr->hea);
   embryo_swap_32((unsigned int *)&hdr->stp);
   embryo_swap_32((unsigned int *)&hdr->cip);
   embryo_swap_32((unsigned int *)&hdr->publics);
   embryo_swap_32((unsigned int *)&hdr->natives);
   embryo_swap_32((unsigned int *)&hdr->libraries);
   embryo_swap_32((unsigned int *)&hdr->pubvars);
   embryo_swap_32((unsigned int *)&hdr->tags);
   embryo_swap_32((unsigned int *)&hdr->nametable);
#endif

   if (hdr->magic != EMBRYO_MAGIC) return 0;
   if ((hdr->file_version < MIN_FILE_VERSION) ||
      (hdr->ep_version > CUR_FILE_VERSION)) return 0;
   if ((hdr->defsize != sizeof(Embryo_Func_Stub)) &&
      (hdr->defsize != (2 * sizeof(unsigned int)))) return 0;
   if (hdr->defsize == (2 * sizeof(unsigned int)))
     {
	unsigned short *len;

	len = (unsigned short*)((unsigned char*)ep->code + hdr->nametable);
#ifdef WORDS_BIGENDIAN
	embryo_swap_16((unsigned short *)len);
#endif
	if (*len > sNAMEMAX) return 0;
     }
   if (hdr->stp <= 0) return 0;
   if ((hdr->flags & EMBRYO_FLAG_COMPACT)) return 0;

#ifdef WORDS_BIGENDIAN
     {
	Embryo_Func_Stub *fs;
	int i, num;

	/* also align all addresses in the public function, public variable and */
	/* public tag tables */
	fs = GETENTRY(hdr, publics, 0);
	num = NUMENTRIES(hdr, publics, natives);
	for (i = 0; i < num; i++)
	  {
	     embryo_swap_32(&(fs->address));
	     fs = (Embryo_Func_Stub *)((unsigned char *)fs + hdr->defsize);
	  }

	fs = GETENTRY(hdr, pubvars, 0);
	num = NUMENTRIES(hdr, pubvars, tags);
	for (i = 0; i < num; i++)
	  {
	     embryo_swap_32(&(fs->address));
	     fs = (Embryo_Func_Stub *)((unsigned char *)fs + hdr->defsize);
	  }

	fs = GETENTRY(hdr, tags, 0);
	num = NUMENTRIES(hdr, tags, nametable);
	for (i = 0; i < num; i++)
	  {
	     embryo_swap_32(&(fs->address));
	     fs = (Embryo_Func_Stub *)((unsigned char *)fs + hdr->defsize);
	  }
     }
#endif
   ep->flags = EMBRYO_FLAG_RELOC;

     {
	Embryo_Cell cip, code_size;
	Embryo_Cell *code;

	code_size = hdr->dat - hdr->cod;
	code = (Embryo_Cell *)((unsigned char *)ep->code + (int)hdr->cod);
	for (cip = 0; cip < (code_size / sizeof(Embryo_Cell)); cip++)
	  {
/* move this here - later we probably want something that verifies opcodes
 * are valid and ok...
 */
#ifdef WORDS_BIGENDIAN
	     embryo_swap_32(&(code[cip]));
#endif

	  }
     }
   /* init native api for handling floating point - default in embryo */
   _embryo_args_init(ep);
   _embryo_fp_init(ep);
   _embryo_rand_init(ep);
   _embryo_str_init(ep);
   _embryo_time_init(ep);
   return 1;
}

/*** EXPORTED CALLS ***/

/**
 * @defgroup Embryo_Program_Creation_Group Program Creation and Destruction Functions
 *
 * Functions that set up programs, and destroy them.
 */

/**
 * Creates a new Embryo program, with bytecode data that can be freed.
 * @param   data Pointer to the bytecode of the program.
 * @param   size Number of bytes of bytecode.
 * @return  A new Embryo program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI Embryo_Program *
embryo_program_new(void *data, int size)
{
   Embryo_Program *ep;
   void *code_data;

   if (size < (int)sizeof(Embryo_Header)) return NULL;

   ep = calloc(1, sizeof(Embryo_Program));
   if (!ep) return NULL;

   code_data = malloc(size);
   if (!code_data)
     {
	free(ep);
	return NULL;
     }
   memcpy(code_data, data, size);
   if (_embryo_program_init(ep, code_data)) return ep;
   free(code_data);
   free(ep);
   return NULL;
}

/**
 * Creates a new Embryo program, with bytecode data that cannot be
 * freed.
 * @param   data Pointer to the bytecode of the program.
 * @param   size Number of bytes of bytecode.
 * @return  A new Embryo program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI Embryo_Program *
embryo_program_const_new(void *data, int size)
{
   Embryo_Program *ep;

   if (size < (int)sizeof(Embryo_Header)) return NULL;

   ep = calloc(1, sizeof(Embryo_Program));
   if (!ep) return NULL;

   if (_embryo_program_init(ep, data))
     {
	ep->dont_free_code = 1;
	return ep;
     }
   free(ep);
   return NULL;
}

/**
 * Creates a new Embryo program based on the bytecode data stored in the
 * given file.
 * @param   file Filename of the given file.
 * @return  A new Embryo program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI Embryo_Program *
embryo_program_load(char *file)
{
   Embryo_Program *ep;
   Embryo_Header   hdr;
   FILE *f;
   void *program = NULL;
   int program_size = 0;

   f = fopen(file, "rb");
   if (!f) return NULL;
   fseek(f, 0, SEEK_END);
   program_size = ftell(f);
   rewind(f);
   if (program_size < (int)sizeof(Embryo_Header))
     {
	fclose(f);
	return NULL;
     }
   if (fread(&hdr, sizeof(Embryo_Header), 1, f) != 1)
     {
	fclose(f);
	return NULL;
     }
   rewind(f);
#ifdef WORDS_BIGENDIAN
   embryo_swap_32((unsigned int *)(&hdr.size));
#endif
   if ((int)hdr.size < program_size) program_size = hdr.size;
   program = malloc(program_size);
   if (!program)
     {
	fclose(f);
	return NULL;
     }
   if (fread(program, program_size, 1, f) != 1)
     {
	free(program);
	fclose(f);
	return NULL;
     }
   ep = embryo_program_new(program, program_size);
   free(program);
   fclose(f);
   return ep;
}

/**
 * Frees the given Embryo program.
 * @param   ep The given program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI void
embryo_program_free(Embryo_Program *ep)
{
   int i;

   if (ep->base) free(ep->base);
   if ((!ep->dont_free_code) && (ep->code)) free(ep->code);
   if (ep->native_calls) free(ep->native_calls);
   for (i = 0; i < ep->params_size; i++)
     {
	if (ep->params[i].string) free(ep->params[i].string);
	if (ep->params[i].cell_array) free(ep->params[i].cell_array);
     }
   if (ep->params) free(ep->params);
   free(ep);
}

/**
 * @defgroup Embryo_Func_Group Function Functions
 *
 * Functions that deal with Embryo program functions.
 */

/**
 * Adds a native program call to the given Embryo program.
 * @param   ep   The given Embryo program.
 * @param   name The name for the call used in the script.
 * @param   func The function to use when the call is made.
 * @ingroup Embryo_Func_Group
 */
EAPI void
embryo_program_native_call_add(Embryo_Program *ep, const char *name, Embryo_Cell (*func) (Embryo_Program *ep, Embryo_Cell *params))
{
   Embryo_Func_Stub *func_entry;
   Embryo_Header    *hdr;
   int               i, num;

   if ((ep == NULL ) || (name == NULL) || (func == NULL)) return;
   if (strlen(name) > sNAMEMAX) return;

   hdr = (Embryo_Header *)ep->code;
   if (hdr->defsize < 1) return;
   num = NUMENTRIES(hdr, natives, libraries);
   if (num <= 0) return;

   ep->native_calls_size++;
   if (ep->native_calls_size > ep->native_calls_alloc)
     {
	Embryo_Native *calls;

	ep->native_calls_alloc += 16;
	calls = realloc(ep->native_calls,
			ep->native_calls_alloc * sizeof(Embryo_Native));
	if (!calls)
	  {
	     ep->native_calls_size--;
	     ep->native_calls_alloc -= 16;
	     return;
	  }
	ep->native_calls = calls;
     }
   ep->native_calls[ep->native_calls_size - 1] = func;

   func_entry = GETENTRY(hdr, natives, 0);
   for (i = 0; i < num; i++)
     {
	if (func_entry->address == 0)
	  {
	     char *entry_name;

	     entry_name = GETENTRYNAME(hdr, func_entry);
	     if ((entry_name) && (!strcmp(entry_name, name)))
	       {
		  func_entry->address = ep->native_calls_size;
		  /* FIXME: embryo_cc is putting in multiple native */
		  /* function call entries - so we need to fill in all */
		  /* of them!!! */
		  /* return; */
	       }
	  }
	func_entry =
	  (Embryo_Func_Stub *)((unsigned char *)func_entry + hdr->defsize);
     }
}

/**
 * @defgroup Embryo_Program_VM_Group Virtual Machine Functions
 *
 * Functions that deal with creating and destroying virtual machine sessions
 * for a given program.
 *
 * A given embryo program can have multiple virtual machine sessions running.
 * This is useful when you have a native call that in turn calls a function in
 * the embryo program.  The native call can start a new virtual machine
 * session to run the function it needs.  Once completed, the session can be
 * popped off the program's stack, and the native call can return its value
 * to the old session.
 *
 * A new virtual machine session is created by pushing a new virtual machine
 * onto the session stack of a program using @ref embryo_program_vm_push.
 * The current virtual machine session can be destroyed by calling
 * @ref embryo_program_vm_pop.
 */

/**
 * Resets the current virtual machine session of the given program.
 * @param   ep The given program.
 * @ingroup Embryo_Program_VM_Group
 */
EAPI void
embryo_program_vm_reset(Embryo_Program *ep)
{
   Embryo_Header *hdr;

   if ((!ep) || (!ep->base)) return;
   hdr = (Embryo_Header *)ep->code;
   memcpy(ep->base, hdr, hdr->size);
   *(Embryo_Cell *)(ep->base + (int)hdr->stp - sizeof(Embryo_Cell)) = 0;

   ep->hlw = hdr->hea - hdr->dat; /* stack and heap relative to data segment */
   ep->stp = hdr->stp - hdr->dat - sizeof(Embryo_Cell);
   ep->hea = ep->hlw;
   ep->stk = ep->stp;
}

/**
 * Starts a new virtual machine session for the given program.
 *
 * See @ref Embryo_Program_VM_Group for more information about how this works.
 *
 * @param   ep The given program.
 * @ingroup Embryo_Program_VM_Group
 */
EAPI void
embryo_program_vm_push(Embryo_Program *ep)
{
   Embryo_Header *hdr;

   if (!ep) return;
   ep->pushes++;
   if (ep->pushes > 1)
     {
	embryo_program_vm_reset(ep);
	return;
     }
   hdr = (Embryo_Header *)ep->code;
   ep->base = malloc(hdr->stp);
   if (!ep->base)
     {
	ep->pushes = 0;
	return;
     }
   embryo_program_vm_reset(ep);
}

/**
 * Frees the current virtual machine session associated with the given program.
 *
 * See @ref Embryo_Program_VM_Group for more information about how this works.
 * Note that you will need to retrieve any return data or data on the stack
 * before you pop.
 *
 * @param   ep The given program.
 * @ingroup Embryo_Program_VM_Group
 */
EAPI void
embryo_program_vm_pop(Embryo_Program *ep)
{
   if ((!ep) || (!ep->base)) return;
   ep->pushes--;
   if (ep->pushes >= 1) return;
   free(ep->base);
   ep->base = NULL;
}

/**
 * @defgroup Embryo_Swap_Group Byte Swapping Functions
 *
 * Functions that are used to ensure that integers passed to the
 * virtual machine are in small endian format.  These functions are
 * used to ensure that the virtual machine operates correctly on big
 * endian machines.
 */

/**
 * Ensures that the given unsigned short integer is in the small
 * endian format.
 * @param   v Pointer to the given integer.
 * @ingroup Embryo_Swap_Group
 */
EAPI void
embryo_swap_16(unsigned short *v)
{
#ifdef WORDS_BIGENDIAN
   _embryo_byte_swap_16(v);
#endif
}

/**
 * Ensures that the given unsigned integer is in the small endian
 * format.
 * @param   v Pointer to the given integer.
 * @ingroup Embryo_Swap_Group
 */
EAPI void
embryo_swap_32(unsigned int *v)
{
#ifdef WORDS_BIGENDIAN
   _embryo_byte_swap_32(v);
#endif
}

/**
 * Returns the function in the given program with the given name.
 * @param   ep The given program.
 * @param   name The given function name.
 * @return  The function if successful.  Otherwise, @c EMBRYO_FUNCTION_NONE.
 * @ingroup Embryo_Func_Group
 */
EAPI Embryo_Function
embryo_program_function_find(Embryo_Program *ep, const char *name)
{
   int            first, last, mid, result;
   char           pname[sNAMEMAX + 1];
   Embryo_Header *hdr;

   if (!ep) return EMBRYO_FUNCTION_NONE;
   hdr = (Embryo_Header *)ep->code;
   last = NUMENTRIES(hdr, publics, natives) - 1;
   first = 0;
   /* binary search */
   while (first <= last)
     {
	mid = (first + last) / 2;
	if (_embryo_func_get(ep, mid, pname) == EMBRYO_ERROR_NONE)
	  result = strcmp(pname, name);
	else
	  return EMBRYO_FUNCTION_NONE;
/*	  result = -1;*/
	if (result > 0) last = mid - 1;
	else if (result < 0) first = mid + 1;
	else return mid;
     }
   return EMBRYO_FUNCTION_NONE;
}

/**
 * @defgroup Embryo_Public_Variable_Group Public Variable Access Functions
 *
 * In an Embryo program, a global variable can be declared public, as
 * described in @ref Small_Scope_Subsection.  The functions here allow
 * the host program to access these public variables.
 */

/**
 * Retrieves the location of the public variable in the given program
 * with the given name.
 * @param   ep   The given program.
 * @param   name The given name.
 * @return  The address of the variable if found.  @c EMBRYO_CELL_NONE
 *          otherwise.
 * @ingroup Embryo_Public_Variable_Group
 */
EAPI Embryo_Cell
embryo_program_variable_find(Embryo_Program *ep, const char *name)
{
   int            first, last, mid, result;
   char           pname[sNAMEMAX + 1];
   Embryo_Cell    paddr;
   Embryo_Header *hdr;

   if (!ep) return EMBRYO_CELL_NONE;
   if (!ep->base) return EMBRYO_CELL_NONE;
   hdr = (Embryo_Header *)ep->base;
   last = NUMENTRIES(hdr, pubvars, tags) - 1;
   first = 0;
   /* binary search */
   while (first <= last)
     {
	mid = (first + last) / 2;
	if (_embryo_var_get(ep, mid, pname, &paddr) == EMBRYO_ERROR_NONE)
	  result = strcmp(pname, name);
	else
	  return EMBRYO_CELL_NONE;
/*	  result = -1;*/
	if (result > 0) last = mid - 1;
	else if (result < 0) first = mid + 1;
	else return paddr;
     }
   return EMBRYO_CELL_NONE;
}

/**
 * Retrieves the number of public variables in the given program.
 * @param   ep The given program.
 * @return  The number of public variables.
 * @ingroup Embryo_Public_Variable_Group
 */
EAPI int
embryo_program_variable_count_get(Embryo_Program *ep)
{
   Embryo_Header *hdr;

   if (!ep) return 0;
   if (!ep->base) return 0;
   hdr = (Embryo_Header *)ep->base;
   return NUMENTRIES(hdr, pubvars, tags);
}

/**
 * Retrieves the location of the public variable in the given program
 * with the given identifier.
 * @param   ep  The given program.
 * @param   num The identifier of the public variable.
 * @return  The virtual machine address of the variable if found.
 *          @c EMBRYO_CELL_NONE otherwise.
 * @ingroup Embryo_Public_Variable_Group
 */
EAPI Embryo_Cell
embryo_program_variable_get(Embryo_Program *ep, int num)
{
   Embryo_Header *hdr;
   Embryo_Cell    paddr;
   char           pname[sNAMEMAX + 1];

   if (!ep) return EMBRYO_CELL_NONE;
   if (!ep->base) return EMBRYO_CELL_NONE;
   hdr = (Embryo_Header *)ep->base;
   if (_embryo_var_get(ep, num, pname, &paddr) == EMBRYO_ERROR_NONE)
     return paddr;
   return EMBRYO_CELL_NONE;
}

/**
 * @defgroup Embryo_Error_Group Error Functions
 *
 * Functions that set and retrieve error codes in Embryo programs.
 */

/**
 * Sets the error code for the given program to the given code.
 * @param   ep The given program.
 * @param   error The given error code.
 * @ingroup Embryo_Error_Group
 */
EAPI void
embryo_program_error_set(Embryo_Program *ep, int error)
{
   if (!ep) return;
   ep->error = error;
}

/**
 * Retrieves the current error code for the given program.
 * @param   ep The given program.
 * @return  The current error code.
 * @ingroup Embryo_Error_Group
 */
EAPI int
embryo_program_error_get(Embryo_Program *ep)
{
   if (!ep) return EMBRYO_ERROR_NONE;
   return ep->error;
}

/**
 * @defgroup Embryo_Program_Data_Group Program Data Functions
 *
 * Functions that set and retrieve data associated with the given
 * program.
 */

/**
 * Sets the data associated to the given program.
 * @param   ep   The given program.
 * @param   data New bytecode data.
 * @ingroup Embryo_Program_Data_Group
 */
EAPI void
embryo_program_data_set(Embryo_Program *ep, void *data)
{
   if (!ep) return;
   ep->data = data;
}

/**
 * Retrieves the data associated to the given program.
 * @param   ep The given program.
 * @ingroup Embryo_Program_Data_Group
 */
EAPI void *
embryo_program_data_get(Embryo_Program *ep)
{
   if (!ep) return NULL;
   return ep->data;
}

/**
 * Retrieves a string describing the given error code.
 * @param   error The given error code.
 * @return  String describing the given error code.  If the given code is not
 *          known, the string "(unknown)" is returned.
 * @ingroup Embryo_Error_Group
 */
EAPI const char *
embryo_error_string_get(int error)
{
   const char *messages[] =
     {
	/* EMBRYO_ERROR_NONE      */ "(none)",
	  /* EMBRYO_ERROR_EXIT      */ "Forced exit",
	  /* EMBRYO_ERROR_ASSERT    */ "Assertion failed",
	  /* EMBRYO_ERROR_STACKERR  */ "Stack/heap collision (insufficient stack size)",
	  /* EMBRYO_ERROR_BOUNDS    */ "Array index out of bounds",
	  /* EMBRYO_ERROR_MEMACCESS */ "Invalid memory access",
	  /* EMBRYO_ERROR_INVINSTR  */ "Invalid instruction",
	  /* EMBRYO_ERROR_STACKLOW  */ "Stack underflow",
	  /* EMBRYO_ERROR_HEAPLOW   */ "Heap underflow",
	  /* EMBRYO_ERROR_CALLBACK  */ "No (valid) native function callback",
	  /* EMBRYO_ERROR_NATIVE    */ "Native function failed",
	  /* EMBRYO_ERROR_DIVIDE    */ "Divide by zero",
	  /* EMBRYO_ERROR_SLEEP     */ "(sleep mode)",
	  /* 13 */                     "(reserved)",
	  /* 14 */                     "(reserved)",
	  /* 15 */                     "(reserved)",
	  /* EMBRYO_ERROR_MEMORY    */ "Out of memory",
	  /* EMBRYO_ERROR_FORMAT    */ "Invalid/unsupported P-code file format",
	  /* EMBRYO_ERROR_VERSION   */ "File is for a newer version of the Embryo_Program",
	  /* EMBRYO_ERROR_NOTFOUND  */ "Native/Public function is not found",
	  /* EMBRYO_ERROR_INDEX     */ "Invalid index parameter (bad entry point)",
	  /* EMBRYO_ERROR_DEBUG     */ "Debugger cannot run",
	  /* EMBRYO_ERROR_INIT      */ "Embryo_Program not initialized (or doubly initialized)",
	  /* EMBRYO_ERROR_USERDATA  */ "Unable to set user data field (table full)",
	  /* EMBRYO_ERROR_INIT_JIT  */ "Cannot initialize the JIT",
	  /* EMBRYO_ERROR_PARAMS    */ "Parameter error",
     };
   if ((error < 0) || (error >= (int)(sizeof(messages) / sizeof(messages[0]))))
     return (const char *)"(unknown)";
   return messages[error];
}

/**
 * @defgroup Embryo_Data_String_Group Embryo Data String Functions
 *
 * Functions that operate on strings in the memory of a virtual machine.
 */

/**
 * Retrieves the length of the string starting at the given cell.
 * @param   ep       The program the cell is part of.
 * @param   str_cell Pointer to the first cell of the string.
 * @return  The length of the string.  @c 0 is returned if there is an error.
 * @ingroup Embryo_Data_String_Group
 */
EAPI int
embryo_data_string_length_get(Embryo_Program *ep, Embryo_Cell *str_cell)
{
   int            len;
   Embryo_Header *hdr;

   if ((!ep) || (!ep->base)) return 0;
   hdr = (Embryo_Header *)ep->base;
   if ((!str_cell) ||
       ((void *)str_cell >= (void *)(ep->base + hdr->stp)) ||
       ((void *)str_cell < (void *)ep->base))
     return 0;
   for (len = 0; str_cell[len] != 0; len++);
   return len;
}

/**
 * Copies the string starting at the given cell to the given buffer.
 * @param   ep       The program the cell is part of.
 * @param   str_cell Pointer to the first cell of the string.
 * @param   dst      The given buffer.
 * @ingroup Embryo_Data_String_Group
 */
EAPI void
embryo_data_string_get(Embryo_Program *ep, Embryo_Cell *str_cell, char *dst)
{
   int            i;
   Embryo_Header *hdr;

   if (!dst) return;
   if ((!ep) || (!ep->base))
     {
	dst[0] = 0;
	return;
     }
   hdr = (Embryo_Header *)ep->base;
   if ((!str_cell) ||
       ((void *)str_cell >= (void *)(ep->base + hdr->stp)) ||
       ((void *)str_cell < (void *)ep->base))
     {
	dst[0] = 0;
	return;
     }
   for (i = 0; str_cell[i] != 0; i++)
     {
#ifdef WORDS_BIGENDIAN
	  {
	     Embryo_Cell tmp;

	     tmp = str_cell[i];
	     _embryo_byte_swap_32(&tmp);
	     dst[i] = tmp;
	  }
#else
	dst[i] = str_cell[i];
#endif
     }
   dst[i] = 0;
}

/**
 * Copies string in the given buffer into the virtual machine memory
 * starting at the given cell.
 * @param ep       The program the cell is part of.
 * @param src      The given buffer.
 * @param str_cell Pointer to the first cell to copy the string to.
 * @ingroup Embryo_Data_String_Group
 */
EAPI void
embryo_data_string_set(Embryo_Program *ep, const char *src, Embryo_Cell *str_cell)
{
   int            i;
   Embryo_Header *hdr;

   if (!ep) return;
   if (!ep->base) return;
   hdr = (Embryo_Header *)ep->base;
   if ((!str_cell) ||
       ((void *)str_cell >= (void *)(ep->base + hdr->stp)) ||
       ((void *)str_cell < (void *)ep->base))
     return;
   if (!src)
     {
	str_cell[0] = 0;
	return;
     }
   for (i = 0; src[i] != 0; i++)
     {
	if ((void *)(&(str_cell[i])) >= (void *)(ep->base + hdr->stp)) return;
	else if ((void *)(&(str_cell[i])) == (void *)(ep->base + hdr->stp - 1))
	  {
	     str_cell[i] = 0;
	     return;
	  }
#ifdef WORDS_BIGENDIAN
	  {
	     Embryo_Cell tmp;

	     tmp = src[i];
	     _embryo_byte_swap_32(&tmp);
	     str_cell[i] = tmp;
	  }
#else
	str_cell[i] = src[i];
#endif
     }
   str_cell[i] = 0;
}

/**
 * Retreives a pointer to the address in the virtual machine given by the
 * given cell.
 * @param   ep   The program whose virtual machine address is being queried.
 * @param   addr The given cell.
 * @return  A pointer to the cell at the given address.
 * @ingroup Embryo_Data_String_Group
 */
EAPI Embryo_Cell *
embryo_data_address_get(Embryo_Program *ep, Embryo_Cell addr)
{
   Embryo_Header *hdr;
   unsigned char *data;

   if ((!ep) || (!ep->base)) return NULL;
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   if ((addr < 0) || (addr >= hdr->stp)) return NULL;
   return (Embryo_Cell *)(data + (int)addr);
}

/**
 * @defgroup Embryo_Heap_Group Heap Functions
 *
 * The heap is an area of memory that can be allocated for program
 * use at runtime.  The heap functions here change the amount of heap
 * memory available.
 */

/**
 * Increases the size of the heap of the given virtual machine by the given
 * number of Embryo_Cells.
 * @param   ep    The program with the given virtual machine.
 * @param   cells The given number of Embryo_Cells.
 * @return  The address of the new memory region on success.
 *          @c EMBRYO_CELL_NONE otherwise.
 * @ingroup Embryo_Heap_Group
 */
EAPI Embryo_Cell
embryo_data_heap_push(Embryo_Program *ep, int cells)
{
   Embryo_Header *hdr;
   unsigned char *data;
   Embryo_Cell    addr;

   if ((!ep) || (!ep->base)) return EMBRYO_CELL_NONE;
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   if (ep->stk - ep->hea - (cells * sizeof(Embryo_Cell)) < STKMARGIN)
     return EMBRYO_CELL_NONE;
   addr = ep->hea;
   ep->hea += (cells * sizeof(Embryo_Cell));
   return addr;
}

/**
 * Decreases the size of the heap of the given virtual machine down to the
 * given size.
 * @param   ep      The program with the given virtual machine.
 * @param   down_to The given size.
 * @ingroup Embryo_Heap_Group
 */
EAPI void
embryo_data_heap_pop(Embryo_Program *ep, Embryo_Cell down_to)
{
   if (!ep) return;
   if (down_to < 0) down_to = 0;
   if (ep->hea > down_to) ep->hea = down_to;
}

/**
 * @defgroup Embryo_Run_Group Program Run Functions
 *
 * Functions that are involved in actually running functions in an
 * Embryo program.
 */

/**
 * Returns the number of virtual machines are running for the given program.
 * @param   ep The given program.
 * @return  The number of virtual machines running.
 * @ingroup Embryo_Run_Group
 */
EAPI int
embryo_program_recursion_get(Embryo_Program *ep)
{
   return ep->run_count;
}

#ifdef __GNUC__
#if 1
#define EMBRYO_EXEC_JUMPTABLE
#endif
#endif

/* jump table optimization - only works for gcc though */
#ifdef EMBRYO_EXEC_JUMPTABLE
#define SWITCH(x) while (1) { goto *switchtable[x];
#define SWITCHEND break; }
#define CASE(x) SWITCHTABLE_##x:
#define BREAK break;
#else
#define SWITCH(x) switch (x) {
#define SWITCHEND }
#define CASE(x) case x:
#define BREAK break
#endif

/**
 * Runs the given function of the given Embryo program in the current
 * virtual machine.  The parameter @p fn can be found using
 * @ref embryo_program_function_find.
 *
 * @note For Embryo to be able to run a function, it must have been
 *       declared @c public in the Small source code.
 *
 * @param   ep The given program.
 * @param   fn The given function.  Normally "main", in which case the
 *             constant @c EMBRYO_FUNCTION_MAIN can be used.
 * @return  @c EMBRYO_PROGRAM_OK on success.  @c EMBRYO_PROGRAM_SLEEP if the
 *          program is halted by the Small @c sleep call.
 *          @c EMBRYO_PROGRAM_FAIL if there is an error.
 *          @c EMBRYO_PROGRAM_TOOLONG if the program executes for longer than
 *          it is allowed to in abstract machine instruction count.
 * @ingroup Embryo_Run_Group
 */
EAPI int
embryo_program_run(Embryo_Program *ep, Embryo_Function fn)
{
   Embryo_Header    *hdr;
   Embryo_Func_Stub *func;
   unsigned char    *code, *data;
   Embryo_Cell      pri, alt, stk, frm, hea, hea_start;
   Embryo_Cell      reset_stk, reset_hea, *cip;
   Embryo_UCell     codesize;
   int              i;
   unsigned char    op;
   Embryo_Cell      offs;
   int              num;
   int              max_run_cycles;
   int              cycle_count;
#ifdef EMBRYO_EXEC_JUMPTABLE
   /* we limit the jumptable to 256 elements. why? above we forced "op" to be
    * a unsigned char - that means 256 max values. we limit opcode overflow
    * here, so eliminating crashes on table lookups with bad/corrupt bytecode.
    * no need to atuall do compares, branches etc. the datatype does the work
    * for us. so that means EXCESS elements are all declared as OP_NONE to
    * keep them innocuous.
    */
   static const void *switchtable[256] =
     {
	   &&SWITCHTABLE_EMBRYO_OP_NONE,
	       &&SWITCHTABLE_EMBRYO_OP_LOAD_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_LOAD_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_LOAD_S_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_LOAD_S_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_LREF_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_LREF_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_LREF_S_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_LREF_S_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_LOAD_I,
	       &&SWITCHTABLE_EMBRYO_OP_LODB_I,
	       &&SWITCHTABLE_EMBRYO_OP_CONST_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_CONST_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_ADDR_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_ADDR_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_STOR_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_STOR_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_STOR_S_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_STOR_S_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_SREF_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SREF_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_SREF_S_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SREF_S_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_STOR_I,
	       &&SWITCHTABLE_EMBRYO_OP_STRB_I,
	       &&SWITCHTABLE_EMBRYO_OP_LIDX,
	       &&SWITCHTABLE_EMBRYO_OP_LIDX_B,
	       &&SWITCHTABLE_EMBRYO_OP_IDXADDR,
	       &&SWITCHTABLE_EMBRYO_OP_IDXADDR_B,
	       &&SWITCHTABLE_EMBRYO_OP_ALIGN_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_ALIGN_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_LCTRL,
	       &&SWITCHTABLE_EMBRYO_OP_SCTRL,
	       &&SWITCHTABLE_EMBRYO_OP_MOVE_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_MOVE_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_XCHG,
	       &&SWITCHTABLE_EMBRYO_OP_PUSH_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_PUSH_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_PUSH_R,
	       &&SWITCHTABLE_EMBRYO_OP_PUSH_C,
	       &&SWITCHTABLE_EMBRYO_OP_PUSH,
	       &&SWITCHTABLE_EMBRYO_OP_PUSH_S,
	       &&SWITCHTABLE_EMBRYO_OP_POP_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_POP_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_STACK,
	       &&SWITCHTABLE_EMBRYO_OP_HEAP,
	       &&SWITCHTABLE_EMBRYO_OP_PROC,
	       &&SWITCHTABLE_EMBRYO_OP_RET,
	       &&SWITCHTABLE_EMBRYO_OP_RETN,
	       &&SWITCHTABLE_EMBRYO_OP_CALL,
	       &&SWITCHTABLE_EMBRYO_OP_CALL_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_JUMP,
	       &&SWITCHTABLE_EMBRYO_OP_JREL,
	       &&SWITCHTABLE_EMBRYO_OP_JZER,
	       &&SWITCHTABLE_EMBRYO_OP_JNZ,
	       &&SWITCHTABLE_EMBRYO_OP_JEQ,
	       &&SWITCHTABLE_EMBRYO_OP_JNEQ,
	       &&SWITCHTABLE_EMBRYO_OP_JLESS,
	       &&SWITCHTABLE_EMBRYO_OP_JLEQ,
	       &&SWITCHTABLE_EMBRYO_OP_JGRTR,
	       &&SWITCHTABLE_EMBRYO_OP_JGEQ,
	       &&SWITCHTABLE_EMBRYO_OP_JSLESS,
	       &&SWITCHTABLE_EMBRYO_OP_JSLEQ,
	       &&SWITCHTABLE_EMBRYO_OP_JSGRTR,
	       &&SWITCHTABLE_EMBRYO_OP_JSGEQ,
	       &&SWITCHTABLE_EMBRYO_OP_SHL,
	       &&SWITCHTABLE_EMBRYO_OP_SHR,
	       &&SWITCHTABLE_EMBRYO_OP_SSHR,
	       &&SWITCHTABLE_EMBRYO_OP_SHL_C_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SHL_C_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_SHR_C_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SHR_C_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_SMUL,
	       &&SWITCHTABLE_EMBRYO_OP_SDIV,
	       &&SWITCHTABLE_EMBRYO_OP_SDIV_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_UMUL,
	       &&SWITCHTABLE_EMBRYO_OP_UDIV,
	       &&SWITCHTABLE_EMBRYO_OP_UDIV_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_ADD,
	       &&SWITCHTABLE_EMBRYO_OP_SUB,
	       &&SWITCHTABLE_EMBRYO_OP_SUB_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_AND,
	       &&SWITCHTABLE_EMBRYO_OP_OR,
	       &&SWITCHTABLE_EMBRYO_OP_XOR,
	       &&SWITCHTABLE_EMBRYO_OP_NOT,
	       &&SWITCHTABLE_EMBRYO_OP_NEG,
	       &&SWITCHTABLE_EMBRYO_OP_INVERT,
	       &&SWITCHTABLE_EMBRYO_OP_ADD_C,
	       &&SWITCHTABLE_EMBRYO_OP_SMUL_C,
	       &&SWITCHTABLE_EMBRYO_OP_ZERO_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_ZERO_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_ZERO,
	       &&SWITCHTABLE_EMBRYO_OP_ZERO_S,
	       &&SWITCHTABLE_EMBRYO_OP_SIGN_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SIGN_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_EQ,
	       &&SWITCHTABLE_EMBRYO_OP_NEQ,
	       &&SWITCHTABLE_EMBRYO_OP_LESS,
	       &&SWITCHTABLE_EMBRYO_OP_LEQ,
	       &&SWITCHTABLE_EMBRYO_OP_GRTR,
	       &&SWITCHTABLE_EMBRYO_OP_GEQ,
	       &&SWITCHTABLE_EMBRYO_OP_SLESS,
	       &&SWITCHTABLE_EMBRYO_OP_SLEQ,
	       &&SWITCHTABLE_EMBRYO_OP_SGRTR,
	       &&SWITCHTABLE_EMBRYO_OP_SGEQ,
	       &&SWITCHTABLE_EMBRYO_OP_EQ_C_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_EQ_C_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_INC_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_INC_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_INC,
	       &&SWITCHTABLE_EMBRYO_OP_INC_S,
	       &&SWITCHTABLE_EMBRYO_OP_INC_I,
	       &&SWITCHTABLE_EMBRYO_OP_DEC_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_DEC_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_DEC,
	       &&SWITCHTABLE_EMBRYO_OP_DEC_S,
	       &&SWITCHTABLE_EMBRYO_OP_DEC_I,
	       &&SWITCHTABLE_EMBRYO_OP_MOVS,
	       &&SWITCHTABLE_EMBRYO_OP_CMPS,
	       &&SWITCHTABLE_EMBRYO_OP_FILL,
	       &&SWITCHTABLE_EMBRYO_OP_HALT,
	       &&SWITCHTABLE_EMBRYO_OP_BOUNDS,
	       &&SWITCHTABLE_EMBRYO_OP_SYSREQ_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SYSREQ_C,
	       &&SWITCHTABLE_EMBRYO_OP_FILE,
	       &&SWITCHTABLE_EMBRYO_OP_LINE,
	       &&SWITCHTABLE_EMBRYO_OP_SYMBOL,
	       &&SWITCHTABLE_EMBRYO_OP_SRANGE,
	       &&SWITCHTABLE_EMBRYO_OP_JUMP_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SWITCH,
	       &&SWITCHTABLE_EMBRYO_OP_CASETBL,
	       &&SWITCHTABLE_EMBRYO_OP_SWAP_PRI,
	       &&SWITCHTABLE_EMBRYO_OP_SWAP_ALT,
	       &&SWITCHTABLE_EMBRYO_OP_PUSHADDR,
	       &&SWITCHTABLE_EMBRYO_OP_NOP,
	       &&SWITCHTABLE_EMBRYO_OP_SYSREQ_D,
	       &&SWITCHTABLE_EMBRYO_OP_SYMTAG,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE,
	  &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE, &&SWITCHTABLE_EMBRYO_OP_NONE
     };
#endif
   if (!ep) return EMBRYO_PROGRAM_FAIL;
   if (!(ep->flags & EMBRYO_FLAG_RELOC))
     {
	ep->error = EMBRYO_ERROR_INIT;
	return EMBRYO_PROGRAM_FAIL;
     }
   if (!ep->base)
     {
	ep->error = EMBRYO_ERROR_INIT;
	return EMBRYO_PROGRAM_FAIL;
     }
   if (ep->run_count > 0)
     {
	/* return EMBRYO_PROGRAM_BUSY; */
	/* FIXME: test C->vm->C->vm recursion more fully */
	/* it seems to work... just fine!!! - strange! */
     }

   /* set up the registers */
   hdr = (Embryo_Header *)ep->base;
   codesize = (Embryo_UCell)(hdr->dat - hdr->cod);
   code = ep->base + (int)hdr->cod;
   data = ep->base + (int)hdr->dat;
   hea_start = hea = ep->hea;
   stk = ep->stk;
   reset_stk = stk;
   reset_hea = hea;
   frm = alt = pri = 0;

   /* get the start address */
   if (fn == EMBRYO_FUNCTION_MAIN)
     {
	if (hdr->cip < 0)
	  {
	     ep->error = EMBRYO_ERROR_INDEX;
	     return EMBRYO_PROGRAM_FAIL;
	  }
	cip = (Embryo_Cell *)(code + (int)hdr->cip);
     }
   else if (fn == EMBRYO_FUNCTION_CONT)
     {
	/* all registers: pri, alt, frm, cip, hea, stk, reset_stk, reset_hea */
	frm = ep->frm;
	stk = ep->stk;
	hea = ep->hea;
	pri = ep->pri;
	alt = ep->alt;
	reset_stk = ep->reset_stk;
	reset_hea = ep->reset_hea;
	cip = (Embryo_Cell *)(code + (int)ep->cip);
     }
   else if (fn < 0)
     {
	ep->error = EMBRYO_ERROR_INDEX;
	return EMBRYO_PROGRAM_FAIL;
     }
   else
     {
	if (fn >= (Embryo_Cell)NUMENTRIES(hdr, publics, natives))
	  {
	     ep->error = EMBRYO_ERROR_INDEX;
	     return EMBRYO_PROGRAM_FAIL;
	  }
	func = GETENTRY(hdr, publics, fn);
	cip = (Embryo_Cell *)(code + (int)func->address);
     }
   /* check values just copied */
   CHKSTACK();
   CHKHEAP();

   if (fn != EMBRYO_FUNCTION_CONT)
     {
	int i;

	for (i = ep->params_size - 1; i >= 0; i--)
	  {
	     Embryo_Param *pr;

	     pr = &(ep->params[i]);
	     if (pr->string)
	       {
		  int len;
		  Embryo_Cell ep_addr, *addr;

		  len = strlen(pr->string);
		  ep_addr = embryo_data_heap_push(ep, len + 1);
		  if (ep_addr == EMBRYO_CELL_NONE)
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  addr = embryo_data_address_get(ep, ep_addr);
		  if (addr)
		    embryo_data_string_set(ep, pr->string, addr);
		  else
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  PUSH(ep_addr);
		  free(pr->string);
	       }
	     else if (pr->cell_array)
	       {
		  int len;
		  Embryo_Cell ep_addr, *addr;

		  len = pr->cell_array_size;
		  ep_addr = embryo_data_heap_push(ep, len + 1);
		  if (ep_addr == EMBRYO_CELL_NONE)
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  addr = embryo_data_address_get(ep, ep_addr);
		  if (addr)
		    memcpy(addr, pr->cell_array,
			   pr->cell_array_size * sizeof(Embryo_Cell));
		  else
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  PUSH(ep_addr);
		  free(pr->cell_array);
	       }
	     else
	       {
		  PUSH(pr->cell);
	       }
	  }
	PUSH(ep->params_size * sizeof(Embryo_Cell));
	PUSH(0);
	if (ep->params)
	  {
	     free(ep->params);
	     ep->params = NULL;
	  }
	ep->params_size = ep->params_alloc = 0;
     }
   /* check stack/heap before starting to run */
   CHKMARGIN();

   /* track recursion depth */
   ep->run_count++;

   max_run_cycles = ep->max_run_cycles;
   /* start running */
   for (cycle_count = 0;;)
     {
	if (max_run_cycles > 0)
	  {
	     if (cycle_count >= max_run_cycles)
	       {
		  TOOLONG(ep);
	       }
	     cycle_count++;
	  }
	op = (Embryo_Opcode)*cip++;
	SWITCH(op);
	CASE(EMBRYO_OP_LOAD_PRI);
	GETPARAM(offs);
	pri = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LOAD_ALT);
	GETPARAM(offs);
	alt = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LOAD_S_PRI);
	GETPARAM(offs);
	pri = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LOAD_S_ALT);
	GETPARAM(offs);
	alt = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LREF_PRI);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)offs);
	pri = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LREF_ALT);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)offs);
	alt = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LREF_S_PRI);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	pri = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LREF_S_ALT);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	alt = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LOAD_I);
	CHKMEM(pri);
	pri = *(Embryo_Cell *)(data + (int)pri);
	BREAK;
	CASE(EMBRYO_OP_LODB_I);
	GETPARAM(offs);
	CHKMEM(pri);
	switch (offs)
	  {
	   case 1:
	     pri = *(data + (int)pri);
	     break;
	   case 2:
	     pri = *(unsigned short *)(data + (int)pri);
	     break;
	   case 4:
	     pri = *(unsigned int *)(data + (int)pri);
	     break;
	   default:
	     ABORT(ep, EMBRYO_ERROR_INVINSTR);
	     break;
	  }
	BREAK;
	CASE(EMBRYO_OP_CONST_PRI);
	GETPARAM(pri);
	BREAK;
	CASE(EMBRYO_OP_CONST_ALT);
	GETPARAM(alt);
	BREAK;
	CASE(EMBRYO_OP_ADDR_PRI);
	GETPARAM(pri);
	pri += frm;
	BREAK;
	CASE(EMBRYO_OP_ADDR_ALT);
	GETPARAM(alt);
	alt += frm;
	BREAK;
	CASE(EMBRYO_OP_STOR_PRI);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)offs) = pri;
	BREAK;
	CASE(EMBRYO_OP_STOR_ALT);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)offs) = alt;
	BREAK;
	CASE(EMBRYO_OP_STOR_S_PRI);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)frm + (int)offs) = pri;
	BREAK;
	CASE(EMBRYO_OP_STOR_S_ALT);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)frm + (int)offs) = alt;
	BREAK;
	CASE(EMBRYO_OP_SREF_PRI);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)offs);
	*(Embryo_Cell *)(data + (int)offs) = pri;
	BREAK;
	CASE(EMBRYO_OP_SREF_ALT);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)offs);
	*(Embryo_Cell *)(data + (int)offs) = alt;
	BREAK;
	CASE(EMBRYO_OP_SREF_S_PRI);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	*(Embryo_Cell *)(data + (int)offs) = pri;
	BREAK;
	CASE(EMBRYO_OP_SREF_S_ALT);
	GETPARAM(offs);
	offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	*(Embryo_Cell *)(data + (int)offs) = alt;
	BREAK;
	CASE(EMBRYO_OP_STOR_I);
	CHKMEM(alt);
	*(Embryo_Cell *)(data + (int)alt) = pri;
	BREAK;
	CASE(EMBRYO_OP_STRB_I);
	GETPARAM(offs);
	CHKMEM(alt);
	switch (offs)
	  {
	   case 1:
	     *(data + (int)alt) = (unsigned char)pri;
	     break;
	   case 2:
	     *(unsigned short *)(data + (int)alt) = (unsigned short)pri;
	     break;
	   case 4:
	     *(unsigned int *)(data + (int)alt) = (unsigned int)pri;
	     break;
	   default:
	     ABORT(ep, EMBRYO_ERROR_INVINSTR);
	     break;
	  }
	BREAK;
	CASE(EMBRYO_OP_LIDX);
	offs = (pri * sizeof(Embryo_Cell)) + alt;
	CHKMEM(offs);
	pri = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_LIDX_B);
	GETPARAM(offs);
	offs = (pri << (int)offs) + alt;
	CHKMEM(offs);
	pri = *(Embryo_Cell *)(data + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_IDXADDR);
	pri = (pri * sizeof(Embryo_Cell)) + alt;
	BREAK;
	CASE(EMBRYO_OP_IDXADDR_B);
	GETPARAM(offs);
	pri = (pri << (int)offs) + alt;
	BREAK;
	CASE(EMBRYO_OP_ALIGN_PRI);
	GETPARAM(offs);
#ifdef WORDS_BIGENDIAN
	if ((size_t)offs < sizeof(Embryo_Cell))
	  pri ^= sizeof(Embryo_Cell) - offs;
#endif
	BREAK;
	CASE(EMBRYO_OP_ALIGN_ALT);
	GETPARAM(offs);
#ifdef WORDS_BIGENDIAN
	if ((size_t)offs < sizeof(Embryo_Cell))
	  alt ^= sizeof(Embryo_Cell) - offs;
#endif
	BREAK;
	CASE(EMBRYO_OP_LCTRL);
	GETPARAM(offs);
	switch (offs)
	  {
	   case 0:
	     pri = hdr->cod;
	     break;
	   case 1:
	     pri = hdr->dat;
	     break;
	   case 2:
	     pri = hea;
	     break;
	   case 3:
	     pri = ep->stp;
	     break;
	   case 4:
	     pri = stk;
	     break;
	   case 5:
	     pri = frm;
	     break;
	   case 6:
	     pri = (Embryo_Cell)((unsigned char *)cip - code);
	     break;
	   default:
	     ABORT(ep, EMBRYO_ERROR_INVINSTR);
	     break;
	  }
	BREAK;
	CASE(EMBRYO_OP_SCTRL);
	GETPARAM(offs);
	switch (offs)
	  {
	   case 0:
	   case 1:
	   case 2:
	     hea = pri;
	     break;
	   case 3:
	     /* cannot change these parameters */
	     break;
	   case 4:
	     stk = pri;
	     break;
	   case 5:
	     frm = pri;
	     break;
	   case 6:
	     cip = (Embryo_Cell *)(code + (int)pri);
	     break;
	   default:
	     ABORT(ep, EMBRYO_ERROR_INVINSTR);
	     break;
	  }
	BREAK;
	CASE(EMBRYO_OP_MOVE_PRI);
	pri = alt;
	BREAK;
	CASE(EMBRYO_OP_MOVE_ALT);
	alt = pri;
	BREAK;
	CASE(EMBRYO_OP_XCHG);
	offs = pri;         /* offs is a temporary variable */
	pri = alt;
	alt = offs;
	BREAK;
	CASE(EMBRYO_OP_PUSH_PRI);
	PUSH(pri);
	BREAK;
	CASE(EMBRYO_OP_PUSH_ALT);
	PUSH(alt);
	BREAK;
	CASE(EMBRYO_OP_PUSH_C);
	GETPARAM(offs);
	PUSH(offs);
	BREAK;
	CASE(EMBRYO_OP_PUSH_R);
	GETPARAM(offs);
	while (offs--) PUSH(pri);
	BREAK;
	CASE(EMBRYO_OP_PUSH);
	GETPARAM(offs);
	PUSH(*(Embryo_Cell *)(data + (int)offs));
	BREAK;
	CASE(EMBRYO_OP_PUSH_S);
	GETPARAM(offs);
	PUSH(*(Embryo_Cell *)(data + (int)frm + (int)offs));
	BREAK;
	CASE(EMBRYO_OP_POP_PRI);
	POP(pri);
	BREAK;
	CASE(EMBRYO_OP_POP_ALT);
	POP(alt);
	BREAK;
	CASE(EMBRYO_OP_STACK);
	GETPARAM(offs);
	alt = stk;
	stk += offs;
	CHKMARGIN();
	CHKSTACK();
	BREAK;
	CASE(EMBRYO_OP_HEAP);
	GETPARAM(offs);
	alt = hea;
	hea += offs;
	CHKMARGIN();
	CHKHEAP();
	BREAK;
	CASE(EMBRYO_OP_PROC);
	PUSH(frm);
	frm = stk;
	CHKMARGIN();
	BREAK;
	CASE(EMBRYO_OP_RET);
	POP(frm);
	POP(offs);
	if ((Embryo_UCell)offs >= codesize)
	  ABORT(ep, EMBRYO_ERROR_MEMACCESS);
	cip = (Embryo_Cell *)(code + (int)offs);
	BREAK;
	CASE(EMBRYO_OP_RETN);
	POP(frm);
	POP(offs);
	if ((Embryo_UCell)offs >= codesize)
	  ABORT(ep, EMBRYO_ERROR_MEMACCESS);
	cip = (Embryo_Cell *)(code + (int)offs);
	stk += *(Embryo_Cell *)(data + (int)stk) + sizeof(Embryo_Cell); /* remove parameters from the stack */
	ep->stk = stk;
	BREAK;
	CASE(EMBRYO_OP_CALL);
	PUSH(((unsigned char *)cip - code) + sizeof(Embryo_Cell));/* skip address */
	cip = JUMPABS(code, cip); /* jump to the address */
	BREAK;
	CASE(EMBRYO_OP_CALL_PRI);
	PUSH((unsigned char *)cip - code);
	cip = (Embryo_Cell *)(code + (int)pri);
	BREAK;
	CASE(EMBRYO_OP_JUMP);
	/* since the GETPARAM() macro modifies cip, you cannot
	 * do GETPARAM(cip) directly */
	cip = JUMPABS(code, cip);
	BREAK;
	CASE(EMBRYO_OP_JREL);
	offs = *cip;
	cip = (Embryo_Cell *)((unsigned char *)cip + (int)offs + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JZER);
	if (pri == 0)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JNZ);
	if (pri != 0)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JEQ);
	if (pri==alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JNEQ);
	if (pri != alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JLESS);
	if ((Embryo_UCell)pri < (Embryo_UCell)alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JLEQ);
	if ((Embryo_UCell)pri <= (Embryo_UCell)alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JGRTR);
	if ((Embryo_UCell)pri > (Embryo_UCell)alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JGEQ);
	if ((Embryo_UCell)pri >= (Embryo_UCell)alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JSLESS);
	if (pri < alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JSLEQ);
	if (pri <= alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JSGRTR);
	if (pri > alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_JSGEQ);
	if (pri >= alt)
	  cip = JUMPABS(code, cip);
	else
	  cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	BREAK;
	CASE(EMBRYO_OP_SHL);
	pri <<= alt;
	BREAK;
	CASE(EMBRYO_OP_SHR);
	pri = (Embryo_UCell)pri >> (int)alt;
	BREAK;
	CASE(EMBRYO_OP_SSHR);
	pri >>= alt;
	BREAK;
	CASE(EMBRYO_OP_SHL_C_PRI);
	GETPARAM(offs);
	pri <<= offs;
	BREAK;
	CASE(EMBRYO_OP_SHL_C_ALT);
	GETPARAM(offs);
	alt <<= offs;
	BREAK;
	CASE(EMBRYO_OP_SHR_C_PRI);
	GETPARAM(offs);
	pri = (Embryo_UCell)pri >> (int)offs;
	BREAK;
	CASE(EMBRYO_OP_SHR_C_ALT);
	GETPARAM(offs);
	alt = (Embryo_UCell)alt >> (int)offs;
	BREAK;
	CASE(EMBRYO_OP_SMUL);
	pri *= alt;
	BREAK;
	CASE(EMBRYO_OP_SDIV);
	if (alt == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	/* divide must always round down; this is a bit
	 * involved to do in a machine-independent way.
	 */
	offs = ((pri % alt) + alt) % alt; /* true modulus */
	pri = (pri - offs) / alt;         /* division result */
	alt = offs;
	BREAK;
	CASE(EMBRYO_OP_SDIV_ALT);
	if (pri == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	/* divide must always round down; this is a bit
	 * involved to do in a machine-independent way.
	 */
	offs = ((alt % pri) + pri) % pri; /* true modulus */
	pri = (alt - offs) / pri;         /* division result */
	alt = offs;
	BREAK;
	CASE(EMBRYO_OP_UMUL);
	pri = (Embryo_UCell)pri * (Embryo_UCell)alt;
	BREAK;
	CASE(EMBRYO_OP_UDIV);
	if (alt == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	offs = (Embryo_UCell)pri % (Embryo_UCell)alt; /* temporary storage */
	pri = (Embryo_UCell)pri / (Embryo_UCell)alt;
	alt = offs;
	BREAK;
	CASE(EMBRYO_OP_UDIV_ALT);
	if (pri == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	offs = (Embryo_UCell)alt % (Embryo_UCell)pri; /* temporary storage */
	pri = (Embryo_UCell)alt / (Embryo_UCell)pri;
	alt = offs;
	BREAK;
	CASE(EMBRYO_OP_ADD);
	pri += alt;
	BREAK;
	CASE(EMBRYO_OP_SUB);
	pri -= alt;
	BREAK;
	CASE(EMBRYO_OP_SUB_ALT);
	pri = alt - pri;
	BREAK;
	CASE(EMBRYO_OP_AND);
	pri &= alt;
	BREAK;
	CASE(EMBRYO_OP_OR);
	pri |= alt;
	BREAK;
	CASE(EMBRYO_OP_XOR);
	pri ^= alt;
	BREAK;
	CASE(EMBRYO_OP_NOT);
	pri = !pri;
	BREAK;
	CASE(EMBRYO_OP_NEG);
	pri = -pri;
	BREAK;
	CASE(EMBRYO_OP_INVERT);
	pri = ~pri;
	BREAK;
	CASE(EMBRYO_OP_ADD_C);
	GETPARAM(offs);
	pri += offs;
	BREAK;
	CASE(EMBRYO_OP_SMUL_C);
	GETPARAM(offs);
	pri *= offs;
	BREAK;
	CASE(EMBRYO_OP_ZERO_PRI);
	pri = 0;
	BREAK;
	CASE(EMBRYO_OP_ZERO_ALT);
	alt = 0;
	BREAK;
	CASE(EMBRYO_OP_ZERO);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)offs) = 0;
	BREAK;
	CASE(EMBRYO_OP_ZERO_S);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)frm + (int)offs) = 0;
	BREAK;
	CASE(EMBRYO_OP_SIGN_PRI);
	if ((pri & 0xff) >= 0x80) pri |= ~(Embryo_UCell)0xff;
	BREAK;
	CASE(EMBRYO_OP_SIGN_ALT);
	if ((alt & 0xff) >= 0x80) alt |= ~(Embryo_UCell)0xff;
	BREAK;
	CASE(EMBRYO_OP_EQ);
	pri = (pri == alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_NEQ);
	pri = (pri != alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_LESS);
	pri = ((Embryo_UCell)pri < (Embryo_UCell)alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_LEQ);
	pri = ((Embryo_UCell)pri <= (Embryo_UCell)alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_GRTR);
	pri = ((Embryo_UCell)pri > (Embryo_UCell)alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_GEQ);
	pri = ((Embryo_UCell)pri >= (Embryo_UCell)alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_SLESS);
	pri = (pri < alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_SLEQ);
	pri = (pri <= alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_SGRTR);
	pri = (pri > alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_SGEQ);
	pri = (pri >= alt) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_EQ_C_PRI);
	GETPARAM(offs);
	pri = (pri == offs) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_EQ_C_ALT);
	GETPARAM(offs);
	pri = (alt == offs) ? 1 : 0;
	BREAK;
	CASE(EMBRYO_OP_INC_PRI);
	pri++;
	BREAK;
	CASE(EMBRYO_OP_INC_ALT);
	alt++;
	BREAK;
	CASE(EMBRYO_OP_INC);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)offs) += 1;
	BREAK;
	CASE(EMBRYO_OP_INC_S);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)frm + (int)offs) += 1;
	BREAK;
	CASE(EMBRYO_OP_INC_I);
	*(Embryo_Cell *)(data + (int)pri) += 1;
	BREAK;
	CASE(EMBRYO_OP_DEC_PRI);
	pri--;
	BREAK;
	CASE(EMBRYO_OP_DEC_ALT);
	alt--;
	BREAK;
	CASE(EMBRYO_OP_DEC);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)offs) -= 1;
	BREAK;
	CASE(EMBRYO_OP_DEC_S);
	GETPARAM(offs);
	*(Embryo_Cell *)(data + (int)frm + (int)offs) -= 1;
	BREAK;
	CASE(EMBRYO_OP_DEC_I);
	*(Embryo_Cell *)(data + (int)pri) -= 1;
	BREAK;
	CASE(EMBRYO_OP_MOVS);
	GETPARAM(offs);
	CHKMEM(pri);
	CHKMEM(pri + offs);
	CHKMEM(alt);
	CHKMEM(alt + offs);
	memcpy(data+(int)alt, data+(int)pri, (int)offs);
	BREAK;
	CASE(EMBRYO_OP_CMPS);
	GETPARAM(offs);
	CHKMEM(pri);
	CHKMEM(pri + offs);
	CHKMEM(alt);
	CHKMEM(alt + offs);
	pri = memcmp(data + (int)alt, data + (int)pri, (int)offs);
	BREAK;
	CASE(EMBRYO_OP_FILL);
	GETPARAM(offs);
	CHKMEM(alt);
	CHKMEM(alt + offs);
	for (i = (int)alt;
	     (size_t)offs >= sizeof(Embryo_Cell);
	     i += sizeof(Embryo_Cell), offs -= sizeof(Embryo_Cell))
	  *(Embryo_Cell *)(data + i) = pri;
	BREAK;
	CASE(EMBRYO_OP_HALT);
	GETPARAM(offs);
	ep->retval = pri;
	/* store complete status */
	ep->frm = frm;
	ep->stk = stk;
	ep->hea = hea;
	ep->pri = pri;
	ep->alt = alt;
	ep->cip = (Embryo_Cell)((unsigned char*)cip - code);
	if (offs == EMBRYO_ERROR_SLEEP)
	  {
	     ep->reset_stk = reset_stk;
	     ep->reset_hea = reset_hea;
	     ep->run_count--;
	     return EMBRYO_PROGRAM_SLEEP;
	  }
	OK(ep, (int)offs);
	CASE(EMBRYO_OP_BOUNDS);
	GETPARAM(offs);
	if ((Embryo_UCell)pri > (Embryo_UCell)offs)
	  ABORT(ep, EMBRYO_ERROR_BOUNDS);
	BREAK;
	CASE(EMBRYO_OP_SYSREQ_PRI);
	/* save a few registers */
	ep->cip = (Embryo_Cell)((unsigned char *)cip - code);
	ep->hea = hea;
	ep->frm = frm;
	ep->stk = stk;
	num = _embryo_native_call(ep, pri, &pri, (Embryo_Cell *)(data + (int)stk));
	if (num != EMBRYO_ERROR_NONE)
	  {
	     if (num == EMBRYO_ERROR_SLEEP)
	       {
		  ep->pri = pri;
		  ep->alt = alt;
		  ep->reset_stk = reset_stk;
		  ep->reset_hea = reset_hea;
		  ep->run_count--;
		  return EMBRYO_PROGRAM_SLEEP;
	       }
	     ABORT(ep, num);
	  }
	BREAK;
	CASE(EMBRYO_OP_SYSREQ_C);
	GETPARAM(offs);
	/* save a few registers */
	ep->cip = (Embryo_Cell)((unsigned char *)cip - code);
	ep->hea = hea;
	ep->frm = frm;
	ep->stk = stk;
	num = _embryo_native_call(ep, offs, &pri, (Embryo_Cell *)(data + (int)stk));
	if (num != EMBRYO_ERROR_NONE)
	  {
	     if (num == EMBRYO_ERROR_SLEEP)
	       {
		  ep->pri = pri;
		  ep->alt = alt;
		  ep->reset_stk = reset_stk;
		  ep->reset_hea = reset_hea;
		  ep->run_count--;
		  return EMBRYO_PROGRAM_SLEEP;
	       }
	       {
		  Embryo_Header    *hdr;
		  int i, num;
		  Embryo_Func_Stub *func_entry;

		  hdr = (Embryo_Header *)ep->code;
		  num = NUMENTRIES(hdr, natives, libraries);
		  func_entry = GETENTRY(hdr, natives, 0);
		  for (i = 0; i < num; i++)
		    {
		       char *entry_name;

		       entry_name = GETENTRYNAME(hdr, func_entry);
		       if (i == offs)
			 printf("EMBRYO: CALL [%i] %s() non-existant!\n", i, entry_name);
		       func_entry =
			 (Embryo_Func_Stub *)((unsigned char *)func_entry + hdr->defsize);
		    }
	       }
	     ABORT(ep, num);
	  }
	BREAK;
	CASE(EMBRYO_OP_SYSREQ_D);
	GETPARAM(offs);
	/* save a few registers */
	ep->cip = (Embryo_Cell)((unsigned char *)cip - code);
	ep->hea = hea;
	ep->frm = frm;
	ep->stk = stk;
	num = _embryo_native_call(ep, offs, &pri, (Embryo_Cell *)(data + (int)stk));
	if (num != EMBRYO_ERROR_NONE)
	  {
	     if (num == EMBRYO_ERROR_SLEEP)
	       {
		  ep->pri = pri;
		  ep->alt = alt;
		  ep->reset_stk = reset_stk;
		  ep->reset_hea = reset_hea;
		  ep->run_count--;
		  return EMBRYO_PROGRAM_SLEEP;
	       }
	     ABORT(ep, ep->error);
	  }
	BREAK;
	CASE(EMBRYO_OP_JUMP_PRI);
	cip = (Embryo_Cell *)(code + (int)pri);
	BREAK;
	CASE(EMBRYO_OP_SWITCH);
	  {
	     Embryo_Cell *cptr;

	     /* +1, to skip the "casetbl" opcode */
	     cptr = (Embryo_Cell *)(code + (*cip)) + 1;
	     /* number of records in the case table */
	     num = (int)(*cptr);
	     /* preset to "none-matched" case */
	     cip = (Embryo_Cell *)(code + *(cptr + 1));
	     for (cptr += 2;
		  (num > 0) && (*cptr != pri);
		  num--, cptr += 2);
	     /* case found */
	     if (num > 0)
	       cip = (Embryo_Cell *)(code + *(cptr + 1));
	  }
	BREAK;
	CASE(EMBRYO_OP_SWAP_PRI);
	offs = *(Embryo_Cell *)(data + (int)stk);
	*(Embryo_Cell *)(data + (int)stk) = pri;
	pri = offs;
	BREAK;
	CASE(EMBRYO_OP_SWAP_ALT);
	offs = *(Embryo_Cell *)(data + (int)stk);
	*(Embryo_Cell *)(data + (int)stk) = alt;
	alt = offs;
	BREAK;
	CASE(EMBRYO_OP_PUSHADDR);
	GETPARAM(offs);
	PUSH(frm + offs);
	BREAK;
	CASE(EMBRYO_OP_NOP);
	BREAK;
	CASE(EMBRYO_OP_NONE);
	CASE(EMBRYO_OP_FILE);
	CASE(EMBRYO_OP_LINE);
	CASE(EMBRYO_OP_SYMBOL);
	CASE(EMBRYO_OP_SRANGE);
	CASE(EMBRYO_OP_CASETBL);
	CASE(EMBRYO_OP_SYMTAG);
	BREAK;
#ifndef EMBRYO_EXEC_JUMPTABLE
      default:
	ABORT(ep, EMBRYO_ERROR_INVINSTR);
#endif
	SWITCHEND;
     }
   ep->max_run_cycles = max_run_cycles;
   ep->run_count--;
   ep->hea = hea_start;
   return EMBRYO_PROGRAM_OK;
}

/**
 * Retreives the return value of the last called function of the given
 * program.
 * @param   ep The given program.
 * @return  An Embryo_Cell representing the return value of the function
 *          that was last called.
 * @ingroup Embryo_Run_Group
 */
EAPI Embryo_Cell
embryo_program_return_value_get(Embryo_Program *ep)
{
   if (!ep) return 0;
   return ep->retval;
}

/**
 * Sets the maximum number of abstract machine cycles any given program run
 * can execute before being put to sleep and returning.
 *
 * @param   ep The given program.
 * @param   max The number of machine cycles as a limit.
 *
 * This sets the maximum number of abstract machine (virtual machine)
 * instructions that a single run of an embryo function (even if its main)
 * can use before embryo embryo_program_run() reutrns with the value
 * EMBRYO_PROGRAM_TOOLONG. If the function fully executes within this number
 * of cycles, embryo_program_run() will return as normal with either
 * EMBRYO_PROGRAM_OK, EMBRYO_PROGRAM_FAIL or EMBRYO_PROGRAM_SLEEP. If the
 * run exceeds this instruction count, then EMBRYO_PROGRAM_TOOLONG will be
 * returned indicating the program exceeded its run count. If the app wishes
 * to continue running this anyway - it is free to process its own events or
 * whatever it wants and continue the function by calling
 * embryo_program_run(program, EMBRYO_FUNCTION_CONT); which will start the
 * run again until the instruction count is reached. This can keep being done
 * to allow the calling program to still be able to control things outside the
 * embryo function being called. If the maximum run cycle count is 0 then the
 * program is allowed to run forever only returning when it is done.
 *
 * It is important to note that abstract machine cycles are NOT the same as
 * the host machine cpu cycles. They are not fixed in runtime per cycle, so
 * this is more of a helper tool than a way to HARD-FORCE a script to only
 * run for a specific period of time. If the cycle count is set to something
 * low like 5000 or 1000, then every 1000 (or 5000) cycles control will be
 * returned to the calling process where it can check a timer to see if a
 * physical runtime limit has been elapsed and then abort runing further
 * assuming a "runaway script" or keep continuing the script run. This
 * limits resolution to only that many cycles which do not take a determined
 * amount of time to execute, as this varies from cpu to cpu and also depends
 * on how loaded the system is. Making the max cycle run too low will
 * impact performance requiring the abstract machine to do setup and teardown
 * cycles too often comapred to cycles actually executed.
 *
 * Also note it does NOT include nested abstract machines. IF this abstract
 * machine run calls embryo script that calls a native function that in turn
 * calls more embryo script, then the 2nd (and so on) levels are not included
 * in this run count. They can set their own max instruction count values
 * separately.
 *
 * The default max cycle run value is 0 in any program until set with this
 * function.
 *
 * @ingroup Embryo_Run_Group
 */
EAPI void
embryo_program_max_cycle_run_set(Embryo_Program *ep, int max)
{
   if (!ep) return;
   if (max < 0) max = 0;
   ep->max_run_cycles = max;
}

/**
 * Retreives the maximum number of abstract machine cycles a program is allowed
 * to run.
 * @param   ep The given program.
 * @return  The number of cycles a run cycle is allowed to run for this
 *          program.
 *
 * This returns the value set by embryo_program_max_cycle_run_set(). See
 * embryo_program_max_cycle_run_set() for more information.
 *
 * @ingroup Embryo_Run_Group
 */
EAPI int
embryo_program_max_cycle_run_get(Embryo_Program *ep)
{
   if (!ep) return 0;
   return ep->max_run_cycles;
}

/**
 * @defgroup Embryo_Parameter_Group Function Parameter Functions
 *
 * Functions that set parameters for the next function that is called.
 */

/**
 * Pushes an Embryo_Cell onto the function stack to use as a parameter for
 * the next function that is called in the given program.
 * @param   ep   The given program.
 * @param   cell The Embryo_Cell to push onto the stack.
 * @return  @c 1 if successful.  @c 0 otherwise.
 * @ingroup Embryo_Parameter_Group
 */
EAPI int
embryo_parameter_cell_push(Embryo_Program *ep, Embryo_Cell cell)
{
   Embryo_Param *pr;

   ep->params_size++;
   if (ep->params_size > ep->params_alloc)
     {
	ep->params_alloc += 8;
	pr = realloc(ep->params, ep->params_alloc * sizeof(Embryo_Param));
	if (!pr) return 0;
	ep->params = pr;
     }
   pr = &(ep->params[ep->params_size - 1]);
   pr->string = NULL;
   pr->cell_array = NULL;
   pr->cell_array_size = 0;
   pr->cell = 0;
   pr->cell = cell;
   return 1;
}

/**
 * Pushes a string onto the function stack to use as a parameter for the
 * next function that is called in the given program.
 * @param   ep The given program.
 * @param   str The string to push onto the stack.
 * @return  @c 1 if successful.  @c 0 otherwise.
 * @ingroup Embryo_Parameter_Group
 */
EAPI int
embryo_parameter_string_push(Embryo_Program *ep, const char *str)
{
   Embryo_Param *pr;
   char *str_dup;

   if (!str)
     return embryo_parameter_string_push(ep, "");
   str_dup = strdup(str);
   if (!str_dup) return 0;
   ep->params_size++;
   if (ep->params_size > ep->params_alloc)
     {
	ep->params_alloc += 8;
	pr = realloc(ep->params, ep->params_alloc * sizeof(Embryo_Param));
	if (!pr)
	  {
	     free(str_dup);
	     return 0;
	  }
	ep->params = pr;
     }
   pr = &(ep->params[ep->params_size - 1]);
   pr->string = NULL;
   pr->cell_array = NULL;
   pr->cell_array_size = 0;
   pr->cell = 0;
   pr->string = str_dup;
   return 1;
}

/**
 * Pushes an array of Embryo_Cells onto the function stack to be used as
 * parameters for the next function that is called in the given program.
 * @param   ep    The given program.
 * @param   cells The array of Embryo_Cells.
 * @param   num   The number of cells in @p cells.
 * @return  @c 1 if successful.  @c 0 otherwise.
 * @ingroup Embryo_Parameter_Group
 */
EAPI int
embryo_parameter_cell_array_push(Embryo_Program *ep, Embryo_Cell *cells, int num)
{
   Embryo_Param *pr;
   Embryo_Cell *cell_array;

   cell_array = malloc(num * sizeof(Embryo_Cell));
   if ((!cells) || (num <= 0))
     return embryo_parameter_cell_push(ep, 0);
   ep->params_size++;
   if (ep->params_size > ep->params_alloc)
     {
	ep->params_alloc += 8;
	pr = realloc(ep->params, ep->params_alloc * sizeof(Embryo_Param));
	if (!pr)
	  {
	     free(cell_array);
	     return 0;
	  }
	ep->params = pr;
     }
   pr = &(ep->params[ep->params_size - 1]);
   pr->string = NULL;
   pr->cell_array = NULL;
   pr->cell_array_size = 0;
   pr->cell = 0;
   pr->cell_array = cell_array;
   pr->cell_array_size = num;
   memcpy(pr->cell_array, cells, num * sizeof(Embryo_Cell));
   return 1;
}
