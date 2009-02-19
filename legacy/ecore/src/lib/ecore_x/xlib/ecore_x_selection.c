/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "ecore_private.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static Ecore_X_Selection_Intern selections[4];
static Ecore_X_Selection_Converter *converters = NULL;
static Ecore_X_Selection_Parser *parsers = NULL;

static int _ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret);
static int _ecore_x_selection_data_default_free(void *data);
static void *_ecore_x_selection_parser_files(const char *target, void *data, int size, int format);
static int _ecore_x_selection_data_files_free(void *data);
static void *_ecore_x_selection_parser_text(const char *target, void *data, int size, int format);
static int _ecore_x_selection_data_text_free(void *data);
static void *_ecore_x_selection_parser_targets(const char *target, void *data, int size, int format);
static int _ecore_x_selection_data_targets_free(void *data);

#define ECORE_X_SELECTION_DATA(x) ((Ecore_X_Selection_Data *)(x))

void
_ecore_x_selection_data_init(void)
{
   /* Initialize global data */
   memset(selections, 0, sizeof(selections));

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

   /* Initialize parsers */
   ecore_x_selection_parser_add("text/plain",
				_ecore_x_selection_parser_text);
   ecore_x_selection_parser_add(ECORE_X_SELECTION_TARGET_UTF8_STRING,
				_ecore_x_selection_parser_text);
   ecore_x_selection_parser_add("text/uri-list",
				_ecore_x_selection_parser_files);
   ecore_x_selection_parser_add("_NETSCAPE_URL",
				_ecore_x_selection_parser_files);
   ecore_x_selection_parser_add(ECORE_X_SELECTION_TARGET_TARGETS,
				_ecore_x_selection_parser_targets);
}

void
_ecore_x_selection_shutdown(void)
{
   Ecore_X_Selection_Converter *cnv;
   Ecore_X_Selection_Parser *prs;

   /* free the selection converters */
   cnv = converters;
   while (cnv)
     {
	Ecore_X_Selection_Converter *tmp;

	tmp = cnv->next;
	free(cnv);
	cnv = tmp;
     }
   converters = NULL;

   /* free the selection parsers */
   prs = parsers;
   while (prs)
     {
	Ecore_X_Selection_Parser *tmp;

	tmp = prs;
	prs = prs->next;
	free(tmp->target);
	free(tmp);
     }
   parsers = NULL;
}

Ecore_X_Selection_Intern *
_ecore_x_selection_get(Ecore_X_Atom selection)
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
_ecore_x_selection_set(Window w, const void *data, int size, Ecore_X_Atom selection)
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
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     in = 3;
   else
     return 0;

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
EAPI int 
ecore_x_selection_primary_set(Ecore_X_Window w, const void *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_PRIMARY);
}

/**
 * Release ownership of the primary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI int 
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
EAPI int 
ecore_x_selection_secondary_set(Ecore_X_Window w, const void *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Release ownership of the secondary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI int 
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
EAPI int 
ecore_x_selection_xdnd_set(Ecore_X_Window w, const void *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_XDND);
}

/**
 * Release ownership of the XDND selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI int 
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
EAPI int 
ecore_x_selection_clipboard_set(Ecore_X_Window w, const void *data, int size)
{
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

/**
 * Release ownership of the clipboard selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI int 
ecore_x_selection_clipboard_clear(void)
{
   return _ecore_x_selection_set(None, NULL, 0, ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

Ecore_X_Atom
_ecore_x_selection_target_atom_get(const char *target)
{
   Ecore_X_Atom x_target;

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
	x_target = ecore_x_atom_get(target);
     }

   return x_target;
}

char *
_ecore_x_selection_target_get(Ecore_X_Atom target)
{
   /* FIXME: Should not return mem allocated with strdup or X mixed,
    * one should use free to free, the other XFree */
   if (target == ECORE_X_ATOM_FILE_NAME)
     return strdup(ECORE_X_SELECTION_TARGET_FILENAME);
   else if (target == ECORE_X_ATOM_STRING)
     return strdup(ECORE_X_SELECTION_TARGET_STRING);
   else if (target == ECORE_X_ATOM_UTF8_STRING)
     return strdup(ECORE_X_SELECTION_TARGET_UTF8_STRING);
   else if (target == ECORE_X_ATOM_TEXT)
     return strdup(ECORE_X_SELECTION_TARGET_TEXT);
   else
     return XGetAtomName(_ecore_x_disp, target);
}

