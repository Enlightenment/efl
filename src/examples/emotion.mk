EXTRA_PROGRAMS += \
emotion/emotion_basic_example \
emotion/emotion_generic_example \
emotion/emotion_generic_subtitle_example \
emotion/emotion_border_example \
emotion/emotion_signals_example

EMOTION_SRCS = \
emotion/emotion_basic_example.c \
emotion/emotion_generic_example.c \
emotion/emotion_generic_subtitle_example.c \
emotion/emotion_border_example.c \
emotion/emotion_signals_example.c

EMOTION_DATA_FILES = emotion/Makefile.examples

DATA_FILES += $(EMOTION_DATA_FILES)

install-examples-emotion:
	$(MKDIR_P) $(DESTDIR)$(datadir)/emotion/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EMOTION_SRCS) $(EMOTION_DATA_FILES) $(DESTDIR)$(datadir)/emotion/examples

uninstall-local-emotion:
	for f in $(EMOTION_SRCS) $(EMOTION_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/emotion/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-emotion
UNINSTALL_EXAMPLES += uninstall-local-emotion
