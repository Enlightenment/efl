#include <Cocoa/Cocoa.h>

#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"

#include <dlfcn.h>

#define TRACE
#undef TRACE // Remove me for intense debug

#ifdef TRACE
# define TRC(...) DBG(__VA_ARGS__)
#else
# define TRC(...)
#endif

static Outbuf *_evas_gl_cocoa_window = NULL;
static NSOpenGLContext *_evas_gl_cocoa_shared_context = nil;
static int _win_count = 0;

@interface EvasGLView : NSOpenGLView

+ (NSOpenGLPixelFormat*) basicPixelFormat;

@end

@implementation EvasGLView

+ (NSOpenGLPixelFormat*) basicPixelFormat
{
   const NSOpenGLPixelFormatAttribute attributes[] = {
      NSOpenGLPFAAccelerated,
      NSOpenGLPFATripleBuffer,
      0 /* Terminator */
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

   /*
   * Evas GL backend uses a global OpenGL context for all windows inside
   * an application. Each window has its own texture to render its
   * content. We use a singleton NSOpenGLContext shared between all
   * NSOpenGLView, it solves rendering issues for multiple windows.
   */
   if (!_evas_gl_cocoa_shared_context)
     {
        _evas_gl_cocoa_shared_context = [[NSOpenGLContext alloc] initWithFormat: pf
                                                                   shareContext: nil];
        ctx = _evas_gl_cocoa_shared_context;
     }
   else
     {
        ctx = [[NSOpenGLContext alloc] initWithFormat: pf
                                         shareContext: _evas_gl_cocoa_shared_context];
     }

   [self setOpenGLContext: ctx];

   return self;
}

- (void)unlockFocus
{
   //[super unlockFocus];
}

- (void)lockFocus
{
   NSOpenGLContext *context;

   context = [self openGLContext];

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
   EvasGLView *glview;

   ob = calloc(1, sizeof(*ob));
   if (EINA_UNLIKELY(!ob))
     {
        CRI("Failed to allocate memory");
        goto die;
     }

   ob->w = w;
   ob->h = h;
   ob->info = info;
   ob->ns_window = info->window;
   ob->rot = 0;
   ob->swap_mode = MODE_TRIPLE;

   glview = [[EvasGLView alloc] initWithFrame: NSMakeRect(0, 0, w, h)];
   if (EINA_UNLIKELY(!glview))
     {
        CRI("Failed to create gl_view");
        goto die;
     }
   ob->ns_gl_view = glview;
   [[glview openGLContext] makeCurrentContext];
   glsym_evas_gl_symbols(_dlsym, NULL); /* XXX: NULL is ok for now for extns
   * but i need to find out howon osx to get the extension string list
   * for egl/whatever */

   ob->gl_context = glsym_evas_gl_common_context_new();
   if (EINA_UNLIKELY(!ob->gl_context))
     {
        CRI("Failed to create gl_context");
        goto die;
     }

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context,
                                       ob->w, ob->h, ob->rot);
   _win_count++;
   TRC("add: %p", ob);
   return ob;

die:
   if (ob) free(ob);
   return NULL;
}

void
evas_outbuf_free(Outbuf *ob)
{
   TRC("del: %p", ob);

   evas_outbuf_use(ob);

   _win_count--;
   if (_win_count == 0) evas_common_font_ext_clear();

   if (ob == _evas_gl_cocoa_window) _evas_gl_cocoa_window = NULL;
   if (ob->gl_context)
     {
        glsym_evas_gl_common_context_free(ob->gl_context);
        [(NSOpenGLView *) ob->ns_gl_view release];
     }

   free(ob);
}

static Eina_Bool
_evas_outbuf_make_current(void *data EINA_UNUSED,
                          void *doit EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(EINA_FALSE);
}

