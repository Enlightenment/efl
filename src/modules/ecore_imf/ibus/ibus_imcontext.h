#ifndef __IBUS_IM_CONTEXT_H_
#define __IBUS_IM_CONTEXT_H_

#include <Ecore_IMF.h>

typedef struct _IBusIMContext IBusIMContext;

void ecore_imf_context_ibus_add(Ecore_IMF_Context *ctx);

void ecore_imf_context_ibus_del(Ecore_IMF_Context *ctx);

void ecore_imf_context_ibus_reset(Ecore_IMF_Context *context);

void ecore_imf_context_ibus_focus_in(Ecore_IMF_Context *context);

void ecore_imf_context_ibus_focus_out(Ecore_IMF_Context *context);

void ecore_imf_context_ibus_preedit_string_get(Ecore_IMF_Context     *context,
                                               char                  **str,
                                               int                   *cursor_pos);

void ecore_imf_context_ibus_preedit_string_with_attributes_get(Ecore_IMF_Context     *context,
                                                               char                  **str,
                                                               Eina_List             **attrs,
                                                               int                   *cursor_pos);

void ecore_imf_context_ibus_cursor_location_set(Ecore_IMF_Context *context,
                                                int x, int y, int w, int h);

void ecore_imf_context_ibus_use_preedit_set(Ecore_IMF_Context *context,
                                            Eina_Bool use_preedit);

void ecore_imf_context_ibus_client_window_set(Ecore_IMF_Context *context, void *window);

void ecore_imf_context_ibus_client_canvas_set(Ecore_IMF_Context *context, void *canvas);

Eina_Bool ecore_imf_context_ibus_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);

IBusIMContext *ecore_imf_context_ibus_new(void);
void ecore_imf_context_ibus_shutdown(void);
#endif
