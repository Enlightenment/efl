#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <Ecore.h>
#include <ecore_private.h>

#include "Ecore_IMF.h"
#include "ecore_imf_private.h"

Ecore_IMF_Context *show_req_ctx = NULL;

EAPI Eina_List *
ecore_imf_context_available_ids_get(void)
{
   return ecore_imf_module_context_ids_get();
}

EAPI Eina_List *
ecore_imf_context_available_ids_by_canvas_type_get(const char *canvas_type)
{
   return ecore_imf_module_context_ids_by_canvas_type_get(canvas_type);
}

/*
 * Match @locale against @against.
 *
 * 'en_US' against 'en_US'       => 4
 * 'en_US' against 'en'          => 3
 * 'en', 'en_UK' against 'en_US' => 2
 *  all locales, against '*'     => 1
 */
/* XXX: disable because nto used anymore - see below
 * ecore_imf_context_default_id_by_canvas_type_get
static int
_ecore_imf_context_match_locale(const char *locale, const char *against, int against_len)
{
   if (strcmp(against, "*") == 0)
     return 1;

   if (strcasecmp(locale, against) == 0)
     return 4;

   if (strncasecmp(locale, against, 2) == 0)
     return (against_len == 2) ? 3 : 2;

   return 0;
}
*/

EAPI const char *
ecore_imf_context_default_id_get(void)
{
   return ecore_imf_context_default_id_by_canvas_type_get(NULL);
}

EAPI const char *
ecore_imf_context_default_id_by_canvas_type_get(const char *canvas_type EINA_UNUSED)
{
   const char *id = getenv("ECORE_IMF_MODULE");

   if (id)
     {
        if (strcmp(id, "none") == 0) return NULL;
        if (ecore_imf_module_get(id)) return id;
     }
   else
     {
        if (getenv("WAYLAND_DISPLAY"))
          {
             id = "wayland";
             if (ecore_imf_module_get(id)) return id;
          }
     }
   return NULL;
/* XXX: I am not sure we need/want this. this causes imf modules to be
 * used where ECORE_IMF_MODULE is not set (and this causes issues with things
 * like scim where on some distros and some versions an scim connect BLOCKS
 * and if no scim is around this means an app blocks/hangs ... so disable
 * this so either you are in wayland mode OR you have to set
 * ECORE_IMF_MODULE
   Eina_List *modules;
   Ecore_IMF_Module *module;
   char *locale;
   char *tmp;
   int best_goodness = 0;

   modules = ecore_imf_module_available_get();
   if (!modules) return NULL;

   locale = setlocale(LC_CTYPE, NULL);
   if (!locale) return NULL;

   locale = strdup(locale);

   tmp = strchr(locale, '.');
   if (tmp) *tmp = '\0';
   tmp = strchr(locale, '@');
   if (tmp) *tmp = '\0';

   id = NULL;

   EINA_LIST_FREE(modules, module)
     {
        if (canvas_type &&
            strcmp(module->info->canvas_type, canvas_type) == 0)
          continue;

        const char *p = module->info->default_locales;
        while (p)
          {
             const char *q = strchr(p, ':');
             int goodness = _ecore_imf_context_match_locale(locale, p, q ? (size_t)(q - p) : strlen (p));

             if (goodness > best_goodness)
               {
                  id = module->info->id;
                  best_goodness = goodness;
               }

             p = q ? q + 1 : NULL;
          }
     }

   free(locale);
   return id;
 */
}

EAPI const Ecore_IMF_Context_Info *
ecore_imf_context_info_by_id_get(const char *id)
{
   Ecore_IMF_Module *module;

   if (!id) return NULL;
   module = ecore_imf_module_get(id);
   if (!module) return NULL;
   return module->info;
}

