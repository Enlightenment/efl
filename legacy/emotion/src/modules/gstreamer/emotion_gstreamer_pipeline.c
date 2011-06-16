#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>

#include "emotion_private.h"
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"


gboolean
emotion_pipeline_pause(GstElement *pipeline)
{
   GstStateChangeReturn res;

   res = gst_element_set_state((pipeline), GST_STATE_PAUSED);
   if (res == GST_STATE_CHANGE_FAILURE)
     {
        GstBus  *bus;
        gboolean done;

        ERR("could not pause");

        done = FALSE;
        bus = gst_element_get_bus(pipeline);
        if (!bus) return 0;

        while (!done)
          {
             GstMessage *message;

             message = gst_bus_pop(bus);
             if (!message)
               /* All messages read, we're done */
               break;

             switch (GST_MESSAGE_TYPE(message))
               {
               case GST_MESSAGE_ERROR:
                 {
                    GError *err = NULL;
                    gchar *dbg_info = NULL;
    
                    gst_message_parse_error(message, &err, &dbg_info);
                    ERR("[from element \"%s\"] %s",
                        GST_OBJECT_NAME (message->src), err->message);
                    if (dbg_info)
                      ERR("%s\n", dbg_info);
                    g_error_free (err);
                    g_free (dbg_info);
                    done = TRUE;
                    break;
                 }
               default:
                 break;
               }
             gst_message_unref(message);
          }
        gst_object_unref(GST_OBJECT(bus));
        return 0;
     }

   res = gst_element_get_state((pipeline), NULL, NULL, GST_CLOCK_TIME_NONE);
   if (res != GST_STATE_CHANGE_SUCCESS)
     {
        ERR("could not complete pause");
        return 0;
     }

   return 1;
}

