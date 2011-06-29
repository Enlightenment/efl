#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

EAPI int ECORE_X_EVENT_XDND_ENTER = 0;
EAPI int ECORE_X_EVENT_XDND_POSITION = 0;
EAPI int ECORE_X_EVENT_XDND_STATUS = 0;
EAPI int ECORE_X_EVENT_XDND_LEAVE = 0;
EAPI int ECORE_X_EVENT_XDND_DROP = 0;
EAPI int ECORE_X_EVENT_XDND_FINISHED = 0;

static Ecore_X_DND_Source *_source = NULL;
static Ecore_X_DND_Target *_target = NULL;
static int _ecore_x_dnd_init_count = 0;

typedef struct _Version_Cache_Item
{
   Ecore_X_Window win;
   int            ver;
} Version_Cache_Item;
static Version_Cache_Item *_version_cache = NULL;
static int _version_cache_num = 0, _version_cache_alloc = 0;
static void (*_posupdatecb)(void *, Ecore_X_Xdnd_Position *);
static void *_posupdatedata;

void
_ecore_x_dnd_init(void)
{
   if (!_ecore_x_dnd_init_count)
     {
        _source = calloc(1, sizeof(Ecore_X_DND_Source));
        if (!_source) return;
        _source->version = ECORE_X_DND_VERSION;
        _source->win = None;
        _source->dest = None;
        _source->state = ECORE_X_DND_SOURCE_IDLE;
        _source->prev.window = 0;

        _target = calloc(1, sizeof(Ecore_X_DND_Target));
        if (!_target)
          {
             free(_source);
             _source = NULL;
             return;
          }
        _target->win = None;
        _target->source = None;
        _target->state = ECORE_X_DND_TARGET_IDLE;

        ECORE_X_EVENT_XDND_ENTER = ecore_event_type_new();
        ECORE_X_EVENT_XDND_POSITION = ecore_event_type_new();
        ECORE_X_EVENT_XDND_STATUS = ecore_event_type_new();
        ECORE_X_EVENT_XDND_LEAVE = ecore_event_type_new();
        ECORE_X_EVENT_XDND_DROP = ecore_event_type_new();
        ECORE_X_EVENT_XDND_FINISHED = ecore_event_type_new();
     }

   _ecore_x_dnd_init_count++;
} /* _ecore_x_dnd_init */

void
_ecore_x_dnd_shutdown(void)
{
   _ecore_x_dnd_init_count--;
   if (_ecore_x_dnd_init_count > 0)
      return;

   if (_source)
      free(_source);

   _source = NULL;

   if (_target)
      free(_target);

   _target = NULL;

   _ecore_x_dnd_init_count = 0;
} /* _ecore_x_dnd_shutdown */

static Eina_Bool
_ecore_x_dnd_converter_copy(char *target  __UNUSED__,
                            void         *data,
                            int           size,
                            void        **data_ret,
                            int          *size_ret,
                            Ecore_X_Atom *tprop __UNUSED__,
                            int          *count __UNUSED__)
{
   XTextProperty text_prop;
   char *mystr;
   XICCEncodingStyle style = XTextStyle;

   if (!data || !size)
      return EINA_FALSE;

   mystr = calloc(1, size + 1);
   if (!mystr)
      return EINA_FALSE;

   memcpy(mystr, data, size);

   if (XmbTextListToTextProperty(_ecore_x_disp, &mystr, 1, style,
                                 &text_prop) == Success)
     {
        int bufsize = strlen((char *)text_prop.value) + 1;
        *data_ret = malloc(bufsize);
        if (!*data_ret)
          {
             free(mystr);
             return EINA_FALSE;
          }
        memcpy(*data_ret, text_prop.value, bufsize);
        *size_ret = bufsize;
        XFree(text_prop.value);
        free(mystr);
        return EINA_TRUE;
     }
   else
     {
        free(mystr);
        return EINA_FALSE;
     }
} /* _ecore_x_dnd_converter_copy */

EAPI void
ecore_x_dnd_aware_set(Ecore_X_Window win, Eina_Bool on)
{
   Ecore_X_Atom prop_data = ECORE_X_DND_VERSION;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (on)
      ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_AWARE,
                                       XA_ATOM, 32, &prop_data, 1);
   else
      ecore_x_window_prop_property_del(win, ECORE_X_ATOM_XDND_AWARE);
} /* ecore_x_dnd_aware_set */

