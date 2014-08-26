#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl_private.h"

/* local function prototypes */
static void _ecore_wl_output_cb_geometry(void *data, struct wl_output *wl_output EINA_UNUSED, int x, int y, int w, int h, int subpixel EINA_UNUSED, const char *make EINA_UNUSED, const char *model EINA_UNUSED, int transform EINA_UNUSED);
static void _ecore_wl_output_cb_mode(void *data, struct wl_output *wl_output EINA_UNUSED, unsigned int flags, int w, int h, int refresh EINA_UNUSED);
static void _ecore_wl_output_cb_done(void *data EINA_UNUSED, struct wl_output *output EINA_UNUSED);
static void _ecore_wl_output_cb_scale(void *data EINA_UNUSED, struct wl_output *output EINA_UNUSED, int scale EINA_UNUSED);

/* wayland listeners */
static const struct wl_output_listener _ecore_wl_output_listener = 
{
   _ecore_wl_output_cb_geometry,
   _ecore_wl_output_cb_mode,
   _ecore_wl_output_cb_done,
   _ecore_wl_output_cb_scale
};

/* @since 1.2 */
EAPI Eina_Inlist *
ecore_wl_outputs_get(void)
{
   return _ecore_wl_disp->outputs;
}

void 
_ecore_wl_output_add(Ecore_Wl_Display *ewd, unsigned int id)
{
   Ecore_Wl_Output *output;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(output = malloc(sizeof(Ecore_Wl_Output)))) return;

   memset(output, 0, sizeof(Ecore_Wl_Output));

   output->display = ewd;

   output->output = 
     wl_registry_bind(ewd->wl.registry, id, &wl_output_interface, 2);

   ewd->outputs = eina_inlist_append(ewd->outputs, EINA_INLIST_GET(output));
   wl_output_add_listener(output->output, &_ecore_wl_output_listener, output);
}

void 
_ecore_wl_output_del(Ecore_Wl_Output *output) 
{
   if (!output) return;
   if (output->destroy) (*output->destroy)(output, output->data);
   if (output->output) wl_output_destroy(output->output);
   _ecore_wl_disp->outputs = 
     eina_inlist_remove(_ecore_wl_disp->outputs, EINA_INLIST_GET(output));
   free(output);
}

/* local functions */
static void 
_ecore_wl_output_cb_geometry(void *data, struct wl_output *wl_output EINA_UNUSED, int x, int y, int w, int h, int subpixel EINA_UNUSED, const char *make EINA_UNUSED, const char *model EINA_UNUSED, int transform)
{
   Ecore_Wl_Output *output;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   output = data;
   output->allocation.x = x;
   output->allocation.y = y;
   output->mw = w;
   output->mh = h;
   output->transform = transform;
}

static void 
_ecore_wl_output_cb_mode(void *data, struct wl_output *wl_output EINA_UNUSED, unsigned int flags, int w, int h, int refresh EINA_UNUSED)
{
   Ecore_Wl_Output *output;
   Ecore_Wl_Display *ewd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   output = data;
   ewd = output->display;
   if (flags & WL_OUTPUT_MODE_CURRENT)
     {
        output->allocation.w = w;
        output->allocation.h = h;
        _ecore_wl_disp->output = output;
        if (ewd->output_configure) (*ewd->output_configure)(output, ewd->data);
     }
}

static void 
_ecore_wl_output_cb_done(void *data EINA_UNUSED, struct wl_output *output EINA_UNUSED)
{

}

static void 
_ecore_wl_output_cb_scale(void *data EINA_UNUSED, struct wl_output *output EINA_UNUSED, int scale EINA_UNUSED)
{

}
