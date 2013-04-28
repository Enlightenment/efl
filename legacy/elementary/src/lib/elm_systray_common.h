/**
 * Emitted when the systray is ready to be used by the application.
 */
EAPI extern int ELM_EVENT_SYSTRAY_READY;

/**
 * Category of the Status Notifier Item.
 *
 * @since 1.8
 */
typedef enum _Elm_Systray_Category
{
   ELM_SYSTRAY_CATEGORY_APP_STATUS, /**< Indicators of application status */
   ELM_SYSTRAY_CATEGORY_COMMUNICATIONS, /**< Communications apps */
   ELM_SYSTRAY_CATEGORY_SYS_SERVICES, /**< System Service apps */
   ELM_SYSTRAY_CATEGORY_HARDWARE, /**< Hardware indicators */
   ELM_SYSTRAY_CATEGORY_OTHER /**< Undefined category */
} Elm_Systray_Category;

/**
 * Application status information.
 *
 * @since 1.8
 */
typedef enum _Elm_Systray_Status
{
   ELM_SYSTRAY_STATUS_PASSIVE, /**< Passive (normal) */
   ELM_SYSTRAY_STATUS_ACTIVE, /**< Active */
   ELM_SYSTRAY_STATUS_ATTENTION /**< Needs Attention */
} Elm_Systray_Status;

