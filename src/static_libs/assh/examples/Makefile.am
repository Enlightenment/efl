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

AM_CPPFLAGS = -I$(top_builddir)/src/assh

noinst_PROGRAMS = keygen

keygen_SOURCES = keygen.c
keygen_LDADD = $(top_builddir)/src/libassh.la
keygen_CFLAGS = -I$(top_srcdir)/src

if COND_client
client_SOURCES = client.c
client_LDADD = $(top_builddir)/src/libassh.la
client_CFLAGS = -I$(top_srcdir)/src
noinst_PROGRAMS += client
endif

if COND_server
server_SOURCES = server.c
server_LDADD = $(top_builddir)/src/libassh.la
server_CFLAGS = -I$(top_srcdir)/src
noinst_PROGRAMS += server
endif

