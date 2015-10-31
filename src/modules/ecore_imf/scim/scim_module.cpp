#include <stdio.h>
#include "scim_imcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

   static const Ecore_IMF_Context_Info isf_imf_info = {
        "scim",                                 /* ID */
        "SCIM immodule for Ecore",              /* Description */
        "*",                                    /* Default locales */
        NULL,                                   /* Canvas type */
        0                                       /* Canvas required */
   };

   static Ecore_IMF_Context_Class isf_imf_class = {
        isf_imf_context_add,                    /* add */
        isf_imf_context_del,                    /* del */
        isf_imf_context_client_window_set,      /* client_window_set */
        isf_imf_context_client_canvas_set,      /* client_canvas_set */
        isf_imf_context_input_panel_show,       /* input_panel_show, - show */
        isf_imf_context_input_panel_hide,       /* input_panel_hide, - hide */
        isf_imf_context_preedit_string_get,     /* get_preedit_string */
        isf_imf_context_focus_in,               /* focus_in */
        isf_imf_context_focus_out,              /* focus_out */
        isf_imf_context_reset,                  /* reset */
        isf_imf_context_cursor_position_set,    /* cursor_position_set */
        isf_imf_context_use_preedit_set,        /* use_preedit_set */
        isf_imf_context_input_mode_set,         /* input_mode_set */
        isf_imf_context_filter_event,           /* filter_event */
        isf_imf_context_preedit_string_with_attributes_get,  /* preedit_string_with_attribute_get */
        isf_imf_context_prediction_allow_set,   /* prediction_allow_set */
        isf_imf_context_autocapital_type_set,   /* autocapital_type_set */
        NULL,                                   /* control panel show */
        NULL,                                   /* control panel hide */
        NULL,                                   /* input_panel_layout_set */
        NULL,                                   /* isf_imf_context_input_panel_layout_get, */
        NULL,                                   /* isf_imf_context_input_panel_language_set, */
        NULL,                                   /* isf_imf_context_input_panel_language_get, */
        isf_imf_context_cursor_location_set,    /* cursor_location_set */
        NULL,                                   /* input_panel_imdata_set */
        NULL,                                   /* input_panel_imdata_get */
        NULL,                                   /* input_panel_return_key_type_set */
        NULL,                                   /* input_panel_return_key_disabled_set */
        NULL,                                   /* input_panel_caps_lock_mode_set */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
   };

   static Ecore_IMF_Context *imf_module_create (void);
   static Ecore_IMF_Context *imf_module_exit (void);

   static Eina_Bool imf_module_init (void)
     {
        const char *s;

        if (!getenv("DISPLAY")) return EINA_FALSE;
        if ((s = getenv("ELM_DISPLAY")))
          {
             if (strcmp(s, "x11")) return EINA_FALSE;
          }
        ecore_imf_module_register (&isf_imf_info, imf_module_create, imf_module_exit);
        return EINA_TRUE;
     }

   static void imf_module_shutdown (void)
     {
        isf_imf_context_shutdown ();
     }

   static Ecore_IMF_Context *imf_module_create (void)
     {
        Ecore_IMF_Context  *ctx = NULL;
        EcoreIMFContextISF *ctxd = NULL;

        ctxd = isf_imf_context_new ();
        if (!ctxd)
          {
             printf ("isf_imf_context_new () failed!!!\n");
             return NULL;
          }

        ctx = ecore_imf_context_new (&isf_imf_class);
        if (!ctx)
          {
             delete ctxd;
             return NULL;
          }

        ecore_imf_context_data_set (ctx, ctxd);

        return ctx;
     }

   static Ecore_IMF_Context *imf_module_exit (void)
     {
        return NULL;
     }

   EINA_MODULE_INIT(imf_module_init);
   EINA_MODULE_SHUTDOWN(imf_module_shutdown);

#ifdef __cplusplus
}
#endif /* __cplusplus */

