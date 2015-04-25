# Copyright (C) 2013 Alexandre Becoulet
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.

BUILT_SOURCES = assh_config.h
DISTCLEANFILES = assh_config.h
nodist_pkginclude_HEADERS = assh_config.h

pkgincludedir = $(includedir)/assh
pkginclude_HEADERS = assh.h assh_algo.h assh_alloc.h assh_bignum.h \
	assh_cipher.h assh_compress.h assh_config.h assh_connection.h \
	assh_context.h assh_event.h assh_hash.h assh_kex.h assh_key.h \
	assh_mac.h assh_map.h assh_packet.h assh_platform.h assh_prng.h \
	assh_queue.h assh_service.h assh_session.h assh_sign.h \
	assh_transport.h assh_userauth_client.h assh_userauth_server.h \
	helper_fd.h helper_key.h key_dsa.h key_eddsa.h key_rsa.h

assh_config.h: $(top_builddir)/config.h
	grep CONFIG_ASSH $(top_builddir)/config.h > $@ || true

