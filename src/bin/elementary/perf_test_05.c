#ifdef T2
TPROT(05);
#endif
#ifdef T1
{ TFUN(05), "Rectangles (Many)", 1.0 },
#endif
#if !defined(T1) && !defined(T2)
# include "perf.h"
static Evas_Object *objs[NUM_MANY];

TST(05, init) (Evas *e) {
   Evas_Object *o;
   int i;

   srnd();
   for (i = 0; i < NUM_MANY; i++)
     {
        objs[i] = o = evas_object_rectangle_add(e);
        cleanup_add(o);
        evas_object_color_set
          (o, rnd() & 0x7f, rnd() & 0x7f, rnd() & 0x7f, 0x80);
        evas_object_pass_events_set(o, EINA_TRUE);
        evas_object_show(o);
     }
}

TST(05, tick) (Evas *e EINA_UNUSED, double f, Evas_Coord win_w, Evas_Coord win_h) {
   int i;
   Evas_Coord x, y, w, h, w0, h0;

   for (i = 0; i < NUM_MANY; i++)
     {
        Evas_Object *o = objs[i];
        w0 = 120;
        h0 = 120;
        w = 5 + ((1.0 + cos((double)((f * 30.0) + (i * 10)))) * w0 * 2);
        h = 5 + ((1.0 + sin((double)((f * 40.0) + (i * 19)))) * h0 * 2);
        x = (win_w / 2) - (w / 2);
        x += sin((double)((f * 50.0) + (i * 13))) * (w0 / 2);
        y = (win_h / 2) - (h / 2);
        y += cos((double)((f * 45.0) + (i * 28))) * (h0 / 2);
        evas_object_geometry_set(o, x, y, w, h);
     }
}
#endif
