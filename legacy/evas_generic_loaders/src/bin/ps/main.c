#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/mman.h>
#include <fcntl.h>

#include <libspectre/spectre.h>

#include <Eina.h>

#include "shmfile.h"
#include "timeout.h"

#define DATA32  unsigned int

#define PS_DBG

#ifdef PS_DBG
#define D(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#define D(fmt, args...)
#endif


SpectreDocument *psdoc;
int page_count;

SpectrePage *page;

int width = 0;
int height = 0;
void *data = NULL;
double dpi = -1.0;

#define DEF_DPI 72.0

static Eina_Bool
_spectre_init(const char *file, int page_nbr, int size_w, int size_h)
{
   double w, h;
   int ww, hh;
   SpectreOrientation rot;
   SpectreStatus status;

   if (!file || !*file)
     return EINA_FALSE;

   if (page_nbr < 0)
     return EINA_FALSE;

   if (!eina_init())
     return EINA_FALSE;

   psdoc = spectre_document_new();
   if (!psdoc)
     goto shutdown_eina;

   spectre_document_load(psdoc, file);
   status = spectre_document_status (psdoc);
   if (status != SPECTRE_STATUS_SUCCESS)
     {
        D("[ps] %s\n", spectre_status_to_string(status));
        goto free_psdoc;
   }

   page_count = spectre_document_get_n_pages(psdoc);
   status = spectre_document_status(psdoc);
   if (status != SPECTRE_STATUS_SUCCESS)
     {
        D("[eps] %s\n", spectre_status_to_string (status));
        goto free_psdoc;
   }

   if (page_nbr >= page_count)
     goto free_psdoc;

   /* load the page */

   page = spectre_document_get_page(psdoc, page_nbr);
   status = spectre_document_status(psdoc);
   if (status != SPECTRE_STATUS_SUCCESS)
     {
        D("[eps] %s\n", spectre_status_to_string (status));
        goto free_page;
   }

   spectre_page_get_size(page, &ww, &hh);
   w = ww;
   h = hh;
   rot = spectre_page_get_orientation(page);

   if ((rot == SPECTRE_ORIENTATION_LANDSCAPE) || (rot == SPECTRE_ORIENTATION_REVERSE_LANDSCAPE))
     {
        double t;
        // swap width & height
        t = w; w = h; h = t;
     }

   if ((size_w > 0) || (size_h > 0))
     {
        double w2 = w, h2 = h;

        w2 = size_w;
        h2 = (size_w * h) / w;
        if (h2 > size_h)
          {
             h2 = size_h;
             w2 = (size_h * w) / h;
          }
        D("XXXXXXXXXXXXXXXXXXXXx %3.3fx%3.3f\n", w2, h2);
        if (w2 > h2) dpi = (w2 * DEF_DPI) / w;
        else dpi = (h2 * DEF_DPI) / h;
     }

   if (dpi > 0.0)
     {
        w = (w * dpi) / DEF_DPI;
        h = (h * dpi) / DEF_DPI;
     }
   width = w;
   height = h;

   return EINA_TRUE;

 free_page:
   spectre_page_free(page);
 free_psdoc:
   spectre_document_free(psdoc);
 shutdown_eina:
   eina_shutdown();

   return EINA_FALSE;
}

static void
_spectre_shutdown()
{
   spectre_page_free(page);
   spectre_document_free(psdoc);
   eina_shutdown();
}

static void
_pixcopy(DATA32 *dst, unsigned char *src, int size)
{
   DATA32 *d;
   unsigned char *s, *e;

   d = dst;
   s = src;
   e = s + size;
   while (s < e)
     {
        d[0] = 
           0xff000000 |
           (s[2] << 16) |
           (s[1] << 8 ) |
           (s[0]      );
        d++;
        s += 4;
     }
}

static void
_spectre_load_image(int size_w EINA_UNUSED, int size_h EINA_UNUSED)
{
   SpectreRenderContext *rc;
   unsigned char        *psdata;
   int                   stride;
   unsigned char        *src;
   DATA32               *dst;
   int                   yy;
   SpectreStatus         status;

   rc = spectre_render_context_new();
   if (!rc)
     return;

   spectre_page_render(page, rc, &psdata, &stride);
   spectre_render_context_set_page_size (rc, width, height);
   status = spectre_page_status(page);
   if (status != SPECTRE_STATUS_SUCCESS)
     {
        D("[eps] %s\n", spectre_status_to_string (status));
        return;
     }

   shm_alloc(width * height * sizeof(DATA32));
   if (!shm_addr) return;
   data = shm_addr;

   if (stride == 4 * width)
     _pixcopy(data, psdata, height * stride);
   else
     {
        src = psdata;
        dst = (DATA32 *)data;
        for (yy = 0; yy < height; src += stride, dst += width, ++yy)
          _pixcopy (dst, src, width * 4);
     }

   spectre_render_context_free(rc);
}

int
main(int argc, char **argv)
{
   char *file;
   int i;
   int size_w = 0, size_h = 0;
   int head_only = 0;
   int page_nbr = 0;

   if (argc < 2) return -1;
   // file is ALWAYS first arg, other options come after
   file = argv[1];
   for (i = 2; i < argc; i++)
     {
        if      (!strcmp(argv[i], "-head"))
           // asked to only load header, not body/data
           head_only = 1;
        else if (!strcmp(argv[i], "-key"))
          {
             i++;
             page_nbr = atoi(argv[i]);
             i++;
          }
        else if (!strcmp(argv[i], "-opt-scale-down-by"))
          { // not used by ps loader
             i++;
             // int scale_down = atoi(argv[i]);
          }
        else if (!strcmp(argv[i], "-opt-dpi"))
          {
             i++;
             dpi = ((double)atoi(argv[i])) / 1000.0; // dpi is an int multiplied by 1000 (so 72dpi is 72000)
             i++;
          }
        else if (!strcmp(argv[i], "-opt-size"))
          { // not used by ps loader
             i++;
             size_w = atoi(argv[i]);
             i++;
             size_h = atoi(argv[i]);
          }
     }

   D("_spectre_init_file\n");
   D("dpi....: %f\n", dpi);
   D("page...: %d\n", page_nbr);

   timeout_init(10);
   
   if (!_spectre_init(file, page_nbr, size_w, size_h))
     return -1;
   D("_spectre_init done\n");

   D("dpi2...: %f\n", dpi);
   if (!head_only)
     {
        _spectre_load_image(size_w, size_h);
     }

   D("size...: %ix%i\n", width, height);
   D("alpha..: 0\n");

   printf("size %i %i\n", width, height);
   printf("alpha 0\n");

   if (!head_only)
     {
        if (shm_fd >= 0) printf("shmfile %s\n", shmfile);
        else
          {
             // could also to "tmpfile %s\n" like shmfile but just
             // a mmaped tmp file on the system
             printf("data\n");
             fwrite(data, width * height * sizeof(DATA32), 1, stdout);
          }
        shm_free();
     }
   else
     printf("done\n");

   _spectre_shutdown();

   return 0;
}
