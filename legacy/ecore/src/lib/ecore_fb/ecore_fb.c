#include "Ecore_Fb.h"
#include "ecore_fb_private.h"

static void _ecore_fb_size_get(int *w, int *h);

EAPI int ECORE_FB_EVENT_KEY_DOWN = 0;
EAPI int ECORE_FB_EVENT_KEY_UP = 0;
EAPI int ECORE_FB_EVENT_MOUSE_BUTTON_DOWN = 0;
EAPI int ECORE_FB_EVENT_MOUSE_BUTTON_UP = 0;
EAPI int ECORE_FB_EVENT_MOUSE_MOVE = 0;
EAPI int ECORE_FB_EVENT_MOUSE_WHEEL = 0;

static int _ecore_fb_init_count = 0;
static int _ecore_fb_console_w = 0;
static int _ecore_fb_console_h = 0;

static double _ecore_fb_double_click_time = 0.25;


/**
 * @defgroup Ecore_FB_Library_Group Framebuffer Library Functions
 *
 * Functions used to set up and shut down the Ecore_Framebuffer functions.
 */

/**
 * Sets up the Ecore_Fb library.
 * @param   name device target name
 * @return  @c 0 on failure.  Otherwise, the number of times the library has
 *          been initialised without being shut down.
 * @ingroup Ecore_FB_Library_Group
 */
EAPI int
ecore_fb_init(const char *name __UNUSED__)
{
	if(!_ecore_fb_init_count)
	{
		if(!ecore_fb_vt_init()) return 0;
		ECORE_FB_EVENT_KEY_DOWN          = ecore_event_type_new();
		ECORE_FB_EVENT_KEY_UP            = ecore_event_type_new();
		ECORE_FB_EVENT_MOUSE_BUTTON_DOWN = ecore_event_type_new();
		ECORE_FB_EVENT_MOUSE_BUTTON_UP   = ecore_event_type_new();
		ECORE_FB_EVENT_MOUSE_MOVE        = ecore_event_type_new();
		ECORE_FB_EVENT_MOUSE_WHEEL       = ecore_event_type_new();
		_ecore_fb_size_get(&_ecore_fb_console_w, &_ecore_fb_console_h);
	}
	return ++_ecore_fb_init_count;
}

/**
 * Shuts down the Ecore_Fb library. 
 * @return  @c The number of times the system has been initialised without
 *             being shut down.
 * @ingroup Ecore_FB_Library_Group
 */
EAPI int
ecore_fb_shutdown(void)
{    
	_ecore_fb_init_count--;
	if(!_ecore_fb_init_count)
	{
		ecore_fb_vt_shutdown();
		return 0;
	}
	return _ecore_fb_init_count;
}


/**
 * Retrieves the width and height of the current frame buffer in pixels.
 * @param w Pointer to an integer in which to store the width.
 * @param h Pointer to an interge in which to store the height.
 */
EAPI void
ecore_fb_size_get(int *w, int *h)
{
   if (w) *w = _ecore_fb_console_w;
   if (h) *h = _ecore_fb_console_h;
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
