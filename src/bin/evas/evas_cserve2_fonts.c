#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef DEBUG_LOAD_TIME
#include <sys/time.h>
#endif

#include <Eet.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SIZES_H
#include FT_TYPES_H
#include FT_MODULE_H
#include FT_OUTLINE_H
#include FT_SYNTHESIS_H

#include "evas_cserve2.h"

#define CACHESIZE 4 * 1024

/* The tangent of the slant angle we do on runtime. This value was
 * retrieved from engines/common/evas_font.h */
#define _EVAS_FONT_SLANT_TAN 0.221694663

#define CHECK_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define MIN_GLYPHS 100 // 26*2 + a nice margin :)
#define MAX_CACHE_SIZE 1 * 1024 * 1024 // 1MB

#define EVAS_FONT_ROUND_26_6_TO_INT(x) \
   (((x + 0x20) & -0x40) >> 6)

static FT_Library cserve2_ft_lib = 0;
static int initialised = 0;

typedef struct _Font_Info Font_Info;
typedef struct _Font_Source_Info Font_Source_Info;

struct _Font_Info
{
   Font_Source_Info *fsi;
   FT_Size size;
   int real_size; // this is probably useless, not used even on client
   int fsize;
   int dpi;
   int max_h;
   unsigned int runtime_rend;
};

struct _Font_Source_Info
{
   FT_Face face;
   int orig_upem;
   int current_size;
   int current_dpi;
   void *data;
   int datasize;
};

static void *
_font_slave_error_send(Error_Type error)
{
   Error_Type *e = calloc(1, sizeof(*e));
   *e = error;

   return e;
}

static void
_font_slave_size_use(Font_Info *fi)
{
   Font_Source_Info *fsi = fi->fsi;
//   if ((fsi->current_size != fi->fsize)
//       || (fsi->current_dpi != fi->dpi))
     {
        FT_Activate_Size(fi->size);
        fsi->current_size = fi->fsize;
        fsi->current_dpi = fi->dpi;
     }
}

static Font_Source_Info *
_font_slave_source_load(const char *file, const char *name)
{
   int error;
   Font_Source_Info *fsi = calloc(1, sizeof(*fsi));

   if (!name)
     {
        error = FT_New_Face(cserve2_ft_lib, file, 0, &(fsi->face));
        if (error)
          {
             ERR("could not open font file: %s", file);
             free(fsi);
             return NULL;
          }
     }
   else
     {
        Eet_File *ef;
        void *fdata;
        int fsize = 0;

        ef = eet_open(file, EET_FILE_MODE_READ);
        if (!ef)
          {
             ERR("failed to read eet: %s", file);
             free(fsi);
             return NULL;
          }
        fdata = eet_read(ef, name, &fsize);
        eet_close(ef);
        if (!fdata)
          {
             ERR("failed to read font from eet: %s:%s", file, name);
             free(fsi);
             return NULL;
          }
        fsi->data = fdata;
        fsi->datasize = fsize;

        error = FT_New_Memory_Face(cserve2_ft_lib, fsi->data, fsi->datasize,
                                   0, &(fsi->face));
        if (error)
          {
             ERR("failed to load font: %s:%s", file, name);
             free(fsi->data);
             free(fsi);
             return NULL;
          }
     }

   error = FT_Select_Charmap(fsi->face, ft_encoding_unicode);
   if (error)
     {
        ERR("could not select unicode charmap for font: %s:%s", file, name);
        FT_Done_Face(fsi->face);
        free(fsi->data);
        free(fsi);
        return NULL;
     }

   fsi->orig_upem = fsi->face->units_per_EM;
   fsi->current_size = 0;
   fsi->current_dpi = 0;

   return fsi;
}

