#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <sys/ioctl.h>

#include <Eina.h>
#include "evas_cs2.h"

#define SHM_FOLDER "/dev/shm"
static int _evas_cserve2_shm_debug_log_dom = -1;

#define ERR(...) EINA_LOG_DOM_ERR(_evas_cserve2_shm_debug_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_evas_cserve2_shm_debug_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_evas_cserve2_shm_debug_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_evas_cserve2_shm_debug_log_dom, __VA_ARGS__)

typedef struct _Shm_File Shm_File;
struct _Shm_File
{
   Eina_File *f;
   char *data;
   size_t size;
   int tag;

   Eina_Bool strings;
   Shared_Array_Header *header;
};

Shm_File *sf_strindex = NULL;
Shm_File *sf_strpool = NULL;
Shm_File *sf_fonts = NULL;
Shm_File *sf_files = NULL;
Shm_File *sf_images = NULL;

static int _termsize = 0;

static void
nprintf(int n, const char *fmt, ...)
{
   char buf[n+1];
   va_list arg;
   int len;

   va_start(arg, fmt);
   if (!n)
     vprintf(fmt, arg);
   else
     {
        len = vsnprintf(buf, n+1, fmt, arg);
        buf[n] = 0;
        fputs(buf, stdout);
        if ((len > 0) && (buf[len-1] != '\n'))
          fputc('\n', stdout);
     }
   va_end(arg);
}

#define printf(...) nprintf(_termsize, __VA_ARGS__)

static Shm_File *
_shm_file_open(const char *path)
{
   Shm_File *sf;
   Eina_File *f;
   char *data;

   f = eina_file_open(path, EINA_TRUE);
   if (!f)
     {
        ERR("Could not open file %s: [%d] %m", path, errno);
        return NULL;
     }

   data = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!data)
     {
        ERR("Could not map the file %s", path);
        eina_file_close(f);
        return NULL;
     }

   sf = calloc(1, sizeof(*sf));
   sf->data = data;
   sf->f = f;
   sf->size = eina_file_size_get(f);

   DBG("Successfully opened %s", path);
   return sf;
}

static void
_shm_file_close(Shm_File *sf)
{
   if (!sf) return;
   eina_file_map_free(sf->f, sf->data);
   eina_file_close(sf->f);
   free(sf);
}

static Eina_Bool
_shm_file_probe(Shm_File *sf)
{
   if (!sf || !sf->data)
     return EINA_FALSE;

   if (sf->size < sizeof(Shared_Array_Header))
     {
        sf->header = NULL;
        return EINA_TRUE;
     }

   sf->header = (Shared_Array_Header *) sf->data;
   switch (sf->header->tag)
     {
      case STRING_INDEX_ARRAY_TAG:
        DBG("Found string index: %s", eina_file_filename_get(sf->f));
        if (sf->header->elemsize != sizeof(Index_Entry))
          {
             ERR("String index looks invalid: elemsize %d",
                 sf->header->elemsize);
             return EINA_FALSE;
          }
        sf->tag = STRING_INDEX_ARRAY_TAG;
        sf_strindex = sf;
        break;

      case STRING_MEMPOOL_FAKETAG:
        DBG("Found string mempool: %s", eina_file_filename_get(sf->f));
        sf->strings = EINA_TRUE;
        sf->tag = STRING_MEMPOOL_FAKETAG;
        sf->header = NULL;
        sf_strpool = sf;
        break;

      case FILE_DATA_ARRAY_TAG:
        DBG("Found index table with tag '%4.4s'", (char *) &sf->header->tag);
        sf->tag = sf->header->tag;
        sf_files = sf;
        break;

      case IMAGE_DATA_ARRAY_TAG:
        DBG("Found index table with tag '%4.4s'", (char *) &sf->header->tag);
        sf->tag = sf->header->tag;
        sf_images = sf;
        break;

      case FONT_DATA_ARRAY_TAG:
        DBG("Found index table with tag '%4.4s'", (char *) &sf->header->tag);
        sf->tag = sf->header->tag;
        sf_fonts = sf;
        break;

      case GLYPH_DATA_ARRAY_TAG:
        DBG("Found index table with tag '%4.4s'", (char *) &sf->header->tag);
        sf->tag = sf->header->tag;
        break;

      default:
        //DBG("Unknown tag found: %d", sf->header->tag);
        sf->header = NULL;
        break;
     }

   return EINA_TRUE;
}

