#ifndef __IBUS_IM_CONTEXT_H_
#define __IBUS_IM_CONTEXT_H_

#include <Ecore_IMF.h>

typedef struct _IBusIMContext IBusIMContext;

EAPI void ibus_im_context_add     (Ecore_IMF_Context *ctx);
EAPI void ibus_im_context_del     (Ecore_IMF_Context *ctx);
EAPI void ibus_im_context_reset   (Ecore_IMF_Context *context);
EAPI void ibus_im_context_focus_in(Ecore_IMF_Context *context);
EAPI void ibus_im_context_focus_out(Ecore_IMF_Context *context);
EAPI void ibus_im_context_preedit_string_get
                                            (Ecore_IMF_Context     *context,
                                             char                  **str,
                                             int                   *cursor_pos);
EAPI void ibus_im_context_preedit_string_with_attributes_get
                                            (Ecore_IMF_Context     *context,
                                             char                  **str,
                                             Eina_List             **attr,
                                             int                   *cursor_pos);

EAPI void ibus_im_context_cursor_location_set(Ecore_IMF_Context *context,
                                              int x, int y, int w, int h);
EAPI void ibus_im_context_use_preedit_set(Ecore_IMF_Context *context,
                                          Eina_Bool use_preedit);
EAPI void
ibus_im_context_client_window_set(Ecore_IMF_Context   *context, void *window);
EAPI void
ibus_im_context_client_canvas_set(Ecore_IMF_Context   *context, void *canvas);
EAPI Eina_Bool
ibus_im_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);

IBusIMContext
        *ibus_im_context_new      (void);
#endif
