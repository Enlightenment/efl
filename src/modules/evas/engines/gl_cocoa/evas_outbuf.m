#include <Cocoa/Cocoa.h>

#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"

#include <dlfcn.h>

static Outbuf *_evas_gl_cocoa_window = NULL;
static int _win_count = 0;

@interface EvasGLView : NSOpenGLView

+ (NSOpenGLPixelFormat*) basicPixelFormat;

@end

@implementation EvasGLView

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

- (id) initWithFrame: (NSRect) frameRect
{
   NSOpenGLPixelFormat *pf;
   NSOpenGLContext *ctx;

   pf = [EvasGLView basicPixelFormat];
   self = [super initWithFrame: frameRect
                   pixelFormat: pf];
   ctx = [[NSOpenGLContext alloc] initWithFormat:pf 
                                    shareContext: nil];

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

static void *
_dlsym(const char *sym)
{
   return dlsym(RTLD_DEFAULT, sym);
}

Outbuf *
evas_outbuf_new(Evas_Engine_Info_GL_Cocoa *info,
                int                        w,
                int                        h)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(info, NULL);

   Outbuf *ob;

   ob = calloc(1, sizeof(*ob));
   if (EINA_UNLIKELY(!ob))
     {
        CRI("Failed to allocate memory");
        goto die;
     }
   _win_count++;

   ob->width = w;
   ob->height = h;
   ob->info = info;
   ob->ns_window = info->window;

   ob->ns_gl_view = [[EvasGLView alloc] initWithFrame: NSMakeRect(0, 0, w, h)];
   if (EINA_UNLIKELY(!ob->ns_gl_view))
     {
        CRI("Failed to create gl_view");
        goto die;
     }
   [[(NSOpenGLView *)ob->ns_gl_view openGLContext] makeCurrentContext];

   glsym_evas_gl_symbols(_dlsym);
   ob->gl_context = glsym_evas_gl_common_context_new();
   if (EINA_UNLIKELY(!ob->gl_context))
     {
        CRI("Failed to create gl_context");
        goto die;
     }


   evas_outbuf_use(ob);

   glsym_evas_gl_common_context_resize(ob->gl_context, ob->width, ob->height, 0); // TODO rotation

   return ob;

die:
   if (ob) free(ob);
   return NULL;
}

void
evas_outbuf_free(Outbuf *ob)
{
   int refs = 0;

   _win_count--;
   evas_outbuf_use(ob);

   if (_win_count == 0) evas_common_font_ext_clear();

   if (ob == _evas_gl_cocoa_window) _evas_gl_cocoa_window = NULL;
   if (ob->gl_context)
     {
        refs = ob->gl_context->references - 1;
        glsym_evas_gl_common_context_free(ob->gl_context);
        [(NSOpenGLView *) ob->ns_gl_view release];
     }

   if (refs == 0)
     {
        // TODO
     }
   free(ob);
}

void
evas_outbuf_use(Outbuf *ob)
{
   Eina_Bool force = EINA_FALSE;

   // TODO preload_render_Lock

   if (_evas_gl_cocoa_window)
     {
        // TODO ifcurrent context is not glcontext
        force = EINA_TRUE;
     }

   if ((_evas_gl_cocoa_window != ob) || (force))
     {
        if (_evas_gl_cocoa_window)
          {
             glsym_evas_gl_common_context_use(_evas_gl_cocoa_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_gl_cocoa_window->gl_context);
          }

        [[(NSOpenGLView *)ob->ns_gl_view openGLContext] makeCurrentContext];
        _evas_gl_cocoa_window = ob;
        // TODO blah
     }

   if (ob) glsym_evas_gl_common_context_use(ob->gl_context);
}

void
evas_outbuf_resize(Outbuf *ob,
                   int     w,
                   int     h)
{
   NSRect view_frame;
   NSOpenGLView *const view = ob->ns_gl_view;

   INF("Resize %d %d\n", w, h);

   view_frame = [view frame];
   view_frame.size.height = h;
   view_frame.size.width = w;

   [view setFrame:view_frame];
   [[view openGLContext] flushBuffer];
}

void
evas_outbuf_lock_focus(Outbuf *ob)
{
   [(NSOpenGLView *)ob->ns_gl_view lockFocus];
}

void
evas_outbuf_unlock_focus(Outbuf *ob)
{
   [(NSOpenGLView *)ob->ns_gl_view unlockFocus];
}

void
evas_outbuf_swap_buffers(Outbuf *ob)
{
   [[(NSOpenGLView *)ob->ns_gl_view openGLContext] flushBuffer];
}

void // FIXME
evas_outbuf_vsync_set(int on EINA_UNUSED)
{
}
