#ifndef EVAS_CS2_H
#define EVAS_CS2_H 1

#include <Eina.h>

#ifdef EVAS_CSERVE2

typedef enum {
   CSERVE2_OPEN = 1,
   CSERVE2_OPENED,
   CSERVE2_SETOPTS,
   CSERVE2_SETOPTSED,
   CSERVE2_LOAD,
   CSERVE2_LOADED,
   CSERVE2_PRELOAD,
   CSERVE2_PRELOADED,
   CSERVE2_UNLOAD,
   CSERVE2_CLOSE,
   CSERVE2_FONT_LOAD,
   CSERVE2_FONT_LOADED,
   CSERVE2_FONT_UNLOAD,
   CSERVE2_FONT_GLYPHS_LOAD,
   CSERVE2_FONT_GLYPHS_LOADED,
   CSERVE2_FONT_GLYPHS_USED,
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
};

struct _Msg_Opened {
   Msg_Base base;
   struct {
      int w, h;
      int frame_count;
      int loop_count;
      int loop_hint; /* include Evas.h? Copy the enum around? */
      Eina_Bool alpha : 1;
   } image;
};

struct _Msg_Setopts {
   Msg_Base base;
   unsigned int file_id;
   unsigned int image_id;
   struct {
      double dpi;
      int w, h;
      int scale_down;
      int rx, ry, rw, rh;
      Eina_Bool orientation;
   } opts;
};

struct _Msg_Setoptsed {
   Msg_Base base;
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
   unsigned int pathlen; // font id
   unsigned int rend_flags; // font id
   unsigned int hint; // font id
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
 * Message from client to request load of glyphs, of inform usage of them.
 *
 * The path strings follow the struct inside the message, as well as
 * the list of glyphs to be loaded.
 */
struct _Msg_Font_Glyphs_Request {
   Msg_Base base;
   unsigned int pathlen; // font id
   unsigned int rend_flags; // font id
   unsigned int hint; // font id
   unsigned int size; // font id
   unsigned int dpi; // font id
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
 *    } glarray[];
 */
struct _Msg_Font_Glyphs_Loaded {
   Msg_Base base;
   unsigned int ncaches;
};

struct _Msg_Error {
   Msg_Base base;
   int error;
};

typedef struct _Msg_Open Msg_Open;
typedef struct _Msg_Opened Msg_Opened;
typedef struct _Msg_Setopts Msg_Setopts;
typedef struct _Msg_Setoptsed Msg_Setoptsed;
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
typedef struct _Msg_Error Msg_Error;

#endif
#endif
