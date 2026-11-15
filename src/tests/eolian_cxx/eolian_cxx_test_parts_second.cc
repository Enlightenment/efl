#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_CXXPERIMENTAL

#define EFL_PART_PROTECTED
#include <Eo.h>
#include <Eina.h>
#include <Efl.h>
#include <Ecore.h>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include <parts_inner.eo.hh>
#include <parts_inner.eo.impl.hh>
#include <parts_owner.eo.hh>
#include <parts_owner.eo.impl.hh>

/* A second translation unit including the generated implementation headers:
 * with non-inline part accessors the suite does not link. */
bool
eolian_cxx_test_parts_second(::parts::Owner& owner)
{
  ::parts::Inner inner = owner.inner();
  return inner._eo_ptr() && efl_isa(inner._eo_ptr(), PARTS_INNER_CLASS);
}
