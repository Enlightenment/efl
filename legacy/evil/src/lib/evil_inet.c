
/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Modifications: Vincent Torri, for the integration in Evil
 * - modification of the name of some functions
 * * modification of the management of the error
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <ctype.h>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

#include "Evil.h"
#include "evil_private.h"


# define SPRINTF(x) ((size_t)sprintf x)

#define ERRNO         ((int)GetLastError())
#define SET_ERRNO(x)  (SetLastError((DWORD)(x)))

#define ISDIGIT(x)  (isdigit((int)  ((unsigned char)x)))
#define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#define ISUPPER(x)  (isupper((int)  ((unsigned char)x)))

#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2
#define NS_INADDRSZ sizeof(IN_ADDR)


struct ares_in6_addr {
  union {
    unsigned char _S6_u8[16];
  } _S6_un;
};

const struct ares_in6_addr ares_in6addr_any = { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } };


/*
 * static int
 * inet_net_pton_ipv4(src, dst, size)
 *      convert IPv4 network number from presentation to network format.
 *      accepts hex octets, hex strings, decimal octets, and /CIDR.
 *      "size" is in bytes and describes "dst".
 * return:
 *      number of bits, either imputed classfully or specified with /CIDR,
 *      or -1 if some failure occurred (check errno).  ENOENT means it was
 *      not an IPv4 network specification.
 * note:
 *      network byte order assumed.  this means 192.5.5.240/28 has
 *      0b11110000 in its fourth octet.
 * note:
 *      On Windows we store the error in the thread errno, not
 *      in the winsock error code. This is to avoid loosing the
 *      actual last winsock error. So use macro ERRNO to fetch the
 *      errno this funtion sets when returning (-1), not SOCKERRNO.
 * author:
 *      Paul Vixie (ISC), June 1996
 */
static int
inet_net_pton_ipv4(const char *src, unsigned char *dst, size_t size)
{
  static const char xdigits[] = "0123456789abcdef";
  static const char digits[] = "0123456789";
  int n, ch, tmp = 0, dirty, bits;
  const unsigned char *odst = dst;

  ch = *src++;
  if (ch == '0' && (src[0] == 'x' || src[0] == 'X')
      && ISXDIGIT(src[1])) {
    /* Hexadecimal: Eat nybble string. */
    if (!size)
      goto emsgsize;
    dirty = 0;
    src++;  /* skip x or X. */
    while ((ch = *src++) != '\0' && ISXDIGIT(ch)) {
      if (ISUPPER(ch))
        ch = tolower(ch);
      n = (int)(strchr(xdigits, ch) - xdigits);
      if (dirty == 0)
        tmp = n;
      else
        tmp = (tmp << 4) | n;
      if (++dirty == 2) {
        if (!size--)
          goto emsgsize;
        *dst++ = (unsigned char) tmp;
        dirty = 0;
      }
    }
    if (dirty) {  /* Odd trailing nybble? */
      if (!size--)
        goto emsgsize;
      *dst++ = (unsigned char) (tmp << 4);
    }
  } else if (ISDIGIT(ch)) {
    /* Decimal: eat dotted digit string. */
    for (;;) {
      tmp = 0;
      do {
        n = (int)(strchr(digits, ch) - digits);
        tmp *= 10;
        tmp += n;
        if (tmp > 255)
          goto enoent;
      } while ((ch = *src++) != '\0' &&
               ISDIGIT(ch));
      if (!size--)
        goto emsgsize;
      *dst++ = (unsigned char) tmp;
      if (ch == '\0' || ch == '/')
        break;
      if (ch != '.')
        goto enoent;
      ch = *src++;
      if (!ISDIGIT(ch))
        goto enoent;
    }
  } else
    goto enoent;

  bits = -1;
  if (ch == '/' &&
      ISDIGIT(src[0]) && dst > odst) {
    /* CIDR width specifier.  Nothing can follow it. */
    ch = *src++;    /* Skip over the /. */
    bits = 0;
    do {
      n = (int)(strchr(digits, ch) - digits);
      bits *= 10;
      bits += n;
    } while ((ch = *src++) != '\0' && ISDIGIT(ch));
    if (ch != '\0')
      goto enoent;
    if (bits > 32)
      goto emsgsize;
  }

  /* Firey death and destruction unless we prefetched EOS. */
  if (ch != '\0')
    goto enoent;

  /* If nothing was written to the destination, we found no address. */
  if (dst == odst)
    goto enoent;
  /* If no CIDR spec was given, infer width from net class. */
  if (bits == -1) {
    if (*odst >= 240)       /* Class E */
      bits = 32;
    else if (*odst >= 224)  /* Class D */
      bits = 8;
    else if (*odst >= 192)  /* Class C */
      bits = 24;
    else if (*odst >= 128)  /* Class B */
      bits = 16;
    else                    /* Class A */
      bits = 8;
    /* If imputed mask is narrower than specified octets, widen. */
    if (bits < ((dst - odst) * 8))
      bits = (int)(dst - odst) * 8;
    /*
     * If there are no additional bits specified for a class D
     * address adjust bits to 4.
     */
    if (bits == 8 && *odst == 224)
      bits = 4;
  }
  /* Extend network to cover the actual mask. */
  while (bits > ((dst - odst) * 8)) {
    if (!size--)
      goto emsgsize;
    *dst++ = '\0';
  }
  return (bits);

  enoent:
  SET_ERRNO(ENOENT);
  return (-1);

  emsgsize:
  SET_ERRNO(EMSGSIZE);
  return (-1);
}

