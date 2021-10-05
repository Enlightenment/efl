#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "shmfile.h"
#include "timeout.h"

#include <Eina.h>

#include <librsvg/rsvg.h>
#ifndef LIBRSVG_CHECK_VERSION
# include <librsvg/librsvg-features.h>
#endif
#if LIBRSVG_CHECK_VERSION(2,36,2)
#else
# include <librsvg/rsvg-cairo.h>
#endif

#define DATA32 unsigned int

static RsvgHandle *rsvg = NULL;
static int width = 0;
static int height = 0;
static RsvgDimensionData dim;

static inline Eina_Bool
evas_image_load_file_is_svg(const char *file)
{
   int i, len = strlen(file);
   Eina_Bool is_gz = EINA_FALSE;

   for (i = len - 1; i > 0; i--)
     {
	if (file[i] == '.')
	  {
	     if (is_gz)
	       break;
	     else if (strcasecmp(file + i + 1, "gz") == 0)
	       is_gz = EINA_TRUE;
	     else
	       break;
	  }
     }

   if (i < 1) return EINA_FALSE;
   i++;
   if (i >= len) return EINA_FALSE;
   if (strncasecmp(file + i, "svg", 3) != 0) return EINA_FALSE;
   i += 3;
   if (is_gz)
     {
	if (file[i] == '.') return EINA_TRUE;
	else return EINA_FALSE;
     }
   else
     {
	if (file[i] == '\0') return EINA_TRUE;
	else if (((file[i] == 'z') || (file[i] == 'Z')) && (!file[i + 1])) return EINA_TRUE;
	else return EINA_FALSE;
     }
}

static int
_svg_init(const char *file)
{
#ifdef HAVE_SVG_2_36
# if !defined(GLIB_VERSION_2_36)
   g_type_init();
# endif
#else
   rsvg_init();
#endif

   if (!evas_image_load_file_is_svg(file)) return 0;

   rsvg = rsvg_handle_new_from_file(file, NULL);
   if (!rsvg) return 0;

   return 1;
}

static void
_svg_shutdown(void)
{
   if (rsvg)
     {
// we don't really need this it seems and it's deprecated in 2.46
//        rsvg_handle_close(rsvg, NULL);
        g_object_unref(rsvg);
     }
   // Maybe it's not crashing anymore, let's try it.
#ifndef HAVE_SVG_2_36
   rsvg_term();
#endif
}

static int
read_svg_header(int scale_down, double dpi, int size_w, int size_h)
{
   rsvg_handle_set_dpi(rsvg, 75.0);

#ifdef HAVE_SVG_2_51
   double owidth, oheight;

   rsvg_handle_get_intrinsic_size_in_pixels(rsvg, &owidth, &oheight);
   width = ceil(owidth);
   height = ceil(oheight);
   if ((width == 0) || (height == 0))
     {
        RsvgLength l1, l2;
        gboolean has_l1, has_l2, has_vb;
        RsvgRectangle vb;

        rsvg_handle_get_intrinsic_dimensions(rsvg, &has_l1, &l1, &has_l2, &l2, &has_vb, &vb);
        width = ceil(vb.width);
        height = ceil(vb.height);
     }
#else
   rsvg_handle_get_dimensions(rsvg, &dim);
   width = dim.width;
   height = dim.height;
#endif

   if ((width < 1) || (height < 1)) return 0;

   if (scale_down > 1)
     {
        width /= scale_down;
        height /= scale_down;
     }
   else if (dpi > 0.0)
     {
        width = (width * dpi) / 75;
        height = (height * dpi) / 75;
     }
   else if (size_w > 0 && size_h > 0)
     {
        int w, h;

        w = size_w;
        h = (size_w * height) / width;
        if (h > size_h)
          {
             h = size_h;
             w = (size_h * width) / height;
          }
        width = w;
        height = h;
     }
   if (width < 1) width = 1;
   if (height < 1) height = 1;

   return 1;
}

static int
read_svg_data(void)
{
   cairo_surface_t *surface;
   cairo_t *cr;

   shm_alloc(width * height * (sizeof(DATA32)));
   if (!shm_addr) return 0;

   memset(shm_addr, 0, width * height * sizeof (DATA32));
   surface = cairo_image_surface_create_for_data((unsigned char *)shm_addr,
                                                 CAIRO_FORMAT_ARGB32,
                                                 width, height,
                                                 width * sizeof(DATA32));;
   if (!surface) return 0;

   cr = cairo_create(surface);
   if (!cr) return 0;

   if ((dim.em > 0.0) && (dim.ex > 0.0))
     cairo_scale(cr, (double) width / dim.em, (double) height / dim.ex);
   else
     cairo_scale(cr, (double)1, (double)1);

#ifdef HAVE_SVG_2_51
   RsvgRectangle vp =
     {
        .x = 0,
        .y = 0,
        .width = width,
        .height = height,
     };
   rsvg_handle_render_document(rsvg, cr, &vp, NULL);
#else
   rsvg_handle_render_cairo(rsvg, cr);
#endif

   cairo_surface_destroy(surface);
   cairo_destroy(cr);

   return 1;
}

int
main(int argc, char **argv)
{
   char *file;
   int i;
   int head_only = 0;
   int scale_down = 0;
   double dpi = 0.0;
   int size_w = 0, size_h = 0;

   if (argc < 2) return -1;
   file = argv[1];

   for (i = 2; i < argc; ++i)
     {
        if (!strcmp(argv[i], "-head"))
          head_only = 1;
        else if (!strcmp(argv[i], "-key"))
          { // not used by svg loader
             i++;
             // const char *key = argv[i];
          }
        else if (!strcmp(argv[i], "-opt-scale-down-by"))
          {
             i++;
             scale_down = atoi(argv[i]);
          }
        else if (!strcmp(argv[i], "-opt-dpi"))
          {
             i++;
             dpi = ((double)atoi(argv[i])) / 1000.0;
          }
        else if (!strcmp(argv[i], "-opt-size"))
          {
             i++;
             size_w = atoi(argv[i]);
             i++;
             size_h = atoi(argv[i]);
          }
     }

   timeout_init(5);

   if (!_svg_init(file)) return -1;
   if (!read_svg_header(scale_down, dpi, size_w, size_h)) return -1;

   if (head_only != 0)
     {
        printf("size %d %d\n", width, height);
        printf("alpha 1\n");
        printf("done\n");
     }
   else
     {
        if (read_svg_data())
          {
             printf("size %d %d\n", width, height);
             printf("alpha 1\n");
#ifdef _WIN32
             if (shm_fd) printf("shmfile %s\n", shmfile);
#else
             if (shm_fd >= 0) printf("shmfile %s\n", shmfile);
#endif
             else
               {
                  printf("data\n");
                  if (fwrite(shm_addr, width * height * sizeof(DATA32), 1, stdout) != 1)
                    {
                       // nothing much to do, the receiver will simply ignore the
                       // data as it's too short
                       //D("fwrite failed (%d bytes): %m\n", width * height * sizeof(DATA32));
                    }
               }
             shm_free();
          }
     }
   _svg_shutdown();
   fflush(stdout);
   return 0;
}

