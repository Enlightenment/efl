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

SG_Span SPAN[250];
SG_Rle
_rle_create(int x, int y, int w, int h)
{
    SG_Rle rle = sg_rle_new();
    int i;
    for(i=0; i< h; i++)
    {
        SPAN[i].x = x;
        SPAN[i].len = w;
        SPAN[i].y = y + i;
        SPAN[i].coverage = 255;
        //printf("%d %d %d \n", SPAN[i].y, SPAN[i].x, SPAN[i].len);
    }
    rle.fn->append_spans(&rle, SPAN, i);

    return rle;
}

// void print_rect(Eina_Rect r)
// {
//    printf("rect : %d %d %d %d \n",r.x, r.y, r.w, r.h);
// }

START_TEST(scenegraph_rle_create)
{
   SG_Rle r = _rle_create(0, 0, 10, 10);

   SG_Rle r1 = r.fn->ref(&r);

   fail_if (_rect_not_equal(r.fn->bounding_rect(&r), EINA_RECT(0, 0, 10,10)));
   fail_if (_rect_not_equal(r1.fn->bounding_rect(&r1), EINA_RECT(0, 0, 10,10)));
   fail_if (r.fn->empty(&r));

   r1.fn->intersect_rect(&r1, EINA_RECT(0, 0, 5,5));
   fail_if (_rect_not_equal(r.fn->bounding_rect(&r), EINA_RECT(0, 0, 10,10)));
   fail_if (_rect_not_equal(r1.fn->bounding_rect(&r1), EINA_RECT(0, 0, 5,5)));

   r.fn->translate(&r, 2, 2);
   r.fn->intersect_rect(&r, EINA_RECT(0, 0, 6,6));
   fail_if (_rect_not_equal(r.fn->bounding_rect(&r), EINA_RECT(0, 0, 4,4)));

   sg_rle_free(&r);
   sg_rle_free(&r1);

}

END_TEST

void
scenegraph_test_rle(TCase *tc)
{
   tcase_add_test(tc, scenegraph_rle_create);
}