/* handy macros */
#define ELM_RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && (((x) + (w)) > (xx)) && (((y) + (h)) > (yy)))
#define ELM_PI 3.14159265358979323846
