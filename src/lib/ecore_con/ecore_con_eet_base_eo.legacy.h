#ifndef _ECORE_CON_EET_BASE_EO_LEGACY_H_
#define _ECORE_CON_EET_BASE_EO_LEGACY_H_

#ifndef _ECORE_CON_EET_BASE_EO_CLASS_TYPE
#define _ECORE_CON_EET_BASE_EO_CLASS_TYPE

typedef Eo Ecore_Con_Eet_Base;

#endif

#ifndef _ECORE_CON_EET_BASE_EO_TYPES
#define _ECORE_CON_EET_BASE_EO_TYPES

/** Ecore connection reply data structure
 *
 * @ingroup Ecore_Con
 */
typedef struct _Ecore_Con_Reply Ecore_Con_Reply;


#endif







/**
 * @brief Function to register a @ref Eet.Data.Descriptor to the ecore_con_eet
 * object.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the eet stream.
 * @param[in] edd The Eet.Data.Descriptor that is to be registered.
 *
 * @ingroup (null)_Group
 */
EAPI void ecore_con_eet(Ecore_Con_Eet_Base *obj, const char *name, Eet_Data_Descriptor *edd);



#endif
