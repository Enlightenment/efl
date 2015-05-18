#ifndef _ECORE_CON_LEGACY_H
#define _ECORE_CON_LEGACY_H
#include <Eina.h>
#include <Eo.h>

#include "ecore_con_base.eo.legacy.h"
#include "ecore_con_server.eo.legacy.h"
#include "ecore_con_connector.eo.legacy.h"
#include "ecore_con_client.eo.legacy.h"


/********************************************************************
 * ecore_con_url.eo.h
 *******************************************************************/
typedef Eo Ecore_Con_Url;


/********************************************************************
 * ecore_con_url.eo.legacy.h
 *******************************************************************/
/**
 * * Controls the URL to send the request to.
 * @param[in] url The URL
 */
EAPI Eina_Bool ecore_con_url_url_set(Ecore_Con_Url *obj, const char *url);

/**
 * * Controls the URL to send the request to.
 */
EAPI const char *ecore_con_url_url_get(const Ecore_Con_Url *obj);

#endif
