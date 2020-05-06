#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdlib.h>
#include <string.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static Ecore_X_Selection_Intern selections[4];
static Ecore_X_Selection_Converter *converters = NULL;
static Ecore_X_Selection_Parser *parsers = NULL;

static int   _ecore_x_selection_data_default_free(void *data);
static void *_ecore_x_selection_parser_files(const char *target,
                                             void *data,
                                             int size,
                                             int format);
static int   _ecore_x_selection_data_files_free(void *data);
static void *_ecore_x_selection_parser_text(const char *target,
                                            void *data,
                                            int size,
                                            int format);
static void *_ecore_x_selection_parser_xmozurl(const char *target,
                                            void *data,
                                            int size,
                                            int format);
static int   _ecore_x_selection_data_text_free(void *data);
static void *_ecore_x_selection_parser_targets(const char *target,
                                               void *data,
                                               int size,
                                               int format);
static int   _ecore_x_selection_data_targets_free(void *data);

#define ECORE_X_SELECTION_DATA(x) ((Ecore_X_Selection_Data *)(x))

void
_ecore_x_selection_data_init(void)
{
   /* Initialize global data */
   memset(selections, 0, sizeof(selections));

   /* Initialize converters */
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_TEXT,
                                        ecore_x_selection_converter_text);
#ifdef X_HAVE_UTF8_STRING
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_UTF8_STRING,
                                        ecore_x_selection_converter_text);
#endif /* ifdef X_HAVE_UTF8_STRING */
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_COMPOUND_TEXT,
                                        ecore_x_selection_converter_text);
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_STRING,
                                        ecore_x_selection_converter_text);

   /* Initialize parsers */
   ecore_x_selection_parser_add("text/plain",
                                _ecore_x_selection_parser_text);
   ecore_x_selection_parser_add(ECORE_X_SELECTION_TARGET_UTF8_STRING,
                                _ecore_x_selection_parser_text);
   ecore_x_selection_parser_add("text/uri-list",
                                _ecore_x_selection_parser_files);
   ecore_x_selection_parser_add("text/x-moz-url",
                                _ecore_x_selection_parser_xmozurl);
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
   Eina_Inlist *inlist;

   /* free the selection converters */
   EINA_INLIST_FOREACH_SAFE(converters, inlist, cnv)
      free(cnv);
   converters = NULL;

   /* free the selection parsers */
   EINA_INLIST_FOREACH_SAFE(parsers, inlist, prs)
     {
        free(prs->target);
        free(prs);
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

Eina_Bool
_ecore_x_selection_set(Window w,
                       const void *data,
                       int size,
                       Ecore_X_Atom selection)
{
   int in;
   unsigned char *buf = NULL;

   XSetSelectionOwner(_ecore_x_disp, selection, w, _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, selection) != w)
     return EINA_FALSE;

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     in = 0;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     in = 1;
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     in = 2;
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     in = 3;
   else
     return EINA_FALSE;

   if (selections[in].data)
     {
        free(selections[in].data);
        memset(&selections[in], 0, sizeof(Ecore_X_Selection_Intern));
     }

   if (data)
     {
        selections[in].win = w;
        selections[in].selection = selection;
        selections[in].length = size;
        selections[in].time = _ecore_x_event_last_time;

        buf = malloc(size);
        if (!buf) return EINA_FALSE;
        memcpy(buf, data, size);
        selections[in].data = buf;
     }

   return EINA_TRUE;
}

/**
 * Claim ownership of the PRIMARY selection and set its data.
 * @param w    The window to which this selection belongs
 * @param data The data associated with the selection
 * @param size The size of the data buffer in bytes
 * @return     Returns 1 if the ownership of the selection was successfully
 *             claimed, or 0 if unsuccessful.
 */
EAPI Eina_Bool
ecore_x_selection_primary_set(Ecore_X_Window w,
                              const void *data,
                              int size)
{
   LOGFN;
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_PRIMARY);
}

/**
 * Release ownership of the primary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI Eina_Bool
ecore_x_selection_primary_clear(void)
{
   LOGFN;
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
EAPI Eina_Bool
ecore_x_selection_secondary_set(Ecore_X_Window w,
                                const void *data,
                                int size)
{
   LOGFN;
   return _ecore_x_selection_set(w,
                                 data,
                                 size,
                                 ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Release ownership of the secondary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI Eina_Bool
ecore_x_selection_secondary_clear(void)
{
   LOGFN;
   return _ecore_x_selection_set(None,
                                 NULL,
                                 0,
                                 ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Claim ownership of the XDND selection and set its data.
 * @param w    The window to which this selection belongs
 * @param data The data associated with the selection
 * @param size The size of the data buffer in bytes
 * @return     Returns 1 if the ownership of the selection was successfully
 *             claimed, or 0 if unsuccessful.
 */
