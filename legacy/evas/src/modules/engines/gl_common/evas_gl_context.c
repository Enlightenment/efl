#include "evas_gl_private.h"

static void _evas_gl_common_viewport_set(Evas_GL_Context *gc);
static void shader_array_flush(Evas_GL_Context *gc);

static Evas_GL_Context *_evas_gl_common_context = NULL;

void
glerr(const char *file, const char *func, int line, const char *op)
{
   GLenum err = glGetError();
   if (err != GL_NO_ERROR)
     {
        fprintf(stderr, "GLERR: %s:%i %s(), %s: ", file, line, func, op);
        switch (err)
          {
          case GL_INVALID_ENUM:
             fprintf(stderr, "GL_INVALID_ENUM\n");
             break;
          case GL_INVALID_VALUE:
             fprintf(stderr, "GL_INVALID_VALUE\n");
             break;
          case GL_INVALID_OPERATION:
             fprintf(stderr, "GL_INVALID_OPERATION\n");
             break;
          case GL_OUT_OF_MEMORY:
             fprintf(stderr, "GL_OUT_OF_MEMORY\n");
             break;
          default:
             fprintf(stderr, "0x%x\n", err);
          }
     }
}

Evas_GL_Context *
evas_gl_common_context_new(void)
{
   Evas_GL_Context *gc;

   if (_evas_gl_common_context)
     {
	_evas_gl_common_context->references++;
	return _evas_gl_common_context;
     }
   gc = calloc(1, sizeof(Evas_GL_Context));
   if (!gc) return NULL;

   gc->references = 1;
   
   _evas_gl_common_context = gc;

   if (!gc->checked)
     {
        GLint linked;
        unsigned int pixel = 0xffffffff;
        const GLubyte *ext;
        
        ext = glGetString(GL_EXTENSIONS);
        if (ext)
          {
             fprintf(stderr, "EXT:\n%s\n", ext);
             if ((strstr(ext, "GL_ARB_texture_non_power_of_two")) ||
                 (strstr(ext, "OES_texture_npot")))
               gc->info.tex_npo2 = 1;
             if ((strstr(ext, "GL_NV_texture_rectangle")) ||
                 (strstr(ext, "GL_EXT_texture_rectangle")))
               gc->info.tex_rect = 1;
          }
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,
                      &(gc->info.max_texture_units));
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,
                      &(gc->info.max_texture_size));
        
        fprintf(stderr, "max tex size %ix%i\n"
                "max units %i\n"
                "non-power-2 tex %i\n"
                "rect tex %i\n"
                , 
                gc->info.max_texture_size, gc->info.max_texture_size,
                gc->info.max_texture_units,
                (int)gc->info.tex_npo2,
                (int)gc->info.tex_rect
                );
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_DITHER);
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
// for dest alpha        
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
#endif
        
        evas_gl_common_shader_program_init(&(gc->shader.rect), 
                                           &(shader_rect_vert_src), 
                                           &(shader_rect_frag_src));
        evas_gl_common_shader_program_init(&(gc->shader.img),
                                           &(shader_img_vert_src), 
                                           &(shader_img_frag_src));
        evas_gl_common_shader_program_init(&(gc->shader.font),
                                           &(shader_font_vert_src), 
                                           &(shader_font_frag_src));
        _evas_gl_common_viewport_set(gc);
        gc->checked = 1;
     }
   
   return gc;
}

void
evas_gl_common_context_free(Evas_GL_Context *gc)
{
   gc->references--;
   if (gc->references > 0) return;

   // free all textures...
   
   if (gc == _evas_gl_common_context) _evas_gl_common_context = NULL;
   free(gc);
}

void
evas_gl_common_context_use(Evas_GL_Context *gc)
{
   if (_evas_gl_common_context == gc) return;
//   _evas_gl_common_context = gc;
}

void
evas_gl_common_context_resize(Evas_GL_Context *gc, int w, int h)
{
   if ((gc->w == w) && (gc->h == h)) return;
   gc->change.size = 1;
   gc->w = w;
   gc->h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_viewport_set(gc);
}

