#define EFL_NET_DIALER_HTTP_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_IO_SIZER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_NET_DIALER_HTTP_BUFFER_RECEIVE_SIZE (1U << 14) /* 16Kb to receive */

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

/*
 * uncomment to test with system's curl.h, by default uses a local
 * replica with required values.
 */
//#define USE_CURL_H 1
#include "ecore_con_url_curl.h"

#include <ctype.h>
#include <fcntl.h>

/* improve usage of lazy-loaded library in _c-> */
#define curl_easy_strerror(...) _c->curl_easy_strerror(__VA_ARGS__)
#define curl_easy_init(...) _c->curl_easy_init(__VA_ARGS__)
#define curl_easy_cleanup(...) _c->curl_easy_cleanup(__VA_ARGS__)
#define curl_easy_pause(...) _c->curl_easy_pause(__VA_ARGS__)

#ifdef curl_easy_setopt
#undef curl_easy_setopt
#endif
#ifndef __CURL_TYPECHECK_GCC_H
#define curl_easy_setopt(easy, option, value) _c->curl_easy_setopt(easy, option, value)
#else
/* curl.h was used with type-checking, so replicate it here from typecheck-gcc.h */
#define curl_easy_setopt(handle, option, value)                               \
__extension__ ({                                                              \
  __typeof__ (option) _curl_opt = option;                                     \
  if(__builtin_constant_p(_curl_opt)) {                                       \
    if(_curl_is_long_option(_curl_opt))                                       \
      if(!_curl_is_long(value))                                               \
        _curl_easy_setopt_err_long();                                         \
    if(_curl_is_off_t_option(_curl_opt))                                      \
      if(!_curl_is_off_t(value))                                              \
        _curl_easy_setopt_err_curl_off_t();                                   \
    if(_curl_is_string_option(_curl_opt))                                     \
      if(!_curl_is_string(value))                                             \
        _curl_easy_setopt_err_string();                                       \
    if(_curl_is_write_cb_option(_curl_opt))                                   \
      if(!_curl_is_write_cb(value))                                           \
        _curl_easy_setopt_err_write_callback();                               \
    if((_curl_opt) == CURLOPT_READFUNCTION)                                   \
      if(!_curl_is_read_cb(value))                                            \
        _curl_easy_setopt_err_read_cb();                                      \
    if((_curl_opt) == CURLOPT_IOCTLFUNCTION)                                  \
      if(!_curl_is_ioctl_cb(value))                                           \
        _curl_easy_setopt_err_ioctl_cb();                                     \
    if((_curl_opt) == CURLOPT_SOCKOPTFUNCTION)                                \
      if(!_curl_is_sockopt_cb(value))                                         \
        _curl_easy_setopt_err_sockopt_cb();                                   \
    if((_curl_opt) == CURLOPT_OPENSOCKETFUNCTION)                             \
      if(!_curl_is_opensocket_cb(value))                                      \
        _curl_easy_setopt_err_opensocket_cb();                                \
    if((_curl_opt) == CURLOPT_PROGRESSFUNCTION)                               \
      if(!_curl_is_progress_cb(value))                                        \
        _curl_easy_setopt_err_progress_cb();                                  \
    if((_curl_opt) == CURLOPT_DEBUGFUNCTION)                                  \
      if(!_curl_is_debug_cb(value))                                           \
        _curl_easy_setopt_err_debug_cb();                                     \
    if((_curl_opt) == CURLOPT_SSL_CTX_FUNCTION)                               \
      if(!_curl_is_ssl_ctx_cb(value))                                         \
        _curl_easy_setopt_err_ssl_ctx_cb();                                   \
    if(_curl_is_conv_cb_option(_curl_opt))                                    \
      if(!_curl_is_conv_cb(value))                                            \
        _curl_easy_setopt_err_conv_cb();                                      \
    if((_curl_opt) == CURLOPT_SEEKFUNCTION)                                   \
      if(!_curl_is_seek_cb(value))                                            \
        _curl_easy_setopt_err_seek_cb();                                      \
    if(_curl_is_cb_data_option(_curl_opt))                                    \
      if(!_curl_is_cb_data(value))                                            \
        _curl_easy_setopt_err_cb_data();                                      \
    if((_curl_opt) == CURLOPT_ERRORBUFFER)                                    \
      if(!_curl_is_error_buffer(value))                                       \
        _curl_easy_setopt_err_error_buffer();                                 \
    if((_curl_opt) == CURLOPT_STDERR)                                         \
      if(!_curl_is_FILE(value))                                               \
        _curl_easy_setopt_err_FILE();                                         \
    if(_curl_is_postfields_option(_curl_opt))                                 \
      if(!_curl_is_postfields(value))                                         \
        _curl_easy_setopt_err_postfields();                                   \
    if((_curl_opt) == CURLOPT_HTTPPOST)                                       \
      if(!_curl_is_arr((value), struct curl_httppost))                        \
        _curl_easy_setopt_err_curl_httpost();                                 \
    if(_curl_is_slist_option(_curl_opt))                                      \
      if(!_curl_is_arr((value), struct curl_slist))                           \
        _curl_easy_setopt_err_curl_slist();                                   \
    if((_curl_opt) == CURLOPT_SHARE)                                          \
      if(!_curl_is_ptr((value), CURLSH))                                      \
        _curl_easy_setopt_err_CURLSH();                                       \
  }                                                                           \
  _c->curl_easy_setopt(handle, _curl_opt, value);                             \
})
#endif

#ifdef curl_easy_getinfo
#undef curl_easy_getinfo
#endif
#ifndef __CURL_TYPECHECK_GCC_H
#define curl_easy_getinfo(easy, info, arg) _c->curl_easy_getinfo(easy, info, arg)
#else
/* curl.h was used with type-checking, so replicate it here from typecheck-gcc.h */

/* wraps curl_easy_getinfo() with typechecking */
/* FIXME: don't allow const pointers */
#define curl_easy_getinfo(handle, info, arg)                                  \
__extension__ ({                                                              \
  __typeof__ (info) _curl_info = info;                                        \
  if(__builtin_constant_p(_curl_info)) {                                      \
    if(_curl_is_string_info(_curl_info))                                      \
      if(!_curl_is_arr((arg), char *))                                        \
        _curl_easy_getinfo_err_string();                                      \
    if(_curl_is_long_info(_curl_info))                                        \
      if(!_curl_is_arr((arg), long))                                          \
        _curl_easy_getinfo_err_long();                                        \
    if(_curl_is_double_info(_curl_info))                                      \
      if(!_curl_is_arr((arg), double))                                        \
        _curl_easy_getinfo_err_double();                                      \
    if(_curl_is_slist_info(_curl_info))                                       \
      if(!_curl_is_arr((arg), struct curl_slist *))                           \
        _curl_easy_getinfo_err_curl_slist();                                  \
  }                                                                           \
  _c->curl_easy_getinfo(handle, _curl_info, arg);                             \
})
#endif

#define curl_multi_strerror(...) _c->curl_multi_strerror(__VA_ARGS__)
#define curl_multi_init(...) _c->curl_multi_init(__VA_ARGS__)
#define curl_multi_cleanup(...) _c->curl_multi_cleanup(__VA_ARGS__)
#ifdef curl_multi_setopt
#undef curl_multi_setopt
#endif
#define curl_multi_setopt(multi, opt, value) _c->curl_multi_setopt(multi, opt, value)

#define curl_multi_add_handle(...) _c->curl_multi_add_handle(__VA_ARGS__)
#define curl_multi_remove_handle(...) _c->curl_multi_remove_handle(__VA_ARGS__)

#define curl_multi_assign(...) _c->curl_multi_assign(__VA_ARGS__)
#define curl_multi_socket_action(...) _c->curl_multi_socket_action(__VA_ARGS__)
#define curl_multi_info_read(...) _c->curl_multi_info_read(__VA_ARGS__)

#define curl_slist_append(...) _c->curl_slist_append(__VA_ARGS__)
#define curl_slist_free_all(...) _c->curl_slist_free_all(__VA_ARGS__)

#define MY_CLASS EFL_NET_DIALER_HTTP_CLASS

typedef struct _Efl_Net_Dialer_Http_Curlm {
   Eo *loop;
   CURLM *multi;
   Eina_List *users;
   Eo *timer;
   int running;
   unsigned int pending_init;
} Efl_Net_Dialer_Http_Curlm;


