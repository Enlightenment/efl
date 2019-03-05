#ifndef _ELM_SCROLLER_EO_LEGACY_H_
#define _ELM_SCROLLER_EO_LEGACY_H_

#ifndef _ELM_SCROLLER_EO_CLASS_TYPE
#define _ELM_SCROLLER_EO_CLASS_TYPE

typedef Eo Elm_Scroller;

#endif

#ifndef _ELM_SCROLLER_EO_TYPES
#define _ELM_SCROLLER_EO_TYPES


#endif

/**
 * @brief Set custom theme elements for the scroller
 *
 * @param[in] obj The object.
 * @param[in] klass Klass name
 * @param[in] group Group name
 *
 * @ingroup Elm_Scroller_Group
 */
EAPI void elm_scroller_custom_widget_base_theme_set(Elm_Scroller *obj, const char *klass, const char *group);

/**
 * @brief Set the maximum of the movable page at a flicking.
 *
 * The value of maximum movable page should be more than 1.
 *
 * @param[in] obj The object.
 * @param[in] page_limit_h The maximum of the movable horizontal page
 * @param[in] page_limit_v The maximum of the movable vertical page
 *
 * @since 1.8
 *
 * @ingroup Elm_Scroller_Group
 */
EAPI void elm_scroller_page_scroll_limit_set(const Elm_Scroller *obj, int page_limit_h, int page_limit_v);

/**
 * @brief Get the maximum of the movable page at a flicking.
 *
 * @param[in] obj The object.
 * @param[in] page_limit_h The maximum of the movable horizontal page
 * @param[in] page_limit_v The maximum of the movable vertical page
 *
 * @since 1.8
 *
 * @ingroup Elm_Scroller_Group
 */
EAPI void elm_scroller_page_scroll_limit_get(const Elm_Scroller *obj, int *page_limit_h, int *page_limit_v);

#endif
