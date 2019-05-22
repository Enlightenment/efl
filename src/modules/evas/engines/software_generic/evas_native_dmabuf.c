#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_native_common.h"

#if defined HAVE_DLSYM && ! defined _WIN32
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#elif _WIN32
# include <evil_private.h> /* dlopen dlclose dlsym */
#else
# warning native_dmabuf should not get compiled if dlsym is not found on the system!
#endif

#include <sys/mman.h>

#define DRM_FORMAT_ARGB8888           0x34325241
#define DRM_FORMAT_XRGB8888           0x34325258

static void
_native_bind_cb(void *image, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   struct dmabuf_attributes *a;
   int size;
   RGBA_Image *im = image;

   if (!im) return;
   Native *n = im->native.data;
   if (!n) return;
   if (n->ns.type != EVAS_NATIVE_SURFACE_WL_DMABUF)
     return;

   if (im->image.data) return;

   a = (struct dmabuf_attributes *)&n->ns_data.wl_surface_dmabuf;
   if (n->ns_data.wl_surface_dmabuf.ptr)
     {
        im->image.data = n->ns_data.wl_surface_dmabuf.ptr;
        return;
     }
   size = a->height * a->stride[0];
   im->image.data = mmap(NULL, size, PROT_READ, MAP_SHARED, a->fd[0], 0);
   if (im->image.data == MAP_FAILED) im->image.data = NULL;
   n->ns_data.wl_surface_dmabuf.size = size;
   n->ns_data.wl_surface_dmabuf.ptr = im->image.data;
}

static void
_native_unbind_cb(void *image)
{
   RGBA_Image *im = image;

   if (!im) return;
   Native *n = im->native.data;
   if (!n) return;
   if (n->ns.type != EVAS_NATIVE_SURFACE_WL_DMABUF)
     return;
}

static void
_native_free_cb(void *image)
{
   RGBA_Image *im = image;

   if (!im) return;
   Native *n = im->native.data;

   if (im->image.data)
     munmap(n->ns_data.wl_surface_dmabuf.ptr,
            n->ns_data.wl_surface_dmabuf.size);

   im->native.data        = NULL;
   im->native.func.bind   = NULL;
   im->native.func.unbind = NULL;
   im->native.func.free   = NULL;
   im->image.data         = NULL;

   free(n);
}

void *
_evas_native_dmabuf_surface_image_set(void *image, void *native)
{
   Evas_Native_Surface *ns = native;
   RGBA_Image *im = image;

   if (!im)
     {
        if (ns && ns->type == EVAS_NATIVE_SURFACE_WL_DMABUF &&
            !ns->data.wl_dmabuf.resource)
          {
             struct dmabuf_attributes *attr;

             attr = ns->data.wl_dmabuf.attr;
             if (attr->version != EVAS_DMABUF_ATTRIBUTE_VERSION)
               return NULL;
             if (attr->n_planes != 1)
               return NULL;
             if (attr->format != DRM_FORMAT_ARGB8888 &&
                 attr->format != DRM_FORMAT_XRGB8888)
               return NULL;

             return evas_cache_image_data(evas_common_image_cache_get(),
                                          attr->width, attr->height,
                                          NULL, 1,
                                          EVAS_COLORSPACE_ARGB8888);
          }
       return NULL;
     }

   if (ns)
     {
        struct dmabuf_attributes *a;
        int h, stride;
        int32_t format;
        Native *n;

        if (ns->type != EVAS_NATIVE_SURFACE_WL_DMABUF)
          return NULL;

        n = im->native.data;
        if (n)
           {
              if (n->ns_data.wl_surface_dmabuf.ptr)
                {
                   munmap(n->ns_data.wl_surface_dmabuf.ptr,
                          n->ns_data.wl_surface_dmabuf.size);
                   n->ns_data.wl_surface_dmabuf.size = 0;
                   n->ns_data.wl_surface_dmabuf.ptr = NULL;
                }
              free(im->native.data);
           }
        n = calloc(1, sizeof(Native));
        if (!n) return NULL;

        a = ns->data.wl_dmabuf.attr;
        if (a->version != EVAS_DMABUF_ATTRIBUTE_VERSION)
          {
             free(n);
             return NULL;
          }

        h = a->height;
        stride = a->stride[0];
        format = a->format;
        im->cache_entry.w = stride;
        im->cache_entry.h = h;

        /* This block assumes single planar formats, which are all we
         * currently support. */
        im->cache_entry.w = stride / 4;
        evas_cache_image_colorspace(&im->cache_entry, EVAS_COLORSPACE_ARGB8888);
        im->cache_entry.flags.alpha = (format == DRM_FORMAT_XRGB8888 ? 0 : 1);
        im->image.data = NULL;;
        im->image.no_free = 1;

        memcpy(n, ns, sizeof(Evas_Native_Surface));
        memcpy(&n->ns_data.wl_surface_dmabuf.attr, a, sizeof(*a));
        im->native.data = n;
        im->native.func.bind   = _native_bind_cb;
        im->native.func.unbind = _native_unbind_cb;
        im->native.func.free   = _native_free_cb;
     }

   return im;
}
