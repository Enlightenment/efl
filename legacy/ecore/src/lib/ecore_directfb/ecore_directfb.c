#include "ecore_private.h"
#include "Ecore_DirectFB.h"
#include "ecore_directfb_private.h"
#include "ecore_directfb_keys.h"
#include "Ecore.h"

/* ecore_directfb */
/******************/
/* About */
/* with this you can create windows of directfb and handle events through ecore
 * TODO:
 * - handle all event types
 * - 
 * */

static int _ecore_directfb_init_count = 0;

static int _window_event_fd = 0;
static int _input_event_fd = 0;

static int _ecore_directfb_fullscreen_window_id = 0;
static int _cursor_x = 0;
static int _cursor_y = 0;

EAPI int ECORE_DIRECTFB_EVENT_POSITION = 0;
EAPI int ECORE_DIRECTFB_EVENT_SIZE = 0;
EAPI int ECORE_DIRECTFB_EVENT_CLOSE = 0;
EAPI int ECORE_DIRECTFB_EVENT_DESTROYED = 0;
EAPI int ECORE_DIRECTFB_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_DIRECTFB_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_DIRECTFB_EVENT_KEY_DOWN = 0;
EAPI int ECORE_DIRECTFB_EVENT_KEY_UP = 0;
EAPI int ECORE_DIRECTFB_EVENT_BUTTON_DOWN = 0;
EAPI int ECORE_DIRECTFB_EVENT_BUTTON_UP = 0;
EAPI int ECORE_DIRECTFB_EVENT_MOTION = 0;
EAPI int ECORE_DIRECTFB_EVENT_ENTER = 0;
EAPI int ECORE_DIRECTFB_EVENT_LEAVE = 0;
EAPI int ECORE_DIRECTFB_EVENT_WHEEL = 0;


static Ecore_Fd_Handler *_window_event_fd_handler_handle = NULL;
static Ecore_Fd_Handler *_input_event_fd_handler_handle = NULL;

/* this hash is to store all the possible key names for fast lookup */
static Eina_Hash *_ecore_directfb_key_symbols_hash = NULL;


static IDirectFB *_dfb = NULL;			// the main interface
static IDirectFBEventBuffer *_window_event;	// the main event buffer (all windows are attached to this)
static IDirectFBEventBuffer *_input_event;	// the main event buffer (all windows are attached to this)
static IDirectFBDisplayLayer *_layer;		// the main layer
static DFBResult _err;				// usefull for DFBCHECK


/*******************/
/* local functions */
/*******************/

/* free ecore directfb events functions */
/****************************************/

