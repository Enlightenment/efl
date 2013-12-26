#ifndef _EPHYSICS_PRIVATE_H
#define _EPHYSICS_PRIVATE_H

#include <Edje.h>
#include <Elementary.h>
#include <Eina.h>
#include <Evas.h>

#include "EPhysics.h"

#ifdef EPHYSICS_LOG_COLOR
#undef EPHYSICS_LOG_COLOR
#endif
#define EPHYSICS_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ephysics_test_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ephysics_test_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ephysics_test_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ephysics_test_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ephysics_test_log_dom, __VA_ARGS__)

#define EPHYSICS_TEST_THEME "ephysics_test"
#define WIDTH (512)
#define HEIGHT (384)
#define DEPTH (100)
#define SHADOW_ALPHA_ID (1)
#define FLOOR_Y (344)
#define FLOOR_WIDTH (750)
#define SH_THRESHOLD (200)

#ifdef __cplusplus
extern "C" {
#endif

extern int _ephysics_test_log_dom;

typedef struct _Test_Data Test_Data;

struct _Test_Data {
     EPhysics_World *world;
     Evas_Object *layout;
     Evas_Object *win;
     Eina_List *constraints;
     Eina_List *evas_objs;
     Eina_List *bodies;
     void *data;
};

Test_Data *test_data_new(void);
void test_data_del(Test_Data *test_data);

void test_clean(Test_Data *test_data);

void test_win_add(Test_Data *test_data, const char *title, Eina_Bool autodel);

void update_object_cb(void *data, EPhysics_Body *body, void *event_info);

#ifdef __cplusplus
}
#endif

#endif
