#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include<Eina.h>
#include "Eolian.h"
#include "eolian_suite.h"

#ifdef HAVE_EVIL
#include "Evil.h"
#endif

static Eina_Bool
_files_compare (const char *ref_filename, const char *tmp_filename)
{
   Eina_Bool result = EINA_FALSE;

   FILE *tmp_file = NULL, *ref_file = NULL;
   char *tmp_content = NULL, *ref_content = NULL;

   tmp_file = fopen(tmp_filename, "rb");
   if (!tmp_file)
     {
        printf("Unable to open %s\n", tmp_filename);
        goto end;
     }
   ref_file = fopen(ref_filename, "rb");
   if (!ref_file)
     {
        printf("Unable to open %s\n", ref_filename);
        goto end;
     }

   fseek(tmp_file, 0, SEEK_END);
   long tmp_filesize = ftell(tmp_file);
   if (tmp_filesize < 0) goto end;
   fseek(tmp_file, 0, SEEK_SET);
   tmp_content = malloc(tmp_filesize + 1);
   if (fread(tmp_content, tmp_filesize, 1, tmp_file) != 1)
     goto end;
   tmp_content[tmp_filesize] = '\0';

   fseek(ref_file, 0, SEEK_END);
   long ref_filesize = ftell(ref_file);
   if (ref_filesize < 0) goto end;
   fseek(ref_file, 0, SEEK_SET);
   ref_content = malloc(ref_filesize + 1);
   if (fread(ref_content, ref_filesize, 1, ref_file) != 1)
     goto end;
   ref_content[ref_filesize] = '\0';

   if (tmp_filesize != ref_filesize) goto end;

   result = !memcmp(ref_content, tmp_content, ref_filesize);
end:
   if (tmp_file) fclose(tmp_file);
   if (ref_file) fclose(ref_file);
   if (tmp_content) free(tmp_content);
   if (ref_content) free(ref_content);

   return result;
}

static int
_eolian_gen_execute(const char *eo_filename, const char *options, const char *output_filename)
{
   char eolian_gen_path[PATH_MAX] = "";
   char command[PATH_MAX];

   if (getenv("EFL_RUN_IN_TREE"))
     {
        snprintf(eolian_gen_path, sizeof(eolian_gen_path),
              "%s/src/bin/eolian/eolian_gen",
              PACKAGE_BUILD_DIR);
     }
   if (eolian_gen_path[0] == '\0')
      return -1;

   snprintf(command, PATH_MAX,
         "%s %s -I "PACKAGE_DATA_DIR"/data -o %s %s",
         eolian_gen_path, options, output_filename, eo_filename);
   return system(command);
}

START_TEST(eolian_dev_impl_code)
{
   char output_filepath[PATH_MAX] = "";
   snprintf(output_filepath, PATH_MAX, "%s/object_impl.c",
#ifdef HAVE_EVIL
         (char *)evil_tmpdir_get()
#else
         "/tmp"
#endif
         );
   remove(output_filepath);
   fail_if(0 != _eolian_gen_execute(PACKAGE_DATA_DIR"/data/object_impl.eo", "--eo --gi", output_filepath));
   fail_if(!_files_compare(PACKAGE_DATA_DIR"/data/object_impl_ref.c", output_filepath));
   /* Check that nothing is added */
   fail_if(0 != _eolian_gen_execute(PACKAGE_DATA_DIR"/data/object_impl.eo", "--eo --gi", output_filepath));
   fail_if(!_files_compare(PACKAGE_DATA_DIR"/data/object_impl_ref.c", output_filepath));
   fail_if(0 != _eolian_gen_execute(PACKAGE_DATA_DIR"/data/object_impl_add.eo", "--eo --gi", output_filepath));
   fail_if(!_files_compare(PACKAGE_DATA_DIR"/data/object_impl_add_ref.c", output_filepath));
}
END_TEST

START_TEST(eolian_types_generation)
{
   char output_filepath[PATH_MAX] = "";
   snprintf(output_filepath, PATH_MAX, "%s/types_output.c",
#ifdef HAVE_EVIL
         (char *)evil_tmpdir_get()
#else
         "/tmp"
#endif
         );
   remove(output_filepath);
   fail_if(0 != _eolian_gen_execute(PACKAGE_DATA_DIR"/data/typedef.eo", "--eo --gh", output_filepath));
   fail_if(!_files_compare(PACKAGE_DATA_DIR"/data/typedef_ref.c", output_filepath));
   fail_if(0 != _eolian_gen_execute(PACKAGE_DATA_DIR"/data/struct.eo", "--eo --gh", output_filepath));
   fail_if(!_files_compare(PACKAGE_DATA_DIR"/data/struct_ref.c", output_filepath));
}
END_TEST

void eolian_generation_test(TCase *tc)
{
   tcase_add_test(tc, eolian_types_generation);
   tcase_add_test(tc, eolian_dev_impl_code);
}

