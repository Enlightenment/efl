#include "Ecore.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void                ecore_ipc_init(char *path);
void                ecore_ipc_cleanup(void);
static void         ecore_ipc_connect_handler(int fd);
static void         ecore_ipc_client_handler(int fd);
void                ecore_ipc_get_data(int fd, void *buf);
void                ecore_ipc_send_data(int fd, void *buf, int size);
void                ecore_add_ipc_service(int service, void (*func) (int fd));
void                ecore_del_ipc_service(int service);

Ecore_Event_Ipc_Service *ipc_services = NULL;

void
ecore_ev_ipc_init(char *path)
{
  int                 fd, len;
  struct sockaddr_un  saun;

  if (path == NULL)
    return;

  /* a UNIX domain, stream socket */
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      printf("Cannot create ipc socket... disabling ipc.\n");
      return;
    }

  /* create the address we will be binding to */
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, path);

  /* unlink the address so the bind won't fail */
  unlink(path);
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  if (bind(fd, (struct sockaddr *)&saun, len) < 0)
    {
      printf("Cannot bind ipc socket... disabling ipc.\n");
      return;
    }

  /* listen on the socket */
  if (listen(fd, 5) < 0)
    {
      printf("Cannot listen on ipc socket... disabling ipc.\n");
      return;
    }

  /* add ipc listener */
  ecore_add_event_ipc(fd, ecore_ipc_connect_handler);
}

void
ecore_ev_ipc_cleanup(void)
{
  Ecore_Event_Ipc_Service *ipc_s;

  /* cleanup services list */
  for (ipc_s = ipc_services; ipc_s; ipc_s = ipc_s->next)
    {
      ecore_del_ipc_service(ipc_s->service);
    }
}

static void
ecore_ipc_connect_handler(int fd)
{
  struct sockaddr_un  fsaun;
  int                 fromlen, nfd;

  /* accept ipc connection */
  fromlen = sizeof(fsaun);

  if ((nfd = accept(fd, (struct sockaddr *)&fsaun, &fromlen)) < 0)
    {
      printf("Cannot accept ipc connection... ignoring connection attempt.\n");
      return;
    }

  /* add ipc client */
  ecore_add_event_ipc(nfd, ecore_ipc_client_handler);
}

static void
ecore_ipc_client_handler(int fd)
{
  int                 nread, service;
  Ecore_Event_Ipc_Service *ipc_s;

  if ((nread = read(fd, &service, sizeof(service))) == 0)
    {
      close(fd);
      ecore_del_event_ipc(fd);
    }
  else if (nread > 0)
    {
      /* call the service function */
      for (ipc_s = ipc_services; ipc_s; ipc_s = ipc_s->next)
	{
	  if (ipc_s->service == service)
	    {
	      ipc_s->func(fd);
	      break;
	    }
	}
    }
  else
    {
      printf("ipc error in read service.\n");
      fflush(stdout);
    }
}

void
ecore_ipc_get_data(int fd, void *buf)
{
  int                 readn, nread;

  /* read number of bytes being sent */
  if ((nread = read(fd, &readn, sizeof(readn))) == -1)
    {
      printf("ipc error in get data.\n");
      fflush(stdout);
      return;
    }

  /* get data structure */
  if ((nread = read(fd, buf, readn)) == -1)
    {
      printf("ipc error in get data.\n");
      fflush(stdout);
      return;
    }
}

void
ecore_ipc_send_data(int fd, void *buf, int size)
{
  int                 n;

  /* send length of data being sent */
  if ((n = write(fd, &size, sizeof(size))) == -1)
    {
      printf("ipc error in send data length.\n");
      fflush(stdout);
      return;
    }

  /* send data */
  if ((n = write(fd, buf, size)) == -1)
    {
      printf("ipc error in send data.\n");
      fflush(stdout);
      return;
    }
}

void
ecore_add_ipc_service(int service, void (*func) (int fd))
{
  Ecore_Event_Ipc_Service *ipc_s;

  /* delete the old service */
  ecore_del_ipc_service(service);
  /* new service struct */
  ipc_s = NEW(Ecore_Event_Ipc_Service, 1);
  ipc_s->next = ipc_services;
  ipc_s->service = service;
  ipc_s->func = func;
  ipc_services = ipc_s;
}

void
ecore_del_ipc_service(int service)
{
  START_LIST_DEL(Ecore_Event_Ipc_Service, ipc_services,
		 (_p->service == service));
  FREE(_p);
  END_LIST_DEL;
}
