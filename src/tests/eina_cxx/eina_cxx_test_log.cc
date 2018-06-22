#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>

#include "eina_cxx_suite.h"

bool expensive_called = false;

int expensive_call()
{
  expensive_called = true;
  return 11;
}

EFL_START_TEST(eina_cxx_level_log)
{
  efl::eina::eina_init init;

  efl::eina::log_domain domain("level_error_domain");

  domain.set_level(efl::eina::log_level::critical);
  ck_assert(domain.get_level() == ::EINA_LOG_LEVEL_CRITICAL);
  domain.set_level(efl::eina::log_level::warning);
  ck_assert(domain.get_level() == ::EINA_LOG_LEVEL_WARN);
  domain.set_level(efl::eina::log_level::debug);
  ck_assert(domain.get_level() == ::EINA_LOG_LEVEL_DBG);
  domain.set_level(efl::eina::log_level::info);
  ck_assert(domain.get_level() == ::EINA_LOG_LEVEL_INFO);
  domain.set_level(efl::eina::log_level::error);
  ck_assert(domain.get_level() == ::EINA_LOG_LEVEL_ERR);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_expensive_log)
{
  efl::eina::eina_init init;

  efl::eina::log_domain domain("expensive_call_error_domain");

  domain.set_level(EINA_LOG_LEVEL_CRITICAL);

  EINA_CXX_DOM_LOG_ERR(domain) << "foo " << ::expensive_call();
  ck_assert(!expensive_called);
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_domain_log)
{
  efl::eina::eina_init init;

  efl::eina::log_domain domain("error_domain");

  EINA_CXX_DOM_LOG_CRIT(domain) << "foo 0x" << std::hex << 10;
  EINA_CXX_DOM_LOG_ERR(domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_INFO(domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_DBG(domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_WARN(domain) << "foo " << 5;

  EINA_CXX_LOG_CRIT() << "foo " << 5;
  EINA_CXX_LOG_ERR() << "foo " << 5;
  EINA_CXX_LOG_INFO() << "foo " << 5;
  EINA_CXX_LOG_DBG() << "foo " << 5;
  EINA_CXX_LOG_WARN() << "foo " << 5;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_default_domain_log)
{
  efl::eina::eina_init init;

  EINA_CXX_DOM_LOG_CRIT(efl::eina::default_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_ERR(efl::eina::default_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_INFO(efl::eina::default_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_DBG(efl::eina::default_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_WARN(efl::eina::default_domain) << "foo " << 5;
}
EFL_END_TEST

EFL_START_TEST(eina_cxx_global_domain_log)
{
  efl::eina::eina_init init;

  efl::eina::log_domain domain("domain");

  EINA_CXX_DOM_LOG_CRIT(efl::eina::global_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_ERR(efl::eina::global_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_INFO(efl::eina::global_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_DBG(efl::eina::global_domain) << "foo " << 5;
  EINA_CXX_DOM_LOG_WARN(efl::eina::global_domain) << "foo " << 5;
}
EFL_END_TEST

void
eina_test_log(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_domain_log);
  tcase_add_test(tc, eina_cxx_default_domain_log);
  tcase_add_test(tc, eina_cxx_global_domain_log);
  tcase_add_test(tc, eina_cxx_expensive_log);
  tcase_add_test(tc, eina_cxx_level_log);
}
