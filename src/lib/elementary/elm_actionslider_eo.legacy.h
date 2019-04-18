#ifndef _ELM_ACTIONSLIDER_EO_LEGACY_H_
#define _ELM_ACTIONSLIDER_EO_LEGACY_H_

#ifndef _ELM_ACTIONSLIDER_EO_CLASS_TYPE
#define _ELM_ACTIONSLIDER_EO_CLASS_TYPE

typedef Eo Elm_Actionslider;

#endif

#ifndef _ELM_ACTIONSLIDER_EO_TYPES
#define _ELM_ACTIONSLIDER_EO_TYPES

/** A position for indicators, magnets, and enabled items.
 *
 * @ingroup Elm_Actionslider
 */
typedef enum
{
  ELM_ACTIONSLIDER_NONE = 0, /**< No position is set. */
  ELM_ACTIONSLIDER_LEFT = 1 /* 1 >> 0 */, /**< Left position. */
  ELM_ACTIONSLIDER_CENTER = 2 /* 1 >> 1 */, /**< Center position. */
  ELM_ACTIONSLIDER_RIGHT = 4 /* 1 >> 2 */, /**< Right position. */
  ELM_ACTIONSLIDER_ALL = 7 /* (1 >> 3) * 1 */ /**< All positions for
                                               * left/center/right. */
} Elm_Actionslider_Pos;


#endif

/**
 * @brief Actionslider position indicator
 *
 * @param[in] obj The object.
 * @param[in] pos The position of the indicator.
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI void elm_actionslider_indicator_pos_set(Elm_Actionslider *obj, Elm_Actionslider_Pos pos);

/**
 * @brief Actionslider position indicator
 *
 * @param[in] obj The object.
 *
 * @return The position of the indicator.
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI Elm_Actionslider_Pos elm_actionslider_indicator_pos_get(const Elm_Actionslider *obj);

/**
 * @brief Actionslider magnet position.
 *
 * To make multiple positions magnets OR them together (e.g.
 * ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
 *
 * @param[in] obj The object.
 * @param[in] pos Bit mask indicating the magnet positions.
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI void elm_actionslider_magnet_pos_set(Elm_Actionslider *obj, Elm_Actionslider_Pos pos);

/**
 * @brief Actionslider magnet position.
 *
 * @param[in] obj The object.
 *
 * @return Bit mask indicating the magnet positions.
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI Elm_Actionslider_Pos elm_actionslider_magnet_pos_get(const Elm_Actionslider *obj);

/**
 * @brief Actionslider enabled position.
 *
 * To set multiple positions as enabled OR them together(e.g.
 * ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
 *
 * @note All the positions are enabled by default.
 *
 * @param[in] obj The object.
 * @param[in] pos Bit mask indicating the enabled positions.
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI void elm_actionslider_enabled_pos_set(Elm_Actionslider *obj, Elm_Actionslider_Pos pos);

/**
 * @brief Actionslider enabled position.
 *
 * @param[in] obj The object.
 *
 * @return Bit mask indicating the enabled positions.
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI Elm_Actionslider_Pos elm_actionslider_enabled_pos_get(const Elm_Actionslider *obj);

/**
 * @brief Get actionslider selected label.
 *
 * @param[in] obj The object.
 *
 * @return Selected label
 *
 * @ingroup Elm_Actionslider_Group
 */
EAPI const char *elm_actionslider_selected_label_get(const Elm_Actionslider *obj);

#endif
