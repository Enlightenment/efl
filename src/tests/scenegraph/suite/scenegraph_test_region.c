#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>

#include "scenegraph_suite.h"
#include "../../../static_libs/scenegraph/Scenegraph.h"

static Eina_Bool
_rect_not_equal(Eina_Rect r1, Eina_Rect r2)
{
   if ((r1.x != r2.x) ||
       (r1.y != r2.y) ||
       (r1.w != r2.w) ||
       (r1.h != r2.h) )
      return EINA_TRUE;
   else
      return EINA_FALSE;
}

START_TEST(scenegraph_region_create)
{
   SG_Region r = sg_region_new();
   r.fn->add_rect(&r, EINA_RECT(0, 0, 10,10));

   SG_Region r1 = r.fn->ref(&r);
   r1.fn->add_rect(&r1, EINA_RECT(5, 5, 10,10));

   fail_if (r.fn->rect_count(&r) != 1);
   fail_if (_rect_not_equal(r.fn->rect_at(&r, 0), EINA_RECT(0, 0, 10,10)));

   fail_if (r1.fn->rect_count(&r1) != 3);
   fail_if (_rect_not_equal(r1.fn->rect_at(&r1, 0), EINA_RECT(0, 0, 10,5)));
   fail_if (_rect_not_equal(r1.fn->rect_at(&r1, 1), EINA_RECT(0, 5, 15,5)));
   fail_if (_rect_not_equal(r1.fn->rect_at(&r1, 2), EINA_RECT(5, 10, 10,5)));

   sg_region_free(&r);

   fail_if (r1.fn->rect_count(&r1) != 3);
   sg_region_free(&r1);

}

END_TEST

void
scenegraph_test_region(TCase *tc)
{
   tcase_add_test(tc, scenegraph_region_create);
}