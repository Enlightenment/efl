#ifndef LIBEFL_MONO_NATIVE_TEST_H
#define LIBEFL_MONO_NATIVE_TEST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define EFL_PART_PROTECTED

#include <Ecore.h>
#include <Eo.h>

#ifdef EOAPI
#undef EOAPI
#endif

#ifdef EWAPI
#undef EWAPI
#endif

#ifdef EAPI
#undef EAPI
#endif

#define EOAPI EAPI EAPI_WEAK
#define EWAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

#include "dummy_test_iface.eo.h"
#include "dummy_inherit_iface.eo.h"
#include "dummy_numberwrapper.eo.h"
#include "dummy_test_object.eo.h"
#include "dummy_child.eo.h"
#include "dummy_inherit_helper.eo.h"
#include "dummy_part_holder.eo.h"
#include "dummy_event_manager.eo.h"

#include <interfaces/efl_part.eo.h>

#define EQUAL(a, b) ((a) == (b) ? 1 : (fprintf(stderr, "NOT EQUAL! %s:%i (%s)", __FILE__, __LINE__, __FUNCTION__), fflush(stderr), 0))
#define STR_EQUAL(a, b) (strcmp((a), (b)) == 0 ? 1 : (fprintf(stderr, "NOT EQUAL! %s:%i (%s) '%s' != '%s'", __FILE__, __LINE__, __FUNCTION__, (a), (b)), fflush(stderr), 0))

#endif
