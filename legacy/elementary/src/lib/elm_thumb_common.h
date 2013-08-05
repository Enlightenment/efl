/**
 * @enum Elm_Thumb_Animation_Setting
 * @typedef Elm_Thumb_Animation_Setting
 *
 * Used to set if a video thumbnail is animating or not.
 *
 * @ingroup Thumb
 */
typedef enum
{
   ELM_THUMB_ANIMATION_START = 0, /**< Play animation once */
   ELM_THUMB_ANIMATION_LOOP, /**< Keep playing animation until stop is requested */
   ELM_THUMB_ANIMATION_STOP, /**< Stop playing the animation */
   ELM_THUMB_ANIMATION_LAST
} Elm_Thumb_Animation_Setting;

/**
 * Get the ethumb_client handle so custom configuration can be made.
 *
 * @return Ethumb_Client instance or NULL.
 *
 * The ethumb client handle returned may or may not be NULL. It may or may
 * not survive into the future (ethumbd may become disconnected). Do not use
 * this unless you know what you are doing.
 *
 * @ingroup Thumb
 */
EAPI void                       *elm_thumb_ethumb_client_get(void);

/**
 * Get the ethumb_client connection state.
 *
 * @return EINA_TRUE if the client is connected to the server or EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                   elm_thumb_ethumb_client_connected_get(void);