EAPI Ecore_IMF_Context *
ecore_imf_context_add(const char *id)
{
   Ecore_IMF_Context *ctx;

   if (!id) return NULL;
   ctx = ecore_imf_module_context_create(id);
   if (!ctx || !ctx->klass) return NULL;
   if (ctx->klass->add) ctx->klass->add(ctx);
   /* default use_preedit is EINA_TRUE, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_use_preedit_set(ctx, EINA_TRUE);

   /* default prediction is EINA_TRUE, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_prediction_allow_set(ctx, EINA_TRUE);

   /* default autocapital type is SENTENCE type, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_autocapital_type_set(ctx, ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE);

   /* default input hint */
   ecore_imf_context_input_hint_set(ctx, ECORE_IMF_INPUT_HINT_AUTO_COMPLETE);

   /* default input panel enabled status is EINA_TRUE, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_input_panel_enabled_set(ctx, EINA_TRUE);

   /* default input panel layout type is NORMAL type, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_input_panel_layout_set(ctx, ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL);

   /* default input_mode is ECORE_IMF_INPUT_MODE_FULL, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_input_mode_set(ctx, ECORE_IMF_INPUT_MODE_FULL);

   ecore_imf_context_bidi_direction_set(ctx, ECORE_IMF_BIDI_DIRECTION_NEUTRAL);

   return ctx;
}

EAPI const Ecore_IMF_Context_Info *
ecore_imf_context_info_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_info_get");
        return NULL;
     }
   return ctx->module->info;
}

EAPI void
ecore_imf_context_del(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Func_Node *fn;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_del");
        return;
     }

   if (show_req_ctx == ctx)
     show_req_ctx = NULL;

   if (ctx->klass && ctx->klass->del) ctx->klass->del(ctx);

   if (ctx->callbacks)
     {
        EINA_LIST_FREE(ctx->callbacks, fn)
           free(fn);
     }

   if (ctx->input_panel_callbacks)
     {
        EINA_LIST_FREE(ctx->input_panel_callbacks, fn)
           free(fn);
     }

   ECORE_MAGIC_SET(ctx, ECORE_MAGIC_NONE);
   free(ctx);
}

EAPI void
ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_client_window_set");
        return;
     }

   if (ctx->klass && ctx->klass->client_window_set) ctx->klass->client_window_set(ctx, window);
   ctx->window = window;
}

EAPI void *
ecore_imf_context_client_window_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_client_window_get");
        return NULL;
     }
   return ctx->window;
}

EAPI void
ecore_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_client_canvas_set");
        return;
     }

   if (ctx->klass && ctx->klass->client_canvas_set) ctx->klass->client_canvas_set(ctx, canvas);
   ctx->client_canvas = canvas;
}

EAPI void *
ecore_imf_context_client_canvas_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_client_canvas_get");
        return NULL;
     }
   return ctx->client_canvas;
}

EAPI void
ecore_imf_context_show(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_show");
        return;
     }

   show_req_ctx = ctx;
   if (ctx->klass && ctx->klass->show) ctx->klass->show(ctx);
}

EAPI void
ecore_imf_context_hide(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_hide");
        return;
     }

   if (ctx->klass && ctx->klass->hide) ctx->klass->hide(ctx);
}

EAPI void
ecore_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char **str, int *cursor_pos)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_preedit_string_get");
        return;
     }

   if (ctx->klass && ctx->klass->preedit_string_get)
     ctx->klass->preedit_string_get(ctx, str, cursor_pos);
   else
     {
        if (str) *str = strdup("");
        if (cursor_pos) *cursor_pos = 0;
     }
}

EAPI void
ecore_imf_context_preedit_string_with_attributes_get(Ecore_IMF_Context *ctx, char **str, Eina_List **attrs, int *cursor_pos)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_preedit_string_with_attributes_get");
        return;
     }
   if (ctx->klass && ctx->klass->preedit_string_with_attributes_get)
     ctx->klass->preedit_string_with_attributes_get(ctx, str, attrs, cursor_pos);
   else
     {
        if (str) *str = strdup("");
        if (attrs) *attrs = NULL;
        if (cursor_pos) *cursor_pos = 0;
     }
}

EAPI void
ecore_imf_context_focus_in(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_focus_in");
        return;
     }

   if (ctx->klass && ctx->klass->focus_in) ctx->klass->focus_in(ctx);
}

EAPI void
ecore_imf_context_focus_out(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_focus_out");
        return;
     }

   if (ctx->klass && ctx->klass->focus_out) ctx->klass->focus_out(ctx);
}

EAPI void
ecore_imf_context_reset(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_reset");
        return;
     }

   if (ctx->klass && ctx->klass->reset) ctx->klass->reset(ctx);
}

EAPI void
ecore_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_cursor_position_set");
        return;
     }

   if (ctx->klass && ctx->klass->cursor_position_set) ctx->klass->cursor_position_set(ctx, cursor_pos);
}

EAPI void
ecore_imf_context_cursor_location_set(Ecore_IMF_Context *ctx, int x, int y, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_cursor_location_set");
        return;
     }
   if (ctx->klass && ctx->klass->cursor_location_set) ctx->klass->cursor_location_set(ctx, x, y, w, h);
}

EAPI void
ecore_imf_context_use_preedit_set(Ecore_IMF_Context *ctx, Eina_Bool use_preedit)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_use_preedit_set");
        return;
     }
   if (ctx->klass && ctx->klass->use_preedit_set) ctx->klass->use_preedit_set(ctx, use_preedit);
}

EAPI void
ecore_imf_context_prediction_allow_set(Ecore_IMF_Context *ctx, Eina_Bool prediction)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_prediction_allow_set");
        return;
     }

   if (ctx->allow_prediction != prediction)
     {
        ctx->allow_prediction = prediction;

        if (ctx->klass && ctx->klass->prediction_allow_set)
          ctx->klass->prediction_allow_set(ctx, prediction);
     }
}

EAPI Eina_Bool
ecore_imf_context_prediction_allow_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_prediction_allow_get");
        return EINA_FALSE;
     }

   return ctx->allow_prediction;
}

EAPI void
ecore_imf_context_autocapital_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Autocapital_Type autocapital_type)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_autocapital_type_set");
        return;
     }

   if (ctx->autocapital_type != autocapital_type)
     {
        ctx->autocapital_type = autocapital_type;

        if (ctx->klass && ctx->klass->autocapital_type_set) ctx->klass->autocapital_type_set(ctx, autocapital_type);
     }
}

EAPI Ecore_IMF_Autocapital_Type
ecore_imf_context_autocapital_type_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_autocapital_allow_get");
        return ECORE_IMF_AUTOCAPITAL_TYPE_NONE;
     }

   return ctx->autocapital_type;
}

EAPI void
ecore_imf_context_retrieve_surrounding_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos), const void *data)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_retrieve_surrounding_callback_set");
        return;
     }

   ctx->retrieve_surrounding_func = func;
   ctx->retrieve_surrounding_data = (void *) data;
}

EAPI void
ecore_imf_context_retrieve_selection_callback_set(Ecore_IMF_Context *ctx, Eina_Bool (*func)(void *data, Ecore_IMF_Context *ctx, char **text), const void *data)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_retrieve_selection_callback_set");
        return;
     }

   ctx->retrieve_selection_func = func;
   ctx->retrieve_selection_data = (void *) data;
}

EAPI void
ecore_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_mode_set");
        return;
     }
   if (ctx->klass && ctx->klass->input_mode_set) ctx->klass->input_mode_set(ctx, input_mode);
   ctx->input_mode = input_mode;
}

EAPI Ecore_IMF_Input_Mode
ecore_imf_context_input_mode_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_mode_get");
        return 0;
     }
   return ctx->input_mode;
}

EAPI Eina_Bool
ecore_imf_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_filter_event");
        return EINA_FALSE;
     }
   if (ctx->klass && ctx->klass->filter_event) return ctx->klass->filter_event(ctx, type, event);
   return EINA_FALSE;
}

EAPI Ecore_IMF_Context *
ecore_imf_context_new(const Ecore_IMF_Context_Class *ctxc)
{
   Ecore_IMF_Context *ctx;

   if (!ctxc) return NULL;
   ctx = calloc(1, sizeof(Ecore_IMF_Context));
   if (!ctx) return NULL;
   ECORE_MAGIC_SET(ctx, ECORE_MAGIC_CONTEXT);
   ctx->klass = ctxc;
   ctx->data = NULL;
   ctx->retrieve_surrounding_func = NULL;
   ctx->retrieve_surrounding_data = NULL;
   return ctx;
}

EAPI void
ecore_imf_context_data_set(Ecore_IMF_Context *ctx, void *data)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_data_set");
        return;
     }
   ctx->data = data;
}

EAPI void *
ecore_imf_context_data_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_data_get");
        return NULL;
     }
   return ctx->data;
}

EAPI Eina_Bool
ecore_imf_context_surrounding_get(Ecore_IMF_Context *ctx, char **text, int *cursor_pos)
{
   int result = EINA_FALSE;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_surrounding_get");
        return EINA_FALSE;
     }

   if (ctx->retrieve_surrounding_func)
     {
        result = ctx->retrieve_surrounding_func(ctx->retrieve_surrounding_data, ctx, text, cursor_pos);
        if (!result)
          {
             if (text) *text = NULL;
             if (cursor_pos) *cursor_pos = 0;
          }
     }
   return result;
}

EAPI Eina_Bool
ecore_imf_context_selection_get(Ecore_IMF_Context *ctx, char **text)
{
   Eina_Bool result = EINA_FALSE;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_selection_get");
        return EINA_FALSE;
     }

   if (ctx->retrieve_selection_func)
     {
        result = ctx->retrieve_selection_func(ctx->retrieve_selection_data, ctx, text);
        if (!result)
          {
             if (text) *text = NULL;
          }
     }
   return result;
}

static void
_ecore_imf_event_free_preedit(void *data EINA_UNUSED, void *event)
{
   free(event);
}

EAPI void
ecore_imf_context_preedit_start_event_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Event_Preedit_Start *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_preedit_start_event_add");
        return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Preedit_Start));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->ctx = ctx;
   ecore_event_add(ECORE_IMF_EVENT_PREEDIT_START,
                   ev, _ecore_imf_event_free_preedit, NULL);
}

EAPI void
ecore_imf_context_preedit_end_event_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Event_Preedit_End *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_preedit_end_event_add");
        return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Preedit_End));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->ctx = ctx;
   ecore_event_add(ECORE_IMF_EVENT_PREEDIT_END,
                   ev, _ecore_imf_event_free_preedit, NULL);
}

EAPI void
ecore_imf_context_preedit_changed_event_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Event_Preedit_Changed *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_preedit_changed_event_add");
        return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Preedit_Changed));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->ctx = ctx;
   ecore_event_add(ECORE_IMF_EVENT_PREEDIT_CHANGED,
                   ev, _ecore_imf_event_free_preedit, NULL);
}

static void
_ecore_imf_event_free_commit(void *data EINA_UNUSED, void *event)
{
   Ecore_IMF_Event_Commit *ev;

   ev = event;
   if (ev->str) free(ev->str);
   free(ev);
}

EAPI void
ecore_imf_context_commit_event_add(Ecore_IMF_Context *ctx, const char *str)
{
   Ecore_IMF_Event_Commit *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_commit_event_add");
        return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Commit));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->ctx = ctx;
   ev->str = str ? strdup(str) : NULL;
   ecore_event_add(ECORE_IMF_EVENT_COMMIT,
                   ev, _ecore_imf_event_free_commit, NULL);
}

static void
_ecore_imf_event_free_delete_surrounding(void *data EINA_UNUSED, void *event)
{
   free(event);
}

EAPI void
ecore_imf_context_delete_surrounding_event_add(Ecore_IMF_Context *ctx, int offset, int n_chars)
{
   Ecore_IMF_Event_Delete_Surrounding *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_delete_surrounding_event_add");
        return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Delete_Surrounding));
   ev->ctx = ctx;
   ev->offset = offset;
   ev->n_chars = n_chars;
   ecore_event_add(ECORE_IMF_EVENT_DELETE_SURROUNDING,
                   ev, _ecore_imf_event_free_delete_surrounding, NULL);
}

EAPI void
ecore_imf_context_event_callback_add(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func, const void *data)
{
   Ecore_IMF_Func_Node *fn = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_event_callback_add");
        return;
     }

   if (!func) return;

   fn = calloc(1, sizeof (Ecore_IMF_Func_Node));
   if (!fn) return;

   fn->func = func;
   fn->data = data;
   fn->type = type;

   ctx->callbacks = eina_list_append(ctx->callbacks, fn);
}

EAPI void *
ecore_imf_context_event_callback_del(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, Ecore_IMF_Event_Cb func)
{
   Eina_List *l = NULL;
   Eina_List *l_next = NULL;
   Ecore_IMF_Func_Node *fn = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_event_callback_del");
        return NULL;
     }

   if (!func) return NULL;
   if (!ctx->callbacks) return NULL;

   EINA_LIST_FOREACH_SAFE(ctx->callbacks, l, l_next, fn)
     {
        if ((fn) && (fn->func == func) && (fn->type == type))
          {
             void *tmp = (void *)fn->data;
             free(fn);
             ctx->callbacks = eina_list_remove_list(ctx->callbacks, l);
             return tmp;
          }
     }
   return NULL;
}

EAPI void
ecore_imf_context_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Callback_Type type, void *event_info)
{
   Ecore_IMF_Func_Node *fn = NULL;
   Eina_List *l = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_event_callback_call");
        return;
     }

   EINA_LIST_FOREACH(ctx->callbacks, l, fn)
     {
        if ((fn) && (fn->type == type) && (fn->func))
          fn->func(fn->data, ctx, event_info);
     }
}

EAPI void
ecore_imf_context_control_panel_show(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_control_panel_show");
        return;
     }

   if (ctx->klass && ctx->klass->control_panel_show) ctx->klass->control_panel_show(ctx);
}

EAPI void
ecore_imf_context_control_panel_hide(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_control_panel_hide");
        return;
     }

   if (ctx->klass && ctx->klass->control_panel_hide) ctx->klass->control_panel_hide(ctx);
}

EAPI void
ecore_imf_context_input_hint_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Hints input_hints)
{
    if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
      {
         ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                          "ecore_imf_context_input_hint_set");
         return;
      }

   if (ctx->input_hints != input_hints)
     {
        if (ctx->klass && ctx->klass->input_hint_set)
          ctx->klass->input_hint_set(ctx, input_hints);

        ctx->input_hints = input_hints;
     }
}

EAPI Ecore_IMF_Input_Hints
ecore_imf_context_input_hint_get(Ecore_IMF_Context *ctx)
{
    if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
      {
         ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                          "ecore_imf_context_input_hint_get");
         return ECORE_IMF_INPUT_HINT_NONE;
      }

    return ctx->input_hints;
}

EAPI void
ecore_imf_context_input_panel_show(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_show");
        return;
     }

   show_req_ctx = ctx;
   if ((ctx->input_panel_enabled) || 
       (getenv("ECORE_IMF_INPUT_PANEL_ENABLED")))
     {
        if (ctx->klass && ctx->klass->show) ctx->klass->show(ctx);
     }
}

EAPI void
ecore_imf_context_input_panel_hide(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_hide");
        return;
     }

   if ((ctx->input_panel_enabled) || 
       (getenv("ECORE_IMF_INPUT_PANEL_ENABLED")))
     {
        if (ctx->klass && ctx->klass->hide) ctx->klass->hide(ctx);
     }
}

EAPI void
ecore_imf_context_input_panel_layout_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Layout layout)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_layout_set");
        return;
     }

   if (ctx->input_panel_layout != layout)
     {
        if (ctx->klass && ctx->klass->input_panel_layout_set)
          ctx->klass->input_panel_layout_set(ctx, layout);

        ctx->input_panel_layout = layout;

        if (layout == ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD)
          ecore_imf_context_autocapital_type_set(ctx, ECORE_IMF_AUTOCAPITAL_TYPE_NONE);
     }
}

EAPI Ecore_IMF_Input_Panel_Layout
ecore_imf_context_input_panel_layout_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_layout_get");
        return ECORE_IMF_INPUT_PANEL_LAYOUT_INVALID;
     }

   if (ctx->klass && ctx->klass->input_panel_layout_get)
     return ctx->input_panel_layout;
   else
     return ECORE_IMF_INPUT_PANEL_LAYOUT_INVALID;
}

EAPI void
ecore_imf_context_input_panel_layout_variation_set(Ecore_IMF_Context *ctx, int variation)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_layout_variation_set");
        return;
     }

   ctx->input_panel_layout_variation = variation;
}

EAPI int
ecore_imf_context_input_panel_layout_variation_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_layout_variation_get");
        return 0;
     }

   return ctx->input_panel_layout_variation;
}

EAPI void
ecore_imf_context_input_panel_language_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Lang lang)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_language_set");
        return;
     }

   if (ctx->input_panel_lang != lang)
     {
        if (ctx->klass && ctx->klass->input_panel_language_set)
          ctx->klass->input_panel_language_set(ctx, lang);

        ctx->input_panel_lang = lang;
     }
}

EAPI Ecore_IMF_Input_Panel_Lang
ecore_imf_context_input_panel_language_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_language_get");
        return ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC;
     }

   return ctx->input_panel_lang;
}

EAPI void
ecore_imf_context_input_panel_enabled_set(Ecore_IMF_Context *ctx,
                                           Eina_Bool enabled)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_enabled_set");
        return;
     }

   ctx->input_panel_enabled = enabled;
}

EAPI Eina_Bool
ecore_imf_context_input_panel_enabled_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_enabled_get");
        return EINA_FALSE;
     }

   return ctx->input_panel_enabled;
}

EAPI void
ecore_imf_context_input_panel_imdata_set(Ecore_IMF_Context *ctx, const void *data, int len)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_imdata_set");
        return;
     }

   if (!data) return;

   if (ctx->klass && ctx->klass->input_panel_imdata_set)
     ctx->klass->input_panel_imdata_set(ctx, data, len);
}

EAPI void
ecore_imf_context_input_panel_imdata_get(Ecore_IMF_Context *ctx, void *data, int *len)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_imdata_get");
        return;
     }

   if (!data) return;

   if (ctx->klass && ctx->klass->input_panel_imdata_get)
     ctx->klass->input_panel_imdata_get(ctx, data, len);
}

EAPI void
ecore_imf_context_input_panel_return_key_type_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Return_Key_Type return_key_type)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_return_key_type_set");
        return;
     }

   if (ctx->input_panel_return_key_type != return_key_type)
     {
        ctx->input_panel_return_key_type = return_key_type;
        if (ctx->klass && ctx->klass->input_panel_return_key_type_set)
          ctx->klass->input_panel_return_key_type_set(ctx, return_key_type);
     }
}

EAPI Ecore_IMF_Input_Panel_Return_Key_Type
ecore_imf_context_input_panel_return_key_type_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_return_key_type_get");
        return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
     }

   return ctx->input_panel_return_key_type;
}

EAPI void
ecore_imf_context_input_panel_return_key_disabled_set(Ecore_IMF_Context *ctx, Eina_Bool disabled)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_return_key_disabled_set");
        return;
     }

   if (ctx->input_panel_return_key_disabled != disabled)
     {
        ctx->input_panel_return_key_disabled = disabled;
        if (ctx->klass && ctx->klass->input_panel_return_key_disabled_set)
          ctx->klass->input_panel_return_key_disabled_set(ctx, disabled);
     }
}

EAPI Eina_Bool
ecore_imf_context_input_panel_return_key_disabled_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_return_key_disabled_get");
        return EINA_FALSE;
     }

   return ctx->input_panel_return_key_disabled;
}

EAPI void
ecore_imf_context_input_panel_caps_lock_mode_set(Ecore_IMF_Context *ctx, Eina_Bool mode)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_caps_lock_mode_set");
        return;
     }

   if (ctx->input_panel_caps_lock_mode != mode)
     {
        if (ctx->klass && ctx->klass->input_panel_caps_lock_mode_set)
          ctx->klass->input_panel_caps_lock_mode_set(ctx, mode);

        ctx->input_panel_caps_lock_mode = mode;
     }
}

EAPI Eina_Bool
ecore_imf_context_input_panel_caps_lock_mode_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_caps_lock_mode_get");
        return EINA_FALSE;
     }

   return ctx->input_panel_caps_lock_mode;
}

EAPI void
ecore_imf_context_input_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_geometry_get");
        return;
     }

   if (ctx->klass && ctx->klass->input_panel_geometry_get)
     ctx->klass->input_panel_geometry_get(ctx, x, y, w, h);
}

EAPI Ecore_IMF_Input_Panel_State
ecore_imf_context_input_panel_state_get(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Input_Panel_State state = ECORE_IMF_INPUT_PANEL_STATE_HIDE;
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_state_get");
        return ECORE_IMF_INPUT_PANEL_STATE_HIDE;
     }

   if (ctx->klass && ctx->klass->input_panel_state_get)
     state = ctx->klass->input_panel_state_get(ctx);

   return state;
}

EAPI void
ecore_imf_context_input_panel_event_callback_add(Ecore_IMF_Context *ctx,
                                                 Ecore_IMF_Input_Panel_Event type,
                                                 void (*func) (void *data, Ecore_IMF_Context *ctx, int value),
                                                 const void *data)
{
   Ecore_IMF_Input_Panel_Callback_Node *fn = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_event_callback_add");
        return;
     }

   if (!func) return;

   fn = calloc(1, sizeof (Ecore_IMF_Input_Panel_Callback_Node));
   if (!fn) return;

   fn->func = func;
   fn->data = data;
   fn->type = type;

   ctx->input_panel_callbacks = eina_list_append(ctx->input_panel_callbacks, fn);
}

EAPI void
ecore_imf_context_input_panel_event_callback_del(Ecore_IMF_Context *ctx,
                                                 Ecore_IMF_Input_Panel_Event type,
                                                 void (*func) (void *data, Ecore_IMF_Context *ctx, int value))
{
   Eina_List *l = NULL;
   Eina_List *l_next = NULL;
   Ecore_IMF_Input_Panel_Callback_Node *fn = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_event_callback_del");
        return;
     }

   if (!func) return;
   if (!ctx->input_panel_callbacks) return;

   EINA_LIST_FOREACH_SAFE(ctx->input_panel_callbacks, l, l_next, fn)
     {
        if ((fn) && (fn->func == func) && (fn->type == type))
          {
             free(fn);
             ctx->input_panel_callbacks = eina_list_remove_list(ctx->input_panel_callbacks, l);
             return;
          }
     }
}

EAPI void
ecore_imf_context_input_panel_event_callback_call(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Panel_Event type, int value)
{
   Ecore_IMF_Input_Panel_Callback_Node *fn = NULL;
   Eina_List *l = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_event_callback_call");
        return;
     }

   EINA_LIST_FOREACH(ctx->input_panel_callbacks, l, fn)
     {
        if ((fn) && (fn->type == type) && (fn->func))
          {
             fn->func(fn->data, ctx, value);
             if (type == ECORE_IMF_INPUT_PANEL_STATE_EVENT &&
                 value == ECORE_IMF_INPUT_PANEL_STATE_HIDE &&
                 show_req_ctx == ctx)
               show_req_ctx = NULL;

             if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
               break;
          }
     }
}

EAPI void
ecore_imf_context_input_panel_event_callback_clear(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Input_Panel_Callback_Node *fn = NULL;
   Eina_List *l = NULL;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_event_callback_clear");
        return;
     }

   for (l = ctx->input_panel_callbacks; l;)
     {
        fn = (Ecore_IMF_Input_Panel_Callback_Node *)l->data;

        if (fn)
          {
             ctx->input_panel_callbacks = eina_list_remove(ctx->input_panel_callbacks, fn);
             free (fn);
          }
        l = l->next;
     }
}

EAPI void
ecore_imf_context_input_panel_language_locale_get(Ecore_IMF_Context *ctx, char **lang)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_language_locale_get");
        return;
     }

   if (ctx->klass && ctx->klass->input_panel_language_locale_get)
     ctx->klass->input_panel_language_locale_get(ctx, lang);
   else
     {
        if (lang) *lang = strdup("");
     }
}

EAPI void
ecore_imf_context_candidate_panel_geometry_get(Ecore_IMF_Context *ctx, int *x, int *y, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_candidate_panel_geometry_get");
        return;
     }

   if (ctx->klass && ctx->klass->candidate_panel_geometry_get)
     ctx->klass->candidate_panel_geometry_get(ctx, x, y, w, h);
}

EAPI void
ecore_imf_context_input_panel_show_on_demand_set(Ecore_IMF_Context *ctx, Eina_Bool ondemand)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_show_on_demand_set");
        return;
     }

   ctx->input_panel_show_on_demand = ondemand;
}

EAPI Eina_Bool
ecore_imf_context_input_panel_show_on_demand_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_input_panel_show_on_demand_get");
        return EINA_FALSE;
     }

   return ctx->input_panel_show_on_demand;
}

EAPI void
ecore_imf_context_bidi_direction_set(Ecore_IMF_Context *ctx, Ecore_IMF_BiDi_Direction direction)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_bidi_direction_set");
        return;
     }

   if (ctx->bidi_direction != direction)
     {
        if (ctx->klass && ctx->klass->bidi_direction_set)
          ctx->klass->bidi_direction_set(ctx, direction);

        ctx->bidi_direction = direction;
     }
}

EAPI Ecore_IMF_BiDi_Direction
ecore_imf_context_bidi_direction_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_bidi_direction_get");
        return ECORE_IMF_BIDI_DIRECTION_NEUTRAL;
     }

   return ctx->bidi_direction;
}

EAPI Ecore_IMF_Input_Panel_Keyboard_Mode
ecore_imf_context_keyboard_mode_get(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Input_Panel_Keyboard_Mode mode = ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE;
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_keyboard_mode_get");
        return ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE;
     }

   if (ctx->klass && ctx->klass->keyboard_mode_get)
     mode = ctx->klass->keyboard_mode_get(ctx);

   return mode;
}

EAPI void
ecore_imf_context_prediction_hint_set(Ecore_IMF_Context *ctx, const char *prediction_hint)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_prediction_hint_set");
        return;
     }

   if (ctx->klass && ctx->klass->prediction_hint_set)
     ctx->klass->prediction_hint_set(ctx, prediction_hint);
}

EAPI void
ecore_imf_context_mime_type_accept_set(Ecore_IMF_Context *ctx, const char *mime_type)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
        ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                         "ecore_imf_context_mime_type_accept_set");
        return;
     }

   if (!mime_type) return;

   if (ctx->klass && ctx->klass->mime_type_accept_set)
     ctx->klass->mime_type_accept_set(ctx, mime_type);
}
