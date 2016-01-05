#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore_Input.h>

#include <Ecore.h>
#include <Ecore_Cocoa.h>
#include <Ecore_Cocoa_Cursor.h>
#import "ecore_cocoa_window.h"
#import "ecore_cocoa_app.h"
#include "ecore_cocoa_private.h"

static NSCursor *_cursors[__ECORE_COCOA_CURSOR_LAST];


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

- (void)windowWillClose:(NSNotification *) notification
{
   NSLog(@"window is going to be closed");
   Ecore_Cocoa_Event_Window *event;

   event = malloc(sizeof(*event));
   if (EINA_UNLIKELY(event == NULL))
     {
        CRI("Failed to allocate Ecore_Cocoa_Event_Window");
        return;
     }
   event->wid = [notification object];
   ecore_event_add(ECORE_COCOA_EVENT_WINDOW_DESTROY, event, NULL, NULL);
}

- (void)windowDidResize:(NSNotification *) notif
{
   Ecore_Cocoa_Event_Video_Resize *event;
   NSSize size = self.frame.size;

   event = malloc(sizeof(*event));
   if (EINA_UNLIKELY(event == NULL))
     {
        CRI("Failed to allocate Ecore_Cocoa_Event_Video_Resize");
        return;
     }
   event->w = size.width;
   event->h = size.height -
      (([self isFullScreen] == YES) ? 0 : ecore_cocoa_titlebar_height_get());
   event->wid = [notif object];
   ecore_event_add(ECORE_COCOA_EVENT_RESIZE, event, NULL, NULL);

   /*
    * During live resize, NSRunLoop blocks, and prevent the ecore_main_loop
    * to be run.
    * This, combined with the -pauseNSRunLoopMonitoring and
    * -resumeNSRunLoopMonitoring methods invoked in
    * -windowWillStartLiveResize and -windowDidEndLiveResize
    * allow the ecore_main_loop to run withing NSRunLoop during the
    * live resizing of a window.
    */
   ecore_main_loop_iterate();
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
   Ecore_Cocoa_Event_Window *e;

   e = malloc(sizeof(*e));
   if (EINA_UNLIKELY(e == NULL))
     {
        CRI("Failed to allocate Ecore_Cocoa_Event_Window");
        return;
     }
   e->wid = [notification object];
   ecore_event_add(ECORE_COCOA_EVENT_GOT_FOCUS, e, NULL, NULL);
}

- (void) windowWillStartLiveResize:(NSNotification *) EINA_UNUSED notification
{
   [NSApp pauseNSRunLoopMonitoring];
}

- (void) windowDidEndLiveResize:(NSNotification *) EINA_UNUSED notification
{
   [NSApp resumeNSRunLoopMonitoring];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
   Ecore_Cocoa_Event_Window *e;

   e = malloc(sizeof(*e));
   if (EINA_UNLIKELY(e == NULL))
     {
        CRI("Failed to allocate Ecore_Cocoa_Event_Window");
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

   //we ignore left click in titlebar as it is handled by the OS (to move window)
   //and the corresponding mouseUp event isn't sent
   if (y <= 0 && [event buttonNumber] == 0) {
	 return;
   }

   Ecore_Event_Mouse_Button * ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));

   if (EINA_UNLIKELY(!ev))
     {
        CRI("Failed to allocate Ecore_Event_Mouse_Button");
        return;
     }

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

   Ecore_Event_Mouse_Button *ev = calloc(1, sizeof(*ev));
   if (EINA_UNLIKELY(!ev))
     {
        CRI("Failed to allocate Ecore_Event_Mouse_Button");
        return;
     }

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
   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
   Ecore_Event_Mouse_Move *ev = calloc(1, sizeof(*ev));
   if (EINA_UNLIKELY(!ev))
     {
        CRI("Failed to allocate Ecore_Event_Mouse_Move");
        return;
     }

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

