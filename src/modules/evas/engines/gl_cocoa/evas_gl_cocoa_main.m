
#include <Cocoa/Cocoa.h>

#include "evas_engine.h"

static Evas_GL_Cocoa_Window *_evas_gl_cocoa_window = NULL;

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
   return self;
}

@end


Evas_GL_Cocoa_Window *
eng_window_new(void *window,
	       int      w,
	       int      h)
{
   Evas_GL_Cocoa_Window *gw;
   int context_attrs[3];
   int config_attrs[20];
   int major_version, minor_version;
   int num_config;

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
   if (gw == _evas_gl_cocoa_window) _evas_gl_cocoa_window = NULL;
   evas_gl_common_context_free(gw->gl_context);
   free(gw);
}

void
eng_window_use(Evas_GL_Cocoa_Window *gw)
{
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
eng_window_vsync_set(int on)
{

}


void
eng_window_resize(Evas_GL_Cocoa_Window *gw, int width, int height)
{
  NSRect view_frame;

  INF("Resize %d %d\n", width, height);

  view_frame = [(EvasGLView*)gw->view frame];
  printf("view_frame : %3.3f %3.3f\n", view_frame.size.height, view_frame.size.width);
  view_frame.size.height = height;
  view_frame.size.width = width;
  printf("view_frame : %3.3f %3.3f\n", view_frame.size.height, view_frame.size.width);
  [(EvasGLView*)gw->view setFrame:view_frame];
  [[(NSOpenGLView*)gw->view openGLContext] flushBuffer];
}

