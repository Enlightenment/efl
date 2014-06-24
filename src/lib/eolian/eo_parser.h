#ifndef __EO_PARSER_H__
#define __EO_PARSER_H__

#include "eo_lexer.h"

void      eo_parser_dump         (Eo_Lexer *ls);
Eina_Bool eo_parser_walk         (Eo_Lexer *ls, Eina_Bool eot);
Eina_Bool eo_parser_database_fill(const char *filename, Eina_Bool eot);

#endif /* __EO_PARSER_H__ */