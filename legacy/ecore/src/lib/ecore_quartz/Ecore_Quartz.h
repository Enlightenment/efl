/*
* vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
*/

#ifndef _ECORE_QUARTZ_H
#define _ECORE_QUARTZ_H

#ifdef EAPI
# undef EAPI
#endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif

#ifdef __cplusplus
extern "C" {
#endif

EAPI extern int ECORE_QUARTZ_EVENT_GOT_FOCUS;
EAPI extern int ECORE_QUARTZ_EVENT_LOST_FOCUS;
EAPI extern int ECORE_QUARTZ_EVENT_RESIZE;
EAPI extern int ECORE_QUARTZ_EVENT_EXPOSE;

typedef struct _Ecore_Quartz_Event_Video_Resize Ecore_Quartz_Event_Video_Resize;
struct _Ecore_Quartz_Event_Video_Resize
{
   int             w;
   int             h;
};

EAPI int        ecore_quartz_init(const char *name);
EAPI int        ecore_quartz_shutdown(void);
EAPI void       ecore_quartz_feed_events(void);

#ifdef __cplusplus
}
#endif

#endif
