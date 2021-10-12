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

EFL_START_TEST(evas_object_map_api)
{
   Evas_Coord x, y, z;
   Evas_Map *map = evas_map_new(4);

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
}
EFL_END_TEST

void evas_test_map(TCase *tc)
{
   tcase_add_test(tc, evas_object_map_api);
}
