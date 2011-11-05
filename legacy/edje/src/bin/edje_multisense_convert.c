#include "edje_multisense_convert.h"

#ifdef HAVE_LIBSNDFILE
# define READBUF 1024
# ifdef HAVE_VORBIS
#  include <vorbis/vorbisenc.h>
# endif

# ifdef HAVE_LIBFLAC
#  include <FLAC/metadata.h>
#  include <FLAC/stream_encoder.h>
# endif

Edje_Sound_Encode *
_edje_multisense_encode(const char *filename, Edje_Sound_Sample *sample, double quality)
{
   SF_INFO sfinfo;
   SNDFILE* sfile;
   Edje_Sound_Encode *enc_info;
   
   enc_info = calloc(1, sizeof(Edje_Sound_Encode));
   if (!enc_info)
     {
        ERR("Error. while allocating memory to load file ");
        exit(-1);
     }
   memset (&sfinfo, 0, sizeof (SF_INFO));
   
   enc_info->encoded = EINA_FALSE;
   enc_info->comp_type = "RAW PCM";
   
   // Open wav file using sndfile
   sfile = sf_open (filename, SFM_READ, &sfinfo);
   if (!sfile)
     {
        ERR("Error. Unable to open audio file : %s", filename);
        exit(-1);
     }

   if (!sf_format_check(&sfinfo))
     {
        ERR("Error. Unknown file, not a valid audio file");
        exit(-1);
     }

   if (sample->compression == EDJE_SOUND_SOURCE_TYPE_INLINE_COMP)
     {
        sf_close(sfile);
#ifdef HAVE_LIBFLAC
        //encode provided wav file to flac
        enc_info->file = _edje_multisense_encode_to_flac((char *)filename, sfinfo);
        if (enc_info->file)
          {
             enc_info->comp_type = "FLAC";
             enc_info->encoded = EINA_TRUE;
          }
#else
        WRN("WARNING: Unable to encode sound %s to FLAC compression",
            sample->name);
#endif
     }
   else if (sample->compression == EDJE_SOUND_SOURCE_TYPE_INLINE_LOSSY)
     {
        sf_close(sfile);
#ifdef HAVE_VORBIS
        //encode provided wav file to ogg-vorbis
        enc_info->file = _edje_multisense_encode_to_ogg_vorbis((char *)filename,
                                                               quality, sfinfo);
        if (enc_info->file)
          {
             enc_info->comp_type = "OGG-VORBIS";
             enc_info->encoded = EINA_TRUE;
          }
#else
        WRN("WARNING: Unable to encode sound %s to Ogg-Vorbis",
            sample->name);
#endif
     }
   else
     eina_stringshare_replace(&enc_info->file, filename);
   return enc_info;
}

#ifdef HAVE_LIBFLAC
const char*
_edje_multisense_encode_to_flac(char *snd_path, SF_INFO sfinfo)
{
   unsigned int total_samples = 0; /* can use a 32-bit number due to WAVE size limitations */
   FLAC__bool ok = 1;
   FLAC__StreamEncoder *encoder = 0;
   FLAC__StreamEncoderInitStatus init_status;
   FLAC__StreamMetadata *metadata[2];
   FLAC__StreamMetadata_VorbisComment_Entry entry;
   SNDFILE *sfile;
   sf_count_t size;
   char *tmp;

   sfile = sf_open(snd_path, SFM_READ, &sfinfo);
   if (!sfile) return NULL;
   if (!sf_format_check(&sfinfo))
     {
        sf_close(sfile);
        return NULL;
     }
   size = sf_seek(sfile, 0, SEEK_END);
   sf_seek(sfile, 0, SEEK_SET);
   tmp = malloc(strlen(snd_path) + 1 + 5);
   if (!tmp)
     {
        sf_close(sfile);
        return NULL;
     }
   strcpy(tmp, snd_path);
   snd_path = tmp;
   strcat(snd_path, ".flac");

   total_samples = size;

   /* allocate the encoder */
   if ((encoder = FLAC__stream_encoder_new()) == NULL)
     {
        ERR("ERROR: Creating FLAC encoder\n");
        free(snd_path);
        sf_close(sfile);
        return NULL;
     }

   /* Verify it's own encoded output. This will slow the encoding process. */
   ok &= FLAC__stream_encoder_set_verify(encoder, 1);

   //Levels range from 0 (fastest, least compression) to 8 (slowest, most compression).
   //A value larger than 8 will be treated as 8.
   //5 is used for good compression and moderate compression/decompression speed.
   ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
   ok &= FLAC__stream_encoder_set_channels(encoder, sfinfo.channels);
   ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, 16);
   ok &= FLAC__stream_encoder_set_sample_rate(encoder, sfinfo.samplerate);
   ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

   /* now add some metadata; we'll add some tags and a padding block */
   if (ok)
     {
        if ((metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL
            || (metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL
            || !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "Encoder", "flac")
            || !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, 0))
          {
             ERR("ERROR: out of memory error or tag error\n");
             ok = 0;
          }
        metadata[1]->length = 16; /* set the padding length */
        ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
     }

   /* initialize encoder */
   if (ok)
     {
        init_status = FLAC__stream_encoder_init_file(encoder, snd_path, NULL,
                                                     (void *)(long)(total_samples));
        if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
          {
             ERR("ERROR: unable to initialize FLAC encoder: %s\n",
                 FLAC__StreamEncoderInitStatusString[init_status]);
             ok = 0;
          }
     }
   
   /* read blocks of samples from WAVE file and feed to encoder */
   while (ok)
     {
        FLAC__int32 readbuffer[READBUF * 2];
        sf_count_t count;
        int i;
        
        count = sf_readf_int(sfile, readbuffer, READBUF);
        if (count <= 0) break;
        for (i = 0; i < (count * sfinfo.channels); i++)
          readbuffer[i] = readbuffer[i] >> 16;
        ok = FLAC__stream_encoder_process_interleaved(encoder, readbuffer,
                                                      count);
     }

   FLAC__stream_encoder_finish(encoder);
   /* now that encoding is finished, the metadata can be freed */
   FLAC__metadata_object_delete(metadata[0]);
   FLAC__metadata_object_delete(metadata[1]);

   FLAC__stream_encoder_delete(encoder);
   sf_close(sfile);
   return (snd_path);
}
#endif

