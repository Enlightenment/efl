/**
 * @addtogroup Elm_Thumb
 *
 * @{
 */

/**
 * Get the ethumb_client handle so custom configuration can be made.
 *
 * @return Ethumb_Client instance or NULL.
 *
 * The ethumb client handle returned may or may not be NULL. It may or may
 * not survive into the future (ethumbd may become disconnected). Do not use
 * this unless you know what you are doing.
 */
EAPI void                       *elm_thumb_ethumb_client_get(void);

/**
 * Get the ethumb_client connection state.
 *
 * @return @c EINA_TRUE if the client is connected to the server or @c
 * EINA_FALSE otherwise.
 */
EAPI Eina_Bool                   elm_thumb_ethumb_client_connected_get(void);

/**
 * @}
 */