EAPI Eina_Bool
ecore_x_selection_xdnd_set(Ecore_X_Window w,
                           const void *data,
                           int size)
{
   LOGFN;
   return _ecore_x_selection_set(w, data, size, ECORE_X_ATOM_SELECTION_XDND);
}

/**
 * Release ownership of the XDND selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI Eina_Bool
ecore_x_selection_xdnd_clear(void)
{
   LOGFN;
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
EAPI Eina_Bool
ecore_x_selection_clipboard_set(Ecore_X_Window w,
                                const void *data,
                                int size)
{
   LOGFN;
   return _ecore_x_selection_set(w,
                                 data,
                                 size,
                                 ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

/**
 * Release ownership of the clipboard selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 */
EAPI Eina_Bool
ecore_x_selection_clipboard_clear(void)
{
   LOGFN;
   return _ecore_x_selection_set(None,
                                 NULL,
                                 0,
                                 ECORE_X_ATOM_SELECTION_CLIPBOARD);
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
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_X_MOZ_URL))
     x_target = ECORE_X_ATOM_X_MOZ_URL;
   else
     x_target = ecore_x_atom_get(target);

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
   else if (target == ECORE_X_ATOM_X_MOZ_URL)
     return strdup(ECORE_X_SELECTION_TARGET_X_MOZ_URL);
   else
     return XGetAtomName(_ecore_x_disp, target);
}

static void
_ecore_x_selection_request(Ecore_X_Window w,
                           Ecore_X_Atom selection,
                           const char *target_str)
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
ecore_x_selection_primary_request(Ecore_X_Window w,
                                  const char *target)
{
   LOGFN;
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_PRIMARY, target);
}

EAPI void
ecore_x_selection_secondary_request(Ecore_X_Window w,
                                    const char *target)
{
   LOGFN;
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_SECONDARY, target);
}

