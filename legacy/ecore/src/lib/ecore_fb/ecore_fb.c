
#include "Ecore.h"
#include "ecore_fb_private.h"
#include "Ecore_Fb.h"
#include "ecore_private.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <signal.h>

typedef struct _Ecore_Fb_Ts_Event Ecore_Fb_Ts_Event;
typedef struct _Ecore_Fb_Ts_Calibrate Ecore_Fb_Ts_Calibrate;
typedef struct _Ecore_Fb_Ts_Backlight Ecore_Fb_Ts_Backlight;
typedef struct _Ecore_Fb_Ts_Contrast Ecore_Fb_Ts_Contrast;
typedef struct _Ecore_Fb_Ts_Led Ecore_Fb_Ts_Led;
typedef struct _Ecore_Fb_Ts_Flite Ecore_Fb_Ts_Flite;
typedef struct _Ecore_Fb_Ps2_Event Ecore_Fb_Ps2_Event;

struct _Ecore_Fb_Ts_Event
{
   unsigned short pressure;
   unsigned short x;
   unsigned short y;
   unsigned short _unused; 
};

struct _Ecore_Fb_Ts_Calibrate
{
   int xscale;
   int xtrans;
   int yscale;
   int ytrans;
   int xyswap;
};

struct _Ecore_Fb_Ts_Backlight
{
   int           on;
   unsigned char brightness;
};

struct _Ecore_Fb_Ts_Contrast
{
   unsigned char contrast;
};

struct _Ecore_Fb_Ts_Led
{
   unsigned char on;
   unsigned char blink_time;
   unsigned char on_time;
   unsigned char off_time;
};

struct _Ecore_Fb_Ts_Flite
{
   unsigned char mode;
   unsigned char pwr;
   unsigned char brightness;
};

struct _Ecore_Fb_Ps2_Event
{
   unsigned char button;
   unsigned char x;
   unsigned char y;
   unsigned char z;
};

static void _ecore_fb_size_get(int *w, int *h);
static int _ecore_fb_ts_fd_handler(Ecore_Fd_Handler *fd_handler, void *data);
static int _ecore_fb_kbd_fd_handler(Ecore_Fd_Handler *fd_handler, void *data);
static int _ecore_fb_ps2_fd_handler(Ecore_Fd_Handler *fd_handler, void *data);
static void _ecore_fb_event_free_key_down(void *data, void *ev);
static void _ecore_fb_event_free_key_up(void *data, void *ev);
static void _ecore_fb_event_free_generic(void *data, void *ev);
static int _ecore_fb_signal_usr_handler(void *data, int type, void *ev);
static void _ecore_fb_vt_switch(int vt);
    
int ECORE_FB_EVENT_KEY_DOWN = 0;
int ECORE_FB_EVENT_KEY_UP = 0;
int ECORE_FB_EVENT_MOUSE_BUTTON_DOWN = 0;
int ECORE_FB_EVENT_MOUSE_BUTTON_UP = 0;
int ECORE_FB_EVENT_MOUSE_MOVE = 0;

static Ecore_Event_Handler *_ecore_fb_user_handler = NULL;
static Ecore_Event_Filter *_ecore_fb_filter_handler = NULL;
static Ecore_Fd_Handler *_ecore_fb_ts_fd_handler_handle = NULL;
static Ecore_Fd_Handler *_ecore_fb_kbd_fd_handler_handle = NULL;
static int _ecore_fb_init_count = 0;
static int _ecore_fb_ts_fd = 0;
static int _ecore_fb_ps2_fd = 0;
static int _ecore_fb_kbd_fd = 0;
static int _ecore_fb_tty_fd = 0;
static int _ecore_fb_console_w = 0;
static int _ecore_fb_console_h = 0;
static int _ecore_fb_ts_event_byte_count = 0;
static int _ecore_fb_ps2_event_byte_count = 0;
static Ecore_Fb_Ts_Event _ecore_fb_ts_event;
static Ecore_Fb_Ps2_Event _ecore_fb_ps2_event;
static int _ecore_fb_tty_prev_mode = 0;
static int _ecore_fb_tty_prev_kd_mode = 0;
static struct termios _ecore_fb_tty_prev_tio_mode;
static struct vt_mode _ecore_fb_vt_prev_mode;
static int _ecore_fb_current_vt = 0;
static int _ecore_fb_ctrl = 0;
static int _ecore_fb_alt = 0;

static void (*_ecore_fb_func_fb_lost) (void *data) = NULL;
static void *_ecore_fb_func_fb_lost_data = NULL;
static void (*_ecore_fb_func_fb_gain) (void *data) = NULL;
static void *_ecore_fb_func_fb_gain_data = NULL;

static void *_ecore_fb_event_filter_start(void *data);
static int   _ecore_fb_event_filter_filter(void *data, void *loop_data, int type, void *event);
static void  _ecore_fb_event_filter_end(void *data, void *loop_data);
					  
static double _ecore_fb_double_click_time = 0.25;

