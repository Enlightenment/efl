#ifndef EVAS_XCB_XDEFAULTS_H
# define EVAS_XCB_XDEFAULTS_H

# include "evas_engine.h"

void _evas_xcb_xdefaults_init(void);
void _evas_xcb_xdefaults_shutdown(void);
char *_evas_xcb_xdefaults_string_get(const char *prog, const char *param);
int _evas_xcb_xdefaults_int_get(const char *prog, const char *param);

#endif
