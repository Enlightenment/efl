#ifndef EVAS_CS2_PRIVATE_H
#define EVAS_CS2_PRIVATE_H 1

#include "evas_common_private.h"
#include "evas_cs2.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#define SHARED_BUFFER_PATH_MAX 64
typedef struct _Data_Entry Data_Entry;
typedef struct _Font_Entry Font_Entry;
typedef struct _Index_Table Index_Table;
typedef struct _Shared_Index Shared_Index;
typedef struct _Shared_Buffer Shared_Buffer;

struct _Data_Entry
{
   unsigned int image_id;
   unsigned int hit_count;
   void (*preloaded_cb)(void *, Eina_Bool);
   struct {
      int mmap_offset;
      int use_offset;
      int mmap_size;
      int image_size;
      Eina_File *f;
      void *data;
   } shm;
};

struct _Shared_Index
{
   char path[SHARED_BUFFER_PATH_MAX];
   int generation_id;
   Eina_File *f;
   union
   {
      const Shared_Array_Header *header;
      char *data;
   };
   union
   {
      const char *p; // Random access
      const Index_Entry *index;
      const Image_Data *idata;
      const File_Data *filedata;
      const Font_Data *fontdata;
      const Glyph_Data *gldata;
   } entries;
   int count;
   Eina_Hash *entries_by_hkey;
   int last_entry_in_hash;
};

struct _Shared_Buffer
{
   char path[SHARED_BUFFER_PATH_MAX];
   Eina_File *f;
   char *data;
   int size;
   EINA_REFCOUNT;
};

struct _Index_Table
{
   int generation_id;
   Shared_Buffer strings_entries;
   Shared_Index strings_index;
   Shared_Index files;
   Shared_Index images;
   Shared_Index fonts;
};

int evas_cserve2_init(void);
int evas_cserve2_shutdown(void);
EAPI int evas_cserve2_use_get(void);
Eina_Bool evas_cserve2_image_load(Image_Entry *ie);
int evas_cserve2_image_load_wait(Image_Entry *ie);
Eina_Bool evas_cserve2_image_data_load(Image_Entry *ie);
int evas_cserve2_image_load_data_wait(Image_Entry *ie);
void evas_cserve2_image_free(Image_Entry *ie);
void evas_cserve2_image_unload(Image_Entry *ie);
Eina_Bool evas_cserve2_image_preload(Image_Entry *ie, void (*preloaded_cb)(void *im, Eina_Bool success));
void evas_cserve2_dispatch(void);

void *evas_cserve2_image_data_get(Image_Entry *ie);
unsigned int evas_cserve2_image_hit(Image_Entry *ie);

Font_Entry *evas_cserve2_font_load(const char *source, const char *name, int size, int dpi, Font_Rend_Flags wanted_rend);
EAPI int evas_cserve2_font_load_wait(Font_Entry *fe);
void evas_cserve2_font_free(Font_Entry *fe);
Eina_Bool evas_cserve2_font_glyph_request(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints);
Eina_Bool evas_cserve2_font_glyph_used(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints) EINA_WARN_UNUSED_RESULT;
RGBA_Font_Glyph_Out *evas_cserve2_font_glyph_bitmap_get(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints);
void evas_cserve2_font_glyph_ref(RGBA_Font_Glyph_Out *glyph, Eina_Bool incref);
#endif
