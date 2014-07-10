#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Cocoa.h>
#import "ecore_cocoa_window.h"

@implementation EcoreCocoaWindow

@synthesize ecore_window_data;

- (id) initWithContentRect: (NSRect) contentRect
                 styleMask: (unsigned int) aStyle
                   backing: (NSBackingStoreType) bufferingType
                     defer: (BOOL) flag
{
    if (![super initWithContentRect: contentRect 
                          styleMask: aStyle 
                            backing: bufferingType 
                              defer: flag]) return nil;

    [self setBackgroundColor: [NSColor whiteColor]];
    [self makeKeyWindow];
    [self setDelegate:self];
    [self setAcceptsMouseMovedEvents:YES];

    return self;
}

- (BOOL)acceptsFirstResponder
{
   return YES;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (void)windowWillClose:(NSNotification *)notification
{
   NSLog(@"window is going to be closed");
}

- (void)windowDidResize:(NSNotification *)notif
{
   Ecore_Cocoa_Event_Video_Resize *event;
   NSSize size = self.frame.size;

   event = malloc(sizeof(Ecore_Cocoa_Event_Video_Resize));
   if (event == NULL)
     {
        // FIXME Use Eina_Log
        printf("Failed to allocate Ecore_Cocoa_Event_Video_Resize\n");
        return;
     }
   event->w = size.width;
   event->h = size.height - ecore_cocoa_titlebar_height_get();
   ecore_event_add(ECORE_COCOA_EVENT_RESIZE, event, NULL, NULL);
}

@end


#include "Ecore_Cocoa.h"
#include "ecore_cocoa_private.h"

static float _title_bar_height(void)
{
  NSRect frame = NSMakeRect (0, 0, 100, 100);
  NSRect contentRect;

  contentRect = [NSWindow contentRectForFrameRect: frame
			  styleMask: NSTitledWindowMask];

  return (frame.size.height - contentRect.size.height);
}

Ecore_Cocoa_Window *
ecore_cocoa_window_new(int x,
		       int y,
		       int width,
		       int height)
{
  Ecore_Cocoa_Window *w;

  EcoreCocoaWindow *window = [[EcoreCocoaWindow alloc] 
  								initWithContentRect:NSMakeRect(x, y, width, height)
  									      styleMask:(NSTitledWindowMask |
  									      			 NSClosableWindowMask |
  									      			 NSResizableWindowMask |
  									      			 NSMiniaturizableWindowMask)
  									      	backing:NSBackingStoreBuffered
  									      	  defer:NO];

  if (!window)
    return NULL;

  //Set the process to be a foreground process,
  //without that it prevents the window to become the key window and
  //receive all mouse mouve events.  
  ProcessSerialNumber psn;
  GetCurrentProcess(&psn);
  TransformProcessType(&psn, kProcessTransformToForegroundApplication);
  SetFrontProcess(&psn);

  w = calloc(1, sizeof(Ecore_Cocoa_Window));
  w->window = window;
  w->borderless = 0;

  window.ecore_window_data = w;

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
  if (!window) return;

  NSRect win_frame;


  win_frame = [window->window frame];
  win_frame.size.height = height + _title_bar_height();
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
  win_frame.size.height = height + _title_bar_height();
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
	    forEdge:NSMinXEdge | NSMinYEdge | NSMaxXEdge | NSMaxYEdge];
}

void
ecore_cocoa_window_view_set(Ecore_Cocoa_Window *window,
			    void *view)
{
  if (!window || !view)
    return;

  //[[window->window contentView] addSubview:view];
  [window->window setContentView:view];

  NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[view frame]
                                                      options:NSTrackingMouseMoved | 
                                                              NSTrackingActiveInActiveApp |
                                                              NSTrackingInVisibleRect
                                                        owner:view
                                                     userInfo:nil];
  [view addTrackingArea:area];
  
  [area release];
}
