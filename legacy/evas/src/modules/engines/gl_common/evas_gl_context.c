#include "evas_gl_private.h"

static void _evas_gl_common_viewport_set(Evas_GL_Context *gc);
static void _evas_gl_common_dither_set(Evas_GL_Context *gc);
static void _evas_gl_common_blend_set(Evas_GL_Context *gc);
static void _evas_gl_common_color_set(Evas_GL_Context *gc);
static void _evas_gl_common_texture_set(Evas_GL_Context *gc);
static void _evas_gl_common_clip_set(Evas_GL_Context *gc);
static void _evas_gl_common_buf_set(Evas_GL_Context *gc);
static void _evas_gl_common_other_set(Evas_GL_Context *gc);

static Evas_GL_Context *_evas_gl_common_context = NULL;

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
   gc->max_texture_depth = 32;
   gc->max_texture_size = 2048;
   gc->read_buf = GL_BACK;
   gc->write_buf = GL_BACK;
   gc->dither = 1;
   gc->blend = 0;
   gc->references     = 1;

   gc->change.size    = 1;
   gc->change.dither  = 1;
   gc->change.blend   = 1;
   gc->change.color   = 1;
   gc->change.texture = 1;
   gc->change.clip    = 1;
   gc->change.buf     = 1;
   gc->change.other   = 1;
   
   return gc;
}

void
evas_gl_common_context_free(Evas_GL_Context *gc)
{
   gc->references--;
   if (gc->references > 0) return;
   if (gc->yuv422p.fshad)
     {
	glDeleteObjectARB(gc->yuv422p.fshad);
     }
   if (gc->yuv422p.prog)
     {
	glDeleteObjectARB(gc->yuv422p.prog);
     }
   
   if (gc == _evas_gl_common_context) _evas_gl_common_context = NULL;
   free(gc);
}

void
evas_gl_common_context_use(Evas_GL_Context *gc)
{
   if (_evas_gl_common_context == gc) return;
   if (!gc->ext.checked)
     {
	const GLubyte *ext;

	ext = glGetString(GL_EXTENSIONS);
	if (ext)
	  {
//	     if (strstr(ext, "GL_SGIS_generate_mipmap")) gc->ext.sgis_generate_mipmap = 1;
//	     if (strstr(ext, "GL_NV_texture_rectangle")) gc->ext.nv_texture_rectangle = 1;
//	     if (strstr(ext, "GL_EXT_texture_rectangle")) gc->ext.nv_texture_rectangle = 1;
	     if (strstr(ext, "GL_ARB_texture_non_power_of_two")) gc->ext.arb_texture_non_power_of_two = 1;
	     if (strstr(ext, "GL_ARB_shader_objects") && strstr(ext, "GL_ARB_vertex_shader")
		&& strstr(ext, "GL_ARB_fragment_shader") && strstr(ext, "GL_ARB_shading_language"))
	       gc->ext.arb_program = 1;
//	     printf("GL EXT supported: GL_SGIS_generate_mipmap = %x\n", gc->ext.sgis_generate_mipmap);
//	     printf("GL EXT supported: GL_NV_texture_rectangle = %x\n", gc->ext.nv_texture_rectangle);
//	     printf("GL EXT supported: GL_ARB_texture_non_power_of_two = %x\n", gc->ext.arb_texture_non_power_of_two);
// this causes at least nvidia's drivers to go into pathological pain when
// changing textures a lot (doing video). so we wont do anything with this
// for now, but it does work.
//	     gc->ext.arb_texture_non_power_of_two = 0; printf("DISABLE GL_ARB_texture_non_power_of_two\n");
//	     gc->ext.nv_texture_rectangle = 0; printf("DISABLE GL_NV_texture_rectangle\n");
	  }
	else
	  {
//	     printf("GL EXT supported: No extensions!!!!!\n");
	  }

        if (gc->ext.arb_program)
          {
             gc->yuv422p.prog = glCreateProgramObjectARB();
             // on an nv 6600gt this is fast - but on a 5500fx its DEAD SLOW!!!!!   
             // if (!gc->ext.arb_texture_non_power_of_two) return NULL;
             /* BEGIN LEAK */
             gc->yuv422p.fshad = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
             {
                const char *code =
                   "uniform sampler2D ytex, utex, vtex;\n"
                   "void main(void) {\n"
                   "  float r, g, b, y, u, v;\n"
                   "  y = texture2D(ytex, gl_TexCoord[0].st).r;\n"
                   "  u = texture2D(utex, gl_TexCoord[0].st).r;\n"
                   "  v = texture2D(vtex, gl_TexCoord[0].st).r;\n"
                   "  y = (y - 0.0625) * 1.164;\n"
                   "  u = u - 0.5;\n"
                   "  v = v - 0.5;\n"
                   "  r = y + (1.402   * v);\n"
                   "  g = y - (0.34414 * u) - (0.71414 * v);\n"
                   "  b = y + (1.772   * u);\n"
                   "  gl_FragColor = vec4(r * gl_Color.r * gl_Color.a, g * gl_Color.g * gl_Color.a, b * gl_Color.b * gl_Color.a, gl_Color.a);\n"
                   "}\n";
                   glShaderSourceARB(gc->yuv422p.fshad, 1, &code, NULL);
             }
             glCompileShaderARB(gc->yuv422p.fshad);
             glAttachObjectARB(gc->yuv422p.prog, gc->yuv422p.fshad);
             /* END LEAK - something in the above leaks... beats me what. */
             glLinkProgramARB(gc->yuv422p.prog);
   
             glUseProgramObjectARB(gc->yuv422p.prog);
             glUniform1iARB(glGetUniformLocationARB(gc->yuv422p.prog, "ytex"), 0);
             glUniform1iARB(glGetUniformLocationARB(gc->yuv422p.prog, "utex"), 1);
             glUniform1iARB(glGetUniformLocationARB(gc->yuv422p.prog, "vtex"), 2);
             glUseProgramObjectARB(0);
          }
          
        gc->ext.checked = 1;
     }
   _evas_gl_common_context = gc;
   _evas_gl_common_viewport_set(gc);
   _evas_gl_common_dither_set(gc);
   _evas_gl_common_blend_set(gc);
   _evas_gl_common_color_set(gc);
   _evas_gl_common_texture_set(gc);
   _evas_gl_common_texture_set(gc);
   _evas_gl_common_clip_set(gc);
   _evas_gl_common_buf_set(gc);
   _evas_gl_common_other_set(gc);
}

