#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ethumb.h>

#include "ethumb_suite.h"

static char tmpdir[PATH_MAX];
static char srcfile[PATH_MAX];
static char thumbfile[PATH_MAX];

static void
_tree_make(void)
{
   FILE *f;

   snprintf(tmpdir, sizeof(tmpdir), "/tmp/ethumb_meta_XXXXXX");
   ck_assert_ptr_ne(mkdtemp(tmpdir), NULL);

   ck_assert_int_lt(snprintf(srcfile, sizeof(srcfile), "%s/pic.png", tmpdir),
                    sizeof(srcfile));
   ck_assert_int_lt(snprintf(thumbfile, sizeof(thumbfile), "%s/thumb.png", tmpdir),
                    sizeof(thumbfile));
   f = fopen(srcfile, "wb");
   ck_assert_ptr_ne(f, NULL);
   ck_assert_int_eq(fwrite(ethumb_test_png, 1, ethumb_test_png_size, f),
                    ethumb_test_png_size);
   fclose(f);
}

static void
_tree_remove(void)
{
   char cmd[PATH_MAX + 16];

   if (!tmpdir[0]) return;
   snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
   if (system(cmd) != 0) fprintf(stderr, "could not remove %s\n", tmpdir);
   tmpdir[0] = '\0';
}

static void
_generated_cb(void *data, Ethumb *e EINA_UNUSED, Eina_Bool success)
{
   Eina_Bool *done = data;

   ck_assert(success);
   *done = EINA_TRUE;
   ecore_main_loop_quit();
}

static unsigned int
_be32(const unsigned char *p)
{
   return ((unsigned int) p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

/* Value of the tEXt chunk of PNG "png" whose keyword is "keyword", or NULL.
 * The CRC of the text chunks crossed on the way is checked. */
static char *
_png_text_get(const unsigned char *png, size_t len, const char *keyword)
{
   size_t pos = 8;
   char *found = NULL;

   ck_assert_int_ge(len, 8);
   ck_assert(!memcmp(png, "\211PNG\r\n\032\n", 8));

   while (pos + 12 <= len)
     {
        unsigned int clen = _be32(png + pos);
        const unsigned char *type = png + pos + 4;

        ck_assert_int_le(pos + 12 + clen, len);

        if (!memcmp(type, "tEXt", 4))
          {
             const char *text = (const char *) png + pos + 8;
             size_t klen = strnlen(text, clen);

             ck_assert_uint_eq(_be32(png + pos + 8 + clen),
                               eina_crc((const char *) type, 4 + clen,
                                        0xffffffff, EINA_TRUE));
             if ((klen < clen) && !strcmp(text, keyword))
               found = strndup(text + klen + 1, clen - klen - 1);
          }
        pos += 12 + clen;
     }
   return found;
}

EFL_START_TEST(ethumb_fdo_thumbnail_names_its_source)
{
   Ethumb *e;
   Eina_Bool done = EINA_FALSE;
   struct stat src, thumb;
   FILE *f;
   unsigned char *png;
   char expected[PATH_MAX + 16], *text;
   char leftovers[PATH_MAX + 16];

   ck_assert_int_eq(ethumb_init(), 1);
   e = ethumb_new();
   ck_assert_ptr_ne(e, NULL);

   ethumb_thumb_format_set(e, ETHUMB_THUMB_FDO);
   ck_assert(ethumb_file_set(e, srcfile, NULL));
   ethumb_thumb_path_set(e, thumbfile, NULL);
   ck_assert(ethumb_generate(e, _generated_cb, &done, NULL));
   if (!done) ecore_main_loop_begin();
   ck_assert(done);

   ck_assert_int_eq(stat(srcfile, &src), 0);
   ck_assert_int_eq(stat(thumbfile, &thumb), 0);
   png = malloc(thumb.st_size);
   ck_assert_ptr_ne(png, NULL);
   f = fopen(thumbfile, "rb");
   ck_assert_ptr_ne(f, NULL);
   ck_assert_int_eq(fread(png, 1, thumb.st_size, f), thumb.st_size);
   fclose(f);

   snprintf(expected, sizeof(expected), "file://%s", srcfile);
   text = _png_text_get(png, thumb.st_size, "Thumb::URI");
   ck_assert_ptr_ne(text, NULL);
   ck_assert_str_eq(text, expected);
   free(text);

   snprintf(expected, sizeof(expected), "%llu", (unsigned long long) src.st_mtime);
   text = _png_text_get(png, thumb.st_size, "Thumb::MTime");
   ck_assert_ptr_ne(text, NULL);
   ck_assert_str_eq(text, expected);
   free(text);

   snprintf(expected, sizeof(expected), "%llu", (unsigned long long) src.st_size);
   text = _png_text_get(png, thumb.st_size, "Thumb::Size");
   ck_assert_ptr_ne(text, NULL);
   ck_assert_str_eq(text, expected);
   free(text);

   free(png);

   snprintf(leftovers, sizeof(leftovers), "%s/.tmp.thumb.png", tmpdir);
   ck_assert_int_ne(stat(leftovers, &thumb), 0);
   snprintf(leftovers, sizeof(leftovers), "%s.meta", thumbfile);
   ck_assert_int_ne(stat(leftovers, &thumb), 0);

   ethumb_free(e);
   ethumb_shutdown();
}
EFL_END_TEST

void
ethumb_test_ethumb(TCase *tc)
{
   tcase_add_checked_fixture(tc, _tree_make, _tree_remove);
   tcase_add_test(tc, ethumb_fdo_thumbnail_names_its_source);
}
