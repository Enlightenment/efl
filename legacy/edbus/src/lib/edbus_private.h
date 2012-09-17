#ifndef EDBUS_PRIVATE_H
#define EDBUS_PRIVATE_H

#include <Eina.h>
#include "eina_safety_checks.h"
#include "EDBus.h"
#include "edbus_private_types.h"

extern int _edbus_log_dom;
#define DBG(...)      EINA_LOG_DOM_DBG(_edbus_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_edbus_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_edbus_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_edbus_log_dom, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_edbus_log_dom, __VA_ARGS__)

#define EDBUS_CONNECTION_MAGIC        (0xdb050001)
#define EDBUS_MESSAGE_MAGIC           (0xdb050002)
#define EDBUS_SIGNAL_HANDLER_MAGIC    (0xdb050003)
#define EDBUS_PENDING_MAGIC           (0xdb050004)
#define EDBUS_OBJECT_MAGIC            (0xdb050005)
#define EDBUS_PROXY_MAGIC             (0xdb050006)
#define EDBUS_MESSAGE_ITERATOR_MAGIC  (0xdb050007)
#define EDBUS_SERVICE_INTERFACE_MAGIC (0xdb050008)

void                   edbus_cbs_free_dispatch(Eina_Inlist **p_lst, const void *dead_pointer);
Eina_Inlist           *edbus_cbs_free_add(Eina_Inlist *lst, EDBus_Free_Cb cb, const void *data);
Eina_Inlist           *edbus_cbs_free_del(Eina_Inlist *lst, EDBus_Free_Cb cb, const void *data);

void                   edbus_data_set(Eina_Inlist **p_lst, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
void                  *edbus_data_get(Eina_Inlist **p_lst, const char *key) EINA_ARG_NONNULL(1, 2);
void                  *edbus_data_del(Eina_Inlist **p_lst, const char *key) EINA_ARG_NONNULL(1, 2);
void                   edbus_data_del_all(Eina_Inlist **p_list) EINA_ARG_NONNULL(1);

Eina_Bool              edbus_message_init(void);
void                   edbus_message_shutdown(void);
EDBus_Message         *edbus_message_new(Eina_Bool writable);

Eina_Bool              edbus_signal_handler_init(void);
void                   edbus_signal_handler_shutdown(void);

Eina_Bool              edbus_pending_init(void);
void                   edbus_pending_shutdown(void);

Eina_Bool              edbus_object_init(void);
void                   edbus_object_shutdown(void);

Eina_Bool              edbus_proxy_init(void);
void                   edbus_proxy_shutdown(void);

Eina_Bool              edbus_service_init(void);
void                   edbus_service_shutdown(void);

void                   edbus_connection_event_callback_call(EDBus_Connection *conn, EDBus_Connection_Event_Type type, const void *event_info) EINA_ARG_NONNULL(1);

Eina_Bool              edbus_object_proxy_del(EDBus_Object *obj, EDBus_Proxy *proxy, const char *interface) EINA_ARG_NONNULL(1, 2);

void                   edbus_connection_signal_handler_add(EDBus_Connection *conn, EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1, 2);
void                   edbus_connection_pending_add(EDBus_Connection *conn, EDBus_Pending *pending) EINA_ARG_NONNULL(1, 2);
void                   edbus_connection_signal_handler_del(EDBus_Connection *conn, EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1, 2);
void                   edbus_connection_signal_handler_del(EDBus_Connection *conn, EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1, 2);
void                   edbus_connection_pending_del(EDBus_Connection *conn, EDBus_Pending *pending) EINA_ARG_NONNULL(1, 2);

EDBus_Object          *edbus_connection_name_object_get(EDBus_Connection *conn, const char *name, const char *path);
void                   edbus_connection_name_object_set(EDBus_Connection *conn, EDBus_Object *obj);

Eina_Bool              edbus_object_proxy_add(EDBus_Object *obj, EDBus_Proxy *proxy) EINA_ARG_NONNULL(1, 2);
EDBus_Proxy           *edbus_object_proxy_get(EDBus_Object *obj, const char *interface);

void                   edbus_connection_name_object_del(EDBus_Connection *conn, const EDBus_Object *obj);
EDBus_Connection_Name *edbus_connection_name_get(EDBus_Connection *conn, const char *name);
void                   edbus_connection_name_owner_monitor(EDBus_Connection *conn, EDBus_Connection_Name *cn, Eina_Bool enable);

EDBus_Pending         *_edbus_connection_send(EDBus_Connection *conn, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout);

#ifdef HAVE_VA_LIST_AS_ARRAY
#define MAKE_PTR_FROM_VA_LIST(arg) ((va_list *)(arg))
#else
#define MAKE_PTR_FROM_VA_LIST(arg) (&(arg))
#endif

#endif
