#include "ecore_xcb_private.h"

#ifndef MIN
# define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* local structures */
typedef struct _Version_Cache_Item 
{
   Ecore_X_Window win;
   int ver;
} Version_Cache_Item;

/* local function prototypes */
static Eina_Bool _ecore_xcb_dnd_converter_copy(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *tprop __UNUSED__, int *count __UNUSED__);

/* local variables */
static int _ecore_xcb_dnd_init_count;
static Ecore_X_DND_Source *_source = NULL;
static Ecore_X_DND_Target *_target = NULL;
static Version_Cache_Item *_version_cache = NULL;
static int _version_cache_num = 0, _version_cache_alloc = 0;
static void (*_posupdatecb)(void *, Ecore_X_Xdnd_Position *);
static void *_posupdatedata;

/* external variables */
EAPI int ECORE_X_EVENT_XDND_ENTER = 0;
EAPI int ECORE_X_EVENT_XDND_POSITION = 0;
EAPI int ECORE_X_EVENT_XDND_STATUS = 0;
EAPI int ECORE_X_EVENT_XDND_LEAVE = 0;
EAPI int ECORE_X_EVENT_XDND_DROP = 0;
EAPI int ECORE_X_EVENT_XDND_FINISHED = 0;

void 
_ecore_xcb_dnd_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!_ecore_xcb_dnd_init_count) 
     {
        _source = calloc(1, sizeof(Ecore_X_DND_Source));
        if (!_source) return;
        _source->version = ECORE_X_DND_VERSION;
        _source->win = XCB_NONE;
        _source->dest = XCB_NONE;
        _source->state = ECORE_X_DND_SOURCE_IDLE;
        _source->prev.window = 0;

        _target = calloc(1, sizeof(Ecore_X_DND_Target));
        if (!_target) 
          {
             free(_source);
             _source = NULL;
             return;
          }
        _target->win = XCB_NONE;
        _target->source = XCB_NONE;
        _target->state = ECORE_X_DND_TARGET_IDLE;

        ECORE_X_EVENT_XDND_ENTER = ecore_event_type_new();
        ECORE_X_EVENT_XDND_POSITION = ecore_event_type_new();
        ECORE_X_EVENT_XDND_STATUS = ecore_event_type_new();
        ECORE_X_EVENT_XDND_LEAVE = ecore_event_type_new();
        ECORE_X_EVENT_XDND_DROP = ecore_event_type_new();
        ECORE_X_EVENT_XDND_FINISHED = ecore_event_type_new();
     }
   _ecore_xcb_dnd_init_count++;
}

void 
_ecore_xcb_dnd_shutdown(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_dnd_init_count--;
   if (_ecore_xcb_dnd_init_count > 0) return;
   if (_source) free(_source);
   _source = NULL;
   if (_target) free(_target);
   _target = NULL;
   _ecore_xcb_dnd_init_count = 0;
}

EAPI void 
ecore_x_dnd_send_status(Eina_Bool will_accept, Eina_Bool suppress, Ecore_X_Rectangle rect, Ecore_X_Atom action) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_target->state == ECORE_X_DND_TARGET_IDLE) return;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   _target->will_accept = will_accept;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.type = ECORE_X_ATOM_XDND_STATUS;
   ev.format = 32;
   ev.window = _target->source;
   ev.data.data32[0] = _target->win;
   ev.data.data32[1] = 0;
   if (will_accept) ev.data.data32[1] |= 0x1UL;
   if (!suppress) ev.data.data32[1] |= 0x2UL;

   ev.data.data32[2] = rect.x;
   ev.data.data32[2] <<= 16;
   ev.data.data32[2] |= rect.y;
   ev.data.data32[3] = rect.width;
   ev.data.data32[3] <<= 16;
   ev.data.data32[3] |= rect.height;

   if (will_accept) 
     ev.data.data32[4] = action;
   else 
     ev.data.data32[4] = XCB_NONE;
   _target->accepted_action = action;

   xcb_send_event(_ecore_xcb_conn, 0, _target->source, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
}

