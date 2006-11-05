/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "config.h"
#include "ecore_private.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static void _ecore_dbus_address_list_free_cb(void *data);

static int _ecore_dbus_address_value_char_optional_encode(char c);
static char * _ecore_dbus_address_value_decode(const char *value);
static char * _ecore_dbus_address_value_encode(const char *value);

static void
_ecore_dbus_address_list_free_cb(void *data)
{
  if (data) free(data);
}

Ecore_DBus_Address *
ecore_dbus_address_new()
{
   Ecore_DBus_Address *a;
   a = calloc(1, sizeof(Ecore_DBus_Address));
   if (!a) return NULL;

   a->keys = ecore_list_new();
   ecore_list_set_free_cb(a->keys, _ecore_dbus_address_list_free_cb);
   a->vals = ecore_list_new();
   ecore_list_set_free_cb(a->vals, _ecore_dbus_address_list_free_cb);

   return a;
}

void
ecore_dbus_address_free(Ecore_DBus_Address *address)
{
   if (!address) return;
   ecore_list_destroy(address->keys);
   ecore_list_destroy(address->vals);
   if (address->transport) free(address->transport);
   free(address);
}


/**
 * Parse an address into an array of Ecore_DBus_Address structs
 */
Ecore_List *
ecore_dbus_address_parse(const char *address) 
{
   Ecore_List *alist = NULL;

   Ecore_DBus_Address *a = NULL;
   char *addcpy;
   char *p;

   addcpy = strdup(address);
   p = addcpy;
   char *start = NULL;
   char *transport = p;
   char *key = NULL;
   char *val = NULL;
   int error = 0;

   alist = ecore_list_new();
   ecore_list_set_free_cb(alist, ECORE_FREE_CB(ecore_dbus_address_free));

   while(1)
     {
	if (!a)
	  {
	     start = p;
	     a = ecore_dbus_address_new();
	     if (!a) { error = 1; break; }
	  }

	if (!*p || *p == ';' || *p == ',')
	  {
	     /* append value */
	     char sep = *p;

	     if (!val) 
	       {
		  if (p != start) error = 1;
		  break;
	       }
	     *p = '\0';
	     ecore_list_append(a->vals, _ecore_dbus_address_value_decode(val));
	     val = NULL;

	     if (sep == ',')
	       {
		  key = p + 1;
	       }
	     else
	       {
		  /* append address to list */
		  ecore_list_append(alist, a);
		  a = NULL;
		  if (!sep) break; /* end of string */
		  transport = p + 1;
	       }
	  }
	else if (*p == '=')
	  {
	     /* append key */
	     if (!key) { error = 1; break; }
	     *p = '\0';
	     ecore_list_append(a->keys, strdup(key));
	     key = NULL;
	     val = p + 1;
	  }
	else if (*p == ':')
	  {
	     /* append transport */
	     if (!transport) { error = 1; break; }
	     *p = '\0';
	     a->transport = strdup(transport);
	     transport = NULL;
	     key = p + 1;
	  }
	p++;
     }

   if (error) 
     {
	ecore_list_destroy(alist);
	alist = NULL;
     }
   free(addcpy);
   return alist;
}

const char *
ecore_dbus_address_value_get(Ecore_DBus_Address *address, const char *key)
{
   int i;
   char *s;
   if (!key) return NULL;

   ecore_list_goto_first(address->keys);
   i = 0;
   while((s = ecore_list_next(address->keys)))
     {
	if (!strcmp(key, s))
	  {
	     return ecore_list_goto_index(address->vals, i);
	  }
	i++;
     }
   return NULL;
}

EAPI char *
ecore_dbus_address_string(Ecore_DBus_Address *address)
{
   char buf[PATH_MAX];
   char *key, *val;
   int left = PATH_MAX - 1; /* space left in the buffer, leaving room for a final null */

   if (!address) return NULL;
  
   snprintf(buf, PATH_MAX, "%s:", address->transport);
   left -= strlen(address->transport) + 1;
   ecore_list_goto_first(address->keys);
   ecore_list_goto_first(address->vals);
   while ((key = ecore_list_next(address->keys)) && (val = ecore_list_next(address->vals)))
     {
	char *encval;
	strncat(buf, key, left);
	left -= strlen(key);
	strncat(buf, "=", left);
	left -= 1;
	encval = _ecore_dbus_address_value_encode(val);
	strncat(buf, encval, left);
	left -= strlen(encval);
	free(encval);
     }
   return strdup(buf);
}

