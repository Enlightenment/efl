#include "ecore_xcb_private.h"
//#include "Ecore_X_Atoms.h"

#define ECORE_XCB_SELECTION_DATA(x) ((Ecore_X_Selection_Data *)(x))

/* local function prototypes */
static Eina_Bool _ecore_xcb_selection_converter_text(char         *target,
                                                     void         *data,
                                                     int           size,
                                                     void        **data_ret,
                                                     int          *size_ret,
                                                     Ecore_X_Atom *type,
                                                     int          *size_type);
static void *_ecore_xcb_selection_parser_text(const char *target __UNUSED__,
                                              void       *data,
                                              int         size,
                                              int         format __UNUSED__);
static void *_ecore_xcb_selection_parser_files(const char *target,
                                               void       *data,
                                               int         size,
                                               int         format __UNUSED__);
static void *_ecore_xcb_selection_parser_targets(const char *target __UNUSED__,
                                                 void       *data,
                                                 int         size,
                                                 int         format __UNUSED__);

//static int _ecore_xcb_selection_data_free(void *data);
static int       _ecore_xcb_selection_data_text_free(void *data);
static int       _ecore_xcb_selection_data_targets_free(void *data);
static int       _ecore_xcb_selection_data_files_free(void *data);
static int       _ecore_xcb_selection_data_default_free(void *data);
static Eina_Bool _ecore_xcb_selection_set(Ecore_X_Window win,
                                          const void    *data,
                                          int            size,
                                          Ecore_X_Atom   selection);
static void _ecore_xcb_selection_request(Ecore_X_Window win,
                                         Ecore_X_Atom   selection,
                                         const char    *target);
static Ecore_X_Atom _ecore_xcb_selection_target_atom_get(const char *target);

/* local variables */
static Ecore_X_Selection_Intern _selections[4];
static Ecore_X_Selection_Converter *_converters = NULL;
static Ecore_X_Selection_Parser *_parsers = NULL;

/* local functions */
void
_ecore_xcb_selection_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   memset(_selections, 0, sizeof(_selections));

   /* init converters */
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_TEXT,
                                        _ecore_xcb_selection_converter_text);
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_UTF8_STRING,
                                        _ecore_xcb_selection_converter_text);
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_COMPOUND_TEXT,
                                        _ecore_xcb_selection_converter_text);
   ecore_x_selection_converter_atom_add(ECORE_X_ATOM_STRING,
                                        _ecore_xcb_selection_converter_text);

   /* init parsers */
   ecore_x_selection_parser_add("text/plain",
                                _ecore_xcb_selection_parser_text);
   ecore_x_selection_parser_add(ECORE_X_SELECTION_TARGET_UTF8_STRING,
                                _ecore_xcb_selection_parser_text);
   ecore_x_selection_parser_add("text/uri-list",
                                _ecore_xcb_selection_parser_files);
   ecore_x_selection_parser_add("_NETSCAPE_URL",
                                _ecore_xcb_selection_parser_files);
   ecore_x_selection_parser_add(ECORE_X_SELECTION_TARGET_TARGETS,
                                _ecore_xcb_selection_parser_targets);
}

void
_ecore_xcb_selection_shutdown(void)
{
   Ecore_X_Selection_Converter *cnv;
   Ecore_X_Selection_Parser *prs;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* free selection converters */
   cnv = _converters;
   while (cnv)
     {
        Ecore_X_Selection_Converter *tmp;

        tmp = cnv->next;
        free(cnv);
        cnv = tmp;
     }
   _converters = NULL;

   /* free parsers */
   prs = _parsers;
   while (prs)
     {
        Ecore_X_Selection_Parser *tmp;

        tmp = prs;
        prs = prs->next;
        free(tmp->target);
        free(tmp);
     }
   _parsers = NULL;
}

/* public functions */
EAPI void
ecore_x_selection_converter_atom_add(Ecore_X_Atom                                          target,
                                     Eina_Bool                                             (*func)(char *target,
                                                                             void         *data,
                                                                             int           size,
                                                                             void        **data_ret,
                                                                             int          *size_ret,
                                                                             Ecore_X_Atom *type,
                                                                             int          *size_type))
{
   Ecore_X_Selection_Converter *cnv;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cnv = _converters;
   if (_converters)
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
        if (!cnv->next) return;
        cnv = cnv->next;
     }
   else
     {
        _converters = calloc(1, sizeof(Ecore_X_Selection_Converter));
        if (!_converters) return;
        cnv = _converters;
     }
   cnv->target = target;
   cnv->convert = func;
}