#define PUSH_VERTEX(x, y, z) \
   gc->array.vertex[nv++] = x; \
   gc->array.vertex[nv++] = y; \
   gc->array.vertex[nv++] = z
#define PUSH_COLOR(r, g, b, a) \
   gc->array.color[nc++] = r; \
   gc->array.color[nc++] = g; \
   gc->array.color[nc++] = b; \
   gc->array.color[nc++] = a
#define PUSH_TEXUV(u, v) \
   gc->array.texuv[nu++] = u; \
   gc->array.texuv[nu++] = v
#define COLOR_FLOAT(r, g, b, a, fr, fg, fb, fa) \
   fr = ((GLfloat)(r)) / 255.0; \
   fg = ((GLfloat)(g)) / 255.0; \
   fb = ((GLfloat)(b)) / 255.0; \
   fa = ((GLfloat)(a)) / 255.0

static void
_evas_gl_common_context_array_alloc(Evas_GL_Context *gc)
{
   if (gc->array.num <= gc->array.alloc) return;
   gc->array.alloc += 1024;
   gc->array.vertex = realloc(gc->array.vertex,
                              gc->array.alloc * sizeof(GLint) * 3);
   gc->array.color  = realloc(gc->array.color,
                              gc->array.alloc * sizeof(GLfloat) * 4);
   gc->array.texuv  = realloc(gc->array.texuv,
                              gc->array.alloc * sizeof(GLfloat) * 2);
}

void
evas_gl_common_context_rectangle_push(Evas_GL_Context *gc, 
                                      int x, int y, int w, int h,
                                      int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   GLfloat rr, gg, bb, aa;
   Eina_Bool blend = 0;
   
   if (a < 255) blend = 1;
   if ((gc->shader.cur_tex != 0)
       || (gc->shader.cur_prog != gc->shader.rect.prog)
       || (gc->shader.blend != blend)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = 0;
        gc->shader.cur_prog = gc->shader.rect.prog;
        gc->shader.blend = blend;
     }
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);
  
   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   for (i = 0; i < 6; i++)
     {
        PUSH_TEXUV(0.0, 0.0);
     }
   COLOR_FLOAT(r, g, b, a, rr, gg, bb, aa);
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(rr, gg, bb, aa);
     }
}

void
evas_gl_common_context_image_push(Evas_GL_Context *gc,
                                  Evas_GL_Texture *tex,
                                  double sx, double sy, double sw, double sh,
                                  int x, int y, int w, int h,
                                  int r, int g, int b, int a,
                                  Eina_Bool smooth)
{
   int pnum, nv, nc, nu, nt, i;
   GLfloat rr, gg, bb, aa, tx1, tx2, ty1, ty2;
   Eina_Bool blend = 1;

   if (tex->pt->format == GL_RGB) blend = 0;
   if (a < 255) blend = 1;
   
   if ((gc->shader.cur_tex != tex->pt->texture)
       || (gc->shader.cur_prog != gc->shader.img.prog)
       || (gc->shader.smooth != smooth)
       || (gc->shader.blend != blend)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = tex->pt->texture;
        gc->shader.cur_prog = gc->shader.img.prog;
        gc->shader.smooth = smooth;
        gc->shader.blend = blend;
     }
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);

   tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
   ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
   tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
   ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
   
   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx1, ty1);
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx1, ty2);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx2, ty2);
   PUSH_TEXUV(tx1, ty2);

   COLOR_FLOAT(r, g, b, a, rr, gg, bb, aa);
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(rr, gg, bb, aa);
     }
}

void
evas_gl_common_context_font_push(Evas_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 int x, int y, int w, int h,
                                 int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   GLfloat rr, gg, bb, aa, tx1, tx2, ty1, ty2;

   if ((gc->shader.cur_tex != tex->pt->texture)
       || (gc->shader.cur_prog != gc->shader.font.prog)
       || (gc->shader.smooth != 0)
       || (gc->shader.blend != 1)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = tex->pt->texture;
        gc->shader.cur_prog = gc->shader.font.prog;
        gc->shader.smooth = 0;
        gc->shader.blend = 1;
     }
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);

   tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
   ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
   tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
   ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
   
   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx1, ty1);
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx1, ty2);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx2, ty2);
   PUSH_TEXUV(tx1, ty2);

   COLOR_FLOAT(r, g, b, a, rr, gg, bb, aa);
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(rr, gg, bb, aa);
     }
}

