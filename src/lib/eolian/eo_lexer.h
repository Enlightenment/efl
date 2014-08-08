#ifndef __EO_LEXER_H__
#define __EO_LEXER_H__

#include <setjmp.h>

#include <Eina.h>
#include <Eolian.h>
#include "eo_definitions.h"

/* a token is an int, custom tokens start at this - single-char tokens are
 * simply represented by their ascii */
#define START_CUSTOM 257

enum Tokens
{
   TOK_EQ = START_CUSTOM, TOK_NQ, TOK_GE, TOK_LE,
   TOK_AND, TOK_OR, TOK_LSH, TOK_RSH,

   TOK_COMMENT, TOK_STRING, TOK_CHAR, TOK_NUMBER, TOK_VALUE
};

/* all keywords in eolian, they can still be used as names (they're TOK_VALUE)
 * they just fill in the "kw" field of the token */
#define KEYWORDS KW(class), KW(const), KW(private), KW(return), KW(struct), \
    KW(virtual), \
    \
    KW(abstract), KW(constructor), KW(constructors), KW(data), \
    KW(destructor), KW(eo_prefix), KW(events), KW(func), KW(get), \
    KW(implements), KW(interface), KW(keys), KW(legacy), KW(legacy_prefix), \
    KW(methods), KW(mixin), KW(own), KW(params), KW(properties), KW(set), \
    KW(type), KW(values), KWAT(class), KWAT(const), KWAT(constructor), \
    KWAT(extern), KWAT(in), KWAT(inout), KWAT(nonull), KWAT(out), \
    KWAT(protected), KWAT(warn_unused), \
    \
    KW(byte), KW(ubyte), KW(char), KW(short), KW(ushort), KW(int), KW(uint), \
    KW(long), KW(ulong), KW(llong), KW(ullong), \
    \
    KW(int8), KW(uint8), KW(int16), KW(uint16), KW(int32), KW(uint32), \
    KW(int64), KW(uint64), KW(int128), KW(uint128), \
    \
    KW(size), KW(ssize), KW(intptr), KW(uintptr), KW(ptrdiff), \
    \
    KW(time), \
    \
    KW(float), KW(double), KW(ldouble), \
    \
    KW(bool), \
    \
    KW(void), \
    \
    KW(true), KW(false), KW(null)

/* "regular" keyword and @ prefixed keyword */
#define KW(x) KW_##x
#define KWAT(x) KW_at_##x

enum Keywords
{
   KW_UNKNOWN = 0,
   KEYWORDS
};

#undef KW
#undef KWAT

enum Numbers
{
   NUM_INT,
   NUM_UINT,
   NUM_LONG,
   NUM_ULONG,
   NUM_LLONG,
   NUM_ULLONG,
   NUM_FLOAT,
   NUM_DOUBLE,
   NUM_LDOUBLE
};

/* a token - "token" is the actual token id, "value" is the value of a token
 * if needed - NULL otherwise - for example the value of a TOK_VALUE, "kw"
 * is the keyword id if this is a keyword, it's 0 when not a keyword */
typedef struct _Eo_Token
{
   int token, kw;
   Eolian_Value value;
} Eo_Token;

enum Nodes
{
   NODE_CLASS = 0
};

/* represents a node, aka a result of parsing - currently class, typedef
 * or struct, they're all stored in a list in lexer state and their type
 * is determined by enum Nodes above */
typedef struct _Eo_Node
{
   unsigned char type;
   union {
      void         *def;
      Eo_Class_Def *def_class;
   };
} Eo_Node;

typedef struct _Lexer_Ctx
{
   int line, column;
   const char *linestr;
} Lexer_Ctx;

/* keeps all lexer state */
typedef struct _Eo_Lexer
{
   /* current character being tested */
   int          current;
   /* column is token aware column number, for example when lexing a keyword
    * it points to the beginning of it after the lexing is done, icolumn is
    * token unaware, always pointing to current column */
   int          column, icolumn;
   /* the current line number */
   int          line_number;
   /* t: "normal" - token to lex into, "lookahead" - a lookahead token, used
    * to look one token past "t", when we need to check for a token after the
    * current one and use it in a conditional without consuming the current
    * token - used in pretty few cases - because we have one extra lookahead
    * token, that makes our grammar LL(2) - two tokens in total */
   Eo_Token     t, lookahead;
   /* a string buffer used to keep contents of token currently being read,
    * if needed at all */
   Eina_Strbuf *buff;
   /* a handle pointing to a memory mapped file representing the file we're
    * currently lexing */
   Eina_File   *handle;
   /* the source file name */
   const char  *source;
   /* points to the current character in our mmapped file being lexed, just
    * incremented until the end */
   const char  *stream;
   /* end pointer - required to check if we've reached past the file, as
    * mmapped data will give us no EOF */
   const char  *stream_end;
   /* points to the current line being lexed, used by error messages to
    * display the current line with a caret at the respective column */
   const char  *stream_line;
   /* this is jumped to when an error happens */
   jmp_buf      err_jmp;

   /* whether we allow lexing expression related tokens */
   Eina_Bool expr_mode;

   /* saved context info */
   Eina_List *saved_ctxs;

   /* represents the results of parsing */
   Eina_List      *nodes;
   /* represents the temporaries, every object that is allocated by the
    * parser is temporarily put here so the resources can be reclaimed in
    * case of error - and it's nulled when it's written into a more permanent
    * position (e.g. as part of another struct, or into nodes */
   Eo_Lexer_Temps  tmp;
} Eo_Lexer;

int         eo_lexer_init           (void);
int         eo_lexer_shutdown       (void);
Eo_Lexer   *eo_lexer_new            (const char *source);
void        eo_lexer_free           (Eo_Lexer *ls);
/* gets a TOK_VALUE balanced token, aka keeps lexing everything until the
 * "end" character, but keeps it balanced, so if it hits "beg" during lexing,
 * it won't end at the next "end", useful for lexing between () or [] */
int         eo_lexer_get_balanced   (Eo_Lexer *ls, char beg, char end);
/* gets a regular token, singlechar or one of TOK_something */
int         eo_lexer_get            (Eo_Lexer *ls);
/* lookahead token - see Eo_Lexer */
int         eo_lexer_lookahead      (Eo_Lexer *ls);
/* "throws" an error, with a custom message and custom token */
void        eo_lexer_lex_error      (Eo_Lexer *ls, const char *msg, int token);
/* like above, but uses the lexstate->t.token, aka current token */
void        eo_lexer_syntax_error   (Eo_Lexer *ls, const char *msg);
/* turns the token into a string, writes into the given buffer */
void        eo_lexer_token_to_str   (int token, char *buf);
/* returns the string representation of a keyword */
const char *eo_lexer_keyword_str_get(int kw);
/* checks if the given keyword is a builtin type */
Eina_Bool   eo_lexer_is_type_keyword(int kw);
/* gets a keyword id from the keyword string */
int         eo_lexer_keyword_str_to_id(const char *kw);
/* gets the C type name for a builtin type name - e.g. uchar -> unsigned char */
const char *eo_lexer_get_c_type     (int kw);
/* save, restore and clear context (line, column, line string) */
void eo_lexer_context_push   (Eo_Lexer *ls);
void eo_lexer_context_pop    (Eo_Lexer *ls);
void eo_lexer_context_restore(Eo_Lexer *ls);
void eo_lexer_context_clear  (Eo_Lexer *ls);

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