#include "evas_gl_private.h"

static const GLenum rgba_fmt   = GL_RGBA;
static const GLenum rgba_ifmt  = GL_RGBA;
static const GLenum rgb_fmt    = GL_RGBA;
static const GLenum rgb_ifmt   = GL_RGB;

#ifdef GL_GLES
static const GLenum bgra_fmt   = GL_BGRA;
static const GLenum bgra_ifmt  = GL_BGRA;
static const GLenum bgr_fmt    = GL_BGRA;
static const GLenum bgr_ifmt   = GL_BGRA;
#else
static const GLenum bgra_fmt   = GL_BGRA;
static const GLenum bgra_ifmt  = GL_RGBA;
static const GLenum bgr_fmt    = GL_BGRA;
static const GLenum bgr_ifmt   = GL_RGB;
#endif

#ifdef GL_GLES
static const GLenum alpha_fmt      = GL_ALPHA;
static const GLenum alpha_ifmt     = GL_ALPHA;
#else
static const GLenum alpha_fmt      = GL_ALPHA;
static const GLenum alpha_ifmt     = GL_ALPHA4;
#endif

static const GLenum lum_fmt        = GL_LUMINANCE;
static const GLenum lum_ifmt       = GL_LUMINANCE;
static const GLenum lum_alpha_fmt  = GL_LUMINANCE_ALPHA;
static const GLenum lum_alpha_ifmt = GL_LUMINANCE_ALPHA;
static const GLenum rgba8_ifmt     = GL_RGBA;
static const GLenum rgba8_fmt      = GL_BGRA;

/* FIXME: RGB8_ETC2 is a superset of ETC1,
 * but is GL_ETC1_RGB8_OES supported whenever GL_COMPRESSED_RGB8_ETC2 is?
 */
static const GLenum etc1_fmt       = GL_ETC1_RGB8_OES;
static const GLenum etc2_rgb_fmt   = GL_COMPRESSED_RGB8_ETC2;
static const GLenum etc2_rgba_fmt  = GL_COMPRESSED_RGBA8_ETC2_EAC;

static struct {
   struct {
      int num, pix;
   } c, a, v, r, n, d;
} texinfo = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

static const struct {
   Eina_Bool alpha;
   Eina_Bool bgra;

   Evas_Colorspace cspace;

   const GLenum *intformat;
   const GLenum *format;
} matching_format[] = {
  { EINA_TRUE, EINA_TRUE, EVAS_COLORSPACE_ARGB8888, &bgra_ifmt, &bgra_fmt },
  { EINA_TRUE, EINA_FALSE, EVAS_COLORSPACE_ARGB8888, &rgba_ifmt, &rgba_fmt },
  { EINA_FALSE, EINA_TRUE, EVAS_COLORSPACE_ARGB8888, &bgr_ifmt, &bgr_fmt },
#ifdef GL_GLES
  { EINA_FALSE, EINA_FALSE, EVAS_COLORSPACE_ARGB8888, &rgba_ifmt, &rgba_fmt },
#else
  { EINA_FALSE, EINA_FALSE, EVAS_COLORSPACE_ARGB8888, &rgb_ifmt, &rgb_fmt },
#endif
  { EINA_FALSE, EINA_FALSE, EVAS_COLORSPACE_GRY8, &lum_fmt, &lum_ifmt },
  { EINA_FALSE, EINA_TRUE, EVAS_COLORSPACE_GRY8, &lum_fmt, &lum_ifmt },
  { EINA_TRUE, EINA_FALSE, EVAS_COLORSPACE_AGRY88, &lum_alpha_fmt, &lum_alpha_ifmt },
  { EINA_TRUE, EINA_TRUE, EVAS_COLORSPACE_AGRY88, &lum_alpha_fmt, &lum_alpha_ifmt },
  { EINA_FALSE, EINA_FALSE, EVAS_COLORSPACE_ETC1, &etc1_fmt, &etc1_fmt },
  { EINA_FALSE, EINA_TRUE, EVAS_COLORSPACE_ETC1, &etc1_fmt, &etc1_fmt },
  { EINA_FALSE, EINA_FALSE, EVAS_COLORSPACE_RGB8_ETC2, &etc2_rgb_fmt, &etc2_rgb_fmt },
  { EINA_FALSE, EINA_TRUE, EVAS_COLORSPACE_RGB8_ETC2, &etc2_rgb_fmt, &etc2_rgb_fmt },
  { EINA_TRUE, EINA_FALSE, EVAS_COLORSPACE_RGBA8_ETC2_EAC, &etc2_rgba_fmt, &etc2_rgba_fmt },
  { EINA_TRUE, EINA_TRUE, EVAS_COLORSPACE_RGBA8_ETC2_EAC, &etc2_rgba_fmt, &etc2_rgba_fmt },
  // images marked as no alpha but format supports it (RGBA8_ETC2_EAC):
  { EINA_FALSE, EINA_FALSE, EVAS_COLORSPACE_RGBA8_ETC2_EAC, &etc2_rgba_fmt, &etc2_rgba_fmt },
  { EINA_FALSE, EINA_TRUE, EVAS_COLORSPACE_RGBA8_ETC2_EAC, &etc2_rgba_fmt, &etc2_rgba_fmt }
};

static const GLenum matching_rgba[] = { GL_RGBA4, GL_RGBA8, GL_RGBA12, GL_RGBA16, 0x0 };
static const GLenum matching_alpha[] = { GL_ALPHA4, GL_ALPHA8, GL_ALPHA12, GL_ALPHA16, 0x0 };
static const GLenum matching_luminance[] = { GL_LUMINANCE4, GL_LUMINANCE8, GL_LUMINANCE12, GL_LUMINANCE16, 0x0 };
static const GLenum matching_luminance_alpha[] = { GL_LUMINANCE4_ALPHA4, GL_LUMINANCE8_ALPHA8, GL_LUMINANCE12_ALPHA12, GL_LUMINANCE16_ALPHA16, 0x0 };

static const struct {
   GLenum master;
   const GLenum *matching;
} matching_fmt[] = {
  { GL_RGBA, matching_rgba },
  { GL_ALPHA, matching_alpha },
  { GL_LUMINANCE, matching_luminance },
  { GL_LUMINANCE_ALPHA, matching_luminance_alpha }
};

static Eina_Bool
_evas_gl_texture_match(GLenum intfmt, GLenum intfmtret)
{
   unsigned int i;

   if (intfmt == intfmtret) return EINA_TRUE;

   for (i = 0; i < sizeof (matching_fmt) / sizeof (matching_fmt[0]); i++)
     if (matching_fmt[i].master == intfmt)
       {
          unsigned int j;

          for (j = 0; matching_fmt[i].matching[j] != 0x0; j++)
            if (matching_fmt[i].matching[j] == intfmtret)
              return EINA_TRUE;
          return EINA_FALSE;
       }

   return EINA_FALSE;
}

static int
_evas_gl_texture_search_format(Eina_Bool alpha, Eina_Bool bgra, Evas_Colorspace cspace)
{
   unsigned int i;

   alpha = !!alpha;
   bgra = !!bgra;

   for (i = 0; i < sizeof (matching_format) / sizeof (matching_format[0]); ++i)
     if (matching_format[i].alpha == alpha &&
         matching_format[i].bgra == bgra &&
         matching_format[i].cspace == cspace)
       return i;

   abort();
   return 0;
}

