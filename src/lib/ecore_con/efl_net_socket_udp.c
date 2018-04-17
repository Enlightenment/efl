#define EFL_NET_SOCKET_UDP_PROTECTED 1
#define EFL_NET_SOCKET_FD_PROTECTED 1
#define EFL_LOOP_FD_PROTECTED 1
#define EFL_IO_READER_FD_PROTECTED 1
#define EFL_IO_WRITER_FD_PROTECTED 1
#define EFL_IO_CLOSER_FD_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_UDP_H
# include <netinet/udp.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef _WIN32
# include <Evil.h>
#endif

#define MY_CLASS EFL_NET_SOCKET_UDP_CLASS

typedef struct _Efl_Net_Socket_Udp_Data
{
   struct {
      Eina_List *groups; /* list of newly allocated strings */
      Eina_List *pending; /* list of nodes of groups pending join */
      uint8_t ttl;
      Eina_Bool loopback;
      Eina_Bool ttl_set;
   } multicast;
   Eina_Stringshare *address_bind;
   struct sockaddr *addr_remote;
   socklen_t addr_remote_len;
   Eina_Bool cork;
   Eina_Bool dont_route;
   Eina_Bool reuse_address;
   Eina_Bool reuse_port;
} Efl_Net_Socket_Udp_Data;

void
_efl_net_socket_udp_init(Eo *o, Efl_Net_Socket_Udp_Data *pd, Efl_Net_Ip_Address *remote_address)
{
   const struct sockaddr *addr = efl_net_ip_address_sockaddr_get(remote_address);
   socklen_t addrlen;

   EINA_SAFETY_ON_NULL_RETURN(addr);

   if (addr->sa_family == AF_INET) addrlen = sizeof(struct sockaddr_in);
   else addrlen = sizeof(struct sockaddr_in6);

   pd->addr_remote = malloc(addrlen);
   EINA_SAFETY_ON_NULL_RETURN(pd->addr_remote);
   memcpy(pd->addr_remote, addr, addrlen);
   pd->addr_remote_len = addrlen;
   efl_net_socket_address_remote_set(o, efl_net_ip_address_string_get(remote_address));
}

static Eina_Error
_efl_net_socket_udp_bind(Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   const char *bhost, *bport;
   struct sockaddr_in bsa4 = {.sin_family = AF_INET};
   struct sockaddr_in6 bsa6 = {.sin6_family = AF_INET6};
   char *str = NULL, *endptr;
   unsigned long ul;
   Eina_Error err = 0;
   SOCKET fd = efl_loop_fd_get(o);
   int family = efl_net_socket_fd_family_get(o);
   int r;

   if (!pd->address_bind) return 0;

   str = strdup(pd->address_bind);
   EINA_SAFETY_ON_NULL_GOTO(str, error_bind);
   if (!efl_net_ip_port_split(str, &bhost, &bport))
     {
        bhost = (family == AF_INET) ? "0.0.0.0" : "::";
        bport = "0";
        err = EINVAL;
        ERR("invalid bind address '%s', using host='%s' port=%s", pd->address_bind, bhost, bport);
     }
   else if (!bport) bport = "0";

   if (family == AF_INET)
     r = inet_pton(AF_INET, bhost, &bsa4.sin_addr);
   else
     r = inet_pton(AF_INET6, bhost, &bsa6.sin6_addr);
   if (r != 1)
     {
        if (r < 0) err = efl_net_socket_error_get();
        else err = EINVAL;
        ERR("invalid host='%s': %s", bhost, eina_error_msg_get(err));
        goto error_bind;
     }

   errno = 0;
   ul = strtoul(bport, &endptr, 10);
   if ((endptr == bport) || (endptr[0] != '\0'))
     errno = EINVAL;
   else if (ul > UINT16_MAX)
     errno = ERANGE;

   if (errno)
     {
        err = errno;
        ERR("invalid port numer '%s': %s", bport, strerror(errno));
        goto error_bind;
     }

   if (family == AF_INET)
     bsa4.sin_port = htons(ul);
   else
     bsa6.sin6_port = htons(ul);

   if (family == AF_INET)
     r = bind(fd, (struct sockaddr *)&bsa4, sizeof(bsa4));
   else
     r = bind(fd, (struct sockaddr *)&bsa6, sizeof(bsa6));
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        ERR("could not bind to host='%s', port=%s: %s", bhost, bport, eina_error_msg_get(err));
        goto error_bind;
     }

 error_bind:
   free(str);
   return err;
}

