#ifndef ELDBUS_ERROR_HH
#define ELDBUS_ERROR_HH

#include <Eina.hh>

namespace efl { namespace eldbus {

inline Eina_Error call_error_code()
{
  static Eina_Error error = eina_error_msg_static_register("Error while making eldbus rpc call");
  return error;
}

inline Eina_Error signature_mismatch_error_code()
{
  static Eina_Error error = eina_error_msg_static_register("Mismatch function signature and signature specification");
  return error;
}

} }

#endif
