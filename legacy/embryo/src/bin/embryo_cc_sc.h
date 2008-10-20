/*  Small compiler
 *
 *  Drafted after the Small-C compiler Version 2.01, originally created
 *  by Ron Cain, july 1980, and enhanced by James E. Hendrix.
 *
 *  This version comes close to a complete rewrite.
 *
 *  Copyright R. Cain, 1980
 *  Copyright J.E. Hendrix, 1982, 1983
 *  Copyright T. Riemersma, 1997-2003
 *
 *  Version: $Id$
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

#ifndef EMBRYO_CC_SC_H
#define EMBRYO_CC_SC_H

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <setjmp.h>

#include <eina_types.h>

#include "embryo_cc_osdefs.h"
#include "embryo_cc_amx.h"

/* Note: the "cell" and "ucell" types are defined in AMX.H */

#define PUBLIC_CHAR '@'		/* character that defines a function "public" */
#define CTRL_CHAR   '\\'	/* default control character */

#define DIRSEP_CHAR '/'		/* directory separator character */

#define sDIMEN_MAX     2	/* maximum number of array dimensions */
#define sDEF_LITMAX  500	/* initial size of the literal pool, in "cells" */
#define sLINEMAX     65535	/* input line length (in characters) */
#define sDEF_AMXSTACK 4096	/* default stack size for AMX files */
#define sSTKMAX       80	/* stack for nested #includes and other uses */
#define PREPROC_TERM  '\x7f'	/* termination character for preprocessor expressions (the "DEL" code) */
#define sDEF_PREFIX   "default.inc"	/* default prefix filename */

typedef void       *stkitem;	/* type of items stored on the stack */

typedef struct __s_arginfo
{				/* function argument info */
   char                name[sNAMEMAX + 1];
   char                ident;	/* iVARIABLE, iREFERENCE, iREFARRAY or iVARARGS */
   char                usage;	/* uCONST */
   int                *tags;	/* argument tag id. list */
   int                 numtags;	/* number of tags in the tag list */
   int                 dim[sDIMEN_MAX];
   int                 numdim;	/* number of dimensions */
   unsigned char       hasdefault;	/* bit0: is there a default value? bit6: "tagof"; bit7: "sizeof" */
   union
   {
      cell                val;	/* default value */
      struct
      {
	 char               *symname;	/* name of another symbol */
	 short               level;	/* indirection level for that symbol */
      } size;			/* used for "sizeof" default value */
      struct
      {
	 cell               *data;	/* values of default array */
	 int                 size;	/* complete length of default array */
	 int                 arraysize;	/* size to reserve on the heap */
	 cell                addr;	/* address of the default array in the data segment */
      } array;
   } defvalue;			/* default value, or pointer to default array */
   int                 defvalue_tag;	/* tag of the default value */
} arginfo;

/*  Equate table, tagname table, library table */
typedef struct __s_constvalue
{
   struct __s_constvalue *next;
   char                name[sNAMEMAX + 1];
   cell                value;
   short               index;
} constvalue;

/*  Symbol table format
 *
 *  The symbol name read from the input file is stored in "name", the
 *  value of "addr" is written to the output file. The address in "addr"
 *  depends on the class of the symbol:
 *      global          offset into the data segment
 *      local           offset relative to the stack frame
 *      label           generated hexadecimal number
 *      function        offset into code segment
 */
typedef struct __s_symbol
{
   struct __s_symbol  *next;
   struct __s_symbol  *parent;	/* hierarchical types (multi-dimensional arrays) */
   char                name[sNAMEMAX + 1];
   unsigned int        hash;	/* value derived from name, for quicker searching */
   cell                addr;	/* address or offset (or value for constant, index for native function) */
   char                vclass;	/* sLOCAL if "addr" refers to a local symbol */
   char                ident;	/* see below for possible values */
   char                usage;	/* see below for possible values */
   int                 compound;	/* compound level (braces nesting level) */
   int                 tag;	/* tagname id */
   union
   {
      int                 declared;	/* label: how many local variables are declared */
      int                 idxtag;	/* array: tag of array indices */
      constvalue         *lib;	/* native function: library it is part of *///??? use "stringlist"
   } x;				/* 'x' for 'extra' */
   union
   {
      arginfo            *arglist;	/* types of all parameters for functions */
      struct
      {
	 cell                length;	/* arrays: length (size) */
	 short               level;	/* number of dimensions below this level */
      } array;
   } dim;			/* for 'dimension', both functions and arrays */
   int                 fnumber;	/* static global variables: file number in which the declaration is visible */
   struct __s_symbol **refer;	/* referrer list, functions that "use" this symbol */
   int                 numrefers;	/* number of entries in the referrer list */
} symbol;

