#ifndef _EMBRYO_PRIVATE_H
#define _EMBRYO_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <eina_types.h>

#include "Embryo.h"

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

typedef enum _Embryo_Opcode Embryo_Opcode;

enum _Embryo_Opcode
{
   EMBRYO_OP_NONE,
     EMBRYO_OP_LOAD_PRI,
     EMBRYO_OP_LOAD_ALT,
     EMBRYO_OP_LOAD_S_PRI,
     EMBRYO_OP_LOAD_S_ALT,
     EMBRYO_OP_LREF_PRI,
     EMBRYO_OP_LREF_ALT,
     EMBRYO_OP_LREF_S_PRI,
     EMBRYO_OP_LREF_S_ALT,
     EMBRYO_OP_LOAD_I,
     EMBRYO_OP_LODB_I,
     EMBRYO_OP_CONST_PRI,
     EMBRYO_OP_CONST_ALT,
     EMBRYO_OP_ADDR_PRI,
     EMBRYO_OP_ADDR_ALT,
     EMBRYO_OP_STOR_PRI,
     EMBRYO_OP_STOR_ALT,
     EMBRYO_OP_STOR_S_PRI,
     EMBRYO_OP_STOR_S_ALT,
     EMBRYO_OP_SREF_PRI,
     EMBRYO_OP_SREF_ALT,
     EMBRYO_OP_SREF_S_PRI,
     EMBRYO_OP_SREF_S_ALT,
     EMBRYO_OP_STOR_I,
     EMBRYO_OP_STRB_I,
     EMBRYO_OP_LIDX,
     EMBRYO_OP_LIDX_B,
     EMBRYO_OP_IDXADDR,
     EMBRYO_OP_IDXADDR_B,
     EMBRYO_OP_ALIGN_PRI,
     EMBRYO_OP_ALIGN_ALT,
     EMBRYO_OP_LCTRL,
     EMBRYO_OP_SCTRL,
     EMBRYO_OP_MOVE_PRI,
     EMBRYO_OP_MOVE_ALT,
     EMBRYO_OP_XCHG,
     EMBRYO_OP_PUSH_PRI,
     EMBRYO_OP_PUSH_ALT,
     EMBRYO_OP_PUSH_R,
     EMBRYO_OP_PUSH_C,
     EMBRYO_OP_PUSH,
     EMBRYO_OP_PUSH_S,
     EMBRYO_OP_POP_PRI,
     EMBRYO_OP_POP_ALT,
     EMBRYO_OP_STACK,
     EMBRYO_OP_HEAP,
     EMBRYO_OP_PROC,
     EMBRYO_OP_RET,
     EMBRYO_OP_RETN,
     EMBRYO_OP_CALL,
     EMBRYO_OP_CALL_PRI,
     EMBRYO_OP_JUMP,
     EMBRYO_OP_JREL,
     EMBRYO_OP_JZER,
     EMBRYO_OP_JNZ,
     EMBRYO_OP_JEQ,
     EMBRYO_OP_JNEQ,
     EMBRYO_OP_JLESS,
     EMBRYO_OP_JLEQ,
     EMBRYO_OP_JGRTR,
     EMBRYO_OP_JGEQ,
     EMBRYO_OP_JSLESS,
     EMBRYO_OP_JSLEQ,
     EMBRYO_OP_JSGRTR,
     EMBRYO_OP_JSGEQ,
     EMBRYO_OP_SHL,
     EMBRYO_OP_SHR,
     EMBRYO_OP_SSHR,
     EMBRYO_OP_SHL_C_PRI,
     EMBRYO_OP_SHL_C_ALT,
     EMBRYO_OP_SHR_C_PRI,
     EMBRYO_OP_SHR_C_ALT,
     EMBRYO_OP_SMUL,
     EMBRYO_OP_SDIV,
     EMBRYO_OP_SDIV_ALT,
     EMBRYO_OP_UMUL,
     EMBRYO_OP_UDIV,
     EMBRYO_OP_UDIV_ALT,
     EMBRYO_OP_ADD,
     EMBRYO_OP_SUB,
     EMBRYO_OP_SUB_ALT,
     EMBRYO_OP_AND,
     EMBRYO_OP_OR,
     EMBRYO_OP_XOR,
     EMBRYO_OP_NOT,
     EMBRYO_OP_NEG,
     EMBRYO_OP_INVERT,
     EMBRYO_OP_ADD_C,
     EMBRYO_OP_SMUL_C,
     EMBRYO_OP_ZERO_PRI,
     EMBRYO_OP_ZERO_ALT,
     EMBRYO_OP_ZERO,
     EMBRYO_OP_ZERO_S,
     EMBRYO_OP_SIGN_PRI,
     EMBRYO_OP_SIGN_ALT,
     EMBRYO_OP_EQ,
     EMBRYO_OP_NEQ,
     EMBRYO_OP_LESS,
     EMBRYO_OP_LEQ,
     EMBRYO_OP_GRTR,
     EMBRYO_OP_GEQ,
     EMBRYO_OP_SLESS,
     EMBRYO_OP_SLEQ,
     EMBRYO_OP_SGRTR,
     EMBRYO_OP_SGEQ,
     EMBRYO_OP_EQ_C_PRI,
     EMBRYO_OP_EQ_C_ALT,
     EMBRYO_OP_INC_PRI,
     EMBRYO_OP_INC_ALT,
     EMBRYO_OP_INC,
     EMBRYO_OP_INC_S,
     EMBRYO_OP_INC_I,
     EMBRYO_OP_DEC_PRI,
     EMBRYO_OP_DEC_ALT,
     EMBRYO_OP_DEC,
     EMBRYO_OP_DEC_S,
     EMBRYO_OP_DEC_I,
     EMBRYO_OP_MOVS,
     EMBRYO_OP_CMPS,
     EMBRYO_OP_FILL,
     EMBRYO_OP_HALT,
     EMBRYO_OP_BOUNDS,
     EMBRYO_OP_SYSREQ_PRI,
     EMBRYO_OP_SYSREQ_C,
     EMBRYO_OP_FILE,
     EMBRYO_OP_LINE,
     EMBRYO_OP_SYMBOL,
     EMBRYO_OP_SRANGE,
     EMBRYO_OP_JUMP_PRI,
     EMBRYO_OP_SWITCH,
     EMBRYO_OP_CASETBL,
     EMBRYO_OP_SWAP_PRI,
     EMBRYO_OP_SWAP_ALT,
     EMBRYO_OP_PUSHADDR,
     EMBRYO_OP_NOP,
     EMBRYO_OP_SYSREQ_D,
     EMBRYO_OP_SYMTAG,
     /* ----- */
     EMBRYO_OP_NUM_OPCODES
};