EAPI int
ecore_x_dnd_version_get(Ecore_X_Window win)
{
   unsigned char *prop_data;
   int num;
   Version_Cache_Item *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   // this looks hacky - and it is, but we need a way of caching info about
   // a window while dragging, because we literally query this every mouse
   // move and going to and from x multiple times per move is EXPENSIVE
   // and slows things down, puts lots of load on x etc.
   if (_source->state == ECORE_X_DND_SOURCE_DRAGGING)
      if (_version_cache)
        {
           int i;

           for (i = 0; i < _version_cache_num; i++)
             {
                if (_version_cache[i].win == win)
                   return _version_cache[i].ver;
             }
        }

   if (ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_AWARE,
                                        XA_ATOM, 32, &prop_data, &num))
     {
        int version = (int)*prop_data;
        free(prop_data);
        if (_source->state == ECORE_X_DND_SOURCE_DRAGGING)
          {
             _version_cache_num++;
             if (_version_cache_num > _version_cache_alloc)
                _version_cache_alloc += 16;

             t = realloc(_version_cache,
                         _version_cache_alloc *
                         sizeof(Version_Cache_Item));
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

        t = realloc(_version_cache, _version_cache_alloc *
                    sizeof(Version_Cache_Item));
        if (!t) return 0;
        _version_cache = t;
        _version_cache[_version_cache_num - 1].win = win;
        _version_cache[_version_cache_num - 1].ver = 0;
     }

   return 0;
} /* ecore_x_dnd_version_get */

EAPI Eina_Bool
ecore_x_dnd_type_isset(Ecore_X_Window win, const char *type)
{
   int num, i, ret = EINA_FALSE;
   unsigned char *data;
   Ecore_X_Atom *atoms, atom;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, &data, &num))
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

   XFree(data);
   return ret;
} /* ecore_x_dnd_type_isset */

EAPI void
ecore_x_dnd_type_set(Ecore_X_Window win, const char *type, Eina_Bool on)
{
   Ecore_X_Atom atom;
   Ecore_X_Atom *oldset = NULL, *newset = NULL;
   int i, j = 0, num = 0;
   unsigned char *data = NULL;
   unsigned char *old_data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   atom = ecore_x_atom_get(type);
   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Ecore_X_Atom *)old_data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (on)
     {
        if (ecore_x_dnd_type_isset(win, type))
          {
             XFree(old_data);
             return;
          }

        newset = calloc(num + 1, sizeof(Ecore_X_Atom));
        if (!newset)
           return;

        data = (unsigned char *)newset;

        for (i = 0; i < num; i++)
           newset[i + 1] = oldset[i];
        /* prepend the new type */
        newset[0] = atom;

        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, data, num + 1);
     }
   else
     {
        if (!ecore_x_dnd_type_isset(win, type))
          {
             XFree(old_data);
             return;
          }

        newset = calloc(num - 1, sizeof(Ecore_X_Atom));
        if (!newset)
          {
             XFree(old_data);
             return;
          }

        data = (unsigned char *)newset;
        for (i = 0; i < num; i++)
           if (oldset[i] != atom)
              newset[j++] = oldset[i];

        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, data, num - 1);
     }

   XFree(oldset);
   free(newset);
} /* ecore_x_dnd_type_set */

EAPI void
ecore_x_dnd_types_set(Ecore_X_Window win,
                      const char   **types,
                      unsigned int   num_types)
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
        if (!newset)
           return;

        data = (unsigned char *)newset;
        for (i = 0; i < num_types; i++)
          {
             newset[i] = ecore_x_atom_get(types[i]);
             ecore_x_selection_converter_atom_add(newset[i],
                                                  _ecore_x_dnd_converter_copy);
          }
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, data, num_types);
        free(newset);
     }
} /* ecore_x_dnd_types_set */

EAPI void
ecore_x_dnd_actions_set(Ecore_X_Window win,
                        Ecore_X_Atom  *actions,
                        unsigned int   num_actions)
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
          {
             ecore_x_selection_converter_atom_add(actions[i],
                                                  _ecore_x_dnd_converter_copy);
          }
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_ACTION_LIST,
                                         XA_ATOM, 32, data, num_actions);
     }
} /* ecore_x_dnd_actions_set */

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
ecore_x_dnd_callback_pos_update_set(
                           void (*cb)(void *, Ecore_X_Xdnd_Position *data),
			   const void *data)
{
   _posupdatecb = cb;
   _posupdatedata = (void *)data; /* Discard the const early */
}

Ecore_X_DND_Source *
_ecore_x_dnd_source_get(void)
{
   return _source;
} /* _ecore_x_dnd_source_get */

