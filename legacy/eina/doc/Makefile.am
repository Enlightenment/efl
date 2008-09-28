
MAINTAINERCLEANFILES = Makefile.in

.PHONY: doc

if EINA_BUILD_DOC

doc: all
	rm -rf html/ latex/ man/
	$(DOXYGEN)

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

clean-local:
	@rm -rf html/ latex/ man/

EXTRA_DIST = Doxyfile
