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
#ifndef __SC_H
#define __SC_H
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#if defined __BORLANDC__ && defined _Windows && !(defined __32BIT__ || defined __WIN32__)
  /* setjmp() and longjmp() not well supported in 16-bit windows */
  #include <windows.h>
  typedef int jmp_buf[9];
  #define setjmp(b)     Catch(b)
  #define longjmp(b,e)  Throw(b,e)
#else
  #include <setjmp.h>
#endif
#include "embryo_cc_osdefs.h"
#include "embryo_cc_amx.h"

/* Note: the "cell" and "ucell" types are defined in AMX.H */

#define PUBLIC_CHAR '@'     /* character that defines a function "public" */
#define CTRL_CHAR   '\\'    /* default control character */

#if defined __MSDOS__ || defined __WIN32__ || defined _Windows
  #define DIRSEP_CHAR '\\'
#elif defined macintosh
  #define DIRSEP_CHAR ':'
#else
  #define DIRSEP_CHAR '/'   /* directory separator character */
#endif

#define sDIMEN_MAX     2    /* maximum number of array dimensions */
#define sDEF_LITMAX  500    /* initial size of the literal pool, in "cells" */
#define sLINEMAX     511    /* input line length (in characters) */
#define sDEF_AMXSTACK 4096  /* default stack size for AMX files */
#define sSTKMAX       80    /* stack for nested #includes and other uses */
#define PREPROC_TERM  '\x7f'/* termination character for preprocessor expressions (the "DEL" code) */
#define sDEF_PREFIX   "default.inc" /* default prefix filename */

typedef void *stkitem;      /* type of items stored on the stack */

typedef struct __s_arginfo {  /* function argument info */
  char name[sNAMEMAX+1];
  char ident;           /* iVARIABLE, iREFERENCE, iREFARRAY or iVARARGS */
  char usage;           /* uCONST */
  int *tags;            /* argument tag id. list */
  int numtags;          /* number of tags in the tag list */
  int dim[sDIMEN_MAX];
  int numdim;           /* number of dimensions */
  unsigned char hasdefault; /* bit0: is there a default value? bit6: "tagof"; bit7: "sizeof" */
  union {
    cell val;           /* default value */
    struct {
      char *symname;    /* name of another symbol */
      short level;      /* indirection level for that symbol */
    } size;             /* used for "sizeof" default value */
    struct {
      cell *data;       /* values of default array */
      int size;         /* complete length of default array */
      int arraysize;    /* size to reserve on the heap */
      cell addr;        /* address of the default array in the data segment */
    } array;
  } defvalue;           /* default value, or pointer to default array */
  int defvalue_tag;     /* tag of the default value */
} arginfo;