EAPI void
ecore_x_selection_converter_add(char                                                 *target,
                                Eina_Bool                                             (*func)(char *target,
                                                                        void         *data,
                                                                        int           size,
                                                                        void        **date_ret,
                                                                        int          *size_ret,
                                                                        Ecore_X_Atom *atom_ret,
                                                                        int          *ret))
{
   Ecore_X_Atom atarget;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!func) || (!target)) return;
   atarget = _ecore_xcb_selection_target_atom_get(target);
   ecore_x_selection_converter_atom_add(atarget, func);
}

EAPI void
ecore_x_selection_converter_del(char *target)
{
   Ecore_X_Atom atarget;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!target) return;
   atarget = _ecore_xcb_selection_target_atom_get(target);
   ecore_x_selection_converter_atom_del(atarget);
}

EAPI void
ecore_x_selection_converter_atom_del(Ecore_X_Atom target)
{
   Ecore_X_Selection_Converter *conv, *pconv = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   conv = _converters;
   while (conv)
     {
        if (conv->target == target)
          {
             if (pconv)
               pconv->next = conv->next;
             else
               _converters = conv->next;
             free(conv);
             return;
          }
        pconv = conv;
        conv = conv->next;
     }
}

EAPI void
ecore_x_selection_parser_add(const char *target,
                             void *(*func)(const char *target, void *data, int size, int format))
{
   Ecore_X_Selection_Parser *prs;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!target) return;
   prs = _parsers;
   if (prs)
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
        _parsers = calloc(1, sizeof(Ecore_X_Selection_Parser));
        prs = _parsers;
     }
   prs->target = strdup(target);
   prs->parse = func;
}

EAPI void
ecore_x_selection_parser_del(const char *target)
{
   Ecore_X_Selection_Parser *prs, *pprs = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!target) return;

   prs = _parsers;
   while (prs)
     {
        if (!strcmp(prs->target, target))
          {
             if (pprs)
               pprs->next = prs->next;
             else
               _parsers = prs->next;
             free(prs->target);
             free(prs);
             return;
          }
        pprs = prs;
        prs = prs->next;
     }
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
ecore_x_selection_primary_set(Ecore_X_Window win,
                              const void    *data,
                              int            size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(win, data, size,
                                   ECORE_X_ATOM_SELECTION_PRIMARY);
}

/**
 * Release ownership of the primary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 */
EAPI Eina_Bool
ecore_x_selection_primary_clear(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(XCB_NONE, NULL, 0,
                                   ECORE_X_ATOM_SELECTION_PRIMARY);
}

EAPI void
ecore_x_selection_primary_request(Ecore_X_Window win,
                                  const char    *target)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_selection_request(win, ECORE_X_ATOM_SELECTION_PRIMARY, target);
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
ecore_x_selection_secondary_set(Ecore_X_Window win,
                                const void    *data,
                                int            size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(win, data, size,
                                   ECORE_X_ATOM_SELECTION_SECONDARY);
}

/**
 * Release ownership of the secondary selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 */
EAPI Eina_Bool
ecore_x_selection_secondary_clear(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(XCB_NONE, NULL, 0,
                                   ECORE_X_ATOM_SELECTION_SECONDARY);
}

EAPI void
ecore_x_selection_secondary_request(Ecore_X_Window win,
                                    const char    *target)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_selection_request(win, ECORE_X_ATOM_SELECTION_SECONDARY, target);
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
ecore_x_selection_xdnd_set(Ecore_X_Window win,
                           const void    *data,
                           int            size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(win, data, size,
                                   ECORE_X_ATOM_SELECTION_XDND);
}

/**
 * Release ownership of the XDND selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 */
EAPI Eina_Bool
ecore_x_selection_xdnd_clear(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(XCB_NONE, NULL, 0,
                                   ECORE_X_ATOM_SELECTION_XDND);
}