void
evas_gl_common_context_resize(Evas_GL_Context *gc, int w, int h)
{
   //if ((gc->w == w) && (gc->h == h)) return;
   gc->change.size = 1;
   gc->w = w;
   gc->h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_viewport_set(gc);
}

void
evas_gl_common_context_color_set(Evas_GL_Context *gc, int r, int g, int b, int a)
{
   if (r < 0) r = 0;
   else if (r > 255) r = 255;
   if (g < 0) g = 0;
   else if (g > 255) g = 255;
   if (b < 0) b = 0;
   else if (b > 255) b = 255;
   if (a < 0) a = 0;
   else if (a > 255) a = 255;
   if ((gc->r == r) && (gc->g == g) && (gc->b == b) && (gc->a == a)) return;
   gc->change.color = 1;
   gc->r = r;
   gc->g = g;
   gc->b = b;
   gc->a = a;
   if (_evas_gl_common_context == gc) _evas_gl_common_color_set(gc);
}

void
evas_gl_common_context_blend_set(Evas_GL_Context *gc, int blend)
{
   if (blend == 1)
     {
	if (gc->blend) return;
	gc->change.blend = 1;
	gc->blend = 1;
	gc->blend_alpha = 0;
     }
   else if (blend == 2)
     {
	if (gc->blend_alpha) return;
	gc->change.blend = 1;
	gc->blend = 0;
	gc->blend_alpha = 1;
     }
   else
     {
	if ((!gc->blend) && (!gc->blend_alpha)) return;
	gc->change.blend = 1;
	gc->blend = 0;
	gc->blend_alpha = 0;
     }
   if (_evas_gl_common_context == gc) _evas_gl_common_blend_set(gc);
}

void
evas_gl_common_context_dither_set(Evas_GL_Context *gc, int dither)
{
   if (((dither) && (gc->dither)) || ((!dither) && (!gc->dither))) return;
   gc->change.dither = 1;
   gc->dither = dither;
   if (_evas_gl_common_context == gc) _evas_gl_common_dither_set(gc);
}

