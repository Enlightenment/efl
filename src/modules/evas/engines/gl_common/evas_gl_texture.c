#include "evas_gl_private.h"

static const GLenum rgba_fmt   = GL_RGBA;
static const GLenum rgba_ifmt  = GL_RGBA;

#ifdef GL_GLES
static const GLenum bgra_fmt   = GL_BGRA;
static const GLenum bgra_ifmt  = GL_BGRA;
static const GLenum bgr_fmt    = GL_BGRA;
static const GLenum bgr_ifmt   = GL_BGRA;
#else
static const GLenum rgb_fmt    = GL_RGBA;
#ifdef WORDS_BIGENDIAN
static const GLenum rgb_ifmt   = GL_RGBA;
#else
static const GLenum rgb_ifmt   = GL_RGB;
#endif
static const GLenum bgra_fmt   = GL_BGRA;
static const GLenum bgra_ifmt  = GL_RGBA;
static const GLenum bgr_fmt    = GL_BGRA;
#ifdef WORDS_BIGENDIAN
static const GLenum bgr_ifmt   = GL_RGBA;
#else
static const GLenum bgr_ifmt   = GL_RGB;
#endif
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

static const GLenum s3tc_rgb_dxt1_fmt   = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
static const GLenum s3tc_rgba_dxt1_fmt  = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
static const GLenum s3tc_rgba_dxt23_fmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
static const GLenum s3tc_rgba_dxt45_fmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

static struct {
   struct {
      int num, pix;
   } c, a, v, r, n, d;
} texinfo = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

typedef enum {
   MATCH_FALSE = EINA_FALSE,
   MATCH_TRUE  = EINA_TRUE,
   MATCH_ANY   = 2
} Eina_Bool_Match;

static const struct {
   Eina_Bool_Match alpha;
   Eina_Bool_Match bgra;

   Evas_Colorspace cspace;

   const GLenum *intformat;
   const GLenum *format;
} matching_format[] = {
  { MATCH_TRUE, MATCH_TRUE, EVAS_COLORSPACE_ARGB8888, &bgra_ifmt, &bgra_fmt },
  { MATCH_TRUE, MATCH_FALSE, EVAS_COLORSPACE_ARGB8888, &rgba_ifmt, &rgba_fmt },
  { MATCH_FALSE, MATCH_TRUE, EVAS_COLORSPACE_ARGB8888, &bgr_ifmt, &bgr_fmt },
#ifdef GL_GLES
  { MATCH_FALSE, MATCH_FALSE, EVAS_COLORSPACE_ARGB8888, &rgba_ifmt, &rgba_fmt },
#else
  { MATCH_FALSE, MATCH_FALSE, EVAS_COLORSPACE_ARGB8888, &rgb_ifmt, &rgb_fmt },
#endif
  { MATCH_FALSE, MATCH_ANY, EVAS_COLORSPACE_GRY8, &lum_fmt, &lum_ifmt },
  { MATCH_TRUE, MATCH_ANY, EVAS_COLORSPACE_GRY8, &alpha_fmt, &alpha_ifmt },
  { MATCH_TRUE, MATCH_ANY, EVAS_COLORSPACE_AGRY88, &lum_alpha_fmt, &lum_alpha_ifmt },
  // ETC1/2 support
  { MATCH_FALSE, MATCH_ANY, EVAS_COLORSPACE_ETC1, &etc1_fmt, &etc1_fmt },
  { MATCH_FALSE, MATCH_ANY, EVAS_COLORSPACE_RGB8_ETC2, &etc2_rgb_fmt, &etc2_rgb_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_RGBA8_ETC2_EAC, &etc2_rgba_fmt, &etc2_rgba_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_ETC1_ALPHA, &etc1_fmt, &etc1_fmt },
  // S3TC support
  { MATCH_FALSE, MATCH_ANY, EVAS_COLORSPACE_RGB_S3TC_DXT1, &s3tc_rgb_dxt1_fmt, &s3tc_rgb_dxt1_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_RGBA_S3TC_DXT1, &s3tc_rgba_dxt1_fmt, &s3tc_rgba_dxt1_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_RGBA_S3TC_DXT2, &s3tc_rgba_dxt23_fmt, &s3tc_rgba_dxt23_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_RGBA_S3TC_DXT3, &s3tc_rgba_dxt23_fmt, &s3tc_rgba_dxt23_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_RGBA_S3TC_DXT4, &s3tc_rgba_dxt45_fmt, &s3tc_rgba_dxt45_fmt },
  { MATCH_ANY, MATCH_ANY, EVAS_COLORSPACE_RGBA_S3TC_DXT5, &s3tc_rgba_dxt45_fmt, &s3tc_rgba_dxt45_fmt }
};

static void
_tex_sub_2d(Evas_Engine_GL_Context *gc, int x, int y, int w, int h, int fmt, int type, const void *pix)
{
   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size)) return;
   glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, type, pix);
}

static void
_comp_tex_sub_2d(Evas_Engine_GL_Context *gc, int x, int y, int w, int h, int fmt, int imgsize, const void *pix)
{
   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size)) return;
   glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, imgsize, pix);
}

#ifdef GL_TEXTURE_INTERNAL_FORMAT
# ifndef GL_GLES
static const GLenum matching_rgb[] = { GL_RGB4, GL_RGB8, GL_RGB12, GL_RGB16, 0x0 };
static const GLenum matching_rgba[] = { GL_RGBA4, GL_RGBA8, GL_RGBA12, GL_RGBA16, 0x0 };
static const GLenum matching_alpha[] = { GL_ALPHA4, GL_ALPHA8, GL_ALPHA12, GL_ALPHA16, 0x0 };
static const GLenum matching_luminance[] = { GL_LUMINANCE4, GL_LUMINANCE8, GL_LUMINANCE12, GL_LUMINANCE16, 0x0 };
static const GLenum matching_luminance_alpha[] = { GL_LUMINANCE4_ALPHA4, GL_LUMINANCE8_ALPHA8, GL_LUMINANCE12_ALPHA12, GL_LUMINANCE16_ALPHA16, 0x0 };

