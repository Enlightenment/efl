/* Portions of this code are Copyright 1990, 1998 The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysymdef.h>

#define TBLNUM 4000
#define MIN_REHASH 15
#define MATCHES 10

typedef struct _Info Info;
static struct _Info 
{
   char *name;
   long unsigned int val;
} info[TBLNUM];

/* local function prototypes */
static int _parseline(const char *buf, char *key, long unsigned int *val, char *prefix);

/* local variables */
static char tab[TBLNUM];
static unsigned short offsets[TBLNUM];
static unsigned short indexes[TBLNUM];
static long unsigned int values[TBLNUM];
static char buf[1024];
static int ksnum = 0;

int 
main(int argc, char **argv) 
{
   int max_rehash = 0;
   unsigned long sig;
   int i = 0, j = 0, k = 0, l = 0, z = 0;
   FILE *fptr;
   char *name = NULL, c;
   int first = 0, num_found = 0;
   int best_max_rehash = 0, best_z = 0;
   long unsigned int val;
   char key[128], prefix[128];

   for (l = 1; l < argc; l++) 
     {
        if (!(fptr = fopen(argv[l], "r"))) 
          {
             fprintf(stderr, "Could not open %s\n", argv[l]);
             continue;
          }

        while (fgets(buf, sizeof(buf), fptr)) 
          {
             if (!_parseline(buf, key, &val, prefix))
               continue;

             if (val == XK_VoidSymbol) val = 0;
             if (val > 0x1fffffff) 
               {
                  fprintf(stderr, "Ignoring illegal keysym (%s %lx)\n", 
                          key, val);
                  continue;
               }

             if (!(name = malloc(strlen(prefix) + strlen(key) + 1))) 
               {
                  fprintf(stderr, "Makekeys: Out Of Memory !!\n");
                  exit(EXIT_FAILURE);
               }

             sprintf(name, "%s%s", prefix, key);
             info[ksnum].name = name;
             info[ksnum].val = val;
             ksnum++;
             if (ksnum == TBLNUM) 
               {
                  fprintf(stderr, "Makekeys: Too Many Keysyms!!\n");
                  exit(EXIT_FAILURE);
               }
          }

        fclose(fptr);
     }

   printf("/* This file is generated from keysymdef.h. */\n");
   printf("/* Do Not Edit !! */\n\n");

   best_max_rehash = ksnum;
   num_found = 0;
   for (z = ksnum; z < TBLNUM; z++) 
     {
        max_rehash = 0;
        for (name = tab, i = z; --i >= 0;) 
          *name++ = 0;
        for (i = 0; i < ksnum; i++) 
          {
             name = info[i].name;
             sig = 0;
             while ((c = *name++))
               sig = (sig << 1) + c;
             first = j = sig % z;
             for (k = 0; tab[j]; k++) 
               {
                  j += (first + 1);
                  if (j >= z) j -= z;
                  if (j == first) goto next1;
               }
             tab[j] = 1;
             if (k > max_rehash) max_rehash = k;
          }
        if (max_rehash < MIN_REHASH) 
          {
             if (max_rehash < best_max_rehash) 
               {
                  best_max_rehash = max_rehash;
                  best_z = z;
               }
             num_found++;
             if (num_found >= MATCHES)
               break;
          }
next1: ;
     }

   z = best_z;
   if (z == 0) 
     {
        fprintf(stderr, "Makekeys: Failed to find small enough hash !!\n"
                "Try increasing TBLNUM in makekeys.c\n");
        exit(EXIT_FAILURE);
     }

   printf("#ifdef NEED_KEYSYM_TABLE\n");
   printf("const unsigned char _ecore_xcb_keytable[] = {\n");
   printf("0,\n");
   k = 1;
   for (i = 0; i < ksnum; i++) 
     {
        name = info[i].name;
        sig = 0;
        while ((c = *name++))
          sig = (sig << 1) + c;
        first = j = sig % z;
        while (offsets[j]) 
          {
             j += (first + 1);
             if (j >= z) j -= z;
          }
        offsets[j] = k;
        indexes[i] = k;
        val = info[i].val;
	printf("0x%.2lx, 0x%.2lx, 0x%.2lx, 0x%.2lx, 0x%.2lx, 0x%.2lx, ",
	       (sig >> 8) & 0xff, sig & 0xff,
	       (val >> 24) & 0xff, (val >> 16) & 0xff,
	       (val >> 8) & 0xff, val & 0xff);
        for (name = info[i].name, k += 7; (c = *name++); k++)
          printf("'%c',", c);
        printf((i == (ksnum - 1)) ? "0\n" : "0,\n");
     }

   printf("};\n\n");
   printf("#define KTABLESIZE %d\n", z);
   printf("#define KMAXHASH %d\n", (best_max_rehash + 1));
   printf("\n");
   printf("static const unsigned short hashString[KTABLESIZE] = {\n");

   for (i = 0; i < z;) 
     {
        printf("0x%.4x", offsets[i]);
        i++;
        if (i == z) break;
        printf((i & 7) ? ", " : ",\n");
     }

   printf("\n");
   printf("};\n");
   printf("#endif\n");

   best_max_rehash = ksnum;
   num_found = 0;
   for (z = ksnum; z < TBLNUM; z++) 
     {
        max_rehash = 0;
        for (name = tab, i = z; --i >= 0;) 
          *name++ = 0;
        for (i = 0; i < ksnum; i++) 
          {
             val = info[i].val;
             first = j = val % z;
             for (k = 0; tab[j]; k++) 
               {
                  if (values[j] == val) goto skip1;
                  j += (first + 1);
                  if (j >= z) j -= z;
                  if (j == first) goto next2;
               }
             tab[j] = 1;
             values[j] = val;
             if (k > max_rehash) max_rehash = k;
skip1: ;
          }
        if (max_rehash < MIN_REHASH) 
          {
             if (max_rehash < best_max_rehash) 
               {
                  best_max_rehash = max_rehash;
                  best_z = z;
               }
             num_found++;
             if (num_found >= MATCHES) break;
          }
next2: ;
     }

   z = best_z;
   if (z == 0) 
     {
        fprintf(stderr, "Makekeys: Failed to find small enough hash !!\n"
                "Try increasing TBLNUM in makekeys.c\n");
        exit(EXIT_FAILURE);
     }
   for (i = z; --i >= 0;)
     offsets[i] = 0;

   for (i = 0; i < ksnum; i++) 
     {
        val = info[i].val;
        first = j = val % z;
        while (offsets[j]) 
          {
             if (values[j] == val) goto skip2;
             j += (first + 1);
             if (j >= z) j -= z;
          }
        offsets[j] = indexes[i] + 2;
        values[j] = val;
skip2: ;
     }

   printf("\n");
   printf("#ifdef NEED_VTABLE\n");
   printf("#define VTABLESIZE %d\n", z);
   printf("#define VMAXHASH %d\n", best_max_rehash + 1);
   printf("\n");
   printf("static const unsigned short hashKeysym[VTABLESIZE] = {\n");
   for (i = 0; i < z;) 
     {
        printf("0x%.4x", offsets[i]);
        i++;
        if (i == z) break;
        printf((i & 7) ? ", " : ",\n");
     }
   printf("\n");
   printf("};\n");
   printf("#endif\n");

   return 0;
}

