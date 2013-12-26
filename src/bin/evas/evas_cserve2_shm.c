#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_cserve2.h"

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct _Shm_Mapping
{
   const char *name;
   size_t length;
   Eina_Inlist *segments;
};

typedef struct _Shm_Mapping Shm_Mapping;

struct _Shm_Handle
{
   EINA_INLIST;
   Shm_Mapping *mapping;
   off_t map_offset;
   off_t image_offset;
   size_t map_size;
   size_t image_size;
   int refcount;
   int shmid;
   void *data;
};

static int id = 0;

size_t
cserve2_shm_size_normalize(size_t size, size_t align)
{
   long pagesize;
   size_t normalized;

   pagesize = eina_cpu_page_size();

   if (align)
     align = ((align + pagesize - 1) / pagesize) * pagesize;
   else
     align = pagesize;
   normalized = ((size + align - 1) / align) * align;

   return normalized;
}

Shm_Handle *
cserve2_shm_request(const char *infix, size_t size)
{
   Shm_Mapping *map;
   Shm_Handle *shm;
   char shmname[NAME_MAX];
   size_t map_size;
   int fd;

   map = calloc(1, sizeof(Shm_Mapping));
   if (!map)
     {
        ERR("Failed to allocate mapping handler.");
        return NULL;
     }

   shm = calloc(1, sizeof(Shm_Handle));
   if (!shm)
     {
        ERR("Failed to allocate shared memory handler.");
        free(map);
        return NULL;
     }

   do {
        snprintf(shmname, sizeof(shmname), "/evas-shm-%05d-%05d-%s-%08x",
                 (int) getuid(), (int) getpid(), infix, ++id);
        fd = shm_open(shmname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd == -1 && errno != EEXIST)
          {
             ERR("Failed to create shared memory object '%s': %m", shmname);
             free(map);
             free(shm);
             return NULL;
          }
   } while (fd == -1);

   map_size = cserve2_shm_size_normalize(size, 0);

   if (ftruncate(fd, map_size) == -1)
     {
        ERR("Failed to set size of shared file: %m");
        close(fd);
        free(map);
        free(shm);
        return NULL;
     }
   close(fd);

   map->name = eina_stringshare_add(shmname);
   map->length = map_size;

   map->segments = eina_inlist_append(map->segments, EINA_INLIST_GET(shm));
   shm->mapping = map;
   shm->map_offset = 0;
   shm->image_offset = 0;

   shm->image_size = size;
   shm->map_size = map_size;
   shm->shmid = id;

   return shm;
}

Shm_Handle *
cserve2_shm_segment_request(Shm_Handle *shm, size_t size)
{
   Shm_Handle *segment;
   size_t map_size;
   Shm_Mapping *map = shm->mapping;
   int fd;

   segment = calloc(1, sizeof (Shm_Handle));
   if (!segment) return NULL;

   fd = shm_open(map->name, O_RDWR, S_IRUSR | S_IWUSR);
   if (fd == -1)
     {
        ERR("Could not reopen shm handle: %m");
        free(segment);
        return NULL;
     }

   map_size  = cserve2_shm_size_normalize(size, 0);
   map_size += map->length;

   if (ftruncate(fd, map_size) == -1)
     {
        ERR("Could not set the size of the shm: %m");
        close(fd);
        free(segment);
        return NULL;
     }
   close(fd);

   segment->mapping    = map;
   segment->map_offset = map->length;
   segment->map_size   = map_size - map->length;
   segment->image_size = size;
   segment->image_offset = segment->map_offset;
   map->length = map_size;
   map->segments = eina_inlist_append(map->segments, EINA_INLIST_GET(segment));

   return segment;
}