static void 
_ecore_x_selection_request(Ecore_X_Window w, Ecore_X_Atom selection, const char *target_str) 
{
   Ecore_X_Atom target, prop;

   target = _ecore_x_selection_target_atom_get(target_str);

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     prop = ECORE_X_ATOM_SELECTION_PROP_PRIMARY;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     prop = ECORE_X_ATOM_SELECTION_PROP_SECONDARY;
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     prop = ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD;
   else
     return;

   XConvertSelection(_ecore_x_disp, selection, target, prop,
		     w, CurrentTime);
}

EAPI void 
ecore_x_selection_primary_request(Ecore_X_Window w, const char *target)
{
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_PRIMARY, target);
}

EAPI void 
ecore_x_selection_secondary_request(Ecore_X_Window w, const char *target)
{
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_SECONDARY, target);
}

EAPI void 
ecore_x_selection_xdnd_request(Ecore_X_Window w, const char *target)
{
   Ecore_X_Atom atom;
   Ecore_X_DND_Target *_target;

   _target = _ecore_x_dnd_target_get();
   atom = _ecore_x_selection_target_atom_get(target);
   XConvertSelection(_ecore_x_disp, ECORE_X_ATOM_SELECTION_XDND, atom,
		     ECORE_X_ATOM_SELECTION_PROP_XDND, w,
		     _target->time);
}

EAPI void 
ecore_x_selection_clipboard_request(Ecore_X_Window w, const char *target)
{
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_CLIPBOARD, target);
}

EAPI void
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

EAPI void
ecore_x_selection_converter_add(char *target, 
      int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret))
{
   Ecore_X_Atom x_target;

   if (!func || !target)
     return;

   x_target = _ecore_x_selection_target_atom_get(target);

   ecore_x_selection_converter_atom_add(x_target, func);
}

EAPI void
ecore_x_selection_converter_atom_del(Ecore_X_Atom target)
{
   Ecore_X_Selection_Converter *cnv, *prev_cnv;

   prev_cnv = NULL;
   cnv = converters;

   while (cnv)
     {
	if (cnv->target == target)
	  {
	     if (prev_cnv)
	       prev_cnv->next = cnv->next;
	     else
	       converters = cnv->next; /* This was the first converter */
	     free(cnv);

	     return;
	  }
	prev_cnv = cnv;
	cnv = cnv->next;
     }
}

EAPI void
ecore_x_selection_converter_del(char *target)
{
   Ecore_X_Atom x_target;

   if (!target)
     return;

   x_target = _ecore_x_selection_target_atom_get(target);
   ecore_x_selection_converter_atom_del(x_target);
}

EAPI int
ecore_x_selection_notify_send(Ecore_X_Window requestor, Ecore_X_Atom selection, Ecore_X_Atom target, Ecore_X_Atom property, Ecore_X_Time time)
{
   XEvent          xev;
   XSelectionEvent xnotify;

   xnotify.type = SelectionNotify;
   xnotify.display = _ecore_x_disp;
   xnotify.requestor = requestor;
   xnotify.selection = selection;
   xnotify.target = target;
   xnotify.property = property;
   xnotify.time = time;
   xnotify.send_event = True;
   xnotify.serial = 0;

   xev.xselection = xnotify;
   return ((XSendEvent(_ecore_x_disp, requestor, False, 0, &xev) > 0) ? 1 : 0);
}

