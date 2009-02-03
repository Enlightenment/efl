
MAINTAINERCLEANFILES = Makefile.in edje.dox

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

if EFL_BUILD_DOC

doc-clean:
	rm -rf html/ latex/ man/ xml/ $(PACKAGE_DOCNAME).tar*

doc: all doc-clean
	cat ../src/lib/Edje.h >> ./edje.dox
	cat `find ../src/lib -name "*.c" -fprint /dev/stdout | grep -v edje_edit.c` >> edje.dox
	$(efl_doxygen)
	cp img/* html/
	rm -rf $(PACKAGE_DOCNAME).tar*
	mkdir -p $(PACKAGE_DOCNAME)/doc
	cp -R html/ latex/ man/ $(PACKAGE_DOCNAME)/doc
	tar cf $(PACKAGE_DOCNAME).tar $(PACKAGE_DOCNAME)/
	bzip2 -9 $(PACKAGE_DOCNAME).tar
	rm -rf $(PACKAGE_DOCNAME)/
	mv $(PACKAGE_DOCNAME).tar.bz2 $(top_srcdir)

clean-local: doc-clean

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

EXTRA_DIST = Doxyfile e.css foot.html head.html img edje.dox.in