static const struct {
   GLenum master;
   const GLenum *matching;
} matching_fmt[] = {
  { GL_RGB, matching_rgb },
  { GL_RGBA, matching_rgba },
  { GL_ALPHA, matching_alpha },
  { GL_ALPHA4, matching_alpha },
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
# endif
#endif

#define MATCH(_r, _v) ((_r == MATCH_ANY) || (_v == MATCH_ANY) || (_r == _v))

static int
_evas_gl_texture_search_format(Eina_Bool alpha, Eina_Bool bgra, Evas_Colorspace cspace)
{
   unsigned int i;

   alpha = !!alpha;
   bgra = !!bgra;

   for (i = 0; i < sizeof (matching_format) / sizeof (matching_format[0]); ++i)
     if (MATCH(matching_format[i].alpha, alpha) &&
         MATCH(matching_format[i].bgra, bgra) &&
         matching_format[i].cspace == cspace)
       return i;

   CRI("There is no supported texture format for this colorspace: "
       "cspace(%d) alpha(%d) bgra(%d)", cspace, alpha, bgra);
   return -1;
}

Evas_Colorspace
evas_gl_common_gl_format_to_colorspace(GLuint f)
{
   unsigned i;

   for (i = 0; i < sizeof(matching_format) / sizeof(matching_format[0]); i++)
     {
        if (*matching_format[i].format == f)
          return matching_format[i].cspace;
     }

   ERR("Unknown texture format!");
   return EVAS_COLORSPACE_ARGB8888;
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
      case GL_ETC1_RGB8_OES:
        return 3;
      case GL_COMPRESSED_RGB8_ETC2:
        return 4;
      case GL_COMPRESSED_RGBA8_ETC2_EAC:
        return 5;
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        return 6;
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        return 7;
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: // dxt2 as well
        return 8;
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: // dxt4 as well
        return 9;
      case GL_LUMINANCE: // never used in atlas
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
        return 10;
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
        return 11;
      default:
        ERR("Unknown format returned specified by GL stack: %x", format);
        return -1;
     }
   return 0;
}

static inline int
_evas_gl_texture_size_get(int w, int h, int intfmt, Eina_Bool *comp)
{
   if (comp) *comp = EINA_FALSE;
   switch (intfmt)
     {
      case GL_RGBA:
      case GL_BGRA:
      case GL_RGB:
        return w * h * 4;
      case GL_ALPHA:
        return w * h * 1;
      case GL_ALPHA4:
        return w * h / 2; // TODO: Check this
      case GL_LUMINANCE:
        return w * h * 1;
      case GL_LUMINANCE_ALPHA:
        return w * h * 2;
      case GL_ETC1_RGB8_OES:
      case GL_COMPRESSED_RGB8_ETC2:
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        if (comp) *comp = EINA_TRUE;
        return ((w + 3) >> 2) * ((h + 3) >> 2) * 8;
      case GL_COMPRESSED_RGBA8_ETC2_EAC:
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        if (comp) *comp = EINA_TRUE;
        return ((w + 3) >> 2) * ((h + 3) >> 2) * 16;
      default:
        return 0;
     }
}

static Eina_Bool
_tex_2d(Evas_Engine_GL_Context *gc, int intfmt, int w, int h, int fmt, int type)
{
   Eina_Bool comp;
   int sz;

   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size))
     {
        ERR("Fail tex too big %ix%i", w, h);
        return EINA_FALSE;
     }
   sz = _evas_gl_texture_size_get(w, h, intfmt, &comp);
   if (!comp)
     glTexImage2D(GL_TEXTURE_2D, 0, intfmt, w, h, 0, fmt, type, NULL);
   else
     glCompressedTexImage2D(GL_TEXTURE_2D, 0, intfmt, w, h, 0, sz, NULL);
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
             ERR("Fail tex alloc %ix%i, intfmt: %X  intfmtret: %X", w, h, intfmt, intfmtret);
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
        _tex_adjust(gc, &w, &h);
     }
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->references = 0;
   pt->eina_pool = eina_rectangle_pool_new(w, h);

   glGenTextures(1, &(pt->texture));
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   ok = _tex_2d(gc, pt->intformat, w, h, pt->format, pt->dataformat);
   glBindTexture(gc->state.current.tex_target, gc->state.current.cur_tex);
   if (!ok)
     {
        glDeleteTextures(1, &(pt->texture));
        if (pt->eina_pool)
          eina_rectangle_pool_free(pt->eina_pool);
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

static Eina_Rectangle *
_pool_tex_alloc(Evas_GL_Texture_Pool *pt, int w, int h, int *u, int *v)
{
   Eina_Rectangle *r;
   r = eina_rectangle_pool_request( pt->eina_pool, w, h);
   if (r)
     {
        *v = r->y;
        *u = r->x;
        pt->allocations = eina_list_prepend(pt->allocations, r);
     }

   return r;
}

static Evas_GL_Texture_Pool *
_pool_tex_find(Evas_Engine_GL_Context *gc, int w, int h,
               GLenum intformat, GLenum format, int *u, int *v,
               Eina_Rectangle **apt, int atlas_w, Eina_Bool disable_atlas)
{
   Evas_GL_Texture_Pool *pt = NULL;
   Eina_List *l;
   int th2;
   int pool_h;
   /*Return texture unit without atlas*/
   if (disable_atlas)
     {
        pt = _pool_tex_new(gc, w, h, intformat, format);
        return pt ? pt : NULL;
     }
   if (atlas_w > gc->shared->info.max_texture_size)
      atlas_w = gc->shared->info.max_texture_size;
   if ((w > gc->shared->info.tune.atlas.max_w) ||
       (h > gc->shared->info.tune.atlas.max_h) ||
       (!gc->shared->info.etc1_subimage && (intformat == etc1_fmt)))
     {
        pt = _pool_tex_new(gc, w, h, intformat, format);
        if (!pt) return NULL;
        gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, pt);
        pt->fslot = -1;
        pt->whole = 1;
        *apt = _pool_tex_alloc(pt, w, h, u, v);
        return pt;
     }

   th2 = _tex_format_index(intformat);
   if (th2 < 0) return NULL;
   EINA_LIST_FOREACH(gc->shared->tex.atlas[th2], l, pt)
     {
        if (pt->render) continue;
        if ((*apt = _pool_tex_alloc(pt, w, h, u, v)) != NULL)
          {
             gc->shared->tex.atlas[th2] =
               eina_list_promote_list(gc->shared->tex.atlas[th2], l);
             return pt;
          }
     }
   pool_h = atlas_w;
   if ( h > pool_h || w > atlas_w )
     {
        atlas_w = gc->shared->info.tune.atlas.max_w;
        pool_h = gc->shared->info.tune.atlas.max_h;
     }
   pt = _pool_tex_new(gc, atlas_w, pool_h, intformat, format);
   if (!pt) return NULL;
   gc->shared->tex.atlas[th2] =
     eina_list_prepend(gc->shared->tex.atlas[th2], pt);
   pt->fslot = th2;

   *apt = _pool_tex_alloc(pt, w, h, u, v);

   return pt;
}