typedef struct
{
   CURL *easy;
   Efl_Net_Dialer_Http_Curlm *cm;
   Ecore_Fd_Handler *fdhandler;
   Eina_Stringshare *address_dial;
   Eina_Stringshare *proxy;
   Eina_Stringshare *cookie_jar;
   Eina_Stringshare *address_local;
   Eina_Stringshare *address_remote;
   Eina_Stringshare *method;
   Eina_Stringshare *user_agent;
   struct {
      struct curl_slist *headers;
      int64_t content_length;
   } request;
   struct {
      Eina_Slice slice;
   } send;
   struct {
      uint8_t *bytes;
      size_t used;
      size_t limit;
   } recv;
   uint64_t size;
   double timeout_dial;
   struct {
      Eina_Stringshare *username;
      char *password;
      Efl_Net_Http_Authentication_Method method;
      Eina_Bool restricted;
   } authentication;
   Eina_Promise *pending_close;
   unsigned int in_curl_callback;
   int fd;
   Eina_Error error;
   Efl_Net_Http_Version version;
   Efl_Net_Dialer_Http_Primary_Mode primary_mode;
   Eina_Bool allow_redirects;
   uint8_t pause;
   Eina_Bool connected;
   Eina_Bool closed;
   Eina_Bool close_on_exec;
   Eina_Bool close_on_destructor;
   Eina_Bool eos;
   Eina_Bool can_read;
   Eina_Bool can_write;
   Eina_Bool pending_headers_done;
   struct {
      Eina_List *headers;
      const Eina_List *last_request_header;
      Efl_Net_Http_Status status;
      Eina_Stringshare *content_type;
      int64_t content_length;
   } response;
   struct {
      struct {
         uint64_t now;
         uint64_t total;
      } download;
      struct {
         uint64_t now;
         uint64_t total;
      } upload;
   } progress;
} Efl_Net_Dialer_Http_Data;

static void
_efl_net_dialer_http_curlm_check_finished_object_deleted(void *data, const Efl_Event *event)
{
   Eina_List **p_finished = data;
   *p_finished = eina_list_remove(*p_finished, event->object);
}

static void
_efl_net_dialer_http_curlm_check_finished_object_add(Eina_List **p_finished, Eo *dialer)
{
   if (eina_list_data_find(*p_finished, dialer)) return;

   efl_event_callback_add(dialer, EFL_EVENT_DEL,
                          _efl_net_dialer_http_curlm_check_finished_object_deleted,
                          p_finished);
   *p_finished = eina_list_append(*p_finished, dialer);
}

static void
_efl_net_dialer_http_curlm_check_finished_object_remove(Eina_List **p_finished, Eo *dialer)
{
   efl_event_callback_del(dialer, EFL_EVENT_DEL,
                          _efl_net_dialer_http_curlm_check_finished_object_deleted,
                          p_finished);
   *p_finished = eina_list_remove(*p_finished, dialer);
}

static void
_efl_net_dialer_http_curlm_check(Efl_Net_Dialer_Http_Curlm *cm)
{
   CURLMsg *msg;
   int remaining;
   Eina_List *finished = NULL;
   Eo *dialer;

   while ((msg = curl_multi_info_read(cm->multi, &remaining)))
     {
        CURLcode re;
        char *priv; /* CURLINFO_PRIVATE checks for this type */

        re = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &priv);
        if ((re == CURLE_OK) && (priv))
          {
             dialer = (Eo *)priv;

             if (msg->data.result != CURLE_OK)
               {
                  Eina_Error err = _curlcode_to_eina_error(msg->data.result);
                  Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(dialer, MY_CLASS);
                  DBG("HTTP dialer=%p error: #%d '%s'",
                      dialer, err, eina_error_msg_get(err));
                  pd->error = err;
                  _efl_net_dialer_http_curlm_check_finished_object_add(&finished, dialer);
               }

             if (msg->msg != CURLMSG_DONE) continue;

             if (!efl_io_closer_closed_get(dialer))
               {
                  DBG("HTTP dialer=%p eos", dialer);
                  _efl_net_dialer_http_curlm_check_finished_object_add(&finished, dialer);
               }
             else
               DBG("HTTP dialer=%p already closed", dialer);
          }
     }

   while (finished)
     {
        Efl_Net_Dialer_Http_Data *pd;

        dialer = finished->data;
        efl_ref(dialer);
        pd = efl_data_scope_get(dialer, MY_CLASS);
        if (pd->error)
          efl_event_callback_call(dialer, EFL_NET_DIALER_EVENT_ERROR, &pd->error);
        else
          efl_io_reader_eos_set(dialer, EINA_TRUE);
        efl_io_closer_close(dialer);
        _efl_net_dialer_http_curlm_check_finished_object_remove(&finished, dialer);
        efl_unref(dialer);
     }
}

static void
_efl_net_dialer_http_curlm_timer_do(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Net_Dialer_Http_Curlm *cm = data;
   CURLMcode r;

   /* expected to trigger only once then reschedule */
   efl_event_freeze(cm->timer);

   r = curl_multi_socket_action(cm->multi,
                                CURL_SOCKET_TIMEOUT, 0, &cm->running);
   if (r != CURLM_OK)
     ERR("socket action CURL_SOCKET_TIMEOUT failed: %s", curl_multi_strerror(r));

   _efl_net_dialer_http_curlm_check(cm);
}

static int
_efl_net_dialer_http_curlm_timer_schedule(CURLM *multi EINA_UNUSED, long timeout_ms, void *data)
{
   Efl_Net_Dialer_Http_Curlm *cm = data;
   double seconds = timeout_ms / 1000.0;

   if (cm->timer)
     {
        efl_loop_timer_interval_set(cm->timer, seconds);
        efl_loop_timer_reset(cm->timer);
        while (efl_event_freeze_count_get(cm->timer) > 0)
          efl_event_thaw(cm->timer);
     }
   else
     {
        cm->timer = efl_add(EFL_LOOP_TIMER_CLASS, cm->loop,
                            efl_loop_timer_interval_set(efl_added, seconds),
                            efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, _efl_net_dialer_http_curlm_timer_do, cm));
        EINA_SAFETY_ON_NULL_RETURN_VAL(cm->timer, -1);
     }

   return 0;
}

#if 0
// it seems the Eo_Loop_Fd isn't working properly when we change connections...
// as it's still built on top of Ecore_Fd_Handler, then use it directly.
static void
_efl_net_dialer_http_curlm_event_fd_read(void *data, const Efl_Event *event)
{
   Efl_Net_Dialer_Http_Curlm *cm = data;
   int fd = efl_loop_fd_get(event->object);
   CURLMcode r;

   ERR("XXX socket=%d CURL_CSELECT_IN", fd);
   r = curl_multi_socket_action(cm->multi, fd, CURL_CSELECT_IN, &cm->running);
   if (r != CURLM_OK)
     ERR("socket action CURL_CSELECT_IN fd=%d failed: %s", fd, curl_multi_strerror(r));

   _efl_net_dialer_http_curlm_check(cm);
}

static void
_efl_net_dialer_http_curlm_event_fd_write(void *data, const Efl_Event *event)
{
   Efl_Net_Dialer_Http_Curlm *cm = data;
   int fd = efl_loop_fd_get(event->object);
   CURLMcode r;

   ERR("XXX socket=%d CURL_CSELECT_OUT", fd);
   r = curl_multi_socket_action(cm->multi, fd, CURL_CSELECT_OUT, &cm->running);
   if (r != CURLM_OK)
     ERR("socket action CURL_CSELECT_OUT fd=%d failed: %s", fd, curl_multi_strerror(r));

   _efl_net_dialer_http_curlm_check(cm);
}

