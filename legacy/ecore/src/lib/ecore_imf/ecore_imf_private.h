#ifndef _ECORE_IMF_PRIVATE_H
#define _ECORE_IMF_PRIVATE_H

#define ECORE_MAGIC_CONTEXT 0x56c1b39a

#ifdef ECORE_IMF_DEFAULT_LOG_COLOR
#undef ECORE_IMF_DEFAULT_LOG_COLOR
#endif
#define ECORE_IMF_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

extern int _ecore_imf_log_dom;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_imf_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_imf_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_imf_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_imf_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_imf_log_dom, __VA_ARGS__)

typedef struct _Ecore_IMF_Module Ecore_IMF_Module;

struct _Ecore_IMF_Context
{
   ECORE_MAGIC;
   const Ecore_IMF_Module        *module;
   const Ecore_IMF_Context_Class *klass;
   void                          *data;
   int                            input_mode;
   void                          *window;
   void                          *client_canvas;
   Eina_Bool                    (*retrieve_surrounding_func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos);
   void                          *retrieve_surrounding_data;
   Ecore_IMF_Autocapital_Type     autocapital_type;
   Ecore_IMF_Input_Panel_Layout   input_panel_layout;
   Ecore_IMF_Input_Panel_Lang     input_panel_lang;
   Eina_Bool                      allow_prediction : 1;
   Eina_Bool                      input_panel_enabled : 1;
};

struct _Ecore_IMF_Module
{
   const Ecore_IMF_Context_Info *info;
   Ecore_IMF_Context            *(*create)(void);
   Ecore_IMF_Context            *(*exit)(void);
};

void               ecore_imf_module_init(void);
void               ecore_imf_module_shutdown(void);
Eina_List         *ecore_imf_module_available_get(void);
Ecore_IMF_Module  *ecore_imf_module_get(const char *ctx_id);
Ecore_IMF_Context *ecore_imf_module_context_create(const char *ctx_id);
Eina_List         *ecore_imf_module_context_ids_get(void);
Eina_List         *ecore_imf_module_context_ids_by_canvas_type_get(const char *canvas_type);

#endif
