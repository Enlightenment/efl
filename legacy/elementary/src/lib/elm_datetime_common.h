/**
 * Identifies a Datetime field, The widget supports 6 fields : Year, month,
 * Date, Hour, Minute, AM/PM
 *
 */
typedef enum _Elm_Datetime_Field_Type
{
   ELM_DATETIME_YEAR    = 0, /**< Indicates Year field */
   ELM_DATETIME_MONTH   = 1, /**< Indicates Month field */
   ELM_DATETIME_DATE    = 2, /**< Indicates Date field */
   ELM_DATETIME_HOUR    = 3, /**< Indicates Hour field */
   ELM_DATETIME_MINUTE  = 4, /**< Indicates Minute field */
   ELM_DATETIME_AMPM    = 5, /**< Indicates AM/PM field */
} Elm_Datetime_Field_Type;

