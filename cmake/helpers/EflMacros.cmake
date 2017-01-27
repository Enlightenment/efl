# NOTE:
#
# - Use function() whenever possible, macro() doesn't start a new
#   variable scope and needs careful handling to avoid left overs.
#
# - To 'export' values or lists, use the macros SET_GLOBAL() and
#   LIST_APPEND_GLOBAL().
#

# LIST_APPEND_GLOBAL(Var Element)
#
# Append to a list in the global variable scope (cache internal)
function(LIST_APPEND_GLOBAL _var _element)
  set(_local ${${_var}})
  list(APPEND _local ${_element})
  set(${_var} ${_local} CACHE INTERNAL "")
endfunction()

# SET_GLOBAL(Var Value [Help])
#
# Set a variable in the global variable scope (cache internal)
function(SET_GLOBAL _var _value)
  set(${_var} "${_value}" CACHE INTERNAL "${ARGN}")
endfunction()

unset(EFL_ALL_OPTIONS CACHE)
unset(EFL_ALL_LIBS CACHE)
unset(EFL_ALL_TESTS CACHE)
unset(EFL_PKG_CONFIG_MISSING_OPTIONAL CACHE)

# EFL_OPTION_BACKEND(Name Help Backend1 ... BackendN
#                    [DEPENDS "COND1; COND2; NOT COND3" FAILED_VALUE]
#                    [REQUIRED])
#
# Wrapper around EFL_OPTION() and EFL_BACKEND_CHOICES()
# that creates an option 'CHOICE' within possible backends
# when these are found (${Backend}_FOUND).
#
# The REQUIRED keyword will force a valid backend, otherwise 'none' is
# allowed.
#
# If a backend was chose, then ${Name}_CFLAGS and ${Name}_LDFLAGS are
# set to lists based on ${Backend}_CFLAGS or
# ${Backend}_INCLUDE_DIR/${Backend}_INCLUDE_DIRS and
# ${Backend}_LDFLAGS or ${Backend}_LIBRARIES
function(EFL_OPTION_BACKEND _name _help)
  cmake_parse_arguments(PARAMS "" "" "DEPENDS" ${ARGN})

  EFL_BACKEND_CHOICES(${_name} ${PARAMS_UNPARSED_ARGUMENTS})

  EFL_OPTION(${_name} "${_help}" "${${_name}_DEFAULT}"
    CHOICE ${${_name}_CHOICES}
    DEPENDS ${PARAMS_DEPENDS})

  if(NOT ${_name} STREQUAL "none")
    string(TOUPPER "${${_name}}" _backend)
    if(DEFINED ${_backend}_CFLAGS)
      set(_cflags ${${_backend}_CFLAGS})
    else()
      set(_cflags "")
      foreach(_d ${${_backend}_INCLUDE_DIR} ${${_backend}_INCLUDE_DIRS})
        list(APPEND _cflags "-I${_d}")
      endforeach()
    endif()
    if(DEFINED ${_backend}_LDFLAGS)
      set(_ldflags ${${_backend}_LDFLAGS})
    else()
      set(_ldflags "")
      foreach(_d ${${_backend}_LIBRARIES})
        list(APPEND _ldflags "-l${_d}")
      endforeach()
    else()
    endif()
    SET_GLOBAL(${_name}_CFLAGS "${_cflags}")
    SET_GLOBAL(${_name}_LDFLAGS "${_ldflags}")
    SET_GLOBAL(${_name}_ENABLED ON)
  else()
    unset(${_name}_CFLAGS CACHE)
    unset(${_name}_LDFLAGS CACHE)
    SET_GLOBAL(${_name}_ENABLED OFF)
  endif()
endfunction()

# EFL_OPTION(Name Help Default [STRING|BOOL|FILEPATH|PATH]
#            [CHOICE c1;...;cN]
#            [DEPENDS "COND1; COND2; NOT COND3" FAILED_VALUE])
#
# Declare an option() that will be automatically printed by
# EFL_OPTIONS_SUMMARY()
#
# To extend the EFL_OPTIONS_SUMMARY() message, use
# EFL_OPTION_SET_MESSAGE(Name Message)
function(EFL_OPTION _name _help _defval)

  cmake_parse_arguments(PARAMS "" "" "CHOICE;DEPENDS" ${ARGN})

  set(_type)
  set(_vartype)
  set(_choices)

  list(LENGTH PARAMS_UNPARSED_ARGUMENTS _argc)
  if(_argc LESS 1)
    set(_type BOOL)
    set(_vartype BOOL)
  else()
    list(GET PARAMS_UNPARSED_ARGUMENTS 0 _type)
    set(_vartype ${_type})
    list(REMOVE_AT PARAMS_UNPARSED_ARGUMENTS 0)
  endif()

  if(PARAMS_CHOICE)
    set(_type CHOICE)
    set(_vartype STRING)
    SET_GLOBAL(EFL_OPTION_CHOICES_${_name} "${PARAMS_CHOICE}" "Possible values for ${_name}")
    set(_choices " (Choices: ${PARAMS_CHOICE})")
  endif()

  if(_type STREQUAL "BOOL")
    # force ON/OFF representation
    if(_defval)
      set(_defval ON)
    else()
      set(_defval OFF)
    endif()
  endif()

  if(EFL_LIB_CURRENT)
    if(NOT DEFINED EFL_ALL_OPTIONS_${EFL_LIB_CURRENT})
      LIST_APPEND_GLOBAL(EFL_ALL_OPTIONS EFL_ALL_OPTIONS_${EFL_LIB_CURRENT})
    endif()
    LIST_APPEND_GLOBAL(EFL_ALL_OPTIONS_${EFL_LIB_CURRENT} ${_name})
  else()
    LIST_APPEND_GLOBAL(EFL_ALL_OPTIONS ${_name})
  endif()

  SET_GLOBAL(EFL_OPTION_DEFAULT_${_name} "${_defval}" "Default value for ${_name}")
  SET_GLOBAL(EFL_OPTION_TYPE_${_name} "${_vartype}" "Type of ${_name}")

  set(_available ON)
  if(PARAMS_DEPENDS)
    list(LENGTH PARAMS_DEPENDS _count)
    list(GET PARAMS_DEPENDS 0 _deps)
    list(GET PARAMS_DEPENDS 1 _deps_failure_value)
    if(_deps_failure_value STREQUAL "NOTFOUND")
      message(FATAL_ERROR "EFL_OPTION(${_name}) has DEPENDS but no value when dependencies fail")
    endif()

    set(_missing_deps "")
    foreach(_d ${_deps})
      if(${_d})
      else()
        set(_available OFF)
        list(APPEND _missing_deps "${_d}")
      endif()
    endforeach()
  endif()

  if(_available)
    if(DEFINED EFL_OPTION_ORIGINAL_VALUE_${_name})
      set(${_name} ${EFL_OPTION_ORIGINAL_VALUE_${_name}} CACHE ${_type} "${_help}${_choices}" FORCE)
      unset(EFL_OPTION_ORIGINAL_VALUE_${_name} CACHE)
    else()
      set(${_name} ${_defval} CACHE ${_type} "${_help}${_choices}")
    endif()
    unset(EFL_OPTION_DEPENDS_MISSING_${_name} CACHE)
    option(${_name} "${_help}${_choices}" "${${_name}}")
  else()
    if(NOT DEFINED EFL_OPTION_ORIGINAL_VALUE_${_name})
      if(DEFINED ${_name})
        SET_GLOBAL(EFL_OPTION_ORIGINAL_VALUE_${_name} "${${_name}}")
      endif()
    endif()
    SET_GLOBAL(EFL_OPTION_DEPENDS_MISSING_${_name} "${_missing_deps}")
    set(${_name} "${_deps_failure_value}" CACHE "${_type}" "Missing dependencies (${_help}${_choices})" FORCE)
  endif()

  if(_choices)
    list(FIND PARAMS_CHOICE "${${_name}}" _ret)
    if(${_ret} EQUAL -1)
      message(FATAL_ERROR "Invalid choice ${_name}=${${_name}}${_choices}")
    endif()
  endif()
endfunction()

# EFL_OPTION_SET_MESSAGE(Name Message)
#
# Extends the summary line output by EFL_OPTIONS_SUMMARY()
# with more details.
function(EFL_OPTION_SET_MESSAGE _name _message)
  SET_GLOBAL(EFL_OPTION_MESSAGE_${_name} "${_message}")
endfunction()


# _EFL_OPTIONS_SUMMARY_INTERNAL(Prefix Option)
#
# Internal function for EFL_OPTIONS_SUMMARY().
function(_EFL_OPTIONS_SUMMARY_INTERNAL _prefix _o)
  set(_v ${${_o}})
  set(_d ${EFL_OPTION_DEFAULT_${_o}})
  if(EFL_OPTION_DEPENDS_MISSING_${_o})
    set(_i "requires: ${EFL_OPTION_DEPENDS_MISSING_${_o}}, was: ${EFL_OPTION_ORIGINAL_VALUE_${_o}}")
  elseif("${_v}" STREQUAL "${_d}")
    set(_i "default")
  else()
    set(_i "default: ${_d}")
  endif()
  if(EFL_OPTION_MESSAGE_${_o})
    set(_m " [${EFL_OPTION_MESSAGE_${_o}}]")
  else()
    set(_m)
  endif()
  message(STATUS "${_prefix}${_o}=${_v} (${_i})${_m}")