Eina_Bool
_emotion_pipeline_build(Emotion_Gstreamer_Video *ev, const char *file)
{
  GstElement *sink;
  int i;

   ev->pipeline = gst_element_factory_make("playbin2", "pipeline");
   if (!ev->pipeline)
     {
       ERR("could not create playbin2 element");
       return EINA_FALSE;
     }

   ev->eos_bus = gst_pipeline_get_bus(GST_PIPELINE(ev->pipeline));
   if (!ev->eos_bus)
     {
       ERR("could not create BUS");
       goto unref_pipeline;
     }

   sink = gst_element_factory_make("fakesink", "videosink");
   if (!sink)
     {
       ERR("could not create video sink");
       goto unref_pipeline;
     }
   g_object_set(G_OBJECT(sink), "sync", TRUE, NULL);
   g_object_set(G_OBJECT(sink), "signal-handoffs", TRUE, NULL);
   g_signal_connect(G_OBJECT(sink),
                    "handoff",
                    G_CALLBACK(cb_handoff), ev);
   g_object_set(G_OBJECT(ev->pipeline), "video-sink", sink, NULL);

   sink = gst_element_factory_make("autoaudiosink", "audiosink");
   if (!sink)
     {
       ERR("could not create audio sink");
       goto unref_pipeline;
     }
   g_object_set(G_OBJECT(ev->pipeline), "audio-sink", sink, NULL);

   if (strstr(file, "://") == NULL)
     {
       Eina_Strbuf *sbuf;

       sbuf = eina_strbuf_new();
       eina_strbuf_append(sbuf, "file://");
       if (*file != '.' && *file != '/')
	 {
	   char *tmp;

	   tmp = get_current_dir_name();
	   eina_strbuf_append(sbuf, tmp);
	   eina_strbuf_append(sbuf, "/");
	   free(tmp);
	 }
       eina_strbuf_append(sbuf, file);

       DBG("Setting file %s\n", eina_strbuf_string_get(sbuf));
       g_object_set(G_OBJECT(ev->pipeline), "uri", eina_strbuf_string_get(sbuf), NULL);

       eina_strbuf_free(sbuf);
     }
   else
     {
       DBG("Setting file %s\n", file);
       g_object_set(G_OBJECT(ev->pipeline), "uri", file, NULL);
     }

   if (!emotion_pipeline_pause(ev->pipeline))
     goto unref_pipeline;

   g_object_get(G_OBJECT(ev->pipeline),
                 "n-audio", &ev->audio_stream_nbr,
                 "n-video", &ev->video_stream_nbr,
                 NULL);

   if ((ev->video_stream_nbr == 0) && (ev->audio_stream_nbr == 0))
     {
       ERR("No audio nor video stream found");
       goto unref_pipeline;
     }

   /* Video streams */

   for (i = 0; i < ev->video_stream_nbr; i++)
     {
       GstPad       *pad;
       GstCaps      *caps;
       GstStructure *structure;
       GstQuery     *query;
       const GValue *val;
       gchar        *str;
       Eina_Bool     build_stream = EINA_FALSE;

       gdouble length_time = 0.0;
       gint width;
       gint height;
       gint fps_num;
       gint fps_den;
       guint32 fourcc = 0;

       g_signal_emit_by_name(ev->pipeline, "get-video-pad", i, &pad);
       if (!pad)
         continue;

       caps = gst_pad_get_negotiated_caps(pad);
       if (!caps)
         goto unref_pad_v;
       structure = gst_caps_get_structure(caps, 0);
       str = gst_caps_to_string(caps);

       if (!gst_structure_get_int(structure, "width", &width))
         goto unref_caps_v;
       if (!gst_structure_get_int(structure, "height", &height))
         goto unref_caps_v;
       if (!gst_structure_get_fraction(structure, "framerate", &fps_num, &fps_den))
         goto unref_caps_v;

       if (g_str_has_prefix(str, "video/x-raw-yuv"))
         {
           val = gst_structure_get_value(structure, "format");
           fourcc = gst_value_get_fourcc(val);
         }
       else if (g_str_has_prefix(str, "video/x-raw-rgb"))
         fourcc = GST_MAKE_FOURCC('A', 'R', 'G', 'B');
       else
         goto unref_caps_v;

       query = gst_query_new_duration(GST_FORMAT_TIME);
       if (gst_pad_peer_query(pad, query))
         {
           gint64 t;

           gst_query_parse_duration(query, NULL, &t);
           length_time = (double)t / (double)GST_SECOND;
         }
       else
         goto unref_query_v;

       build_stream = EINA_TRUE;

     unref_query_v:
       gst_query_unref(query);
     unref_caps_v:
       gst_caps_unref(caps);
     unref_pad_v:
       gst_object_unref(pad);

       if (build_stream)
         {
           Emotion_Video_Stream *vstream;

           vstream = emotion_video_stream_new(ev);
           if (!vstream) continue;

           vstream->length_time = length_time;
           vstream->width = width;
           vstream->height = height;
           vstream->fps_num = fps_num;
           vstream->fps_den = fps_den;
           vstream->fourcc = fourcc;
         }
     }

   /* Audio streams */

   for (i = 0; i < ev->audio_stream_nbr; i++)
     {
       GstPad       *pad;
       GstCaps      *caps;
       GstStructure *structure;
       GstQuery     *query;
       Eina_Bool     build_stream = EINA_FALSE;

       gdouble length_time = 0.0;
       gint channels;
       gint samplerate;

       g_signal_emit_by_name(ev->pipeline, "get-audio-pad", i, &pad);
       if (!pad)
         continue;

       caps = gst_pad_get_negotiated_caps(pad);
       if (!caps)
         goto unref_pad_a;
       structure = gst_caps_get_structure(caps, 0);

       if (!gst_structure_get_int(structure, "channels", &channels))
         goto unref_caps_a;
       if (!gst_structure_get_int(structure, "rate", &samplerate))
         goto unref_caps_a;

       query = gst_query_new_duration(GST_FORMAT_TIME);
       if (gst_pad_peer_query(pad, query))
         {
           gint64 t;

           gst_query_parse_duration(query, NULL, &t);
           length_time = (double)t / (double)GST_SECOND;
         }
       else
         goto unref_query_a;

       build_stream = EINA_TRUE;

     unref_query_a:
       gst_query_unref(query);
     unref_caps_a:
       gst_caps_unref(caps);
     unref_pad_a:
       gst_object_unref(pad);

       if (build_stream)
         {
           Emotion_Audio_Stream *astream;

           astream = (Emotion_Audio_Stream *)calloc(1, sizeof(Emotion_Audio_Stream));
           if (!astream) continue;
           ev->audio_streams = eina_list_append(ev->audio_streams, astream);
           if (eina_error_get())
             {
               free(astream);
               continue;
             }

           astream->length_time = length_time;
           astream->channels = channels;
           astream->samplerate = samplerate;
         }
     }

   /* Visualization sink */

   if (ev->video_stream_nbr == 0)
     {
       GstElement *vis = NULL;
       Emotion_Video_Stream *vstream;
       Emotion_Audio_Stream *astream;
       gint flags;
       const char *vis_name;

       if (!(vis_name = emotion_visualization_element_name_get(ev->vis)))
         {
           printf ("pb vis name %d\n", ev->vis);
           goto finalize;
         }

       astream = (Emotion_Audio_Stream *)eina_list_data_get(ev->audio_streams);

       vis = gst_element_factory_make(vis_name, "vissink");
       vstream = emotion_video_stream_new(ev);
       if (!vstream)
         goto finalize;
       else
         DBG("could not create visualization stream");

       vstream->length_time = astream->length_time;
       vstream->width = 320;
       vstream->height = 200;
       vstream->fps_num = 25;
       vstream->fps_den = 1;
       vstream->fourcc = GST_MAKE_FOURCC('A', 'R', 'G', 'B');

       g_object_set(G_OBJECT(ev->pipeline), "vis-plugin", vis, NULL);
       g_object_get(G_OBJECT(ev->pipeline), "flags", &flags, NULL);
       flags |= 0x00000008;
       g_object_set(G_OBJECT(ev->pipeline), "flags", flags, NULL);
     }

 finalize:

   ev->video_stream_nbr = eina_list_count(ev->video_streams);
   ev->audio_stream_nbr = eina_list_count(ev->audio_streams);

   if (ev->video_stream_nbr == 1)
     {
       Emotion_Video_Stream *vstream;

       vstream = (Emotion_Video_Stream *)eina_list_data_get(ev->video_streams);
       ev->ratio = (double)vstream->width / (double)vstream->height;
     }

   return EINA_TRUE;

 unref_pipeline:
   g_object_get(G_OBJECT(ev->pipeline),
                 "audio-sink", &sink,
                 NULL);
   gst_element_set_state(sink, GST_STATE_NULL);
   g_object_get(G_OBJECT(ev->pipeline),
                 "video-sink", &sink,
                 NULL);
   gst_element_set_state(sink, GST_STATE_NULL);
   gst_element_set_state(ev->pipeline, GST_STATE_NULL);
   gst_object_unref(ev->pipeline);
   ev->pipeline = NULL;

   return EINA_FALSE;
}

