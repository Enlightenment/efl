#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libraw.h>
#include "shmfile.h"
#include "timeout.h"

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <Eina.h>


#define DATA32 unsigned int
#define DATA8 unsigned char

#define ARGB_JOIN(a,r,g,b) \
   (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

//#define RAW_DBG 1

#ifdef RAW_DBG
#define D(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#define D(fmt, args...)
#endif

static int fd = -1;
static int seg_size = 0;
static unsigned char *seg = MAP_FAILED;
static libraw_data_t *raw_data = NULL;
static void *data = NULL;
static int width = 0;
static int height = 0;

static int
_raw_init(const char *file)
{
   struct stat ss;
   fd = open(file, O_RDONLY);
   if (fd < 0) return EINA_FALSE;

   if (stat(file, &ss)) goto close_file;
   seg_size = ss.st_size;
   seg = mmap(0, seg_size, PROT_READ, MAP_SHARED, fd, 0);
   if (seg == MAP_FAILED) goto close_file;

   D("raw_init\n");
   raw_data = libraw_init(0);
   raw_data->params.half_size = 0;
   raw_data->params.user_qual = 2;

   D("raw_open_buffer\n");
   if (libraw_open_buffer(raw_data, seg, seg_size) != LIBRAW_SUCCESS)
     return EINA_FALSE;
   return EINA_TRUE;

close_file:
   close(fd);
   return EINA_FALSE;
}

static void
_raw_shutdown()
{
   D("raw_shutdown\n");
   if (raw_data)
     libraw_close(raw_data);
   if (seg != MAP_FAILED) munmap(seg, seg_size);
   close(fd);
}

static int
read_raw_header()
{
   int ret;

   D("raw_open_buffer\n");
   if ((ret = libraw_open_buffer(raw_data, seg, seg_size)) != LIBRAW_SUCCESS)
     return 0;

   D("raw_adjust_size\n");
   if ((ret = libraw_adjust_sizes_info_only(raw_data)) != LIBRAW_SUCCESS)
     {
        if (LIBRAW_FATAL_ERROR(ret))
          return 0;
     }

   if ((raw_data->sizes.width < 1) || (raw_data->sizes.height < 1))
     return 0;

   width = raw_data->sizes.iwidth;
   height = raw_data->sizes.iheight;

   return 1;

}


static int
read_raw_data()
{
   int ret;
   unsigned int count;
   libraw_processed_image_t *image = NULL;
   DATA8 *bufptr;
   DATA32 *dataptr;


   D("raw_open_unpack\n");
   if ((ret = libraw_unpack(raw_data)) != LIBRAW_SUCCESS)
     return 0;

   D("raw_dcraw_process\n");
   if ((ret = libraw_dcraw_process(raw_data)) != LIBRAW_SUCCESS)
     {
        if (LIBRAW_FATAL_ERROR(ret))
          return 0;;
     }

   D("raw_make_mem_image\n");
   image = libraw_dcraw_make_mem_image(raw_data, &ret);
   if (image)
     {
        if ((image->width < 1) || (image->height < 1))
          goto clean_image;
        width = image->width;
        height = image->height;
        if (image->type != LIBRAW_IMAGE_BITMAP)
          goto clean_image;
        if (image->colors != 3)
          goto clean_image;
#define SWAP(a, b) { a ^= b; a = (b ^=a); }
        if ((image->bits == 16) && (htons(0x55aa) != 0x55aa))
          for (count = 0; count < image->data_size; count +=2)
            SWAP(image->data[count], image->data[count + 1]);
#undef SWAP
        shm_alloc(image->width * image->height * (sizeof(DATA32)));
        if (!shm_addr)
          goto clean_image;
        data = shm_addr;
        memset(shm_addr, 0, image->width * image->height * (sizeof(DATA32)));
        dataptr = data;
        bufptr = image->data;
        for (count = image->width * image->height; count > 0; --count)
          {
             *dataptr = ARGB_JOIN(0xff, bufptr[0], bufptr[1], bufptr[2]);
             dataptr++;
             bufptr += 3;
          }

        free(image);
     }
   return 1;

clean_image:
   free(image);
   return 0;
}


int main(int argc, char **argv)
{
   char *file;
   int i;
   int head_only = 0;

   if (argc < 2) return -1;
   file = argv[1];

   for (i = 2; i < argc; ++i)
     {
        if (!strcmp(argv[i], "-head"))
          head_only = 1;
        else if (!strcmp(argv[i], "-key"))
          { // not used by raw loader
             i++;
             // const char *key = argv[i];
          }
        else if (!strcmp(argv[i], "-opt-scale-down-by"))
          { // not used by raw loader
             i++;
             // int scale_down = atoi(argv[i]);
          }
        else if (!strcmp(argv[i], "-opt-dpi"))
          { // not used by raw loader
             i++;
             // double dpi = ((double)atoi(argv[i])) / 1000.0;
          }
        else if (!strcmp(argv[i], "-opt-size"))
          { // not used by raw loader
             i++;
             // int size_w = atoi(argv[i]);
             i++;
             // int size_h = atoi(argv[i]);
          }
     }

   timeout_init(4);
   
   if (!_raw_init(file)) return -1;
   if (head_only != 0)
     {
        if (read_raw_header())
          {
             printf("size %d %d\n", width, height);
             printf("alpha 1\n");
          }
        printf("done\n");
     }
   else
     {
        if (read_raw_data())
          {
             printf("size %d %d\n", width, height);
             printf("alpha 1\n");
             if (shm_fd >= 0) printf("shmfile %s\n", shmfile);
             else
               {
                  printf("data\n");
                  fwrite(data, width * height * sizeof(DATA32), 1, stdout);
               }
             shm_free();
          }
     }
   _raw_shutdown();
   return 0;

}