/*  Equate table, tagname table, library table */
typedef struct __s_constvalue {
  struct __s_constvalue *next;
  char name[sNAMEMAX+1];
  cell value;
  short index;
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
typedef struct __s_symbol {
  struct __s_symbol *next;
  struct __s_symbol *parent;  /* hierarchical types (multi-dimensional arrays) */
  char name[sNAMEMAX+1];
  uint32_t hash;        /* value derived from name, for quicker searching */
  cell addr;            /* address or offset (or value for constant, index for native function) */
  char vclass;          /* sLOCAL if "addr" refers to a local symbol */
  char ident;           /* see below for possible values */
  char usage;           /* see below for possible values */
  int compound;         /* compound level (braces nesting level) */
  int tag;              /* tagname id */
  union {
    int declared;       /* label: how many local variables are declared */
    int idxtag;         /* array: tag of array indices */
    constvalue *lib;    /* native function: library it is part of */ //??? use "stringlist"
  } x;                  /* 'x' for 'extra' */
  union {
    arginfo *arglist;   /* types of all parameters for functions */
    struct {
      cell length;      /* arrays: length (size) */
      short level;      /* number of dimensions below this level */
    } array;
  } dim;                /* for 'dimension', both functions and arrays */
  int fnumber;          /* static global variables: file number in which the declaration is visible */
  struct __s_symbol **refer;  /* referrer list, functions that "use" this symbol */
  int numrefers;              /* number of entries in the referrer list */
} symbol;


/*  Possible entries for "ident". These are used in the "symbol", "value"
 *  and arginfo structures. Not every constant is valid for every use.
 *  In an argument list, the list is terminated with a "zero" ident; labels
 *  cannot be passed as function arguments, so the value 0 is overloaded.
 */
#define iLABEL      0
#define iVARIABLE   1   /* cell that has an address and that can be fetched directly (lvalue) */
#define iREFERENCE  2   /* iVARIABLE, but must be dereferenced */
#define iARRAY      3
#define iREFARRAY   4   /* an array passed by reference (i.e. a pointer) */
#define iARRAYCELL  5   /* array element, cell that must be fetched indirectly */
#define iARRAYCHAR  6   /* array element, character from cell from array */
#define iEXPRESSION 7   /* expression result, has no address (rvalue) */
#define iCONSTEXPR  8   /* constant expression (or constant symbol) */
#define iFUNCTN     9
#define iREFFUNC    10  /* function passed as a parameter */
#define iVARARGS    11  /* function specified ... as argument(s) */

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
#define uRETVALUE 0x04  /* function returns (or should return) a value */
#define uCONST    0x08
#define uPROTOTYPED 0x08
#define uPREDEF   0x08  /* constant is pre-defined */
#define uPUBLIC   0x10
#define uNATIVE   0x20
#define uSTOCK    0x40
#define uMISSING  0x80
/* uRETNONE is not stored in the "usage" field of a symbol. It is
 * used during parsing a function, to detect a mix of "return;" and
 * "return value;" in a few special cases.
 */
#define uRETNONE  0x10

#define uTAGOF    0x40  /* set in the "hasdefault" field of the arginfo struct */
#define uSIZEOF   0x80  /* set in the "hasdefault" field of the arginfo struct */

#define uMAINFUNC "main"

#define sGLOBAL   0     /* global/local variable/constant class */
#define sLOCAL    1
#define sSTATIC   2     /* global life, local scope */

typedef struct {
  symbol *sym;          /* symbol in symbol table, NULL for (constant) expression */
  cell constval;        /* value of the constant expression (if ident==iCONSTEXPR)
                         * also used for the size of a literal array */
  int tag;              /* tagname id (of the expression) */
  char ident;           /* iCONSTEXPR, iVARIABLE, iARRAY, iARRAYCELL,
                         * iEXPRESSION or iREFERENCE */
  char boolresult;      /* boolean result for relational operators */
  cell *arrayidx;       /* last used array indices, for checking self assignment */
} value;

/*  "while" statement queue (also used for "for" and "do - while" loops) */
enum {
  wqBRK,        /* used to restore stack for "break" */
  wqCONT,       /* used to restore stack for "continue" */
  wqLOOP,       /* loop start label number */
  wqEXIT,       /* loop exit label number (jump if false) */
  /* --- */
  wqSIZE        /* "while queue" size */
};
#define wqTABSZ (24*wqSIZE)    /* 24 nested loop statements */

enum {
  statIDLE,     /* not compiling yet */
  statFIRST,    /* first pass */
  statWRITE,    /* writing output */
  statSKIP,     /* skipping output */
};

typedef struct __s_stringlist {
  struct __s_stringlist *next;
  char *line;
} stringlist;

typedef struct __s_stringpair {
  struct __s_stringpair *next;
  char *first;
  char *second;
  int matchlength;
} stringpair;

/* macros for code generation */
#define opcodes(n)      ((n)*sizeof(cell))      /* opcode size */
#define opargs(n)       ((n)*sizeof(cell))      /* size of typical argument */

/*  Tokens recognized by lex()
 *  Some of these constants are assigned as well to the variable "lastst"
 */
#define tFIRST   256    /* value of first multi-character operator */
#define tMIDDLE  279    /* value of last multi-character operator */
#define tLAST    320    /* value of last multi-character match-able token */
/* multi-character operators */
#define taMULT   256    /* *= */
#define taDIV    257    /* /= */
#define taMOD    258    /* %= */
#define taADD    259    /* += */
#define taSUB    260    /* -= */
#define taSHL    261    /* <<= */
#define taSHRU   262    /* >>>= */
#define taSHR    263    /* >>= */
#define taAND    264    /* &= */
#define taXOR    265    /* ^= */
#define taOR     266    /* |= */
#define tlOR     267    /* || */
#define tlAND    268    /* && */
#define tlEQ     269    /* == */
#define tlNE     270    /* != */
#define tlLE     271    /* <= */
#define tlGE     272    /* >= */
#define tSHL     273    /* << */
#define tSHRU    274    /* >>> */
#define tSHR     275    /* >> */
#define tINC     276    /* ++ */
#define tDEC     277    /* -- */
#define tELLIPS  278    /* ... */
#define tDBLDOT  279    /* .. */
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
#define tpASSERT 308    /* #assert */
#define tpDEFINE 309
#define tpELSE   310    /* #else */
#define tpEMIT   311
#define tpENDIF  312
#define tpENDINPUT 313
#define tpENDSCRPT 314
#define tpFILE   315
#define tpIF     316    /* #if */
#define tINCLUDE 317
#define tpLINE   318
#define tpPRAGMA 319
#define tpUNDEF  320
/* semicolon is a special case, because it can be optional */
#define tTERM    321    /* semicolon or newline */
#define tENDEXPR 322    /* forced end of expression */
/* other recognized tokens */
#define tNUMBER  323    /* integer number */
#define tRATIONAL 324   /* rational number */
#define tSYMBOL  325
#define tLABEL   326
#define tSTRING  327
#define tEXPR    328    /* for assigment to "lastst" only */

/* (reversed) evaluation of staging buffer */
#define sSTARTREORDER 1
#define sENDREORDER   2
#define sEXPRSTART    0xc0      /* top 2 bits set, rest is free */
#define sMAXARGS      64        /* relates to the bit pattern of sEXPRSTART */

/* codes for ffabort() */
#define xEXIT           1       /* exit code in PRI */
#define xASSERTION      2       /* abort caused by failing assertion */
#define xSTACKERROR     3       /* stack/heap overflow */
#define xBOUNDSERROR    4       /* array index out of bounds */
#define xMEMACCESS      5       /* data access error */
#define xINVINSTR       6       /* invalid instruction */
#define xSTACKUNDERFLOW 7       /* stack underflow */
#define xHEAPUNDERFLOW  8       /* heap underflow */
#define xCALLBACKERR    9       /* no, or invalid, callback */
#define xSLEEP         12       /* sleep, exit code in PRI, tag in ALT */

/* Miscellaneous  */
#if !defined TRUE
  #define FALSE         0
  #define TRUE          1
#endif
#define sIN_CSEG        1       /* if parsing CODE */
#define sIN_DSEG        2       /* if parsing DATA */
#define sCHKBOUNDS      1       /* bit position in "debug" variable: check bounds */
#define sSYMBOLIC       2       /* bit position in "debug" variable: symbolic info */
#define sNOOPTIMIZE     4       /* bit position in "debug" variable: no optimization */
#define sRESET          0       /* reset error flag */
#define sFORCESET       1       /* force error flag on */
#define sEXPRMARK       2       /* mark start of expression */
#define sEXPRRELEASE    3       /* mark end of expression */

#if INT_MAX<0x8000u
  #define PUBLICTAG   0x8000u
  #define FIXEDTAG    0x4000u
#else
  #define PUBLICTAG   0x80000000Lu
  #define FIXEDTAG    0x40000000Lu
#endif
#define TAGMASK       (~PUBLICTAG)


/* interface functions */
#if defined __cplusplus
  extern "C" {
#endif

/*
 * Functions you call from the "driver" program
 */
int sc_compile(int argc, char **argv);
int sc_addconstant(char *name,cell value,int tag);
int sc_addtag(char *name);

/*
 * Functions called from the compiler (to be implemented by you)
 */

/* general console output */
int sc_printf(const char *message,...);

/* error report function */
int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr);

/* input from source file */
void *sc_opensrc(char *filename); /* reading only */
void sc_closesrc(void *handle);   /* never delete */
void sc_resetsrc(void *handle,void *position);  /* reset to a position marked earlier */
char *sc_readsrc(void *handle,char *target,int maxchars);
void *sc_getpossrc(void *handle); /* mark the current position */
int  sc_eofsrc(void *handle);

/* output to intermediate (.ASM) file */
void *sc_openasm(char *filename); /* read/write */
void sc_closeasm(void *handle,int deletefile);
void sc_resetasm(void *handle);
int  sc_writeasm(void *handle,char *str);
char *sc_readasm(void *handle,char *target,int maxchars);

/* output to binary (.AMX) file */
void *sc_openbin(char *filename);
void sc_closebin(void *handle,int deletefile);
void sc_resetbin(void *handle);
int  sc_writebin(void *handle,void *buffer,int size);
long sc_lengthbin(void *handle); /* return the length of the file */

#if defined __cplusplus
  }
