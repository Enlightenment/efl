#ifndef _ECORE_EVAS_TEST_H
#define _ECORE_EVAS_TEST_H

#include "config.h"
#include "Ecore.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef BUILD_ECORE_EVAS
#include "Ecore_Evas.h"
#include "Ecore_Fb.h"

#define IM PACKAGE_DATA_DIR"/images/"
#define PT PACKAGE_DATA_DIR"/pointers/"
#define FN PACKAGE_DATA_DIR"/fonts/"

extern double       start_time;
extern Ecore_Evas  *ee;
extern Evas        *evas;



void calibrate_pos_set(int pos);
int  calibrate_pos_get(void);
void calibrate_finish(void);
void calibrate_start(void);

void bg_resize(double w, double h);
void bg_start(void);
void bg_go(void);
    
int  app_start(int argc, const char **argv);
void app_finish(void);

#endif

#endif