static char *_ecore_fb_kbd_syms[128] =
{
   "0x00",
     "Escape", 
     "1", 
     "2", 
     "3", 
     "4", 
     "5", 
     "6",
     "7", 
     "8", 
     "9", 
     "0", 
     "-", 
     "=", 
     "BackSpace", 
     "Rab",
     "q", 
     "w", 
     "e", 
     "r", 
     "t", 
     "y", 
     "u", 
     "i",
     "o", 
     "p", 
     "bracketleft", 
     "bracketright",
     "Return",
     "Control_L",
     "a", 
     "s", 
     "d", 
     "f", 
     "g", 
     "h", 
     "j", 
     "k", 
     "l", 
     "semicolon",
     "apostrophe", 
     "0x29", 
     "Shift_L", 
     "backslash", 
     "z", 
     "x", 
     "c", 
     "v",
     "b", 
     "n", 
     "m", 
     "comma", 
     "period", 
     "slash", 
     "Shift_R", 
     "KP_Multiply",
     "Alt_L", 
     "space", 
     "Caps_Lock", 
     "F1", 
     "F2", 
     "F3", 
     "F4", 
     "F5",
     "F6", 
     "F7", 
     "F8", 
     "F9", 
     "F10", 
     "Num_Lock", 
     "Scroll_Lock", 
     "KP_Home",
     "KP_Up", 
     "KP_Prior", 
     "KP_Subtract", 
     "KP_Left", 
     "KP_Begin", 
     "KP_Right", 
     "KP_Add", 
     "KP_End",
     "KP_Down", 
     "KP_Next", 
     "KP_Insert", 
     "KP_Delete", 
     "0x54", 
     "0x55", 
     "0x56", 
     "F11",
     "F12", 
     "0x59", 
     "0x5a", 
     "0x5b", 
     "0x5c", 
     "0x5d", 
     "0x5e", 
     "0x5f",
     "KP_Enter", 
     "Control_R", 
     "KP_Divide", 
     "Print", 
     "Alt_R", 
     "0x65", 
     "Home", 
     "Up",
     "Prior", 
     "Left", 
     "right", 
     "End", 
     "Down", 
     "Next", 
     "Insert", 
     "Delete",
     "0x70", 
     "0x71", 
     "0x72", 
     "0x73", 
     "0x74", 
     "0x75", 
     "0x76", 
     "Pause",
     "0x78", 
     "0x79", 
     "0x7a", 
     "0x7b", 
     "0x7c", 
     "Super_L", 
     "Super_R", 
     "0x7f"
};

static char *_ecore_fb_btn_syms[128] =
{
   "0x00",
     "Escape", 
     "F1", 
     "F2", 
     "F3", 
     "F4", 
     "Up", 
     "Right",
     "Left", 
     "Down", 
     "Return", 
     "0x1b", 
     "0x1c", 
     "0x1d", 
     "0x1e",
     "0x1f",
     "0x20", 
     "0x21", 
     "0x22", 
     "0x23", 
     "0x24", 
     "0x25", 
     "0x26", 
     "0x27",
     "0x28", 
     "0x29", 
     "0x2a",
     "0x2b",
     "0x2c",
     "0x2d",
     "0x2e", 
     "0x2f", 
     "0x30", 
     "0x31", 
     "0x32", 
     "0x33", 
     "0x34", 
     "0x35", 
     "0x36", 
     "0x37",
     "0x38", 
     "0x39", 
     "0x3a",
     "0x3b",
     "0x3c",
     "0x3d",
     "0x3e", 
     "0x3f", 
     "0x40", 
     "0x41", 
     "0x42", 
     "0x43", 
     "0x44", 
     "0x45", 
     "0x46", 
     "0x47",
     "0x48", 
     "0x49", 
     "0x4a",
     "0x4b",
     "0x4c",
     "0x4d",
     "0x4e", 
     "0x4f", 
     "0x50", 
     "0x51", 
     "0x52", 
     "0x53", 
     "0x54", 
     "0x55", 
     "0x56", 
     "0x57",
     "0x58", 
     "0x59", 
     "0x5a",
     "0x5b",
     "0x5c",
     "0x5d",
     "0x5e", 
     "0x5f", 
     "0x60", 
     "0x61", 
     "0x62", 
     "0x63", 
     "0x64", 
     "0x65", 
     "0x66", 
     "0x67",
     "0x68", 
     "0x69", 
     "0x6a",
     "0x6b",
     "0x6c",
     "0x6d",
     "0x6e", 
     "0x6f", 
     "0x70", 
     "0x71", 
     "0x72", 
     "0x73", 
     "0x74", 
     "0x75", 
     "0x76", 
     "0x77",
     "0x78", 
     "0x79", 
     "0x7a",
     "0x7b",
     "0x7c",
     "0x7d",
     "0x7e", 
     "0x7f"
};

