#ifndef _EO_INTERNAL_H_
#define _EO_INTERNAL_H_

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

typedef unsigned char Eina_Bool;
typedef struct _Eo_Opaque Eo;

EAPI void ___efl_auto_unref_set(Eo *obj_id, Eina_Bool enable);

EAPI int ___efl_ref2_count(const Eo *obj_id);
EAPI void ___efl_ref2_reset(const Eo *obj_id);

#undef EAPI
#define EAPI

#endif
