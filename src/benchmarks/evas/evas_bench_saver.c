#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>

#include "Evas.h"
#include "Evas_Engine_Buffer.h"
#include "evas_bench.h"

static const char *
_test_image_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_SRC_DIR"/images/%s", name);

   return filename;
}

static Evas *
_setup_evas()
{
   Evas *evas;
   Evas_Engine_Info_Buffer *einfo;

   evas = evas_new();

   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(evas);

   einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
   einfo->info.dest_buffer = malloc(sizeof (char) * 500 * 500 * 4);
   einfo->info.dest_buffer_row_bytes = 500 * sizeof (char) * 4;

   evas_engine_info_set(evas, (Evas_Engine_Info *)einfo);

   evas_output_size_set(evas, 500, 500);
   evas_output_viewport_set(evas, 0, 0, 500, 500);

   return evas;
}

static void
evas_bench_saver_tgv(int request)
{
   Evas *e = _setup_evas();
   const char *source;
   Eina_Tmpstr *dest;
   Evas_Object *o;
   int fd;
   int i;

   source = _test_image_get("mars_rover_panorama_half-size.jpg");
   fd = eina_file_mkstemp("evas_saver_benchXXXXXX.tgv", &dest);
   if (fd < 0) return;
   close(fd);

   o = evas_object_image_add(e);
   evas_object_image_file_set(o, source, NULL);

   for (i = 0; i < request; i++)
     {
        evas_object_image_save(o, dest, NULL, "compress=1 quality=50");
     }

   unlink(dest);
   eina_tmpstr_del(dest);

   evas_free(e);
}

void evas_bench_saver(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "tgv-saver", EINA_BENCHMARK(evas_bench_saver_tgv), 20, 2000, 100);
}
