include(CheckSymbolExists)
include(CheckIncludeFile)
include(CheckTypeSize)
include(FindPkgConfig)
include(CTest)
find_package(Threads REQUIRED)

pkg_check_modules(CHECK check)

EFL_OPTION(BUILD_PROFILE "How to build EFL" dev CHOICE release;dev;debug)
EFL_OPTION(EFL_NOLEGACY_API_SUPPORT "Whenever to disable non-Eo (Legacy) APIs" ON)
EFL_OPTION(EFL_EO_API_SUPPORT "Whenever to enable new Eo APIs" ON)
if(NOT EFL_EO_API_SUPPORT)
  EFL_OPTION_SET_MESSAGE(EFL_EO_API_SUPPORT "Risky!")
endif()


#define all our header / function checks
include(./CommonHeaderChecks)


#this is needed to build efl
set(EFL_BETA_API_SUPPORT 1)

#if there are thread init support from cmake then efl has threads

if (CMAKE_THREAD_LIBS_INIT)
    set(EFL_HAVE_THREADS 1)
endif()