EAPI void
ecore_x_selection_xdnd_request(Ecore_X_Window win,
                               const char    *target)
{
   Ecore_X_Atom atom;
   Ecore_X_DND_Target *_target;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   _target = _ecore_xcb_dnd_target_get();
   atom = _ecore_xcb_selection_target_atom_get(target);

   xcb_convert_selection(_ecore_xcb_conn, win, ECORE_X_ATOM_SELECTION_XDND,
                         atom, ECORE_X_ATOM_SELECTION_PROP_XDND, _target->time);
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
ecore_x_selection_clipboard_set(Ecore_X_Window win,
                                const void    *data,
                                int            size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(win, data, size,
                                   ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

/**
 * Release ownership of the clipboard selection
 * @return     Returns 1 if the selection was successfully cleared,
 *             or 0 if unsuccessful.
 */
EAPI Eina_Bool
ecore_x_selection_clipboard_clear(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_selection_set(XCB_NONE, NULL, 0,
                                   ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

EAPI void
ecore_x_selection_clipboard_request(Ecore_X_Window win,
                                    const char    *target)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_selection_request(win, ECORE_X_ATOM_SELECTION_CLIPBOARD, target);
}

EAPI Eina_Bool
ecore_x_selection_convert(Ecore_X_Atom  selection,
                          Ecore_X_Atom  target,
                          void        **data_ret,
                          int          *size,
                          Ecore_X_Atom *targtype,
                          int          *typesize)
{
   Ecore_X_Selection_Intern *sel;
   Ecore_X_Selection_Converter *cnv;
   void *data;
   char *tgt_str;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   sel = _ecore_xcb_selection_get(selection);
   tgt_str = _ecore_xcb_selection_target_get(target);

   for (cnv = _converters; cnv; cnv = cnv->next)
     {
        if (cnv->target == target)
          {
             int r = 0;

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

   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_selection_notify_send(Ecore_X_Window requestor,
                              Ecore_X_Atom   selection,
                              Ecore_X_Atom   target,
                              Ecore_X_Atom   property,
                              Ecore_X_Time   tim)
{
   xcb_selection_notify_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   memset(&ev, 0, sizeof(xcb_selection_notify_event_t));

   ev.response_type = XCB_SELECTION_NOTIFY;
   ev.requestor = requestor;
   ev.selection = selection;
   ev.target = target;
   ev.property = property;
   ev.time = tim;

   xcb_send_event(_ecore_xcb_conn, 0, requestor,
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
//   ecore_x_flush();

   return EINA_TRUE;
}

EAPI void
ecore_x_selection_owner_set(Ecore_X_Window win,
                            Ecore_X_Atom   atom,
                            Ecore_X_Time   tim)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   xcb_set_selection_owner(_ecore_xcb_conn, win, atom, tim);
}

EAPI Ecore_X_Window
ecore_x_selection_owner_get(Ecore_X_Atom atom)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;
   Ecore_X_Window ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie = xcb_get_selection_owner(_ecore_xcb_conn, atom);
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   ret = reply->owner;
   free(reply);
   return ret;
}

void *
_ecore_xcb_selection_parse(const char *target,
                           void       *data,
                           int         size,
                           int         format)
{
   Ecore_X_Selection_Parser *prs;
   Ecore_X_Selection_Data *sel;

   for (prs = _parsers; prs; prs = prs->next)
     {
        if (!strcmp(prs->target, target))
          {
             sel = prs->parse(target, data, size, format);
             if (sel) return sel;
          }
     }

   sel = calloc(1, sizeof(Ecore_X_Selection_Data));
   if (!sel) return NULL;
   sel->free = _ecore_xcb_selection_data_default_free;
   sel->length = size;
   sel->format = format;
   sel->data = data;

   return sel;
}

Ecore_X_Selection_Intern *
_ecore_xcb_selection_get(Ecore_X_Atom selection)
{
   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     return &_selections[0];
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     return &_selections[1];
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     return &_selections[2];
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     return &_selections[3];
   else
     return NULL;
}

/* local functions */
static Eina_Bool
_ecore_xcb_selection_set(Ecore_X_Window win,
                         const void    *data,
                         int            size,
                         Ecore_X_Atom   selection)
{
   xcb_get_selection_owner_cookie_t cookie;
   xcb_get_selection_owner_reply_t *reply;
   int in = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   xcb_set_selection_owner(_ecore_xcb_conn, win, selection, XCB_CURRENT_TIME);

   cookie = xcb_get_selection_owner(_ecore_xcb_conn, selection);
   reply = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   if (reply->owner != win)
     {
        free(reply);
        return EINA_FALSE;
     }
   free(reply);

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

   if (data)
     {
        unsigned char *buff = NULL;

        _selections[in].win = win;
        _selections[in].selection = selection;
        _selections[in].length = size;
        _selections[in].time = _ecore_xcb_events_last_time_get();

        buff = malloc(size);
        if (!buff) return EINA_FALSE;
        memcpy(buff, data, size);
        _selections[in].data = buff;
     }
   else if (_selections[in].data)
     {
        free(_selections[in].data);
        memset(&_selections[in], 0, sizeof(Ecore_X_Selection_Data));
     }

   return EINA_TRUE;
}

static void
_ecore_xcb_selection_request(Ecore_X_Window win,
                             Ecore_X_Atom   selection,
                             const char    *target)
{
   Ecore_X_Atom atarget, prop;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     prop = ECORE_X_ATOM_SELECTION_PROP_PRIMARY;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     prop = ECORE_X_ATOM_SELECTION_PROP_SECONDARY;
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     prop = ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD;
   else
     return;

   atarget = _ecore_xcb_selection_target_atom_get(target);

   xcb_convert_selection(_ecore_xcb_conn, win, selection, atarget, prop,
                         XCB_CURRENT_TIME);
}

static Eina_Bool
_ecore_xcb_selection_converter_text(char         *target,
                                    void         *data,
                                    int           size,
                                    void        **data_ret,
                                    int          *size_ret,
                                    Ecore_X_Atom *type __UNUSED__,
                                    int          *size_type __UNUSED__)
{
   Ecore_Xcb_Encoding_Style style;
   Ecore_Xcb_Textproperty ret;
   char *str;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!data) || (!size)) return EINA_FALSE;

   if (!strcmp(target, ECORE_X_SELECTION_TARGET_TEXT))
     style = XcbTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_COMPOUND_TEXT))
     style = XcbCompoundTextStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_STRING))
     style = XcbStringStyle;
   else if (!strcmp(target, ECORE_X_SELECTION_TARGET_UTF8_STRING))
     style = XcbUTF8StringStyle;
   else
     return EINA_FALSE;

   str = alloca(size + 1);
   memcpy(str, data, size);
   str[size] = '\0';

#ifdef HAVE_ICONV
   if (_ecore_xcb_utf8_textlist_to_textproperty(&str, 1, style, &ret))
     {
        int size = 0;

        size = (strlen((char *)ret.value) + 1);
        *data_ret = malloc(size);
        if (!*data_ret) return EINA_FALSE;
        memcpy(*data_ret, ret.value, size);
        *size_ret = size;
        if (ret.value) free(ret.value);
        return EINA_TRUE;
     }
#else
   if (_ecore_xcb_mb_textlist_to_textproperty(&str, 1, style, &ret))
     {
        int size = 0;

        size = (strlen((char *)ret.value) + 1);
        *data_ret = malloc(size);
        if (!*data_ret) return EINA_FALSE;
        memcpy(*data_ret, ret.value, size);
        *size_ret = size;
        if (ret.value) free(ret.value);
        return EINA_TRUE;
     }
#endif
   else
     return EINA_TRUE;

   return EINA_FALSE;
}

