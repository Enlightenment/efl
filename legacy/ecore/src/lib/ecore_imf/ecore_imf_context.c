/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <locale.h>
#include <stdlib.h>

#include "config.h"

#include "Ecore_IMF.h"
#include "ecore_imf_private.h"

#include <Ecore.h>

/**
 * @defgroup Ecore_IMF_Context_Group Ecore Input Method Context Functions
 *
 * Functions that operate on Ecore Input Method Context objects.
 */

/**
 * Get the list of the available Input Method Context ids.
 *
 * Note that the caller is responsible for freeing the Ecore_List
 * when finished with it. There is no need to finish the list strings.
 *
 * @return Return an Ecore_List of strings;
 *         on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI Ecore_List *
ecore_imf_context_available_ids_get(void)
{
   return ecore_imf_module_context_ids_get();
}

EAPI Ecore_List *
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

/**
 * Get the id of the default Input Method Context.
 * The id may to used to create a new instance of an Input Method
 * Context object.
 *
 * @return Return a string containing the id of the default Input
 *         Method Context; on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI const char *
ecore_imf_context_default_id_get(void)
{
   return ecore_imf_context_default_id_by_canvas_type_get(NULL);
}

EAPI const char *
ecore_imf_context_default_id_by_canvas_type_get(const char *canvas_type)
{
   const char *id;
   Ecore_List *modules;
   Ecore_IMF_Module *module;
   char *locale;
   char *tmp;
   int best_goodness = 0;

   id = getenv("ECORE_IMF_MODULE");
   if (id)
     {
	if (strcmp(id, "none") == 0) return NULL;
	if (ecore_imf_module_get(id)) return id;
     }

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

   ecore_list_first_goto(modules);
   while ((module = ecore_list_next(modules)))
     {
	if (canvas_type &&
	    strcmp(module->info->canvas_type, canvas_type) == 0)
	  continue;

	const char *p = module->info->default_locales;
	while (p)
	  {
	     const char *q = strchr(p, ':');
	     int goodness = _ecore_imf_context_match_locale(locale, p, q ? q - p : strlen (p));

	      if (goodness > best_goodness)
		{
		   id = module->info->id;
		   best_goodness = goodness;
		}

	      p = q ? q + 1 : NULL;
	  }
     }
   ecore_list_destroy(modules);

   free(locale);
   return id;
}

/**
 * Retrieve the info for the Input Method Context with @p id.
 *
 * @param id The Input Method Context id to query for.
 * @return Return a #Ecore_IMF_Context_Info for the Input Method Context with @p id;
 *         on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI const Ecore_IMF_Context_Info *
ecore_imf_context_info_by_id_get(const char *id)
{
   Ecore_IMF_Module *module;

   if (!id) return NULL;
   module = ecore_imf_module_get(id);
   if (!module) return NULL;
   return module->info;
}

/**
 * Create a new Input Method Context defined by the given id.
 *
 * @param id The Input Method Context id.
 * @return A newly allocated Input Method Context;
 *         on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI Ecore_IMF_Context *
ecore_imf_context_add(const char *id)
{
   Ecore_IMF_Context *ctx;

   if (!id) return NULL;
   ctx = ecore_imf_module_context_create(id);
   if (!ctx || !ctx->klass) return NULL;
   if (ctx->klass->add) ctx->klass->add(ctx);
   /* default use_preedit is 1, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_use_preedit_set(ctx, 1);
   /* default input_mode is ECORE_IMF_INPUT_MODE_FULL, so let's make sure it's
    * set on the immodule */
   ecore_imf_context_input_mode_set(ctx, ECORE_IMF_INPUT_MODE_FULL);
   return ctx;
}

/**
 * Retrieve the info for the given Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return Return a #Ecore_IMF_Context_Info for the given Input Method Context;
 *         on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Group
 */
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