/**
 * Set up the ecore fb system.
 * @param name device target name
 * @return 0 on failure, or greter than 0 on success.
 *
 * This starts up the ecore fb system
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_fb_init(const char *name)
{
   int prev_flags;

   _ecore_fb_init_count++;
   if (_ecore_fb_init_count > 1) return _ecore_fb_init_count;
   _ecore_fb_ts_fd = open("/dev/touchscreen/0", O_RDONLY);
   if (_ecore_fb_ts_fd >= 0)
     {
	prev_flags = fcntl(_ecore_fb_ts_fd, F_GETFL);
	fcntl(_ecore_fb_ts_fd, F_SETFL, prev_flags | O_NONBLOCK);
	_ecore_fb_ts_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_ts_fd, 
								   ECORE_FD_READ,
								   _ecore_fb_ts_fd_handler, NULL,
								   NULL, NULL);
	if (!_ecore_fb_ts_fd_handler_handle)
	  {
	     close(_ecore_fb_ts_fd);
	  }
     }
   if (_ecore_fb_ts_fd < 0)
     {
	_ecore_fb_ps2_fd = open("/dev/psaux", O_RDWR);
	if (_ecore_fb_ps2_fd >= 0)
	  {
	     prev_flags = fcntl(_ecore_fb_ps2_fd, F_GETFL);
	     fcntl(_ecore_fb_ps2_fd, F_SETFL, prev_flags | O_NONBLOCK);
	     _ecore_fb_ts_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_ps2_fd, 
									ECORE_FD_READ,
									_ecore_fb_ps2_fd_handler, NULL,
									NULL, NULL);
	     if (!_ecore_fb_ts_fd_handler_handle)
	       {
		  close(_ecore_fb_ps2_fd);
	       }
	  }
     }
   _ecore_fb_kbd_fd = open("/dev/touchscreen/key", O_RDONLY);
   _ecore_fb_tty_fd = open("/dev/tty0", O_RDONLY);
   if (_ecore_fb_tty_fd >= 0)
     {
	struct termios tio;
	struct vt_mode new_vtmode;
	int vtno;
	
	if ((ioctl(_ecore_fb_tty_fd, VT_OPENQRY, &vtno) != -1))
	  {
	     char buf[64];
	     
	     _ecore_fb_current_vt = vtno;
	     ioctl(_ecore_fb_tty_fd, VT_ACTIVATE, _ecore_fb_current_vt);
	     close(_ecore_fb_tty_fd);
	     snprintf(buf, sizeof(buf), "/dev/tty%i", _ecore_fb_current_vt);
/* FIXME: switch away works.. but switch to the allocated vt doesnt */	     
/*	     printf("%s\n", buf); */
	     _ecore_fb_tty_fd = open(buf, O_RDWR);
	     if (_ecore_fb_current_vt == 1)
	       {
		  Ecore_Event_Signal_User *e;
		  
		  e = _ecore_event_signal_user_new();
		  if (e)
		    {
		       e->number = 2;
		       _ecore_event_add(ECORE_EVENT_SIGNAL_USER, e,
					_ecore_event_signal_user_free, NULL);
		    }
	       }
	  }
	tcgetattr(_ecore_fb_tty_fd, &_ecore_fb_tty_prev_tio_mode);
	ioctl(_ecore_fb_tty_fd, KDGETMODE, &_ecore_fb_tty_prev_kd_mode);
	ioctl(_ecore_fb_tty_fd, KDGKBMODE, &_ecore_fb_tty_prev_mode);
	ioctl(_ecore_fb_tty_fd, VT_GETMODE, &_ecore_fb_vt_prev_mode);
	tio.c_iflag = tio.c_oflag = tio.c_cflag = tio.c_lflag = 0;
	tio.c_cc[VTIME] = 0;
	tio.c_cc[VMIN] = 1;
	new_vtmode.mode = VT_PROCESS;
	new_vtmode.waitv = 0;
	new_vtmode.relsig = SIGUSR1;
	new_vtmode.acqsig = SIGUSR2;
	tcsetattr(_ecore_fb_tty_fd, TCSAFLUSH, &tio);	
	ioctl(_ecore_fb_tty_fd, KDSETMODE, KD_GRAPHICS);	
	ioctl(_ecore_fb_tty_fd, KDSKBMODE, K_MEDIUMRAW);
	ioctl(_ecore_fb_tty_fd, VT_SETMODE, &new_vtmode);
     }
   if (_ecore_fb_kbd_fd <= 0) _ecore_fb_kbd_fd = _ecore_fb_tty_fd;
   if (_ecore_fb_kbd_fd >= 0)
     {
	prev_flags = fcntl(_ecore_fb_kbd_fd, F_GETFL);
	fcntl(_ecore_fb_kbd_fd, F_SETFL, prev_flags | O_NONBLOCK);
	_ecore_fb_kbd_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_kbd_fd,
								    ECORE_FD_READ,
								    _ecore_fb_kbd_fd_handler, NULL,
								    NULL, NULL);
	if (!_ecore_fb_kbd_fd_handler_handle)
	  {
	     tcsetattr(_ecore_fb_tty_fd, TCSAFLUSH, &_ecore_fb_tty_prev_tio_mode);	
	     ioctl(_ecore_fb_tty_fd, KDSETMODE, _ecore_fb_tty_prev_kd_mode);
	     ioctl(_ecore_fb_tty_fd, KDSKBMODE, _ecore_fb_tty_prev_mode);
	     close(_ecore_fb_tty_fd);
	     close(_ecore_fb_ts_fd);
	     close(_ecore_fb_kbd_fd);
	     if (_ecore_fb_ps2_fd > 0) close(_ecore_fb_ps2_fd);
	     if (_ecore_fb_tty_fd != _ecore_fb_kbd_fd) 
	       close(_ecore_fb_tty_fd);
	     ecore_main_fd_handler_del(_ecore_fb_ts_fd_handler_handle);
	     _ecore_fb_ts_fd_handler_handle = NULL;
	     _ecore_fb_init_count--;
	     return 0;
	  }
     }
   if (!ECORE_FB_EVENT_KEY_DOWN)
     {
	ECORE_FB_EVENT_KEY_DOWN          = ecore_event_type_new();
	ECORE_FB_EVENT_KEY_UP            = ecore_event_type_new();
	ECORE_FB_EVENT_MOUSE_BUTTON_DOWN = ecore_event_type_new();
	ECORE_FB_EVENT_MOUSE_BUTTON_UP   = ecore_event_type_new();
	ECORE_FB_EVENT_MOUSE_MOVE        = ecore_event_type_new();

     }
   _ecore_fb_size_get(&_ecore_fb_console_w, &_ecore_fb_console_h);
   _ecore_fb_user_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
						    _ecore_fb_signal_usr_handler,
						    NULL);
   _ecore_fb_filter_handler = ecore_event_filter_add(_ecore_fb_event_filter_start, _ecore_fb_event_filter_filter, _ecore_fb_event_filter_end, NULL);
   return _ecore_fb_init_count;
}