/* Send the video frame to the evas object */
void
cb_handoff(GstElement *fakesrc __UNUSED__,
           GstBuffer  *buffer,
           GstPad     *pad,
           gpointer    user_data)
{
   GstQuery *query;
   void *buf[2];

   Emotion_Gstreamer_Video *ev = (Emotion_Gstreamer_Video *)user_data;
   if (!ev)
     return;

   if (!ev->video_mute)
     {
        if (!ev->obj_data)
          ev->obj_data = malloc(GST_BUFFER_SIZE(buffer) * sizeof(void));

        memcpy(ev->obj_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
        buf[0] = GST_BUFFER_DATA(buffer);
        buf[1] = buffer;
        ecore_pipe_write(ev->pipe, buf, sizeof(buf));
     }
   else
     {
        Emotion_Audio_Stream *astream;
        astream = (Emotion_Audio_Stream *)eina_list_nth(ev->audio_streams, ev->audio_stream_nbr - 1);
        _emotion_video_pos_update(ev->obj, ev->position, astream->length_time);
     }

   query = gst_query_new_position(GST_FORMAT_TIME);
   if (gst_pad_query(gst_pad_get_peer(pad), query))
     {
        gint64 position;

        gst_query_parse_position(query, NULL, &position);
        ev->position = (double)position / (double)GST_SECOND;
     }
   gst_query_unref(query);
}

Emotion_Video_Stream *
emotion_video_stream_new(Emotion_Gstreamer_Video *ev)
{
   Emotion_Video_Stream *vstream;

   if (!ev) return NULL;

   vstream = (Emotion_Video_Stream *)calloc(1, sizeof(Emotion_Video_Stream));
   if (!vstream) return NULL;

   ev->video_streams = eina_list_append(ev->video_streams, vstream);
   if (eina_error_get())
     {
        free(vstream);
        return NULL;
     }
   return vstream;
}

void
emotion_video_stream_free(Emotion_Gstreamer_Video *ev, Emotion_Video_Stream *vstream)
{
   if (!ev || !vstream) return;

   ev->video_streams = eina_list_remove(ev->video_streams, vstream);
        free(vstream);
}

int
emotion_pipeline_cdda_track_count_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   GstBus                  *bus;
   guint                    tracks_count = 0;
   gboolean                 done;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return tracks_count;

   done = FALSE;
   bus = gst_element_get_bus(ev->pipeline);
   if (!bus) return tracks_count;

   while (!done)
     {
        GstMessage *message;

        message = gst_bus_pop(bus);
        if (!message)
          /* All messages read, we're done */
          break;

        switch (GST_MESSAGE_TYPE(message))
          {
           case GST_MESSAGE_TAG:
                {
                   GstTagList *tags;

                   gst_message_parse_tag(message, &tags);

                   gst_tag_list_get_uint(tags, GST_TAG_TRACK_COUNT, &tracks_count);
                   if (tracks_count) done = TRUE;
                   break;
                }
           case GST_MESSAGE_ERROR:
           default:
              break;
          }
        gst_message_unref(message);
     }

   gst_object_unref(GST_OBJECT(bus));

   return tracks_count;
}

