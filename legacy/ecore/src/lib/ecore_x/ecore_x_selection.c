/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static Ecore_X_Selection_Data selections[4];
static Ecore_X_Selection_Data request_data[4];
static Ecore_X_Selection_Converter *converters = NULL;

static int _ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret);

void
_ecore_x_selection_data_init(void)
{
   /* Initialize global data */
   memset(selections, 0, sizeof(selections));
   memset(request_data, 0, sizeof(request_data));

   /* Initialize converters */
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_TEXT, 
					_ecore_x_selection_converter_text);
#ifdef X_HAVE_UTF8_STRING
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_UTF8_STRING, 
					_ecore_x_selection_converter_text);
#endif
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_COMPOUND_TEXT,
					_ecore_x_selection_converter_text);
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_STRING,
					_ecore_x_selection_converter_text);
}

void
_ecore_x_selection_shutdown(void)
{
   Ecore_X_Selection_Converter *cnv = converters, *tmp;

   if (!converters)
     return;

   /* free the selection converters */
   while (cnv)
     {	
	tmp = cnv->next;
	free(cnv);
	cnv = tmp;
     }

   converters = NULL;
}

static void
_ecore_x_selection_request_data_get(Ecore_X_Atom selection, void **buf, int *len)
{
   int i;
   char *data;
   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     i = 0;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     i = 1;
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     i = 2;
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     i = 3;
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

/**
 * Fetch the data returned by a PRIMARY selection request.
 * @param buf A pointer to hold the selection data
 * @param len The size of the data
 *
 * Get the converted data from a previous PRIMARY selection
 * request. The buffer must be freed when done with.
 */
void
ecore_x_selection_primary_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(ECORE_X_ATOM_SELECTION_PRIMARY,
                                       buf, len);
}

/**
 * Fetch the data returned by a SECONDARY selection request.
 * @param buf A pointer to hold the selection data
 * @param len The size of the data
 *
 * Get the converted data from a previous SECONDARY selection
 * request. The buffer must be freed when done with.
 */
void
ecore_x_selection_secondary_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(ECORE_X_ATOM_SELECTION_SECONDARY,
                                       buf, len);
}

/**
 * Fetch the data returned by a XDND selection request.
 * @param buf A pointer to hold the selection data
 * @param len The size of the data
 *
 * Get the converted data from a previous XDND selection
 * request. The buffer must be freed when done with.
 */
void
ecore_x_selection_xdnd_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(ECORE_X_ATOM_SELECTION_XDND,
                                       buf, len);
}

/**
 * Fetch the data returned by a CLIPBOARD selection request.
 * @param buf A pointer to hold the selection data
 * @param len The size of the data
 *
 * Get the converted data from a previous CLIPBOARD selection
 * request. The buffer must be freed when done with.
 */
void
ecore_x_selection_clipboard_request_data_get(void **buf, int *len)
{
   _ecore_x_selection_request_data_get(ECORE_X_ATOM_SELECTION_CLIPBOARD,
                                       buf, len);
}

