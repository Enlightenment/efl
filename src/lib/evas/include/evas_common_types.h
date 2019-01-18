#ifndef EVAS_COMMON_TYPES_H
# define EVAS_COMMON_TYPES_H 1
typedef unsigned long long DATA64;
typedef unsigned int       DATA32;
typedef unsigned short     DATA16;
typedef unsigned char      DATA8;

typedef void (*RGBA_Gfx_Func)    (DATA32 *src, DATA8 *mask, DATA32 col, DATA32 *dst, int len);
typedef void (*RGBA_Gfx_Pt_Func) (DATA32 src, DATA8 mask, DATA32 col, DATA32 *dst);
#endif
