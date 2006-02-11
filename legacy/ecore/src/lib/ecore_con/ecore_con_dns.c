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
 *   We should store all names returned when CNAME, might have different ttl.
 *   Check against search and hostname when querying cache?
 * * Remember all querys and delete them on shutdown
 *
 * * Need more buffer overflow checks.
 */
#include "Ecore.h"
#include "ecore_private.h"

#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <netdb.h>

#define SERVERS 3

typedef struct _Ecore_Con_Dns_Query Ecore_Con_Dns_Query;
typedef struct _Ecore_Con_Dns_Cache Ecore_Con_Dns_Cache;

struct _Ecore_Con_Dns_Query {
     Ecore_List2 list;

     /* Can ask three servers */
     unsigned int id[SERVERS];
     int socket[SERVERS];
     Ecore_Fd_Handler *fd_handlers[SERVERS];

     Ecore_Timer *timeout;

     int search;

     /* The name the user searches for */
     char *searchname;

     struct {
	  void (*cb)(void *data, struct hostent *hostent);
	  void *data;
     } done;

};

struct _Ecore_Con_Dns_Cache {
     Ecore_List2 list;

     int ttl;
     double time;
     struct hostent *he;
};

static void _ecore_con_dns_ghbn(Ecore_Con_Dns_Query *query, const char *hostname);
static int  _ecore_con_dns_timeout(void *data);
static int  _ecore_con_cb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static void _ecore_con_dns_query_free(Ecore_Con_Dns_Query *query);
static void _ecore_con_dns_cache_free(Ecore_Con_Dns_Cache *cache);
static int  _ecore_con_hostname_get(unsigned char *buf, char *hostname,
				    int pos, int length);

static int dns_init = 0;

static struct in_addr servers[SERVERS];
static int            server_count;

static char *search[6];
static int   search_count = 0;

static char *domain = NULL;

static uint16_t dns_id = 0;

static Ecore_Con_Dns_Cache *dns_cache = NULL;

#define SET_16BIT(p, v) \
   (((p)[0]) = ((v) >> 8) & 0xff), \
   (((p)[1]) = v & 0xff)

#define GET_16BIT(p) (((p)[0]) << 8 | ((p)[1]))
#define GET_32BIT(p) (((p)[0]) << 24 | ((p)[1]) << 16 | ((p)[2]) << 8 | ((p)[3]))

int
ecore_con_dns_init(void)
{
#if 1   
   dns_init++;
   return dns_init;
#else
   FILE *file;
   char buf[1024];
   char *p, *p2;
   int ret;

   if (++dns_init != 1) return dns_init;

   memset(servers, 0, sizeof(servers));
   server_count = 0;

   file = fopen("/etc/resolv.conf", "rb");
   if (!file) return --dns_init;
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
	     if (server_count >= SERVERS) continue;

	     servers[server_count].s_addr = inet_addr(p);
	     server_count++;
	  }
	else if (!strncmp(buf, "domain", 6))
	  {
	     int i;

	     /* Skip leading dot */
	     if (*p == '.')
	       p++;
	     /* Get the domain */
	     domain = strdup(p);
	     /* clear search */
	     for (i = 0; i < search_count; i++)
	       {
		  free(search[i]);
		  search[i] = NULL;
	       }
	     search_count = 0;
	  }
	else if (!strncmp(buf, "search", 6))
	  {
	     while ((p) && (search_count < 6))
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
		  search[search_count] = strdup(p);
		  search_count++;
		  if (p2) p = p2 + 1;
		  else p = NULL;
	       }
	     if (domain)
	       {
		  free(domain);
		  domain = NULL;
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

   if (!server_count)
     {
	/* We should try localhost */
	servers[server_count].s_addr = inet_addr("127.0.0.1");
	server_count++;
     }
   if ((!search_count) && (!domain))
     {
	/* Get domain from hostname */
	ret = gethostname(buf, sizeof(buf));
	if ((ret > 0) && (ret < 1024))
	  {
	     p = strchr(buf, '.');
	     if (p)
	       {
		  p++;
		  domain = strdup(p);
	       }
	  }
     }

   return dns_init;
#endif   
}

