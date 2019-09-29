#ifndef _EFL2_INPUT_TEXT_EO_H_
#define _EFL2_INPUT_TEXT_EO_H_

#ifndef _EFL2_INPUT_TEXT_EO_CLASS_TYPE
#define _EFL2_INPUT_TEXT_EO_CLASS_TYPE

typedef Eo Efl2_Input_Text;

#endif

#ifndef _EFL2_INPUT_TEXT_EO_TYPES
#define _EFL2_INPUT_TEXT_EO_TYPES

/** Input panel (virtual keyboard) layout types. Type of input panel (virtual
 * keyboard) to use - this is a hint and may not provide exactly what is
 * desired.
 *
 * @ingroup Efl2_Input_Text_Panel
 */
typedef enum
{
  EFL2_INPUT_TEXT_PANEL_LAYOUT_NORMAL = 0, /**< Default layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_NUMBER, /**< Number layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_EMAIL, /**< Email layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_URL, /**< URL layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_PHONENUMBER, /**< Phone Number layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_IP, /**< IP layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_MONTH, /**< Month layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_NUMBERONLY, /**< Number Only layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_INVALID, /**< Never use this. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_HEX, /**< Hexadecimal layout. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_TERMINAL, /**< Command-line terminal layout
                                          * including esc, alt, ctrl key, so on
                                          * (no auto-correct, no
                                          * auto-capitalization). */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_PASSWORD, /**< Like normal, but no auto-correct,
                                          * no auto-capitalization etc. */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_DATETIME, /**< Date and time layout
                                          *
                                          * @since 1.8 */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_EMOTICON, /**< Emoticon layout
                                          *
                                          * @since 1.10 */
  EFL2_INPUT_TEXT_PANEL_LAYOUT_VOICE /**< Voice layout, but if the IME does not
                                      * support voice layout, then normal
                                      * layout will be shown.
                                      *
                                      * @since 1.19 */
} Efl2_Input_Text_Panel_Layout;

/** Input panel (virtual keyboard) language modes.
 *
 * @ingroup Efl2_Input_Text_Panel
 */
typedef enum
{
  EFL2_INPUT_TEXT_PANEL_LANG_AUTOMATIC = 0, /**< Automatic */
  EFL2_INPUT_TEXT_PANEL_LANG_ALPHABET /**< Alphabet */
} Efl2_Input_Text_Panel_Lang;

/** Autocapitalization Types. Choose method of auto-capitalization.
 *
 * @ingroup Efl2_Input_Text_Autocapital
 */
typedef enum
{
  EFL2_INPUT_TEXT_AUTOCAPITAL_TYPE_NONE = 0, /**< No auto-capitalization when
                                              * typing. */
  EFL2_INPUT_TEXT_AUTOCAPITAL_TYPE_WORD, /**< Autocapitalize each word typed. */
  EFL2_INPUT_TEXT_AUTOCAPITAL_TYPE_SENTENCE, /**< Autocapitalize the start of
                                              * each sentence. */
  EFL2_INPUT_TEXT_AUTOCAPITAL_TYPE_ALLCHARACTER /**< Autocapitalize all letters.
                                                 */
} Efl2_Input_Text_Autocapital_Type;

/** "Return" Key types on the input panel (virtual keyboard).
 *
 * @ingroup Efl2_Input_Text_Panel_Return_Key
 */
typedef enum
{
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_DEFAULT = 0, /**< Default. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_DONE, /**< Done. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_GO, /**< Go. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_JOIN, /**< Join. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_LOGIN, /**< Login. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_NEXT, /**< Next. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_SEARCH, /**< Search string or magnifier
                                                 * icon. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_SEND, /**< Send. */
  EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_SIGNIN /**< Sign-in
                                                *
                                                * @since 1.8 */
} Efl2_Input_Text_Panel_Return_Key_Type;

/** Enumeration that defines the types of Input Hints.
 *
 * @since 1.12
 *
 * @ingroup Efl2_Input
 */
