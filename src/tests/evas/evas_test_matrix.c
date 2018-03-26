#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../lib/evas/include/evas_common_private.h"
#include "../../lib/evas/include/evas_private.h"

#include "evas_suite.h"
#include "evas_tests_helpers.h"

EFL_START_TEST(evas_matrix)
{
   Eina_Vector3 position;
   Eina_Quaternion orientation;
   Eina_Vector3 scale;
   Eina_Matrix4 mat;

   eina_vector3_set(&position, 5.0, 3.0, 2.0);
   eina_quaternion_set(&orientation, 30.0, 1.0, 0.0, 0.0);
   eina_vector3_set(&scale, 1.0, 2.0, 1.0);

   evas_mat4_build(&mat, &position, &orientation, &scale);
   fail_if((mat.xx != -1) || (mat.xy != 60) ||
           (mat.wx != 5) || (mat.wy != 3) ||
           (mat.wz != 2) || (mat.ww != 1));

   evas_mat4_inverse_build(&mat, &position, &orientation, &scale);
   fail_if((mat.xx - 0.99 < DBL_EPSILON) || (mat.xy - 0.0 < DBL_EPSILON) ||
           (mat.yx -0.0 < DBL_EPSILON) || (mat.yy -0.49 < DBL_EPSILON));
}
EFL_END_TEST

void evas_test_matrix(TCase *tc)
{
   (void) _setup_evas;
   tcase_add_test(tc, evas_matrix);
}
