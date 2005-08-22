/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* 
 * Simple dns lookup
 *
 * http://www.faqs.org/rfcs/rfc1035.html
 * man resolv.conf
 *
 * And a sneakpeek at ares, ftp://athena-dist.mit.edu/pub/ATHENA/ares/
 */
/*
 * TODO
 * * Check env LOCALDOMAIN to override search
 * * Check env RES_OPTIONS to override options
 *
 * * Read /etc/host.conf
 * * host.conf env
 *   RESOLV_HOST_CONF, RESOLV_SERV_ORDER
 * * Check /etc/hosts
 *
 * * Caching
 * * Remember all querys and delete on shutdown
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <netdb.h>

#include <Ecore.h>
#include "ecore_private.h"

#define SERVERS 3

typedef struct _Ecore_Con_Dns_Query Ecore_Con_Dns_Query;
struct _Ecore_Con_Dns_Query {
     Ecore_Oldlist list;

     /* Can ask three servers */
     unsigned int id[SERVERS];
     int socket[SERVERS];
     Ecore_Fd_Handler *fd_handlers[SERVERS];

     Ecore_Timer *timeout;

     int search;

     /* The name the user searches for */
     char *searchname;
     /* The name vi send to dns and return to the user */
     char *hostname;

     struct {
	  void (*cb)(struct hostent *hostent, void *data);
	  void *data;
     } done;

};

static void _ecore_con_dns_ghbn(Ecore_Con_Dns_Query *query);
static int  _ecore_con_dns_timeout(void *data);
static int  _ecore_con_cb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static void _ecore_con_dns_query_free(Ecore_Con_Dns_Query *query);

static int _init = 0;

static struct in_addr _servers[SERVERS];
static int            _server_count;

static char *_search[6];
static int   _search_count = 0;

static char *_domain = NULL;

static uint16_t _id = 0;

#define SET_16BIT(p, v) \
   (((p)[0]) = ((v) >> 8) & 0xff), \
   (((p)[1]) = v & 0xff)

#define GET_16BIT(p) (((p)[0]) << 8 | ((p)[1]))

int
ecore_con_dns_init(void)
{
   FILE *file;
   char buf[1024];
   char *p, *p2;
   int ret;

   _init++;
   if (_init > 1) return 1;

   memset(_servers, 0, sizeof(_servers));
   _server_count = 0;

   file = fopen("/etc/resolv.conf", "rb");
   if (!file) return 0;
   while (fgets(buf, sizeof(buf), file))
     {
	if (strlen(buf) >= 1023)
	  printf("WARNING: Very long line in resolv.conf\n");

	/* remove whitespace */
	p = strchr(buf, ' ');
	if (!p)
	  p = strchr(buf, '\t');
	if (!p) continue;
	while ((*p) && (isspace(*p)))
	  p++;
	/* Remove trailing newline */
	p2 = strchr(buf, '\n');
	if (p2)
	  *p2 = 0;

	if ((buf[0] == ';') || (buf[0] == '#'))
	  {
	     /* Ignore comment */
	  }
	else if (!strncmp(buf, "nameserver", 10))
	  {
	     if (_server_count >= SERVERS) continue;

	     _servers[_server_count].s_addr = inet_addr(p);
	     _server_count++;
	  }
	else if (!strncmp(buf, "domain", 6))
	  {
	     int i;

	     _domain = strdup(p);
	     /* clear search */
	     for (i = 0; i < _search_count; i++)
	       {
		  free(_search[i]);
		  _search[i] = NULL;
	       }
	     _search_count = 0;
	  }
	else if (!strncmp(buf, "search", 6))
	  {
	     while ((p) && (_search_count < 6))
	       {
		  /* Remove whitespace */
		  while ((*p) && (isspace(*p)))
		    p++;
		  /* Find next element */
		  p2 = strchr(p, ' ');
		  if (!p2)
		    p2 = strchr(p, '\t');
		  if (p2)
		    *p2 = 0;
		  /* Get this element */
		  _search[_search_count] = strdup(p);
		  _search_count++;
		  if (p2) p = p2 + 1;
		  else p = NULL;
	       }
	     if (_domain)
	       {
		  free(_domain);
		  _domain = NULL;
	       }
	  }
	else if (!strncmp(buf, "sortlist", 8))
	  {
	     /* TODO */
	  }
	else if (!strncmp(buf, "options", 8))
	  {
	     /* TODO */
	  }
	else
	  printf("WARNING: Weird line in resolv.conf: %s\n", buf);
     }
   fclose(file);

   if (!_server_count)
     {
	/* We should try localhost */
	_servers[_server_count].s_addr = inet_addr("127.0.0.1");
	_server_count++;
     }
   if ((!_search_count) && (!_domain))
     {
	/* Get domain from hostname */
	ret = gethostname(buf, sizeof(buf));
	if ((ret > 0) && (ret < 1024))
	  {
	     p = strchr(buf, '.');
	     if (p)
	       {
		  p++;
		  _domain = strdup(p);
	       }
	  }
     }
	     
   return 1;
}