Evas_GL_Texture *
evas_gl_common_texture_new(Evas_Engine_GL_Context *gc, RGBA_Image *im, Eina_Bool disable_atlas)
{
   Evas_GL_Texture *tex;
   GLsizei w, h;
   int u = 0, v = 0, xoffset = 1, yoffset = 1;
   int lformat;

   lformat = _evas_gl_texture_search_format(im->cache_entry.flags.alpha, gc->shared->info.bgra, im->cache_entry.space);
   if (lformat < 0) return NULL;

   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
        // Add border to avoid artifacts
        w = im->cache_entry.w + im->cache_entry.borders.l + im->cache_entry.borders.r;
        h = im->cache_entry.h + im->cache_entry.borders.t + im->cache_entry.borders.b;
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!(w & 0x3) && !(h & 0x3), NULL);
        xoffset = im->cache_entry.borders.l;
        yoffset = im->cache_entry.borders.t;
        break;
      case EVAS_COLORSPACE_ETC1_ALPHA:
        return evas_gl_common_texture_rgb_a_pair_new(gc, im);
     default:
        if (disable_atlas)
          {
             /*Just one pixel gap. Generate texture unit without pixels for
             borders in case using this for evas_canvas3d in repeat mode of the
             texture unit*/
             w = im->cache_entry.w + 1;
             h = im->cache_entry.h + 1;
          }
        else
          {
             /*One pixel gap and two pixels for duplicated borders*/
             w = im->cache_entry.w + 3;
             h = im->cache_entry.h + 3;
          }
        break;
     }

   tex = evas_gl_common_texture_alloc(gc, im->cache_entry.w, im->cache_entry.h,
                                      im->cache_entry.flags.alpha);
   if (!tex) return NULL;

   tex->pt = _pool_tex_find(gc, w, h,
                            *matching_format[lformat].intformat,
                            *matching_format[lformat].format,
                            &u, &v, &tex->apt,
                            gc->shared->info.tune.atlas.max_alloc_size, disable_atlas);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->x = u + xoffset;
   tex->y = v + yoffset;

   tex->pt->references++;
   evas_gl_common_texture_update(tex, im);

   return tex;
}

static Evas_GL_Texture_Pool *
_pool_tex_render_new(Evas_Engine_GL_Context *gc, int w, int h, int intformat, int format, int stencil)
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
   pt->eina_pool = eina_rectangle_pool_new(w, h);
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
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   ok = _tex_2d(gc, pt->intformat, w, h, pt->format, pt->dataformat);

   if (ok)
     {
        glsym_glGenFramebuffers(1, &(pt->fb));
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, pt->fb);
        glsym_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pt->texture, 0);
        // note: should check fbo completeness
     }

   if (stencil)
     {
        glGenRenderbuffers(1, &(pt->stencil));
        glBindRenderbuffer(GL_RENDERBUFFER, pt->stencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, pt->w, pt->h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pt->stencil);
     }

   glsym_glBindFramebuffer(GL_FRAMEBUFFER, fnum);
   glBindTexture(gc->state.current.tex_target, gc->state.current.cur_tex);

   if (!ok)
     {
        glDeleteTextures(1, &(pt->texture));
        if (pt->eina_pool)
          eina_rectangle_pool_free(pt->eina_pool);
        free(pt);
        return NULL;
     }

   texinfo.r.num++;
   texinfo.r.pix += pt->w * pt->h;
   _print_tex_count();
   return pt;
}

static Evas_GL_Texture_Pool *
_pool_tex_render_find(Evas_Engine_GL_Context *gc, int w, int h,
                      GLenum intformat, GLenum format, int *u, int *v,
                      Eina_Rectangle **apt, int atlas_w, Eina_Bool disable_atlas)
{
   Evas_GL_Texture_Pool *pt = NULL;
   Eina_List *l;
   int th2;
   int pool_h;
   /*Return texture unit without atlas*/
   if (disable_atlas)
     {
        pt = _pool_tex_render_new(gc, w, h, intformat, format, EINA_FALSE);
        return pt ? pt : NULL;
     }
   if (atlas_w > gc->shared->info.max_texture_size)
      atlas_w = gc->shared->info.max_texture_size;
   if ((w > gc->shared->info.tune.atlas.max_w) ||
       (h > gc->shared->info.tune.atlas.max_h) ||
       (!gc->shared->info.etc1_subimage && (intformat == etc1_fmt)))
     {
        pt = _pool_tex_render_new(gc, w, h, intformat, format, EINA_FALSE);
        if (!pt) return NULL;
        gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, pt);
        pt->fslot = -1;
        pt->whole = 1;
        *apt = _pool_tex_alloc(pt, w, h, u, v);
        return pt;
     }

   th2 = _tex_format_index(intformat);
   if (th2 < 0) return NULL;
   EINA_LIST_FOREACH(gc->shared->tex.atlas[th2], l, pt)
     {
        if (!pt->render) continue;
        if ((*apt = _pool_tex_alloc(pt, w, h, u, v)) != NULL)
          {
             gc->shared->tex.atlas[th2] =
               eina_list_promote_list(gc->shared->tex.atlas[th2], l);
             return pt;
          }
     }
   pool_h = atlas_w;
   if ( h > pool_h || w > atlas_w )
     {
        atlas_w = gc->shared->info.tune.atlas.max_w;
        pool_h = gc->shared->info.tune.atlas.max_h;
     }
   pt = _pool_tex_render_new(gc, atlas_w, pool_h, intformat, format, EINA_FALSE);
   if (!pt) return NULL;
   gc->shared->tex.atlas[th2] =
     eina_list_prepend(gc->shared->tex.atlas[th2], pt);
   pt->fslot = th2;

   *apt = _pool_tex_alloc(pt, w, h, u, v);

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
   pt->eina_pool = eina_rectangle_pool_new(w, h);
   glGenTextures(1, &(pt->texture));
   glBindTexture(im->native.target, pt->texture);

