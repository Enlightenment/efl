#ifndef _ECORE_TXT_H
#define _ECORE_TXT_H

/**
 * @file Ecore_Txt.h
 * @brief Provides a text encoding conversion function.
 */

#ifdef __cplusplus
extern "C" {
#endif

char *ecore_txt_convert(char *enc_from, char *enc_to, char *text);

#ifdef __cplusplus
}
#endif

#endif
