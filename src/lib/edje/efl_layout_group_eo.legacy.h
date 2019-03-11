#ifndef _EFL_LAYOUT_GROUP_EO_LEGACY_H_
#define _EFL_LAYOUT_GROUP_EO_LEGACY_H_

#ifndef _EFL_LAYOUT_GROUP_EO_CLASS_TYPE
#define _EFL_LAYOUT_GROUP_EO_CLASS_TYPE

typedef Eo Efl_Layout_Group;

#endif

#ifndef _EFL_LAYOUT_GROUP_EO_TYPES
#define _EFL_LAYOUT_GROUP_EO_TYPES


#endif



/**
 * @brief Retrives an EDC data field's value from a given Edje object's group.
 *
 * This function fetches an EDC data field's value, which is declared on the
 * objects building EDC file, under its group. EDC data blocks are most
 * commonly used to pass arbitrary parameters from an application's theme to
 * its code.
 *
 * EDC data fields always hold  strings as values, hence the return type of
 * this function. Check the complete "syntax reference" for EDC files.
 *
 * This is how a data item is defined in EDC: collections { group { name:
 * "a_group"; data { item: "key1" "value1"; item: "key2" "value2"; } } }
 *
 * @warning Do not confuse this call with edje_file_data_get(), which queries
 * for a global EDC data field on an EDC declaration file.
 *
 * @param[in] obj The object.
 * @param[in] key The data field's key string
 *
 * @return The data's value string.
 *
 * @since 1.21
 *
 * @ingroup (null)_Group
 */
EAPI const char *edje_object_data_get(const Efl_Layout_Group *obj, const char *key);


#endif
