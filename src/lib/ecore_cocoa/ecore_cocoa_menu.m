/* ecore_cocoa_menu.m
 * Cocoa NSMenu layer for the EFL Cocoa menu bridge.
 * Manual Retain-Release — NO ARC.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#import  <Cocoa/Cocoa.h>
#include "Ecore_Cocoa.h"
@class EcoreCocoaWindow; /* forward-declare for ecore_cocoa_private.h */
#include "ecore_cocoa_private.h"

/* ------------------------------------------------------------------ */
/* ObjC helper: one instance per menu item, retained in the targets    */
/* array; dispatches the C callback on NSMenuItem action.              */
/* ------------------------------------------------------------------ */
@interface _EcoreCocoaMenuTarget : NSObject
{
   void  (*_cb)(void *data, Ecore_Cocoa_Menu *menu, int idx);
   void           *_data;
   Ecore_Cocoa_Menu *_menu;
   int              _idx;
}
- (id)initWithCallback:(void (*)(void *, Ecore_Cocoa_Menu *, int))cb
                  data:(void *)data
                  menu:(Ecore_Cocoa_Menu *)menu
                   idx:(int)idx;
- (void)menuItemClicked:(id)sender;
@end

@implementation _EcoreCocoaMenuTarget
- (id)initWithCallback:(void (*)(void *, Ecore_Cocoa_Menu *, int))cb
                  data:(void *)data
                  menu:(Ecore_Cocoa_Menu *)menu
                   idx:(int)idx
{
   if (!(self = [super init])) return nil;
   _cb   = cb;
   _data = data;
   _menu = menu;
   _idx  = idx;
   return self;
}

- (void)menuItemClicked:(id)sender
{
   (void)sender;
   if (_cb) _cb(_data, _menu, _idx);
}

@end

/* ------------------------------------------------------------------ */
/* Opaque handle                                                        */
/* ------------------------------------------------------------------ */
struct _Ecore_Cocoa_Menu
{
   NSMenu         *ns_menu;
   NSMutableArray *targets; /* retains _EcoreCocoaMenuTarget instances */
};

/* Module-level main menu pointer */
static Ecore_Cocoa_Menu *_main_menu = NULL;

/* ------------------------------------------------------------------ */
/* Public C API                                                         */
/* ------------------------------------------------------------------ */

EAPI Ecore_Cocoa_Menu *
ecore_cocoa_menu_new(const char *title)
{
   Ecore_Cocoa_Menu *menu;
   NSString *ns_title;

   menu = calloc(1, sizeof(*menu));
   if (EINA_UNLIKELY(!menu))
     {
        CRI("Failed to allocate Ecore_Cocoa_Menu");
        return NULL;
     }

   ns_title = title ? [NSString stringWithUTF8String:title] : @"";
   menu->ns_menu = [[NSMenu alloc] initWithTitle:ns_title];
   if (EINA_UNLIKELY(!menu->ns_menu))
     {
        CRI("Failed to create NSMenu");
        free(menu);
        return NULL;
     }
   [menu->ns_menu setAutoenablesItems:NO];

   menu->targets = [[NSMutableArray alloc] init];
   if (EINA_UNLIKELY(!menu->targets))
     {
        CRI("Failed to create targets array");
        [menu->ns_menu release];
        free(menu);
        return NULL;
     }

   return menu;
}

EAPI void
ecore_cocoa_menu_free(Ecore_Cocoa_Menu *menu)
{
   if (!menu) return;

   [menu->targets release];
   [menu->ns_menu release];
   free(menu);
}

EAPI int
ecore_cocoa_menu_item_add(Ecore_Cocoa_Menu *menu,
                          const char       *label,
                          const char       *icon EINA_UNUSED,
                          void (*cb)(void *data, Ecore_Cocoa_Menu *menu, int idx),
                          void             *data)
{
   NSMenuItem           *item;
   _EcoreCocoaMenuTarget *target;
   NSString             *ns_label;
   int                   idx;

   EINA_SAFETY_ON_NULL_RETURN_VAL(menu, -1);

   idx      = [menu->ns_menu numberOfItems];
   ns_label = label ? [NSString stringWithUTF8String:label] : @"";

   target = [[_EcoreCocoaMenuTarget alloc] initWithCallback:cb
                                                       data:data
                                                       menu:menu
                                                        idx:idx];
   if (EINA_UNLIKELY(!target))
     {
        CRI("Failed to create menu target");
        return -1;
     }

   item = [[NSMenuItem alloc] initWithTitle:ns_label
                                     action:@selector(menuItemClicked:)
                              keyEquivalent:@""];
   if (EINA_UNLIKELY(!item))
     {
        CRI("Failed to create NSMenuItem");
        [target release];
        return -1;
     }

   [item setTarget:target];
   [item setEnabled:YES];
   [item setTag:(NSInteger)idx]; /* default tag = positional index at creation */
   [menu->ns_menu addItem:item];
   [menu->targets addObject:target];

   [item release];
   [target release]; /* targets array retains it */

   return idx;
}

EAPI int
ecore_cocoa_menu_item_add_separator(Ecore_Cocoa_Menu *menu)
{
   int idx;

   EINA_SAFETY_ON_NULL_RETURN_VAL(menu, -1);

   idx = [menu->ns_menu numberOfItems];
   [menu->ns_menu addItem:[NSMenuItem separatorItem]];
   /* separators have no target; push NULL placeholder to keep array aligned */
   [menu->targets addObject:[NSNull null]];

   return idx;
}

