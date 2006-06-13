#include "Eet.h"
#include "Eet_private.h"

/*---*/

typedef struct _JPEG_error_mgr             *emptr;

/*---*/

struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};

/*---*/

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);

static int   eet_data_image_jpeg_header_decode(const void *data, int size, unsigned int *w, unsigned int *h);
static void *eet_data_image_jpeg_rgb_decode(const void *data, int size, unsigned int *w, unsigned int *h);
static void *eet_data_image_jpeg_alpha_decode(const void *data, int size, unsigned int *d, unsigned int *w, unsigned int *h);
static void *eet_data_image_lossless_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha);
static void *eet_data_image_lossless_compressed_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int compression);
static void *eet_data_image_jpeg_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality);
static void *eet_data_image_jpeg_alpha_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality);

/*---*/

static int words_bigendian = -1;

/*---*/

#define SWAP64(x) (x) = \
   ((((unsigned long long)(x) & 0x00000000000000ffULL ) << 56) |\
       (((unsigned long long)(x) & 0x000000000000ff00ULL ) << 40) |\
       (((unsigned long long)(x) & 0x0000000000ff0000ULL ) << 24) |\
       (((unsigned long long)(x) & 0x00000000ff000000ULL ) << 8) |\
       (((unsigned long long)(x) & 0x000000ff00000000ULL ) >> 8) |\
       (((unsigned long long)(x) & 0x0000ff0000000000ULL ) >> 24) |\
       (((unsigned long long)(x) & 0x00ff000000000000ULL ) >> 40) |\
       (((unsigned long long)(x) & 0xff00000000000000ULL ) >> 56))
#define SWAP32(x) (x) = \
   ((((int)(x) & 0x000000ff ) << 24) |\
       (((int)(x) & 0x0000ff00 ) << 8) |\
       (((int)(x) & 0x00ff0000 ) >> 8) |\
       (((int)(x) & 0xff000000 ) >> 24))
#define SWAP16(x) (x) = \
   ((((short)(x) & 0x00ff ) << 8) |\
       (((short)(x) & 0xff00 ) >> 8))

#define CONV8(x)
#define CONV16(x) {if (words_bigendian) SWAP16(x);}
#define CONV32(x) {if (words_bigendian) SWAP32(x);}
#define CONV64(x) {if (words_bigendian) SWAP64(x);}

/*---*/

static void
_JPEGFatalErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;

   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   longjmp(errmgr->setjmp_buffer, 1);
   return;
}

static void
_JPEGErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;

   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
}

static void
_JPEGErrorHandler2(j_common_ptr cinfo, int msg_level)
{
   emptr errmgr;

   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
   msg_level = 0;
}

static int
eet_data_image_jpeg_header_decode(const void *data, int size, unsigned int *w, unsigned int *h)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   FILE *f;

   f = _eet_memfile_read_open(data, (size_t)size);
   if (!f) return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   *w = cinfo.output_width;
   *h = cinfo.output_height;
   /* end head decoding */
   jpeg_destroy_decompress(&cinfo);
   _eet_memfile_read_close(f);
   return 1;
}

static void *
eet_data_image_jpeg_rgb_decode(const void *data, int size, unsigned int *w, unsigned int *h)
{
   unsigned int *d;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned char *ptr, *line[16], *tdata = NULL;
   unsigned int *ptr2;
   unsigned int x, y, l, scans;
   int i, count, prevy;
   FILE *f;

   f = _eet_memfile_read_open(data, (size_t)size);
   if (!f) return NULL;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	if (tdata) free(tdata);
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.dct_method = JDCT_FASTEST;
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   *w = cinfo.output_width;
   *h = cinfo.output_height;
   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   tdata = alloca((*w) * 16 * 3);
   d = malloc((*w) * (*h) * 4);
   if (!d)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   ptr2 = d;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 3)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = tdata + (i * (*w) * 3);
	for (l = 0; l < (*h); l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	                  scans = cinfo.rec_outbuf_height;
	     if (((*h) - l) < scans) scans = (*h) - l;
	     ptr = tdata;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < (*w); x++)
		    {
		       *ptr2 =
			 (0xff000000) | ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
		       ptr += 3;
		       ptr2++;
		    }
	       }
	  }
     }
   else if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = tdata + (i * (*w));
	for (l = 0; l < (*h); l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if (((*h) - l) < scans) scans = (*h) - l;
	     ptr = tdata;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < (*w); x++)
		    {
		       *ptr2 =
			 (0xff000000) | ((ptr[0]) << 16) | ((ptr[0]) << 8) | (ptr[0]);
		       ptr++;
		       ptr2++;
		    }
	       }
	  }
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   _eet_memfile_read_close(f);
   return d;
}

