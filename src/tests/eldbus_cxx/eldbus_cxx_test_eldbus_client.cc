
#include "Eldbus.hh"
#include "Ecore.hh"

#include <eldbus_freedesktop.hh>

#include <algorithm>

#include <iostream>

#include <check.h>

const char g_bus[] = "org.Enlightenment";
const char g_path[] = "/org/enlightenment";
const char g_interface[] = "org.enlightenment.Test";

static void
on_name_request(std::error_code const& ec, efl::eldbus::const_message
                , efl::eldbus::pending, unsigned int reply)
{
  if(!ec)
    {
       if(reply != ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
         {
           printf("error name already in use\n");
           return;
         }
     }
}

START_TEST(eldbus_cxx_client)
{
  namespace edb = efl::eldbus;

  efl::ecore::ecore_init ecore_init;
  edb::eldbus_init init;

  edb::connection c_(edb::session);

  namespace es = edb::service;

  bool expected_bool = true;
  char expected_byte = 0xAA;
  uint32_t expected_uint32 = 0xFFFFFFFF;
  int32_t expected_int32 = -1;
  int16_t expected_int16 = -1;
  double expected_double = 3.1415926;
  std::string expected_string = "expected string";

  edb::service_interface iface = edb::service_interface_register
    (c_, g_path, g_interface
     , es::method("SendBool"
                  , [expected_bool] (edb::const_message, edb::service_interface, bool b)
                  {
                    ck_assert(b == expected_bool);
                    return b;
                  }
                  , es::ins<bool>("bool")
                  , es::outs<bool>("bool")
                  )
     , es::method("SendByte"
                  , [expected_byte] (edb::const_message, edb::service_interface, char c)
                  {
                    ck_assert(c == expected_byte);
                    return c;
                  }
                  , es::ins<char>("byte")
                  , es::outs<char>("byte")
                  )
     , es::method("SendUint32"
                  , [expected_uint32] (edb::const_message, edb::service_interface, uint32_t n)
                  {
                    ck_assert(n == expected_uint32);
                    return n;
                  }
                  , es::ins<uint32_t>("uint32")
                  , es::outs<uint32_t>("uint32")
                  )
     , es::method("SendInt32"
                  , [expected_int32] (edb::const_message, edb::service_interface, int32_t n)
                  {
                    ck_assert(n == expected_int32);
                    return n;
                  }
                  , es::ins<int32_t>("int32")
                  , es::outs<int32_t>("int32")
                  )
     , es::method("SendInt16"
                  , [expected_int16] (edb::const_message, edb::service_interface, int16_t n)
                  {
                    ck_assert(n == expected_int16);
                    return n;
                  }
                  , es::ins<int16_t>("int16")
                  , es::outs<int16_t>("int16")
                  )
     , es::method("SendDouble"
                  , [expected_double] (edb::const_message, edb::service_interface, double n)
                  {
                    ck_assert(n == expected_double);
                    return n;
                  }
                  , es::ins<double>("double")
                  , es::outs<double>("double")
                  )
     , es::method("SendString"
                  , [expected_string] (edb::const_message, edb::service_interface, std::string const& n)
                  {
                    std::cout << "SendString " << n.size() << " " << n << std::endl;
                    ck_assert(n == expected_string);
                    return n;
                  }
                  , es::ins<std::string>("string")
                  , es::outs<std::string>("string")
                  )
     , es::method("GetVoid"
                  , [expected_bool] (edb::const_message, edb::service_interface, bool b)
                  {
                    ck_assert(b == expected_bool);
                  }
                  , es::ins<bool>("string")
                  )
     , es::method("SendStringWithBool"
                  , [expected_string, expected_bool] (edb::const_message, edb::service_interface
                                                      , std::string const& n, bool b)
                  {
                    ck_assert(n == expected_string);
                    ck_assert(b == expected_bool);
                    return n;
                  }
                  , es::ins<std::string, bool>("string", "bool")
                  , es::outs<std::string>("string")
                  )
     , es::method("SendStringAndBool"
                  , [expected_string, expected_bool] (edb::const_message, edb::service_interface
                                                      , std::string const& n, bool b
                                                      , bool* out)
                  {
                    std::cout << "Running SendStringAndBool" << std::endl;
                    ck_assert(n == expected_string);
                    ck_assert(b == expected_bool);
                    *out = b;
                    return n;
                  }
                  , es::ins<std::string, bool>("string", "bool")
                  , es::outs<std::string, bool>("string", "bool")
                  )
     , es::method("SendStringAndBoolWithoutReturn"
                  , [expected_string, expected_bool] (edb::const_message, edb::service_interface
                                                      , std::string const& s, bool b
                                                      , std::string* out_s, bool* out_b)
                  {
                    std::cout << "Running SendStringAndBool" << std::endl;
                    ck_assert(s == expected_string);
                    ck_assert(b == expected_bool);
                    *out_s = s;
                    *out_b = b;
                  }
                  , es::ins<std::string, bool>("string", "bool")
                  , es::outs<std::string, bool>("string", "bool")
                  )
    );
  static_cast<void>(iface);

  using namespace std::placeholders;
  edb::name_request<std::uint32_t>(c_, g_bus, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE
                                   , & ::on_name_request);

  std::cout << "registered" << std::endl;

  edb::object o = c_.get_object(g_bus, g_path);
  edb::proxy p = o.get_proxy(g_interface);

  using namespace std::placeholders;
  p.call<bool>
    ("SendBool"
     , -1
     , std::bind
     ([expected_bool] (std::error_code const& ec, edb::const_message const& /*msg*/, bool b)
      {
        if(!ec)
          {
            std::cout << "bool received " << b << std::endl;
            ck_assert(b == expected_bool);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname = "", *errmsg = "";
            // eldbus_message_error_get(msg, &errname, &errmsg);
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_bool
     );

  p.call<char>
    ("SendByte"
     , -1
     , std::bind
     ([expected_byte] (std::error_code const& ec, edb::const_message const& msg, char c)
      {
        if(!ec)
          {
            std::cout << "char received " << c << std::endl;
            ck_assert(c == expected_byte);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_byte
     );

  p.call<uint32_t>
    ("SendUint32"
     , -1
     , std::bind
     ([expected_uint32] (std::error_code const& ec, edb::const_message const& msg, uint32_t i)
      {
        if(!ec)
          {
            std::cout << "uint32_t received " << i << std::endl;
            ck_assert(i == expected_uint32);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_uint32
     );

  p.call<int32_t>
    ("SendInt32"
     , -1
     , std::bind
     ([expected_int32] (std::error_code const& ec, edb::const_message const& msg, int32_t i)
      {
        if(!ec)
          {
            std::cout << "int32_t received " << i << std::endl;
            ck_assert(i == expected_int32);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_int32
     );

  p.call<int16_t>
    ("SendInt16"
     , -1
     , std::bind
     ([expected_int16] (std::error_code const& ec, edb::const_message const& msg, int16_t i)
      {
        if(!ec)
          {
            std::cout << "int16_t received " << i << std::endl;
            ck_assert(i == expected_int16);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_int16
     );

  p.call<double>
    ("SendDouble"
     , -1
     , std::bind
     ([expected_double] (std::error_code const& ec, edb::const_message const& msg, double i)
      {
        if(!ec)
          {
            std::cout << "double received " << i << std::endl;
            ck_assert(i == expected_double);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_double
     );

  p.call<std::string>
    ("SendString"
     , -1
     , std::bind
     ([expected_string] (std::error_code const& ec, edb::const_message const& msg, std::string i)
      {
        if(!ec)
          {
            std::cout << "string received " << i << std::endl;
            ck_assert(i == expected_string);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_string
     );

  p.call<void>
    ("GetVoid"
     , -1
     , std::bind
     ([] (std::error_code const& ec, edb::const_message const& msg)
      {
        if(!ec)
          {
            std::cout << "GetVoid returned succesfully" << std::endl;
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2)
     , expected_bool
     );

  p.call<std::string>
    ("SendStringWithBool"
     , -1
     , std::bind
     ([expected_string] (std::error_code const& ec, edb::const_message const& msg, std::string i)
      {
        if(!ec)
          {
            std::cout << "string received " << i << std::endl;
            ck_assert(i == expected_string);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4)
     , expected_string
     , expected_bool
     );

  p.call<std::tuple<std::string, bool> >
    ("SendStringAndBool"
     , -1
     , std::bind
     ([expected_string, expected_bool] 
      (std::error_code const& ec, edb::const_message const& msg, std::string i, bool b)
      {
        if(!ec)
          {
            std::cout << "string received " << i << std::endl;
            ck_assert(i == expected_string);
            ck_assert(b == expected_bool);
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4, _5)
     , expected_string
     , expected_bool
     );

  p.call<std::tuple<std::string, bool> >
    ("SendStringAndBoolWithoutReturn"
     , -1
     , std::bind
     ([expected_string, expected_bool]
      (std::error_code const& ec, edb::const_message const& msg, std::string i, bool b)
      {
        if(!ec)
          {
            std::cout << "string received " << i << std::endl;
            ck_assert(i == expected_string);
            ck_assert(b == expected_bool);

            ::ecore_main_loop_quit();
          }
        else
          {
            std::cout << "error " << ec.message() << std::endl;
            const char *errname, *errmsg;
            std::tie(errname, errmsg) = msg.error_get();
            std::cout << "error " << errname << " " << errmsg << std::endl;
            std::abort();
          }
      }
      , _1, _2, _4, _5)
     , expected_string
     , expected_bool
     );

  // eldbus_name_owner_changed_callback_add(c.native_handle(), bus, on_name_owner_changed,
  //                                        c.native_handle(), EINA_TRUE);

  ecore_main_loop_begin();

  std::cout << "out of loop" << std::endl;
}
END_TEST

void
eldbus_test_client(TCase* tc)
{
  tcase_add_test(tc, eldbus_cxx_client);
}
