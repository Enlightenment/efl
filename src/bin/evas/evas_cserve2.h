#ifndef _EVAS_CSERVE2_H
#define _EVAS_CSERVE2_H

#include <sys/types.h>

#include <Eina.h>
#include "evas_cs2.h"

#ifndef CSERVE2_LOG_LEVEL
#define CSERVE2_LOG_LEVEL 4
#endif

#ifdef CRI
#undef CRI
#endif
#if CSERVE2_LOG_LEVEL >= 0
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#else
#define CRI(...) do {} while(0)
#endif

#ifdef ERR
#undef ERR
#endif
#if CSERVE2_LOG_LEVEL >= 1
#define ERR(...) EINA_LOG_DOM_ERR(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#else
#define ERR(...) do {} while(0)
#endif

#ifdef WRN
#undef WRN
#endif
#if CSERVE2_LOG_LEVEL >= 2
#define WRN(...) EINA_LOG_DOM_WARN(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#else
#define WRN(...) do {} while(0)
#endif

#ifdef INF
#undef INF
#endif
#if CSERVE2_LOG_LEVEL >= 3
#define INF(...) EINA_LOG_DOM_INFO(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#else
#define INF(...) do {} while(0)
#endif

#ifdef DBG
#undef DBG
#endif
#if CSERVE2_LOG_LEVEL >= 4
#define DBG(...) EINA_LOG_DOM_DBG(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#else
#define DBG(...) do {} while(0)
#endif

#define DEBUG_LOAD_TIME 1

extern int _evas_cserve2_bin_log_dom;

extern Eina_Prefix *_evas_cserve2_pfx;

typedef struct _Slave Slave;
typedef struct _Slave_Thread_Data Slave_Thread_Data;
typedef struct _Shm_Handle Shm_Handle;
typedef struct _Shared_Array Shared_Array;
typedef struct _Shared_Mempool Shared_Mempool;

typedef enum {
   FD_READ = 1,
   FD_WRITE = 2,
   FD_ERROR = 4
} Fd_Flags;

struct _Client {
    unsigned int id;
    int socket;
    struct {
       Eina_Bool reading;
       char *buf; // buffer of data being read
       int done, size;
       Eina_Binbuf *pending; // pending data to send
    } msg;
    struct {
       Eina_Hash *referencing; // indexed by client file id
    } files;
    struct {
       Eina_Hash *referencing; // indexed by client image id
    } images;
    struct {
       Eina_List *referencing;
    } fonts;
};

typedef struct _Client Client;

typedef enum {
   IMAGE_OPEN,
   IMAGE_LOAD,
   FONT_LOAD,
   FONT_GLYPHS_LOAD,
   SLAVE_QUIT,
   ERROR,
   SLAVE_COMMAND_LAST
} Slave_Command;

struct _Slave_Msg_Image_Open {
   struct {
      struct {
         unsigned int x, y, w, h;
      } region;
      double       dpi;
      unsigned int w, h;
      int          scale_down_by;
      Eina_Bool    orientation;
   } lo;
   // const char path[];
   // const char key[];
   // const char loader[];
};

struct _Slave_Msg_Image_Opened {
   int w, h;
   int degree;
   int scale; /* used by jpeg when loading in smaller sizes */
   int frame_count;
   int loop_count;
   int loop_hint; /* include Evas.h? Copy the enum around? */
   Eina_Bool alpha : 1;
   Eina_Bool animated : 1;
   Eina_Bool rotated : 1;

   Eina_Bool has_loader_data : 1;
};

struct _Slave_Msg_Image_Load {
   int w, h;
   Evas_Image_Load_Opts opts;
   struct {
      int mmap_offset;
      int image_offset;
      int mmap_size;
      int image_size;
   } shm;
   Eina_Bool alpha : 1;
   Eina_Bool has_loader_data : 1;
};

struct _Slave_Msg_Image_Loaded {
   int w, h;
   Eina_Bool alpha : 1;
   Eina_Bool alpha_sparse : 1;
};

typedef struct _Slave_Msg_Image_Open Slave_Msg_Image_Open;
typedef struct _Slave_Msg_Image_Opened Slave_Msg_Image_Opened;
typedef struct _Slave_Msg_Image_Load Slave_Msg_Image_Load;
typedef struct _Slave_Msg_Image_Loaded Slave_Msg_Image_Loaded;