EAPI Eina_Bool 
ecore_x_dnd_drop(void) 
{
   xcb_client_message_event_t ev;
   Eina_Bool status = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   if (_source->dest) 
     {
        ev.response_type = XCB_CLIENT_MESSAGE;
        ev.format = 32;
        ev.window = _source->dest;
        ev.data.data32[0] = _source->win;
        ev.data.data32[1] = 0;

        if (_source->will_accept) 
          {
             ev.type = ECORE_X_ATOM_XDND_DROP;
             ev.data.data32[2] = _source->time;

             xcb_send_event(_ecore_xcb_conn, 0, _source->dest, 
                            XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
             _source->state = ECORE_X_DND_SOURCE_DROPPED;
             status = EINA_TRUE;
          }
        else 
          {
             ev.type = ECORE_X_ATOM_XDND_LEAVE;
             xcb_send_event(_ecore_xcb_conn, 0, _source->dest, 
                            XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
             _source->state = ECORE_X_DND_SOURCE_IDLE;
          }
     }
   else 
     {
        ecore_x_selection_xdnd_clear();
        _source->state = ECORE_X_DND_SOURCE_IDLE;
     }

   ecore_x_window_ignore_set(_source->win, 0);
   _source->prev.window = 0;

   return status;
}

EAPI void 
ecore_x_dnd_aware_set(Ecore_X_Window win, Eina_Bool on) 
{
   Ecore_X_Atom prop_data = ECORE_X_DND_VERSION;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (on)
     ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_AWARE, 
                                      ECORE_X_ATOM_ATOM, 32, &prop_data, 1);
   else
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_XDND_AWARE);
}

EAPI int 
ecore_x_dnd_version_get(Ecore_X_Window win) 
{
   unsigned char *data;
   int num = 0;
   Version_Cache_Item *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_source->state == ECORE_X_DND_SOURCE_DRAGGING) 
     {
        if (_version_cache) 
          {
             int i = 0;

             for (i = 0; i < _version_cache_num; i++) 
               {
                  if (_version_cache[i].win == win)
                    return _version_cache[i].ver;
               }
          }
     }

   if (ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_AWARE, 
                                        ECORE_X_ATOM_ATOM, 32, &data, &num)) 
     {
        int version = 0;

        version = (int)*data;
        free(data);
        if (_source->state == ECORE_X_DND_SOURCE_DRAGGING) 
          {
             _version_cache_num++;
             if (_version_cache_num > _version_cache_alloc)
               _version_cache_alloc += 16;
             t = realloc(_version_cache, 
                         _version_cache_alloc * sizeof(Version_Cache_Item));
             if (!t) return 0;
             _version_cache = t;
             _version_cache[_version_cache_num - 1].win = win;
             _version_cache[_version_cache_num - 1].ver = version;
          }
        return version;
     }

   if (_source->state == ECORE_X_DND_SOURCE_DRAGGING) 
     {
        _version_cache_num++;
        if (_version_cache_num > _version_cache_alloc)
          _version_cache_alloc += 16;
        t = realloc(_version_cache, 
                    _version_cache_alloc * sizeof(Version_Cache_Item));
        if (!t) return 0;
        _version_cache = t;
        _version_cache[_version_cache_num - 1].win = win;
        _version_cache[_version_cache_num - 1].ver = 0;
     }

   return 0;
}

EAPI Eina_Bool 
ecore_x_dnd_type_isset(Ecore_X_Window win, const char *type) 
{
   int num = 0, i = 0;
   Eina_Bool ret = EINA_FALSE;
   unsigned char *data;
   Ecore_X_Atom *atoms, atom;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_TYPE_LIST, 
                                         ECORE_X_ATOM_ATOM, 32, &data, &num))
     return ret;

   atom = ecore_x_atom_get(type);
   atoms = (Ecore_X_Atom *)data;
   for (i = 0; i < num; ++i) 
     {
        if (atom == atoms[i]) 
          {
             ret = EINA_TRUE;
             break;
          }
     }

   free(data);
   return ret;
}

EAPI void 
ecore_x_dnd_type_set(Ecore_X_Window win, const char *type, Eina_Bool on) 
{
   Ecore_X_Atom atom, *oldset = NULL, *newset = NULL;
   int i = 0, j = 0, num = 0;
   unsigned char *data = NULL, *old_data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   atom = ecore_x_atom_get(type);
   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_TYPE_LIST, 
                                    ECORE_X_ATOM_ATOM, 32, &old_data, &num);
   oldset = (Ecore_X_Atom *)old_data;
   if (on) 
     {
        if (ecore_x_dnd_type_isset(win, type)) 
          {
             free(old_data);
             return;
          }
        newset = calloc(num + 1, sizeof(Ecore_X_Atom));
        if (!newset) return;
        data = (unsigned char *)newset;
        for (i = 0; i < num; i++)
          newset[i + 1] = oldset[i];
        newset[0] = atom;
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST, 
                                         ECORE_X_ATOM_ATOM, 32, data, num + 1);
     }
   else 
     {
        if (!ecore_x_dnd_type_isset(win, type)) 
          {
             free(old_data);
             return;
          }
        newset = calloc(num - 1, sizeof(Ecore_X_Atom));
        if (!newset) 
          {
             free(old_data);
             return;
          }
        data = (unsigned char *)newset;
        for (i = 0; i < num; i++)
          if (oldset[i] != atom) 
            newset[j++] = oldset[i];
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST, 
                                         ECORE_X_ATOM_ATOM, 32, data, num - 1);
     }
   free(oldset);
   free(newset);
}