Ecore_X_DND_Target *
_ecore_x_dnd_target_get(void)
{
   return _target;
} /* _ecore_x_dnd_target_get */

EAPI Eina_Bool
ecore_x_dnd_begin(Ecore_X_Window source, unsigned char *data, int size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!ecore_x_dnd_version_get(source))
      return EINA_FALSE;

   /* Take ownership of XdndSelection */
   if (!ecore_x_selection_xdnd_set(source, data, size))
      return EINA_FALSE;

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
   _source->time = _ecore_x_event_last_time;
   _source->prev.window = 0;

   /* Default Accepted Action: move */
   _source->action = ECORE_X_ATOM_XDND_ACTION_MOVE;
   _source->accepted_action = None;
   _source->dest = None;

   return EINA_TRUE;
} /* ecore_x_dnd_begin */

EAPI Eina_Bool
ecore_x_dnd_drop(void)
{
   XEvent xev;
   int status = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (_source->dest)
     {
        xev.xany.type = ClientMessage;
        xev.xany.display = _ecore_x_disp;
        xev.xclient.format = 32;
        xev.xclient.window = _source->dest;

        if (_source->will_accept)
          {
             xev.xclient.message_type = ECORE_X_ATOM_XDND_DROP;
             xev.xclient.data.l[0] = _source->win;
             xev.xclient.data.l[1] = 0;
             xev.xclient.data.l[2] = _source->time;
             XSendEvent(_ecore_x_disp, _source->dest, False, 0, &xev);
             _source->state = ECORE_X_DND_SOURCE_DROPPED;
             status = EINA_TRUE;
          }
        else
          {
             xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
             xev.xclient.data.l[0] = _source->win;
             xev.xclient.data.l[1] = 0;
             XSendEvent(_ecore_x_disp, _source->dest, False, 0, &xev);
             _source->state = ECORE_X_DND_SOURCE_IDLE;
          }
     }
   else
     {
        /* Dropping on nothing */
        ecore_x_selection_xdnd_clear();
        _source->state = ECORE_X_DND_SOURCE_IDLE;
     }

   ecore_x_window_ignore_set(_source->win, 0);

   _source->prev.window = 0;

   return status;
} /* ecore_x_dnd_drop */

EAPI void
ecore_x_dnd_send_status(Eina_Bool         will_accept,
                        Eina_Bool         suppress,
                        Ecore_X_Rectangle rectangle,
                        Ecore_X_Atom      action)
{
   XEvent xev;

   if (_target->state == ECORE_X_DND_TARGET_IDLE)
      return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   memset(&xev, 0, sizeof(XEvent));

   _target->will_accept = will_accept;

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.message_type = ECORE_X_ATOM_XDND_STATUS;
   xev.xclient.format = 32;
   xev.xclient.window = _target->source;

   xev.xclient.data.l[0] = _target->win;
   xev.xclient.data.l[1] = 0;
   if (will_accept)
      xev.xclient.data.l[1] |= 0x1UL;

   if (!suppress)
      xev.xclient.data.l[1] |= 0x2UL;

   /* Set rectangle information */
   xev.xclient.data.l[2] = rectangle.x;
   xev.xclient.data.l[2] <<= 16;
   xev.xclient.data.l[2] |= rectangle.y;
   xev.xclient.data.l[3] = rectangle.width;
   xev.xclient.data.l[3] <<= 16;
   xev.xclient.data.l[3] |= rectangle.height;

   if (will_accept)
     {
        xev.xclient.data.l[4] = action;
        _target->accepted_action = action;
     }
   else
     {
        xev.xclient.data.l[4] = None;
        _target->accepted_action = action;
     }

   XSendEvent(_ecore_x_disp, _target->source, False, 0, &xev);
} /* ecore_x_dnd_send_status */

EAPI void
ecore_x_dnd_send_finished(void)
{
   XEvent xev;

   if (_target->state == ECORE_X_DND_TARGET_IDLE)
      return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.message_type = ECORE_X_ATOM_XDND_FINISHED;
   xev.xclient.format = 32;
   xev.xclient.window = _target->source;

   xev.xclient.data.l[0] = _target->win;
   xev.xclient.data.l[1] = 0;
   xev.xclient.data.l[2] = 0;
   if (_target->will_accept)
     {
        xev.xclient.data.l[1] |= 0x1UL;
        xev.xclient.data.l[2] = _target->accepted_action;
     }

   XSendEvent(_ecore_x_disp, _target->source, False, 0, &xev);

   _target->state = ECORE_X_DND_TARGET_IDLE;
} /* ecore_x_dnd_send_finished */