#ifdef GL_GLES
#else
   if (im->native.loose)
     {
        if (im->native.func.bind)
          im->native.func.bind(im);
     }
#endif

   glTexParameteri(im->native.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(im->native.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(im->native.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(im->native.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glBindTexture(im->native.target, 0);
   glBindTexture(gc->state.current.tex_target, gc->state.current.cur_tex);

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
   void *egldisplay;

   if (intformat != format) return NULL;

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
   pt->eina_pool = eina_rectangle_pool_new(w, h);
   texinfo.d.num++;
   texinfo.d.pix += pt->w * pt->h;

   _print_tex_count();

   pt->dyn.target = GL_TEXTURE_2D;

   glGenTextures(1, &(pt->texture));
   glBindTexture(pt->dyn.target, pt->texture);
   glTexParameteri(pt->dyn.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(pt->dyn.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(pt->dyn.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(pt->dyn.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   egldisplay = pt->gc->egldisp;

   if (gc->shared->info.sec_tbm_surface)
     {
        tbm_format buffer_format = TBM_FORMAT_RGBA8888;
        tbm_surface_info_s info;

        switch (intformat)
          {
           case GL_LUMINANCE: buffer_format = TBM_FORMAT_C8; break;
           case GL_LUMINANCE_ALPHA: buffer_format = TBM_FORMAT_C8; break;
           case GL_RGBA: buffer_format = TBM_FORMAT_RGBA8888; break;
           case GL_BGRA: buffer_format = TBM_FORMAT_BGRA8888; break;
           case GL_RGB: buffer_format = TBM_FORMAT_RGB888; break;
           default: ERR("TBM: unknown format"); goto error;
          }

        pt->dyn.buffer = (void *)secsym_tbm_surface_create(pt->w, pt->h,
                                                           buffer_format);
        if (!pt->dyn.buffer) goto error;

        pt->dyn.img = evas_gl_common_eglCreateImage(egldisplay,
                                                    EGL_NO_CONTEXT,
                                                    EGL_NATIVE_SURFACE_TIZEN,
                                                    pt->dyn.buffer, NULL);
        if (!pt->dyn.img)
          {
             secsym_tbm_surface_destroy(pt->dyn.buffer);
             goto error;
          }
        secsym_tbm_surface_get_info(pt->dyn.buffer, &info);
        pt->dyn.w = info.width;
        pt->dyn.h = info.height;
        pt->dyn.stride = info.planes[0].stride;
     }
   else if (gc->shared->info.sec_image_map)
     {
        int fmt; // EGL_MAP_GL_TEXTURE_RGBA_SEC or EGL_MAP_GL_TEXTURE_RGB_SEC or bust
        int pixtype; // EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC or bust
        EGLAttrib attr[] =
          {
             EGL_MAP_GL_TEXTURE_WIDTH_SEC, 32,
             EGL_MAP_GL_TEXTURE_HEIGHT_SEC, 32,
             EGL_MAP_GL_TEXTURE_FORMAT_SEC, EGL_MAP_GL_TEXTURE_RGBA_SEC,
             EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC, EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC,
             EGL_NONE
          };

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
           default: ERR("SEC map: unknown format"); goto error;
          }

        attr[1] = pt->w;
        attr[3] = pt->h;

        // FIXME: seems a bit slower than i'd like - maybe too many flushes?
        // FIXME: YCbCr no support as yet
        pt->dyn.img = evas_gl_common_eglCreateImage(egldisplay,
                                                    EGL_NO_CONTEXT,
                                                    EGL_MAP_GL_TEXTURE_2D_SEC,
                                                    0, attr);
        if (!pt->dyn.img) goto error;

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
     }
   else
     {
        ERR("TBM surface or SEC image map should be enabled!");
        goto error;
     }

   glBindTexture(gc->state.current.tex_target, gc->state.current.cur_tex);
#else
   if (gc + w + h + intformat + format) return pt;
#endif
   return pt;

/* ERROR HANDLING */
#ifdef GL_GLES
error:
  if (pt->dyn.img)
    {
       eglsym_eglDestroyImage(egldisplay, pt->dyn.img);
       pt->dyn.img = NULL;
    }
  glBindTexture(pt->dyn.target, 0);
  glDeleteTextures(1, &(pt->texture));
  if (pt->eina_pool)
    eina_rectangle_pool_free(pt->eina_pool);
  free(pt);
  return NULL;
#endif
}

void
evas_gl_texture_pool_empty(Evas_GL_Texture_Pool *pt)
{
   Eina_Rectangle *apt;

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
          {
             if (pt->gc->shared->info.sec_tbm_surface)
               secsym_tbm_surface_unmap(pt->dyn.buffer);
             else if (pt->gc->shared->info.sec_image_map)
               secsym_eglUnmapImageSEC(pt->gc->egldisp, pt->dyn.img, EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);
          }
        if (pt->dyn.buffer)
          secsym_tbm_surface_destroy(pt->dyn.buffer);
        eglsym_eglDestroyImage(pt->gc->egldisp, pt->dyn.img);
        pt->dyn.img = NULL;
        pt->dyn.buffer = NULL;
        pt->dyn.data = NULL;
        pt->dyn.w = 0;
        pt->dyn.h = 0;
        pt->dyn.stride = 0;
        pt->dyn.checked_out = 0;
     }
#endif

   glDeleteTextures(1, &(pt->texture));
   if (pt->gc->state.current.cur_tex == pt->texture)
     pt->gc->state.current.cur_tex = 0;
   if (pt->stencil)
     {
        glDeleteRenderbuffers(1, &(pt->stencil));
        pt->stencil = 0;
     }
   if (pt->fb)
     {
        glsym_glDeleteFramebuffers(1, &(pt->fb));
        pt->fb = 0;
     }
    EINA_LIST_FREE(pt->allocations, apt)
      eina_rectangle_pool_release(apt);
   pt->texture = 0;
   pt->gc = NULL;
   pt->w = 0;
   pt->h = 0;
}

void
pt_unref(Evas_GL_Texture_Pool *pt)
{
   if (!pt) return;
   pt->references--;
   if (pt->references != 0) return;

   if ((pt->gc) && !(pt->native))
     {
        if (pt->whole)
           pt->gc->shared->tex.whole =
           eina_list_remove(pt->gc->shared->tex.whole, pt);
        else
           pt->gc->shared->tex.atlas [pt->fslot] =
           eina_list_remove(pt->gc->shared->tex.atlas[pt->fslot], pt);
     }
   evas_gl_texture_pool_empty(pt);
   if (pt->eina_pool)
     eina_rectangle_pool_free(pt->eina_pool);
   free(pt);
}

static void
pt_link(Evas_Engine_GL_Context *gc, Evas_GL_Texture_Pool *pt)
{
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, pt);
   pt->fslot = -1;
   pt->whole = 1;
   pt->references++;
}

Evas_GL_Texture *
evas_gl_common_texture_native_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_GL_Image *im)
{
   Evas_GL_Texture *tex;
   int lformat;

   lformat = _evas_gl_texture_search_format(alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   if (lformat < 0) return NULL;

   tex = evas_gl_common_texture_alloc(gc, w, h, alpha);
   if (!tex) return NULL;
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
evas_gl_common_texture_render_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, int stencil)
{
   Evas_GL_Texture *tex;
   int lformat;

   lformat = _evas_gl_texture_search_format(alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   if (lformat < 0) return NULL;

   tex = evas_gl_common_texture_alloc(gc, w, h, alpha);
   if (!tex) return NULL;
   tex->pt = _pool_tex_render_new(gc, w, h,
                                  *matching_format[lformat].intformat,
                                  *matching_format[lformat].format, stencil);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->pt->references++;
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_render_noscale_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha)
{
   Evas_GL_Texture *tex;
   int u = 0, v = 0;
   int lformat;

   lformat = _evas_gl_texture_search_format(alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   if (lformat < 0) return NULL;

   tex = evas_gl_common_texture_alloc(gc, w, h, alpha);
   if (!tex) return NULL;
   tex->pt = _pool_tex_render_find(gc, w, h,
                                   *matching_format[lformat].intformat,
                                   *matching_format[lformat].format,
                                   &u, &v, &tex->apt,
                                   // XXX: should this be another atlas size?
                                   gc->shared->info.tune.atlas.max_alloc_size * 8,
                                   EINA_FALSE);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   tex->x = u;
   tex->y = v;

   tex->pt->references++;
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_dynamic_new(Evas_Engine_GL_Context *gc, Evas_GL_Image *im)
{
   Evas_GL_Texture *tex;
   int lformat;

   lformat = _evas_gl_texture_search_format(im->alpha, gc->shared->info.bgra, EVAS_COLORSPACE_ARGB8888);
   if (lformat < 0) return NULL;

   tex = evas_gl_common_texture_alloc(gc, im->w, im->h, im->alpha);
   if (!tex) return NULL;
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

   if ((((int)im->cache_entry.w) <= 0) ||
       (((int)im->cache_entry.h) <= 0)) return;
   fmt = tex->pt->format;
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   if (tex->gc->shared->info.unpack_row_length)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   glPixelStorei(GL_UNPACK_ALIGNMENT, bytes_count);

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
               im->image.data8 + (((im->cache_entry.h - 1) * im->cache_entry.w)) * bytes_count);
   //  xxx
   //  xxx
   // o
   _tex_sub_2d(tex->gc, tex->x - 1, tex->y + im->cache_entry.h,
               1, 1,
               fmt, tex->pt->dataformat,
               im->image.data8 + (((im->cache_entry.h - 1) * im->cache_entry.w)) * bytes_count);
   //  xxx
   //  xxx
   //     o
   _tex_sub_2d(tex->gc, tex->x + im->cache_entry.w, tex->y + im->cache_entry.h,
               1, 1,
               fmt, tex->pt->dataformat,
               im->image.data8 + (im->cache_entry.h * im->cache_entry.w - 1) * bytes_count);
   //2D packing
   // ---
   // xxx
   // xxx
   _tex_sub_2d(tex->gc, tex->x, tex->y - 1,
               im->cache_entry.w, 1,
               fmt, tex->pt->dataformat,
               im->image.data);
   // o
   //  xxx
   //  xxx
   _tex_sub_2d(tex->gc, tex->x - 1, tex->y - 1,
               1, 1,
               fmt, tex->pt->dataformat,
               im->image.data);
   //    o
   // xxx
   // xxx
   _tex_sub_2d(tex->gc, tex->x + im->cache_entry.w, tex->y - 1,
               1, 1,
               fmt, tex->pt->dataformat,
               im->image.data8 + (im->cache_entry.w - 1) * bytes_count);
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, im->cache_entry.w);
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
                    im->image.data8 + (im->cache_entry.w - 1) * bytes_count);
        //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
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
   //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   if (tex->pt->texture != tex->gc->state.current.cur_tex)
     glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
}

void
evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im)
{
   unsigned int bytes_count, bsize = 8;

   if (tex->alpha != im->cache_entry.flags.alpha)
     {
        int lformat;

        lformat = _evas_gl_texture_search_format(tex->alpha, tex->gc->shared->info.bgra, im->cache_entry.space);
        if (lformat < 0) return;

        tex->pt->allocations = eina_list_remove(tex->pt->allocations, tex->apt);
        if (tex->apt)
          eina_rectangle_pool_release(tex->apt);

        // FIXME: why a 'render' new here ??? Should already have been allocated, quite a weird path.
        tex->pt = _pool_tex_render_new(tex->gc, tex->w, tex->h,
                                       *matching_format[lformat].intformat,
                                       *matching_format[lformat].format, EINA_FALSE);
     }
   // If image was preloaded then we need a ptt
   if (!tex->pt) return;
   if (!im->image.data) return;

   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888: bytes_count = 4; break;
      case EVAS_COLORSPACE_GRY8: bytes_count = 1; break;
      case EVAS_COLORSPACE_AGRY88: bytes_count = 2; break;
      // Compressed texture formats: S3TC and ETC1/2
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
        bsize = 16;
        // fallthrough
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
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

           x = tex->x - im->cache_entry.borders.l;
           y = tex->y - im->cache_entry.borders.t;
           width = im->cache_entry.w + im->cache_entry.borders.l + im->cache_entry.borders.r;
           height = im->cache_entry.h + im->cache_entry.borders.t + im->cache_entry.borders.b;
           EINA_SAFETY_ON_FALSE_RETURN(!(width & 0x3) && !(height & 0x3));

           glBindTexture(GL_TEXTURE_2D, tex->pt->texture);

           if ((tex->gc->shared->info.etc1_subimage ||
               (im->cache_entry.space != EVAS_COLORSPACE_ETC1))
               && (tex->pt->w != width || tex->pt->h != height))
             {
                int err;
                err = glGetError();

                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
                                          x, y, width, height,
                                          tex->pt->format,
                                          ((width * height) >> 4) * bsize,
                                          im->image.data);

                err = glGetError();
                if (err != GL_NO_ERROR)
                  {
                     __evas_gl_err(err, __FILE__, __FUNCTION__, __LINE__,
                                   "glCompressedTexSubImage2D");

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
                                       ((width * height) >> 4) * bsize,
                                       im->image.data);
             }

           if (tex->pt->texture != tex->gc->state.current.cur_tex)
             {
                glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
             }
           return;
        }
      case EVAS_COLORSPACE_ETC1_ALPHA:
        // One must call evas_gl_common_texture_rgb_a_pair_update() instead.
        ERR("Can't upload ETC1+Alpha texture as a normal texture. Abort.");
        return;
      default:
        ERR("Can't upload texture in colorspace %i.", im->cache_entry.space);
        return;
     }

   // if preloaded, then async push it in after uploading a miniature of it
   if (im->cache_entry.flags.preload_done
       && (tex->w > (2 * EVAS_GL_TILE_SIZE))
       && (tex->h > (2 * EVAS_GL_TILE_SIZE))
       && evas_gl_preload_enabled())
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
        ystep = (float)tex->h / (EVAS_GL_TILE_SIZE - 2);
        in = im->image.data8;

        // top-left
        memcpy(&out[0],
               &in[0],
               bytes_count);

        // top
        for (x = xstep * 0.5f, i = 1; i < EVAS_GL_TILE_SIZE - 1; x += xstep, i++)
          {
             memcpy(&out[i * bytes_count],
                    &in[(int)x * bytes_count],
                    bytes_count);
          }

        // top-right
        memcpy(&out[((EVAS_GL_TILE_SIZE - 1) * bytes_count)],
               &in[(im->cache_entry.w  - 1) * bytes_count],
               bytes_count);

        for (y = ystep * 0.5f, j = 1; j < EVAS_GL_TILE_SIZE - 1; y += ystep, j++)
          {
             // left
             memcpy(&out[j * EVAS_GL_TILE_SIZE * bytes_count],
                    &in[((int)y * im->cache_entry.w) * bytes_count],
                    bytes_count);
             // middle
             for (x = xstep * 0.5f, i = 1; i < EVAS_GL_TILE_SIZE - 1; x += xstep, i++)
               memcpy(&out[(j * EVAS_GL_TILE_SIZE + i) * bytes_count],
                      &in[(((int)y * im->cache_entry.w) + (int)x) * bytes_count],
                      bytes_count);
             // right
             memcpy(&out[(j * EVAS_GL_TILE_SIZE + i) * bytes_count],
                    &in[(((int)y * im->cache_entry.w) + (im->cache_entry.w - 1)) * bytes_count],
                    bytes_count);
          }

        // bottom-left
        memcpy(&out[(j * EVAS_GL_TILE_SIZE) * bytes_count],
               &in[((im->cache_entry.w * (im->cache_entry.h - 1)) + 1) * bytes_count],
               bytes_count);

        // bottom
        for (x = xstep * 0.5f, i = 1; i < EVAS_GL_TILE_SIZE - 1; x += xstep, i++)
          {
             memcpy(&out[((EVAS_GL_TILE_SIZE * j) + i) * bytes_count],
                    &in[((int)x + im->cache_entry.w * (im->cache_entry.h - 1)) * bytes_count],
                    bytes_count);
          }

        // bottom-right
        memcpy(&out[((EVAS_GL_TILE_SIZE * EVAS_GL_TILE_SIZE) - 1) * bytes_count],
               &in[((im->cache_entry.w * im->cache_entry.h) - 1) * bytes_count],
               bytes_count);

        // out is a miniature of the texture, upload that now and schedule the data for later.

        // Creating the mini picture texture
        lformat = _evas_gl_texture_search_format(tex->alpha, tex->gc->shared->info.bgra, im->cache_entry.space);
        if (lformat < 0) return;

        tex->ptt = _pool_tex_find(tex->gc, EVAS_GL_TILE_SIZE, EVAS_GL_TILE_SIZE,
                                  *matching_format[lformat].intformat,
                                  *matching_format[lformat].format,
                                  &u, &v, &tex->aptt,
                                  tex->gc->shared->info.tune.atlas.max_alloc_size, EINA_FALSE);
        if (!tex->ptt)
          goto upload;

        tex->tx = u + 1;
        tex->ty = v;
        tex->ptt->references++;

        // Bind and upload ! Vooom !
        fmt = tex->ptt->format;
        glBindTexture(GL_TEXTURE_2D, tex->ptt->texture);
        if (tex->gc->shared->info.unpack_row_length)
          {
             glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
          }
        glPixelStorei(GL_UNPACK_ALIGNMENT, bytes_count);

        _tex_sub_2d(tex->gc, u, tex->ty, EVAS_GL_TILE_SIZE, EVAS_GL_TILE_SIZE, fmt, tex->ptt->dataformat, out);

        // Switch back to current texture
        if (tex->ptt->texture != tex->gc->state.current.cur_tex)
          glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);

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
   im->cache_entry.flags.textured = 1;
}