static void
_print_tex_count(void)
{
   if (getenv("EVAS_GL_MEMINFO"))
     {
        fprintf(stderr,
                "T: c:%i/%ik | a:%i/%ik | v:%i/%ik | r:%i/%ik | n:%i/%ik | d:%i/%ik\n",
                texinfo.c.num, (texinfo.c.pix * 4) / 1024,
                texinfo.a.num, (texinfo.a.pix    ) / 1024,
                texinfo.v.num, (texinfo.v.pix    ) / 1024,
                texinfo.r.num, (texinfo.r.pix * 4) / 1024,
                texinfo.n.num, (texinfo.n.pix * 4) / 1024,
                texinfo.d.num, (texinfo.d.pix * 4) / 1024
               );
     }
}

static int
_nearest_pow2(int num)
{
   unsigned int n = num - 1;
   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;
   return n + 1;
}

static void
_tex_adjust(Evas_Engine_GL_Context *gc, int *w, int *h)
{
   if (gc->shared->info.tex_npo2) return;
   /*if (gc->shared->info.tex_rect) return;*/
   *w = _nearest_pow2(*w);
   *h = _nearest_pow2(*h);
}

static int
_tex_round_slot(Evas_Engine_GL_Context *gc, int h)
{
   if (!gc->shared->info.tex_npo2)
     h = _nearest_pow2(h);
   return (h + gc->shared->info.tune.atlas.slot_size - 1) /
     gc->shared->info.tune.atlas.slot_size;
}

static int
_tex_format_index(GLuint format)
{
   switch (format)
     {
      case GL_RGBA:
      case GL_RGBA4:
      case GL_RGBA8:
      case GL_RGBA12:
      case GL_RGBA16:
      case GL_BGRA:
        return 0;
      case GL_RGB:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
        return 1;
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
        return 2;
      case GL_LUMINANCE: // never used in atlas
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
        return 3;
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
         return 4;
      default:
        return 0;
     }
   return 0;
}

static Eina_Bool
_tex_2d(Evas_Engine_GL_Context *gc, int intfmt, int w, int h, int fmt, int type)
{
   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size))
     {
        ERR("Fail tex too big %ix%i", w, h);
        return EINA_FALSE;
     }
   glTexImage2D(GL_TEXTURE_2D, 0, intfmt, w, h, 0, fmt, type, NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#ifdef GL_TEXTURE_INTERNAL_FORMAT
# ifdef GL_GLES
# else   
// this is not in opengles!!! hrrrm   
//   if (glGetTexLevelParameteriv) // in case of weak symbols?
     {
        int intfmtret = -1;
        
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                 GL_TEXTURE_INTERNAL_FORMAT, &intfmtret);
        if (!_evas_gl_texture_match(intfmt, intfmtret))
          {
             ERR("Fail tex alloc %ix%i", w, h);
             //        XXX send async err to evas
             return EINA_FALSE;
          }
     }
//   else
//     {
//        ERR("GL_TEXTURE_INTERNAL_FORMAT defined but no symbol loaded.");
//     }
# endif   
#endif
   return EINA_TRUE;
}

static Evas_GL_Texture *
evas_gl_common_texture_alloc(Evas_Engine_GL_Context *gc,
			     Evas_Coord w, Evas_Coord h,
                             Eina_Bool alpha)
{
   Evas_GL_Texture *tex;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;

   tex->gc = gc;
   tex->references = 1;
   tex->alpha = alpha;
   tex->w = w;
   tex->h = h;

   return tex;
}

static void
evas_gl_common_texture_light_free(Evas_GL_Texture *tex)
{
   free(tex);
}

static Evas_GL_Texture_Pool *
_pool_tex_new(Evas_Engine_GL_Context *gc, int w, int h, GLenum intformat, GLenum format)
{
   Evas_GL_Texture_Pool *pt;
   Eina_Bool ok, no_rounding = EINA_FALSE;

   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size))
     {
        ERR("Fail tex too big %ix%i", w, h);
        return NULL;
     }
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;

   if (!gc->shared->info.etc1_subimage && (intformat == etc1_fmt))
     no_rounding = EINA_TRUE;

   if (!no_rounding)
     {
        h = _tex_round_slot(gc, h) * gc->shared->info.tune.atlas.slot_size;
        _tex_adjust(gc, &w, &h);
     }
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->references = 0;

   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = _tex_2d(gc, pt->intformat, w, h, pt->format, pt->dataformat);
   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        glDeleteTextures(1, &(pt->texture));
        free(pt);
        return NULL;
     }

   if (format == alpha_fmt)
     {
        texinfo.a.num++;
        texinfo.a.pix += pt->w * pt->h;
     }
   else if (format == lum_fmt)
     {
        texinfo.v.num++;
        texinfo.v.pix += pt->w * pt->h;
     }
   else
     {
        texinfo.c.num++;
        texinfo.c.pix += pt->w * pt->h;
     }
   _print_tex_count();
   return pt;
}

static Evas_GL_Texture_Alloca *
_pool_tex_define(Evas_GL_Texture_Pool *pt, int lastx, int w, int *u, Eina_List *l)
{
   Evas_GL_Texture_Alloca *napt;

   *u = lastx;

   napt = malloc(sizeof (Evas_GL_Texture_Alloca));
   if (!napt) return NULL;

   napt->tex = NULL;
   napt->x = lastx;
   napt->w = w;

   if (l == NULL)
     pt->allocations = eina_list_append(pt->allocations, napt);
   else
     pt->allocations = eina_list_prepend_relative_list(pt->allocations, napt, l);

   return napt;
}

static Evas_GL_Texture_Alloca *
_pool_tex_alloc(Evas_GL_Texture_Pool *pt, int w, int h EINA_UNUSED, int *u, int *v)
{
   Evas_GL_Texture_Alloca *apt;
   Eina_List *l;
   int lastx = 0;

   *v = 0;

   EINA_LIST_FOREACH(pt->allocations, l, apt)
     {
        if (apt->x - lastx >= w)
          return _pool_tex_define(pt, lastx, w, u, l);

        lastx = apt->x + apt->w;
     }

   if (pt->w - lastx >= w)
     return _pool_tex_define(pt, lastx, w, u, NULL);

   return NULL;
}

static Evas_GL_Texture_Pool *
_pool_tex_find(Evas_Engine_GL_Context *gc, int w, int h,
               GLenum intformat, GLenum format, int *u, int *v,
               Evas_GL_Texture_Alloca **apt, int atlas_w)
{
   Evas_GL_Texture_Pool *pt = NULL;
   Eina_List *l;
   int th, th2;

   if (atlas_w > gc->shared->info.max_texture_size)
      atlas_w = gc->shared->info.max_texture_size;
   if ((w > gc->shared->info.tune.atlas.max_w) ||
       (h > gc->shared->info.tune.atlas.max_h) ||
       (!gc->shared->info.etc1_subimage && (intformat == etc1_fmt)))
     {
        pt = _pool_tex_new(gc, w, h, intformat, format);
        if (!pt) return NULL;
        gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, pt);
        pt->slot = -1;
        pt->fslot = -1;
        pt->whole = 1;
        *apt = _pool_tex_alloc(pt, w, h, u, v);
        return pt;
     }

   th = _tex_round_slot(gc, h);
   th2 = _tex_format_index(intformat);
   EINA_LIST_FOREACH(gc->shared->tex.atlas[th][th2], l, pt)
     {
        if ((*apt = _pool_tex_alloc(pt, w, h, u, v)) != NULL)
          {
             gc->shared->tex.atlas[th][th2] =
               eina_list_remove_list(gc->shared->tex.atlas[th][th2], l);
             gc->shared->tex.atlas[th][th2] =
               eina_list_prepend(gc->shared->tex.atlas[th][th2], pt);
             return pt;
          }
     }

   pt = _pool_tex_new(gc, atlas_w, h, intformat, format);
   if (!pt) return NULL;
   gc->shared->tex.atlas[th][th2] =
     eina_list_prepend(gc->shared->tex.atlas[th][th2], pt);
   pt->slot = th;
   pt->fslot = th2;

   *apt = _pool_tex_alloc(pt, w, h, u, v);

   return pt;
}

