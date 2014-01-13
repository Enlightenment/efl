#ifndef EVAS_CS2_H
#define EVAS_CS2_H 1

#include <Eina.h>

#ifdef EVAS_CSERVE2

#include <Evas_Loader.h>

typedef enum {
   CSERVE2_OPEN = 1,
   CSERVE2_OPENED,
   CSERVE2_LOAD,
   CSERVE2_LOADED,
   CSERVE2_PRELOAD,
   CSERVE2_UNLOAD,
   CSERVE2_CLOSE,
   CSERVE2_FONT_LOAD,
   CSERVE2_FONT_LOADED,
   CSERVE2_FONT_UNLOAD,
   CSERVE2_FONT_GLYPHS_LOAD,
   CSERVE2_FONT_GLYPHS_LOADED,
   CSERVE2_FONT_GLYPHS_USED,
   CSERVE2_STATS,
   CSERVE2_FONT_DEBUG,
   CSERVE2_INDEX_LIST,
   CSERVE2_ERROR
} Message_Type;

typedef enum {
   CSERVE2_NONE,
   CSERVE2_GENERIC,
   CSERVE2_DOES_NOT_EXIST,
   CSERVE2_PERMISSION_DENIED,
   CSERVE2_RESOURCE_ALLOCATION_FAILED,
   CSERVE2_CORRUPT_FILE,
   CSERVE2_UNKNOWN_FORMAT,
   CSERVE2_INVALID_COMMAND,
   CSERVE2_LOADER_DIED,
   CSERVE2_LOADER_EXEC_ERR,
   CSERVE2_INVALID_CACHE, // invalid cserve cache entry
   CSERVE2_NOT_LOADED,
   CSERVE2_FILE_CHANGED,
   CSERVE2_REQUEST_CANCEL
} Error_Type;

struct _Msg_Base {
   int type;
   unsigned int rid;
};

typedef struct _Msg_Base Msg_Base;

struct _Msg_Open {
   Msg_Base base;
   unsigned int file_id;
   int path_offset;
   int key_offset;
   unsigned int image_id;

   Eina_Bool has_load_opts : 1;

   // Followed by:
   // const char file[];
   // const char key[];
   // Evas_Image_Load_Opts opts; // if has_load_opts
};

struct _Msg_Opened {
   Msg_Base base;
   struct {
      int w, h;
      int frame_count;
      int loop_count;
      int loop_hint; /* include Evas.h? Copy the enum around? */
      Eina_Bool alpha : 1;
      Eina_Bool animated : 1;
   } image;
};

struct _Msg_Load {
   Msg_Base base;
   unsigned int image_id;
};

struct _Msg_Loaded {
   Msg_Base base;
   struct {
      int mmap_offset;
      int use_offset;
      int mmap_size;
      int image_size;
   } shm;
   struct {
      unsigned int w, h; // Real dimensions of this image. May differ from Msg_Opened::image::{w,h} after scaling.
   } image;
   Eina_Bool alpha : 1;
   Eina_Bool alpha_sparse : 1;
};

struct _Msg_Preload {
   Msg_Base base;
   unsigned int image_id;
};

struct _Msg_Preloaded {
   Msg_Base base;
};

struct _Msg_Unload {
   Msg_Base base;
   unsigned int image_id;
};

struct _Msg_Close {
   Msg_Base base;
   unsigned int file_id;
};

/**
 * @struct _Msg_Font_Load
 *
 * Message from client to request load or unload of a font.
 *
 * The path strings follow the struct inside the message.
 */
struct _Msg_Font_Load {
   Msg_Base base;
   unsigned int sourcelen; // font id
   unsigned int pathlen; // font id
   unsigned int rend_flags; // font id
   unsigned int size; // font id
   unsigned int dpi; // font id
};

/**
 * @struct _Msg_Font_Loaded
 *
 * Message from server to inform that a font was loaded.
 */
struct _Msg_Font_Loaded {
   Msg_Base base;
};

/**
 * @struct _Msg_Font_Glyphs_Request
 *
 * Message from client to request load of glyphs, or inform usage of them.
 *
 * The path strings follow the struct inside the message, as well as
 * the list of glyphs to be loaded.
 */
struct _Msg_Font_Glyphs_Request {
   Msg_Base base;
   unsigned int sourcelen; // font id
   unsigned int pathlen; // font id
   unsigned int rend_flags; // font id
   unsigned int size; // font id
   unsigned int dpi; // font id
   unsigned int hint;
   unsigned int nglyphs;
};

