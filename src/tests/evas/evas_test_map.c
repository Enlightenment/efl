#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ecore.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

EFL_START_TEST(evas_object_map_api)
{
   Evas_Coord x, y, z;
   int r, g, b, a;
   Evas_Map *map;

   map = evas_map_new(4);
   ck_assert_int_eq(evas_map_count_get(map), 4);

   evas_map_alpha_set(map, EINA_TRUE);
   ck_assert(evas_map_alpha_get(map));

   evas_map_alpha_set(map, EINA_FALSE);
   ck_assert(!evas_map_alpha_get(map));

   evas_map_smooth_set(map, EINA_TRUE);
   ck_assert(evas_map_smooth_get(map));

   evas_map_smooth_set(map, EINA_FALSE);
   ck_assert(!evas_map_smooth_get(map));

   evas_map_util_object_move_sync_set(map, EINA_TRUE);
   ck_assert(evas_map_util_object_move_sync_get(map));

   evas_map_util_object_move_sync_set(map, EINA_FALSE);
   ck_assert(!evas_map_util_object_move_sync_get(map));

   evas_map_point_coord_set(map, 0, 10, 20, 30);
   evas_map_point_coord_get(map, 0, &x, &y, &z);
   ck_assert((x == 10) && (y == 20) && (z == 30));

   evas_map_point_coord_set(map, 1, 40, 50, 60);
   evas_map_point_coord_get(map, 1, &x, &y, &z);
   ck_assert((x == 40) && (y == 50) && (z == 60));

   evas_map_point_coord_set(map, 2, 70, 80, 90);
   evas_map_point_coord_get(map, 2, &x, &y, &z);
   ck_assert((x == 70) && (y == 80) && (z == 90));

   evas_map_point_coord_set(map, 3, 100, 110, 120);
   evas_map_point_coord_get(map, 3, &x, &y, &z);
   ck_assert((x == 100) && (y == 110) && (z == 120));

   evas_map_point_color_set(map, 0, 0, 0, 0, 255);
   evas_map_point_color_get(map, 0, &r, &g, &b, &a);
   ck_assert((r == 0) && (g == 0) && (b == 0) && (a == 255));

   evas_map_point_color_set(map, 1, 255, 0, 0, 255);
   evas_map_point_color_get(map, 1, &r, &g, &b, &a);
   ck_assert((r == 255) && (g == 0) && (b == 0) && (a == 255));

   evas_map_point_color_set(map, 2, 255, 255, 0, 255);
   evas_map_point_color_get(map, 2, &r, &g, &b, &a);
   ck_assert((r == 255) && (g == 255) && (b == 0) && (a == 255));

   evas_map_point_color_set(map, 3, 255, 255, 255, 255);
   evas_map_point_color_get(map, 3, &r, &g, &b, &a);
   ck_assert((r == 255) && (g == 255) && (b == 255) && (a == 255));

   evas_map_free(map);
}
EFL_END_TEST

EFL_START_TEST(evas_object_map_rect)
{
   Evas *e;
   Evas_Object *rect;
   Evas_Map *map;
   Evas_Coord x, y, z;
   double u, v;

   e = _setup_evas();
   rect = evas_object_rectangle_add(e);
   evas_object_resize(rect, 100, 100);

   map = evas_map_new(4);
   evas_map_util_points_populate_from_object(map, rect);

   evas_map_point_coord_get(map, 0, &x, &y, &z);
   ck_assert((x == 0) && (y == 0) && (z == 0));

   evas_map_point_coord_get(map, 1, &x, &y, &z);
   ck_assert((x == 100) && (y == 0) && (z == 0));
   
   evas_map_point_coord_get(map, 2, &x, &y, &z);
   ck_assert((x == 100) && (y == 100) && (z == 0));

   evas_map_point_coord_get(map, 3, &x, &y, &z);
   ck_assert((x == 0) && (y == 100) && (z == 0));

   evas_map_point_image_uv_get(map, 0, &u, &v);
   ck_assert((u == 0) && (v == 0));

   evas_map_point_image_uv_get(map, 1, &u, &v);
   ck_assert((u == 100) && (v == 0));

   evas_map_point_image_uv_get(map, 2, &u, &v);
   ck_assert((u == 100) && (v == 100));

   evas_map_point_image_uv_get(map, 3, &u, &v);
   ck_assert((u == 0) && (v == 100));

   evas_map_free(map);
   evas_free(e);
}
EFL_END_TEST

void evas_test_map(TCase *tc)
{
   tcase_add_test(tc, evas_object_map_api);
   tcase_add_test(tc, evas_object_map_rect);
}