/**
 * Shut down the ecore fb system.
 * @return 0 if the system was shut down, or 1 if it still needs to be shut down
 *
 * This shuts down the ecore fb system.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_fb_shutdown(void)
{    
   _ecore_fb_init_count--;
   if (_ecore_fb_init_count > 0) return _ecore_fb_init_count;
   if (_ecore_fb_init_count < 0)
     {
	_ecore_fb_init_count = 0;
	return 0;
     }
   if (_ecore_fb_tty_fd != 0)
     {
	tcsetattr(_ecore_fb_tty_fd, TCSAFLUSH, &_ecore_fb_tty_prev_tio_mode);
	ioctl(_ecore_fb_tty_fd, KDSETMODE, _ecore_fb_tty_prev_kd_mode);
	ioctl(_ecore_fb_tty_fd, KDSKBMODE, _ecore_fb_tty_prev_mode);
	ioctl(_ecore_fb_tty_fd, VT_SETMODE, &_ecore_fb_vt_prev_mode);
	close(_ecore_fb_tty_fd);
	if (_ecore_fb_tty_fd == _ecore_fb_kbd_fd) _ecore_fb_kbd_fd = 0;
     }
   if (_ecore_fb_ps2_fd > 0) close(_ecore_fb_ps2_fd);
   if (_ecore_fb_ts_fd >= 0) close(_ecore_fb_ts_fd);
   if (_ecore_fb_kbd_fd >= 0) close(_ecore_fb_kbd_fd);
   if (_ecore_fb_ts_fd_handler_handle)
     ecore_main_fd_handler_del(_ecore_fb_ts_fd_handler_handle);
   if (_ecore_fb_kbd_fd_handler_handle) 
     ecore_main_fd_handler_del(_ecore_fb_kbd_fd_handler_handle);
   if (_ecore_fb_user_handler)
     ecore_event_handler_del(_ecore_fb_user_handler);
   ecore_event_filter_del(_ecore_fb_filter_handler);
   _ecore_fb_ts_fd = 0;
   _ecore_fb_kbd_fd = 0;
   _ecore_fb_tty_fd = 0;
   _ecore_fb_ps2_fd = 0;
   _ecore_fb_ts_fd_handler_handle = NULL;
   _ecore_fb_filter_handler = NULL;
   _ecore_fb_kbd_fd_handler_handle = NULL;
   _ecore_fb_user_handler = NULL;
   _ecore_fb_ctrl = 0;
   _ecore_fb_alt = 0;
   return _ecore_fb_init_count;
}

/**
 * Set the timeout for double/triple click to be flagged.
 * @param t The time in seconds
 *
 * This sets the time between clicks before the double_click flag is set in a 
 * button down event. If 3 clicks occur within double this time then the 
 * triple_click flag is also set.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_fb_double_click_time = t;
}

/**
 * Get the double/triple click timeout.
 * @return The timeout for double clicks in seconds
 *
 * This returns the tiemout that can be set by ecore_fb_double_click_time_set()
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
double
ecore_fb_double_click_time_get(void)
{
   return _ecore_fb_double_click_time;
}

/**
 * Get the current fb width and height in pixels.
 * @param w Width in pixels
 * @param h Height in pixels
 * 
 * Get the current fb width and height in pixels
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_size_get(int *w, int *h)
{
   if (w) *w = _ecore_fb_console_w;
   if (h) *h = _ecore_fb_console_h;
}

/**
 * Calibrate the touschreen.
 * @param xscale X scaling (256 = 1.0)
 * @param xtrans X translation
 * @param yscale Y scaling
 * @param ytrans Y translation
 * @param xyswap Swap X & Y flag
 * 
 * Calibrate the touchscreen using the above params
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_touch_screen_calibrate_set(int xscale, int xtrans, int yscale, int ytrans, int xyswap)
{
   Ecore_Fb_Ts_Calibrate cal;
   
   if (_ecore_fb_ts_fd < 0) return;
   cal.xscale = xscale;
   cal.xtrans = xtrans;
   cal.yscale = yscale;
   cal.ytrans = ytrans;
   cal.xyswap = xyswap;
   ioctl(_ecore_fb_ts_fd, TS_SET_CAL, (void *)&cal);
}

/**
 * Get calibratio og the touschreen.
 * @param xscale X scaling (256 = 1.0)
 * @param xtrans X translation
 * @param yscale Y scaling
 * @param ytrans Y translation
 * @param xyswap Swap X & Y flag
 * 
 * Get calibration of the touchscreen
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_touch_screen_calibrate_get(int *xscale, int *xtrans, int *yscale, int *ytrans, int *xyswap)
{
   Ecore_Fb_Ts_Calibrate cal;
   
   if (_ecore_fb_ts_fd < 0) return;
   ioctl(_ecore_fb_ts_fd, TS_GET_CAL, (void *)&cal);
   if (xscale) *xscale = cal.xscale;
   if (xtrans) *xtrans = cal.xtrans;
   if (yscale) *yscale = cal.yscale;
   if (ytrans) *ytrans = cal.ytrans;
   if (xyswap) *xyswap = cal.xyswap;
}

/**
 * Set the backlight.
 * @param on 1 or 0 corresponding to on or off
 * 
 * Set the backlight to the @p on state
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_backlight_set(int on)
{
   Ecore_Fb_Ts_Backlight bl;
   
   if (_ecore_fb_ts_fd < 0) return;
   ioctl(_ecore_fb_ts_fd, TS_GET_BACKLIGHT, &bl);
   bl.on = on;
   ioctl(_ecore_fb_ts_fd, TS_SET_BACKLIGHT, &bl);
}

/**
 * Get the backlight state.
 * @return The current backlight state
 * 
 * Get the current backlight state
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_fb_backlight_get(void)
{
   Ecore_Fb_Ts_Backlight bl;
   
   if (_ecore_fb_ts_fd < 0) return 1;
   ioctl(_ecore_fb_ts_fd, TS_GET_BACKLIGHT, &bl);
   return bl.on;
}

/**
 * Set the backlight brightness.
 * @param br Brightness 0.0 to 1.0
 * 
 * Set the backglith brightness to @p br, where 0 is darkest and 1.0 is 
 * the brightest.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void 
ecore_fb_backlight_brightness_set(double br)
{
   Ecore_Fb_Ts_Backlight bl;   
   int val;
   
   if (br < 0) br = 0;
   if (br > 1) br = 1;
   val = (int)(255.0 * br);
   ioctl(_ecore_fb_ts_fd, TS_GET_BACKLIGHT, &bl);
   bl.brightness = val;
   ioctl(_ecore_fb_ts_fd, TS_SET_BACKLIGHT, &bl);
}

/**
 * Get the backlight brightness.
 * @return The current backlight brigntess
 * 
 * Get the current backlight brightness as per 
 * ecore_fb_backlight_brightness_set().
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
double
ecore_fb_backlight_brightness_get(void)
{
   Ecore_Fb_Ts_Backlight bl;
   
   if (_ecore_fb_ts_fd < 0) return 1.0;
   ioctl(_ecore_fb_ts_fd, TS_GET_BACKLIGHT, &bl);
   return (double)bl.brightness / 255.0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_led_set(int on)
{
   Ecore_Fb_Ts_Led led;
   
   if (_ecore_fb_ts_fd < 0) return;
   if (on) led.on = 1;
   else led.on = 0;
   ioctl(_ecore_fb_ts_fd, LED_ON, &led);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_led_blink_set(double speed)
{
   Ecore_Fb_Ts_Led led;
   
   if (_ecore_fb_ts_fd < 0) return;
   led.on = 1;
   led.on_time = (unsigned char)(speed * 10);
   led.off_time = (unsigned char)(speed * 10);
   led.blink_time = 255;
   ioctl(_ecore_fb_ts_fd, LED_ON, &led);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void 
ecore_fb_contrast_set(double cr)
{
   Ecore_Fb_Ts_Contrast ct;
   int val;
   
   if (cr < 0) cr = 0;
   if (cr > 1) cr = 1;
   val = (int)(255.0 * cr);
   ct.contrast = val;
   ioctl(_ecore_fb_ts_fd, TS_SET_CONTRAST, &ct);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
double
ecore_fb_contrast_get(void)
{
   Ecore_Fb_Ts_Contrast ct;
   
   if (_ecore_fb_ts_fd < 0) return 1.0;
   ioctl(_ecore_fb_ts_fd, TS_GET_CONTRAST, &ct);
   return (double)ct.contrast / 255.0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
double
ecore_fb_light_sensor_get(void)
{
   Ecore_Fb_Ts_Flite fl;
   
   if (_ecore_fb_ts_fd < 0) return 0.0;
   fl.mode = 3;
   fl.brightness = 0;
   ioctl(_ecore_fb_ts_fd, FLITE_ON, &fl);   
   return (double)fl.brightness / 255.0;   
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_callback_gain_set(void (*func) (void *data), void *data)
{
   _ecore_fb_func_fb_gain = func;
   _ecore_fb_func_fb_gain_data = data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_fb_callback_lose_set(void (*func) (void *data), void *data)
{
   _ecore_fb_func_fb_lost = func;
   _ecore_fb_func_fb_lost_data = data;
}

static void
_ecore_fb_size_get(int *w, int *h)
{
   struct fb_var_screeninfo fb_var;
   int fb;
   
   fb = open("/dev/fb0", O_RDWR);
   if (fb < 0)
     {
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_var) == -1)
     {
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   close(fb);
   if (w) *w = fb_var.xres;
   if (h) *h = fb_var.yres;
}

static int
_ecore_fb_ts_fd_handler(Ecore_Fd_Handler *fd_handler, void *data)
{
   static int prev_x = 0, prev_y = 0, prev_pressure = 0;
   static double last_time = 0;
   static double last_last_time = 0;
   int v = 0;
   
   do
     {
	int x, y, pressure;
	int num;
	char *ptr;
	double t;
	int did_triple = 0;
	
	ptr = (char *)&(_ecore_fb_ts_event);
	ptr += _ecore_fb_ts_event_byte_count;
	num = sizeof(Ecore_Fb_Ts_Event) - _ecore_fb_ts_event_byte_count;
	v = read(_ecore_fb_ts_fd, ptr, num);
	if (v < 0) return 1;
	_ecore_fb_ts_event_byte_count += v;
	if (v < num) return 1;
	t = ecore_time_get();
	_ecore_fb_ts_event_byte_count = 0;
	x = _ecore_fb_ts_event.x;
	y = _ecore_fb_ts_event.y;
	pressure = _ecore_fb_ts_event.pressure;
	/* add event to queue */
	/* always add a move event */
	if ((pressure) || (prev_pressure))
	  {
	     /* MOVE: mouse is down and was */
	     Ecore_Fb_Event_Mouse_Move *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Mouse_Move));
	     if (!e) goto retry;
	     e->x = x;
	     e->y = y;
	     ecore_event_add(ECORE_FB_EVENT_MOUSE_MOVE, e, _ecore_fb_event_free_generic, NULL);
	  }
	if ((pressure) && (!prev_pressure))
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
	     ecore_event_add(ECORE_FB_EVENT_MOUSE_BUTTON_DOWN, e, _ecore_fb_event_free_generic, NULL);
	  }
	else if ((!pressure) && (prev_pressure))
	  {
	     /* UP: mouse was down, but is not now */
	     Ecore_Fb_Event_Mouse_Button_Up *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Mouse_Button_Up));
	     if (!e) goto retry;
	     e->x = prev_x;
	     e->y = prev_y;
	     e->button = 1;
	     ecore_event_add(ECORE_FB_EVENT_MOUSE_BUTTON_UP, e, _ecore_fb_event_free_generic, NULL);
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
	prev_pressure = pressure;
     }
   while (v > 0);
   return 1;
}

