#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef BUILD_ENGINE_BUFFER

#include <Evas.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

#define TEST_FONT_NAME "DejaVuSans,UnDotum"
#define TEST_FONT_SOURCE TESTS_SRC_DIR "/fonts/TestFont.eet"

#define START_MASK_TEST(w, h) \
   Ecore_Evas *ee; Evas *e; \
   ee = ecore_evas_buffer_new(w, h); \
   ecore_evas_show(ee); \
   ecore_evas_manual_render_set(ee, EINA_TRUE); \
   e = ecore_evas_get(ee); \
   Eina_List *tofree = NULL; \
   do {} while (0)

#define END_MASK_TEST() do { \
   Evas_Object *o; \
   EINA_LIST_FREE(tofree, o) { evas_object_del(o); } \
   ecore_evas_free(ee); \
   } while (0)

#define AUTODEL(o) do { tofree = eina_list_prepend(tofree, o); } while (0)

static int
_bgra_compare(unsigned int *data, unsigned int *ref, int w, int h)
{
   int i, j, ret = 0;
   for (j = 0; j < h; j++)
     {
#if 0
        printf("data: ");
        for (i = 0; i < w; i++) printf("%#x ", data[i+j*w]);
        printf("\nref:  ");
        for (i = 0; i < w; i++) printf("%#x ", ref[i+j*w]);
        printf("\n\n");
#endif
        for (i = 0; i < w; i++)
          if (data[i+j*w] != ref[i+j*w])
            {
               printf("Pixel %2dx%-2d differ: %#x vs. %#x\n", i, j, data[i+j*w], ref[i+j*w]);
               fflush(stdout);
               ret = 1;
            }
     }
   return ret;
}

// The usual useless unit test
EFL_START_TEST(evas_mask_test_setget)
{
   Evas *e = _setup_evas();
   Evas_Object *obj = NULL, *mask = NULL;
   const Eina_List *list, *l;
   Eina_Iterator *it;
   Evas_Object *o;
   int i;

   obj = evas_object_text_add(e);
   fail_if(evas_object_clip_get(obj) != NULL);

   mask = evas_object_image_filled_add(e);
   evas_object_clip_set(obj, mask);
   fail_if(evas_object_clip_get(obj) != mask);

   list = evas_object_clipees_get(mask);
   i = 0;
   EINA_LIST_FOREACH(list, l, o)
     {
        if (!i) fail_if(o != obj);
        i++;
     }
   fail_if(i != 1);

   it = efl_canvas_object_clipees_get(mask);
   i = 0;
   EINA_ITERATOR_FOREACH(it, o)
     {
        if (!i) fail_if(o != obj);
        i++;
     }
   fail_if(i != 1);
   eina_iterator_free(it);

   evas_object_clip_unset(obj);
   fail_if(evas_object_clip_get(obj) != NULL);

   evas_object_del(mask);
   evas_object_del(obj);

   evas_free(e);
}
EFL_END_TEST