/**
 * Delete the given Input Method Context and free its memory.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_del(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_del");
	return;
     }
   if (ctx->klass->del) ctx->klass->del(ctx);
   ECORE_MAGIC_SET(ctx, ECORE_MAGIC_NONE);
   free(ctx);
}

/**
 * Set the client window for the Input Method Context; this is the
 * Ecore_X_Window when using X11, Ecore_Win32_Window when using Win32, etc.
 * This window is used in order to correctly position status windows, and may
 * also be used for purposes internal to the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param window The client window. This may be NULL to indicate
 *               that the previous client window no longer exists.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_client_window_set");
	return;
     }
   if (ctx->klass->client_window_set) ctx->klass->client_window_set(ctx, window);
}

/**
 * Set the client canvas for the Input Method Context; this is the
 * canvas in which the input appears.
 * The canvas type can be determined by using the context canvas type.
 * Actually only canvas with type "evas" (Evas *) is supported.
 * This canvas may be used in order to correctly position status windows, and may
 * also be used for purposes internal to the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param canas The client canvas. This may be NULL to indicate
 *              that the previous client canvas no longer exists.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_client_window_set");
	return;
     }
   if (ctx->klass->client_canvas_set) ctx->klass->client_canvas_set(ctx, canvas);
}

/**
 * Ask the Input Method Context to show itself.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_show(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_show");
	return;
     }
   if (ctx->klass->show) ctx->klass->show(ctx);
}

/**
 * Ask the Input Method Context to hide itself.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_hide(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_hide");
	return;
     }
   if (ctx->klass->hide) ctx->klass->hide(ctx);
}

/*
 * Retrieve the current preedit string and cursor position
 * for the Input Method Context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param str Location to store the retrieved string. The
 *            string retrieved must be freed with free().
 * @param cursor_pos Location to store position of cursor (in characters)
 *                   within the preedit string.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char **str, int *cursor_pos)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_preedit_string_get");
	return;
     }
   if (ctx->klass->preedit_string_get)
     ctx->klass->preedit_string_get(ctx, str, cursor_pos);
   else
     {
	if (str) *str = strdup("");
	if (cursor_pos) *cursor_pos = 0;
     }
}

/**
 * Notify the Input Method Context that the widget to which its
 * correspond has gained focus.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_focus_in(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_focus_in");
	return;
     }
   if (ctx->klass->focus_in) ctx->klass->focus_in(ctx);
}

/**
 * Notify the Input Method Context that the widget to which its
 * correspond has lost focus.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_focus_out(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_focus_out");
	return;
     }
   if (ctx->klass->focus_out) ctx->klass->focus_out(ctx);
}

/**
 * Notify the Input Method Context that a change such as a
 * change in cursor position has been made. This will typically
 * cause the Input Method Context to clear the preedit state.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_reset(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_reset");
	return;
     }
   if (ctx->klass->reset) ctx->klass->reset(ctx);
}

/**
 * Notify the Input Method Context that a change in the cursor
 * position has been made.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param cursor_pos New cursor position in characters.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_cursor_position_set");
	return;
     }
   if (ctx->klass->cursor_position_set) ctx->klass->cursor_position_set(ctx, cursor_pos);
}

/**
 * Set whether the IM context should use the preedit string
 * to display feedback. If @use_preedit is 0 (default
 * is 1), then the IM context may use some other method to display
 * feedback, such as displaying it in a child of the root window.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param use_preedit Whether the IM context should use the preedit string.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_use_preedit_set(Ecore_IMF_Context *ctx, int use_preedit)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_use_preedit_set");
	return;
     }
   if (ctx->klass->use_preedit_set) ctx->klass->use_preedit_set(ctx, use_preedit);
}

/**
 * Set the callback to be used on get_surrounding request.
 *
 * This callback will be called when the Input Method Context
 * module requests the surrounding context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param func The callback to be called.
 * @param data The data pointer to be passed to @p func
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_retrieve_surrounding_callback_set(Ecore_IMF_Context *ctx, int (*func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos), const void *data)
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

/**
 * Set the input mode used by the Ecore Input Context.
 *
 * The input mode can be one of the input modes defined in
 * #Ecore_IMF_Input_Mode. The default input mode is
 * ECORE_IMF_INPUT_MODE_FULL.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param input_mode The input mode to be used by @p ctx.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI void
ecore_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_input_mode_set");
	return;
     }
   if (ctx->klass->input_mode_set) ctx->klass->input_mode_set(ctx, input_mode);
   ctx->input_mode = input_mode;
}

/**
 * Get the input mode being used by the Ecore Input Context.
 *
 * See @ref ecore_imf_context_input_mode_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The input mode being used by @p ctx.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI Ecore_IMF_Input_Mode
ecore_imf_context_input_mode_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_input_mode_set");
	return 0;
     }
   return ctx->input_mode;
}

/**
 * Allow an Ecore Input Context to internally handle an event.
 * If this function returns 1, then no further processing
 * should be done for this event.
 *
 * Input methods must be able to accept all types of events (simply
 * returning 0 if the event was not handled), but there is no
 * obligation of any events to be submitted to this function.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param type The type of event defined by #Ecore_IMF_Event_Type.
 * @param event The event itself.
 * @return 1 if the event was handled; otherwise 0.
 * @ingroup Ecore_IMF_Context_Group
 */
EAPI int
ecore_imf_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_filter_event");
	return 0;
     }
   if (ctx->klass->filter_event) return ctx->klass->filter_event(ctx, type, event);
   return 0;
}

/**
 * @defgroup Ecore_IMF_Context_Module_Group Ecore Input Method Context Module Functions
 *
 * Functions that should be used by Ecore Input Method Context modules.
 */