#define NUMENTRIES(hdr, field, nextfield) \
(int)(((hdr)->nextfield - (hdr)->field) / (hdr)->defsize)
#define GETENTRY(hdr, table, index) \
(Embryo_Func_Stub *)((unsigned char*)(hdr) + \
(int)(hdr)->table + index * (hdr)->defsize)
#ifdef WORDS_BIGENDIAN
static int __inline __entryswap32(int v)
{int vv; vv = v; embryo_swap_32((unsigned int *)&vv); return vv;}
# define GETENTRYNAME(hdr, entry) \
(((hdr)->defsize == 2 * sizeof(unsigned int)) \
? (char *)((unsigned char*)(hdr) + \
__entryswap32(*((unsigned int *)(entry) + 1))) \
: (entry)->name)
#else
# define GETENTRYNAME(hdr, entry) \
(((hdr)->defsize == 2 * sizeof(unsigned int)) \
? (char *)((unsigned char*)(hdr) + *((unsigned int *)(entry) + 1)) \
: (entry)->name)
#endif

#define CUR_FILE_VERSION    7      /* current file version; also the current Embryo_Program version */
#define MIN_FILE_VERSION    7      /* lowest supported file format version for the current Embryo_Program version */
#define MIN_AMX_VERSION     7      /* minimum Embryo_Program version needed to support the current file format */
#define sEXPMAX             19     /* maximum name length for file version <= 6 */
#define sNAMEMAX            31     /* maximum name length of symbol name */
#define EMBRYO_MAGIC        0xf1e0 /* magic byte pattern */
#define EMBRYO_FLAG_COMPACT 0x04   /* compact encoding */
#define EMBRYO_FLAG_RELOC   0x8000 /* jump/call addresses relocated */
#define GETPARAM(v)         (v = *(Embryo_Cell *)cip++)
#define PUSH(v)             (stk -= sizeof(Embryo_Cell), *(Embryo_Cell *)(data + (int)stk) = v)
#define POP(v)              (v = *(Embryo_Cell *)(data + (int)stk), stk += sizeof(Embryo_Cell))
#define ABORT(ep,v)         {(ep)->stk = reset_stk; (ep)->hea = reset_hea; (ep)->run_count--; ep->error = v; (ep)->max_run_cycles = max_run_cycles; return EMBRYO_PROGRAM_FAIL;}
#define OK(ep,v)            {(ep)->stk = reset_stk; (ep)->hea = reset_hea; (ep)->run_count--; ep->error = v; (ep)->max_run_cycles = max_run_cycles; return EMBRYO_PROGRAM_OK;}
#define TOOLONG(ep)         {(ep)->pri = pri; (ep)->cip = (Embryo_Cell)((unsigned char *)cip - code); (ep)->alt = alt; (ep)->frm = frm; (ep)->stk = stk; (ep)->hea = hea; (ep)->reset_stk = reset_stk; (ep)->reset_hea = reset_hea; (ep)->run_count--; (ep)->max_run_cycles = max_run_cycles; return EMBRYO_PROGRAM_TOOLONG;}
#define STKMARGIN           ((Embryo_Cell)(16 * sizeof(Embryo_Cell)))
#define CHKMARGIN()         if ((hea + STKMARGIN) > stk) {ep->error = EMBRYO_ERROR_STACKERR; return 0;}
#define CHKSTACK()          if (stk > ep->stp) {ep->run_count--; ep->error = EMBRYO_ERROR_STACKLOW; return 0;}
#define CHKHEAP()           if (hea < ep->hlw) {ep->run_count--; ep->error = EMBRYO_ERROR_HEAPLOW; return 0;}
#define CHKMEM(x)           if ((((x) >= hea) && ((x) < stk)) || ((Embryo_UCell)(x) >= (Embryo_UCell)ep->stp)) ABORT(ep, EMBRYO_ERROR_MEMACCESS);

