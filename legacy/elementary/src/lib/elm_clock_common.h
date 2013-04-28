/**
 * Identifiers for which clock digits should be editable, when a
 * clock widget is in edition mode. Values may be OR-ed together to
 * make a mask, naturally.
 *
 * @see elm_clock_edit_set()
 * @see elm_clock_edit_mode_set()
 */
typedef enum
{
   ELM_CLOCK_EDIT_DEFAULT = 0, /**< Default value. Means that all digits are editable, when in edition mode. */
   ELM_CLOCK_EDIT_HOUR_DECIMAL = 1 << 0, /**< Decimal digit of hours value should be editable */
   ELM_CLOCK_EDIT_HOUR_UNIT = 1 << 1, /**< Unit digit of hours value should be editable */
   ELM_CLOCK_EDIT_MIN_DECIMAL = 1 << 2, /**< Decimal digit of minutes value should be editable */
   ELM_CLOCK_EDIT_MIN_UNIT = 1 << 3, /**< Unit digit of minutes value should be editable */
   ELM_CLOCK_EDIT_SEC_DECIMAL = 1 << 4, /**< Decimal digit of seconds value should be editable */
   ELM_CLOCK_EDIT_SEC_UNIT = 1 << 5, /**< Unit digit of seconds value should be editable */
   ELM_CLOCK_EDIT_ALL = (1 << 6) - 1 /**< All digits should be editable */
} Elm_Clock_Edit_Mode;

