/********************************************************************
 * ecore_con_url.eo.c
 *******************************************************************/
EAPI Eina_Bool
ecore_con_url_url_set(Ecore_Con_Url *obj, const char *url)
{
   return efl_network_url_set((Ecore_Con_Url *)obj, url);
}

EAPI const char *
ecore_con_url_url_get(const Ecore_Con_Url *obj)
{
   return efl_network_url_get((Ecore_Con_Url *)obj);
}

