#ifndef __EO_PARSER_H__
#define __EO_PARSER_H__

#include "eo_lexer.h"

Eolian_Unit *eo_parser_database_fill(Eolian_Unit *parent, const char *filename, Eina_Bool eot);

#endif /* __EO_PARSER_H__ */