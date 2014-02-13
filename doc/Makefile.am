SUBDIRS = previews

MAINTAINERCLEANFILES = Makefile.in 

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

if EFL_BUILD_DOC

doc-clean:
	rm -rf html/ latex/ man/ xml/ $(top_builddir)/$(PACKAGE_DOCNAME).tar*

previews-build:
	@$(MAKE) -C previews

previews-data:
	@$(MAKE) -C previews doc

doc: all previews-build previews-data
	$(efl_doxygen)
	cp $(srcdir)/img/* html/
	cp $(srcdir)/img/*.eps latex/
	rm -rf $(PACKAGE_DOCNAME).tar*
	mkdir -p $(PACKAGE_DOCNAME)/doc
	cp -R html/ latex/ man/ $(PACKAGE_DOCNAME)/doc
	tar cf $(PACKAGE_DOCNAME).tar $(PACKAGE_DOCNAME)/
	bzip2 -9 $(PACKAGE_DOCNAME).tar
	rm -rf $(PACKAGE_DOCNAME)/
	mv $(PACKAGE_DOCNAME).tar.bz2 $(top_builddir)
	@echo "Documentation Package: doc/$(PACKAGE_DOCNAME).tar.bz2"
	@echo "Documentation HTML: doc/html"

clean-local: doc-clean

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

EXTRA_DIST = \
Doxyfile.in \
e.css \
head.html \
foot.html \
ecore_examples.dox \
eldbus_examples.dox \
edje_examples.dox \
eet_examples.dox \
eina_examples.dox \
eio_examples.dox \
emotion_examples.dox \
eo_tutorial.dox \
ephysics_examples.dox \
evas_examples.dox \
main.dox \
pkgconfig.dox

include images.mk

all-local:
	@rm -f images.tmp; \
	echo 'EXTRA_DIST += \' > images.tmp; \
	(cd $(srcdir) && find ./img -type f -print | cut -d/ -f2- | grep -v '~' | LANG=C LANGUAGE=C LC_ALL=C LC_CTYPE=C LC_NUMERIC=C LC_TIME=C LC_COLLATE=C LC_MONETARY=C LC_MESSAGES=C LC_PAPER=C LC_NAME=C LC_ADDRESS=C LC_TELEPHONE=C LC_MEASUREMENT=C LC_IDENTIFICATION=C sort | tr '\n' ' ') >> images.tmp && \
	echo >> images.tmp && \
	if ! cmp "$(srcdir)/images.mk" images.tmp >/dev/null; then \
	   echo "warning: new images added to '$(srcdir)/images.mk'"; \
	   mv -f images.tmp "$(srcdir)/images.mk"; \
	else \
	   rm images.tmp; \
	fi