/*  Possible entries for "ident". These are used in the "symbol", "value"
 *  and arginfo structures. Not every constant is valid for every use.
 *  In an argument list, the list is terminated with a "zero" ident; labels
 *  cannot be passed as function arguments, so the value 0 is overloaded.
 */
#define iLABEL      0
#define iVARIABLE   1		/* cell that has an address and that can be fetched directly (lvalue) */
#define iREFERENCE  2		/* iVARIABLE, but must be dereferenced */
#define iARRAY      3
#define iREFARRAY   4		/* an array passed by reference (i.e. a pointer) */
#define iARRAYCELL  5		/* array element, cell that must be fetched indirectly */
#define iARRAYCHAR  6		/* array element, character from cell from array */
#define iEXPRESSION 7		/* expression result, has no address (rvalue) */
#define iCONSTEXPR  8		/* constant expression (or constant symbol) */
#define iFUNCTN     9
#define iREFFUNC    10		/* function passed as a parameter */
#define iVARARGS    11		/* function specified ... as argument(s) */

/*  Possible entries for "usage"
 *
 *  This byte is used as a serie of bits, the syntax is different for
 *  functions and other symbols:
 *
 *  VARIABLE
 *  bits: 0     (uDEFINE) the variable is defined in the source file
 *        1     (uREAD) the variable is "read" (accessed) in the source file
 *        2     (uWRITTEN) the variable is altered (assigned a value)
 *        3     (uCONST) the variable is constant (may not be assigned to)
 *        4     (uPUBLIC) the variable is public
 *        6     (uSTOCK) the variable is discardable (without warning)
 *
 *  FUNCTION
 *  bits: 0     (uDEFINE) the function is defined ("implemented") in the source file
 *        1     (uREAD) the function is invoked in the source file
 *        2     (uRETVALUE) the function returns a value (or should return a value)
 *        3     (uPROTOTYPED) the function was prototyped
 *        4     (uPUBLIC) the function is public
 *        5     (uNATIVE) the function is native
 *        6     (uSTOCK) the function is discardable (without warning)
 *        7     (uMISSING) the function is not implemented in this source file
 *
 *  CONSTANT
 *  bits: 0     (uDEFINE) the symbol is defined in the source file
 *        1     (uREAD) the constant is "read" (accessed) in the source file
 *        3     (uPREDEF) the constant is pre-defined and should be kept between passes
 */
#define uDEFINE   0x01
#define uREAD     0x02
#define uWRITTEN  0x04
#define uRETVALUE 0x04		/* function returns (or should return) a value */
#define uCONST    0x08
#define uPROTOTYPED 0x08
#define uPREDEF   0x08		/* constant is pre-defined */
#define uPUBLIC   0x10
#define uNATIVE   0x20
#define uSTOCK    0x40
#define uMISSING  0x80
/* uRETNONE is not stored in the "usage" field of a symbol. It is
 * used during parsing a function, to detect a mix of "return;" and
 * "return value;" in a few special cases.
 */
#define uRETNONE  0x10

#define uTAGOF    0x40		/* set in the "hasdefault" field of the arginfo struct */
#define uSIZEOF   0x80		/* set in the "hasdefault" field of the arginfo struct */

#define uMAINFUNC "main"

#define sGLOBAL   0		/* global/local variable/constant class */
#define sLOCAL    1
#define sSTATIC   2		/* global life, local scope */

typedef struct
{
   symbol             *sym;	/* symbol in symbol table, NULL for (constant) expression */
   cell                constval;	/* value of the constant expression (if ident==iCONSTEXPR)
					 * also used for the size of a literal array */
   int                 tag;	/* tagname id (of the expression) */
   char                ident;	/* iCONSTEXPR, iVARIABLE, iARRAY, iARRAYCELL,
				 * iEXPRESSION or iREFERENCE */
   char                boolresult;	/* boolean result for relational operators */
   cell               *arrayidx;	/* last used array indices, for checking self assignment */
} value;

