#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eo_Js.hh>
#include <Eina.hh>
#include <Eo.hh>

#include <check.h>


#include <iostream>
#include <cassert>
#include <fstream>

typedef struct _Elm_Calendar_Mark Elm_Calendar_Mark;

#include "constructor_method_class.eo.js.cc"
#include "test_object.eo.js.cc"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
#  define EAPI __declspec(dllimport)
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

void eolian_js_test_register_eolian_js_binding(v8::Handle<v8::Object> exports)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  test::register_object(exports, isolate);
  register_constructor_method_class(exports, isolate);
}
