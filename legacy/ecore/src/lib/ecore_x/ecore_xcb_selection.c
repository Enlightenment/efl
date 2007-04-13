/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"

static Ecore_X_Selection_Intern     selections[4];
static Ecore_X_Selection_Converter *converters = NULL;
static Ecore_X_Selection_Parser    *parsers = NULL;

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
_ecore_x_selection_init(void)
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


/*
 * Sends the GetSelectionOwner request.
 */
void
_ecore_xcb_get_selection_owner_prefetch(Ecore_X_Atom selection)
{
   xcb_get_selection_owner_cookie_t cookie;

   cookie = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, selection);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/*
 * Gets the reply of the GetSelectionOwner request sent by _ecore_xcb_get_selection_owner_prefetch().
 */
void
_ecore_xcb_get_selection_owner_fetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/*
 * To use this function, you must call before, and in order,
 * _ecore_xcb_get_selection_owner_prefetch(), which sends the GetSelectionOwner request,
 * then _ecore_xcb_get_selection_owner_fetch(), which gets the reply.
 */
int
_ecore_x_selection_set(Ecore_X_Window window,
                       const void    *data,
                       int            size,
                       Ecore_X_Atom   selection)
{
   xcb_get_selection_owner_reply_t *reply;
   unsigned char                   *buf = NULL;
   int                              in;

   xcb_set_selection_owner(_ecore_xcb_conn, window, selection, _ecore_xcb_event_last_time);

   reply = _ecore_xcb_reply_get();
   if (!reply || (reply->owner != window)) return 0;

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
	selections[in].win = window;
	selections[in].selection = selection;
	selections[in].length = size;
	selections[in].time = _ecore_xcb_event_last_time;

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
 * Sends the GetSelectionOwner request.
 */
EAPI void
ecore_x_selection_primary_prefetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;

   cookie = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, ECORE_X_ATOM_SELECTION_PRIMARY);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetSelectionOwner request sent by ecore_x_selection_primary_prefetch().
 */
EAPI void
ecore_x_selection_primary_fetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Claim ownership of the PRIMARY selection and set its data.
 * @param window The window to which this selection belongs
 * @param data   The data associated with the selection
 * @param size   The size of the data buffer in bytes
 * @return       Returns 1 if the ownership of the selection was successfully
 *               claimed, or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_primary_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_primary_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_primary_set(Ecore_X_Window window,
                              const void    *data,
                              int            size)
{
   return _ecore_x_selection_set(window, data, size, ECORE_X_ATOM_SELECTION_PRIMARY);
}

/**
 * Release ownership of the primary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_primary_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_primary_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_primary_clear(void)
{
   return _ecore_x_selection_set(XCB_NONE, NULL, 0, ECORE_X_ATOM_SELECTION_PRIMARY);
}


/**
 * Sends the GetSelectionOwner request.
 */
EAPI void
ecore_x_selection_secondary_prefetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;

   cookie = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, ECORE_X_ATOM_SELECTION_SECONDARY);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetSelectionOwner request sent by ecore_x_selection_secondary_prefetch().
 */
EAPI void
ecore_x_selection_secondary_fetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}


/**
 * Claim ownership of the SECONDARY selection and set its data.
 * @param window The window to which this selection belongs
 * @param data   The data associated with the selection
 * @param size   The size of the data buffer in bytes
 * @return       Returns 1 if the ownership of the selection was successfully
 *               claimed, or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_secondary_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_secondary_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_secondary_set(Ecore_X_Window window,
                                const void    *data,
                                int            size)
{
   return _ecore_x_selection_set(window, data, size, ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Release ownership of the secondary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_secondary_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_secondary_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_secondary_clear(void)
{
   return _ecore_x_selection_set(XCB_NONE, NULL, 0, ECORE_X_ATOM_SELECTION_SECONDARY);
}


/**
 * Sends the GetSelectionOwner request.
 */
EAPI void
ecore_x_selection_xdnd_prefetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;

   cookie = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, ECORE_X_ATOM_SELECTION_XDND);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetSelectionOwner request sent by ecore_x_selection_xdnd_prefetch().
 */
EAPI void
ecore_x_selection_xdnd_fetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Claim ownership of the XDND selection and set its data.
 * @param window The window to which this selection belongs
 * @param data   The data associated with the selection
 * @param size   The size of the data buffer in bytes
 * @return       Returns 1 if the ownership of the selection was successfully
 *               claimed, or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_xdnd_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_xdnd_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_xdnd_set(Ecore_X_Window window,
                           const void    *data,
                           int            size)
{
   return _ecore_x_selection_set(window, data, size, ECORE_X_ATOM_SELECTION_XDND);
}

/**
 * Release ownership of the XDND selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_xdnd_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_xdnd_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_xdnd_clear(void)
{
   return _ecore_x_selection_set(XCB_NONE, NULL, 0, ECORE_X_ATOM_SELECTION_XDND);
}


/**
 * Sends the GetSelectionOwner request.
 */