static Eina_List *
_shm_files_list(const char *folder)
{
   Eina_List *lst = NULL;
   Eina_Iterator *iter;
   const Eina_File_Direct_Info *f_info;
   char pattern[64];

   sprintf(pattern, "/evas-shm-%05d-", (int) getuid());
   iter = eina_file_direct_ls(folder);
   EINA_ITERATOR_FOREACH(iter, f_info)
     {
        if (strstr(f_info->path, pattern))
          {
             const char *shmname = strrchr(f_info->path, '/');
             if (!shmname) continue;

             lst = eina_list_append(lst, strdup(shmname));
          }
        else
          DBG("cserve2 scan: ignoring %s", f_info->path);
     }
   eina_iterator_free(iter);

   return lst;
}

static void
printf_newline(Eina_Bool flush)
{
   printf("----------------------------------------------------------------"
          "----------------\n");

   if (flush)
     {
        printf("\n\n");
        fflush(stdout);
     }
}

static void
_index_tables_summary_print(void)
{
   printf("Shared index tables\n\n");
   printf("Table     Tag MapSize  Gen ElSz Count  Last  Sort Path\n");
   printf_newline(0);

   printf("StrIdx   %4.4s %7d %4d %4d %5d %5d %5d %s\n",
          (char *) &sf_strindex->tag,
          (int) eina_file_size_get(sf_strindex->f),
          sf_strindex->header->generation_id, sf_strindex->header->elemsize,
          sf_strindex->header->count, sf_strindex->header->emptyidx,
          sf_strindex->header->sortedidx,
          eina_file_filename_get(sf_strindex->f));

   printf("StrData  %4.4s %7d                             %s\n",
          (char *) &sf_strpool->tag,
          (int) eina_file_size_get(sf_strpool->f),
          eina_file_filename_get(sf_strpool->f));

   if (sf_files)
     printf("FileIdx  %4.4s %7d %4d %4d %5d %5d %5d %s\n",
            (char *) &sf_files->tag,
            (int) eina_file_size_get(sf_files->f),
            sf_files->header->generation_id, sf_files->header->elemsize,
            sf_files->header->count, sf_files->header->emptyidx,
            sf_files->header->sortedidx,
            eina_file_filename_get(sf_files->f));
   else
     printf("FileIdx  <not found>\n");

   if (sf_images)
     printf("Images   %4.4s %7d %4d %4d %5d %5d %5d %s\n",
            (char *) &sf_images->tag,
            (int) eina_file_size_get(sf_images->f),
            sf_images->header->generation_id, sf_images->header->elemsize,
            sf_images->header->count, sf_images->header->emptyidx,
            sf_images->header->sortedidx,
            eina_file_filename_get(sf_images->f));
   else
     printf("Images   <not found>\n");

   if (sf_fonts)
     printf("FontIdx  %4.4s %7d %4d %4d %5d %5d %5d %s\n",
            (char *) &sf_fonts->tag,
            (int) eina_file_size_get(sf_fonts->f),
            sf_fonts->header->generation_id, sf_fonts->header->elemsize,
            sf_fonts->header->count, sf_fonts->header->emptyidx,
            sf_fonts->header->sortedidx,
            eina_file_filename_get(sf_fonts->f));
   else
     printf("FontIdx  <not found>\n");

   printf_newline(1);
}

static const Shm_Object *
_shared_index_item_get_by_id(Shm_File *si, int elemsize, unsigned int id)
{
   const Shm_Object *obj;
   const char *base;
   int low = 0, high, start_high;
   int cur;

   if (!si || elemsize <= 0 || !id)
     return NULL;

   // FIXME: HACK (consider all arrays always sorted by id)
   high = si->header->emptyidx; // Should be si->header->sortedidx

   if (high > si->header->count)
     high = si->header->count;

   base = si->data  + sizeof(Shared_Array_Header);

   // Direct access, works for non-repacked arrays
   if ((int) id < high)
     {
        obj = (Shm_Object *) (base + (elemsize * id));
        if (obj->id == id)
          return obj;
     }

   // Binary search
   start_high = high;
   while(high != low)
     {
        cur = low + ((high - low) / 2);
        obj = (Shm_Object *) (base + (elemsize * cur));
        if (obj->id == id)
          return obj;
        if (obj->id < id)
          low = cur + 1;
        else
          high = cur;
     }

   // Linear search
   for (cur = start_high; cur < si->header->count; cur++)
     {
        obj = (Shm_Object *) (base + (elemsize * cur));
        if (!obj->id)
          return NULL;
        if (obj->id == id)
          return obj;
     }

   return NULL;
}