/* Locate and run conversion callback for specified selection target */
EAPI int
ecore_x_selection_convert(Ecore_X_Atom selection, Ecore_X_Atom target, void **data_ret)
{
   Ecore_X_Selection_Intern *sel;
   Ecore_X_Selection_Converter *cnv;
   void *data;
   int size;
   char *tgt_str;
   
   sel = _ecore_x_selection_get(selection);
   tgt_str = _ecore_x_selection_target_get(target);

   for (cnv = converters; cnv; cnv = cnv->next)
     {
	if (cnv->target == target)
	  {
	     int r;
	     r = cnv->convert(tgt_str, sel->data, sel->length, &data, &size);
	     free(tgt_str);
	     if (r)
	       {
		  *data_ret = data;
		  return r;
	       }
	     else
	       return 0;
	  }
     }

   /* ICCCM says "If the selection cannot be converted into a form based on the target (and parameters, if any), the owner should refuse the SelectionRequest as previously described." */
   return 0; 

   /* Default, just return the data 
   *data_ret = malloc(sel->length);
   memcpy(*data_ret, sel->data, sel->length);
   free(tgt_str);
   return 1;
   */
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

#ifdef X_HAVE_UTF8_STRING
   if (Xutf8TextListToTextProperty(_ecore_x_disp, &mystr, 1, style, &text_prop) == Success)
     {
	int bufsize = strlen((char *)text_prop.value) + 1;
	*data_ret = malloc(bufsize);
	memcpy(*data_ret, text_prop.value, bufsize);
	*size_ret = bufsize;
	XFree(text_prop.value);
	free(mystr);
	return 1;
     }
#else
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
#endif
   else
     {
	free(mystr);
	return 0;
     }
}

EAPI void
ecore_x_selection_parser_add(const char *target,
      void *(*func)(const char *target, void *data, int size, int format))
{
   Ecore_X_Selection_Parser *prs;

   if (!target)
     return;

   prs = parsers;
   if (parsers) 
     {
	while (prs->next)
	  {
	     if (!strcmp(prs->target, target))
	       {
		  prs->parse = func;
		  return;
	       }
	     prs = prs->next;
	  }

	prs->next = calloc(1, sizeof(Ecore_X_Selection_Parser));
	prs = prs->next;
     }
   else
     {
	parsers = calloc(1, sizeof(Ecore_X_Selection_Parser));
	prs = parsers;
     }
   prs->target = strdup(target);
   prs->parse = func;
}

EAPI void
ecore_x_selection_parser_del(const char *target)
{
   Ecore_X_Selection_Parser *prs, *prev_prs;

   if (!target)
     return;

   prev_prs = NULL;
   prs = parsers;

   while (prs)
     {
	if (!strcmp(prs->target, target))
	  {
	     if (prev_prs)
	       prev_prs->next = prs->next;
	     else
	       parsers = prs->next; /* This was the first parser */
	     free(prs->target);
	     free(prs);

	     return;
	  }
	prev_prs = prs;
	prs = prs->next;
     }
}

/* Locate and run conversion callback for specified selection target */
void *
_ecore_x_selection_parse(const char *target, void *data, int size, int format)
{
   Ecore_X_Selection_Parser *prs;
   Ecore_X_Selection_Data *sel;
   
   for (prs = parsers; prs; prs = prs->next)
     {
	if (!strcmp(prs->target, target))
	  {
	     sel = prs->parse(target, data, size, format);
	     return sel;
	  }
     }

   /* Default, just return the data */
   sel = calloc(1, sizeof(Ecore_X_Selection_Data));
   sel->free = _ecore_x_selection_data_default_free;
   sel->length = size;
   sel->format = format;
   sel->data = data;
   return sel;
}

static int
_ecore_x_selection_data_default_free(void *data)
{
   Ecore_X_Selection_Data *sel;

   sel = data;
   free(sel->data);
   free(sel);
   return 1;
}