typedef struct _Embryo_Param        Embryo_Param;
typedef struct _Embryo_Header       Embryo_Header;
typedef struct _Embryo_Func_Stub    Embryo_Func_Stub;

typedef Embryo_Cell (*Embryo_Native)(Embryo_Program *ep, Embryo_Cell *params);

struct _Embryo_Param
{
   char        *string;
   Embryo_Cell *cell_array;
   int          cell_array_size;
   Embryo_Cell  cell;
};

struct _Embryo_Program
{
   unsigned char *base; /* points to the Embryo_Program header ("ephdr") plus the code, optionally also the data */
   int pushes; /* number of pushes - pops */
   /* for external functions a few registers must be accessible from the outside */
   Embryo_Cell cip; /* instruction pointer: relative to base + ephdr->cod */
   Embryo_Cell frm; /* stack frame base: relative to base + ephdr->dat */
   Embryo_Cell hea; /* top of the heap: relative to base + ephdr->dat */
   Embryo_Cell hlw; /* bottom of the heap: relative to base + ephdr->dat */
   Embryo_Cell stk; /* stack pointer: relative to base + ephdr->dat */
   Embryo_Cell stp; /* top of the stack: relative to base + ephdr->dat */
   int flags; /* current status  */
   /* native functions can raise an error */
   int error;
   /* the sleep opcode needs to store the full Embryo_Program status */
   Embryo_Cell pri;
   Embryo_Cell alt;
   Embryo_Cell reset_stk;
   Embryo_Cell reset_hea;
   Embryo_Cell *syscall_d; /* relocated value/address for the SYSCALL.D opcode */

   /* extended stuff */
   Embryo_Native *native_calls;
   int            native_calls_size;
   int            native_calls_alloc;

   unsigned char *code;
   unsigned char  dont_free_code : 1;
   Embryo_Cell    retval;

   Embryo_Param  *params;
   int            params_size;
   int            params_alloc;

   int            run_count;

   int            max_run_cycles;

   void          *data;
};

struct _Embryo_Func_Stub
{
   int  address;
   char name[sEXPMAX+1];
} __attribute__((packed));

struct _Embryo_Header
{
   unsigned int size; /* size of the "file" */
   unsigned short magic; /* signature */
   char file_version; /* file format version */
   char ep_version; /* required version of the Embryo_Program */
   short flags;
   short defsize; /* size of a definition record */
   int cod; /* initial value of COD - code block */
   int dat; /* initial value of DAT - data block */
   int hea; /* initial value of HEA - start of the heap */
   int stp; /* initial value of STP - stack top */
   int cip; /* initial value of CIP - the instruction pointer */
   int publics; /* offset to the "public functions" table */
   int natives; /* offset to the "native functions" table */
   int libraries; /* offset to the table of libraries */
   int pubvars; /* the "public variables" table */
   int tags; /* the "public tagnames" table */
   int nametable; /* name table, file version 7 only */
} __attribute__((packed));

void _embryo_args_init(Embryo_Program *ep);
void _embryo_fp_init(Embryo_Program *ep);
void _embryo_rand_init(Embryo_Program *ep);
void _embryo_str_init(Embryo_Program *ep);
void _embryo_time_init(Embryo_Program *ep);

#endif