static void *
eet_data_image_jpeg_alpha_decode(const void *data, int size, unsigned int *d, unsigned int *w, unsigned int *h)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned char *ptr, *line[16], *tdata = NULL;
   unsigned int *ptr2;
   unsigned int x, y, l, scans;
   int i, count, prevy;
   FILE *f;

   f = _eet_memfile_read_open(data, (size_t)size);
   if (!f) return NULL;

   if (0)
     {
	char buf[1];

	while (fread(buf, 1, 1, f));
	_eet_memfile_read_close(f);
	return d;
     }
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	if (tdata) free(tdata);
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.dct_method = JDCT_FASTEST;
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   if ((*w) != cinfo.output_width)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   if ((*h) != cinfo.output_height)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   *w = cinfo.output_width;
   *h = cinfo.output_height;
   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   tdata = alloca((*w) * 16 * 3);
   ptr2 = d;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = tdata + (i * (*w));
	for (l = 0; l < (*h); l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if (((*h) - l) < scans) scans = (*h) - l;
	     ptr = tdata;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < (*w); x++)
		    {
		       *ptr2 =
			 ((*ptr2) & 0x00ffffff) |
			 ((ptr[0]) << 24);
		       ptr++;
		       ptr2++;
		    }
	       }
	  }
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   _eet_memfile_read_close(f);
   return d;
}

static void *
eet_data_image_lossless_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha)
{
   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }
     {
	unsigned char *d;
	int           *header;

	d = malloc((w * h * 4) + (8 * 4));
	if (!d) return NULL;

	header = (int *)d;
	memset(d, 0, 32);

	header[0] = 0xac1dfeed;
	header[1] = w;
	header[2] = h;
	header[3] = alpha;

	memcpy(d + 32, data, w * h * 4);

	if (words_bigendian)
	  {
	     unsigned int i;

	     for (i = 0; i < ((w * h) + 8); i++) SWAP32(header[i]);
	  }
	*size = ((w * h * 4) + (8 * 4));
	return d;
     }
}

static void *
eet_data_image_lossless_compressed_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int compression)
{
   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

     {
	unsigned char *d;
	unsigned char *comp;
	int           *header;
	int            ret;
	uLongf         buflen;

	d = malloc((w * h * 4) + (8 * 4));
	if (!d) return NULL;
	buflen = (((w * h * 101) / 100) + 3) * 4;
	comp = malloc(buflen);
	if (!comp)
	  {
	     free(d);
	     return NULL;
	  }
	header = (int *)d;
	memset(d, 0, 32);

	header[0] = 0xac1dfeed;
	header[1] = w;
	header[2] = h;
	header[3] = alpha;
	header[4] = compression;
	memcpy(d + 32, data, w * h * 4);

	if (words_bigendian)
	  {
	     unsigned int i;

	     for (i = 0; i < ((w * h) + 8); i++) SWAP32(header[i]);
	  }
	ret = compress2((Bytef *)comp, &buflen,
			(Bytef *)(d + 32),
			(uLong)(w * h * 4),
			compression);
	if (buflen > (w * h * 4))
	  {
	     free(comp);
	     *size = ((w * h * 4) + (8 * 4));
	     return d;
	  }
	memcpy(d + 32, comp, buflen);
	*size = (8 * 4) + buflen;
	free(comp);
	return d;
     }
}