/**
 * Creates a new Input Method Context with klass specified by @p ctxc.
 *
 * This method should be used by modules implementing the Input
 * Method Context interface.
 *
 * @param ctxc An #Ecore_IMF_Context_Class.
 * @return A new #Ecore_IMF_Context; on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
EAPI Ecore_IMF_Context *
ecore_imf_context_new(const Ecore_IMF_Context_Class *ctxc)
{
   Ecore_IMF_Context *ctx;

   if (!ctxc) return NULL;
   ctx = malloc(sizeof(Ecore_IMF_Context));
   if (!ctx) return NULL;
   ECORE_MAGIC_SET(ctx, ECORE_MAGIC_CONTEXT);
   ctx->klass = ctxc;
   ctx->data = NULL;
   ctx->retrieve_surrounding_func = NULL;
   ctx->retrieve_surrounding_data = NULL;
   return ctx;
}

/**
 * Set the Input Method Context specific data.
 *
 * Note that this method should be used by modules to set
 * the Input Method Context specific data and it's not meant to
 * be used by applications to store application specific data.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param data The Input Method Context specific data.
 * @return A new #Ecore_IMF_Context; on failure it returns NULL.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
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

/**
 * Get the Input Method Context specific data.
 *
 * See @ref ecore_imf_context_data_set for more details.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @return The Input Method Context specific data.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
EAPI void *ecore_imf_context_data_get(Ecore_IMF_Context *ctx)
{
   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_data_get");
	return NULL;
     }
   return ctx->data;
}

/**
 * Retrieve context around insertion point.
 *
 * This function is implemented by calling the
 * Ecore_IMF_Context::retrieve_surrounding_func (
 * set using #ecore_imf_context_retrieve_surrounding_callback_set).
 *
 * There is no obligation for a widget to respond to the
 * ::retrieve_surrounding_func, so input methods must be prepared
 * to function without context.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param text Location to store a UTF-8 encoded string of text
 *             holding context around the insertion point.
 *             If the function returns 1, then you must free
 *             the result stored in this location with free().
 * @param cursor_pos Location to store the position in characters of
 *                   the insertion cursor within @text.
 * @return 1 if surrounding text was provided; otherwise 0.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
EAPI int
ecore_imf_context_surrounding_get(Ecore_IMF_Context *ctx, char **text, int *cursor_pos)
{
   int result = 0;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_surrounding_get");
	return 0;
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

static void
_ecore_imf_event_free_preedit(void *data __UNUSED__, void *event)
{
   free(event);
}

/**
 * Adds ECORE_IMF_EVENT_PREEDIT_START to the event queue.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
EAPI void
ecore_imf_context_preedit_start_event_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Event_Commit *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_preedit_start_event_add");
	return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Preedit_Start));
   ev->ctx = ctx;
   ecore_event_add(ECORE_IMF_EVENT_PREEDIT_START,
		   ev, _ecore_imf_event_free_preedit, NULL);
}

/**
 * Adds ECORE_IMF_EVENT_PREEDIT_END to the event queue.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
EAPI void
ecore_imf_context_preedit_end_event_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Event_Commit *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_preedit_end_event_add");
	return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Preedit_End));
   ev->ctx = ctx;
   ecore_event_add(ECORE_IMF_EVENT_PREEDIT_END,
		   ev, _ecore_imf_event_free_preedit, NULL);
}

/**
 * Adds ECORE_IMF_EVENT_PREEDIT_CHANGED to the event queue.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
EAPI void
ecore_imf_context_preedit_changed_event_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Event_Commit *ev;

   if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
     {
	ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
			 "ecore_imf_context_preedit_changed_event_add");
	return;
     }

   ev = malloc(sizeof(Ecore_IMF_Event_Preedit_Changed));
   ev->ctx = ctx;
   ecore_event_add(ECORE_IMF_EVENT_PREEDIT_CHANGED,
		   ev, _ecore_imf_event_free_preedit, NULL);
}

static void
_ecore_imf_event_free_commit(void *data __UNUSED__, void *event)
{
   Ecore_IMF_Event_Commit *ev;

   ev = event;
   if (ev->str) free(ev->str);
   free(ev);
}

/**
 * Adds ECORE_IMF_EVENT_COMMIT to the event queue.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param str The committed string.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
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
   ev->ctx = ctx;
   ev->str = str ? strdup(str) : NULL;
   ecore_event_add(ECORE_IMF_EVENT_COMMIT,
		   ev, _ecore_imf_event_free_commit, NULL);

}

static void
_ecore_imf_event_free_delete_surrounding(void *data __UNUSED__, void *event)
{
   free(event);
}

/**
 * Adds ECORE_IMF_EVENT_DELETE_SURROUNDING to the event queue.
 *
 * @param ctx An #Ecore_IMF_Context.
 * @param offset The start offset of surrounding to be deleted.
 * @param n_chars The number of characters to be deleted.
 * @ingroup Ecore_IMF_Context_Module_Group
 */
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
