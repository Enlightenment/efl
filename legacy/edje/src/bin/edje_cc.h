#ifndef EDJE_CC_H
#define EDJE_CC_H

#include "edje.h"
/* Imlib2 stuff for loading up input images */
#define X_DISPLAY_MISSING
#include <Imlib2.h>
/* done Imlib2 stuff */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;

struct _New_Object_Handler
{
   char *type;
   void (*func)(void);
};

struct _New_Statement_Handler
{
   char *type;
   void (*func)(void);
};

void  data_setup(void);
void  data_write(void);
void  compile(void);
int   object_handler_num(void);
int   statement_handler_num(void);

char   *parse_str(int n);
int     parse_enum(int n, ...);
int     parse_int(int n);
int     parse_int_range(int n, int f, int t);
double  parse_float(int n);
double  parse_float_range(int n, int f, int t);    
    
extern Evas_List             *img_dirs;
extern char                  *file_in;
extern char                  *file_out;
extern char                  *progname;
extern int                    line;
extern Evas_List             *stack;
extern Evas_List             *params;
extern Edje_File             *edje_file;
extern New_Object_Handler     object_handlers[];
extern New_Statement_Handler  statement_handlers[];

#endif
