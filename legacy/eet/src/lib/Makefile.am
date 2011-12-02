
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@ \
@EET_CPPFLAGS@ \
@EFL_EET_BUILD@ \
@EFL_COVERAGE_CFLAGS@ \
@OPENSSL_CFLAGS@ \
@GNUTLS_CFLAGS@

includes_HEADERS = Eet.h
includesdir = $(includedir)/eet-@VMAJ@

lib_LTLIBRARIES = libeet.la

base_sources = \
eet_alloc.c \
eet_lib.c \
eet_data.c \
eet_image.c \
eet_cipher.c \
eet_dictionary.c \
eet_node.c \
eet_utils.c \
eet_connection.c

if EET_AMALGAMATION
nodist_libeet_la_SOURCES = eet_amalgamation.c

eet_amalgamation.c: $(base_sources) Makefile
	-rm -f eet_amalgamation.c

	@echo "#ifdef HAVE_CONFIG_H" >> eet_amalgamation.c
	@echo "# include \"config.h\"" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifdef HAVE_ALLOCA_H" >> eet_amalgamation.c
	@echo "# include <alloca.h>" >> eet_amalgamation.c
	@echo "#elif defined __GNUC__" >> eet_amalgamation.c
	@echo "# define alloca __builtin_alloca" >> eet_amalgamation.c
	@echo "#elif defined _AIX" >> eet_amalgamation.c
	@echo "# define alloca __alloca" >> eet_amalgamation.c
	@echo "#elif defined _MSC_VER" >> eet_amalgamation.c
	@echo "# include <malloc.h>" >> eet_amalgamation.c
	@echo "# define alloca _alloca" >> eet_amalgamation.c
	@echo "#else" >> eet_amalgamation.c
	@echo "# include <stddef.h>" >> eet_amalgamation.c
	@echo "# ifdef __cplusplus" >> eet_amalgamation.c
	@echo "#extern \"C\"" >> eet_amalgamation.c
	@echo "# endif" >> eet_amalgamation.c
	@echo "#void *alloca (size_t);" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#include <stdio.h>" >> eet_amalgamation.c
	@echo "#include <string.h>" >> eet_amalgamation.c
	@echo "#include <math.h>" >> eet_amalgamation.c
	@echo "#include <ctype.h>" >> eet_amalgamation.c
	@echo "#include <limits.h>" >> eet_amalgamation.c
	@echo "#include <sys/types.h>" >> eet_amalgamation.c
	@echo "#include <sys/stat.h>" >> eet_amalgamation.c
	@echo "#ifdef HAVE_SIGNATURE" >> eet_amalgamation.c
	@echo "# include <sys/mman.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c
	@echo "#include <setjmp.h>" >> eet_amalgamation.c
	@echo "#include <errno.h>" >> eet_amalgamation.c
	@echo "#include <time.h>" >> eet_amalgamation.c
	@echo "#include <fnmatch.h>" >> eet_amalgamation.c
	@echo "#include <fcntl.h>" >> eet_amalgamation.c

	@echo "#ifdef _WIN32" >> eet_amalgamation.c
	@echo "# include <winsock2.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifndef _MSC_VER" >> eet_amalgamation.c
	@echo "# include <unistd.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifdef HAVE_NETINET_IN_H" >> eet_amalgamation.c
	@echo "# include <netinet/in.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifdef HAVE_GNUTLS" >> eet_amalgamation.c
	@echo "# include <gnutls/gnutls.h>" >> eet_amalgamation.c
	@echo "# include <gcrypt.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifdef HAVE_OPENSSL" >> eet_amalgamation.c
	@echo "# include <openssl/err.h>" >> eet_amalgamation.c
	@echo "# include <openssl/evp.h>" >> eet_amalgamation.c
	@echo "# include <openssl/sha.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifdef HAVE_SIGNATURE" >> eet_amalgamation.c
	@echo "# ifdef HAVE_GNUTLS" >> eet_amalgamation.c
	@echo "#  include <gnutls/gnutls.h>" >> eet_amalgamation.c
	@echo "#  include <gnutls/x509.h>" >> eet_amalgamation.c
	@echo "# else" >> eet_amalgamation.c
	@echo "#  include <openssl/rsa.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/objects.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/err.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/ssl.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/dh.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/dsa.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/evp.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/x509.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/pem.h>" >> eet_amalgamation.c
	@echo "# endif" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#ifdef HAVE_CIPHER" >> eet_amalgamation.c
	@echo "# ifdef HAVE_GNUTLS" >> eet_amalgamation.c
	@echo "#  include <gnutls/x509.h>" >> eet_amalgamation.c
	@echo "#  include <gcrypt.h>" >> eet_amalgamation.c
	@echo "# else" >> eet_amalgamation.c
	@echo "#  include <openssl/evp.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/hmac.h>" >> eet_amalgamation.c
	@echo "#  include <openssl/rand.h>" >> eet_amalgamation.c
	@echo "# endif" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#include <zlib.h>" >> eet_amalgamation.c
	@echo "#include <jpeglib.h>" >> eet_amalgamation.c

	@echo "#ifdef HAVE_EVIL" >> eet_amalgamation.c
	@echo "# include <Evil.h>" >> eet_amalgamation.c
	@echo "#endif" >> eet_amalgamation.c

	@echo "#include <Eet.h>" >> eet_amalgamation.c

	@echo "#include \"Eet_private.h\"" >> eet_amalgamation.c
	@echo "#include \"Eet.h\"" >> eet_amalgamation.c

	@for f in $(base_sources); do \
	   if [ `echo $$f | sed -e 's/^...\(.\).*/\1/'` != '/' ]; then \
		  file="$(srcdir)/$$f" ; \
	   else \
		  file="$$f" ; \
	   fi ; \
	   echo "/* file: $$file */" >> eet_amalgamation.c; \
	   grep -v -e '^# *include \+.\(config\|\|Evil\|Eina\|stdio\|string\|math\|ctype\|limits\|sys/types\|sys/stat\|sys/mman\|setjmp\|errno\|time\|fnmatch\|fcntl\|winsock2\|unistd\|netinet/in\|gnutls/gnutls\|gcrypt\|gnutls/x509\|openssl/rsa\|openssl/objects\|openssl/err\|openssl/ssl\|openssl/dh\|openssl/dsa\|openssl/evp\|openssl/pem\|openssl/sha\|openssl/hmac\|openssl/x509\|openssl/rand\|zlib\|jpeglib\|Eet_private\|Eet\)[.]h.*' $$file >> eet_amalgamation.c; \
	done
	@echo "eet_amalgamation.c generated"

else
libeet_la_SOURCES = $(base_sources)
endif

libeet_la_CFLAGS = @EET_CFLAGS@ @DEBUG_CFLAGS@
libeet_la_LIBADD = @GNUTLS_LIBS@ @OPENSSL_LIBS@ @EFL_COVERAGE_LIBS@ @EET_LIBS@ @EINA_LIBS@ @EVIL_LIBS@ -lz -ljpeg -lm
libeet_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = Eet_private.h

clean-local:
	@rm -rf *.gcno eet_amalgamation.c
