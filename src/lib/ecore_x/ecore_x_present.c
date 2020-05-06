#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>

#include "ecore_x_private.h"
#include "Ecore_X.h"

int _ecore_x_present_major = 0;
static Eina_Bool _ecore_x_present_exists = EINA_FALSE;

void
_ecore_x_present_init(void)
{
   ECORE_X_EVENT_PRESENT_CONFIGURE = ecore_event_type_new();
   ECORE_X_EVENT_PRESENT_COMPLETE = ecore_event_type_new();
   ECORE_X_EVENT_PRESENT_IDLE = ecore_event_type_new();
#ifdef ECORE_XPRESENT
   LOGFN;
   _ecore_x_present_exists = XPresentQueryExtension(_ecore_x_disp, &_ecore_x_present_major, NULL, NULL);
#endif
}

#ifdef ECORE_XPRESENT
#define SET(X) e->X = ev->X

static void
_present_configure(XPresentConfigureNotifyEvent *ev)
{
   Ecore_X_Event_Present_Configure *e;

   e = calloc(1, sizeof(Ecore_X_Event_Present_Configure));
   if (!e) return;

   e->win = ev->window;
   SET(x), SET(y);
   SET(width), SET(height);
   SET(off_x), SET(off_y);
   SET(pixmap_width), SET(pixmap_height);
   SET(pixmap_flags);

   ecore_event_add(ECORE_X_EVENT_PRESENT_CONFIGURE, e, NULL, NULL);
}

static void
_present_complete(XPresentCompleteNotifyEvent *ev)
{
   unsigned int mode[] =
   {
    [PresentCompleteModeCopy] = ECORE_X_PRESENT_COMPLETE_MODE_COPY,
    [PresentCompleteModeFlip] = ECORE_X_PRESENT_COMPLETE_MODE_FLIP,
    [PresentCompleteModeSkip] = ECORE_X_PRESENT_COMPLETE_MODE_SKIP,
   };
   Ecore_X_Event_Present_Complete *e;

   e = calloc(1, sizeof(Ecore_X_Event_Present_Complete));
   if (!e) return;

   e->win = ev->window;
   e->serial = ev->serial_number;
   SET(ust), SET(msc);
   e->kind = (ev->kind == 1); //libXpresent doesn't expose this...
   e->mode = mode[ev->mode];
   ecore_event_add(ECORE_X_EVENT_PRESENT_COMPLETE, e, NULL, NULL);
}

static void
_present_idle(XPresentIdleNotifyEvent *ev)
{
   Ecore_X_Event_Present_Idle *e;

   e = calloc(1, sizeof(Ecore_X_Event_Present_Idle));
   if (!e) return;

   e->win = ev->window;
   e->serial = ev->serial_number;
   SET(pixmap);
   SET(idle_fence);
   ecore_event_add(ECORE_X_EVENT_PRESENT_IDLE, e, NULL, NULL);
}
#undef SET

void
_ecore_x_present_handler(XGenericEvent *ge)
{
   XGenericEventCookie *gec = (XGenericEventCookie*)ge;

   if (XGetEventData(_ecore_x_disp, gec))
     {
        switch (gec->evtype)
          {
           case PresentConfigureNotify:
             _present_configure(gec->data);
             break;
           case PresentCompleteNotify:
             _present_complete(gec->data);
             break;
           case PresentIdleNotify:
             _present_idle(gec->data);
             break;
           default: break;
          }
     }
   XFreeEventData(_ecore_x_disp, gec);
}
#endif

EAPI void
ecore_x_present_select_events(Ecore_X_Window win, unsigned int events)
{
#ifdef ECORE_XPRESENT
   XPresentSelectInput(_ecore_x_disp, win, events);
   if (_ecore_xlib_sync) ecore_x_sync();
#else
   (void)win;
   (void)events;
#endif
}

EAPI void
ecore_x_present_notify_msc(Ecore_X_Window win, unsigned int serial, unsigned long long target_msc, unsigned long long divisor, unsigned long long remainder)
{
#ifdef ECORE_XPRESENT
   XPresentNotifyMSC(_ecore_x_disp, win, serial, target_msc, divisor, remainder);
   if (_ecore_xlib_sync) ecore_x_sync();
#else
   (void)win;
   (void)serial;
   (void)target_msc;
   (void)divisor;
   (void)remainder;
#endif
}

EAPI void
ecore_x_present_pixmap(Ecore_X_Window win, Ecore_X_Pixmap pixmap, unsigned int serial, Ecore_X_Region valid,
                       Ecore_X_Region update, int x_off, int y_off, Ecore_X_Randr_Crtc target_crtc,
                       Ecore_X_Sync_Fence wait_fence, Ecore_X_Sync_Fence idle_fence, unsigned int options,
                       unsigned long long target_msc, unsigned long long divisor, unsigned long long remainder,
                       Ecore_X_Present *notifies, int num_notifies)
{
#ifdef ECORE_XPRESENT
   XPresentPixmap(_ecore_x_disp, win, pixmap, serial, valid, update,
                  x_off, y_off, target_crtc, wait_fence, idle_fence, options, target_msc,
                  divisor, remainder, (XPresentNotify*)notifies, num_notifies);
   if (_ecore_xlib_sync) ecore_x_sync();
#else
   (void)win;
   (void)pixmap;
   (void)serial;
   (void)valid;
   (void)update,
   (void)x_off;
   (void)y_off;
   (void)target_crtc;
   (void)wait_fence;
   (void)idle_fence;
   (void)options;
   (void)target_msc,
   (void)divisor;
   (void)remainder;
   (void)notifies;
   (void)num_notifies;
#endif
}

EAPI Eina_Bool
ecore_x_present_exists(void)
{
   return _ecore_x_present_exists;
}
