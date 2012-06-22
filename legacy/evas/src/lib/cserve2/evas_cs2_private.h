#ifndef EVAS_CS2_PRIVATE_H
#define EVAS_CS2_PRIVATE_H 1

#include "evas_common.h"

struct _Data_Entry {
   unsigned int image_id;
   void (*preloaded_cb)(void *, Eina_Bool);
   struct {
      const char *path;
      int mmap_offset;
      int use_offset;
      int mmap_size;
      int image_size;
      Eina_File *f;
      void *data;
   } shm;
};

typedef struct _Data_Entry Data_Entry;
typedef struct _Font_Entry Font_Entry;

int evas_cserve2_init(void);
int evas_cserve2_shutdown(void);
EAPI int evas_cserve2_use_get(void);
Eina_Bool evas_cserve2_image_load(Image_Entry *ie, const char *file, const char *key, RGBA_Image_Loadopts *lopt);
int evas_cserve2_image_load_wait(Image_Entry *ie);
Eina_Bool evas_cserve2_image_data_load(Image_Entry *ie);
void evas_cserve2_image_load_data_wait(Image_Entry *ie);
void evas_cserve2_image_free(Image_Entry *ie);
void evas_cserve2_image_unload(Image_Entry *ie);
Eina_Bool evas_cserve2_image_preload(Image_Entry *ie, void (*preloaded_cb)(void *im, Eina_Bool success));
void evas_cserve2_dispatch(void);

void *evas_cserve2_image_data_get(Image_Entry *ie);

Font_Entry *evas_cserve2_font_load(const char *source, const char *name, int size, int dpi, Font_Rend_Flags wanted_rend);
void evas_cserve2_font_free(Font_Entry *fe);
Eina_Bool evas_cserve2_font_glyph_request(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints);
Eina_Bool evas_cserve2_font_glyph_used(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints);
RGBA_Font_Glyph_Out *evas_cserve2_font_glyph_bitmap_get(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints);
#endif