void
evas_gl_common_texture_free(Evas_GL_Texture *tex, Eina_Bool force)
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
        if (tex->apt) eina_rectangle_pool_release(tex->apt);
        tex->apt = NULL;
        pt_unref(tex->pt);
        tex->pt = NULL;
     }
   if (tex->pt2)
     {
        tex->pt2->allocations = eina_list_remove(tex->pt2->allocations, tex->apt);
        if (tex->apt) eina_rectangle_pool_release(tex->apt);
        tex->apt = NULL;
        pt_unref(tex->pt2);
        tex->pt2 = NULL;
     }
   if (tex->ptt)
     {
        tex->ptt->allocations = eina_list_remove(tex->ptt->allocations, tex->aptt);
        if (tex->aptt) eina_rectangle_pool_release(tex->aptt);
        tex->aptt = NULL;
        pt_unref(tex->ptt);
        tex->ptt = NULL;
     }
   if (tex->ptu) pt_unref(tex->ptu);
   if (tex->ptv) pt_unref(tex->ptv);
   if (tex->ptuv) pt_unref(tex->ptuv);
   if (tex->ptu2) pt_unref(tex->ptu2);
   if (tex->ptv2) pt_unref(tex->ptv2);
   tex->ptu = NULL;
   tex->ptv = NULL;
   tex->ptu2 = NULL;
   tex->ptv2 = NULL;
   tex->ptuv = NULL;

   if (tex->im) 
     {
        tex->im->tex = NULL;
        if (tex->im->im) tex->im->im->cache_entry.flags.textured = 0;
     }

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
                            gc->shared->info.tune.atlas.max_alloc_alpha_size, EINA_FALSE);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
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
   if (tex->gc->shared->info.unpack_row_length)
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
     }
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   _tex_sub_2d(tex->gc, tex->x, tex->y, w, h, tex->pt->format,
               tex->pt->dataformat, pixels);
   if (tex->pt->texture != tex->gc->state.current.cur_tex)
     glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
}

