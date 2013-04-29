/**
 * @def elm_layout_icon_set
 * Convenience macro to set the icon object in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_icon_set(_ly, _obj)                                 \
  do {                                                                 \
       const char *sig;                                                \
       elm_layout_content_set((_ly), "elm.swallow.icon", (_obj));      \
       if ((_obj)) sig = "elm,state,icon,visible";                     \
       else sig = "elm,state,icon,hidden";                             \
       elm_layout_signal_emit((_ly), sig, "elm");                      \
    } while (0)

/**
 * @def elm_layout_icon_get
 * Convenience macro to get the icon object from a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_icon_get(_ly) \
  elm_layout_content_get((_ly), "elm.swallow.icon")

/**
 * @def elm_layout_end_set
 * Convenience macro to set the end object in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_end_set(_ly, _obj)                                 \
  do {                                                                \
       const char *sig;                                               \
       elm_layout_content_set((_ly), "elm.swallow.end", (_obj));      \
       if ((_obj)) sig = "elm,state,end,visible";                     \
       else sig = "elm,state,end,hidden";                             \
       elm_layout_signal_emit((_ly), sig, "elm");                     \
    } while (0)

/**
 * @def elm_layout_end_get
 * Convenience macro to get the end object in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_end_get(_ly) \
  elm_layout_content_get((_ly), "elm.swallow.end")