static void *
_ecore_xcb_selection_parser_text(const char *target __UNUSED__,
                                 void       *data,
                                 int         size,
                                 int         format __UNUSED__)
{
   Ecore_X_Selection_Data_Text *sel;
   unsigned char *_data;
   void *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(_data = data)) return NULL;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Text));
   if (!sel) return NULL;

   if (_data[size - 1])
     {
        size++;
        t = realloc(_data, size);
        if (!t)
          {
             free(sel);
             return NULL;
          }
        _data = t;
        _data[size - 1] = 0;
     }
   sel->text = (char *)_data;
   ECORE_XCB_SELECTION_DATA(sel)->length = size;
   ECORE_XCB_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_TEXT;
   ECORE_XCB_SELECTION_DATA(sel)->data = _data;
   ECORE_XCB_SELECTION_DATA(sel)->free = _ecore_xcb_selection_data_text_free;
   return sel;
}

static void *
_ecore_xcb_selection_parser_files(const char *target,
                                  void       *data,
                                  int         size,
                                  int         format __UNUSED__)
{
   Ecore_X_Selection_Data_Files *sel;
   char *_data, *tmp, *t, **t2;
   int i = 0, is = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((strcmp(target, "text/uri-list")) &&
       (strcmp(target, "_NETSCAPE_URL"))) return NULL;

   if (!(_data = data)) return NULL;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Files));
   if (!sel) return NULL;

   ECORE_XCB_SELECTION_DATA(sel)->free = _ecore_xcb_selection_data_files_free;

   if (_data[size - 1])
     {
        size++;
        t = realloc(_data, size);
        if (!t)
          {
             free(sel);
             return NULL;
          }
        _data = t;
        _data[size - 1] = 0;
     }

   tmp = malloc(size);
   if (!tmp)
     {
        free(sel);
        return NULL;
     }

   while ((is < size) && (_data[is]))
     {
        if ((i == 0) && (_data[is] == '#'))
          {
             for (; ((_data[is]) && (_data[is] != '\n')); is++) ;
          }
        else
          {
             if ((_data[is] != '\r') && (_data[is] != '\n'))
               tmp[i++] = _data[is++];
             else
               {
                  while ((_data[is] == '\r') || (_data[is] == '\n'))
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
   if (tmp) free(tmp);
   if (_data) free(_data);

   ECORE_XCB_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_FILES;
   ECORE_XCB_SELECTION_DATA(sel)->length = sel->num_files;

   return ECORE_XCB_SELECTION_DATA(sel);
}

static void *
_ecore_xcb_selection_parser_targets(const char *target __UNUSED__,
                                    void       *data,
                                    int         size,
                                    int         format __UNUSED__)
{
   Ecore_X_Selection_Data_Targets *sel;
   unsigned long *targets;
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!(targets = (unsigned long *)data)) return NULL;

   sel = calloc(1, sizeof(Ecore_X_Selection_Data_Targets));
   if (!sel) return NULL;

   sel->num_targets = (size - 2);
   sel->targets = malloc((size - 2) * sizeof(char *));
   if (!sel->targets)
     {
        free(sel);
        return NULL;
     }

   for (i = 2; i < size; i++)
     {
        xcb_get_atom_name_cookie_t cookie;
        xcb_get_atom_name_reply_t *reply;
        char *name = NULL;
        int len = 0;

        cookie = xcb_get_atom_name_unchecked(_ecore_xcb_conn, targets[i]);
        reply = xcb_get_atom_name_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
             len = xcb_get_atom_name_name_length(reply);
             name = (char *)malloc(sizeof(char) * (len + 1));
             if (name)
               {
                  memcpy(name, xcb_get_atom_name_name(reply), len);
                  name[len] = '\0';
                  sel->targets[i - 2] = name;
               }
             free(reply);
          }
     }

   ECORE_XCB_SELECTION_DATA(sel)->free =
     _ecore_xcb_selection_data_targets_free;
   ECORE_XCB_SELECTION_DATA(sel)->content = ECORE_X_SELECTION_CONTENT_TARGETS;
   ECORE_XCB_SELECTION_DATA(sel)->length = size;
   ECORE_XCB_SELECTION_DATA(sel)->data = data;

   return sel;
}