Evas_GL_Texture *
evas_gl_common_texture_rgb_a_pair_new(Evas_Engine_GL_Context *gc,
                                      RGBA_Image *im)
{
   Evas_GL_Texture *tex;
   int lformat, w, h;

   // FIXME/TODO: We don't support texture atlasses here (a bit tricky)

   // Some debugging. We could return.
   if (im->cache_entry.space != EVAS_COLORSPACE_ETC1_ALPHA)
     WRN("Using RGB+A texture pair with format %d", im->cache_entry.space);

   w = im->cache_entry.w;
   h = im->cache_entry.h;

   lformat = _evas_gl_texture_search_format(EINA_TRUE, gc->shared->info.bgra,
                                            im->cache_entry.space);
   if (lformat < 0) return NULL;

   tex = evas_gl_common_texture_alloc(gc, w, h, EINA_TRUE);
   if (!tex) return NULL;

   w += im->cache_entry.borders.l + im->cache_entry.borders.r;
   h += im->cache_entry.borders.t + im->cache_entry.borders.b;
   tex->x = im->cache_entry.borders.l;
   tex->y = im->cache_entry.borders.t;

   // Allocate RGB texture normally - as a 'whole'
   tex->pt = _pool_tex_new(gc, w, h,
                           *matching_format[lformat].intformat,
                           *matching_format[lformat].format);
   if (!tex->pt)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   pt_link(gc, tex->pt);
   tex->pt->slot = -1;

   // And now Alpha texture -- FIXME could intformat be different? (eg. ALPHA4)
   tex->pta = _pool_tex_new(gc, w, h,
                            *matching_format[lformat].intformat,
                            *matching_format[lformat].format);
   if (!tex->pta)
     {
        pt_unref(tex->pt);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   pt_link(gc, tex->pta);
   tex->pta->slot = -1;

   evas_gl_common_texture_rgb_a_pair_update(tex, im);
   return tex;
}

void
evas_gl_common_texture_rgb_a_pair_update(Evas_GL_Texture *tex,
                                         RGBA_Image *im)
{
   DATA8 *data1, *data2;
   Eina_Bool comp, upload, subimage = EINA_TRUE;
   int w, h, sz, rowlen, ystep = 1;

   if (!tex->pt) return;

   // Handle compressed formats with 4x4 blocks format
   if (((int) im->cache_entry.space >= (int) EVAS_COLORSPACE_ETC1) &&
       ((int) im->cache_entry.space <= (int) EVAS_COLORSPACE_RGBA_S3TC_DXT5))
     ystep = 4;

   if ((im->cache_entry.space == EVAS_COLORSPACE_ETC1) ||
       (im->cache_entry.space == EVAS_COLORSPACE_ETC1_ALPHA))
     subimage = tex->gc->shared->info.etc1_subimage;

   w = im->cache_entry.w + im->cache_entry.borders.l + im->cache_entry.borders.r;
   h = im->cache_entry.h + im->cache_entry.borders.t + im->cache_entry.borders.b;
   rowlen = _evas_gl_texture_size_get(w, ystep, tex->pt->intformat, NULL);
   sz = _evas_gl_texture_size_get(w, h, tex->pt->intformat, &comp);
   data1 = im->image.data8;
   data2 = data1 + sz;
   upload = !!data1;

   if ((w == tex->pt->w) && (h == tex->pt->h))
     subimage = EINA_FALSE;

   if (!subimage || tex->gc->shared->info.unpack_row_length)
     {
        if (tex->gc->shared->info.unpack_row_length)
          glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          goto on_error;
        if (upload)
          {
             if (comp)
               glCompressedTexImage2D(GL_TEXTURE_2D, 0, tex->pt->intformat, w, h, 0, sz, data1);
             else
               _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, data1);
          }
        glBindTexture(GL_TEXTURE_2D, tex->pta->texture);
        if (!_tex_2d(tex->gc, tex->pta->intformat, w, h, tex->pta->format, tex->pta->dataformat))
          goto on_error;
        if (upload)
          {
             if (comp)
               glCompressedTexImage2D(GL_TEXTURE_2D, 0, tex->pta->intformat, w, h, 0, sz, data2);
             else
               _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pta->format, tex->pta->dataformat, data2);
          }
     }
   else
     {
        int y;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format,
                     tex->pt->dataformat))
          goto on_error;
        if (upload)
          {
             if (w == tex->w)
               {
                  if (comp)
                    _comp_tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, sz, data1);
                  else
                    _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, data1);
               }
             else
               {
                  for (y = 0; y < h; y += ystep)
                    {
                       if (comp)
                         _comp_tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, sz, data1);
                       else
                         _tex_sub_2d(tex->gc, 0, y, w, ystep, tex->pt->format,
                                     tex->pt->dataformat, data1 + rowlen * y / ystep);
                    }
               }
          }

        glBindTexture(GL_TEXTURE_2D, tex->pta->texture);
        if (!_tex_2d(tex->gc, tex->pta->intformat, w, h, tex->pta->format,
                     tex->pta->dataformat))
          goto on_error;
        if (upload)
          {
             if (w == tex->w)
               {
                  if (comp)
                    _comp_tex_sub_2d(tex->gc, 0, 0, w, h, tex->pta->format, sz, data2);
                  else
                    _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pta->format, tex->pta->dataformat, data2);
               }
             else
               {
                  for (y = 0; y < h; y += ystep)
                    {
                       if (comp)
                         _comp_tex_sub_2d(tex->gc, 0, 0, w, h, tex->pta->format, sz, data2);
                       else
                         _tex_sub_2d(tex->gc, 0, y, w, ystep, tex->pta->format,
                                     tex->pta->dataformat, data2 + rowlen * y / ystep);
                    }
               }
          }
     }