static void *
_shared_array_item_get(Shm_File *sf, int idx)
{
   if (!sf || !sf->header || idx < 0)
     return NULL;

   if ((idx + 1) * sf->header->elemsize > (int) sf->size)
     return NULL;

   return (sf->data + sizeof(Shared_Array_Header) + idx * sf->header->elemsize);
}

static const char *
_shared_string_get(int id)
{
   Index_Entry *ie;

   ie = (Index_Entry *)
         _shared_index_item_get_by_id(sf_strindex, sizeof(*ie), id);
   if (!ie) return NULL;
   if (ie->offset < 0) return NULL;
   if (!ie->refcount) return NULL;
   if (ie->offset + ie->length > (int) sf_strpool->size) return NULL;

   return sf_strpool->data + ie->offset;
}

static void
_strings_all_print(Eina_Bool full)
{
   int k;
   const char *mindata = sf_strpool->data;
   const char *maxdata = sf_strpool->data + sf_strpool->size;

   printf("List of strings\n");
   printf("Indexes: %s\n", eina_file_filename_get(sf_strindex->f));
   printf("Data:    %s\n", eina_file_filename_get(sf_strpool->f));
   printf(" String   BufID  Refcnt  Offset  Buflen Content\n");
   printf_newline(0);

   for (k = 0; k < sf_strindex->header->count; k++)
     {
        Index_Entry *ie;
        const char *data;

        ie = _shared_array_item_get(sf_strindex, k);
        if (!ie) break;

        if (!ie->id || (!full && !ie->refcount))
          continue;

        data = sf_strpool->data + ie->offset;
        if ((data < mindata) || (data + ie->length > maxdata))
          data = "<invalid offset>";

        printf("%7d %7d %7d %7d %7d '%s'\n",
               k, ie->id, ie->refcount, ie->offset, ie->length, data);
     }

   printf_newline(1);
}

static void
_files_all_print_short()
{
   int k;

   if (!sf_files) return;

   printf("List of image files: %s\n", eina_file_filename_get(sf_files->f));
   printf("A: Alpha, I: Invalid.\n\n");
   printf("  Index  FileID WIDTHxHEIGHT A I Loader PathID  KeyID Path:Key\n");
   printf_newline(0);

   for (k = 0; k < sf_files->header->count; k++)
     {
        File_Data *fd;

        fd = _shared_array_item_get(sf_files, k);
        if (!fd) break;
        if (!fd->id || !fd->refcount) continue;

        printf("%7d %7d %5dx%-6d %d %d %6.6s %6d %6d '%s':'%s'\n",
               k, fd->id, fd->w, fd->h, !!fd->alpha, !!fd->changed,
               fd->loader_data ? _shared_string_get(fd->loader_data) : "",
               fd->path, fd->key, _shared_string_get(fd->path),
               _shared_string_get(fd->key));
     }

   printf_newline(1);
}

