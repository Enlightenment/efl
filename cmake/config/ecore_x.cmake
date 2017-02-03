include(FindX11)

EFL_OPTION(XGESTURE "Whenever you want ecore_x to use gesture" OFF)
EFL_OPTION(XPRESENT "Whenever you want ecore_x to use xpresent" OFF)
EFL_OPTION(XINPUT2 "Whenever you want ecore_x to use xinput2" ON)
EFL_OPTION(XINPUT22 "Whenever you want ecore_x to use xinput22" OFF)

#the rest is just statically enabled
CHECK_APPEND_DEFINE(HAVE_ECORE_X_XLIB ON)
CHECK_APPEND_DEFINE(ECORE_XKB ON)
CHECK_APPEND_DEFINE(ECORE_XCOMPOSITE ON)
CHECK_APPEND_DEFINE(ECORE_XDAMAGE ON)
CHECK_APPEND_DEFINE(ECORE_XDPMS ON)
CHECK_APPEND_DEFINE(ECORE_XFIXES ON)
CHECK_APPEND_DEFINE(ECORE_XINERAMA ON)
CHECK_APPEND_DEFINE(ECORE_XRANDR ON)
CHECK_APPEND_DEFINE(ECORE_XSS ON)
CHECK_APPEND_DEFINE(ECORE_XTEST ON)

#FindX11 does not cover all parts we need so here are the 4 extensions we need to check

#copied from https://github.com/Kitware/CMake/blob/master/Modules/FindX11.cmake#L65
set(X11_INC_SEARCH_PATH
  /usr/pkg/xorg/include
  /usr/X11R6/include
  /usr/X11R7/include
  /usr/include/X11
  /usr/openwin/include
  /usr/openwin/share/include
  /opt/graphics/OpenGL/include
  /opt/X11/include
)

set(X11_LIB_SEARCH_PATH
  /usr/pkg/xorg/lib
  /usr/X11R6/lib
  /usr/X11R7/lib
  /usr/openwin/lib
  /opt/X11/lib
)

macro(find_lib_and_include name path lib)
  string(TOUPPER ${name} ecore_name)
  set(X11_${name}_FOUND OFF)
  find_library(X11_${name}_LIB ${lib} ${X11_LIB_SEARCH_PATH})
  find_path(X11_${name}_INCLUDE_PATH X11/extensions/${path} ${X11_INC_SEARCH_PATH})
  if (X11_${name}_LIB AND X11_${name}_INCLUDE_PATH)
    set(X11_${name}_FOUND ON)
  endif()

  CHECK_APPEND_DEFINE(ECORE_${ecore_name} ON)
endmacro()

if (${XGESTURE})
  find_lib_and_include(XGesture gesture.h Xgesture)
endif()

if(${XPRESENT})
  find_lib_and_include(Xpresent Xpresent.h Xpresent)
endif()

if(${XINPUT2})
  find_lib_and_include(Xi2 XInput2.h Xi)
endif()

if(${XINPUT22})
  find_lib_and_include(Xi2_2 Xpresent2.h Xi)
endif()

#build a list of all components we use
set(_X_EXTENSIONS
  X11
  X11_Xkb
  X11_Xcomposite
  X11_Xdamage
  X11_dpms
  X11_Xfixes
  X11_Xinerama
  X11_Xrandr
  X11_Xrender
  X11_XTest
  X11_Xscreensaver
)

if(${XGESTURE})
  list(APPEND _X_EXTENSIONS X11_Xgesture)
endif()

if(${XPRESENT})
  list(APPEND _X_EXTENSIONS X11_Xpresent)
endif()

if(${XINPUT2})
  list(APPEND _X_EXTENSIONS X11_Xi2)
endif()

if(${XINPUT22})
  list(APPEND _X_EXTENSIONS X11_Xi2_2)
endif()

foreach(ext ${_X_EXTENSIONS})
  if (NOT ${ext}_FOUND)
    message(FATAL_ERROR "Xserver extension ${ext} not found")
  endif()
  list(APPEND X_EXTENSIONS find-${ext})
endforeach()

configure_file(
  ${PROJECT_SOURCE_DIR}/src/lib/ecore_x/ecore_x_version.h.cmake
  ${PROJECT_BINARY_DIR}/src/lib/ecore_x/ecore_x_version.h)
