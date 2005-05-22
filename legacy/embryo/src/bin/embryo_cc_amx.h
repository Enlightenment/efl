/*  Abstract Machine for the Small compiler
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
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
 *
 *  Version: $Id$
 */

#include "embryo_cc_osdefs.h"

#ifndef EMBRYO_CC_AMX_H
#define EMBRYO_CC_AMX_H

#include <sys/types.h>

/* calling convention for all interface functions and callback functions */

/* File format version                          Required AMX version
 *   0 (original version)                       0
 *   1 (opcodes JUMP.pri, SWITCH and CASETBL)   1
 *   2 (compressed files)                       2
 *   3 (public variables)                       2
 *   4 (opcodes SWAP.pri/alt and PUSHADDR)      4
 *   5 (tagnames table)                         4
 *   6 (reformatted header)                     6
 *   7 (name table, opcodes SYMTAG & SYSREQ.D)  7
 */
#define CUR_FILE_VERSION  7	/* current file version; also the current AMX version */
#define MIN_FILE_VERSION  6	/* lowest supported file format version for the current AMX version */
#define MIN_AMX_VERSION   7	/* minimum AMX version needed to support the current file format */

#if !defined CELL_TYPE
#define CELL_TYPE
   typedef unsigned int    ucell;
   typedef int     cell;
#endif

   struct tagAMX;
   typedef             cell(*AMX_NATIVE) (struct tagAMX * amx,
							   cell * params);
   typedef int         (* AMX_CALLBACK) (struct tagAMX * amx, cell index,
						cell * result, cell * params);
   typedef int         (* AMX_DEBUG) (struct tagAMX * amx);

   typedef struct
   {
      char          *name;
      AMX_NATIVE func    ;
   } AMX_NATIVE_INFO  ;

#define AMX_USERNUM     4
#define sEXPMAX         19	/* maximum name length for file version <= 6 */
#define sNAMEMAX        31	/* maximum name length of symbol name */

   typedef struct tagAMX_FUNCSTUB
   {
      unsigned int        address;
      char                name[sEXPMAX + 1];
   } __attribute__((packed)) AMX_FUNCSTUB;

/* The AMX structure is the internal structure for many functions. Not all
 * fields are valid at all times; many fields are cached in local variables.
 */
   typedef struct tagAMX
   {
      unsigned char *base;	/* points to the AMX header ("amxhdr") plus the code, optionally also the data */
      unsigned char *data;	/* points to separate data+stack+heap, may be NULL */
      AMX_CALLBACK callback;
      AMX_DEBUG debug    ;	/* debug callback */
      /* for external functions a few registers must be accessible from the outside */
      cell cip           ;	/* instruction pointer: relative to base + amxhdr->cod */
      cell frm           ;	/* stack frame base: relative to base + amxhdr->dat */
      cell hea           ;	/* top of the heap: relative to base + amxhdr->dat */
      cell hlw           ;	/* bottom of the heap: relative to base + amxhdr->dat */
      cell stk           ;	/* stack pointer: relative to base + amxhdr->dat */
      cell stp           ;	/* top of the stack: relative to base + amxhdr->dat */
      int flags          ;	/* current status, see amx_Flags() */
      /* for assertions and debug hook */
      cell curline       ;
      cell curfile       ;
      int dbgcode        ;
      cell dbgaddr       ;
      cell dbgparam      ;
      char          *dbgname;
      /* user data */
      long                usertags[AMX_USERNUM];
      void          *userdata[AMX_USERNUM];
      /* native functions can raise an error */
      int error          ;
      /* the sleep opcode needs to store the full AMX status */
      cell pri           ;
      cell alt           ;
      cell reset_stk     ;
      cell reset_hea     ;
      cell          *syscall_d;	/* relocated value/address for the SYSCALL.D opcode */
   } __attribute__((packed)) AMX;

/* The AMX_HEADER structure is both the memory format as the file format. The
 * structure is used internaly.
 */
   typedef struct tagAMX_HEADER
   {
      int size       ;	/* size of the "file" */
      unsigned short magic     ;	/* signature */
      char file_version  ;	/* file format version */
      char amx_version   ;	/* required version of the AMX */
      unsigned short flags      ;
      unsigned short defsize    ;	/* size of a definition record */
      int cod        ;	/* initial value of COD - code block */
      int dat        ;	/* initial value of DAT - data block */
      int hea        ;	/* initial value of HEA - start of the heap */
      int stp        ;	/* initial value of STP - stack top */
      int cip        ;	/* initial value of CIP - the instruction pointer */
      int publics    ;	/* offset to the "public functions" table */
      int natives    ;	/* offset to the "native functions" table */
      int libraries  ;	/* offset to the table of libraries */
      int pubvars    ;	/* the "public variables" table */
      int tags       ;	/* the "public tagnames" table */
      int nametable  ;	/* name table, file version 7 only */
   } __attribute__((packed)) AMX_HEADER;
