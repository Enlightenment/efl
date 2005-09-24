#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#ifndef _WIN32_WCE
#	include <sys/time.h>
#else
#	include <time.h>
#	include <windows.h>
#endif

#include "Evas.h"


//_WIN32_WCE
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

extern Evas        *evas;
extern int          win_w;
extern int          win_h;

extern int          loop_count;
extern int          orig_loop_count;

extern double       start_time;
extern double       orig_start_time;

extern Evas_Object *r1, *r2, *r3, *g1, *g2, *c1, *l1, *p1;

/*****************************************************************************/

double              get_time(void);
void                loop(void);
void                setup(void);

void 		    setdown(void);
void 		    scroll_setdown(void);
void 		    scroll_setup(void);


#ifdef __cplusplus
}
#endif