static int
_efl_net_dialer_http_curlm_socket_manage(CURL *e, curl_socket_t fd, int what, void *cm_data, void *fdhandler_data)
{
   Efl_Net_Dialer_Http_Curlm *cm = cm_data;
   Efl_Net_Dialer_Http_Data *pd;
   Eo *dialer, *fdhandler = fdhandler_data;
   char *priv;
   CURLcode re;

   re = curl_easy_getinfo(e, CURLINFO_PRIVATE, &priv);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(re != CURLE_OK, -1);
   if (!priv) return -1;
   dialer = (Eo *)priv;
   pd = efl_data_scope_get(dialer, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, -1);

   if (what == CURL_POLL_REMOVE)
     {
        pd->fdhandler = NULL;
        efl_del(fdhandler);
     }
   else
     {
        int flags;
        Eina_Bool was_read, is_read, was_write, is_write;

        if (fdhandler)
          flags = (intptr_t)efl_key_data_get(fdhandler, "curl_flags");
        else
          {
             pd->fdhandler = fdhandler = efl_add(EFL_LOOP_FD_CLASS, cm->loop);
             EINA_SAFETY_ON_NULL_RETURN_VAL(fdhandler, -1);
             curl_multi_assign(cm->multi, fd, fdhandler);
             flags = 0;
          }

        if (what == flags)
          return 0;

        was_read = !!(flags & CURL_POLL_IN);
        was_write = !!(flags & CURL_POLL_OUT);

        is_read = !!(what & CURL_POLL_IN);
        is_write = !!(what & CURL_POLL_OUT);

        ERR("changed flags %#x -> %#x, read: %d/%d, write: %d/%d", flags, what, was_read, is_read, was_write, is_write);

        if (was_read && !is_read)
          {
             //efl_event_callback_del(fdhandler, EFL_LOOP_FD_EVENT_READ, _efl_net_dialer_http_curlm_event_fd_read, cm);
             ERR("XXX del %d read cb", fd);
          }
        else if (!was_read && is_read)
          {
             efl_event_callback_add(fdhandler, EFL_LOOP_FD_EVENT_READ, _efl_net_dialer_http_curlm_event_fd_read, cm);
             ERR("XXX add %d read cb", fd);
          }

        if (was_write && !is_write)
          {
             //efl_event_callback_del(fdhandler, EFL_LOOP_FD_EVENT_WRITE, _efl_net_dialer_http_curlm_event_fd_write, cm);
             ERR("XXX del %d write cb", fd);
          }
        else if (!was_write && is_write)
          {
             efl_event_callback_add(fdhandler, EFL_LOOP_FD_EVENT_WRITE, _efl_net_dialer_http_curlm_event_fd_write, cm);
             ERR("XXX add %d write cb", fd);
          }

        efl_key_data_set(fdhandler, "curl_flags", (void *)(intptr_t)what);
     }

   ERR("XXX finished manage fd=%d, what=%#x, cm=%p, fdhandler=%p", fd, what, cm, fdhandler);

   return 0;
}
#else
// XXX BEGIN Legacy FD Handler:
static Eina_Bool
_efl_net_dialer_http_curlm_event_fd(void *data, Ecore_Fd_Handler *fdhandler)
{
   Efl_Net_Dialer_Http_Curlm *cm = data;
   int fd, flags = 0;
   CURLMcode r;

   if (ecore_main_fd_handler_active_get(fdhandler, ECORE_FD_READ))
     flags |= CURL_CSELECT_IN;
   if (ecore_main_fd_handler_active_get(fdhandler, ECORE_FD_WRITE))
     flags |= CURL_CSELECT_OUT;

   fd = ecore_main_fd_handler_fd_get(fdhandler);
   r = curl_multi_socket_action(cm->multi, fd, flags, &cm->running);
   if (r != CURLM_OK)
     ERR("socket action %#x fd=%d failed: %s", flags, fd, curl_multi_strerror(r));

   _efl_net_dialer_http_curlm_check(cm);

   return EINA_TRUE;
}

static int
_efl_net_dialer_http_curlm_socket_manage(CURL *e, curl_socket_t fd, int what, void *cm_data, void *fdhandler_data)
{
   Efl_Net_Dialer_Http_Curlm *cm = cm_data;
   Ecore_Fd_Handler *fdhandler = fdhandler_data;
   Eo *dialer;
   Efl_Net_Dialer_Http_Data *pd;
   char *priv;
   CURLcode re;

   re = curl_easy_getinfo(e, CURLINFO_PRIVATE, &priv);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(re != CURLE_OK, -1);
   if (!priv) return -1;
   dialer = (Eo *)priv;
   pd = efl_data_scope_get(dialer, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, -1);

   if (what == CURL_POLL_REMOVE)
     {
        pd->fdhandler = NULL;
        ecore_main_fd_handler_del(fdhandler);
     }
   else
     {
        Ecore_Fd_Handler_Flags flags = 0;

        if (what & CURL_POLL_IN) flags |= ECORE_FD_READ;
        if (what & CURL_POLL_OUT) flags |= ECORE_FD_WRITE;

        if (fdhandler)
          ecore_main_fd_handler_active_set(fdhandler, flags);
        else
          {
             pd->fdhandler = fdhandler = ecore_main_fd_handler_add(fd, flags, _efl_net_dialer_http_curlm_event_fd, cm, NULL, NULL);
             EINA_SAFETY_ON_NULL_RETURN_VAL(fdhandler, -1);
             curl_multi_assign(cm->multi, fd, fdhandler);
          }
     }

   DBG("dialer=%p fdhandler=%p, fd=%d, curl_easy=%p, flags=%#x",
       dialer, pd->fdhandler, fd, e, what);

   return 0;
}
// XXX END Legacy FD Handler.
#endif

static Eina_Bool
_efl_net_dialer_http_curlm_add(Efl_Net_Dialer_Http_Curlm *cm, Eo *o, CURL *handle)
{
   CURLMcode r;

   if (!cm->multi)
     {
        cm->multi = curl_multi_init();
        if (!cm->multi)
          {
             ERR("could not create curl multi handle");
             return EINA_FALSE;
          }

        curl_multi_setopt(cm->multi, CURLMOPT_SOCKETFUNCTION, _efl_net_dialer_http_curlm_socket_manage);
        curl_multi_setopt(cm->multi, CURLMOPT_SOCKETDATA, cm);
        curl_multi_setopt(cm->multi, CURLMOPT_TIMERFUNCTION, _efl_net_dialer_http_curlm_timer_schedule);
        curl_multi_setopt(cm->multi, CURLMOPT_TIMERDATA, cm);
     }

   r = curl_multi_add_handle(cm->multi, handle);
   if (r != CURLM_OK)
     {
        ERR("could not register curl multi handle %p: %s",
            handle, curl_multi_strerror(r));
        return EINA_FALSE;
     }

   cm->users = eina_list_append(cm->users, o);

   return EINA_TRUE;
}

static void
_efl_net_dialer_http_curlm_remove(Efl_Net_Dialer_Http_Curlm *cm, Eo *o, CURL *handle)
{
   CURLMcode r = curl_multi_remove_handle(cm->multi, handle);

   DBG("removed handle cm=%p multi=%p easy=%p: %s",
       cm, cm->multi, handle, curl_multi_strerror(r));
   if (r != CURLM_OK)
     {
        ERR("could not unregister curl multi handle %p: %s",
            handle, curl_multi_strerror(r));
     }

   cm->users = eina_list_remove(cm->users, o);
   if (!cm->users)
     {
        DBG("cleaned up cm=%p multi=%p", cm, cm->multi);
        curl_multi_cleanup(cm->multi);
        cm->multi = NULL;

        if (cm->timer)
          {
             efl_del(cm->timer);
             cm->timer = NULL;
          }
     }
}

// TODO: move this per-loop when multiple main loops are possible
static Efl_Net_Dialer_Http_Curlm _cm_global;

static long
_efl_net_http_version_to_curl(Efl_Net_Http_Version version)
{
   switch (version)
     {
      case EFL_NET_HTTP_VERSION_V1_0: return CURL_HTTP_VERSION_1_0;
      case EFL_NET_HTTP_VERSION_V1_1: return CURL_HTTP_VERSION_1_1;
      case EFL_NET_HTTP_VERSION_V2_0: return CURL_HTTP_VERSION_2_0;
      default:
         ERR("unsupported HTTP version code %d", version);
         return CURL_HTTP_VERSION_NONE;
     }
}

static Efl_Net_Http_Version
_efl_net_http_version_from_curl(long version)
{
   switch (version)
     {
      case CURL_HTTP_VERSION_1_0: return EFL_NET_HTTP_VERSION_V1_0;
      case CURL_HTTP_VERSION_1_1: return EFL_NET_HTTP_VERSION_V1_1;
      case CURL_HTTP_VERSION_2_0: return EFL_NET_HTTP_VERSION_V2_0;
      default:
         ERR("unsupported HTTP version from CURL: %ld", version);
         return 0;
     }
}

static Efl_Net_Dialer_Http_Primary_Mode
_efl_net_dialer_http_primary_mode_effective_get(const Efl_Net_Dialer_Http_Data *pd)
{
   if (pd->primary_mode == EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD)
     return EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD;
   else if (pd->primary_mode == EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD)
     return EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD;
   else if (strcasecmp(pd->method, "PUT") == 0)
     return EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD;
   else
     return EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD;
}

static long
_efl_net_http_authentication_method_to_curl(Efl_Net_Http_Authentication_Method method, Eina_Bool restricted)
{
   long flags = 0;

   switch (method) {
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_NONE:
       flags |= CURLAUTH_NONE;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_BASIC:
       flags |= CURLAUTH_BASIC;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_DIGEST:
       flags |= CURLAUTH_DIGEST;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_NEGOTIATE:
       flags |= CURLAUTH_NEGOTIATE;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_NTLM:
       flags |= CURLAUTH_NTLM;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_NTLM_WINBIND:
       flags |= CURLAUTH_NTLM_WB;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY_SAFE:
       flags |= CURLAUTH_ANYSAFE;
       break;
    case EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY:
       flags |= CURLAUTH_ANY;
       break;
   }

   if (restricted) flags |= CURLAUTH_ONLY;

   return flags;
}