struct _Slave_Msg_Font_Load {
   void *ftdata1; // Freetype file source info comes here
   void *ftdata2; // Freetype font info comes here
   unsigned int rend_flags;
   unsigned int size;
   unsigned int dpi;
   const char *name;
   const char *file;
   void *data;
   int datasize;
};

struct _Slave_Msg_Font_Loaded {
   void *ftdata1;
   void *ftdata2;
};

struct _Slave_Msg_Font_Glyphs_Load {
   struct {
      void *ftdata1;
      void *ftdata2;
      unsigned int rend_flags;
      unsigned int hint;
   } font;
   struct {
      unsigned int nglyphs;
      unsigned int *glyphs;
   } glyphs;
   struct {
      Shared_Mempool *mempool;
   } cache;
};

struct _Slave_Msg_Glyph {
   unsigned int index;
   unsigned int buffer_id;
   unsigned int offset;
   unsigned int size;
   unsigned int rows;
   unsigned int width;
   unsigned int pitch;
};

typedef struct _Slave_Msg_Glyph Slave_Msg_Glyph;

struct _Slave_Msg_Font_Glyphs_Loaded {
   Shared_Mempool *mempool;
   unsigned int gl_load_time;
   unsigned int gl_render_time;
   unsigned int gl_slave_time;
   Slave_Msg_Glyph *glyphs;
   unsigned int nglyphs;
};

typedef struct _Slave_Msg_Font_Load Slave_Msg_Font_Load;
typedef struct _Slave_Msg_Font_Loaded Slave_Msg_Font_Loaded;
typedef struct _Slave_Msg_Font_Glyphs_Load Slave_Msg_Font_Glyphs_Load;
typedef struct _Slave_Msg_Font_Glyphs_Loaded Slave_Msg_Font_Glyphs_Loaded;

typedef void *(*Slave_Request_Msg_Create)(void *data, int *size);
typedef void (*Slave_Request_Msg_Free)(void *msg, void *data);
typedef Msg_Base *(*Slave_Request_Response)(void *data, void *resp, int *size);
typedef void (*Slave_Request_Error)(void *data, Error_Type error);

struct _Slave_Request_Funcs {
   Slave_Request_Msg_Create msg_create;
   Slave_Request_Msg_Free msg_free;
   Slave_Request_Response response;
   Slave_Request_Error error;
};

typedef struct _Slave_Request Slave_Request;
typedef struct _Slave_Request_Funcs Slave_Request_Funcs;

/* This enum must be kept in sync with lib/evas/include/evas_common_private.h */
typedef enum {
   FONT_REND_REGULAR = 0,
   FONT_REND_SLANT   = (1 << 0),
   FONT_REND_WEIGHT  = (1 << 2)
} Font_Rend_Flags;

typedef enum {
   CSERVE2_REQ_IMAGE_OPEN = 0,
   CSERVE2_REQ_IMAGE_LOAD,
   CSERVE2_REQ_IMAGE_SPEC_LOAD,
   CSERVE2_REQ_FONT_LOAD,
   CSERVE2_REQ_FONT_GLYPHS_LOAD,
   CSERVE2_REQ_LAST
} Slave_Request_Type;

typedef struct _Glyph_Entry Glyph_Entry;

typedef void (*Fd_Watch_Cb)(int fd, Fd_Flags flags, void *data);
typedef void (*Timeout_Cb)(void); /* void* for compat? */
typedef void (*Main_Loop_Child_Dead_Cb)(int pid, int status); /* void* for compat? */
typedef void (*Slave_Dead_Cb)(Slave *slave, void *data);
typedef void (*Slave_Read_Cb)(Slave *slave, Slave_Command cmd, void *msg, void *data);
typedef void *(*Slave_Thread_Cb)(Slave_Thread_Data *sd, Slave_Command *cmd, const void *cmddata, void *userdata);
typedef void (*File_Change_Cb)(const char *path, Eina_Bool deleted, void *data);

void cserve2_client_accept(int fd);
ssize_t cserve2_client_read(Client *client, void *buf, size_t len);
ssize_t cserve2_client_write(Client *client, const void *buf, size_t len);
void cserve2_client_del(Client *client);
void cserve2_client_deliver(Client *client);
void cserve2_client_error_send(Client *client, unsigned int rid, int error_code);
ssize_t cserve2_client_send(Client *client, const void *data, size_t size);
void cserve2_index_list_send(int generation_id, const char *strings_index_path, const char *strings_entries_path, const char *files_index_path, const char *images_index_path, const char *fonts_index_path, Client *client);

