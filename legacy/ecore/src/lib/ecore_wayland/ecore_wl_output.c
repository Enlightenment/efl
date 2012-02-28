#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_wl_private.h"
#include "Ecore_Wayland.h"

/* local function prototypes */
static void _ecore_wl_output_cb_geometry(void *data, struct wl_output *wl_output __UNUSED__, int x, int y, int w __UNUSED__, int h __UNUSED__, int subpixel __UNUSED__, const char *make __UNUSED__, const char *model __UNUSED__);
static void _ecore_wl_output_cb_mode(void *data, struct wl_output *wl_output __UNUSED__, unsigned int flags, int w, int h, int refresh __UNUSED__);

/* wayland listeners */
static const struct wl_output_listener _ecore_wl_output_listener = 
{
   _ecore_wl_output_cb_geometry,
   _ecore_wl_output_cb_mode
};

void 
_ecore_wl_output_add(Ecore_Wl_Display *ewd, unsigned int id)
{
   Ecore_Wl_Output *output;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(output = malloc(sizeof(Ecore_Wl_Output)))) return;

   memset(output, 0, sizeof(Ecore_Wl_Output));

   output->display = ewd;

   output->output = wl_display_bind(ewd->wl.display, id, &wl_output_interface);
   wl_list_insert(ewd->outputs.prev, &output->link);
   wl_output_add_listener(output->output, &_ecore_wl_output_listener, output);
}

void 
_ecore_wl_output_del(Ecore_Wl_Output *output) 
{
   if (!output) return;
   if (output->destroy) (*output->destroy)(output, output->data);
   if (output->output) wl_output_destroy(output->output);
   wl_list_remove(&output->link);
   free(output);
}

/* local functions */
static void 
_ecore_wl_output_cb_geometry(void *data, struct wl_output *wl_output __UNUSED__, int x, int y, int w __UNUSED__, int h __UNUSED__, int subpixel __UNUSED__, const char *make __UNUSED__, const char *model __UNUSED__)
{
   Ecore_Wl_Output *output;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   output = data;
   output->allocation.x = x;
   output->allocation.y = y;
}

static void 
_ecore_wl_output_cb_mode(void *data, struct wl_output *wl_output __UNUSED__, unsigned int flags, int w, int h, int refresh __UNUSED__)
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
