#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Fb.h"
#include "ecore_fb_private.h"
#include <limits.h>

static void _ecore_fb_size_get(const char *name, int *w, int *h);

static int _ecore_fb_init_count = 0;
static int _ecore_fb_console_w = 0;
static int _ecore_fb_console_h = 0;

/**
 * @addtogroup Ecore_FB_Group Ecore_FB - Frame buffer convenience functions.
 *
 * @{
 */

static sighandler_t oldhand = NULL;

static void
nosigint(int val EINA_UNUSED)
{
}

/**
 * @brief Initialize the Ecore_Fb library.
 *
 * @param name Device target name.
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up all the Ecore_Fb library. It returns 0 on
 * failure, otherwise it returns the number of times it has already
 * been called.
 *
 * When Ecore_Fb is not used anymore, call ecore_fb_shutdown() to shut down
 * the Ecore_Fb library.
 */
EAPI int
ecore_fb_init(const char *name)
{
   const char *s;

   if (++_ecore_fb_init_count != 1)
      return _ecore_fb_init_count;

   s = getenv("ECORE_FB_NO_VT");
   if ((!s) || (atoi(s) == 0))
     {
        if (!ecore_fb_vt_init())
          return --_ecore_fb_init_count;
     }

   if (!oldhand)
     {
        oldhand = signal(SIGINT, nosigint);
     }

   _ecore_fb_size_get(name, &_ecore_fb_console_w, &_ecore_fb_console_h);

   return _ecore_fb_init_count;
}

/**
 * @brief Shut down the Ecore_Fb library.
 *
 * @return 0 when the library is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Ecore_Fb library. It returns 0 when it has
 * been called the same number of times than ecore_fb_init().
 */
EAPI int
ecore_fb_shutdown(void)
{
   const char *s;

   if (--_ecore_fb_init_count != 0)
      return _ecore_fb_init_count;

   if (oldhand)
     {
        signal(SIGINT, oldhand);
        oldhand = NULL;
     }

   s = getenv("ECORE_FB_NO_VT");
   if ((!s) || (atoi(s) == 0))
     {
        ecore_fb_vt_shutdown();
     }

   return _ecore_fb_init_count;
}


/**
 * @brief Retrieve the width and height of the current frame buffer in
 * pixels.
 *
 * @param w Pointer to an integer in which to store the width.
 * @param h Pointer to an interge in which to store the height.
 *
 * This function retrieves the size of the current frame buffer in
 * pixels. @p w and @p h can be buffers that will be filled with the
 * corresponding values. If one of them is @c NULL, nothing will be
 * done for that parameter.
 */
EAPI void
ecore_fb_size_get(int *w, int *h)
{
   if (w) *w = _ecore_fb_console_w;
   if (h) *h = _ecore_fb_console_h;
}

static void
_ecore_fb_size_get(const char *name, int *w, int *h)
{
   struct fb_var_screeninfo fb_var;
   int fb;
   const char *s;

   if ((s = getenv("EVAS_FB_DEV")) &&
       (((!strncmp(s, "/dev/fb", 7)) &&
         ((s[7] >= '0' && s[7] <= '9') || (s[7] == 0))) ||
           ((!strncmp(s, "/dev/fb/", 8)) && (s[8] != '.'))))
     {
        fb = open(s, O_RDWR);
        if (fb < 0)
          fprintf(stderr, "[ecore_fb] error opening $EVAS_FB_DEV=%s: %s\n", s, strerror(errno));
     }
   else
     {
        char dev[PATH_MAX];
        int device;

        /* consistent with ecore_evas_default_display in ecore_evas_fb.c */
        if (!name) name = "0";

        /* consistent with ecore_evas_fb.c -> evas_fb_main.c */
        device = strtol(name, NULL, 10);

        snprintf(dev, sizeof(dev), "/dev/fb/%i", device);
        fb = open(dev, O_RDWR);
        if (fb == -1)
          {
             snprintf(dev, sizeof(dev), "/dev/fb%i", device);
             fb = open(dev, O_RDWR);
             if (fb < 0)
               fprintf(stderr, "[ecore_fb] error opening /dev/fb/%i and /dev/fb%i: %s\n", device, device, strerror(errno));
          }
     }
   if (fb < 0)
      goto exit;

   if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_var) == -1)
     {
        perror("[ecore_fb] ioctl FBIOGET_VSCREENINFO");
        goto err_ioctl;
     }

   *w = fb_var.xres;
   *h = fb_var.yres;

err_ioctl:
   close(fb);
exit:
   return;
}

/**
 * @}
 */
