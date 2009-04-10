#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Cocoa/Cocoa.h>

#include <Eina.h>

#include <ecore_private.h>
#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_Quartz.h"
#include "Ecore_Quartz_Keys.h"

EAPI int ECORE_QUARTZ_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_QUARTZ_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_QUARTZ_EVENT_RESIZE = 0;
EAPI int ECORE_QUARTZ_EVENT_EXPOSE = 0;

static int _ecore_quartz_init_count = 0;

static int old_flags;

EAPI int
ecore_quartz_init(const char *name __UNUSED__)
{
   if (!_ecore_quartz_init_count)
     {
        ECORE_QUARTZ_EVENT_GOT_FOCUS         = ecore_event_type_new();
        ECORE_QUARTZ_EVENT_LOST_FOCUS        = ecore_event_type_new();
        ECORE_QUARTZ_EVENT_RESIZE            = ecore_event_type_new();
        ECORE_QUARTZ_EVENT_EXPOSE            = ecore_event_type_new();
     }

   ecore_event_init();

   return ++_ecore_quartz_init_count;
}

/**
 * Shuts down the Ecore_Quartz library.
 * @return  @c The number of times the system has been initialised without
 *             being shut down.
 * @ingroup Ecore_Quartz_Library_Group
 */
EAPI int
ecore_quartz_shutdown(void)
{
   _ecore_quartz_init_count--;

   ecore_event_shutdown();

   return _ecore_quartz_init_count;
}

EAPI void
ecore_quartz_feed_events(void)
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
         ev->x = [event locationInWindow].x;
         ev->y = [event locationInWindow].y;
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
         ev->x = [event locationInWindow].x;
         ev->y = [event locationInWindow].y;
         ev->root.x = ev->x;
         ev->root.y = ev->y;
         ev->timestamp = time;
         ev->buttons = [event buttonNumber] + 1; // Apple indexes buttons from 0

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
         ev->x = [event locationInWindow].x;
         ev->y = [event locationInWindow].y;
         ev->root.x = ev->x;
         ev->root.y = ev->y;
         ev->timestamp = time;
         ev->buttons = [event buttonNumber] + 1; // Apple indexes buttons from 0

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
         unsigned int     i;

         ev = calloc(1, sizeof (Ecore_Event_Key));
         if (!ev) return;
         ev->timestamp = time;

         for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_quartz_keys_s); ++i)
         {
            if (keystable[i].code == tolower([[event charactersIgnoringModifiers] characterAtIndex:0]))
            {
               ev->keyname = keystable[i].name;
               ev->string = keystable[i].compose;

               ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);
               return;
            }
         }

         break;
      }
      case NSKeyUp:
      {
         Ecore_Event_Key *ev;
         unsigned int     i;

         ev = calloc(1, sizeof (Ecore_Quartz_Event_Key_Up));
         if (!ev) return;
         ev->timestamp = time;

         for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_quartz_keys_s); ++i)
         {
            if (keystable[i].code == tolower([[event charactersIgnoringModifiers] characterAtIndex:0]))
            {
               ev->keyname = keystable[i].name;
               ev->string = keystable[i].compose;

               ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
               return;
            }
         }

         break;
      }
      case NSFlagsChanged:
      {
         int flags = [event modifierFlags];

         Ecore_Event_Key *evDown = NULL;
         Ecore_Event_Key *evUp = NULL;

         evDown = calloc(1, sizeof (Ecore_Quartz_Event_Key_Down));
         if (!evDown) return;

         evUp = calloc(1, sizeof (Ecore_Quartz_Event_Key_Up));
         if (!evUp)
           {
              free(evDown);
              return;
           }

         // Turn special key flags on
         if (flags & NSShiftKeyMask)
            evDown->keyname = "Shift_L";
         else if (flags & NSControlKeyMask)
            evDown->keyname = "Control_L";
         else if (flags & NSAlternateKeyMask)
            evDown->keyname = "Alt_L";
         else if (flags & NSCommandKeyMask)
            evDown->keyname = "Super_L";
         else if (flags & NSAlphaShiftKeyMask)
            evDown->keyname = "Caps_Lock";

         if (evDown->keyname)
         {
            evDown->timestamp = time;
            evDown->string = "";
            ecore_event_add(ECORE_QUARTZ_EVENT_KEY_DOWN, evDown, NULL, NULL);
            old_flags = flags;
            break;
         }

         int changed_flags = flags ^ old_flags;

         // Turn special key flags off
         if (changed_flags & NSShiftKeyMask)
            evUp->keyname = "Shift_L";
         else if (changed_flags & NSControlKeyMask)
            evUp->keyname = "Control_L";
         else if (changed_flags & NSAlternateKeyMask)
            evUp->keyname = "Alt_L";
         else if (changed_flags & NSCommandKeyMask)
            evUp->keyname = "Super_L";
         else if (changed_flags & NSAlphaShiftKeyMask)
            evUp->keyname = "Caps_Lock";

         if (evUp->keyname)
         {
            evUp->timestamp = time;
            evUp->string = "";
            ecore_event_add(ECORE_QUARTZ_EVENT_KEY_UP, evUp, NULL, NULL);
            old_flags = flags;
            break;
         }

         break;
      }
      case NSAppKitDefined:
      {
         if ([event subtype] == NSApplicationActivatedEventType)
            ecore_event_add(ECORE_QUARTZ_EVENT_GOT_FOCUS, NULL, NULL, NULL);
         else if ([event subtype] == NSApplicationDeactivatedEventType)
            ecore_event_add(ECORE_QUARTZ_EVENT_LOST_FOCUS, NULL, NULL, NULL);
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