Shm_Handle *
cserve2_shm_resize(Shm_Handle *shm, size_t newsize)
{
   size_t map_size;
   int fd;

   if (!shm)
     return NULL;

   if (shm->map_offset || shm->image_offset)
     {
        CRI("Can not resize shm with non-zero offset");
        return NULL;
     }

   if (eina_inlist_count(shm->mapping->segments) > 1)
     {
        CRI("Can not resize shm with more than one segment");
        return NULL;
     }

   fd = shm_open(shm->mapping->name, O_RDWR, S_IRUSR | S_IWUSR);
   if (fd == -1)
     {
        ERR("Could not reopen shm handle: %m");
        return NULL;
     }

   map_size = cserve2_shm_size_normalize(newsize, 0);
   if (ftruncate(fd, map_size))
     {
        ERR("Could not set the size of the shm: %m");
        close(fd);
        return NULL;
     }

   if (shm->data)
     {
        munmap(shm->data, shm->image_size);
        shm->data = mmap(NULL, shm->image_size, PROT_WRITE, MAP_SHARED,
                         fd, shm->image_offset);
     }
   close(fd);

   shm->map_size = map_size;
   shm->image_size = newsize;
   shm->mapping->length = map_size;

   return shm;
}

void
cserve2_shm_unref(Shm_Handle *shm)
{
   Shm_Mapping *map = shm->mapping;

   map->segments = eina_inlist_remove(map->segments, EINA_INLIST_GET(shm));

   if (shm->data)
     munmap(shm->data, shm->image_size);
   free(shm);

   if (map->segments)
     return;

   shm_unlink(map->name);
   eina_stringshare_del(map->name);
   free(map);
}

const char *
cserve2_shm_name_get(const Shm_Handle *shm)
{
   return shm->mapping->name;
}

int
cserve2_shm_id_get(const Shm_Handle *shm)
{
   return shm->shmid;
}

off_t
cserve2_shm_map_offset_get(const Shm_Handle *shm)
{
   return shm->map_offset;
}

off_t
cserve2_shm_offset_get(const Shm_Handle *shm)
{
   return shm->image_offset;
}

size_t
cserve2_shm_map_size_get(const Shm_Handle *shm)
{
   return shm->map_size;
}

size_t
cserve2_shm_size_get(const Shm_Handle *shm)
{
   return shm->image_size;
}

void *
cserve2_shm_map(Shm_Handle *shm)
{
   int fd;
   const char *name;

   if (shm->refcount++)
     return shm->data;

   name = cserve2_shm_name_get(shm);

   fd = shm_open(name, O_RDWR, S_IWUSR);
   if (fd == -1)
     return MAP_FAILED;

   shm->data = mmap(NULL, shm->image_size, PROT_WRITE, MAP_SHARED,
                    fd, shm->image_offset);

   close(fd);

   return shm->data;
}

void
cserve2_shm_unmap(Shm_Handle *shm)
{
   if (--shm->refcount)
     return;

   munmap(shm->data, shm->image_size);
   shm->data = NULL;
}

static void
_cserve2_shm_cleanup()
{
   Eina_Iterator *iter;
   const Eina_File_Direct_Info *f_info;
   char pattern[NAME_MAX];

   sprintf(pattern, "evas-shm-%05d-", (int) getuid());
   iter = eina_file_direct_ls("/dev/shm");
   EINA_ITERATOR_FOREACH(iter, f_info)
     {
        if (strstr(f_info->path, pattern))
          {
             const char *shmname = strrchr(f_info->path, '/');
             if (!shmname) continue;

             if (shm_unlink(shmname) == -1)
               ERR("Failed to remove shm entry at %s: %d %s", shmname, errno, strerror(errno));
             else
               DBG("cserve2 cleanup: removed %s", shmname);
          }
        else
          DBG("cserve2 cleanup: ignoring %s", f_info->path);
     }
   eina_iterator_free(iter);
}

void
cserve2_shm_init()
{
   _cserve2_shm_cleanup();
}

void
cserve2_shm_shutdown()
{
   _cserve2_shm_cleanup();
}
