#ifndef _ELUA_PRIVATE_H
#define _ELUA_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>

#include <Eina.h>
#include <Ecore.h>

#ifdef HAVE_EVIL
#include <Evil.h>
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern int _elua_log_dom;

#define DBG(...) EINA_LOG_DOM_DBG(_elua_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_elua_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_elua_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_elua_log_dom, __VA_ARGS__)
#define CRT(...) EINA_LOG_DOM_CRITICAL(_elua_log_dom, __VA_ARGS__)

#endif
