#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <ecore_private.h>

#include "Ecore_IMF.h"
#include "ecore_imf_private.h"

EAPI int ECORE_IMF_EVENT_PREEDIT_START = 0;
EAPI int ECORE_IMF_EVENT_PREEDIT_END = 0;
EAPI int ECORE_IMF_EVENT_PREEDIT_CHANGED = 0;
EAPI int ECORE_IMF_EVENT_COMMIT = 0;
EAPI int ECORE_IMF_EVENT_DELETE_SURROUNDING = 0;

int _ecore_imf_log_dom = -1;
static int _ecore_imf_init_count = 0;
extern Ecore_IMF_Context *show_req_ctx;

EAPI int
ecore_imf_init(void)
{
   if (++_ecore_imf_init_count != 1) return _ecore_imf_init_count;

   if (!ecore_init()) return --_ecore_imf_init_count;
   _ecore_imf_log_dom = eina_log_domain_register
      ("ecore_imf", ECORE_IMF_DEFAULT_LOG_COLOR);
   if (_ecore_imf_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the Ecore IMF module.");
        ecore_shutdown();
        return --_ecore_imf_init_count;
     }
   ecore_imf_module_init();

   ECORE_IMF_EVENT_PREEDIT_START = ecore_event_type_new();
   ECORE_IMF_EVENT_PREEDIT_END = ecore_event_type_new();
   ECORE_IMF_EVENT_PREEDIT_CHANGED = ecore_event_type_new();
   ECORE_IMF_EVENT_COMMIT = ecore_event_type_new();
   ECORE_IMF_EVENT_DELETE_SURROUNDING = ecore_event_type_new();

   return _ecore_imf_init_count;
}

EAPI int
ecore_imf_shutdown(void)
{
   if (--_ecore_imf_init_count != 0) return _ecore_imf_init_count;

   ecore_event_type_flush(ECORE_IMF_EVENT_PREEDIT_START,
                          ECORE_IMF_EVENT_PREEDIT_END,
                          ECORE_IMF_EVENT_PREEDIT_CHANGED,
                          ECORE_IMF_EVENT_COMMIT,
                          ECORE_IMF_EVENT_DELETE_SURROUNDING);

   ecore_imf_module_shutdown();
   eina_log_domain_unregister(_ecore_imf_log_dom);
   _ecore_imf_log_dom = -1;
   ecore_shutdown();
   return _ecore_imf_init_count;
}

EAPI Eina_Bool
ecore_imf_input_panel_hide(void)
{
   if (show_req_ctx)
     {
        if (ecore_imf_context_input_panel_state_get(show_req_ctx) != ECORE_IMF_INPUT_PANEL_STATE_HIDE)
          {
             ecore_imf_context_input_panel_hide(show_req_ctx);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}