static void
_files_all_print_all(void)
{
   int k;

   if (!sf_files) return;

   printf("List of opened image files: %s\n", eina_file_filename_get(sf_files->f));
   printf_newline(0);

   for (k = 0; k < sf_files->header->count; k++)
     {
        File_Data *fd;

        fd = _shared_array_item_get(sf_files, k);
        if (!fd) break;
        if (!fd->id) continue;

        printf("File #%-8d %d\n", k, fd->id);
        printf("Path:Key:      '%s':'%s'\n",
               _shared_string_get(fd->path), _shared_string_get(fd->key));
        printf("LoadOpts:      Region:      %d,%d-%dx%d\n",
               fd->lo.region.x, fd->lo.region.y, fd->lo.region.w, fd->lo.region.h);
        if (fd->lo.dpi != 0)
          printf("               DPI:         %f\n");
        else
          printf("               DPI:         0\n");
        printf("               Requested:   %dx%d\n", fd->lo.w, fd->lo.h);
        printf("               Scale down:  %d\n", fd->lo.scale_down_by);
        printf("               Orientation: %s\n", fd->lo.orientation ? "YES" : "NO");
        printf("Loader:        %s\n", _shared_string_get(fd->loader_data));
        printf("Geometry:      %dx%d\n", fd->w, fd->h);
        printf("Animation:     anim: %s, frames: %d, loop: %d, hint: %d\n",
               fd->animated ? "YES" : "NO",
               fd->frame_count, fd->loop_count, fd->loop_hint);
        printf("Alpha:         %s\n", fd->alpha ? "YES" : "NO");
        printf("Changed:       %s\n", fd->changed ? "YES" : "NO");

        printf_newline(0);
     }

   printf("\n\n");
   fflush(stdout);
}

static void
_images_all_print_short(void)
{
   int k;

   if (!sf_images) return;

   printf("List of loaded images: %s\n", eina_file_filename_get(sf_images->f));
   printf("A: Sparse alpha, U: Unused, L: Load requested.\n\n");
   printf("  Index ImageID  FileID  Refcnt A U L   ShmID ShmPath\n");
   printf_newline(0);

   for (k = 0; k < sf_images->header->count; k++)
     {
        Image_Data *id;

        id = _shared_array_item_get(sf_images, k);
        if (!id) break;
        if (!id->id || !id->refcount) continue;

        printf("%7d %7d %7d %7d %d %d %d %7d '%s'\n",
               k, id->id, id->file_id, id->refcount,
               !!id->alpha_sparse, !!id->unused, !!id->doload, id->shm_id,
               id->shm_id ? _shared_string_get(id->shm_id) : "");
     }

   printf_newline(1);
}

static void
_images_all_print_full(void)
{
   int k;

   if (!sf_images) return;

   printf("List of loaded images: %s\n\n",
          eina_file_filename_get(sf_images->f));
   printf_newline(0);

   for (k = 0; k < sf_images->header->count; k++)
     {
        Image_Data *id;
        File_Data *fd;
        const char *scale_hint;

        id = _shared_array_item_get(sf_images, k);
        if (!id) break;
        if (!id->id) continue;

        printf("Image #%-8d %d\n", k, id->id);
        printf("Refcount        %d\n", id->refcount);
        printf("Sparse alpha    %s\n"
               "Unused:         %s\n"
               "Load requested: %s\n"
               "Valid:          %s\n",
               id->alpha_sparse ? "YES" : "NO",
               id->unused ? "YES" : "NO",
               id->doload ? "YES" : "NO",
               id->valid ? "YES" : "NO");
        printf("Shm Path:       '%s'\n",
               id->shm_id ? _shared_string_get(id->shm_id) : "");
        printf("LoadOpts: width          %d\n", id->opts.w);
        printf("          height         %d\n", id->opts.h);
        printf("          degree         %d\n", id->opts.degree);
        printf("          scale_down_by  %d\n", id->opts.scale_down_by);
        if (id->opts.dpi)
          printf("          dpi            %.2f\n", id->opts.dpi);
        else
          printf("          dpi            0\n");
        printf("          orientation    %s\n", id->opts.orientation ? "YES" : "NO");
        printf("          region         (%d,%d) %dx%d\n",
               id->opts.region.x, id->opts.region.y,
               id->opts.region.w, id->opts.region.h);

        switch (id->opts.scale_load.scale_hint)
          {
           case EVAS_IMAGE_SCALE_HINT_NONE:
             scale_hint = "EVAS_IMAGE_SCALE_HINT_NONE"; break;
           case EVAS_IMAGE_SCALE_HINT_DYNAMIC:
             scale_hint = "EVAS_IMAGE_SCALE_HINT_DYNAMIC"; break;
           case EVAS_IMAGE_SCALE_HINT_STATIC:
             scale_hint = "EVAS_IMAGE_SCALE_HINT_STATIC"; break;
           default:
             scale_hint = "<invalid>"; break;
          }

        printf("          scale src      (%d,%d) %dx%d\n",
               id->opts.scale_load.src_x, id->opts.scale_load.src_y,
               id->opts.scale_load.src_w, id->opts.scale_load.src_h);
        printf("          scale dst      %dx%d\n",
               id->opts.scale_load.dst_w, id->opts.scale_load.dst_h);
        printf("          scale smooth   %s\n",
               id->opts.scale_load.smooth ? "YES" : "NO");
        printf("          scale hint     %s (%d)\n",
               scale_hint, id->opts.scale_load.scale_hint);

        fd = (File_Data *)
              _shared_index_item_get_by_id(sf_files, sizeof(*fd), id->file_id);
        if (fd)
          {
             printf("File:     ID             %d\n", id->file_id);
             printf("          Path:Key:      '%s':'%s'\n",
                    _shared_string_get(fd->path), _shared_string_get(fd->key));
             printf("          Loader:        %s\n",
                    _shared_string_get(fd->loader_data));
             printf("          Geometry:      %dx%d\n", fd->w, fd->h);
             printf("          Animation:     anim: %s, frames: %d, loop: %d, hint: %d\n",
                    fd->animated ? "YES" : "NO",
                    fd->frame_count, fd->loop_count, fd->loop_hint);
             printf("          Alpha:         %s\n", fd->alpha ? "YES" : "NO");
             printf("          Changed:       %s\n", fd->changed ? "YES" : "NO");
          }
        else
          printf("File: ID %d not found\n", id->file_id);

        printf_newline(0);
     }

   printf("\n\n");
   fflush(stdout);
}