Evas_GL_Texture *
evas_gl_common_texture_new(Evas_Engine_GL_Context *gc, RGBA_Image *im)
{
   Evas_GL_Texture *tex;
   GLsizei w, h;
   int u = 0, v = 0, yoffset = 0;
   int lformat;

   tex = evas_gl_common_texture_alloc(gc, im->cache_entry.w, im->cache_entry.h, im->cache_entry.flags.alpha);
   if (!tex) return NULL;

#define TEX_HREP 1
#define TEX_VREP 1

   lformat = _evas_gl_texture_search_format(im->cache_entry.flags.alpha, gc->shared->info.bgra, im->cache_entry.space);
   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
        // Add border to avoid artifacts
        w = im->cache_entry.w + 2;
        h = im->cache_entry.h + 2;
        yoffset = 1;

        // Adjust w and h for ETC1/2 formats (multiple of 4 pixels on both axes)
        w = ((w >> 2) + (w & 0x3 ? 1 : 0)) << 2;
        h = ((h >> 2) + (h & 0x3 ? 1 : 0)) << 2;
        break;

     default:
        /* This need to be adjusted if we do something else than strip allocation */
        w = im->cache_entry.w + TEX_HREP + 2; /* one pixel stop gap and two pixels for the border */
        h = im->cache_entry.h + TEX_VREP; /* only one added border for security down */
     }

   tex->pt = _pool_tex_find(gc, w, h,
                            *matching_format[lformat].intformat,
                            *matching_format[lformat].format,
                            &u, &v, &tex->apt,
                            gc->shared->info.tune.atlas.max_alloc_size);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->apt->tex = tex;
   tex->x = u + 1;
   tex->y = v + yoffset;

   tex->pt->references++;
   evas_gl_common_texture_update(tex, im);

   return tex;
}

static Evas_GL_Texture_Pool *
_pool_tex_render_new(Evas_Engine_GL_Context *gc, int w, int h, int intformat, int format)
{
   Evas_GL_Texture_Pool *pt;
   int fnum;
   Eina_Bool ok;

   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size))
     {
        ERR("Fail tex too big %ix%i", w, h);
        return NULL;
     }
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   _tex_adjust(gc, &w, &h);
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->render = 1;
   pt->references = 0;
#ifdef GL_GLES
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif
# ifndef GL_COLOR_ATTACHMENT0
#  define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
# endif
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif
# ifndef GL_COLOR_ATTACHMENT0
#  define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
# endif
#endif
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fnum);
   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = _tex_2d(gc, pt->intformat, w, h, pt->format, pt->dataformat);

   glsym_glGenFramebuffers(1, &(pt->fb));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glsym_glBindFramebuffer(GL_FRAMEBUFFER, pt->fb);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glsym_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pt->texture, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glsym_glBindFramebuffer(GL_FRAMEBUFFER, fnum);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   
   if (!ok)
     {
        glDeleteTextures(1, &(pt->texture));
        free(pt);
        return NULL;
     }
   
   texinfo.r.num++;
   texinfo.r.pix += pt->w * pt->h;
   _print_tex_count();
   return pt;
}

static Evas_GL_Texture_Pool *
_pool_tex_native_new(Evas_Engine_GL_Context *gc, int w, int h, int intformat, int format, Evas_GL_Image *im)
{
   Evas_GL_Texture_Pool *pt;

   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size))
     {
        ERR("Fail tex too big %ix%i", w, h);
        return NULL;
     }
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   pt->gc = gc;
#ifdef GL_TEXTURE_RECTANGLE_ARB
   if (im->native.target == GL_TEXTURE_RECTANGLE_ARB)
     {
        printf("REEEEEEEEECT\n");
        pt->w = w;
        pt->h = h;
     }
   else
#endif
     {
        // FIXME: handle po2 only textures
        pt->w = w;
        pt->h = h;
     }
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->references = 0;
   pt->native = 1;
   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(im->native.target, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

#ifdef GL_GLES
#else
   if (im->native.loose)
     {
        if (im->native.func.bind)
          im->native.func.bind(im->native.func.data, im);
     }
#endif

   glTexParameteri(im->native.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(im->native.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(im->native.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(im->native.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(im->native.target, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(im->native.target, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   
   texinfo.n.num++;
   texinfo.n.pix += pt->w * pt->h;
   _print_tex_count();
   return pt;
}

static Evas_GL_Texture_Pool *
_pool_tex_dynamic_new(Evas_Engine_GL_Context *gc, int w, int h, int intformat, int format)
{
   Evas_GL_Texture_Pool *pt = NULL;

#ifdef GL_GLES
   int fmt; // EGL_MAP_GL_TEXTURE_RGBA_SEC or EGL_MAP_GL_TEXTURE_RGB_SEC or bust
   int pixtype; // EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC or bust
   int attr[] =
     {
        EGL_MAP_GL_TEXTURE_WIDTH_SEC, 32,
        EGL_MAP_GL_TEXTURE_HEIGHT_SEC, 32,
        EGL_MAP_GL_TEXTURE_FORMAT_SEC, EGL_MAP_GL_TEXTURE_RGBA_SEC,
        EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC, EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC,
        EGL_NONE
     };
   void *egldisplay;

   if (intformat != format) return NULL;

   switch (intformat)
     {
#ifdef EGL_MAP_GL_TEXTURE_LUMINANCE_SEC
     case GL_LUMINANCE: attr[5] = EGL_MAP_GL_TEXTURE_LUMINANCE_SEC; break;
#endif
#ifdef EGL_MAP_GL_TEXTURE_LUMINANCE_ALPHA_SEC
     case GL_LUMINANCE_ALPHA: attr[5] = EGL_MAP_GL_TEXTURE_LUMINANCE_ALPHA_SEC; break;
#endif
     case GL_RGBA: attr[5] = EGL_MAP_GL_TEXTURE_RGBA_SEC; break;
     case GL_BGRA: attr[5] = EGL_MAP_GL_TEXTURE_BGRA_SEC; break;
     default: fprintf(stderr, "unknown format\n"); return NULL;
     }

   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   h = _tex_round_slot(gc, h) * gc->shared->info.tune.atlas.slot_size;
   _tex_adjust(gc, &w, &h);
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->render = 1;
   pt->references = 0;
   texinfo.d.num++;
   texinfo.d.pix += pt->w * pt->h;

   _print_tex_count();

   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   egldisplay = pt->gc->egldisp;

   attr[1] = pt->w;
   attr[3] = pt->h;

   // FIXME: seems a bit slower than i'd like - maybe too many flushes?
   // FIXME: YCbCr no support as yet
   pt->dyn.img = secsym_eglCreateImage(egldisplay,
                                       EGL_NO_CONTEXT,
                                       EGL_MAP_GL_TEXTURE_2D_SEC,
                                       0, attr);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!pt->dyn.img)
     {
        glBindTexture(GL_TEXTURE_2D, 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glDeleteTextures(1, &(pt->texture));
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        free(pt);
        return NULL;
     }
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_WIDTH_SEC,
                                   &(pt->dyn.w)) != EGL_TRUE) goto error;
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_HEIGHT_SEC,
                                   &(pt->dyn.h)) != EGL_TRUE) goto error;
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_STRIDE_IN_BYTES_SEC,
                                   &(pt->dyn.stride)) != EGL_TRUE) goto error;
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_FORMAT_SEC,
                                   &(fmt)) != EGL_TRUE) goto error;

   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC,
                                   &(pixtype)) != EGL_TRUE) goto error;

   if (pixtype != EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC) goto error;

   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#else
   if (gc + w + h + intformat + format) return pt;
