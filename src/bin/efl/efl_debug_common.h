#ifndef EFL_DEBUG_COMMON_H
#define EFL_DEBUG_COMMON_H 1

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void _protocol_collect(unsigned char **buf, unsigned int *buf_size,
                       void *data, int size);
int _proto_read(unsigned char **buf, unsigned int *buf_size,
                char *op, unsigned char **data);

#define fetch_val(dst, buf, off) \
   memcpy(&dst, ((unsigned char *)buf) + off, sizeof(dst))
#define store_val(buf, off, src) \
   memcpy(buf + off, &src, sizeof(src))
#define send_svr(svr, op, data, size) \
   do { \
      unsigned char head[8]; \
      char *op2 = op; \
      int size2 = size + 4; \
      memcpy(head + 0, &size2, 4); \
      memcpy(head + 4, op2, 4); \
      ecore_con_server_send(svr, head, 8); \
      if (size > 0) ecore_con_server_send(svr, data, size); \
   } while (0)
#define send_cli(cli, op, data, size) \
   do { \
      unsigned char head[8]; \
      char *op2 = op; \
      int size2 = size + 4; \
      memcpy(head + 0, &size2, 4); \
      memcpy(head + 4, op2, 4); \
      ecore_con_client_send(cli, head, 8); \
      if (size > 0) ecore_con_client_send(cli, data, size); \
   } while (0)

#endif