static void
_secure_free(char **pstr)
{
   char *str = *pstr;
   if (!str) return;
   memset(str, 0, strlen(str));
   __asm__ __volatile__ ("" : : "g" (str) : "memory");
   free(str);
   *pstr = NULL;
}

static int
_efl_net_dialer_http_debug(CURL *easy EINA_UNUSED, curl_infotype type, char *msg, size_t size, void *data)
{
   Eo *o = data;
   const char *cls = efl_class_name_get(efl_class_get(o));
   switch (type)
     {
      case CURLINFO_TEXT:
         while ((size > 0) && isspace(msg[size - 1])) size--;
         DBG("%s=%p curl said: %.*s", cls, o, (int)size, msg);
         break;
      case CURLINFO_HEADER_IN:
         while ((size > 0) && isspace(msg[size - 1])) size--;
         DBG("%s=%p received header: %.*s", cls, o, (int)size, msg);
         break;
      case CURLINFO_HEADER_OUT:
         while ((size > 0) && isspace(msg[size - 1])) size--;
         DBG("%s=%p sent header: %.*s", cls, o, (int)size, msg);
         break;
      case CURLINFO_DATA_IN:
         DBG("%s=%p received %zd bytes", cls, o, size);
         break;
      case CURLINFO_DATA_OUT:
         DBG("%s=%p sent %zd bytes", cls, o, size);
         break;
      case CURLINFO_SSL_DATA_IN:
         DBG("%s=%p received SSL %zd bytes", cls, o, size);
         break;
      case CURLINFO_SSL_DATA_OUT:
         DBG("%s=%p sent SSL %zd bytes", cls, o, size);
         break;
      default:
         DBG("%s=%p unkown debug type %d, msg=%p, size=%zd", cls, o, type, msg, size);
     }
   return 0;
}