static void 
_ecore_directfb_event_free_key_down(void *data __UNUSED__, void *ev)
{
   Ecore_DirectFB_Event_Key_Up *e;
   
   e = ev;
   if(e->name) free(e->name);
   if (e->string) free(e->string);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_directfb_event_free_key_up(void *data __UNUSED__, void *ev)
{
   Ecore_DirectFB_Event_Key_Up *e;
   
   e = ev;
   if(e->name) free(e->name);
   if (e->string) free(e->string);
   if (e->key_compose) free(e->key_compose);
   free(e);
}


/* directfb window input events handler */
/****************************************/

static void
_ecore_directfb_event_handle_motion(DFBEvent *evt)
{

	Ecore_DirectFB_Event_Motion *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Motion));

	switch(evt->clazz)
	{
		case DFEC_INPUT:
			e->modifiers = 0;
			switch(evt->input.axis)
			{
				case DIAI_X:
					e->x = _cursor_x = evt->input.axisabs;
					e->y = _cursor_y;
					break;
				case DIAI_Y:
					e->y = _cursor_y = evt->input.axisabs;
					e->x = _cursor_x;
					break;
				case DIAI_Z:
					//_ecore_directfb_event_handle_wheel(evt);
					return;
				default:
					return;
			}
			e->win = _ecore_directfb_fullscreen_window_id;
			e->time = 0;
			break;
	      
		case DFEC_WINDOW:
			e->modifiers = 0;
			e->x = evt->window.x;
			e->y = evt->window.y;
			e->win = evt->window.window_id;
			e->time = 0;
			break;
		default:
			break;
	}
	ecore_event_add(ECORE_DIRECTFB_EVENT_MOTION, e, NULL, NULL);
}
static void
_ecore_directfb_event_handle_key_down(DFBEvent *evt)
{

	Ecore_DirectFB_Event_Key_Down *e;
	unsigned int key_symbol;
	struct keymap *k;
	
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Key_Down));
	
	switch(evt->clazz)
	{
		case DFEC_INPUT:
			key_symbol = evt->input.key_symbol;
			k = eina_hash_find(_ecore_directfb_key_symbols_hash, &key_symbol);

			if(!k) 
			{
				printf("error en el numero, %0X\n", evt->input.key_symbol);
				return;
			}
			e->name = strdup(k->name);
  			e->string = strdup(k->string);
	 		e->key_compose = NULL;
			e->win = _ecore_directfb_fullscreen_window_id;
			e->time = 0;
			break;
	      
		case DFEC_WINDOW:
			key_symbol = evt->window.key_symbol;
			k = eina_hash_find(_ecore_directfb_key_symbols_hash, &key_symbol);

			if(!k) 
			{
				printf("error en el numero, %0X\n", evt->window.key_symbol);
				return;
			}
			e->name = strdup(k->name);
  			e->string = strdup(k->string);
	 		e->key_compose = NULL;
			e->win = evt->window.window_id;
			e->time = 0;
			break;
		default:
			break;
	}
				
	ecore_event_add(ECORE_DIRECTFB_EVENT_KEY_DOWN, e, _ecore_directfb_event_free_key_down, NULL);
}
static void
_ecore_directfb_event_handle_key_up(DFBEvent *evt)
{
	Ecore_DirectFB_Event_Key_Up *e;
	unsigned int key_symbol;
	struct keymap *k;
	
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Key_Up));
	
	switch(evt->clazz)
	{
		case DFEC_INPUT:
			key_symbol = evt->input.key_symbol;
			k = eina_hash_find(_ecore_directfb_key_symbols_hash, &key_symbol);

			
			if(!k) 
			{
				printf("error en el numero, %0X\n", evt->input.key_symbol);
				return;
			}
			e->name = strdup(k->name);
  			e->string = strdup(k->string);
	 		e->key_compose = NULL;
			e->win = _ecore_directfb_fullscreen_window_id;
			e->time = 0;
			break;
	      
		case DFEC_WINDOW:
			key_symbol = evt->window.key_symbol;
			k = eina_hash_find(_ecore_directfb_key_symbols_hash, &key_symbol);

			if(!k) 
			{
				printf("error en el numero, %0X\n", evt->window.key_symbol);
				return;
			}
			e->name = strdup(k->name);
  			e->string = strdup(k->string);
	 		e->key_compose = NULL;
			e->win = evt->window.window_id;
			e->time = 0;
			break;
		default:
			break;
	}
	ecore_event_add(ECORE_DIRECTFB_EVENT_KEY_UP, e, _ecore_directfb_event_free_key_up, NULL);

}

static void
_ecore_directfb_event_handle_button_down(DFBEvent *evt)
{
	Ecore_DirectFB_Event_Button_Down *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Button_Down));

	switch(evt->clazz)
	{
		case DFEC_INPUT:
			e->button = evt->input.button + 1;
			e->modifiers = 0;
			DFBCHECK(_layer->GetCursorPosition(_layer,&e->x,&e->y));
			e->x = _cursor_x;
			e->y = _cursor_y;
			e->win = _ecore_directfb_fullscreen_window_id;
			e->time = 0;
			
			break;
	      
		case DFEC_WINDOW:
			e->button = evt->window.button + 1;
			e->modifiers = 0;
			e->x = evt->window.x;
			e->y = evt->window.y;
			e->win = evt->window.window_id;
			e->time = 0;
			break;
		default:
			break;
	}
			
	ecore_event_add(ECORE_DIRECTFB_EVENT_BUTTON_DOWN, e, NULL, NULL);

}