static Font_Info *
_font_slave_int_load(const Slave_Msg_Font_Load *msg, Font_Source_Info *fsi)
{
   int error;
   int val, dv;
   int ret;
   Font_Info *fi = calloc(1, sizeof(*fi));

   error = FT_New_Size(fsi->face, &(fi->size));
   if (!error)
     FT_Activate_Size(fi->size);

   fi->fsize = msg->size;
   fi->dpi = msg->dpi;
   fi->real_size = msg->size * 64;
   fi->fsi = fsi;
   error = FT_Set_Char_Size(fsi->face, 0, fi->real_size, msg->dpi, msg->dpi);
   if (error)
     error = FT_Set_Pixel_Sizes(fsi->face, 0, fi->real_size);

   if (error)
     {
        int i, maxd = 0x7fffffff;
        int chosen_size = 0;
        int chosen_size2 = 0;

        for (i = 0; i < fsi->face->num_fixed_sizes; i++)
          {
             int s, cd;

             s = fsi->face->available_sizes[i].size;
             cd = chosen_size - fi->real_size;
             if (cd < 0) cd = -cd;
             if (cd < maxd)
               {
                  maxd = cd;
                  chosen_size = s;
                  chosen_size2 = fsi->face->available_sizes[i].y_ppem;
                  if (maxd == 0) break;
               }
          }
        fi->real_size = chosen_size;
        error = FT_Set_Pixel_Sizes(fsi->face, 0, fi->real_size);

        if (error)
          {
             error = FT_Set_Char_Size(fsi->face, 0, fi->real_size, fi->dpi, fi->dpi);
             if (error)
               {
                  /* hack around broken fonts */
                  fi->real_size = (chosen_size2 / 64) * 60;
                  error = FT_Set_Char_Size(fsi->face, 0, fi->real_size, fi->dpi, fi->dpi);
                  if (error)
                    {
                       ERR("Could not choose the font size for font: '%s:%s'.",
                           msg->file, msg->name);
                       FT_Done_Size(fi->size);
                       free(fi);
                       return NULL;
                    }
               }
          }
     }

   fi->max_h = 0;
   val = (int)fsi->face->bbox.yMax;
   if (fsi->face->units_per_EM != 0)
     {
        dv = (fsi->orig_upem * 2048) / fsi->face->units_per_EM;
        ret = (val * fsi->face->size->metrics.y_scale) / (dv * dv);
     }
   else ret = val;
   fi->max_h += ret;
   val = -(int)fsi->face->bbox.yMin;
   if (fsi->face->units_per_EM != 0)
     {
        dv = (fsi->orig_upem * 2048) / fsi->face->units_per_EM;
        ret = (val * fsi->face->size->metrics.y_scale) / (dv * dv);
     }
   else ret = val;
   fi->max_h += ret;

   fi->runtime_rend = FONT_REND_REGULAR;
   if ((msg->rend_flags & FONT_REND_SLANT) &&
       !(fsi->face->style_flags & FT_STYLE_FLAG_ITALIC))
     fi->runtime_rend |= FONT_REND_SLANT;
   if ((msg->rend_flags & FONT_REND_WEIGHT) &&
       !(fsi->face->style_flags & FT_STYLE_FLAG_BOLD))
     fi->runtime_rend |= FONT_REND_WEIGHT;

   return fi;
}

static Slave_Msg_Font_Loaded *
_font_slave_load(const void *cmddata, void *data EINA_UNUSED)
{
   const Slave_Msg_Font_Load *msg = cmddata;
   Slave_Msg_Font_Loaded *response;
   Font_Source_Info *fsi;
   Font_Info *fi;

   DBG("slave received FONT_LOAD: '%s'", msg->name);
   fsi = msg->ftdata1;

   /* Loading Font Source */
   if (!fsi)
     fsi = _font_slave_source_load(msg->file, msg->name);

   // FIXME: Return correct error message
   if (!fsi)
     {
        ERR("Could not load font source: '%s'", msg->file);
        return NULL;
     }

   fi = _font_slave_int_load(msg, fsi);
   if (!fi)
     {
        if (!msg->ftdata1)
          cserve2_font_source_ft_free(fsi);
        ERR("Could not load font '%s' from source '%s'", msg->name, msg->file);
        return NULL;
     }

   response = calloc(1, sizeof(*response));
   response->ftdata1 = fsi;
   response->ftdata2 = fi;

   return response;
}

