#ifndef _EVAS_FB_H
#define _EVAS_FB_H 1

/* -------------------------------------------------------------------- */
/* LINUX FBCON FRAMEBUFFER UTILITY CODE                                 */
/* makes setting up the framebuffer easy. Also makes it eays to port to */
/* some other system if needed.                                         */
/* Copyright (c) 1999 - Carsten Haitzler (The Rasterman)                */
/* -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>

typedef struct _fb_mode FB_Mode;

struct _fb_mode
{
  int             width;
  int             height;
  int             refresh;
  int             depth;
  int             bpp;
  int             fb_fd;
  void           *mem;
  int             mem_offset;
  struct fb_var_screeninfo fb_var;
};

/* init a framebuffer (and switch to) vt number vt. If vt == 0 use current   */
/* vt                                                                        */
void fb_init(int vt, int device);
/* call this afetr setting or getting the fb mode (whichever) to complete    */
/* the dsetup                                                                */
int  fb_postinit(FB_Mode *mode);
/* console switching - if a switch was requested this with block if block    */
/* is 1, otherwise it will return 1 if current console is active or 0 if     */
/* the user has switched away in the meantime                                */
int  fb_await_switch(int block);
/* list all current possible video modes listed in /etc/fb.modes             */
/* returns pointer to an aray of FB_Mode, and sets num_return to the number  */
/* of elements int he returned array                                         */
FB_Mode *fb_list_modes(int *num_return);
/* sets the fb mode to the resolution width x height and the depth to depth. */
/* and if refresh > 0 attempts to use a mode with a refresh rate (in Hz) of  */
/* that. If this fails it will return NULL - if it succeeds it will return   */
/* a pointer to the FB_Mode thatwas set. only modes in /etc/fb.modes will    */
/* be used. If refresh is 0 it uses the DEFAULT mode (the one with no        */
/* refresh rate at the end of its name (WIDTHxHEIGHT-REFRESH)                */
/* NB: in 8bpp you get a 332 palette. This is fixed so all modes are         */
/* "truecolor" - the onyl difference is how many bits bep red, green and     */
/* blue channel. This is for speed reasons                                   */
FB_Mode *fb_setmode(int width, int height, int depth, int refresh);
/* returns the current fb mode being used in FB_Mode                         */
FB_Mode *fb_getmode(void);
/* changes the bit depth of the current fb mode to depth and returns a new   */
/* handle to a new fb mode with updated parameters. frees cur_mode for you.  */
FB_Mode *fb_changedepth(FB_Mode *cur_mode, int depth);
/* changes resolution - retaining current depth of the current mode,         */
/* returning a handle to the new mode once done. frees cur_mode for you.     */
FB_Mode *fb_changeres(FB_Mode *cur_mode, int width, int height, int refresh);
/* chnages both resolution and depth and returns a handle to the new mode    */
/* when done. frees cur_mode for you                                         */
FB_Mode *fb_changemode(FB_Mode *cur_mode, int width, int height, int depth, int refresh);

/* ------------------------------------------------------------------------- */
/* How to init:                                                              */
/* (Example)                                                                 */

/* FB_Mode *mode;                                                            */
/* int fb_fd = -1;                                                           */
/* fb_init(0);                                                               */
/* mode = fb_setmode(640, 480, 8, 0);                                        */
/* if (mode)                                                                 */
/*   fb_fd = fb_postinit(mode);                                              */
/* if (fb_fd == -1)                                                          */
/*   {                                                                       */
/*     fprintf(stderr, "Frambuffer init failed\n");                          */
/*     exit(1);                                                              */
/*   }                                                                       */
/* .... code to play with the FB                                             */
#endif
