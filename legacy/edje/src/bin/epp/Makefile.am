# Makefile for GNU C compiler.
#   Copyright (C) 1987, 88, 90-94, 1995 Free Software Foundation, Inc.

#This file is part of GNU CC.

#GNU CC is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2, or (at your option)
#any later version.

#GNU CC is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with GNU CC; see the file COPYING.  If not, write to
#the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

MAINTAINERCLEANFILES = Makefile.in

eppdir = @libdir@/@PACKAGE@/utils

epp_PROGRAMS = epp

epp_SOURCES = \
cpplib.h \
cpphash.h \
cppalloc.c \
cpperror.c \
cppexp.c \
cpphash.c \
cpplib.c \
cppmain.c

epp_CPPFLAGS = \
-I$(top_builddir) \
$(CWARNFLAGS)

DEFS= \
-DHAVE_CONFIG_H \
-DHAVE_STRERROR \
-DFATAL_EXIT_CODE=1 \
-DSUCCESS_EXIT_CODE=0 \
-DGCC_INCLUDE_DIR=\"/usr/include\" \
-DGPLUSPLUS_INCLUDE_DIR=\"/usr/include\" \
-DTOOL_INCLUDE_DIR=\"/usr/bin\" \
-DHOST_BITS_PER_LONG=32 \
-DBITS_PER_UNIT=8 \
-DHOST_BITS_PER_INT=32 \
-DBITS_PER_WORD=16 \
-DTARGET_BELL=7 \
-DTARGET_BS=8 \
-DTARGET_FF=12 \
-DTARGET_NEWLINE=10 \
-DTARGET_CR=13 \
-DTARGET_TAB=9 \
-DTARGET_VT=11

