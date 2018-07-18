EINA_CXX_SRCS = \
eina_cxx/eina_cxx_list_01.cc \
eina_cxx/eina_cxx_thread_01.cc

if HAVE_CXX

EXTRA_PROGRAMS += \
eina_cxx/eina_cxx_list_01 \
eina_cxx/eina_cxx_thread_01

eina_cxx_eina_cxx_list_01_SOURCES = \
eina_cxx/eina_cxx_list_01.cc

eina_cxx_eina_cxx_thread_01_SOURCES = \
eina_cxx/eina_cxx_thread_01.cc

endif

install-examples-einacxx:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eina_cxx/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EINA_CXX_SRCS) $(DESTDIR)$(datadir)/eina_cxx/examples

uninstall-local-einacxx:
	for f in $(EINA_CXX_SRCS); do \
	  rm -f $(DESTDIR)$(datadir)/eina_cxx/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-einacxx
UNINSTALL_EXAMPLES += uninstall-local-einacxx