// This test is also very basic but will check the pixels
EFL_START_TEST(evas_mask_test_compare_clip)
{
   Evas_Object *obj, *mask, *rect, *bg, *clip, *text;
   unsigned int *data, *refdata[3];
   const int W = 64;
   const int H = 64;
   int i, tw, th;

   static unsigned int mask_data[] = {
      0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
      0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000,
   };

   static unsigned int ref_data[2][16] = {
      // blue masked with alpha 0xFF over green bg
      {
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
         0xFF00FF00, 0xFF0000FF, 0xFF0000FF, 0xFF00FF00,
         0xFF00FF00, 0xFF0000FF, 0xFF0000FF, 0xFF00FF00,
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
      },
      // blue masked with alpha 0x80 over green bg
      {
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
         0xFF00FF00, 0xFF007F80, 0xFF007F80, 0xFF00FF00,
         0xFF00FF00, 0xFF007F80, 0xFF007F80, 0xFF00FF00,
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
      }
   };

   START_MASK_TEST(W, H);
   printf("Testing basic mask render... ");

   // Create reference data -- scaled up images
   obj = evas_object_image_filled_add(e);
   evas_object_image_smooth_scale_set(obj, 0);
   evas_object_image_size_set(obj, 4, 4);
   evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_data_copy_set(obj, ref_data[0]);
   evas_object_image_data_update_add(obj, 0, 0, 4, 4);
   evas_object_geometry_set(obj, 0, 0, W, H);
   evas_object_show(obj);

   ecore_evas_manual_render(ee);
   refdata[0] = calloc(W * H, 4);
   memcpy(refdata[0], ecore_evas_buffer_pixels_get(ee), W * H * 4);

   evas_object_image_data_copy_set(obj, ref_data[1]);
   evas_object_image_data_update_add(obj, 0, 0, 4, 4);
   ecore_evas_manual_render(ee);
   refdata[1] = calloc(W * H, 4);
   memcpy(refdata[1], ecore_evas_buffer_pixels_get(ee), W * H * 4);
   evas_object_del(obj);

   // Green background
   bg = evas_object_rectangle_add(e);
   evas_object_geometry_set(bg, 0, 0, W, H);
   evas_object_color_set(bg, 0, 0xFF, 0, 0xFF);
   evas_object_show(bg);
   AUTODEL(bg);

   // Blue rectangle
   rect = evas_object_rectangle_add(e);
   evas_object_geometry_set(rect, 0, 0, W, H);
   evas_object_color_set(rect, 0, 0, 0xFF, 0xFF);
   evas_object_show(rect);
   AUTODEL(rect);

   // Mask image
   mask = evas_object_image_filled_add(e);
   evas_object_image_smooth_scale_set(mask, 0);
   evas_object_image_size_set(mask, 4, 4);
   evas_object_image_colorspace_set(mask, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_data_copy_set(mask, mask_data);
   evas_object_image_data_update_add(mask, 0, 0, 4, 4);
   evas_object_geometry_set(mask, 0, 0, W, H);
   evas_object_clip_set(rect, mask);
   evas_object_show(mask);
   AUTODEL(mask);

   // Compare dump data
   data = calloc(W * H, 4);
   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), W * H * 4);
   fail_if(_bgra_compare(data, refdata[0], W,  H) != 0);

   // Try again with mask alpha = 0x80
   evas_object_color_set(mask, 0x80, 0x80, 0x80, 0x80);
   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), W * H * 4);
   fail_if(_bgra_compare(data, refdata[1], W, H) != 0);

   // Now try again with a clip instead - this verifies clip == mask
   clip = evas_object_rectangle_add(e);
   evas_object_geometry_set(clip, W / 4, H / 4, W / 2, H / 2);
   evas_object_color_set(clip, 0xFF, 0xFF, 0xFF, 0xFF);
   evas_object_clip_set(rect, clip);
   evas_object_hide(mask);
   evas_object_show(clip);

   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), W * H * 4);
   fail_if(_bgra_compare(data, refdata[0], W, H));

   evas_object_color_set(clip, 0x80, 0x80, 0x80, 0x80);
   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), W * H * 4);
   fail_if(_bgra_compare(data, refdata[1], W, H));

   // Reset objects
   evas_object_hide(rect);
   evas_object_hide(clip);
   evas_object_hide(mask);

   // Text masking test
   text = evas_object_text_add(e);
   evas_object_text_font_source_set(text, TEST_FONT_SOURCE);
   evas_object_text_font_set(text, TEST_FONT_NAME, 20);
   evas_object_text_text_set(text, "TEXT MASKING SHOULD CUT");
   evas_object_color_set(text, 0xFF, 0xFF, 0xFF, 0xFF);
   evas_object_geometry_get(text, NULL, NULL, &tw, &th);
   evas_object_geometry_set(text, W/2 - tw/2, H/2 - th/2, tw, th);
   evas_object_show(text);
   evas_object_show(clip);
   evas_object_color_set(clip, 0xFF, 0xFF, 0xFF, 0xFF);
   evas_object_clip_set(text, clip);
   ecore_evas_manual_render(ee);
   refdata[2] = calloc(W * H, 4);
   memcpy(refdata[2], ecore_evas_buffer_pixels_get(ee), W * H * 4);

   evas_object_color_set(mask, 0xFF, 0xFF, 0xFF, 0xFF);
   evas_object_clip_set(text, mask);
   evas_object_hide(clip);
   evas_object_show(mask);
   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), W * H * 4);

   fail_if(_bgra_compare(data, refdata[2], W, H));

   printf("PASSED!\n");
   for (i = 0; i < 3; i++) free(refdata[i]);
   free(data);
   END_MASK_TEST();
}
EFL_END_TEST