/* This function will load the "index" glyph to the glyph slot of the font.
 * In order to use or render it, one should access it from the glyph slot,
 * or get the glyph using FT_Get_Glyph().
 */
static Eina_Bool
_font_slave_glyph_load(Font_Info *fi, unsigned int idx, unsigned int hint)
{
   Font_Source_Info *fsi = fi->fsi;
   FT_Error error;
   const FT_Int32 hintflags[3] =
     { FT_LOAD_NO_HINTING, FT_LOAD_FORCE_AUTOHINT, FT_LOAD_NO_AUTOHINT };
   static FT_Matrix transform = {0x10000, _EVAS_FONT_SLANT_TAN * 0x10000,
        0x00000, 0x10000};

   error = FT_Load_Glyph(fsi->face, idx,
                         FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP |
                         hintflags[hint]);
   if (error)
     {
        ERR("Could not load glyph %d", idx);
        return EINA_FALSE;
     }

   /* Transform the outline of Glyph according to runtime_rend. */
   if (fi->runtime_rend & FONT_REND_SLANT)
     FT_Outline_Transform(&fsi->face->glyph->outline, &transform);
   /* Embolden the outline of Glyph according to rundtime_rend. */
   if (fi->runtime_rend & FONT_REND_WEIGHT)
      FT_GlyphSlot_Embolden(fsi->face->glyph);

   return EINA_TRUE;
}

// import the 1 func we need
EAPI void *evas_common_font_glyph_compress(void *data, int num_grays, int pixel_mode, int pitch_data, int w, int h, int *size_ret);

/* This function will render the glyph currently in the glyph slot into the
 * given Font Cache.
 */
static Eina_Bool
_font_slave_glyph_render(Font_Info *fi, Slave_Msg_Font_Glyphs_Loaded *response,
                         unsigned int idx)
{
   Font_Source_Info *fsi = fi->fsi;
   int glyphsize = 0;
   FT_Glyph glyph;
   FT_BitmapGlyph bglyph;
   char *data;
   int buffer_id = 0;
   Eina_Bool valid = EINA_FALSE;
   void *buf;

   FT_Get_Glyph(fsi->face->glyph, &glyph);
   FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
   bglyph = (FT_BitmapGlyph)glyph;

   if ((bglyph->bitmap.pitch < 1) || (bglyph->bitmap.rows < 1))
     {
        FT_Done_Glyph(glyph);
        goto on_error;
     }
   glyphsize = bglyph->bitmap.pitch * bglyph->bitmap.rows;

   valid = EINA_TRUE;
   buf = evas_common_font_glyph_compress(bglyph->bitmap.buffer,
                                         bglyph->bitmap.num_grays,
                                         bglyph->bitmap.pixel_mode,
                                         bglyph->bitmap.pitch,
                                         bglyph->bitmap.width,
                                         bglyph->bitmap.rows,
                                         &glyphsize);
   if (!buf)
     {
        FT_Done_Glyph(glyph);
        goto on_error;
     }

   buffer_id = cserve2_shared_mempool_buffer_new(response->mempool, glyphsize);
   data = cserve2_shared_mempool_buffer_get(response->mempool, buffer_id);
   if (!data)
     {
        free(buf);
        FT_Done_Glyph(glyph);
        goto on_error;
     }
   memcpy(data, buf, glyphsize);
   free(buf);

   // TODO: Check if we have problems with alignment
   response->glyphs[response->nglyphs].index = idx;
   response->glyphs[response->nglyphs].buffer_id = buffer_id;
   response->glyphs[response->nglyphs].offset =
         cserve2_shared_mempool_buffer_offset_get(response->mempool, buffer_id);
   response->glyphs[response->nglyphs].size = glyphsize;
   response->glyphs[response->nglyphs].rows = bglyph->bitmap.rows;
   response->glyphs[response->nglyphs].width = bglyph->bitmap.width;
   response->glyphs[response->nglyphs].pitch = bglyph->bitmap.pitch;
   response->nglyphs++;

   FT_Done_Glyph(glyph);

   return EINA_TRUE;

on_error:
   // Create invalid entry for this index. There will be an empty slot in
   // the mempool (usually 8 bytes) because we need the Glyph_Data index entry.
   if (valid)
     ERR("Failed to load glyph %d. Running out of memory?", idx);
   else
     WRN("Could not load glyph %d. Creating empty invalid entry.", idx);
   memset(&response->glyphs[response->nglyphs], 0, sizeof(Slave_Msg_Glyph));
   if (buffer_id > 0)
     cserve2_shared_mempool_buffer_del(response->mempool, buffer_id);
   buffer_id = cserve2_shared_mempool_buffer_new(response->mempool, 1);
   response->glyphs[response->nglyphs].index = idx;
   response->glyphs[response->nglyphs].buffer_id = buffer_id;
   response->nglyphs++;
   return EINA_FALSE;
}

