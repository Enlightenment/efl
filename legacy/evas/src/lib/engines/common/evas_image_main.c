#include "evas_common.h"

static Evas_Hash        * images = NULL;
static Evas_Object_List * cache = NULL;
static int                cache_size = 0;
static int                cache_usage = 0;

#if 0
int
image_debug_hash_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   RGBA_Image *im;
   
   im = data;
   printf("  [%i] %3ix%3i %6i %6i [%2x %2x] %i %s\n",
	  im->references, 
	  im->image->w, im->image->h,
	  im->image->w * im->image->h * 4,
	  evas_common_image_ram_usage(im),
	  im->flags & RGBA_IMAGE_IS_DIRTY,
	  im->flags & RGBA_IMAGE_INDEXED,
	  im->info.format, 
	  im->info.file);
}

static void
image_debug(void)
{
   Evas_Object_List *l;
   
   printf("active images:\n");
   evas_hash_foreach(images, image_debug_hash_cb, NULL);
   printf("cache size: %i\n", cache_size);
   printf("cache usage: %i\n", cache_usage);
   printf("cached images:\n");
   for (l = cache; l; l = l->next)
     {
	RGBA_Image *im;
	
	im = l;
	printf("  [%i] %3ix%3i %6i %6i [%2x %2x] %i %s\n",
	       im->references, 
	       im->image->w, im->image->h,
	       im->image->w * im->image->h * 4,
	       evas_common_image_ram_usage(im),
	       im->flags & RGBA_IMAGE_IS_DIRTY,
	       im->flags & RGBA_IMAGE_INDEXED,
	       im->info.format, 
	       im->info.file);
     }
}
#endif

void
evas_common_image_init(void)
{
}

RGBA_Surface *
evas_common_image_surface_new(void)
{
   RGBA_Surface *is;
   
   is = calloc(1, sizeof(RGBA_Surface));
   return is;
}

void
evas_common_image_surface_free(RGBA_Surface *is)
{
   evas_common_image_surface_dealloc(is);
   free(is);
}

void
evas_common_image_surface_alloc(RGBA_Surface *is)
{
   is->data = malloc(is->w * is->h * sizeof(DATA32));
}

void
evas_common_image_surface_dealloc(RGBA_Surface *is)
{
   if ((is->data) && (!is->no_free))
     {
	free(is->data);
	is->data = NULL;
     }
}

