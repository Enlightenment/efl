#ifndef _ELM_ATSPI_BRIDGE_EO_H_
#define _ELM_ATSPI_BRIDGE_EO_H_

#ifndef _ELM_ATSPI_BRIDGE_EO_CLASS_TYPE
#define _ELM_ATSPI_BRIDGE_EO_CLASS_TYPE

typedef Eo Elm_Atspi_Bridge;

#endif

#ifndef _ELM_ATSPI_BRIDGE_EO_TYPES
#define _ELM_ATSPI_BRIDGE_EO_TYPES


#endif
/** AT-SPI bridge class
 *
 * @ingroup Elm_Atspi_Bridge
 */
#define ELM_ATSPI_BRIDGE_CLASS elm_atspi_bridge_class_get()

EWAPI const Efl_Class *elm_atspi_bridge_class_get(void);

/**
 * @brief Indicate if connection with AT-SPI2 bus has been established.
 *
 * @param[in] obj The object.
 *
 * @return @c true if connection has been established, @c false otherwise
 *
 * @ingroup Elm_Atspi_Bridge
 */
EOAPI Eina_Bool elm_obj_atspi_bridge_connected_get(const Eo *obj);

EWAPI extern const Efl_Event_Description _ELM_ATSPI_BRIDGE_EVENT_CONNECTED;

/** Connected with AT-SPI2 bus
 *
 * @ingroup Elm_Atspi_Bridge
 */
#define ELM_ATSPI_BRIDGE_EVENT_CONNECTED (&(_ELM_ATSPI_BRIDGE_EVENT_CONNECTED))

EWAPI extern const Efl_Event_Description _ELM_ATSPI_BRIDGE_EVENT_DISCONNECTED;

/** Disconnected from AT-SPI2 bus
 *
 * @ingroup Elm_Atspi_Bridge
 */
#define ELM_ATSPI_BRIDGE_EVENT_DISCONNECTED (&(_ELM_ATSPI_BRIDGE_EVENT_DISCONNECTED))

#endif
