/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Ecore.h"
#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"


EAPI int ECORE_X_EVENT_XDND_ENTER    = 0;
EAPI int ECORE_X_EVENT_XDND_POSITION = 0;
EAPI int ECORE_X_EVENT_XDND_STATUS   = 0;
EAPI int ECORE_X_EVENT_XDND_LEAVE    = 0;
EAPI int ECORE_X_EVENT_XDND_DROP     = 0;
EAPI int ECORE_X_EVENT_XDND_FINISHED = 0;

static Ecore_X_DND_Source *_source = NULL;
static Ecore_X_DND_Target *_target = NULL;
static int _ecore_x_dnd_init_count = 0;


void
_ecore_x_dnd_init(void)
{
   if (!_ecore_x_dnd_init_count)
     {

	_source = calloc(1, sizeof(Ecore_X_DND_Source));
	_source->version = ECORE_X_DND_VERSION;
	_source->win = XCB_NONE;
	_source->dest = XCB_NONE;
	_source->state = ECORE_X_DND_SOURCE_IDLE;

	_target = calloc(1, sizeof(Ecore_X_DND_Target));
	_target->win = XCB_NONE;
	_target->source = XCB_NONE;
	_target->state = ECORE_X_DND_TARGET_IDLE;

	ECORE_X_EVENT_XDND_ENTER    = ecore_event_type_new();
	ECORE_X_EVENT_XDND_POSITION = ecore_event_type_new();
	ECORE_X_EVENT_XDND_STATUS   = ecore_event_type_new();
	ECORE_X_EVENT_XDND_LEAVE    = ecore_event_type_new();
	ECORE_X_EVENT_XDND_DROP     = ecore_event_type_new();
	ECORE_X_EVENT_XDND_FINISHED = ecore_event_type_new();
     }

   _ecore_x_dnd_init_count++;
}

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
}

EAPI void
ecore_x_dnd_aware_set(Ecore_X_Window window,
                      int            on)
{
   Ecore_X_Atom prop_data = ECORE_X_DND_VERSION;

   if (on)
     ecore_x_window_prop_property_set(window, ECORE_X_ATOM_XDND_AWARE,
				      ECORE_X_ATOM_ATOM, 32, &prop_data, 1);
   else
     ecore_x_window_prop_property_del(window, ECORE_X_ATOM_XDND_AWARE);
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 */
EAPI void
ecore_x_dnd_version_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       ECORE_X_ATOM_XDND_AWARE,
                                       ECORE_X_ATOM_ATOM,
                                       0, LONG_MAX);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_dnd_version_get_prefetch().
 */
EAPI void
ecore_x_dnd_version_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the DnD version.
 * @param  window Unused.
 * @return        0 on failure, the version otherwise.
 *
 * Get the DnD version. Returns 0 on failure, the version otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dnd_version_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_dnd_version_get_fetch(), which gets the reply.
 */
EAPI int
ecore_x_dnd_version_get(Ecore_X_Window window)
{
   unsigned char            *prop_data;
   int                       num;

   if (ecore_x_window_prop_property_get(window, ECORE_X_ATOM_XDND_AWARE,
                                        ECORE_X_ATOM_ATOM, 32, &prop_data, &num))
     {
	int version = (int) *prop_data;
	free(prop_data);
	return version;
     }
   else
     return 0;
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 */
EAPI void
ecore_x_dnd_type_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       ECORE_X_ATOM_XDND_TYPE_LIST,
                                       ECORE_X_ATOM_ATOM,
                                       0, LONG_MAX);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_dnd_type_get_prefetch().
 */
EAPI void
ecore_x_dnd_type_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/* FIXME: round trip (InternAtomGet request) */

/**
 * Check if the type is set.
 * @param   window Unused.
 * @param   type   The type to check
 * @return         0 on failure, 1 otherwise.
 *
 * Check if the type is set. 0 on failure, 1 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dnd_type_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_dnd_type_get_fetch(), which gets the reply.
 */
EAPI int
ecore_x_dnd_type_isset(Ecore_X_Window window,
                       const char    *type)
{
   xcb_intern_atom_cookie_t cookie;
   xcb_intern_atom_reply_t *reply;
   Ecore_X_Atom            *atoms;
   unsigned char           *data;
   int                      num;
   int                      i;
   uint8_t                  ret = 0;

   cookie = xcb_intern_atom_unchecked(_ecore_xcb_conn, 0,
                                      strlen(type), type);

   if (!ecore_x_window_prop_property_get(window, ECORE_X_ATOM_XDND_TYPE_LIST,
					 ECORE_X_ATOM_ATOM, 32, &data, &num))
     {
        reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) free(reply);
        return ret;
     }

   reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
     {
        free(data);
        return 0;
     }
   atoms = (Ecore_X_Atom *)data;

   for (i = 0; i < num; ++i)
     {
	if (reply->atom == atoms[i])
	  {
	     ret = 1;
	     break;
	  }
     }

   free(data);
   free(reply);

   return ret;
}

