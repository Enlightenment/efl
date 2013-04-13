#ifndef __ISF_IMF_CONTEXT_H
#define __ISF_IMF_CONTEXT_H

#include <Ecore_IMF.h>

typedef struct _EcoreIMFContextISF      EcoreIMFContextISF;
typedef struct _EcoreIMFContextISFImpl  EcoreIMFContextISFImpl;

struct _EcoreIMFContextISF {
    Ecore_IMF_Context *ctx;

    EcoreIMFContextISFImpl *impl;

    int id; /* Input Context id*/
    struct _EcoreIMFContextISF *next;
};

void isf_imf_context_add (Ecore_IMF_Context *ctx);
void isf_imf_context_del (Ecore_IMF_Context *ctx);
void isf_imf_context_client_window_set (Ecore_IMF_Context *ctx, void *window);
void isf_imf_context_client_canvas_set (Ecore_IMF_Context *ctx, void *window);
void isf_imf_context_focus_in (Ecore_IMF_Context *ctx);
void isf_imf_context_focus_out (Ecore_IMF_Context *ctx);
void isf_imf_context_reset (Ecore_IMF_Context *ctx);
void isf_imf_context_cursor_position_set (Ecore_IMF_Context *ctx, int cursor_pos);
void isf_imf_context_cursor_location_set (Ecore_IMF_Context *ctx, int x, int y, int w, int h);
void isf_imf_context_input_mode_set (Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode);
void isf_imf_context_preedit_string_get (Ecore_IMF_Context *ctx, char** str, int *cursor_pos);
void isf_imf_context_preedit_string_with_attributes_get (Ecore_IMF_Context *ctx, char** str, Eina_List **attrs, int *cursor_pos);
void isf_imf_context_use_preedit_set (Ecore_IMF_Context* ctx, Eina_Bool use_preedit);
Eina_Bool  isf_imf_context_filter_event (Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event);
void isf_imf_context_prediction_allow_set (Ecore_IMF_Context* ctx, Eina_Bool prediction);
void isf_imf_context_autocapital_type_set (Ecore_IMF_Context* ctx, Ecore_IMF_Autocapital_Type autocapital_type);
void isf_imf_context_input_panel_show(Ecore_IMF_Context *ctx);
void isf_imf_context_input_panel_hide(Ecore_IMF_Context *ctx);

EcoreIMFContextISF* isf_imf_context_new      (void);
void                isf_imf_context_shutdown (void);

#endif  /* __ISF_IMF_CONTEXT_H */