#endif


/* by default, functions and variables used in throughout the compiler
 * files are "external"
 */
#if !defined SC_FUNC
  #define SC_FUNC
#endif
#if !defined SC_VDECL
  #define SC_VDECL  extern
#endif
#if !defined SC_VDEFINE
  #define SC_VDEFINE
#endif

/* function prototypes in SC1.C */
SC_FUNC void set_extension(char *filename,char *extension,int force);
SC_FUNC symbol *fetchfunc(char *name,int tag);
SC_FUNC char *operator_symname(char *symname,char *opername,int tag1,int tag2,int numtags,int resulttag);
SC_FUNC char *funcdisplayname(char *dest,char *funcname);
SC_FUNC int constexpr(cell *val,int *tag);
SC_FUNC constvalue *append_constval(constvalue *table,char *name,cell val,short index);
SC_FUNC constvalue *find_constval(constvalue *table,char *name,short index);
SC_FUNC void delete_consttable(constvalue *table);
SC_FUNC void add_constant(char *name,cell val,int vclass,int tag);
SC_FUNC void exporttag(int tag);

/* function prototypes in SC2.C */
SC_FUNC void pushstk(stkitem val);
SC_FUNC stkitem popstk(void);
SC_FUNC int plungequalifiedfile(char *name);  /* explicit path included */
SC_FUNC int plungefile(char *name,int try_currentpath,int try_includepaths);   /* search through "include" paths */
SC_FUNC void preprocess(void);
SC_FUNC void lexinit(void);
SC_FUNC int lex(cell *lexvalue,char **lexsym);
SC_FUNC void lexpush(void);
SC_FUNC void lexclr(int clreol);
SC_FUNC int matchtoken(int token);
SC_FUNC int tokeninfo(cell *val,char **str);
SC_FUNC int needtoken(int token);
SC_FUNC void stowlit(cell value);
SC_FUNC int alphanum(char c);
SC_FUNC int ishex(char c);
SC_FUNC void delete_symbol(symbol *root,symbol *sym);
SC_FUNC void delete_symbols(symbol *root,int level,int del_labels,int delete_functions);
SC_FUNC int refer_symbol(symbol *entry,symbol *bywhom);
SC_FUNC void markusage(symbol *sym,int usage);
SC_FUNC uint32_t namehash(char *name);
SC_FUNC symbol *findglb(char *name);
SC_FUNC symbol *findloc(char *name);
SC_FUNC symbol *findconst(char *name);
SC_FUNC symbol *finddepend(symbol *parent);
SC_FUNC symbol *addsym(char *name,cell addr,int ident,int vclass,int tag,
                       int usage);
