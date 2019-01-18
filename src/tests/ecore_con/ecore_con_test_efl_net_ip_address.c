#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <Ecore.h>
#include <Ecore_Con.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include <ctype.h>

#include "ecore_con_suite.h"
#include "ecore_con_private.h"

struct log_ctx {
   const char *dom;
   const char *msg;
   int level;
   unsigned int did;
};

/* tests should not output on success, just uncomment this for debugging */
//#define SHOW_LOG 1

static void
_eina_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *ctx = data;
   const char *str;

   if (ctx->level != level) goto log;

   if (strcmp(fmt, "%s") != 0)
     str = fmt;
   else
     {
        va_list cp_args;
        va_copy(cp_args, args);
        str = va_arg(cp_args, const char *);
        va_end(cp_args);
     }

   if (ctx->dom)
     {
        if ((!d->name) || (strcmp(ctx->dom, d->name) != 0))
          goto log;
     }

   if (ctx->msg)
     {
        if (strcmp(ctx->msg, str) != 0)
          goto log;
     }

   ctx->did++;

#ifndef SHOW_LOG
   return;
#endif
 log:

   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

#define TRAP_ERRORS_BEGIN(_dom, _level, _msg) \
  do \
    { \
      struct log_ctx _log_ctx = { \
        .dom = #_dom, \
        .level = EINA_LOG_LEVEL_ ## _level, \
        .msg = _msg, \
      }; \
      eina_log_print_cb_set(_eina_test_safety_print_cb, &_log_ctx)

#define TRAP_ERRORS_FINISH(cnt) \
      eina_log_print_cb_set(eina_log_print_cb_stderr, NULL); \
      if (_log_ctx.did != cnt) \
        { \
           ck_abort_msg("Expected error %u (did: %u) messages to be logged to domain=%s, level=%d, mesage='%s'", cnt, _log_ctx.did, _log_ctx.dom, _log_ctx.level, _log_ctx.msg); \
        } \
    } \
  while (0)

static Eina_Value
_timeout(void *data,
         const Eina_Value t,
         const Eina_Future *dead EINA_UNUSED)
{
   Eina_Bool *did = data;

   if (t.type == EINA_VALUE_TYPE_ERROR)
     return t;

   *did = EINA_TRUE;
   ck_abort_msg("timed out!");

   return t;
}

#define LOOP_WITH_TIMEOUT(t) \
  do \
    { \
       Eina_Bool _did_timeout = EINA_FALSE; \
       Eina_Future *_timeout_future = efl_loop_timeout(efl_main_loop_get(), t); \
       eina_future_then(_timeout_future, _timeout, &_did_timeout, NULL); \
       mark_point(); \
       ecore_main_loop_begin(); \
       if (!_did_timeout) eina_future_cancel(_timeout_future); \
       else ck_abort_msg("Timed out!"); \
    } \
  while (0)


struct resolve_ctx {
   Eina_Value *results;
   Eina_Stringshare *canonical_name;
   Eina_Stringshare *request_address;
   Eina_Future *future;
   Eina_Error err;
};

static void
_resolve_cleanup(struct resolve_ctx *ctx)
{
   mark_point();

   if (ctx->results) eina_value_free(ctx->results);
   ctx->results = NULL;

   ctx->err = 0;
   eina_stringshare_replace(&ctx->canonical_name, NULL);
   eina_stringshare_replace(&ctx->request_address, NULL);

   if (ctx->future) eina_future_cancel(ctx->future);
   ctx->future = NULL;
}

