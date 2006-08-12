#include "evas_common.h"
#include "evas_private.h"


int evas_image_load_file_head_xpm(RGBA_Image *im, const char *file, const char *key);
int evas_image_load_file_data_xpm(RGBA_Image *im, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_xpm_func =
{
  evas_image_load_file_head_xpm,
  evas_image_load_file_data_xpm
};


static FILE        *rgb_txt = NULL;

static void
xpm_parse_color(char *color, int *r, int *g, int *b)
{
   char                buf[4096];

   /* is a #ff00ff like color */
   if (color[0] == '#')
     {
        int                 len;
        char                val[32];

        len = strlen(color) - 1;
        if (len < 96)
          {
             int                 i;

             len /= 3;
             for (i = 0; i < len; i++)
                val[i] = color[1 + i + (0 * len)];
             val[i] = 0;
             sscanf(val, "%x", r);
             for (i = 0; i < len; i++)
                val[i] = color[1 + i + (1 * len)];
             val[i] = 0;
             sscanf(val, "%x", g);
             for (i = 0; i < len; i++)
                val[i] = color[1 + i + (2 * len)];
             val[i] = 0;
             sscanf(val, "%x", b);
             if (len == 1)
               {
                  *r = (*r << 4) | *r;
                  *g = (*g << 4) | *g;
                  *b = (*b << 4) | *b;
               }
             else if (len > 2)
               {
                  *r >>= (len - 2) * 4;
                  *g >>= (len - 2) * 4;
                  *b >>= (len - 2) * 4;
               }
          }
        return;
     }
   /* look in rgb txt database */
   if (!rgb_txt)
      rgb_txt = fopen("/usr/X11/lib/X11/rgb.txt", "r");
   if (!rgb_txt)
      return;
   fseek(rgb_txt, 0, SEEK_SET);
   while (fgets(buf, 4000, rgb_txt))
     {
        if (buf[0] != '!')
          {
             int                 rr, gg, bb;
             char                name[4096];

             sscanf(buf, "%i %i %i %[^\n]", &rr, &gg, &bb, name);
             if (!strcasecmp(name, color))
               {
                  *r = rr;
                  *g = gg;
                  *b = bb;
                  return;
               }
          }
     }
}

static void
xpm_parse_done(void)
{
   if (rgb_txt)
      fclose(rgb_txt);
   rgb_txt = NULL;
}


/** FIXME: clean this up and make more efficient  **/
static int
evas_image_load_file_xpm(RGBA_Image *im, const char *file, const char *key, int load_data)
{
   DATA32             *ptr, *end;
   FILE               *f;

   int                 pc, c, i, j, k, w, h, ncolors, cpp, comment, transp,
                       quote, context, len, done, r, g, b, backslash;
   char               *line, s[256], tok[128], col[256];
   int                 lsz = 256;
   struct _cmap {
      unsigned char       str[6];
      unsigned char       transp;
      short               r, g, b;
   }                  *cmap;

   short               lookup[128 - 32][128 - 32];
   float               per = 0.0, per_inc = 0.0;
   int                 last_per = 0, last_y = 0;
   int                 count, pixels;

   if (!file) return 0;
   done = 0;
//   transp = -1;
   transp = 0;

   /* if immediate_load is 1, then dont delay image laoding as below, or */
   /* already data in this image - dont load it again */

   f = fopen(file, "rb");
   if (!f)
     {
        xpm_parse_done();
        return 0;
     }
   fread(s, 1, 9, f);
   rewind(f);
   s[9] = 0;
   if (strcmp("/* XPM */", s))
     {
        fclose(f);
        xpm_parse_done();
        return 0;
     }

   i = 0;
   j = 0;
   cmap = NULL;
   w = 10;
   h = 10;
   ptr = NULL;
   end = NULL;
   c = ' ';
   comment = 0;
   quote = 0;
   context = 0;
   pixels = 0;
   count = 0;
   line = malloc(lsz);
   if (!line)
     {
        fclose(f);
        xpm_parse_done();
        return 0;
     }

   backslash = 0;
   memset(lookup, 0, sizeof(lookup));
   while (!done)
     {
        pc = c;
        c = fgetc(f);
        if (c == EOF)
           break;
        if (!quote)
          {
             if ((pc == '/') && (c == '*'))
                comment = 1;
             else if ((pc == '*') && (c == '/') && (comment))
                comment = 0;
          }
        if (!comment)
          {
             if ((!quote) && (c == '"'))
               {
                  quote = 1;
                  i = 0;
               }
             else if ((quote) && (c == '"'))
               {
                  line[i] = 0;
                  quote = 0;
                  if (context == 0)
                    {
                       /* Header */
                       sscanf(line, "%i %i %i %i", &w, &h, &ncolors, &cpp);
                       if ((ncolors > 32766) || (ncolors < 1))
                         {
                            fprintf(stderr,
                                    "XPM ERROR: XPM files with colors > 32766 or < 1 not supported\n");
                            free(line);
                            fclose(f);
                            xpm_parse_done();
                            return 0;
                         }
                       if ((cpp > 5) || (cpp < 1))
                         {
                            fprintf(stderr,
                                    "XPM ERROR: XPM files with characters per pixel > 5 or < 1not supported\n");
                            free(line);
                            fclose(f);
                            xpm_parse_done();
                            return 0;
                         }
                       if ((w > 32767) || (w < 1))
                         {
                            fprintf(stderr,
                                    "XPM ERROR: Image width > 32767 or < 1 pixels for file\n");
                            free(line);
                            fclose(f);
                            xpm_parse_done();
                            return 0;
                         }
                       if ((h > 32767) || (h < 1))
                         {
                            fprintf(stderr,
                                    "XPM ERROR: Image height > 32767 or < 1 pixels for file\n");
                            free(line);
                            fclose(f);
                            xpm_parse_done();
                            return 0;
                         }

                       if (!cmap)
                         {
                            cmap = malloc(sizeof(struct _cmap) * ncolors);
                            if (!cmap)
                              {
                                free(line);
                                fclose(f);
                                xpm_parse_done();
                                return 0;
                              }
                         }
                       if (!im->image)
                         {
                            im->image = evas_common_image_surface_new(im);
                            if (!im->image)
                              {
                                free(cmap);
                                free(line);
                                fclose(f);
                                xpm_parse_done();
                                return 0;
                              }
                         }
                       im->image->w = w;
                       im->image->h = h;

                       per_inc = 100.0 / (((float)w) * h);

                       j = 0;
                       context++;
                    }
                  else if (context == 1)
                    {
                       /* Color Table */
                       if (j < ncolors)
                         {
                            int                 slen;
                            int                 hascolor, iscolor;

                            iscolor = 0;
                            hascolor = 0;
                            tok[0] = 0;
                            col[0] = 0;
                            s[0] = 0;
                            len = strlen(line);
                            strncpy(cmap[j].str, line, cpp);
                            cmap[j].str[cpp] = 0;
                            cmap[j].r = -1;
                            cmap[j].transp = 0;
                            for (k = cpp; k < len; k++)
                              {
                                 if (line[k] != ' ')
                                   {
                                      s[0] = 0;
                                      sscanf(&line[k], "%65535s", s);
                                      slen = strlen(s);
                                      k += slen;
                                      if (!strcmp(s, "c"))
                                         iscolor = 1;
                                      if ((!strcmp(s, "m")) || (!strcmp(s, "s"))
                                          || (!strcmp(s, "g4"))
                                          || (!strcmp(s, "g"))
                                          || (!strcmp(s, "c")) || (k >= len))
                                        {
                                           if (k >= len)
                                             {
                                                if (col[0])
						  {
						     if (strlen(col) < ( sizeof(col) - 2))
						       strcat(col, " ");
						     else
						       done = 1;
						  }
                                                if (strlen(col) + strlen(s) <
                                                    (sizeof(col) - 1))
                                                   strcat(col, s);
                                             }
                                           if (col[0])
                                             {
                                                if (!strcasecmp(col, "none"))
                                                  {
                                                     transp = 1;
                                                     cmap[j].transp = 1;
                                                  }
                                                else
                                                  {
                                                     if ((((cmap[j].r < 0) ||
                                                           (!strcmp(tok, "c")))
                                                          && (!hascolor)))
                                                       {
                                                          r = 0;
                                                          g = 0;
                                                          b = 0;
                                                          xpm_parse_color(col,
                                                                          &r,
                                                                          &g,
                                                                          &b);
                                                          cmap[j].r = r;
                                                          cmap[j].g = g;
                                                          cmap[j].b = b;
                                                          if (iscolor)
                                                             hascolor = 1;
                                                       }
                                                  }
                                             }
                                           strcpy(tok, s);
                                           col[0] = 0;
                                        }
                                      else
                                        {
					   if (col[0])
					     {
						if (strlen(col) < ( sizeof(col) - 2))
						  strcat(col, " ");
						else
						  done = 1;
					     }
					   if (strlen(col) + strlen(s) <
					       (sizeof(col) - 1))
					     strcat(col, s);
                                        }
                                   }
                              }
                         }
                       j++;
                       if (j >= ncolors)
                         {
                            if (cpp == 1)
                               for (i = 0; i < ncolors; i++)
                                  lookup[(int)cmap[i].str[0] - 32][0] = i;
                            if (cpp == 2)
                               for (i = 0; i < ncolors; i++)
                                  lookup[(int)cmap[i].str[0] -
                                         32][(int)cmap[i].str[1] - 32] = i;
                            context++;
                         }

//                       if (transp >= 0)
                           im->flags |= RGBA_IMAGE_HAS_ALPHA;
                       if (load_data)
                         {
                            evas_common_image_surface_alloc(im->image);
                            if (!im->image->data)
                              {
                                 evas_common_image_surface_free(im->image);
                                 free(cmap);
                                 free(line);
                                 fclose(f);
                                 xpm_parse_done();
                                 return 0;
                              }
                            ptr = im->image->data;
                            end = ptr + (sizeof(DATA32) * w * h);
                            pixels = w * h;
                         }
                       else
                         {
                            free(cmap);
                            free(line);
                            fclose(f);
                            xpm_parse_done();
                            return 1;
                         }
                    }
                  else
                    {
                       /* Image Data */
                       i = 0;
                       if (cpp == 0)
                         {
                            /* Chars per pixel = 0? well u never know */
                         }
                       if (cpp == 1)
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      col[0] = line[i];
                                      if (cmap[lookup[(int)col[0] - 32][0]].
                                          transp)
                                        {
                                           r = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [0]].r;
                                           g = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [0]].g;
                                           b = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [0]].b;
                                           *ptr++ =
                                               0x00ffffff & ((r << 16) |
                                                             (g << 8) | b);
                                           count++;
                                        }
                                      else
                                        {
                                           r = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [0]].r;
                                           g = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [0]].g;
                                           b = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [0]].b;
                                           *ptr++ =
                                               (0xff << 24) | (r << 16) | (g <<
                                                                           8) |
                                               b;
                                           count++;
                                        }
                                   }
                              }
                            else
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      col[0] = line[i];
                                      r = (unsigned char)
                                          cmap[lookup[(int)col[0] - 32][0]].r;
                                      g = (unsigned char)
                                          cmap[lookup[(int)col[0] - 32][0]].g;
                                      b = (unsigned char)
                                          cmap[lookup[(int)col[0] - 32][0]].b;
                                      *ptr++ =
                                          (0xff << 24) | (r << 16) | (g << 8) |
                                          b;
                                      count++;
                                   }
                              }
                         }
                       else if (cpp == 2)
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      col[0] = line[i++];
                                      col[1] = line[i];
                                      if (cmap
                                          [lookup[(int)col[0] - 32]
                                           [(int)col[1] - 32]].transp)
                                        {
                                           r = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [(int)col[1] - 32]].r;
                                           g = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [(int)col[1] - 32]].g;
                                           b = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [(int)col[1] - 32]].b;
                                           *ptr++ =
                                               0x00ffffff & ((r << 16) |
                                                             (g << 8) | b);
                                           count++;
                                        }
                                      else
                                        {
                                           r = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [(int)col[1] - 32]].r;
                                           g = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [(int)col[1] - 32]].g;
                                           b = (unsigned char)
                                               cmap[lookup[(int)col[0] - 32]
                                                    [(int)col[1] - 32]].b;
                                           *ptr++ =
                                               (0xff << 24) | (r << 16) | (g <<
                                                                           8) |
                                               b;
                                           count++;
                                        }
                                   }
                              }
                            else
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      col[0] = line[i++];
                                      col[1] = line[i];
                                      r = (unsigned char)
                                          cmap[lookup[(int)col[0] - 32]
                                               [(int)col[1] - 32]].r;
                                      g = (unsigned char)
                                          cmap[lookup[(int)col[0] - 32]
                                               [(int)col[1] - 32]].g;
                                      b = (unsigned char)
                                          cmap[lookup[(int)col[0] - 32]
                                               [(int)col[1] - 32]].b;
                                      *ptr++ =
                                          (0xff << 24) | (r << 16) | (g << 8) |
                                          b;
                                      count++;
                                   }
                              }
                         }
                       else
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      for (j = 0; j < cpp; j++, i++)
                                        {
                                           col[j] = line[i];
                                        }
                                      col[j] = 0;
                                      i--;
                                      for (j = 0; j < ncolors; j++)
                                        {
                                           if (!strcmp(col, cmap[j].str))
                                             {
                                                if (cmap[j].transp)
                                                  {
                                                     r = (unsigned char)
                                                         cmap[lookup
                                                              [(int)col[0] -
                                                               32][0]].r;
                                                     g = (unsigned char)
                                                         cmap[lookup
                                                              [(int)col[0] -
                                                               32][0]].g;
                                                     b = (unsigned char)
                                                         cmap[lookup
                                                              [(int)col[0] -
                                                               32][0]].b;
                                                     *ptr++ =
                                                         0x00ffffff & ((r << 16)
                                                                       | (g <<
                                                                          8) |
                                                                       b);
                                                     count++;
                                                  }
                                                else
                                                  {
                                                     r = (unsigned char)cmap[j].
                                                         r;
                                                     g = (unsigned char)cmap[j].
                                                         g;
                                                     b = (unsigned char)cmap[j].
                                                         b;
                                                     *ptr++ =
                                                         (0xff << 24) | (r <<
                                                                         16) |
                                                         (g << 8) | b;
                                                     count++;
                                                  }
                                                j = ncolors;
                                             }
                                        }
                                   }
                              }
                            else
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      for (j = 0; j < cpp; j++, i++)
                                        {
                                           col[j] = line[i];
                                        }
                                      col[j] = 0;
                                      i--;
                                      for (j = 0; j < ncolors; j++)
                                        {
                                           if (!strcmp(col, cmap[j].str))
                                             {
                                                r = (unsigned char)cmap[j].r;
                                                g = (unsigned char)cmap[j].g;
                                                b = (unsigned char)cmap[j].b;
                                                *ptr++ =
                                                    (0xff << 24) | (r << 16) |
                                                    (g << 8) | b;
                                                count++;
                                                j = ncolors;
                                             }
                                        }
                                   }
                              }
                         }
                       per += per_inc;
                    }
               }
          }
        /* Scan in line from XPM file */
        if ((!comment) && (quote) && (c != '"'))
          {
             if (c < 32)
                c = 32;
             else if (c > 127)
                c = 127;
	     if ( c=='\\' ) {
	       if ( ++backslash<2 ) {
		 line[i++] = c;
	       }
	       else {
		 backslash = 0;
	       }
	     }
	     else {
	       backslash = 0;
	       line[i++] = c;
	     }
          }
        if (i >= lsz)
          {
             lsz += 256;
             line = realloc(line, lsz);
             if (!line)
               {
                 free(cmap);
                 fclose(f);
               }
          }
        if (((ptr) && ((ptr - im->image->data) >= (w * h * sizeof(DATA32)))) ||
            ((context > 1) && (count >= pixels)))
           done = 1;
     }

   free(cmap);
   free(line);
   fclose(f);

   xpm_parse_done();

   return 1;
}


int
evas_image_load_file_head_xpm(RGBA_Image *im, const char *file, const char *key)
{
  return evas_image_load_file_xpm(im, file, key, 0);
}

int
evas_image_load_file_data_xpm(RGBA_Image *im, const char *file, const char *key)
{
  return evas_image_load_file_xpm(im, file, key, 1);
}



int module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_xpm_func);
   return 1;
}

void module_close(void)
{
   
}

Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_LOADER,
     "xpm",
     "none"
};
