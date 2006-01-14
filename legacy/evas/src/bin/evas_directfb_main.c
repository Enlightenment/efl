#include "evas_test_main.h"

#include <directfb/directfb.h>

#include "Evas.h"
#include "Evas_Engine_DirectFB.h"

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

int
main(int argc, char *argv[])
{
   IDirectFB          *dfb = NULL;
   IDirectFBSurface   *primary;
   IDirectFBDisplayLayer *layer;
   IDirectFBInputDevice *keyboard;
   IDirectFBInputDevice *mouse;
   IDirectFBEventBuffer *buffer;
   DFBResult           err;
   DFBSurfaceDescription dsc;
   DFBDisplayLayerConfig layer_config;
   DFBInputEvent       evt;

   int                 quit = 0;
   int                 paused = 0;

   DFBCHECK(DirectFBInit(&argc, &argv));

   /* create the super interface */
   DFBCHECK(DirectFBCreate(&dfb));

   dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN);

   DFBCHECK(dfb->GetDisplayLayer(dfb, DLID_PRIMARY, &layer));
   layer->GetConfiguration(layer, &layer_config);

   /* get the primary surface, i.e. the surface of the primary layer we have
    * exclusive access to */
   memset(&dsc, 0, sizeof(DFBSurfaceDescription));
   dsc.flags = DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT;
   dsc.width = layer_config.width;
   dsc.height = layer_config.height;

   dsc.caps = DSCAPS_PRIMARY;

   DFBCHECK(dfb->CreateSurface(dfb, &dsc, &primary));

   DFBCHECK(dfb->GetInputDevice(dfb, DIDID_KEYBOARD, &keyboard));
   DFBCHECK(dfb->GetInputDevice(dfb, DIDID_MOUSE, &mouse));
   DFBCHECK (dfb->CreateInputEventBuffer (dfb, DICAPS_ALL, DFB_TRUE, &buffer));

   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("directfb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_DirectFB *einfo;

      einfo = (Evas_Engine_Info_DirectFB *) evas_engine_info_get(evas);

      einfo->info.dfb = dfb;
      einfo->info.surface = primary;
      einfo->info.flags = DSDRAW_BLEND;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   start_time = get_time();

   while (!quit)
     {

	while (buffer->GetEvent(buffer, DFB_EVENT(&evt)) == DFB_OK)
	  {

	     switch (evt.type)
	       {

	       case DIET_BUTTONPRESS:
		  switch (evt.button)
		    {
		    case DIBI_LEFT:
		       if (paused == 0)
			  paused = 1;
		       else if (paused == 1)
			  paused = 0;
		       break;
		    default:
		       break;
		    }
		  break;
	       case DIET_KEYPRESS:
		  switch (evt.key_symbol)
		    {
		    case DIKS_ESCAPE:
		    case DIKS_SMALL_Q:
		    case DIKS_CAPITAL_Q:
		    case DIKS_BACK:
		    case DIKS_STOP:
		       quit = 1;
		       break;
		    default:

		       break;
		    }

	       default:
		  break;
	       }
	  }

	if (!paused)
	  {
	     loop();
	     evas_render(evas);
	  }
     }
   buffer->Release(buffer);
   layer->Release(layer);
   primary->Release(primary);
   dfb->Release(dfb);

   evas_shutdown();
   return 0;
}