static Eina_Bool
_resolve_found(const struct resolve_ctx *ctx, const char *string)
{
   const Efl_Net_Ip_Address *o;
   unsigned int i, len;

   ck_assert_ptr_ne(ctx->results, NULL);

   EINA_VALUE_ARRAY_FOREACH(ctx->results, len, i, o)
     {
        if (strcmp(string, efl_net_ip_address_string_get(o)) == 0)
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

#define _assert_found(...) _assert_found_internal(__FILE__, __LINE__, __VA_ARGS__)
static void
_assert_found_internal(const char *file, int line, const struct resolve_ctx *ctx, const char *string, Eina_Bool expected, Eina_Error err)
{
   Eina_Bool found;
   unsigned int i, len;
   const Efl_Net_Ip_Address *o;

   if (ctx->err != err)
     _ck_assert_failed(file, line, "Failed",
                       "Expected error=%d (%s), got %d (%s) resolving=%s",
                       err, err ? eina_error_msg_get(err) : "success",
                       ctx->err, ctx->err ? eina_error_msg_get(ctx->err) : "success",
                       string,
                       NULL);

   if (err) return;

   found = _resolve_found(ctx, string);
   if (found == expected) return;

   fprintf(stderr, "ERROR: did%s expect '%s' in results:\n",
           expected ? "" : " NOT", string);

   EINA_VALUE_ARRAY_FOREACH(ctx->results, len, i, o)
     fprintf(stderr, "result %u: %s\n", i, efl_net_ip_address_string_get(o));

   _ck_assert_failed(file, line, "Failed",
                     "Expected found=%hhu, got %hhu resolving=%s",
                     expected, found,
                     string,
                     NULL);
}

static Eina_Value
_resolve_done(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   struct resolve_ctx *ctx = data;
   Eina_Value_Array desc = { 0 };

   mark_point();

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     {
        eina_value_error_get(&v, &ctx->err);
        goto end;
     }

   eina_value_struct_get(&v, "canonical_name", &ctx->canonical_name);
   eina_value_struct_get(&v, "request_address", &ctx->request_address);
   eina_value_struct_get(&v, "results", &desc);

   ctx->results = eina_value_new(EINA_VALUE_TYPE_ARRAY);
   eina_value_pset(ctx->results, &desc);

 end:
   ecore_main_loop_quit();

   ctx->future = NULL;

   mark_point();

   return v;
}

static void
_resolve(struct resolve_ctx *ctx, const char *address, int family, int flags)
{
   ctx->future = efl_net_ip_address_resolve(EFL_NET_IP_ADDRESS_CLASS,
                                            address, family, flags);
   ck_assert_ptr_ne(ctx->future, NULL);
   ctx->future = eina_future_then(ctx->future, _resolve_done, ctx, NULL);

   LOOP_WITH_TIMEOUT(10);
}

/* IPv4 *****************************************************************/

static void
_ipv4_check(Eo *o, const struct sockaddr_in *addr)
{
   Eina_Slice slice = { .mem = &addr->sin_addr, .len = sizeof(addr->sin_addr) };
   Eina_Slice rs;
   char buf[INET_ADDRSTRLEN + sizeof(":65536")] = "";

   ck_assert_ptr_ne(o, NULL);

   ck_assert_int_eq(efl_net_ip_address_family_get(o), AF_INET);
   ck_assert_int_eq(efl_net_ip_address_port_get(o), ntohs(addr->sin_port));

   rs = efl_net_ip_address_get(o);
   ck_assert_int_eq(eina_slice_compare(rs, slice), 0);

   inet_ntop(AF_INET, slice.mem, buf, INET_ADDRSTRLEN);

   if (addr->sin_port)
     {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                 ":%hu", htons(addr->sin_port));
     }
   ck_assert_ptr_ne(efl_net_ip_address_string_get(o), NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), buf);
}

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_manual_ok)
{
   struct sockaddr_in addr = {
     .sin_family = AF_INET,
   };
   Eina_Slice slice = { .mem = &addr.sin_addr, .len = sizeof(addr.sin_addr) };
   Eo *o;

   addr.sin_port = htons(12345);
   addr.sin_addr.s_addr = htonl(0xabcdefafU);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET),
               efl_net_ip_address_port_set(efl_added, ntohs(addr.sin_port)),
               efl_net_ip_address_set(efl_added, slice));
   _ipv4_check(o, &addr);
   efl_unref(o);

   addr.sin_port = htons(8081);
   addr.sin_addr.s_addr = htonl(0);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET),
               efl_net_ip_address_port_set(efl_added, ntohs(addr.sin_port)),
               efl_net_ip_address_set(efl_added, slice));
   _ipv4_check(o, &addr);
   efl_unref(o);

   addr.sin_port = htons(0);
   addr.sin_addr.s_addr = htonl(0x12345678);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET),
               efl_net_ip_address_set(efl_added, slice));
   _ipv4_check(o, &addr);
   efl_unref(o);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_manual_fail)
{
   uint8_t c = 123;
   Eina_Slice wrong_slice = { .mem = &c, .len = 1 };
   struct sockaddr_in addr = {
     .sin_family = AF_INET,
   };
   Eina_Slice slice = { .mem = &addr.sin_addr, .len = sizeof(addr.sin_addr) };
   Eo *o;

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: pd->addr.sa_family == 0 is true");
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL);
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, 12345));
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(2);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: pd->addr.sa_family == 0 is true");
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_set(efl_added, wrong_slice));
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(2);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: pd->addr.sa_family == 0 is true");
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_port_set(efl_added, 1234));
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(2);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: pd->addr.sa_family == 0 is true");
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_port_set(efl_added, 1234),
               efl_net_ip_address_set(efl_added, wrong_slice));
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(3);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: rw_slice.len != address.len is true");
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET),
               efl_net_ip_address_set(efl_added, wrong_slice));
   _ipv4_check(o, &addr);
   efl_unref(o);
   TRAP_ERRORS_FINISH(1);

   addr.sin_port = htons(12345);
   addr.sin_addr.s_addr = htonl(0xabcdefafU);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET),
               efl_net_ip_address_port_set(efl_added, ntohs(addr.sin_port)),
               efl_net_ip_address_set(efl_added, slice));
   _ipv4_check(o, &addr);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "port already set to %hu, new %hu");
   efl_net_ip_address_port_set(o, ntohs(addr.sin_port));
   TRAP_ERRORS_FINISH(0);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "port already set to %hu, new %hu");
   efl_net_ip_address_port_set(o, 999);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: rw_slice.len != address.len is true");
   slice.len = 1;
   efl_net_ip_address_set(o, slice);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "address already set to %s, new %s");
   slice.len = sizeof(addr.sin_addr.s_addr);
   efl_net_ip_address_set(o, slice);
   TRAP_ERRORS_FINISH(0);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "address already set to %s, new %s");
   addr.sin_addr.s_addr = htonl(0x12345678);
   slice.len = sizeof(addr.sin_addr.s_addr);
   efl_net_ip_address_set(o, slice);
   TRAP_ERRORS_FINISH(1);

   efl_unref(o);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_create_ok)
{
   struct sockaddr_in addr = {
     .sin_family = AF_INET,
   };
   Eina_Slice slice = { .mem = &addr.sin_addr, .len = sizeof(addr.sin_addr) };
   Eo *o;

   addr.sin_port = htons(12345);
   addr.sin_addr.s_addr = htonl(0xabcdefafU);
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 ntohs(addr.sin_port),
                                 slice);
   _ipv4_check(o, &addr);
   efl_unref(o);

   addr.sin_port = htons(8081);
   addr.sin_addr.s_addr = htonl(0);
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 ntohs(addr.sin_port),
                                 slice);
   _ipv4_check(o, &addr);
   efl_unref(o);

   addr.sin_port = htons(0);
   addr.sin_addr.s_addr = htonl(0x12345678);
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 ntohs(addr.sin_port),
                                 slice);
   _ipv4_check(o, &addr);
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_create_fail)
{
   uint8_t c = 123;
   Eina_Slice wrong_slice = { .mem = &c, .len = 1 };
   Eo *o;

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: address.len != 4 && address.len != 16 is true");
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 1234,
                                 wrong_slice);
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(1);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_create_sockaddr_ok)
{
   struct sockaddr_in addr = {
     .sin_family = AF_INET,
   };
   Eo *o;

   addr.sin_port = htons(12345);
   addr.sin_addr.s_addr = htonl(0xabcdefafU);
   o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &addr);
   ck_assert_ptr_ne(&addr, efl_net_ip_address_sockaddr_get(o));
   _ipv4_check(o, &addr);
   efl_unref(o);

   addr.sin_port = htons(0);
   addr.sin_addr.s_addr = htonl(0);
   o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &addr);
   ck_assert_ptr_ne(&addr, efl_net_ip_address_sockaddr_get(o));
   _ipv4_check(o, &addr);
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_create_sockaddr_fail)
{
   struct sockaddr_in addr = {
     .sin_family = AF_UNSPEC,
   };
   Eo *o;

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: sockaddr == NULL");
   o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, NULL);
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: (sockaddr->sa_family != AF_INET) && (sockaddr->sa_family != AF_INET6) is true");
   o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &addr);
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(1);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_parse_ok)
{
   Eo *o;

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "127.0.0.1:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "127.0.0.1:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "127.0.0.1:0");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "127.0.0.1");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "127.0.0.1");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "127.0.0.1");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "192.168.0.123:80");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "192.168.0.123:80");
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_parse_fail)
{
   Eo *o;

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: numeric_address == NULL");
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, NULL);
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(1);

   /* incomplete numbers */
   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "127.");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "127.0.0.");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

   /* hostnames are not numeric, shouldn't return an object */
   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "google.com");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

   /* port names are not numeric, shouldn't return an object */
   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "127.0.0.1:http");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_resolve_ok)
{
   struct resolve_ctx ctx = { };

   _resolve(&ctx, "localhost:http", 0, 0);
   _assert_found(&ctx, "127.0.0.1:80", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "localhost", 0, 0);
   _assert_found(&ctx, "127.0.0.1", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "127.0.0.1", 0, 0);
   _assert_found(&ctx, "127.0.0.1", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "127.0.0.1:http", 0, 0);
   _assert_found(&ctx, "127.0.0.1:80", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "127.0.0.1:80", 0, 0);
   _assert_found(&ctx, "127.0.0.1:80", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "localhost:80", 0, 0);
   _assert_found(&ctx, "127.0.0.1:80", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "localhost:http", AF_INET, 0);
   _assert_found(&ctx, "[::1]:80", EINA_FALSE, 0);
   _resolve_cleanup(&ctx);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_resolve_fail)
{
   struct resolve_ctx ctx = { };

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: address == NULL");
   ctx.future = efl_net_ip_address_resolve(EFL_NET_IP_ADDRESS_CLASS,
                                           NULL, 0, 0);
   ck_assert_ptr_eq(ctx.future, NULL);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: (family != AF_UNSPEC) && (family != AF_INET) && (family != AF_INET6) is true");
   ctx.future = efl_net_ip_address_resolve(EFL_NET_IP_ADDRESS_CLASS,
                                           "localhost", 1234, 0);
   ck_assert_ptr_eq(ctx.future, NULL);
   TRAP_ERRORS_FINISH(1);

   _resolve(&ctx, "xxlocalhost:xxhttp", 0, 0);
   _assert_found(&ctx, "xxlocalhost:xxhttp", EINA_FALSE, EFL_NET_ERROR_COULDNT_RESOLVE_HOST);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "[::1]:http", AF_INET, 0);
   _assert_found(&ctx, "[::1]:http", EINA_FALSE, EFL_NET_ERROR_COULDNT_RESOLVE_HOST);
   _resolve_cleanup(&ctx);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv4_checks)
{
   const struct test {
      uint32_t addr;
      Eina_Bool is_a, is_b, is_c, is_d, is_multicast, is_loopback, is_any;
   } *itr, tests[] = {
#define TEST(_addr) \
     { .addr = _addr, \
     .is_a = IN_CLASSA(_addr), \
     .is_b = IN_CLASSB(_addr), \
     .is_c = IN_CLASSC(_addr), \
     .is_d = IN_CLASSD(_addr), \
     .is_multicast = IN_MULTICAST(_addr) , \
     .is_loopback = _addr == INADDR_LOOPBACK, \
     .is_any = _addr == INADDR_ANY, \
     }
     TEST(INADDR_LOOPBACK),
     TEST(INADDR_ANY),
     TEST(0x0a000001),
     TEST(0x80000002),
     TEST(0xc0000003),
     TEST(0xe0000004),
#undef TEST
   };

   for (itr = tests; itr < tests + sizeof(tests)/sizeof(tests[0]); itr++)
     {
        struct sockaddr_in a = {
          .sin_family = AF_INET,
          .sin_port = 0,
          .sin_addr.s_addr = htonl(itr->addr),
        };
        Eo *o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &a);
        ck_assert_ptr_ne(o, NULL);

        ck_assert_int_eq(efl_net_ip_address_ipv4_class_a_check(o), itr->is_a);
        ck_assert_int_eq(efl_net_ip_address_ipv4_class_b_check(o), itr->is_b);
        ck_assert_int_eq(efl_net_ip_address_ipv4_class_c_check(o), itr->is_c);
        ck_assert_int_eq(efl_net_ip_address_ipv4_class_d_check(o), itr->is_d);
        ck_assert_int_eq(efl_net_ip_address_multicast_check(o), itr->is_multicast);
        ck_assert_int_eq(efl_net_ip_address_loopback_check(o), itr->is_loopback);
        ck_assert_int_eq(efl_net_ip_address_any_check(o), itr->is_any);

        ck_assert_int_eq(efl_net_ip_address_ipv6_v4mapped_check(o), EINA_FALSE);
        ck_assert_int_eq(efl_net_ip_address_ipv6_v4compat_check(o), EINA_FALSE);
        ck_assert_int_eq(efl_net_ip_address_ipv6_local_link_check(o), EINA_FALSE);
        ck_assert_int_eq(efl_net_ip_address_ipv6_local_site_check(o), EINA_FALSE);

        efl_unref(o);
     }

}
EFL_END_TEST