EAPI void
ecore_cocoa_menu_item_del(Ecore_Cocoa_Menu *menu, int idx)
{
   EINA_SAFETY_ON_NULL_RETURN(menu);
   if (idx < 0 || (NSInteger)idx >= [menu->ns_menu numberOfItems]) return;

   /* targets array is kept parallel to ns_menu items — remove the same slot */
   if ([menu->targets count] != (NSUInteger)[menu->ns_menu numberOfItems])
     CRI("targets/items array mismatch (%lu vs %ld) — skipping target removal",
         (unsigned long)[menu->targets count],
         (long)[menu->ns_menu numberOfItems]);
   else if ((NSUInteger)idx < [menu->targets count])
     [menu->targets removeObjectAtIndex:(NSUInteger)idx];

   [menu->ns_menu removeItemAtIndex:idx];
}

EAPI void
ecore_cocoa_menu_item_tag_set(Ecore_Cocoa_Menu *menu, int idx, int tag)
{
   NSMenuItem *item;

   EINA_SAFETY_ON_NULL_RETURN(menu);
   if (idx < 0 || (NSInteger)idx >= [menu->ns_menu numberOfItems]) return;

   item = [menu->ns_menu itemAtIndex:idx];
   [item setTag:(NSInteger)tag];
}

EAPI void
ecore_cocoa_menu_item_del_by_tag(Ecore_Cocoa_Menu *menu, int tag)
{
   NSInteger ns_idx;

   EINA_SAFETY_ON_NULL_RETURN(menu);

   ns_idx = [menu->ns_menu indexOfItemWithTag:(NSInteger)tag];
   if (ns_idx < 0) return;

   if ([menu->targets count] != (NSUInteger)[menu->ns_menu numberOfItems])
     CRI("targets/items array mismatch (%lu vs %ld) — skipping target removal",
         (unsigned long)[menu->targets count],
         (long)[menu->ns_menu numberOfItems]);
   else if ((NSUInteger)ns_idx < [menu->targets count])
     [menu->targets removeObjectAtIndex:(NSUInteger)ns_idx];

   [menu->ns_menu removeItemAtIndex:ns_idx];
}

EAPI int
ecore_cocoa_menu_item_index_by_tag(Ecore_Cocoa_Menu *menu, int tag)
{
   NSInteger ns_idx;

   EINA_SAFETY_ON_NULL_RETURN_VAL(menu, -1);

   ns_idx = [menu->ns_menu indexOfItemWithTag:(NSInteger)tag];
   /* NSNotFound is typically a large positive value; map it to -1 */
   if (ns_idx == NSNotFound || ns_idx < 0) return -1;

   return (int)ns_idx;
}

EAPI void
ecore_cocoa_menu_item_enabled_set(Ecore_Cocoa_Menu *menu, int idx, Eina_Bool enabled)
{
   NSMenuItem *item;

   EINA_SAFETY_ON_NULL_RETURN(menu);
   if (idx < 0 || (NSInteger)idx >= [menu->ns_menu numberOfItems]) return;

   item = [menu->ns_menu itemAtIndex:idx];
   [item setEnabled:(enabled ? YES : NO)];
}

EAPI void
ecore_cocoa_menu_item_enabled_set_by_tag(Ecore_Cocoa_Menu *menu, int tag, Eina_Bool enabled)
{
   NSInteger ns_idx;
   NSMenuItem *item;

   EINA_SAFETY_ON_NULL_RETURN(menu);

   ns_idx = [menu->ns_menu indexOfItemWithTag:(NSInteger)tag];
   if (ns_idx < 0) return;

   item = [menu->ns_menu itemAtIndex:ns_idx];
   [item setEnabled:(enabled ? YES : NO)];
}

EAPI void
ecore_cocoa_menu_item_label_set(Ecore_Cocoa_Menu *menu, int idx, const char *label)
{
   NSMenuItem *item;

   EINA_SAFETY_ON_NULL_RETURN(menu);
   if (idx < 0 || (NSInteger)idx >= [menu->ns_menu numberOfItems]) return;

   item = [menu->ns_menu itemAtIndex:idx];
   [item setTitle:label ? [NSString stringWithUTF8String:label] : @""];
}

EAPI void
ecore_cocoa_menu_submenu_set(Ecore_Cocoa_Menu *menu, int idx,
                             Ecore_Cocoa_Menu *submenu)
{
   NSMenuItem *item;

   EINA_SAFETY_ON_NULL_RETURN(menu);
   EINA_SAFETY_ON_NULL_RETURN(submenu);
   if (idx < 0 || idx >= [menu->ns_menu numberOfItems]) return;

   item = [menu->ns_menu itemAtIndex:idx];
   /* Submenu-bearing items must not have a target/action — macOS ignores
    * the submenu and fires the action instead if both are set. */
   [item setTarget:nil];
   [item setAction:NULL];
   [menu->ns_menu setSubmenu:submenu->ns_menu forItem:item];
}

EAPI void
ecore_cocoa_menu_main_set(Ecore_Cocoa_Menu *menu)
{
   EINA_SAFETY_ON_NULL_RETURN(menu);

   /* Note: [NSApp isRunning] returns NO after the initial bootstrap
    * because ecore_cocoa calls [NSApp stop:nil] in
    * applicationDidFinishLaunching and drives the run-loop manually.
    * So we set the main menu unconditionally — by the time an EFL
    * window exists and calls elm_win_main_menu_get(), NSApp is fully
    * initialised even though isRunning is false. */
   [NSApp setMainMenu:menu->ns_menu];
   _main_menu = menu;
}

EAPI Ecore_Cocoa_Menu *
ecore_cocoa_menu_main_get(void)
{
   return _main_menu;
}
