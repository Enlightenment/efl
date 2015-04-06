/*
 * A small binary utility for packing basic data and images into eet files.
 * Can be used in combination with the "eet" utility and can even be used
 * to stuff more data into edj files (which are eet). Run tool for help.
 */
#include <Eina.h>
#include <Eet.h>
#include <Evas.h>
#include <Ecore_Evas.h>

static const char *output = NULL;
static Eet_File *ef = NULL;
static Ecore_Evas *ee = NULL;
static Evas *evas = NULL;
static Evas_Object *im_obj = NULL;

static void
file_add(void)
{
   if (ef) return;
   ef = eet_open(output, EET_FILE_MODE_READ_WRITE);
   if (!ef) ef = eet_open(output, EET_FILE_MODE_WRITE);
   if (!ef)
     {
        printf("ERROR: Cannot open %s for reading or writing!\n", output);
        exit(-1);
     }
}

static void
file_del(void)
{
   if (ef) return;
   ef = eet_open(output, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     {
        printf("ERROR: Cannot open %s for writing!\n", output);
        exit(-1);
     }
}

static void
check_argc(int argc, int i)
{
   if (i >= argc)
     {
        printf("ERROR: ran out of arguments at argument #%i\n", i);
        exit(-1);
     }
}

static void
commands(int start, int argc, char **argv)
{
   int i;
   const char *key, *file, *mode;
   void *pixels, *data;
   int w, h, alpha, compress, quality;
   Eet_Image_Encoding lossy;
   Eina_File *f;
   size_t size;

   for (i = start; i < argc;)
     {
        if (!strcmp(argv[i], "+"))
          {
             i++; check_argc(argc, i);
             key = argv[i];
             i++; check_argc(argc, i);
             file_add();
             if (!strcmp(argv[i], "im"))
               {
                  i++; check_argc(argc, i);
                  file = argv[i];
                  i++; check_argc(argc, i);
                  mode = argv[i];
                  evas_object_image_file_set(im_obj, file, NULL);
                  if (evas_object_image_load_error_get(im_obj) != EVAS_LOAD_ERROR_NONE)
                    {
                       printf("ERROR: cannot load image file '%s'\n", file);
                       exit(-1);
                    }
                  pixels = evas_object_image_data_get(im_obj, EINA_FALSE);
                  if (!pixels)
                    {
                       printf("ERROR: cannot get pixel data for file '%s'\n", file);
                       exit(-1);
                    }
                  evas_object_image_size_get(im_obj, &w, &h);
                  if ((w <= 0)  || (h <= 0))
                    {
                       printf("ERROR: width or height <= 0 for file '%s'\n", file);
                       exit(-1);
                    }
                  alpha = evas_object_image_alpha_get(im_obj);
                  quality = 0;
                  compress = 0;
                  lossy = EET_IMAGE_LOSSLESS;
                  if (!strcmp(mode, "none"))
                    {
                       lossy = EET_IMAGE_LOSSLESS;
                       compress = EET_COMPRESSION_NONE;
                    }
                  else if (!strcmp(mode, "lo"))
                    {
                       lossy = EET_IMAGE_LOSSLESS;
                       compress = EET_COMPRESSION_LOW;
                    }
                  else if (!strcmp(mode, "med"))
                    {
                       lossy = EET_IMAGE_LOSSLESS;
                       compress = EET_COMPRESSION_MED;
                    }
                  else if (!strcmp(mode, "hi"))
                    {
                       lossy = EET_IMAGE_LOSSLESS;
                       compress = EET_COMPRESSION_HI;
                    }
                  else if (!strcmp(mode, "fast"))
                    {
                       lossy = EET_IMAGE_LOSSLESS;
                       compress = EET_COMPRESSION_VERYFAST;
                    }
                  else if (!strcmp(mode, "super"))
                    {
                       lossy = EET_IMAGE_LOSSLESS;
                       compress = EET_COMPRESSION_SUPERFAST;
                    }
                  else if (!strcmp(mode, "etc1"))
                    {
                       if (alpha) lossy = EET_IMAGE_ETC1_ALPHA;
                       else lossy = EET_IMAGE_ETC1;
                       compress = EET_COMPRESSION_SUPERFAST;
                    }
                  else if (!strcmp(mode, "etc2"))
                    {
                       if (alpha) lossy = EET_IMAGE_ETC2_RGBA;
                       else lossy = EET_IMAGE_ETC2_RGB;
                       compress = EET_COMPRESSION_SUPERFAST;
                    }
                  else
                    {
                       quality = atoi(mode);
                       lossy = EET_IMAGE_JPEG;
                    }
                  if (eet_data_image_write(ef, key, pixels, w, h, alpha,
                                           compress, quality, lossy) <= 0)
                    {
                       printf("ERROR: cannot encode file '%s' in key '%s'\n", file, key);
                       exit(-1);
                    }
               }
             else if (!strcmp(argv[i], "data"))
               {
                  i++; check_argc(argc, i);
                  file = argv[i];
                  i++; check_argc(argc, i);
                  mode = argv[i];
                  f = eina_file_open(file, EINA_FALSE);
                  if (!f)
                    {
                       printf("ERROR: cannot open file '%s'\n", file);
                       exit(-1);
                    }
                  size = eina_file_size_get(f);
                  if (size == 0)
                    {
                       printf("ERROR: file '%s' is zero sized\n", file);
                       exit(-1);
                    }
                  if (size >= 0x7f000000)
                    {
                       printf("ERROR: file '%s' is too big (a bit under 2GB max)\n", file);
                       exit(-1);
                    }
                  data = eina_file_map_all(f, EINA_FILE_POPULATE);
                  if (!data)
                    {
                       printf("ERROR: cannot mmap file '%s'\n", file);
                       exit(-1);
                    }
                  compress = 0;
                  if (!strcmp(mode, "none"))
                    {
                       compress = EET_COMPRESSION_NONE;
                    }
                  else if (!strcmp(mode, "lo"))
                    {
                       compress = EET_COMPRESSION_LOW;
                    }
                  else if (!strcmp(mode, "med"))
                    {
                       compress = EET_COMPRESSION_MED;
                    }
                  else if (!strcmp(mode, "hi"))
                    {
                       compress = EET_COMPRESSION_HI;
                    }
                  else if (!strcmp(mode, "fast"))
                    {
                       compress = EET_COMPRESSION_VERYFAST;
                    }
                  else if (!strcmp(mode, "super"))
                    {
                       compress = EET_COMPRESSION_SUPERFAST;
                    }
                  else
                    {
                       printf("ERROR: invalid compress mode '%s' for file '%s'\n", mode, file);
                       exit(-1);
                    }
                  if (eet_write(ef, key, data, size, compress) <= 0)
                    {
                       printf("ERROR: cannot encode file '%s' in key '%s'\n", file, key);
                       exit(-1);
                    }
                  eina_file_map_free(f, data);
                  eina_file_close(f);
               }
             i++;
          }
        else if (!strcmp(argv[i], "-"))
          {
             i++; check_argc(argc, i);
             key = argv[i];
             file_del();
             eet_delete(ef, key);
             i++;
          }
        else
          {
             printf("invalid argument #%i of '%s'\n", i, argv[i]);
             exit(-1);
          }
     }
}

static void
scratch_canvas_init(void)
{
   ee = ecore_evas_buffer_new(1, 1);
   if (!ee)
     {
        printf("ERROR: cannot create buffer canvas!\n");
        exit(-1);
     }
   evas = ecore_evas_get(ee);
   im_obj = evas_object_image_add(evas);
}

int
main(int argc, char **argv)
{
   if (argc <= 1)
     {
        printf
        ("USAGE: \n"
         "  eetpack outputfile.eet [commands]\n"
         "WHERE commands are a series of one or more of:\n"
         "  + KEY im IMG-FILE [none|lo|med|hi|fast|super|etc1|etc2|0-100]\n"
         "  + KEY data DATA-FILE [none|lo|med|hi|fast|super]\n"
         "  - KEY\n"
         "\n"
         "e.g.\n"
         "add 2 images and a text file to the archive:\n"
         "  eetpack archive.eet \\\n"
         "    + mykey1 im image.png fast \\\n"
         "    + mykey2 im logo.png 70 \\\n"
         "    + mydata1 data file.txt hi \\\n"
         "\n"
         "remove keys from an archive:\n"
         "  eetpack archive.eet - mykey2 - mydata1\n"
        );

        return -1;
     }
   eina_init();
   eet_init();
   evas_init();
   ecore_evas_init();

   scratch_canvas_init();
   output = argv[1];
   commands(2, argc, argv);
   if (ef) eet_close(ef);

   ecore_evas_shutdown();
   evas_shutdown();
   eet_shutdown();
   eina_shutdown();
   return 0;
}