RGBA_Image *
evas_common_image_create(int w, int h)
{
   RGBA_Image *im;
   
   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new();
   if (!im->image) 
     {
	evas_common_image_free(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_free(im);
	return NULL;	
     }
   im->flags = RGBA_IMAGE_IS_DIRTY;
   im->references = 1;
   return im;
}

RGBA_Image *
evas_common_image_new(void)
{
   RGBA_Image *im;
   
   im = calloc(1, sizeof(RGBA_Image));
   if (!im) return NULL;   
   im->flags = RGBA_IMAGE_NOTHING;
   return im;
}

void
evas_common_image_free(RGBA_Image *im)
{
   int i;
   
   if (im->image) evas_common_image_surface_free(im->image);
   for (i = 0; i < im->mipmaps.num; i++)
     {
	if (im->mipmaps.levels[i])
	  evas_common_image_surface_free(im->mipmaps.levels[i]);
     }
   if (im->mipmaps.levels) free(im->mipmaps.levels);
   if (im->info.file) free(im->info.file);
   if (im->info.key) free(im->info.key);
   if (im->info.comment) free(im->info.comment);
   free(im);
}

void
evas_common_image_ref(RGBA_Image *im)
{
   im->references++;
   if (im->references == 1) /* we were in cache - take us out */
     {
	evas_common_image_uncache(im);
	evas_common_image_store(im);
     }
}

void
evas_common_image_unref(RGBA_Image *im)
{
   im->references--;
   if (im->references <= 0) /* we were are now in cache - put us in */
     {
	evas_common_image_unstore(im);
	if ((cache_size > 0) && 
	    (!(im->flags & RGBA_IMAGE_IS_DIRTY)))
	  {
	     evas_common_image_cache(im);
	     evas_common_image_flush_cache();
	  }
	else
	  {
	     evas_common_image_free(im);
	  }
     }
}

void
evas_common_image_cache(RGBA_Image *im)
{
   int ram;

   if (im->flags & RGBA_IMAGE_INDEXED) return;
   im->flags |= RGBA_IMAGE_INDEXED;
   cache = evas_object_list_prepend(cache, im);
   ram = evas_common_image_ram_usage(im);
   cache_usage += ram;
   evas_common_image_flush_cache();
}

void
evas_common_image_uncache(RGBA_Image *im)
{
   int ram;

   if (!(im->flags & RGBA_IMAGE_INDEXED)) return;
   im->flags &= ~RGBA_IMAGE_INDEXED;
   cache = evas_object_list_remove(cache, im);
   ram = evas_common_image_ram_usage(im);
   cache_usage -= ram;
}

void
evas_common_image_flush_cache(void)
{
   Evas_Object_List *l, *l_next;
   
   if (!cache) return;
   if (cache_usage < cache_size) return;
   
   for (l = cache->last; l;)
     {
	RGBA_Image *im;
	
	l_next = l->prev;
	im = (RGBA_Image *)l;
	evas_common_image_uncache(im);
	evas_common_image_free(im);
	if (cache_usage <= cache_size) return;
	l = l_next;
     }   
}

void
evas_common_image_set_cache(int size)
{
   cache_size = size;
   evas_common_image_flush_cache();
}

int
evas_common_image_get_cache(void)
{
   return cache_size;
}

void
evas_common_image_store(RGBA_Image *im)
{
   char *key;
   int l1, l2, l3;
   char buf[256];

   if (im->flags & RGBA_IMAGE_IS_DIRTY) return;
   if (im->flags & RGBA_IMAGE_INDEXED) return;
   if ((!im->info.file) && (!im->info.key)) return;
   l1 = 0;
   if (im->info.file) l1 = strlen(im->info.file);
   l2 = 0;
   if (im->info.key) l2 = strlen(im->info.key);
   snprintf(buf, sizeof(buf), "%llx", im->timestamp);
   l3 = strlen(buf);
   key = malloc(l1 + 3 + l2 + 3 + l3 +1);
   if (!key) return;
   key[0] = 0;
   if (im->info.file) strcpy(key, im->info.file);
   strcat(key, "/:/");
   if (im->info.key) strcat(key, im->info.key);
   strcat(key, "/:/");
   strcat(key, buf);
   images = evas_hash_add(images, key, im);
   free(key);
   im->flags |= RGBA_IMAGE_INDEXED;   
}

void
evas_common_image_unstore(RGBA_Image *im)
{
   char *key;
   int l1, l2, l3;
   char buf[256];

   if (!(im->flags & RGBA_IMAGE_INDEXED)) return;
   if ((!im->info.file) && (!im->info.key)) return;
   l1 = 0;
   if (im->info.file) l1 = strlen(im->info.file);
   l2 = 0;
   if (im->info.key) l2 = strlen(im->info.key);
   snprintf(buf, sizeof(buf), "%llx", im->timestamp);
   l3 = strlen(buf);
   key = malloc(l1 + 3 + l2 + 3 + l3 +1);
   if (!key) return;
   key[0] = 0;
   if (im->info.file) strcpy(key, im->info.file);
   strcat(key, "/:/");
   if (im->info.key) strcat(key, im->info.key);
   strcat(key, "/:/");
   strcat(key, buf);
   images = evas_hash_del(images, key, im);
   free(key);
   im->flags &= ~RGBA_IMAGE_INDEXED;
}


RGBA_Image *
evas_common_image_find(const char *filename, const char *key, DATA64 timestamp)
{
   Evas_Object_List *l;
   RGBA_Image *im;
   char *str;
   int l1, l2, l3;
   char buf[256];
   
   if ((!filename) && (!key)) return NULL;
   l1 = 0;
   if (filename) l1 = strlen(filename);
   l2 = 0;
   if (key) l2 = strlen(key);
   sprintf(buf, "%llx", timestamp);
   l3 = strlen(buf);
   str = malloc(l1 + 3 + l2 + 3 + l3 +1);
   if (!str) return NULL;
   str[0] = 0;
   if (filename) strcpy(str, filename);
   strcat(str, "/:/");
   if (key) strcat(str, key);
   strcat(str, "/:/");
   strcat(str, buf);   
   im = evas_hash_find(images, str);
   free(str);
   if (im) return im;
   
   for (l = cache; l; l = l->next)
     {
	int ok;
	
	im = (RGBA_Image *)l;
	ok = 0;
	if ((filename) && (im->info.file) && 
	    (!strcmp(filename, im->info.file)))
	  ok++;
	if ((!filename) && (!im->info.file))
	  ok++;
	if ((key) && (im->info.key) && 
	    (!strcmp(key, im->info.key)))
	  ok++;
	if ((!key) && (!im->info.key))
	  ok++;
	if (im->timestamp == timestamp)
	  ok++;	
	if (ok >= 3) return im;
     }
   return NULL;
}

int
evas_common_image_ram_usage(RGBA_Image *im)
{
   int ram = 0;
   int i;
   
   ram += sizeof(struct _RGBA_Image);
   if (im->info.file) ram += strlen(im->info.file);
   if (im->info.key) ram += strlen(im->info.key);
   if (im->info.comment) ram += strlen(im->info.comment);
   if ((im->image) && (im->image->data) && (!im->image->no_free))
     ram += im->image->w * im->image->h * sizeof(DATA32);
   ram += im->mipmaps.num * sizeof(RGBA_Surface);
   for (i = 0; i < im->mipmaps.num; i++)
     {
	if ((im->mipmaps.levels[i]) && (im->mipmaps.levels[i]->data) && (!im->mipmaps.levels[i]->no_free))
	  ram += im->mipmaps.levels[i]->w * im->mipmaps.levels[i]->h * sizeof(DATA32);
     }
   return ram;
}

void
evas_common_image_dirty(RGBA_Image *im)
{
   int i;
   
   evas_common_image_unstore(im);
   im->flags |= RGBA_IMAGE_IS_DIRTY;
   for (i = 0; i < im->mipmaps.num; i++)
     evas_common_image_surface_dealloc(im->mipmaps.levels[i]);
}

