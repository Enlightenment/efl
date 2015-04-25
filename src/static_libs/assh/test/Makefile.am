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

AUTOMAKE_OPTIONS = serial-tests

AM_CPPFLAGS = -I$(top_builddir)/src/assh

noinst_PROGRAMS =
TESTS =

noinst_PROGRAMS += hash
TESTS += hash
hash_SOURCES = hash.c
hash_LDADD = $(top_builddir)/src/libassh.la
hash_CFLAGS = -I$(top_srcdir)/src

if COND_server
if COND_client

noinst_PROGRAMS += connection
TESTS += connection
connection_SOURCES = connection.c
connection_LDADD = $(top_builddir)/src/libassh.la
connection_CFLAGS = -I$(top_srcdir)/src

noinst_PROGRAMS += kex
TESTS += kex
kex_SOURCES = kex.c
kex_LDADD = $(top_builddir)/src/libassh.la
kex_CFLAGS = -I$(top_srcdir)/src
endif
endif

noinst_PROGRAMS += bignum
TESTS += bignum
bignum_SOURCES = bignum.c
bignum_LDADD = $(top_builddir)/src/libassh.la
bignum_CFLAGS = -I$(top_srcdir)/src

noinst_PROGRAMS += dsa
TESTS += dsa
dsa_SOURCES = dsa.c
dsa_LDADD = $(top_builddir)/src/libassh.la
dsa_CFLAGS = -I$(top_srcdir)/src

noinst_PROGRAMS += signature
TESTS += signature
signature_SOURCES = signature.c
signature_LDADD = $(top_builddir)/src/libassh.la
signature_CFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = \
fifo.h \
keys.h \
leaks_check.h \
prng_weak.h \
prng_weak.c \
connection.c \
kex.c