static int
_efl_net_dialer_http_xferinfo(void *data, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
{
   Eo *o = data;
   Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(o, MY_CLASS);

   pd->progress.download.total = dltotal;
   pd->progress.download.now = dlnow;
   pd->progress.upload.total = ultotal;
   pd->progress.upload.now = ulnow;
   return 0;
}

static void
_efl_net_dialer_http_connected(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   CURLcode r;
   long n;
   const char *s;

   r = curl_easy_getinfo(pd->easy, CURLINFO_RESPONSE_CODE, &n);
   if (r != CURLE_OK)
     ERR("dialer=%p could not get response code: %s", o, curl_easy_strerror(r));
   else
     pd->response.status = n;

   r = curl_easy_getinfo(pd->easy, CURLINFO_EFFECTIVE_URL, &s);
   if (r != CURLE_OK)
     ERR("dialer=%p could not get effective url: %s", o, curl_easy_strerror(r));
   else
     efl_net_socket_address_remote_set(o, s);

   r = curl_easy_getinfo(pd->easy, CURLINFO_LOCAL_IP, &s);
   if (r != CURLE_OK)
     ERR("dialer=%p could not get local IP: %s", o, curl_easy_strerror(r));
   else
     {
        r = curl_easy_getinfo(pd->easy, CURLINFO_LOCAL_PORT, &n);
        if (r != CURLE_OK)
          ERR("dialer=%p could not get local port: %s", o, curl_easy_strerror(r));
        else
          {
             char buf[256];

             if (strchr(s, ':'))
               snprintf(buf, sizeof(buf), "[%s]:%ld", s, n);
             else
               snprintf(buf, sizeof(buf), "%s:%ld", s, n);

             efl_net_socket_address_local_set(o, buf);
          }
     }

   r = curl_easy_getinfo(pd->easy, CURLINFO_HTTP_VERSION, &n);
   if (r != CURLE_OK)
     ERR("dialer=%p could not get effective HTTP version: %s", o, curl_easy_strerror(r));
   else
     pd->version = _efl_net_http_version_from_curl(n);

   pd->pending_headers_done = EINA_TRUE;
   efl_net_dialer_connected_set(o, EINA_TRUE);
}

static void
_efl_net_dialer_http_headers_done(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   double d;
   const char *s;
   CURLcode r;

   r = curl_easy_getinfo(pd->easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d);
   if (r != CURLE_OK)
     {
        DBG("could not query content-length for reponse: %s",
            curl_easy_strerror(r));
        d = -1;
     }
   efl_net_dialer_http_response_content_length_set(o, d);

   r = curl_easy_getinfo(pd->easy, CURLINFO_CONTENT_TYPE, &s);
   if (r != CURLE_OK)
     {
        DBG("could not query content-type for response: %s",
            curl_easy_strerror(r));
        s = NULL;
     }
   efl_net_dialer_http_response_content_type_set(o, s);

   pd->pending_headers_done = EINA_FALSE;
   efl_event_callback_call(o, EFL_NET_DIALER_HTTP_EVENT_HEADERS_DONE, NULL);
}

/* take data from internal buffer filled with efl_io_writer_write()
 * and send to curl.
 */
static size_t
_efl_net_dialer_http_send_data_safe(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Rw_Slice rw_slice)
{
   if (pd->pending_headers_done) _efl_net_dialer_http_headers_done(o, pd);
   if ((!pd->send.slice.mem) || (pd->send.slice.len == 0))
     {
        efl_io_writer_can_write_set(o, EINA_TRUE);
        pd->pause |= CURLPAUSE_SEND;
        return CURL_READFUNC_PAUSE;
     }

   rw_slice = eina_rw_slice_copy(rw_slice, pd->send.slice);
   pd->send.slice.len -= rw_slice.len;
   pd->send.slice.bytes += rw_slice.len;

   if (rw_slice.len == 0)
     {
        pd->pause |= CURLPAUSE_SEND;
        return CURL_READFUNC_PAUSE;
     }

   return rw_slice.len;
}

static CURL *
_efl_net_dialer_http_curl_safe_begin(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   if (!o || !pd)
     return NULL;
   if (efl_io_closer_closed_get(o))
     return NULL;

   pd->in_curl_callback++;
   efl_ref(o);

   return pd->easy;
}

static void
_efl_net_dialer_http_curl_cleanup(void *data, void *value EINA_UNUSED)
{
   CURL *easy = data;
   DBG("cleanup curl=%p", easy);
   curl_easy_cleanup(easy);
}

static void
_efl_net_dialer_http_curl_cleanup_error(void *data, Eina_Error err)
{
   CURL *easy = data;
   DBG("cleanup curl=%p, promise error=%d '%s'", easy, err, eina_error_msg_get(err));
   curl_easy_cleanup(easy);
}

static void
_efl_net_dialer_http_curl_safe_end(Eo *o, Efl_Net_Dialer_Http_Data *pd, CURL *easy)
{
   Eina_Promise *p;
   int refs;

   refs = efl_ref_get(o);
   efl_unref(o);
   if (refs > 1)
     {
        pd->in_curl_callback--;
        return;
     }

   /* object deleted from CURL callback, CURL* easy was
    * dissociated and we must delete it ourselves.
    */
   p = efl_loop_job(ecore_main_loop_get(), easy);
   eina_promise_then(p,
                     _efl_net_dialer_http_curl_cleanup,
                     _efl_net_dialer_http_curl_cleanup_error,
                     easy);
   DBG("dialer=%p deleted from CURL callback, cleanup curl from job=%p.", o, p);
}

static size_t
_efl_net_dialer_http_send_data(char *buffer, size_t count, size_t nitems, void *data)
{
   Eo *o = data;
   Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Rw_Slice rw_slice = {.mem = buffer, .len = count * nitems};
   CURL *easy;
   size_t ret;

   easy = _efl_net_dialer_http_curl_safe_begin(o, pd);
   if (!easy) return CURL_READFUNC_PAUSE;

   ret = _efl_net_dialer_http_send_data_safe(o, pd, rw_slice);

   _efl_net_dialer_http_curl_safe_end(o, pd, easy);
   DBG("dialer=%p in=%zd used=%zd", o, rw_slice.len, ret);

   return ret;
}

/* take data from curl into our internal buffer until
 * efl_io_reader_read() consumes it
 */
static size_t
_efl_net_dialer_http_receive_data_safe(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Slice ro_slice)
{
   Eina_Rw_Slice rw_slice = {
     .bytes = pd->recv.bytes + pd->recv.used,
     .len = pd->recv.limit - pd->recv.used,
   };

   if (pd->pending_headers_done) _efl_net_dialer_http_headers_done(o, pd);

   if (ro_slice.len == 0)
     {
        efl_io_reader_can_read_set(o, EINA_FALSE);
        efl_io_reader_eos_set(o, EINA_TRUE);
        return ro_slice.len;
     }

   if (rw_slice.len < ro_slice.len)
     {
        /* throttle CURL and let users read */
        DBG("dialer=%p in=%zd, available %zd (limit=%zd)",
            o, ro_slice.len, pd->recv.limit - pd->recv.used, pd->recv.limit);
        efl_io_reader_can_read_set(o, EINA_TRUE);
        pd->pause |= CURLPAUSE_RECV;
        return CURL_WRITEFUNC_PAUSE;
      }
   rw_slice = eina_rw_slice_copy(rw_slice, ro_slice);
   pd->recv.used += rw_slice.len;

   // TODO: optimize readers from immediate event
   // with  pd->tmp_buf + pd->tmp_buflen that is read after
   // pd->buf.recv inside _efl_io_reader_read()
   efl_io_reader_can_read_set(o, EINA_TRUE);

   return rw_slice.len;
}

static size_t
_efl_net_dialer_http_receive_data(const void *buffer, size_t count, size_t nitems, void *data)
{
   Eo *o = data;
   Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Slice ro_slice = {
     .bytes = buffer,
     .len = count * nitems,
   };
   CURL *easy;
   size_t ret;

   easy = _efl_net_dialer_http_curl_safe_begin(o, pd);
   if (!easy) return CURL_WRITEFUNC_PAUSE;

   ret = _efl_net_dialer_http_receive_data_safe(o, pd, ro_slice);

   _efl_net_dialer_http_curl_safe_end(o, pd, easy);
   if (ret == CURL_WRITEFUNC_PAUSE)
     DBG("dialer=%p in=%zd is now paused", o, ro_slice.len);
   else
     DBG("dialer=%p in=%zd used=%zd", o, ro_slice.len, ret);

   return ret;
}

static size_t
_efl_net_dialer_http_receive_header_safe(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Slice ro_slice)
{
   Efl_Net_Http_Header *h;
   char *p;

   if (ro_slice.len == 0)
     {
        if (!pd->connected) _efl_net_dialer_http_connected(o, pd);
        return 0;
     }

   h = malloc(sizeof(Efl_Net_Http_Header) + ro_slice.len + 1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(h, 0);

   h->key = p = (char *)h + sizeof(Efl_Net_Http_Header);
   memcpy(p, ro_slice.mem, ro_slice.len);
   p[ro_slice.len] = '\0';

   h->value = p = strchr(p, ':');
   if (!p)
     p = (char *)h->key + ro_slice.len - 1;
   else
     {
        char *t;

        p[0] = '\0';
        p--;
        h->value++;
        while (h->value[0] && isspace(h->value[0]))
          h->value++;

        t = (char *)h->key + ro_slice.len - 1;
        while ((t > h->value) && isspace(t[0]))
          {
             t[0] = '\0';
             t--;
          }
     }

   while (h->key[0] && isspace(h->key[0]))
     h->key++;

   while ((p > h->key) && isspace(p[0]))
     {
        p[0] = '\0';
        p--;
     }

   if ((!h->key[0]) && (!h->value || !h->value[0]))
     {
        if (!pd->connected) _efl_net_dialer_http_connected(o, pd);
        if (pd->pending_headers_done) _efl_net_dialer_http_headers_done(o, pd);
        free(h);
        return ro_slice.len;
     }

   if (!h->value)
     {
        if (strncmp(h->key, "HTTP/", strlen("HTTP/")) != 0)
          DBG("unexpected header '" EINA_SLICE_STR_FMT "'", EINA_SLICE_STR_PRINT(ro_slice));
        else
          {
             h->value = h->key;
             h->key = NULL; /* documented as a start of a new request */
             /* notify headers of the previous request */
             if (pd->pending_headers_done) _efl_net_dialer_http_headers_done(o, pd);
             /* reload properties for the previous request */
             _efl_net_dialer_http_connected(o, pd);
          }
     }

   pd->response.headers = eina_list_append(pd->response.headers, h);
   if (!h->key)
     pd->response.last_request_header = eina_list_last(pd->response.headers);

   return ro_slice.len;
}

static size_t
_efl_net_dialer_http_receive_header(const char *buffer, size_t count, size_t nitems, void *data)
{
   Eo *o = data;
   Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Slice ro_slice = {
     .mem = buffer,
     .len = count * nitems,
   };
   CURL *easy;
   size_t ret;

   easy = _efl_net_dialer_http_curl_safe_begin(o, pd);
   if (!easy) return 0;

   ret = _efl_net_dialer_http_receive_header_safe(o, pd, ro_slice);

   _efl_net_dialer_http_curl_safe_end(o, pd, easy);
   DBG("dialer=%p in=%zd used=%zd", o, ro_slice.len, ret);

   return ret;
}

static curl_socket_t
_efl_net_dialer_http_socket_open(void *data, curlsocktype purpose EINA_UNUSED, struct curl_sockaddr *addr)
{
   Eo *o = data;
   Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(o, MY_CLASS);

   pd->fd = efl_net_socket4(addr->family, addr->socktype, addr->protocol, pd->close_on_exec);
   if (pd->fd < 0)
     ERR("could not create curl socket family=%d, type=%d, protocol=%d",
         addr->family, addr->socktype, addr->protocol);
   else
     DBG("socket(%d, %d, %d) = %d",
         addr->family, addr->socktype, addr->protocol, pd->fd);

   return pd->fd;
}

EOLIAN static Efl_Object *
_efl_net_dialer_http_efl_object_constructor(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   if (!_c_init())
     {
        ERR("dialer=%p failed to initialize CURL", o);
        return NULL;
     }

   pd->primary_mode = EFL_NET_DIALER_HTTP_PRIMARY_MODE_AUTO;

   pd->recv.limit = EFL_NET_DIALER_HTTP_BUFFER_RECEIVE_SIZE;
   pd->recv.used = 0;
   pd->recv.bytes = malloc(pd->recv.limit);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->recv.bytes, NULL);

   pd->easy = curl_easy_init();
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->easy, NULL);

   curl_easy_setopt(pd->easy, CURLOPT_PRIVATE, o);

   curl_easy_setopt(pd->easy, CURLOPT_DEBUGFUNCTION, _efl_net_dialer_http_debug);
   curl_easy_setopt(pd->easy, CURLOPT_DEBUGDATA, o);
   curl_easy_setopt(pd->easy, CURLOPT_XFERINFOFUNCTION, _efl_net_dialer_http_xferinfo);
   curl_easy_setopt(pd->easy, CURLOPT_XFERINFODATA, o);

   curl_easy_setopt(pd->easy, CURLOPT_HEADERFUNCTION, _efl_net_dialer_http_receive_header);
   curl_easy_setopt(pd->easy, CURLOPT_HEADERDATA, o);
   curl_easy_setopt(pd->easy, CURLOPT_WRITEFUNCTION, _efl_net_dialer_http_receive_data);
   curl_easy_setopt(pd->easy, CURLOPT_WRITEDATA, o);
   curl_easy_setopt(pd->easy, CURLOPT_BUFFERSIZE, (long)pd->recv.limit);
   curl_easy_setopt(pd->easy, CURLOPT_READFUNCTION, _efl_net_dialer_http_send_data);
   curl_easy_setopt(pd->easy, CURLOPT_READDATA, o);

   curl_easy_setopt(pd->easy, CURLOPT_OPENSOCKETFUNCTION, _efl_net_dialer_http_socket_open);
   curl_easy_setopt(pd->easy, CURLOPT_OPENSOCKETDATA, o);

   curl_easy_setopt(pd->easy, CURLOPT_NOPROGRESS, 0L);
   curl_easy_setopt(pd->easy, CURLOPT_NOSIGNAL, 1L);

   curl_easy_setopt(pd->easy, CURLOPT_VERBOSE, (long)(eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG)));

   o = efl_constructor(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_net_dialer_http_method_set(o, "GET");
   efl_net_dialer_http_version_set(o, EFL_NET_HTTP_VERSION_V1_1);
   efl_net_dialer_http_allow_redirects_set(o, EINA_TRUE);
   efl_net_dialer_timeout_dial_set(o, 30.0);
   return o;
}