void
ecore_con_dns_shutdown(void)
{
   int i;

   _init--;
   if (_init > 0) return;

   if (_domain)
     {
	free(_domain);
	_domain = NULL;
     }
   for (i = 0; i < _search_count; i++)
     free(_search[i]);
   _search_count = 0;
}

int
ecore_con_dns_gethostbyname(const char *name,
			    void (*done_cb)(struct hostent *hostent, void *data),
		       	    void *data)
{
   Ecore_Con_Dns_Query *query;

   if (!_server_count) return 0;
   if ((!name) || (!*name)) return 0;

   query = calloc(1, sizeof(Ecore_Con_Dns_Query));
   if (!query) return 0;

   query->done.cb = done_cb;
   query->done.data = data;
   query->timeout = ecore_timer_add(20.0, _ecore_con_dns_timeout, query);
   query->hostname = strdup(name);
   query->searchname = strdup(name);
   query->search = -1;

   _ecore_con_dns_ghbn(query);
   return 1;
}

static void
_ecore_con_dns_ghbn(Ecore_Con_Dns_Query *query)
{
   char buf[256];
   char *p, *q, *pl;
   int i, len, total_len;

   /* Create buf */
   memset(buf, 0, sizeof(buf));
   p = buf;
   total_len = 0;

   p += 2;
   /* opcode */
   *p |= (QUERY & 0xf) << 3;
   /* TODO: rd, do we always want recursive? */
   *p |= 1 & 0x1;
   /* qdcount, only asking for one name */
   p += 2;
   SET_16BIT(p, 1);

   total_len += HFIXEDSZ;
   p = &buf[HFIXEDSZ];

   /* remember where the length shall be placed */
   pl = p;
   p++;
   total_len++;
   /* name */
   q = query->hostname;
   len = 0;
   while ((*q) && (total_len < 1024))
     {
	if (*q == '.')
	  {
	     if (len)
	       {
		  *pl = len;
		  pl = p;
		  p++;
		  len = 0;
		  total_len++;
	       }
	     q++;
	  }
	else if ((*q == '\\') && (*(q + 1) == 0))
	  {
	     q++;

	     *p++ = *q++;
	     len++;
	     total_len++;
	  }
	else
	  {
	     *p++ = *q++;
	     len++;
	     total_len++;
	  }
     }
   /* Null at the end of the query */
   if (len)
     {
	*pl = len;
	*p = 0;
	p++;
	total_len++;
     }

   /* type */
   SET_16BIT(p, T_A);
   p += 2;
   /* class */
   SET_16BIT(p, C_IN);
   p += 2;
   total_len += QFIXEDSZ;

   /* We're crazy, just ask all servers! */
   for (i = 0; i < _server_count; i++)
     {
	struct sockaddr_in sin;

	query->socket[i] = socket(AF_INET, SOCK_DGRAM, 0);
	if (query->socket[i] == -1)
	  {
	     printf("ERROR: Couldn't create socket\n");
	     continue;
	  }

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = _servers[i];
	sin.sin_port = htons(NAMESERVER_PORT);
	if (connect(query->socket[i], (struct sockaddr *) &sin, sizeof(sin)) == -1)
	  {
	     /* TODO: EINPROGRESS isn't a fatal error */
	     printf("ERROR: Couldn't connect to nameserver\n");
	     close(query->socket[i]);
	     query->socket[i] = 0;
	     continue;
	  }

	/* qid */
	query->id[i] = ++_id;
	SET_16BIT(buf, query->id[i]);

	if (send(query->socket[i], buf, total_len, 0) == -1)
	  {
	     printf("ERROR: Send failed\n");
	     close(query->socket[i]);
	     query->socket[i] = 0;
	     continue;
	  }

	query->fd_handlers[i] = ecore_main_fd_handler_add(query->socket[i],
							  ECORE_FD_READ,
							  _ecore_con_cb_fd_handler, query,
							  NULL, NULL);

     }
}

