
## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = INSTALL Makefile.in aclocal.m4 config.guess \
		       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in

SUBDIRS		     = src test doc debian

bin_SCRIPTS = evas-config

EXTRA_DIST = README AUTHORS COPYING evas.spec evas.m4

m4datadir = $(datadir)/aclocal
m4data_DATA = evas.m4
