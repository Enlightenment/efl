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
#include <locale.h>
#include <ctype.h>
#include <alloca.h>

/* types */
typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;
typedef struct _Font                  Font;
typedef struct _Code                  Code;
typedef struct _Code_Program          Code_Program;

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

struct _Font
{
   char *file;
   char *name;
};

struct _Code
{
   char      *shared;
   Evas_List *programs; 
};

struct _Code_Program
{
   int        id;
   char      *script;
};

/* global fn calls */
void    data_setup(void);
void    data_write(void);
void    data_queue_part_lookup(Edje_Part_Collection *pc, char *name, int *dest);
void    data_queue_program_lookup(Edje_Part_Collection *pc, char *name, int *dest);
void    data_queue_image_lookup(char *name, int *dest);
void    data_process_lookups(void);
    

int     is_verbatim(void);
void    track_verbatim(int on);
void    set_verbatim(char *s);
char   *get_verbatim(void);
void    compile(void);
int     is_param(int n);
int     is_num(int n);    
char   *parse_str(int n);
int     parse_enum(int n, ...);
int     parse_int(int n);
int     parse_int_range(int n, int f, int t);
double  parse_float(int n);
double  parse_float_range(int n, double f, double t);

int     object_handler_num(void);
int     statement_handler_num(void);

void   *mem_alloc(size_t size);
char   *mem_strdup(const char *s);
#define SZ sizeof

/* global vars */
extern Evas_List             *img_dirs;
extern Evas_List             *fnt_dirs;
extern char                  *file_in;
extern char                  *file_out;
extern char                  *progname;
extern int                    verbose;
extern int                    line;
extern Evas_List             *stack;
extern Evas_List             *params;
extern Edje_File             *edje_file;
extern Evas_List             *edje_collections;
extern Evas_List             *fonts;
extern Evas_List             *codes;
extern New_Object_Handler     object_handlers[];
extern New_Statement_Handler  statement_handlers[];

#endif
