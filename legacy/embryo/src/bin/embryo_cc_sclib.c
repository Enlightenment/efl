/*  SCLIB.C
 *
 *  This is an example file that shows how to embed the Small compiler into a
 *  program. This program contains the "default" implementation of all
 *  functions that the Small compiler calls for I/O.
 *
 *  This program also contains a main(), so it compiles, again, to a
 *  stand-alone compiler. This is for illustration purposes only
 *
 *  What this file does is (in sequence):
 *  1. Declare the NO_MAIN macro, so that the function main() and all
 *     "callback" functions that are in SC1.C are not compiled.
 *  2. Declare SC_FUNC and SC_VDEFINE as "static" so that all functions and
 *     global variables are "encapsulated" in the object file. This solves
 *     the global namespace polution problem.
 *  3. Declare the SC_SKIP_VDECL macro which is needed to avoid variables to
 *     be doubly declared when the C files are *not* independently compiled.
 *  4. And, the dirtiest trick of all, include the remaining C files. That is,
 *     the entire Small compiler compiles to a single object file (.OBJ in
 *     Windows). This is the only way to get rid of the global namespace
 *     polution.
 *
 *  Note that the interface of the Small compiler is subject to change.
 *
 *  Compilation:
 *      wcl386 /l=nt sclib.c
 *
 *  Copyright (c) ITB CompuPhase, 2000-2003
 *  This file may be freely used. No warranties of any kind.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NO_MAIN
#define SC_FUNC    static
#define SC_VDEFINE static
#define SC_SKIP_VDECL   /* skip variable "forward declaration" */
#include "sc.h"

#include "scvars.c"
#include "sc1.c"
#include "sc2.c"
#include "sc3.c"
#include "sc4.c"
#include "sc5.c"
#include "sc6.c"
#include "sc7.c"
#include "sclist.c"
#include "scexpand.c"

int main(int argc, char **argv)
{
  return sc_compile(argc,argv);
}

/* sc_printf
 * Called for general purpose "console" output. This function prints general
 * purpose messages; errors go through sc_error(). The function is modelled
 * after printf().
 */
int sc_printf(const char *message,...)
{
  int ret;
  va_list argptr;

  va_start(argptr,message);
  ret=vprintf(message,argptr);
  va_end(argptr);

  return ret;
}

/* sc_error
 * Called for producing error output.
 *    number      the error number (as documented in the manual)
 *    message     a string describing the error with embedded %d and %s tokens
 *    filename    the name of the file currently being parsed
 *    firstline   the line number at which the expression started on which
 *                the error was found, or -1 if there is no "starting line"
 *    lastline    the line number at which the error was detected
 *    argptr      a pointer to the first of a series of arguments (for macro
 *                "va_arg")
 * Return:
 *    If the function returns 0, the parser attempts to continue compilation.
 *    On a non-zero return value, the parser aborts.
 */
int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr)
{
static char *prefix[3]={ "Error", "Fatal", "Warning" };

  if (number!=0) {
    char *pre;

    pre=prefix[number/100];
    if (firstline>=0)
      printf("%s(%d -- %d) %s [%03d]: ",filename,firstline,lastline,pre,number);
    else
      printf("%s(%d) %s [%03d]: ",filename,lastline,pre,number);
  } /* if */
  vprintf(message,argptr);
  fflush(stdout);
  return 0;
}

/* sc_opensrc
 * Opens a source file (or include file) for reading. The "file" does not have
 * to be a physical file, one might compile from memory.
 *    filename    the name of the "file" to read from
 * Return:
 *    The function must return a pointer, which is used as a "magic cookie" to
 *    all I/O functions. When failing to open the file for reading, the
 *    function must return NULL.
 */
void *sc_opensrc(char *filename)
{
  return fopen(filename,"rt");
}

/* sc_closesrc
 * Closes a source file (or include file). The "handle" parameter has the
 * value that sc_opensrc() returned in an earlier call.
 */
void sc_closesrc(void *handle)
{
  assert(handle!=NULL);
  fclose((FILE*)handle);
}

/* sc_resetsrc
 * "position" may only hold a pointer that was previously obtained from
 * sc_getpossrc() */
void sc_resetsrc(void *handle,void *position)
{
  assert(handle!=NULL);
  fsetpos((FILE*)handle,(fpos_t *)position);
}

char *sc_readsrc(void *handle,char *target,int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

void *sc_getpossrc(void *handle)
{
  static fpos_t lastpos;

  fgetpos((FILE*)handle,&lastpos);
  return &lastpos;
}

int sc_eofsrc(void *handle)
{
  return feof((FILE*)handle);
}

/* should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure
 */
void *sc_openasm(char *filename)
{
  return fopen(filename,"w+t");
}

void sc_closeasm(void *handle, int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(outfname);
}

void sc_resetasm(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int sc_writeasm(void *handle,char *st)
{
  return fputs(st,(FILE*)handle) >= 0;
}

char *sc_readasm(void *handle, char *target, int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

/* Should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure.
 */
void *sc_openbin(char *filename)
{
  return fopen(filename,"wb");
}

void sc_closebin(void *handle,int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(binfname);
}

void sc_resetbin(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int sc_writebin(void *handle,void *buffer,int size)
{
  return fwrite(buffer,1,size,(FILE*)handle) == size;
}

long sc_lengthbin(void *handle)
{
  return ftell((FILE*)handle);
}

