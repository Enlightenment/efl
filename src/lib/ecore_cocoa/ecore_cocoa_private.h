#ifndef _ECORE_COCOA_PRIVATE_H
#define _ECORE_COCOA_PRIVATE_H

extern int _ecore_cocoa_log_domain;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_cocoa_log_domain, __VA_ARGS__)

/*
 * macOS Sierra (10.12) deprecated enumeration types in profit to others,
 * more meaningful ones. We define aliases to these new types to use the
 * most recent API while being retro-compatible.
 */
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101200 /* Before 10.12 */
# define NSWindowStyleMaskTitled NSTitledWindowMask
# define NSWindowStyleMaskClosable NSClosableWindowMask
# define NSWindowStyleMaskResizable NSResizableWindowMask
# define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
# define NSWindowStyleMaskFullScreen NSFullScreenWindowMask
# define NSEventModifierFlagShift NSShiftKeyMask
# define NSEventModifierFlagControl NSControlKeyMask
# define NSEventModifierFlagOption NSAlternateKeyMask
# define NSEventModifierFlagCommand NSCommandKeyMask
# define NSEventModifierFlagCapsLock NSAlphaShiftKeyMask
# define NSEventTypeScrollWheel NSScrollWheel
# define NSEventMaskAny NSAnyEventMask
# define NSEventTypePeriodic NSPeriodic
# define NSEventTypeMouseMoved NSMouseMoved
# define NSEventTypeRightMouseDown NSRightMouseDown
# define NSEventTypeLeftMouseDown NSLeftMouseDown
# define NSEventTypeOtherMouseDown NSOtherMouseDown
# define NSEventTypeLeftMouseUp NSLeftMouseUp
# define NSEventTypeRightMouseUp NSRightMouseUp
# define NSEventTypeOtherMouseUp NSOtherMouseUp
# define NSEventTypeKeyDown NSKeyDown
# define NSEventTypeKeyUp NSKeyUp
# define NSEventTypeFlagsChanged NSFlagsChanged
# define NSEventTypeLeftMouseDragged NSLeftMouseDragged
# define NSEventTypeRightMouseDragged NSRightMouseDragged
# define NSEventTypeOtherMouseDragged NSOtherMouseDragged
# define NSEventModifierFlagNumericPad NSNumericPadKeyMask
#endif

struct _Ecore_Cocoa_Window
{
   EcoreCocoaWindow *window;
   unsigned int borderless : 1;
};

struct _Ecore_Cocoa_Screen
{
  int dummy;
};


/* Internal init */
Eina_Bool _ecore_cocoa_window_init(void);

Eina_Bool _ecore_cocoa_feed_events(void *anEvent);

unsigned int ecore_cocoa_event_modifiers(NSUInteger mod);



#endif