#endif
   return pt;

/* ERROR HANDLING */
#ifdef GL_GLES
error:
  secsym_eglDestroyImage(egldisplay, pt->dyn.img);
  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
  pt->dyn.img = NULL;
  glBindTexture(GL_TEXTURE_2D, 0);
  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
  glDeleteTextures(1, &(pt->texture));
  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
  free(pt);
  return NULL;
#endif   
}

void
evas_gl_texture_pool_empty(Evas_GL_Texture_Pool *pt)
{
   Evas_GL_Texture_Alloca *apt;

   if (!pt->gc) return;

   if (pt->format == alpha_fmt)
      {
         texinfo.a.num--;
         texinfo.a.pix -= pt->w * pt->h;
      }
   else if (pt->format == lum_fmt)
      {
         texinfo.v.num--;
         texinfo.v.pix -= pt->w * pt->h;
      }
   else if (pt->dyn.img)
      {
         texinfo.d.num--;
         texinfo.d.pix -= pt->w * pt->h;
      }
   else if (pt->render)
      {
         texinfo.r.num--;
         texinfo.r.pix -= pt->w * pt->h;
      }
   else if (pt->native)
      {
         texinfo.n.num--;
         texinfo.n.pix -= pt->w * pt->h;
      }
   else
      {
         texinfo.c.num--;
         texinfo.c.pix -= pt->w * pt->h;
      }

   _print_tex_count();

#ifdef GL_GLES
   if (pt->dyn.img)
     {
        if (pt->dyn.checked_out > 0)
          secsym_eglUnmapImageSEC(pt->gc->egldisp, pt->dyn.img, EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);
        secsym_eglDestroyImage(pt->gc->egldisp, pt->dyn.img);
        pt->dyn.img = NULL;
        pt->dyn.data = NULL;
        pt->dyn.w = 0;
        pt->dyn.h = 0;
        pt->dyn.stride = 0;
        pt->dyn.checked_out = 0;
     }
#endif

   glDeleteTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (pt->gc->pipe[0].shader.cur_tex == pt->texture)
     pt->gc->pipe[0].shader.cur_tex = 0;
   if (pt->fb)
     {
        glsym_glDeleteFramebuffers(1, &(pt->fb));
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        pt->fb = 0;
     }

   EINA_LIST_FREE(pt->allocations, apt)
     free(apt);
   pt->texture = 0;
   pt->gc = NULL;
   pt->w = 0;
   pt->h = 0;
}

void
pt_unref(Evas_GL_Texture_Pool *pt)
{
   if (!pt) return;
   if (!pt->gc) return;
   pt->references--;
   if (pt->references != 0) return;

   if (!((pt->render) || (pt->native)))
     {
        if (pt->whole)
           pt->gc->shared->tex.whole =
           eina_list_remove(pt->gc->shared->tex.whole, pt);
        else
           pt->gc->shared->tex.atlas [pt->slot][pt->fslot] =
           eina_list_remove(pt->gc->shared->tex.atlas[pt->slot][pt->fslot], pt);
     }
   evas_gl_texture_pool_empty(pt);
   free(pt);
}

static void
pt_link(Evas_Engine_GL_Context *gc, Evas_GL_Texture_Pool *pt)
{
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, pt);
   pt->slot = -1;
   pt->fslot = -1;
   pt->whole = 1;
   pt->references++;
}

