#include "Eolian.h"
#include "eolian_database.h"
#include "eo_lexer.h"

static int _eolian_init_counter = 0;
int _eolian_log_dom = -1;

EAPI int eolian_init(void)
{
   const char *log_dom = "eolian";
   if (_eolian_init_counter > 0) return ++_eolian_init_counter;

   eina_init();
   _eolian_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_LIGHTBLUE);
   if (_eolian_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        return EINA_FALSE;
     }

   eina_log_timing(_eolian_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   INF("Init");
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
        INF("Shutdown");
        eina_log_timing(_eolian_log_dom,
              EINA_LOG_STATE_START,
              EINA_LOG_STATE_SHUTDOWN);

        eo_tokenizer_shutdown();
        database_shutdown();

        eina_log_domain_unregister(_eolian_log_dom);
        _eolian_log_dom = -1;
        eina_shutdown();
     }

   return _eolian_init_counter;
}

EAPI Eina_Bool eolian_eo_file_parse(const char *filename)
{
   return eo_tokenizer_database_fill(filename);
}

