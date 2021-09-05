
/**
 * @brief Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
typedef enum
{
  ELM_MAP_ZOOM_MODE_MANUAL = 0, /**< Zoom controlled manually by
                                 * elm_map_zoom_set(). It's set by default. */
  ELM_MAP_ZOOM_MODE_AUTO_FIT, /**< Zoom until map fits inside the scroll frame
                               * with no pixels outside this area. */
  ELM_MAP_ZOOM_MODE_AUTO_FILL, /**< Zoom until map fills scroll, ensuring no
                                * pixels are left unfilled. */
  ELM_MAP_ZOOM_MODE_LAST /**< Sentinel value to indicate last enum field during
                          * iteration */
} Elm_Map_Zoom_Mode;

/**
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI Evas_Object          *elm_map_add(Evas_Object *parent);


/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_zoom_set(Evas_Object *obj, int zoom);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI int elm_map_zoom_get(const Evas_Object *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_zoom_mode_set(Evas_Object *obj, Elm_Map_Zoom_Mode mode);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Zoom_Mode elm_map_zoom_mode_get(const Evas_Object *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_wheel_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI Eina_Bool elm_map_wheel_disabled_get(const Evas_Object *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_paused_set(Evas_Object *obj, Eina_Bool paused);

/**
 * @brief Widget is broken due to on-line service API breaks
 * Widget is broken due to on-line service API breaks
 * @ingroup Elm_Map_Group
 */
EAPI Eina_Bool elm_map_paused_get(const Evas_Object *obj);

#include "elm_map_eo.legacy.h"
