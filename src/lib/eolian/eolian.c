#include "Eolian.h"
#include "eolian_database.h"
#include "eo_lexer.h"

static int _eolian_init_counter = 0;

EAPI int eolian_init(void)
{
   if (_eolian_init_counter > 0) return ++_eolian_init_counter;
   database_init();
   eo_tokenizer_init();
   return ++_eolian_init_counter;
}

EAPI int eolian_shutdown(void)
{
   if (_eolian_init_counter <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   _eolian_init_counter--;

   if (_eolian_init_counter == 0)
     {
        eo_tokenizer_shutdown();
        database_shutdown();
     }

   return _eolian_init_counter;
}

EAPI Eina_Bool eolian_eo_file_parse(const char *filename)
{
   return eo_tokenizer_database_fill(filename);
}