/* local functions */
static int 
_parseline(const char *buf, char *key, long unsigned int *val, char *prefix) 
{
   int i = 0;
   char alias[128];
   char *tmp = NULL, *tmpa = NULL;

   /* try to match XK_foo first */
   i = sscanf(buf, "#define %127s 0x%lx", key, val);
   if ((i == 2) && (tmp = strstr(key, "XK_")))
     {
        memcpy(prefix, key, (tmp - key));
        prefix[tmp - key] = '\0';
        tmp += 3;
        memmove(key, tmp, strlen(tmp) + 1);
        return 1;
     }

   /* try to match an alias */
   i = sscanf(buf, "#define %127s %127s", key, alias);
   if (((i == 2) && (tmp = strstr(key, "XK_"))) && 
       (tmpa = strstr(alias, "XK_"))) 
     {
        memcpy(prefix, key, (tmp - key));
        prefix[tmp - key] = '\0';
        tmp += 3;
        memmove(key, tmp, strlen(tmp) + 1);
        memmove(tmpa, tmpa + 3, strlen(tmpa + 3) + 1);

        for (i = ksnum - 1; i >= 0; i--) 
          {
             if (!strcmp(info[i].name, alias)) 
               {
                  *val = info[i].val;
                  return 1;
               }
          }
        fprintf(stderr, "Cannot find matching definition %s for keysym %s%s\n", 
                alias, prefix, key);
     }

   return 0;
}
