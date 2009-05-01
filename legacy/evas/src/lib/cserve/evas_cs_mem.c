#include "evas_cs.h"

#ifdef EVAS_CSERVE

EAPI Mem *
evas_cserve_mem_new(int size, const char *name)
{
   Mem *m;
   static int id = 0;
   char buf[PATH_MAX];
   
   m = calloc(1, sizeof(Mem));
   if (!m) return NULL;
   if (name)
     snprintf(buf, sizeof(buf), "/evas-shm-%x.%s", getuid(), name);
   else
     {
        id++;
        snprintf(buf, sizeof(buf), "/evas-shm-%x.%x.%x", getuid(), getpid(), id);
     }
   m->id = id;
   m->offset = 0;
   m->name = strdup(buf);
   if (!m->name)
     {
        free(m);
        return NULL;
     }
   m->size = size;
   m->fd = shm_open(m->name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   if (m->fd < 0)
     {
        free(m->name);
        free(m);
        return NULL;
     }
   if (ftruncate(m->fd, m->size) < 0)
     {
        shm_unlink(m->name);
        close(m->fd);
        free(m->name);
        free(m);
        return NULL;
     }
   m->data = mmap(NULL, m->size, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
   if (m->data == MAP_FAILED)
     {
        shm_unlink(m->name);
        close(m->fd);
        free(m->name);
        free(m);
        return NULL;
     }
   m->ref = 1;
   m->write = 1;
   return m;
}

EAPI void
evas_cserve_mem_free(Mem *m)
{
   shm_unlink(m->name);
   munmap(m->data, m->size);
   close(m->fd);
   free(m->name);
   free(m);
}

EAPI Mem *
evas_cserve_mem_open(int pid, int id, const char *name, int size, int write)
{
   Mem *m;
   char buf[PATH_MAX];
   
   m = calloc(1, sizeof(Mem));
   if (!m) return NULL;
   if (name)
     snprintf(buf, sizeof(buf), "/evas-shm-%x.%s", getuid(), name);
   else
     snprintf(buf, sizeof(buf), "/evas-shm-%x.%x.%x", getuid(), pid, id);
   m->name = strdup(buf);
   if (!m->name)
     {
        free(m);
        return NULL;
     }
   m->size = size;
   if (write)
     m->fd = shm_open(m->name, O_RDWR, S_IRUSR | S_IWUSR);
   else
     m->fd = shm_open(m->name, O_RDONLY, S_IRUSR);
   if (m->fd < 0)
     {
        free(m->name);
        free(m);
        return NULL;
     }
   m->write = write;
   if (write)
     m->data = mmap(NULL, m->size, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
   else
     m->data = mmap(NULL, m->size, PROT_READ, MAP_SHARED, m->fd, 0);
   if (m->data == MAP_FAILED)
     {
        close(m->fd);
        free(m->name);
        free(m);
        return NULL;
     }
   m->ref = 1;
   return m;
}

EAPI void
evas_cserve_mem_close(Mem *m)
{
   munmap(m->data, m->size);
   close(m->fd);
   free(m->name);
   free(m);
}

EAPI Eina_Bool
evas_cserve_mem_resize(Mem *m, int size)
{
   if (m->write)
     {
        if (ftruncate(m->fd, size) < 0) return 0;
        munmap(m->data, m->size);
        m->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
     }
   else
     {
        munmap(m->data, m->size);
        m->data = mmap(NULL, size, PROT_READ, MAP_SHARED, m->fd, 0);
     }
   if (m->data == MAP_FAILED)
     {
        m->data = NULL;
        return 0;
     }
   m->size = size;
   return 1;
}

EAPI void
evas_cserve_mem_del(int pid, int id)
{
   char buf[PATH_MAX];
   
   snprintf(buf, sizeof(buf), "/evas-shm-%x.%x.%x", getuid(), pid, id);
   shm_unlink(buf);
}

#endif