static int
_ecore_con_dns_timeout(void *data)
{
   Ecore_Con_Dns_Query *query;

   query = data;

   if (query->done.cb)
     query->done.cb(NULL, query->done.data);
   _ecore_con_dns_query_free(query);
   return 0;
}

static int
_ecore_con_cb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Dns_Query *query;
   int i, n, fd, found = 0;
   unsigned int id;
   unsigned char buf[1024];
   unsigned char *p;
   int size;
   struct hostent he;

   query = data;
   fd = ecore_main_fd_handler_fd_get(fd_handler);

   memset(buf, 0, sizeof(buf));
   n = recv(fd, buf, sizeof(buf), 0);
   if (n == -1) goto error;
   /* Check if this message is for us */
   id = GET_16BIT(buf);
   for (i = 0; i < _server_count; i++)
     {
	if (query->id[i] == id)
	  {
	     found = 1;
	     id = i;
	     break;
	  }
     }
   if (!found) goto error;

   /* This should be it! */
   p = buf;

   /* Skip the query */
   p += HFIXEDSZ;
   while (*p)
     p += (*p + 1);
   p++;
   p += QFIXEDSZ;

   /* Skip the header */
   p += RRFIXEDSZ;
   size = GET_16BIT(p);
   /* We should get 4 bytes, 1 for each octet in the IP addres */
   if (size != 4) goto error;

   p += 2;

   /* Get the IP address */
   he.h_addr_list = malloc(2 * sizeof(char *));
   if (!he.h_addr_list) goto error;
   /* Fill in the hostent and return successfully. */
   /* TODO: Maybe get the hostname from the reply */
   he.h_name = strdup(query->hostname);
   /* he.h_aliases = aliases; */
   he.h_addrtype = AF_INET;
   he.h_length = sizeof(struct in_addr);
   he.h_addr_list[0] = malloc(4 * sizeof(char));
   memcpy(he.h_addr_list[0], p, he.h_length);
   he.h_addr_list[1] = NULL;

   if (query->done.cb)
     query->done.cb(&he, query->done.data);
   free(he.h_addr_list);
   _ecore_con_dns_query_free(query);
   return 0;

error:
   found = 0;
   for (i = 0; i < _server_count; i++)
     {
	if (query->fd_handlers[i] == fd_handler)
	  {
	     /* This server didn't do it */
	     if (query->socket[i]) close(query->socket[i]);
	     query->socket[i] = 0;
	     query->fd_handlers[i] = NULL;
	  }
	else if (query->socket[i])
	  {
	     /* We're still looking */
	     found = 1;
	  }
     }

   if (!found)
     {
	char buf[256];

	/* Should we look more? */
	if ((_domain) && (query->search++))
	  {
	     if (snprintf(buf, sizeof(buf), "%s%s", query->searchname, _domain) < sizeof(buf))
	       {
		  free(query->hostname);
		  query->hostname = strdup(buf);
		  _ecore_con_dns_ghbn(query);
	       }
	     else
	       {
		  if (query->done.cb)
		    query->done.cb(NULL, query->done.data);
		  _ecore_con_dns_query_free(query);
	       }
	  }
	else if ((++query->search) < _search_count)
	  {
	     if (snprintf(buf, sizeof(buf), "%s%s", query->searchname, _search[query->search]) < sizeof(buf))
	       {
		  free(query->hostname);
		  query->hostname = strdup(buf);
		  _ecore_con_dns_ghbn(query);
	       }
	     else
	       {
		  if (query->done.cb)
		    query->done.cb(NULL, query->done.data);
		  _ecore_con_dns_query_free(query);
	       }
	  }
	else
	  {
	     /* Shutdown */
	     if (query->done.cb)
	       query->done.cb(NULL, query->done.data);
	     _ecore_con_dns_query_free(query);
	  }
     }
   return 0;
}

static void
_ecore_con_dns_query_free(Ecore_Con_Dns_Query *query)
{
   int i;
 
   for (i = 0; i < _server_count; i++)
     {
	if (query->socket[i]) close(query->socket[i]);
	query->socket[i] = 0;
	if (query->fd_handlers[i]) ecore_main_fd_handler_del(query->fd_handlers[i]);
	query->fd_handlers[i] = NULL;
     }
   if (query->timeout) ecore_timer_del(query->timeout);
   query->timeout = NULL;
   free(query->hostname);
   free(query);
}
