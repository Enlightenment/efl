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
 * This must be called before the objects are created to be sure no object is
 * visible and no generation started.
 *
 * Example of usage:
 *
 * @code
 * #include <Elementary.h>
 * #ifndef ELM_LIB_QUICKLAUNCH
 * EAPI_MAIN int
 * elm_main(int argc, char **argv)
 * {
 *    Ethumb_Client *client;
 *
 *    elm_need_ethumb();
 *
 *    // ... your code
 *
 *    client = elm_thumb_ethumb_client_get();
 *    if (!client)
 *      {
 *         ERR("could not get ethumb_client");
 *         return 1;
 *      }
 *    ethumb_client_size_set(client, 100, 100);
 *    ethumb_client_crop_align_set(client, 0.5, 0.5);
 *    // ... your code
 *
 *    // Create elm_thumb objects here
 *
 *    elm_run();
 *    elm_shutdown();
 *    return 0;
 * }
 * #endif
 * ELM_MAIN()
 * @endcode
 *
 * @note There's only one client handle for Ethumb, so once a configuration
 * change is done to it, any other request for thumbnails (for any thumbnail
 * object) will use that configuration. Thus, this configuration is global.
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

