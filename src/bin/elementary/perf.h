#include <Elementary.h>

#define NUM_MANY 1024
#define NUM        64
#define NUM_FEW     8

void         srnd(void);
unsigned int rnd(void);

void         cleanup_add(Evas_Object *o);

#define TST(x, y) \
   void test_ ## x ## _ ## y
#define TPROT(x) \
   TST(x, init)(Evas *e); \
   TST(x, tick)(Evas *e, double f, Evas_Coord win_w, Evas_Coord win_h)
