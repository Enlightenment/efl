/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* ############## bad */
#define HAVE_EVAS2

#include <signal.h>
#include <dlfcn.h>
#include <stdio.h>
#include <glob.h>
#include <sys/param.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>		/* malloc(), free() */

#include "Ecore.h"
#include "Ecore_Config.h"
#include "ecore_config_util.h"
#include "ecore_config_ipc.h"

#include "ecore_config_private.h"

static Ecore_Config_Server *__ecore_config_servers;
Ecore_Timer *ipc_timer = NULL;

Ecore_Config_Server *
_ecore_config_server_convert(void *srv)
{
   Ecore_Config_Server *srv_tmp;

   srv_tmp = __ecore_config_servers;
   while (srv_tmp)
     {
	if (srv_tmp->server == srv)
	  return srv_tmp;
	srv_tmp = srv_tmp->next;
     }

   return __ecore_config_server_global;
}

/*****************************************************************************/
/* INTERFACE FOR IPC MODULES */
/*****************************/

char               *
_ecore_config_ipc_prop_list(Ecore_Config_Server * srv, const long serial)
{
   Ecore_Config_Bundle *theme;
   Ecore_Config_Prop  *e;
   estring            *s;
   int                 f;

   theme = ecore_config_bundle_by_serial_get(srv, serial);
   e = theme ? theme->data : NULL;
   s = estring_new(8192);
   f = 0;
   while (e)
     {
	/* ignore system properties in listings, unless they have been overridden */
	if (e->flags & ECORE_CONFIG_FLAG_SYSTEM && !(e->flags & ECORE_CONFIG_FLAG_MODIFIED))
	  {
	     e = e->next;
	     continue;
	  }
	estring_appendf(s, "%s%s: %s", f ? "\n" : "", e->key,
			ecore_config_type_get(e));
	if (e->flags & ECORE_CONFIG_FLAG_BOUNDS)
	  {
	     if (e->type == ECORE_CONFIG_FLT)
		estring_appendf(s, ", range %le..%le",
				(float)e->lo / ECORE_CONFIG_FLOAT_PRECISION,
				(float)e->hi / ECORE_CONFIG_FLOAT_PRECISION);
	     else
		estring_appendf(s, ", range %d..%d", e->lo, e->hi);
	  }
	if (e->type == ECORE_CONFIG_THM)
	   estring_appendf(s, ", group %s", e->data ? e->data : "Main");
	f = 1;
	e = e->next;
     }

   return estring_disown(s);
}

char               *
_ecore_config_ipc_prop_desc(Ecore_Config_Server * srv, const long serial,
			    const char *key)
{
#ifdef HAVE_EVAS2
   Ecore_Config_Bundle *theme;
   Ecore_Config_Prop  *e;

   theme = ecore_config_bundle_by_serial_get(srv, serial);
   e = ecore_config_get(key);

   if (e)
     {
	estring            *s = estring_new(512);

	estring_appendf(s, "%s: %s", e->key, ecore_config_type_get(e));
	if (e->flags & ECORE_CONFIG_FLAG_BOUNDS)
	   estring_appendf(s, ", range %d..%d", e->lo, e->hi);
	return estring_disown(s);
     }
#endif
   return strdup("<undefined>");
}

char               *
_ecore_config_ipc_prop_get(Ecore_Config_Server * srv, const long serial,
			   const char *key)
{
#ifdef HAVE_EVAS2
   char               *ret;
   Ecore_Config_Bundle *theme;

   ret = NULL;
   theme = ecore_config_bundle_by_serial_get(srv, serial);
   if ((ret = ecore_config_as_string_get( /*theme, */ key)))
      return ret;
#endif
   return strdup("<undefined>");
}

int
_ecore_config_ipc_prop_set(Ecore_Config_Server * srv, const long serial,
			   const char *key, const char *val)
{
#ifdef HAVE_EVAS2
   int                 ret;
   Ecore_Config_Bundle *theme;

   theme = ecore_config_bundle_by_serial_get(srv, serial);
   ret = ecore_config_set(key, (char *)val);
   E(1, "ipc.prop.set(%s->%s,\"%s\") => %d\n", theme ? theme->identifier : "",
     key, val, ret);
   return ret;
#else
   return ECORE_CONFIG_ERR_NOTSUPP;
#endif
}

