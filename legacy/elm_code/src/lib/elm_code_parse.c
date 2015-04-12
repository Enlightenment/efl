#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

struct _Elm_Code_Parser
{
   void (*parse_line)(Elm_Code_Line *, void *);

   void (*parse_file)(Elm_Code_File *, void *);

   void *data;
};


void
_elm_code_parse_line(Elm_Code *code, Elm_Code_Line *line)
{
   Elm_Code_Parser *parser;
   Eina_List *item;

   elm_code_line_tokens_clear(line);

   EINA_LIST_FOREACH(code->parsers, item, parser)
     {
        if (parser->parse_line)
          parser->parse_line(line, parser->data);
     }
}

void
_elm_code_parse_file(Elm_Code *code, Elm_Code_File *file)
{
   Elm_Code_Parser *parser;
   Eina_List *item;
   Elm_Code_Line *line;

   EINA_LIST_FOREACH(file->lines, item, line)
     {
        elm_code_line_tokens_clear(line);
     }

   EINA_LIST_FOREACH(code->parsers, item, parser)
     {
        if (parser->parse_file)
          parser->parse_file(file, parser->data);
     }
}

static Elm_Code_Parser *
_elm_code_parser_new(void (*parse_line)(Elm_Code_Line *, void *),
                     void (*parse_file)(Elm_Code_File *, void *))
{
   Elm_Code_Parser *parser;

   parser = calloc(1, sizeof(Elm_Code_Parser));
   if (!parser)
     return NULL;

   parser->parse_line = parse_line;
   parser->parse_file = parse_file;

   return parser;
}

EAPI void
elm_code_parser_add(Elm_Code *code,
                    void (*parse_line)(Elm_Code_Line *, void *),
                    void (*parse_file)(Elm_Code_File *, void *), void *data)
{
   Elm_Code_Parser *parser;

   parser = _elm_code_parser_new(parse_line, parse_file);
   if (!parser)
     return;

   parser->data = data;

   code->parsers = eina_list_append(code->parsers, parser);
}

EAPI void
elm_code_parser_standard_add(Elm_Code *code, Elm_Code_Parser *parser)
{
   if (!parser || !code)
     return;

   code->parsers = eina_list_append(code->parsers, parser);
}

EAPI Elm_Code_Parser *
ELM_CODE_PARSER_STANDARD_DIFF;

static void
_elm_code_parser_diff_trim_leading(Elm_Code_Line *line, unsigned int count)
{
   char *replace, *old = NULL;

   if (line->modified)
     {
        old = line->modified;
        replace = malloc(sizeof(char) * (line->length - count));

        strncpy(replace, old + count, line->length - count);
        line->modified = replace;
        free(old);
     }
   else
     {
        line->content += count;
     }

   line->length -= count;
}

static void
_elm_code_parser_diff_parse_line(Elm_Code_Line *line, void *data EINA_UNUSED)
{
   const char *content;
   unsigned int length;

   content = elm_code_line_text_get(line, &length);
   if (length < 1)
     return;

   if (content[0] == 'd' || content[0] == 'i' || content[0] == 'n')
     {
        elm_code_line_status_set(line, ELM_CODE_STATUS_TYPE_CHANGED);
        return;
     }

   if (content[0] == '+')
     elm_code_line_status_set(line, ELM_CODE_STATUS_TYPE_ADDED);
   else if (content[0] == '-')
     elm_code_line_status_set(line, ELM_CODE_STATUS_TYPE_REMOVED);

   _elm_code_parser_diff_trim_leading(line, 1);
}

static void
_elm_code_parser_diff_parse_file(Elm_Code_File *file, void *data EINA_UNUSED)
{
   Eina_List *item;
   Elm_Code_Line *line;
   const char *content;
   unsigned int length, offset;

   offset = 0;
   EINA_LIST_FOREACH(file->lines, item, line)
     {
        content = elm_code_line_text_get(line, &length);

        if (length > 0 && (content[0] == 'd' || content[0] == 'i' || content[0] == 'n'))
          {
             offset = 0;
             continue;
          }

        if (offset <= 1 && (content[0] == '+' || content[0] == '-'))
          {
             elm_code_line_status_set(line, ELM_CODE_STATUS_TYPE_CHANGED);
             _elm_code_parser_diff_trim_leading(line, 3);
          }

        offset++;
     }
}

void
_elm_code_parse_setup()
{
   ELM_CODE_PARSER_STANDARD_DIFF = _elm_code_parser_new(_elm_code_parser_diff_parse_line,
                                                        _elm_code_parser_diff_parse_file);
}
