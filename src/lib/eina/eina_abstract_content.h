#ifndef EINA_ABSTRACT_CONTENT_H
#define EINA_ABSTRACT_CONTENT_H


/**
 * @typedef Eina_Content
 * Container for any type of content.
 *
 * Each Eina_Content is made of an Eina_Slice of memory and an IANA MIME type:
 * https://www.iana.org/assignments/media-types/media-types.xhtml
 *
 * @note If the type is any kind of text the last byte of the slice must be \0.
 *
 * @since 1.24
 */
typedef struct _Eina_Content Eina_Content;

/**
 * @typedef Eina_Content_Conversion_Callback
 *
 * Method called when conversion from one type to another is requested.
 * The from and to types are specified when the callback is registered.
 * The to type is also passed in the callback here.
 * The type of the from pointer does not need to be checked.
 */
typedef Eina_Content* (*Eina_Content_Conversion_Callback)(Eina_Content *from, const char *to_type);

/**
 * Get the path to a file, containing the slice memory as content.
 *
 * @param[in] content The content that will be in the file.
 *
 * @return The path to the file. Do not free this.
 *
 */
EINA_API const char* eina_content_as_file(Eina_Content *content);

/**
 * Convert the content of the object to another type.
 *
 * In case the conversion cannot be performed, NULL is returned.
 *
 * @param[in] content The content to convert.
 * @param[in] new_type The new type the returned content will have.
 *
 * @return A new content object. The caller of this function is owning this.
 */
EINA_API Eina_Content* eina_content_convert(Eina_Content *content, const char *new_type);

/**
 * Get the type of the passed content.
 *
 * @param[in] content The content to fetch the type from.
 *
 * @return The type of this content. Do no free this.
 */
EINA_API const char* eina_content_type_get(Eina_Content *content);

/**
 * Get the Eina_Slice of the passed content.
 *
 * @param[in] content The content to fetch the data from.
 *
 * @return An Eina_Slice containing the data. Do not free.
 */
EINA_API Eina_Slice eina_content_data_get(Eina_Content *content);

/**
 * Create a new content object, with the provided data and type.
 *
 * @param[in] data A slice of memory. The memory is copied.
 * @param[in] type The type this data represents.
 *
 * @return The new content object. The caller owns this object.
 */
EINA_API Eina_Content* eina_content_new(Eina_Slice data, const char *type);

/**
 * Free the content object.
 *
 * @param[in] content The content to free.
 */
EINA_API void eina_content_free(Eina_Content *content);

/**
 * Register a new conversion callback.
 *
 * @param[in] from The type to convert from.
 * @param[in] to The type to convert to.
 *
 * @return True if the callback was successfully registered.
 */
EINA_API Eina_Bool eina_content_converter_conversion_register(const char *from, const char *to, Eina_Content_Conversion_Callback convertion);

/**
 * Check if a specific conversion can be performed.
 *
 * A conversion can only be performed if a callback is registered.
 *
 * @param[in] from The type to convert from.
 * @param[in] to The type to convert to.
 *
 * @return True if the conversion can be performed.
 */
EINA_API Eina_Bool eina_content_converter_convert_can(const char *from, const char *to);

/**
 * Returns an iterator containing all the target types that the provided source type can be converted to.
 *
 * @param[in] from The type to convert from.
 *
 * @return An Iterator containing MIME type strings. Free this via eina_iterator_free.
 */
EINA_API Eina_Iterator* eina_content_converter_possible_conversions(const char *from);

EINA_API extern const Eina_Value_Type *EINA_VALUE_TYPE_CONTENT;

/**
 * Convert the Eina_Content object to an Eina_Value.
 *
 * @param[in] content The Eina_Content struct that will be converted to an Eina_Value.
 *
 * @return An newly-allocated Eina_Value. Caller owns it.
 */
EINA_API Eina_Value* eina_value_content_new(Eina_Content *content);

/**
 * Creates an Eina_Value from an Eina_Content.
 *
 * @param[in] content The Eina_Content struct that will be converted to an Eina_Value.
 *
 * @return An Eina_Value with type EINA_VALUE_TYPE_CONTENT.
 */
EINA_API Eina_Value eina_value_content_init(Eina_Content *content);

/**
 * Gets the content from the Eina_Value.
 *
 * If the value is not of the type EINA_VALUE_TYPE_CONTENT, NULL will be returned and an error will be printed.
 *
 * @param[in] value The value to get the content from
 *
 * @return A newly-allocated Eina_Content. Caller owns it.
 */
EINA_API Eina_Content* eina_value_to_content(const Eina_Value *value);


static inline Eina_Iterator*
eina_content_possible_conversions(Eina_Content *content)
{
   return eina_content_converter_possible_conversions(eina_content_type_get(content));
}

#endif
