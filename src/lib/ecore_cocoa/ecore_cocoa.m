#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#import <Cocoa/Cocoa.h>
#import "ecore_cocoa_window.h"
#import "ecore_cocoa_app.h"

#include <Eina.h>

#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_Cocoa.h"

#define _ECORE_COCOA_KEYS_MANUAL_GUARD_
#include "ecore_cocoa_keys.h"
#undef _ECORE_COCOA_KEYS_MANUAL_GUARD_

#include "ecore_cocoa_private.h"

EAPI int ECORE_COCOA_EVENT_WINDOW_UNFOCUSED = 0;
EAPI int ECORE_COCOA_EVENT_WINDOW_FOCUSED = 0;
EAPI int ECORE_COCOA_EVENT_WINDOW_RESIZE_REQUEST = 0;
EAPI int ECORE_COCOA_EVENT_WINDOW_DESTROY = 0;

static int _ecore_cocoa_init_count = 0;

static int old_flags;

int _ecore_cocoa_log_domain = -1;

EAPI int
ecore_cocoa_init(void)
{
   if (++_ecore_cocoa_init_count != 1)
     return _ecore_cocoa_init_count;

   if (!ecore_init())
     return --_ecore_cocoa_init_count;

   if (!ecore_event_init())
     return --_ecore_cocoa_init_count;

   _ecore_cocoa_log_domain = eina_log_domain_register("ecore_cocoa", EINA_COLOR_BLUE);
   if(_ecore_cocoa_log_domain < 0)
     {
        EINA_LOG_ERR("Unable to create a log domain for ecore_cocoa.");
        return 0;
     }

   DBG("");

   ECORE_COCOA_EVENT_WINDOW_UNFOCUSED = ecore_event_type_new();
   ECORE_COCOA_EVENT_WINDOW_FOCUSED = ecore_event_type_new();
   ECORE_COCOA_EVENT_WINDOW_RESIZE_REQUEST = ecore_event_type_new();
   ECORE_COCOA_EVENT_WINDOW_DESTROY = ecore_event_type_new();


   /* Init the Application handler */
   [Ecore_Cocoa_Application sharedApplication];
   [NSApp setDelegate:[Ecore_Cocoa_AppDelegate appDelegate]];

   /* Start events monitoring */
   [NSApp run];

   if (!_ecore_cocoa_window_init())
     return --_ecore_cocoa_init_count;

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

   DBG("Ecore Cocoa shutdown");

   eina_log_domain_unregister(_ecore_cocoa_log_domain);
   ecore_event_shutdown();

   return _ecore_cocoa_init_count;
}

static unsigned int
_ecore_cocoa_event_modifiers(NSUInteger mod)
{
   unsigned int modifiers = 0;

   if(mod & NSShiftKeyMask) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & NSControlKeyMask) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & NSAlternateKeyMask) modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
   if(mod & NSCommandKeyMask) modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if(mod & NSNumericPadKeyMask) modifiers |= ECORE_EVENT_LOCK_NUM;

   DBG("key modifiers: 0x%lx, %u", mod, modifiers);
   return modifiers;
}


static Ecore_Event_Key*
_ecore_cocoa_event_key(NSEvent     *event,
                       NSEventType  keyType,
                       unsigned int time)
{
   static Eina_Bool compose = EINA_FALSE;
   static NSText *edit;
   unsigned int i;

   Ecore_Event_Key *ev;

   EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
   NSString *keychar = [event charactersIgnoringModifiers];
   NSString *keycharRaw = [event characters];

   DBG("Event Key, keyType : %lu", keyType);

   ev = calloc(1, sizeof (Ecore_Event_Key));
   if (!ev) return NULL;

   if (compose && keyType == NSKeyDown)
     {
        [edit interpretKeyEvents:[NSArray arrayWithObject:event]];
        compose=EINA_FALSE;
     }

   ev->timestamp = time;
   ev->modifiers = _ecore_cocoa_event_modifiers([event modifierFlags]);

   ev->keycode = event.keyCode;
   ev->string = [keycharRaw cStringUsingEncoding:NSUTF8StringEncoding];
   ev->compose = ev->string;

   ev->window = (Ecore_Window)window.ecore_window_data;
   ev->event_window = ev->window;

   if ([keychar length] > 0)
     {
        for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_cocoa_keys_s); ++i)
          {
             if (keystable[i].code == [keychar characterAtIndex:0])
               {
                  ev->keyname = keystable[i].name;
                  ev->key = ev->keyname;
                  break;
               }
          }
        if (ev->keyname == NULL)
          {
             ev->keyname = "";
             ev->key = "";
          }
     }

   if ([keycharRaw length] == 0  && keyType == NSKeyDown)
     {
        compose=EINA_TRUE;
        edit = [[event window]  fieldEditor:YES forObject:nil];
        [edit interpretKeyEvents:[NSArray arrayWithObject:event]];
        free(ev);
        return NULL;
     }

   return ev;
}