void 
_ecore_x_selection_request_data_set(Ecore_X_Selection_Data data)
{
   int i;
   if (data.selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     i = 0;
   else if (data.selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     i = 1;
   else if (data.selection == ECORE_X_ATOM_SELECTION_XDND)
     i = 2;
   else if (data.selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     i = 3;
   else
     return;

   request_data[i] = data;
}

Ecore_X_Selection_Data *
_ecore_x_selection_get(Atom selection)
{
   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     return &selections[0];
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     return &selections[1];
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     return &selections[2];
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     return &selections[3];
   else
     return NULL;
}

int 
_ecore_x_selection_set(Window w, unsigned char *data, int size, Atom selection)
{
   int in;
   unsigned char *buf = NULL;
   
   XSetSelectionOwner(_ecore_x_disp, selection, w, _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, selection) != w)
     return 0;
   
   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     in = 0;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     in = 1;
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     in = 2;
   else
     in = 3;

   if (data)
     {
	selections[in].win = w;
	selections[in].selection = selection;
	selections[in].length = size;
	selections[in].time = _ecore_x_event_last_time;

	buf = malloc(size);
	memcpy(buf, data, size);
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

   return 1;
}

/**
 * Claim ownership of the PRIMARY selection and set its data.
 * @param w    The window to which this selection belongs
 * @param data The data associated with the selection
 * @param size The size of the data buffer in bytes
 * @return     Returns 1 if the ownership of the selection was successfully 
 *             claimed, or 0 if unsuccessful.
 */
int 
ecore_x_selection_primary_set(Ecore_X_Window w, unsigned char *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_PRIMARY);
}

/**
 * Release ownership of the primary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
int 
ecore_x_selection_primary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, ECORE_X_ATOM_SELECTION_PRIMARY);
}

/**
 * Claim ownership of the SECONDARY selection and set its data.
 * @param w    The window to which this selection belongs
 * @param data The data associated with the selection
 * @param size The size of the data buffer in bytes
 * @return     Returns 1 if the ownership of the selection was successfully 
 *             claimed, or 0 if unsuccessful.
 */
int 
ecore_x_selection_secondary_set(Ecore_X_Window w, unsigned char *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Release ownership of the secondary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
int 
ecore_x_selection_secondary_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Claim ownership of the XDND selection and set its data.
 * @param w    The window to which this selection belongs
 * @param data The data associated with the selection
 * @param size The size of the data buffer in bytes
 * @return     Returns 1 if the ownership of the selection was successfully 
 *             claimed, or 0 if unsuccessful.
 */
int 
ecore_x_selection_xdnd_set(Ecore_X_Window w, unsigned char *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_XDND);
}

/**
 * Release ownership of the XDND selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
int 
ecore_x_selection_xdnd_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, ECORE_X_ATOM_SELECTION_XDND);
}

/**
 * Claim ownership of the CLIPBOARD selection and set its data.
 * @param w    The window to which this selection belongs
 * @param data The data associated with the selection
 * @param size The size of the data buffer in bytes
 * @return     Returns 1 if the ownership of the selection was successfully 
 *             claimed, or 0 if unsuccessful.
 *
 * Get the converted data from a previous CLIPBOARD selection
 * request. The buffer must be freed when done with.
 */
int 
ecore_x_selection_clipboard_set(Ecore_X_Window w, unsigned char *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

/**
 * Release ownership of the clipboard selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
int 
ecore_x_selection_clipboard_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

Atom
_ecore_x_selection_target_atom_get(char *target)
{
   Atom x_target;

   if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT))
     x_target = ECORE_X_ATOM_TEXT;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT))
     x_target = ECORE_X_ATOM_COMPOUND_TEXT;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
     x_target = ECORE_X_ATOM_STRING;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING))
     x_target = ECORE_X_ATOM_UTF8_STRING;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_FILENAME))
     x_target = ECORE_X_ATOM_FILE_NAME;
   else
     {
	char *atom_name;
	atom_name = malloc(strlen(target) + 4);
	sprintf(atom_name, "_E_%s", target);
	x_target = XInternAtom(_ecore_x_disp, atom_name, False);
	free(atom_name);
     }

   return x_target;
}

char *
_ecore_x_selection_target_get(Atom target)
{
   if (target == ECORE_X_ATOM_FILE_NAME)
     return strdup(ECORE_X_SELECTION_TARGET_FILENAME);
   else if (target == ECORE_X_ATOM_STRING)
     return strdup(ECORE_X_SELECTION_TARGET_STRING);
   else if (target == ECORE_X_ATOM_UTF8_STRING)
     return strdup(ECORE_X_SELECTION_TARGET_UTF8_STRING);
   else if (target == ECORE_X_ATOM_TEXT)
     return strdup(ECORE_X_SELECTION_TARGET_TEXT);
   else
     return NULL;
}

static void 
_ecore_x_selection_request(Ecore_X_Window w, Ecore_X_Atom selection, char *target_str) 
{
   Ecore_X_Atom target, prop;

   target = _ecore_x_selection_target_atom_get(target_str);

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     prop = ECORE_X_ATOM_SELECTION_PROP_PRIMARY;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     prop = ECORE_X_ATOM_SELECTION_PROP_SECONDARY;
   else
     prop = ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD;

   XConvertSelection(_ecore_x_disp, selection, target, prop,
		     w, _ecore_x_event_last_time);
}

void 
ecore_x_selection_primary_request(Ecore_X_Window w, char *target)
{
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_PRIMARY, target);
}

void 
ecore_x_selection_secondary_request(Ecore_X_Window w, char *target)
{
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_SECONDARY, target);
}

void 
ecore_x_selection_xdnd_request(Ecore_X_Window w, char *target)
{
   Ecore_X_Atom atom;

   atom = ecore_x_atom_get(target);
   XConvertSelection(_ecore_x_disp, ECORE_X_ATOM_SELECTION_XDND, atom,
		     ECORE_X_ATOM_SELECTION_PROP_XDND, w,
		     _ecore_x_event_last_time);
}

void 
ecore_x_selection_clipboard_request(Ecore_X_Window w, char *target)
{
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_CLIPBOARD, target);
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
     }
   else
     {
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

   if (!func || !target)
     return;

   x_target = _ecore_x_selection_target_atom_get(target);

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
	     if ((target == ECORE_X_ATOM_TEXT) ||
		 (target == ECORE_X_ATOM_COMPOUND_TEXT) ||
		 (target == ECORE_X_ATOM_STRING))
	       {
		  cnv->convert = _ecore_x_selection_converter_text;
	       }
	     else
	       {
		  if (prev_cnv)
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

   if (!target)
     return;

   x_target = _ecore_x_selection_target_atom_get(target);
   ecore_x_selection_converter_atom_del(x_target);
}


/* Locate and run conversion callback for specified selection target */
int
_ecore_x_selection_convert(Atom selection, Atom target, void **data_ret)
{
   Ecore_X_Selection_Data *sel;
   Ecore_X_Selection_Converter *cnv;
   void *data;
   int size;
   char *tgt_str;
   
   sel = _ecore_x_selection_get(selection);
   tgt_str = _ecore_x_selection_target_get(target);

   if (tgt_str)
     {
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
		    return 0;
	       }
	  }

	free(tgt_str);
     }
   else
     {
	*data_ret = malloc(sel->length);
	memcpy(*data_ret, sel->data, sel->length);
	return 1;
     }

   return 0;
}

/* TODO: We need to work out a mechanism for automatic conversion to any requested
 * locale using Ecore_Txt functions */
/* Converter for standard non-utf8 text targets */
static int
_ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret)
{
   XTextProperty text_prop;
   char *mystr;
   XICCEncodingStyle style;

   if (!data || !size)
     return 0;

   if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT))
     style = XTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT))
     style = XCompoundTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
     style = XStringStyle;
#ifdef X_HAVE_UTF8_STRING
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING))
     style = XUTF8StringStyle;
#endif
   else
     return 0;

   if (!(mystr = strdup(data)))
     return 0;

   if (XmbTextListToTextProperty(_ecore_x_disp, &mystr, 1, style, &text_prop) == Success)
     {
	int bufsize = strlen(text_prop.value) + 1;
	*data_ret = malloc(bufsize);
	memcpy(*data_ret, text_prop.value, bufsize);
	*size_ret = bufsize;
	XFree(text_prop.value);
	free(mystr);
	return 1;
     }
   else
     {
	free(mystr);
	return 0;
     }
}


