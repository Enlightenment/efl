#ifndef _EVAS_CSERVE2_H
#define _EVAS_CSERVE2_H

#include <Eina.h>
#include "evas_cs2.h"

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_cserve2_bin_log_dom, __VA_ARGS__)
#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_cserve2_bin_log_dom, __VA_ARGS__)

extern int _evas_cserve2_bin_log_dom;

typedef struct _Slave_Proc Slave_Proc;
typedef struct _Shm_Handle Shm_Handle;

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

struct _Image_Load_Opts {
   unsigned int w, h;
   unsigned int rx, ry, rw, rh;
   unsigned int scale_down_by;
   double dpi;
   Eina_Bool orientation;
};

typedef struct _Image_Load_Opts Image_Load_Opts;

typedef enum {
   IMAGE_OPEN,
   IMAGE_LOAD,
   SLAVE_QUIT,
   ERROR
} Slave_Command;

struct _Slave_Msg_Image_Open {
   Eina_Bool has_opts : 1;
   Eina_Bool has_loader_data : 1;
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
   Image_Load_Opts opts;
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
   Eina_Bool alpha_sparse : 1;
};

typedef struct _Slave_Msg_Image_Open Slave_Msg_Image_Open;
typedef struct _Slave_Msg_Image_Opened Slave_Msg_Image_Opened;
typedef struct _Slave_Msg_Image_Load Slave_Msg_Image_Load;
typedef struct _Slave_Msg_Image_Loaded Slave_Msg_Image_Loaded;

typedef void (*Fd_Watch_Cb)(int fd, Fd_Flags flags, void *data);
typedef void (*Timeout_Cb)(void); /* void* for compat? */
typedef void (*Main_Loop_Child_Dead_Cb)(int pid, int status); /* void* for compat? */
typedef void (*Slave_Dead_Cb)(Slave_Proc *slave, void *data);
typedef void (*Slave_Read_Cb)(Slave_Proc *slave, Slave_Command cmd, void *msg, void *data);
typedef void (*File_Change_Cb)(const char *path, Eina_Bool deleted, void *data);

void cserve2_client_accept(int fd);
ssize_t cserve2_client_read(Client *client, void *buf, size_t len);
ssize_t cserve2_client_write(Client *client, const void *buf, size_t len);
void cserve2_client_del(Client *client);
void cserve2_client_deliver(Client *client);
void cserve2_client_error_send(Client *client, unsigned int rid, int error_code);
ssize_t cserve2_client_send(Client *client, const void *data, size_t size);

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
Slave_Proc *cserve2_slave_run(const char *exe, Slave_Read_Cb read_cb, Slave_Dead_Cb dead_cb, const void *data);
void cserve2_slave_send(Slave_Proc *s, Slave_Command cmd, const char *data, size_t size);
void cserve2_slave_kill(Slave_Proc *s);

void cserve2_message_handler(int fd, Fd_Flags flags, void *data);

Shm_Handle *cserve2_shm_request(size_t size);
void cserve2_shm_unref(Shm_Handle *shm);
const char *cserve2_shm_name_get(const Shm_Handle *shm);
off_t cserve2_shm_map_offset_get(const Shm_Handle *shm);
off_t cserve2_shm_offset_get(const Shm_Handle *shm);
size_t cserve2_shm_map_size_get(const Shm_Handle *shm);
size_t cserve2_shm_size_get(const Shm_Handle *shm);

void cserve2_command_run(Client *client, Message_Type type);

void cserve2_cache_init(void);
void cserve2_cache_shutdown(void);
void cserve2_cache_client_new(Client *client);
void cserve2_cache_client_del(Client *client);
int cserve2_cache_file_open(Client *client, unsigned int client_file_id, const char *path, const char *key, unsigned int rid);
void cserve2_cache_file_close(Client *client, unsigned int client_file_id);
int cserve2_cache_image_opts_set(Client *client, Msg_Setopts *msg);
void cserve2_cache_image_load(Client *client, unsigned int client_image_id, unsigned int rid);
void cserve2_cache_image_preload(Client *client, unsigned int client_image_id, unsigned int rid);
void cserve2_cache_image_unload(Client *client, unsigned int client_image_id);

int cserve2_cache_font_load(Client *client, const char *name, unsigned int namelen, unsigned int rend_flags, unsigned int hint, unsigned int size, unsigned int dpi, unsigned int rid);

void cserve2_cache_requests_process(void);
void cserve2_cache_requests_response(Slave_Command type, void *msg, void *data);

#endif /* _EVAS_CSERVE2_H */
