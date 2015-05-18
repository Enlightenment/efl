/********************************************************************
 * ecore_con_url.eo.c
 *******************************************************************/
EAPI Eina_Bool
ecore_con_url_url_set(Ecore_Con_Url *obj, const char *url)
{
   Eina_Bool ret;
   eo_do((Ecore_Con_Url *)obj, ret = efl_network_url_set(url));
   return ret;
}

EAPI const char *
ecore_con_url_url_get(const Ecore_Con_Url *obj)
{
   const char * ret;
   eo_do((Ecore_Con_Url *)obj, ret = efl_network_url_get());
   return ret;
}

