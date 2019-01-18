#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SHM_OPEN
# include <sys/mman.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>

#include <Eina.h>

#include <poppler-global.h>
#include <poppler-rectangle.h>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>

#include "shmfile.h"
#include "timeout.h"

#define DATA32 unsigned int
typedef char RGB24[3];

//#define PDF_DBG

#ifdef PDF_DBG
#define D(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#define D(fmt, args...)
#endif

using namespace poppler;

bool locked = false;

document *doc;
page *doc_page;
int width = 0, height = 0;
int crop_width = 0, crop_height = 0;
void *data = NULL;
double dpi = -1.0;

#define DEF_DPI 72.0

Eina_Bool poppler_init(const char *file, int page_nbr, int size_w, int size_h)
{
   double w, h, cw, ch;

   if (!file || !*file)
     return EINA_FALSE;

   if (page_nbr < 0)
     return EINA_FALSE;

   if (!eina_init())
     return EINA_FALSE;


   doc = document::load_from_file(file);

   if (!doc)
     return EINA_FALSE;

   if (page_nbr >= doc->pages())
     goto del_pdfdoc;

   /* load the page */

   doc_page = doc->create_page(page_nbr);
   if (!doc_page)
     goto del_pdfdoc;

   w = doc_page->page_rect(media_box).width();
   h = doc_page->page_rect(media_box).height();
   cw = doc_page->page_rect().width();
   ch = doc_page->page_rect().height();

   if (cw > w) cw = w;
   if (ch > h) ch = h;

   if ((size_w > 0) || (size_h > 0))
     {
        double w2 = cw, h2 = ch;

        w2 = size_w;
        h2 = (size_w * ch) / cw;
        if (h2 > size_h)
          {
             h2 = size_h;
             w2 = (size_h * cw) / ch;
          }
        D("XXXXXXXXXXXXXXXXXXXXx %3.3fx%3.3f\n", w2, h2);
        if (w2 > h2) dpi = (w2 * DEF_DPI) / cw;
        else dpi = (h2 * DEF_DPI) / ch;
     }

   if (dpi > 0.0)
     {
        cw = (cw * dpi) / DEF_DPI;
        ch = (ch * dpi) / DEF_DPI;
        w = (w * dpi) / DEF_DPI;
        h = (h * dpi) / DEF_DPI;
     }
   width = w;
   height = h;
   crop_width = cw;
   crop_height = ch;

   return EINA_TRUE;

 del_pdfdoc:
   delete doc;

   return EINA_FALSE;
}

void poppler_shutdown()
{
   delete doc;
   eina_shutdown();
}


void poppler_load_image(int size_w EINA_UNUSED, int size_h EINA_UNUSED)
{
   page_renderer *renderer;
   image out;
   DATA32 *dst;
   int y, x;

   if (dpi <= 0.0) dpi = DEF_DPI;

   renderer = new page_renderer();

   renderer->set_render_hint(page_renderer::text_antialiasing, 1);
   renderer->set_render_hint(page_renderer::antialiasing, 1);

   out = renderer->render_page(doc_page, dpi, dpi,
                               0, 0, width, height,
                               rotate_0);

   shm_alloc(crop_width * crop_height * sizeof(DATA32));
   if (!shm_addr) goto end;
   data = shm_addr;
   dst = (DATA32 *)data;

#define IMAGE_PIXEL_ITERATOR \
   for (y = 0; y < crop_height; y++) \
     for (x = 0; x < crop_width; x++)

#define ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

   if (out.format() == image::format_mono)
     {
        // Looks like this is 1 bit per pixel, padded to a single byte.
        // The problem is testing. I have no sample (and no doc).

        uint8_t *src;
        for (y = 0; y < crop_height; y++)
          {
             src = (uint8_t *) out.data() + y * crop_width;
             for (x = 0; x < crop_width; x++)
               {
                  int bit = x & 0x7;
                  int c = (*src & (1 << bit)) ? 0xFF : 0x00;
                  *dst++ = ARGB_JOIN(0xFF, c, c, c);
                  if ((x & 0x7) == 0x7) src++;
               }
          }
     }
   if (out.format() == image::format_rgb24)
     {
        RGB24 *src;
        src = (RGB24*) out.data();
        IMAGE_PIXEL_ITERATOR
          {
             int pos = x+y*crop_width;
             dst[pos] = ARGB_JOIN(0xFF, src[pos][0], src[pos][1], src[pos][2]);
          }
      }
    else if (out.format() == image::format_argb32)
      {
         DATA32 *src;

         src = (DATA32*) out.data();
         IMAGE_PIXEL_ITERATOR
           *dst++ = *src++;
      }

 end:
   delete renderer;
}