/*  "while" statement queue (also used for "for" and "do - while" loops) */
enum
{
   wqBRK,			/* used to restore stack for "break" */
   wqCONT,			/* used to restore stack for "continue" */
   wqLOOP,			/* loop start label number */
   wqEXIT,			/* loop exit label number (jump if false) */
   /* --- */
   wqSIZE			/* "while queue" size */
};

#define wqTABSZ (24*wqSIZE)	/* 24 nested loop statements */

enum
{
   statIDLE,			/* not compiling yet */
   statFIRST,			/* first pass */
   statWRITE,			/* writing output */
   statSKIP,			/* skipping output */
};

typedef struct __s_stringlist
{
   struct __s_stringlist *next;
   char               *line;
} stringlist;

typedef struct __s_stringpair
{
   struct __s_stringpair *next;
   char               *first;
   char               *second;
   int                 matchlength;
} stringpair;

/* macros for code generation */
#define opcodes(n)      ((n)*sizeof(cell))	/* opcode size */
#define opargs(n)       ((n)*sizeof(cell))	/* size of typical argument */

/*  Tokens recognized by lex()
 *  Some of these constants are assigned as well to the variable "lastst"
 */
#define tFIRST   256		/* value of first multi-character operator */
#define tMIDDLE  279		/* value of last multi-character operator */
#define tLAST    320		/* value of last multi-character match-able token */
/* multi-character operators */
#define taMULT   256		/* *= */
#define taDIV    257		/* /= */
#define taMOD    258		/* %= */
#define taADD    259		/* += */
#define taSUB    260		/* -= */
#define taSHL    261		/* <<= */
#define taSHRU   262		/* >>>= */
#define taSHR    263		/* >>= */
#define taAND    264		/* &= */
#define taXOR    265		/* ^= */
#define taOR     266		/* |= */
#define tlOR     267		/* || */
#define tlAND    268		/* && */
#define tlEQ     269		/* == */
#define tlNE     270		/* != */
#define tlLE     271		/* <= */
#define tlGE     272		/* >= */
#define tSHL     273		/* << */
#define tSHRU    274		/* >>> */
#define tSHR     275		/* >> */
#define tINC     276		/* ++ */
#define tDEC     277		/* -- */
#define tELLIPS  278		/* ... */
#define tDBLDOT  279		/* .. */
/* reserved words (statements) */
#define tASSERT  280
#define tBREAK   281
#define tCASE    282
#define tCHAR    283
#define tCONST   284
#define tCONTINUE 285
#define tDEFAULT 286
#define tDEFINED 287
#define tDO      288
#define tELSE    289
#define tENUM    290
#define tEXIT    291
#define tFOR     292
#define tFORWARD 293
#define tGOTO    294
#define tIF      295
#define tNATIVE  296
#define tNEW     297
#define tOPERATOR 298
#define tPUBLIC  299
#define tRETURN  300
#define tSIZEOF  301
#define tSLEEP   302
#define tSTATIC  303
#define tSTOCK   304
#define tSWITCH  305
#define tTAGOF   306
#define tWHILE   307
/* compiler directives */
#define tpASSERT 308		/* #assert */
#define tpDEFINE 309
#define tpELSE   310		/* #else */
#define tpEMIT   311
#define tpENDIF  312
#define tpENDINPUT 313
#define tpENDSCRPT 314
#define tpFILE   315
#define tpIF     316		/* #if */
#define tINCLUDE 317
#define tpLINE   318
#define tpPRAGMA 319
#define tpUNDEF  320
/* semicolon is a special case, because it can be optional */
#define tTERM    321		/* semicolon or newline */
#define tENDEXPR 322		/* forced end of expression */
/* other recognized tokens */
#define tNUMBER  323		/* integer number */
#define tRATIONAL 324		/* rational number */
#define tSYMBOL  325
#define tLABEL   326
#define tSTRING  327
#define tEXPR    328		/* for assigment to "lastst" only */

/* (reversed) evaluation of staging buffer */
#define sSTARTREORDER 1
#define sENDREORDER   2
#define sEXPRSTART    0xc0	/* top 2 bits set, rest is free */
#define sMAXARGS      64	/* relates to the bit pattern of sEXPRSTART */

