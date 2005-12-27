#ifndef _ECORE_TXT_H
#define _ECORE_TXT_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
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

/**
 * @file Ecore_Txt.h
 * @brief Provides a text encoding conversion function.
 */

#ifdef __cplusplus
extern "C" {
#endif

EAPI char *ecore_txt_convert(const char *enc_from, const char *enc_to, const char *text);

#ifdef __cplusplus
}
#endif

#endif
