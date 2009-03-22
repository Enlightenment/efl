#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Cocoa/Cocoa.h>

#include "Ecore_Quartz.h"
#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"
#include "Ecore_Quartz_Keys.h"

EAPI int ECORE_QUARTZ_EVENT_KEY_DOWN = 0;
EAPI int ECORE_QUARTZ_EVENT_KEY_UP = 0;
EAPI int ECORE_QUARTZ_EVENT_MOUSE_BUTTON_DOWN = 0;
EAPI int ECORE_QUARTZ_EVENT_MOUSE_BUTTON_UP = 0;
EAPI int ECORE_QUARTZ_EVENT_MOUSE_MOVE = 0;
EAPI int ECORE_QUARTZ_EVENT_MOUSE_WHEEL = 0;
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
      ECORE_QUARTZ_EVENT_KEY_DOWN          = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_KEY_UP            = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_MOUSE_BUTTON_DOWN = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_MOUSE_BUTTON_UP   = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_MOUSE_MOVE        = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_MOUSE_WHEEL       = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_GOT_FOCUS         = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_LOST_FOCUS        = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_RESIZE            = ecore_event_type_new();
      ECORE_QUARTZ_EVENT_EXPOSE            = ecore_event_type_new();
	}
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
         Ecore_Quartz_Event_Mouse_Move * ev = malloc(sizeof(Ecore_Quartz_Event_Mouse_Move));
         ev->x = [event locationInWindow].x;
         ev->y = [event locationInWindow].y;
         ev->time = time;
         ev->window = [event window];
         
         ecore_event_add(ECORE_QUARTZ_EVENT_MOUSE_MOVE, ev, NULL, NULL);
            
         [NSApp sendEvent:event]; // pass along mouse events, for window manager
         break;
      }
      case NSLeftMouseDown:
      case NSRightMouseDown:
      case NSOtherMouseDown:
      {
         Ecore_Quartz_Event_Mouse_Button_Down * ev = malloc(sizeof(Ecore_Quartz_Event_Mouse_Button_Down));
         ev->x = [event locationInWindow].x;
         ev->y = [event locationInWindow].y;
         ev->button = [event buttonNumber] + 1; // Apple indexes buttons from 0
         
         if ([event clickCount] == 2)
            ev->double_click = 1;
         else
            ev->double_click = 0;
         
         if ([event clickCount] >= 3)
            ev->triple_click = 1;
         else
            ev->triple_click = 0;
         
         ev->time = time;
         
         ecore_event_add(ECORE_QUARTZ_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
         
         [NSApp sendEvent:event]; // pass along mouse events, for window manager
         break;
      }
      case NSLeftMouseUp:
      case NSRightMouseUp:
      case NSOtherMouseUp:
      {
         Ecore_Quartz_Event_Mouse_Button_Up * ev = malloc(sizeof(Ecore_Quartz_Event_Mouse_Button_Up));
         ev->x = [event locationInWindow].x;
         ev->y = [event locationInWindow].y;
         ev->button = [event buttonNumber] + 1; // Apple indexes buttons from 0
         
         if ([event clickCount] == 2)
            ev->double_click = 1;
         else
            ev->double_click = 0;
         
         if ([event clickCount] >= 3)
            ev->triple_click = 1;
         else
            ev->triple_click = 0;
         
         ev->time = time;
         
         ecore_event_add(ECORE_QUARTZ_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
         
         [NSApp sendEvent:event]; // pass along mouse events, for window manager
         break;
      }
      case NSKeyDown:
      {
         Ecore_Quartz_Event_Key_Down   *ev;
         unsigned int               i;

         ev = malloc(sizeof (Ecore_Quartz_Event_Key_Down));
         ev->time = time;
         
         for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_quartz_keys_s); ++i)
         {
            if (keystable[i].code == tolower([[event charactersIgnoringModifiers] characterAtIndex:0]))
            {
               ev->keyname = keystable[i].name;
               ev->keycompose = keystable[i].compose;

               ecore_event_add(ECORE_QUARTZ_EVENT_KEY_DOWN, ev, NULL, NULL);
               return;
            }
         }
         
         free(ev);
         break;
      }
      case NSKeyUp:
      {
         Ecore_Quartz_Event_Key_Up   *ev;
         unsigned int                i;

         ev = malloc(sizeof (Ecore_Quartz_Event_Key_Up));
         ev->time = time;
         
         for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_quartz_keys_s); ++i)
         {
            if (keystable[i].code == tolower([[event charactersIgnoringModifiers] characterAtIndex:0]))
            {
               ev->keyname = keystable[i].name;
               ev->keycompose = keystable[i].compose;

               ecore_event_add(ECORE_QUARTZ_EVENT_KEY_UP, ev, NULL, NULL);
               return;
            }
         }
         
         free(ev);
         break;
      }
      case NSFlagsChanged:
      {
         int flags = [event modifierFlags];
         
         Ecore_Quartz_Event_Key_Down *evDown = NULL;
         Ecore_Quartz_Event_Key_Up   *evUp = NULL;

         evDown = malloc(sizeof (Ecore_Quartz_Event_Key_Down));
         evDown->keyname = NULL;
         
         evUp = malloc(sizeof (Ecore_Quartz_Event_Key_Up));
         evUp->keyname = NULL;
         
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
            evDown->time = time;
            evDown->keycompose = "";
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
            evUp->time = time;
            evUp->keycompose = "";
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
