#include "eo_parser.h"
#include "eolian_database.h"

static int _eolian_init_counter = 0;
int _eolian_log_dom = -1;
Eina_Prefix *_eolian_prefix = NULL;

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

   _eolian_prefix = eina_prefix_new(NULL, eolian_init, "EOLIAN", "eolian",
                                    NULL, "", "", PACKAGE_DATA_DIR, "");
   if (!_eolian_prefix)
     {
        ERR("Could not initialize the Eolian prefix.");
        return EINA_FALSE;
     }

   database_init();
   eo_lexer_init();
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

        eo_lexer_shutdown();
        database_shutdown();
        eina_prefix_free(_eolian_prefix);
        _eolian_prefix = NULL;

        eina_log_domain_unregister(_eolian_log_dom);
        _eolian_log_dom = -1;
        eina_shutdown();
     }

   return _eolian_init_counter;
}