static int
_ecore_fb_kbd_fd_handler(Ecore_Fd_Handler *fd_handler, void *data)
{
   int v = 0;
   
   do
     {
	unsigned char buf;
	
	v = read(_ecore_fb_kbd_fd, &buf, 1);
	if (v < 0) return 1;
	if (v < 1) return 1;
	if (buf & 0x80)
	  {
	     /* DOWN */
	     int vt_switch = -1;
	     Ecore_Fb_Event_Key_Down *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Key_Down));
	     if (!e) goto retry;
	     if (_ecore_fb_kbd_fd == _ecore_fb_tty_fd)
	       e->keyname = strdup(_ecore_fb_kbd_syms[buf & 0x7f]);
	     else
	       e->keyname = strdup(_ecore_fb_btn_syms[buf & 0x7f]);
	     if (!e->keyname)
	       {
		  free(e);
		  goto retry;
	       }
	     ecore_event_add(ECORE_FB_EVENT_KEY_DOWN, e, _ecore_fb_event_free_key_down, NULL);
	     if (!strcmp(e->keyname, "Control_L"))
	       _ecore_fb_ctrl++;
	     if (!strcmp(e->keyname, "Control_R"))
	       _ecore_fb_ctrl++;
	     else if (!strcmp(e->keyname, "Alt_L"))
	       _ecore_fb_alt++;
	     else if (!strcmp(e->keyname, "Alt_R"))
	       _ecore_fb_alt++;
	     else if (!strcmp(e->keyname, "F1")) vt_switch = 0;
	     else if (!strcmp(e->keyname, "F2")) vt_switch = 1;
	     else if (!strcmp(e->keyname, "F3")) vt_switch = 2;
	     else if (!strcmp(e->keyname, "F4")) vt_switch = 3;
	     else if (!strcmp(e->keyname, "F5")) vt_switch = 4;
	     else if (!strcmp(e->keyname, "F6")) vt_switch = 5;
	     else if (!strcmp(e->keyname, "F7")) vt_switch = 6;
	     else if (!strcmp(e->keyname, "F8")) vt_switch = 7;
	     else if (!strcmp(e->keyname, "F9")) vt_switch = 8;
	     else if (!strcmp(e->keyname, "F10")) vt_switch = 9;
	     else if (!strcmp(e->keyname, "F11")) vt_switch = 10;
	     else if (!strcmp(e->keyname, "F12")) vt_switch = 11;
	     if (_ecore_fb_ctrl > 2) _ecore_fb_ctrl = 2;
	     if (_ecore_fb_alt > 2) _ecore_fb_alt = 2;
	     if ((vt_switch >= 0) &&
		 (_ecore_fb_ctrl) &&
		 (_ecore_fb_alt))
	       _ecore_fb_vt_switch(vt_switch);
	  }
	else
	  {
	     /* UP */
	     Ecore_Fb_Event_Key_Up *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Key_Up));
	     if (!e) goto retry;
	     if (_ecore_fb_kbd_fd == _ecore_fb_tty_fd)
	       e->keyname = strdup(_ecore_fb_kbd_syms[buf & 0x7f]);
	     else
	       e->keyname = strdup(_ecore_fb_btn_syms[buf & 0x7f]);
	     if (!e->keyname)
	       {
		  free(e);
		  goto retry;
	       }
	     ecore_event_add(ECORE_FB_EVENT_KEY_UP, e, _ecore_fb_event_free_key_up, NULL);
	     if (!strcmp(e->keyname, "Control_L"))
	       _ecore_fb_ctrl--;
	     if (!strcmp(e->keyname, "Control_R"))
	       _ecore_fb_ctrl--;
	     else if (!strcmp(e->keyname, "Alt_L"))
	       _ecore_fb_alt--;
	     else if (!strcmp(e->keyname, "Alt_R"))
	       _ecore_fb_alt--;
	     if (_ecore_fb_ctrl < 0) _ecore_fb_ctrl = 0;
	     if (_ecore_fb_alt < 0) _ecore_fb_alt = 0;
	  }
	retry:
     }
   while (v > 0);
   return 1;
}