EAPI void
ecore_x_dnd_source_action_set(Ecore_X_Atom action)
{
   _source->action = action;
   if (_source->prev.window)
      _ecore_x_dnd_drag(_source->prev.window, _source->prev.x, _source->prev.y);
} /* ecore_x_dnd_source_action_set */

EAPI Ecore_X_Atom
ecore_x_dnd_source_action_get(void)
{
   return _source->action;
} /* ecore_x_dnd_source_action_get */

void
_ecore_x_dnd_drag(Ecore_X_Window root, int x, int y)
{
   XEvent xev;
   Ecore_X_Window win;
   Ecore_X_Window *skip;
   Ecore_X_Xdnd_Position pos;
   int num;

   if (_source->state != ECORE_X_DND_SOURCE_DRAGGING)
      return;

   /* Preinitialize XEvent struct */
   memset(&xev, 0, sizeof(XEvent));
   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.format = 32;

   /* Attempt to find a DND-capable window under the cursor */
   skip = ecore_x_window_ignore_list(&num);
// WARNING - this function is HEAVY. it goes to and from x a LOT walking the
// window tree - use the SHADOW version - makes a 1-off tree copy, then uses
// that instead.
//   win = ecore_x_window_at_xy_with_skip_get(x, y, skip, num);
   win = ecore_x_window_shadow_tree_at_xy_with_skip_get(root, x, y, skip, num);

// NOTE: This now uses the shadow version to find parent windows
//   while ((win) && !(ecore_x_dnd_version_get(win)))
//     win = ecore_x_window_parent_get(win);
   while ((win) && !(ecore_x_dnd_version_get(win)))
      win = ecore_x_window_shadow_parent_get(root, win);

   /* Send XdndLeave to current destination window if we have left it */
   if ((_source->dest) && (win != _source->dest))
     {
        xev.xclient.window = _source->dest;
        xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
        xev.xclient.data.l[0] = _source->win;
        xev.xclient.data.l[1] = 0;

        XSendEvent(_ecore_x_disp, _source->dest, False, 0, &xev);
        _source->suppress = 0;
     }

   if (win)
     {
        int x1, x2, y1, y2;

        _source->version = MIN(ECORE_X_DND_VERSION,
                               ecore_x_dnd_version_get(win));
        if (win != _source->dest)
          {
             int i;
             unsigned char *data;
             Ecore_X_Atom *types;

             ecore_x_window_prop_property_get(_source->win,
                                              ECORE_X_ATOM_XDND_TYPE_LIST,
                                              XA_ATOM,
                                              32,
                                              &data,
                                              &num);
             types = (Ecore_X_Atom *)data;

             /* Entered new window, send XdndEnter */
             xev.xclient.window = win;
             xev.xclient.message_type = ECORE_X_ATOM_XDND_ENTER;
             xev.xclient.data.l[0] = _source->win;
             xev.xclient.data.l[1] = 0;
             if (num > 3)
                xev.xclient.data.l[1] |= 0x1UL;
             else
                xev.xclient.data.l[1] &= 0xfffffffeUL;

             xev.xclient.data.l[1] |= ((unsigned long)_source->version) << 24;

             for (i = 2; i < 5; i++)
                xev.xclient.data.l[i] = 0;
             for (i = 0; i < MIN(num, 3); ++i)
                xev.xclient.data.l[i + 2] = types[i];
             XFree(data);
             XSendEvent(_ecore_x_disp, win, False, 0, &xev);
             _source->await_status = 0;
             _source->will_accept = 0;
          }

        /* Determine if we're still in the rectangle from the last status */
        x1 = _source->rectangle.x;
        x2 = _source->rectangle.x + _source->rectangle.width;
        y1 = _source->rectangle.y;
        y2 = _source->rectangle.y + _source->rectangle.height;

        if ((!_source->await_status) ||
            (!_source->suppress) ||
            ((x < x1) || (x > x2) || (y < y1) || (y > y2)))
          {
             xev.xclient.window = win;
             xev.xclient.message_type = ECORE_X_ATOM_XDND_POSITION;
             xev.xclient.data.l[0] = _source->win;
             xev.xclient.data.l[1] = 0; /* Reserved */
             xev.xclient.data.l[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
             xev.xclient.data.l[3] = _source->time; /* Version 1 */
             xev.xclient.data.l[4] = _source->action; /* Version 2, Needs to be pre-set */
             XSendEvent(_ecore_x_disp, win, False, 0, &xev);

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


} /* _ecore_x_dnd_drag */



/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