// This will simply check that a mask is recursively applied to children
EFL_START_TEST(evas_mask_test_mask_of_mask)
{
   Evas_Object *bg, *tbl, *rect0, *mask0, *mask1, *obj;
   unsigned int *data, *refdata;
   const int W = 16;
   const int H = 16;

   // Note: This test isn't great as a mask will hijack the clipper
   // set by the table to its children.

   static unsigned int mask_data[3][16] =
   {
      // Table's mask
      {
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
         0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
      },
      // Rect's mask
      {
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
         0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
      },
      // Reference image with colors
      {
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
         0xFF00FF00, 0xFF00FF00, 0xFFFF00FF, 0xFF00FF00, // look here!
         0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
      }
   };

   START_MASK_TEST(W, H);
   printf("Testing basic masks of masks... ");

   // Green background
   bg = evas_object_rectangle_add(e);
   evas_object_geometry_set(bg, 0, 0, W, H);
   evas_object_color_set(bg, 0, 0xFF, 0, 0xFF);
   evas_object_show(bg);
   AUTODEL(bg);

   // Table
   tbl = evas_object_table_add(e);
   evas_object_geometry_set(tbl, 0, 0, W, H);
   evas_object_table_homogeneous_set(tbl, EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE);
   evas_object_show(tbl);
   AUTODEL(tbl);

   // Table's mask
   mask0 = evas_object_image_filled_add(e);
   evas_object_image_smooth_scale_set(mask0, 0);
   evas_object_image_size_set(mask0, 4, 4);
   evas_object_image_colorspace_set(mask0, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_data_copy_set(mask0, mask_data[0]);
   evas_object_image_data_update_add(mask0, 0, 0, 4, 4);
   evas_object_geometry_set(mask0, 0, 0, W, H);
   evas_object_show(mask0);
   AUTODEL(mask0);

   evas_object_clip_set(tbl, mask0);

   // Rect is table's content
   rect0 = evas_object_rectangle_add(e);
   evas_object_color_set(rect0, 255, 0, 255, 255);
   evas_object_show(rect0);

   evas_object_size_hint_expand_set(rect0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(rect0, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_table_pack(tbl, rect0, 0, 0, 1, 1);

   // mask1 is also table content
   mask1 = evas_object_image_filled_add(e);
   evas_object_image_smooth_scale_set(mask1, 0);
   evas_object_image_size_set(mask1, 4, 4);
   evas_object_image_colorspace_set(mask1, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_data_copy_set(mask1, mask_data[1]);
   evas_object_image_data_update_add(mask1, 0, 0, 4, 4);
   evas_object_show(mask1);

   evas_object_size_hint_expand_set(mask1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(mask1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_table_pack(tbl, mask1, 0, 0, 1, 1);

   // BAAAAD: Hijack rect0's clipper
   obj = evas_object_clip_get(rect0);
   evas_object_clip_set(rect0, mask1);
   if (!evas_object_clipees_has(obj))
     evas_object_hide(obj);

   // Screenshot mask of mask
   data = calloc(W * H, 4);
   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), W * H * 4);

   // Render reference image
   obj = evas_object_image_filled_add(e);
   evas_object_image_smooth_scale_set(obj, 0);
   evas_object_image_size_set(obj, 4, 4);
   evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_data_copy_set(obj, mask_data[2]);
   evas_object_image_data_update_add(obj, 0, 0, 4, 4);
   evas_object_geometry_set(obj, 0, 0, W, H);
   evas_object_show(obj);
   AUTODEL(obj);

   refdata = calloc(W * H, 4);
   ecore_evas_manual_render(ee);
   memcpy(refdata, ecore_evas_buffer_pixels_get(ee), W * H * 4);
   fail_if(_bgra_compare(data, refdata, W, H) != 0);

   printf("PASSED!\n");
   free(refdata);
   free(data);
   END_MASK_TEST();
}
EFL_END_TEST

// NOTE: Much more extensive tests are required. But they should
// be based on "exactness" or a pixel similarity tool.
// The GL engine is not tested at all. Even masking images is not tested...

void evas_test_mask(TCase *tc)
{
   tcase_add_test(tc, evas_mask_test_setget);
   tcase_add_test(tc, evas_mask_test_compare_clip);
   tcase_add_test(tc, evas_mask_test_mask_of_mask);
}

#endif // BUILD_ENGINE_BUFFER