/* codes for ffabort() */
#define xEXIT           1	/* exit code in PRI */
#define xASSERTION      2	/* abort caused by failing assertion */
#define xSTACKERROR     3	/* stack/heap overflow */
#define xBOUNDSERROR    4	/* array index out of bounds */
#define xMEMACCESS      5	/* data access error */
#define xINVINSTR       6	/* invalid instruction */
#define xSTACKUNDERFLOW 7	/* stack underflow */
#define xHEAPUNDERFLOW  8	/* heap underflow */
#define xCALLBACKERR    9	/* no, or invalid, callback */
#define xSLEEP         12	/* sleep, exit code in PRI, tag in ALT */

/* Miscellaneous  */
#if !defined TRUE
#define FALSE         0
#define TRUE          1
#endif
#define sIN_CSEG        1	/* if parsing CODE */
#define sIN_DSEG        2	/* if parsing DATA */
#define sCHKBOUNDS      1	/* bit position in "debug" variable: check bounds */
#define sSYMBOLIC       2	/* bit position in "debug" variable: symbolic info */
#define sNOOPTIMIZE     4	/* bit position in "debug" variable: no optimization */
#define sRESET          0	/* reset error flag */
#define sFORCESET       1	/* force error flag on */
#define sEXPRMARK       2	/* mark start of expression */
#define sEXPRRELEASE    3	/* mark end of expression */

#if INT_MAX<0x8000u
#define PUBLICTAG   0x8000u
#define FIXEDTAG    0x4000u
#else
#define PUBLICTAG   0x80000000Lu
#define FIXEDTAG    0x40000000Lu
#endif
#define TAGMASK       (~PUBLICTAG)


/*
 * Functions you call from the "driver" program
 */
   int                 sc_compile(int argc, char **argv);
   int                 sc_addconstant(char *name, cell value, int tag);
   int                 sc_addtag(char *name);

/*
 * Functions called from the compiler (to be implemented by you)
 */

/* general console output */
   int                 sc_printf(const char *message, ...);

/* error report function */
   int                 sc_error(int number, char *message, char *filename,
				int firstline, int lastline, va_list argptr);

/* input from source file */
   void               *sc_opensrc(char *filename);	/* reading only */
   void                sc_closesrc(void *handle);	/* never delete */
   void                sc_resetsrc(void *handle, void *position);	/* reset to a position marked earlier */
   char               *sc_readsrc(void *handle, char *target, int maxchars);
   void               *sc_getpossrc(void *handle);	/* mark the current position */
   int                 sc_eofsrc(void *handle);

/* output to intermediate (.ASM) file */
   void               *sc_openasm(int fd);	/* read/write */
   void                sc_closeasm(void *handle);
   void                sc_resetasm(void *handle);
   int                 sc_writeasm(void *handle, char *str);
   char               *sc_readasm(void *handle, char *target, int maxchars);

/* output to binary (.AMX) file */
   void               *sc_openbin(char *filename);
   void                sc_closebin(void *handle, int deletefile);
   void                sc_resetbin(void *handle);
   int                 sc_writebin(void *handle, void *buffer, int size);
   long                sc_lengthbin(void *handle);	/* return the length of the file */

/* function prototypes in SC1.C */
symbol     *fetchfunc(char *name, int tag);
char       *operator_symname(char *symname, char *opername, int tag1,
				     int tag2, int numtags, int resulttag);
char       *funcdisplayname(char *dest, char *funcname);
int         constexpr(cell * val, int *tag);
constvalue *append_constval(constvalue * table, char *name, cell val,
				    short index);
constvalue *find_constval(constvalue * table, char *name, short index);
void        delete_consttable(constvalue * table);
void        add_constant(char *name, cell val, int vclass, int tag);
void        exporttag(int tag);

/* function prototypes in SC2.C */
void        pushstk(stkitem val);
stkitem     popstk(void);
int         plungequalifiedfile(char *name);	/* explicit path included */
int         plungefile(char *name, int try_currentpath, int try_includepaths);	/* search through "include" paths */
void        preprocess(void);
void        lexinit(void);
int         lex(cell * lexvalue, char **lexsym);
void        lexpush(void);
void        lexclr(int clreol);
int         matchtoken(int token);
int         tokeninfo(cell * val, char **str);
int         needtoken(int token);
void        stowlit(cell value);
int         alphanum(char c);
void        delete_symbol(symbol * root, symbol * sym);
void        delete_symbols(symbol * root, int level, int del_labels,
				   int delete_functions);