static void
_font_slave_int_metrics_get(Font_Info *fi, unsigned int hint, unsigned int c, int *width, int *height, int *depth)
{
   unsigned int idx;
   Font_Source_Info *fsi = fi->fsi;
   FT_BBox outbox;
   FT_Glyph glyph;

   idx = FT_Get_Char_Index(fsi->face, c);
   if (!idx)
     goto end;

   if (!_font_slave_glyph_load(fi, idx, hint))
     goto end;

   FT_Get_Glyph(fsi->face->glyph, &glyph);
   FT_Glyph_Get_CBox(glyph,
                     ((hint == 0) ? FT_GLYPH_BBOX_UNSCALED :
                      FT_GLYPH_BBOX_GRIDFIT),
                     &outbox);
   if (width) *width = EVAS_FONT_ROUND_26_6_TO_INT(outbox.xMax - outbox.xMin);
   if (height)
     *height = EVAS_FONT_ROUND_26_6_TO_INT(outbox.yMax - outbox.xMin);
   if (depth) *depth = 1; // FIXME: Do we need to check this?
   FT_Done_Glyph(glyph);
   return;

end:
   if (width) *width = 0;
   if (height) *height = 0;
   if (depth) *depth = 0;
}

static unsigned int
_font_slave_int_shm_calculate(Font_Info *fi, unsigned int hint)
{
   const char *c;
   int i;
   int size = 0;
   int average;

   for (c = CHECK_CHARS, i = 0; *c != '\0'; c++, i++)
     {
        int w, h, depth;
        _font_slave_int_metrics_get(fi, hint, *c, &w, &h, &depth);
        size += w * h * depth;
     }

   average = size / i; // average glyph size
   size = MIN_GLYPHS * average;

   size = cserve2_shm_size_normalize(size, 0);

   if (size > MAX_CACHE_SIZE)
     return MAX_CACHE_SIZE; // Assumes no glyph will be bigger than this

   return size;
}

#ifdef DEBUG_LOAD_TIME
static int
_timeval_sub(const struct timeval *tv2, const struct timeval *tv1)
{
    int t1, t2;

    t1 = tv1->tv_usec + tv1->tv_sec * 1000000;
    t2 = tv2->tv_usec + tv2->tv_sec * 1000000;

    if (t2 > t1)
      return t2 - t1;

    return 0;
}
#endif

