#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Cocoa/Cocoa.h>

#include "Ecore_Cocoa.h"
#include "ecore_cocoa_private.h"

Ecore_Cocoa_Window *
ecore_cocoa_window_new(int x,
		       int y,
		       int width,
		       int height)
{
  Ecore_Cocoa_Window *w;

  NSWindow *window = [[NSWindow alloc]
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

  [window setBackgroundColor:[NSColor whiteColor]];

  w = calloc(1, sizeof(Ecore_Cocoa_Window));
  w->window = window;
  w->borderless = 0;

  return w;
}

void
ecore_cocoa_window_free(Ecore_Cocoa_Window *window)
{
  if (!window)
    return;

  [window->window release];
  free(window);
}

void
ecore_cocoa_window_move(Ecore_Cocoa_Window *window,
			int                 x,
			int                 y)
{
  NSRect win_frame;

  if (!window)
    return;

  win_frame = [window->window frame];
  win_frame.origin.x = x;
  win_frame.origin.y = y;

  [window->window setFrame:win_frame display:YES];
}

void
ecore_cocoa_window_resize(Ecore_Cocoa_Window *window,
			  int                 width,
			  int                 height)
{
  if (!window)
    return;

  NSRect win_frame;

  if (!window)
    return;

  win_frame = [window->window frame];
  win_frame.size.height = height;
  win_frame.size.width = width;

  [window->window setFrame:win_frame display:YES];
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

  NSRect win_frame;

  if (!window)
    return;

  win_frame = [window->window frame];
  win_frame.size.height = height;
  win_frame.size.width = width;
  win_frame.origin.x = x;
  win_frame.origin.y = y;

  [window->window setFrame:win_frame display:YES];
}

void
ecore_cocoa_window_title_set(Ecore_Cocoa_Window *window, const char *title)
{
  if (!window || !title)
    return;

  [window->window setTitle:[NSString stringWithUTF8String:title]];
}

void
ecore_cocoa_window_show(Ecore_Cocoa_Window *window)
{
  if (!window || [window->window isVisible])
    {
      printf("Window(%p) is not visible\n", window->window);
      return;
    }

  [window->window makeKeyAndOrderFront:NSApp];
}

void
ecore_cocoa_window_hide(Ecore_Cocoa_Window *window)
{
  if (!window || ![window->window isVisible])
    return;

  [window->window orderOut:NSApp];
}

void
ecore_cocoa_window_borderless_set(Ecore_Cocoa_Window *window,
                                  int                 on)
{
  if (!window)
    return;

  if (on)
    [window->window setContentBorderThickness:0.0
	    forEdje:NSMinXEdge | NSMinYEdge | NSMaxXEdge | NSMaxYEdge];
}

void
ecore_cocoa_window_view_set(Ecore_Cocoa_Window *window,
			    void *view)
{
  if (!window || !view)
    return;

  [[window->window contentView] addSubview:view];

}