int         refer_symbol(symbol * entry, symbol * bywhom);
void        markusage(symbol * sym, int usage);
unsigned int namehash(char *name);
symbol     *findglb(char *name);
symbol     *findloc(char *name);
symbol     *findconst(char *name);
symbol     *finddepend(symbol * parent);
symbol     *addsym(char *name, cell addr, int ident, int vclass,
			   int tag, int usage);
symbol     *addvariable(char *name, cell addr, int ident, int vclass,
				int tag, int dim[], int numdim, int idxtag[]);
int         getlabel(void);
char       *itoh(ucell val);

/* function prototypes in SC3.C */
int         check_userop(void (*oper) (void), int tag1, int tag2,
				 int numparam, value * lval, int *resulttag);
int         matchtag(int formaltag, int actualtag, int allowcoerce);
int         expression(int *constant, cell * val, int *tag,
			       int chkfuncresult);
int         hier14(value * lval1);	/* the highest expression level */

/* function prototypes in SC4.C */
void        writeleader(void);
void        writetrailer(void);
void        begcseg(void);
void        begdseg(void);
void        setactivefile(int fnumber);
cell        nameincells(char *name);
void        setfile(char *name, int fileno);
void        setline(int line, int fileno);
void        setlabel(int index);
void        endexpr(int fullexpr);
void        startfunc(char *fname);
void        endfunc(void);
void        alignframe(int numbytes);
void        defsymbol(char *name, int ident, int vclass, cell offset,
			      int tag);
void        symbolrange(int level, cell size);
void        rvalue(value * lval);
void        address(symbol * ptr);
void        store(value * lval);
void        memcopy(cell size);
void        copyarray(symbol * sym, cell size);
void        fillarray(symbol * sym, cell size, cell value);
void        const1(cell val);
void        const2(cell val);
void        moveto1(void);
void        push1(void);
void        push2(void);
void        pushval(cell val);
void        pop1(void);
void        pop2(void);
void        swap1(void);
void        ffswitch(int label);
void        ffcase(cell value, char *labelname, int newtable);
void        ffcall(symbol * sym, int numargs);
void        ffret(void);
void        ffabort(int reason);
void        ffbounds(cell size);
void        jumplabel(int number);
void        defstorage(void);
void        modstk(int delta);
void        setstk(cell value);
void        modheap(int delta);
void        setheap_pri(void);
void        setheap(cell value);
void        cell2addr(void);
void        cell2addr_alt(void);
void        addr2cell(void);
void        char2addr(void);
void        charalign(void);
void        addconst(cell value);

/*  Code generation functions for arithmetic operators.
 *
 *  Syntax: o[u|s|b]_name
 *          |   |   | +--- name of operator
 *          |   |   +----- underscore
 *          |   +--------- "u"nsigned operator, "s"igned operator or "b"oth
 *          +------------- "o"perator
 */
void        os_mult(void);	/* multiplication (signed) */
void        os_div(void);	/* division (signed) */
void        os_mod(void);	/* modulus (signed) */
void        ob_add(void);	/* addition */
void        ob_sub(void);	/* subtraction */
void        ob_sal(void);	/* shift left (arithmetic) */
void        os_sar(void);	/* shift right (arithmetic, signed) */
void        ou_sar(void);	/* shift right (logical, unsigned) */
void        ob_or(void);	/* bitwise or */
void        ob_xor(void);	/* bitwise xor */
void        ob_and(void);	/* bitwise and */
void        ob_eq(void);	/* equality */
void        ob_ne(void);	/* inequality */
void        relop_prefix(void);
void        relop_suffix(void);
void        os_le(void);	/* less or equal (signed) */
void        os_ge(void);	/* greater or equal (signed) */
void        os_lt(void);	/* less (signed) */
void        os_gt(void);	/* greater (signed) */

void        lneg(void);
void        neg(void);
void        invert(void);
void        nooperation(void);
void        inc(value * lval);
void        dec(value * lval);
void        jmp_ne0(int number);
void        jmp_eq0(int number);
void        outval(cell val, int newline);