EAPI Ecore_Cocoa_Window *
ecore_cocoa_window_new(int x,
                       int y,
                       int width,
                       int height)
{
   Ecore_Cocoa_Window *w;
   EcoreCocoaWindow *window;
   NSRect frame = NSMakeRect(x, y, width, height);
   NSUInteger style =
      NSTitledWindowMask        |
      NSClosableWindowMask      |
      NSResizableWindowMask     |
      NSMiniaturizableWindowMask;

   window = [[EcoreCocoaWindow alloc] initWithContentRect:frame
                                                styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
   if (EINA_UNLIKELY(!window))
     {
        CRI("Failed to create EcoreCocoaWindow");
        return NULL;
     }

   w = calloc(1, sizeof(Ecore_Cocoa_Window));
   if (EINA_UNLIKELY(w == NULL))
     {
        CRI("Failed to allocate Ecore_Cocoa_Window");
        [window release];
        return NULL;
     }
   w->window = window;
   w->borderless = 0;

   window.ecore_window_data = w;

   return w;
}

EAPI void
ecore_cocoa_window_free(Ecore_Cocoa_Window *window)
{
   if (!window)
     return;

   [window->window release];
   free(window);
}

EAPI void
ecore_cocoa_window_size_min_set(Ecore_Cocoa_Window *window,
                                unsigned int w,
                                unsigned int h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   window->window.contentMinSize = NSMakeSize(w, h);
}

EAPI void
ecore_cocoa_window_size_min_get(const Ecore_Cocoa_Window *window,
                                unsigned int             *min_width,
                                unsigned int             *min_height)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   const NSSize size = window->window.contentMinSize;
   if (min_width) *min_width = size.width;
   if (min_height) *min_height = size.height;
}

EAPI void
ecore_cocoa_window_size_max_set(Ecore_Cocoa_Window *window,
                                unsigned int w,
                                unsigned int h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   window->window.contentMaxSize = NSMakeSize(w, h);
}

EAPI void
ecore_cocoa_window_size_max_get(const Ecore_Cocoa_Window *window,
                                unsigned int             *max_width,
                                unsigned int             *max_height)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   const NSSize size = window->window.contentMaxSize;
   if (max_width) *max_width = size.width;
   if (max_height) *max_height = size.height;
}

EAPI void
ecore_cocoa_window_size_step_set(Ecore_Cocoa_Window *window,
                                 unsigned int w,
                                 unsigned int h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   window->window.contentResizeIncrements = NSMakeSize(w, h);
}

EAPI void
ecore_cocoa_window_size_step_get(const Ecore_Cocoa_Window *window,
                                 unsigned int             *step_width,
                                 unsigned int             *step_height)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   const NSSize size = window->window.contentResizeIncrements;
   if (step_width) *step_width = size.width;
   if (step_height) *step_height = size.height;
}

EAPI void
ecore_cocoa_window_move(Ecore_Cocoa_Window *window,
                        int                 x,
                        int                 y)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   NSRect win_frame;

   win_frame = [window->window frame];
   win_frame.origin.x = x;
   win_frame.origin.y = y;

   [window->window setFrame:win_frame display:YES];
}

EAPI void
ecore_cocoa_window_resize(Ecore_Cocoa_Window *window,
                          int                 width,
                          int                 height)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   NSRect win_frame;

   win_frame = [window->window frame];
   win_frame.size.height = height +
      (([window->window isFullScreen] == YES) ? 0 : ecore_cocoa_titlebar_height_get());
   win_frame.size.width = width;

   [window->window setFrame:win_frame display:YES];
}

EAPI void
ecore_cocoa_window_geometry_get(const Ecore_Cocoa_Window *window,
                                int                      *x,
                                int                      *y,
                                int                      *w,
                                int                      *h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   const NSRect frame = window->window.frame;
   if (x) *x = frame.origin.x;
   if (y) *y = frame.origin.y;
   if (w) *w = frame.size.width;
   if (h) *h = frame.size.height;
}

EAPI void
ecore_cocoa_window_size_get(const Ecore_Cocoa_Window *window,
                            int                      *w,
                            int                      *h)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   const NSSize size = window->window.frame.size;
   if (w) *w = size.width;
   if (h) *h = size.height;
}

EAPI void
ecore_cocoa_window_move_resize(Ecore_Cocoa_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   NSRect win_frame;

   win_frame = [window->window frame];
   win_frame.size.height = height +
      (([window->window isFullScreen] == YES) ? 0 : ecore_cocoa_titlebar_height_get());
   win_frame.size.width = width;
   win_frame.origin.x = x;
   win_frame.origin.y = y;

   [window->window setFrame:win_frame display:YES];
}

EAPI void
ecore_cocoa_window_title_set(Ecore_Cocoa_Window *window, const char *title)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(title);

   [window->window setTitle:[NSString stringWithUTF8String:title]];
}

EAPI void
ecore_cocoa_window_show(Ecore_Cocoa_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (![window->window isVisible])
     [window->window makeKeyAndOrderFront:NSApp];
}

EAPI void
ecore_cocoa_window_hide(Ecore_Cocoa_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (![window->window isVisible])
     [window->window orderOut:NSApp];
}

EAPI void
ecore_cocoa_window_raise(Ecore_Cocoa_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   [window->window orderFront:nil];
}