static void
_fonts_all_print_short(void)
{
   int k;

   static const char *rend_flags[] = { "R", "S", "W", "SW", "X" };

   if (!sf_fonts) return;

   printf("List of loaded fonts: %s\n", eina_file_filename_get(sf_fonts->f));
   printf("Rendering flags (RF): "
          "R: Regular, S: Slanted, W: Weight, X: Invalid\n\n");
   printf(" Index FontID Refcnt  Size   DPI RF  GlIdx GlData File[:Name]\n");
   printf_newline(0);

   for (k = 0; k < sf_fonts->header->count; k++)
     {
        Font_Data *fd;
        int rflag;
        const char *rf;

        fd = _shared_array_item_get(sf_fonts, k);
        if (!fd) break;
        if (!fd->id || !fd->refcount) continue;

        rflag = fd->rend_flags;
        if (rflag < 0 || rflag > 3) rflag = 4;
        rf = rend_flags[rflag];

        if (fd->name)
          printf("%6d %6d %6d %5d %5d %-2s %6d %6d '%s':'%s'\n",
                 k, fd->id, fd->refcount, fd->size, fd->dpi, rf,
                 fd->glyph_index_shm, fd->mempool_shm,
                 fd->file ? _shared_string_get(fd->file) : "",
                 _shared_string_get(fd->name));
        else
           printf("%6d %6d %6d %5d %5d %-2s %6d %6d '%s'\n",
                  k, fd->id, fd->refcount, fd->size, fd->dpi, rf,
                  fd->glyph_index_shm, fd->mempool_shm,
                  fd->file ? _shared_string_get(fd->file) : "");
     }

   printf_newline(1);
}

static void
_glyphs_all_print(Shm_File *sf)
{
   int k;
   int nglyphs = 0;
   int mem_used = 0;

   printf("  GlyphID Refcnt  Index  Size  Rows Width Pitch Grays H M  "
          "BufID Offset ShmPath\n");

   for (k = 0; k < sf->header->count; k++)
     {
        Glyph_Data *gd;

        gd = _shared_array_item_get(sf, k);
        if (!gd) break;
        if (!gd->id) continue;

        printf(" %8u %6u %6u %5u %5u %5u %5u %5u %1u %1u %6u %6u '%s'\n",
               gd->id, gd->refcount, gd->index, gd->size, gd->rows, gd->width,
               gd->pitch, gd->num_grays, gd->hint, gd->pixel_mode, gd->buffer_id,
               gd->offset, _shared_string_get(gd->mempool_id));

        nglyphs++;
        mem_used += gd->size;
     }

   printf("Total %d glyph(s) loaded, using %d bytes (w/o padding)\n",
          nglyphs, mem_used);
}