EOLIAN static void
_efl_net_dialer_http_efl_object_destructor(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   if (pd->in_curl_callback)
     {
        DBG("deleting HTTP dialer=%p from CURL callback.", o);
        curl_easy_setopt(pd->easy, CURLOPT_PRIVATE, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_DEBUGFUNCTION, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_DEBUGDATA, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_XFERINFOFUNCTION, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_XFERINFODATA, NULL);

        curl_easy_setopt(pd->easy, CURLOPT_HEADERFUNCTION, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_HEADERDATA, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_WRITEDATA, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_READDATA, NULL);

        curl_easy_setopt(pd->easy, CURLOPT_OPENSOCKETFUNCTION, NULL);
        curl_easy_setopt(pd->easy, CURLOPT_OPENSOCKETDATA, NULL);

        curl_easy_setopt(pd->easy, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(pd->easy, CURLOPT_VERBOSE, 0L);

        pd->easy = NULL;
     }
   else if (pd->pending_close)
     {
        eina_promise_cancel(pd->pending_close);
        pd->pending_close = NULL;
        efl_io_closer_close(o);
     }
   else if (efl_io_closer_close_on_destructor_get(o) &&
            (!efl_io_closer_closed_get(o)))
     efl_io_closer_close(o);

   efl_net_dialer_http_response_headers_clear(o);

   if (pd->easy)
     {
        DBG("cleanup curl=%p", pd->easy);
        curl_easy_cleanup(pd->easy);
        pd->easy = NULL;
     }

   efl_destructor(efl_super(o, MY_CLASS));

   if (pd->recv.bytes)
     {
        free(pd->recv.bytes);
        pd->recv.bytes = NULL;
     }

   if (pd->request.headers)
     {
        curl_slist_free_all(pd->request.headers);
        pd->request.headers = NULL;
     }

   eina_stringshare_replace(&pd->address_dial, NULL);
   eina_stringshare_replace(&pd->proxy, NULL);
   eina_stringshare_replace(&pd->cookie_jar, NULL);
   eina_stringshare_replace(&pd->address_local, NULL);
   eina_stringshare_replace(&pd->address_remote, NULL);
   eina_stringshare_replace(&pd->method, NULL);
   eina_stringshare_replace(&pd->user_agent, NULL);
   eina_stringshare_replace(&pd->response.content_type, NULL);
   eina_stringshare_replace(&pd->authentication.username, NULL);
   _secure_free(&pd->authentication.password);
}

EOLIAN static Eina_Error
_efl_net_dialer_http_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *address)
{
   Efl_Net_Dialer_Http_Curlm *cm;
   CURLcode r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->method, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->cm != NULL, EALREADY);

   pd->pending_headers_done = EINA_FALSE;

   efl_net_dialer_address_dial_set(o, address);

   r = curl_easy_setopt(pd->easy, CURLOPT_HTTPHEADER, pd->request.headers);
   if (r != CURLE_OK)
     {
        ERR("dialer=%p could not set HTTP headers: %s",
            o, curl_easy_strerror(r));
        return EINVAL;
     }

   // TODO: move this to be per-loop once multiple mainloops are supported
   // this would need to attach something to the loop
   cm = &_cm_global;
   if (!cm->loop) cm->loop = efl_loop_user_loop_get(o);
   if (!_efl_net_dialer_http_curlm_add(cm, o, pd->easy))
     {
        ERR("dialer=%p could not add curl easy handle to multi manager", o);
        return ENOSYS;
     }

   pd->cm = cm;

   DBG("HTTP dialer=%p, curl_easy=%p, address='%s'",
       o, pd->easy, pd->address_dial);

   return 0;
}

EOLIAN static void
_efl_net_dialer_http_efl_net_dialer_address_dial_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *address)
{
   CURLcode r;

   r = curl_easy_setopt(pd->easy, CURLOPT_URL, address);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set HTTP URL '%s': %s",
         o, address, curl_easy_strerror(r));

   eina_stringshare_replace(&pd->address_dial, address);
}

EOLIAN static const char *
_efl_net_dialer_http_efl_net_dialer_address_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->address_dial;
}

EOLIAN static void
_efl_net_dialer_http_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Bool connected)
{
   /* always set and emit connected...
    * allow_redirects will trigger more than once
    */
   pd->connected = connected;
   if (connected) efl_event_callback_call(o, EFL_NET_DIALER_EVENT_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_net_dialer_connected_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->connected;
}

EOLIAN static void
_efl_net_dialer_http_efl_net_dialer_proxy_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, const char *proxy_url)
{
   CURLcode r;

   r = curl_easy_setopt(pd->easy, CURLOPT_PROXY, proxy_url);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set proxy to '%s': %s",
         o, proxy_url, curl_easy_strerror(r));

   eina_stringshare_replace(&pd->proxy, proxy_url);
}

EOLIAN static const char *
_efl_net_dialer_http_efl_net_dialer_proxy_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->proxy;
}

EOLIAN static void
_efl_net_dialer_http_efl_net_dialer_timeout_dial_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, double seconds)
{
   CURLcode r;

   r = curl_easy_setopt(pd->easy, CURLOPT_CONNECTTIMEOUT_MS,
                        (long)(seconds * 1000));
   if (r != CURLE_OK)
     ERR("dialer=%p could not connection timeout %f seconds: %s",
         o, seconds, curl_easy_strerror(r));

   pd->timeout_dial = seconds;
}

EOLIAN static double
_efl_net_dialer_http_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static void
_efl_net_dialer_http_efl_net_socket_address_local_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_local, address);
}

EOLIAN static const char *
_efl_net_dialer_http_efl_net_socket_address_local_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->address_local;
}

EOLIAN static void
_efl_net_dialer_http_efl_net_socket_address_remote_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *address)
{
   if (eina_stringshare_replace(&pd->address_remote, address))
     efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
}

EOLIAN static const char *
_efl_net_dialer_http_efl_net_socket_address_remote_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->address_remote;
}

static Eina_Error
_efl_net_dialer_http_pause_reset(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   CURLcode re;
   CURLMcode rm;
   Eina_Error err;

   re = curl_easy_pause(pd->easy, pd->pause);
   if (re != CURLE_OK)
     {
        err = _curlcode_to_eina_error(re);
        ERR("dialer=%p could not unpause receive (flags=%#x): %s",
            o, pd->pause, eina_error_msg_get(err));
        return err;
     }

   rm = curl_multi_socket_action(pd->cm->multi, CURL_SOCKET_TIMEOUT, 0, &pd->cm->running);
   if (rm != CURLM_OK)
     {
        err = _curlmcode_to_eina_error(rm);
        ERR("dialer=%p could trigger timeout action: %s",
            o, eina_error_msg_get(err));
        return err;
     }

   return 0;
}

EOLIAN static Eina_Error
_efl_net_dialer_http_efl_io_reader_read(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Slice ro_slice;
   size_t remaining;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);

   ro_slice.len = pd->recv.used;
   if (ro_slice.len == 0)
     {
        rw_slice->len = 0;
        return EAGAIN;
     }
   ro_slice.bytes = pd->recv.bytes;

   *rw_slice = eina_rw_slice_copy(*rw_slice, ro_slice);

   remaining = pd->recv.used - rw_slice->len;
   if (remaining)
     memmove(pd->recv.bytes, pd->recv.bytes + rw_slice->len, remaining);

   pd->recv.used = remaining;
   efl_io_reader_can_read_set(o, remaining > 0);

   if ((pd->pause & CURLPAUSE_RECV) && (pd->recv.used < pd->recv.limit))
     {
        Eina_Error err;
        pd->pause &= ~CURLPAUSE_RECV;
        err = _efl_net_dialer_http_pause_reset(o, pd);
        if (err)
          return err;
     }

   return 0;
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_reader_can_read_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_net_dialer_http_efl_io_reader_can_read_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_reader_eos_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->eos;
}

EOLIAN static void
_efl_net_dialer_http_efl_io_reader_eos_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Bool is_eos)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (is_eos) pd->connected = EINA_FALSE;
   if (is_eos)
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Error
_efl_net_dialer_http_efl_io_writer_write(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   Eina_Error err = EINVAL;
   CURLMcode rm;

   EINA_SAFETY_ON_NULL_RETURN_VAL(slice, EINVAL);
   EINA_SAFETY_ON_TRUE_GOTO(efl_io_closer_closed_get(o), error);
   err = EBUSY;
   EINA_SAFETY_ON_TRUE_GOTO(pd->send.slice.mem != NULL, error);

   pd->send.slice = *slice;
   efl_io_writer_can_write_set(o, EINA_FALSE);
   pd->pause &= ~CURLPAUSE_SEND;
   err = _efl_net_dialer_http_pause_reset(o, pd);
   if (err) goto error;

   pd->error = 0;
   rm = curl_multi_socket_action(pd->cm->multi,
                                 pd->fd,
                                 CURL_CSELECT_OUT, &pd->cm->running);
   if (rm != CURLM_OK)
     {
        err = _curlcode_to_eina_error(rm);
        ERR("dialer=%p could not trigger socket=%d (fdhandler=%p) action: %s",
            o, pd->fd, pd->fdhandler, eina_error_msg_get(err));
        goto error;
     }
   _efl_net_dialer_http_curlm_check(pd->cm);
   if (pd->error) return pd->error;

   if (remaining) *remaining = pd->send.slice;
   slice->len -= pd->send.slice.len;
   slice->bytes += pd->send.slice.len;
   pd->send.slice.mem = NULL;
   pd->send.slice.len = 0;

   if (slice->len == 0)
     return EAGAIN;

   return 0;

 error:
   if (remaining) *remaining = *slice;
   slice->len = 0;
   slice->mem = NULL;
   pd->send.slice.mem = NULL;
   pd->send.slice.len = 0;
   return err;
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_writer_can_write_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_net_dialer_http_efl_io_writer_can_write_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Bool can_write)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

