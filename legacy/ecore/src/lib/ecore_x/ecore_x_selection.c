#include <Ecore.h>
#include "ecore_x_private.h"
#include <Ecore_X.h>
#include <Ecore_Txt.h>

/* FIXME: Initialize! */
static Ecore_X_Selection_Data selections[3];
static Ecore_X_Selection_Data request_data[3];
static Ecore_X_Selection_Converter *converters;

static int _ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret);

void
_ecore_x_selection_data_initialize(void)
{
   memset(selections, 0, sizeof(selections));
   memset(request_data, 0, sizeof(request_data));

   /* Initialize converters */
   converters = NULL;
   ecore_x_selection_converter_atom_add(_ecore_x_atom_text, 
         _ecore_x_selection_converter_text);
   ecore_x_selection_converter_atom_add(_ecore_x_atom_compound_text,
         _ecore_x_selection_converter_text);
   ecore_x_selection_converter_atom_add(_ecore_x_atom_string,
         _ecore_x_selection_converter_text);
   
}

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

void
ecore_x_selection_converter_atom_add(Ecore_X_Atom target,
      int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret))
{
   Ecore_X_Selection_Converter *cnv;

   cnv = converters;
   if (converters) 
   {
      while (1)
      {
         if (cnv->target == target)
         {
            cnv->convert = func;
            return;
         }
         if (cnv->next)
            cnv = cnv->next;
         else
            break;
      }
   
      cnv->next = calloc(1, sizeof(Ecore_X_Selection_Converter));
      cnv = cnv->next;
   } else {
      converters = calloc(1, sizeof(Ecore_X_Selection_Converter));
      cnv = converters;
   }
   cnv->target = target;
   cnv->convert = func;
}


void
ecore_x_selection_converter_add(char *target, 
      int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret))
{
   Ecore_X_Atom x_target;
   char *atom_name;
   
   if (!func || !target)
      return;

   /* FIXME: Some of these are just made up because I can't find
    * standard "mime type" strings for them at the moment" */
   if (!strcmp(target, "TEXT"))
      x_target = _ecore_x_atom_text;
   else if (!strcmp(target, "COMPOUND_TEXT"))
      x_target = _ecore_x_atom_compound_text;
   else if (!strcmp(target, "STRING"))
      x_target = _ecore_x_atom_string;
   else if (!strcmp(target, "UTF8_STRING"))
      x_target = _ecore_x_atom_utf8_string;
   else if (!strcmp(target, "FILENAME"))
      x_target = _ecore_x_atom_file_name;
   else
   {
      atom_name = malloc(strlen(target) + 4);
      sprintf(atom_name, "_E_%s", target);
      x_target = XInternAtom(_ecore_x_disp, atom_name, False);
   }

   ecore_x_selection_converter_atom_add(x_target, func);
}

void
ecore_x_selection_converter_atom_del(Ecore_X_Atom target)
{
   Ecore_X_Selection_Converter *cnv, *prev_cnv;
   
   prev_cnv = NULL;
   cnv = converters;
   
   while (cnv)
   {
      if (cnv->target == target)
      {
         if (target == _ecore_x_atom_text ||
             target == _ecore_x_atom_compound_text ||
             target == _ecore_x_atom_string)
         {
            cnv->convert = _ecore_x_selection_converter_text;
         }
         else
         {
            if(prev_cnv)
               prev_cnv->next = cnv->next;
            else
               converters = NULL; /* This was the only converter */
            free(cnv);
         }
         
         return;
      }
      prev_cnv = cnv;
      cnv = cnv->next;
   }
}

void
ecore_x_selection_converter_del(char *target)
{
   Ecore_X_Atom x_target;
   char *atom_name;
   
   if (!target)
      return;
   
   if (!strcmp(target, "TEXT"))
      x_target = _ecore_x_atom_text;
   else if (!strcmp(target, "COMPOUND_TEXT"))
      x_target = _ecore_x_atom_compound_text;
   else if (!strcmp(target, "STRING"))
      x_target = _ecore_x_atom_string;
   else if (!strcmp(target, "UTF8_STRING"))
      x_target = _ecore_x_atom_utf8_string;
   else if (!strcmp(target, "FILENAME"))
      x_target = _ecore_x_atom_file_name;
   else
   {
      atom_name = malloc(strlen(target) + 4);
      sprintf(atom_name, "_E_%s", target);
      x_target = XInternAtom(_ecore_x_disp, atom_name, False);
   }

   ecore_x_selection_converter_atom_del(x_target);
}


/* Locate and run conversion callback for specified selection target */
int
_ecore_x_selection_convert(Ecore_X_Atom selection, Ecore_X_Atom target, void **data_ret)
{
   Ecore_X_Selection_Data *sel;
   Ecore_X_Selection_Converter *cnv;
   void *data;
   int size;
   char *tgt_str;
   
   sel = _ecore_x_selection_get(selection);
   /* COMPOUND_TEXT will be the default format for text requests */
   if (target == _ecore_x_atom_text)
      tgt_str = strdup("TEXT");
   else if (target == _ecore_x_atom_compound_text)
      tgt_str = strdup("COMPOUND_TEXT");
   else if (target == _ecore_x_atom_string)
      tgt_str = strdup("STRING");
   else if (target == _ecore_x_atom_utf8_string)
      tgt_str = strdup("UTF8_STRING");
   else if (target == _ecore_x_atom_file_name)
      tgt_str = strdup("FILENAME");
   else
   {
      char *atom_name = XGetAtomName(_ecore_x_disp, target);
      tgt_str = strdup(atom_name);
      XFree(atom_name);
   }
   
   for (cnv = converters; cnv; cnv = cnv->next)
   {
      if (cnv->target == target)
      {
         int r;
         r = cnv->convert(tgt_str, sel->data, sel->length, &data, &size);
         if (r)
         {
            *data_ret = data;
            return r;
         }
         else
            return -1;
      }
   }

   return -1;
}

/* Converter for standard non-utf8 text targets */
static int _ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret)
{
   XTextProperty text_prop;
   char *mystr;
   XICCEncodingStyle style;
   
   if (!data || !size)
      return 0;

   if (!strcmp(target, "TEXT"))
      style = XTextStyle;
   else if (!strcmp(target, "COMPOUND_TEXT"))
      style = XCompoundTextStyle;
   else if (!strcmp(target, "STRING"))
      style = XStringStyle;
   else
      return 0;
   
   mystr = strdup(data);
   
   if (XmbTextListToTextProperty(_ecore_x_disp, &mystr, 1, style, &text_prop) == Success)
   {
      int bufsize = strlen(text_prop.value) + 1;
      *data_ret = malloc(bufsize);
      memcpy(*data_ret, text_prop.value, bufsize);
      *size_ret = bufsize;
      XFree(text_prop.value);
      return 1;
   }
   else
      return 0;
}