static void
_ecore_directfb_event_handle_button_up(DFBEvent *evt)
{
	Ecore_DirectFB_Event_Button_Up *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Button_Up));

	switch(evt->clazz)
	{
		case DFEC_INPUT:
			e->button = evt->input.button + 1;
			e->modifiers = 0;
			e->x = _cursor_x;
			e->y = _cursor_y; 
			e->win = _ecore_directfb_fullscreen_window_id;
			e->time = 0;
			
			break;
	      
		case DFEC_WINDOW:
			e->button = evt->window.button + 1;
			e->modifiers = 0;
			e->x = evt->window.x;
			e->y = evt->window.y;
			e->win = evt->window.window_id;
			e->time = 0;
			break;
		default:
			break;
	}
	ecore_event_add(ECORE_DIRECTFB_EVENT_BUTTON_UP, e, NULL, NULL);

}

static void
_ecore_directfb_event_handle_enter(DFBWindowEvent *evt)
{
	Ecore_DirectFB_Event_Enter *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Enter));

	e->modifiers = 0;
	e->x = evt->x;
	e->y = evt->y;
	e->win = evt->window_id;
	e->time = 0;
			
	ecore_event_add(ECORE_DIRECTFB_EVENT_ENTER, e, NULL, NULL);

}

static void
_ecore_directfb_event_handle_leave(DFBWindowEvent *evt)
{
	Ecore_DirectFB_Event_Leave *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Leave));

	e->modifiers = 0;
	e->x = evt->x;
	e->y = evt->y;
	e->win = evt->window_id;
	e->time = 0;
			
	ecore_event_add(ECORE_DIRECTFB_EVENT_LEAVE, e, NULL, NULL);

}

static void
_ecore_directfb_event_handle_wheel(DFBWindowEvent *evt)
{
	Ecore_DirectFB_Event_Wheel *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Wheel));

	// currently there's no direction (only up/down);
	e->direction = 0;
	e->z = evt->step;
	e->modifiers = 0;
	e->win = evt->window_id;
	e->time = 0;
			
	ecore_event_add(ECORE_DIRECTFB_EVENT_WHEEL, e, NULL, NULL);

}

static void
_ecore_directfb_event_handle_got_focus(DFBWindowEvent *evt)
{
	Ecore_DirectFB_Event_Got_Focus *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Got_Focus));

	e->win = evt->window_id;
	e->time = 0;
			
	ecore_event_add(ECORE_DIRECTFB_EVENT_GOT_FOCUS, e, NULL, NULL);

}

static void
_ecore_directfb_event_handle_lost_focus(DFBWindowEvent *evt)
{
	Ecore_DirectFB_Event_Lost_Focus *e;
	e = calloc(1, sizeof(Ecore_DirectFB_Event_Lost_Focus));

	e->win = evt->window_id;
	e->time = 0;
			
	ecore_event_add(ECORE_DIRECTFB_EVENT_LOST_FOCUS, e, NULL, NULL);

}


/* inputs and windows fds handlers */
/***********************************/
/* TODO fix this to handle windows and input events (fullscreen/window mode)
 * in fullscreen theres no window_id so get the id from a global var (only one fullscreen
 * window at a time */


static int
_ecore_directfb_input_event_fd_handler(void *data __UNUSED__,Ecore_Fd_Handler *fd_handler __UNUSED__)
{
	DFBEvent evt;
	int v = 0;
		
	v = read(_input_event_fd, &evt, sizeof(DFBEvent));
	if (v < 0) return 1;
	if (v < 1) return 1;
	
	/* we are getting duplicate events, only parse if we are in fullscreen */
	//if(_ecore_directfb_fullscreen_window_id == 0) break;
	if(evt.input.type == DIET_KEYPRESS)
		_ecore_directfb_event_handle_key_down(&evt);
	if(evt.input.type == DIET_KEYRELEASE)
		_ecore_directfb_event_handle_key_up(&evt);
	if(evt.input.type == DIET_BUTTONPRESS)
		_ecore_directfb_event_handle_button_down(&evt);
	if(evt.input.type == DIET_BUTTONRELEASE)
		_ecore_directfb_event_handle_button_up(&evt);
	if(evt.input.type == DIET_AXISMOTION)
		_ecore_directfb_event_handle_motion(&evt);

	return 1;
}
	