static void *
eet_data_image_jpeg_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality)
{
   const int *ptr;
   void *d = NULL;
   size_t sz = 0;
   struct _JPEG_error_mgr jerr;
   JSAMPROW *jbuf;
   struct jpeg_compress_struct cinfo;
   FILE *f;
   unsigned char *buf;

   (void) alpha; /* unused */

   f =_eet_memfile_write_open(&d, &sz);
   if (!f) return NULL;

   buf = alloca(3 * w);

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_compress(&cinfo);
	_eet_memfile_write_close(f);
	if (d) free(d);
	return NULL;
     }
   jpeg_create_compress(&cinfo);
   jpeg_stdio_dest(&cinfo, f);
   cinfo.image_width = w;
   cinfo.image_height = h;
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, quality, TRUE);
   if (quality >= 90)
     {
	cinfo.comp_info[0].h_samp_factor = 1;
	cinfo.comp_info[0].v_samp_factor = 1;
	cinfo.comp_info[1].h_samp_factor = 1;
	cinfo.comp_info[1].v_samp_factor = 1;
	cinfo.comp_info[2].h_samp_factor = 1;
	cinfo.comp_info[2].v_samp_factor = 1;
     }
   jpeg_start_compress(&cinfo, TRUE);

   ptr = data;
   while (cinfo.next_scanline < cinfo.image_height)
     {
	unsigned int i, j;

	/* convert scaline from ARGB to RGB packed */
	for (j = 0, i = 0; i < w; i++)
	  {
	     buf[j++] = ((*ptr) >> 16) & 0xff;
	     buf[j++] = ((*ptr) >> 8) & 0xff;
	     buf[j++] = ((*ptr)) & 0xff;
	     ptr++;
	  }
	jbuf = (JSAMPROW *) (&buf);
	jpeg_write_scanlines(&cinfo, jbuf, 1);
     }

   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);

   _eet_memfile_write_close(f);
   *size = sz;
   return d;
}

static void *
eet_data_image_jpeg_alpha_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality)
{
   unsigned char *d1, *d2;
   unsigned char *d;
   int *header;
   int sz1, sz2;

   (void) alpha; /* unused */

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

     {
	const int *ptr;
	void *d = NULL;
	size_t sz = 0;
	struct _JPEG_error_mgr jerr;
	JSAMPROW *jbuf;
	struct jpeg_compress_struct cinfo;
	FILE *f;
	unsigned char *buf;

	f = _eet_memfile_write_open(&d, &sz);
	if (!f) return NULL;

	buf = alloca(3 * w);

	cinfo.err = jpeg_std_error(&(jerr.pub));
	jerr.pub.error_exit = _JPEGFatalErrorHandler;
	jerr.pub.emit_message = _JPEGErrorHandler2;
	jerr.pub.output_message = _JPEGErrorHandler;
	if (setjmp(jerr.setjmp_buffer))
	  {
	     jpeg_destroy_compress(&cinfo);
	     _eet_memfile_write_close(f);
	     if (d) free(d);
	     return NULL;
	  }
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, f);
	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	if (quality >= 90)
	  {
	     cinfo.comp_info[0].h_samp_factor = 1;
	     cinfo.comp_info[0].v_samp_factor = 1;
	     cinfo.comp_info[1].h_samp_factor = 1;
	     cinfo.comp_info[1].v_samp_factor = 1;
	     cinfo.comp_info[2].h_samp_factor = 1;
	     cinfo.comp_info[2].v_samp_factor = 1;
	  }
	jpeg_start_compress(&cinfo, TRUE);

	ptr = data;
	while (cinfo.next_scanline < cinfo.image_height)
	  {
	     unsigned int i, j;

	     /* convert scaline from ARGB to RGB packed */
	     for (j = 0, i = 0; i < w; i++)
	       {
		  buf[j++] = ((*ptr) >> 16) & 0xff;
		  buf[j++] = ((*ptr) >> 8) & 0xff;
		  buf[j++] = ((*ptr)) & 0xff;
		  ptr++;
	       }
	     jbuf = (JSAMPROW *) (&buf);
	     jpeg_write_scanlines(&cinfo, jbuf, 1);
	  }

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	_eet_memfile_write_close(f);
	d1 = d;
	sz1 = sz;
     }
     {
	const int *ptr;
	void *d = NULL;
	size_t sz = 0;
	struct _JPEG_error_mgr jerr;
	JSAMPROW *jbuf;
	struct jpeg_compress_struct cinfo;
	FILE *f;
	unsigned char *buf;

	f = _eet_memfile_write_open(&d, &sz);
	if (!f)
	  {
	     free(d1);
	     return NULL;
	  }

	buf = alloca(3 * w);

	cinfo.err = jpeg_std_error(&(jerr.pub));
	jerr.pub.error_exit = _JPEGFatalErrorHandler;
	jerr.pub.emit_message = _JPEGErrorHandler2;
	jerr.pub.output_message = _JPEGErrorHandler;
	if (setjmp(jerr.setjmp_buffer))
	  {
	     jpeg_destroy_compress(&cinfo);
	     _eet_memfile_write_close(f);
	     if (d) free(d);
	     free(d1);
	     return NULL;
	  }
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, f);
	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = 1;
	cinfo.in_color_space = JCS_GRAYSCALE;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	if (quality >= 90)
	  {
	     cinfo.comp_info[0].h_samp_factor = 1;
	     cinfo.comp_info[0].v_samp_factor = 1;
	     cinfo.comp_info[1].h_samp_factor = 1;
	     cinfo.comp_info[1].v_samp_factor = 1;
	     cinfo.comp_info[2].h_samp_factor = 1;
	     cinfo.comp_info[2].v_samp_factor = 1;
	  }
	jpeg_start_compress(&cinfo, TRUE);

	ptr = data;
	while (cinfo.next_scanline < cinfo.image_height)
	  {
	     unsigned int i, j;

	     /* convert scaline from ARGB to RGB packed */
	     for (j = 0, i = 0; i < w; i++)
	       {
		  buf[j++] = ((*ptr) >> 24) & 0xff;
		  ptr++;
	       }
	     jbuf = (JSAMPROW *) (&buf);
	     jpeg_write_scanlines(&cinfo, jbuf, 1);
	  }

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	_eet_memfile_write_close(f);
	d2 = d;
	sz2 = sz;
     }
   d = malloc(12 + sz1 + sz2);
   if (!d)
     {
	free(d1);
	free(d2);
	return NULL;
     }
   header = (int *)d;
   header[0] = 0xbeeff00d;
   header[1] = sz1;
   header[2] = sz2;
   if (words_bigendian)
     {
	int i;

	for (i = 0; i < 3; i++) SWAP32(header[i]);
     }
   memcpy(d + 12, d1, sz1);
   memcpy(d + 12 + sz1, d2, sz2);

   free(d1);
   free(d2);
   *size = 12 + sz1 + sz2;
   return d;
}