int
main(int argc, char **argv)
{
   Eina_Tmpstr *tmpdir = NULL;
   Eina_Tmpstr *generated = NULL;
   char *extension;
   char *dir;
   char *file;
   int i;
   int size_w = 0, size_h = 0;
   int head_only = 0;
   int page_num = 0;

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
             page_num = atoi(argv[i]);
             i++;
          }
        else if (!strcmp(argv[i], "-opt-scale-down-by"))
          { // not used by pdf loader
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
          { // not used by pdf loader
             i++;
             size_w = atoi(argv[i]);
             i++;
             size_h = atoi(argv[i]);
          }
     }

   D("dpi....: %f\n", dpi);
   D("page...: %d\n", page_num);

   // This is a funny hack to call an external tool to generate a pdf that will then be processed by poppler
   extension = strrchr(file, '.');
   dir = dirname(argv[0]);
   if (extension && dir && strcmp(extension, ".pdf"))
     {
        if (eina_file_mkdtemp("evas_generic_pdf_loaderXXXXXX", &tmpdir))
          {
             Eina_Strbuf *tmp;
             FILE *cmd;

             tmp = eina_strbuf_new();
             eina_strbuf_append_printf(tmp, "%s/evas_generic_pdf_loader.%s '%s' %s", dir, extension + 1, file, tmpdir);

             cmd = popen(eina_strbuf_string_get(tmp), "r");
             D("running preprocessing process '%s'...\n", eina_strbuf_string_get(tmp));
             eina_strbuf_reset(tmp);

             if (cmd)
               {
                  struct stat st;
                  const char *filename;
                  char buf[1024];

                  while (fgets(buf, sizeof (buf), cmd))
                    ;
                  pclose(cmd);

                  filename = basename(file);
                  generated = eina_tmpstr_add_length(filename, strlen(filename) - strlen(extension));

                  eina_strbuf_append_printf(tmp, "%s/%s.pdf", tmpdir, generated);

                  eina_tmpstr_del(generated);
                  generated = NULL;

                  if (stat(eina_strbuf_string_get(tmp), &st) == 0)
                    {
                       generated = eina_tmpstr_add_length(eina_strbuf_string_get(tmp),
                                                          eina_strbuf_length_get(tmp));
                       file = (char*) generated;
                    }
               }

             D("generated file: '%s'\n", generated);
             eina_strbuf_free(tmp);
          }
     }

   // Let's force a timeout if things go wrong
   timeout_init(10);

   // Now process the pdf (or the generated pdf)
   D("poppler_file_init\n");
   if (!poppler_init(file, page_num, size_w, size_h))
     return -1;
   D("poppler_file_init done\n");

   D("dpi2...: %f\n", dpi);
   if (!head_only)
     {
        poppler_load_image(size_w, size_h);
     }

   D("size...: %ix%i\n", width, height);
   D("crop...: %ix%i\n", crop_width, crop_height);
   D("alpha..: 1\n");

   printf("size %i %i\n", crop_width, crop_height);
   printf("alpha 0\n");

   if (!head_only)
     {
#ifdef HAVE_SHM_OPEN
        if (shm_fd >= 0)
          printf("shmfile %s\n", shmfile);
        else
#endif
          {
             // could also to "tmpfile %s\n" like shmfile but just
             // a mmaped tmp file on the system
             printf("data\n");
             if (fwrite(data, crop_width * crop_height * sizeof(DATA32), 1, stdout) != 1)
               {
                  // nothing much to do, the receiver will simply ignore the
                  // data as it's too short
                  D("fwrite failed (%d bytes): %m\n", crop_width * crop_height * sizeof(DATA32));
               }
          }
        shm_free();
     }
   else
     printf("done\n");

   poppler_shutdown();

   if (tmpdir)
     {
        if (generated) unlink(generated);
        if (rmdir(tmpdir) < 0)
          {
             D("Failed to delete tmpdir %s\n", tmpdir);
          }

        eina_tmpstr_del(tmpdir);
        eina_tmpstr_del(generated);
     }
   fflush(stdout);
   return 0;
}