static void _efl_net_dialer_http_pending_close(void *data, void *value);

EOLIAN static Eina_Error
_efl_net_dialer_http_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   Eina_Error err = 0;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);

   pd->fd = -1;

   if (pd->in_curl_callback)
     {
        if (!pd->pending_close)
          {
             pd->pending_close = efl_loop_job(efl_loop_user_loop_get(o), o);
             eina_promise_then(pd->pending_close, _efl_net_dialer_http_pending_close, NULL, o);
             DBG("dialer=%p closed from CURL callback, schedule close job=%p", o, pd->pending_close);
          }
        return 0;
     }

   if (!pd->easy) goto end;

   if (pd->cm)
     {
        DBG("close dialer=%p, cm=%p, easy=%p", o, pd->cm, pd->easy);
        _efl_net_dialer_http_curlm_remove(pd->cm, o, pd->easy);
        pd->cm = NULL;
     }
   if (pd->fdhandler)
     {
        ERR("dialer=%p fdhandler=%p still alive!", o, pd->fdhandler);
        ecore_main_fd_handler_del(pd->fdhandler);
        pd->fdhandler = NULL;
     }

 end:
   efl_io_writer_can_write_set(o, EINA_FALSE);
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);
   efl_net_dialer_connected_set(o, EINA_FALSE);
   pd->closed = EINA_TRUE;
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
   return err;
}

static void
_efl_net_dialer_http_pending_close(void *data, void *value EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Dialer_Http_Data *pd = efl_data_scope_get(o, MY_CLASS);

   pd->pending_close = NULL;
   _efl_net_dialer_http_efl_io_closer_close(o, pd);
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_closer_closed_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->closed || (!!pd->pending_close);
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, Eina_Bool close_on_exec)
{
#ifdef _WIN32
   DBG("close on exec is not supported on windows");
   pd->close_on_exec = close_on_exec;
   return EINA_FALSE;
#else
   int flags;
   Eina_Bool old = pd->close_on_exec;

   pd->close_on_exec = close_on_exec;

   if (pd->fd < 0) return EINA_TRUE; /* postpone until _efl_net_dialer_http_socket_open */

   flags = fcntl(pd->fd, F_GETFD);
   if (flags < 0)
     {
        ERR("fcntl(%d, F_GETFD): %s", pd->fd, strerror(errno));
        pd->close_on_exec = old;
        return EINA_FALSE;
     }
   if (close_on_exec)
     flags |= FD_CLOEXEC;
   else
     flags &= (~FD_CLOEXEC);
   if (fcntl(pd->fd, F_SETFD, flags) < 0)
     {
        ERR("fcntl(%d, F_SETFD, %#x): %s", pd->fd, flags, strerror(errno));
        pd->close_on_exec = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
#endif
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_closer_close_on_exec_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->close_on_exec;
}

EOLIAN static void
_efl_net_dialer_http_efl_io_closer_close_on_destructor_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, Eina_Bool close_on_destructor)
{
   pd->close_on_destructor = close_on_destructor;
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_efl_io_closer_close_on_destructor_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->close_on_destructor;
}

EOLIAN static Eina_Error
_efl_net_dialer_http_efl_io_sizer_resize(Eo *o, Efl_Net_Dialer_Http_Data *pd, uint64_t size)
{
   Efl_Net_Dialer_Http_Primary_Mode pm;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(size > INT64_MAX, ERANGE);

   pm = _efl_net_dialer_http_primary_mode_effective_get(pd);
   if (pm == EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD)
     {
        efl_net_dialer_http_request_content_length_set(o, size);
        return 0;
     }
   else
     {
        ERR("dialer=%p cannot resize when EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD", o);
        return EPERM;
     }
}

EOLIAN static uint64_t
_efl_net_dialer_http_efl_io_sizer_size_get(Eo *o, Efl_Net_Dialer_Http_Data *pd)
{
   Efl_Net_Dialer_Http_Primary_Mode pm;
   int64_t len;

   pm = _efl_net_dialer_http_primary_mode_effective_get(pd);
   if (pm == EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD)
     len = efl_net_dialer_http_request_content_length_get(o);
   else
     len = efl_net_dialer_http_response_content_length_get(o);

   if (len < 0)
     return 0;
   return len;
}

static void
_efl_net_dialer_http_request_apply(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *method, Efl_Net_Dialer_Http_Primary_Mode primary_mode)
{
   CURLcode r;

   if (primary_mode == EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD)
     {
        if (strcasecmp(method, "PUT") == 0)
          r = curl_easy_setopt(pd->easy, CURLOPT_PUT, 1L);
        else
          {
             /* upload with non PUT method:
              *  1 - set CURLOPT_UPLOAD = 1, this forces httpreq = PUT
              *  2 - set CURLOPT_CUSTOMREQUEST = method, this overrides
              *      the string to send.
              */
             r = curl_easy_setopt(pd->easy, CURLOPT_UPLOAD, 1L);
             if (r != CURLE_OK)
               ERR("dialer=%p could not configure upload mode: %s",
                   o, curl_easy_strerror(r));

             r = curl_easy_setopt(pd->easy, CURLOPT_CUSTOMREQUEST, method);
          }
     }
   else
     {
        if (strcasecmp(method, "GET") == 0)
          r = curl_easy_setopt(pd->easy, CURLOPT_HTTPGET, 1L);
        else if (strcasecmp(method, "POST") == 0)
          r = curl_easy_setopt(pd->easy, CURLOPT_POST, 1L);
        else if (strcasecmp(method, "PUT") == 0)
          {
             if (primary_mode == EFL_NET_DIALER_HTTP_PRIMARY_MODE_AUTO)
               r = curl_easy_setopt(pd->easy, CURLOPT_PUT, 1L);
             else
               {
                  r = curl_easy_setopt(pd->easy, CURLOPT_UPLOAD, 0L);
                  if (r != CURLE_OK)
                    ERR("dialer=%p could not configure no-upload mode: %s",
                        o, curl_easy_strerror(r));

                  r = curl_easy_setopt(pd->easy, CURLOPT_CUSTOMREQUEST, method);
               }
          }
        else
          r = curl_easy_setopt(pd->easy, CURLOPT_CUSTOMREQUEST, method);
     }

   if (r != CURLE_OK)
     ERR("dialer=%p could not configure HTTP method: %s: %s",
         o, method, curl_easy_strerror(r));
}

EOLIAN static void
_efl_net_dialer_http_method_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *method)
{
   EINA_SAFETY_ON_NULL_RETURN(method);

   _efl_net_dialer_http_request_apply(o, pd, method, pd->primary_mode);
   eina_stringshare_replace(&pd->method, method);
}

EOLIAN static const char *
_efl_net_dialer_http_method_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->method;
}

EOLIAN static void
_efl_net_dialer_http_primary_mode_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Efl_Net_Dialer_Http_Primary_Mode primary_mode)
{
   _efl_net_dialer_http_request_apply(o, pd, pd->method, primary_mode);
   pd->primary_mode = primary_mode;
}

EOLIAN static Efl_Net_Dialer_Http_Primary_Mode
_efl_net_dialer_http_primary_mode_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return _efl_net_dialer_http_primary_mode_effective_get(pd);
}

EOLIAN static void
_efl_net_dialer_http_user_agent_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *user_agent)
{
   CURLcode r;

   r = curl_easy_setopt(pd->easy, CURLOPT_USERAGENT, user_agent);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set user-agent '%s': %s",
         o, user_agent, curl_easy_strerror(r));

   eina_stringshare_replace(&pd->user_agent, user_agent);
}

EOLIAN static const char *
_efl_net_dialer_http_user_agent_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->user_agent;
}

EOLIAN static void
_efl_net_dialer_http_http_version_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Efl_Net_Http_Version http_version)
{
   CURLcode r;

   r = curl_easy_setopt(pd->easy, CURLOPT_HTTP_VERSION,
                      _efl_net_http_version_to_curl(http_version));
   if (r != CURLE_OK)
     ERR("dialer=%p could not configure HTTP version code %d: %s",
         o, http_version, curl_easy_strerror(r));
   pd->version = http_version;
}

EOLIAN static Efl_Net_Http_Version
_efl_net_dialer_http_http_version_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->version;
}

