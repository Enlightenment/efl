#include <Ecore.h>
#include "ecore_x_private.h"
#include <Ecore_X.h>
#include <Ecore_Txt.h>

/* FIXME: Initialize! */
static Ecore_X_Selection_Data selections[3];
static Ecore_X_Selection_Data request_data[3];

static void
_ecore_x_selection_request_data_get(Ecore_X_Atom selection, void **buf, int *len)
{
   int i;
   char *data;
   if (selection == _ecore_x_atom_selection_primary)
      i = 0;
   else if (selection == _ecore_x_atom_selection_secondary)
      i = 1;
   else if (selection == _ecore_x_atom_selection_clipboard)
      i = 2;
   else
      return;

   if (!request_data[i].data || !request_data[i].length)
   {
      *len = 0;
      *buf = NULL;
      return;
   }

   data = malloc(request_data[i].length);
   memcpy(data, request_data[i].data, request_data[i].length);
   *len = request_data[i].length;
   *buf = data;
   return;
}

void
ecore_x_selection_primary_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(_ecore_x_atom_selection_primary,
                                       buf, len);
}

void
ecore_x_selection_secondary_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(_ecore_x_atom_selection_secondary,
                                       buf, len);
}

void
ecore_x_selection_clipboard_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(_ecore_x_atom_selection_clipboard,
                                       buf, len);
}

void 
_ecore_x_selection_request_data_set(Ecore_X_Selection_Data data)
{
   int i;
   if (data.selection == _ecore_x_atom_selection_primary)
      i = 0;
   else if (data.selection == _ecore_x_atom_selection_secondary)
      i = 1;
   else if (data.selection == _ecore_x_atom_selection_clipboard)
      i = 2;
   else
      return;

   request_data[i] = data;
}

Ecore_X_Selection_Data *
_ecore_x_selection_get(Atom selection)
{
   if (selection == _ecore_x_atom_selection_primary)
      return &selections[0];
   else if (selection == _ecore_x_atom_selection_secondary)
      return &selections[1];
   else if (selection == _ecore_x_atom_selection_clipboard)
      return &selections[2];
   else
      return NULL;
}

int 
_ecore_x_selection_set(Window w, char *data, int len, Atom selection)
{
   int in;
   char *buf = NULL;
   
   XSetSelectionOwner(_ecore_x_disp, selection, w, _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, selection) != w)
      return 0;
   
   if (selection == _ecore_x_atom_selection_primary)
      in = 0;
   else if (selection == _ecore_x_atom_selection_secondary)
      in = 1;
   else
      in = 2;
   
   if (data)
   {
      selections[in].win = w;
      selections[in].selection = selection;
      selections[in].length = len;
      selections[in].time = _ecore_x_event_last_time;
      
      buf = malloc(sizeof(char) * len);
      memcpy(buf, data, sizeof(char) * len);
      selections[in].data = buf;
   }
   else
   {
      if (selections[in].data)
      {
         free(selections[in].data);
         memset(&selections[in], 0, sizeof(Ecore_X_Selection_Data));
      }
   }
   
   /* ecore_x_window_prop_property_set(_ecore_x_disp, w, selection, 
         XA_STRING, 8, data, len); */
   return 1;
}

int 
ecore_x_selection_primary_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_primary);
}

int 
ecore_x_selection_primary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_primary);
}

int 
ecore_x_selection_secondary_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_secondary);
}

int 
ecore_x_selection_secondary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_secondary);
}

int 
ecore_x_selection_clipboard_set(Ecore_X_Window w, char *data, int len)
{
   return _ecore_x_selection_set(w, data, len, _ecore_x_atom_selection_clipboard);
}

int 
ecore_x_selection_clipboard_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, _ecore_x_atom_selection_clipboard);
}

static void 
_ecore_x_selection_request(Ecore_X_Window w, Ecore_X_Atom selection, Ecore_X_Selection_Target t) 
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

void 
ecore_x_selection_primary_request(Ecore_X_Window w, Ecore_X_Selection_Target t)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_primary, t);
}

void 
ecore_x_selection_secondary_request(Ecore_X_Window w, Ecore_X_Selection_Target t)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_secondary, t);
}

void 
ecore_x_selection_clipboard_request(Ecore_X_Window w, Ecore_X_Selection_Target t)
{
   _ecore_x_selection_request(w, _ecore_x_atom_selection_clipboard, t);
}

char *
ecore_x_selection_convert_to_string(char *data)
{
   /* FIXME: Do something */
   return data;
}

char *
ecore_x_selection_convert_to_utf8_string(char *data)
{
   /* FIXME: Do something */
   return data;
}

Ecore_X_Selection_Target
ecore_x_selection_target_get(Ecore_X_Atom target)
{
   if (target == _ecore_x_atom_file_name)
      return ECORE_X_SELECTION_TARGET_FILENAME;
   else if (target == _ecore_x_atom_string)
      return ECORE_X_SELECTION_TARGET_STRING;
   else if (target == _ecore_x_atom_utf8_string)
      return ECORE_X_SELECTION_TARGET_UTF8_STRING;
   else if (target == _ecore_x_atom_text)
      return ECORE_X_SELECTION_TARGET_TEXT;
   else
      return ECORE_X_SELECTION_TARGET_TEXT;
}

