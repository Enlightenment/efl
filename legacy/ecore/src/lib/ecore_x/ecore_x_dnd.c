#include <Ecore.h>
#include "ecore_x_private.h"
#include <Ecore_X.h>

static Ecore_X_Selection_Data _xdnd_selection;
static Ecore_X_DND_Protocol *_xdnd;

void
_ecore_x_dnd_init (void)
{
   _xdnd = calloc(1, sizeof(Ecore_X_DND_Protocol));
   
   _xdnd->version = ECORE_X_DND_VERSION;
   _xdnd->source = None;
   _xdnd->dest = None;
   _xdnd->state = ECORE_X_DND_FINISHED;
}

void
ecore_x_dnd_enabled_set (Ecore_X_Window win, int on)
{
   /* FIXME: This property needs to contain the version number */
   unsigned char *prop_data = "5";
   if (on)
      ecore_x_window_prop_property_set(win, _ecore_x_atom_xdnd_aware, 
                                       XA_ATOM, 8, prop_data, 1);
   else
      ecore_x_window_prop_property_del(win, _ecore_x_atom_xdnd_aware);
}

int
ecore_x_dnd_version_get (Ecore_X_Window win)
{
   unsigned char *prop_data;
   int num;
   
   if (ecore_x_window_prop_property_get(win, _ecore_x_atom_xdnd_aware,
                                        XA_ATOM, 8, &prop_data, &num))
   {
      free(prop_data);
      return 1;
   }
   else
      return 0;
} 

Ecore_X_DND_Protocol *
_ecore_x_dnd_protocol_get (void)
{
   return _xdnd;
}

int 
ecore_x_dnd_begin (Ecore_X_Window source, unsigned char *data, int size)
{
   unsigned char *buf;
   
   /* Take ownership of XdndSelection */
   XSetSelectionOwner(_ecore_x_disp, _ecore_x_atom_selection_xdnd, source,
                      _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, _ecore_x_atom_selection_xdnd) != source)
      return 0;
   
   /* Initialize Selection Data Struct */
   _xdnd_selection.win = source;
   _xdnd_selection.selection = _ecore_x_atom_selection_xdnd;
   _xdnd_selection.length = size;
   _xdnd_selection.time = _ecore_x_event_last_time;
   
   buf = malloc(size);
   memcpy(buf, data, size);
   _xdnd_selection.data = buf;

   _xdnd->source = source;
   _xdnd->state = ECORE_X_DND_OUT;

   return 1;
}