static int
_ecore_fb_ps2_fd_handler(Ecore_Fd_Handler *fd_handler, void *data)
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
	     ecore_event_add(ECORE_FB_EVENT_MOUSE_MOVE, e, _ecore_fb_event_free_generic, NULL);
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
		  ecore_event_add(ECORE_FB_EVENT_MOUSE_BUTTON_DOWN, e, _ecore_fb_event_free_generic, NULL);
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
		  ecore_event_add(ECORE_FB_EVENT_MOUSE_BUTTON_UP, e, _ecore_fb_event_free_generic, NULL);
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

static void 
_ecore_fb_event_free_key_down(void *data, void *ev)
{
   Ecore_Fb_Event_Key_Up *e;
   
   e = ev;
   free(e->keyname);
   free(e);
}

static void
_ecore_fb_event_free_key_up(void *data, void *ev)
{
   Ecore_Fb_Event_Key_Up *e;
   
   e = ev;
   free(e->keyname);
   free(e);
}

static void
_ecore_fb_event_free_generic(void *data, void *ev)
{
   free(ev);
}

static int
_ecore_fb_signal_usr_handler(void *data, int type, void *ev)
{
   Ecore_Event_Signal_User *e;
   
   e = (Ecore_Event_Signal_User *)ev;
   if (e->number == 1)
     {
	/* release */
	if (_ecore_fb_func_fb_lost)
	  _ecore_fb_func_fb_lost(_ecore_fb_func_fb_lost_data);
	if (_ecore_fb_ps2_fd > 0) close(_ecore_fb_ps2_fd);
	if (_ecore_fb_ts_fd >= 0) close(_ecore_fb_ts_fd);
	if (_ecore_fb_ts_fd_handler_handle)
	  ecore_main_fd_handler_del(_ecore_fb_ts_fd_handler_handle);
	_ecore_fb_ps2_fd = 0;
	_ecore_fb_ts_fd = 0;
	_ecore_fb_ts_fd_handler_handle = NULL;
	ioctl(_ecore_fb_tty_fd, VT_RELDISP, 1);
     }
   else if (e->number == 2)
     {
	int prev_flags;
	
	/* gain */
	if (_ecore_fb_func_fb_gain)
	  _ecore_fb_func_fb_gain(_ecore_fb_func_fb_gain_data);
	if (!_ecore_fb_ts_fd)
	  {
	     _ecore_fb_ts_fd = open("/dev/touchscreen/0", O_RDONLY);
	     if (_ecore_fb_ts_fd >= 0)
	       {
		  prev_flags = fcntl(_ecore_fb_ts_fd, F_GETFL);
		  fcntl(_ecore_fb_ts_fd, F_SETFL, prev_flags | O_NONBLOCK);
		  _ecore_fb_ts_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_ts_fd, 
									     ECORE_FD_READ,
									     _ecore_fb_ts_fd_handler, NULL,
									     NULL, NULL);
		  if (!_ecore_fb_ts_fd_handler_handle)
		    {
		       close(_ecore_fb_ts_fd);
		    }
	       }
	  }
	if (_ecore_fb_ts_fd < 0)
	  {
	     if (!_ecore_fb_ps2_fd)
	       {
		  _ecore_fb_ps2_fd = open("/dev/psaux", O_RDWR);
		  if (_ecore_fb_ps2_fd >= 0)
		    {
		       prev_flags = fcntl(_ecore_fb_ps2_fd, F_GETFL);
		       fcntl(_ecore_fb_ps2_fd, F_SETFL, prev_flags | O_NONBLOCK);
		       _ecore_fb_ts_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_ps2_fd, 
										  ECORE_FD_READ,
										  _ecore_fb_ps2_fd_handler, NULL,
										  NULL, NULL);
		       if (!_ecore_fb_ts_fd_handler_handle)
			 {
			    close(_ecore_fb_ps2_fd);
			 }
		    }
	       }
	  }
	if (_ecore_fb_tty_fd != 0)
	  {
	     struct termios tio;
	     struct vt_mode new_vtmode;
	     
	     tio.c_iflag = tio.c_oflag = tio.c_cflag = tio.c_lflag = 0;
	     tio.c_cc[VTIME] = 0;
	     tio.c_cc[VMIN] = 1;
	     new_vtmode.mode = VT_PROCESS;
	     new_vtmode.waitv = 0;
	     new_vtmode.relsig = SIGUSR1;
	     new_vtmode.acqsig = SIGUSR2;
	     tcsetattr(_ecore_fb_tty_fd, TCSAFLUSH, &tio);	
	     ioctl(_ecore_fb_tty_fd, KDSETMODE, KD_GRAPHICS);	
	     ioctl(_ecore_fb_tty_fd, KDSKBMODE, K_MEDIUMRAW);
	     ioctl(_ecore_fb_tty_fd, VT_SETMODE, &new_vtmode);
	  }
     }
   return 1;
}

