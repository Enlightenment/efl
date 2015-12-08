#ifndef __EVIL_PRIVATE_H__
#define __EVIL_PRIVATE_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define EVIL_UNUSED __attribute__ ((__unused__))
# else
#  define EVIL_UNUSED
# endif
#endif

#ifdef __cplusplus
}
#endif


#endif /* __EVIL_PRIVATE_H__ */