typedef enum
{
  EFL2_INPUT_HINT_NONE = 0, /**< No active hints
                            *
                            * @since 1.12 */
  EFL2_INPUT_HINT_AUTO_COMPLETE = 1 /* 1 >> 0 */, /**< Suggest word auto
                                                  * completion
                                                  *
                                                  * @since 1.12 */
  EFL2_INPUT_HINT_SENSITIVE_DATA = 2 /* 1 >> 1 */, /**< Typed text should not be
                                                   * stored.
                                                   *
                                                   * @since 1.12 */
  EFL2_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_DATE = 256, /**< Autofill hint for a credit card
                                                              * expiration date
                                                              *
                                                              * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_DAY = 512, /**< Autofill hint for a credit card
                                                             * expiration day
                                                             *
                                                             * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_MONTH = 768, /**< Autofill hint for a credit card
                                                               * expiration month
                                                               *
                                                               * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_CREDIT_CARD_EXPIRATION_YEAR = 1024, /**< Autofill hint for a credit card
                                                               * expiration year
                                                               *
                                                               * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_CREDIT_CARD_NUMBER = 1280, /**< Autofill hint for a
                                                      * credit card number
                                                      *
                                                      * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_EMAIL_ADDRESS = 1536, /**< Autofill hint for an email
                                                 * address
                                                 *
                                                 * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_NAME = 1792, /**< Autofill hint for a user's real name
                                        *
                                        * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_PHONE = 2048, /**< Autofill hint for a phone number
                                         *
                                         * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_POSTAL_ADDRESS = 2304, /**< Autofill hint for a postal
                                                  * address
                                                  *
                                                  * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_POSTAL_CODE = 2560, /**< Autofill hint for a postal
                                               * code
                                               *
                                               * @since 1.21 */
  EFL2_INPUT_HINT_AUTOFILL_ID = 2816 /**< Autofill hint for a user's ID
                                     *
                                     * @since 1.21 */
} Efl2_Input_Text_Hints;


#endif
#ifdef EFL_BETA_API_SUPPORT
/** All the functionality relating to input hints
 *
 * @ingroup Efl2_Input_Text
 */
#define EFL2_INPUT_TEXT_INTERFACE efl2_input_text_interface_get()

EWAPI const Efl_Class *efl2_input_text_interface_get(void);