const char *
emotion_visualization_element_name_get(Emotion_Vis visualisation)
{
   switch (visualisation)
     {
      case EMOTION_VIS_NONE:
         return NULL;
      case EMOTION_VIS_GOOM:
         return "goom";
      case EMOTION_VIS_LIBVISUAL_BUMPSCOPE:
         return "libvisual_bumpscope";
      case EMOTION_VIS_LIBVISUAL_CORONA:
         return "libvisual_corona";
      case EMOTION_VIS_LIBVISUAL_DANCING_PARTICLES:
         return "libvisual_dancingparticles";
      case EMOTION_VIS_LIBVISUAL_GDKPIXBUF:
         return "libvisual_gdkpixbuf";
      case EMOTION_VIS_LIBVISUAL_G_FORCE:
         return "libvisual_G-Force";
      case EMOTION_VIS_LIBVISUAL_GOOM:
         return "libvisual_goom";
      case EMOTION_VIS_LIBVISUAL_INFINITE:
         return "libvisual_infinite";
      case EMOTION_VIS_LIBVISUAL_JAKDAW:
         return "libvisual_jakdaw";
      case EMOTION_VIS_LIBVISUAL_JESS:
         return "libvisual_jess";
      case EMOTION_VIS_LIBVISUAL_LV_ANALYSER:
         return "libvisual_lv_analyzer";
      case EMOTION_VIS_LIBVISUAL_LV_FLOWER:
         return "libvisual_lv_flower";
      case EMOTION_VIS_LIBVISUAL_LV_GLTEST:
         return "libvisual_lv_gltest";
      case EMOTION_VIS_LIBVISUAL_LV_SCOPE:
         return "libvisual_lv_scope";
      case EMOTION_VIS_LIBVISUAL_MADSPIN:
         return "libvisual_madspin";
      case EMOTION_VIS_LIBVISUAL_NEBULUS:
         return "libvisual_nebulus";
      case EMOTION_VIS_LIBVISUAL_OINKSIE:
         return "libvisual_oinksie";
      case EMOTION_VIS_LIBVISUAL_PLASMA:
         return "libvisual_plazma";
      default:
         return "goom";
     }
}
