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

   /* Disable macOS automatic window tabbing (Sierra+).  EFL applications
    * manage their own window/tab lifecycle, so we do not want the system
    * to silently merge new windows into a single tabbed frame. */
   if (@available(macOS 10.12, *))
     [NSWindow setAllowsAutomaticWindowTabbing:NO];

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

   ecore_event_type_flush(ECORE_COCOA_EVENT_WINDOW_UNFOCUSED,
                          ECORE_COCOA_EVENT_WINDOW_FOCUSED,
                          ECORE_COCOA_EVENT_WINDOW_RESIZE_REQUEST,
                          ECORE_COCOA_EVENT_WINDOW_DESTROY);

   ecore_event_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_ecore_cocoa_log_domain);

   return _ecore_cocoa_init_count;
}

unsigned int
ecore_cocoa_event_modifiers(NSUInteger mod)
{
   unsigned int modifiers = 0;

   if (mod & NSEventModifierFlagShift) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   /* Swap Command ↔ Control so that Command (⌘) acts as the primary
    * "Control" modifier for EFL applications, matching the standard
    * macOS convention used by Qt, Electron, and most cross-platform
    * toolkits.  The physical Control key becomes "Super". */
   if (mod & NSEventModifierFlagCommand) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mod & NSEventModifierFlagControl) modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mod & NSEventModifierFlagOption) modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
   if (mod & NSEventModifierFlagNumericPad) modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mod & NSEventModifierFlagCapsLock) modifiers |= ECORE_EVENT_LOCK_CAPS;

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
   int kchar = -1;

   Ecore_Event_Key *ev;

   EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
   NSString *keychar = [event charactersIgnoringModifiers];
   NSString *keycharRaw = [event characters];

   DBG("Event Key, keyType : %lu", keyType);

   ev = calloc(1, sizeof(*ev));
   if (EINA_UNLIKELY(!ev)) return NULL;

   if (compose && (keyType == NSEventTypeKeyDown))
     {
        [edit interpretKeyEvents:[NSArray arrayWithObject:event]];
        compose = EINA_FALSE;
     }

   ev->timestamp = time;
   ev->modifiers = ecore_cocoa_event_modifiers([event modifierFlags]);

   ev->keycode = event.keyCode;
   ev->string = [keycharRaw cStringUsingEncoding:NSUTF8StringEncoding];
   ev->compose = ev->string;

   ev->window = (Ecore_Window)window.ecore_window_data;
   ev->event_window = ev->window;

   /*
    * Try to look for the keychar data if available.
    * If not, try the raw keychar.
    */
   if ([keychar length] > 0)
     kchar = [keychar characterAtIndex: 0];
   if ((kchar < 0) && ([keycharRaw length] > 0))
     kchar = [keycharRaw characterAtIndex: 0];

   if (kchar >= 0)
     {
        for (i = 0; i < EINA_C_ARRAY_LENGTH(keystable); ++i)
          {
             if (keystable[i].code == kchar)
               {
                  ev->keyname = keystable[i].name;
                  ev->key = ev->keyname;
                  break;
               }
          }
     }

   /* Fallback */
   if (!ev->keyname)
     {
        ev->keyname = "";
        ev->key = "";
     }

   if (([keycharRaw length] == 0) && (keyType == NSEventTypeKeyDown))
     {
        compose = EINA_TRUE;
        edit = [[event window] fieldEditor:YES forObject:nil];
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
   if (EINA_UNLIKELY(_ecore_cocoa_init_count <= 0)) return EINA_FALSE;

   NSEvent *event = anEvent;
   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
   Eina_Bool pass = EINA_FALSE;

   DBG("Feed events, event type ; %lx", [event type]);

   switch ([event type])
     {
      case NSEventTypeMouseMoved:
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
      case NSEventTypeLeftMouseDown:
      case NSEventTypeRightMouseDown:
      case NSEventTypeOtherMouseDown:
      case NSEventTypeLeftMouseUp:
      case NSEventTypeRightMouseUp:
      case NSEventTypeOtherMouseUp:
        {
           //mouse events are managed in EcoreCocoaWindow
           return EINA_TRUE;
        }
      case NSEventTypeKeyDown:
        {
           Ecore_Event_Key *ev;
           NSUInteger flags = [event modifierFlags];

           if (flags & NSEventModifierFlagCommand)
             {
                NSString *keychar = [event charactersIgnoringModifiers];
                if ([keychar characterAtIndex:0] == 'q')
                  {
                     [NSApp performSelector:@selector(terminate:)
                                            withObject:nil afterDelay:0.0];
                     return EINA_TRUE;
                  }
             }

           ev = _ecore_cocoa_event_key(event, NSEventTypeKeyDown, time);
           if (ev == NULL) return EINA_TRUE;

           ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);

           break;
        }
      case NSEventTypeKeyUp:
        {
           Ecore_Event_Key *ev;

           ev = _ecore_cocoa_event_key(event, NSEventTypeKeyUp, time);
           if (ev == NULL) return EINA_TRUE;

           ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);

           break;
        }
      case NSEventTypeFlagsChanged:
        {
           NSUInteger flags = [event modifierFlags];
           unsigned short keyCode = [event keyCode];
           EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];

           Ecore_Event_Key *evDown = NULL;
           Ecore_Event_Key *evUp = NULL;
           const char *key = NULL;
           int keylen;

           /* Use keyCode to distinguish left/right modifier keys.
            * macOS keyCodes: LShift=56 RShift=60 LCmd=55 RCmd=54
            *                 LCtrl=59 RCtrl=62 LOpt=58 ROpt=61 */
           // Turn special key flags on
           if (flags & NSEventModifierFlagShift)
             key = (keyCode == 60) ? "Shift_R" : "Shift_L";
           else if (flags & NSEventModifierFlagCommand)
             key = (keyCode == 54) ? "Control_R" : "Control_L";
           else if (flags & NSEventModifierFlagControl)
             key = (keyCode == 62) ? "Super_R" : "Super_L";
           else if (flags & NSEventModifierFlagOption)
             key = (keyCode == 61) ? "Alt_R" : "Alt_L";
           else if (flags & NSEventModifierFlagCapsLock)
             key = "Caps_Lock";
           else if (flags & NSEventModifierFlagNumericPad)
             key = "Num_Lock";

           if (key)
             {
                keylen = strlen(key);
                evDown = calloc(1, sizeof (Ecore_Event_Key) + (keylen * 2) + 2);
                if (!evDown) return pass;

                evDown->keyname = (char *)(evDown + 1);
                evDown->key = evDown->keyname + keylen + 1;

                strcpy((char *) evDown->keyname, key);
                strcpy((char *) evDown->key, key);

                evDown->window = (Ecore_Window)window.ecore_window_data;
                evDown->event_window = evDown->window;
                evDown->timestamp = time;
                evDown->string = NULL;
                evDown->keycode = keyCode;
                ecore_event_add(ECORE_EVENT_KEY_DOWN, evDown, NULL, NULL);
                old_flags = flags;
                break;
             }

           key = NULL;

           NSUInteger changed_flags = flags ^ old_flags;

           // Turn special key flags off
           if (changed_flags & NSEventModifierFlagShift)
             key = (keyCode == 60) ? "Shift_R" : "Shift_L";
           else if (changed_flags & NSEventModifierFlagCommand)
             key = (keyCode == 54) ? "Control_R" : "Control_L";
           else if (changed_flags & NSEventModifierFlagControl)
             key = (keyCode == 62) ? "Super_R" : "Super_L";
           else if (changed_flags & NSEventModifierFlagOption)
             key = (keyCode == 61) ? "Alt_R" : "Alt_L";
           else if (changed_flags & NSEventModifierFlagCapsLock)
             key = "Caps_Lock";
           else if (changed_flags & NSEventModifierFlagNumericPad)
             key = "Num_Lock";

           if (key)
             {
                keylen = strlen(key);
                evUp = calloc(1, sizeof (Ecore_Event_Key) + (keylen * 2) + 2);
                if (!evUp) return pass;

                evUp->keyname = (char *)(evUp + 1);
                evUp->key = evUp->keyname + keylen + 1;

                strcpy((char *) evUp->keyname, key);
                strcpy((char *) evUp->key, key);

                evUp->window = (Ecore_Window)window.ecore_window_data;
                evUp->event_window = evUp->window;
                evUp->timestamp = time;
                evUp->string = NULL;
                ecore_event_add(ECORE_EVENT_KEY_UP, evUp, NULL, NULL);
                old_flags = flags;
                break;
             }

           break;
        }
      case NSEventTypeScrollWheel:
        {
           /* Trackpads report fine-grained pixel deltas via
            * hasPreciseScrollingDeltas.  Each tiny delta used to produce
            * z = ±1, making scrolling absurdly fast.  Accumulate the
            * pixel values and only emit a wheel event once the built-up
            * amount crosses a per-line threshold. */
           static float _scroll_acc_x = 0.0f;
           static float _scroll_acc_y = 0.0f;
           static const float PRECISE_SCROLL_THRESHOLD = 40.0f;

           DBG("Scroll Wheel");

           EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];

           if ([event hasPreciseScrollingDeltas])
             {
                NSEventPhase phase = [event phase];
                NSEventPhase momentum = [event momentumPhase];
                int steps;

                if (phase == NSEventPhaseBegan)
                  {
                     _scroll_acc_x = 0.0f;
                     _scroll_acc_y = 0.0f;
                  }

                _scroll_acc_x += [event scrollingDeltaX];
                _scroll_acc_y += [event scrollingDeltaY];

                /* vertical */
                steps = (int)(_scroll_acc_y / PRECISE_SCROLL_THRESHOLD);
                if (steps != 0)
                  {
                     Ecore_Event_Mouse_Wheel *ev;
                     ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
                     if (ev)
                       {
                          _scroll_acc_y -= steps * PRECISE_SCROLL_THRESHOLD;
                          ev->window = (Ecore_Window)window.ecore_window_data;
                          ev->event_window = ev->window;
                          ev->modifiers = ecore_cocoa_event_modifiers([event modifierFlags]);
                          ev->timestamp = time;
                          ev->direction = 0;
                          ev->z = -steps;
                          ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
                       }
                  }

                /* horizontal */
                steps = (int)(_scroll_acc_x / PRECISE_SCROLL_THRESHOLD);
                if (steps != 0)
                  {
                     Ecore_Event_Mouse_Wheel *ev;
                     ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
                     if (ev)
                       {
                          _scroll_acc_x -= steps * PRECISE_SCROLL_THRESHOLD;
                          ev->window = (Ecore_Window)window.ecore_window_data;
                          ev->event_window = ev->window;
                          ev->modifiers = ecore_cocoa_event_modifiers([event modifierFlags]);
                          ev->timestamp = time;
                          ev->direction = 1;
                          ev->z = -steps;
                          ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
                       }
                  }

                if (phase == NSEventPhaseEnded ||
                    phase == NSEventPhaseCancelled ||
                    momentum == NSEventPhaseEnded ||
                    momentum == NSEventPhaseCancelled)
                  {
                     _scroll_acc_x = 0.0f;
                     _scroll_acc_y = 0.0f;
                  }
             }
           else
             {
                /* Discrete scrolling (mouse wheel): one event per click. */
                Ecore_Event_Mouse_Wheel *ev;
                float dx = -[event deltaX];
                float dy = -[event deltaY];

                if (dx == 0 && dy == 0) break;

                ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
                if (!ev) break;

                ev->window = (Ecore_Window)window.ecore_window_data;
                ev->event_window = ev->window;
                ev->modifiers = ecore_cocoa_event_modifiers([event modifierFlags]);
                ev->timestamp = time;
                if (dy != 0)
                  ev->z = (dy > 0) ? 1 : -1;
                else
                  ev->z = (dx > 0) ? 1 : -1;
                ev->direction = (dy != 0) ? 0 : 1;
                ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
             }

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
                                              styleMask:NSWindowStyleMaskTitled];
        height = (frame.size.height - contentRect.size.height);
        DBG("Titlebar Heigt : %d", height);
     }
   return height;
}

EAPI void
ecore_cocoa_terminate_cb_set(Ecore_Cocoa_Terminate_Cb cb)
{
   EINA_SAFETY_ON_NULL_RETURN(cb);
   [NSApp setTerminateCb: cb];
}

EAPI Eina_Bool
ecore_cocoa_app_icon_set(const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);

   @autoreleasepool {
      NSString *ns_path = [NSString stringWithUTF8String:path];
      NSImage *icon = [[NSImage alloc] initWithContentsOfFile:ns_path];
      if (!icon)
        {
           ERR("Failed to load icon from: %s", path);
           return EINA_FALSE;
        }
      [NSApp setApplicationIconImage:icon];
#if !__has_feature(objc_arc)
      [icon release];
#endif
   }
   return EINA_TRUE;
}

EAPI void
ecore_cocoa_reopen_cb_set(Ecore_Cocoa_Reopen_Cb cb)
{
   [(Ecore_Cocoa_Application *)NSApp setReopenCb:cb];
}

EAPI void
ecore_cocoa_url_open_cb_set(Ecore_Cocoa_URL_Open_Cb cb)
{
   [(Ecore_Cocoa_Application *)NSApp setUrlOpenCb:cb];
}
