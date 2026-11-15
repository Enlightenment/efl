#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_BETA_API_SUPPORT
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

static const char payload[] = "the quick brown fox jumps over the lazy dog";

static void
_copier_done(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Bool *done = data;

   *done = EINA_TRUE;
   efl_loop_quit(efl_main_loop_get(), eina_value_int_init(0));
}

/* A source set after construction still starts the copy of what it holds. */
EFL_START_TEST(efl_io_test_copier_source_set_after_construction)
{
   Eo *source, *copier;
   Eina_Slice to_write;
   Eina_Bool done = EINA_FALSE;
   Eina_Binbuf *collected;

   source = efl_add(EFL_IO_BUFFER_CLASS, efl_main_loop_get());
   to_write.mem = payload;
   to_write.len = sizeof(payload) - 1;
   efl_io_writer_write(source, &to_write, NULL);

   copier = efl_add(EFL_IO_COPIER_CLASS, efl_main_loop_get(),
                    efl_event_callback_add(efl_added, EFL_IO_COPIER_EVENT_DONE,
                                           _copier_done, &done));
   fail_if(!copier);

   efl_io_copier_source_set(copier, source);

   efl_loop_begin(efl_main_loop_get());
   ck_assert(done);

   collected = efl_io_copier_binbuf_steal(copier);
   fail_if(!collected);
   ck_assert_int_eq(eina_binbuf_length_get(collected), sizeof(payload) - 1);
   eina_binbuf_free(collected);

   efl_del(copier);
   efl_del(source);
}
EFL_END_TEST

void
efl_app_test_efl_io(TCase *tc)
{
   tcase_add_test(tc, efl_io_test_copier_source_set_after_construction);
}