/**
 * @struct _Msg_Font_Glyphs_Loaded
 *
 * Message from server to inform that some glyphs were loaded.
 *
 * This message is quite complex: it contains the font id, representing for
 * which font these glyphs should be loaded, and the number of caches on which
 * the loaded glyphs are stored. Each cache is a SHM segment. The information
 * about each SHM comes serialized just after this struct, and can be read in
 * order as follows:
 *
 * shm name:
 *  - unsigned int size;
 *  - char name[];
 * glyphs:
 *  - unsigned int nglyphs;
 *  - struct {
 *      unsigned int index;
 *      unsigned int offset;
 *      unsigned int size;
 *      unsigned int rows;
 *      unsigned int width;
 *      unsigned int pitch;
 *      unsigned int num_grays;
 *      unsigned int pixel_mode;
 *    } glarray[];
 */
struct _Msg_Font_Glyphs_Loaded {
   Msg_Base base;
};

struct _Msg_Stats {
   Msg_Base base;
   struct {
      unsigned int requested_size;
      unsigned int real_size;
      unsigned int unused_size;
      unsigned int fonts_loaded; /* number of loaded fonts */
      unsigned int fonts_unused; /* number of loaded fonts without reference
                                  * from any clients */
      int fonts_load_time; /* total time spent loading fonts */
      int fonts_used_load_time; /* total time spent loading fonts that are
                                 * really used, i.e. fonts that have glyphs
                                 * rendered */
      int fonts_used_saved_time;
      int glyphs_request_time; /* time spent waiting for glyph requests */
      int glyphs_load_time; /* total time spent loading glyphs */
      int glyphs_render_time; /* total time spent rendering glyphs */
      int glyphs_slave_time; /* total time spent on the slave processing glyph load requests */
      int glyphs_saved_time; /* total time spent loading glyphs */
   } fonts;
   struct {
      unsigned int files_loaded; /* number of file headers loaded */
      unsigned int images_loaded; /* number of image data loaded */
      unsigned int images_unused; /* number of image data loaded and unused */

      unsigned int requested_size; /* memory usage originally requested by
                                    * the client */
      unsigned int files_size; /* memory usage from image headers */
      unsigned int images_size; /* memory usage from image data */
      unsigned int unused_size; /* memory usage from image data */

      int files_load_time;
      int files_saved_time;
      int images_load_time;
      int images_saved_time;
   } images;
};

/**
 * @struct _Msg_Font_Debug
 *
 * Message from server containing all font cache info.
 *
 * Content of the message follows:
 *
 * * char fonts_index_path[64]
 * * number of font entries;
 * * each font entry:
 *   - unsigned int filelen
 *   - const char file
 *   - unsigned int namelen
 *   - const char name
 *   - unsigned int rend_flags;
 *   - unsigned int size;
 *   - unsigned int dpi;
 *   - unsigned int unused;
 *   - const char glyph_data_shm[64];
 *   - const char glyph_mempool_shm[64];
 *   - unsigned int nglyphs;
 *   - each glyph: Glyph_Data struct
 *       - unsigned int id;
 *       - unsigned int refcount;
 *       - unsigned int index;
 *       - unsigned int shm_id; // shared string id
 *       - unsigned int buffer_id;
 *       - unsigned int offset;
 *       - unsigned int size;
 *       - unsigned int rows;
 *       - unsigned int width;
 *       - unsigned int pitch;
 *       - unsigned int num_grays;
 *       - unsigned int pixel_mode;
 */
struct _Msg_Font_Debug {
    Msg_Base base;
    char fonts_index_path[64];
    int nfonts;
};

/**
 * @brief The Msg_Index_List struct
 *
 * Message sent from the server, without request from the client,
 * to inform all clients of the shared index files. Contains the paths
 * to the latest File, Image and Font index shm.
 *
 * The paths contain only the filename used in the call to shm_open.
 * All strings must be null-terminated.
 */
struct _Msg_Index_List {
   Msg_Base base;
   int generation_id;
   char strings_index_path[64];
   char strings_entries_path[64];
   char files_index_path[64];
   char images_index_path[64];
   char fonts_index_path[64];
};