/* FIXME: round trip (InternAtomGet request) */

/**
 * Set the type.
 * @param   window Unused.
 * @param   type   The type to set
 * @param   on     0 or non 0...
 *
 * Set the type.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dnd_type_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_dnd_type_get_fetch(), which gets the reply.
 */
EAPI void
ecore_x_dnd_type_set(Ecore_X_Window window,
                     const char    *type,
                     int            on)
{
   xcb_intern_atom_cookie_t cookie;
   xcb_intern_atom_reply_t *reply;
   Ecore_X_Atom            *oldset = NULL;
   Ecore_X_Atom            *newset = NULL;
   unsigned char           *data = NULL;
   unsigned char           *old_data = NULL;
   Ecore_X_Atom             atom;
   int                      i, j = 0, num = 0;

   cookie = xcb_intern_atom_unchecked(_ecore_xcb_conn, 0,
                                      strlen(type), type);

   atom = ecore_x_atom_get(type);
   if (!ecore_x_window_prop_property_get(window, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         ECORE_X_ATOM_ATOM,
                                         32, &old_data, &num))
     {
        reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) free(reply);
        return;
     }
   oldset = (Ecore_X_Atom *)old_data;

   if (on)
     {
	if (ecore_x_dnd_type_isset(window, type))
	  {
	     free(old_data);
             reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
             if (reply) free(reply);
	     return;
	  }
	data = calloc(num + 1, sizeof(Ecore_X_Atom));
	if (!data)
          {
	     free(old_data);
             reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
             if (reply) free(reply);
             return;
          }
	newset = (Ecore_X_Atom *)data;

	for (i = 0; i < num; i++)
	  newset[i + 1] = oldset[i];
	/* prepend the new type */

        reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
        if (!reply)
          {
            free(old_data);
            return;
          }
	newset[0] = reply->atom;
        free(reply);

	ecore_x_window_prop_property_set(window,
                                         ECORE_X_ATOM_XDND_TYPE_LIST,
                                         ECORE_X_ATOM_ATOM,
                                         32, data, num + 1);
     }
   else
     {
	if (!ecore_x_dnd_type_isset(window, type))
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

	ecore_x_window_prop_property_set(window,
                                         ECORE_X_ATOM_XDND_TYPE_LIST,
                                         ECORE_X_ATOM_ATOM,
                                         32, data, num - 1);
     }

   free(oldset);
   free(newset);
}

/* FIXME: round trips, but I don't think we can do much, here */

/**
 * Set the types.
 * @param   window Unused.
 * @param   types  The types to set
 * @param   num_types The number of types
 *
 * Set the types.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dnd_type_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_dnd_type_get_fetch(), which gets the reply.
 */
EAPI void
ecore_x_dnd_types_set(Ecore_X_Window window,
                      char         **types,
                      unsigned int   num_types)
{
   Ecore_X_Atom *newset = NULL;
   void         *data = NULL;
   uint32_t      i;

   if (!num_types)
     {
	ecore_x_window_prop_property_del(window, ECORE_X_ATOM_XDND_TYPE_LIST);
     }
   else
     {
        xcb_intern_atom_cookie_t *cookies;
        xcb_intern_atom_reply_t  *reply;

        cookies = (xcb_intern_atom_cookie_t *)malloc(sizeof(xcb_intern_atom_cookie_t));
        if (!cookies) return;
	for (i = 0; i < num_types; i++)
	  cookies[i] = xcb_intern_atom_unchecked(_ecore_xcb_conn, 0,
                                                 strlen(types[i]), types[i]);
        data = calloc(num_types, sizeof(Ecore_X_Atom));
	if (!data)
          {
            for (i = 0; i < num_types; i++)
              {
                 reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookies[i], NULL);
                 if (reply) free(reply);
              }
            free(cookies);
            return;
          }
	newset = data;
	for (i = 0; i < num_types; i++)
          {
             reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookies[i], NULL);
             if (reply)
               {
                  newset[i] = reply->atom;
                  free(reply);
               }
             else
               newset[i] = XCB_NONE;
          }
        free(cookies);
	ecore_x_window_prop_property_set(window, ECORE_X_ATOM_XDND_TYPE_LIST,
                                         ECORE_X_ATOM_ATOM, 32, data, num_types);
	free(data);
     }
}

Ecore_X_DND_Source *
_ecore_x_dnd_source_get(void)
{
   return _source;
}

Ecore_X_DND_Target *
_ecore_x_dnd_target_get(void)
{
   return _target;
}

/**
 * Sends the GetProperty request.
 * @param source Window whose properties are requested.
 */