void
evas_gl_common_context_texture_set(Evas_GL_Context *gc, Evas_GL_Texture *tex, int smooth, int w, int h)
{
   if (gc->font_texture > 0)
     {
	gc->font_texture = 0;
	gc->change.texture = 1;
     }
   if (gc->texture != tex)
     {
	if (gc->texture) gc->texture->references--;
	gc->texture = tex;
	gc->change.texture = 1;
	if (tex) tex->references++;
     }
   if (tex)
     {
	if (((smooth) && (!tex->smooth)) ||
	    ((!smooth) && (tex->smooth)))
	  {
	     tex->smooth = smooth;
	     tex->changed = 1;
	  }
	tex->uw = w;
	tex->uh = h;
     }
   if (_evas_gl_common_context == gc) _evas_gl_common_texture_set(gc);
}

void
evas_gl_common_context_font_texture_set(Evas_GL_Context *gc, Evas_GL_Font_Texture *ft)
{
   if (gc->texture)
     {
	if (gc->texture) gc->texture->references--;
	gc->texture = NULL;
	gc->change.texture = 1;
     }
   if (gc->font_texture != ft->texture)
     {
	gc->font_texture = ft->texture;
	gc->font_texture_rectangle = ft->pool->rectangle;
	gc->change.texture = 1;
     }
   if (!gc->change.texture) return;
   if (_evas_gl_common_context == gc) _evas_gl_common_texture_set(gc);
}