Evas_GL_Texture *
evas_gl_common_texture_native_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_GL_Image *im)
{
   Evas_GL_Texture *tex;
   int lformat;

   tex = evas_gl_common_texture_alloc(gc, w, h, alpha);
   if (!tex) return NULL;

   lformat = _evas_gl_texture_search_format(alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   tex->pt = _pool_tex_native_new(gc, w, h,
                                  *matching_format[lformat].intformat,
                                  *matching_format[lformat].format,
                                  im);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->pt->references++;
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_render_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha)
{
   Evas_GL_Texture *tex;
   int lformat;

   tex = evas_gl_common_texture_alloc(gc, w, h, alpha);
   if (!tex) return NULL;

   lformat = _evas_gl_texture_search_format(alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   tex->pt = _pool_tex_render_new(gc, w, h,
                                  *matching_format[lformat].intformat,
                                  *matching_format[lformat].format);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->pt->references++;
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_dynamic_new(Evas_Engine_GL_Context *gc, Evas_GL_Image *im)
{
   Evas_GL_Texture *tex;
   int lformat;

   tex = evas_gl_common_texture_alloc(gc, im->w, im->h, im->alpha);
   if (!tex) return NULL;

   lformat = _evas_gl_texture_search_format(tex->alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   tex->pt = _pool_tex_dynamic_new(gc, tex->w, tex->h,
                                   *matching_format[lformat].intformat,
                                   *matching_format[lformat].format);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->pt->references++;
   return tex;
}

void
evas_gl_common_texture_upload(Evas_GL_Texture *tex, RGBA_Image *im, unsigned int bytes_count)
{
   GLuint fmt;

   fmt = tex->pt->format;
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

//   printf("tex upload %ix%i\n", im->cache_entry.w, im->cache_entry.h);
   //  +-+
   //  +-+
   //
   _tex_sub_2d(tex->gc, tex->x, tex->y,
               im->cache_entry.w, im->cache_entry.h,
               fmt, tex->pt->dataformat,
               im->image.data);
   //  xxx
   //  xxx
   //  ---
   _tex_sub_2d(tex->gc, tex->x, tex->y + im->cache_entry.h,
               im->cache_entry.w, 1,
               fmt, tex->pt->dataformat,
               (unsigned char *) im->image.data + (((im->cache_entry.h - 1) * im->cache_entry.w)) * bytes_count);
   //  xxx
   //  xxx
   // o
   _tex_sub_2d(tex->gc, tex->x - 1, tex->y + im->cache_entry.h,
               1, 1,
               fmt, tex->pt->dataformat,
               (unsigned char *) im->image.data + (((im->cache_entry.h - 1) * im->cache_entry.w)) * bytes_count);
   //  xxx
   //  xxx
   //     o
   _tex_sub_2d(tex->gc, tex->x + im->cache_entry.w, tex->y + im->cache_entry.h,
               1, 1,
               fmt, tex->pt->dataformat,
               (unsigned char *) im->image.data + (((im->cache_entry.h - 1) * im->cache_entry.w) + (im->cache_entry.w - 1)) * bytes_count);
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, im->cache_entry.w);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        // |xxx
        // |xxx
        //
        _tex_sub_2d(tex->gc, tex->x - 1, tex->y,
                    1, im->cache_entry.h,
                    fmt, tex->pt->dataformat,
                    im->image.data);
        //  xxx|
        //  xxx|
        //
        _tex_sub_2d(tex->gc, tex->x + im->cache_entry.w, tex->y,
                    1, im->cache_entry.h,
                    fmt, tex->pt->dataformat,
                    (unsigned char *) im->image.data + (im->cache_entry.w - 1) * bytes_count);
     }
   else
     {
        DATA8 *tpix, *ps, *pd;
        int i;

        tpix = alloca(im->cache_entry.h * bytes_count);
        pd = tpix;
        ps = im->image.data8;
        for (i = 0; i < (int)im->cache_entry.h; i++)
          {
             memcpy(pd, ps, bytes_count);
             pd += bytes_count;
             ps += im->cache_entry.w * bytes_count;
          }
        // |xxx
        // |xxx
        //
        _tex_sub_2d(tex->gc, tex->x - 1, tex->y,
                    1, im->cache_entry.h,
                    fmt, tex->pt->dataformat,
                    tpix);
        pd = tpix;
        ps = im->image.data8 + (im->cache_entry.w - 1) * bytes_count;
        for (i = 0; i < (int)im->cache_entry.h; i++)
          {
             memcpy(pd, ps, bytes_count);
             pd += bytes_count;
             ps += im->cache_entry.w * bytes_count;
          }
        //  xxx|
        //  xxx|
        //
        _tex_sub_2d(tex->gc, tex->x + im->cache_entry.w, tex->y,
                    1, im->cache_entry.h,
                    fmt, tex->pt->dataformat,
                    tpix);
     }
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

void
evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im)
{
   unsigned int bytes_count;

   if (tex->alpha != im->cache_entry.flags.alpha)
     {
        int lformat;

        tex->pt->allocations = eina_list_remove(tex->pt->allocations, tex->apt);
        pt_unref(tex->pt);
        tex->alpha = im->cache_entry.flags.alpha;

        lformat = _evas_gl_texture_search_format(tex->alpha, tex->gc->shared->info.bgra, im->cache_entry.space);
        // FIXME: why a 'render' new here ??? Should already have been allocated, quite a weird path.
        tex->pt = _pool_tex_render_new(tex->gc, tex->w, tex->h,
                                       *matching_format[lformat].intformat,
                                       *matching_format[lformat].format);
     }
   // If image was preloaded then we need a ptt
   if (!tex->pt) return;
   if (!im->image.data) return;

   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888: bytes_count = 4; break;
      case EVAS_COLORSPACE_GRY8: bytes_count = 1; break;
      case EVAS_COLORSPACE_AGRY88: bytes_count = 2; break;
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
        {
           /*
             ETC1/2 can't be scaled down on the fly and interpolated, like it is
             required for preloading, so we don't take that path. Also as the content
             already have duplicated border and we use a specific function to
             upload the compressed data, there is no need to use the normal path at
             all.
           */
           GLsizei width, height;
           GLint x, y;
           int etc_block_size = 8;

           if (im->cache_entry.space == EVAS_COLORSPACE_RGBA8_ETC2_EAC)
             etc_block_size = 16;

           x = tex->x - 1;
           y = tex->y - 1;
           width = im->cache_entry.w + 2;
           height = im->cache_entry.h + 2;
           width = ((width >> 2) + (width & 0x3 ? 1 : 0)) << 2;
           height = ((height >> 2) + (height & 0x3 ? 1 : 0)) << 2;

           glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
           GLERR(__FUNCTION__, __FILE__, __LINE__, "");

           if ((tex->gc->shared->info.etc1_subimage ||
               (im->cache_entry.space != EVAS_COLORSPACE_ETC1))
               && (tex->pt->w != width || tex->pt->h != height))
             {
                int glerr;
                glerr = glGetError();

                if (!tex->pt->comptex_ready)
                  {
                     GLsizei tw, th;
                     tw = ((tex->pt->w >> 2) + (tex->pt->w & 0x3 ? 1 : 0)) << 2;
                     th = ((tex->pt->h >> 2) + (tex->pt->h & 0x3 ? 1 : 0)) << 2;
                     glCompressedTexImage2D(GL_TEXTURE_2D, 0, tex->pt->format,
                                            tw, th, 0,
                                            ((tw * th) >> 4) * etc_block_size,
                                            NULL);
                     GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                     tex->pt->comptex_ready = EINA_TRUE;
                  }

                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
                                          x, y, width, height,
                                          tex->pt->format,
                                          ((width * height) >> 4) * etc_block_size,
                                          im->image.data);

                glerr = glGetError();
                if (glerr != GL_NO_ERROR)
                  {
                     ERR("glCompressedTexSubImage2D failed with ETC1/2: %d", glerr);

                     // FIXME: Changing settings on the fly.
                     // The first texture will be black.
                     // How to fallback? We need a whole texture now.
                     if (im->cache_entry.space == EVAS_COLORSPACE_ETC1)
                       tex->gc->shared->info.etc1_subimage = EINA_FALSE;
                  }
             }
           else
             {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, tex->pt->format,
                                       width, height, 0,
                                       ((width * height) >> 4) * etc_block_size,
                                       im->image.data);
                GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             }

           if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
             {
                glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
                GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             }

           return;
        }
      default:
         ERR("Don't know how to upload texture in colorspace %i.", im->cache_entry.space);
         return;
     }

   // if preloaded, then async push it in after uploading a miniature of it
   if (im->cache_entry.flags.preload_done && tex->w > 2 * EVAS_GL_TILE_SIZE && tex->h > 2 * EVAS_GL_TILE_SIZE)
     {
        Evas_GL_Texture_Async_Preload *async;
        unsigned char *in;
        unsigned char *out;
        GLuint fmt;
        float xstep, ystep;
        float x, y;
        int i, j;
        int lformat;
        int u, v;

        if (tex->ptt) return ;

        out = alloca(bytes_count *  EVAS_GL_TILE_SIZE * EVAS_GL_TILE_SIZE);
        xstep = (float)tex->w / (EVAS_GL_TILE_SIZE - 2);
        ystep = (float)tex->h / (EVAS_GL_TILE_SIZE - 1);
        in = im->image.data8;

        for (y = 0, j = 0; j < EVAS_GL_TILE_SIZE - 1; y += ystep, j++)
          {
             memcpy(&out[j * EVAS_GL_TILE_SIZE * bytes_count],
                    &in[(int)y * im->cache_entry.w * bytes_count],
                    bytes_count);
             for (x = 0, i = 1; i < EVAS_GL_TILE_SIZE - 1; x += xstep, i++)
               memcpy(&out[(j * EVAS_GL_TILE_SIZE + i) * bytes_count],
                      &in[((int)y * im->cache_entry.w + (int)x) * bytes_count],
                      bytes_count);
             memcpy(&out[(j * EVAS_GL_TILE_SIZE + i) * bytes_count],
                    &in[((int)y * im->cache_entry.w + (int)(x - xstep)) * bytes_count],
                    bytes_count);
          }

        memcpy(&out[(j * EVAS_GL_TILE_SIZE) * bytes_count],
               &out[((j - 1) * EVAS_GL_TILE_SIZE) * bytes_count],
               EVAS_GL_TILE_SIZE * bytes_count);

        // out is a miniature of the texture, upload that now and schedule the data for later.

        // Creating the mini picture texture
        lformat = _evas_gl_texture_search_format(tex->alpha, tex->gc->shared->info.bgra, im->cache_entry.space);
        tex->ptt = _pool_tex_find(tex->gc, EVAS_GL_TILE_SIZE, EVAS_GL_TILE_SIZE,
                                  *matching_format[lformat].intformat,
                                  *matching_format[lformat].format,
                                  &u, &v, &tex->aptt,
                                  tex->gc->shared->info.tune.atlas.max_alloc_size);
        if (!tex->ptt)
          goto upload;
        tex->aptt->tex = tex;

        tex->tx = u + 1;
        tex->ty = v;
        tex->ptt->references++;

        // Bind and upload ! Vooom !
        fmt = tex->ptt->format;
        glBindTexture(GL_TEXTURE_2D, tex->ptt->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (tex->gc->shared->info.unpack_row_length)
          {
             glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");

        _tex_sub_2d(tex->gc, u, tex->ty, EVAS_GL_TILE_SIZE, EVAS_GL_TILE_SIZE, fmt, tex->ptt->dataformat, out);

        // Switch back to current texture
        if (tex->ptt->texture != tex->gc->pipe[0].shader.cur_tex)
          {
             glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }

        // Now prepare uploading the main texture before returning;
        async = malloc(sizeof (Evas_GL_Texture_Async_Preload));
        if (!async)
          {
             goto upload;
          }

        async->tex = tex;
        async->tex->references++;
        async->im = im;
#ifdef EVAS_CSERVE2
        if (evas_cache2_image_cached(&async->im->cache_entry))
          evas_cache2_image_ref(&async->im->cache_entry);
        else
#endif
        evas_cache_image_ref(&async->im->cache_entry);
        async->unpack_row_length = tex->gc->shared->info.unpack_row_length;

        if (evas_gl_preload_push(async))
          return ;

        // Failed to start asynchronous upload, likely due to preload not being supported by the backend
        async->tex->references--;
#ifdef EVAS_CSERVE2
        if (evas_cache2_image_cached(&async->im->cache_entry))
          evas_cache2_image_close(&async->im->cache_entry);
        else
#endif
        evas_cache_image_drop(&async->im->cache_entry);
        free(async);

     upload:
        pt_unref(tex->ptt);
        tex->ptt = NULL;
     }

   evas_gl_common_texture_upload(tex, im, bytes_count);
}

void
evas_gl_common_texture_free(Evas_GL_Texture *tex, Eina_Bool force EINA_UNUSED)
{
   if (!tex) return;
   if (force)
     {
        evas_gl_preload_pop(tex);

        while (tex->targets)
          evas_gl_preload_target_unregister(tex, eina_list_data_get(tex->targets));
     }
   tex->references--;
   if (tex->references != 0) return;
   if (tex->fglyph)
     {
        tex->gc->font_glyph_textures = eina_list_remove(tex->gc->font_glyph_textures, tex);
        tex->fglyph->ext_dat = NULL;
        tex->fglyph->ext_dat_free = NULL;
     }

   if (tex->pt)
     {
        tex->pt->allocations = eina_list_remove(tex->pt->allocations, tex->apt);
        free(tex->apt);
        tex->apt = NULL;
        pt_unref(tex->pt);
     }
   if (tex->ptt)
     {
        tex->ptt->allocations = eina_list_remove(tex->ptt->allocations, tex->aptt);
        free(tex->aptt);
        tex->aptt = NULL;
        pt_unref(tex->ptt);
     }
   if (tex->ptu)
     pt_unref(tex->ptu);
   if (tex->ptv)
     pt_unref(tex->ptv);
   if (tex->ptuv)
     pt_unref(tex->ptuv);

   evas_gl_common_texture_light_free(tex);
}

Evas_GL_Texture *
evas_gl_common_texture_alpha_new(Evas_Engine_GL_Context *gc, DATA8 *pixels,
                                 unsigned int w, unsigned int h, int fh)
{
   Evas_GL_Texture *tex;
   int u = 0, v = 0;

   tex = evas_gl_common_texture_alloc(gc, w, h, EINA_FALSE);
   if (!tex) return NULL;

   tex->pt = _pool_tex_find(gc, w + 3, fh, alpha_ifmt, alpha_fmt, &u, &v,
                            &tex->apt,
                            gc->shared->info.tune.atlas.max_alloc_alpha_size);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->apt->tex = tex;
   tex->x = u + 1;
   tex->y = v;
   tex->pt->references++;
   evas_gl_common_texture_alpha_update(tex, pixels, w, h, fh);
   return tex;
}

void
evas_gl_common_texture_alpha_update(Evas_GL_Texture *tex, DATA8 *pixels,
                                    unsigned int w, unsigned int h, int fh EINA_UNUSED)
{
   if (!tex->pt) return;
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_sub_2d(tex->gc, tex->x, tex->y, w, h, tex->pt->format,
               tex->pt->dataformat, pixels);
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

Evas_GL_Texture *
evas_gl_common_texture_yuv_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h)
{
   Evas_GL_Texture *tex;

   tex = evas_gl_common_texture_alloc(gc, w, h, EINA_FALSE);
   if (!tex) return NULL;

   tex->ptu = _pool_tex_new(gc, w / 2 + 1, h / 2 + 1, lum_ifmt, lum_fmt);
   if (!tex->ptu)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptu);
   tex->ptu->slot = -1;
   tex->ptu->fslot = -1;
   tex->ptu->whole = 1;
   tex->ptv = _pool_tex_new(gc,  tex->ptu->w, tex->ptu->h, lum_ifmt, lum_fmt);
   if (!tex->ptv)
     {
        pt_unref(tex->ptu);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptv);
   tex->ptv->slot = -1;
   tex->ptv->fslot = -1;
   tex->ptv->whole = 1;
   tex->pt = _pool_tex_new(gc, tex->ptu->w * 2, tex->ptu->h * 2, lum_ifmt, lum_fmt);
   if (!tex->pt)
     {
        pt_unref(tex->ptu);
        pt_unref(tex->ptv);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->pt);
   tex->pt->slot = -1;
   tex->pt->fslot = -1;
   tex->pt->whole = 1;
   tex->pt->references++;
   tex->ptu->references++;
   tex->ptv->references++;
   evas_gl_common_texture_yuv_update(tex, rows, w, h);
   return tex;
}