Eina_Bool cserve2_fd_watch_add(int fd, Fd_Flags flags, Fd_Watch_Cb cb, const void *data);
Eina_Bool cserve2_fd_watch_del(int fd);
Eina_Bool cserve2_fd_watch_flags_set(int fd, Fd_Flags flags);
Eina_Bool cserve2_fd_watch_flags_get(int fd, Fd_Flags *flags);

Eina_Bool cserve2_file_change_watch_add(const char *path, File_Change_Cb cb, const void *data);
Eina_Bool cserve2_file_change_watch_del(const char *path);

void cserve2_on_child_dead_set(Main_Loop_Child_Dead_Cb cb);

void cserve2_timeout_cb_set(int t, Timeout_Cb cb);

Eina_Bool cserve2_main_loop_setup(void);
void cserve2_main_loop_run(void);
void cserve2_main_loop_finish(void);

Eina_Bool cserve2_slaves_init(void);
void cserve2_slaves_shutdown(void);
int cserve2_slave_available_get(void);
Eina_Bool cserve2_slave_cmd_dispatch(void *data, Slave_Command cmd, const void *msg, int size);
Slave *cserve2_slave_run(const char *exe, Slave_Read_Cb read_cb, Slave_Dead_Cb dead_cb, const void *data);
Slave *cserve2_slave_thread_run(Slave_Thread_Cb thread_cb, void *thread_data, Slave_Read_Cb read_cb, Slave_Dead_Cb dead_cb, const void *data);
void cserve2_slave_send(Slave *s, Slave_Command cmd, const char *data, size_t size);
void cserve2_slave_kill(Slave *s);

void cserve2_message_handler(int fd, Fd_Flags flags, void *data);

void cserve2_shm_init(void);
void cserve2_shm_shutdown(void);
Shm_Handle *cserve2_shm_request(const char *infix, size_t size);
Shm_Handle *cserve2_shm_segment_request(Shm_Handle *shm, size_t size);
Shm_Handle *cserve2_shm_resize(Shm_Handle *shm, size_t newsize);
void cserve2_shm_unref(Shm_Handle *shm);
const char *cserve2_shm_name_get(const Shm_Handle *shm);
int cserve2_shm_id_get(const Shm_Handle *shm);
off_t cserve2_shm_map_offset_get(const Shm_Handle *shm);
off_t cserve2_shm_offset_get(const Shm_Handle *shm);
size_t cserve2_shm_map_size_get(const Shm_Handle *shm);
size_t cserve2_shm_size_get(const Shm_Handle *shm);
void *cserve2_shm_map(Shm_Handle *shm);
void cserve2_shm_unmap(Shm_Handle *shm);
size_t cserve2_shm_size_normalize(size_t size, size_t align);

void cserve2_command_run(Client *client, Message_Type type);

void cserve2_scale_init(void);
void cserve2_scale_shutdown(void);

void cserve2_cache_init(void);
void cserve2_cache_shutdown(void);
void cserve2_cache_client_new(Client *client);
void cserve2_cache_client_del(Client *client);
int cserve2_cache_file_open(Client *client, unsigned int client_file_id, const char *path, const char *key, unsigned int rid, Evas_Image_Load_Opts *lo);
void cserve2_cache_file_close(Client *client, unsigned int client_file_id);
int cserve2_cache_image_entry_create(Client *client, int rid, unsigned int client_file_id, unsigned int image_id, const Evas_Image_Load_Opts *opts);
void cserve2_rgba_image_scale_do(void *src_data, int src_full_w, int src_full_h, void *dst_data, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int alpha, int smooth);
void cserve2_cache_image_load(Client *client, unsigned int client_image_id, unsigned int rid);
void cserve2_cache_image_preload(Client *client, unsigned int client_image_id, unsigned int rid);
void cserve2_cache_image_unload(Client *client, unsigned int client_image_id);

int cserve2_cache_font_load(Client *client, const char *source, const char *name, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int rid);
int cserve2_cache_font_unload(Client *client, const char *source, const char *name, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int rid);
int cserve2_cache_font_glyphs_load(Client *client, const char *source, const char *name, unsigned int rend_flags, unsigned int hint, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs, unsigned int rid);
int cserve2_cache_font_glyphs_used(Client *client, const char *source, const char *name, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs, unsigned int rid);
void cserve2_cache_stats_get(Client *client, unsigned int rid);
void cserve2_cache_font_debug(Client *client, unsigned int rid);


