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
int width = 0;
int height = 0;
void *data;

static int shm_fd = -1;
static int shm_size = 0;
static void *shm_addr = NULL;
static char shmfile[1024] = "";

static void
shm_alloc(int dsize)
{
#ifdef HAVE_SHM_OPEN
   srand(time(NULL));
   do
     {
        snprintf(shmfile, sizeof(shmfile), "/evas-loader-xcf.%i.%i",
                 (int)getpid(), (int)rand());
        shm_fd = shm_open(shmfile, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
     }
   while (shm_fd < 0);

   if (ftruncate(shm_fd, dsize) < 0)
     {
        close(shm_fd);
        shm_unlink(shmfile);
        shm_fd = -1;
	goto failed;
     }
   shm_addr = mmap(NULL, dsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
   if (shm_addr == MAP_FAILED)
     {
        close(shm_fd);
        shm_unlink(shmfile);
        shm_fd = -1;
        goto failed;
     }
   shm_size = dsize;
   return;
failed:
#endif
   shm_addr = malloc(dsize);
}

static void
shm_free(void)
{
#ifdef HAVE_SHM_OPEN
   if (shm_fd >= 0)
     {
        munmap(shm_addr, shm_size);
        close(shm_fd);
        shm_fd = -1;
        shm_addr = NULL;
        return;
     }
#endif
   free(shm_addr);
   shm_addr = NULL;
   shm_fd = -1;
}

Eina_Bool poppler_init(const char *file, int page_nbr, double dpi, int size_w, int size_h)
{
   Object obj;
   SplashColor white;

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

   width = page->getMediaWidth();
   height = page->getMediaHeight();

   if ((size_w > 0) || (size_h > 0))
     {
        /* FIXME: tell poller to render at the new width and height
        unsigned int w2 = width, h2 = height;
        if (size_w > 0)
          {
             w2 = size_w;
             h2 = (size_w * h) / w;
             if ((size_h > 0) && (h2 > size_h))
               {
                  unsigned int w3;
                  h2 = size_h;
                  w3 = (size_h * w) / h;
                  if (w3 > w2)
                     w2 = w3;
               }
          }
        else if (size_h > 0)
          {
             h2 = size_h;
             w2 = (size_h * w) / h;
          }
        width = w2;
        height = h2;
         */
     }
   else if (dpi > 0.0)
     {
        /* FIXME: tell poppler to render at this size
        width = (width * dpi) / 72.0;
        height = (height * dpi) / 72.0;
         */
     }

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

void poppler_load_image(double dpi, int size_w, int size_h)
{
   SplashOutputDev *output_dev;
   SplashColor      white;
   SplashColorPtr   color_ptr;

   white[0] = 255;
   white[1] = 255;
   white[2] = 255;
   white[3] = 255;

   output_dev = new SplashOutputDev(splashModeXBGR8, 4, gFalse, white);
   if (!output_dev)
     return;

   output_dev->startDoc(pdfdoc->getXRef());

   if (dpi <= 0.0) dpi = 72.0;

   page->display(output_dev,
                 dpi, dpi, 0,
                 false, false, false,
                 pdfdoc->getCatalog());
   color_ptr = output_dev->getBitmap()->getDataPtr();

   shm_alloc(width * height * sizeof(DATA32));
   if (!shm_addr)
     goto del_outpput_dev;
   data = shm_addr;
   memcpy(data, color_ptr, width * height * sizeof(DATA32));

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
   double dpi = -1.0;

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

   if (!poppler_init(file, page, dpi, size_w, size_h))
     return -1;
   D("poppler_file_init done\n");

   if (!head_only)
     {
        poppler_load_image(dpi, size_w, size_h);
     }

   D("size...: %ix%i\n", width, height);
   D("alpha..: 1\n");

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

   poppler_shutdown();

   return 0;
}