typedef struct _Shm_Object Shm_Object;
typedef struct _Index_Entry Index_Entry;
typedef struct _File_Data File_Data;
typedef struct _Image_Data Image_Data;
typedef struct _Font_Data Font_Data;
typedef struct _Glyph_Data Glyph_Data;
typedef struct _Shared_Array_Header Shared_Array_Header;
typedef int string_t;
#define SHMOBJECT unsigned int id; unsigned int refcount

struct _Shared_Array_Header {
   int32_t tag;
   int32_t generation_id;
   int32_t elemsize;
   int32_t count;
   int32_t emptyidx;
   int32_t sortedidx;
   int32_t _reserved1;
   int32_t _reserved2;
};

struct _Shm_Object {
   SHMOBJECT;
};

#define STRING_INDEX_ARRAY_TAG ('S' | 'T' << 8 | 'R' << 16 | 'N' << 24)
#define STRING_MEMPOOL_FAKETAG ('S' | 'T' << 8 | 'R' << 16 | 'M' << 24)
struct _Index_Entry {
   SHMOBJECT;
   // Block entry
   int32_t length;
   int32_t offset;
   int32_t shmid;
};

#define FILE_DATA_ARRAY_TAG ('F' | 'I' << 8 | 'L' << 16 | 'E' << 24)
struct _File_Data {
   SHMOBJECT;
   // Hash entry elements (see Evas_Image_Load_Opts)
   string_t path;
   string_t key;
   struct {
      struct {
         unsigned int x, y, w, h;
      } region;
      double       dpi;
      unsigned int w, h;
      int          scale_down_by;
      Eina_Bool    orientation;
   } lo;
   // Properties set after opening the file
   string_t loader_data; // Can also be set during open (force this loader)
   int w, h;
   int frame_count;
   int loop_count;
   int loop_hint;
   Eina_Bool alpha : 1;
   Eina_Bool changed : 1;
   Eina_Bool valid : 1;
   Eina_Bool animated : 1;
};

#define IMAGE_DATA_ARRAY_TAG ('I' | 'M' << 8 | 'A' << 16 | 'G' << 24)
struct _Image_Data {
   SHMOBJECT;
   uint32_t file_id;
   string_t shm_id;
   Evas_Image_Load_Opts opts;
   uint32_t w, h;
   Eina_Bool alpha : 1;
   Eina_Bool alpha_sparse : 1;
   Eina_Bool unused : 1;
   Eina_Bool doload : 1;
   Eina_Bool valid : 1;
};

#define FONT_DATA_ARRAY_TAG ('F' | 'O' << 8 | 'N' << 16 | 'T' << 24)
struct _Font_Data {
   SHMOBJECT;
   string_t name;
   string_t file;
   string_t glyph_index_shm;
   string_t mempool_shm;
   uint32_t rend_flags;
   uint32_t size;
   uint32_t dpi;
};

#define GLYPH_DATA_ARRAY_TAG ('G' | 'L' << 8 | 'P' << 16 | 'H' << 24)
struct _Glyph_Data {
   // Index_Entry
   SHMOBJECT;
   int32_t length;
   int32_t offset;
   int32_t shmid;
   // Glyph data stuff
   uint32_t index;
   string_t mempool_id; // TODO: Merge with shmid? (Internally impossible atm)
   uint32_t buffer_id;
   uint32_t size;
   uint32_t rows;
   uint32_t width;
   uint32_t pitch;
   uint32_t num_grays;
   uint32_t pixel_mode;
   uint32_t hint;
};

struct _Msg_Error {
   Msg_Base base;
   int error;
};

typedef struct _Msg_Open Msg_Open;
typedef struct _Msg_Opened Msg_Opened;
typedef struct _Msg_Load Msg_Load;
typedef struct _Msg_Loaded Msg_Loaded;
typedef struct _Msg_Preload Msg_Preload;
typedef struct _Msg_Preloaded Msg_Preloaded;
typedef struct _Msg_Unload Msg_Unload;
typedef struct _Msg_Close Msg_Close;
typedef struct _Msg_Font_Load Msg_Font_Load;
typedef struct _Msg_Font_Loaded Msg_Font_Loaded;
typedef struct _Msg_Font_Glyphs_Request Msg_Font_Glyphs_Request;
typedef struct _Msg_Font_Glyphs_Loaded Msg_Font_Glyphs_Loaded;
typedef struct _Msg_Stats Msg_Stats;
typedef struct _Msg_Font_Debug Msg_Font_Debug;
typedef struct _Msg_Error Msg_Error;
typedef struct _Msg_Index_List Msg_Index_List;

#endif
#endif
