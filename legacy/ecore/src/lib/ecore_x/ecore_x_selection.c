#include <Ecore.h>
#include "ecore_x_private.h"
#include <Ecore_X.h>

static int _ecore_x_selection_set(Ecore_X_Window w, char *data, int len, Ecore_X_Atom selection)
{
   XSetSelectionOwner(_ecore_x_disp, selection, w, _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, selection) != w)
      return 0;
   /* ecore_x_window_prop_property_set(_ecore_x_disp, w, selection, 
         XA_STRING, 8, data, len); */
   return 1;
}

int ecore_x_selection_primary_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_primary);
}

int ecore_x_selection_primary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_primary);
}

int ecore_x_selection_secondary_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_secondary);
}

int ecore_x_selection_secondary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_secondary);
}

int ecore_x_selection_clipboard_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_clipboard);
}

int ecore_x_selection_clipboard_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_clipboard);
}

static void _ecore_x_selection_request(Ecore_X_Window w, Ecore_X_Atom selection, Ecore_X_Selection_Target t) 
{
   Ecore_X_Atom target, prop;

   switch (t) {
      case ECORE_X_SELECTION_TARGET_FILENAME:
         target = _ecore_x_atom_file_name;
         break;
      case ECORE_X_SELECTION_TARGET_STRING:
         target = _ecore_x_atom_string;
         break;
      case ECORE_X_SELECTION_TARGET_UTF8_STRING:
         target = _ecore_x_atom_utf8_string;
         break;
      case ECORE_X_SELECTION_TARGET_TEXT:
         target = _ecore_x_atom_text;
         break;
      default:
         target = _ecore_x_atom_text;
   }
   
   if (selection == _ecore_x_atom_selection_primary)
      prop = _ecore_x_atom_selection_prop_primary;
   else if (selection == _ecore_x_atom_selection_secondary)
      prop = _ecore_x_atom_selection_prop_secondary;
   else
      prop = _ecore_x_atom_selection_prop_clipboard;

   XConvertSelection(_ecore_x_disp, selection, target, prop,
                     w, _ecore_x_event_last_time);
}

void ecore_x_selection_primary_request(Ecore_X_Window w, Ecore_X_Selection_Target t)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_primary, t);
}

void ecore_x_selection_secondary_request(Ecore_X_Window w, Ecore_X_Selection_Target t)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_secondary, t);
}

void ecore_x_selection_clipboard_request(Ecore_X_Window w, Ecore_X_Selection_Target t)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_clipboard, t);
}