/*****************************************************************************/

char               *
_ecore_config_ipc_bundle_list(Ecore_Config_Server * srv)
{
   Ecore_Config_Bundle *ns;
   estring            *s;
   int                 f;

   ns = ecore_config_bundle_1st_get(srv);
   s = estring_new(8192);
   f = 0;
   if (!ns)
      return strdup("<no_bundles_created>");

   while (ns)
     {
	estring_appendf(s, "%s%d: %s", f ? "\n" : "",
			ecore_config_bundle_serial_get(ns),
			ecore_config_bundle_label_get(ns));
	f = 1;
	ns = ecore_config_bundle_next_get(ns);
     }

   return estring_disown(s);
}

int
_ecore_config_ipc_bundle_new(Ecore_Config_Server * srv, const char *label)
{
   if (ecore_config_bundle_new(srv, label))
      return ECORE_CONFIG_ERR_SUCC;
   return ECORE_CONFIG_ERR_FAIL;
}

char               *
_ecore_config_ipc_bundle_label_get(Ecore_Config_Server * srv, const long serial)
{
   Ecore_Config_Bundle *ns;
   char               *label;

   ns = ecore_config_bundle_by_serial_get(srv, serial);
   label = ecore_config_bundle_label_get(ns);
   return strdup(label ? label : "<no such bundle>");
}

int
_ecore_config_ipc_bundle_label_set(Ecore_Config_Server * srv, const long serial,
				   const char *label)
{
   Ecore_Config_Bundle *ns;

   ns = ecore_config_bundle_by_serial_get(srv, serial);
   if (!(ns->identifier = malloc(sizeof(label))))
      return ECORE_CONFIG_ERR_OOM;
   memcpy(ns->identifier, label, sizeof(label));
   return ECORE_CONFIG_ERR_SUCC;
}

long
_ecore_config_ipc_bundle_label_find(Ecore_Config_Server * srv,
				    const char *label)
{
   Ecore_Config_Bundle *ns;

   ns = ecore_config_bundle_by_label_get(srv, label);
   return ns ? ecore_config_bundle_serial_get(ns) : -1;
}

static int
_ecore_config_ipc_poll(void *data __UNUSED__)
{
   Ecore_Config_Server *s;

   s = __ecore_config_servers;
   while (s)
     {
	_ecore_config_ipc_ecore_poll(&s->server);
        s = s->next;
     }

   return 1;
}

int
_ecore_config_ipc_exit(void)
{
   Ecore_Config_Server *l;

   if (ipc_timer)
      ecore_timer_del(ipc_timer);

   l = __ecore_config_servers;
   while (l)
     {
	_ecore_config_ipc_ecore_exit(&l->server);
        if (l->name)
          free(l->name);
	l = l->next;
     }

   return ECORE_CONFIG_ERR_SUCC;
}

Ecore_Config_Server *
_ecore_config_ipc_init(const char *pipe_name)
{
   int                 ret;
   Ecore_Config_Server *list;
   Ecore_Config_Server *ret_srv;

   list = NULL;
   ret_srv = NULL;
   list = NULL;

   list = malloc(sizeof(Ecore_Config_Server));
   memset(list, 0, sizeof(Ecore_Config_Server));
   if ((ret = _ecore_config_ipc_ecore_init(pipe_name, &list->server)) != ECORE_CONFIG_ERR_SUCC)
     {
	E(2, "_ecore_config_ipc_init: failed to register %s, code %d\n",
	  pipe_name, ret);
     }

   E(2, "_ecore_config_ipc_init: registered \"%s\"...\n", pipe_name);

   list->name = strdup(pipe_name);
   list->next = __ecore_config_servers;

   __ecore_config_servers = list;
   if (!ret_srv)
     ret_srv = list;

   if (!ipc_timer)
     ipc_timer = ecore_timer_add(100, _ecore_config_ipc_poll, NULL);
   
   return ret_srv;
}
/*****************************************************************************/
