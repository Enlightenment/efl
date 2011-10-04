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
   FakeEOSBin *fakeeos = (FakeEOSBin *)(bin);

   switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_EOS:
       /* what to do here ? just returning at the moment */
       return ;
    default:
       break;
   }

   GST_BIN_CLASS(parent_class)->handle_message(bin, message);
}

static void
fakeeos_bin_base_init(gpointer g_class __UNUSED__)
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
fakeeos_bin_init(FakeEOSBin *src __UNUSED__,
                 FakeEOSBinClass *klass __UNUSED__)
{
}
