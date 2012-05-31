#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SIZES_H
#include FT_MODULE_H

#include "evas_cserve2.h"

static FT_Library cserve2_ft_lib = 0;
static int initialised = 0;

struct _Font_Info
{
   FT_Size size;
   int real_size;
   int max_h;
   unsigned int runtime_rend;
};

struct _Font_Source_Info
{
   FT_Face face;
   int orig_upem;
   int current_size;
};

typedef struct _Font_Info Font_Info;
typedef struct _Font_Source_Info Font_Source_Info;

static Font_Source_Info *
_font_slave_source_load(const char *file)
{
   int error;
   Font_Source_Info *fsi = malloc(sizeof(*fsi));

   error = FT_New_Face(cserve2_ft_lib, file, 0, &(fsi->face));
   if (error)
     {
        free(fsi);
        return NULL;
     }

   error = FT_Select_Charmap(fsi->face, ft_encoding_unicode);
   if (error)
     {
        FT_Done_Face(fsi->face);
        free(fsi);
        return NULL;
     }

   fsi->orig_upem = fsi->face->units_per_EM;
   fsi->current_size = 0;

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

   fi->real_size = msg->size * 64;
   error = FT_Set_Char_Size(fsi->face, 0, fi->real_size, msg->dpi, msg->dpi);
   if (error)
     {
        fi->real_size = msg->size;
        error = FT_Set_Pixel_Sizes(fsi->face, 0, fi->real_size);
     }

   if (error)
     {
        int i;
        int chosen_size = 0;
        int chosen_width = 0;

        for (i = 0; i < fsi->face->num_fixed_sizes; i++)
          {
             int s;
             int d, cd;

             s = fsi->face->available_sizes[i].height;
             cd = chosen_size - msg->size;
             if (cd < 0) cd = -cd;
             d = s - msg->size;
             if (d < 0) d = -d;
             if (d < cd)
               {
                  chosen_width = fsi->face->available_sizes[i].width;
                  chosen_size = s;
               }
             if (d == 0) break;
          }
        fi->real_size = chosen_size;
        error = FT_Set_Pixel_Sizes(fsi->face, chosen_width, fi->real_size);

        if (error)
          ERR("Could not choose the font size.");
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
_font_slave_load(const void *cmddata, void *data __UNUSED__)
{
   const Slave_Msg_Font_Load *msg = cmddata;
   Slave_Msg_Font_Loaded *response;
   Font_Source_Info *fsi;
   Font_Info *fi;

   fsi = msg->ftdata1;

   /* Loading Font Source */
   if (!fsi)
     fsi = _font_slave_source_load(msg->file);

   // FIXME: Return correct error message
   if (!fsi)
     return NULL;

   fi = _font_slave_int_load(msg, fsi);

   response = calloc(1, sizeof(*response));
   response->ftdata1 = fsi;
   response->ftdata2 = fi;

   return response;
}

void *
cserve2_font_slave_cb(Slave_Thread_Data *sd __UNUSED__, Slave_Command cmd, const void *cmddata, void *data)
{
   void *response = NULL;

   switch (cmd)
     {
      case FONT_LOAD:
         _font_slave_load(cmddata, data);
         break;
      case FONT_GLYPHS_LOAD:
         // command for FONT_GLYPHS_LOAD
         break;
      default:
         ERR("Invalid command for font slave: %d", cmd);
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
}
