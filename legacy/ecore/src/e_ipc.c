#include "Ecore.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void e_ipc_init(char *path);
void e_ipc_cleanup(void);
static void e_ipc_connect_handler(int fd);
static void e_ipc_client_handler(int fd);
void e_add_ipc_service(int service, char *(*func) (char *argv));
void e_del_ipc_service(int service);

Ev_Ipc_Service     *ipc_services = NULL;

void
e_ev_ipc_init(char *path)
{
  int fd, len;
  struct sockaddr_un saun;

  if(path == NULL)
    return;

  /* a UNIX domain, stream socket */
  if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    printf("Cannot create ipc socket... disabling ipc.\n");
    return;
  }

  /* create the address we will be binding to */
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, path);

  /* unlink the address so the bind won't fail */
  unlink(path);
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  if(bind(fd, &saun, len) < 0) {
     printf("Cannot bind ipc socket... disabling ipc.\n");
    return;
  }

  /* listen on the socket */
  if (listen(fd, 5) < 0) {
    printf("Cannot listen on ipc socket... disabling ipc.\n");
    return;
  }

  /* add ipc listener */
  e_add_event_ipc(fd, e_ipc_connect_handler);
}

void 
e_ev_ipc_cleanup(void)
{
  Ev_Ipc_Service     *ipc_s;

  /* cleanup services list */
  for (ipc_s = ipc_services; ipc_s; ipc_s = ipc_s->next)
     {
       e_del_ipc_service(ipc_s->service);
     }
}

static void
e_ipc_connect_handler(int fd)
{
  struct sockaddr_un fsaun;
  int fromlen, nfd;

  /* accept ipc connection */
  fromlen = sizeof(fsaun);

  if ((nfd = accept(fd, &fsaun, &fromlen)) < 0) {
    printf("Cannot accept ipc connection... ignoring connection attempt.\n");
        return;
  }

  /* set nonblocking */
  if (fcntl(nfd, F_SETFL, O_NONBLOCK) < 0) {
    printf("Cannot fcntl ipc connection... ignoring connection attempt.\n");
    return;
  }

  /* add ipc client */
  e_add_event_ipc(nfd, e_ipc_client_handler);
}

static void
e_ipc_client_handler(int fd)
{
  int nread, service;
  char ptr[4096];
  Ev_Ipc_Service *ipc_s;

  /* in no way done here yet, lots to do */
  if ((nread = read(fd, ptr, sizeof(ptr))) == 0)
    {
      close(fd);
      e_del_event_ipc(fd);
    }
  else if (nread > 0)
    {
      printf("IPC Client sent %d bytes\n", nread); fflush(stdout);
      service = atoi(&ptr[0]);

      for (ipc_s = ipc_services; ipc_s; ipc_s = ipc_s->next)
         {
           if (ipc_s->service == service)
             {
               ipc_s->func("TESTING");
               break;
             }
         }
    }
  else
    {
      printf("error\n"); fflush(stdout);
    }
}

void
e_add_ipc_service(int service, char *(*func) (char *argv))
{
   Ev_Ipc_Service     *ipc_s;

   /* delete the old service */
   e_del_ipc_service(service);
   /* new service struct */
   ipc_s = NEW(Ev_Ipc_Service, 1);
   ipc_s->next = ipc_services;
   ipc_s->service = service;
   ipc_s->func = func;
   ipc_services = ipc_s;
}

void e_del_ipc_service(int service)
{
   START_LIST_DEL(Ev_Ipc_Service, ipc_services, (_p->service == service));
   FREE(_p);
   END_LIST_DEL;
}