static int
_ecore_directfb_window_event_fd_handler(void *data __UNUSED__,Ecore_Fd_Handler *fd_handler __UNUSED__)
{
	DFBEvent evt;
	int v = 0;
		
	v = read(_window_event_fd, &evt, sizeof(DFBEvent));
	if (v < 0) return 1;
	if (v < 1) return 1;
			
	if(evt.window.type & DWET_POSITION)
		printf("position\n");
	if(evt.window.type & DWET_SIZE)
		printf("size\n");
	if(evt.window.type & DWET_CLOSE)
		printf("close\n");
	if(evt.window.type & DWET_DESTROYED)
		printf("destroyed\n");
	if(evt.window.type & DWET_GOTFOCUS)
		_ecore_directfb_event_handle_got_focus(&evt.window);
	if(evt.window.type & DWET_LOSTFOCUS)
		_ecore_directfb_event_handle_lost_focus(&evt.window);
	if(evt.window.type & DWET_KEYDOWN)
		_ecore_directfb_event_handle_key_down(&evt);
	if(evt.window.type & DWET_KEYUP)
		_ecore_directfb_event_handle_key_up(&evt);
	if(evt.window.type & DWET_BUTTONDOWN)
		_ecore_directfb_event_handle_button_down(&evt);
	if(evt.window.type & DWET_BUTTONUP)
		_ecore_directfb_event_handle_button_up(&evt);
	if(evt.window.type & DWET_MOTION)
		_ecore_directfb_event_handle_motion(&evt);
	if(evt.window.type & DWET_ENTER)
		_ecore_directfb_event_handle_enter(&evt.window);
	if(evt.window.type & DWET_LEAVE)
		_ecore_directfb_event_handle_leave(&evt.window);
	if(evt.window.type & DWET_WHEEL)
		_ecore_directfb_event_handle_wheel(&evt.window);
	return 1;
}
	
/* api functions */
/*****************/


EAPI IDirectFB *
ecore_directfb_interface_get(void)
{
	return _dfb;
}



