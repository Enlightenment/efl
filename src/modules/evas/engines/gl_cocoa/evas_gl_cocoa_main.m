#include <Cocoa/Cocoa.h>

#include "evas_engine.h"

static Evas_GL_Cocoa_Window *_evas_gl_cocoa_window = NULL;
static NSOpenGLContext *_evas_gl_cocoa_shared_context = NULL;

@interface EvasGLView : NSOpenGLView
{
}

+ (NSOpenGLPixelFormat*) basicPixelFormat;
- (id) initWithFrame: (NSRect) frameRect;

@end


@implementation EvasGLView

- (id) init
{
   self = [super init];
   return self;
}

+ (NSOpenGLPixelFormat*) basicPixelFormat
{
   NSOpenGLPixelFormatAttribute attributes [] = {
     NSOpenGLPFAWindow,
     NSOpenGLPFAAccelerated,
     NSOpenGLPFADoubleBuffer,
     /*NSOpenGLPFAColorSize, 24,
       NSOpenGLPFAAlphaSize, 8,
       NSOpenGLPFADepthSize, 24,*/
     0
   };
   return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
}

// ---------------------------------

-(id) initWithFrame: (NSRect) frameRect
{
   NSOpenGLPixelFormat * pf = [EvasGLView basicPixelFormat];
   self = [super initWithFrame: frameRect pixelFormat: pf];


   NSOpenGLContext *ctx;
   if (!_evas_gl_cocoa_shared_context) {
      _evas_gl_cocoa_shared_context = [[NSOpenGLContext alloc] initWithFormat: [EvasGLView basicPixelFormat] shareContext: nil];
      ctx = _evas_gl_cocoa_shared_context;
   } else {
      ctx = [[NSOpenGLContext alloc] initWithFormat: [EvasGLView basicPixelFormat] shareContext: _evas_gl_cocoa_shared_context];
   }
   [self setOpenGLContext: ctx];
   [ctx setView: self];

   return self;
}

- (void)unlockFocus
{
   //[super unlockFocus];
}

- (void)lockFocus
{
   NSOpenGLContext* context = [self openGLContext];

   //[super lockFocus];
   if ([context view] != self) {
      [context setView:self];
   }
   [context makeCurrentContext];
}

@end


Evas_GL_Cocoa_Window *
eng_window_new(void *window,
               int      w,
               int      h)
{
   Evas_GL_Cocoa_Window *gw;

   gw = calloc(1, sizeof(Evas_GL_Cocoa_Window));
   if (!gw) return NULL;

   _evas_gl_cocoa_window = gw;
   gw->window = window;
   gw->view = [[EvasGLView alloc] initWithFrame:NSMakeRect(0,0,w,h)];
   NSOpenGLContext *ctx = [(NSOpenGLView*)gw->view openGLContext];
   [ctx makeCurrentContext];
   gw->gl_context = evas_gl_common_context_new();

   if (!gw->gl_context)
     {
        free(gw);
        return NULL;
     }
   evas_gl_common_context_use(gw->gl_context);
   evas_gl_common_context_resize(gw->gl_context, w, h, 0);

   return gw;
}

void
eng_window_free(Evas_GL_Cocoa_Window *gw)
{
   if (gw == _evas_gl_cocoa_window)
     _evas_gl_cocoa_window = NULL;

   evas_common_font_ext_clear();
   evas_gl_common_context_free(gw->gl_context);
   [(EvasGLView*)gw->view release];
   free(gw);
}

void
eng_window_use(Evas_GL_Cocoa_Window *gw)
{
   if ((gw) && (!gw->gl_context)) return;
   if (_evas_gl_cocoa_window != gw)
     {
        [[(NSOpenGLView*)gw->view openGLContext] makeCurrentContext];
        if (_evas_gl_cocoa_window)
          evas_gl_common_context_flush(_evas_gl_cocoa_window->gl_context);
        _evas_gl_cocoa_window = gw;

     }
   evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_swap_buffers(Evas_GL_Cocoa_Window *gw)
{
   [[(NSOpenGLView*)gw->view openGLContext] flushBuffer];
}

void
eng_window_vsync_set(int on EINA_UNUSED)
{

}


void
eng_window_resize(Evas_GL_Cocoa_Window *gw, int width, int height)
{
   NSRect view_frame;

   INF("Resize %d %d\n", width, height);

   view_frame = [(EvasGLView*)gw->view frame];
   view_frame.size.height = height;
   view_frame.size.width = width;

   [(EvasGLView*)gw->view setFrame:view_frame];
   [[(NSOpenGLView*)gw->view openGLContext] flushBuffer];
}

void
eng_window_lock_focus(Evas_GL_Cocoa_Window *gw)
{
   [(NSOpenGLView*)gw->view lockFocus];
}

void
eng_window_unlock_focus(Evas_GL_Cocoa_Window *gw)
{
   [(NSOpenGLView*)gw->view unlockFocus];
}