/*
   static int
   _ecore_xcb_selection_data_free(void *data)
   {
   Ecore_X_Selection_Data *sel;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sel = data)) return 0;
   if (sel->data) free(sel->data);
   free(sel);
   return 1;
   }
 */

static int
_ecore_xcb_selection_data_text_free(void *data)
{
   Ecore_X_Selection_Data_Text *sel;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sel = data)) return 0;
   if (sel->text) free(sel->text);
   free(sel);
   return 1;
}

static int
_ecore_xcb_selection_data_targets_free(void *data)
{
   Ecore_X_Selection_Data_Targets *sel;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sel = data)) return 0;
   if (sel->targets)
     {
        int i = 0;

        for (i = 0; i < sel->num_targets; i++)
          if (sel->targets[i]) free(sel->targets[i]);
        if (sel->targets) free(sel->targets);
     }
   free(sel);
   return 1;
}

static int
_ecore_xcb_selection_data_files_free(void *data)
{
   Ecore_X_Selection_Data_Files *sel;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sel = data)) return 0;
   if (sel->files)
     {
        int i = 0;

        for (i = 0; i < sel->num_files; i++)
          if (sel->files[i]) free(sel->files[i]);
        if (sel->files) free(sel->files);
     }
   free(sel);
   return 0;
}

static int
_ecore_xcb_selection_data_default_free(void *data)
{
   Ecore_X_Selection_Data *sel;

   if (!(sel = data)) return 1;
   free(sel->data);
   free(sel);
   return 1;
}

static Ecore_X_Atom
_ecore_xcb_selection_target_atom_get(const char *target)
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
     x_target = ecore_x_atom_get(target);

   return x_target;
}

char *
_ecore_xcb_selection_target_get(Ecore_X_Atom target)
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
     return ecore_x_atom_name_get(target);
}