EAPI Ecore_DirectFB_Window *
ecore_directfb_window_new(int x, int y, int w, int h)
{
	Ecore_DirectFB_Window *window;
	IDirectFBWindow *dfb_window;
	IDirectFBSurface *dfb_surface = NULL;
	DFBWindowDescription desc;
	DFBWindowID id;
	
	memset(&desc, 0, sizeof(DFBWindowDescription));
	desc.flags = (DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
	desc.posx = x;
	desc.posy = y;
	desc.width = w;
	desc.height = h;
	desc.caps = DWCAPS_ALPHACHANNEL;
	
	DFBCHECK(_layer->CreateWindow(_layer, &desc, &dfb_window));
	
	dfb_window->AttachEventBuffer(dfb_window, _window_event);
	dfb_window->SetOptions(dfb_window,DWOP_NONE);
	dfb_window->SetOpacity(dfb_window, 0xFF);
	
	DFBCHECK(dfb_window->GetID(dfb_window, &id));
	DFBCHECK(dfb_window->GetSurface(dfb_window,&dfb_surface));
	
	window = malloc(sizeof(Ecore_DirectFB_Window));	
	window->id = id;
	window->window = dfb_window;
	window->surface = dfb_surface;
	window->cursor = NULL;

	return window;
}

EAPI void
ecore_directfb_window_del(Ecore_DirectFB_Window *ecore_window)
{
	DFBCHECK(ecore_window->surface->Release(ecore_window->surface));
	DFBCHECK(ecore_window->window->Release(ecore_window->window));
	free(ecore_window);
}


EAPI void
ecore_directfb_window_move(Ecore_DirectFB_Window *ecore_window, int x, int y)
{
	DFBCHECK(ecore_window->window->MoveTo(ecore_window->window, x, y));
}

EAPI void
ecore_directfb_window_resize(Ecore_DirectFB_Window *ecore_window, int w, int h)
{
	DFBCHECK(ecore_window->window->Resize(ecore_window->window, w, h));
}

EAPI void 
ecore_directfb_window_focus(Ecore_DirectFB_Window *ecore_window)
{	
	DFBCHECK(ecore_window->window->RequestFocus(ecore_window->window));	
}

EAPI void 
ecore_directfb_window_hide(Ecore_DirectFB_Window *ecore_window)
{
	DFBCHECK(ecore_window->window->SetOpacity(ecore_window->window,0));

}

EAPI void
ecore_directfb_window_show(Ecore_DirectFB_Window *ecore_window)
{
	DFBCHECK(ecore_window->window->SetOpacity(ecore_window->window, 0xFF));
}

EAPI void
ecore_directfb_window_shaped_set(Ecore_DirectFB_Window *ecore_window, int set)
{
	DFBWindowOptions opts;
	
	DFBCHECK(ecore_window->window->GetOptions(ecore_window->window, &opts));
	if(set)
	{
		opts |= DWOP_SHAPED;
		opts |= DWOP_ALPHACHANNEL;
		DFBCHECK(ecore_window->window->SetOptions(ecore_window->window, opts));
	}
	else
	{
		opts &= ~DWOP_SHAPED;
		opts &= ~DWOP_ALPHACHANNEL;
		DFBCHECK(ecore_window->window->SetOptions(ecore_window->window, opts));
	}
}

EAPI void
ecore_directfb_window_cursor_show(Ecore_DirectFB_Window *ecore_window, int show)
{
	if(!show)
	{
		/* create an empty cursor and set it */
		IDirectFBSurface *cursor;
		DFBSurfaceDescription desc;
			
		memset(&desc, 0, sizeof(DFBSurfaceDescription));
		desc.flags = (DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PIXELFORMAT);
		desc.width = 1;
		desc.height = 1;
		desc.pixelformat = DSPF_A1;
			
		DFBCHECK(_dfb->CreateSurface(_dfb,&desc,&cursor));
		DFBCHECK(cursor->Clear(cursor,0,0,0,0));
		DFBCHECK(ecore_window->window->SetCursorShape(ecore_window->window, cursor, 0, 0));
	}
	else
	{
		/* we already have a cursor surface so set it*/
		if(ecore_window->cursor)
		{
			DFBCHECK(ecore_window->window->SetCursorShape(ecore_window->window, ecore_window->cursor->surface, ecore_window->cursor->hot_x, ecore_window->cursor->hot_y));
		}
		/* or just set the default directfb cursor */
		else
		{
			DFBCHECK(ecore_window->window->SetCursorShape(ecore_window->window, NULL, 0, 0));
		}
			
	}
}

EAPI void
ecore_directfb_window_cursor_set(Ecore_DirectFB_Window *ecore_window, Ecore_DirectFB_Cursor *cursor)
{
	if( (!cursor) && (ecore_window->cursor))
	{
		ecore_window->cursor = NULL;
		DFBCHECK(ecore_window->window->SetCursorShape(ecore_window->window, NULL, 0, 0));
		return;
	}
	if(cursor)	
	{
		ecore_window->cursor = cursor;
		DFBCHECK(ecore_window->window->SetCursorShape(ecore_window->window, cursor->surface, cursor->hot_x, cursor->hot_y));
		
	}

}

EAPI void 
ecore_directfb_window_fullscreen_set(Ecore_DirectFB_Window *ecore_window, int on)
{
	// always release the surface (we are going to get a new one in both cases)
	DFBCHECK(ecore_window->surface->Release(ecore_window->surface));
	if(on)
	{
		DFBCHECK(_layer->SetCooperativeLevel(_layer,DLSCL_EXCLUSIVE));
		DFBCHECK(_layer->GetSurface(_layer,&ecore_window->surface));
		DFBCHECK(_dfb->CreateInputEventBuffer(_dfb, DICAPS_ALL, DFB_FALSE, &_input_event));
		DFBCHECK(_input_event->CreateFileDescriptor(_input_event,&_input_event_fd));
		/* the event of axismove sends one axis at a time, so we must store both */
		DFBCHECK(_layer->GetCursorPosition(_layer,&_cursor_x,&_cursor_y));
		
		_input_event_fd_handler_handle = ecore_main_fd_handler_add(_input_event_fd,ECORE_FD_READ,_ecore_directfb_input_event_fd_handler, NULL,NULL,NULL);
		_ecore_directfb_fullscreen_window_id = ecore_window->id;
	}
	else
	{
		DFBCHECK(_input_event->Release(_input_event));	
		DFBCHECK(_layer->SetCooperativeLevel(_layer,DLSCL_SHARED));
		DFBCHECK(ecore_window->window->GetSurface(ecore_window->window, &ecore_window->surface));
		ecore_main_fd_handler_del(_input_event_fd_handler_handle);
		_ecore_directfb_fullscreen_window_id = 0;
	}
		
}

EAPI void
ecore_directfb_window_size_get(Ecore_DirectFB_Window *ecore_window, int *w, int *h)
{
	DFBCHECK(ecore_window->surface->GetSize(ecore_window->surface,w,h));
	return;
}

EAPI int
ecore_directfb_init(const char *name)
{
	int i = 0;
	
	_ecore_directfb_init_count++;
	if (_ecore_directfb_init_count > 1) return _ecore_directfb_init_count;
	
	DFBCHECK(DirectFBInit(NULL,NULL));
	DFBCHECK(DirectFBCreate(&_dfb));
	
	DFBCHECK(_dfb->GetDisplayLayer(_dfb, DLID_PRIMARY, &_layer));
	DFBCHECK(_layer->SetCooperativeLevel(_layer, DLSCL_SHARED));
	
	
	/* window events and fd */
	DFBCHECK(_dfb->CreateEventBuffer(_dfb, &_window_event));
	DFBCHECK(_window_event->CreateFileDescriptor(_window_event,&_window_event_fd));
	_window_event_fd_handler_handle = ecore_main_fd_handler_add(_window_event_fd,ECORE_FD_READ,_ecore_directfb_window_event_fd_handler, NULL,NULL,NULL);
	
	/* register ecore directfb events */
	ECORE_DIRECTFB_EVENT_POSITION	= ecore_event_type_new();
	ECORE_DIRECTFB_EVENT_SIZE = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_CLOSE = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_DESTROYED = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_GOT_FOCUS = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_LOST_FOCUS = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_KEY_DOWN = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_KEY_UP = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_BUTTON_DOWN = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_BUTTON_UP = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_MOTION = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_ENTER = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_LEAVE = ecore_event_type_new();;
	ECORE_DIRECTFB_EVENT_WHEEL = ecore_event_type_new();;

	/* create the hash table for the keynames */
	_ecore_directfb_key_symbols_hash = eina_hash_int32_new(free);
	for(i=0; i<_ecore_directfb_key_symbols_count; i++)
	{
		struct keymap *k;
		k = malloc(sizeof(struct keymap));
		k->name = _ecore_directfb_key_symbols[i].name;
		k->string = _ecore_directfb_key_symbols[i].string;
		eina_hash_add(_ecore_directfb_key_symbols_hash, &_ecore_directfb_key_symbols[i].id, k);
	}
	/* create the hash for the windows(key = windowid, val = Ecore_DirectFB_Window struct) */
	return _ecore_directfb_init_count;
}

EAPI int
ecore_directfb_shutdown(void)
{
	int i;

	_ecore_directfb_init_count--;
	if (_ecore_directfb_init_count > 0) return _ecore_directfb_init_count;
	if (_ecore_directfb_init_count < 0)
	{
		_ecore_directfb_init_count = 0;
		return 0;
	}
	ecore_main_fd_handler_del(_window_event_fd_handler_handle);
	eina_hash_free(_ecore_directfb_key_symbols_hash);

	if(_ecore_directfb_fullscreen_window_id)
	{
		DFBCHECK(_input_event->Release(_input_event));	
		ecore_main_fd_handler_del(_input_event_fd_handler_handle);
	}
	DFBCHECK(_window_event->Release(_window_event));	
	DFBCHECK(_layer->Release(_layer));
	DFBCHECK(_dfb->Release(_dfb));
	
	return 1;
}
