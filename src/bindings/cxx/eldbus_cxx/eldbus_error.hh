/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
