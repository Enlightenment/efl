#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Cocoa/Cocoa.h>

#include <Eina.h>

#include <Ecore.h>
#include <ecore_private.h>
#include <Ecore_Input.h>

#include "Ecore_Cocoa.h"
#include "Ecore_Cocoa_Keys.h"


EAPI int ECORE_COCOA_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_COCOA_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_COCOA_EVENT_RESIZE = 0;
EAPI int ECORE_COCOA_EVENT_EXPOSE = 0;

static int _ecore_cocoa_init_count = 0;

static int old_flags;

static  unsigned int
_ecore_cocoa_event_modifiers(int mod)
{
   unsigned int modifiers = 0;

   if(mod & NSShiftKeyMask) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & NSControlKeyMask) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & NSAlternateKeyMask) modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if(mod & NSAlphaShiftKeyMask) modifiers |= ECORE_EVENT_LOCK_CAPS;

   return modifiers;
}

static Ecore_Event_Key*
_ecore_cocoa_event_key(NSEvent *nsevent, double timestamp)
{
   Ecore_Event_Key *ev;
   unsigned int i;
   int flags = [nsevent modifierFlags];

   ev = malloc(sizeof(Ecore_Event_Key));
   if (!ev) return NULL;

   ev->timestamp = timestamp;
   ev->window = 0;
   ev->event_window = 0;
   ev->modifiers = _ecore_cocoa_event_modifiers(flags);
   ev->key = NULL;
   ev->compose = NULL;

   printf("key code : %d\n", [[nsevent charactersIgnoringModifiers] characterAtIndex:0]);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(keystable); ++i)
     if (keystable[i].code == [[nsevent charactersIgnoringModifiers] characterAtIndex:0])
       {
          printf("keycode : %d key pressed : %s\n", keystable[i].code, keystable[i].name);
          ev->keyname = keystable[i].name;
          ev->key = ev->keyname;
          ev->string = keystable[i].compose;

          return ev;
       }
   free(ev);
   return NULL;
}



EAPI int
ecore_cocoa_init(void)
{
   if (++_ecore_cocoa_init_count != 1)
     return _ecore_cocoa_init_count;

   if (!ecore_event_init())
     return --_ecore_cocoa_init_count;

   NSApplicationLoad();

   ECORE_COCOA_EVENT_GOT_FOCUS  = ecore_event_type_new();
   ECORE_COCOA_EVENT_LOST_FOCUS = ecore_event_type_new();
   ECORE_COCOA_EVENT_RESIZE     = ecore_event_type_new();
   ECORE_COCOA_EVENT_EXPOSE     = ecore_event_type_new();

   return _ecore_cocoa_init_count;
}

/**
 * Shuts down the Ecore_Cocoa library.
 * @return  @c The number of times the system has been initialised without
 *             being shut down.
 * @ingroup Ecore_Cocoa_Library_Group
 */
EAPI int
ecore_cocoa_shutdown(void)
{
   if (--_ecore_cocoa_init_count != 0)
     return _ecore_cocoa_init_count;

   ecore_event_shutdown();

   return _ecore_cocoa_init_count;
}