/**
 * Connect to the first successful server in a list of addresses.
 */
EAPI Ecore_DBus_Server *
ecore_dbus_address_list_connect(Ecore_List *addrs, const void *data)
{
  Ecore_DBus_Address *addr;
  ecore_list_goto_first(addrs);
  /* try each listed address in turn */
  while ((addr = ecore_list_next(addrs))) 
    {
       Ecore_DBus_Server *svr;
       svr = ecore_dbus_address_connect(addr, data);
       if (svr) return svr;
    }
  return NULL;
}

/**
 * Connect to a server by its Ecore_DBus_Address
 */
EAPI Ecore_DBus_Server *
ecore_dbus_address_connect(Ecore_DBus_Address *addr, const void *data)
{
  const char *name;
  int type;
  int port;

  char *addr_string;
  addr_string = ecore_dbus_address_string(addr);
  printf("[ecore_dbus] connecting to address: %s\n", addr_string);
  free(addr_string);

  if (!strcmp(addr->transport, "unix")) 
    {
       type = ECORE_CON_LOCAL_SYSTEM;
       name = ecore_dbus_address_value_get(addr, "path");

       if (!name)
	 {
	    name = ecore_dbus_address_value_get(addr, "abstract");
	    type = ECORE_CON_LOCAL_ABSTRACT;
	 }

       if (!name) return NULL;
       port = -1;
    }
  else if (!strcmp(addr->transport, "tcp")) 
    {
       /* XXX implement (and verify transport name is actually 'tcp') */
       return NULL;
    }
  else
    {
       return NULL;
    }
  return ecore_dbus_server_connect(type, name, port, data);
}

void
ecore_dbus_print_address_list(Ecore_List *addresses)
{
   Ecore_DBus_Address *a;

   ecore_list_goto_first(addresses);
   while((a = ecore_list_next(addresses)))
     {
	char *k, *v;
	printf("Transport: %s\n", a->transport);

	ecore_list_goto_first(a->keys);
	ecore_list_goto_first(a->vals);
	k = ecore_list_next(a->keys); 
	v = ecore_list_next(a->vals);
	while (k || v)
	  {
	     printf("    %s => %s\n", k, v);
	     k = ecore_list_next(a->keys); 
	     v = ecore_list_next(a->vals);
	  }
     }
}

static int
_ecore_dbus_address_value_char_optional_encode(char c)
{
   /* addl optional chars (other than 0-9A-Za-z) */
   static const char OPTIONAL_CHARS[] = {'_', '-', '/', '.', '\\'};
   unsigned int i;

   if (isascii(c) && (isalpha(c) || isdigit(c))) return 1;
   for (i = 0; i < sizeof(OPTIONAL_CHARS); i++)
     if (c == OPTIONAL_CHARS[i]) return 1;
   
   return 0;


}

static char *
_ecore_dbus_address_value_encode(const char *value)
{
   char *buf;
   const char *p;
   int i;

   static const char hexdigits[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f'
   };

   
   if (!value) return NULL;
   buf = malloc(3 * strlen(value) + 1);

   p = value;
   i = 0;
   while (*p)
     {
	if (_ecore_dbus_address_value_char_optional_encode(*p))
	  buf[i++] = *p;
	else
	  {
	     buf[i++] = '%';
	     buf[i++] = hexdigits[(*p >> 4)];
	     buf[i++] = hexdigits[(*p & 0xf)];
	  }
	p++;
     }

   buf[i] = '\0';
   return buf;
}

static char *
_ecore_dbus_address_value_decode(const char *value)
{
   char *buf;
   const char *p;
   int i;

   buf = malloc(strlen(value) + 1);
 
   *buf = '\0';
   p = value;
   i = 0;
   while (*p)
     {
	if (*p == '%')
	  {
	     char c = 0;
	     int j;
	     for (j = 0; j < 2; j++) 
	       {
		  p++;
		  c = c << 4;
		  if ('0' <= *p && *p <= '9')
		    c |= *p - '0';
		  else if ('A' <= *p && *p <= 'F')
		    c |= 10 + *p - 'A';
		  else if ('a' <= *p && *p <= 'f') /* a-f */
		    c |= 10 + *p - 'a';
	       }
	     buf[i++] = c;
	  }
	else
	  buf[i++] = *p;

	p++;
     }

   buf[i] = '\0';
   return buf;
}