static Slave_Msg_Font_Glyphs_Loaded *
_font_slave_glyphs_load(const void *cmddata, void *data EINA_UNUSED)
{
   const Slave_Msg_Font_Glyphs_Load *msg = cmddata;
   Slave_Msg_Font_Glyphs_Loaded *response;
   Font_Info *fi;
   unsigned int i;
#ifdef DEBUG_LOAD_TIME
   unsigned int gl_load_time = 0;
   unsigned int gl_render_time = 0;
   struct timeval tv_start, tv_end;
   struct timeval rstart, rfinish;
#endif

   fi = msg->font.ftdata2;

#ifdef DEBUG_LOAD_TIME
   gettimeofday(&rstart, NULL);
#endif

   _font_slave_size_use(fi);

   response = malloc(sizeof(*response)
                     + sizeof(Slave_Msg_Glyph) * (msg->glyphs.nglyphs));
   if (!response) return NULL;

   response->nglyphs = 0;
   response->glyphs = (void *) (response + 1);
   response->mempool = msg->cache.mempool;
   if (!response->mempool)
     {
        unsigned shmsize = _font_slave_int_shm_calculate(fi, msg->font.hint);
        response->mempool = cserve2_shared_mempool_new(GLYPH_DATA_ARRAY_TAG,
                                                       sizeof(Glyph_Data), 0,
                                                       shmsize);
        if (!response->mempool) return NULL;
     }

   for (i = 0; i < msg->glyphs.nglyphs; i++)
     {
#ifdef DEBUG_LOAD_TIME
        gettimeofday(&tv_start, NULL);
#endif
        if (_font_slave_glyph_load(fi, msg->glyphs.glyphs[i], msg->font.hint))
          {
#ifdef DEBUG_LOAD_TIME
             gettimeofday(&tv_end, NULL);
             gl_load_time += _timeval_sub(&tv_end, &tv_start);
             // copy the time that we got here to be used as start of render
             tv_start.tv_sec = tv_end.tv_sec;
             tv_start.tv_usec = tv_end.tv_usec;
#endif
             _font_slave_glyph_render(fi, response, msg->glyphs.glyphs[i]);
#ifdef DEBUG_LOAD_TIME
             gettimeofday(&tv_end, NULL);
             gl_render_time += _timeval_sub(&tv_end, &tv_start);
#endif
          }
     }

#ifdef DEBUG_LOAD_TIME
   response->gl_load_time = gl_load_time;
   response->gl_render_time = gl_render_time;

   gettimeofday(&rfinish, NULL);
   response->gl_slave_time = _timeval_sub(&rfinish, &rstart);
#endif

   return response;
}

void *
cserve2_font_slave_cb(Slave_Thread_Data *sd EINA_UNUSED, Slave_Command *cmd, const void *cmddata, void *data)
{
   void *response = NULL;

   switch (*cmd)
     {
      case FONT_LOAD:
         response = _font_slave_load(cmddata, data);
         break;
      case FONT_GLYPHS_LOAD:
         response = _font_slave_glyphs_load(cmddata, data);
         break;
      default:
         ERR("Invalid command for font slave: %d", *cmd);
         *cmd = ERROR;
         return _font_slave_error_send(CSERVE2_INVALID_COMMAND);
     }

   if (!response)
     {
        *cmd = ERROR;
        return _font_slave_error_send(CSERVE2_GENERIC);
     }

   return response;
}

void
cserve2_font_init(void)
{
   int error;

   if (initialised++ > 0) return;

   error = FT_Init_FreeType(&cserve2_ft_lib);
   if (error) return;

   eet_init();
}

void
cserve2_font_shutdown(void)
{
   initialised--;
   if (initialised > 0) return;
   if (initialised < 0)
     {
        ERR("Invalid shutdown of cserve2 font.");
        return;
     }

   FT_Done_FreeType(cserve2_ft_lib);
   cserve2_ft_lib = 0;

   eet_shutdown();
}

void
cserve2_font_source_ft_free(void *fontsource)
{
   Font_Source_Info *fsi = fontsource;

   if (!fsi) return;

   FT_Done_Face(fsi->face);
   free(fsi->data);
   free(fsi);
}

void
cserve2_font_ft_free(void *fontinfo)
{
   Font_Info *fi = fontinfo;

   if (!fi) return;

   FT_Done_Size(fi->size);
   free(fi);
}