EAPI void
ecore_x_selection_xdnd_request(Ecore_X_Window w,
                               const char *target)
{
   Ecore_X_Atom atom;
   Ecore_X_DND_Target *_target;

   LOGFN;
   _target = _ecore_x_dnd_target_get();
   atom = _ecore_x_selection_target_atom_get(target);
   XConvertSelection(_ecore_x_disp, ECORE_X_ATOM_SELECTION_XDND, atom,
                     ECORE_X_ATOM_SELECTION_PROP_XDND, w,
                     _target->time);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_selection_clipboard_request(Ecore_X_Window w,
                                    const char *target)
{
   LOGFN;
   _ecore_x_selection_request(w, ECORE_X_ATOM_SELECTION_CLIPBOARD, target);
}

EAPI void
ecore_x_selection_converter_atom_add(Ecore_X_Atom target,
                                     Eina_Bool (*func)(char *target,
                                                       void *data,
                                                       int size,
                                                       void **data_ret,
                                                       int *size_ret,
                                                       Ecore_X_Atom *ttype,
                                                       int *tsize))
{
   Ecore_X_Selection_Converter *cnv;

   LOGFN;

   EINA_INLIST_FOREACH(converters, cnv)
      if (cnv->target == target)
        {
           cnv->convert = func;
           return;
        }

   cnv = calloc(1, sizeof(Ecore_X_Selection_Converter));
   if (!cnv) return;

   cnv->target = target;
   cnv->convert = func;
   converters = (Ecore_X_Selection_Converter *)eina_inlist_append
      (EINA_INLIST_GET(converters), EINA_INLIST_GET(cnv));
}

EAPI void
ecore_x_selection_converter_add(char *target,
                                Eina_Bool (*func)(char *target,
                                                  void *data,
                                                  int size,
                                                  void **data_ret,
                                                  int *size_ret,
                                                  Ecore_X_Atom *,
                                                  int *))
{
   Ecore_X_Atom x_target;

   if (!func || !target)
     return;

   LOGFN;
   x_target = _ecore_x_selection_target_atom_get(target);

   ecore_x_selection_converter_atom_add(x_target, func);
}

EAPI void
ecore_x_selection_converter_atom_del(Ecore_X_Atom target)
{
   Ecore_X_Selection_Converter *cnv;

   LOGFN;

   EINA_INLIST_FOREACH(converters, cnv)
     {
        if (cnv->target == target)
          {
             converters = (Ecore_X_Selection_Converter *)eina_inlist_remove
                (EINA_INLIST_GET(converters), EINA_INLIST_GET(cnv));
             free(cnv);
             return;
          }
     }
}

EAPI void
ecore_x_selection_converter_del(char *target)
{
   Ecore_X_Atom x_target;

   if (!target)
     return;

   LOGFN;
   x_target = _ecore_x_selection_target_atom_get(target);
   ecore_x_selection_converter_atom_del(x_target);
}

EAPI Eina_Bool
ecore_x_selection_notify_send(Ecore_X_Window requestor,
                              Ecore_X_Atom selection,
                              Ecore_X_Atom target,
                              Ecore_X_Atom property,
                              Ecore_X_Time tim)
{
   XEvent xev = { 0 };
   XSelectionEvent xnotify;

   LOGFN;
   xnotify.type = SelectionNotify;
   xnotify.display = _ecore_x_disp;
   xnotify.requestor = requestor;
   xnotify.selection = selection;
   xnotify.target = target;
   xnotify.property = property;
   xnotify.time = tim;
   xnotify.send_event = True;
   xnotify.serial = 0;

   xev.xselection = xnotify;
   return (XSendEvent(_ecore_x_disp, requestor, False, 0, &xev) > 0) ? EINA_TRUE : EINA_FALSE;
}

/* Locate and run conversion callback for specified selection target */
EAPI Eina_Bool
ecore_x_selection_convert(Ecore_X_Atom selection,
                          Ecore_X_Atom target,
                          void **data_ret,
                          int *size,
                          Ecore_X_Atom *targtype,
                          int *typesize)
{
   Ecore_X_Selection_Intern *sel;
   Ecore_X_Selection_Converter *cnv;
   void *data = NULL;
   char *tgt_str;

   LOGFN;
   sel = _ecore_x_selection_get(selection);
   tgt_str = _ecore_x_selection_target_get(target);

   EINA_INLIST_FOREACH(converters, cnv)
     {
        if (cnv->target == target)
          {
             int r;
             r = cnv->convert(tgt_str, sel->data, sel->length, &data, size,
                              targtype, typesize);
             free(tgt_str);
             if (r)
               {
                  if (data_ret) *data_ret = data;
                  return r;
               }
             else
               return EINA_FALSE;
          }
     }
   free(tgt_str);

   /* ICCCM says "If the selection cannot be converted into a form based on the target (and parameters, if any), the owner should refuse the SelectionRequest as previously described." */
   return EINA_FALSE;

   /* Default, just return the data
    * data_ret = malloc(sel->length);
      memcpy(*data_ret, sel->data, sel->length);
      free(tgt_str);
      return 1;
    */
}

/* TODO: We need to work out a mechanism for automatic conversion to any requested
 * locale using Ecore_Txt functions */
/* Converter for standard non-utf8 text targets */
EAPI Eina_Bool
ecore_x_selection_converter_text(char *target,
                                 void *data,
                                 int size,
                                 void **data_ret,
                                 int *size_ret,
                                 Ecore_X_Atom *targprop EINA_UNUSED,
                                 int *s EINA_UNUSED)
{
   XTextProperty text_prop;
   char *mystr;
   XICCEncodingStyle style;

   if (!data || !size)
     return EINA_FALSE;

   LOGFN;
   if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT))
     style = XTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT))
     style = XCompoundTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
     style = XStringStyle;

#ifdef X_HAVE_UTF8_STRING
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING))
     style = XUTF8StringStyle;
#endif /* ifdef X_HAVE_UTF8_STRING */
   else
     return EINA_FALSE;

   mystr = alloca(size + 1);
   memcpy(mystr, data, size);
   mystr[size] = '\0';

#ifdef X_HAVE_UTF8_STRING
   if (Xutf8TextListToTextProperty(_ecore_x_disp, &mystr, 1, style,
                                   &text_prop) == Success)
     {
        int bufsize = strlen((char *)text_prop.value);
        char *str = malloc(bufsize + 1);
        if (!str) return EINA_FALSE;
        *data_ret = str;
        memcpy(str, text_prop.value, bufsize);
        str[bufsize] = 0;
        *size_ret = bufsize;
        XFree(text_prop.value);
        return EINA_TRUE;
     }