EAPI void
ecore_x_selection_clipboard_prefetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;

   cookie = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, ECORE_X_ATOM_SELECTION_CLIPBOARD);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetSelectionOwner request sent by ecore_x_selection_clipboard_prefetch().
 */
EAPI void
ecore_x_selection_clipboard_fetch(void)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Claim ownership of the CLIPBOARD selection and set its data.
 * @param window The window to which this selection belongs
 * @param data   The data associated with the selection
 * @param size   The size of the data buffer in bytes
 * @return       Returns 1 if the ownership of the selection was successfully
 *               claimed, or 0 if unsuccessful.
 *
 * Get the converted data from a previous CLIPBOARD selection
 * request. The buffer must be freed when done with.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_clipboard_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_clipboard_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_clipboard_set(Ecore_X_Window window,
                                const void    *data,
                                int            size)
{
   return _ecore_x_selection_set(window, data, size, ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

/**
 * Release ownership of the clipboard selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_selection_clipboard_prefetch(), which sends the GetSelectionOwner request,
 * then ecore_x_selection_clipboard_fetch(), which gets the reply.
 */
EAPI int
ecore_x_selection_clipboard_clear(void)
{
   return _ecore_x_selection_set(XCB_NONE, NULL, 0, ECORE_X_ATOM_SELECTION_CLIPBOARD);
}


/* FIXME: roundtrip if target is not handled in the tests */
Ecore_X_Atom
_ecore_x_selection_target_atom_get(const char *target)
{
   Ecore_X_Atom x_target = XCB_NONE;

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
        xcb_intern_atom_cookie_t cookie;
        xcb_intern_atom_reply_t *reply;

        cookie = xcb_intern_atom_unchecked(_ecore_xcb_conn, 0,
                                           strlen(target), target);
        reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
        if (!reply)
          return XCB_NONE;
	x_target = reply->atom;
        free(reply);
     }

   return x_target;
}


/* FIXME: roundtrip if target is not handled in the tests */
char *
_ecore_x_selection_target_get(Ecore_X_Atom target)
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
     {
        xcb_get_atom_name_cookie_t cookie;
        xcb_get_atom_name_reply_t *reply;
        char                      *name;

        cookie = xcb_get_atom_name_unchecked(_ecore_xcb_conn, target);
        reply = xcb_get_atom_name_reply(_ecore_xcb_conn, cookie, NULL);
        if (!reply)
          return NULL;
        name = (char *)malloc(sizeof(char) * (reply->length + 1));
        if (!name)
          {
             free(reply);
             return NULL;
          }
        memcpy(name, xcb_get_atom_name_name(reply), reply->length);
        name[reply->length] = '\0';
        free(reply);
        return name;
     }
}

static void
_ecore_x_selection_request(Ecore_X_Window window,
                           Ecore_X_Atom   selection,
                           const char    *target_str)
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

   xcb_convert_selection(_ecore_xcb_conn, window,
                         selection, target, prop,
                         XCB_CURRENT_TIME);
}

EAPI void
ecore_x_selection_primary_request(Ecore_X_Window window,
                                  const char    *target)
{
   _ecore_x_selection_request(window, ECORE_X_ATOM_SELECTION_PRIMARY, target);
}

EAPI void
ecore_x_selection_secondary_request(Ecore_X_Window window,
                                    const char    *target)
{
   _ecore_x_selection_request(window, ECORE_X_ATOM_SELECTION_SECONDARY, target);
}

EAPI void
ecore_x_selection_xdnd_request(Ecore_X_Window window,
                               const char    *target)
{
   Ecore_X_Atom        atom;
   Ecore_X_DND_Target *_target;

   _target = _ecore_x_dnd_target_get();
   atom = _ecore_x_selection_target_atom_get(target);
   xcb_convert_selection(_ecore_xcb_conn, window,
                         ECORE_X_ATOM_SELECTION_XDND, atom,
                         ECORE_X_ATOM_SELECTION_PROP_XDND,
                         _target->time);
}

EAPI void
ecore_x_selection_clipboard_request(Ecore_X_Window window, const char *target)
{
   _ecore_x_selection_request(window, ECORE_X_ATOM_SELECTION_CLIPBOARD, target);
}

EAPI void
ecore_x_selection_converter_atom_add(Ecore_X_Atom target,
                                     int        (*func)(char  *target,
                                                        void  *data,
                                                        int    size,
                                                        void **data_ret,
                                                        int   *size_ret))
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
                                int (*func)(char  *target,
                                            void  *data,
                                            int    size,
                                            void **data_ret,
                                            int   *size_ret))
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
ecore_x_selection_notify_send(Ecore_X_Window requestor,
                              Ecore_X_Atom   selection,
                              Ecore_X_Atom   target,
                              Ecore_X_Atom   property,
                              Ecore_X_Time   time)
{
   xcb_selection_notify_event_t ev;

   ev.time = time;
   ev.requestor = requestor;
   ev.selection = selection;
   ev.target = target;
   ev.property = property;
   /* send_event is bit 7 (0x80) of response_type */
   ev.response_type = 0x80;

   xcb_send_event(_ecore_xcb_conn, 0,
                  requestor, 0, (const char *)&ev);
   return 1;
}