EAPI int
eet_data_image_write(Eet_File *ef, const char *name,
		     const void *data, unsigned int w, unsigned int h, int alpha,
		     int compress, int quality, int lossy)
{
   void *d = NULL;
   int size = 0;

   d = eet_data_image_encode(data, &size, w, h, alpha, compress, quality, lossy);
   if (d)
     {
	int v;

	v = eet_write(ef, name, d, size, 0);
	free(d);
	return v;
     }
   return 0;
}

EAPI void *
eet_data_image_read(Eet_File *ef, const char *name,
		    unsigned int *w, unsigned int *h, int *alpha,
		    int *compress, int *quality, int *lossy)
{
   void		*data;
   int		size;
   unsigned int *d = NULL;
   int		free_data = 0;

   data = eet_read_direct (ef, name, &size);
   if (!data)
     {
	data = eet_read(ef, name, &size);
	free_data = 1;
     }

   if (!data) return NULL;
   d = eet_data_image_decode(data, size, w, h, alpha, compress, quality, lossy);

   if (free_data)
     free(data);

   return d;
}

EAPI int
eet_data_image_header_read(Eet_File *ef, const char *name,
			   unsigned int *w, unsigned int *h, int *alpha,
			   int *compress, int *quality, int *lossy)
{
   void	*data = NULL;
   int	size = 0;
   int	d;
   int	free_data = 0;

   data = eet_read_direct (ef, name, &size);
   if (!data)
     {
	data = eet_read(ef, name, &size);
	free_data = 1;
     }

   if (!data) return 0;
   d = eet_data_image_header_decode(data, size, w, h, alpha, compress, quality, lossy);
   if (free_data)
     free(data);

   return d;
}

EAPI void *
eet_data_image_encode(const void *data, int *size_ret, unsigned int w, unsigned int h, int alpha, int compress, int quality, int lossy)
{
   void *d = NULL;
   int size = 0;

   if (lossy == 0)
     {
	if (compress <= 0)
	  d = eet_data_image_lossless_convert(&size, data, w, h, alpha);
	else
	  d = eet_data_image_lossless_compressed_convert(&size, data, w, h, alpha, compress);
     }
   else
     {
	if (!alpha)
	  d = eet_data_image_jpeg_convert(&size, data, w, h, alpha, quality);
	else
	  d = eet_data_image_jpeg_alpha_convert(&size, data, w, h, alpha, quality);
     }
   if (size_ret) *size_ret = size;
   return d;
}

