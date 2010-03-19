#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Cocoa/Cocoa.h>

#include "Ecore_Cocoa.h"

Ecore_Cocoa_Window *
ecore_cocoa_window_new(int x,
		       int y,
		       int width,
		       int height)
{
  NSWindow *window;

  window = [[NSWindow alloc]
	     initWithContentRect:NSMakeRect(x, y, width, height)
	     styleMask:(NSTitledWindowMask |
			NSClosableWindowMask |
			NSResizableWindowMask |
			NSMiniaturizableWindowMask)
	     backing:NSBackingStoreBuffered
	     defer:NO
	     screen:nil
	    ];
  if (!window)
    return NULL;

  return window;
}

void
ecore_cocoa_window_free(Ecore_Cocoa_Window *window)
{
  if (!window)
    return;

  [window release];
}

void
ecore_cocoa_window_move(Ecore_Cocoa_Window *window,
			int                 x,
			int                 y)
{
  if (!window)
    return;
}

void
ecore_cocoa_window_resize(Ecore_Cocoa_Window *window,
			  int                 width,
			  int                 height)
{
  if (!window)
    return;

  [window setContentSize: NSMakeSize(width, height)];
}

void
ecore_cocoa_window_move_resize(Ecore_Cocoa_Window *window,
			       int                 x,
			       int                 y,
			       int                 width,
			       int                 height)
{
  if (!window)
    return;
}

void
ecore_cocoa_window_title_set(Ecore_Cocoa_Window *window, const char *title)
{
  if (!window || !title)
    return;

  [window setTitle:[NSString stringWithUTF8String:title]];
}

void
ecore_cocoa_window_show(Ecore_Cocoa_Window *window)
{
  if (!window || [window isVisible])
    return;

  [window orderFront:NSApp];
}

void
ecore_cocoa_window_hide(Ecore_Cocoa_Window *window)
{
  if (!window || ![window isVisible])
    return;

  [window orderOut:NSApp];
}

void
ecore_cocoa_window_borderless_set(Ecore_Cocoa_Window *window,
                                  int                 on)
{
  if (!window)
    return;

  if (on)
    [window setContentBorderThickness:0.0
	    forEdje:NSMinXEdge | NSMinYEdge | NSMaxXEdge | NSMaxYEdge];
}
