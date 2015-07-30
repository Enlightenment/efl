check_PROGRAMS = \
	src/tests/ejson_suite
TESTS = src/tests/ejson_suite

src_tests_ejson_suite_SOURCES = \
	src/tests/ejson_suite.c \
	src/tests/ejson_test_ejson_model.c
src_tests_ejson_suite_CFLAGS = $(MOD_CFLAGS) \
	-DTESTS_BUILD_DIR=\"$(top_builddir)/src/tests\"
src_tests_ejson_suite_LDADD = $(MOD_LIBS) $(CHECK_LIBS)