Slave_Request *cserve2_request_add(Slave_Request_Type type, unsigned int rid, Client *client, Slave_Request *dep, Slave_Request_Funcs *funcs, void *data);
void cserve2_request_waiter_add(Slave_Request *req, unsigned int rid, Client *client);
void cserve2_request_type_set(Slave_Request *req, Slave_Request_Type type);
void cserve2_request_cancel(Slave_Request *req, Client *client, Error_Type err);
void cserve2_request_cancel_all(Slave_Request *req, Error_Type err);
void cserve2_requests_init(void);
void cserve2_requests_shutdown(void);
void cserve2_request_dependents_drop(Slave_Request *req, Slave_Request_Type type);
void cserve2_entry_request_drop(void *data, Slave_Request_Type type);

void cserve2_font_init(void);
void cserve2_font_shutdown(void);
void *cserve2_font_slave_cb(Slave_Thread_Data *sd, Slave_Command *cmd, const void *cmddata, void *data);
void cserve2_font_source_ft_free(void *fontsource);
void cserve2_font_ft_free(void *fontinfo);

// Shared buffers & indexes
void cserve2_shared_index_init(void);
void cserve2_shared_index_shutdown(void);

typedef Eina_Bool (* Shared_Array_Repack_Skip_Cb) (Shared_Array *sa,
                                                   const void *elem,
                                                   void *user_data);

// Shared arrays (arrays of fixed size object)
Shared_Array *cserve2_shared_array_new(int tag, int generation_id, int elemsize, int initcount);
const char *cserve2_shared_array_name_get(Shared_Array *sa);
void cserve2_shared_array_del(Shared_Array *sa);
int cserve2_shared_array_size_get(Shared_Array *sa);
int cserve2_shared_array_count_get(Shared_Array *sa);
int cserve2_shared_array_map_size_get(Shared_Array *sa);
int cserve2_shared_array_item_size_get(Shared_Array *sa);
int cserve2_shared_array_generation_id_get(Shared_Array *sa);
int cserve2_shared_array_generation_id_set(Shared_Array *sa, int generation_id);
int cserve2_shared_array_size_set(Shared_Array *sa, int newcount);
int cserve2_shared_array_item_new(Shared_Array *sa);
void *cserve2_shared_array_item_data_get(Shared_Array *sa, int elemid);
Shared_Array *cserve2_shared_array_repack(Shared_Array *sa, int generation_id,
                                          Shared_Array_Repack_Skip_Cb skip,
                                          Eina_Compare_Cb cmp, void *user_data);
int cserve2_shared_array_item_find(Shared_Array *sa, void *data,
                                   Eina_Compare_Cb cmp);
void *cserve2_shared_array_item_data_find(Shared_Array *sa, void *data,
                                          Eina_Compare_Cb cmp);
int cserve2_shared_array_foreach(Shared_Array *sa, Eina_Each_Cb cb, void *data);

// Shared buffers and memory pools
Shared_Mempool *cserve2_shared_mempool_new(int indextag, int index_elemsize, int generation_id, int initsize);
void cserve2_shared_mempool_del(Shared_Mempool *sm);
int cserve2_shared_mempool_buffer_new(Shared_Mempool *sm, int size);
int cserve2_shared_mempool_buffer_ref(Shared_Mempool *sm, int bufferid);
void cserve2_shared_mempool_buffer_del(Shared_Mempool *sm, int bufferid);
void *cserve2_shared_mempool_buffer_get(Shared_Mempool *sm, int bufferid);
int cserve2_shared_mempool_buffer_offset_get(Shared_Mempool *sm, int bufferid);
size_t cserve2_shared_mempool_size_get(Shared_Mempool *sm);
const char *cserve2_shared_mempool_name_get(Shared_Mempool *sm);
int cserve2_shared_mempool_generation_id_get(Shared_Mempool *sm);
int cserve2_shared_mempool_generation_id_set(Shared_Mempool *sm, int generation_id);
Shared_Array *cserve2_shared_mempool_index_get(Shared_Mempool *sm);

// Shared strings
const char *cserve2_shared_strings_table_name_get(void);
const char *cserve2_shared_strings_index_name_get(void);
int cserve2_shared_string_add(const char *str);
int cserve2_shared_string_ref(int id);
void cserve2_shared_string_del(int id);
const char *cserve2_shared_string_get(int id);
int cserve2_shared_strings_repack(Shared_Array_Repack_Skip_Cb skip, Eina_Compare_Cb cmp);

#endif /* _EVAS_CSERVE2_H */
