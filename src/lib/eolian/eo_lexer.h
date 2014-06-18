#ifndef __EO_TOKENIZER_H__
#define __EO_TOKENIZER_H__

#include <Eina.h>
#include <Eolian.h>
#include "eo_definitions.h"

/* TOKENIZER */

#define BUFSIZE 256*1024

typedef struct _eo_tokenizer
{
   /* ragel vars */
   int cs;        /* current machine state */
   int act;       /* last pattern matched */
   char *ts;      /* current token match start */
   char *te;      /* current token match end */
   char *p;       /* data start */
   char *pe;      /* data end */
   char *eof;     /* eof = (EOF ? pe : NULL) */
   /* int stack[10]; /1* state stack used by fret fcall ... *1/ */
   /* int top;       /1* stack pointer *1/ */

   const char *source;
   int current_line;
   int current_nesting;
   int max_nesting;
   Eo_Method_Type current_methods_type;
   char buf[BUFSIZE];
   struct {
      char *tok;
      int line;
   } saved;

   Eina_List *classes;
   Eina_List *typedefs;
   struct {
      Eina_List **params;
      Eolian_Class_Type kls_type;
      const char *typedef_alias;
      Eo_Class_Def *kls;
      Eo_Type_Def *type_def;
      Eo_Property_Def *prop;
      Eo_Method_Def *meth;
      Eo_Param_Def *param;
      Eo_Accessor_Def *accessor;
      Eo_Accessor_Param *accessor_param;
      Eina_List *str_items;
      Eo_Event_Def *event;
      Eo_Implement_Def *impl;
      Eo_Implement_Legacy_Param_Def *impl_leg_param;
      int fscope;
   } tmp;

} Eo_Lexer;

int eo_tokenizer_init();

int eo_tokenizer_shutdown();

Eo_Lexer* eo_tokenizer_get(void);

Eina_Bool eo_tokenizer_walk(Eo_Lexer *toknz, const char *source);

void eo_tokenizer_dump(Eo_Lexer *toknz);

void eo_tokenizer_free(Eo_Lexer *toknz);

Eina_Bool eo_tokenizer_database_fill(const char *filename);

#endif /* __EO_TOKENIZER_H__ */