Eina_Bool
_ecore_cocoa_feed_events(void *anEvent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(anEvent, EINA_FALSE);

   NSEvent *event = anEvent;
   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
   Eina_Bool pass = EINA_FALSE;

   DBG("Feed events, event type ; %lu", [event type]);

   switch ([event type])
     {
      case NSMouseMoved:
      case NSLeftMouseDragged:
      case NSRightMouseDragged:
      case NSOtherMouseDragged:
      case NSLeftMouseDown:
      case NSRightMouseDown:
      case NSOtherMouseDown:
      case NSLeftMouseUp:
      case NSRightMouseUp:
      case NSOtherMouseUp:
        {
           //mouse events are managed in EcoreCocoaWindow
           return EINA_TRUE;
        }
      case NSKeyDown:
        {
           Ecore_Event_Key *ev;

           ev = _ecore_cocoa_event_key(event, NSKeyDown, time);
           if (ev == NULL) return EINA_TRUE;

           ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);

           break;
        }
      case NSKeyUp:
        {
           Ecore_Event_Key *ev;

           ev = _ecore_cocoa_event_key(event, NSKeyUp, time);
           if (ev == NULL) return EINA_TRUE;

           ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);

           break;
        }
      case NSFlagsChanged:
        {
           NSUInteger flags = [event modifierFlags];

           Ecore_Event_Key *evDown = NULL;
           Ecore_Event_Key *evUp = NULL;

           evDown = calloc(1, sizeof (Ecore_Event_Key));
           if (!evDown) return pass;

           // Turn special key flags on
           if (flags & NSShiftKeyMask)
             evDown->key = "Shift_L";
           else if (flags & NSControlKeyMask)
             evDown->key = "Control_L";
           else if (flags & NSAlternateKeyMask)
             evDown->key = "Alt_L";
           else if (flags & NSCommandKeyMask)
             evDown->key = "Super_L";
           else if (flags & NSAlphaShiftKeyMask)
             evDown->key = "Caps_Lock";

           if (evDown->key)
             {
                evDown->keyname = evDown->key;
                evDown->timestamp = time;
                evDown->string = NULL;
                ecore_event_add(ECORE_EVENT_KEY_DOWN, evDown, NULL, NULL);
                old_flags = flags;
                break;
             }

           free(evDown);

           evUp = calloc(1, sizeof (Ecore_Event_Key));
           if (!evUp)
             {
                return pass;
             }

           NSUInteger changed_flags = flags ^ old_flags;

           // Turn special key flags off
           if (changed_flags & NSShiftKeyMask)
             evUp->key = "Shift_L";
           else if (changed_flags & NSControlKeyMask)
             evUp->key = "Control_L";
           else if (changed_flags & NSAlternateKeyMask)
             evUp->key = "Alt_L";
           else if (changed_flags & NSCommandKeyMask)
             evUp->key = "Super_L";
           else if (changed_flags & NSAlphaShiftKeyMask)
             evUp->key = "Caps_Lock";

           if (evUp->key)
             {
                evUp->keyname = evDown->key;
                evUp->timestamp = time;
                evUp->string = NULL;
                ecore_event_add(ECORE_EVENT_KEY_UP, evUp, NULL, NULL);
                old_flags = flags;
                break;
             }

           break;
        }
      case NSScrollWheel:
        {
           DBG("Scroll Wheel");

           EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
           Ecore_Event_Mouse_Wheel *ev;
           float dx, dy = 0;

           ev = malloc(sizeof(Ecore_Event_Mouse_Wheel));
           if (!ev) return pass;

           if ([event hasPreciseScrollingDeltas])
             {
                dx = -[event scrollingDeltaX];
                dy = -[event scrollingDeltaY];
             }
           else
             {
                dx = -[event deltaX];
                dy = -[event deltaY];
             }

           if (dx == 0 && dy == 0)
             {
                break;
             }

           ev->window = (Ecore_Window)window.ecore_window_data;
           ev->event_window = ev->window;
           ev->modifiers = 0; /* FIXME: keep modifier around. */
           ev->timestamp = time;
           if (dy != 0)
             {
                ev->z = (dy >  0) ? 1 : -1;
             }
           else
             {
                ev->z = (dx >  0) ? 1 : -1;
             }
           ev->direction = (dy != 0) ? 0 : 1;

           ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);

           break;
        }
      default:
        {
           pass = EINA_TRUE;
           break;
        }
     }

   return pass;
}

EAPI void
ecore_cocoa_screen_size_get(Ecore_Cocoa_Screen *screen EINA_UNUSED, int *w, int *h)
{
   NSSize pt =  [[[NSScreen screens] objectAtIndex:0] frame].size;

   if (w) *w = (int)pt.width;
   if (h) *h = (int)pt.height;
   
   DBG("Screen size get : %dx%d", (int)pt.width, (int)pt.height);
}

EAPI int
ecore_cocoa_titlebar_height_get(void)
{
   static int height = -1;

   if (height == -1)
     {
        NSRect frame = NSMakeRect(0, 0, 100, 100);
        NSRect contentRect;
        contentRect = [NSWindow contentRectForFrameRect:frame
                                              styleMask:NSTitledWindowMask];
        height = (frame.size.height - contentRect.size.height);
        DBG("Titlebar Heigt : %d", height);
     }
   return height;
}
