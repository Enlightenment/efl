#ifndef EDJE_H
#define EDJE_H

#include <Evas.h>
#include <Ecore.h>
#ifndef EDJE_FB_ONLY
#include <Ecore_X.h>
#else
#include <Ecore_Fb.h>
#endif
#include <Ecore_Evas.h>
/* ... only for testing */
#include <Eet.h>
#include "edje_private.h"
/* ... end testing */
#include "Edje.h"
#include "config.h"

#define DAT PACKAGE_DATA_DIR"/"

#endif
