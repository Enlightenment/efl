/*

  -----------------------------[ XCF Loader ]-----------------------------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  ------------------------------------------------------------------------

  There's a quick overview of the XCF file structure in Gimp's source
  tree, in docs/xcf.doc. However it's brief, so here's a more verbose
  overview based on my understanding of XCF.  All image characteristics
  are defined in "properties". In the data stream properties are defined
  through a 4 bit index number (see enum below), followed by a 4 byte
  length. The property data directly follows this information. A list of
  properties ends when PROP_END is encountered. There is a properties
  block at the beginning of the file, as well as at the beginning of each
  layer and channel. Layers and channels are read at offsets in the file,
  the list of layers (resp. channels) is exhausted when the next offset
  read in is zero.

  The actual image data is stored in tiles, which are by default 64x64 in
  size, likely with smaller ones on the right and bottom edges.

  The position of the tiles on the image is left->right, row by row. The
  actual DATA8* data is contained in a "level" which is contained in a
  "hierarchy". I've not really understood the purpose of the hierarchy, as
  it seems to always contain only one level anyway.

  Layer masks are stored as channels (basically grayscale layers with a
  single color definition. For the purpose of this loader I replaced the
  concept of a channel with a layer, since it doesn't really matter.

  Ok, hope this helps with understanding XCF.                 -- cK.

*/
#include <Eina.h>
#include "common.h"
#include "shmfile.h"
#include "timeout.h"

#define FREE(X) { free(X); X = NULL; }

#define TILE_WIDTH   64
#define TILE_HEIGHT  64

/* --------------------------------------------------------------------------- typedefs ------------ */

typedef struct _Layer Layer;
typedef struct _Tile Tile;

/* ------------------------------------------------------------------------------ enums ------------ */


/* These are all the properties that a layer or channel can have.
   Only some of them are actually used. */
typedef enum
{
  PROP_END = 0,
  PROP_COLORMAP = 1,
  PROP_ACTIVE_LAYER = 2,
  PROP_ACTIVE_CHANNEL = 3,
  PROP_SELECTION = 4,
  PROP_FLOATING_SELECTION = 5,
  PROP_OPACITY = 6,
  PROP_MODE = 7,
  PROP_VISIBLE = 8,
  PROP_LINKED = 9,
  PROP_PRESERVE_TRANSPARENCY = 10,
  PROP_APPLY_MASK = 11,
  PROP_EDIT_MASK = 12,
  PROP_SHOW_MASK = 13,
  PROP_SHOW_MASKED = 14,
  PROP_OFFSETS = 15,
  PROP_COLOR = 16,
  PROP_COMPRESSION = 17,
  PROP_GUIDES = 18,
  PROP_RESOLUTION = 19,
  PROP_TATTOO = 20,
  PROP_PARASITES = 21,
  PROP_UNIT = 22,
  PROP_PATHS = 23,
  PROP_USER_UNIT = 24
}
PropType;

/* The tiles can be stored in an encrypted fashion, defined as follows: */
typedef enum
{
  COMPRESS_NONE = 0,
  COMPRESS_RLE = 1,
  COMPRESS_ZLIB = 2,
  COMPRESS_FRACTAL = 3  /* Unused. */
} CompressionType;

/* Layer modes (*SIGH*) */
typedef enum
{
  NORMAL_MODE,
  DISSOLVE_MODE,
  BEHIND_MODE,
  MULTIPLY_MODE,
  SCREEN_MODE,
  OVERLAY_MODE,
  DIFFERENCE_MODE,
  ADDITION_MODE,
  SUBTRACT_MODE,
  DARKEN_ONLY_MODE,
  LIGHTEN_ONLY_MODE,
  HUE_MODE,
  SATURATION_MODE,
  COLOR_MODE,
  VALUE_MODE,
  DIVIDE_MODE,
  ERASE_MODE,         /*< skip >*/
  REPLACE_MODE,       /*< skip >*/
  ANTI_ERASE_MODE     /*< skip >*/
}
LayerModeEffects;

/* Base image types */
typedef enum
{
  RGB,
  GRAY,
  INDEXED
} GimpImageBaseType;

/* Image types */
typedef enum
{
  RGB_GIMAGE,
  RGBA_GIMAGE,
  GRAY_GIMAGE,
  GRAYA_GIMAGE,
  INDEXED_GIMAGE,
  INDEXEDA_GIMAGE
} GimpImageType;

/* ---------------------------------------------------------------------------- structs ------------ */

/* Ok, this is what's left of Gimp's layer abstraction. I kicked out
   all the stuff that's unnecessary and added the necessary stuff
   from the Gimp drawable superclass. This one also serves as a
   Channel, e.g. for use as a layer mask.
                                            --cK.
*/
struct _Layer
{
  int            visible;               /* controls visibility            */
  int            width, height;		/* size of drawable               */
  int            bpp;                   /* depth                          */
  int            offset_x, offset_y;	/* offset of layer in image       */

  int            ID;			/* provides a unique ID           */
  GimpImageType  type;			/* type of drawable               */
  char           has_alpha;		/* drawable has alpha             */

  int            preserve_trans;	/*  preserve transparency         */

  Layer          *mask;                 /*  possible layer mask           */

  int        opacity;                   /*  layer opacity                 */
  LayerModeEffects  mode;               /*  layer combination mode        */


  /* XCF stores the actual image data as tiles. A Layer is covered with
     tiles, the tiles on the right and bottom will (usually) be a bit
     smaller. num_rows and num_cols gives the number of tile rows and
     columns.
  */

  Tile*   tiles;                        /* tiles for drawable data        */
  int     num_rows;
  int     num_cols;

  /* After the tiles are read in, they're serialized int an array
     of DATA8's, that will always contain 4 bpp data. Not optimal,
     I know, but makes life easier
  */

  DATA8*  data;

  /* Layers are stored as a linked list. */
  struct _Layer* next;
  struct _Layer* prev;
};


/* The tile structure:
*/
struct _Tile
{
  unsigned char  bpp;     /* the bytes per pixel (1, 2, 3 or 4) */
  unsigned short ewidth;  /* the effective width of the tile */
  unsigned short eheight; /* the effective height of the tile */