on_error:
   glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
}

Evas_GL_Texture *
evas_gl_common_texture_yuv_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h)
{
   Evas_GL_Texture *tex;

   tex = evas_gl_common_texture_alloc(gc, w, h, EINA_FALSE);
   if (!tex) return NULL;

   //////////////////////////////////////////////////////////////////////

   tex->ptu = _pool_tex_new(gc, (w + 1) / 2, (h + 1) / 2, lum_ifmt, lum_fmt);
   if (!tex->ptu)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptu);
   tex->ptu->slot = -1;
   tex->ptu->fslot = -1;
   tex->ptu->whole = 1;

   tex->ptu2 = _pool_tex_new(gc, (w + 1) / 2, (h + 1) / 2, lum_ifmt, lum_fmt);
   if (!tex->ptu2)
     {
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptu2);
   tex->ptu2->slot = -1;
   tex->ptu2->fslot = -1;
   tex->ptu2->whole = 1;

   //////////////////////////////////////////////////////////////////////

   tex->ptv = _pool_tex_new(gc,  tex->ptu->w, tex->ptu->h, lum_ifmt, lum_fmt);
   if (!tex->ptv)
     {
        pt_unref(tex->ptu);
        pt_unref(tex->ptu2);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptv);
   tex->ptv->slot = -1;
   tex->ptv->fslot = -1;
   tex->ptv->whole = 1;

   tex->ptv2 = _pool_tex_new(gc,  tex->ptu->w, tex->ptu->h, lum_ifmt, lum_fmt);
   if (!tex->ptv2)
     {
        pt_unref(tex->ptu);
        pt_unref(tex->ptu2);
        pt_unref(tex->ptv);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptv2);
   tex->ptv2->slot = -1;
   tex->ptv2->fslot = -1;
   tex->ptv2->whole = 1;

   //////////////////////////////////////////////////////////////////////

   tex->pt = _pool_tex_new(gc, tex->ptu->w * 2, tex->ptu->h * 2, lum_ifmt, lum_fmt);
   if (!tex->pt)
     {
        pt_unref(tex->ptu);
        pt_unref(tex->ptu2);
        pt_unref(tex->ptv);
        pt_unref(tex->ptv2);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->pt);
   tex->pt->fslot = -1;
   tex->pt->whole = 1;

   tex->pt2 = _pool_tex_new(gc, tex->ptu->w * 2, tex->ptu->h * 2, lum_ifmt, lum_fmt);
   if (!tex->pt2)
     {
        pt_unref(tex->ptu);
        pt_unref(tex->ptu2);
        pt_unref(tex->ptv);
        pt_unref(tex->ptv2);
        pt_unref(tex->pt);
        evas_gl_common_texture_light_free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->pt2);
   tex->pt2->fslot = -1;
   tex->pt2->whole = 1;

   //////////////////////////////////////////////////////////////////////

   tex->pt->references++;
   tex->ptu->references++;
   tex->ptv->references++;
   tex->pt2->references++;
   tex->ptu2->references++;
   tex->ptv2->references++;
   evas_gl_common_texture_yuv_update(tex, rows, w, h);
   return tex;
}