EOLIAN static void
_efl_net_dialer_http_authentication_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *username, const char *password, Efl_Net_Http_Authentication_Method method, Eina_Bool restricted)
{
   CURLcode r;
   long flags;
   char *tmp;

   r = curl_easy_setopt(pd->easy, CURLOPT_USERNAME, username);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set username '%s': %s",
         o, username, curl_easy_strerror(r));

   r = curl_easy_setopt(pd->easy, CURLOPT_PASSWORD, password);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set password: %s", o, curl_easy_strerror(r));

   flags = _efl_net_http_authentication_method_to_curl(method, restricted);
   r = curl_easy_setopt(pd->easy, CURLOPT_HTTPAUTH, flags);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set HTTP authentication method %#x restricted %hhu (%#lx): %s",
         o, method, restricted, flags, curl_easy_strerror(r));

   eina_stringshare_replace(&pd->authentication.username, username);

   tmp = password ? strdup(password) : NULL;
   _secure_free(&pd->authentication.password);
   pd->authentication.password = tmp;

   pd->authentication.method = method;
   pd->authentication.restricted = restricted;
}

EOLIAN static void
_efl_net_dialer_http_authentication_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, const char **username, const char **password, Efl_Net_Http_Authentication_Method *method, Eina_Bool *restricted)
{
   if (username) *username = pd->authentication.username;
   if (password) *password = pd->authentication.password;
   if (method) *method = pd->authentication.method;
   if (restricted) *restricted = pd->authentication.restricted;
}

EOLIAN static void
_efl_net_dialer_http_allow_redirects_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, Eina_Bool allow_redirects)
{
   CURLcode r;

   r = curl_easy_setopt(pd->easy, CURLOPT_FOLLOWLOCATION, (long)allow_redirects);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set allow redirects %d: %s",
         o, allow_redirects, curl_easy_strerror(r));

   pd->allow_redirects = allow_redirects;
}

EOLIAN static Eina_Bool
_efl_net_dialer_http_allow_redirects_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->allow_redirects;
}

EOLIAN static Efl_Net_Http_Status
_efl_net_dialer_http_response_status_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->response.status;
}

EOLIAN static void
_efl_net_dialer_http_request_header_add(Eo *o, Efl_Net_Dialer_Http_Data *pd, const char *key, const char *value)
{
   char *s = NULL;

   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(value);

   if (asprintf(&s, "%s: %s", key, value) < 0)
     {
        ERR("dialer=%p could not allocate header string '%s: %s'",
            o, key, value);
        return;
     }

   pd->request.headers = curl_slist_append(pd->request.headers, s);
   free(s);
}

EOLIAN static void
_efl_net_dialer_http_request_headers_clear(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   curl_slist_free_all(pd->request.headers);
   pd->request.headers = NULL;
}

typedef struct _Eina_Iterator_Curl_Slist_Header
{
   Eina_Iterator iterator;
   const struct curl_slist *head;
   const struct curl_slist *current;
   Efl_Net_Http_Header header;
   char *mem;
} Eina_Iterator_Curl_Slist_Header;

static Eina_Bool
eina_iterator_curl_slist_header_next(Eina_Iterator_Curl_Slist_Header *it, void **data)
{
   char *p;

   if (!it->current)
     return EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(it->current->data, EINA_FALSE);

   free(it->mem);
   it->mem = strdup(it->current->data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(it->mem, EINA_FALSE);

   it->header.key = it->mem;

   p = strchr(it->mem, ':');
   if (!p)
     it->header.value = "";
   else
     {
        p[0] = '\0';
        p++;
        while ((p[0] != '\0') && (isspace(p[0])))
          p++;

        it->header.value = p;
     }

   *data = &it->header;
   it->current = it->current->next;
   return EINA_TRUE;
}

static const struct curl_slist *
eina_iterator_curl_slist_header_get_container(Eina_Iterator_Curl_Slist_Header *it)
{
   return it->head;
}

static void
eina_iterator_curl_slist_header_free(Eina_Iterator_Curl_Slist_Header *it)
{
   free(it->mem);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_net_dialer_http_request_headers_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   Eina_Iterator_Curl_Slist_Header *it;

   it = calloc(1, sizeof(Eina_Iterator_Curl_Slist_Header));
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   it->head = pd->request.headers;
   it->current = it->head;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(eina_iterator_curl_slist_header_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_iterator_curl_slist_header_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_iterator_curl_slist_header_free);

   return &it->iterator;
}

EOLIAN static void
_efl_net_dialer_http_request_content_length_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, int64_t length)
{
   Efl_Net_Dialer_Http_Primary_Mode pm;
   CURLcode r;

   if (strcmp(pd->method, "POST") == 0)
     r = curl_easy_setopt(pd->easy, CURLOPT_POSTFIELDSIZE_LARGE, length);
   else
     r = curl_easy_setopt(pd->easy, CURLOPT_INFILESIZE_LARGE, length);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set file size %" PRId64 ": %s",
         o, length, curl_easy_strerror(r));

   pd->request.content_length = length;
   if (length < 0)
     return;

   pm = _efl_net_dialer_http_primary_mode_effective_get(pd);
   if (pm == EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD)
     efl_event_callback_call(o, EFL_IO_SIZER_EVENT_SIZE_CHANGED, NULL);
}

EOLIAN static int64_t
_efl_net_dialer_http_request_content_length_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->request.content_length;
}

EOLIAN static void
_efl_net_dialer_http_response_content_length_set(Eo *o, Efl_Net_Dialer_Http_Data *pd, int64_t length)
{
   Efl_Net_Dialer_Http_Primary_Mode pm;

   pd->response.content_length = length;
   if (length < 0)
     return;

   pm = _efl_net_dialer_http_primary_mode_effective_get(pd);
   if (pm == EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD)
     efl_event_callback_call(o, EFL_IO_SIZER_EVENT_SIZE_CHANGED, NULL);
}

EOLIAN static int64_t
_efl_net_dialer_http_response_content_length_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->response.content_length;
}

EOLIAN static void
_efl_net_dialer_http_response_content_type_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, const char *content_type)
{
   eina_stringshare_replace(&pd->response.content_type, content_type);
}

EOLIAN static const char *
_efl_net_dialer_http_response_content_type_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->response.content_type;
}

EOLIAN static Eina_Iterator *
_efl_net_dialer_http_response_headers_all_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return eina_list_iterator_new(pd->response.headers);
}

EOLIAN static Eina_Iterator *
_efl_net_dialer_http_response_headers_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   const Eina_List *lst = pd->response.last_request_header;

   if (lst) lst = lst->next;
   else lst = pd->response.headers;

   return eina_list_iterator_new(lst);
}

EOLIAN static void
_efl_net_dialer_http_response_headers_clear(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   void *mem;

   EINA_LIST_FREE(pd->response.headers, mem)
     free(mem); /* key and value are in the same memory */

   pd->response.last_request_header = NULL;
}

EOLIAN static void
_efl_net_dialer_http_progress_download_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, uint64_t *now, uint64_t *total)
{
   if (now) *now = pd->progress.download.now;
   if (total) *total = pd->progress.download.total;
}

EOLIAN static void
_efl_net_dialer_http_progress_upload_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, uint64_t *now, uint64_t *total)
{
   if (now) *now = pd->progress.upload.now;
   if (total) *total = pd->progress.upload.total;
}

EOLIAN static void
_efl_net_dialer_http_cookie_jar_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd, const char *path)
{
   CURLcode r;

   if (pd->cookie_jar)
     {
        r = curl_easy_setopt(pd->easy, CURLOPT_COOKIELIST, "FLUSH");
        if (r != CURLE_OK)
          ERR("dialer=%p could not flush cookie_jar to '%s': %s",
              o, pd->cookie_jar, curl_easy_strerror(r));

        r = curl_easy_setopt(pd->easy, CURLOPT_COOKIELIST, "ALL");
        if (r != CURLE_OK)
          ERR("dialer=%p could not empty cookie_jar: %s",
              o, curl_easy_strerror(r));

        r = curl_easy_setopt(pd->easy, CURLOPT_COOKIEFILE, NULL);
        if (r != CURLE_OK)
          ERR("dialer=%p could not unset cookie_jar (read): %s",
              o, curl_easy_strerror(r));

        r = curl_easy_setopt(pd->easy, CURLOPT_COOKIEJAR, NULL);
        if (r != CURLE_OK)
          ERR("dialer=%p could not unset cookie_jar (write): %s",
              o, curl_easy_strerror(r));
     }

   if (!path) goto end;

   r = curl_easy_setopt(pd->easy, CURLOPT_COOKIEFILE, path);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set cookie_jar (read) to '%s': %s",
         o, path, curl_easy_strerror(r));

   r = curl_easy_setopt(pd->easy, CURLOPT_COOKIEJAR, path);
   if (r != CURLE_OK)
     ERR("dialer=%p could not set cookie_jar (write) to '%s': %s",
         o, path, curl_easy_strerror(r));

 end:
   eina_stringshare_replace(&pd->cookie_jar, path);
}

EOLIAN static const char *
_efl_net_dialer_http_cookie_jar_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Http_Data *pd)
{
   return pd->cookie_jar;
}

#include "efl_net_dialer_http.eo.c"
