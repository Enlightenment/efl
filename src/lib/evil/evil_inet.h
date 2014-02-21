
/* Copyright (C) 2005 by Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef __EVIL_INET_H__
#define __EVIL_INET_H__


#if ! (_WIN32_WINNT >= 0x600 /* _WIN32_WINNT_VISTA */)


/**
 * @file evil_inet.h
 * @brief The file that provides functions ported from Unix in arpa/inet.h.
 * @defgroup Evil_Inet_Group Inet.h functions
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in arpa/inet.h.
 *
 * @{
 */

/**
 * @brief Convert IPv4 and IPv6 addresses from text to binary form.
 *
 * @param af The address family.
 * @param src The address to convert.
 * @param dst The converted address structure.
 * @return 1 on success, 0 or -1 otherwise.
 *
 * This function converts IPv4 and IPv6 addresses from @p src to the
 * binary form @p dst. The following address families to pass to @p af
 * are currently supported:
 *
 * <ul>
 * <li>i AF_INET: @p src points to a character string containing an IPv4
 * network address in dotted-decimal format, "ddd.ddd.ddd.ddd", where
 * ddd is a decimal number of up to three digits in the range 0 to
 * 255. The address is converted to a struct in_addr and copied to
 * dst, which must be sizeof(struct in_addr) (4) bytes (32 bits) long.
 * <li> AF_INET6: @p     src points to a character string containing an
 * IPv6 network address. The address is converted to a struct in6_addr
 * and copied to dst, which must be sizeof(struct in6_addr) (16) bytes
 * (128 bits) long. The allowed formats for IPv6 addresses follow
 * these rules:
 * <ol>
 * <li>The preferred format is x:x:x:x:x:x:x:x. This form consists of
 * eight hexadecimal numbers, each of which expresses a 16-bit value
 * (i.e., each x can be up to 4 hex digits).
 * <li>A series of contiguous zero values in the preferred format can
 * be abbreviated to ::. Only one instance of :: can occur in an
 * address. For example, the loopback address 0:0:0:0:0:0:0:1 can be
 * abbreviated as ::1. The wildcard address, consisting of all zeros,
 * can be written as ::.
 * <li>An alternate format is useful for expressing IPv4-mapped IPv6
 * addresses. This form is written as x:x:x:x:x:x:d.d.d.d, where the
 * six leading xs are hexadecimal values that define the six
 * most-significant 16-bit pieces of the address (i.e., 96 bits), and
 * the ds express a value in dotted-decimal notation that defines the
 * least significant 32 bits of the address. An example of such an
 * address is :: FFFF:204.152.189.116.
 * </ul>
 * </ul>
 * On success this function returns 1 (network address was successfully
 * converted). 0 is returned if @p src does not contain a character
 * string representing a valid network address in the specified
 * address family. If af does not contain a valid address family, -1
 * is returned and errno is set to EAFNOSUPPORT.
 *
 * @see evil_inet_ntop()
 * @see inet_ntop()
 *
 * Conformity: POSIX.1-2001.
 *
 * Supported OS: Windows XP, CE.
 *
 */
EAPI int evil_inet_pton(int af, const char *src, void *dst);

/**
 * @def inet_pton(x,y,z)
 *
 * Wrapper around evil_inet_pton().
 */
#define inet_pton(x,y,z) evil_inet_pton(x,y,z)

/**
 * @brief Convert IPv4 and IPv6 addresses from binary to text form.
 *
 * @param af The address family.
 * @param src The address structure to convert.
 * @param dst A buffer containing the converted string.
 * @param size The size of the buffer.
 * @return 1 on success, 0 otherwise.
 *
 * This function converts the network address structure @p src in the
 * @p af address family into a character string. The resulting string
 * is copied to the buffer pointed to by @p dst, which must be a
 * non-NULL pointer. The caller specifies the number of bytes
 * available in this buffer in the argument @p size. The following
 * address families to pass to @p af are currently supported:
 *
 * @li AF_INET: @p src points to a struct in_addr (in network byte
 * order) which is converted to an IPv4 network address in the
 * dotted-decimal format, "ddd.ddd.ddd.ddd". The buffer @p dst must be
 * at least INET_ADDRSTRLEN bytes long.
 * @li AF_INET6: @p src points to a struct in6_addr (in network byte
 * order) which is converted to a representation of this address in
 * the most appropriate IPv6 network address format for this
 * address. The buffer @p dst must be at least INET6_ADDRSTRLEN bytes
 * long.
 *
 * On success, this function returns a non-NULL pointer to @p dst. NULL is
 * returned if there was an error, with errno set to indicate the
 * error.
 *
 * @see evil_inet_pton()
 * @see inet_pton()
 *
 * Conformity: POSIX.1-2001.
 *
 * Supported OS: Windows XP, CE.
 *
 */
EAPI const char *evil_inet_ntop(int af, const char *src, void *dst, size_t size);

/**
 * @def inet_ntop(x,y,z,s)
 *
 * Wrapper around evil_inet_ntop().
 */
#define inet_ntop(x,y,z,s) evil_inet_ntop(x,y,z,s)


#endif /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */


/**
 * @}
 */


#endif /* __EVIL_INET_H__ */