static int
getbits(const char *src, int *bitsp)
{
  static const char digits[] = "0123456789";
  int n;
  int val;
  char ch;

  val = 0;
  n = 0;
  while ((ch = *src++) != '\0') {
    const char *pch;

    pch = strchr(digits, ch);
    if (pch != NULL) {
      if (n++ != 0 && val == 0)       /* no leading zeros */
        return (0);
      val *= 10;
      val += (pch - digits);
      if (val > 128)                  /* range */
        return (0);
      continue;
    }
    return (0);
  }
  if (n == 0)
    return (0);
  *bitsp = val;
  return (1);
}

static int
getv4(const char *src, unsigned char *dst, int *bitsp)
{
  static const char digits[] = "0123456789";
  unsigned char *odst = dst;
  int n;
  unsigned int val;
  char ch;

  val = 0;
  n = 0;
  while ((ch = *src++) != '\0') {
    const char *pch;

    pch = strchr(digits, ch);
    if (pch != NULL) {
      if (n++ != 0 && val == 0)       /* no leading zeros */
        return (0);
      val *= 10;
      val += (pch - digits);
      if (val > 255)                  /* range */
        return (0);
      continue;
    }
    if (ch == '.' || ch == '/') {
      if (dst - odst > 3)             /* too many octets? */
        return (0);
      *dst++ = (unsigned char)val;
      if (ch == '/')
        return (getbits(src, bitsp));
      val = 0;
      n = 0;
      continue;
    }
    return (0);
  }
  if (n == 0)
    return (0);
  if (dst - odst > 3)             /* too many octets? */
    return (0);
  *dst++ = (unsigned char)val;
  return (1);
}

