/**
 * Identifies the day of the week.
 * API can call the selection/unselection of day with this as a parameter.
 *
 * @see elm_dayselector_day_selected_set()
 * @see elm_dayselector_day_selected_get()
 */
typedef enum
{
   ELM_DAYSELECTOR_SUN = 0,/**< indicates Sunday */
   ELM_DAYSELECTOR_MON,    /**< indicates Monday */
   ELM_DAYSELECTOR_TUE,    /**< indicates Tuesday */
   ELM_DAYSELECTOR_WED,    /**< indicates Wednesday */
   ELM_DAYSELECTOR_THU,    /**< indicates Thursday */
   ELM_DAYSELECTOR_FRI,    /**< indicates Friday */
   ELM_DAYSELECTOR_SAT,    /**< indicates Saturday */
   ELM_DAYSELECTOR_MAX     /**< Sentinel value, @b don't use */
} Elm_Dayselector_Day;