void
evas_gl_common_context_clip_set(Evas_GL_Context *gc, int on, int x, int y, int w, int h)
{
   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if (y < 0)
     {
	h += y;
	y = 0;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if (((!on) && (!gc->clip.active)) ||
       ((on) && (gc->clip.active) &&
	(x == gc->clip.x) && (y == gc->clip.y) &&
	(w == gc->clip.w) && (h == gc->clip.h)))
     return;
   gc->change.clip = 1;
   gc->clip.active = on;
   gc->clip.x = x;
   gc->clip.y = y;
   gc->clip.w = w;
   gc->clip.h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_clip_set(gc);
}

void
evas_gl_common_context_read_buf_set(Evas_GL_Context *gc, GLenum buf)
{
   if (gc->read_buf == buf) return;
   gc->change.buf = 1;
   gc->read_buf = buf;
   if (_evas_gl_common_context == gc) _evas_gl_common_buf_set(gc);
}

void
evas_gl_common_context_write_buf_set(Evas_GL_Context *gc, GLenum buf)
{
   if (gc->write_buf == buf) return;
   gc->change.buf = 1;
   gc->write_buf = buf;
   if (_evas_gl_common_context == gc) _evas_gl_common_buf_set(gc);
}

static void
_evas_gl_common_viewport_set(Evas_GL_Context *gc)
{
   if (!gc->change.size) return;
   glViewport(0, 0, gc->w, gc->h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, gc->w, 0, gc->h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, - gc->h, 0);
   gc->change.size = 0;
}

static void
_evas_gl_common_dither_set(Evas_GL_Context *gc)
{
   if (!gc->change.dither) return;
   if (gc->dither)
     glEnable(GL_DITHER);
   else
     glDisable(GL_DITHER);
   gc->change.dither = 0;
}

static void
_evas_gl_common_blend_set(Evas_GL_Context *gc)
{
   if (!gc->change.blend) return;
   if (gc->blend_alpha)
     {
	glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     }
   else if (gc->blend)
     {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
     }
   else
     glDisable(GL_BLEND);
   gc->change.blend = 0;
}

static void
_evas_gl_common_color_set(Evas_GL_Context *gc)
{
   if (!gc->change.color) return;
   glColor4d((double)gc->r / 255.0,
	     (double)gc->g / 255.0,
	     (double)gc->b / 255.0,
	     (double)gc->a / 255.0);
   gc->change.color = 0;
}

static void
_evas_gl_common_texture_set(Evas_GL_Context *gc)
{
   if (!gc->change.texture) return;
   if (gc->font_texture > 0)
     {
	if (gc->texture_program)
	  {
	     glUseProgramObjectARB(0);
	     
	     glActiveTexture(GL_TEXTURE1);
	     glDisable(GL_TEXTURE_2D);
	     glActiveTexture(GL_TEXTURE2);
	     glDisable(GL_TEXTURE_2D);
	     gc->texture_program = 0;
	  }
	if (gc->font_texture_rectangle)
	  {
	     glActiveTexture(GL_TEXTURE0);
	     glEnable(GL_TEXTURE_2D);
	     glEnable(GL_TEXTURE_RECTANGLE_NV);
	     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gc->font_texture);
	  }
	else
	  {
	     glActiveTexture(GL_TEXTURE0);
	     if (gc->ext.nv_texture_rectangle)
	       glDisable(GL_TEXTURE_RECTANGLE_NV);
	     glEnable(GL_TEXTURE_2D);
	     glBindTexture(GL_TEXTURE_2D, gc->font_texture);
	  }
     }
   else if (gc->texture)
     {
	if (gc->texture->rectangle)
	  {
	     glActiveTexture(GL_TEXTURE0);
	     glEnable(GL_TEXTURE_2D);
	     glEnable(GL_TEXTURE_RECTANGLE_NV);
	     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gc->texture->texture);
	  }
	else
	  {
	     if (gc->ext.nv_texture_rectangle) glDisable(GL_TEXTURE_RECTANGLE_NV);
	     if ((gc->texture->prog) && 
		 (gc->texture->texture2) && (gc->texture->texture3))
	       {
		  gc->texture_program = 1;
		  glActiveTexture(GL_TEXTURE0);
		  glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, gc->texture->texture);

		  glActiveTexture(GL_TEXTURE1);
		  glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, gc->texture->texture2);
		  
		  glActiveTexture(GL_TEXTURE2);
		  glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, gc->texture->texture3);
		  glUseProgramObjectARB(gc->texture->prog);
	       }
	     else
	       {
		  if (gc->texture_program)
		    {
		       glUseProgramObjectARB(0);
		       
		       glActiveTexture(GL_TEXTURE1);
		       glDisable(GL_TEXTURE_2D);
		       glActiveTexture(GL_TEXTURE2);
		       glDisable(GL_TEXTURE_2D);
		       gc->texture_program = 0;
		    }
                  glActiveTexture(GL_TEXTURE0);
		  glBindTexture(GL_TEXTURE_2D, gc->texture->texture);
		  glEnable(GL_TEXTURE_2D);
	       }
	  }
	if (gc->texture->rectangle)
	  {
	     if (gc->texture->changed)
	       {
		  gc->texture->changed = 0;
	       }
	  }
	else
	  {
	     if (gc->texture->changed)
	       {
		  if (gc->texture->rectangle)
		    {
		       if (gc->texture->smooth)
			 {
			    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			 }
		       else
			 {
			    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			 }
		    }
		  else
		    {
		       if (gc->texture->smooth)
			 {
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//			    if (gc->texture->have_mipmaps)
//			      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//			    else
			      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			 }
		       else
			 {
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			 }
		    }
		  gc->texture->changed = 0;
	       }
	  }
     }
   else if (gc->font_texture == 0)
     {
	glDisable(GL_TEXTURE_2D);
	if (gc->ext.nv_texture_rectangle) glDisable(GL_TEXTURE_RECTANGLE_NV);
     }
   gc->change.texture = 0;
}

static void
_evas_gl_common_clip_set(Evas_GL_Context *gc)
{
   if (!gc->change.clip) return;
   /* might be faster using clip planes ??? glClipPlane() */
   if (gc->clip.active)
     {
	glEnable(GL_SCISSOR_TEST);
	glScissor(gc->clip.x,
		  gc->h - gc->clip.y - gc->clip.h,
		  gc->clip.w,
		  gc->clip.h);
     }
   else
     glDisable(GL_SCISSOR_TEST);
   gc->change.clip = 0;
}

static void
_evas_gl_common_buf_set(Evas_GL_Context *gc)
{
   if (!gc->change.buf) return;
   glDrawBuffer(gc->write_buf);
   glReadBuffer(gc->read_buf);
   gc->change.buf = 0;
}

static void
_evas_gl_common_other_set(Evas_GL_Context *gc)
{
   if (!gc->change.other) return;
   glShadeModel(GL_FLAT);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   gc->change.other = 0;
}
