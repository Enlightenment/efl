#ifndef _ELDBUS_FAKE_SERVER_H
#define _ELDBUS_FAKE_SERVER_H

#include <Eldbus.h>

#define FAKE_SERVER_BUS "org.Enlightenment.Test"
#define FAKE_SERVER_PATH "/org/enlightenment"
#define FAKE_SERVER_INTERFACE "org.enlightenment.FakeServer"
#define FAKE_SERVER_READONLY_PROPERTY "r"
#define FAKE_SERVER_WRITEONLY_PROPERTY "w"
#define FAKE_SERVER_READWRITE_PROPERTY "rw"
#define FAKE_SERVER_SUM_METHOD_NAME "Sum"
#define FAKE_SERVER_PING_METHOD_NAME "Ping"
#define FAKE_SERVER_PONG_SIGNAL_NAME "Pong"

typedef struct _Fake_Server_Data Fake_Server_Data;

struct _Fake_Server_Data
{
   int readonly_property;
   int writeonly_property;
   int readwrite_property;
   int pong_response;
};

Eldbus_Service_Interface *fake_server_start(Fake_Server_Data *data, const char *name);
void fake_server_stop(Eldbus_Service_Interface *interface);

#endif
