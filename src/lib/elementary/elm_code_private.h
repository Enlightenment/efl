#ifndef ELM_CODE_PRIVATE_H
# define ELM_CODE_PRIVATE_H

#include "elm_priv.h"

Eina_Bool _elm_code_text_char_is_whitespace(char c);

/* Private parser callbacks */

void _elm_code_parse_setup();

void _elm_code_parse_line(Elm_Code *code, Elm_Code_Line *line);

void _elm_code_parse_file(Elm_Code *code, Elm_Code_File *file);

void _elm_code_parse_reset_file(Elm_Code *code, Elm_Code_File *file);

void _elm_code_parser_free(Elm_Code_Parser *parser);

#endif