endfunction()

# EFL_OPTIONS_SUMMARY()
# Shows the summary of options, their values and related messages.
function(EFL_OPTIONS_SUMMARY)
  message(STATUS "EFL ${PROJECT_VERSION} Options Summary:")
  message(STATUS "  CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
  message(STATUS "  CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
  foreach(_o ${EFL_ALL_OPTIONS})
    if(_o MATCHES "^EFL_ALL_OPTIONS_")
      string(REGEX REPLACE "^EFL_ALL_OPTIONS_" "" _name "${_o}")
      message(STATUS "")
      message(STATUS "  ${_name} options:")
      foreach(_so ${${_o}})
        _EFL_OPTIONS_SUMMARY_INTERNAL("    " ${_so})
      endforeach()
    else()
      _EFL_OPTIONS_SUMMARY_INTERNAL("  " ${_o})
    endif()
  endforeach()
  message(STATUS "")
  message(STATUS "EFL Libraries:")
  foreach(_o ${EFL_ALL_LIBS})
    message(STATUS "  ${_o}${_mods}")
    foreach(_m ${${_o}_MODULES})
      string(REGEX REPLACE "^${_o}-module-" "" _m ${_m})
      message(STATUS "    dynamic: ${_m}")
    endforeach()
    foreach(_m ${${_o}_STATIC_MODULES})
      string(REGEX REPLACE "^${_o}-module-" "" _m ${_m})
      message(STATUS "    static.: ${_m}")
    endforeach()
    unset(_m)
  endforeach()

  if(EFL_PKG_CONFIG_MISSING_OPTIONAL)
    message(STATUS "")
    message(STATUS "The following pkg-config optional modules are missing:")
    foreach(_m ${EFL_PKG_CONFIG_MISSING_OPTIONAL})
      message(STATUS "  ${_m}")
    endforeach()
    unset(_m)
  endif()
endfunction()

set(EFL_ALL_LIBS)
set(EFL_ALL_TESTS)

# EFL_FINALIZE()
#
# Finalize EFL processing, adding extra targets.
function(EFL_FINALIZE)
  add_custom_target(all-libs DEPENDS ${EFL_ALL_LIBS})
  add_custom_target(all-tests DEPENDS ${EFL_ALL_TESTS})
endfunction()

set(VAR_HEADER_FILE_CONTENT HEADER_FILE_CONTENT CACHE INTERNAL "")
unset(${VAR_HEADER_FILE_CONTENT} CACHE)
unset(CHECK_SCOPE CACHE)
unset(CHECK_SCOPE_UPPERCASE CACHE)

# CHECK_INIT(scope)
#
# Initialize the scope for the following FUNC_CHECK, TYPE_CHECK,
# HEADER_CHECK... calls.
function(CHECK_INIT scope)
  set(CHECK_SCOPE scope CACHE INTERNAL "Scope of current *_CHECK functions")
  if(scope)
    string(TOUPPER ${scope} scope_uc)
    SET_GLOBAL(CHECK_SCOPE_UPPERCASE ${scope_uc})
    set(_suffix "_${scope_uc}")
  else()
    set(_suffix "")
  endif()
  SET_GLOBAL(VAR_HEADER_FILE_CONTENT HEADER_FILE_CONTENT${_suffix})
  SET_GLOBAL(${VAR_HEADER_FILE_CONTENT} "")
endfunction()

# CHECK_APPEND_DEFINE(name value)
#
# If value evaluates to true:
#    #define ${name} ${value}
# otherwise:
#    /* #undef ${name} */
#
# NOTE: ${name} is not modified at all, if it must include
# CHECK_SCOPE_UPPERCASE or CHECK_SCOPE, do it yourself.
function(CHECK_APPEND_DEFINE name value)
  SET_GLOBAL(${VAR_HEADER_FILE_CONTENT} "${${VAR_HEADER_FILE_CONTENT}}#ifdef ${name}\n#undef ${name}\n#endif\n")
  if(value)
    if(value STREQUAL ON OR value STREQUAL TRUE)
      set(value 1)
    endif()
    SET_GLOBAL(${VAR_HEADER_FILE_CONTENT} "${${VAR_HEADER_FILE_CONTENT}}#define ${name} ${value}\n\n")
  else()
    SET_GLOBAL(${VAR_HEADER_FILE_CONTENT} "${${VAR_HEADER_FILE_CONTENT}}/* #undef ${name} */\n\n")
  endif()
endfunction()

# CHECK_NAME_DEFAULT(name variable)
#
# Create the default name based on ${name}
# and stores in ${variable}.
#
# This will automatically prepend ${CHECK_SCOPE_UPPERCASE} if it's
# defined, will translate everything to uppercase and fix it to be a
# valid C-symbol.
function(CHECK_NAME_DEFAULT name var)
  string(TOUPPER ${name} v)
  string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "${v}")
  string(REGEX REPLACE "_{2,}" "_" v "${v}")
  if(CHECK_SCOPE_UPPERCASE)
    set(v "${CHECK_SCOPE_UPPERCASE}_${v}")
  endif()
  set(${var} ${v} PARENT_SCOPE)
endfunction()

# HEADER_CHECK(header [NAME variable] [INCLUDE_FILES extra1.h .. extraN.h])
#
# Check if the header file exists, in such case define variable
# in configuration file.
#
# Variable defaults to HAVE_${HEADER}, where HEADER is the uppercase
# representation of the first parameter. It can be overridden using
# NAME keyword.
#
# To include extra files, then use INCLUDE_FILES keyword.
function(HEADER_CHECK header)
  CHECK_NAME_DEFAULT(HAVE_${header} var)

  cmake_parse_arguments(PARAMS "" "NAME" "INCLUDE_FILES" ${ARGN})

  if(PARAMS_NAME)
    set(var ${PARAMS_NAME})
  endif()

  set(CMAKE_EXTRA_INCLUDE_FILES "${PARAMS_INCLUDE_FILES}")

  CHECK_INCLUDE_FILE(${header} ${var})
  CHECK_APPEND_DEFINE(${var} "${${var}}")
endfunction()

# FUNC_CHECK(func [NAME variable]
#            [INCLUDE_FILES header1.h .. headerN.h]
#            [LIBRARIES lib1 ... libN]
#            [DEFINITIONS -DA=1 .. -DN=123]
#            [FLAGS -cmdlineparam1 .. -cmdlineparamN]
#            [CXX])
#
# Check if the function exists, in such case define variable in
# configuration file.
#
# Variable defaults to HAVE_${FUNC}, where FUNC is the uppercase
# representation of the first parameter. It can be overridden using
# NAME keyword.
#
# To define include files use INCLUDE_FILES keyword.
#
# To use C++ compiler, use CXX keyword
function(FUNC_CHECK func)
  CHECK_NAME_DEFAULT(HAVE_${func} var)

  cmake_parse_arguments(PARAMS "CXX" "NAME" "INCLUDE_FILES;LIBRARIES;DEFINITIONS;FLAGS" ${ARGN})

  set(CMAKE_REQUIRED_LIBRARIES "${PARAMS_LIBRARIES}")
  set(CMAKE_REQUIRED_DEFINITIONS "${PARAMS_DEFINITIONS}")
  set(CMAKE_REQUIRED_FLAGS "${PARAMS_FLAGS}")

  if(PARAMS_NAME)
    set(var ${PARAMS_NAME})
  endif()

  if(PARAMS_CXX)
    check_cxx_symbol_exists(${func} "${PARAMS_INCLUDE_FILES}" ${var})
  else()
    check_symbol_exists(${func} "${PARAMS_INCLUDE_FILES}" ${var})
  endif()

  CHECK_APPEND_DEFINE(${var} "${${var}}")
endfunction()

# TYPE_CHECK(type [NAME variable] [SIZEOF variable]
#           [INCLUDE_FILES file1.h ... fileN.h]
#           [LIBRARIES lib1 ... libN]
#           [DEFINITIONS -DA=1 .. -DN=123]
#           [FLAGS -cmdlineparam1 .. -cmdlineparamN]
#           [CXX])
#
# Check if the type exists and its size, in such case define variable
# in configuration file.
#
# Variable defaults to HAVE_${TYPE}, where TYPE is the uppercase
# representation of the first parameter. It can be overridden using
# NAME keyword.
#
# To define include files use INCLUDE_FILES keyword.
#
# To use C++ compiler, use CXX keyword
function(TYPE_CHECK type)
  CHECK_NAME_DEFAULT(HAVE_${type} var)
  CHECK_NAME_DEFAULT(SIZEOF_${type} sizeof)

  cmake_parse_arguments(PARAMS "CXX" "NAME;SIZEOF" "INCLUDE_FILES;LIBRARIES;DEFINITIONS;FLAGS" ${ARGN})

  set(CMAKE_REQUIRED_LIBRARIES "${PARAMS_LIBRARIES}")
  set(CMAKE_REQUIRED_DEFINITIONS "${PARAMS_DEFINITIONS}")
  set(CMAKE_REQUIRED_FLAGS "${PARAMS_FLAGS}")
  set(CMAKE_EXTRA_INCLUDE_FILES "${PARAMS_INCLUDE_FILES}")

  if(PARAMS_NAME)
    set(var ${PARAMS_NAME})
  endif()

  if(PARAMS_SIZEOF)
    set(sizeof ${PARAMS_SIZEOF})
  endif()

  if(PARAMS_CXX)
    set(lang CXX)
  else()
    set(lang C)
  endif()

  CHECK_TYPE_SIZE(${type} ${var} LANGUAGE ${lang})
  CHECK_APPEND_DEFINE(${var} "${HAVE_${var}}")
  CHECK_APPEND_DEFINE(${sizeof} "${${var}}")
endfunction()

# EFL_HEADER_CHECKS_FINALIZE(file)
#
# Write the configuration gathered with HEADER_CHECK(), TYPE_CHECK()
# and FUNC_CHECK() to the given file.
function(EFL_HEADER_CHECKS_FINALIZE file)
  get_filename_component(filename ${file} NAME)
  string(TOUPPER _${filename}_ file_sym)
  string(REGEX REPLACE "[^a-zA-Z0-9]" "_" file_sym "${file_sym}")

  file(WRITE ${file}.new "#ifndef ${file_sym}\n#define ${file_sym} 1\n\n${${VAR_HEADER_FILE_CONTENT}}\n#endif /* ${file_sym} */\n")
  if (NOT EXISTS ${file})
    file(RENAME ${file}.new ${file})
    message(STATUS "${file} was generated.")
  else()
    file(MD5 ${file}.new _new_md5)
    file(MD5 ${file} _old_md5)
    if(_new_md5 STREQUAL _old_md5)
      message(STATUS "${file} is unchanged.")
    else()
      file(REMOVE ${file})
      file(RENAME ${file}.new ${file})
      message(STATUS "${file} was updated.")
    endif()
  endif()
  unset(${VAR_HEADER_FILE_CONTENT} CACHE) # allow to reuse with an empty contents
  unset(CHECK_SCOPE CACHE)
  unset(CHECK_SCOPE_UPPERCASE CACHE)
  set(VAR_HEADER_FILE_CONTENT HEADER_FILE_CONTENT CACHE INTERNAL "")
endfunction()

# EFL_FILES_TO_ABSOLUTE(Var Source_Dir Binary_Dir [file1 ... fileN])
#
# Convert list of files to absolute path. If not absolute, then
# check inside Source_Dir and if it fails assumes it's inside Binary_Dir
function(EFL_FILES_TO_ABSOLUTE _var _srcdir _bindir)
  set(_lst "")
  foreach(f ${ARGN})
    if(EXISTS "${f}" OR IS_ABSOLUTE "${f}")
      list(APPEND _lst "${f}")
    elseif(EXISTS "${_srcdir}/${f}")
      list(APPEND _lst "${_srcdir}/${f}")
    else()
      list(APPEND _lst "${_bindir}/${f}")
    endif()
  endforeach()
  set(${_var} "${_lst}" PARENT_SCOPE)
endfunction()

# EFL_PKG_CONFIG_EVAL_TO(Var Name [module1 ... moduleN])
#
# Evaluate the list of of pkg-config modules and assign to variable
# Var. If it's missing, abort with a message saying ${Name} is missing
# the list of modules.
#
# OPTIONAL keyword may be used to convert the remaining elements in optional
# packages.
function(EFL_PKG_CONFIG_EVAL_TO _var _name)
  set(_found "")
  set(_missing "")
  set(_missing_optional "")
  set(_optional OFF)
  set(_have_definitions "")
  foreach(f ${ARGN})
    if(${f} STREQUAL "OPTIONAL")
      set(_optional ON)
    else()
      string(REGEX REPLACE "[><=].*\$" "" v "${f}")
      string(REGEX REPLACE "[^A-Za-z0-9]" "_" v "${v}")
      string(TOUPPER "${v}" v)
      pkg_check_modules(PKG_CONFIG_DEP_${v} ${f})
      if(PKG_CONFIG_DEP_${v}_FOUND)
        list(APPEND _found ${f})
        list(APPEND _have_definitions "-DHAVE_${v}=1" "-DENABLE_${v}=1")
      elseif(_optional)
        list(APPEND _missing_optional ${f})
        LIST_APPEND_GLOBAL(EFL_PKG_CONFIG_MISSING_OPTIONAL ${f})
      else()
        list(APPEND _missing ${f})
      else()
      endif()
    endif()
  endforeach()

  unset(${_var}_CFLAGS CACHE)
  unset(${_var}_LDFLAGS CACHE)
  unset(PKG_CONFIG_${_var} CACHE)
  unset(PKG_CONFIG_${_var}_CFLAGS CACHE)
  unset(PKG_CONFIG_${_var}_FOUND CACHE)
  unset(PKG_CONFIG_${_var}_INCLUDE_DIRS CACHE)
  unset(PKG_CONFIG_${_var}_INCLUDEDIR CACHE)
  unset(PKG_CONFIG_${_var}_LDFLAGS CACHE)
  unset(PKG_CONFIG_${_var}_LIBDIR CACHE)
  unset(PKG_CONFIG_${_var}_LIBRARIES CACHE)
  unset(PKG_CONFIG_${_var}_LIBRARIES CACHE)
  unset(PKG_CONFIG_${_var}_LIBRARIES_DIR CACHE)
  unset(PKG_CONFIG_${_var}_LIBS CACHE)
  unset(PKG_CONFIG_${_var}_VERSION CACHE)
  if(NOT _missing)
    SET_GLOBAL(${_var} "${_found}")
    SET_GLOBAL(${_var}_MISSING "${_missing_optional}")

    if(_found)
      pkg_check_modules(PKG_CONFIG_${_var} ${_found})
      SET_GLOBAL(${_var}_CFLAGS "${_have_definitions};${PKG_CONFIG_${_var}_CFLAGS}")
      SET_GLOBAL(${_var}_LDFLAGS "${PKG_CONFIG_${_var}_LDFLAGS}")
    endif()
  else()
    message(FATAL_ERROR "${_name} missing required pkg-config modules: ${_missing}")
  endif()
endfunction()

# EFL_PKG_CONFIG_EVAL(Name Private_List Public_List)
#
# Evaluates both lists and creates ${Name}_PKG_CONFIG_REQUIRES as well as
# ${Name}_PKG_CONFIG_REQUIRES_PRIVATE with found elements.
#
# OPTIONAL keyword may be used to convert the remaining elements in optional
# packages.
function(EFL_PKG_CONFIG_EVAL _target _private _public)
  EFL_PKG_CONFIG_EVAL_TO(${_target}_PKG_CONFIG_REQUIRES ${_target} ${_public})
  EFL_PKG_CONFIG_EVAL_TO(${_target}_PKG_CONFIG_REQUIRES_PRIVATE ${_target} ${_private})

  set(_lst ${${_target}_PKG_CONFIG_REQUIRES_MISSING})
  foreach(_e ${${_target}_PKG_CONFIG_REQUIRES_PRIVATE_MISSING})
    list(APPEND _lst ${_e})
  endforeach()
  if(_lst)
    message(STATUS "${_target} missing optional pkg-config: ${_lst}")
  endif()
endfunction()

function(EFL_PKG_CONFIG_LIB_WRITE)
  set(_pkg_config_requires)
  set(_pkg_config_requires_private)
  set(_libraries)
  set(_public_libraries)

  get_target_property(eo_files_public ${EFL_LIB_CURRENT} EFL_EO_PUBLIC)
  if(eo_files_public)
    set(_eoinfo "eoincludedir=\${datarootdir}/eolian/include
eolian_flags=-I\${pc_sysrootdir}\${eoincludedir}/${EFL_LIB_CURRENT}-${PROJECT_VERSION_MAJOR}")
  else()
    set(_eoinfo "")
  endif()

  foreach(_e ${${EFL_LIB_CURRENT}_PKG_CONFIG_REQUIRES})
    set(_pkg_config_requires "${_pkg_config_requires} ${_e}")
  endforeach()

  foreach(_e ${${EFL_LIB_CURRENT}_PKG_CONFIG_REQUIRES_PRIVATE})
    set(_pkg_config_requires_private "${_pkg_config_requires_private} ${_e}")
  endforeach()

  foreach(_e ${LIBRARIES})
    if(TARGET ${_e})
      set(_pkg_config_requires_private "${_pkg_config_requires_private} ${_e}>=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    else()
      set(_libraries "${_libraries} -l${_e}")
    endif()
  endforeach()

  foreach(_e ${PUBLIC_LIBRARIES})
    if(TARGET ${_e})
      set(_pkg_config_requires "${_pkg_config_requires} ${_e}>=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    else()
      set(_public_libraries "${_public_libraries} -l${_e}")
    endif()
  endforeach()

  if(NOT ${EFL_LIB_CURRENT} STREQUAL "efl")
    set(_cflags " -I\${includedir}/${EFL_LIB_CURRENT}-${PROJECT_VERSION_MAJOR}")
  endif()

  # TODO: handle eolian needs

  set(_contents
"prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include
datarootdir=\${prefix}/share
datadir=\${datarootdir}
pkgdatadir=\${datadir}/${EFL_LIB_CURRENT}
modules=\${libdir}/${EFL_LIB_CURRENT}/modules
${_eoinfo}

Name: ${EFL_LIB_CURRENT}
Description: ${DESCRIPTION}
Version: ${VERSION}
Requires:${_pkg_config_requires}
Requires.private:${_pkg_config_requires_private}
Libs: -L\${libdir} -l${EFL_LIB_CURRENT}${_public_libraries}
Libs.private:${_libraries}
Cflags: -I\${includedir}/efl-${PROJECT_VERSION_MAJOR}${_cflags}
")
  file(WRITE "${CMAKE_BINARY_DIR}/lib/pkgconfig/${EFL_LIB_CURRENT}.pc" "${_contents}")
  install(FILES "${CMAKE_BINARY_DIR}/lib/pkgconfig/${EFL_LIB_CURRENT}.pc"
    DESTINATION "lib/pkgconfig")
endfunction()

# _EFL_INCLUDE_OR_DETECT(Name Source_Dir)
#
# Internal macro that will include(${Source_Dir}/CMakeLists.txt) if
# that exists, otherwise will check if there is a single source file,
# in that case it will automatically define SOURCES to that (including
# extras such as headers and .eo)
#
# Name is only used to print out messages when it's auto-detected.
macro(_EFL_INCLUDE_OR_DETECT _name _srcdir)
  if(EXISTS ${_srcdir}/CMakeLists.txt)
    include(${_srcdir}/CMakeLists.txt)
  else()
    # doc says it's not recommended because it can't know if more files
    # were added, but we're doing this explicitly to handle one file.
    file(GLOB _autodetect_files RELATIVE ${_srcdir}
        ${_srcdir}/*.c
        ${_srcdir}/*.h
        ${_srcdir}/*.hh
        ${_srcdir}/*.cxx
        ${_srcdir}/*.cpp
        )
    list(LENGTH _autodetect_files _autodetect_files_count)
    if(_autodetect_files_count GREATER 1)
      message(WARNING "${_name}: ${_srcdir} contains no CMakeLists.txt and contains more than one source file. Don't know what to do, then ignored.")
    elseif(_autodetect_files_count EQUAL 1)
      file(GLOB SOURCES RELATIVE ${_srcdir}
        ${_srcdir}/*.c
        ${_srcdir}/*.h
        ${_srcdir}/*.hh
        ${_srcdir}/*.cxx
        ${_srcdir}/*.cpp
        )
      file(GLOB EO_FILES RELATIVE ${_srcdir}
        ${_srcdir}/*.eo
        ${_srcdir}/*.eot
        )
      message(STATUS "${_name} auto-detected as: ${SOURCES}")
      if(EO_FILES)
        message(STATUS "${_name} EO auto-detected as: ${EO_FILES}")
      endif()
    else()
      message(STATUS "${_name} contains no auto-detectable sources.")
    endif()
    unset(_autodetect_files_count)
    unset(_autodetect_files)
  endif()
endmacro()

# _EFL_LIB_PROCESS_MODULES_INTERNAL()
#
# Internal function to process modules of current EFL_LIB()
function(_EFL_LIB_PROCESS_MODULES_INTERNAL)
  unset(${EFL_LIB_CURRENT}_MODULES CACHE)
  unset(${EFL_LIB_CURRENT}_STATIC_MODULES CACHE)

  if(EXISTS ${EFL_MODULES_SOURCE_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "${EFL_MODULES_SOURCE_DIR}/CMakeLists.txt shouldn't exist. Modules are expected to be defined in their own directory.")
  else()
    file(GLOB modules RELATIVE ${EFL_MODULES_SOURCE_DIR} ${EFL_MODULES_SOURCE_DIR}/*)
    foreach(module ${modules})
      if(IS_DIRECTORY ${EFL_MODULES_SOURCE_DIR}/${module})
        set(EFL_MODULE_SCOPE ${module})

        file(GLOB submodules RELATIVE ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE} ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE}/*)
        foreach(submodule ${submodules})
          if(IS_DIRECTORY ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE}/${submodule})
            EFL_MODULE(${submodule})
          endif()
          unset(submodule)
          unset(submodules)
        endforeach()
      else()
        set(EFL_MODULE_SCOPE)
        EFL_MODULE(${module})
      endif()
      unset(EFL_MODULE_SCOPE)
    endforeach()
  endif()

  if(${EFL_LIB_CURRENT}_MODULES)
    add_custom_target(${EFL_LIB_CURRENT}-modules DEPENDS ${${EFL_LIB_CURRENT}_MODULES})
  endif()
endfunction()

# _EFL_LIB_PROCESS_BINS_INTERNAL()
#
# Internal function to process bins of current EFL_LIB()
function(_EFL_LIB_PROCESS_BINS_INTERNAL)
  unset(${EFL_LIB_CURRENT}_BINS CACHE)

  if(EXISTS ${EFL_BIN_SOURCE_DIR}/CMakeLists.txt)
    EFL_BIN(${EFL_LIB_CURRENT})
  else()
    file(GLOB bins RELATIVE ${EFL_BIN_SOURCE_DIR} ${EFL_BIN_SOURCE_DIR}/*)
    foreach(bin ${bins})
      if(IS_DIRECTORY ${EFL_BIN_SOURCE_DIR}/${bin})
        EFL_BIN(${bin})
      endif()
    endforeach()
  endif()

  if(NOT ${EFL_LIB_CURRENT}_BINS AND EXISTS ${EFL_BIN_SOURCE_DIR} AND NOT EXISTS ${EFL_BIN_SOURCE_DIR}/CMakeLists.txt)
    EFL_BIN(${EFL_LIB_CURRENT})
  endif()

  if(${EFL_LIB_CURRENT}_BINS)
    add_custom_target(${EFL_LIB_CURRENT}-bins DEPENDS ${${EFL_LIB_CURRENT}_BINS})
  endif()
endfunction()

# _EFL_LIB_PROCESS_TESTS_INTERNAL()
#
# Internal function to process tests of current EFL_LIB()
function(_EFL_LIB_PROCESS_TESTS_INTERNAL)
  unset(${EFL_LIB_CURRENT}_TESTS CACHE)

  if(EXISTS ${EFL_TESTS_SOURCE_DIR}/CMakeLists.txt)
    EFL_TEST(${EFL_LIB_CURRENT})
  else()
    file(GLOB tests RELATIVE ${EFL_TESTS_SOURCE_DIR} ${EFL_TESTS_SOURCE_DIR}/*)
    foreach(test ${tests})
      if(IS_DIRECTORY ${EFL_TESTS_SOURCE_DIR}/${test})
        EFL_TEST(${test})
      endif()
    endforeach()
  endif()

  if(NOT ${EFL_LIB_CURRENT}_TESTS AND EXISTS ${EFL_TESTS_SOURCE_DIR} AND NOT EXISTS ${EFL_TESTS_SOURCE_DIR}/CMakeLists.txt)
    EFL_TEST(${EFL_LIB_CURRENT})
  endif()

  if(${EFL_LIB_CURRENT}_TESTS)
    add_custom_target(${EFL_LIB_CURRENT}-tests DEPENDS ${${EFL_LIB_CURRENT}_TESTS})
    LIST_APPEND_GLOBAL(EFL_ALL_TESTS ${EFL_LIB_CURRENT}-tests)
  endif()
endfunction()

define_property(TARGET PROPERTY EFL_EO_PRIVATE
  BRIEF_DOCS "EFL's .eo/.eot files associated with this target and not installed"
  FULL_DOCS "The list of all .eo or .eot files this target uses but doesn't install")

define_property(TARGET PROPERTY EFL_EO_PUBLIC
  BRIEF_DOCS "EFL's .eo/.eot files associated with this target and installed"
  FULL_DOCS "The list of all .eo or .eot files this target uses and installs")

# EFL_LIB(Name)
#
# adds a library ${Name} automatically setting object/target
# properties based on script-modifiable variables:
#  - DESCRIPTION: results in ${Name}_DESCRIPTION and fills pkg-config files.
#  - PKG_CONFIG_REQUIRES: results in ${Name}_PKG_CONFIG_REQUIRES and
#    fills pkg-config files. Elements after 'OPTIONAL' keyword are
#    optional.
#  - PKG_CONFIG_REQUIRES_PRIVATE: results in
#    ${Name}_PKG_CONFIG_REQUIRES_PRIVATE and fills pkg-config
#    files. Elements after 'OPTIONAL' keyword are optional.
#  - INCLUDE_DIRECTORIES: results in target_include_directories
#  - SYSTEM_INCLUDE_DIRECTORIES: results in target_include_directories(SYSTEM)
#  - OUTPUT_NAME
#  - SOURCES source files that are needed. Eo files should go in
#    PUBLIC_EO_FILES or EO_FILES.
#  - PUBLIC_HEADERS
#  - VERSION (defaults to project version)
#  - SOVERSION (defaults to project major version)
#  - LIBRARY_TYPE: SHARED or STATIC, defaults to SHARED
#  - OBJECT_DEPENDS: say this object depends on other files (ie: includes)
#  - DEPENDENCIES: results in add_dependencies()
#  - LIBRARIES: results in target_link_libraries(LINK_PRIVATE)
#  - PUBLIC_LIBRARIES: results in target_link_libraries(LINK_PUBLIC)
#  - DEFINITIONS: target_compile_definitions()
#  - PUBLIC_EO_FILES: the eo files will be used to build that lib, and will be installed to the filesystem
#  - EO_FILES: the eo files will be used to build that lib but not installed.
#  - COMPILE_FLAGS: extra CFLAGS to append.
#  - LINK_FLAGS: extra LDFLAGS to append.
#
# Defines the following variables that can be used within the included files:
#  - EFL_LIB_CURRENT to ${Name}
#  - EFL_LIB_SOURCE_DIR to source dir of ${Name} libraries
#  - EFL_LIB_BINARY_DIR to binary dir of ${Name} libraries
#  - EFL_BIN_SOURCE_DIR to source dir of ${Name} executables
#  - EFL_BIN_BINARY_DIR to binary dir of ${Name} executables
#  - EFL_MODULES_SOURCE_DIR to source dir of ${Name} modules
#  - EFL_MODULES_BINARY_DIR to binary dir of ${Name} modules
#  - EFL_TESTS_SOURCE_DIR to source dir of ${Name} tests
#  - EFL_TESTS_BINARY_DIR to binary dir of ${Name} tests
#
# Modules are processed like:
#   - loop for directories in src/modules/${EFL_LIB_CURRENT}:
#      - if a src/modules/${EFL_LIB_CURRENT}/${Module}/CMakeLists.txt
#        use variables as documented in EFL_MODULE()
#      - otherwise loop for scoped-modules in
#        src/modules/${EFL_LIB_CURRENT}/${EFL_MODULE_SCOPE}/CMakeLists.txt
#        and use variables as documented in EFL_MODULE()
#
# EFL_MODULE() will handle MODULE_TYPE=ON;OFF;STATIC, handling
# dependencies and installation in the proper path, considering
# ${EFL_MODULE_SCOPE} whenever it's set.
#
# Binaries and tests are processed similarly:
#   - if src/bin/${EFL_LIB_CURRENT}/CMakeLists.txt exist, then use
#     variables as documented in EFL_BIN() or EFL_TEST().  The target
#     will be called ${EFL_LIB_CURRENT}-bin or ${EFL_LIB_CURRENT}-test
#     and the test OUTPUT_NAME defaults to ${EFL_LIB_CURRENT}_suite.
#   - otherwise loop for directories in src/bin/${EFL_LIB_CURRENT} and
#     for each src/bin/${EFL_LIB_CURRENT}/${Entry}/CMakeLists.txt use
#     variables as documented in EFL_BIN() or EFL_TEST().  Binaries
#     must provide an unique name that will be used as both target and
#     OUTPUT_NAME. Tests will generate targets
#     ${EFL_LIB_CURRENT}-test-${Entry}, while OUTPUT_NAME is ${Entry}.
#
# NOTE: src/modules/${EFL_LIB_CURRENT}/CMakeLists.txt is not
#       allowed as it makes no sense to have a single module named
#       after the library.
#
function(EFL_LIB _target)
  set(EFL_LIB_CURRENT ${_target})
  set(EFL_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/${_target})
  set(EFL_LIB_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/lib/${_target})
  set(EFL_BIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/bin/${_target})
  set(EFL_BIN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/bin/${_target})
  set(EFL_MODULES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${_target})
  set(EFL_MODULES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/modules/${_target})
  set(EFL_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/${_target})
  set(EFL_TESTS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/tests/${_target})

  unset(EFL_ALL_OPTIONS_${EFL_LIB_CURRENT} CACHE)

  set(DESCRIPTION)
  set(PKG_CONFIG_REQUIRES)
  set(PKG_CONFIG_REQUIRES_PRIVATE)
  set(INCLUDE_DIRECTORIES)
  set(SYSTEM_INCLUDE_DIRECTORIES)
  set(OUTPUT_NAME)
  set(SOURCES)
  set(PUBLIC_HEADERS)
  set(VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
  set(SOVERSION ${PROJECT_VERSION_MAJOR})
  set(LIBRARY_TYPE SHARED)
  set(OBJECT_DEPENDS)
  set(DEPENDENCIES)
  set(LIBRARIES)
  set(PUBLIC_LIBRARIES)
  set(DEFINITIONS)
  set(PUBLIC_EO_FILES)
  set(EO_FILES)
  set(COMPILE_FLAGS)
  set(LINK_FLAGS)

  string(TOUPPER "${_target}" _target_uc)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/${_target}.cmake OPTIONAL)
  include(${EFL_LIB_SOURCE_DIR}/CMakeLists.txt OPTIONAL)
  if(LIBRARY_TYPE STREQUAL SHARED AND NOT PUBLIC_HEADERS)
    message(FATAL_ERROR "Shared libraries must install public headers!")
  endif()

  #merge public eo files into sources
  set(SOURCES ${SOURCES} ${PUBLIC_EO_FILES} ${EO_FILES})

  EFL_FILES_TO_ABSOLUTE(_headers ${EFL_LIB_SOURCE_DIR} ${EFL_LIB_BINARY_DIR}
    ${PUBLIC_HEADERS})
  EFL_FILES_TO_ABSOLUTE(_sources ${EFL_LIB_SOURCE_DIR} ${EFL_LIB_BINARY_DIR}
    ${SOURCES})
  EFL_FILES_TO_ABSOLUTE(_obj_deps ${EFL_LIB_SOURCE_DIR} ${EFL_LIB_BINARY_DIR}
    ${OBJECT_DEPENDS})
  EFL_FILES_TO_ABSOLUTE(_public_eo_files ${EFL_LIB_SOURCE_DIR} ${EFL_LIB_BINARY_DIR}
    ${PUBLIC_EO_FILES})
  EFL_FILES_TO_ABSOLUTE(_eo_files ${EFL_LIB_SOURCE_DIR} ${EFL_LIB_BINARY_DIR}
    ${EO_FILES})

  foreach(public_eo_file ${PUBLIC_EO_FILES})
    list(APPEND _headers ${EFL_LIB_BINARY_DIR}/${public_eo_file}.h)
  endforeach()
  unset(public_eo_file)

  EFL_PKG_CONFIG_EVAL(${_target} "${PKG_CONFIG_REQUIRES_PRIVATE}" "${PKG_CONFIG_REQUIRES}")

  set(__link_flags ${${_target}_PKG_CONFIG_REQUIRES_PRIVATE_LDFLAGS} ${${_target}_PKG_CONFIG_REQUIRES_LDFLAGS} ${LINK_FLAGS})
  set(__compile_flags ${${_target}_PKG_CONFIG_REQUIRES_PRIVATE_CFLAGS} ${${_target}_PKG_CONFIG_REQUIRES_CFLAGS} -DPACKAGE_DATA_DIR=\\"${CMAKE_INSTALL_FULL_DATADIR}/${_target}/\\" ${COMPILE_FLAGS})

  set(_link_flags)
  foreach(_l ${__link_flags})
    set(_link_flags "${_link_flags} ${_l}")
  endforeach()

  set(_compile_flags)
  foreach(_c ${__compile_flags})
    set(_compile_flags "${_compile_flags} ${_c}")
  endforeach()

  add_library(${_target} ${LIBRARY_TYPE} ${_sources} ${_headers})
  set_target_properties(${_target} PROPERTIES
    FRAMEWORK TRUE
    DEFINE_SYMBOL "EFL_${_target_uc}_BUILD"
    PUBLIC_HEADER "${_headers}"
    OBJECT_DEPENDS "${_obj_deps}"
    EFL_EO_PRIVATE "${_eo_files}"
    EFL_EO_PUBLIC "${_public_eo_files}"
    LINK_FLAGS "${_link_flags}"
    COMPILE_FLAGS "${_compile_flags} -DDLL_EXPORT")

  if(DEPENDENCIES)
    add_dependencies(${_target} ${DEPENDENCIES})
  endif()

  if(LIBRARIES)
    target_link_libraries(${_target} LINK_PRIVATE ${LIBRARIES})
  endif()
  if(PUBLIC_LIBRARIES)
    target_link_libraries(${_target} PUBLIC ${PUBLIC_LIBRARIES})
  endif()

  target_include_directories(${_target} PUBLIC
    ${INCLUDE_DIRECTORIES}
    ${EFL_LIB_SOURCE_DIR}
    ${EFL_LIB_BINARY_DIR}
    )
  if(SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories(${_target} SYSTEM PUBLIC ${SYSTEM_INCLUDE_DIRECTORIES})
  endif()

  if(DEFINITIONS)
    target_compile_definitions(${_target} PRIVATE ${DEFINITIONS})
  endif()

  if(OUTPUT_NAME)
    set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
  endif()

  if(VERSION AND SOVERSION)
    set_target_properties(${_target} PROPERTIES
      VERSION ${VERSION}
      SOVERSION ${SOVERSION})
  endif()

  EFL_CREATE_EO_RULES(${_target} ${EFL_LIB_SOURCE_DIR} ${EFL_LIB_BINARY_DIR})

  EFL_PKG_CONFIG_LIB_WRITE()

  install(TARGETS ${_target}
    PUBLIC_HEADER DESTINATION include/${_target}-${PROJECT_VERSION_MAJOR}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
  install(FILES
    ${_public_eo_files} DESTINATION share/eolian/include/${_target}-${PROJECT_VERSION_MAJOR}
    )
  # do not leak those into binaries, modules or tests
  unset(_sources)
  unset(_headers)
  unset(_obj_deps)
  unset(_public_eo_files)
  unset(_eo_files)
  unset(INCLUDE_DIRECTORIES)
  unset(SYSTEM_INCLUDE_DIRECTORIES)
  unset(OUTPUT_NAME)
  unset(SOURCES)
  unset(PUBLIC_HEADERS)
  unset(VERSION)
  unset(SOVERSION)
  unset(LIBRARY_TYPE)
  unset(OBJECT_DEPENDS)
  unset(DEPENDENCIES)
  unset(LIBRARIES)
  unset(PUBLIC_LIBRARIES)
  unset(DEFINITIONS)
  unset(DESCRIPTION)
  unset(PUBLIC_EO_FILES)
  unset(EO_FILES)
  unset(PKG_CONFIG_REQUIRES)
  unset(PKG_CONFIG_REQUIRES_PRIVATE)

  _EFL_LIB_PROCESS_BINS_INTERNAL()
  _EFL_LIB_PROCESS_MODULES_INTERNAL()
  _EFL_LIB_PROCESS_TESTS_INTERNAL()

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/post/${_target}.cmake OPTIONAL)

  LIST_APPEND_GLOBAL(EFL_ALL_LIBS ${_target})
endfunction()

# EFL_BIN(Name)
#
# Adds a binary (executable) for ${EFL_LIB_CURRENT} using
# ${EFL_BIN_SOURCE_DIR} and ${EFL_BIN_BINARY_DIR}
#
# Settings:
#  - INCLUDE_DIRECTORIES: results in target_include_directories
#  - SYSTEM_INCLUDE_DIRECTORIES: results in target_include_directories(SYSTEM)
#  - OUTPUT_NAME
#  - SOURCES
#  - OBJECT_DEPENDS: say this object depends on other files (ie: includes)
#  - DEPENDENCIES: results in add_dependencies(), defaults to
#    ${EFL_LIB_CURRENT}-modules
#  - LIBRARIES: results in target_link_libraries()
#  - DEFINITIONS: target_compile_definitions()
#  - INSTALL_DIR: defaults to bin. If empty, won't install.
#  - COMPILE_FLAGS: extra CFLAGS to append.
#  - LINK_FLAGS: extra LDFLAGS to append.
#
# NOTE: it's meant to be called by files included by EFL_LIB() or similar,
# otherwise you need to prepare the environment yourself.
function(EFL_BIN _binname)
  set(INCLUDE_DIRECTORIES)
  set(SYSTEM_INCLUDE_DIRECTORIES)
  set(OUTPUT_NAME ${_binname})
  set(SOURCES)
  set(OBJECT_DEPENDS)
  if(TARGET ${EFL_LIB_CURRENT}-modules)
    set(DEPENDENCIES ${EFL_LIB_CURRENT}-modules)
  else()
    set(DEPENDENCIES)
  endif()
  set(LIBRARIES)
  set(DEFINITIONS)
  set(INSTALL ON)
  set(INSTALL_DIR bin)
  set(PKG_CONFIG_REQUIRES)
  set(PKG_CONFIG_REQUIRES_PRIVATE)
  set(COMPILE_FLAGS)
  set(LINK_FLAGS)

  if(_binname STREQUAL ${EFL_LIB_CURRENT})
    set(_binsrcdir "${EFL_BIN_SOURCE_DIR}")
    set(_binbindir "${EFL_BIN_BINARY_DIR}")
    set(_bintarget "${EFL_LIB_CURRENT}-bin") # otherwise target would exist
  else()
    set(_binsrcdir "${EFL_BIN_SOURCE_DIR}/${_binname}")
    set(_binbindir "${EFL_BIN_BINARY_DIR}/${_binname}")
    set(_bintarget "${_binname}")
  endif()

  _EFL_INCLUDE_OR_DETECT("Binary ${_bintarget}" ${_binsrcdir})

  if(NOT SOURCES)
    message(WARNING "${_binsrcdir}/CMakeLists.txt defines no SOURCES")
    return()
  endif()
  if(PUBLIC_HEADERS)
    message(WARNING "${_binsrcdir}/CMakeLists.txt should not define PUBLIC_HEADERS, it's not to be installed.")
  endif()
  if(PKG_CONFIG_REQUIRES)
    message(WARNING "${_binsrcdir}/CMakeLists.txt should not define PKG_CONFIG_REQUIRES. Use PKG_CONFIG_REQUIRES_PRIVATE instead")
  endif()

  EFL_FILES_TO_ABSOLUTE(_sources ${_binsrcdir} ${_binbindir} ${SOURCES})
  EFL_FILES_TO_ABSOLUTE(_obj_deps ${_binsrcdir} ${_binbindir} ${OBJECT_DEPENDS})

  EFL_PKG_CONFIG_EVAL(${_bintarget} "${PKG_CONFIG_REQUIRES_PRIVATE}" "")

  add_executable(${_bintarget} ${_sources})

  if(_obj_deps)
    set_target_properties(${_bintarget} PROPERTIES
      OBJECT_DEPENDS "${_obj_deps}")
  endif()

  if(DEPENDENCIES)
    add_dependencies(${_bintarget} ${DEPENDENCIES})
  endif()

  target_include_directories(${_bintarget} PRIVATE
    ${_binrcdir}
    ${_binbindir}
    ${INCLUDE_DIRECTORIES})
  if(SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories(${_bintarget} SYSTEM PRIVATE
      ${SYSTEM_INCLUDE_DIRECTORIES})
  endif()
  target_link_libraries(${_bintarget} LINK_PRIVATE
    ${EFL_LIB_CURRENT}
    ${LIBRARIES})

  if(DEFINITIONS)
    target_compile_definitions(${_bintarget} PRIVATE ${DEFINITIONS})
  endif()

  if(OUTPUT_NAME)
    set_target_properties(${_bintarget} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
  endif()

  set(_link_flags)
  foreach(_l ${${_bintarget}_PKG_CONFIG_REQUIRES_PRIVATE_LDFLAGS} ${LINK_FLAGS})
    set(_link_flags "${_link_flags} ${_l}")
  endforeach()

  set(_compile_flags)
  foreach(_c ${${_bintarget}_PKG_CONFIG_REQUIRES_PRIVATE_CFLAGS} ${COMPILE_FLAGS})
    set(_compile_flags "${_compile_flags} ${_c}")
  endforeach()

  set_target_properties(${_bintarget} PROPERTIES
    LINK_FLAGS "${_link_flags}"
    COMPILE_FLAGS "${_compile_flags} -DPACKAGE_DATA_DIR=\\\"${CMAKE_INSTALL_FULL_DATADIR}/${_target}/\\\" -DPACKAGE_SRC_DIR=\\\"${CMAKE_SOURCE_DIR}/\\\"")

  if(INSTALL_DIR)
    install(TARGETS ${_bintarget} RUNTIME DESTINATION ${INSTALL_DIR})
  endif()

  LIST_APPEND_GLOBAL(${EFL_LIB_CURRENT}_BINS ${_bintarget})
endfunction()

# EFL_TEST(Name)
#
# Adds a test for ${EFL_LIB_CURRENT} using
# ${EFL_TESTS_SOURCE_DIR} and ${EFL_TESTS_BINARY_DIR}
#
# Settings:
#  - INCLUDE_DIRECTORIES: results in target_include_directories
#  - SYSTEM_INCLUDE_DIRECTORIES: results in target_include_directories(SYSTEM)
#  - OUTPUT_NAME
#  - SOURCES
#  - OBJECT_DEPENDS: say this object depends on other files (ie: includes)
#  - DEPENDENCIES: results in add_dependencies(), defaults to
#    ${EFL_LIB_CURRENT}-modules
#  - LIBRARIES: results in target_link_libraries()
#  - DEFINITIONS: target_compile_definitions()
#  - COMPILE_FLAGS: extra CFLAGS to append.
#  - LINK_FLAGS: extra LDFLAGS to append.
#
# NOTE: it's meant to be called by files included by EFL_LIB() or similar,
# otherwise you need to prepare the environment yourself.
function(EFL_TEST _testname)
  if(NOT CHECK_FOUND)
    message(STATUS "${EFL_LIB_CURRENT} test ${_testname} ignored since no 'check' library was found.")
    return()
  endif()

  set(INCLUDE_DIRECTORIES)
  set(SYSTEM_INCLUDE_DIRECTORIES)
  set(OUTPUT_NAME ${_testname})
  set(SOURCES)
  set(OBJECT_DEPENDS)
  if(TARGET ${EFL_LIB_CURRENT}-modules)
    set(DEPENDENCIES ${EFL_LIB_CURRENT}-modules)
  else()
    set(DEPENDENCIES)
  endif()
  set(LIBRARIES)
  set(DEFINITIONS)
  set(PKG_CONFIG_REQUIRES)
  set(PKG_CONFIG_REQUIRES_PRIVATE)
  set(COMPILE_FLAGS)
  set(LINK_FLAGS)

  if(_testname STREQUAL ${EFL_LIB_CURRENT})
    set(_testsrcdir "${EFL_TESTS_SOURCE_DIR}")
    set(_testbindir "${EFL_TESTS_BINARY_DIR}")
    set(_testtarget "${EFL_LIB_CURRENT}-test") # otherwise target would exist
    set(OUTPUT_NAME "${EFL_LIB_CURRENT}_suite") # backward compatible
  elseif(_testname STREQUAL "suite")
    set(_testsrcdir "${EFL_TESTS_SOURCE_DIR}/${_testname}")
    set(_testbindir "${EFL_TESTS_BINARY_DIR}/${_testname}")
    set(_testtarget "${EFL_LIB_CURRENT}-test") # target for main test, as above
    set(OUTPUT_NAME "${EFL_LIB_CURRENT}_suite") # backward compatible
  else()
    set(_testsrcdir "${EFL_TESTS_SOURCE_DIR}/${_testname}")
    set(_testbindir "${EFL_TESTS_BINARY_DIR}/${_testname}")
    set(_testtarget "${EFL_LIB_CURRENT}-test-${_testname}")
  endif()

  _EFL_INCLUDE_OR_DETECT("Test ${_testtarget}" ${_testsrcdir})

  if(NOT SOURCES)
    message(WARNING "${_testsrcdir}/CMakeLists.txt defines no SOURCES")
    return()
  endif()
  if(PUBLIC_HEADERS)
    message(WARNING "${_testsrcdir}/CMakeLists.txt should not define PUBLIC_HEADERS, it's not to be installed.")
  endif()
  if(PKG_CONFIG_REQUIRES)
    message(WARNING "${_testsrcdir}/CMakeLists.txt should not define PKG_CONFIG_REQUIRES. Use PKG_CONFIG_REQUIRES_PRIVATE instead")
  endif()

  EFL_FILES_TO_ABSOLUTE(_sources ${_testsrcdir} ${_testbindir} ${SOURCES})
  EFL_FILES_TO_ABSOLUTE(_obj_deps ${_testsrcdir} ${_testbindir} ${OBJECT_DEPENDS})

  EFL_PKG_CONFIG_EVAL(${_testtarget} "${PKG_CONFIG_REQUIRES_PRIVATE}" "")

  add_executable(${_testtarget} EXCLUDE_FROM_ALL ${_sources})

  if(_obj_deps)
    set_target_properties(${_testtarget} PROPERTIES
      OBJECT_DEPENDS "${_obj_deps}")
  endif()

  if(DEPENDENCIES)
    add_dependencies(${_testtarget} ${DEPENDENCIES})
  endif()

  target_include_directories(${_testtarget} PRIVATE
    ${_testsrcdir}
    ${_testbindir}
    ${EFL_TESTS_SOURCE_DIR}
    ${EFL_TESTS_BINARY_DIR}
    ${INCLUDE_DIRECTORIES})
  target_include_directories(${_testtarget} SYSTEM PRIVATE
    ${SYSTEM_INCLUDE_DIRECTORIES}
    ${CHECK_INCLUDE_DIRS})
  target_link_libraries(${_testtarget} LINK_PRIVATE
    ${EFL_LIB_CURRENT}
    ${LIBRARIES}
    ${CHECK_LIBRARIES})

  target_compile_definitions(${_testtarget} PRIVATE
    "-DPACKAGE_DATA_DIR=\"${EFL_TESTS_SOURCE_DIR}\""
    "-DTESTS_SRC_DIR=\"${_testsrcdir}\""
    "-DTESTS_BUILD_DIR=\"${_testbindir}\""
    "-DTESTS_WD=\"\""
    ${DEFINITIONS}
    )

  if(OUTPUT_NAME)
    set_target_properties(${_testtarget} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
  endif()

  set(_link_flags)
  foreach(_l ${${_testtarget}_PKG_CONFIG_REQUIRES_PRIVATE_LDFLAGS} ${LINK_FLAGS})
    set(_link_flags "${_link_flags} ${_l}")
  endforeach()

  set(_compile_flags)
  foreach(_c ${${_testtarget}_PKG_CONFIG_REQUIRES_PRIVATE_CFLAGS} ${COMPILE_FLAGS})
    set(_compile_flags "${_compile_flags} ${_c}")
  endforeach()

  set_target_properties(${_testtarget} PROPERTIES
    LINK_FLAGS "${_link_flags}"
    COMPILE_FLAGS "${_compile_flags}"
    LIBRARY_OUTPUT_DIRECTORY "${_testbindir}"
    RUNTIME_OUTPUT_DIRECTORY "${_testbindir}")

  add_test(NAME ${_testtarget} COMMAND ${_testtarget})
  LIST_APPEND_GLOBAL(${EFL_LIB_CURRENT}_TESTS ${_testtarget})

  add_test(${_testtarget}-build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${_testtarget})
  set_tests_properties(${_testtarget} PROPERTIES DEPENDS ${_testtarget}-build)
endfunction()

# EFL_MODULE(Name)
#
# Adds a module for ${EFL_LIB_CURRENT} using
# ${EFL_MODULES_SOURCE_DIR} and ${EFL_MODULES_BINARY_DIR}
# as well as ${EFL_MODULE_SCOPE} if it's contained into
# a subdir, such as eina's "mp" or evas "engines".
#
# To keep it simple to use, user is only expected to define variables:
#  - SOURCES
#  - OBJECT_DEPENDS
#  - LIBRARIES
#  - INCLUDE_DIRECTORIES
#  - SYSTEM_INCLUDE_DIRECTORIES
#  - DEFINITIONS
#  - MODULE_TYPE: one of ON;OFF;STATIC, defaults to ON
#  - INSTALL_DIR: defaults to
#    lib/${EFL_LIB_CURRENT}/modules/${EFL_MODULE_SCOPE}/${Name}/v-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}/.
#    If empty, won't install.
#  - PKG_CONFIG_REQUIRES_PRIVATE: results in
#    ${Name}_PKG_CONFIG_REQUIRES_PRIVATE. Elements after 'OPTIONAL'
#    keyword are optional.
#  - COMPILE_FLAGS: extra CFLAGS to append.
#  - LINK_FLAGS: extra LDFLAGS to append.
#
# NOTE: since the file will be included it shouldn't mess with global variables!
function(EFL_MODULE _modname)
  if(EFL_MODULE_SCOPE)
    set(_modsrcdir ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE}/${_modname})
    set(_modoutdir lib/${EFL_LIB_CURRENT}/modules/${EFL_MODULE_SCOPE}/${_modname}/v-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set(_modbindir ${EFL_MODULES_BINARY_DIR}/${EFL_MODULE_SCOPE}/${_modname})
    set(_modtarget ${EFL_LIB_CURRENT}-module-${EFL_MODULE_SCOPE}-${_modname})
    string(TOUPPER "${EFL_LIB_CURRENT}_MODULE_TYPE_${EFL_MODULE_SCOPE}_${_modname}" _modoptionname)
    if(NOT DEFINED ${_modoptionname}_DEFAULT)
      set(${_modoptionname}_DEFAULT "ON")
    endif()
    EFL_OPTION(${_modoptionname} "Build ${EFL_LIB_CURRENT} module ${EFL_MODULE_SCOPE}/${_modname}" ${${_modoptionname}_DEFAULT} CHOICE ON;OFF;STATIC)
  else()
    set(_modsrcdir ${EFL_MODULES_SOURCE_DIR}/${_modname})
    set(_modoutdir lib/${EFL_LIB_CURRENT}/modules/${_modname}/v-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set(_modbindir ${EFL_MODULES_BINARY_DIR}/${_modname})
    set(_modtarget ${EFL_LIB_CURRENT}-module-${_modname})
    string(TOUPPER "${EFL_LIB_CURRENT}_MODULE_TYPE_${_modname}" _modoptionname)
    if(NOT DEFINED ${_modoptionname}_DEFAULT)
      set(${_modoptionname}_DEFAULT "ON")
    endif()
    EFL_OPTION(${_modoptionname} "Build ${EFL_LIB_CURRENT} module ${_modname}" ${${_modoptionname}_DEFAULT} CHOICE ON;OFF;STATIC)
  endif()

  set(SOURCES)
  set(OBJECT_DEPENDS)
  set(LIBRARIES)
  set(INCLUDE_DIRECTORIES)
  set(SYSTEM_INCLUDE_DIRECTORIES)
  set(DEFINITIONS)
  set(MODULE_TYPE "${${_modoptionname}}")
  set(INSTALL_DIR ${_modoutdir})
  set(PKG_CONFIG_REQUIRES)
  set(PKG_CONFIG_REQUIRES_PRIVATE)
  set(COMPILE_FLAGS)
  set(LINK_FLAGS)

  _EFL_INCLUDE_OR_DETECT("Module ${_modtarget}" ${_modsrcdir})

  if(NOT SOURCES)
    message(WARNING "${_modsrcdir}/CMakeLists.txt defines no SOURCES")
    return()
  endif()
  if(PUBLIC_HEADERS)
    message(WARNING "${_modsrcdir}/CMakeLists.txt should not define PUBLIC_HEADERS, it's not to be installed.")
  endif()
  if(PKG_CONFIG_REQUIRES)
    message(WARNING "${_modsrcdir}/CMakeLists.txt should not define PKG_CONFIG_REQUIRES. Use PKG_CONFIG_REQUIRES_PRIVATE instead")
  endif()

  if("${MODULE_TYPE}" STREQUAL "OFF")
    return()
  elseif("${MODULE_TYPE}" STREQUAL "STATIC")
    set(_modtype STATIC)
  else()
    set(_modtype MODULE)
  endif()

  EFL_FILES_TO_ABSOLUTE(_sources ${_modsrcdir} ${_modbindir} ${SOURCES})
  EFL_FILES_TO_ABSOLUTE(_obj_deps ${_modsrcdir} ${_modbindir} ${OBJECT_DEPENDS})

  EFL_PKG_CONFIG_EVAL(${_modtarget} "${PKG_CONFIG_REQUIRES_PRIVATE}" "")

  add_library(${_modtarget} ${_modtype} ${_sources})
  set_target_properties(${_modtarget} PROPERTIES
    OBJECT_DEPENDS "${_obj_deps}"
    PREFIX ""
    OUTPUT_NAME "module")

  target_include_directories(${_modtarget} PRIVATE
    ${_modsrcdir}
    ${_modbindir}
    ${INCLUDE_DIRECTORIES})
  target_include_directories(${_modtarget} SYSTEM PUBLIC
    ${SYSTEM_INCLUDE_DIRECTORIES})
  target_link_libraries(${_modtarget} LINK_PRIVATE ${LIBRARIES})

  target_compile_definitions(${_modtarget} PRIVATE ${DEFINITIONS})

  set(_link_flags)
  foreach(_l ${${_modtarget}_PKG_CONFIG_REQUIRES_PRIVATE_LDFLAGS} ${LINK_FLAGS})
    set(_link_flags "${_link_flags} ${_l}")
  endforeach()

  set(_compile_flags)
  foreach(_c ${${_modtarget}_PKG_CONFIG_REQUIRES_PRIVATE_CFLAGS} ${COMPILE_FLAGS})
    set(_compile_flags "${_compile_flags} ${_c}")
  endforeach()

  set_target_properties(${_modtarget} PROPERTIES
    LINK_FLAGS "${_link_flags}"
    COMPILE_FLAGS "${_compile_flags}"
    LIBRARY_OUTPUT_DIRECTORY "${_modoutdir}"
    ARCHIVE_OUTPUT_DIRECTORY "${_modoutdir}"
    RUNTIME_OUTPUT_DIRECTORY "${_modoutdir}")

  if("${MODULE_TYPE}" STREQUAL "STATIC")
    target_link_libraries(${EFL_LIB_CURRENT} LINK_PRIVATE ${_modtarget})
    target_compile_definitions(${EFL_LIB_CURRENT} PRIVATE "-D${_modoptionname}_STATIC=1")
    target_include_directories(${_modtarget} PRIVATE
      ${EFL_LIB_SOURCE_DIR}
      ${EFL_LIB_BINARY_DIR})
    set_target_properties(${_modtarget} PROPERTIES
      POSITION_INDEPENDENT_CODE TRUE)

    LIST_APPEND_GLOBAL(${EFL_LIB_CURRENT}_STATIC_MODULES ${_modtarget})
  else()
    target_link_libraries(${_modtarget} LINK_PRIVATE ${EFL_LIB_CURRENT})
    target_compile_definitions(${EFL_LIB_CURRENT} PRIVATE "-D${_modoptionname}_DYNAMIC=1")
    LIST_APPEND_GLOBAL(${EFL_LIB_CURRENT}_MODULES ${_modtarget})
    if(INSTALL_DIR)
      install(TARGETS ${_modtarget} LIBRARY DESTINATION "${INSTALL_DIR}")
    endif()
  endif()
endfunction()

macro(EFL_PROJECT version)
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
      project(efl VERSION ${version})
  else ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    execute_process(
      COMMAND git rev-list --count HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    project(efl VERSION ${version}.${GIT_VERSION})
  endif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  message("VERSION ${PROJECT_VERSION}")
endmacro()

# Will use the source of the given target to create rules for creating
# the .eo.c and .eo.h files. The INCLUDE_DIRECTORIES of the target will be used
function(EFL_CREATE_EO_RULES target source_dir generation_dir)
   get_target_property(eo_files_private ${target} EFL_EO_PRIVATE)
   get_target_property(eo_files_public ${target} EFL_EO_PUBLIC)
   if(NOT eo_files_private AND NOT eo_files_public)
     return()
   endif()

   get_target_property(link_libraries ${target} LINK_LIBRARIES)
   set(all_libraries ${target} ${link_libraries})
   set(include_cmd "")
   foreach(link_target ${all_libraries})
     if(TARGET ${link_target})
       list(APPEND include_cmd -I${CMAKE_SOURCE_DIR}/src/lib/${link_target})
     endif()
   endforeach()

   set(all_eo_gen_files "")
   set(extra_include_dirs "")
   foreach(file ${eo_files_private} ${eo_files_public})
      get_filename_component(ext ${file} EXT)
      string(REGEX REPLACE "^${source_dir}/" "" filename "${file}")
      string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" relfile "${file}")

      # if sources are located in subdiretories
      get_filename_component(reldir "${filename}" DIRECTORY)
      if(reldir)
        file(MAKE_DIRECTORY "${generation_dir}/${reldir}")
        get_filename_component(absdir "${file}" DIRECTORY)
        set(rel_include_cmd -I${absdir})
        list(APPEND extra_include_dirs "${generation_dir}/${reldir}")
      else()
        set(rel_include_cmd)
      endif()

      if(${ext} STREQUAL ".eo")
        set(file_eo_gen_files ${generation_dir}/${filename}.c ${generation_dir}/${filename}.h) # TODO: ${generation_dir}/${filename}.legacy.h)
        set(out_cmd -o c:${generation_dir}/${filename}.c -o h:${generation_dir}/${filename}.h) # TODO: bug in eolian_gen -o l:${generation_dir}/${filename}.legacy.h)
      elseif(${ext} STREQUAL ".eot")
        set(file_eo_gen_files ${generation_dir}/${filename}.h)
        # TODO: looks like a bug in eolian_gen needs '-gh'
        set(out_cmd -gh -o h:${generation_dir}/${filename}.h)
      else()
        message(FATAL_ERROR "Unsupported eo file type: ${file}")
      endif()

      #add the custom rule
      if(file_eo_gen_files)
        add_custom_command(
           OUTPUT ${file_eo_gen_files}
           COMMAND ${EOLIAN_BIN} ${rel_include_cmd} ${include_cmd} ${EOLIAN_EXTRA_PARAMS} ${out_cmd} ${file}
           DEPENDS ${file}
           COMMENT "EOLIAN ${relfile}"
        )
        list(APPEND all_eo_gen_files ${file_eo_gen_files})
        # TODO: looks like a bug in eolian_gen doesn't generate -o l:...
        # TODO: then add an extra command
        if(${ext} STREQUAL ".eo")
          add_custom_command(
            OUTPUT ${generation_dir}/${filename}.legacy.h
            COMMAND ${EOLIAN_BIN} ${rel_include_cmd} ${include_cmd} ${EOLIAN_EXTRA_PARAMS} -gl -o l:${generation_dir}/${filename}.legacy.h ${file}
            DEPENDS ${file}
            COMMENT "EOLIAN LEGACY ${relfile}"
            )
          list(APPEND all_eo_gen_files ${generation_dir}/${filename}.legacy.h)
        endif()
      endif()
    endforeach()
    if(all_eo_gen_files)
      file(MAKE_DIRECTORY ${generation_dir})
      add_custom_target(${target}-eo
        DEPENDS ${all_eo_gen_files}
      )
      if(extra_include_dirs)
        list(REMOVE_DUPLICATES extra_include_dirs)
        target_include_directories(${target} PUBLIC ${extra_include_dirs})
      endif()
      add_dependencies(${target} ${target}-eo)
      if(${EOLIAN_BIN} STREQUAL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eolian_gen)
        add_dependencies(${target}-eo eolian-bin)
      endif()
    endif()
endfunction()

# EFL_BACKEND_CHOICES(Prefix Choice1 .. ChoiceN)
#
# Helper that will check ${ChoiceN}_FOUND and if so append to
# ${_prefix}_CHOICES as well set the first found option (in order!) as
# ${_prefix}_DEFAULT and ${_prefix}_FOUND to ON if at least one was
# found.
function(EFL_BACKEND_CHOICES _prefix)
  cmake_parse_arguments(PARAMS "REQUIRED" "" "" ${ARGN})

  set(_choices "")
  set(_defval "none")
  set(_found OFF)
  foreach(c ${PARAMS_UNPARSED_ARGUMENTS})
    if(${c}_FOUND)
      string(TOLOWER "${c}" c_lc)
      if(_defval STREQUAL "none")
        set(_defval ${c_lc})
      endif()
      set(_found ON)
      list(APPEND _choices ${c_lc})
    endif()
  endforeach()

  if(PARAMS_REQUIRED AND NOT _found)
    message(FATAL_ERROR "${_prefix} backend was required (one of ${PARAMS_UNPARSED_ARGUMENTS}) but none was found!")
  elseif(NOT PARAMS_REQUIRED)
    list(APPEND _choices "none")
  endif()

  set(${_prefix}_FOUND ${_found} PARENT_SCOPE)
  set(${_prefix}_CHOICES ${_choices} PARENT_SCOPE)
  set(${_prefix}_DEFAULT ${_defval} PARENT_SCOPE)
endfunction()
