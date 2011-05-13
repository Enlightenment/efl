#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/mman.h>
#include <fcntl.h>

#include <GlobalParams.h>
#include <PDFDoc.h>
#include <ErrorCodes.h>
#include <Page.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>

#include <Eina.h>

#include "shmfile.h"

#define DATA32  unsigned int

//#define PDF_DBG

#ifdef PDF_DBG
#define D(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#define D(fmt, args...)
#endif


PDFDoc *pdfdoc;
bool locked = false;
SplashOutputDev *output_dev;

::Page *page;
int width = 0, height = 0;
int crop_width = 0, crop_height = 0;
void *data = NULL;
double dpi = -1.0;

#define DEF_DPI 72.0

Eina_Bool poppler_init(const char *file, int page_nbr, int size_w, int size_h)
{
   Object obj;
   SplashColor white;
   double w, h, cw, ch;
   int rot;

   if (!file || !*file)
     return EINA_FALSE;

   if (page_nbr < 0)
     return EINA_FALSE;

   if (!(globalParams = new GlobalParams()))
     return EINA_FALSE;

   if (!eina_init())
     goto del_global_param;

   if (globalParams->getAntialias())
     globalParams->setAntialias((char *)"yes");
   if (globalParams->getVectorAntialias())
     globalParams->setVectorAntialias((char *)"yes");

   pdfdoc = new PDFDoc(new GooString(file), NULL);
   if (!pdfdoc)
     goto del_global_param;

   if (!pdfdoc->isOk() || (pdfdoc->getErrorCode() == errEncrypted))
     goto del_pdfdoc;

   if (page_nbr >= pdfdoc->getNumPages())
     goto del_pdfdoc;

   /* load the page */

   page = pdfdoc->getCatalog()->getPage(page_nbr + 1);
   if (!page || !page->isOk())
     goto del_pdfdoc;

   w = page->getMediaWidth();
   h = page->getMediaHeight();
   cw = page->getCropWidth();
   ch = page->getCropHeight();
   rot = page->getRotate();
   if (cw > w) cw = w;
   if (ch > h) ch = h;
   if ((rot == 90) || (rot == 270))
     {
        double t;
        // swap width & height
        t = w; w = h; h = t;
        // swap crop width & height
        t = cw; cw = ch; ch = t;
     }
   
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
   delete pdfdoc;
 del_global_param:
   delete globalParams;

   return EINA_FALSE;
}

void poppler_shutdown()
{
   delete pdfdoc;
   eina_shutdown();
   delete globalParams;
}

void poppler_load_image(int size_w, int size_h)
{
   SplashOutputDev *output_dev;
   SplashColor      white;
   SplashColorPtr   color_ptr;
   DATA32          *src, *dst;
   int              y;

   white[0] = 255;
   white[1] = 255;
   white[2] = 255;
   white[3] = 255;

   output_dev = new SplashOutputDev(splashModeXBGR8, 4, gFalse, white);
   if (!output_dev)
     return;

   output_dev->startDoc(pdfdoc->getXRef());

   if (dpi <= 0.0) dpi = DEF_DPI;

   page->displaySlice(output_dev, dpi, dpi, 
                      0, false, false,
                      0, 0, width, height,
                      false, pdfdoc->getCatalog());
   color_ptr = output_dev->getBitmap()->getDataPtr();

   shm_alloc(crop_width * crop_height * sizeof(DATA32));
   if (!shm_addr) goto del_outpput_dev;
   data = shm_addr;
   src = (DATA32 *)color_ptr;
   dst = (DATA32 *)data;
   for (y = 0; y < crop_height; y++)
     {
        memcpy(dst, src, crop_width * sizeof(DATA32));
        src += width;
        dst += crop_width;
     }

 del_outpput_dev:
   delete output_dev;
}

int
main(int argc, char **argv)
{
   char *file;
   int i;
   int size_w = 0, size_h = 0;
   int head_only = 0;
   int page = 0;

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
             page = atoi(argv[i]);
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

   D("poppler_file_init\n");
   D("dpi....: %f\n", dpi);
   D("page...: %d\n", page);

   if (!poppler_init(file, page, size_w, size_h))
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
        if (shm_fd >= 0) printf("shmfile %s\n", shmfile);
        else
          {
             // could also to "tmpfile %s\n" like shmfile but just
             // a mmaped tmp file on the system
             printf("data\n");
             fwrite(data, crop_width * crop_height * sizeof(DATA32), 1, stdout);
          }
        shm_free();
     }
   else
     printf("done\n");

   poppler_shutdown();

   return 0;
}
