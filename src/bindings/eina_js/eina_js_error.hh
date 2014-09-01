#ifndef EINA_JS_ERROR_HH
#define EINA_JS_ERROR_HH

#include <eina_js_compatibility.hh>

namespace efl { namespace eina { namespace js {

/**
 * Converts the error value set through Eina's error tool to a JavaScript
 * exception.
 *
 * The exception object will have a `code` string field with the `"Eina_Error"`
 * string value and a `value` string field with the value extracted from
 * `eina_error_msg_get`.
 *
 * It won't reset the error to NULL, so you can still access the error object,
 * but if you keep calling this function without clearing the error, a new
 * exception will be generated for each call after some error is reached. We,
 * therefore, suggest you to call `eina_error_set(0)` afterwards.
 */
js::compatibility_return_type convert_error_to_javascript_exception(v8::Isolate *isolate);

} } } // namespace efl::eina::js

#endif /* EINA_JS_ERROR_HH */
