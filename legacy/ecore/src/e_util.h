#ifndef E_UTIL_H
#define E_UTIL_H 1

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef EFM_DEBUG
# define D(x)  do {printf(__FILE__ ":%d:  ", __LINE__); printf x; fflush(stdout);} while (0)
#else
# define D(x)  ((void) 0)
#endif

#define XY_IN_RECT(x, y, rx, ry, rw, rh) \
(((x) >= (rx)) && ((y) >= (ry)) && ((x) < ((rx) + (rw))) && ((y) < ((ry) + (rh))))
#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - x;} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - y;}
#define BOUND_RECTS(cx, cy, cw, ch, x, y, w, h, d) \
if (d) { \
if ((x) < cx) {cw += (cx - (x)); cx = x;} \
if ((y) < cy) {ch += (cy - (y)); cy = y;} \
if (((x) + (w)) > (cx + cw)) cw += (((x) + (w)) - (cx + cw)); \
if (((y) + (h)) > (cy + ch)) ch += (((y) + (h)) - (cy + ch)); } \
else { cx = x; cy = y; cw = w; ch = h; (d) = 1; }
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#define RECT_INSIDE(x, y, w, h, xx, yy, ww, hh) \
(((xx) >= (x)) && (((xx) + (ww)) <= ((x) + (w))) && \
 ((yy) >= (y)) && (((yy) + (hh)) <= ((y) + (h))))
#define START_LIST_DEL(type, base, cmp) \
type *_p, *_pp; _pp = NULL; _p = (base); while(_p) { if (cmp) { \
if (_pp) _pp->next = _p->next; else (base) = _p->next;
#define END_LIST_DEL \
return; } _pp = _p; _p = _p->next; }
#define IC(name) e_path_find(path_icons, name)

double              e_get_time(void);
pid_t               e_run(char *exe);
pid_t               e_run_in_dir(char *exe, char *dir);
void                e_set_env(char *variable, char *content);
uid_t               e_user_id(void);
gid_t               e_group_id(void);
void                sort_alpha(char **list, int num);
char               *e_home_dir(void);
const char         *e_get_basename(const char *);
void                spawn_info_window(char *path);

#endif