SC_FUNC symbol *addvariable(char *name,cell addr,int ident,int vclass,int tag,
                            int dim[],int numdim,int idxtag[]);
SC_FUNC int getlabel(void);
SC_FUNC char *itoh(ucell val);

/* function prototypes in SC3.C */
SC_FUNC int check_userop(void (*oper)(void),int tag1,int tag2,int numparam,
                         value *lval,int *resulttag);
SC_FUNC int matchtag(int formaltag,int actualtag,int allowcoerce);
SC_FUNC int expression(int *constant,cell *val,int *tag,int chkfuncresult);
SC_FUNC int hier14(value *lval1);       /* the highest expression level */

/* function prototypes in SC4.C */
SC_FUNC void writeleader(void);
SC_FUNC void writetrailer(void);
SC_FUNC void begcseg(void);
SC_FUNC void begdseg(void);
SC_FUNC void setactivefile(int fnumber);
SC_FUNC cell nameincells(char *name);
SC_FUNC void setfile(char *name,int fileno);
SC_FUNC void setline(int line,int fileno);
SC_FUNC void setfiledirect(char *name);
SC_FUNC void setlinedirect(int line);
SC_FUNC void setlabel(int index);
SC_FUNC void endexpr(int fullexpr);
SC_FUNC void startfunc(char *fname);
SC_FUNC void endfunc(void);
SC_FUNC void alignframe(int numbytes);
SC_FUNC void defsymbol(char *name,int ident,int vclass,cell offset,int tag);
SC_FUNC void symbolrange(int level,cell size);
SC_FUNC void rvalue(value *lval);
SC_FUNC void address(symbol *ptr);
SC_FUNC void store(value *lval);
SC_FUNC void memcopy(cell size);
SC_FUNC void copyarray(symbol *sym,cell size);
SC_FUNC void fillarray(symbol *sym,cell size,cell value);
SC_FUNC void const1(cell val);
SC_FUNC void const2(cell val);
SC_FUNC void moveto1(void);
SC_FUNC void push1(void);
SC_FUNC void push2(void);
SC_FUNC void pushval(cell val);
SC_FUNC void pop1(void);
SC_FUNC void pop2(void);
SC_FUNC void swap1(void);
SC_FUNC void ffswitch(int label);
SC_FUNC void ffcase(cell value,char *labelname,int newtable);
SC_FUNC void ffcall(symbol *sym,int numargs);
SC_FUNC void ffret(void);
SC_FUNC void ffabort(int reason);
SC_FUNC void ffbounds(cell size);
SC_FUNC void jumplabel(int number);
SC_FUNC void defstorage(void);
SC_FUNC void modstk(int delta);
SC_FUNC void setstk(cell value);
SC_FUNC void modheap(int delta);
SC_FUNC void setheap_pri(void);
SC_FUNC void setheap(cell value);
SC_FUNC void cell2addr(void);
SC_FUNC void cell2addr_alt(void);
SC_FUNC void addr2cell(void);
SC_FUNC void char2addr(void);
SC_FUNC void charalign(void);
SC_FUNC void addconst(cell value);

