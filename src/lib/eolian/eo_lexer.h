#ifndef __EO_LEXER_H__
#define __EO_LEXER_H__

#include <setjmp.h>

#include <Eina.h>
#include <Eolian.h>
#include "eo_definitions.h"

#define START_CUSTOM 257

enum Tokens
{
   TOK_COMMENT = START_CUSTOM, TOK_EOF, TOK_VALUE
};

#define KEYWORDS KW(class), KW(const), KW(private), KW(protected), \
    KW(return), KW(signed), KW(struct), KW(unsigned), KW(virtual), \
    \
    KW(abstract), KW(constructor), KW(constructors), KW(data), \
    KW(destructor), KW(eo_prefix), KW(events), KW(func), KW(get), \
    KW(implements), KW(interface), KW(keys), KW(legacy), KW(legacy_prefix), \
    KW(methods), KW(mixin), KW(own), KW(params), KW(properties), KW(set), \
    KW(type), KW(values), KWAT(in), KWAT(inout), KWAT(nonull), KWAT(out), \
    KWAT(warn_unused), \
    \
    KW(char), KW(uchar), KW(schar), KW(short), KW(ushort), KW(int), KW(uint), \
    KW(long), KW(ulong), KW(llong), KW(ullong), \
    \
    KW(int8), KW(uint8), KW(int16), KW(uint16), KW(int32), KW(uint32), \
    KW(int64), KW(uint64), KW(int128), KW(uint128), \
    \
    KW(float), KW(double), KW(ldouble), \
    \
    KW(void)

#define KW(x) KW_##x
#define KWAT(x) KW_at_##x

enum Keywords
{
   KW_UNKNOWN = 0,
   KEYWORDS
};

#undef KW
#undef KWAT

typedef struct _Eo_Token
{
   int         token;
   const char *value;
   int         kw;
} Eo_Token;

enum Nodes
{
   NODE_CLASS = 0,
   NODE_TYPEDEF
};

typedef struct _Eo_Node
{
   unsigned char type;
   union {
      void           *def;
      Eo_Class_Def   *def_class;
      Eo_Typedef_Def *def_typedef;
   };
} Eo_Node;

typedef struct _Eo_Lexer
{
   int          current;
   int          column;
   int          line_number;
   Eo_Token     t, lookahead;
   Eina_Strbuf *buff;
   Eina_File   *handle;
   const char  *source;
   const char  *stream;
   const char  *stream_end;
   jmp_buf      err_jmp;

   Eina_List      *nodes;
   Eo_Lexer_Temps  tmp;
} Eo_Lexer;

int         eo_lexer_init           (void);
int         eo_lexer_shutdown       (void);
Eo_Lexer   *eo_lexer_new            (const char *source);
void        eo_lexer_free           (Eo_Lexer *ls);
int         eo_lexer_get_balanced   (Eo_Lexer *ls, char beg, char end);
int         eo_lexer_get_until      (Eo_Lexer *ls, char end);
int         eo_lexer_get            (Eo_Lexer *ls);
int         eo_lexer_get_ident      (Eo_Lexer *ls, const char *chars);
int         eo_lexer_lookahead      (Eo_Lexer *ls);
int         eo_lexer_lookahead_ident(Eo_Lexer *ls, const char *chars);
void        eo_lexer_lex_error      (Eo_Lexer *ls, const char *msg, int token);
void        eo_lexer_syntax_error   (Eo_Lexer *ls, const char *msg);
void        eo_lexer_token_to_str   (int token, char *buf);
const char *eo_lexer_keyword_str_get(int kw);
Eina_Bool   eo_lexer_is_type_keyword(int kw);
const char *eo_lexer_get_c_type     (int kw);

extern int _eo_lexer_log_dom;
#ifdef CRITICAL
#undef CRITICAL
#endif
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_eo_lexer_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eo_lexer_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eo_lexer_log_dom, __VA_ARGS__)

#define INF_ENABLED EINA_FALSE
#ifdef INF
#undef INF
#endif
#define INF(...) if (INF_ENABLED) EINA_LOG_DOM_INFO(_eo_lexer_log_dom, __VA_ARGS__)

#define DBG_ENABLED EINA_FALSE
#ifdef DBG
#undef DBG
#endif
#define DBG(...) if (DBG_ENABLED) EINA_LOG_DOM_DBG(_eo_lexer_log_dom, __VA_ARGS__)


#endif /* __EO_LEXER_H__ */