EAPI void
ecore_x_dnd_begin_prefetch(Ecore_X_Window source)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       source ? source : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       ECORE_X_ATOM_XDND_AWARE,
                                       ECORE_X_ATOM_ATOM,
                                       0, LONG_MAX);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_dnd_begin_prefetch().
 */
EAPI void
ecore_x_dnd_begin_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/* FIXME: round trip */

/**
 * Begins the DnD.
 * @param  source Unused.
 * @param  data   The data.
 * @param  size   The size of the data.
 * @return        0 on failure, 1 otherwise.
 *
 * Begins the DnD. Returns 0 on failure, 1 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dnd_begin_prefetch(), which sends the GetProperty request,
 * then ecore_x_dnd_begin_fetch(), which gets the reply.
 */
EAPI int
ecore_x_dnd_begin(Ecore_X_Window source,
                  unsigned char *data,
                  int            size)
{
   ecore_x_selection_xdnd_prefetch();
   if (!ecore_x_dnd_version_get(source))
     {
        ecore_x_selection_xdnd_fetch();
        return 0;
     }

   /* Take ownership of XdndSelection */
   ecore_x_selection_xdnd_prefetch();
   ecore_x_selection_xdnd_fetch();
   if (!ecore_x_selection_xdnd_set(source, data, size))
     return 0;

   _source->win = source;
   ecore_x_window_ignore_set(_source->win, 1);
   _source->state = ECORE_X_DND_SOURCE_DRAGGING;
   _source->time = _ecore_xcb_event_last_time;

   /* Default Accepted Action: ask */
   _source->action = ECORE_X_ATOM_XDND_ACTION_COPY;
   _source->accepted_action = XCB_NONE;
   return 1;
}