EOLIAN static void
_efl_net_socket_udp_efl_loop_fd_fd_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, int pfd)
{
   SOCKET fd = (SOCKET)pfd;
   Eina_List *node;
   struct sockaddr_storage addr;
   socklen_t addrlen;
   int family;

   efl_loop_fd_set(efl_super(o, MY_CLASS), pfd);

   if (fd == INVALID_SOCKET) return;

   family = efl_net_socket_fd_family_get(o);
   if (family == AF_UNSPEC) return;

   /* apply postponed values or fetch & sync */
   if (pd->cork == 0xff)
     efl_net_socket_udp_cork_get(o);
   else
     efl_net_socket_udp_cork_set(o, pd->cork);

   if (pd->dont_route == 0xff)
     efl_net_socket_udp_dont_route_get(o);
   else
     efl_net_socket_udp_dont_route_set(o, pd->dont_route);

   if (pd->reuse_address == 0xff)
     efl_net_socket_udp_reuse_address_get(o);
   else
     efl_net_socket_udp_reuse_address_set(o, pd->reuse_address);

   if (pd->reuse_port == 0xff)
     efl_net_socket_udp_reuse_port_get(o);
   else
     efl_net_socket_udp_reuse_port_set(o, pd->reuse_port);

   _efl_net_socket_udp_bind(o, pd);

   EINA_LIST_FREE(pd->multicast.pending, node)
     {
        const char *mcast_addr = node->data;
        Eina_Error mr = efl_net_multicast_join(fd, family, mcast_addr);
        if (mr)
          ERR("could not join pending multicast group '%s': %s", mcast_addr, eina_error_msg_get(mr));
     }

   if (!pd->multicast.ttl_set)
     efl_net_socket_udp_multicast_time_to_live_get(o);
   else
     efl_net_socket_udp_multicast_time_to_live_set(o, pd->multicast.ttl);

   if (pd->multicast.loopback == 0xff)
     efl_net_socket_udp_multicast_loopback_get(o);
   else
     efl_net_socket_udp_multicast_loopback_set(o, pd->multicast.loopback);

   addrlen = sizeof(addr);
   if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
     ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
   else
     {
        char str[INET6_ADDRSTRLEN + sizeof("[]:65536")];
        if (efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr))
          efl_net_socket_address_local_set(o, str);
     }
}

EOLIAN static size_t
_efl_net_socket_udp_next_datagram_size_query(Eo *o, Efl_Net_Socket_Udp_Data *pd EINA_UNUSED)
{
   SOCKET fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return 0;
   return efl_net_udp_datagram_size_query(fd);
}

static inline int
_cork_option_get(void)
{
#if defined(HAVE_UDP_CORK)
   return UDP_CORK;
#else
   return -1;
#endif
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_cork_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Bool cork)
{
   SOCKET fd;
   int value, option;
   Eina_Bool old = pd->cork;

   option = _cork_option_get();
   if (EINA_UNLIKELY(option < 0))
     {
        if (cork)
          ERR("Could not find a UDP_CORK equivalent on your system");
        return EINA_FALSE;
     }

   pd->cork = cork;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = cork;
   if (setsockopt(fd, IPPROTO_UDP, option, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", IPPROTO_UDP, 0x%x, %d): %s",
            fd, option, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->cork = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_cork_get(const Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   SOCKET fd;
   int value = 0;
   socklen_t valuelen;
   int option;

   option = _cork_option_get();
   if (EINA_UNLIKELY(option < 0))
     {
        WRN("Could not find a UDP_CORK equivalent on your system");
        return EINA_FALSE;
     }

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->cork;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, IPPROTO_UDP, option, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", IPPROTO_UDP, 0x%x): %s",
            fd, option, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->cork = !!value; /* sync */
   return pd->cork;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_dont_route_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Bool dont_route)
{
   Eina_Bool old = pd->dont_route;
   SOCKET fd = efl_loop_fd_get(o);
#ifdef _WIN32
   DWORD value = dont_route;
#else
   int value = dont_route;
#endif

   pd->dont_route = dont_route;

   if (fd == INVALID_SOCKET) return EINA_TRUE;

   if (setsockopt(fd, SOL_SOCKET, SO_DONTROUTE, (const char *)&value, sizeof(value)) != 0)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_DONTROUTE, %u): %s", fd, dont_route, eina_error_msg_get(err));
        pd->dont_route = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_dont_route_get(const Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);
#ifdef _WIN32
   DWORD value;
#else
   int value;
#endif
   socklen_t valuelen;

   if (fd == INVALID_SOCKET) return pd->dont_route;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_DONTROUTE, (char *)&value, &valuelen) != 0)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_DONTROUTE): %s", fd, eina_error_msg_get(err));
        return EINA_FALSE;
     }

   pd->dont_route = !!value; /* sync */
   return pd->dont_route;
}


