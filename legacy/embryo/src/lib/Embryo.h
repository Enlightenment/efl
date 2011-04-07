#ifndef _EMBRYO_H
#define _EMBRYO_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EMBRYO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EMBRYO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef  __cplusplus
extern "C" {
#endif

#define EMBRYO_VERSION_MAJOR 1
#define EMBRYO_VERSION_MINOR 0
   
   typedef struct _Embryo_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Embryo_Version;
   
   EAPI extern Embryo_Version *embryo_version;
   
   /* potential error values */
   typedef enum _Embryo_Error
     {
	EMBRYO_ERROR_NONE,
	  /* reserve the first 15 error codes for exit codes of the abstract machine */
	  EMBRYO_ERROR_EXIT,         /** Forced exit */
	  EMBRYO_ERROR_ASSERT,       /** Assertion failed */
	  EMBRYO_ERROR_STACKERR,     /** Stack/heap collision */
	  EMBRYO_ERROR_BOUNDS,       /** Index out of bounds */
	  EMBRYO_ERROR_MEMACCESS,    /** Invalid memory access */
	  EMBRYO_ERROR_INVINSTR,     /** Invalid instruction */
	  EMBRYO_ERROR_STACKLOW,     /** Stack underflow */
	  EMBRYO_ERROR_HEAPLOW,      /** Heap underflow */
	  EMBRYO_ERROR_CALLBACK,     /** No callback, or invalid callback */
	  EMBRYO_ERROR_NATIVE,       /** Native function failed */
	  EMBRYO_ERROR_DIVIDE,       /** Divide by zero */
	  EMBRYO_ERROR_SLEEP,        /** Go into sleepmode - code can be restarted */

	  EMBRYO_ERROR_MEMORY = 16,  /** Out of memory */
	  EMBRYO_ERROR_FORMAT,       /** Invalid file format */
	  EMBRYO_ERROR_VERSION,      /** File is for a newer version of the Embryo_Program */
	  EMBRYO_ERROR_NOTFOUND,     /** Function not found */
	  EMBRYO_ERROR_INDEX,        /** Invalid index parameter (bad entry point) */
	  EMBRYO_ERROR_DEBUG,        /** Debugger cannot run */
	  EMBRYO_ERROR_INIT,         /** Embryo_Program not initialized (or doubly initialized) */
	  EMBRYO_ERROR_USERDATA,     /** Unable to set user data field (table full) */
	  EMBRYO_ERROR_INIT_JIT,     /** Cannot initialize the JIT */
	  EMBRYO_ERROR_PARAMS,       /** Parameter error */
	  EMBRYO_ERROR_DOMAIN,       /** Domain error, expression result does not fit in range */
     } Embryo_Error;

   /* program run return values */
   typedef enum _Embryo_Status
     {
        EMBRYO_PROGRAM_FAIL = 0,
        EMBRYO_PROGRAM_OK = 1,
        EMBRYO_PROGRAM_SLEEP = 2,
        EMBRYO_PROGRAM_BUSY = 3,
        EMBRYO_PROGRAM_TOOLONG = 4
     } Embryo_Status;
   
   typedef unsigned int                Embryo_UCell;
   typedef int                         Embryo_Cell;
  /** An invalid cell reference */
#define EMBRYO_CELL_NONE     0x7fffffff
   
   typedef struct _Embryo_Program      Embryo_Program;
   typedef int                         Embryo_Function;
   /* possible function type values that are enumerated */
#define EMBRYO_FUNCTION_NONE 0x7fffffff /* An invalid/non existent function */
#define EMBRYO_FUNCTION_MAIN -1         /* Start at program entry point */
#define EMBRYO_FUNCTION_CONT -2         /* Continue from last address */

   typedef union
     {
	float       f;
	Embryo_Cell c;
     } Embryo_Float_Cell;

#if defined _MSC_VER || defined __SUNPRO_C
/** Float to Embryo_Cell */
# define EMBRYO_FLOAT_TO_CELL(f) (((Embryo_Float_Cell *)&(f))->c)
/** Embryo_Cell to float */
# define EMBRYO_CELL_TO_FLOAT(c) (((Embryo_Float_Cell *)&(c))->f)
#else
/** Float to Embryo_Cell */
# define EMBRYO_FLOAT_TO_CELL(f) ((Embryo_Float_Cell) f).c
/** Embryo_Cell to float */
# define EMBRYO_CELL_TO_FLOAT(c) ((Embryo_Float_Cell) c).f
#endif

   /**
    * @defgroup Embryo_Library_Group Library Maintenance Functions
    *
    * Functions that start up and shutdown the Embryo library.
    */
   
   
/**
 * Initialises the Embryo library.
 * @return  The number of times the library has been initialised without being
 *          shut down.
 * @ingroup Embryo_Library_Group
 */
EAPI int              embryo_init(void);
   
/**
 * Shuts down the Embryo library.
 * @return  The number of times the library has been initialised without being
 *          shutdown.
 * @ingroup Embryo_Library_Group
 */
EAPI int              embryo_shutdown(void);

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
EAPI Embryo_Program  *embryo_program_new(void *data, int size);
   
/**
 * Creates a new Embryo program, with bytecode data that cannot be
 * freed.
 * @param   data Pointer to the bytecode of the program.
 * @param   size Number of bytes of bytecode.
 * @return  A new Embryo program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI Embryo_Program  *embryo_program_const_new(void *data, int size);
   
/**
 * Creates a new Embryo program based on the bytecode data stored in the
 * given file.
 * @param   file Filename of the given file.
 * @return  A new Embryo program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI Embryo_Program  *embryo_program_load(const char *file);
   
/**
 * Frees the given Embryo program.
 * @param   ep The given program.
 * @ingroup Embryo_Program_Creation_Group
 */
EAPI void             embryo_program_free(Embryo_Program *ep);
   
/**
 * Adds a native program call to the given Embryo program.
 * @param   ep   The given Embryo program.
 * @param   name The name for the call used in the script.
 * @param   func The function to use when the call is made.
 * @ingroup Embryo_Func_Group
 */

/**
 * @defgroup Embryo_Func_Group Function Functions
 *
 * Functions that deal with Embryo program functions.
 */
EAPI void             embryo_program_native_call_add(Embryo_Program *ep, const char *name, Embryo_Cell (*func) (Embryo_Program *ep, Embryo_Cell *params));
   
/**
 * Resets the current virtual machine session of the given program.
 * @param   ep The given program.
 * @ingroup Embryo_Program_VM_Group
 */

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
EAPI void             embryo_program_vm_reset(Embryo_Program *ep);
   
/**
 * Starts a new virtual machine session for the given program.
 *
 * See @ref Embryo_Program_VM_Group for more information about how this works.
 *
 * @param   ep The given program.
 * @ingroup Embryo_Program_VM_Group
 */
EAPI void             embryo_program_vm_push(Embryo_Program *ep);
   
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
EAPI void             embryo_program_vm_pop(Embryo_Program *ep);
   
/**
 * Ensures that the given unsigned short integer is in the small
 * endian format.
 * @param   v Pointer to the given integer.
 * @ingroup Embryo_Swap_Group
 */

/**
 * @defgroup Embryo_Swap_Group Byte Swapping Functions
 *
 * Functions that are used to ensure that integers passed to the
 * virtual machine are in small endian format.  These functions are
 * used to ensure that the virtual machine operates correctly on big
 * endian machines.
 */
EAPI void             embryo_swap_16(unsigned short *v);
   
/**
 * Ensures that the given unsigned integer is in the small endian
 * format.
 * @param   v Pointer to the given integer.
 * @ingroup Embryo_Swap_Group
 */
EAPI void             embryo_swap_32(unsigned int *v);
   
/**
 * Returns the function in the given program with the given name.
 * @param   ep The given program.
 * @param   name The given function name.
 * @return  The function if successful.  Otherwise, @c EMBRYO_FUNCTION_NONE.
 * @ingroup Embryo_Func_Group
 */
EAPI Embryo_Function  embryo_program_function_find(Embryo_Program *ep, const char *name);
   
/**
 * Retrieves the location of the public variable in the given program
 * with the given name.
 * @param   ep   The given program.
 * @param   name The given name.
 * @return  The address of the variable if found.  @c EMBRYO_CELL_NONE
 *          otherwise.
 * @ingroup Embryo_Public_Variable_Group
 */

/**
 * @defgroup Embryo_Public_Variable_Group Public Variable Access Functions
 *
 * In an Embryo program, a global variable can be declared public, as
 * described in @ref Small_Scope_Subsection.  The functions here allow
 * the host program to access these public variables.
 */
EAPI Embryo_Cell      embryo_program_variable_find(Embryo_Program *ep, const char *name);
   
/**
 * Retrieves the number of public variables in the given program.
 * @param   ep The given program.
 * @return  The number of public variables.
 * @ingroup Embryo_Public_Variable_Group
 */
EAPI int              embryo_program_variable_count_get(Embryo_Program *ep);
   
/**
 * Retrieves the location of the public variable in the given program
 * with the given identifier.
 * @param   ep  The given program.
 * @param   num The identifier of the public variable.
 * @return  The virtual machine address of the variable if found.
 *          @c EMBRYO_CELL_NONE otherwise.
 * @ingroup Embryo_Public_Variable_Group
 */
EAPI Embryo_Cell      embryo_program_variable_get(Embryo_Program *ep, int num);
   
/**
 * Sets the error code for the given program to the given code.
 * @param   ep The given program.
 * @param   error The given error code.
 * @ingroup Embryo_Error_Group
 */

/**
 * @defgroup Embryo_Error_Group Error Functions
 *
 * Functions that set and retrieve error codes in Embryo programs.
 */
EAPI void             embryo_program_error_set(Embryo_Program *ep, Embryo_Error error);
   
/**
 * Retrieves the current error code for the given program.
 * @param   ep The given program.
 * @return  The current error code.
 * @ingroup Embryo_Error_Group
 */
EAPI Embryo_Error     embryo_program_error_get(Embryo_Program *ep);
   
/**
 * Sets the data associated to the given program.
 * @param   ep   The given program.
 * @param   data New bytecode data.
 * @ingroup Embryo_Program_Data_Group
 */

/**
 * @defgroup Embryo_Program_Data_Group Program Data Functions
 *
 * Functions that set and retrieve data associated with the given
 * program.
 */
EAPI void             embryo_program_data_set(Embryo_Program *ep, void *data);
   
/**
 * Retrieves the data associated to the given program.
 * @param   ep The given program.
 * @ingroup Embryo_Program_Data_Group
 */
EAPI void            *embryo_program_data_get(Embryo_Program *ep);
   
/**
 * Retrieves a string describing the given error code.
 * @param   error The given error code.
 * @return  String describing the given error code.  If the given code is not
 *          known, the string "(unknown)" is returned.
 * @ingroup Embryo_Error_Group
 */
EAPI const char      *embryo_error_string_get(Embryo_Error error);
   
/**
 * Retrieves the length of the string starting at the given cell.
 * @param   ep       The program the cell is part of.
 * @param   str_cell Pointer to the first cell of the string.
 * @return  The length of the string.  @c 0 is returned if there is an error.
 * @ingroup Embryo_Data_String_Group
 */

/**
 * @defgroup Embryo_Data_String_Group Embryo Data String Functions
 *
 * Functions that operate on strings in the memory of a virtual machine.
 */
EAPI int              embryo_data_string_length_get(Embryo_Program *ep, Embryo_Cell *str_cell);
   
/**
 * Copies the string starting at the given cell to the given buffer.
 * @param   ep       The program the cell is part of.
 * @param   str_cell Pointer to the first cell of the string.
 * @param   dst      The given buffer.
 * @ingroup Embryo_Data_String_Group
 */
EAPI void             embryo_data_string_get(Embryo_Program *ep, Embryo_Cell *str_cell, char *dst);
   
/**
 * Copies string in the given buffer into the virtual machine memory
 * starting at the given cell.
 * @param ep       The program the cell is part of.
 * @param src      The given buffer.
 * @param str_cell Pointer to the first cell to copy the string to.
 * @ingroup Embryo_Data_String_Group
 */
EAPI void             embryo_data_string_set(Embryo_Program *ep, const char *src, Embryo_Cell *str_cell);
   
/**
 * Retreives a pointer to the address in the virtual machine given by the
 * given cell.
 * @param   ep   The program whose virtual machine address is being queried.
 * @param   addr The given cell.
 * @return  A pointer to the cell at the given address.
 * @ingroup Embryo_Data_String_Group
 */
EAPI Embryo_Cell     *embryo_data_address_get(Embryo_Program *ep, Embryo_Cell addr);
   
/**
 * Increases the size of the heap of the given virtual machine by the given
 * number of Embryo_Cells.
 * @param   ep    The program with the given virtual machine.
 * @param   cells The given number of Embryo_Cells.
 * @return  The address of the new memory region on success.
 *          @c EMBRYO_CELL_NONE otherwise.
 * @ingroup Embryo_Heap_Group
 */

/**
 * @defgroup Embryo_Heap_Group Heap Functions
 *
 * The heap is an area of memory that can be allocated for program
 * use at runtime.  The heap functions here change the amount of heap
 * memory available.
 */
EAPI Embryo_Cell      embryo_data_heap_push(Embryo_Program *ep, int cells);
   
/**
 * Decreases the size of the heap of the given virtual machine down to the
 * given size.
 * @param   ep      The program with the given virtual machine.
 * @param   down_to The given size.
 * @ingroup Embryo_Heap_Group
 */
EAPI void             embryo_data_heap_pop(Embryo_Program *ep, Embryo_Cell down_to);
   
/**
 * Returns the number of virtual machines are running for the given program.
 * @param   ep The given program.
 * @return  The number of virtual machines running.
 * @ingroup Embryo_Run_Group
 */

/**
 * @defgroup Embryo_Run_Group Program Run Functions
 *
 * Functions that are involved in actually running functions in an
 * Embryo program.
 */
EAPI int              embryo_program_recursion_get(Embryo_Program *ep);
   
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
EAPI Embryo_Status    embryo_program_run(Embryo_Program *ep, Embryo_Function func);
   
/**
 * Retreives the return value of the last called function of the given
 * program.
 * @param   ep The given program.
 * @return  An Embryo_Cell representing the return value of the function
 *          that was last called.
 * @ingroup Embryo_Run_Group
 */
EAPI Embryo_Cell      embryo_program_return_value_get(Embryo_Program *ep);
   
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
 * physical runtime limit has been elapsed and then abort running further
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
EAPI void             embryo_program_max_cycle_run_set(Embryo_Program *ep, int max);
   
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
EAPI int              embryo_program_max_cycle_run_get(Embryo_Program *ep);
   
/**
 * Pushes an Embryo_Cell onto the function stack to use as a parameter for
 * the next function that is called in the given program.
 * @param   ep   The given program.
 * @param   cell The Embryo_Cell to push onto the stack.
 * @return  @c 1 if successful.  @c 0 otherwise.
 * @ingroup Embryo_Parameter_Group
 */

/**
 * @defgroup Embryo_Parameter_Group Function Parameter Functions
 *
 * Functions that set parameters for the next function that is called.
 */
EAPI int              embryo_parameter_cell_push(Embryo_Program *ep, Embryo_Cell cell);
   
/**
 * Pushes a string onto the function stack to use as a parameter for the
 * next function that is called in the given program.
 * @param   ep The given program.
 * @param   str The string to push onto the stack.
 * @return  @c 1 if successful.  @c 0 otherwise.
 * @ingroup Embryo_Parameter_Group
 */
EAPI int              embryo_parameter_string_push(Embryo_Program *ep, const char *str);
   
/**
 * Pushes an array of Embryo_Cells onto the function stack to be used as
 * parameters for the next function that is called in the given program.
 * @param   ep    The given program.
 * @param   cells The array of Embryo_Cells.
 * @param   num   The number of cells in @p cells.
 * @return  @c 1 if successful.  @c 0 otherwise.
 * @ingroup Embryo_Parameter_Group
 */
EAPI int              embryo_parameter_cell_array_push(Embryo_Program *ep, Embryo_Cell *cells, int num);

#ifdef  __cplusplus
}
#endif

#endif