static int
inet_net_pton_ipv6(const char *src, unsigned char *dst, size_t size)
{
  static const char xdigits_l[] = "0123456789abcdef",
    xdigits_u[] = "0123456789ABCDEF";
  unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
  const char *xdigits, *curtok;
  int ch, saw_xdigit;
  unsigned int val;
  int digits;
  int bits;
  size_t bytes;
  int words;
  int ipv4;

  memset((tp = tmp), '\0', NS_IN6ADDRSZ);
  endp = tp + NS_IN6ADDRSZ;
  colonp = NULL;
  /* Leading :: requires some special handling. */
  if (*src == ':')
    if (*++src != ':')
      goto enoent;
  curtok = src;
  saw_xdigit = 0;
  val = 0;
  digits = 0;
  bits = -1;
  ipv4 = 0;
  while ((ch = *src++) != '\0') {
    const char *pch;

    if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
      pch = strchr((xdigits = xdigits_u), ch);
    if (pch != NULL) {
      val <<= 4;
      val |= (pch - xdigits);
      if (++digits > 4)
        goto enoent;
      saw_xdigit = 1;
      continue;
    }
    if (ch == ':') {
      curtok = src;
      if (!saw_xdigit) {
        if (colonp)
          goto enoent;
        colonp = tp;
        continue;
      } else if (*src == '\0')
        goto enoent;
      if (tp + NS_INT16SZ > endp)
        return (0);
      *tp++ = (unsigned char)((val >> 8) & 0xff);
      *tp++ = (unsigned char)(val & 0xff);
      saw_xdigit = 0;
      digits = 0;
      val = 0;
      continue;
    }
    if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
        getv4(curtok, tp, &bits) > 0) {
      tp += NS_INADDRSZ;
      saw_xdigit = 0;
      ipv4 = 1;
      break;  /* '\0' was seen by inet_pton4(). */
    }
    if (ch == '/' && getbits(src, &bits) > 0)
      break;
    goto enoent;
  }
  if (saw_xdigit) {
    if (tp + NS_INT16SZ > endp)
      goto enoent;
    *tp++ = (unsigned char)((val >> 8) & 0xff);
    *tp++ = (unsigned char)(val & 0xff);
  }
  if (bits == -1)
    bits = 128;

  words = (bits + 15) / 16;
  if (words < 2)
    words = 2;
  if (ipv4)
    words = 8;
  endp =  tmp + 2 * words;

  if (colonp != NULL) {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const ssize_t n = tp - colonp;
    ssize_t i;

    if (tp == endp)
      goto enoent;
    for (i = 1; i <= n; i++) {
      *(endp - i) = *(colonp + n - i);
      *(colonp + n - i) = 0;
    }
    tp = endp;
  }
  if (tp != endp)
    goto enoent;

  bytes = (bits + 7) / 8;
  if (bytes > size)
    goto emsgsize;
  memcpy(dst, tmp, bytes);
  return (bits);

  enoent:
  SET_ERRNO(ENOENT);
  return (-1);

  emsgsize:
  SET_ERRNO(EMSGSIZE);
  return (-1);
}

/*
 * int
 * inet_net_pton(af, src, dst, size)
 *      convert network number from presentation to network format.
 *      accepts hex octets, hex strings, decimal octets, and /CIDR.
 *      "size" is in bytes and describes "dst".
 * return:
 *      number of bits, either imputed classfully or specified with /CIDR,
 *      or -1 if some failure occurred (check errno).  ENOENT means it was
 *      not a valid network specification.
 * note:
 *      On Windows we store the error in the thread errno, not
 *      in the winsock error code. This is to avoid loosing the
 *      actual last winsock error. So use macro ERRNO to fetch the
 *      errno this funtion sets when returning (-1), not SOCKERRNO.
 * author:
 *      Paul Vixie (ISC), June 1996
 */
static int
ares_inet_net_pton(int af, const char *src, void *dst, size_t size)
{
  switch (af) {
  case AF_INET:
    return (inet_net_pton_ipv4(src, dst, size));
  case AF_INET6:
    return (inet_net_pton_ipv6(src, dst, size));
  default:
    SET_ERRNO(EAFNOSUPPORT);
    return (-1);
  }
}

int
evil_inet_pton(int af, const char *src, void *dst)
{
  int result;
  size_t size;

  if (af == AF_INET)
    size = sizeof(struct in_addr);
  else if (af == AF_INET6)
    size = sizeof(struct ares_in6_addr);
  else
  {
    SET_ERRNO(EAFNOSUPPORT);
    return -1;
  }
  result = ares_inet_net_pton(af, src, dst, size);
  if ((result == -1) && (ERRNO == ENOENT))
    return 0;
  return (result > -1 ? 1 : -1);
}


/* const char *
 * inet_ntop4(src, dst, size)
 *     format an IPv4 address, more or less like inet_ntoa()
 * return:
 *     `dst' (as a const)
 * notes:
 *     (1) uses no statics
 *     (2) takes a unsigned char* not an in_addr as input
 * author:
 *     Paul Vixie, 1996.
 */