EOLIAN static Eina_Bool
_efl_net_socket_udp_reuse_address_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Bool reuse_address)
{
   SOCKET fd;
   int value;
   Eina_Bool old = pd->reuse_address;

   pd->reuse_address = reuse_address;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = reuse_address;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEADDR, %d): %s",
            fd, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->reuse_address = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_reuse_address_get(const Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   SOCKET fd;
   int value = 0;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->reuse_address;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEADDR): %s",
            fd, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->reuse_address = !!value; /* sync */
   return pd->reuse_address;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_reuse_port_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Bool reuse_port)
{
#ifdef SO_REUSEPORT
   SOCKET fd;
   int value;
   Eina_Bool old = pd->reuse_port;
#endif

   pd->reuse_port = reuse_port;

#ifdef SO_REUSEPORT
   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = reuse_port;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEPORT, %d): %s",
            fd, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->reuse_port = old;
        return EINA_FALSE;
     }
#else
   (void)o;
#endif

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_reuse_port_get(const Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
#ifdef SO_REUSEPORT
   SOCKET fd;
   int value = 0;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->reuse_port;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEPORT): %s",
            fd, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->reuse_port = !!value; /* sync */
#else
   (void)o;
#endif

   return pd->reuse_port;
}

EOLIAN static Eina_Error
_efl_net_socket_udp_bind_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, const char *address)
{
   SOCKET fd = efl_loop_fd_get(o);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(fd != INVALID_SOCKET, EALREADY);

   eina_stringshare_replace(&pd->address_bind, address);
   return 0;
}

EOLIAN static const char *
_efl_net_socket_udp_bind_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Udp_Data *pd)
{
   return pd->address_bind;
}

EOLIAN Efl_Object *
_efl_net_socket_udp_efl_object_constructor(Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   pd->multicast.ttl = 1;
   pd->multicast.ttl_set = EINA_FALSE;
   pd->multicast.loopback = 0xff;
   pd->cork = 0xff;
   pd->dont_route = 0xff;
   pd->reuse_address = 0xff;
   pd->reuse_port = 0xff;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN void
_efl_net_socket_udp_efl_object_destructor(Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   if (pd->multicast.pending)
     {
        eina_list_free(pd->multicast.pending);
        pd->multicast.pending = NULL;
     }

   while (pd->multicast.groups)
     efl_net_socket_udp_multicast_leave(o, pd->multicast.groups->data);

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_bind, NULL);

   free(pd->addr_remote);
   pd->addr_remote = NULL;
   pd->addr_remote_len = 0;
}