void
evas_gl_common_context_flush(Evas_GL_Context *gc)
{
   shader_array_flush(gc);
//   fprintf(stderr, "------------FRAME: done\n");
}

static void
shader_array_flush(Evas_GL_Context *gc)
{
   if (gc->array.num <= 0) return;

//   fprintf(stderr, "  flush array %i\n", gc->array.num);
   glUseProgram(gc->shader.cur_prog);
  
   if (gc->shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, gc->shader.cur_tex);
     }
   else
     {
        glBindTexture(GL_TEXTURE_2D, gc->shader.cur_tex);
     }
   if (gc->shader.blend)
     glEnable(GL_BLEND);
   else 
     glDisable(GL_BLEND);
   if (gc->shader.smooth)
     {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
#endif
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
#endif
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   
   glEnableVertexAttribArray(SHAD_VERTEX);
   glVertexAttribPointer(SHAD_VERTEX, 3, GL_INT, GL_FALSE, 0, gc->array.vertex);
   
   glEnableVertexAttribArray(SHAD_COLOR);
   glVertexAttribPointer(SHAD_COLOR, 4, GL_FLOAT, GL_FALSE, 0, gc->array.color);
   
   glEnableVertexAttribArray(SHAD_TEXUV);
   glVertexAttribPointer(SHAD_TEXUV, 2, GL_FLOAT, GL_FALSE, 0, gc->array.texuv);
   
   glDrawArrays(GL_TRIANGLES, 0, gc->array.num);
   
   gc->shader.cur_prog = 0;
   gc->shader.cur_tex = 0;
   
   free(gc->array.vertex);
   free(gc->array.color);
   free(gc->array.texuv);
   
   gc->array.vertex = NULL;
   gc->array.color = NULL;
   gc->array.texuv = NULL;
   
   gc->array.num = 0;
   gc->array.alloc = 0;
}

static void
matrix_ident(GLfloat *m)
{
   memset(m, 0, 16 * sizeof(GLfloat));
   m[0] = m[5] = m[10] = m[15] = 1.0;
}

static void
matrix_ortho(GLfloat *m, GLfloat l, GLfloat r, GLfloat t, GLfloat b, GLfloat near, GLfloat far)
{
   m[0] = 2.0 / (r - l);
   m[1] = m[2] = m[3] = 0.0;
   
   m[4] = 0.0;
   m[5] = 2.0 / (t - b);
   m[6] = m[7] = 0.0;
   
   m[8] = m[9] = 0.0;
   m[10] = -(2.0 / (far - near));
   m[11] = 0.0;
   
   m[12] = -((r + l)/(r - l));
   m[13] = -((t + b)/(t - b));
   m[14] = -((near + far)/(far - near));
   m[15] = 1.0;
}

static void
_evas_gl_common_viewport_set(Evas_GL_Context *gc)
{
   GLfloat proj[16];
   
   if (!gc->change.size) return;
   gc->change.size = 0;
   
   glViewport(0, 0, gc->w, gc->h);
   
   matrix_ident(proj);
   matrix_ortho(proj, 0, gc->w, 0, gc->h, -1.0, 1.0);
   
   glUseProgram(gc->shader.rect.prog);
   glUniformMatrix4fv(glGetUniformLocation(gc->shader.rect.prog, "mvp"), 1,
                      GL_FALSE, proj);
   glUseProgram(gc->shader.img.prog);
   glUniformMatrix4fv(glGetUniformLocation(gc->shader.img.prog, "mvp"), 1,
                      GL_FALSE, proj);
   glUseProgram(gc->shader.font.prog);
   glUniformMatrix4fv(glGetUniformLocation(gc->shader.font.prog, "mvp"), 1,
                      GL_FALSE, proj);
}
