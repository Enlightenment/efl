/* handy macros */
#define ELM_RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && (((x) + (w)) > (xx)) && (((y) + (h)) > (yy)))
#define ELM_PI 3.14159265358979323846

// checks if the point(xx, yy) stays out of the rectangle(x, y, w, h) area.
#define ELM_RECTS_POINT_OUT(x, y, w, h, xx, yy) (((xx) < (x)) || ((yy) < (y)) || ((xx) > ((x) + (w))) || ((yy) > ((y) + (h))))

// check if the rect (x, y, w, h) includes whole body of rect (xx, yy, ww, hh)
#define ELM_RECTS_INCLUDE(x, y, w, h, xx, yy, ww, hh) (((x) <= (xx)) && (((x) + (w)) >= ((xx + (ww))) && ((y) <= (yy)) && (((y) + (h)) >= ((yy) + (hh)))))
