#include "Eolian.h"
#include "eolian_database.h"
#include "eo_lexer.h"

EAPI Eina_Bool eolian_init()
{
   database_init();
   eo_tokenizer_init();
   return EINA_TRUE;
}

EAPI Eina_Bool eolian_shutdown()
{
   eo_tokenizer_shutdown();
   database_shutdown();
   return EINA_TRUE;
}

EAPI Eina_Bool eolian_eo_file_parse(const char *filename)
{
   return eo_tokenizer_database_fill(filename);
}