  /* a tile's effective width and height may be smaller
   * (but not larger) than TILE_WIDTH and TILE_HEIGHT.
   * This is to handle edge tiles of a drawable.
   */

  DATA8 *data;
};


/* This struct simply contains everything that didn't fit elsewhere,
   based on GimpImage :)
*/
struct _GimpImage
{
  void               *file;
  char               *filename;
  long                cp;
  int                 compression;     /*  file compression mode        */
  int                 file_version;

  int                 width, height;   /*  width and height attributes  */
  GimpImageBaseType   base_type;       /*  base gimp_image type         */

  DATA32              floating_sel_offset;

  DATA8*              cmap;            /*  colormap--for indexed        */
  int                 num_cols;        /*  number of colors in map      */

 /* If a layer number was passed to the loader, it goes here: */
  int                 single_layer_index;

  /* Tadaa -- the final image data. Layers get pasted
     onto this one, bottom-up.
  */
  DATA8*              data;

  Layer*              layers;
  Layer*              last_layer;
  Layer*              floating_sel;
}
_image;

/* ------------------------------------------------------------------------- prototypes ------------ */

typedef struct _File  File;
typedef struct _Chunk Chunk;

#define FBUF 1
#define CHUNK_SIZE (32 * 1024)

struct _Chunk
{
   int            size;
   unsigned char  data[CHUNK_SIZE];
};

struct _File
{
   int            fd;
   gzFile         fp;
   long           pos, size;
   int            chunk_num;
   Chunk        **chunk;
};

static File *
f_open(const char *file)
{
   File *f;
   
   f = calloc(1, sizeof(File));
   if (!f) return NULL;
   f->fd = open(file, O_RDONLY);
   if (f->fd < 0)
     {
        D("open of %s failed\n", file);
        free(f);
        return NULL;
     }
   f->fp = gzdopen(f->fd, "r");
   if (!f->fp)
     {
        D("gzdopen of %i failed\n", f->fd);
        close(f->fd);
        free(f);
        return NULL;
     }
   return f;
}

static void
f_close(File *f)
{
   // FIXME: free chunks
   gzclose(f->fp);
   free(f);
}

#ifdef FBUF   
static void
_f_read_pos(File *f, long pos, long bytes)
{
   long i, cnum;
   Chunk **cks;
   
   if (f->size > 0) return;
   cnum = ((pos + bytes) / CHUNK_SIZE) + 1;
   if (f->chunk_num >= cnum) return;
   D("FFFF: go up to %li + %li, chunks %li\n", pos, bytes, cnum);
   cks = realloc(f->chunk, sizeof(Chunk *) * cnum);
   if (!cks) return;
   f->chunk = cks;
   for (i = f->chunk_num; i < cnum; i++)
     {
        if (f->size != 0)
          {
             f->chunk[i] = NULL;
             continue;
          }
        f->chunk[i] = malloc(sizeof(Chunk));
        if (f->chunk[i])
          {
             f->chunk[i]->size = gzread(f->fp, f->chunk[i]->data, CHUNK_SIZE);
             D("FFFF: go %i\n", f->chunk[i]->size);
             if (f->chunk[i]->size < CHUNK_SIZE)
               {
                  f->size = (i * CHUNK_SIZE) + f->chunk[i]->size;
               }
          }
     }
   f->chunk_num = cnum;
}
#endif

static long
f_read(File *f, unsigned char *dest, long bytes)
{
#ifdef FBUF   
   long done = 0, off = 0;
   int c;
   unsigned char *p;
   _f_read_pos(f, f->pos, bytes);
   
   c = f->pos / CHUNK_SIZE;
   off = f->pos - (c * CHUNK_SIZE);
   p = dest;
   while ((done < bytes) && (c < f->chunk_num))
     {
        long amount = bytes - done;
        
        if (!f->chunk[c]) break;
        if (amount > (f->chunk[c]->size - off))
           amount = (f->chunk[c]->size - off);
        if (amount < 1) return 0;
        memcpy(p, f->chunk[c]->data + off, amount);
        p += amount;
        off = 0;
        done += amount;
        c++;
     }
   f->pos += done;
   return done;
#else
   long done = gzread(f->fp, dest, bytes);
   f->pos += done;
   return done;
#endif   
}

static void
f_seek(File *f, long pos)
{
#ifdef FBUF
   if (f->size > 0)
     {
        if (pos >= f->size) pos = f->size -1;
     }
#endif   
   if (f->pos == pos) return;
   f->pos = pos;
#ifdef FBUF
   _f_read_pos(f, f->pos, 1);
#else
   gzseek(f->fp, f->pos, SEEK_SET);
#endif   
}


/* stuff that was adapted from xcf.c */

static void       xcf_seek_pos(int pos);
static int        xcf_read_int32(void *fp, DATA32 *data, int count);
static int        xcf_read_int8(void *fp, DATA8 *data, int count);
static int        xcf_read_string(void *fp, char **data, int count);
static char       xcf_load_prop(PropType *prop_type, DATA32 *prop_size);
static void       xcf_load_image(void);
static char       xcf_load_image_props(void);

static Layer*     xcf_load_channel(void);
static char       xcf_load_channel_props(Layer *layer);
static Layer*     xcf_load_layer(void);
static char       xcf_load_layer_props(Layer *layer);
static char       xcf_load_hierarchy(Tile** tiles, int *num_rows, int *num_cols, int *bpp);
static char       xcf_load_level(Tile** tiles, int hierarchy_width, int hierarchy_height, int bpp, int* num_rows, int *num_cols);
static char       xcf_load_tile(Tile *tile);
static char       xcf_load_tile_rle(Tile *tile, int data_length);

/* new stuff :) */