EAPI void
ecore_cocoa_window_lower(Ecore_Cocoa_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   [window->window orderBack:nil];
}

EAPI void
ecore_cocoa_window_activate(Ecore_Cocoa_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   [window->window makeKeyAndOrderFront:nil];
}

EAPI void
ecore_cocoa_window_iconified_set(Ecore_Cocoa_Window *window,
                                 int                 on)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (on)
     {
        [window->window miniaturize:nil];
     }
   else
     {
        [window->window deminiaturize:nil];
     }
}

EAPI void
ecore_cocoa_window_borderless_set(Ecore_Cocoa_Window *window,
                                  int                 on)
{
   EINA_SAFETY_ON_NULL_RETURN(window);

   if (on)
     [window->window setContentBorderThickness:0.0
                                       forEdge:NSMinXEdge | NSMinYEdge | NSMaxXEdge | NSMaxYEdge];
}

EAPI void
ecore_cocoa_window_view_set(Ecore_Cocoa_Window *window,
                            void *view)
{
   EINA_SAFETY_ON_NULL_RETURN(window);
   EINA_SAFETY_ON_NULL_RETURN(view);

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

EAPI Ecore_Cocoa_Window_Id
ecore_cocoa_window_get_window_id(const Ecore_Cocoa_Window *window)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);
   return window->window;
}

EAPI void
ecore_cocoa_window_cursor_set(Ecore_Cocoa_Window *win,
                              Ecore_Cocoa_Cursor  c)
{
   EINA_SAFETY_ON_NULL_RETURN(win);
   EINA_SAFETY_ON_FALSE_RETURN((c >= 0) && (c <= __ECORE_COCOA_CURSOR_LAST));

   NSCursor *cursor = _cursors[c];

   DBG("Setting cursor %i (%s)", c, [[cursor description] UTF8String]);
   [cursor set];
}

EAPI void
ecore_cocoa_window_cursor_show(Ecore_Cocoa_Window *win,
                               Eina_Bool           show)
{
   EINA_SAFETY_ON_NULL_RETURN(win);

   if (show) [NSCursor unhide];
   else [NSCursor hide];
}

Eina_Bool
_ecore_cocoa_window_init(void)
{
   _cursors[ECORE_COCOA_CURSOR_ARROW]                 = [NSCursor arrowCursor];
   _cursors[ECORE_COCOA_CURSOR_CONTEXTUAL_MENU]       = [NSCursor contextualMenuCursor];
   _cursors[ECORE_COCOA_CURSOR_CLOSED_HAND]           = [NSCursor closedHandCursor];
   _cursors[ECORE_COCOA_CURSOR_CROSSHAIR]             = [NSCursor crosshairCursor];
   _cursors[ECORE_COCOA_CURSOR_DISAPPEARING_ITEM]     = [NSCursor disappearingItemCursor];
   _cursors[ECORE_COCOA_CURSOR_DRAG_COPY]             = [NSCursor dragCopyCursor];
   _cursors[ECORE_COCOA_CURSOR_DRAG_LINK]             = [NSCursor dragLinkCursor];
   _cursors[ECORE_COCOA_CURSOR_IBEAM]                 = [NSCursor IBeamCursor];
   _cursors[ECORE_COCOA_CURSOR_OPEN_HAND]             = [NSCursor openHandCursor];
   _cursors[ECORE_COCOA_CURSOR_OPERATION_NOT_ALLOWED] = [NSCursor operationNotAllowedCursor];
   _cursors[ECORE_COCOA_CURSOR_POINTING_HAND]         = [NSCursor pointingHandCursor];
   _cursors[ECORE_COCOA_CURSOR_RESIZE_DOWN]           = [NSCursor resizeDownCursor];
   _cursors[ECORE_COCOA_CURSOR_RESIZE_LEFT]           = [NSCursor resizeLeftCursor];
   _cursors[ECORE_COCOA_CURSOR_RESIZE_LEFT_RIGHT]     = [NSCursor resizeLeftRightCursor];
   _cursors[ECORE_COCOA_CURSOR_RESIZE_RIGHT]          = [NSCursor resizeRightCursor];
   _cursors[ECORE_COCOA_CURSOR_RESIZE_UP]             = [NSCursor resizeUpCursor];
   _cursors[ECORE_COCOA_CURSOR_RESIZE_UP_DOWN]        = [NSCursor resizeUpDownCursor];
   _cursors[ECORE_COCOA_CURSOR_IBEAM_VERTICAL]        = [NSCursor IBeamCursorForVerticalLayout];

   return EINA_TRUE;
}