EOLIAN static Eina_Error
_efl_net_socket_udp_efl_io_reader_read(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Rw_Slice *rw_slice)
{
   SOCKET fd = efl_io_reader_fd_get(o);
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);
   if (fd == INVALID_SOCKET) goto error;
   do
     {
        struct sockaddr_storage addr = {};
        socklen_t addrlen = sizeof(addr);
        r = recvfrom(fd, rw_slice->mem, rw_slice->len, 0, (struct sockaddr *)&addr, &addrlen);
        if (r == SOCKET_ERROR)
          {
             Eina_Error err = efl_net_socket_error_get();

             if (err == EINTR) continue;

             rw_slice->len = 0;
             rw_slice->mem = NULL;

             efl_io_reader_can_read_set(o, EINA_FALSE);
             return err;
          }

        if (addr.ss_family == AF_INET)
          {
             const struct sockaddr_in *a = (const struct sockaddr_in *)pd->addr_remote;
             uint32_t ipv4 = ntohl(a->sin_addr.s_addr);
             if ((ipv4 != INADDR_BROADCAST) && (ipv4 != INADDR_ANY) && (!IN_MULTICAST(ipv4)))
               {
                  if ((addrlen != pd->addr_remote_len) ||
                      (memcmp(&addr, pd->addr_remote, addrlen) != 0))
                    {
                       char buf[INET_ADDRSTRLEN + sizeof(":65536")];
                       efl_net_ip_port_fmt(buf, sizeof(buf), (struct sockaddr *)&addr);
                       ERR("dropping spurious datagram from %s (expected %s)", buf, efl_net_socket_address_remote_get(o));
                       rw_slice->len = 0;
                       rw_slice->mem = NULL;
                       efl_io_reader_can_read_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "read" */
                       return EAGAIN;
                    }
               }
          }
        else
          {
             const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)pd->addr_remote;
             if ((!IN6_IS_ADDR_MULTICAST(&a->sin6_addr)) && (memcmp(&a->sin6_addr, &in6addr_any, sizeof(in6addr_any)) != 0))
               {
                  if ((addrlen != pd->addr_remote_len) ||
                      (memcmp(&addr, pd->addr_remote, addrlen) != 0))
                    {
                       char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
                       efl_net_ip_port_fmt(buf, sizeof(buf), (struct sockaddr *)&addr);
                       ERR("dropping spurious datagram from %s (expected %s)", buf, efl_net_socket_address_remote_get(o));
                       rw_slice->len = 0;
                       rw_slice->mem = NULL;
                       efl_io_reader_can_read_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "read" */
                       return EAGAIN;
                    }
               }
          }
     }
   while (r == SOCKET_ERROR);

   rw_slice->len = r;
   efl_io_reader_can_read_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "read" */
   if (r == 0)
     efl_io_reader_eos_set(o, EINA_TRUE);

   return 0;

 error:
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(o, EINA_FALSE);
   return EINVAL;
}

EOLIAN static Eina_Error
_efl_net_socket_udp_efl_io_writer_write(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Slice *ro_slice, Eina_Slice *remaining)
{
   SOCKET fd = efl_io_writer_fd_get(o);
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ro_slice, EINVAL);
   if (fd == INVALID_SOCKET) goto error;

   do
     {
        r = sendto(fd, ro_slice->mem, ro_slice->len, 0, pd->addr_remote, pd->addr_remote_len);
        if (r == SOCKET_ERROR)
          {
             Eina_Error err = efl_net_socket_error_get();

             if (err == EINTR) continue;

             if (remaining) *remaining = *ro_slice;
             ro_slice->len = 0;
             ro_slice->mem = NULL;
             efl_io_writer_can_write_set(o, EINA_FALSE);
             return err;
          }
     }
   while (r == SOCKET_ERROR);

   if (remaining)
     {
        remaining->len = ro_slice->len - r;
        remaining->bytes = ro_slice->bytes + r;
     }
   ro_slice->len = r;
   efl_io_writer_can_write_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "write" */

   return 0;

 error:
   if (remaining) *remaining = *ro_slice;
   ro_slice->len = 0;
   ro_slice->mem = NULL;
   efl_io_writer_can_write_set(o, EINA_FALSE);
   return EINVAL;
}

static Eina_List *
_efl_net_socket_udp_multicast_find(const Eina_List *lst, const char *address)
{
   const char *str;
   const Eina_List *node;

   EINA_LIST_FOREACH(lst, node, str)
     {
        if (strcmp(str, address) == 0)
          return (Eina_List *)node;
     }

   return NULL;
}