int
ecore_con_dns_shutdown(void)
{
#if 1
   dns_init--;
   return  dns_init;
#else   
   Ecore_List2 *l;
   int i;

   if (--dns_init != 0) return dns_init;

   for (l = (Ecore_List2 *)dns_cache; l;)
     {
	Ecore_Con_Dns_Cache *current;
	
	current = (Ecore_Con_Dns_Cache *)l;
	l = l->next;
	_ecore_con_dns_cache_free(current);
     }
   dns_cache = NULL;

   if (domain)
     {
	free(domain);
	domain = NULL;
     }
   for (i = 0; i < search_count; i++)
     free(search[i]);
   search_count = 0;

   return dns_init;
#endif   
}

#if 1
typedef struct _CB_Data CB_Data;

struct _CB_Data
{
   void (*cb_done) (void *data, struct hostent *hostent);
   void *data;
   Ecore_Fd_Handler *fdh;
   pid_t pid;
   Ecore_Event_Handler *handler;
};

static void
_ecore_con_dns_readdata(CB_Data *cbdata)
{
   struct hostent he;
   struct in_addr addr;
   char *addr2;
   ssize_t size;
   
   size = read(ecore_main_fd_handler_fd_get(cbdata->fdh), &(addr.s_addr),
	       sizeof(in_addr_t));
   if (size == sizeof(in_addr_t))
     {
	addr2 = (char *)&addr;
	he.h_addrtype = AF_INET;
	he.h_length = sizeof(in_addr_t);
	he.h_addr_list = &addr2;
	cbdata->cb_done(cbdata->data, &he);
     }
   else
     cbdata->cb_done(cbdata->data, NULL);
   cbdata->cb_done = NULL;
}

static int
_ecore_con_dns_data_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   CB_Data *cbdata;

   cbdata = data;
   if (cbdata->cb_done)
     {
	if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
	  _ecore_con_dns_readdata(cbdata);
	else
	  {
	     cbdata->cb_done(cbdata->data, NULL);
	     cbdata->cb_done = NULL;
	  }
     }
   return 1;
}

static int
_ecore_con_dns_exit_handler(void *data, int type, void *event)
{
   CB_Data *cbdata;
   Ecore_Exe_Event_Del *ev;
   
   ev = event;
   cbdata = data;
   if (cbdata->pid != ev->pid) return 1;
   close(ecore_main_fd_handler_fd_get(cbdata->fdh));
   ecore_main_fd_handler_del(cbdata->fdh);
   ecore_event_handler_del(cbdata->handler);
   free(cbdata);
   return 0;
}
#else
#endif

int
ecore_con_dns_lookup(const char *name,
		     void (*done_cb) (void *data, struct hostent *hostent),
		     void *data)
{
#if 1
   CB_Data *cbdata;
   int fd[2];
   
   /* FIXME: set up one-way pipe back to parent and fdhandler */
   if (pipe(fd) < 0) return 0;
   cbdata = calloc(1, sizeof(CB_Data));
   if (!cbdata)
     {
	close(fd[0]);
	close(fd[1]);
	return 0;
     }
   cbdata->cb_done = done_cb;
   cbdata->data = data;
   if (!(cbdata->fdh = ecore_main_fd_handler_add(fd[0], ECORE_FD_READ, 
						 _ecore_con_dns_data_handler,
						 cbdata,
						 NULL, NULL)))
     {
	free(cbdata);
	close(fd[0]);
	close(fd[1]);
	return 0;
     }
			     
   if ((cbdata->pid = fork()) == 0)
     {
	struct hostent *he;
	
	/* CHILD */
	he = gethostbyname(name);
	if (he)
	  {
	     struct in_addr addr;
	     
	     memcpy((struct in_addr *)&addr, he->h_addr,
		    sizeof(struct in_addr));
	     write(fd[1], &(addr.s_addr), sizeof(in_addr_t));
	  }
	close(fd[1]);
# ifdef __USE_ISOC99
	Exit(0);
# else	
	_exit(0);
# endif	
     }
   /* PARENT */
   cbdata->handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _ecore_con_dns_exit_handler, cbdata);
   if (!cbdata->handler)
     {
	ecore_main_fd_handler_del(cbdata->fdh);
	free(cbdata);
	close(fd[0]);
	close(fd[1]);
	return 0;
     }
