#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <webp/encode.h>

#include "evas_common_private.h"
#include "evas_private.h"

static int evas_image_save_file_webp(RGBA_Image *im, const char *file, const char *key, int quality, int compress, const char *encoding);

static Evas_Image_Save_Func evas_image_save_webp_func =
{
   evas_image_save_file_webp
};

static int writer(const uint8_t *data, size_t data_size, const WebPPicture *const pic)
{
	FILE *out = (FILE *)pic->custom_ptr;
	return data_size ? (fwrite(data, data_size, 1, out) == 1) : 1;
}

static int
save_image_webp(RGBA_Image *im, const char *file, int quality)
{
	WebPPicture picture;
	WebPConfig config;
	int result = 0;

	if (!im || !im->image.data || !file)
		return 0;

	if (!WebPPictureInit(&picture) || !WebPConfigInit(&config))
		return 0;

	picture.width = im->cache_entry.w;
	picture.height = im->cache_entry.h;
	picture.use_argb = 1;
	if (im->cache_entry.flags.alpha)
		picture.colorspace |= WEBP_CSP_ALPHA_BIT;
	else
		picture.colorspace &= ~WEBP_CSP_ALPHA_BIT;

	if (!WebPPictureAlloc(&picture)) // allocates picture.argb
		return 0;
	memcpy(picture.argb, im->image.data, picture.width * picture.height * sizeof(DATA32));
	evas_common_convert_argb_unpremul(picture.argb, picture.width * picture.height);

	if (quality == 100)
		config.lossless = 1;
	else
		config.quality = quality;
	// config.method = 6; // slower, but better quality

	if (!WebPValidateConfig(&config))
		goto free_picture;

	FILE *f = fopen(file, "wb");
	if (f == NULL)
		goto free_picture;

	picture.writer = writer;
	picture.custom_ptr = (void *)f;

	result = WebPEncode(&config, &picture);

	fclose(f);
 free_picture:
	WebPPictureFree(&picture);

	return result;
}

static int evas_image_save_file_webp(RGBA_Image *im, const char *file, const char *key EINA_UNUSED,
                                     int quality, int compress EINA_UNUSED, const char *encoding EINA_UNUSED)
{
	return save_image_webp(im, file, quality);
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_webp_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "webp",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_SAVER, image_saver, webp);

#ifndef EVAS_STATIC_BUILD_WEBP
EVAS_EINA_MODULE_DEFINE(image_saver, webp);
#endif