EOLIAN static Eina_Error
_efl_net_socket_udp_multicast_join(Eo *o, Efl_Net_Socket_Udp_Data *pd, const char *address)
{
   const Eina_List *found;
   SOCKET fd = efl_loop_fd_get(o);

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   found = _efl_net_socket_udp_multicast_find(pd->multicast.groups, address);
   if (found) return EEXIST;

   pd->multicast.groups = eina_list_append(pd->multicast.groups, strdup(address));

   if (fd == INVALID_SOCKET)
     {
        pd->multicast.pending = eina_list_append(pd->multicast.pending, eina_list_last(pd->multicast.groups));
        return 0;
     }

   return efl_net_multicast_join(fd, efl_net_socket_fd_family_get(o), address);
}

EOLIAN static Eina_Error
_efl_net_socket_udp_multicast_leave(Eo *o, Efl_Net_Socket_Udp_Data *pd, const char *address)
{
   Eina_List *found;
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   found = _efl_net_socket_udp_multicast_find(pd->multicast.groups, address);
   if (!found) return ENOENT;

   if (fd == INVALID_SOCKET)
     {
        free(found->data);
        pd->multicast.pending = eina_list_remove(pd->multicast.pending, found);
        pd->multicast.groups = eina_list_remove_list(pd->multicast.groups, found);
        return 0;
     }

   err = efl_net_multicast_leave(fd, efl_net_socket_fd_family_get(o), address);

   free(found->data);
   pd->multicast.groups = eina_list_remove_list(pd->multicast.groups, found);
   return err;
}

EOLIAN static Eina_Iterator *
_efl_net_socket_udp_multicast_groups_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Udp_Data *pd)
{
   return eina_list_iterator_new(pd->multicast.groups);
}

EOLIAN static Eina_Error
_efl_net_socket_udp_multicast_time_to_live_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, uint8_t ttl)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   uint8_t old = pd->multicast.ttl;

   pd->multicast.ttl_set = EINA_TRUE;
   pd->multicast.ttl = ttl;

   if (fd == INVALID_SOCKET) return 0;

   err = efl_net_multicast_ttl_set(fd, efl_net_socket_fd_family_get(o), ttl);
   if (err)
     {
        ERR("could not set multicast time to live=%hhu: %s", ttl, eina_error_msg_get(err));
        pd->multicast.ttl = old;
     }

   return err;
}

EOLIAN static uint8_t
_efl_net_socket_udp_multicast_time_to_live_get(const Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   uint8_t ttl = pd->multicast.ttl;

   if (fd == INVALID_SOCKET) return pd->multicast.ttl;

   err = efl_net_multicast_ttl_get(fd, efl_net_socket_fd_family_get(o), &ttl);
   if (err)
     ERR("could not get multicast time to live: %s", eina_error_msg_get(err));
   else
     pd->multicast.ttl = ttl;

   return pd->multicast.ttl;
}

EOLIAN static Eina_Error
_efl_net_socket_udp_multicast_loopback_set(Eo *o, Efl_Net_Socket_Udp_Data *pd, Eina_Bool loopback)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   Eina_Bool old = pd->multicast.loopback;

   pd->multicast.loopback = loopback;

   if (fd == INVALID_SOCKET) return 0;

   err = efl_net_multicast_loopback_set(fd, efl_net_socket_fd_family_get(o), loopback);
   if (err)
     {
        ERR("could not set multicast loopback=%hhu: %s", loopback, eina_error_msg_get(err));
        pd->multicast.loopback = old;
     }

   return err;
}

EOLIAN static Eina_Bool
_efl_net_socket_udp_multicast_loopback_get(const Eo *o, Efl_Net_Socket_Udp_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   Eina_Bool loopback = pd->multicast.loopback;

   if (fd == INVALID_SOCKET) return pd->multicast.loopback;

   err = efl_net_multicast_loopback_get(fd, efl_net_socket_fd_family_get(o), &loopback);
   if (err)
     ERR("could not get multicast loopback: %s", eina_error_msg_get(err));
   else
     pd->multicast.loopback = loopback;

   return pd->multicast.loopback;
}

#include "efl_net_socket_udp.eo.c"