#else
   Ecore_Con_Dns_Query *query;
   Ecore_Con_Dns_Cache *current;
   Ecore_List2 *l;

   if (!server_count) return 0;
   if ((!name) || (!*name)) return 0;

   for (l = (Ecore_List2 *)dns_cache; l;)
     {
	double time;
	int i;

	current = (Ecore_Con_Dns_Cache *)l;
	l = l->next;

	time = ecore_time_get();
	if ((time - current->time) > current->ttl)
	  {
	     dns_cache = _ecore_list2_remove(dns_cache, current);
	     _ecore_con_dns_cache_free(current);
	  }
	else
	  {
	     /* Check if we have a match */
	     if (!strcmp(name, current->he->h_name))
	       {
		  if (done_cb)
		    done_cb(data, current->he);
		  dns_cache = _ecore_list2_remove(dns_cache, current);
		  dns_cache = _ecore_list2_prepend(dns_cache, current);
		  return 1;
	       }
	     for (i = 0; current->he->h_aliases[i]; i++)
	       {
		  if (!strcmp(name, current->he->h_aliases[i]))
		    {
		       if (done_cb)
			 done_cb(data, current->he);
		       dns_cache = _ecore_list2_remove(dns_cache, current);
		       dns_cache = _ecore_list2_prepend(dns_cache, current);
		       return 1;
		    }
	       }
	  }
     }

   query = calloc(1, sizeof(Ecore_Con_Dns_Query));
   if (!query) return 0;

   query->done.cb = done_cb;
   query->done.data = data;
   query->timeout = ecore_timer_add(20.0, _ecore_con_dns_timeout, query);
   query->searchname = strdup(name);
   query->search = -1;

   _ecore_con_dns_ghbn(query, name);
#endif   
   return 1;
}

#if 1
#else
static void
_ecore_con_dns_ghbn(Ecore_Con_Dns_Query *query, const char *hostname)
{
   char buf[256];
   char *p, *pl;
   const char *q;
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
   q = hostname;
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
   for (i = 0; i < server_count; i++)
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
	sin.sin_addr = servers[i];
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
	query->id[i] = ++dns_id;
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
     query->done.cb(query->done.data, NULL);
   _ecore_con_dns_query_free(query);
   return 0;
}

