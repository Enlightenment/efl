#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Evas.h>

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideosink.h>

#ifdef HAVE_ECORE_X
# include <Ecore_X.h>
# ifdef HAVE_XOVERLAY_H
#  include <gst/interfaces/xoverlay.h>
# endif
#endif

#include "Emotion.h"
#include "emotion_gstreamer.h"

typedef struct _FakeEOSBin
{
   GstBin parent;
} FakeEOSBin;

typedef struct _FakeEOSBinClass
{
   GstBinClass parent;
} FakeEOSBinClass;

GST_BOILERPLATE(FakeEOSBin, fakeeos_bin, GstBin,
                GST_TYPE_BIN);

static void
fakeeos_bin_handle_message(GstBin * bin, GstMessage * message)
{
   /* FakeEOSBin *fakeeos = (FakeEOSBin *)(bin); */

   switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_EOS:
       /* what to do here ? just returning at the moment */
       return;
    default:
       break;
   }

   GST_BIN_CLASS(parent_class)->handle_message(bin, message);
}

static void
fakeeos_bin_base_init(gpointer g_class EINA_UNUSED)
{
}

static void
fakeeos_bin_class_init(FakeEOSBinClass * klass)
{
   GstBinClass *gstbin_class = GST_BIN_CLASS(klass);

   gstbin_class->handle_message =
     GST_DEBUG_FUNCPTR (fakeeos_bin_handle_message);
}

static void
fakeeos_bin_init(FakeEOSBin *src EINA_UNUSED,
                 FakeEOSBinClass *klass EINA_UNUSED)
{
}
