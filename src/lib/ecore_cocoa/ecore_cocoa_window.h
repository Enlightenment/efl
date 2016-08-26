#import <Cocoa/Cocoa.h>

@interface EcoreCocoaWindow: NSWindow <NSWindowDelegate>
{
   void *ecore_window_data;
   int _live_resize;
}

@property (nonatomic, assign) void *ecore_window_data;

- (id) initWithContentRect: (NSRect) contentRect
                 styleMask: (unsigned int) aStyle
                   backing: (NSBackingStoreType) bufferingType
                     defer: (BOOL) flag;

- (BOOL)isFullScreen;

- (BOOL) requestResize: (NSSize) size;

@end