/* Locate and run conversion callback for specified selection target */
EAPI int
ecore_x_selection_convert(Ecore_X_Atom selection,
                          Ecore_X_Atom target,
                          void       **data_ret)
{
   Ecore_X_Selection_Intern    *sel;
   Ecore_X_Selection_Converter *cnv;
   void                        *data;
   char                        *tgt_str;
   int                          size;

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

   /* Default, just return the data */
   *data_ret = malloc(sel->length);
   memcpy(*data_ret, sel->data, sel->length);
   free(tgt_str);
   return 1;
}

/* TODO: We need to work out a mechanism for automatic conversion to any requested
 * locale using Ecore_Txt functions */
/* Converter for standard non-utf8 text targets */
static int
_ecore_x_selection_converter_text(char *target, void *data, int size, void **data_ret, int *size_ret)
{

  /* FIXME: to do... */

/*    XTextProperty text_prop; */
/*    char *mystr; */
/*    XICCEncodingStyle style; */

/*    if (!data || !size) */
/*      return 0; */

/*    if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT)) */
/*      style = XTextStyle; */
/*    else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT)) */
/*      style = XCompoundTextStyle; */
/*    else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING)) */
/*      style = XStringStyle; */
/* #ifdef X_HAVE_UTF8_STRING */
/*    else if (!strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING)) */
/*      style = XUTF8StringStyle; */
/* #endif */
/*    else */
/*      return 0; */

/*    if (!(mystr = strdup(data))) */
/*      return 0; */

/* #ifdef X_HAVE_UTF8_STRING */
/*    if (Xutf8TextListToTextProperty(_ecore_x_disp, &mystr, 1, style, &text_prop) == Success) */
/*      { */
/* 	int bufsize = strlen((char *)text_prop.value) + 1; */
/* 	*data_ret = malloc(bufsize); */
/* 	memcpy(*data_ret, text_prop.value, bufsize); */
/* 	*size_ret = bufsize; */
/* 	XFree(text_prop.value); */
/* 	free(mystr); */
/* 	return 1; */
/*      } */
/* #else */
/*    if (XmbTextListToTextProperty(_ecore_x_disp, &mystr, 1, style, &text_prop) == Success) */
/*      { */
/* 	int bufsize = strlen(text_prop.value) + 1; */
/* 	*data_ret = malloc(bufsize); */
/* 	memcpy(*data_ret, text_prop.value, bufsize); */
/* 	*size_ret = bufsize; */
/* 	XFree(text_prop.value); */
/* 	free(mystr); */
/* 	return 1; */
/*      } */
/* #endif */
/*    else */
/*      { */
/* 	free(mystr); */
/* 	return 0; */
/*      } */

  return 0;
}

EAPI void
ecore_x_selection_parser_add(const char *target,
                             void     *(*func)(const char *target,
                                               void       *data,
                                               int         size,
                                               int         format))
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
_ecore_x_selection_parser_text(const char *target __UNUSED__,
                               void       *_data,
                               int         size,
                               int         format __UNUSED__)
{
   Ecore_X_Selection_Data_Text *sel;
   char                        *data = _data;

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
_ecore_x_selection_parser_targets(const char *target __UNUSED__,
                                  void       *data,
                                  int         size,
                                  int         format __UNUSED__)
{
   Ecore_X_Selection_Data_Targets *sel;
   uint32_t                       *targets;
   xcb_get_atom_name_cookie_t     *cookies;
   int                             i;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Targets));
   targets = (uint32_t *)data;

   sel->num_targets = size - 2;
   sel->targets = malloc((size - 2) * sizeof(char *));
   cookies = (xcb_get_atom_name_cookie_t *)malloc ((size - 2) * sizeof (xcb_get_atom_name_cookie_t));
   for (i = 0; i < size - 2; i++)
     cookies[i] = xcb_get_atom_name_unchecked(_ecore_xcb_conn, targets[i + 2]);

   /* FIXME: do we let the declaration of reply inside the loop ? */
   for (i = 0; i < size - 2; i++)
     {
       xcb_get_atom_name_reply_t *reply;
       char                      *name;
       int                        length;

       reply =xcb_get_atom_name_reply(_ecore_xcb_conn, cookies[i], NULL);
       length = xcb_get_atom_name_name_length(reply);
       name = (char *)malloc (length + 1);
       memcpy(name, xcb_get_atom_name_name(reply), length);
       name[length] = '\0';
       sel->targets[i - 2] = name;
     }
   free(cookies);
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
	  free(sel->targets[i]);
	free(sel->targets);
     }
   free(sel);
   return 1;
}
