#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"
#include "elm_code_parse.h"

#include "elm_code_private.h"

EAPI void elm_code_parse_line(Elm_Code *code, Elm_Code_Line *line)
{
   Elm_Code_Parser *parser;
   Eina_List *item;

   EINA_LIST_FOREACH(code->parsers, item, parser)
     {
        parser->parse_line(line);
     }
}

EAPI void elm_code_parse_file(Elm_Code *code, Elm_Code_File *file)
{
   Elm_Code_Parser *parser;
   Eina_List *item;

   EINA_LIST_FOREACH(code->parsers, item, parser)
     {
        parser->parse_file(file);
     }
}

EAPI void elm_code_parser_add(Elm_Code *code,
                              void (*parse_line)(Elm_Code_Line *),
                              void (*parse_file)(Elm_Code_File *))
{
   Elm_Code_Parser *parser;

   parser = calloc(1, sizeof(Elm_Code_Parser));
   if (!parser)
     return;

   parser->parse_line = parse_line;
   parser->parse_file = parse_file;

   code->parsers = eina_list_append(code->parsers, parser);
}

