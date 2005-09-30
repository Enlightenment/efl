
if BUILD_DOCS

# install documentation
docdir = $(datadir)/$(PACKAGE)/doc

all-local: doc-build.stamp

# rule to remove all old created files
doc-prepare.stamp:
	@if test -d html ; then \
	  rm -rf html/ latex/ man/ xml/; \
	fi

# rule to build documentation and copy necessary files
doc-build.stamp: doc-prepare.stamp
	@doxygen
	@cp img/*.png html/

# rules to clean
clean-local:
	@rm -rf html/ latex/ man/ xml/

# rule to install the documentation in $(docdir)
install-data-local:
	@if ! test -d "$(DESTDIR)$(datadir)/$(PACKAGE)"; then \
	  $(mkinstalldirs) "$(DESTDIR)$(datadir)/$(PACKAGE)"; \
	fi
	@if ! test -d "$(DESTDIR)$(docdir)"; then \
	  $(mkinstalldirs) "$(DESTDIR)$(docdir)"; \
	fi
	@cp -pr html/ man/ latex/ xml/ "$(DESTDIR)$(docdir)"

# rule to uninstall the documentation
uninstall-local:
	rm -rf $(DESTDIR)$(docdir)


MAINTAINERCLEANFILES = Makefile.in eet.c

DISTCLEANFILES = Makefile.in eet.c

endif

EXTRA_DIST = Doxyfile eet.css foot.html head.html img/

