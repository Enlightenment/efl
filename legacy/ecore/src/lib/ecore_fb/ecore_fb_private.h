#ifndef _ECORE_FB_PRIVATE_H
#define _ECORE_FB_PRIVATE_H

#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/version.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
 #define kernel_ulong_t unsigned long 
 #define BITS_PER_LONG 32
 #include <linux/input.h>
 #undef kernel_ulong_t
 #undef BITS_PER_LONG
#else
 #include <linux/input.h>
#endif

#include <signal.h>
#include <fcntl.h>
#include <errno.h>

/* ecore_fb_li.c */
struct _Ecore_Fb_Input_Device
{
	int fd;
	Ecore_Fd_Handler *handler;
	int listen;
	struct {
		Ecore_Fb_Input_Device_Cap cap;
		char *name;
		char *dev;
	} info;
	struct
	{
		/* common mouse */
		int x,y;
		int w,h;
		
		double last;
		double prev;
		double threshold;
		/* absolute axis */
		int min_w, min_h;
		double rel_w, rel_h;
		int event;
	} mouse;
	struct
	{
		int shift;
		int ctrl;
		int alt;
		int lock;
	} keyboard;
};

/* ecore_fb_ts.c */
EAPI int    ecore_fb_ts_init(void);
EAPI void   ecore_fb_ts_shutdown(void);

/* ecore_fb_vt.c */
int  ecore_fb_vt_init(void);
void ecore_fb_vt_shutdown(void);

/* hacks to stop people NEEDING #include <linux/h3600_ts.h> */
#ifndef TS_SET_CAL
#define TS_SET_CAL 0x4014660b
#endif
#ifndef TS_GET_CAL
#define TS_GET_CAL 0x8014660a
#endif
  
#endif