#else /* ifdef X_HAVE_UTF8_STRING */
   if (XmbTextListToTextProperty(_ecore_x_disp, &mystr, 1, style,
                                 &text_prop) == Success)
     {
        int bufsize = strlen(text_prop.value);
        *data_ret = malloc(bufsize);
        if (!*data_ret) return EINA_FALSE;
        memcpy(*data_ret, text_prop.value, bufsize);
        *size_ret = bufsize;
        XFree(text_prop.value);
        return EINA_TRUE;
     }

#endif /* ifdef X_HAVE_UTF8_STRING */
   else
     {
        return EINA_TRUE;
     }
}

EAPI void
ecore_x_selection_parser_add(const char *target,
                             void *(*func)(const char *target, void *data,
                                           int size,
                                           int format))
{
   Ecore_X_Selection_Parser *prs;

   if (!target)
     return;

   LOGFN;

   EINA_INLIST_FOREACH(parsers, prs)
      if (!strcmp(prs->target, target))
        {
           prs->parse = func;
           return;
        }

   prs = calloc(1, sizeof(Ecore_X_Selection_Parser));
   if (!prs) return;

   prs->target = strdup(target);
   prs->parse = func;

   parsers = (Ecore_X_Selection_Parser *)eina_inlist_append
      (EINA_INLIST_GET(parsers), EINA_INLIST_GET(prs));
}

EAPI void
ecore_x_selection_parser_del(const char *target)
{
   Ecore_X_Selection_Parser *prs;

   if (!target)
     return;

   LOGFN;

   EINA_INLIST_FOREACH(parsers, prs)
     {
        if (!strcmp(prs->target, target))
          {
             parsers = (Ecore_X_Selection_Parser *)eina_inlist_remove
                (EINA_INLIST_GET(parsers), EINA_INLIST_GET(prs));
             free(prs->target);
             free(prs);
             return;
          }
     }
}

/**
 * Change the owner and last-change time for the specified selection.
 * @param win The owner of the specified atom.
 * @param atom The selection atom
 * @param tim Specifies the time
 * @since 1.1.0
 */
EAPI void
ecore_x_selection_owner_set(Ecore_X_Window win,
                            Ecore_X_Atom atom,
                            Ecore_X_Time tim)
{
   XSetSelectionOwner(_ecore_x_disp, atom, win, tim);
}

/**
 * Return the window that currently owns the specified selection.
 *
 * @param atom The specified selection atom.
 *
 * @return The window that currently owns the specified selection.
 * @since 1.1.0
 */
EAPI Ecore_X_Window
ecore_x_selection_owner_get(Ecore_X_Atom atom)
{
   return XGetSelectionOwner(_ecore_x_disp, atom);
}

/* Locate and run conversion callback for specified selection target */
void *
_ecore_x_selection_parse(const char *target,
                         void *data,
                         int size,
                         int format)
{
   Ecore_X_Selection_Parser *prs;
   Ecore_X_Selection_Data *sel;

   EINA_INLIST_FOREACH(parsers, prs)
     {
        if (!strcmp(prs->target, target))
          {
             sel = prs->parse(target, data, size, format);
             if (sel) return sel;
          }
     }

   /* Default, just return the data */
   sel = calloc(1, sizeof(Ecore_X_Selection_Data));
   if (!sel) return NULL;
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
_ecore_x_selection_parser_files(const char *target,
                                void *_data,
                                int size,
                                int format EINA_UNUSED)
{
   Ecore_X_Selection_Data_Files *sel;
   char *data = _data;

   if (strcmp(target, "text/uri-list") &&
       strcmp(target, "_NETSCAPE_URL"))
     return NULL;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Files));
   if (!sel) return NULL;
   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_files_free;

   if (data && (size > 0))
     {
        int i, is;
        char *tmp;
        char **t2;

        if (data[size - 1])
          {
             char *t;

             /* Isn't nul terminated */
             size++;
             t = realloc(data, size);
             if (!t) goto done;
             data = t;
             data[size - 1] = 0;
          }

        tmp = malloc(size);
        if (!tmp) goto done;
        i = 0;
        is = 0;
        while ((is < size) && (data[is]))
          {
             if ((i == 0) && (data[is] == '#'))
               for (; ((data[is]) && (data[is] != '\n')); is++) ;
             else
               {
                  if ((data[is] != '\r') &&
                      (data[is] != '\n'))
                    tmp[i++] = data[is++];
                  else
                    {
                       while ((data[is] == '\r') || (data[is] == '\n'))
                         is++;
                       tmp[i] = 0;
                       sel->num_files++;
                       t2 = realloc(sel->files, sel->num_files * sizeof(char *));
                       if (t2)
                         {
                            sel->files = t2;
                            sel->files[sel->num_files - 1] = strdup(tmp);
                         }
                       tmp[0] = 0;
                       i = 0;
                    }
               }
          }
        if (i > 0)
          {
             tmp[i] = 0;
             sel->num_files++;
             t2 = realloc(sel->files, sel->num_files * sizeof(char *));
             if (t2)
               {
                  sel->files = t2;
                  sel->files[sel->num_files - 1] = strdup(tmp);
               }
          }

        free(tmp);
     }