/* IPv6 *****************************************************************/

static void
_ipv6_check(Eo *o, const struct sockaddr_in6 *addr)
{
   Eina_Slice slice = { .mem = &addr->sin6_addr, .len = sizeof(addr->sin6_addr) };
   Eina_Slice rs;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";

   ck_assert_ptr_ne(o, NULL);

   ck_assert_int_eq(efl_net_ip_address_family_get(o), AF_INET6);
   ck_assert_int_eq(efl_net_ip_address_port_get(o), ntohs(addr->sin6_port));

   rs = efl_net_ip_address_get(o);
   ck_assert_int_eq(eina_slice_compare(rs, slice), 0);

   buf[0] = '[';
   inet_ntop(AF_INET6, slice.mem, buf + 1, INET6_ADDRSTRLEN);
   buf[strlen(buf)] = ']';

   if (addr->sin6_port)
     {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                 ":%hu", htons(addr->sin6_port));
     }

   ck_assert_ptr_ne(efl_net_ip_address_string_get(o), NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), buf);
}

static void
_ipv6_set(struct sockaddr_in6 *addr, uint16_t s1, uint16_t s2, uint16_t s3, uint16_t s4, uint16_t s5, uint16_t s6, uint16_t s7, uint16_t s8)
{
   uint16_t s[8] = { s1, s2, s3, s4, s5, s6, s7, s8 };
   memcpy(&addr->sin6_addr, s, 16);
}

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_manual_ok)
{
   struct sockaddr_in6 addr = {
     .sin6_family = AF_INET6,
   };
   Eina_Slice slice = { .mem = &addr.sin6_addr, .len = sizeof(addr.sin6_addr) };
   Eo *o;

   addr.sin6_port = htons(12345);
   _ipv6_set(&addr, 1, 2, 3, 4, 5, 6, 7, 8);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET6),
               efl_net_ip_address_port_set(efl_added, ntohs(addr.sin6_port)),
               efl_net_ip_address_set(efl_added, slice));
   _ipv6_check(o, &addr);
   efl_unref(o);

   addr.sin6_port = htons(8081);
   _ipv6_set(&addr, 0, 0, 0, 0, 0, 0, 0, 0);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET6),
               efl_net_ip_address_port_set(efl_added, ntohs(addr.sin6_port)),
               efl_net_ip_address_set(efl_added, slice));
   _ipv6_check(o, &addr);
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_manual_fail)
{
   uint8_t c = 123;
   Eina_Slice wrong_slice = { .mem = &c, .len = 1 };
   struct sockaddr_in6 addr = {
     .sin6_family = AF_INET6,
   };
   Eina_Slice slice = { .mem = &addr.sin6_addr, .len = sizeof(addr.sin6_addr) };
   Eo *o;

   /* generic errors checked at ecore_test_efl_net_ip_address_ipv4_manual_fail */

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: rw_slice.len != address.len is true");
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET6),
               efl_net_ip_address_set(efl_added, wrong_slice));
   _ipv6_check(o, &addr);
   efl_unref(o);
   TRAP_ERRORS_FINISH(1);

   addr.sin6_port = htons(12345);
   _ipv6_set(&addr, 0, 0, 0, 0, 0, 0, 0, 1);
   o = efl_add_ref(EFL_NET_IP_ADDRESS_CLASS, NULL,
               efl_net_ip_address_family_set(efl_added, AF_INET6),
               efl_net_ip_address_port_set(efl_added, ntohs(addr.sin6_port)),
               efl_net_ip_address_set(efl_added, slice));
   _ipv6_check(o, &addr);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "port already set to %hu, new %hu");
   efl_net_ip_address_port_set(o, ntohs(addr.sin6_port));
   TRAP_ERRORS_FINISH(0);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "port already set to %hu, new %hu");
   efl_net_ip_address_port_set(o, 999);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(eina_safety, ERR, "safety check failed: rw_slice.len != address.len is true");
   slice.len = 1;
   efl_net_ip_address_set(o, slice);
   TRAP_ERRORS_FINISH(1);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "address already set to %s, new %s");
   slice.len = sizeof(addr.sin6_addr);
   efl_net_ip_address_set(o, slice);
   TRAP_ERRORS_FINISH(0);

   TRAP_ERRORS_BEGIN(ecore_con, ERR, "address already set to %s, new %s");
   _ipv6_set(&addr, 1, 2, 3, 4, 5, 6, 7, 8);
   slice.len = sizeof(addr.sin6_addr);
   efl_net_ip_address_set(o, slice);
   TRAP_ERRORS_FINISH(1);

   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_create_ok)
{
   struct sockaddr_in6 addr = {
     .sin6_family = AF_INET6,
   };
   Eina_Slice slice = { .mem = &addr.sin6_addr, .len = sizeof(addr.sin6_addr) };
   Eo *o;

   addr.sin6_port = htons(12365);
   _ipv6_set(&addr, 1, 2, 3, 4, 5, 6, 7, 8);
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 ntohs(addr.sin6_port),
                                 slice);
   _ipv6_check(o, &addr);
   efl_unref(o);

   addr.sin6_port = htons(8081);
   _ipv6_set(&addr, 0, 0, 0, 0, 0, 0, 0, 0);
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 ntohs(addr.sin6_port),
                                 slice);
   _ipv6_check(o, &addr);
   efl_unref(o);

   addr.sin6_port = htons(0);
   _ipv6_set(&addr, 0, 0, 0, 0, 0, 0, 0, 1);
   o = efl_net_ip_address_create(EFL_NET_IP_ADDRESS_CLASS,
                                 ntohs(addr.sin6_port),
                                 slice);
   _ipv6_check(o, &addr);
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_create_sockaddr_ok)
{
   struct sockaddr_in6 addr = {
     .sin6_family = AF_INET6,
   };
   Eo *o;

   addr.sin6_port = htons(12345);
   _ipv6_set(&addr, 1, 2, 3, 4, 5, 6, 7, 8);
   o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &addr);
   ck_assert_ptr_ne(&addr, efl_net_ip_address_sockaddr_get(o));
   _ipv6_check(o, &addr);
   efl_unref(o);

   addr.sin6_port = htons(0);
   _ipv6_set(&addr, 0, 0, 0, 0, 0, 0, 0, 0);
   o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &addr);
   ck_assert_ptr_ne(&addr, efl_net_ip_address_sockaddr_get(o));
   _ipv6_check(o, &addr);
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_parse_ok)
{
   Eo *o;

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::1]:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::1]:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::1]:0");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::1]");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::1]");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::1]");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "::1");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::1]");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::]:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::]:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::]:0");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::]");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::]");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::]");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "::");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::]");
   efl_unref(o);

   /* IPv4 Mapped */
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::ffff:192.168.0.1]:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::ffff:192.168.0.1]:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::ffff:192.168.0.1]");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::ffff:192.168.0.1]");
   efl_unref(o);

   /* IPv4 Compatible */
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::192.168.0.1]:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::192.168.0.1]:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::192.168.0.1]");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[::192.168.0.1]");
   efl_unref(o);

   /* Link Local */
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[fe80::1]:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[fe80::1]:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[fe80::1]");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[fe80::1]");
   efl_unref(o);

   /* Site Local */
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[fc00::1]:12345");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[fc00::1]:12345");
   efl_unref(o);

   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[fc00::1]");
   ck_assert_ptr_ne(o, NULL);
   ck_assert_str_eq(efl_net_ip_address_string_get(o), "[fc00::1]");
   efl_unref(o);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_parse_fail)
{
   Eo *o;

   /* generic error (null ptr) checked in ipv4_parse_fail */

   /* incomplete numbers */
   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "::9999999");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "ab:cd:ef:gh");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

   /* port names are not numeric, shouldn't return an object */
   TRAP_ERRORS_BEGIN(eina_safety, ERR, NULL);
   o = efl_net_ip_address_parse(EFL_NET_IP_ADDRESS_CLASS, "[::1]:http");
   ck_assert_ptr_eq(o, NULL);
   TRAP_ERRORS_FINISH(0); /* no error messages! */

}
EFL_END_TEST

