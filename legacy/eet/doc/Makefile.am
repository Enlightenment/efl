
MAINTAINERCLEANFILES = Makefile.in eet.c

.PHONY: doc

if EFL_BUILD_DOC

doc-clean:
	rm -rf html/ latex/ man/ xml/ $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar*

doc: all doc-clean
	$(DOXYGEN)
	cp img/*.png html/
	rm -rf $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION).tar*
	mkdir -p $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION)/doc
	cp -R html/ latex/ man/ $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION)/doc
	tar cf $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION).tar $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION)
	bzip2 -9 $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION).tar
	rm -rf $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION)
	mv $(PACKAGE_TARNAME)-doc-$(PACKAGE_VERSION).tar.bz2 $(top_srcdir)

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

clean-local: doc-clean

EXTRA_DIST = Doxyfile eet.css foot.html head.html img/ eet.c.in