#define AMX_MAGIC       0xf1e0

   enum
   {
      AMX_ERR_NONE,
      /* reserve the first 15 error codes for exit codes of the abstract machine */
      AMX_ERR_EXIT,		/* forced exit */
      AMX_ERR_ASSERT,		/* assertion failed */
      AMX_ERR_STACKERR,		/* stack/heap collision */
      AMX_ERR_BOUNDS,		/* index out of bounds */
      AMX_ERR_MEMACCESS,	/* invalid memory access */
      AMX_ERR_INVINSTR,		/* invalid instruction */
      AMX_ERR_STACKLOW,		/* stack underflow */
      AMX_ERR_HEAPLOW,		/* heap underflow */
      AMX_ERR_CALLBACK,		/* no callback, or invalid callback */
      AMX_ERR_NATIVE,		/* native function failed */
      AMX_ERR_DIVIDE,		/* divide by zero */
      AMX_ERR_SLEEP,		/* go into sleepmode - code can be restarted */

      AMX_ERR_MEMORY = 16,	/* out of memory */
      AMX_ERR_FORMAT,		/* invalid file format */
      AMX_ERR_VERSION,		/* file is for a newer version of the AMX */
      AMX_ERR_NOTFOUND,		/* function not found */
      AMX_ERR_INDEX,		/* invalid index parameter (bad entry point) */
      AMX_ERR_DEBUG,		/* debugger cannot run */
      AMX_ERR_INIT,		/* AMX not initialized (or doubly initialized) */
      AMX_ERR_USERDATA,		/* unable to set user data field (table full) */
      AMX_ERR_INIT_JIT,		/* cannot initialize the JIT */
      AMX_ERR_PARAMS,		/* parameter error */
      AMX_ERR_DOMAIN,		/* domain error, expression result does not fit in range */
   };

   enum
   {
      DBG_INIT,			/* query/initialize */
      DBG_FILE,			/* file number in curfile, filename in name */
      DBG_LINE,			/* line number in curline, file number in curfile */
      DBG_SYMBOL,		/* address in dbgaddr, class/type in dbgparam */
      DBG_CLRSYM,		/* stack address below which locals should be removed. stack address in stk */
      DBG_CALL,			/* function call, address jumped to in dbgaddr */
      DBG_RETURN,		/* function returns */
      DBG_TERMINATE,		/* program ends, code address in dbgaddr, reason in dbgparam */
      DBG_SRANGE,		/* symbol size and dimensions (arrays); level in dbgaddr (!); length in dbgparam */
      DBG_SYMTAG,		/* tag of the most recent symbol (if non-zero), tag in dbgparam */
   };

#define AMX_FLAG_CHAR16   0x01	/* characters are 16-bit */
#define AMX_FLAG_DEBUG    0x02	/* symbolic info. available */
#define AMX_FLAG_COMPACT  0x04	/* compact encoding */
#define AMX_FLAG_BIGENDIAN 0x08	/* big endian encoding */
#define AMX_FLAG_NOCHECKS  0x10	/* no array bounds checking */
#define AMX_FLAG_BROWSE 0x4000	/* browsing/relocating or executing */
#define AMX_FLAG_RELOC  0x8000	/* jump/call addresses relocated */

#define AMX_EXEC_MAIN   -1	/* start at program entry point */
#define AMX_EXEC_CONT   -2	/* continue from last address */

#define AMX_USERTAG(a,b,c,d)    ((a) | ((b)<<8) | ((long)(c)<<16) | ((long)(d)<<24))

#define AMX_EXPANDMARGIN  64

/* for native functions that use floating point parameters, the following
 * two macros are convenient for casting a "cell" into a "float" type _without_
 * changing the bit pattern
 */
#define amx_ftoc(f)     ( * ((cell*)&f) )	/* float to cell */
#define amx_ctof(c)     ( * ((float*)&c) )	/* cell to float */

#define amx_StrParam(amx,param,result) {                             \
            cell *amx_cstr_; int amx_length_;                        \
            amx_GetAddr((amx), (param), &amx_cstr_);                 \
            amx_StrLen(amx_cstr_, &amx_length_);                     \
            if (amx_length_ > 0 &&                                   \
                ((result) = (char*)alloca(amx_length_ + 1)) != NULL) \
              amx_GetString((result), amx_cstr_);                    \
            else (result) = NULL;                                    \
}

#endif				/* __AMX_H */