void
evas_gl_common_texture_yuv_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h)
{
   if (!tex->pt) return;
   // FIXME: works on lowest size 4 pixel high buffers. must also be multiple of 2
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
        glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + 1] - rows[h]);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->ptu->intformat, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat, rows[h]);
        glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + (h / 2) + 1] - rows[h + (h / 2)]);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->ptv->intformat, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2)]);
     }
   else
     {
        unsigned int y;
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          return;
        if ((rows[1] - rows[0]) == (int)w)
          _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
        else
          {
             for (y = 0; y < h; y++)
               _tex_sub_2d(tex->gc, 0, y, w, 1, tex->pt->format, tex->pt->dataformat, rows[y]);
          }
        
        glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->ptu->intformat, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat))
          return;
        if ((rows[h + 1] - rows[h]) == (int)(w / 2))
          _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat, rows[h]);
        else
          {
             for (y = 0; y < (h / 2); y++)
               _tex_sub_2d(tex->gc, 0, y, w / 2, 1, tex->ptu->format, tex->ptu->dataformat, rows[h + y]);
          }
        
        glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->ptv->intformat, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat))
          return;
        if ((rows[h + (h / 2) + 1] - rows[h + (h / 2)]) == (int)(w / 2))
          _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2)]);
        else
          {
             for (y = 0; y < (h / 2); y++)
               _tex_sub_2d(tex->gc, 0, y, w / 2, 1, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2) + y]);
          }
     }
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

