#ifndef EOLIAN_AUX_H
#define EOLIAN_AUX_H

#include <Eolian.h>

#ifdef EAPI
# undef EAPI
#endif

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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page eolian_aux_main Eolian Auxiliary Library (BETA)
 *
 * @date 2018 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eolian_aux_main_intro
 *
 * @section eolian_main_intro Introduction
 *
 * This is the Eolian auxiliary library, providing API to support generators
 * and other utilities which do not fit into the primary Eolian library. All
 * these APIs are built on top of the core Eolian APIs.
 *
 *
 * Recommended reading:
 *
 * @li @ref Eolian
 *
 * @addtogroup Eolian
 * @{
 */

#ifdef EFL_BETA_API_SUPPORT

EAPI Eina_Hash *eolian_aux_state_class_children_find(const Eolian_State *state);

EAPI size_t eolian_aux_class_callables_get(const Eolian_Class *klass, Eina_List **funcs, Eina_List **events, size_t *ownfuncs, size_t *ownevs);

EAPI Eina_List *eolian_aux_function_all_implements_get(const Eolian_Function *func, Eina_Hash *class_children);

EAPI const Eolian_Implement *eolian_aux_implement_parent_get(const Eolian_Implement *impl);

EAPI const Eolian_Documentation *eolian_aux_implement_documentation_get(const Eolian_Implement *impl, Eolian_Function_Type ftype);

EAPI const Eolian_Documentation *eolian_aux_implement_documentation_fallback_get(const Eolian_Implement *impl);

#endif

/**
 * @}
 */

#ifdef __cplusplus
} // extern "C" {
#endif

#undef EAPI
#define EAPI

#endif