void
evas_outbuf_use(Outbuf *ob)
{
   if (_evas_gl_cocoa_window != ob)
     {
        if (_evas_gl_cocoa_window)
          {
             glsym_evas_gl_common_context_use(_evas_gl_cocoa_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_gl_cocoa_window->gl_context);
          }
        _evas_gl_cocoa_window = ob;
     }
   [[(NSOpenGLView* )ob->ns_gl_view openGLContext] makeCurrentContext];
   glsym_evas_gl_common_context_use(ob->gl_context);
}

int
evas_outbuf_rot_get(Outbuf *ob)
{
   return ob->rot;
}

Render_Output_Swap_Mode
evas_outbuf_buffer_state_get(Outbuf *ob)
{
   return ob->swap_mode;
}

Evas_Engine_GL_Context *
evas_outbuf_gl_context_get(Outbuf *ob)
{
   return ob->gl_context;
}

void
evas_outbuf_update_region_push(Outbuf     *ob,
                               RGBA_Image *update EINA_UNUSED,
                               int         x      EINA_UNUSED,
                               int         y      EINA_UNUSED,
                               int         w      EINA_UNUSED,
                               int         h      EINA_UNUSED)
{
   TRC("");
   /* Is it really necessary to flush per region ? Shouldn't we be able to
      still do that for the full canvas when doing partial update */
   ob->drew = EINA_TRUE;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void
evas_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   TRC("");
   /* Nothing to do here as we don't really create an image per area */
}

void *
evas_outbuf_update_region_new(Outbuf *ob,
                              int     x,
                              int     y,
                              int     w,
                              int     h,
                              int    *cx EINA_UNUSED,
                              int    *cy EINA_UNUSED,
                              int    *cw EINA_UNUSED,
                              int    *ch EINA_UNUSED)
{
   TRC("x,y,w,h = %i,%i,%i,%i", x, y, w, h);

   if ((w == ob->w) && (h == ob->h))
     ob->gl_context->master_clip.enabled = EINA_FALSE;
   else
     {
        ob->gl_context->master_clip.enabled = EINA_TRUE;
        ob->gl_context->master_clip.x = x;
        ob->gl_context->master_clip.y = y;
        ob->gl_context->master_clip.w = w;
        ob->gl_context->master_clip.h = h;
     }

   return ob->gl_context->def_surface;
}

Eina_Bool
evas_outbuf_update_region_first_rect(Outbuf *ob)
{
   TRC("");

   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);
   evas_outbuf_use(ob);

   glsym_evas_gl_common_context_resize(ob->gl_context,
                                       ob->w, ob->h,
                                       ob->rot);
   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);

   /* To debug partial rendering */
   //glClearColor(0.2, 0.5, 1.0, 1.0);
   //glClear(GL_COLOR_BUFFER_BIT);

   return EINA_FALSE;
}

void
evas_outbuf_flush(Outbuf           *ob,
                  Tilebuf_Rect     *surface_damage      EINA_UNUSED,
                  Tilebuf_Rect     *buffer_damage       EINA_UNUSED,
                  Evas_Render_Mode  render_mode)
{
   NSOpenGLView *const view = ob->ns_gl_view;

   TRC("drew is %s", ob->drew ? "true" : "false");

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;
   if (!ob->drew) goto end;

   ob->drew = EINA_FALSE;
   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_done(ob->gl_context);

   [[view openGLContext] flushBuffer];
end:
   glsym_evas_gl_preload_render_unlock(_evas_outbuf_make_current, ob);
}

void
evas_outbuf_reconfigure(Outbuf       *ob,
                        int           w,
                        int           h,
                        int           rot,
                        Outbuf_Depth  depth EINA_UNUSED)
{
   TRC("w,h = %i,%i", w, h);

   ob->w = w;
   ob->h = h;
   ob->rot = rot;
   evas_outbuf_use(ob);
   [[(NSOpenGLView *)ob->ns_gl_view openGLContext] flushBuffer];
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);
}

void
evas_outbuf_gl_context_use(Context_3D *ctx EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN();
}

Context_3D *
evas_outbuf_gl_context_new(Outbuf *ob EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

void *
evas_outbuf_egl_display_get(Outbuf *ob EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}
