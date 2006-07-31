typedef struct _Ecore_Fb_Ps2_Event Ecore_Fb_Ps2_Event;
struct _Ecore_Fb_Ps2_Event
{
   unsigned char button;
   unsigned char x;
   unsigned char y;
   unsigned char z;
};

static int _ecore_fb_ps2_event_byte_count = 0;
static Ecore_Fb_Ps2_Event _ecore_fb_ps2_event;
static int _ecore_fb_ps2_fd = 0;
static int _ecore_fb_ps2_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);

int
ecore_fb_ps2_init(void)
{
   _ecore_fb_ps2_fd = open("/dev/psaux", O_RDWR);
   if (_ecore_fb_ps2_fd >= 0)
     {
	prev_flags = fcntl(_ecore_fb_ps2_fd, F_GETFL);
 	fcntl(_ecore_fb_ps2_fd, F_SETFL, prev_flags | O_NONBLOCK);
	_ecore_fb_ts_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_ps2_fd, 
									ECORE_FD_READ,
									_ecore_fb_ps2_fd_handler, NULL,	NULL, NULL);
	if (!_ecore_fb_ts_fd_handler_handle)
	 {
	   close(_ecore_fb_ps2_fd);
	   return 0;
         }
	return 1;
     }
  return 0;   
}

void
ecore_fb_ps2_shutdown(void)
{
   if (_ecore_fb_ps2_fd > 0) close(_ecore_fb_ps2_fd);
   _ecore_fb_ps2_fd = 0;
}

static int
_ecore_fb_ps2_fd_handler(void *data __UNUSED__, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   static int prev_x = 0, prev_y = 0, prev_button = 0;
   static double last_time = 0;
   static double last_last_time = 0;
   int v = 0;

   do
     {
	int x, y, button, i;
	int num;
	char *ptr;
	double t;
	int did_triple = 0;
	
	ptr = (char *)&(_ecore_fb_ps2_event);
	ptr += _ecore_fb_ps2_event_byte_count;
	num = sizeof(Ecore_Fb_Ps2_Event) - _ecore_fb_ps2_event_byte_count;
	v = read(_ecore_fb_ps2_fd, ptr, num);
	if (v < 0) return 1;
	_ecore_fb_ps2_event_byte_count += v;
	if (v < num) return 1;
	t = ecore_time_get();
	_ecore_fb_ps2_event_byte_count = 0;
	if (_ecore_fb_ps2_event.button & 0x10)
	  x = prev_x + (0xffffff00 | _ecore_fb_ps2_event.x);
	else
	  x = prev_x + _ecore_fb_ps2_event.x;
	if (_ecore_fb_ps2_event.button & 0x20)
	  y = prev_y - (0xffffff00 | _ecore_fb_ps2_event.y);
	else
	  y = prev_y - _ecore_fb_ps2_event.y;
	button = _ecore_fb_ps2_event.button & 0x7;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= _ecore_fb_console_w) x = _ecore_fb_console_w - 1;
	if (y >= _ecore_fb_console_h) y = _ecore_fb_console_h - 1;
	/* add event to queue */
	/* always add a move event */
	if (1)
	  {
	     /* MOVE: mouse is down and was */
	     Ecore_Fb_Event_Mouse_Move *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Mouse_Move));
	     if (!e) goto retry;
	     e->x = x;
	     e->y = y;
	     ecore_event_add(ECORE_FB_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	for (i = 1; i <= 3; i++)
	  {
	     int mask;
	     
	     mask = 1 << (i - 1);
	     if (((button & mask)) && (!(prev_button & mask)))
	       {
		  /* DOWN: mouse is down, but was not now */
		  Ecore_Fb_Event_Mouse_Button_Down *e;
		  
		  e = calloc(1, sizeof(Ecore_Fb_Event_Mouse_Button_Down));
		  if (!e) goto retry;
		  e->x = x;
		  e->y = y;
		  e->button = 1;
		  if ((t - last_time) <= _ecore_fb_double_click_time)
		    e->double_click = 1;
		  if ((t - last_last_time) <= (2 * _ecore_fb_double_click_time))
		    {
		       did_triple = 1;
		       e->triple_click = 1;
		    }
		  ecore_event_add(ECORE_FB_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
	       }
	     else if ((!(button & mask)) && ((prev_button & mask)))
	       {
		  /* UP: mouse was down, but is not now */
		  Ecore_Fb_Event_Mouse_Button_Up *e;
		  
		  e = calloc(1, sizeof(Ecore_Fb_Event_Mouse_Button_Up));
		  if (!e) goto retry;
		  e->x = x;
		  e->y = y;
		  e->button = 1;
		  ecore_event_add(ECORE_FB_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
	       }
	  }
	if (did_triple)
	  {
	     last_time = 0;
	     last_last_time = 0;
	  }
	else
	  {
	     last_last_time = last_time;
	     last_time = t;
	  }
	retry:	     
	prev_x = x;
	prev_y = y;
	prev_button = button;
     }
   while (v > 0);
   return 1;
}
/**
 * @defgroup Ecore_FB_Click_Group Framebuffer Double Click Functions
 *
 * Functions that deal with the double click time of the framebuffer.
 */

/**
 * Sets the timeout for a double and triple clicks to be flagged.
 * 
 * This sets the time between clicks before the double_click flag is
 * set in a button down event. If 3 clicks occur within double this
 * time, the triple_click flag is also set.
 *
 * @param   t The time in seconds
 * @ingroup Ecore_FB_Click_Group
 */
EAPI void
ecore_fb_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_fb_double_click_time = t;
}

/**
 * Retrieves the double and triple click flag timeout.
 *
 * See @ref ecore_x_double_click_time_set for more information.
 *
 * @return  The timeout for double clicks in seconds.
 * @ingroup Ecore_FB_Click_Group
 */
EAPI double
ecore_fb_double_click_time_get(void)
{
   return _ecore_fb_double_click_time;
}