EAPI int
ecore_x_dnd_drop(void)
{
   uint8_t status = 0;

   if (_source->dest)
     {
        xcb_client_message_event_t ev;

	ev.response_type = XCB_CLIENT_MESSAGE;
	ev.format = 32;
	ev.window = _source->dest;

	if (_source->will_accept)
	  {
	     ev.type = ECORE_X_ATOM_XDND_DROP;
	     ev.data.data32[0] = _source->win;
	     ev.data.data32[1] = 0;
	     ev.data.data32[2] = _source->time;
	     xcb_send_event(_ecore_xcb_conn, 0, _source->dest, 0, (const char *)&ev);
	     _source->state = ECORE_X_DND_SOURCE_DROPPED;
	     status = 1;
	  }
	else
	  {
	     ev.type = ECORE_X_ATOM_XDND_LEAVE;
	     ev.data.data32[0] = _source->win;
	     ev.data.data32[1] = 0;
	     xcb_send_event(_ecore_xcb_conn, 0, _source->dest, 0, (const char *)&ev);
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

   _source->dest = XCB_NONE;

   return status;
}

EAPI void
ecore_x_dnd_send_status(int               will_accept,
                        int               suppress,
                        Ecore_X_Rectangle rectangle,
                        Ecore_X_Atom      action)
{
   xcb_client_message_event_t ev;

   if (_target->state == ECORE_X_DND_TARGET_IDLE)
     return;

   _target->will_accept = will_accept;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = _target->source;
   ev.type = ECORE_X_ATOM_XDND_STATUS;

   ev.data.data32[0] = _target->win;
   ev.data.data32[1] = 0;
   if (will_accept)
     ev.data.data32[1] |= 0x1UL;
   if (!suppress)
     ev.data.data32[1] |= 0x2UL;

   /* Set rectangle information */
   ev.data.data32[2] = rectangle.x;
   ev.data.data32[2] <<= 16;
   ev.data.data32[2] |= rectangle.y;
   ev.data.data32[3] = rectangle.width;
   ev.data.data32[3] <<= 16;
   ev.data.data32[3] |= rectangle.height;

   if (will_accept)
     {
	ev.data.data32[4] = action;
	_target->accepted_action = action;
     }
   else
     {
	ev.data.data32[4] = XCB_NONE;
	_target->accepted_action = action;
     }

   xcb_send_event(_ecore_xcb_conn, 0, _target->source, 0, (const char *)&ev);
}

EAPI void
ecore_x_dnd_send_finished(void)
{
   xcb_client_message_event_t ev;

   if (_target->state == ECORE_X_DND_TARGET_IDLE)
     return;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = _target->source;
   ev.type = ECORE_X_ATOM_XDND_FINISHED;

   ev.data.data32[0] = _target->win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   if (_target->will_accept)
     {
	ev.data.data32[1] |= 0x1UL;
	ev.data.data32[2] = _target->accepted_action;
     }
   xcb_send_event(_ecore_xcb_conn, 0, _target->source, 0, (const char *)&ev);

   _target->state = ECORE_X_DND_TARGET_IDLE;
}

void
_ecore_x_dnd_drag(Ecore_X_Window root,
		  int x,
                  int y)
{
   xcb_client_message_event_t ev;
   Ecore_X_Window             win;
   Ecore_X_Window            *skip;
   int                        num;

   if (_source->state != ECORE_X_DND_SOURCE_DRAGGING)
     return;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;

   /* Attempt to find a DND-capable window under the cursor */
   skip = ecore_x_window_ignore_list(&num);
//   win = ecore_x_window_at_xy_with_skip_get(x, y, skip, num);
   win = ecore_x_window_shadow_tree_at_xy_with_skip_get(root, x, y, skip, num);
   while (win)
     {
        xcb_query_tree_cookie_t cookie_tree;
        xcb_query_tree_reply_t *reply_tree;

        ecore_x_dnd_version_get_prefetch(win);
        cookie_tree = xcb_query_tree_unchecked(_ecore_xcb_conn, win);

        ecore_x_dnd_version_get_fetch();
        /* We found the correct window ? */
        if (ecore_x_dnd_version_get(win))
          {
             reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
             if (reply_tree) free(reply_tree);
             break;
          }
        reply_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_tree, NULL);
        if (reply_tree)
          {
            win = reply_tree->parent;
            free(reply_tree);
          }
     }

   /* Send XdndLeave to current destination window if we have left it */
   if ((_source->dest) && (win != _source->dest))
     {
	ev.window = _source->dest;
	ev.type = ECORE_X_ATOM_XDND_LEAVE;
	ev.data.data32[0] = _source->win;
	ev.data.data32[1] = 0;

	xcb_send_event(_ecore_xcb_conn, 0, _source->dest, 0, (const char *)&ev);
	_source->suppress = 0;
     }

   if (win)
     {
	int16_t                   x1;
	int16_t                   x2;
	int16_t                   y1;
	int16_t                   y2;

        ecore_x_dnd_version_get_prefetch(win);
        ecore_x_dnd_type_get_prefetch(_source->win);

        ecore_x_dnd_version_get_fetch();
        if (!ecore_x_dnd_version_get(win))
          {
            ecore_x_dnd_type_get_fetch();
            return;
          }

	_source->version = MIN(ECORE_X_DND_VERSION,
                               ecore_x_dnd_version_get(win));
	if (win != _source->dest)
	  {
	     unsigned char *data;
	     Ecore_X_Atom  *types;
	     int            num;
	     int            i;

             ecore_x_dnd_type_get_fetch();
             if (!ecore_x_window_prop_property_get(_source->win,
                                                   ECORE_X_ATOM_XDND_TYPE_LIST,
                                                   ECORE_X_ATOM_ATOM,
                                                   32, &data, &num))
               return;

	     types = (Ecore_X_Atom *)data;

	     /* Entered new window, send XdndEnter */
	     ev.window = win;
	     ev.type = ECORE_X_ATOM_XDND_ENTER;
	     ev.data.data32[0] = _source->win;
	     ev.data.data32[1] = 0;
	     if (num > 3)
	       ev.data.data32[1] |= 0x1UL;
	     else
	       ev.data.data32[1] &= 0xfffffffeUL;
	     ev.data.data32[1] |= ((unsigned long) _source->version) << 24;

	     for (i = 2; i < 5; i++)
	       ev.data.data32[i] = 0;
	     for (i = 0; i < MIN(num, 3); ++i)
	       ev.data.data32[i + 2] = types[i];
	     free(data);
	     xcb_send_event(_ecore_xcb_conn, 0, win, 0, (const char *)&ev);
	     _source->await_status = 0;
	     _source->will_accept = 0;
	  }
        else
          ecore_x_dnd_type_get_fetch();

	/* Determine if we're still in the rectangle from the last status */
	x1 = _source->rectangle.x;
	x2 = _source->rectangle.x + _source->rectangle.width;
	y1 = _source->rectangle.y;
	y2 = _source->rectangle.y + _source->rectangle.height;

	if ((!_source->await_status) ||
	    (!_source->suppress) ||
	    ((x < x1) || (x > x2) || (y < y1) || (y > y2)))
	  {
	     ev.window = win;
	     ev.type = ECORE_X_ATOM_XDND_POSITION;
	     ev.data.data32[0] = _source->win;
	     ev.data.data32[1] = 0; /* Reserved */
	     ev.data.data32[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
	     ev.data.data32[3] = _source->time; /* Version 1 */
	     ev.data.data32[4] = _source->action; /* Version 2, Needs to be pre-set */
	     xcb_send_event(_ecore_xcb_conn, 0, win, 0, (const char *)&ev);

	     _source->await_status = 1;
	  }
     }

   _source->dest = win;
}
