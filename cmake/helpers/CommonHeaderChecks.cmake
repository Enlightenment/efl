unset(HEADER_FILE_CONTENT CACHE)

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
  string(TOUPPER HAVE_${header} var)
  string(REGEX REPLACE "[^a-zA-Z0-9]" "_" var "${var}")
  string(REGEX REPLACE "_{2,}" "_" var "${var}")

  cmake_parse_arguments(PARAMS "" "NAME" "INCLUDE_FILES" ${ARGN})

  if(PARAMS_NAME)
    set(var ${PARAMS_NAME})
  endif()

  set(CMAKE_EXTRA_INCLUDE_FILES "${PARAMS_INCLUDE_FILES}")

  CHECK_INCLUDE_FILE(${header} ${var})

  if(${${var}})
    SET_GLOBAL(HEADER_FILE_CONTENT "${HEADER_FILE_CONTENT}#define ${var} 1\n")
  else()
    SET_GLOBAL(HEADER_FILE_CONTENT "${HEADER_FILE_CONTENT}#undef ${var}\n")
  endif()
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
  string(TOUPPER HAVE_${func} var)
  string(REGEX REPLACE "_{2,}" "_" var "${var}")

  cmake_parse_arguments(PARAMS "CXX" "NAME" "INCLUDE_FILES;LIBRARIES;DEFINITIONS;FLAGS" ${ARGN})

  set(CMAKE_REQUIRED_LIBRARIES "${PARAMS_LIBRARIES}")
  set(CMAKE_REQUIRED_DEFINITIONS "${PARAMS_DEFINITIONS}")
  set(CMAKE_REQUIRED_FLAGS "${PARAMS_FLAGS}")

  if(PARAMS_CXX)
    check_cxx_symbol_exists(${func} "${PARAMS_INCLUDE_FILES}" ${var})
  else()
    check_symbol_exists(${func} "${PARAMS_INCLUDE_FILES}" ${var})
  endif()

  if(${${var}} )
    SET_GLOBAL(HEADER_FILE_CONTENT "${HEADER_FILE_CONTENT}#define ${var} 1\n")
  else()
    SET_GLOBAL(HEADER_FILE_CONTENT "${HEADER_FILE_CONTENT}#undef ${var}\n")
  endif()
endfunction()

# TYPE_CHECK(type [NAME variable]
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
  string(TOUPPER HAVE_${type} var)
  string(REGEX REPLACE "_{2,}" "_" var "${var}")

  cmake_parse_arguments(PARAMS "CXX" "NAME" "INCLUDE_FILES;LIBRARIES;DEFINITIONS;FLAGS" ${ARGN})

  set(CMAKE_REQUIRED_LIBRARIES "${PARAMS_LIBRARIES}")
  set(CMAKE_REQUIRED_DEFINITIONS "${PARAMS_DEFINITIONS}")
  set(CMAKE_REQUIRED_FLAGS "${PARAMS_FLAGS}")
  set(CMAKE_EXTRA_INCLUDE_FILES "${PARAMS_INCLUDE_FILES}")

  if(PARAMS_CXX)
    set(lang CXX)
  else()
    set(lang C)
  endif()

  CHECK_TYPE_SIZE(${type} ${var} LANGUAGE ${lang})

  if(HAVE_${var})
    SET_GLOBAL(HEADER_FILE_CONTENT "${HEADER_FILE_CONTENT}#define ${var} 1\n")
  else()
    SET_GLOBAL(HEADER_FILE_CONTENT "${HEADER_FILE_CONTENT}#undef ${var}\n")
  endif()
endfunction()


FUNC_CHECK(geteuid INCLUDE_FILES unistd.h)
FUNC_CHECK(getuid INCLUDE_FILES unistd.h)
FUNC_CHECK(getpagesize INCLUDE_FILES unistd.h)
FUNC_CHECK(strlcpy INCLUDE_FILES bsd/string.h)
FUNC_CHECK(mmap INCLUDE_FILES sys/mman.h)
FUNC_CHECK(fchmod INCLUDE_FILES sys/stat.h)
FUNC_CHECK(clock_gettime INCLUDE_FILES time.h)
FUNC_CHECK(fstatat NAME HAVE_ATFILE_SOURCE INCLUDE_FILES sys/stat.h)
FUNC_CHECK(dirfd INCLUDE_FILES dirent.h sys/types.h)
FUNC_CHECK(fpathconf INCLUDE_FILES unistd.h)
FUNC_CHECK(listxattr INCLUDE_FILES sys/types.h sys/xattr.h)
FUNC_CHECK(setxattr INCLUDE_FILES sys/types.h sys/xattr.h)
FUNC_CHECK(getxattr INCLUDE_FILES sys/types.h;sys/xattr.h)
FUNC_CHECK(dlopen INCLUDE_FILES dlfcn.h LIBRARIES dl)
FUNC_CHECK(mtrace INCLUDE_FILES mcheck.h)
FUNC_CHECK(strerror_r INCLUDE_FILES string.h)
FUNC_CHECK(shm_open INCLUDE_FILES sys/mman.h sys/stat.h fcntl.h LIBRARIES rt)
FUNC_CHECK(sched_getcpu INCLUDE_FILES sched.h DEFINITIONS "-D_GNU_SOURCE=1")
FUNC_CHECK(splice INCLUDE_FILES fcntl.h DEFINITIONS "-D_GNU_SOURCE=1")
FUNC_CHECK(fcntl INCLUDE_FILES fcntl.h)
FUNC_CHECK(dladdr INCLUDE_FILES dlfcn.h LIBRARIES dl DEFINITIONS "-D_GNU_SOURCE=1")

TYPE_CHECK(siginfo_t INCLUDE_FILES signal.h)

HEADER_CHECK(stdlib.h)
HEADER_CHECK(sys/mman.h)
HEADER_CHECK(dirent.h)
HEADER_CHECK(iconv.h)
HEADER_CHECK(sys/auxv.h)
HEADER_CHECK(asm/hwcap.h)
HEADER_CHECK(mcheck.h)
HEADER_CHECK(sys/types.h)
HEADER_CHECK(execinfo.h)
HEADER_CHECK(libunwind.h NAME HAVE_UNWIND)

# EFL_HEADER_CHECKS_FINALIZE(file)
#
# Write the configuration gathered with HEADER_CHECK(), TYPE_CHECK()
# and FUNC_CHECK() to the given file.
function(EFL_HEADER_CHECKS_FINALIZE file)
  file(WRITE ${file}.new ${HEADER_FILE_CONTENT})
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
  unset(HEADER_FILE_CONTENT CACHE) # allow to reuse with an empty contents
endfunction()
