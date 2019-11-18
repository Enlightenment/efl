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
#ifndef EFL_OBJECT_MANUAL_IMPL_HH
#define EFL_OBJECT_MANUAL_IMPL_HH

// Skip del. Must be implemented in eo::concrete.
#define EOLIAN_CXX_EFL_DEL_DECLARATION
#define EOLIAN_CXX_EFL_DEL_IMPLEMENTATION

// Skip constructor, destructor and finalize.
#define EOLIAN_CXX_EFL_CONSTRUCTOR_DECLARATION
#define EOLIAN_CXX_EFL_CONSTRUCTOR_IMPLEMENTATION
#define EOLIAN_CXX_EFL_DESTRUCTOR_DECLARATION
#define EOLIAN_CXX_EFL_DESTRUCTOR_IMPLEMENTATION
#define EOLIAN_CXX_EFL_FINALIZE_DECLARATION
#define EOLIAN_CXX_EFL_FINALIZE_IMPLEMENTATION

// Skip auto_unref
#define EOLIAN_CXX_EFL_AUTO_UNREF_SET_DECLARATION
#define EOLIAN_CXX_EFL_AUTO_UNREF_SET_IMPLEMENTATION

#endif