EAPI void
ecore_x_dnd_types_set(Ecore_X_Window win, const char **types, unsigned int num_types)
{
   Ecore_X_Atom *newset = NULL;
   unsigned int i;
   unsigned char *data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!num_types)
      ecore_x_window_prop_property_del(win, ECORE_X_ATOM_XDND_TYPE_LIST);
   else
     {
        newset = calloc(num_types, sizeof(Ecore_X_Atom));
        if (!newset) return;

        data = (unsigned char *)newset;
        for (i = 0; i < num_types; i++)
          {
             newset[i] = ecore_x_atom_get(types[i]);
             ecore_x_selection_converter_atom_add(newset[i],
                                                  _ecore_xcb_dnd_converter_copy);
          }
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         ECORE_X_ATOM_ATOM, 32, data, 
                                         num_types);
        free(newset);
     }
}

EAPI void
ecore_x_dnd_actions_set(Ecore_X_Window win, Ecore_X_Atom *actions, unsigned int num_actions)
{
   unsigned int i;
   unsigned char *data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!num_actions)
      ecore_x_window_prop_property_del(win, ECORE_X_ATOM_XDND_ACTION_LIST);
   else
     {
        data = (unsigned char *)actions;
        for (i = 0; i < num_actions; i++)
          ecore_x_selection_converter_atom_add(actions[i],
                                               _ecore_xcb_dnd_converter_copy);
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_ACTION_LIST,
                                         ECORE_X_ATOM_ATOM, 32, data, 
                                         num_actions);
     }
}

/**
 * The DND position update cb is called Ecore_X sends a DND position to a
 * client.
 *
 * It essentially mirrors some of the data sent in the position message.
 * Generally this cb should be set just before position update is called.
 * Please note well you need to look after your own data pointer if someone
 * trashes you position update cb set.
 *
 * It is considered good form to clear this when the dnd event finishes.
 *
 * @param cb Callback to updated each time ecore_x sends a position update.
 * @param data User data.
 */
EAPI void
ecore_x_dnd_callback_pos_update_set(void (*cb)(void *, Ecore_X_Xdnd_Position *data), const void *data)
{
   _posupdatecb = cb;
   _posupdatedata = (void *)data;
}

EAPI Eina_Bool 
ecore_x_dnd_begin(Ecore_X_Window source, unsigned char *data, int size) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_dnd_version_get(source)) return EINA_FALSE;

   /* Take ownership of XdndSelection */
   if (!ecore_x_selection_xdnd_set(source, data, size)) return EINA_FALSE;

   if (_version_cache)
     {
        free(_version_cache);
        _version_cache = NULL;
        _version_cache_num = 0;
        _version_cache_alloc = 0;
     }

   ecore_x_window_shadow_tree_flush();

   _source->win = source;
   ecore_x_window_ignore_set(_source->win, 1);
   _source->state = ECORE_X_DND_SOURCE_DRAGGING;
   _source->time = _ecore_xcb_events_last_time_get();
   _source->prev.window = 0;

   /* Default Accepted Action: move */
   _source->action = ECORE_X_ATOM_XDND_ACTION_MOVE;
   _source->accepted_action = XCB_NONE;
   _source->dest = XCB_NONE;

   return EINA_TRUE;
}

EAPI void 
ecore_x_dnd_send_finished(void) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_target->state == ECORE_X_DND_TARGET_IDLE) return;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.type = ECORE_X_ATOM_XDND_FINISHED;
   ev.window = _target->source;
   ev.data.data32[0] = _target->win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   if (_target->will_accept) 
     {
        ev.data.data32[1] |= 0x1UL;
        ev.data.data32[2] = _target->accepted_action;
     }

   xcb_send_event(_ecore_xcb_conn, 0, _target->source, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
   _target->state = ECORE_X_DND_TARGET_IDLE;
}

EAPI void 
ecore_x_dnd_source_action_set(Ecore_X_Atom action) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _source->action = action;
   if (_source->prev.window)
     _ecore_xcb_dnd_drag(_source->prev.window, 
                         _source->prev.x, _source->prev.y);
}

Ecore_X_DND_Source *
_ecore_xcb_dnd_source_get(void) 
{
   return _source;
}

Ecore_X_DND_Target *
_ecore_xcb_dnd_target_get(void) 
{
   return _target;
}