EAPI void
ecore_cocoa_feed_events(void)
{
   NSDate *date = [NSDate dateWithTimeIntervalSinceNow:0.001];
   NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                       untilDate:date
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES];
   [date release];
   if (!event) return; // SDL loops until null; maybe we should do that too. or not.

   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);

   switch([event type])
   {
      case NSMouseMoved:
      case NSLeftMouseDragged:
      case NSRightMouseDragged:
      case NSOtherMouseDragged:
      {
         Ecore_Event_Mouse_Move * ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
         if (!ev) return;

         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
         NSView *view = [window contentView];
         NSPoint pt = [event locationInWindow];

         ev->x = pt.x;
         ev->y = [view frame].size.height - pt.y;
         ev->root.x = ev->x;
         ev->root.y = ev->y;
         ev->timestamp = time;
         ev->window = [event window];
         ev->modifiers = 0; /* FIXME: keep modifier around. */

         ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);

         [NSApp sendEvent:event]; // pass along mouse events, for window manager
         break;
      }
      case NSLeftMouseDown:
      case NSRightMouseDown:
      case NSOtherMouseDown:
      {
         Ecore_Event_Mouse_Button * ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
         if (!ev) return;

         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
         NSView *view = [window contentView];
         NSPoint pt = [event locationInWindow];

         ev->x = pt.x;
         ev->y = [view frame].size.height - pt.y;
         ev->root.x = ev->x;
         ev->root.y = ev->y;
         ev->timestamp = time;

         if ([event buttonNumber] == 0)
           ev->buttons = 1;
         else if ([event buttonNumber] == 2)
           ev->buttons = 2;
         else
           ev->buttons = 3;

         printf("ev buttons : %d - %d\n", ev->buttons, [event buttonNumber]);
         ev->window = window.ecore_window_data;
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

         [NSApp sendEvent:event]; // pass along mouse events, for window manager
         break;
      }
      case NSLeftMouseUp:
      case NSRightMouseUp:
      case NSOtherMouseUp:
      {
         Ecore_Event_Mouse_Button * ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
         if (!ev) return;

         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
         NSView *view = [window contentView];
         NSPoint pt = [event locationInWindow];

         ev->x = pt.x;
         ev->y = [view frame].size.height - pt.y;
         ev->root.x = ev->x;
         ev->root.y = ev->y;
         ev->timestamp = time;

         if ([event buttonNumber] == 0)
           ev->buttons = 1;
         else if ([event buttonNumber] == 2)
           ev->buttons = 2;
         else
           ev->buttons = 3;

         ev->window = window.ecore_window_data;
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

         [NSApp sendEvent:event]; // pass along mouse events, for window manager
         break;
      }
      case NSKeyDown:
      {
         Ecore_Event_Key *ev;
         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];

         ev = _ecore_cocoa_event_key(event, time);
         if (!ev) return;

         ev->window = window.ecore_window_data;
         ev->event_window = ev->window;
         ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);

         break;
      }
      case NSKeyUp:
      {
         Ecore_Event_Key *ev;
         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];

         ev = _ecore_cocoa_event_key(event, time);
         if (!ev) return;

         ev->window = window.ecore_window_data;
         ev->event_window = ev->window;
         ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);

         break;
      }
      case NSFlagsChanged:
      {

         int flags = [event modifierFlags];
         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
         Ecore_Event_Key *ev;

         ev = calloc(1, sizeof(Ecore_Event_Key));
         if (!ev) return;

         if (flags & NSShiftKeyMask)
            ev->keyname = "Shift_L";
         else if (flags & NSControlKeyMask)
            ev->keyname = "Control_L";
         else if (flags & NSAlternateKeyMask)
            ev->keyname = "Alt_L";
         else if (flags & NSCommandKeyMask)
            ev->keyname = "Super_L";
         else if (flags & NSAlphaShiftKeyMask)
            ev->keyname = "Caps_Lock";

         if (ev->keyname)
         {
            ev->key = ev->keyname;
            ev->timestamp = time;
            ev->string = "";
            ev->modifiers = _ecore_cocoa_event_modifiers(flags);
            printf("Flags changed keyup\n");
            ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
            old_flags = flags;
            break;
         }

         break;
      }
      case NSAppKitDefined:
      {
         if ([event subtype] == NSApplicationActivatedEventType)
            ecore_event_add(ECORE_COCOA_EVENT_GOT_FOCUS, NULL, NULL, NULL);
         else if ([event subtype] == NSApplicationDeactivatedEventType)
            ecore_event_add(ECORE_COCOA_EVENT_LOST_FOCUS, NULL, NULL, NULL);
         [NSApp sendEvent:event]; // pass along AppKit events, for window manager
         break;
      }
      case NSScrollWheel:
      {
         break;
      }
      default:
      {
         [NSApp sendEvent:event];
         break;
      }
   }

   [event release];
}
