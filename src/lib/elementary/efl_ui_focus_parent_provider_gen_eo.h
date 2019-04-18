#ifndef _EFL_UI_FOCUS_PARENT_PROVIDER_GEN_EO_H_
#define _EFL_UI_FOCUS_PARENT_PROVIDER_GEN_EO_H_

#ifndef _EFL_UI_FOCUS_PARENT_PROVIDER_GEN_EO_CLASS_TYPE
#define _EFL_UI_FOCUS_PARENT_PROVIDER_GEN_EO_CLASS_TYPE

typedef Eo Efl_Ui_Focus_Parent_Provider_Gen;

#endif

#ifndef _EFL_UI_FOCUS_PARENT_PROVIDER_GEN_EO_TYPES
#define _EFL_UI_FOCUS_PARENT_PROVIDER_GEN_EO_TYPES


#endif
#ifdef EFL_BETA_API_SUPPORT
/** EFL UI Focus Parent Provider Gen class
 *
 * @ingroup Efl_Ui_Focus_Parent_Provider_Gen
 */
#define EFL_UI_FOCUS_PARENT_PROVIDER_GEN_CLASS efl_ui_focus_parent_provider_gen_class_get()

EWAPI const Efl_Class *efl_ui_focus_parent_provider_gen_class_get(void);

/**
 * @brief Content item map property
 *
 * @param[in] obj The object.
 * @param[in] map Item map
 *
 * @ingroup Efl_Ui_Focus_Parent_Provider_Gen
 */
EOAPI void efl_ui_focus_parent_provider_gen_content_item_map_set(Eo *obj, Eina_Hash *map);

/**
 * @brief Content item map property
 *
 * @param[in] obj The object.
 *
 * @return Item map
 *
 * @ingroup Efl_Ui_Focus_Parent_Provider_Gen
 */
EOAPI Eina_Hash *efl_ui_focus_parent_provider_gen_content_item_map_get(const Eo *obj);

/**
 * @brief Container property
 *
 * @param[in] obj The object.
 * @param[in] container Container widget
 *
 * @ingroup Efl_Ui_Focus_Parent_Provider_Gen
 */
EOAPI void efl_ui_focus_parent_provider_gen_container_set(Eo *obj, Efl_Ui_Widget *container);

/**
 * @brief Container property
 *
 * @param[in] obj The object.
 *
 * @return Container widget
 *
 * @ingroup Efl_Ui_Focus_Parent_Provider_Gen
 */
EOAPI Efl_Ui_Widget *efl_ui_focus_parent_provider_gen_container_get(const Eo *obj);

/**
 * @brief Fetch the item where the item is a subchild from
 *
 * @param[in] obj The object.
 * @param[in] widget
 *
 * @ingroup Efl_Ui_Focus_Parent_Provider_Gen
 */
EOAPI Efl_Ui_Widget *efl_ui_focus_parent_provider_gen_item_fetch(Eo *obj, Efl_Ui_Widget *widget);
#endif /* EFL_BETA_API_SUPPORT */

#endif