/**
 * @brief Set the attribute to show the input panel in case of only a user's
 * explicit Mouse Up event. It doesn't request to show the input panel even
 * though it has focus.
 *
 * @param[in] obj The object.
 * @param[in] ondemand If @c true, the input panel will be shown in case of
 * only Mouse up event. (Focus event will be ignored.)
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_show_on_demand_set(Eo *obj, Eina_Bool ondemand);

/**
 * @brief Get the attribute to show the input panel in case of only an user's
 * explicit Mouse Up event.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the input panel will be shown in case of only Mouse up
 * event. (Focus event will be ignored.)
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Eina_Bool efl2_input_text_input_panel_show_on_demand_get(const Eo *obj);

/**
 * @brief Set the language mode of the input panel.
 *
 * This API can be used if you want to show the alphabet keyboard mode.
 *
 * @param[in] obj The object.
 * @param[in] lang Language to be set to the input panel.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_language_set(Eo *obj, Efl2_Input_Text_Panel_Lang lang);

/**
 * @brief Get the language mode of the input panel.
 *
 * @param[in] obj The object.
 *
 * @return Language to be set to the input panel.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Efl2_Input_Text_Panel_Lang efl2_input_text_input_panel_language_get(const Eo *obj);

/**
 * @brief Set the input panel layout variation of the entry
 *
 * @param[in] obj The object.
 * @param[in] variation Layout variation type.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_layout_variation_set(Eo *obj, int variation);

/**
 * @brief Get the input panel layout variation of the entry
 *
 * @param[in] obj The object.
 *
 * @return Layout variation type.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI int efl2_input_text_input_panel_layout_variation_get(const Eo *obj);

/**
 * @brief Set the autocapitalization type on the immodule.
 *
 * @param[in] obj The object.
 * @param[in] autocapital_type The type of autocapitalization.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_autocapital_type_set(Eo *obj, Efl2_Input_Text_Autocapital_Type autocapital_type);

/**
 * @brief Get the autocapitalization type on the immodule.
 *
 * @param[in] obj The object.
 *
 * @return The type of autocapitalization.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Efl2_Input_Text_Autocapital_Type efl2_input_text_autocapital_type_get(const Eo *obj);

/**
 * @brief Set the return key on the input panel to be disabled.
 *
 * @param[in] obj The object.
 * @param[in] disabled The state to put in in: @c true for disabled, @c false
 * for enabled.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_return_key_disabled_set(Eo *obj, Eina_Bool disabled);

/**
 * @brief Get whether the return key on the input panel should be disabled or
 * not.
 *
 * @param[in] obj The object.
 *
 * @return The state to put in in: @c true for disabled, @c false for enabled.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Eina_Bool efl2_input_text_input_panel_return_key_disabled_get(const Eo *obj);

/**
 * @brief Set whether the entry should allow predictive text.
 *
 * @param[in] obj The object.
 * @param[in] prediction Whether the entry should allow predictive text.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_prediction_allow_set(Eo *obj, Eina_Bool prediction);

/**
 * @brief Get whether the entry allows predictive text.
 *
 * @param[in] obj The object.
 *
 * @return Whether the entry should allow predictive text.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Eina_Bool efl2_input_text_prediction_allow_get(const Eo *obj);

/**
 * @brief Sets the input hint which allows input methods to fine-tune their
 * behavior.
 *
 * @param[in] obj The object.
 * @param[in] hints Input hint.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_hint_set(Eo *obj, Efl2_Input_Text_Hints hints);

/**
 * @brief Gets the value of input hint.
 *
 * @param[in] obj The object.
 *
 * @return Input hint.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Efl2_Input_Text_Hints efl2_input_text_input_hint_get(const Eo *obj);

/**
 * @brief Set the input panel layout of the entry.
 *
 * @param[in] obj The object.
 * @param[in] layout Layout type.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_layout_set(Eo *obj, Efl2_Input_Text_Panel_Layout layout);

/**
 * @brief Get the input panel layout of the entry.
 *
 * @param[in] obj The object.
 *
 * @return Layout type.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Efl2_Input_Text_Panel_Layout efl2_input_text_input_panel_layout_get(const Eo *obj);

/**
 * @brief Set the "return" key type. This type is used to set string or icon on
 * the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type.
 *
 * @param[in] obj The object.
 * @param[in] return_key_type The type of "return" key on the input panel.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_return_key_type_set(Eo *obj, Efl2_Input_Text_Panel_Return_Key_Type return_key_type);

/**
 * @brief Get the "return" key type.
 *
 * @param[in] obj The object.
 *
 * @return The type of "return" key on the input panel.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Efl2_Input_Text_Panel_Return_Key_Type efl2_input_text_input_panel_return_key_type_get(const Eo *obj);

/**
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @param[in] obj The object.
 * @param[in] enabled If @c true, the input panel is appeared when entry is
 * clicked or has a focus.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_enabled_set(Eo *obj, Eina_Bool enabled);

/**
 * @brief Get the attribute to show the input panel automatically.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the input panel is appeared when entry is clicked or has
 * a focus.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI Eina_Bool efl2_input_text_input_panel_enabled_get(const Eo *obj);

/**
 * @brief Set whether the return key on the input panel is disabled
 * automatically when entry has no text.
 *
 * If @c enabled is @c true, the return key on input panel is disabled when the
 * entry has no text. The return key on the input panel is automatically
 * enabled when the entry has text. The default value is @c false.
 *
 * @param[in] obj The object.
 * @param[in] enabled If @c enabled is @c true, the return key is automatically
 * disabled when the entry has no text.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_return_key_autoenabled_set(Eo *obj, Eina_Bool enabled);

/**
 * @brief Show the input panel (virtual keyboard) based on the input panel
 * property of entry such as layout, autocapital types and so on.
 *
 * Note that input panel is shown or hidden automatically according to the
 * focus state of entry widget. This API can be used in the case of manually
 * controlling by using @ref efl2_input_text_input_panel_enabled_set(en,
 * @c false).
 * @param[in] obj The object.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_show(Eo *obj);

/**
 * @brief Set the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input
 * panel. The data format MUST be negotiated by both application and the input
 * panel. The size and format of data are defined by the input panel.
 *
 * @param[in] obj The object.
 * @param[in] data The specific data to be set to the input panel.
 * @param[in] len The length of data, in bytes, to send to the input panel.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_imdata_set(Eo *obj, const void *data, int len);

/**
 * @brief Get the specific data of the current input panel.
 *
 * @param[in] obj The object.
 * @param[in,out] data The specific data to be obtained from the input panel.
 * @param[out] len The length of data.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_imdata_get(const Eo *obj, void *data, int *len);

/**
 * @brief Hide the input panel (virtual keyboard).
 *
 * Note that input panel is shown or hidden automatically according to the
 * focus state of entry widget. This API can be used in the case of manually
 * controlling by using @ref efl2_input_text_input_panel_enabled_set(en,
 * @c false)
 * @param[in] obj The object.
 *
 * @ingroup Efl2_Input_Text
 */
EOAPI void efl2_input_text_input_panel_hide(Eo *obj);
#endif /* EFL_BETA_API_SUPPORT */

#endif