EAPI int
eet_data_image_header_decode(const void *data, int size, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy)
{
   int header[8];

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

   if (size < 32) return 0;

   memcpy(header, data, 32);
   if (words_bigendian)
     {
	int i;

	for (i = 0; i < 8; i++) SWAP32(header[i]);
     }
   if ((unsigned)header[0] == 0xac1dfeed)
     {
	int iw, ih, al, cp;

	iw = header[1];
	ih = header[2];
	al = header[3];
	cp = header[4];
	if ((iw > 8192) || (ih > 8192)) return 0;
	if ((cp == 0) && (size < ((iw * ih * 4) + 32))) return 0;
	if (w) *w = iw;
	if (h) *h = ih;
	if (alpha) *alpha = al;
	if (compress) *compress = cp;
	if (lossy) *lossy = 0;
	if (quality) *quality = 100;
	return 1;
     }
   else if ((unsigned)header[0] == 0xbeeff00d)
     {
	unsigned int iw = 0, ih = 0;
	unsigned const char *dt;
	int sz1, sz2;
	int ok;

	sz1 = header[1];
	sz2 = header[2];
	dt = data;
	dt += 12;
	ok = eet_data_image_jpeg_header_decode(dt, sz1, &iw, &ih);
	if (ok)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = 1;
	     if (compress) *compress = 0;
	     if (lossy) *lossy = 1;
	     if (quality) *quality = 75;
	     return 1;
	  }
     }
   else
     {
	unsigned int iw = 0, ih = 0;
	int ok;

	ok = eet_data_image_jpeg_header_decode(data, size, &iw, &ih);
	if (ok)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = 0;
	     if (compress) *compress = 0;
	     if (lossy) *lossy = 1;
	     if (quality) *quality = 75;
	     return 1;
	  }
     }
   return 0;
}

EAPI void *
eet_data_image_decode(const void *data, int size, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy)
{
   unsigned int *d = NULL;
   int header[8];

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

   if (size < 32) return NULL;

   memcpy(header, data, 32);
   if (words_bigendian)
     {
	int i;

	for (i = 0; i < 8; i++) SWAP32(header[i]);
     }
   if ((unsigned)header[0] == 0xac1dfeed)
     {
	int iw, ih, al, cp;
	unsigned int *body;

	iw = header[1];
	ih = header[2];
	al = header[3];
	cp = header[4];
	if ((iw > 8192) || (ih > 8192)) return NULL;
	if ((cp == 0) && (size < ((iw * ih * 4) + 32))) return NULL;
	body = ((unsigned int *)data) + 8;
	d = malloc(iw * ih * 4);
	if (!d) return NULL;
	if (!cp)
	  {
	     memcpy(d, body, iw * ih * 4);
	     if (words_bigendian)
	       {
		  int x;

		  for (x = 0; x < (iw * ih); x++) SWAP32(d[x]);
	       }
	  }
	else
	  {
	     uLongf dlen;

	     dlen = iw * ih * 4;
	     uncompress((Bytef *)d, &dlen, (Bytef *)body,
			(uLongf)(size - 32));
	     if (words_bigendian)
	       {
		  int x;

		  for (x = 0; x < (iw * ih); x++) SWAP32(d[x]);
	       }
	  }
	if (d)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = al;
	     if (compress) *compress = cp;
	     if (lossy) *lossy = 0;
	     if (quality) *quality = 100;
	  }
     }
   else if ((unsigned)header[0] == 0xbeeff00d)
     {
	unsigned int iw = 0, ih = 0;
	unsigned const char *dt;
	int sz1, sz2;

	sz1 = header[1];
	sz2 = header[2];
	dt = data;
	dt += 12;
	d = eet_data_image_jpeg_rgb_decode(dt, sz1, &iw, &ih);
	if (d)
	  {
	     dt += sz1;
	     eet_data_image_jpeg_alpha_decode(dt, sz2, d, &iw, &ih);
	  }
	if (d)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = 1;
	     if (compress) *compress = 0;
	     if (lossy) *lossy = 1;
	     if (quality) *quality = 75;
	  }
     }
   else
     {
	unsigned int iw = 0, ih = 0;

	d = eet_data_image_jpeg_rgb_decode(data, size, &iw, &ih);
	if (d)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = 0;
	     if (compress) *compress = 0;
	     if (lossy) *lossy = 1;
	     if (quality) *quality = 75;
	  }
     }
   return d;
}