static void *
_ecore_x_selection_parser_files(const char *target, void *_data, int size, int format __UNUSED__)
{
   Ecore_X_Selection_Data_Files *sel;
   char *data = _data;
   int i, is;
   char *tmp;

   if (strcmp(target, "text/uri-list") &&
       strcmp(target, "_NETSCAPE_URL"))
     return NULL;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Files));
   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_files_free;

   if (data[size - 1])
     {
	/* Isn't nul terminated */
	size++;
	data = realloc(data, size);
	data[size - 1] = 0;
     }

   tmp = malloc(size);
   i = 0;
   is = 0;
   while ((is < size) && (data[is]))
     {
	if ((i == 0) && (data[is] == '#'))
	  {
	     for (; ((data[is]) && (data[is] != '\n')); is++);
	  }
	else
	  {
	     if ((data[is] != '\r') &&
		 (data[is] != '\n'))
	       {
		  tmp[i++] = data[is++];
	       }
	     else
	       {
		  while ((data[is] == '\r') || (data[is] == '\n')) is++;
		  tmp[i] = 0;
		  sel->num_files++;
		  sel->files = realloc(sel->files, sel->num_files * sizeof(char *));
		  sel->files[sel->num_files - 1] = strdup(tmp);
		  tmp[0] = 0;
		  i = 0;
	       }
	  }
     }
   if (i > 0)
     {
	tmp[i] = 0;
	sel->num_files++;
	sel->files = realloc(sel->files, sel->num_files * sizeof(char *));
	sel->files[sel->num_files - 1] = strdup(tmp);
     }
   free(tmp);
   free(data);

   ECORE_X_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_FILES;
   ECORE_X_SELECTION_DATA(sel)->length = sel->num_files;

   return ECORE_X_SELECTION_DATA(sel);
}

static int
_ecore_x_selection_data_files_free(void *data)
{
   Ecore_X_Selection_Data_Files *sel;
   int i;

   sel = data;
   if (sel->files)
     {
	for (i = 0; i < sel->num_files; i++)
	  free(sel->files[i]);
	free(sel->files);
     }
   free(sel);
   return 0;
}

static void *
_ecore_x_selection_parser_text(const char *target __UNUSED__, void *_data, int size, int format __UNUSED__)
{
   Ecore_X_Selection_Data_Text *sel;
   char *data = _data;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Text));

   if (data[size - 1])
     {
	/* Isn't nul terminated */
	size++;
	data = realloc(data, size);
	data[size - 1] = 0;
     }

   sel->text = (char *)data;
   ECORE_X_SELECTION_DATA(sel)->length = size;
   ECORE_X_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_TEXT;
   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_text_free;
   return sel;
}

static int
_ecore_x_selection_data_text_free(void *data)
{
   Ecore_X_Selection_Data_Text *sel;

   sel = data;
   free(sel->text);
   free(sel);
   return 1;
}

static void *
_ecore_x_selection_parser_targets(const char *target __UNUSED__, void *data, int size, int format __UNUSED__)
{
   Ecore_X_Selection_Data_Targets *sel;
   unsigned long *targets;
   int i;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Targets));
   targets = (unsigned long *)data;

   sel->num_targets = size - 2;
   sel->targets = malloc((size - 2) * sizeof(char *));
   for (i = 2; i < size; i++)
     sel->targets[i - 2] = XGetAtomName(_ecore_x_disp, targets[i]);
   free(data);

   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_targets_free;
   ECORE_X_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_TARGETS;
   ECORE_X_SELECTION_DATA(sel)->length = size;
   return sel;
}

static int
_ecore_x_selection_data_targets_free(void *data)
{
   Ecore_X_Selection_Data_Targets *sel;
   int i;

   sel = data;

   if (sel->targets)
     {
	for (i = 0; i < sel->num_targets; i++)
	  XFree(sel->targets[i]);
	free(sel->targets);
     }
   free(sel);
   return 1;
}