static const char *
inet_ntop4(const unsigned char *src, char *dst, size_t size)
{
  static const char fmt[] = "%u.%u.%u.%u";
  char tmp[sizeof "255.255.255.255"];

  if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) > size)
    {
      SET_ERRNO(ENOSPC);
      return (NULL);
    }
    strcpy(dst, tmp);
    return (dst);
}

/* const char *
 * inet_ntop6(src, dst, size)
 *    convert IPv6 binary address into presentation (printable) format
 * author:
 *    Paul Vixie, 1996.
 */
static const char *
inet_ntop6(const unsigned char *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
  char *tp;
  struct {
    long base;
    long len;
  } best, cur;
  unsigned long words[NS_IN6ADDRSZ / NS_INT16SZ];
  int i;

  /*
   * Preprocess:
   *  Copy the input (bytewise) array into a wordwise array.
   *  Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset(words, '\0', sizeof(words));
  for (i = 0; i < NS_IN6ADDRSZ; i++)
      words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));

  best.base = -1;
  cur.base = -1;
  best.len = 0;
  cur.len = 0;

  for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
      if (words[i] == 0)
        {
          if (cur.base == -1)
            cur.base = i, cur.len = 1;
          else
            cur.len++;
        }
      else
        {
          if (cur.base != -1)
            {
              if (best.base == -1 || cur.len > best.len)
                best = cur;
              cur.base = -1;
            }
        }
    }
  if (cur.base != -1)
    {
      if (best.base == -1 || cur.len > best.len)
        best = cur;
    }
  if (best.base != -1 && best.len < 2)
    best.base = -1;

  /*
   * Format the result.
   */
  tp = tmp;
  for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
      /* Are we inside the best run of 0x00's? */
      if (best.base != -1 && i >= best.base &&
          i < (best.base + best.len))
        {
          if (i == best.base)
             *tp++ = ':';
          continue;
        }
      /* Are we following an initial run of 0x00s or any real hex? */
      if (i != 0)
        *tp++ = ':';
      /* Is this address an encapsulated IPv4? */
      if (i == 6 && best.base == 0 &&
          (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
        {
          if (!inet_ntop4(src+12, tp, sizeof(tmp) - (tp - tmp)))
            return (NULL);
          tp += strlen(tp);
          break;
        }
        tp += SPRINTF((tp, "%lx", words[i]));
    }

  /* Was it a trailing run of 0x00's? */
  if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ))
    *tp++ = ':';
  *tp++ = '\0';

  /*
   * Check for overflow, copy, and we're done.
   */
  if ((size_t)(tp - tmp) > size)
    {
      SET_ERRNO(ENOSPC);
      return (NULL);
    }
  strcpy(dst, tmp);
  return (dst);
}

/* char *
 * inet_ntop(af, src, dst, size)
 *     convert a network format address to presentation format.
 * return:
 *     pointer to presentation format address (`dst'), or NULL (see errno).
 * note:
 *      On Windows we store the error in the thread errno, not
 *      in the winsock error code. This is to avoid loosing the
 *      actual last winsock error. So use macro ERRNO to fetch the
 *      errno this funtion sets when returning NULL, not SOCKERRNO.
 * author:
 *     Paul Vixie, 1996.
 */
static const char *
ares_inet_ntop(int af, const void *src, char *dst, size_t size)
{
  switch (af)
    {
    case AF_INET:
      return (inet_ntop4(src, dst, size));
    case AF_INET6:
      return (inet_ntop6(src, dst, size));
    default:
      SET_ERRNO(EAFNOSUPPORT);
      return (NULL);
    }
  /* NOTREACHED */
}

const char *evil_inet_ntop(int af, const char *src, void *dst)
{
  const char *result;
  size_t size;

  if (af == AF_INET)
    size = sizeof(struct in_addr);
  else if (af == AF_INET6)
    size = sizeof(struct ares_in6_addr);
  else
  {
    SET_ERRNO(EAFNOSUPPORT);
    return NULL;
  }
  result = ares_inet_ntop(af, src, dst, size);
  if ((result == NULL) && (ERRNO == ENOSPC))
    return NULL;
  return result;
}