static Evas_GL_Texture *
_evas_gl_common_texture_y2uv_new(Evas_Engine_GL_Context *gc,
				 unsigned int yw, unsigned int yh,
                                 Eina_Bool uv2w, Eina_Bool uv2h,
                                 GLenum y_ifmt, GLenum y_fmt,
                                 GLenum uv_ifmt, GLenum uv_fmt,
				 Eina_Bool dynamic)
{
   Evas_GL_Texture_Pool *pt[2] = { NULL, NULL };
   Evas_GL_Texture_Pool *ptuv[2] = { NULL, NULL };
   Evas_GL_Texture *tex;
   unsigned int uvw, uvh;

   uvw = uv2w ? yw / 2 + 1 : yw + 1;
   uvh = uv2h ? yh / 2 + 1 : yh + 1;

   if (!dynamic)
     {
        ptuv[0] = _pool_tex_new(gc, uvw, uvh, uv_ifmt, uv_fmt);
        ptuv[1] = _pool_tex_new(gc, uvw, uvh, uv_ifmt, uv_fmt);

	if (ptuv[0] && ptuv[1])
	  {
             pt[0] = _pool_tex_new(gc,
                                   ptuv[0]->w * (uv2w ? 2 : 1),
                                   ptuv[0]->h * (uv2h ? 2 : 1),
                                   y_ifmt, y_fmt);
             pt[1] = _pool_tex_new(gc,
                                   ptuv[1]->w * (uv2w ? 2 : 1),
                                   ptuv[1]->h * (uv2h ? 2 : 1),
                                   y_ifmt, y_fmt);
	  }
     }
   else
     {
        ptuv[0] = _pool_tex_dynamic_new(gc, uvw, uvh, uv_ifmt, uv_fmt);
        ptuv[1] = _pool_tex_dynamic_new(gc, uvw, uvh, uv_ifmt, uv_fmt);

	if (ptuv[0] && ptuv[1])
	  {
             pt[0] = _pool_tex_dynamic_new(gc,
                                           ptuv[0]->w * (uv2w ? 2 : 1),
                                           ptuv[0]->h * (uv2h ? 2 : 1),
                                           y_ifmt, y_fmt);
             pt[1] = _pool_tex_dynamic_new(gc,
                                           ptuv[1]->w * (uv2w ? 2 : 1),
                                           ptuv[1]->h * (uv2h ? 2 : 1),
                                           y_ifmt, y_fmt);
	  }
     }

   if (!pt[0] || !pt[1] || !ptuv[0] || !ptuv[1])
     goto on_error;

   INF("YUV [%i, %i] => Y[%i, %i], UV[%i, %i]",
       yw, yh,
       pt[0]->w, pt[0]->h,
       ptuv[0]->w, ptuv[0]->h);
   tex = evas_gl_common_texture_alloc(gc, yw, yh, EINA_FALSE);
   if (!tex)
     goto on_error;

   tex->pt = pt[0];
   tex->ptuv = ptuv[0];
   tex->dyn = dynamic;

   pt_link(gc, pt[0]);
   pt_link(gc, pt[1]);
   pt_link(gc, ptuv[0]);
   pt_link(gc, ptuv[1]);

   tex->double_buffer.source = 0;
   memcpy(tex->double_buffer.pt, pt, sizeof (Evas_GL_Texture_Pool *) * 2);
   memcpy(tex->double_buffer.ptuv, ptuv, sizeof (Evas_GL_Texture_Pool *) * 2);

   return tex;

 on_error:
   pt_unref(pt[0]);
   pt_unref(pt[1]);
   pt_unref(ptuv[0]);
   pt_unref(ptuv[1]);
   return NULL;
}

Evas_GL_Texture *
evas_gl_common_texture_yuy2_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h)
{
   Evas_GL_Texture *tex;

   tex = _evas_gl_common_texture_y2uv_new(gc, w, h, EINA_TRUE, EINA_FALSE, lum_alpha_ifmt, lum_alpha_fmt, rgba8_ifmt, rgba8_fmt, 0);
   evas_gl_common_texture_yuy2_update(tex, rows, w, h);
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_nv12_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h)
{
   Evas_GL_Texture *tex;

#ifdef GL_GLES
   tex = _evas_gl_common_texture_y2uv_new(gc, w, h, EINA_TRUE, EINA_TRUE, lum_ifmt, lum_fmt, lum_alpha_ifmt, lum_alpha_fmt, 1);
   if (!tex)
#endif
     tex = _evas_gl_common_texture_y2uv_new(gc, w, h, EINA_TRUE, EINA_TRUE, lum_ifmt, lum_fmt, lum_alpha_ifmt, lum_alpha_fmt, 0);

   evas_gl_common_texture_nv12_update(tex, rows, w, h);
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_nv12tiled_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h)
{
   Evas_GL_Texture *tex = NULL;

#ifdef GL_GLES
   tex = _evas_gl_common_texture_y2uv_new(gc, w, h, EINA_TRUE, EINA_TRUE, lum_ifmt, lum_fmt, lum_alpha_ifmt, lum_alpha_fmt, 1);
   if (!tex)
#endif
     tex = _evas_gl_common_texture_y2uv_new(gc, w, h, EINA_TRUE, EINA_TRUE, lum_ifmt, lum_fmt, lum_alpha_ifmt, lum_alpha_fmt, 0);

   evas_gl_common_texture_nv12tiled_update(tex, rows, w, h);
   return tex;
}

void
evas_gl_common_texture_yuy2_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h)
{
   if (!tex->pt) return;
   // FIXME: works on lowest size 4 pixel high buffers. must also be multiple of 2
   unsigned int y;

   tex->double_buffer.source = 1 - tex->double_buffer.source;
   tex->pt = tex->double_buffer.pt[tex->double_buffer.source];
   tex->ptuv = tex->double_buffer.ptuv[tex->double_buffer.source];

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
     return;
   if ((rows[1] - rows[0]) == (int)w * 4)
     _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
   else
     {
        for (y = 0; y < h; y++)
          _tex_sub_2d(tex->gc, 0, y, w, 1, tex->pt->format, tex->pt->dataformat, rows[y]);
     }

   glBindTexture(GL_TEXTURE_2D, tex->ptuv->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!_tex_2d(tex->gc, tex->ptuv->intformat, w / 2, h, tex->ptuv->format, tex->ptuv->dataformat))
     return;
#if 0
   /*
     FIXME: this piece of code doesn't work anymore since texture width
     is not anymore exactly w / 2. I don't understand why.
   */
   if ((rows[1] - rows[0]) == (int)(w * 2))
     _tex_sub_2d(tex->gc, 0, 0, w / 2, h, tex->ptuv->format, tex->ptuv->dataformat, rows[0]);
   else
#endif
     {
        for (y = 0; y < h; y++)
          _tex_sub_2d(tex->gc, 0, y, w / 2, 1, tex->ptuv->format, tex->ptuv->dataformat, rows[y]);
     }

   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

void
evas_gl_common_texture_nv12_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h)
{
   if (!tex->pt) return;

   tex->double_buffer.source = 1 - tex->double_buffer.source;
   tex->pt = tex->double_buffer.pt[tex->double_buffer.source];
   tex->ptuv = tex->double_buffer.ptuv[tex->double_buffer.source];

   // FIXME: works on lowest size 4 pixel high buffers. must also be multiple of 2
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
        glBindTexture(GL_TEXTURE_2D, tex->ptuv->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + 1] - rows[h]);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->ptuv->intformat, w / 2, h / 2, tex->ptuv->format, tex->ptuv->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptuv->format, tex->ptuv->dataformat, rows[h]);
     }
   else
     {
        unsigned int y;
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          return;
        if ((rows[1] - rows[0]) == (int)w)
          _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
        else
          {
             for (y = 0; y < h; y++)
               _tex_sub_2d(tex->gc, 0, y, w, 1, tex->pt->format, tex->pt->dataformat, rows[y]);
          }
        
        glBindTexture(GL_TEXTURE_2D, tex->ptuv->texture);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (!_tex_2d(tex->gc, tex->ptuv->intformat, w / 2, h / 2, tex->ptuv->format, tex->ptuv->dataformat))
          return;
        if ((rows[h + 1] - rows[h]) == (int)(w / 2))
          _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptuv->format, tex->ptuv->dataformat, rows[h]);
        else
          {
             for (y = 0; y < (h / 2); y++)
               _tex_sub_2d(tex->gc, 0, y, w / 2, 1, tex->ptuv->format, tex->ptuv->dataformat, rows[h + y]);
          }
     }
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

