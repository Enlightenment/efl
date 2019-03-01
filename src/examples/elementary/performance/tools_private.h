#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif
#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#define COORDRANGE 1000
#define ANGLERANGE 360
#define axiskeys "xyz"
#define pb "panel_struct"

typedef struct _Panel_Struct
{
   Evas_Object *cspX, *cspY, *cspZ;
   Evas_Object *cspangle;
   Evas_Object *lspX, *lspY, *lspZ;
   Evas_Object *lspangle;
} Panel_Struct;

typedef struct _Axis_Key
{
   float x;
   float y;
   float z;
} Axis_Key;

/*callbacks*/
static void
_camera_light_changeX_cb(void *data, Evas_Object *obj, void *event_info);
static void
_camera_light_changeY_cb(void *data, Evas_Object *obj, void *event_info);
static void
_camera_light_changeZ_cb(void *data, Evas_Object *obj, void *event_info);
static void
_camera_light_angle_change_cb(void *data, Evas_Object *obj, void *event_info);
static void
_camera_light_axisX_change_cb(void *data, Evas_Object *obj, void *event_info);
static void
_camera_light_axisY_change_cb(void *data, Evas_Object *obj, void *event_info);
static void
_camera_light_axisZ_change_cb(void *data, Evas_Object *obj, void *event_info);
