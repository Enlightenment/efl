/* THIS FILE TO BE SHARED WITH THE BIN PART. KEEP IT CLEAN. THERE BE DRAGONS */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <Eina.h>

#include "evas_cs2_utils.h"

/* fash */
typedef struct _Fash_Glyph_Map  Fash_Glyph_Map;
typedef struct _Fash_Glyph_Map2 Fash_Glyph_Map2;

struct _Fash_Glyph_Map
{
   void *item[256];
};

struct _Fash_Glyph_Map2
{
   Fash_Glyph_Map *bucket[256];
};

struct _Fash_Glyph
{
   Fash_Glyph_Map2 *bucket[256];
   void (*free_cb)(void *glyph);
};

static void
_fash_item_free(Fash_Glyph *fash, Fash_Glyph_Map *map)
{
   int i;

   if (fash->free_cb)
     for (i = 0; i < 256; i++)
       if (map->item[i])
         fash->free_cb(map->item[i]);
   free(map);
}

static void
_fash_gl2_free(Fash_Glyph *fash, Fash_Glyph_Map2 *fash2)
{
   int i;

   for (i = 0; i < 256; i++)
     if (fash2->bucket[i]) _fash_item_free(fash, fash2->bucket[i]);
   free(fash2);
}

void
fash_gl_free(Fash_Glyph *fash)
{
   int i;

   for (i = 0; i < 256; i++)
     if (fash->bucket[i]) _fash_gl2_free(fash, fash->bucket[i]);
   free(fash);
}

Fash_Glyph *
fash_gl_new(void (*free_cb)(void *glyph))
{
   Fash_Glyph *fash = calloc(1, sizeof(Fash_Glyph));
   fash->free_cb = free_cb;
   return fash;
}

void *
fash_gl_find(Fash_Glyph *fash, int item)
{
   int grp, maj, min;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp]) return NULL;
   if (!fash->bucket[grp]->bucket[maj]) return NULL;
   return fash->bucket[grp]->bucket[maj]->item[min];
}

void
fash_gl_add(Fash_Glyph *fash, int item, void *glyph)
{
   int grp, maj, min;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp])
     fash->bucket[grp] = calloc(1, sizeof(Fash_Glyph_Map2));
   EINA_SAFETY_ON_NULL_RETURN(fash->bucket[grp]);
   if (!fash->bucket[grp]->bucket[maj])
     fash->bucket[grp]->bucket[maj] = calloc(1, sizeof(Fash_Glyph_Map));
   EINA_SAFETY_ON_NULL_RETURN(fash->bucket[grp]->bucket[maj]);
   fash->bucket[grp]->bucket[maj]->item[min] = glyph;
}

void
fash_gl_del(Fash_Glyph *fash, int item)
{
   int grp, maj, min;
   void *data;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp]) return;
   if (!fash->bucket[grp]->bucket[maj]) return;
   if (!fash->bucket[grp]->bucket[maj]->item[min]) return;

   data = fash->bucket[grp]->bucket[maj]->item[min];
   fash->free_cb(data);
   fash->bucket[grp]->bucket[maj]->item[min] = NULL;
}
