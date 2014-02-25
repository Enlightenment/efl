
#include "Eina.hh"

#include <check.h>

Eina_Error my_error, my_error_2;

START_TEST(eina_cxx_get_error)
{
  efl::eina::eina_init eina_init;

  my_error = ::eina_error_msg_static_register("Message 1");

  ::eina_error_set(0);
  
  efl::eina::error_code ec1 = efl::eina::get_error_code();
  ck_assert(!ec1);

  ::eina_error_set(my_error);

  efl::eina::error_code ec2 = efl::eina::get_error_code();
  ck_assert(!!ec2);
  
  ck_assert(ec2.message() == "Message 1");

  ::eina_error_set(EINA_ERROR_OUT_OF_MEMORY);

  efl::eina::error_code ec3 = efl::eina::get_error_code();
  ck_assert(!!ec3);
  
  ck_assert(ec3.message() == "Out of memory");
}
END_TEST

START_TEST(eina_cxx_throw_on_error)
{
  efl::eina::eina_init eina_init;

  ::eina_error_set(my_error_2);
  my_error_2 = ::eina_error_msg_static_register("Message 2");

  ::eina_error_set(0);
  try
    {
      efl::eina::throw_on_error();
    }
  catch(std::exception const&)
    {
      std::abort();
    }

  ::eina_error_set(my_error_2);
  try
    {
      efl::eina::throw_on_error();
      std::abort();
    }
  catch(efl::eina::system_error const& e)
    {
      ck_assert(e.code().value() == my_error_2);
      ck_assert(e.code().message() == "Message 2");
      ck_assert(!efl::eina::get_error_code());
    }
}
END_TEST

void
eina_test_error(TCase *tc)
{
  tcase_add_test(tc, eina_cxx_get_error);
  tcase_add_test(tc, eina_cxx_throw_on_error);
}