#ifdef HAVE_VORBIS
const char *
_edje_multisense_encode_to_ogg_vorbis(char *snd_path, double quality, SF_INFO sfinfo)
{
   ogg_stream_state os; /* take physical pages, weld into a logical stream of packets */
   ogg_page og; /* one Ogg bitstream page.  Vorbis packets are inside */
   ogg_packet op; /* one raw packet of data for decode */
   vorbis_info vi; /* struct that stores all the static vorbis bitstream settings */
   vorbis_comment vc; /* struct that stores all the user comments */
   vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
   vorbis_block vb; /* local working space for packet->PCM decode */
   int eos = 0, ret;
   char *tmp;
   SNDFILE *sfile;
   FILE *fout;

   sfile = sf_open(snd_path, SFM_READ, &sfinfo);
   if (!sfile) return NULL;
   if (!sf_format_check(&sfinfo))
     {
        sf_close(sfile);
        return NULL;
     }
   tmp = malloc(strlen(snd_path) + 1 + 4);
   if (!tmp)
     {
        sf_close(sfile);
        return NULL;
     }
   strcpy(tmp, snd_path);
   snd_path = tmp;
   strcat(snd_path, ".ogg");
   fout = fopen(snd_path, "wb");
   if (!fout)
     {
        free(snd_path);
        sf_close(sfile);
        return NULL;
     }

   /********** Encode setup ************/
   vorbis_info_init(&vi);
   ret = vorbis_encode_init(&vi, sfinfo.channels, sfinfo.samplerate, 
                            -1, (long)(quality * 1000), -1);
   if (ret == OV_EFAULT) printf("OV_EFAULT\n");
   if (ret == OV_EINVAL) printf("OV_EINVAL\n");
   if (ret == OV_EIMPL) printf("OV_EIMPL\n");

   if (ret)
     {
        fclose(fout);
        free(snd_path);
        sf_close(sfile);
        return NULL;
     }

   /* add a comment */
   vorbis_comment_init(&vc);
   vorbis_comment_add_tag(&vc, "", "");

   /* set up the analysis state and auxiliary encoding storage */
   vorbis_analysis_init(&vd, &vi);
   vorbis_block_init(&vd, &vb);

   srand(time(NULL));
   ogg_stream_init(&os, rand());

   ogg_packet header;
   ogg_packet header_comm;
   ogg_packet header_code;

   vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
   ogg_stream_packetin(&os, &header); /* automatically placed in its own page */
   ogg_stream_packetin(&os, &header_comm);
   ogg_stream_packetin(&os, &header_code);

   while (!eos)
     {
        int result = ogg_stream_flush(&os, &og);
        if (!result) break;
        fwrite(og.header, 1, og.header_len, fout);
        fwrite(og.body, 1, og.body_len, fout);
     }

   while (!eos)
     {
        int i, ch;
        float readbuffer[READBUF * 2];
        sf_count_t count;
        
        count = sf_readf_float(sfile, readbuffer, READBUF);

        if (!count)
          vorbis_analysis_wrote(&vd, 0);
        else
          {
             float **buffer = vorbis_analysis_buffer(&vd, count);
             
             /* uninterleave samples */
             for (i = 0; i < count; i++)
               {
                  for (ch = 0; ch < sfinfo.channels; ch++)
                    buffer[ch][i]= readbuffer[(i * sfinfo.channels) + ch];
               }
             vorbis_analysis_wrote(&vd, i);
          }
        while (vorbis_analysis_blockout(&vd, &vb) == 1)
          {
             vorbis_analysis(&vb, NULL);
             vorbis_bitrate_addblock(&vb);

             while (vorbis_bitrate_flushpacket(&vd, &op))
               {
                  ogg_stream_packetin(&os, &op);
                  while (!eos)
                    {
                       int result = ogg_stream_pageout(&os, &og);
                       if (!result) break;
                       fwrite(og.header, 1, og.header_len, fout);
                       fwrite(og.body, 1, og.body_len, fout);
                       if (ogg_page_eos(&og)) eos = 1;
                    }
               }
          }
     }
   ogg_stream_clear(&os);
   vorbis_block_clear(&vb);
   vorbis_dsp_clear(&vd);
   vorbis_comment_clear(&vc);
   vorbis_info_clear(&vi);
   sf_close(sfile);
   fclose (fout);
   return snd_path;
}
#endif
#endif
