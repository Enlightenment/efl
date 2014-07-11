
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eet.hh"

#include <algorithm>

#include <iostream>

#include <check.h>

struct pod_type
{
  int i;
  char c;
};

START_TEST(eet_cxx_descriptors)
{
  efl::eet::eet_init init;

  auto d = efl::eet::make_descriptor
    ("pod_type"
     , efl::eet::type("i", &pod_type::i)
     , efl::eet::type("c", &pod_type::c));
  static_assert(std::is_same<efl::eet::descriptor<pod_type, int, char>, decltype(d)>::value, "");

  Eet_File* file = eet_open("/tmp/eet_file_test.eet", EET_FILE_MODE_READ_WRITE);
  ck_assert(file != 0);

  pod_type pod = {1, 2};

  int s = eet_data_write(file, d.native_handle(), "pod", &pod, true);
  std::cout << "bytes written " << s << std::endl;
  ck_assert(s > 0);
  eet_sync(file);
  auto p = efl::eet::read_by_ptr(file, "pod", d);
  ck_assert(p != 0);
  ck_assert(p->i == 1);
  ck_assert(p->c == 2);

  eet_close(file);
}
END_TEST

int constructors_called = 0
    , destructors_called = 0;

struct non_pod
{
  non_pod() : i(10)
  {
    ++constructors_called;
  }
  non_pod(non_pod const& other)
    : i(other.i)
  {
    ++constructors_called;
  }
  ~non_pod()
  {
    ++destructors_called;
  }

  int i;
};

START_TEST(eet_cxx_descriptors_non_pod)
{
  efl::eet::eet_init init;

  auto d = efl::eet::make_descriptor
    ("pod_type", EET_CXX_MEMBER(non_pod, i));
  static_assert(std::is_same<efl::eet::descriptor<non_pod, int>, decltype(d)>::value, "");

  {
    Eet_File* file = eet_open("/tmp/eet_file_test.eet", EET_FILE_MODE_READ_WRITE);
    ck_assert(file != 0);

    ::non_pod non_pod;

    int s = eet_data_write(file, d.native_handle(), "non_pod", &non_pod, true);
    std::cout << "bytes written " << s << std::endl;
    ck_assert(s > 0);
    eet_sync(file);
    auto p = efl::eet::read_by_ptr(file, "non_pod", d);
    ck_assert(p != 0);
    ck_assert(p->i == 10);

    auto v = efl::eet::read(file, "non_pod", d);
    ck_assert(v.i == 10);

    eet_close(file);
  }

  std::cout << "constructors called for non pod: " << constructors_called
            << " destructors called for non pod: " << destructors_called << std::endl;

  ck_assert(constructors_called == destructors_called);
}
END_TEST

struct pod_composited
{
  pod_type* member;
};

struct pod_composited_with_non_pod
{
  non_pod* member;
};

struct pod_value_composited
{
  pod_type member;
};

START_TEST(eet_cxx_descriptors_composition)
{
  efl::eet::eet_init init;

  auto pod_descriptor = efl::eet::make_descriptor
    ("pod_type"
     , efl::eet::type("i", &pod_type::i)
     , efl::eet::type("c", &pod_type::c));
  static_assert(std::is_same<efl::eet::descriptor<pod_type, int, char>
                , decltype(pod_descriptor)>::value, "");

  auto non_pod_descriptor = efl::eet::make_descriptor
    ("non_pod"
     , efl::eet::type("i", &non_pod::i));
  static_assert(std::is_same<efl::eet::descriptor<non_pod, int>
                , decltype(non_pod_descriptor)>::value, "");

  {
    auto d = efl::eet::make_descriptor
      ("pod_composited", efl::eet::type("pod_composited", &pod_composited::member, pod_descriptor));
    static_assert(std::is_same<efl::eet::descriptor<pod_composited, pod_type*>, decltype(d)>::value, "");

    Eet_File* file = eet_open("/tmp/eet_file_test.eet", EET_FILE_MODE_READ_WRITE);
    ck_assert(file != 0);
    
    ::pod_composited pod_composited {new pod_type{5, 'a'}};

    int s = eet_data_write(file, d.native_handle(), "foo", &pod_composited, false);
    ck_assert(s > 0);
    eet_sync(file);
    auto p = efl::eet::read_by_ptr(file, "foo", d);
    ck_assert(p != 0);
    ck_assert(p->member->i == 5);
    ck_assert(p->member->c == 'a');

    delete p->member;

    auto v = efl::eet::read(file, "foo", d);
    ck_assert(v.member->i == 5);
    ck_assert(v.member->c == 'a');

    delete v.member;

    eet_close(file);
  }

  {
    auto d = efl::eet::make_descriptor
      ("pod_composited_with_non_pod", efl::eet::type("pod_composited_with_non_pod", &pod_composited_with_non_pod::member, non_pod_descriptor));
    static_assert(std::is_same<efl::eet::descriptor<pod_composited_with_non_pod, non_pod*>, decltype(d)>::value, "");

    Eet_File* file = eet_open("/tmp/eet_file_test.eet", EET_FILE_MODE_READ_WRITE);
    ck_assert(file != 0);
    
    ::pod_composited_with_non_pod pod_composited_with_non_pod {new non_pod};

    int s = eet_data_write(file, d.native_handle(), "foo", &pod_composited_with_non_pod, false);
    ck_assert(s > 0);
    eet_sync(file);
    auto p = efl::eet::read_by_ptr(file, "foo", d);
    ck_assert(p != 0);
    ck_assert(p->member->i == 10);

    delete p->member;

    auto v = efl::eet::read(file, "foo", d);
    ck_assert(v.member->i == 10);

    delete v.member;

    eet_close(file);

    delete pod_composited_with_non_pod.member;
  }

  std::cout << "constructors called for non pod: " << constructors_called
            << " destructors called for non pod: " << destructors_called << std::endl;

  ck_assert(constructors_called == destructors_called);

  {
    auto d = efl::eet::make_descriptor
      ("pod_value_composited", efl::eet::type("member"
                                              , &pod_value_composited::member, pod_descriptor));
    static_assert(std::is_same<efl::eet::descriptor<pod_value_composited, pod_type>, decltype(d)>::value, "");

    Eet_File* file = eet_open("/tmp/eet_file_test.eet", EET_FILE_MODE_READ_WRITE);
    ck_assert(file != 0);
    
    ::pod_value_composited pod_value_composited {{5, 'a'}};

    int s = eet_data_write(file, d.native_handle(), "foo", &pod_value_composited, false);
    ck_assert(s > 0);
    eet_sync(file);
    auto p = efl::eet::read_by_ptr(file, "foo", d);
    ck_assert(p != 0);
    ck_assert(p->member.i == 5);
    ck_assert(p->member.c == 'a');

    auto v = efl::eet::read(file, "foo", d);
    ck_assert(v.member.i == 5);
    ck_assert(v.member.c == 'a');

    eet_close(file);
  }

}
END_TEST

void
eet_test_descriptors(TCase* tc)
{
  tcase_add_test(tc, eet_cxx_descriptors);
  tcase_add_test(tc, eet_cxx_descriptors_non_pod);
  tcase_add_test(tc, eet_cxx_descriptors_composition);
}