void
evas_gl_common_texture_yuv_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h)
{
   Evas_GL_Texture_Pool *pt, *ptu, *ptv;

   if (!tex->pt) return;

   pt = tex->pt;
   ptu = tex->ptu;
   ptv = tex->ptv;
   tex->pt = tex->pt2;
   tex->ptu = tex->ptu2;
   tex->ptv = tex->ptv2;
   tex->pt2 = pt;
   tex->ptu2 = ptu;
   tex->ptv2 = ptv;

   // FIXME: works on lowest size 4 pixel high buffers. must also be multiple of 2
   if ((tex->gc->shared->info.unpack_row_length) && (rows[0] && rows[h]))
     {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
        glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + 1] - rows[h]);
        if (!_tex_2d(tex->gc, tex->ptu->intformat, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat, rows[h]);
        glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + (h / 2) + 1] - rows[h + (h / 2)]);
        if (!_tex_2d(tex->gc, tex->ptv->intformat, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2)]);
     }
   else if (rows[0] && rows[h])
     {
        unsigned int y;
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
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
   if (tex->pt->texture != tex->gc->state.current.cur_tex)
     glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
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
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
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

   if (tex->pt->texture != tex->gc->state.current.cur_tex)
     glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
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
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
        if (!_tex_2d(tex->gc, tex->pt->intformat, w, h, tex->pt->format, tex->pt->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
        glBindTexture(GL_TEXTURE_2D, tex->ptuv->texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (rows[h + 1] - rows[h]) / 2);
        if (!_tex_2d(tex->gc, tex->ptuv->intformat, w / 2, h / 2, tex->ptuv->format, tex->ptuv->dataformat))
          return;
        _tex_sub_2d(tex->gc, 0, 0, w / 2, h / 2, tex->ptuv->format, tex->ptuv->dataformat, rows[h]);
     }
   else
     {
        unsigned int y;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
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
   if (tex->pt->texture != tex->gc->state.current.cur_tex)
     glBindTexture(tex->gc->state.current.tex_target, tex->gc->state.current.cur_tex);
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
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);

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