done:
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
_ecore_x_selection_parser_text(const char *target EINA_UNUSED,
                               void *_data,
                               int size,
                               int format EINA_UNUSED)
{
   Ecore_X_Selection_Data_Text *sel;
   unsigned char *data = _data;
   void *t;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Text));
   if (!sel) return NULL;
   if (data && data[size - 1])
     {
        /* Isn't nul terminated */
        size++;
        t = realloc(data, size);
        if (!t)
          {
             free(sel);
             return NULL;
          }
        data = t;
        data[size - 1] = 0;
     }

   sel->text = (char *)data;
   ECORE_X_SELECTION_DATA(sel)->length = size;
   ECORE_X_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_TEXT;
   ECORE_X_SELECTION_DATA(sel)->data = data;
   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_text_free;
   return sel;
}

static int
_ecore_x_selection_data_xmozurl_free(void *data)
{
   Ecore_X_Selection_Data_X_Moz_Url *sel = data;
   char **buf;

   buf = eina_inarray_nth(sel->links, 0);
   free(*buf);
   eina_inarray_free(sel->links);
   eina_inarray_free(sel->link_names);
   free(sel);
   return 1;
}
#ifdef HAVE_ICONV
# include <errno.h>
# include <iconv.h>
#endif
static void *
_ecore_x_selection_parser_xmozurl(const char *target EINA_UNUSED,
                               void *_data,
                               int size,
                               int format EINA_UNUSED)
{
   Ecore_X_Selection_Data_X_Moz_Url *sel;
   char *prev, *n, *buf, *data = _data;
   size_t sz;
   int num = 0;

   buf = eina_str_convert_len("UTF-16LE", "UTF-8", data, size, &sz);
   if (!buf) return NULL;
   sel = calloc(1, sizeof(Ecore_X_Selection_Data_X_Moz_Url));
   if (!sel)
      goto error_sel;

   sz = strlen(buf);
   sel->links = eina_inarray_new(sizeof(char*), 0);
   if (!sel->links)
      goto error_links;

   sel->link_names = eina_inarray_new(sizeof(char*), 0);
   if (!sel->link_names)
      goto error_link_names;

   prev = buf;
   for (n = memchr(buf, '\n', sz); n; n = memchr(prev, '\n', sz - (prev - buf)))
     {
        n[0] = 0;
        if (num % 2 == 0)
          eina_inarray_push(sel->links, &prev);
        else
          eina_inarray_push(sel->link_names, &prev);
        num++;
        prev = n + 1;
     }
   eina_inarray_push(sel->link_names, &prev);

   ECORE_X_SELECTION_DATA(sel)->length = size;
   ECORE_X_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_X_MOZ_URL;
   ECORE_X_SELECTION_DATA(sel)->data = (void*)data;
   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_xmozurl_free;
   return sel;

error_link_names:
   eina_inarray_free(sel->links);

error_links:
   free(sel);

error_sel:
   free(buf);
   return NULL;
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
_ecore_x_selection_parser_targets(const char *target EINA_UNUSED,
                                  void *data,
                                  int size,
                                  int format EINA_UNUSED)
{
   Ecore_X_Selection_Data_Targets *sel;
   int *targets;
   int i;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Targets));
   if (!sel) return NULL;
   targets = data;

   sel->num_targets = size;
   sel->targets = malloc((sel->num_targets) * sizeof(char *));
   if (!sel->targets)
     {
        free(sel);
        return NULL;
     }
   for (i = 0; i < size; i++)
     sel->targets[i] = XGetAtomName(_ecore_x_disp, targets[i]);

   ECORE_X_SELECTION_DATA(sel)->free = _ecore_x_selection_data_targets_free;
   ECORE_X_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_TARGETS;
   ECORE_X_SELECTION_DATA(sel)->length = size;
   ECORE_X_SELECTION_DATA(sel)->data = data;
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

   free(ECORE_X_SELECTION_DATA(sel)->data);
   free(sel);
   return 1;
}

