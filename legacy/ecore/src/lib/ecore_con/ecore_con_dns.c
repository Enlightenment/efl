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
 * * Remember all querys and delete them on shutdown
 *
 * * Need more buffer overflow checks.
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
static int  _ecore_con_hostname_get(unsigned char *buf, char *hostname,
				    int pos, int length);

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

	if (!strncmp(buf, "nameserver", 10))
	  {
	     if (_server_count >= SERVERS) continue;

	     _servers[_server_count].s_addr = inet_addr(p);
	     _server_count++;
	  }
	else if (!strncmp(buf, "domain", 6))
	  {
	     int i;

	     /* Skip leading dot */
	     if (*p == '.')
	       p++;
	     /* Get the domain */
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
		  /* Skip leading dot */
		  if (*p == '.')
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
ecore_con_dns_lookup(const char *name,
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

   query->timeout = NULL;
   if (query->done.cb)
     query->done.cb(NULL, query->done.data);
   _ecore_con_dns_query_free(query);
   return 0;
}

static int
_ecore_con_cb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Dns_Query *query;
   int i, n, fd, found, len;
   unsigned int id;
   unsigned char buf[1024];
   char hostname[1024];
   unsigned char *p;
   char **aliases = NULL;
   struct in_addr *addrs = NULL;
   int naliases = 0, naddrs = 0;
   int ancount;
   struct hostent he;

   query = data;
   fd = ecore_main_fd_handler_fd_get(fd_handler);

   memset(buf, 0, sizeof(buf));
   n = recv(fd, buf, sizeof(buf), 0);
   if ((n == -1) || (n < HFIXEDSZ) || (n > sizeof(buf))) goto error;
   /* Check if this message is for us */
   id = GET_16BIT(buf);
   found = 0;
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
   /* Skip id and flags */
   p += 4;
   /* Check question count (QDCOUNT)*/
   i = GET_16BIT(p);
   p += 2;
   if (i != 1) goto error;
   /* Get the number of answers (ANCOUNT) */
   ancount = GET_16BIT(p);
   p += 2;
   if (ancount < 1) goto error;
   /* Skip NSCOUNT */
   p += 2;
   /* Skip ARCOUNT */
   p += 2;

   /* Skip the hostname */
   if ((len = _ecore_con_hostname_get(buf, hostname, p - buf, n)) == -1) goto error;
   if (strcmp(hostname, query->hostname))
     printf("WARNING: Not the same hostname: %s %s?\n", hostname, query->hostname);
   p += len;
   /* Skip the question */
   if (((p + QFIXEDSZ) - buf) >= n) goto error;
   p += QFIXEDSZ;

   aliases = malloc((ancount + 1) * sizeof(char *));
   addrs = malloc((ancount + 1) * sizeof(struct in_addr));

   for (i = 0; i < ancount; i++)
     {
	int rr_type, rr_class, rr_len;
	char rr_name[1024], rr_data[1024];

	/* Get the name */
	if ((len = _ecore_con_hostname_get(buf, rr_name, p - buf, n)) == -1) goto error;
	p += len;
	if (((p + RRFIXEDSZ) - buf) >= n) goto error;
	/* Get the resource record type */
	rr_type = GET_16BIT(p);
	p += 2;
	/* Get the resource record class */
	rr_class = GET_16BIT(p);
	p += 2;
	/* Skip resource record ttl */
	p += 4;
	/* Get the resource record length */
	rr_len = GET_16BIT(p);
	p += 2;
	/* > n is correct here. On the last message p will point after the last
	 * data bit, but for all other messages p will point to the next data
	 */
	if (((p + rr_len) - buf) > n) goto error;

	if ((rr_class == C_IN) && (rr_type == T_CNAME))
	  {
	     /* Store name as alias */
	     aliases[naliases++] = strdup(rr_name);

	     /* Get hostname */
	     if ((len = _ecore_con_hostname_get(buf, rr_data, p - buf, n)) == -1) goto error;
	     strcpy(hostname, rr_data);
	     p += rr_len;
	  }
	else if ((rr_class == C_IN) && (rr_type == T_A) && (!strcmp(hostname, rr_name)))
	  {
	     /* We should get 4 bytes, 1 for each octet in the IP addres */
	     if (rr_len != 4) goto error;
	     memcpy(&addrs[naddrs++], p, sizeof(struct in_addr));
	     p += rr_len;
	  }
	else
	  p += rr_len;
     }

   /* Fill in the hostent and return successfully. */
   he.h_addr_list = malloc((naddrs + 1) * sizeof(char *));
   if (!he.h_addr_list) goto error;
   he.h_name = strdup(hostname);
   aliases[naliases] = NULL;
   he.h_aliases = aliases;
   he.h_addrtype = AF_INET;
   he.h_length = sizeof(struct in_addr);
   for (i = 0; i < naddrs; i++)
     he.h_addr_list[i] = (char *) &addrs[i];
   he.h_addr_list[naddrs] = NULL;

   if (query->done.cb)
     query->done.cb(&he, query->done.data);

   free(he.h_addr_list);
   free(he.h_name);
   free(addrs);
   for (i = 0; i < naliases; i++)
     free(aliases[i]);
   free(aliases);

   _ecore_con_dns_query_free(query);
   return 0;

error:
   if (addrs) free(addrs);
   if (aliases)
     {
	for (i = 0; i < naliases; i++)
	  free(aliases[i]);
	free(aliases);
     }

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
	     if (snprintf(buf, sizeof(buf), "%s.%s", query->searchname, _domain) < sizeof(buf))
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
	     if (snprintf(buf, sizeof(buf), "%s.%s", query->searchname, _search[query->search]) < sizeof(buf))
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
   free(query->searchname);
   free(query);
}

static int
_ecore_con_hostname_get(unsigned char *buf, char *hostname,
			int pos, int length)
{
   unsigned char *p;
   char *q;
   int offset, indir, len, data;

   p = buf;
   p += pos;

   q = hostname;

   offset = pos;
   data = 0;
   indir = 0;
   while (*p)
     {
	if ((*p & INDIR_MASK) == INDIR_MASK)
	  {
	     /* Check offset */
	     if (((p + 1) - buf) >= length) return -1;
	     offset = (*p & ~INDIR_MASK) << 8 | *(p + 1);
	     if (offset >= length) return -1;
	     p = buf + offset;

	     if (!indir)
	       {
		  data = 2;
		  indir = 1;
	       }
	  }
	else
	  {
	     offset += (*p + 1);
	     if (offset >= length) return -1;

	     len = *p;
	     if (!indir)
	       data += len + 1;

	     /* Get the name */
	     *p++;
	     while (len--)
	       {
		  if (*p == '.')
		    *q++ = '\\';
		  *q++ = *p++;
	       }
	     if (*(p + 1))
	       *q++ = '.';
	     else
	       *q++ = 0;
	  }
     }
   if (!indir)
     data++;
   return data;
}
