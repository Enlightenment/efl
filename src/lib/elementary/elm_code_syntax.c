#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Eina.h>
#include "Elementary.h"

#include "elm_code_private.h"

typedef struct _Elm_Code_Syntax
{
   const char *symbols;
   const char *numparts;
   const char *preprocessor;
   const char *comment_single;
   const char *comment_start;
   const char *comment_end;
   int (*scope_change)(Elm_Code_Line *line);
   const char *keywords[];
} Elm_Code_Syntax;

static int
_elm_code_syntax_scope_change_braces(Elm_Code_Line *line)
{
   unsigned int length, i;
   const char *content;
   int change = 0;

   content = elm_code_line_text_get(line, &length);

   for (i = 0; i < length; i++)
     {
        if (*(content + i) == '{')
          change++;
        else if (*(content + i) == '}')
          change--;
     }

   return change;
}

static Elm_Code_Syntax _elm_code_syntax_c =
{
   "{}()[]:;%^/*+&|~!=<->,.",
   ".",
   "#",
   "//",
   "/*",
   "*/",
   _elm_code_syntax_scope_change_braces,
   {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",  "enum", "extern", \
      "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", \
      "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", NULL}
};

static Elm_Code_Syntax _elm_code_syntax_rust =
{
   "-*!&+/%|^<=>:;.,{}()[]",
   "._",
   "#",
   "//",
   NULL,
   NULL,
   _elm_code_syntax_scope_change_braces,
   {"as", "break", "const", "continue", "create", "else",  "enum", "extern", "false", "fn", "for", "if", \
      "impl", "in", "let", "loop", "match", "mod", "move", "mut", "pub", "ref", "return", "Self", "self", \
      "static", "struct", "super", "trait", "true", "type", "unsafe", "use", "where", "while",
      "bool", "i8", "i16", "i32", "i64", "isize", "u8", "u16", "u32", "u64", "usize", "f32", "f64", "char", NULL}
};

static Elm_Code_Syntax _elm_code_syntax_py =
{
   "{}()[]:;%/*+!=<->,.",
   ".",
   NULL,
   "#",
   "\"\"\"",
   "\"\"\"",
   NULL,
   {"False", "None", "True", "and", "as", "assert", "break", "class", \
    "continue", "def", "del", "elif", "else", "except", "finally", "for", \
    "from", "global", "if", "import", "in", "is", "lambda", "nonlocal", "not", \
    "or", "pass", "raise", "return", "try", "while", "with", "yield"}
};

static Elm_Code_Syntax _elm_code_syntax_eo =
{
   "{}():;*,.",
   ".",
   NULL,
   "//",
   "[[",
   "]]",
   _elm_code_syntax_scope_change_braces,
   {"byte", "ubyte", "char", "short", "ushort", "int", "uint", "long", "ulong", \
    "llong", "ullong", "int8", "uint8", "int16", "uint16", "int32", "uint32", \
    "int64", "uint64", "int128", "uint128", "size", "ssize", "intptr", "uintptr", \
    "ptrdiff", "time", "float", "double", "bool", "void", "void_ptr", \
    "string", "stringshare", "any_value", \
    "abstract", "class", "data", "mixin", "import", "interface", "type", "const", "var", \
    "own", "free", "struct", "enum", "@extern", "@free", "@auto", "@empty", \
    "@private", "@protected", "@beta", "@hot", "@const", "@class", "@pure_virtual", \
    "@property", "@nonull", "@nullable", "@optional", "@in", "@out", "@inout", "@warn_unused", \
    "eo_prefix", "legacy_prefix", "methods", "events", "params", "return", "legacy", \
    "implements", "constructors", "get", "set", "keys", "values", "true", "false", "null"}
};

static Elm_Code_Syntax _elm_code_syntax_go =
{
   "{}()[]:;%^/*+&|~!=<->,.",
   ".",
   NULL,
   "//",
   "/*",
   "*/",
   _elm_code_syntax_scope_change_braces,
   { "break", "case", "chan", "const", "continue", "default", "defer", "else", "fallthrough", "for", "func", "go", "goto",  \
     "if", "import", "interface", "map", "package", "range", "return", "select", "struct", "switch", "type", "var", \
     "true", "false", "iota", "nil", \
     "int", "int8", "int16", "int32", "int64", "uint", "uint8", "uint16", "uint32", "uint64", "uintptr", "float32", \
     "float64", "complex64", "complex128", "bool", "byte", "rune", "string", "error", "make", "len", "cap", "new", "append", \
     "copy", "close", "delete", "complex", "real", "imag", "panic", "recover", NULL }
};

static Elm_Code_Syntax _elm_code_syntax_md =
{
   "()[]*+-_=#.>!:\\`~|",
   "",
   NULL,
   NULL,
   "<!--",
   "-->",
   NULL,
   {}
};

EAPI Elm_Code_Syntax *
elm_code_syntax_for_mime_get(const char *mime)
{
   if (!mime) return NULL;

   if (!strcmp("text/x-chdr", mime) || !strcmp("text/x-csrc", mime))
     return &_elm_code_syntax_c;
   if (!strcmp("text/rust", mime))
     return &_elm_code_syntax_rust;
   if (!strcmp("text/x-python", mime))
     return &_elm_code_syntax_py;
   if (!strcmp("text/x-eolian", mime))
     return &_elm_code_syntax_eo;
   if (!strcmp("text/markdown", mime))
     return &_elm_code_syntax_md;
   if (!strcmp("text/x-go", mime))
     return &_elm_code_syntax_go;

   return NULL;
}

static Eina_Bool
_char_is_number(char c, Elm_Code_Syntax *syntax)
{
   const char *sym;

   if (isdigit(c))
     return EINA_TRUE;

   for (sym = syntax->numparts; *sym; sym++)
     if (c == *sym)
       return EINA_TRUE;

   return EINA_FALSE;
}

static void
_elm_code_syntax_parse_token(Elm_Code_Syntax *syntax, Elm_Code_Line *line, unsigned int pos, const char *token, unsigned int length)
{
   const char **keyword;
   unsigned int i;

   for (keyword = syntax->keywords; *keyword; keyword++)
     if (strlen(*keyword) == length && !strncmp(token, *keyword, length))
       {
          elm_code_line_token_add(line, pos, pos + length - 1, 1, ELM_CODE_TOKEN_TYPE_KEYWORD);
          return;
       }

   for (i = 0; i < length; i++)
     {
        if (!_char_is_number(token[i], syntax))
          break;
        if (i == length - 1)
          elm_code_line_token_add(line, pos, pos + length - 1, 1, ELM_CODE_TOKEN_TYPE_NUMBER);
     }
}

static Eina_Bool
_content_starts_with(const char *content, const char *prefix, unsigned int length)
{
   unsigned int i;
   unsigned int prefix_length;

   if (!prefix)
     return EINA_FALSE;
   prefix_length = strlen(prefix);
   if (!content || length < prefix_length)
     return EINA_FALSE;

   for (i = 0; i < prefix_length; i++)
     if (content[i] != prefix[i])
       return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_starts_single_comment(Elm_Code_Syntax *syntax, const char *content, unsigned int length)
{
   return _content_starts_with(content, syntax->comment_single, length);
}

static Eina_Bool
_starts_comment(Elm_Code_Syntax *syntax, const char *content, unsigned int length)
{
   return _content_starts_with(content, syntax->comment_start, length);
}

static Eina_Bool
_ends_comment(Elm_Code_Syntax *syntax, const char *content, unsigned int length)
{
   return _content_starts_with(content, syntax->comment_end, length);
}

static Elm_Code_Token_Type
_previous_line_continue_type(Elm_Code_Line *line)
{
   Elm_Code_Line *prev;
   Elm_Code_Token *token;
   Eina_List *item;

   if (line->number < 2)
     return ELM_CODE_TOKEN_TYPE_DEFAULT;

   prev = elm_code_file_line_get(line->file, line->number - 1);
   if (!prev || !prev->tokens)
     return ELM_CODE_TOKEN_TYPE_DEFAULT;

   EINA_LIST_FOREACH(prev->tokens, item, token)
     if (token->continues)
       return token->type;

   return ELM_CODE_TOKEN_TYPE_DEFAULT;
}

unsigned int
_previous_line_scope(Elm_Code_Line *line)
{
   Elm_Code_Line *prev;

   if (line->number < 2)
     return 0;

   prev = elm_code_file_line_get(line->file, line->number - 1);
   if (!prev)
     return 0;

   return prev->scope;
}

EAPI void
elm_code_syntax_parse_line(Elm_Code_Syntax *syntax, Elm_Code_Line *line)
{
   unsigned int i, i2, count, length;
   const char *content;
   const char *sym, *ptr;
   Elm_Code_Token_Type previous_type;

   EINA_SAFETY_ON_NULL_RETURN(syntax);
   line->scope = _previous_line_scope(line) + _elm_code_syntax_scope_change_braces(line);

   i = 0;
   content = elm_code_line_text_get(line, &length);
   previous_type = _previous_line_continue_type(line);
   if (previous_type == ELM_CODE_TOKEN_TYPE_COMMENT)
     {
        for (i2 = i; i2 < length; i2++)
          if (_ends_comment(syntax, content + i2, length - i2))
             {
                i2 += strlen(syntax->comment_end) - 1;
                break;
             }

        elm_code_line_token_add(line, 0, i2, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
        if (i2 == length)
          {
             Elm_Code_Token *token = eina_list_last_data_get(line->tokens);
             token->continues = EINA_TRUE;
             return;
          }
        i = i2 + 1;
     }
   else if (previous_type == ELM_CODE_TOKEN_TYPE_PREPROCESSOR)
     {
        elm_code_line_token_add(line, 0, length, 1, ELM_CODE_TOKEN_TYPE_PREPROCESSOR);
        if (length >= 1 && content[length-1] == '\\')
          {
             Elm_Code_Token *token = eina_list_last_data_get(line->tokens);
             token->continues = EINA_TRUE;
          }
        return;
     }

   ptr = content;
   count = 0;
   for (; i < length; i++)
     {
        ptr = content + i - count;
        if (_elm_code_text_char_is_whitespace(content[i]))
          {
             if (count)
               _elm_code_syntax_parse_token(syntax, line, ptr-content, ptr, count);

             count = 0;
             continue;
          }

        if (syntax->preprocessor && _content_starts_with(content+i, syntax->preprocessor, strlen(syntax->preprocessor)))
          {
             elm_code_line_token_add(line, i, length - 1, 1, ELM_CODE_TOKEN_TYPE_PREPROCESSOR);
             if (content[length-1] == '\\')
               {
                  Elm_Code_Token *token = eina_list_last_data_get(line->tokens);
                  token->continues = EINA_TRUE;
               }
             return;
          }
        else if (_starts_single_comment(syntax, content + i, length - i))
          {
             elm_code_line_token_add(line, i, length, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
             return;
          }
        else if (_starts_comment(syntax, content + i, length - i))
          {
             for (i2 = i+strlen(syntax->comment_start); i2 < length; i2++)
               if (_ends_comment(syntax, content + i2, length - i2))
                 {
                    i2 += strlen(syntax->comment_end) - 1;
                    break;
                 }

             elm_code_line_token_add(line, i, i2, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
             if (i2 == length)
               {
                  Elm_Code_Token *token = eina_list_last_data_get(line->tokens);
                  token->continues = EINA_TRUE;
                  // TODO reset all below of here
                  return;
               }
             i = i2;
             count = 0;
             continue;
          }
        else if (content[i] == '"')
          {
             unsigned int start = i, end;

             for (i++; i < length && (content[i] != '"' || (content[i-1] == '\\' && content[i-2] != '\\')); i++) {}
             end = i;

             elm_code_line_token_add(line, start, end, 1, ELM_CODE_TOKEN_TYPE_STRING);
             count = 0;
             continue;
          }
        else if (content[i] == '\'')
          {
             unsigned int start = i, end;

             for (i++; i < length && (content[i] != '\'' || (content[i-1] == '\\' && content[i-2] != '\\')); i++) {}
             end = i;

             elm_code_line_token_add(line, start, end, 1, ELM_CODE_TOKEN_TYPE_STRING);
             count = 0;
             continue;
         }

        for (sym = syntax->symbols; *sym; sym++)
          if (content[i] == *sym)
            {
               if (count)
                 _elm_code_syntax_parse_token(syntax, line, ptr-content, ptr, count);

               elm_code_line_token_add(line, i, i, 1, ELM_CODE_TOKEN_TYPE_BRACE);

               count = -1;
               break;
             }

       count++;
   }

   if (count)
     _elm_code_syntax_parse_token(syntax, line, ptr-content, ptr, count);
}

EAPI void
elm_code_syntax_parse_file(Elm_Code_Syntax *syntax, Elm_Code_File *file EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN(syntax);
}