void 
_ecore_xcb_dnd_drag(Ecore_X_Window root, int x, int y) 
{
   xcb_client_message_event_t ev;
   Ecore_X_Window win, *skip;
   Ecore_X_Xdnd_Position pos;
   int num = 0;

   if (_source->state != ECORE_X_DND_SOURCE_DRAGGING) return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;

   skip = ecore_x_window_ignore_list(&num);
   win = ecore_x_window_shadow_tree_at_xy_with_skip_get(root, x, y, skip, num);
   while ((win) && !(ecore_x_dnd_version_get(win)))
     win = ecore_x_window_shadow_parent_get(root, win);

   if ((_source->dest) && (win != _source->dest)) 
     {
        ev.window = _source->dest;
        ev.type = ECORE_X_ATOM_XDND_LEAVE;
        ev.data.data32[0] = _source->win;
        ev.data.data32[1] = 0;

        xcb_send_event(_ecore_xcb_conn, 0, _source->dest, 
                       XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
        _source->suppress = 0;
     }

   if (win) 
     {
        int x1, x2, y1, y2;

        _source->version = MIN(ECORE_X_DND_VERSION, 
                               ecore_x_dnd_version_get(win));
        if (win != _source->dest) 
          {
             int i = 0;
             unsigned char *data;
             Ecore_X_Atom *types;

             ecore_x_window_prop_property_get(_source->win, 
                                              ECORE_X_ATOM_XDND_TYPE_LIST, 
                                              ECORE_X_ATOM_ATOM, 32, 
                                              &data, &num);
             types = (Ecore_X_Atom *)data;
             ev.window = win;
             ev.type = ECORE_X_ATOM_XDND_ENTER;
             ev.data.data32[0] = _source->win;
             ev.data.data32[1] = 0;
             if (num > 3)
               ev.data.data32[1] |= 0x1UL;
             else
               ev.data.data32[1] &= 0xfffffffeUL;
             ev.data.data32[1] |= ((unsigned long)_source->version) << 24;

             for (i = 2; i < 5; i++)
               ev.data.data32[i] = 0;
             for (i = 0; i < MIN(num, 3); ++i)
               ev.data.data32[i + 2] = types[i];
             free(data);

             xcb_send_event(_ecore_xcb_conn, 0, win, 
                            XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
             _source->await_status = 0;
             _source->will_accept = 0;
          }

        x1 = _source->rectangle.x;
        x2 = _source->rectangle.x + _source->rectangle.width;
        y1 = _source->rectangle.y;
        y2 = _source->rectangle.y + _source->rectangle.height;

        if ((!_source->await_status) || (!_source->suppress) || 
            ((x < x1) || (x > x2) || (y < y1) || (y > y2))) 
          {
             ev.window = win;
             ev.type = ECORE_X_ATOM_XDND_POSITION;
             ev.data.data32[0] = _source->win;
             ev.data.data32[1] = 0;
             ev.data.data32[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
             ev.data.data32[3] = _source->time;
             ev.data.data32[4] = _source->action;

             xcb_send_event(_ecore_xcb_conn, 0, win, 
                            XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
             _source->await_status = 1;
          }
     }

   if (_posupdatecb) 
     {
        pos.position.x = x;
        pos.position.y = y;
        pos.win = win;
        pos.prev = _source->dest;
        _posupdatecb(_posupdatedata, &pos);
     }

   _source->prev.x = x;
   _source->prev.y = y;
   _source->prev.window = root;
   _source->dest = win;
}

EAPI Ecore_X_Atom 
ecore_x_dnd_source_action_get(void) 
{
   return _source->action;
}

/* local functions */
static Eina_Bool 
_ecore_xcb_dnd_converter_copy(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *tprop __UNUSED__, int *count __UNUSED__) 
{
   Ecore_Xcb_Textproperty text_prop;
   Ecore_Xcb_Encoding_Style style = XcbTextStyle;
   char *mystr;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!data) || (!size)) return EINA_FALSE;

   mystr = calloc(1, size + 1);
   if (!mystr) return EINA_FALSE;

   memcpy(mystr, data, size);
   if (_ecore_xcb_mb_textlist_to_textproperty(&mystr, 1, style, &text_prop)) 
     {
        int len;

        len = strlen((char *)text_prop.value) + 1;
        if (!(*data_ret = malloc(len))) 
          {
             free(mystr);
             return EINA_FALSE;
          }
        memcpy(*data_ret, text_prop.value, len);
        *size_ret = len;
        free(text_prop.value);
        free(mystr);
        return EINA_TRUE;
     }
   else 
     {
        free(mystr);
        return EINA_FALSE;
     }
}
