#ifndef ELDBUS_PRIVATE_H
#define ELDBUS_PRIVATE_H

#include <dbus/dbus.h>
#include <Eina.h>
#include "eina_safety_checks.h"
#include <Ecore.h>
#include "Eldbus.h"
#include "eldbus_private_types.h"

extern int _eldbus_log_dom;
#define DBG(...)      EINA_LOG_DOM_DBG(_eldbus_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_eldbus_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_eldbus_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_eldbus_log_dom, __VA_ARGS__)
#define CRI(...)      EINA_LOG_DOM_CRIT(_eldbus_log_dom, __VA_ARGS__)

#define ELDBUS_CONNECTION_MAGIC        (0xdb050001)
#define ELDBUS_MESSAGE_MAGIC           (0xdb050002)
#define ELDBUS_SIGNAL_HANDLER_MAGIC    (0xdb050003)
#define ELDBUS_PENDING_MAGIC           (0xdb050004)
#define ELDBUS_OBJECT_MAGIC            (0xdb050005)
#define ELDBUS_PROXY_MAGIC             (0xdb050006)
#define ELDBUS_MESSAGE_ITERATOR_MAGIC  (0xdb050007)
#define ELDBUS_SERVICE_INTERFACE_MAGIC (0xdb050008)

void                   eldbus_cbs_free_dispatch(Eina_Inlist **p_lst, const void *dead_pointer);
Eina_Inlist           *eldbus_cbs_free_add(Eina_Inlist *lst, Eldbus_Free_Cb cb, const void *data);
Eina_Inlist           *eldbus_cbs_free_del(Eina_Inlist *lst, Eldbus_Free_Cb cb, const void *data);

void                   eldbus_data_set(Eina_Inlist **p_lst, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
void                  *eldbus_data_get(Eina_Inlist **p_lst, const char *key) EINA_ARG_NONNULL(1, 2);
void                  *eldbus_data_del(Eina_Inlist **p_lst, const char *key) EINA_ARG_NONNULL(1, 2);
void                   eldbus_data_del_all(Eina_Inlist **p_list) EINA_ARG_NONNULL(1);

Eina_Bool              eldbus_message_init(void);
void                   eldbus_message_shutdown(void);
Eldbus_Message         *eldbus_message_new(Eina_Bool writable);

Eina_Bool              eldbus_signal_handler_init(void);
void                   eldbus_signal_handler_shutdown(void);

Eina_Bool              eldbus_pending_init(void);
void                   eldbus_pending_shutdown(void);

Eina_Bool              eldbus_object_init(void);
void                   eldbus_object_shutdown(void);

Eina_Bool              eldbus_proxy_init(void);
void                   eldbus_proxy_shutdown(void);

Eina_Bool              eldbus_service_init(void);
void                   eldbus_service_shutdown(void);

void                   eldbus_connection_event_callback_call(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, const void *event_info) EINA_ARG_NONNULL(1);

Eina_Bool              eldbus_object_proxy_del(Eldbus_Object *obj, Eldbus_Proxy *proxy, const char *interface) EINA_ARG_NONNULL(1, 2);

void                   eldbus_connection_signal_handler_add(Eldbus_Connection *conn, Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1, 2);
void                   eldbus_connection_pending_add(Eldbus_Connection *conn, Eldbus_Pending *pending) EINA_ARG_NONNULL(1, 2);
void                   eldbus_connection_signal_handler_del(Eldbus_Connection *conn, Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1, 2);
void                   eldbus_connection_signal_handler_del(Eldbus_Connection *conn, Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1, 2);
void                   eldbus_connection_pending_del(Eldbus_Connection *conn, Eldbus_Pending *pending) EINA_ARG_NONNULL(1, 2);

Eldbus_Object          *eldbus_connection_name_object_get(Eldbus_Connection *conn, const char *name, const char *path);
void                   eldbus_connection_name_object_set(Eldbus_Connection *conn, Eldbus_Object *obj);

Eina_Bool              eldbus_object_proxy_add(Eldbus_Object *obj, Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1, 2);
Eldbus_Proxy           *eldbus_object_proxy_get(Eldbus_Object *obj, const char *interface);

void                   eldbus_connection_name_object_del(Eldbus_Connection *conn, const Eldbus_Object *obj);
Eldbus_Connection_Name *eldbus_connection_name_get(Eldbus_Connection *conn, const char *name);
void                   eldbus_connection_name_owner_monitor(Eldbus_Connection *conn, Eldbus_Connection_Name *cn, Eina_Bool enable);

Eldbus_Pending         *_eldbus_connection_send(Eldbus_Connection *conn, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout);
Eldbus_Message         *_eldbus_connection_send_and_block(Eldbus_Connection *conn, Eldbus_Message *msg, double timeout);

Eldbus_Message_Iter    *eldbus_message_iter_sub_iter_get(Eldbus_Message_Iter *iter);
Eina_Value             *_message_iter_struct_to_eina_value(Eldbus_Message_Iter *iter);
Eina_Bool              _message_iter_from_eina_value(const char *signature, Eldbus_Message_Iter *iter, const Eina_Value *value);
Eina_Bool              _message_iter_from_eina_value_struct(const char *signature, Eldbus_Message_Iter *iter, const Eina_Value *value);

void                   eldbus_connection_name_ref(Eldbus_Connection_Name *cn);
void                   eldbus_connection_name_unref(Eldbus_Connection *conn, Eldbus_Connection_Name *cn);
Eldbus_Signal_Handler  *_eldbus_signal_handler_add(Eldbus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, Eldbus_Signal_Cb cb, const void *cb_data);
Eldbus_Message         *eldbus_message_signal_new(const char *path, const char *interface, const char *name) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

const Eina_Value_Type  *_dbus_type_to_eina_value_type(char type);

#endif
