#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

static int _evas_loader_xpm_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_xpm_log_dom, __VA_ARGS__)

static Eina_File *rgb_txt;
static void *rgb_txt_map;

static int
_xpm_hexa_int(const char *s, int len)
{
   const char *hexa = "0123456789abcdef";
   const char *lookup;
   int i, c, r;

   for (r = 0, i = 0; i < len; i++)
     {
        c = s[i];
        lookup = strchr(hexa, tolower(c));
        r = (r << 4) | (lookup ? lookup - hexa : 0);
     }

   return r;
}

static void
xpm_parse_color(char *color, int *r, int *g, int *b)
{
   char *tmp;
   char *max;
   char *endline;
   char buf[4096];

   /* is a #ff00ff like color */
   if (color[0] == '#')
     {
        int                 len;

        len = strlen(color) - 1;
        if (len < 13)
          {

             len /= 3;
             *r = _xpm_hexa_int(&(color[1 + (0 * len)]), len);
             *g = _xpm_hexa_int(&(color[1 + (1 * len)]), len);
             *b = _xpm_hexa_int(&(color[1 + (2 * len)]), len);
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
   if (!rgb_txt) return;
   tmp = rgb_txt_map;
   max = tmp + eina_file_size_get(rgb_txt);

   while (tmp < max)
     {
        endline = memchr(tmp, '\n', max - tmp);
        if (!endline) endline = max;
        if ((*tmp != '!') && ((endline - tmp) < (int) (sizeof(buf) - 1)))
          {
             int rr, gg, bb;
             char name[4096];

             /* FIXME: not really efficient, should be loaded once in memory with a lookup table */
             memcpy(buf, tmp, endline - tmp);
             buf[endline - tmp + 1] = '\0';

             if (sscanf(buf, "%i %i %i %[^\n]", &rr, &gg, &bb, name) == 4)
	       {
		  if (!strcasecmp(name, color))
		    {
		       *r = rr;
		       *g = gg;
		       *b = bb;
		       return;
		    }
	       }
          }
        tmp = endline + 1;
     }
}

typedef struct _CMap CMap;
struct _CMap {
   EINA_RBTREE;
   short         r, g, b;
   char          str[6];
   unsigned char transp;
};

Eina_Rbtree_Direction
_cmap_cmp_node_cb(const Eina_Rbtree *left, const Eina_Rbtree *right, void *data EINA_UNUSED)
{
   CMap *lcm;
   CMap *rcm;

   lcm = EINA_RBTREE_CONTAINER_GET(left, CMap);
   rcm = EINA_RBTREE_CONTAINER_GET(right, CMap);

   if (strcmp(lcm->str, rcm->str) < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

int
_cmap_cmp_key_cb(const Eina_Rbtree *node, const void *key, int length EINA_UNUSED, void *data EINA_UNUSED)
{
   CMap *root = EINA_RBTREE_CONTAINER_GET(node, CMap);

   return strcmp(root->str, key);
}

/** FIXME: clean this up and make more efficient  **/
static Eina_Bool
evas_image_load_file_xpm(Eina_File *f, Evas_Image_Property *prop, void *pixels, int load_data, int *error)
{
   DATA32      *ptr, *end, *head = NULL;
   const char  *map = NULL;
   size_t       length;
   size_t       position;

   int          pc, c, i, j, k, w, h, ncolors, cpp, comment, transp,
                quote, context, len, done, r, g, b, backslash, lu1, lu2;
   char        *line = NULL;
   char         s[256], tok[256], col[256], *tl;
   int          lsz = 256;
   CMap        *cmap = NULL;
   Eina_Rbtree *root = NULL;

   short        lookup[128 - 32][128 - 32];
   int          count, size;
   Eina_Bool    res = EINA_FALSE;

   done = 0;
//   transp = -1;
   transp = 1;

   /* if immediate_load is 1, then dont delay image laoding as below, or */
   /* already data in this image - dont load it again */

   length = eina_file_size_get(f);
   position = 0;
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (length < 9)
     {
        ERR("XPM ERROR: file size, %zd, is to small", length);
        goto on_error;
     }

   map = eina_file_map_all(f, load_data ? EINA_FILE_WILLNEED : EINA_FILE_RANDOM);
   if (!map)
     {
        ERR("XPM ERROR: file failed to mmap");
        goto on_error;
     }

   if (strncmp("/* XPM */", map, 9))
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
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
   size = 0;
   count = 0;
   line = malloc(lsz);
   if (!line)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error;
     }

   backslash = 0;
   memset(lookup, 0, sizeof(lookup));
   while (!done)
     {
        pc = c;
        if (position == length) break ;
        c = (char) map[position++];
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
                       if (sscanf(line, "%i %i %i %i", &w, &h, &ncolors, &cpp) != 4)
			 {
			    ERR("XPM ERROR: XPM file malformed header");
			    *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
                            goto on_error;
			 }
                       if ((ncolors > 32766) || (ncolors < 1))
                         {
                            ERR("XPM ERROR: XPM files with colors > 32766 or < 1 not supported");
			    *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                            goto on_error;
                         }
                       if ((cpp > 5) || (cpp < 1))
                         {
			    ERR("XPM ERROR: XPM files with characters per pixel > 5 or < 1not supported");
			    *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                            goto on_error;
                         }
                       if ((w > IMG_MAX_SIZE) || (w < 1))
                         {
			    ERR("XPM ERROR: Image width > IMG_MAX_SIZE or < 1 pixels for file");
			    *error = EVAS_LOAD_ERROR_GENERIC;
                            goto on_error;
                         }
                       if ((h > IMG_MAX_SIZE) || (h < 1))
                         {
			    ERR("XPM ERROR: Image height > IMG_MAX_SIZE or < 1 pixels for file");
			    *error = EVAS_LOAD_ERROR_GENERIC;
                            goto on_error;
                         }
                       if (IMG_TOO_BIG(w, h))
                         {
                            ERR("XPM ERROR: Image just too big to ever allocate");
			    *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                            goto on_error;
                         }

                       if (!cmap)
                         {
                            cmap = malloc(sizeof(CMap) * ncolors);
                            if (!cmap)
                              {
				*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                                goto on_error;
                              }
                         }

                       if (!load_data)
                         {
                            prop->w = w;
                            prop->h = h;
                         }
                       else if ((int) prop->w != w ||
                                (int) prop->h != h)
                         {
                            *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                            goto on_error;
                         }


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
                            if (load_data) root = eina_rbtree_inline_insert(root, EINA_RBTREE_GET(&cmap[j]), _cmap_cmp_node_cb, NULL);
			    for (slen = 0; slen < cpp; slen++)
			      {
				 /* fix the ascii of the  color string - if its < 32 - just limit to 32 */
				 if (cmap[j].str[slen] < 32) cmap[j].str[slen] = 0;
			      }
                            cmap[j].r = -1;
                            cmap[j].transp = 0;
                            for (k = cpp; k < len; k++)
                              {
                                 if (line[k] != ' ')
                                   {
                                      const char *tmp = strchr(&line[k], ' ');
                                      slen = tmp ? tmp - &line[k]: 255;

                                      strncpy(s, &line[k], slen);
                                      s[slen] = 0;
                                      k += slen;
                                      if (slen == 1 && *s == 'c') iscolor = 1;
                                      if ((slen == 1 && ((s[0] == 'm') || (s[0] == 's') || (s[0] == 'g') || (s[0] == 'c'))) ||
					  (slen == 2 && (s[0] == 'g') && (s[1] == '4')) ||
					  (k >= len))
                                        {
                                           if (k >= len)
                                             {
                                                if (col[0])
						  {
						     if (strlen(col) < (sizeof(col) - 2))
						       strcat(col, " ");
						     else
						       done = 1;
						  }
                                                if ((strlen(col) + strlen(s)) < (sizeof(col) - 1))
						  strcat(col, s);
                                             }
                                           if (col[0])
                                             {
                                                if (!strcasecmp(col, "none"))
                                                  {
                                                     transp = 1;
                                                     cmap[j].transp = 1;
						     cmap[j].r = 0;
						     cmap[j].g = 0;
						     cmap[j].b = 0;
                                                  }
                                                else
                                                  {
                                                     if ((((cmap[j].r < 0) || (!strcmp(tok, "c"))) && (!hascolor)))
                                                       {
                                                          r = g = b = 0;
                                                          xpm_parse_color(col, &r, &g, &b);
                                                          cmap[j].r = r;
                                                          cmap[j].g = g;
                                                          cmap[j].b = b;
                                                          if (iscolor) hascolor = 1;
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
					   if ((strlen(col) + strlen(s)) < (sizeof(col) - 1))
					     strcat(col, s);
                                        }
                                   }
                              }
                         }
                       j++;
                       if (load_data && j >= ncolors)
                         {
                            if (cpp == 1)
			      {
				 for (i = 0; i < ncolors; i++)
				   lookup[(int)cmap[i].str[0] - 32][0] = i;
			      }
                            if (cpp == 2)
			      {
				 for (i = 0; i < ncolors; i++)
				   lookup[(int)cmap[i].str[0] - 32][(int)cmap[i].str[1] - 32] = i;
			      }
                            context++;
                         }

                       if (transp) prop->alpha = 1;

                       if (load_data)
                         {
                            ptr = pixels;
                            head = ptr;
                            if (!ptr)
                              {
				 *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                                 goto on_error;
                              }
                            size = w * h;
                            end = ptr + size;
                         }
                       else
                         {
			    *error = EVAS_LOAD_ERROR_NONE;
                            goto on_success;
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
		       /* it's xpm - don't care about speed too much. still faster
			* that most xpm loaders anyway */
                       else if (cpp == 1)
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
				      lu1 = (int)line[i] - 32;
				      if (lu1 < 0) continue;
                                      if (cmap[lookup[lu1][0]].transp)
                                        {
					   r = (unsigned char)cmap[lookup[lu1][0]].r;
                                           g = (unsigned char)cmap[lookup[lu1][0]].g;
                                           b = (unsigned char)cmap[lookup[lu1][0]].b;
                                           *ptr = RGB_JOIN(r, g, b);
					   ptr++;
                                           count++;
                                        }
                                      else
                                        {
                                           r = (unsigned char)cmap[lookup[lu1][0]].r;
                                           g = (unsigned char)cmap[lookup[lu1][0]].g;
                                           b = (unsigned char)cmap[lookup[lu1][0]].b;
                                           *ptr = ARGB_JOIN(0xff, r, g, b);
					   ptr++;
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
				      lu1 = (int)line[i] - 32;
				      if (lu1 < 0) continue;
				      r = (unsigned char)cmap[lookup[lu1][0]].r;
				      g = (unsigned char)cmap[lookup[lu1][0]].g;
				      b = (unsigned char)cmap[lookup[lu1][0]].b;
				      *ptr = ARGB_JOIN(0xff, r, g, b);
				      ptr++;
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
				      lu1 = (int)line[i] - 32;
				      i++;
				      lu2 = (int)line[i] - 32;
				      if (lu1 < 0) continue;
				      if (lu2 < 0) continue;
                                      if (cmap[lookup[lu1][lu2]].transp)
                                        {
					   r = (unsigned char)cmap[lookup[lu1][lu2]].r;
                                           g = (unsigned char)cmap[lookup[lu1][lu2]].g;
                                           b = (unsigned char)cmap[lookup[lu1][lu2]].b;
                                           *ptr = RGB_JOIN(r, g, b);
					   ptr++;
                                           count++;
                                        }
                                      else
                                        {
                                           r = (unsigned char)cmap[lookup[lu1][lu2]].r;
                                           g = (unsigned char)cmap[lookup[lu1][lu2]].g;
                                           b = (unsigned char)cmap[lookup[lu1][lu2]].b;
                                           *ptr = ARGB_JOIN(0xff, r, g, b);
					   ptr++;
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
				      lu1 = (int)line[i] - 32;
				      i++;
				      lu2 = (int)line[i] - 32;
				      if (lu1 < 0) continue;
				      if (lu2 < 0) continue;
				      r = (unsigned char)cmap[lookup[lu1][lu2]].r;
				      g = (unsigned char)cmap[lookup[lu1][lu2]].g;
				      b = (unsigned char)cmap[lookup[lu1][lu2]].b;
				      *ptr = ARGB_JOIN(0xff, r, g, b);
				      ptr++;
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
                                      Eina_Rbtree *l;

                                      for (j = 0; j < cpp; j++, i++)
					{
					   col[j] = line[i];
					   if (col[j] < 32) col[j] = 32;
					}
                                      col[j] = 0;
                                      i--;

                                      l = eina_rbtree_inline_lookup(root, col, j, _cmap_cmp_key_cb, NULL);
                                      if (l)
                                        {
                                           CMap *cm = EINA_RBTREE_CONTAINER_GET(l, CMap);

                                           r = (unsigned char)cm->r;
                                           g = (unsigned char)cm->g;
                                           b = (unsigned char)cm->b;
                                           if (cm->transp)
                                             {
                                                *ptr = RGB_JOIN(r, g, b);
                                             }
                                           else
                                             {
                                                *ptr = ARGB_JOIN(0xff, r, g, b);
                                             }

                                           ptr++;
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
                                      Eina_Rbtree *l;

                                      for (j = 0; j < cpp; j++, i++)
                                        {
                                           col[j] = line[i];
                                        }
                                      col[j] = 0;
                                      i--;

                                      l = eina_rbtree_inline_lookup(root, col, 0, _cmap_cmp_key_cb, NULL);
                                      if (l)
                                        {
                                           CMap *cm = EINA_RBTREE_CONTAINER_GET(l, CMap);
                                           
                                           r = (unsigned char)cm->r;
                                           g = (unsigned char)cm->g;
                                           b = (unsigned char)cm->b;
                                           *ptr = ARGB_JOIN(0xff, r, g, b);
                                           ptr++;
                                           count++;
                                        }
                                   }
                              }
                         }
                    }
               }
          }
        /* Scan in line from XPM file */
        if ((!comment) && (quote) && (c != '"'))
          {
             if (c < 32) c = 32;
             else if (c > 127) c = 127;
	     if (c =='\\')
	       {
		  if (++backslash < 2)
		    line[i++] = c;
		  else
		    backslash = 0;
	       }
	     else
	       {
		  backslash = 0;
		  line[i++] = c;
	       }
          }
        if (i >= lsz)
          {
             lsz += 256;
             tl = realloc(line, lsz);
             if (!tl) break;
	     line = tl;
          }
        if (((ptr) && ((ptr - head) >= (w * h))) ||
            ((context > 1) && (count >= size)))
	  break;
     }

 on_success:
   *error = EVAS_LOAD_ERROR_NONE;
   res = EINA_TRUE;

 on_error:
   if (map) eina_file_map_free(f, (void*) map);
   free(cmap);
   free(line);
   return res;
}

static void *
evas_image_load_file_open_xpm(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			      Evas_Image_Load_Opts *opts EINA_UNUSED,
			      Evas_Image_Animated *animated EINA_UNUSED,
			      int *error EINA_UNUSED)
{
  return f;
}

static void
evas_image_load_file_close_xpm(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_xpm(void *loader_data,
			      Evas_Image_Property *prop,
			      int *error)
{
   Eina_File *f = loader_data;

   return evas_image_load_file_xpm(f, prop, NULL, 0, error);
}

static Eina_Bool
evas_image_load_file_data_xpm(void *loader_data,
			      Evas_Image_Property *prop,
			      void *pixels,
			      int *error)
{
   Eina_File *f = loader_data;

   return evas_image_load_file_xpm(f, prop, pixels, 1, error);
}

static Evas_Image_Load_Func evas_image_load_xpm_func =
{
  evas_image_load_file_open_xpm,
  evas_image_load_file_close_xpm,
  evas_image_load_file_head_xpm,
  evas_image_load_file_data_xpm,
  NULL,
  EINA_FALSE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_loader_xpm_log_dom = eina_log_domain_register
     ("evas-xpm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_loader_xpm_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* Shouldn't we make that PATH configurable ? */
   rgb_txt = eina_file_open("/usr/lib/X11/rgb.txt", 0);
   if (!rgb_txt) rgb_txt = eina_file_open("/usr/X11/lib/X11/rgb.txt", 0);
   if (!rgb_txt) rgb_txt = eina_file_open("/usr/X11R6/lib/X11/rgb.txt", 0);
   if (!rgb_txt) rgb_txt = eina_file_open("/usr/openwin/lib/X11/rgb.txt", 0);
   if (rgb_txt)
     rgb_txt_map = eina_file_map_all(rgb_txt, EINA_FILE_WILLNEED);
   em->functions = (void *)(&evas_image_load_xpm_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (rgb_txt)
     {
        eina_file_map_free(rgb_txt, rgb_txt_map);
        eina_file_close(rgb_txt);
        rgb_txt = NULL;
     }
   eina_log_domain_unregister(_evas_loader_xpm_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "xpm",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, xpm);

#ifndef EVAS_STATIC_BUILD_XPM
EVAS_EINA_MODULE_DEFINE(image_loader, xpm);
#endif