/*  Code generation functions for arithmetic operators.
 *
 *  Syntax: o[u|s|b]_name
 *          |   |   | +--- name of operator
 *          |   |   +----- underscore
 *          |   +--------- "u"nsigned operator, "s"igned operator or "b"oth
 *          +------------- "o"perator
 */
SC_FUNC void os_mult(void); /* multiplication (signed) */
SC_FUNC void os_div(void);  /* division (signed) */
SC_FUNC void os_mod(void);  /* modulus (signed) */
SC_FUNC void ob_add(void);  /* addition */
SC_FUNC void ob_sub(void);  /* subtraction */
SC_FUNC void ob_sal(void);  /* shift left (arithmetic) */
SC_FUNC void os_sar(void);  /* shift right (arithmetic, signed) */
SC_FUNC void ou_sar(void);  /* shift right (logical, unsigned) */
SC_FUNC void ob_or(void);   /* bitwise or */
SC_FUNC void ob_xor(void);  /* bitwise xor */
SC_FUNC void ob_and(void);  /* bitwise and */
SC_FUNC void ob_eq(void);   /* equality */
SC_FUNC void ob_ne(void);   /* inequality */
SC_FUNC void relop_prefix(void);
SC_FUNC void relop_suffix(void);
SC_FUNC void os_le(void);   /* less or equal (signed) */
SC_FUNC void os_ge(void);   /* greater or equal (signed) */
SC_FUNC void os_lt(void);   /* less (signed) */
SC_FUNC void os_gt(void);   /* greater (signed) */

SC_FUNC void lneg(void);
SC_FUNC void neg(void);
SC_FUNC void invert(void);
SC_FUNC void nooperation(void);
SC_FUNC void inc(value *lval);
SC_FUNC void dec(value *lval);
SC_FUNC void jmp_ne0(int number);
SC_FUNC void jmp_eq0(int number);
SC_FUNC void outval(cell val,int newline);

/* function prototypes in SC5.C */
SC_FUNC int error(int number,...);
SC_FUNC void errorset(int code);

/* function prototypes in SC6.C */
SC_FUNC void assemble(FILE *fout,FILE *fin);

/* function prototypes in SC7.C */
SC_FUNC void stgbuffer_cleanup(void);
SC_FUNC void stgmark(char mark);
SC_FUNC void stgwrite(char *st);
SC_FUNC void stgout(int index);
SC_FUNC void stgdel(int index,cell code_index);
SC_FUNC int stgget(int *index,cell *code_index);
SC_FUNC void stgset(int onoff);
SC_FUNC int phopt_init(void);
SC_FUNC int phopt_cleanup(void);