/* function prototypes in SC5.C */
int         error(int number, ...);
void        errorset(int code);

/* function prototypes in SC6.C */
void        assemble(FILE * fout, FILE * fin);

/* function prototypes in SC7.C */
void        stgbuffer_cleanup(void);
void        stgmark(char mark);
void        stgwrite(char *st);
void        stgout(int index);
void        stgdel(int index, cell code_index);
int         stgget(int *index, cell * code_index);
void        stgset(int onoff);
int         phopt_init(void);
int         phopt_cleanup(void);

/* function prototypes in SCLIST.C */
stringpair *insert_alias(char *name, char *alias);
stringpair *find_alias(char *name);
int         lookup_alias(char *target, char *name);
void        delete_aliastable(void);
stringlist *insert_path(char *path);
char       *get_path(int index);
void        delete_pathtable(void);
stringpair *insert_subst(char *pattern, char *substitution,
				 int prefixlen);
int         get_subst(int index, char **pattern, char **substitution);
stringpair *find_subst(char *name, int length);
int         delete_subst(char *name, int length);
void        delete_substtable(void);

/* external variables (defined in scvars.c) */
extern symbol     loctab;	/* local symbol table */
extern symbol     glbtab;	/* global symbol table */
extern cell      *litq;	/* the literal queue */
extern char       pline[];	/* the line read from the input file */
extern char      *lptr;	/* points to the current position in "pline" */
extern constvalue tagname_tab;	/* tagname table */
extern constvalue libname_tab;	/* library table (#pragma library "..." syntax) *///??? use "stringlist" type
extern constvalue *curlibrary;	/* current library */
extern symbol    *curfunc;	/* pointer to current function */
extern char      *inpfname;	/* name of the file currently read from */
extern char       outfname[];	/* output file name */
extern char       sc_ctrlchar;	/* the control character (or escape character) */
extern int        litidx;	/* index to literal table */
extern int        litmax;	/* current size of the literal table */
extern int        stgidx;	/* index to the staging buffer */
extern int        labnum;	/* number of (internal) labels */
extern int        staging;	/* true if staging output */
extern cell       declared;	/* number of local cells declared */
extern cell       glb_declared;	/* number of global cells declared */
extern cell       code_idx;	/* number of bytes with generated code */
extern int        ntv_funcid;	/* incremental number of native function */
extern int        errnum;	/* number of errors */
extern int        warnnum;	/* number of warnings */
extern int        sc_debug;	/* debug/optimization options (bit field) */
extern int        charbits;	/* number of bits for a character */
extern int        sc_packstr;	/* strings are packed by default? */
extern int        sc_asmfile;	/* create .ASM file? */
extern int        sc_listing;	/* create .LST file? */
extern int        sc_compress;	/* compress bytecode? */
extern int        sc_needsemicolon;	/* semicolon required to terminate expressions? */
extern int        sc_dataalign;	/* data alignment value */
extern int        sc_alignnext;	/* must frame of the next function be aligned? */
extern int        curseg;	/* 1 if currently parsing CODE, 2 if parsing DATA */
extern cell       sc_stksize;	/* stack size */
extern int        freading;	/* is there an input file ready for reading? */
extern int        fline;	/* the line number in the current file */
extern int        fnumber;	/* number of files in the file table (debugging) */
extern int        fcurrent;	/* current file being processed (debugging) */
extern int        intest;	/* true if inside a test */
extern int        sideeffect;	/* true if an expression causes a side-effect */
extern int        stmtindent;	/* current indent of the statement */
extern int        indent_nowarn;	/* skip warning "217 loose indentation" */
extern int        sc_tabsize;	/* number of spaces that a TAB represents */
extern int        sc_allowtags;	/* allow/detect tagnames in lex() */
extern int        sc_status;	/* read/write status */
extern int        sc_rationaltag;	/* tag for rational numbers */
extern int        rational_digits;	/* number of fractional digits */

extern FILE      *inpf;	/* file read from (source or include) */
extern FILE      *inpf_org;	/* main source file */
extern FILE      *outf;	/* file written to */

extern jmp_buf    errbuf;	/* target of longjmp() on a fatal error */

#endif