static void
_ecore_fb_vt_switch(int vt)
{
   vt++;
   if (_ecore_fb_tty_fd != 0)
     {
	if (vt != _ecore_fb_current_vt)
	  {
	     tcsetattr(_ecore_fb_tty_fd, TCSAFLUSH, &_ecore_fb_tty_prev_tio_mode);
	     ioctl(_ecore_fb_tty_fd, KDSETMODE, _ecore_fb_tty_prev_kd_mode);
	     ioctl(_ecore_fb_tty_fd, KDSKBMODE, _ecore_fb_tty_prev_mode);
	  }
     }
   ioctl(_ecore_fb_tty_fd, VT_ACTIVATE, vt);
}

typedef struct _Ecore_Fb_Filter_Data Ecore_Fb_Filter_Data;
   
struct _Ecore_Fb_Filter_Data
{
   int last_event_type;
};
                
static void *
_ecore_fb_event_filter_start(void *data)
{
   Ecore_Fb_Filter_Data *filter_data;
   
   filter_data = calloc(1, sizeof(Ecore_Fb_Filter_Data));
   return filter_data;
}

static int
_ecore_fb_event_filter_filter(void *data, void *loop_data,int type, void *event)
{
   Ecore_Fb_Filter_Data *filter_data;
   
   filter_data = loop_data;
   if (!filter_data) return 1;
   if (type == ECORE_FB_EVENT_MOUSE_MOVE)
     {
	if ((filter_data->last_event_type) == ECORE_FB_EVENT_MOUSE_MOVE)
	  {
	     filter_data->last_event_type = type;
	     return 0;
	  }
     }
   filter_data->last_event_type = type;
   return 1;
}

static void
_ecore_fb_event_filter_end(void *data, void *loop_data)
{
   Ecore_Fb_Filter_Data *filter_data;
   
   filter_data = loop_data;
   if (filter_data) free(filter_data);
}