/* function prototypes in SCLIST.C */
SC_FUNC char* duplicatestring(const char* sourcestring);
SC_FUNC stringpair *insert_alias(char *name,char *alias);
SC_FUNC stringpair *find_alias(char *name);
SC_FUNC int lookup_alias(char *target,char *name);
SC_FUNC void delete_aliastable(void);
SC_FUNC stringlist *insert_path(char *path);
SC_FUNC char *get_path(int index);
SC_FUNC void delete_pathtable(void);
SC_FUNC stringpair *insert_subst(char *pattern,char *substitution,int prefixlen);
SC_FUNC int get_subst(int index,char **pattern,char **substitution);
SC_FUNC stringpair *find_subst(char *name,int length);
SC_FUNC int delete_subst(char *name,int length);
SC_FUNC void delete_substtable(void);

/* external variables (defined in scvars.c) */
#if !defined SC_SKIP_VDECL
SC_VDECL symbol loctab;       /* local symbol table */
SC_VDECL symbol glbtab;       /* global symbol table */
SC_VDECL cell *litq;          /* the literal queue */
SC_VDECL char pline[];        /* the line read from the input file */
SC_VDECL char *lptr;          /* points to the current position in "pline" */
SC_VDECL constvalue tagname_tab;/* tagname table */
SC_VDECL constvalue libname_tab;/* library table (#pragma library "..." syntax) */ //??? use "stringlist" type
SC_VDECL constvalue *curlibrary;/* current library */
SC_VDECL symbol *curfunc;     /* pointer to current function */
SC_VDECL char *inpfname;      /* name of the file currently read from */
SC_VDECL char outfname[];     /* output file name */
SC_VDECL char errfname[];     /* error file name */
SC_VDECL char sc_ctrlchar;    /* the control character (or escape character) */
SC_VDECL int litidx;          /* index to literal table */
SC_VDECL int litmax;          /* current size of the literal table */
SC_VDECL int stgidx;          /* index to the staging buffer */
SC_VDECL int labnum;          /* number of (internal) labels */
SC_VDECL int staging;         /* true if staging output */
SC_VDECL cell declared;       /* number of local cells declared */
SC_VDECL cell glb_declared;   /* number of global cells declared */
SC_VDECL cell code_idx;       /* number of bytes with generated code */
SC_VDECL int ntv_funcid;      /* incremental number of native function */
SC_VDECL int errnum;          /* number of errors */
SC_VDECL int warnnum;         /* number of warnings */
SC_VDECL int sc_debug;        /* debug/optimization options (bit field) */
SC_VDECL int charbits;        /* number of bits for a character */
SC_VDECL int sc_packstr;      /* strings are packed by default? */
SC_VDECL int sc_asmfile;      /* create .ASM file? */
SC_VDECL int sc_listing;      /* create .LST file? */
SC_VDECL int sc_compress;     /* compress bytecode? */
SC_VDECL int sc_needsemicolon;/* semicolon required to terminate expressions? */
SC_VDECL int sc_dataalign;    /* data alignment value */
SC_VDECL int sc_alignnext;    /* must frame of the next function be aligned? */
SC_VDECL int curseg;          /* 1 if currently parsing CODE, 2 if parsing DATA */
SC_VDECL cell sc_stksize;     /* stack size */
SC_VDECL int freading;        /* is there an input file ready for reading? */
SC_VDECL int fline;           /* the line number in the current file */
SC_VDECL int fnumber;         /* number of files in the file table (debugging) */
SC_VDECL int fcurrent;        /* current file being processed (debugging) */
SC_VDECL int intest;          /* true if inside a test */
SC_VDECL int sideeffect;      /* true if an expression causes a side-effect */
SC_VDECL int stmtindent;      /* current indent of the statement */
SC_VDECL int indent_nowarn;   /* skip warning "217 loose indentation" */
SC_VDECL int sc_tabsize;      /* number of spaces that a TAB represents */
SC_VDECL int sc_allowtags;    /* allow/detect tagnames in lex() */
SC_VDECL int sc_status;       /* read/write status */
SC_VDECL int sc_rationaltag;  /* tag for rational numbers */
SC_VDECL int rational_digits; /* number of fractional digits */

SC_VDECL FILE *inpf;          /* file read from (source or include) */
SC_VDECL FILE *inpf_org;      /* main source file */
SC_VDECL FILE *outf;          /* file written to */

SC_VDECL jmp_buf errbuf;      /* target of longjmp() on a fatal error */

#if !defined SC_LIGHT
  SC_VDECL int sc_makereport; /* generate a cross-reference report */
#endif

#endif /* SC_SKIP_VDECL */

#endif /* __SC_H */
