#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Cocoa.h>
#include "ecore_cocoa_private.h"
#import "ecore_cocoa_window.h"
#include <Ecore_Input.h>

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

    [self setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

    return self;
}

- (BOOL)isFullScreen
{
   return (([self styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask);
}

- (BOOL)acceptsFirstResponder
{
   return YES;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (void)windowWillClose:(NSNotification *) EINA_UNUSED notification
{
   NSLog(@"window is going to be closed");
   Ecore_Cocoa_Event_Window *event;

   event = malloc(sizeof(Ecore_Cocoa_Event_Window));
   if (event == NULL)
     {
	   // FIXME Use Eina_Log
	   printf("Failed to allocate Ecore_Cocoa_Event_Window_destroy\n");
	   return;
     }
   event->wid = [notification object];
   ecore_event_add(ECORE_COCOA_EVENT_WINDOW_DESTROY, event, NULL, NULL);
}

- (void)windowDidResize:(NSNotification *) EINA_UNUSED notif
{
   Ecore_Cocoa_Event_Video_Resize *event;
   NSSize size = self.frame.size;

   event = malloc(sizeof(Ecore_Cocoa_Event_Video_Resize));
   if (event == NULL)
     {
        DBG("Failed to allocate Ecore_Cocoa_Event_Video_Resize\n");
        return;
     }
   event->w = size.width;
   event->h = size.height -
      (([self isFullScreen] == YES) ? 0 : ecore_cocoa_titlebar_height_get());
   event->wid = [notif object];
   ecore_event_add(ECORE_COCOA_EVENT_RESIZE, event, NULL, NULL);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
  Ecore_Cocoa_Event_Window *e;

  e = malloc(sizeof(Ecore_Cocoa_Event_Window));
  if (!e)
    {
      DBG("GOT_FOCUS: Failed to allocate Ecore_Cocoa_Event_Window\n");
      return;
    }
  e->wid = [notification object];
  ecore_event_add(ECORE_COCOA_EVENT_GOT_FOCUS, e, NULL, NULL);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
  Ecore_Cocoa_Event_Window *e;

  e = malloc(sizeof(Ecore_Cocoa_Event_Window));
  if (!e)
    {
      DBG("LOST_FOCUS: Failed to allocate Ecore_Cocoa_Event_Window\n");
      return;
    }
  e->wid = [notification object];
  ecore_event_add(ECORE_COCOA_EVENT_LOST_FOCUS, e, NULL, NULL);
}

- (void) mouseDown:(NSEvent*) event
{
   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);

   NSView *view = [self contentView];
   NSPoint event_location = [event locationInWindow];
   NSPoint pt = [view convertPoint:event_location fromView:nil];

   int h = [view frame].size.height;
   int x = pt.x;
   int y = h - pt.y;

   Ecore_Event_Mouse_Button * ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   ev->x = x;
   ev->y = y;
   ev->root.x = ev->x;
   ev->root.y = ev->y;
   ev->timestamp = time;
   switch ([event buttonNumber])
     {
      case 0: ev->buttons = 1; break;
      case 1: ev->buttons = 3; break;
      case 2: ev->buttons = 2; break;
      default: ev->buttons = 0; break;
     }
   ev->window = (Ecore_Window)self.ecore_window_data;
   ev->event_window = ev->window;

   if ([event clickCount] == 2)
     ev->double_click = 1;
   else
     ev->double_click = 0;

   if ([event clickCount] >= 3)
     ev->triple_click = 1;
   else
     ev->triple_click = 0;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
}

- (void) rightMouseDown:(NSEvent*) event
{
	[self mouseDown: event];
}

- (void) otherMouseDown:(NSEvent*) event
{
	[self mouseDown: event];
}

- (void) mouseUp:(NSEvent*) event
{
   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);

   NSView *view = [self contentView];
   NSPoint event_location = [event locationInWindow];
   NSPoint pt = [view convertPoint:event_location fromView:nil];

   int h = [view frame].size.height;
   int x = pt.x;
   int y = h - pt.y;

   Ecore_Event_Mouse_Button * ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   ev->x = x;
   ev->y = y;
   ev->root.x = ev->x;
   ev->root.y = ev->y;
   ev->timestamp = time;
   switch ([event buttonNumber])
     {
      case 0: ev->buttons = 1; break;
      case 1: ev->buttons = 3; break;
      case 2: ev->buttons = 2; break;
      default: ev->buttons = 0; break;
     }
   ev->window = (Ecore_Window)self.ecore_window_data;
   ev->event_window = ev->window;

   if ([event clickCount] == 2)
     ev->double_click = 1;
   else
     ev->double_click = 0;

   if ([event clickCount] >= 3)
     ev->triple_click = 1;
   else
     ev->triple_click = 0;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

- (void) rightMouseUp:(NSEvent*) event
{
	[self mouseUp: event];
}

- (void) otherMouseUp:(NSEvent*) event
{
	[self mouseUp: event];
}

- (void) mouseMoved:(NSEvent*) event
{
   Ecore_Event_Mouse_Move * ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   NSView *view = [self contentView];
   NSPoint event_location = [event locationInWindow];
   NSPoint pt = [view convertPoint:event_location fromView:nil];

   ev->x = pt.x;
   ev->y = [view frame].size.height - pt.y;
   ev->root.x = ev->x;
   ev->root.y = ev->y;
   ev->timestamp = time;
   ev->window = (Ecore_Window)self.ecore_window_data;
   ev->event_window = ev->window;
   ev->modifiers = 0; /* FIXME: keep modifier around. */

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

- (void) mouseDragged: (NSEvent*) event
{
   [self mouseMoved:event];
}

@end

Ecore_Cocoa_Window *
ecore_cocoa_window_new(int x,
		       int y,
		       int width,
		       int height)
{
  Ecore_Cocoa_Window *w;

  EcoreCocoaWindow *window = [[EcoreCocoaWindow alloc] initWithContentRect:NSMakeRect(x, y, width, height)
                                                                 styleMask:(NSTitledWindowMask    |
                                                                            NSClosableWindowMask  |
                                                                            NSResizableWindowMask |
                                                                            NSMiniaturizableWindowMask)
                                                                    backing:NSBackingStoreBuffered
                                                                    defer:NO];

  if (!window)
    return NULL;

  //Set the process to be a foreground process,
  //without that it prevents the window to become the key window and
  //receive all mouse mouve events.
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp activateIgnoringOtherApps:YES];


  w = calloc(1, sizeof(Ecore_Cocoa_Window));
  if (w == NULL)
    {
       return NULL;
    }
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
ecore_cocoa_window_size_min_set(Ecore_Cocoa_Window *window,
                                unsigned int w,
                                unsigned int h)
{
  if (!window)
    return;
  NSSize size = {w,h};
  window->window.contentMinSize = size;
}

void
ecore_cocoa_window_size_max_set(Ecore_Cocoa_Window *window,
                                unsigned int w,
                                unsigned int h)
{
  if (!window)
    return;
  NSSize size = {w,h};
  window->window.contentMaxSize = size;
}

void
ecore_cocoa_window_size_step_set(Ecore_Cocoa_Window *window,
                                 unsigned int w,
                                 unsigned int h)
{
  if (!window)
    return;
  NSSize size = {w,h};
  window->window.contentResizeIncrements = size;
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
  win_frame.size.height = height +
     (([window->window isFullScreen] == YES) ? 0 : ecore_cocoa_titlebar_height_get());
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
  if (!window) return;

  NSRect win_frame;

  win_frame = [window->window frame];
  win_frame.size.height = height +
     (([window->window isFullScreen] == YES) ? 0 : ecore_cocoa_titlebar_height_get());
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
      DBG("Window(%p) is not visible\n", window->window);
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
  NSView *v = view;
  [window->window setContentView:view];

  NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[v frame]
                                                      options:NSTrackingMouseMoved |
                                                              NSTrackingActiveInActiveApp |
                                                              NSTrackingInVisibleRect
                                                        owner:v
                                                     userInfo:nil];
  [v addTrackingArea:area];

  [area release];
}

Ecore_Cocoa_Window_Id ecore_cocoa_window_get_window_id(Ecore_Cocoa_Window *window)
{
  if (!window)
    return 0;

  DBG("Return : %p", window->window);

  return window->window;
}