void
evas_gl_common_texture_nv12tiled_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h)
{
   unsigned int mb_x, mb_y, mb_w, mb_h;
   unsigned int base_h;

   if (!tex->pt) return;

   tex->double_buffer.source = 1 - tex->double_buffer.source;
   tex->pt = tex->double_buffer.pt[tex->double_buffer.source];
   tex->ptuv = tex->double_buffer.ptuv[tex->double_buffer.source];

   mb_w = w / 64 + (w % 64 ? 1 : 0);
   mb_h = h / 32 + (h % 32 ? 1 : 0);

#ifdef GL_GLES
   if (tex->dyn)
     {
        char *texture_addr;
	char *tmp;

        texture_addr = secsym_eglMapImageSEC(tex->gc->egldisp,
                                             tex->pt->dyn.img,
                                             EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC,
                                             EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC);

	/* Iterate each Y macroblock like we do in evas_convert_yuv.c */
	for (mb_y = 0; mb_y < (mb_h >> 1); mb_y++)
	  {
	     int step = 2;
	     int offset = 0;
	     int x = 0;
	     int rmb_x = 0;
	     int ry[2];

	     ry[0] = mb_y * 2 * 32 * tex->pt->dyn.stride;
	     ry[1] = ry[0] + 32 * tex->pt->dyn.stride;

	     for (mb_x = 0; mb_x < mb_w * 2; mb_x++, rmb_x += 64 * 32)
	       {
		  unsigned int i;

		  tmp = texture_addr + x + ry[offset];

		  for (i = 0; i < 32 * 64; i += 64, tmp += tex->pt->dyn.stride)
		    memcpy(tmp, rows[mb_y] + rmb_x + i, 64);

		  step++;
		  if ((step & 0x3) == 0)
		    {
		       offset = 1 - offset;
		       x -= 64;
		    }
		  else
		    {
		       x += 64;
		    }
	       }
	  }

	if (mb_h & 0x1)
	  {
 	     int rmb_x = 0;
	     int x = 0;
	     int ry;

	     ry = mb_y * 2 * 32 * tex->pt->dyn.stride;

	     for (mb_x = 0; mb_x < mb_w; mb_x++, x += 64, rmb_x += 64 * 32)
	       {
		  unsigned int i;

		  tmp = texture_addr + x + ry;

		  for (i = 0; i < 32 * 64; i += 64, tmp += tex->pt->dyn.stride)
		    memcpy(tmp, rows[mb_y] + rmb_x + i, 64);
	       }
	  }

	secsym_eglUnmapImageSEC(tex->gc->egldisp, tex->pt->dyn.img, EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);

        texture_addr = secsym_eglMapImageSEC(tex->gc->egldisp,
                                             tex->ptuv->dyn.img,
                                             EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC,
                                             EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC);

	/* Iterate each UV macroblock like we do in evas_convert_yuv.c */
	base_h = (mb_h >> 1) + (mb_h & 0x1);

        /* h is always a multiple of 32 */
        mb_h = h / 2;
        mb_h = (mb_h / 32 + (mb_h % 32 ? 1 : 0));

        mb_w = w / 2;
        mb_w = (mb_w / 64 + (mb_w % 64 ? 1 : 0));

	for (mb_y = 0; mb_y < (mb_h >> 1); mb_y++)
	  {
	     int step = 2;
	     int offset = 0;
	     int x = 0;
	     int rmb_x = 0;
	     int ry[2];

	     ry[0] = mb_y * 2 * 32 * tex->ptuv->dyn.stride;
	     ry[1] = ry[0] + 32 * tex->ptuv->dyn.stride;

	     for (mb_x = 0; mb_x < mb_w * 4; mb_x++, rmb_x += 64 * 32)
	       {
		  unsigned int i = 0;

		  tmp = texture_addr + x + ry[offset];

                  for (i = 0; i < 32 * 64; i += 64, tmp += tex->ptuv->dyn.stride)
                    memcpy(tmp, rows[mb_y + base_h] + rmb_x + i, 64);

		  step++;
		  if ((step & 0x3) == 0)
		    {
		       offset = 1 - offset;
		       x -= 64;
		    }
		  else
		    {
		       x += 64;
		    }
	       }
	  }

	if (mb_h & 0x1)
	  {
	     int rmb_x = 0;
	     int x = 0;
	     int ry;

	     ry = mb_y * 2 * 32 * tex->ptuv->dyn.stride;

	     for (mb_x = 0; mb_x < mb_w * 2; mb_x++, x += 64, rmb_x += 64 * 32)
	       {
		  unsigned int i;

		  tmp = texture_addr + x + ry;

		  /* It has horizontaly half the pixels, but they are double the size*/
		  for (i = 0; i < 32 * 64; i += 64, tmp += tex->ptuv->dyn.stride)
		    memcpy(tmp, rows[mb_y + base_h] + rmb_x + i, 64);
	       }
	  }

	secsym_eglUnmapImageSEC(tex->gc->egldisp, tex->ptuv->dyn.img, EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);
	return;
     }
#endif

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   // We are telling the driver to not swizzle back the buffer as we are going to replace all pixel
   if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
     return;

   /* Iterate each Y macroblock like we do in evas_convert_yuv.c */
   for (mb_y = 0; mb_y < (mb_h >> 1); mb_y++)
     {
        int step = 2;
	int offset = 0;
	int x = 0;
	int rmb_x = 0;
	int ry[2];

	ry[0] = mb_y * 2 * 32;
	ry[1] = ry[0] + 32;

	for (mb_x = 0; mb_x < mb_w * 2; mb_x++, rmb_x += 64 * 32)
	  {
	     _tex_sub_2d(tex->gc, x, ry[offset], 64, 32, tex->pt->format, tex->pt->dataformat, rows[mb_y] + rmb_x);

	     step++;
	     if ((step & 0x3) == 0)
	       {
		  offset = 1 - offset;
		  x -= 64;
	       }
	     else
	       {
		  x += 64;
	       }
	  }
     }

   if (mb_h & 0x1)
     {
        int rmb_x = 0;
	int x = 0;
	int ry;

	ry = mb_y * 2 * 32;

	for (mb_x = 0; mb_x < mb_w; mb_x++, x += 64, rmb_x += 64 * 32)
	  _tex_sub_2d(tex->gc, x, ry, 64, 32, tex->pt->format, tex->pt->dataformat, rows[mb_y] + rmb_x);
     }

   glBindTexture(GL_TEXTURE_2D, tex->ptuv->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   if (!_tex_2d(tex->gc, tex->ptuv->intformat, w, h, tex->ptuv->format, tex->ptuv->dataformat))
     return;

   /* Iterate each UV macroblock like we do in evas_convert_yuv.c */
   base_h = (mb_h >> 1) + (mb_h & 0x1);

   /* h is always a multiple of 32 */
   mb_h = h / 2;
   mb_h = (mb_h / 32 + (mb_h % 32 ? 1 : 0));

   mb_w = w / 2;
   mb_w = (mb_w / 32 + (mb_w % 32 ? 1 : 0));

   for (mb_y = 0; mb_y < (mb_h >> 1); mb_y++)
     {
        int step = 2;
	int offset = 0;
	int x = 0;
	int rmb_x = 0;
	int ry[2];

	ry[0] = mb_y * 2 * 32;
	ry[1] = ry[0] + 32;

	for (mb_x = 0; mb_x < mb_w * 2; mb_x++, rmb_x += 64 * 32)
	  {
             _tex_sub_2d(tex->gc, x, ry[offset], 32, 32,
                         tex->ptuv->format, tex->ptuv->dataformat,
                         rows[mb_y + base_h] + rmb_x);
             step++;
             if ((step & 0x3) == 0)
               {
                  offset = 1 - offset;
                  x -= 32;
               }
             else
               {
                  x += 32;
               }
	  }
     }

   if (mb_h & 0x1)
     {
        int rmb_x = 0;
	int x = 0;
	int ry;

	ry = mb_y * 2 * 32;

	for (mb_x = 0; mb_x < mb_w; mb_x++, x += 32, rmb_x += 64 * 32)
	  _tex_sub_2d(tex->gc, x, ry, 64, 32, tex->ptuv->format, tex->ptuv->dataformat, rows[mb_y + base_h] + rmb_x);
     }
}