static void
_fonts_all_print_full(void)
{
   int k;

   static const char *rend_flags[] = { "R", "S", "W", "SW", "X" };

   if (!sf_fonts) return;

   printf("List of loaded fonts: %s\n", eina_file_filename_get(sf_fonts->f));
   printf("Rendering flags: "
          "R: Regular, S: Slanted, W: Weight, X: Invalid\n");
   printf("H: Hint, M: Pixel mode\n\n");
   printf_newline(0);

   for (k = 0; k < sf_fonts->header->count; k++)
     {
        Shm_File *sf;
        Font_Data *fd;
        int rflag;
        const char *rf;

        fd = _shared_array_item_get(sf_fonts, k);
        if (!fd) break;
        if (!fd->id) continue;
        if (!fd->glyph_index_shm) continue;

        sf = _shm_file_open(_shared_string_get(fd->glyph_index_shm));
        if (!_shm_file_probe(sf))
          {
             _shm_file_close(sf);
             continue;
          }

        rflag = fd->rend_flags;
        if (rflag < 0 || rflag > 3) rflag = 4;
        rf = rend_flags[rflag];

        printf("Font #%-8d %d\n", k, fd->id);
        printf("Refcount       %d\n", fd->refcount);
        printf("File           '%s'\n", _shared_string_get(fd->file));
        printf("Name           '%s'\n",
               fd->name ? _shared_string_get(fd->name) : "");
        printf("Size           %d\n", fd->size);
        printf("DPI            %d\n", fd->dpi);
        printf("Rendering flag %s (%d)\n", rf, fd->rend_flags);
        printf("Glyph index    '%s'\n", _shared_string_get(fd->glyph_index_shm));
        printf("Glyph data     '%s'\n", _shared_string_get(fd->mempool_shm));

        _glyphs_all_print(sf);

        _shm_file_close(sf);
        printf_newline(0);
     }

   printf("\n\n");
   fflush(stdout);
}


int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_List *opened_sf = NULL;
   Eina_List *shmfiles = NULL, *l1, *l2;
   const char *shmfolder = SHM_FOLDER;
   Eina_Bool full = EINA_FALSE;
   Shm_File *sf;
   char *name;
   struct winsize w;

   if (isatty(STDOUT_FILENO))
     {
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
          _termsize = w.ws_col;
     }

   if (argc > 1)
     {
        if (!strcmp(argv[1], "-h"))
          {
             printf("Usage: %s [-f] [folder]\n", argv[0]);
             return 0;
          }
        else if (!strcmp(argv[1], "-f"))
          {
             full = EINA_TRUE;
             if (argc > 2)
               shmfolder = argv[2];
          }
        else
          shmfolder = argv[1];
     }

   eina_init();
   _evas_cserve2_shm_debug_log_dom = eina_log_domain_register
         ("evas_cserve2_shm_debug", EINA_COLOR_BLUE);

   shmfiles = _shm_files_list(shmfolder);
   if (!shmfiles)
     {
        WRN("No shm file was found in %s", shmfolder);
        goto finish;
     }

   DBG("Found %d files in %s", eina_list_count(shmfiles), shmfolder);

   // Look for strings index & mempool
   EINA_LIST_FOREACH_SAFE(shmfiles, l1, l2, name)
     {
        sf = _shm_file_open(name);
        if (!_shm_file_probe(sf))
          {
             shmfiles = eina_list_remove_list(shmfiles, l1);
             _shm_file_close(sf);
             free(name);
             continue;
          }

        if (!sf->tag)
          _shm_file_close(sf);
        else
          opened_sf = eina_list_append(opened_sf, sf);

        if (!!sf_strindex
            && !!sf_strpool
            && !!sf_fonts
            && !!sf_images
            && !!sf_files)
          break;
     }

   if (!sf_strindex || !sf_strpool)
     {
        ERR("Could not identify strings memory pool");
        goto finish;
     }

   _index_tables_summary_print();
   _strings_all_print(full);
   _files_all_print_short();
   if (full) _files_all_print_all();
   _images_all_print_short();
   if (full) _images_all_print_full();
   _fonts_all_print_short();
   if (full) _fonts_all_print_full();

finish:
   EINA_LIST_FREE(opened_sf, sf)
     _shm_file_close(sf);

   EINA_LIST_FREE(shmfiles, name)
     free(name);

   eina_shutdown();
   return 0;
}