static const char *
_ipv6_localhost_check(void)
{
#ifndef ETC_HOSTS
#ifdef _WIN32
#define ETC_HOSTS "C:\\Windows\\System32\\Drivers\\etc\\hosts"
#else
#define ETC_HOSTS "/etc/hosts"
#endif
#endif
   Eina_File *f;
   Eina_Iterator *it;
   Eina_File_Line *line;
   static const char localhost[] = "localhost";
   static const char localhost6[] = "localhost6";
   const char *found = NULL;

   f = eina_file_open(ETC_HOSTS, EINA_FALSE);
   if (!f)
     {
        fprintf(stderr, "WARNING: your system misses %s: %s\n", ETC_HOSTS, eina_error_msg_get(eina_error_get() ? : errno));
        return NULL;
     }

   it = eina_file_map_lines(f);
   if (!it)
     {
        fprintf(stderr, "ERROR: could not map lines of %s\n", ETC_HOSTS);
        goto end;
     }
   EINA_ITERATOR_FOREACH(it, line)
     {
        const char *p;

        if (line->length < strlen("::1 localhost")) continue;

        for (p = line->start; p < line->end; p++)
          if (!isspace(p[0])) break;

        if (p >= line->end) continue;
        if (p[0] == '#') continue;
        if ((size_t)(line->end - p) < strlen("::1 localhost")) continue;
        if (memcmp(p, "::1", strlen("::1")) != 0) continue;

        p += strlen("::1");

        while (p < line->end)
          {
             const char *e;

             if (!isspace(p[0]))
               {
                  p = line->end;
                  break;
               }
             p++;

             for (; p < line->end; p++)
               if (!isspace(p[0])) break;
             if (p >= line->end) break;
             if (p[0] == '#')
               {
                  p = line->end;
                  break;
               }

             for (e = p; e < line->end; e++)
               if (isspace(e[0])) break;

             if (e > p)
               {
                  size_t len = e - p;
                  if ((len == strlen(localhost)) && (memcmp(p, localhost, strlen(localhost)) == 0))
                    found = localhost;
                  else if ((len == strlen(localhost6)) && (memcmp(p, localhost6, strlen(localhost6)) == 0))
                    found = localhost6;
               }
             p = e;
          }
        if (found) break;
     }
   eina_iterator_free(it);

 end:
   if (!found) fprintf(stderr, "WARNING: your system miss '::1 localhost' or '::1 localhost6' in %s\n", ETC_HOSTS);
   eina_file_close(f);
   return found;
}

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_resolve_ok)
{
   struct resolve_ctx ctx = { };
   char buf[4096];
   const char *localhost_str;

   localhost_str = _ipv6_localhost_check();

   if (localhost_str)
     {
        snprintf(buf, sizeof(buf), "%s:http", localhost_str);
        _resolve(&ctx, buf, 0, 0);
        _assert_found(&ctx, "[::1]:80", EINA_TRUE, 0);
        _resolve_cleanup(&ctx);

        _resolve(&ctx, localhost_str, 0, 0);
        _assert_found(&ctx, "[::1]", EINA_TRUE, 0);
        _resolve_cleanup(&ctx);
     }

   _resolve(&ctx, "::1", 0, 0);
   _assert_found(&ctx, "[::1]", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

#if defined(AI_V4MAPPED) && (AI_V4MAPPED > 0)
   _resolve(&ctx, "127.0.0.1", AF_INET6, AI_V4MAPPED);
   _assert_found(&ctx, "[::ffff:127.0.0.1]", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);
#endif

   _resolve(&ctx, "[::1]:http", 0, 0);
   _assert_found(&ctx, "[::1]:80", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   _resolve(&ctx, "[::1]:80", 0, 0);
   _assert_found(&ctx, "[::1]:80", EINA_TRUE, 0);
   _resolve_cleanup(&ctx);

   if (localhost_str)
     {
        snprintf(buf, sizeof(buf), "%s:80", localhost_str);
        _resolve(&ctx, buf, 0, 0);
        _assert_found(&ctx, "[::1]:80", EINA_TRUE, 0);
        _resolve_cleanup(&ctx);

        snprintf(buf, sizeof(buf), "%s:http", localhost_str);
        _resolve(&ctx, buf, AF_INET6, 0);
        _assert_found(&ctx, "127.0.0.1:80", EINA_FALSE, 0);
        _resolve_cleanup(&ctx);
     }

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_resolve_fail)
{
   struct resolve_ctx ctx = { };

   /* generic checks at ipv4_resolve_fail */

#if defined(AI_V4MAPPED) && (AI_V4MAPPED > 0)
   _resolve(&ctx, "127.0.0.1:http", AF_INET6, AI_CANONNAME); /* do NOT set V4MAPPED, but use non-zero */
   _assert_found(&ctx, "127.0.0.1:http", EINA_FALSE, EFL_NET_ERROR_COULDNT_RESOLVE_HOST);
   _resolve_cleanup(&ctx);
#endif

}
EFL_END_TEST

EFL_START_TEST(ecore_test_efl_net_ip_address_ipv6_checks)
{
   const struct test {
      const char *str;
   } *itr, tests[] = {
#define TEST(_addr) { .str = _addr }
     TEST("::1"),
     TEST("::"),
     TEST("1:2:3:4:5:6:7:8"),
     TEST("::ffff:192.168.0.1"),
     TEST("::192.168.0.1"),
     TEST("fe80::1"),
     TEST("fc00::2"),
#undef TEST
   };

   for (itr = tests; itr < tests + sizeof(tests)/sizeof(tests[0]); itr++)
     {
        struct sockaddr_in6 a = {
          .sin6_family = AF_INET6,
          .sin6_port = 0,
        };
        const struct in6_addr any = { };
        struct in6_addr *ia = &a.sin6_addr;
        int r;

        r = inet_pton(AF_INET6, itr->str, ia);
        ck_assert_int_eq(r, 1);

        Eo *o = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, &a);
        ck_assert_ptr_ne(o, NULL);

        ck_assert_int_eq(efl_net_ip_address_ipv4_class_a_check(o), EINA_FALSE);
        ck_assert_int_eq(efl_net_ip_address_ipv4_class_b_check(o), EINA_FALSE);
        ck_assert_int_eq(efl_net_ip_address_ipv4_class_c_check(o), EINA_FALSE);
        ck_assert_int_eq(efl_net_ip_address_ipv4_class_d_check(o), EINA_FALSE);

        ck_assert_int_eq(efl_net_ip_address_multicast_check(o), IN6_IS_ADDR_MULTICAST(ia));
        ck_assert_int_eq(efl_net_ip_address_loopback_check(o), IN6_IS_ADDR_LOOPBACK(ia));
        ck_assert_int_eq(efl_net_ip_address_any_check(o), memcmp(ia, &any, 16) == 0);

        ck_assert_int_eq(efl_net_ip_address_ipv6_v4mapped_check(o), IN6_IS_ADDR_V4MAPPED(ia));
        ck_assert_int_eq(efl_net_ip_address_ipv6_v4compat_check(o), IN6_IS_ADDR_V4COMPAT(ia));
        ck_assert_int_eq(efl_net_ip_address_ipv6_local_link_check(o), IN6_IS_ADDR_LINKLOCAL(ia));
        ck_assert_int_eq(efl_net_ip_address_ipv6_local_site_check(o), IN6_IS_ADDR_SITELOCAL(ia));

        efl_unref(o);
     }

}
EFL_END_TEST

void ecore_con_test_efl_net_ip_address(TCase *tc)
{
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_manual_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_manual_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_create_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_create_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_create_sockaddr_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_create_sockaddr_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_parse_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_parse_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_resolve_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_resolve_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv4_checks);

   if (!getenv("EFL_TEST_ECORE_CON_IPV6"))
     {
        fputs("\nWARNING: your system has IPv6 disabled. Skipping IPv6 tests!\n\n"
              "\nexport EFL_TEST_ECORE_CON_IPV6=1\n"
              "\nTo enable it\n",
              stderr);
        return;
     }

   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_manual_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_manual_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_create_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_create_sockaddr_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_parse_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_parse_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_resolve_ok);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_resolve_fail);
   tcase_add_test(tc, ecore_test_efl_net_ip_address_ipv6_checks);
}