static Tile*      allocate_tiles(int width, int height, int bpp, int* num_rows, int* num_cols);
static void       free_tiles(Tile* tiles, int num_tiles);
static void       init_tile(Tile* tile, int width, int height, int bpp);
static Layer*     new_layer(int width, int height, GimpImageType type, int opacity, LayerModeEffects mode);
static void       free_layer(Layer* layer);
static void       add_layer_to_image(Layer* layer);
static void       read_tiles_into_data(Tile* tiles, int num_cols, int width, int height, int bpp, DATA8** data, int use_cmap);
static void       apply_layer_mask(Layer* layer);
static void       set_layer_opacity(Layer* layer);
static void       flatten_image(void);

static char       xcf_file_init(char* filename);
static void       xcf_cleanup(void);

/* Stuff for layer merging: */
extern void combine_pixels_normal (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_add (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_sub (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_diff (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_darken (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_lighten (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_mult (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_div (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_screen (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_overlay (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_hue (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_sat (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_val (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_col (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);
extern void combine_pixels_diss (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y);

/* ---------------------------------------------------------------------------- globals ------------ */

/* This makes using the Gimp sources easier */
static struct _GimpImage * image = &_image;

/* ------------------------------------------------------------------------------- code ------------ */

static void
xcf_seek_pos(int pos)
{
   if (image->cp != pos)
     {
        image->cp = pos;
        f_seek(image->file, image->cp);
     }
}

static int
xcf_read_int32(void     *fp,
               DATA32   *data,
               int       count)
{
   int total;

   total = count;
   if (count > 0)
     {
        xcf_read_int8(fp, (DATA8*) data, count * 4);
        while (count--)
          {
             *data = (DATA32)eina_ntohl(*data);
             data++;
          }
     }
   return total * 4;
}

static int
xcf_read_int8(void     *fp,
              DATA8    *data,
              int       count)
{
   int total;
   int bytes;

   total = count;
   while (count > 0)
     {
        bytes = f_read(fp, data, count);
        if (bytes <= 0) /* something bad happened */
           break;
        count -= bytes;
        data += bytes;
     }
   return total;
}

static int
xcf_read_string(void     *fp,
                char    **data,
                int       count)
{
   DATA32 tmp;
   int total;
   int i;

   total = 0;
   for (i = 0; i < count; i++)
     {
        total += xcf_read_int32(fp, &tmp, 1);
        if (tmp > 0)
          {
             data[i] = malloc(sizeof(DATA8) * tmp);
             if (data[i]) total += xcf_read_int8(fp, (DATA8 *)data[i], tmp);
          }
        else
           data[i] = NULL;
     }
   return total;
}


static char
xcf_load_prop(PropType *prop_type,
              DATA32 *prop_size)
{
   image->cp += xcf_read_int32(image->file, (DATA32 *)prop_type, 1);
   image->cp += xcf_read_int32(image->file, (DATA32 *)prop_size, 1);
   return 1;
}


static char
xcf_load_image_props(void)
{
   PropType prop_type;
   DATA32 prop_size;

   while (1)
     {
        if (!xcf_load_prop (&prop_type, &prop_size)) return 0;
        switch (prop_type)
          {
           case PROP_END:
               {
                  D("Finished reading image properties.\n");
                  return 1;
               }
           case PROP_COLORMAP:
               {
                  if (image->file_version == 0)
                    {
                       int i;
                       fprintf (stderr,
                                "XCF warning: version 0 of XCF file format\n"
                                "did not save indexed colormaps correctly.\n"
                                "Substituting grayscale map.\n");
                       image->cp += xcf_read_int32(image->file, (DATA32 *)&image->num_cols, 1);
                       image->cmap = malloc(sizeof(DATA8) * image->num_cols * 3);
                       if (!image->cmap) return 0;
                       xcf_seek_pos (image->cp + image->num_cols);
                       for (i = 0; i < image->num_cols; i++)
                         {
                            image->cmap[(i * 3) + 0] = i;
                            image->cmap[(i * 3) + 1] = i;
                            image->cmap[(i * 3) + 2] = i;
                         }
                    }
                  else
                    {
                       D("Loading colormap.\n");
                       image->cp += xcf_read_int32(image->file, (DATA32 *)&image->num_cols, 1);
                       image->cmap = malloc(sizeof(DATA8) * image->num_cols * 3);
                       if (!image->cmap) return 0;
                       image->cp += xcf_read_int8(image->file, (DATA8 *)image->cmap, image->num_cols * 3);
                    }
               }
             break;
           case PROP_COMPRESSION:
               {
                  char compression = 0;

                  image->cp += xcf_read_int8(image->file, (DATA8 *)&compression, 1);

                  if ((compression != COMPRESS_NONE) &&
                      (compression != COMPRESS_RLE) &&
                      (compression != COMPRESS_ZLIB) &&
                      (compression != COMPRESS_FRACTAL))
                    {
                       fprintf (stderr, "unknown xcf compression type: %d\n", (int) compression);
                       return 0;
                    }

                  D("Image compression type: %i\n", compression);

                  image->compression = compression;
               }
             break;
             /* I threw out all of the following: --cK */
           case PROP_TATTOO:
           case PROP_PARASITES:
           case PROP_UNIT:
           case PROP_PATHS:
           case PROP_USER_UNIT:
           case PROP_GUIDES:
           case PROP_RESOLUTION:
	default:
               {
                  DATA8 buf[16];
                  int amount;

                  D("Skipping unexpected/unknown image property: %d\n", prop_type);

                  while (prop_size > 0)
                    {
                       amount = (16 < prop_size ? 16 : prop_size);
                       image->cp += xcf_read_int8(image->file, buf, amount);
                       prop_size -= amount;
                    }
               }
             break;
          }
     }
   return 0;
}


static void
xcf_load_image(void)
{
   Layer *layer;
   DATA32 saved_pos;
   DATA32 offset;
   int num_successful_elements = 0;

   /* read the image properties */
   if (!xcf_load_image_props()) goto hard_error;

   while (1)
     {
        /* read in the offset of the next layer */
        image->cp += xcf_read_int32(image->file, &offset, 1);
        /* if the offset is 0 then we are at the end
         *  of the layer list. */
        if (offset == 0) break;
        /* save the current position as it is where the
         *  next layer offset is stored. */
        saved_pos = image->cp;
        /* seek to the layer offset */
        xcf_seek_pos(offset);
        /* read in the layer */
        layer = xcf_load_layer();
        if (!layer) goto error;
        num_successful_elements++;
        /* add the layer to the image if it's visible */
        if (layer->visible) add_layer_to_image(layer);
        else free_layer(layer);
        /* restore the saved position so we'll be ready to
         *  read the next offset. */
        xcf_seek_pos (saved_pos);
     }
   /* If we were a Gimp we would now load the user-defined channels here ... */
   /* Flat-o-rama now :) */
   flatten_image();
   return;
error:
   if (num_successful_elements == 0) goto hard_error;
   fprintf(stderr, "XCF: This file is corrupt!  I have loaded as much\nof it as I can, but it is incomplete.\n");
   return;
hard_error:
   fprintf(stderr, "XCF: This file is corrupt!  I could not even\nsalvage any partial image data from it.\n");
   return;
}

static char
xcf_load_layer_props(Layer *layer)
{
   PropType prop_type;
   DATA32 prop_size;

   while (1)
     {
        if (!xcf_load_prop(&prop_type, &prop_size)) return 0;
        switch (prop_type)
          {
           case PROP_END:
               {
                  D("Finished reading layer properties.\n");
                  return 1;
               }
           case PROP_FLOATING_SELECTION:
             D("Loading floating selection.\n");
             image->floating_sel = layer;
             image->cp += xcf_read_int32(image->file, (DATA32 *)&image->floating_sel_offset, 1);
             break;
           case PROP_OPACITY:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->opacity, 1);
             break;
           case PROP_VISIBLE:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->visible, 1);
             break;
           case PROP_PRESERVE_TRANSPARENCY:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->preserve_trans, 1);
             break;
           case PROP_OFFSETS:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->offset_x, 1);
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->offset_y, 1);
             break;
           case PROP_MODE:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->mode, 1);
             break;

             /* I threw out all of the following: --cK */
           case PROP_LINKED:
           case PROP_ACTIVE_LAYER:
           case PROP_TATTOO:
           case PROP_APPLY_MASK:
           case PROP_EDIT_MASK:
           case PROP_SHOW_MASK:
           case PROP_PARASITES:
           default:
               {
                  DATA8 buf[16];
                  int amount;

                  D("Skipping unexpected/unknown/unneeded channel property: %d\n", prop_type);
                  while (prop_size > 0)
                    {
                       amount = (16 < prop_size ? 16 : prop_size);
                       image->cp += xcf_read_int8 (image->file, buf, amount);
                       prop_size -= amount;
                    }
               }
             break;
          }
     }

   return 0;
}


static Layer *
xcf_load_layer(void)
{
   Layer  *layer;
   Layer  *layer_mask;
   DATA32  hierarchy_offset;
   DATA32  layer_mask_offset;
   int     width;
   int     height;
   int     type;
   char   *name;

   D("Loading one layer ...\n");
   /* read in the layer width, height and type */
   image->cp += xcf_read_int32(image->file, (DATA32 *)&width, 1);
   image->cp += xcf_read_int32(image->file, (DATA32 *)&height, 1);
   image->cp += xcf_read_int32(image->file, (DATA32 *)&type, 1);
   image->cp += xcf_read_string(image->file, &name, 1);
   /* ugly, I know */
   FREE(name);

   /* create a new layer */
   layer = new_layer(width, height, type, 255, NORMAL_MODE);
   if (!layer) return NULL;

   /* read in the layer properties */
   if (!xcf_load_layer_props(layer)) goto error;

   D("Loading opacity: %i \n", layer->opacity);
   if (!layer->visible) return layer;

   /* read the hierarchy and layer mask offsets */
   image->cp += xcf_read_int32(image->file, &hierarchy_offset, 1);
   image->cp += xcf_read_int32(image->file, &layer_mask_offset, 1);
   /* read in the hierarchy */
   xcf_seek_pos(hierarchy_offset);
   if (!xcf_load_hierarchy(&(layer->tiles), &(layer->num_rows),
                           &(layer->num_cols), &(layer->bpp)))
      goto error;

   /* read in the layer mask */
   if (layer_mask_offset != 0)
     {
        D("Loading layer mask.\n");
        xcf_seek_pos(layer_mask_offset);

        layer_mask = xcf_load_channel();
        if (!layer_mask) goto error;

        /* set the offsets of the layer_mask */
        layer_mask->offset_x = layer->offset_x;
        layer_mask->offset_y = layer->offset_y;
        layer->mask = layer_mask;
     }
   read_tiles_into_data(layer->tiles, layer->num_cols,
                        layer->width, layer->height,
                        layer->bpp, &(layer->data),
                        1);
   free_tiles(layer->tiles, layer->num_rows * layer->num_cols);
   layer->tiles = NULL;
   set_layer_opacity(layer);
   if (layer->mask) apply_layer_mask(layer);

   return layer;

error:
   free_layer(layer);
   return NULL;
}


static void
read_tiles_into_data(Tile *tiles, int num_cols, int width,
		     int height, int bpp, DATA8 **data_p, int use_cmap)
{
   int tile_x, tile_y, x, y, offset_x, offset_y;
   DATA8 *data;
   DATA8 *ptr;
   DATA8 *ptr2;
   Tile *t;
   int warned = 0;

   if (tiles)
     {
        if (*data_p) FREE(*data_p);

        /* Always allocate the data as 4 bytes per pixel */
        data = (*data_p) = malloc(sizeof(DATA32) * width * height);
        if (!data) return;

        ptr = data;
        for (y = 0; y < height; y++)
          {
             for (x = 0; x < width; x++)
               {
                  tile_x = x / TILE_WIDTH;
                  tile_y = y / TILE_HEIGHT;
                  offset_x = x % TILE_WIDTH;
                  offset_y = y % TILE_HEIGHT;

                  t = &tiles[(tile_y * num_cols) + tile_x];
                  ptr2 = &(t->data[(offset_y * t->ewidth * bpp) +
                                   (offset_x * bpp)]);
                  switch (bpp)
                    {
                     case 1:
                         {
                            /* use colormap if the image has one */
                            if (image->cmap && use_cmap)
                              {
                                 R_VAL(ptr) = image->cmap[(*(ptr2) * 3)];
                                 G_VAL(ptr) = image->cmap[(*(ptr2) * 3) + 1];
                                 B_VAL(ptr) = image->cmap[(*(ptr2) * 3) + 2];
                                 A_VAL(ptr) = 255;
                              }
                            /* else use colors themselves */
                            else
                              {
                                 R_VAL(ptr) = *(ptr2);
                                 G_VAL(ptr) = *(ptr2);
                                 B_VAL(ptr) = *(ptr2);
                                 A_VAL(ptr) = 255;
                              }
                            break;
                         }
                     case 2:
                         {
                            /* use colormap if the image has one */
                            if (image->cmap && use_cmap)
                              {
                                 R_VAL(ptr) = image->cmap[(*(ptr2) * 3)];
                                 G_VAL(ptr) = image->cmap[(*(ptr2) * 3) + 1];
                                 B_VAL(ptr) = image->cmap[(*(ptr2) * 3) + 2];
                                 A_VAL(ptr) = *(ptr2 + 1);
                              }
                            /* else use colors themselves */
                            else if (warned == 0)
                              {
                                 warned++;
                                 fprintf (stderr, "There's nothing to see here. 2 bpp without colormap not implemented yet.\n");
                              }
                            break;
                         }
                     case 3:
                         {
                            if (image->cmap)
                              {
                                 if (warned == 0)
                                   {
                                      warned++;
                                      fprintf (stderr, "There's nothing to see here. 3 bpp with colormap not implemented yet.\n");
                                   }
                              }
                            else
                              {
                                 R_VAL(ptr) = *(ptr2);
                                 G_VAL(ptr) = *(ptr2 + 1);
                                 B_VAL(ptr) = *(ptr2 + 2);
                                 A_VAL(ptr) = 255;
                              }
                            break;
                         }
                     default:
                         {
                            R_VAL(ptr) = *(ptr2);
                            G_VAL(ptr) = *(ptr2 + 1);
                            B_VAL(ptr) = *(ptr2 + 2);
                            A_VAL(ptr) = *(ptr2 + 3);
                            break;
                         }
                    }
                  ptr += 4;
               }
          }
     }
}

static Layer *
xcf_load_channel(void)
{
   Layer *layer;
   DATA32 hierarchy_offset;
   int width;
   int height;
   char *name;

   D("Loading channel ...\n");
   /* read in the layer width, height and name */
   image->cp += xcf_read_int32(image->file, (DATA32 *)&width, 1);
   image->cp += xcf_read_int32(image->file, (DATA32 *)&height, 1);
   image->cp += xcf_read_string(image->file, &name, 1);

   /* Yeah, still ugly :) */
   FREE(name);

   /* create a new channel */
   layer = new_layer(width, height, GRAY_GIMAGE, 255, NORMAL_MODE);
   if (!layer) return NULL;
   /* read in the channel properties */
   if (!xcf_load_channel_props(layer)) goto error;
   /* read the hierarchy and layer mask offsets */
   image->cp += xcf_read_int32(image->file, &hierarchy_offset, 1);
   /* read in the hierarchy */
   xcf_seek_pos(hierarchy_offset);
   if (!xcf_load_hierarchy(&(layer->tiles), &(layer->num_rows), &(layer->num_cols), &(layer->bpp)))
      goto error;
   read_tiles_into_data(layer->tiles, layer->num_cols, layer->width,
                        layer->height, layer->bpp, &(layer->data), 0);
   free_tiles(layer->tiles, layer->num_rows * layer->num_cols);
   layer->tiles = NULL;

   D("Channel loaded successfully.\n");
   return layer;

error:
   free_layer(layer);
   return NULL;
}

static char
xcf_load_channel_props(Layer *layer)
{
   PropType prop_type;
   DATA32 prop_size;

   while (1)
     {
        if (!xcf_load_prop(&prop_type, &prop_size)) return 0;

        switch (prop_type)
          {
           case PROP_END:
               {
                  D("Finished loading channel props.\n");
                  return 1;
               }
           case PROP_OPACITY:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->opacity, 1);
             break;
           case PROP_VISIBLE:
             image->cp += xcf_read_int32(image->file, (DATA32 *)&layer->visible, 1);
             break;
           case PROP_ACTIVE_CHANNEL:
           case PROP_SHOW_MASKED:
           case PROP_SELECTION:
           case PROP_COLOR:
           case PROP_TATTOO:
           case PROP_PARASITES:
           default:
               {
                  DATA8 buf[16];
                  int amount;

                  D("Skipping unexpected/unknown/unneeded channel property: %d\n", prop_type);

                  while (prop_size > 0)
                    {
                       amount = (16 < prop_size ? 16 : prop_size);
                       image->cp += xcf_read_int8(image->file, buf, amount);
                       prop_size -= amount;
                    }
               }
             break;
          }
     }
   return 0;
}

static char
xcf_load_hierarchy(Tile **tiles, int *num_rows, int *num_cols, int *bpp)
{
   DATA32 saved_pos;
   DATA32 offset;
   DATA32 junk;
   int width;
   int height;

   image->cp += xcf_read_int32(image->file, (DATA32 *)&width, 1);
   image->cp += xcf_read_int32(image->file, (DATA32 *)&height, 1);
   image->cp += xcf_read_int32(image->file, (DATA32 *)bpp, 1);
   image->cp += xcf_read_int32(image->file, &offset, 1); /* top level */

   D("Loading hierarchy: width %i, height %i,  bpp %i\n", width, height, *bpp);

   /* discard offsets for layers below first, if any. */
   do
     {
        image->cp += xcf_read_int32(image->file, &junk, 1);
     }
   while (junk != 0);
   /* save the current position as it is where the
    *  next level offset is stored. */
   saved_pos = image->cp;
   /* seek to the level offset */
   xcf_seek_pos(offset);
   /* read in the level */
   if (!xcf_load_level(tiles, width, height, *bpp, num_rows, num_cols))
      return 0;
   /* restore the saved position so we'll be ready to
    *  read the next offset. */
   xcf_seek_pos (saved_pos);
   D("Loaded hierarchy successfully.\n");
   return 1;
}

static char
xcf_load_level(Tile **tiles_p, int hierarchy_width, int hierarchy_height,
               int bpp, int *num_rows, int *num_cols)
{
   DATA32 saved_pos;
   DATA32 offset, offset2;
   int ntiles;
   int width;
   int height;
   int i;
   int fail;
   Tile *tiles;
   Tile *current_tile;

   image->cp += xcf_read_int32(image->file, (DATA32*) &width, 1);
   image->cp += xcf_read_int32(image->file, (DATA32*) &height, 1);

   if ((width != hierarchy_width) || (height != hierarchy_height)) return 0;

   D("Loading level of size %ix%i.\n", width, height);
   (*tiles_p) = allocate_tiles(width, height, bpp, num_rows, num_cols);
   tiles = (*tiles_p);

   image->cp += xcf_read_int32(image->file, &offset, 1);
   if (offset == 0) return 1;

   ntiles = (*num_rows) * (*num_cols);
   for (i = 0; i < ntiles; i++)
     {
        current_tile = &(tiles[i]);
        fail = 0;

        if (offset == 0)
          {
             D("Not enough tiles found in level\n");
             return 0;
          }

        /* save the current position as it is where the
         *  next tile offset is stored.
         */
        saved_pos = image->cp;

        /* read in the offset of the next tile so we can calculate the amount
	 of data needed for this tile*/
        image->cp += xcf_read_int32(image->file, &offset2, 1);

        /* if the offset is 0 then we need to read in the maximum possible
	 allowing for negative compression */
        if (offset2 == 0)
           offset2 = offset + (TILE_WIDTH * TILE_WIDTH * 4 * 1.5);
        /* 1.5 is probably more than we need to allow */

        /* seek to the tile offset */
        xcf_seek_pos(offset);

        /* read in the current_tile */
        switch (image->compression)
          {
           case COMPRESS_NONE:
             if (!xcf_load_tile(current_tile)) fail = 1;
             break;
           case COMPRESS_RLE:
             if (!xcf_load_tile_rle(current_tile, offset2 - offset)) fail = 1;
             break;
           case COMPRESS_ZLIB:
             fprintf (stderr, "xcf: zlib compression unimplemented\n");
             fail = 1;
             break;
           case COMPRESS_FRACTAL:
             fprintf (stderr, "xcf: fractal compression unimplemented\n");
             fail = 1;
             break;
          }

        if (fail)
          {
             D("Couldn't load tiles.\n");
             free_tiles(tiles, (*num_rows) * (*num_cols));
             return 0;
          }
        /* restore the saved position so we'll be ready to
         *  read the next offset.
         */
        xcf_seek_pos(saved_pos);
        /* read in the offset of the next tile */
        image->cp += xcf_read_int32(image->file, &offset, 1);
     }

   if (offset != 0)
     {
        D("encountered garbage after reading level: %d\n", offset);
        return 0;
     }

   D("Loaded level successfully.\n");
   return 1;
}

static char
xcf_load_tile(Tile *tile)
{
   image->cp += xcf_read_int8(image->file, tile->data,
                              tile->ewidth * tile->eheight * tile->bpp);
   return 1;
}

static char
xcf_load_tile_rle(Tile    *tile,
                  int     data_length)
{
   DATA8 *data;
   DATA8 val;
   int size;
   int count;
   int length;
   int bpp;
   int i, j;
   int nmemb_read_successfully;
   DATA8 *xcfdata, *xcfodata, *xcfdatalimit;

   data = tile->data;
   bpp = tile->bpp;

   /*printf ("Reading encrypted tile %ix%ix%i, data_length %i\n", tile->ewidth, tile->eheight, tile->bpp, data_length);*/

   xcfdata = xcfodata = malloc(sizeof(DATA8) * data_length);
   if (!xcfdata) return 0;

   /* we have to use fread instead of xcf_read_* because we may be
    reading past the end of the file here */
   nmemb_read_successfully = f_read(image->file, xcfdata, data_length);
   image->cp += nmemb_read_successfully;

   xcfdatalimit = &xcfodata[nmemb_read_successfully - 1];

   for (i = 0; i < bpp; i++)
     {
        data = (tile->data) + i;
        size = tile->ewidth * tile->eheight;
        count = 0;

        while (size > 0)
          {
             if (xcfdata > xcfdatalimit) goto bogus_rle;

             val = *xcfdata++;

             length = val;
             if (length >= 128)
               {
                  length = 255 - (length - 1);
                  if (length == 128)
                    {
                       if (xcfdata >= xcfdatalimit) goto bogus_rle;

                       length = (*xcfdata << 8) + xcfdata[1];
                       xcfdata += 2;
                    }

                  count += length;
                  size -= length;

                  if (size < 0) goto bogus_rle;
                  if (&xcfdata[length-1] > xcfdatalimit) goto bogus_rle;

                  while (length-- > 0)
                    {
                       *data = *xcfdata++;
                       data += bpp;
                    }
               }
             else
               {
                  length += 1;
                  if (length == 128)
                    {
                       if (xcfdata >= xcfdatalimit) goto bogus_rle;

                       length = (*xcfdata << 8) + xcfdata[1];
                       xcfdata += 2;
                    }

                  count += length;
                  size -= length;

                  if (size < 0) goto bogus_rle;
                  if (xcfdata > xcfdatalimit) goto bogus_rle;

                  val = *xcfdata++;

                  for (j = 0; j < length; j++)
                    {
                       *data = val;
                       data += bpp;
                    }
               }
          }
     }
   FREE(xcfodata);
   return 1;

bogus_rle:
   fprintf(stderr, "WHOOOOOP -- bogus rle? Highly unlikely, blame cK for this one :) \n");
   if (xcfodata) FREE(xcfodata);
   return 0;
}

static Layer *
new_layer(int width, int height, GimpImageType type, int opacity, LayerModeEffects mode)
{
   Layer *layer;

   layer = calloc(1, sizeof(Layer));
   if (!layer)
     {
        D("Couldn't allocate layer.\n");
        return NULL;
     }

   layer->width = width;
   layer->height = height;
   layer->type = type;
   layer->opacity = opacity;
   layer->mode = mode;
   layer->tiles = NULL;
   layer->next = NULL;
   layer->mask = NULL;
   return layer;
}

static void
free_layer(Layer *layer)
{
   if (layer)
     {
        if (layer->tiles)
           free_tiles(layer->tiles, layer->num_rows * layer->num_cols);
        if (layer->mask) free_layer(layer->mask);
        if (layer->data) FREE(layer->data);
        FREE(layer);
     }
}

static Tile *
allocate_tiles(int width, int height, int bpp, int* num_rows, int* num_cols)
{
   Tile* tiles;
   int i, j, k, right_tile, bottom_tile;
   int tile_width, tile_height;

   (*num_rows) = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
   (*num_cols) = (width + TILE_WIDTH - 1) / TILE_WIDTH;

   tiles = malloc(sizeof(Tile) * (*num_rows) * (*num_cols));
   if (!tiles)
     {
        D("Couldn't allocate tiles.\n");
        return NULL;
     }

   right_tile = width - (((*num_cols) - 1) * TILE_WIDTH);
   bottom_tile = height - (((*num_rows) - 1) * TILE_HEIGHT);

   for (i = 0, k = 0; i < (*num_rows); i++)
     {
        for (j = 0; j < (*num_cols); j++, k++)
          {
             tile_width = ((j == (*num_cols) - 1) ? right_tile : TILE_WIDTH);
             tile_height = ((i == (*num_rows) - 1) ? bottom_tile : TILE_HEIGHT);
             init_tile(&(tiles[k]), tile_width, tile_height, bpp);
          }
     }
   D("Allocated %ix%i tiles.\n", (*num_cols), (*num_rows));
  return tiles;
}

static void
init_tile(Tile *tile, int width, int height, int bpp)
{
   if (tile)
     {
        tile->bpp = bpp;
        tile->ewidth = width;
        tile->eheight = height;
        tile->data = malloc(sizeof(DATA8) * width * height * bpp);
        if (!tile->data)
          {
             D("Couldn't allocate tile.\n");
          }
     }
}

static void
free_tiles(Tile *tiles, int num_tiles)
{
   int i;

   for (i = 0; i < num_tiles; i++)
     {
        if (tiles[i].data) FREE(tiles[i].data);
     }
   FREE(tiles);
}

static void
add_layer_to_image(Layer *layer)
{
   if (layer)
     {
        if (image->last_layer)
          {
             image->last_layer->next = layer;
             layer->prev = image->last_layer;
          }
        else
          {
             image->layers = layer;
             layer->prev = NULL;
          }
        layer->next = NULL;
        image->last_layer = layer;
     }
}

static void
set_layer_opacity(Layer *layer)
{
   int i;
   DATA8* ptr;

   if (layer)
     {
        if (layer->opacity != 255)
          {
             for (i = 0, ptr = layer->data; i < layer->width * layer->height; i++, ptr += 4)
               {
                  *(ptr + 3) = (*(ptr + 3) * layer->opacity) >> 8;
               }
          }
     }
}

static void
apply_layer_mask(Layer *layer)
{
   DATA8* ptr1;
   DATA8* ptr2;
   int i, tmp;

   D("Applying layer mask.\n");
   if (layer)
     {
        if (layer->mask)
          {
             ptr1 = layer->data;
             ptr2 = layer->mask->data;
             for (i = 0; i < layer->width * layer->height; i++)
               {
                  tmp = (*(ptr1 + 3) * *(ptr2)) / 256;
                  if (tmp > 255) tmp = 255;
                  *(ptr1 + 3) = (DATA8)tmp;
                  ptr1 += 4;
                  ptr2 += 4;
               }
          }
     }
}

static void
flatten_image(void)
{
   Layer* l = image->last_layer;
   Layer* lp;
   int  layer_index;

   shm_alloc(image->width * image->height * sizeof(DATA32));
   if (!shm_addr) return;
   image->data = shm_addr;
   memset(image->data, 0, image->width * image->height * sizeof(DATA32));

   layer_index = 0;

   while (l)
     {
        /* Ok, paste each layer on top of the image, using the mode's merging type.
	 We're moving upward through the layer stack.
         --cK.
         */
        if ((image->single_layer_index < 0) ||
            (layer_index == image->single_layer_index))
          {
             // FIXME: not all modes are implemented right
             // xcf's may not render right :)
             switch (l->mode)
               {
                case MULTIPLY_MODE:
                  D("MULTIPLY\n");
                  combine_pixels_mult(l->data, l->width, l->height,
                                      image->data, image->width, image->height,
                                      l->offset_x, l->offset_y);
                  break;
                case DIVIDE_MODE:
                  D("DIVIDE\n");
                  combine_pixels_div(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case SCREEN_MODE:
                  D("SCREEN\n");
                  combine_pixels_screen(l->data, l->width, l->height,
                                        image->data, image->width, image->height,
                                        l->offset_x, l->offset_y);
                  break;
                case OVERLAY_MODE:
                  D("OVERLAY\n");
                  combine_pixels_overlay(l->data, l->width, l->height,
                                         image->data, image->width, image->height,
                                         l->offset_x, l->offset_y);
                  break;
                case DIFFERENCE_MODE:
                  D("DIFF\n");
                  combine_pixels_diff(l->data, l->width, l->height,
                                      image->data, image->width, image->height,
                                      l->offset_x, l->offset_y);
                  break;
                case ADDITION_MODE:
                  D("ADD\n");
                  combine_pixels_add(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case SUBTRACT_MODE:
                  D("SUB\n");
                  combine_pixels_sub(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case DARKEN_ONLY_MODE:
                  D("DARKEN\n");
                  combine_pixels_darken(l->data, l->width, l->height,
                                        image->data, image->width, image->height,
                                        l->offset_x, l->offset_y);
                  break;
                case LIGHTEN_ONLY_MODE:
                  D("LIGHTEN\n");
                  combine_pixels_lighten(l->data, l->width, l->height,
                                         image->data, image->width, image->height,
                                         l->offset_x, l->offset_y);
                  break;
                case HUE_MODE:
                  D("HUE\n");
                  combine_pixels_hue(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case SATURATION_MODE:
                  D("SATURATION\n");
                  combine_pixels_sat(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case VALUE_MODE:
                  D("VALUE\n");
                  combine_pixels_val(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case COLOR_MODE:
                  D("COLOR\n");
                  combine_pixels_col(l->data, l->width, l->height,
                                     image->data, image->width, image->height,
                                     l->offset_x, l->offset_y);
                  break;
                case DISSOLVE_MODE:
                  D("DISSOLVE\n");
                  combine_pixels_diss(l->data, l->width, l->height,
                                      image->data, image->width, image->height,
                                      l->offset_x, l->offset_y);
                  break;
                  /* None of the following is actually valid for layer blending, fall through: */
                case BEHIND_MODE:
                case REPLACE_MODE:
                case ERASE_MODE:
                case ANTI_ERASE_MODE:
                  D("EEEEEK -- this mode shouldn't be here\n");
                  /* Fall through */

                case NORMAL_MODE:
                  D("NORMAL\n");
                  combine_pixels_normal(l->data, l->width, l->height,
                                        image->data, image->width, image->height,
                                        l->offset_x, l->offset_y);
                  break;

                default:
                  D("Unknown layer mode: %i. Skipping.\n", l->mode);
               }
          }

        lp = l->prev;
        /* free the layer now, since it's not needed anymore */
        free_layer(l);

        l = lp;
        layer_index++;
     }

   /* We've used up all the layers now, so set them to NULL in the image: */
   image->layers = NULL;
   image->last_layer = NULL;
}

static char
xcf_file_init(char *filename)
{
   char success = 1;
   char id[14];
   int width;
   int height;
   int image_type;

   image->single_layer_index = -1;
   image->file = f_open(filename);
   D("image->file = %p\n", image->file);
   if (!image->file) return 0;

   image->filename = filename;
   image->layers = NULL;
   image->last_layer = NULL;
   image->cmap = NULL;
   image->num_cols = 0;
   image->data = NULL;

   image->cp = 0;

   image->cp += xcf_read_int8(image->file, (DATA8 *)id, 14);
   if (strncmp(id, "gimp xcf ", 9))
     {
        success = 0;
        f_close(image->file);
     }
   else if (!strcmp(id + 9, "file"))
     {
        image->file_version = 0;
     }
   else if (id[9] == 'v')
     {
        image->file_version = atoi(id + 10);
     }
   else
     {
        success = 0;
        f_close(image->file);
     }

   if (success)
     {
        image->cp += xcf_read_int32(image->file, (DATA32 *)&width, 1);
        image->cp += xcf_read_int32(image->file, (DATA32 *)&height, 1);
        image->cp += xcf_read_int32(image->file, (DATA32 *)&image_type, 1);

        image->width = width;
        image->height = height;
        image->base_type = image_type;

        D("Loading %ix%i image.\n", width, height);
     }

   return success;
}

static void
xcf_cleanup(void)
{
   Layer *l, *lp;

   if (image->file) f_close(image->file);
   for (l = image->last_layer; l; l = lp)
     {
        lp = l->prev;
        free_layer(l);
     }
   if (image->cmap) FREE(image->cmap);
}

static void
premul_image(void)
{
   DATA32 *p, *end;

   end =  (DATA32 *)image->data + (image->width * image->height);
   for (p = (DATA32 *)image->data; p < end; p++)
     {
        unsigned int r, g, b, a;

        a = A_VAL(p);
        r = (R_VAL(p) * a) / 255;
        R_VAL(p) = r;
        g = (G_VAL(p) * a) / 255;
        G_VAL(p) = g;
        b = (B_VAL(p) * a) / 255;
        B_VAL(p) = b;
     }
}

int
main(int argc, char **argv)
{
   char *file;
   int w, h, i;
   int head_only = 0;

   if (argc < 2) return -1;
   // file is ALWAYS first arg, other options come after
   file = argv[1];
   for (i = 2; i < argc; i++)
     {
        if      (!strcmp(argv[i], "-head"))
           // asked to only load header, not body/data
           head_only = 1;
        else if (!strcmp(argv[i], "-key"))
          { // not used by xcf loader
             i++;
             // const char *key = argv[i];
          }
        else if (!strcmp(argv[i], "-opt-scale-down-by"))
          { // not used by xcf loader
             i++;
             // int scale_down = atoi(argv[i]);
          }
        else if (!strcmp(argv[i], "-opt-dpi"))
          { // not used by xcf loader
             i++;
             // double dpi = ((double)atoi(argv[i])) / 1000.0;
          }
        else if (!strcmp(argv[i], "-opt-size"))
          { // not used by xcf loader
             i++;
             // int size_w = atoi(argv[i]);
             i++;
             // int size_h = atoi(argv[i]);
          }
     }
   
   timeout_init(8);
   
   D("xcf_file_init\n");
   if (!xcf_file_init(file)) return -1;
   
   D("size %i %i\n", image->width, image->height);
   if (!head_only)
     {
        xcf_load_image();
        premul_image();
     }
   w = image->width;
   h = image->height;
   printf("size %i %i\n", w, h);
   printf("alpha 1\n");
   if (!head_only)
     {
        if (shm_fd >= 0) printf("shmfile %s\n", shmfile);
        else
          {
             // could also to "tmpfile %s\n" like shmfile but just
             // a mmaped tmp file on the system
             printf("data\n");
             if (fwrite(image->data, w * h * sizeof(DATA32), 1, stdout) != 1)
             {
                // nothing much to do, the receiver will simply ignore the
                // data as it's too short
                D("fwrite failed (%d bytes): %m\n", w * h * sizeof(DATA32));
             }
          }
        shm_free();
     }
   else
      printf("done\n");
   xcf_cleanup();
   fflush(stdout);
   return 0;
}