static int
_ecore_con_cb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Dns_Query *query;
   Ecore_Con_Dns_Cache *cache;
   int i, n, fd, found, len;
   unsigned int id;
   unsigned char buf[1024];
   char hostname[1024];
   unsigned char *p;
   char **aliases = NULL;
   struct in_addr *addrs = NULL;
   int naliases = 0, naddrs = 0;
   int ancount, ttl = INT_MAX;
   struct hostent *he;

   query = data;
   fd = ecore_main_fd_handler_fd_get(fd_handler);

   memset(buf, 0, sizeof(buf));
   n = recv(fd, buf, sizeof(buf), 0);
   if ((n == -1) || (n < HFIXEDSZ) || (n > sizeof(buf))) goto error;
   /* Check if this message is for us */
   id = GET_16BIT(buf);
   found = 0;
   for (i = 0; i < server_count; i++)
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
   p += len;
   /* Skip the question */
   if (((p + QFIXEDSZ) - buf) >= n) goto error;
   p += QFIXEDSZ;

   aliases = malloc((ancount + 1) * sizeof(char *));
   addrs = malloc((ancount + 1) * sizeof(struct in_addr));

   for (i = 0; i < ancount; i++)
     {
	int rr_type, rr_class, rr_len, rr_ttl;
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
	/* Get the resource record ttl */
	rr_ttl = GET_32BIT(p);
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
	     ttl = MIN(rr_ttl, ttl);
	  }
	else if ((rr_class == C_IN) && (rr_type == T_A) && (!strcmp(hostname, rr_name)))
	  {
	     /* We should get 4 bytes, 1 for each octet in the IP addres */
	     if (rr_len != 4) goto error;
	     memcpy(&addrs[naddrs++], p, sizeof(struct in_addr));
	     p += rr_len;
	     ttl = MIN(rr_ttl, ttl);
	  }
	else
	  p += rr_len;
     }

   /* Fill in the hostent and return successfully. */
   he = malloc(sizeof(struct hostent));
   if (!he) goto error;
   he->h_addr_list = malloc((naddrs + 1) * sizeof(char *));
   if (!he->h_addr_list) goto error;
   he->h_name = strdup(hostname);
   aliases[naliases] = NULL;
   he->h_aliases = aliases;
   he->h_addrtype = AF_INET;
   he->h_length = sizeof(struct in_addr);
   for (i = 0; i < naddrs; i++)
     he->h_addr_list[i] = (char *) &addrs[i];
   he->h_addr_list[naddrs] = NULL;

   if (query->done.cb)
     query->done.cb(query->done.data, he);

   cache = malloc(sizeof(Ecore_Con_Dns_Cache));
   if (cache)
     {
	Ecore_List2 *l;

	cache->ttl = ttl;
	cache->time = ecore_time_get();
	cache->he = he;
	dns_cache = _ecore_list2_prepend(dns_cache, cache);

	/* Check cache size */
	i = 1;
	l = (Ecore_List2 *)dns_cache;
	while ((l = l->next))
	  i++;

	/* Remove old stuff if cache to big */
	if (i > 16)
	  {
	     cache = (Ecore_Con_Dns_Cache *)((Ecore_List2 *)dns_cache)->last;
	     dns_cache = _ecore_list2_remove(dns_cache, cache);
	     _ecore_con_dns_cache_free(cache);
	  }
     }
   else
     {
	free(he->h_addr_list);
	free(he->h_name);
	free(addrs);
	for (i = 0; i < naliases; i++)
	  free(aliases[i]);
	free(aliases);
	free(he);
     }

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
   for (i = 0; i < server_count; i++)
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
	if ((domain) && (query->search++))
	  {
	     if (snprintf(buf, sizeof(buf), "%s.%s", query->searchname, domain) < sizeof(buf))
	       {
		  _ecore_con_dns_ghbn(query, buf);
	       }
	     else
	       {
		  if (query->done.cb)
		    query->done.cb(query->done.data, NULL);
		  _ecore_con_dns_query_free(query);
	       }
	  }
	else if ((++query->search) < search_count)
	  {
	     if (snprintf(buf, sizeof(buf), "%s.%s", query->searchname, search[query->search]) < sizeof(buf))
	       {
		  _ecore_con_dns_ghbn(query, buf);
	       }
	     else
	       {
		  if (query->done.cb)
		    query->done.cb(query->done.data, NULL);
		  _ecore_con_dns_query_free(query);
	       }
	  }
	else
	  {
	     /* Shutdown */
	     if (query->done.cb)
	       query->done.cb(query->done.data, NULL);
	     _ecore_con_dns_query_free(query);
	  }
     }
   return 0;
}

static void
_ecore_con_dns_query_free(Ecore_Con_Dns_Query *query)
{
   int i;
 
   for (i = 0; i < server_count; i++)
     {
	if (query->socket[i]) close(query->socket[i]);
	query->socket[i] = 0;
	if (query->fd_handlers[i]) ecore_main_fd_handler_del(query->fd_handlers[i]);
	query->fd_handlers[i] = NULL;
     }
   if (query->timeout) ecore_timer_del(query->timeout);
   query->timeout = NULL;
   free(query->searchname);
   free(query);
}

static void
_ecore_con_dns_cache_free(Ecore_Con_Dns_Cache *cache)
{
   int i;

   free(cache->he->h_name);
   free(cache->he->h_addr_list[0]);
   free(cache->he->h_addr_list);
   for (i = 0; cache->he->h_aliases[i]; i++)
     free(cache->he->h_aliases[i]);
   free(cache->he->h_aliases);
   free(cache->he);
   free(cache);
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
#endif
