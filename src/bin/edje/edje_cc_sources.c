#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "edje_cc.h"

static Eet_Data_Descriptor *_srcfile_edd = NULL;
static Eet_Data_Descriptor *_srcfile_list_edd = NULL;

static Eet_Data_Descriptor *_external_edd = NULL;
static Eet_Data_Descriptor *_external_list_edd = NULL;

static Eet_Data_Descriptor *_font_edd = NULL;
static Eet_Data_Descriptor *_font_list_edd = NULL;

static SrcFile_List srcfiles = {NULL};

void
source_edd(void)
{
   Eet_Data_Descriptor_Class eddc;

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "srcfile", sizeof (SrcFile));
   _srcfile_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "name", name, EET_T_INLINED_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "file", file, EET_T_INLINED_STRING);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "srcfile_list", sizeof (SrcFile_List));
   _srcfile_list_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_srcfile_list_edd, SrcFile_List, "list", list, _srcfile_edd);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "external", sizeof (External));
   _external_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_external_edd, External, "name", name, EET_T_INLINED_STRING);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "external_list", sizeof (External_List));
   _external_list_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_external_list_edd, External_List, "list", list, _external_edd);

   _edje_data_font_list_desc_make(&_font_list_edd, &_font_edd);
}

static void source_fetch_file(const char *fil, const char *filname);

static void
source_fetch_file(const char *fil, const char *filname)
{
   FILE *f;
   char buf[16 * 1024], *dir = NULL;
   long sz;
   size_t tmp;
   ssize_t dir_len = 0;
   SrcFile *sf;

   f = fopen(fil, "rb");
   if (!f)
     {
        ERR("Cannot open file '%s'", fil);
        exit(-1);
     }

   fseek(f, 0, SEEK_END);
   sz = ftell(f);
   fseek(f, 0, SEEK_SET);
   sf = mem_alloc(SZ(SrcFile));
   sf->name = mem_strdup(filname);
   sf->file = mem_alloc(sz + 1);
   if (sz > 0)
     {
        tmp = fread(sf->file, sz, 1, f);
        if (tmp != 1)
          {
             ERR("file length for (%s) doesn't match!", filname);
             exit(-1);
          }
     }

   sf->file[sz] = '\0';
   fseek(f, 0, SEEK_SET);
   srcfiles.list = eina_list_append(srcfiles.list, sf);

   while (fgets(buf, sizeof(buf), f))
     {
        char *p, *pp;
        int forgetit = 0;
        int haveinclude = 0;
        char *file = NULL, *fname = NULL;

        p = buf;
        while ((!forgetit) && (*p))
          {
             if (!isspace(*p))
               {
                  if (*p != '#')
                    forgetit = 1;
               }
             p++;

             if (!haveinclude)
               {
                  if (!isspace(*p))
                    {
                       if (!strncmp(p, "include", 7))
                         {
                            haveinclude = 1;
                            p += 7;
                         }
                       /* HACK! the logic above should be fixed so
                        * preprocessor statements don't have to begin
                        * in column 0.
                        * otoh, edje_cc should print a warning in that case,
                        * since according to the standard, preprocessor
                        * statements need to be put in column 0.
                        */
                       else if (!strncmp(p, "#include", 8))
                         {
                            haveinclude = 1;
                            p += 8;
                         }
                       else
                         forgetit = 1;
                    }
               }
             else
               {
                  if (!isspace(*p))
                    {
                       char end = '\0';

                       if (*p == '"') end = '"';
                       else if (*p == '<')
                         end = '>';

                       if (end)
                         {
                            pp = strchr(p + 1, end);
                            if (!pp)
                              forgetit = 1;
                            else
                              {
                                 ssize_t l = 0;

                                 /* get the directory of the current file
                                  * if we haven't already done so
                                  */
                                 if (!dir)
                                   {
                                      dir = ecore_file_dir_get(fil);
                                      if (dir) dir_len = strlen(dir);
                                   }

                                 l = pp - p + dir_len + 1;
                                 file = mem_alloc(l);

                                 if (!dir_len)
                                   {
                                      snprintf(file, l - 1, "%s", p + 1);
                                      file[l - 2] = 0;
                                   }
                                 else
                                   {
                                      snprintf(file, l, "%s/%s", dir, p + 1);
                                      file[l - 1] = 0;
                                   }

                                 fname = strdup(p + 1);
                                 pp = strrchr(fname, end);
                                 if (pp) *pp = 0;
                                 forgetit = 1;
                              }
                         }
                       else
                         forgetit = 1;
                    }
                  else
                    p++;
               }
          }
        if ((file) && (fname))
          source_fetch_file(file, fname);

        if (file) free(file);
        if (fname) free(fname);
     }
   free(dir);
   fclose(f);
}

void
source_fetch(void)
{
   source_fetch_file(file_in, ecore_file_file_get(file_in));
}

int
source_append(Eet_File *ef)
{
   return eet_data_write(ef, _srcfile_list_edd, "edje_sources", &srcfiles,
                         compress_mode);
}

SrcFile_List *
source_load(Eet_File *ef)
{
   SrcFile_List *s;

   s = eet_data_read(ef, _srcfile_list_edd, "edje_sources");
   return s;
}

int
source_fontmap_save(Eet_File *ef, Eina_List *font_list)
{
   Edje_Font_List fl;

   fl.list = font_list;
   return eet_data_write(ef, _font_list_edd, "edje_source_fontmap", &fl,
                         compress_mode);
}

Edje_Font_List *
source_fontmap_load(Eet_File *ef)
{
   Edje_Font_List *fl;

   fl = eet_data_read(ef, _font_list_edd, "edje_source_fontmap");
   return fl;
}

