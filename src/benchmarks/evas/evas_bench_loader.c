#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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
evas_bench_loader_tgv(int request)
{
   Evas *e = _setup_evas();
   char *large;
   char *small;
   char *computer;
   Evas_Object *o;
   Eina_List *l;
   int i;

   large = strdup(_test_image_get("Light-50.tgv"));
   small = strdup(_test_image_get("Sunrise-100.tgv"));
   computer = strdup(_test_image_get("Pic1-50.tgv"));

   for (i = 0; i < request; i++)
     {
        o = evas_object_image_add(e);

        evas_object_image_file_set(o, large, NULL);
        if (!evas_object_image_data_get(o, 0)) break ;

        evas_object_image_file_set(o, small, NULL);
        if (!evas_object_image_data_get(o, 0)) break ;

        evas_object_image_file_set(o, computer, NULL);
        if (!evas_object_image_data_get(o, 0)) break ;

        evas_object_del(o);

        l = evas_render_updates(e);
        evas_render_updates_free(l);

        evas_render_idle_flush(e);
        evas_render_dump(e);
     }

   fprintf(stderr, "i: %i (%s, %s, %s)\n",
           i, large, small, computer);

   free(large);
   free(small);
   free(computer);

   evas_free(e);
}

void evas_bench_loader(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "tgv-loader", EINA_BENCHMARK(evas_bench_loader_tgv), 20, 2000, 100);
}
