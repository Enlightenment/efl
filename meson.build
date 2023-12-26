project('efl', ['c','cpp'],
  version: '1.27.99',
  default_options : ['buildtype=plain', 'warning_level=1', 'cpp_std=c++11'],
  meson_version : '>=0.50'
)

if host_machine.system() == 'darwin'
  add_languages('objc')
endif

pkgconfig = import('pkgconfig')

version_arr = meson.project_version().split('.')

version_major = version_arr[0]
version_minor = version_arr[1]
version_micro = version_arr[2]
version_name = 'v-' + version_major + '.' + version_minor

cc = meson.get_compiler('c')
host_os = host_machine.system()

if host_os == 'linux'
  if cc.has_header_symbol('features.h', '__UCLIBC__')
    host_os = 'linux-uclibc'
  elif cc.has_header_symbol('features.h', '__dietlibc__')
    host_os = 'linux-dietlibc'
  else
    host_os = 'linux-gnu'
  endif
endif

#prepare a special linker args flag for binaries on macos
bin_linker_args = []
if host_machine.system() == 'darwin'
  bin_linker_args = ['-pagezero_size', '10000', '-image_base', '100000000']
endif

windows = ['windows', 'cygwin']
#bsd for meson 0.46 and 0.47
bsd = ['bsd', 'freebsd', 'dragonfly', 'netbsd', 'openbsd']
linux = ['linux']
osx = ['darwin']
sun = ['sunos']

sys_linux = linux.contains(host_machine.system())
sys_bsd = bsd.contains(host_machine.system())
sys_windows = windows.contains(host_machine.system())
sys_osx = osx.contains(host_machine.system())
sys_sun = sun.contains(host_machine.system())

module_files = []
evas_loader_map = []

#install paths
dir_prefix    = get_option('prefix')
dir_sysconf   = join_paths(dir_prefix, get_option('sysconfdir'))
dir_bin       = join_paths(dir_prefix, get_option('bindir'))
dir_data      = join_paths(dir_prefix, get_option('datadir'))
dir_include   = join_paths(dir_prefix, get_option('includedir'))
dir_lib       = join_paths(dir_prefix, get_option('libdir'))

#local paths
local_lib = join_paths('src', 'lib')
local_bindings = join_paths('src', 'bindings')
local_bin = join_paths('src', 'bin')
local_module = join_paths('src', 'modules')
local_tests = join_paths('src', 'tests')
local_benchmark = join_paths('src', 'benchmarks')
local_examples = join_paths('src', 'examples')
local_scripts = join_paths('src', 'scripts')

dev_cflags = []
dev_cflags_try = [
  '-fvisibility=hidden',
  '-Wfloat-compare',
  '-Wpointer-arith',
  '-Wunused-parameter',
  '-Wsign-compare',
  '-Wno-missing-field-initializers',
]

foreach cf: dev_cflags_try
  if cc.has_argument(cf)
    dev_cflags += cf
  endif
endforeach

add_project_arguments(dev_cflags, language: 'c')
add_project_arguments(dev_cflags, language: 'cpp')


langs = ['c', 'objc', 'cpp']
add_project_arguments('-DHAVE_CONFIG_H=1', language: langs)
add_project_arguments('-D_GNU_SOURCE=1', language: langs)
add_project_arguments('-DEFL_BETA_API_SUPPORT=1', language: langs)
add_project_arguments('-DNEED_RUN_IN_TREE=1', language: langs)
add_project_arguments('-DELM_INTERNAL_API_ARGESFSDFEFC=1', language: langs)
if sys_windows
  add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: langs)
  add_project_arguments('-DDLL_EXPORT=1', language: langs)
  if get_option('windows-version') == 'vista'
    add_project_arguments('-DWINVER=0x060', language: langs)
    add_project_arguments('-D_WIN32_WINNT=0x0600', language: langs)
  elif get_option('windows-version') == 'win7'
    add_project_arguments('-DWINVER=0x0601', language: langs)
    add_project_arguments('-D_WIN32_WINNT=0x0601', language: langs)
  elif get_option('windows-version') == 'win8'
    add_project_arguments('-DWINVER=0x0602', language: langs)
    add_project_arguments('-D_WIN32_WINNT=0x0602', language: langs)
  elif get_option('windows-version') == 'win81'
    add_project_arguments('-DWINVER=0x0603', language: langs)
    add_project_arguments('-D_WIN32_WINNT=0x0603', language: langs)
  elif get_option('windows-version') == 'win10'
    add_project_arguments('-DWINVER=0x0A00', language: langs)
    add_project_arguments('-D_WIN32_WINNT=0x0A00', language: langs)
  else
    error('Version of targetted Windows incorrect')
  endif
  add_project_arguments('-D__USE_MINGW_ANSI_STDIO', language: langs)
  add_global_arguments('-define:WIN32', language: 'cs')
endif

if sys_sun
# for getpwuid_r()
  add_global_arguments('-D_POSIX_PTHREAD_SEMANTICS', language: 'c')
endif

env = find_program('env', native: true)

config_h = configuration_data()
config_h.set_quoted('MODULE_ARCH', version_name)
config_h.set_quoted('PACKAGE', meson.project_name())
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('VERSION', meson.project_version())
config_h.set_quoted('LOCALE_DIR', join_paths([dir_prefix, 'share/locale']))
config_h.set_quoted('PACKAGE_URL', 'https://www.enlightenment.org')
config_h.set_quoted('PACKAGE_TARNAME', meson.project_name())
config_h.set_quoted('PACKAGE_BUGREPORT', 'enlightenment-devel@lists.sourceforge.net')
config_h.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + meson.project_version())
config_h.set_quoted('PACKAGE_NAME', meson.project_name())
config_h.set_quoted('PACKAGE_BIN_DIR', dir_bin)
config_h.set_quoted('PACKAGE_LIB_DIR', dir_lib)
config_h.set_quoted('PACKAGE_SRC_DIR', meson.source_root())
config_h.set_quoted('PACKAGE_BUILD_DIR', meson.current_build_dir())
config_h.set_quoted('PACKAGE_SYSCONF_DIR', dir_sysconf)
config_h.set_quoted('BINDIR', dir_bin)
config_h.set10('EFL_HAVE_THREADS', true)
config_h.set10('SLOPPY_SPEC', true)

## have to get compiler again for this to work
code = '''#define _GNU_SOURCE 1
#include <unistd.h>
#include <stdio.h>

extern char **environ;

void func(void) { printf("%p\n", environ); }
'''
if cc.compiles(code, args : '-lc', name : 'environ check')
  config_h.set10('HAVE_ENVIRON', true)
endif

cpu_sse3 = false
cpu_neon = false
cpu_neon_intrinsics = false
native_arch_opt_c_args = [ ]

if host_machine.endian() == 'big'
  config_h.set10('WORDS_BIGENDIAN', true)
endif

if get_option('native-arch-optimization')
  if host_machine.cpu_family() == 'x86' or host_machine.cpu_family() == 'x86_64'
    cpu_sse3 = true
    config_h.set10('BUILD_MMX', true)
    config_h.set10('BUILD_SSE3', true)
    native_arch_opt_c_args = [ '-msse3' ]
    message('x86 build - MMX + SSE3 enabled')
  elif host_machine.cpu_family() == 'arm'
    cpu_neon = true
    config_h.set10('BUILD_NEON', true)
    add_project_arguments('-mfpu=neon', language: 'c')
    add_project_arguments('-ftree-vectorize', language: 'c')
    message('ARM build - NEON enabled')
  elif host_machine.cpu_family() == 'aarch64'
    cpu_neon = true
    cpu_neon_intrinsics = true
    config_h.set10('BUILD_NEON', true)
    config_h.set10('BUILD_NEON_INTRINSICS', true)
    add_project_arguments('-ftree-vectorize', language: 'c')
    native_arch_opt_c_args = [ '-ftree-vectorize' ]
    message('ARM64 build - NEON + intrinsics enabled')
  elif host_machine.cpu_family() == 'ppc' or host_machine.cpu_family() == 'ppc64'
    config_h.set10('BUILD_ALTIVEC', true)
    add_project_arguments('-ftree-vectorize', language: 'c')
    add_project_arguments('-maltivec', language: 'c')
    message('PPC/POWER build - ALTIVEC enabled')
  endif
endif

config_dir = [include_directories('.')]
eolian_include_directories = []

if sys_linux or sys_bsd or sys_sun
  sys_lib_extension = 'so'
  sys_exe_extension = ''
  sys_mod_extension = 'so'
elif sys_windows
  sys_lib_extension = 'dll'
  sys_exe_extension = 'exe'
  sys_mod_extension = 'dll'
elif sys_osx
  sys_lib_extension = 'dylib'
  sys_exe_extension = ''
  sys_mod_extension = 'so'
  config_h.set('environ', '(*_NSGetEnviron())')
else
  error('System ' + host_machine.system() + ' not known')
endif

if sys_linux or sys_bsd or sys_sun
  config_h.set('_STAT_VER_LINUX', '1')
endif

if host_os == 'freebsd' or host_os == 'dragonfly'
  # This is necessary. We MUST use OpenSSL in base as bringing in from ports
  # can cause major issues (2 copies of the same library).
  crypto = declare_dependency(link_args : [ '-lssl', '-lcrypto'])
  config_h.set('HAVE_OPENSSL', '1')
elif get_option('crypto') == 'openssl'
  crypto = dependency('openssl')
  config_h.set('HAVE_OPENSSL', '1')
endif

if get_option('crypto') != ''
  config_h.set('HAVE_CIPHER', '1')
  config_h.set('HAVE_SIGNATURE', '1')
endif

config_h.set_quoted('SHARED_LIB_SUFFIX', '.' + sys_lib_extension)
config_h.set_quoted('MOD_SUFFIX', '.' + sys_mod_extension)
if sys_exe_extension == ''
  config_h.set_quoted('EXE_SUFFIX', '')
else
  config_h.set_quoted('EXE_SUFFIX', '.' + sys_exe_extension)
endif

if get_option('tslib')
  config_h.set('HAVE_TSLIB', '1')
endif

subdir('header_checks')
subdir('po')

if get_option('wl')
  subdir(join_paths('src', 'wayland_protocol'))
endif

ecore_evas_wayland_engine_include_dir = []

evas_static_list = []

luaold_interpreters = [
  ['lua',     ['>=5.1.0', '<5.3.0']],
  ['lua51',   ['>=5.1.0', '<5.2.0']],
  ['lua-5.1', ['>=5.1.0', '<5.2.0']],
  ['lua5.1',  ['>=5.1.0', '<5.2.0']],
  ['lua52',   ['>=5.2.0', '<5.3.0']],
  ['lua-5.2', ['>=5.2.0', '<5.3.0']],
  ['lua5.2',  ['>=5.2.0', '<5.3.0']],
]

lua_pc_name = ''
have_elua = get_option('elua')

if get_option('lua-interpreter') == 'lua'
  config_h.set('ENABLE_LUA_OLD', '1')
  foreach l : luaold_interpreters
    lua = dependency(l[0], version: l[1], required:false)
    lua_pc_name = l[0]
    if lua.found()
      break
    endif
  endforeach
  if not lua.found()
    error('Lua not found')
  endif
  if have_elua
    message('Using experimental Elua with interpreter support...')
  endif
else
  lua = dependency(get_option('lua-interpreter'))
  lua_pc_name = 'luajit'
endif

if sys_osx and get_option('lua-interpreter') == 'luajit'
  # luajit on macro is broken, this means we need to generate our own
  # dependency with our arguments, a library later still needs to link to
  # luajit for the pagesize argument thingy
  lua = declare_dependency(
    include_directories: include_directories(lua.get_pkgconfig_variable('includedir')),
    link_args: ['-L' + lua.get_pkgconfig_variable('libdir'), '-l' + lua.get_pkgconfig_variable('libname')]
  )
endif

subprojects = [
  # name              |   option             | mod  | lib  | bin  | bench | tests | examples | true if build in efl-one | pkg-config options | name of static libs
  ['evil'             ,[]                    , false,  true, false,  false,  false,     false,  true, [], []],
  ['eina'             ,[]                    , false,  true,  true,   true,   true,      true,  true, [], []],
  ['eolian'           ,[]                    , false,  true,  true,  false,   true,     false, false, ['eina'], []],
  ['eo'               ,[]                    , false,  true, false,   true,   true,     false,  true, ['eina'], []],
  ['efl'              ,[]                    , false,  true, false,  false,   true,     false,  true, ['eo'], []],
  ['emile'            ,[]                    , false,  true, false,  false,   true,      true,  true, ['eina', 'efl'], ['lz4', 'rg_etc']],
  ['eet'              ,[]                    , false,  true,  true,  false,   true,      true,  true, ['eina', 'emile', 'efl'], []],
  ['ecore'            ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina', 'eo', 'efl'], ['buildsystem']],
  ['eldbus'           ,[]                    , false,  true,  true,  false,   true,      true,  true, ['eina', 'eo', 'efl'], []],
  ['ecore'            ,[]                    ,  true, false, false,  false,   true,      true,  true, ['eina', 'eo', 'efl'], []], #ecores modules depend on eldbus
  ['ecore_audio'      ,['audio']             , false,  true, false,  false,  false,     false,  true, ['eina', 'eo'], []],
  ['ecore_avahi'      ,['avahi']             , false,  true, false,  false,  false,      true, false, ['eina', 'ecore'], []],
  ['ecore_con'        ,[]                    , false,  true,  true,  false,   true,     false,  true, ['eina', 'eo', 'efl', 'ecore'], ['http-parser']],
  ['ecore_file'       ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['eeze'             ,['eeze']              ,  true,  true,  true,  false,   true,     false,  true, ['eina', 'efl'], []],
  ['ecore_input'      ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina', 'eo'], []],
  ['ecore_x'          ,['x11']               , false,  true, false,  false,  false,     false,  true, ['eina', 'efl'], []],
  ['ecore_fb'         ,['fb']                , false,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['ecore_wl2'        ,['wl']                ,  true,  true, false,  false,   true,     false,  true, ['eina'], ['libdrm']],
  ['ecore_sdl'        ,['sdl']               , false,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['ecore_win32'      ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['ecore_ipc'        ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['ecore_buffer'     ,['buffer']            ,  true,  true,  true,  false,  false,     false,  true, ['eina'], []],
  ['ector'            ,[]                    , false,  true, false,  false,   true,     false,  true, ['eina', 'efl'], ['draw', 'triangulator', 'freetype']],
  ['elput'            ,['input']             , false,  true, false,  false,   true,     false,  true, ['eina', 'eldbus'], []],
  ['ecore_drm2'       ,['drm']               , false,  true, false,  false,  false,     false,  true, ['ecore'], ['libdrm']],
  ['ecore_cocoa'      ,['cocoa']             , false,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['evas'             ,[]                    ,  true,  true, false,  false,   true,      true,  true, ['eina', 'efl', 'eo'], ['vg_common', 'libunibreak']],
  ['efreet'           ,[]                    , false,  true, false,  false,   true,     false,  true, ['eina', 'efl', 'eo'], []],
  ['ecore_input_evas' ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina', 'evas'], []],
  ['ecore_evas'       ,[]                    ,  true,  true,  true,  false,  false,     false,  true, ['evas', 'ector'], []],
  ['ecore_imf'        ,[]                    ,  true,  true, false,  false,  false,     false,  true, ['eina'], []],
  ['embryo'           ,[]                    , false,  true,  true,  false,  false,     false,  true, ['eina', 'efl', 'eo'], []],
  ['eio'              ,[]                    , false,  true, false,  false,   true,      true,  true, ['eina', 'eet'], []],
  ['efreet'           ,[]                    , false, false,  true,  false,  false,     false,  true, ['eina', 'efl', 'eo'], []],
  ['ecore_imf_evas'   ,[]                    , false,  true, false,  false,  false,     false,  true, ['eina', 'efl', 'eo'], []],
  ['ephysics'         ,['physics']           , false,  true, false,  false,  false,     false,  true, ['eina', 'efl', 'eo'], []],
  ['edje'             ,[]                    , false,  true,  true,  false,   true,      true,  true, ['evas', 'eo', 'efl', lua_pc_name], []],
  ['emotion'          ,[]                    ,  true,  true, false,  false,   true,      true,  true, ['eina', 'efl', 'eo'], []],
  ['ethumb'           ,[]                    ,  true,  true,  true,  false,  false,     false,  true, ['eina', 'efl', 'eo'], []],
  ['ethumb_client'    ,[]                    , false,  true,  true,  false,  false,      true,  true, ['eina', 'efl', 'eo', 'ethumb'], []],
  ['elementary'       ,[]                    ,  true,  true,  true,   true,   true,      true,  true, ['eina', 'efl', 'eo', 'eet', 'evas', 'ecore', 'ecore-evas', 'ecore-file', 'ecore-input', 'edje', 'ethumb-client', 'emotion', 'ecore-imf', 'ecore-con', 'eldbus', 'efreet', 'efreet-mime', 'efreet-trash', 'eio'], ['atspi']],
  ['efl_canvas_wl'    ,['wl']                , false,  true,  true,  false,  false,     false,  true, ['eina', 'efl', 'eo', 'evas', 'ecore'], []],
  ['elua'             ,['elua']              , false,  true,  true,  false,   true,     false, false, ['eina', lua_pc_name], []],
  ['ecore_wayland'    ,['wl-deprecated']     , false,  true, false,  false,  false,     false, false, ['eina'], []],
  ['ecore_drm'        ,['drm-deprecated']    , false,  true, false,  false,  false,     false, false, ['eina'], []],
  ['exactness'        ,[]                    , false,  false,  true, false,  false,     false, false, ['eina, evas, eet'], []],
]

# We generate Efl_Config.h and config.h later, they will be available here
config_dir += include_directories('.')

#we have to do that first, eina modules are required by eina
#the other modules require theire package
subdir(join_paths(local_module, 'eina'))

# List of dependency objects that might be disabled due to configurations
# If they are enabled, the object gets overwritten by the library file.
ecore_audio = declare_dependency()

test_dirs = []
example_dirs = []
efl_one_parts = []
efl_one_deps = []
efl_one_eo_deps = []
efl_one_include_dirs = []
efl_one_sub_dirs = []
tmp_empty = declare_dependency()

foreach package : subprojects
  package_name = package[0]
  package_version_name = '-'.join(package_name.split('_')) + '-' + version_major
  automatic_pkgfile = true
  if package[1].length() == 0 or get_option(package[1][0])
    config_h.set('HAVE_' + package_name.to_upper().underscorify(), '1')

    dir_package_include = join_paths(dir_include, package_version_name)
    dir_package_modules = join_paths(dir_lib, package_name, 'modules')

    # ensure that we really dont copy the eo file targets from a previous
    # library Those are the variables that can be used to reflect the libraries
    # speical handlings -> at the end is used to indicate where to find this
    # variable outside of this for loop

    # public eo and eot files - which probebly have to be used later for bindings
    pub_eo_files = []           # -> package_name + '_eo_files'
    pub_eo_types_files = []     # -> package_name + '_eot_files'
    # All subdirs where eo files that are listed in the pub_* variables can be
    # found
    # For every element != '' a variable called package_name + '_' + subir + '_eot_files' and package_name + '_' + subir + '_eo_files' must exist.
    package_eo_subdirs = ['']     # -> package_name + '_eo_subdirs'
    # All subdirs that should be included in order to include every requried header
    package_header_subdirs = [] # -> package_name + '_include_subdirs'
    # eo file targets, this list of targets can be used to ensure the files are created before accessed
    pub_eo_file_target = []
    # private eo files target - never use this :)
    priv_eo_file_target = []
    # use this variable to store custom variables in that should be placed in the .pc file
    package_pc_variables = []

    foreach static_lib : package[10]
      if get_variable(static_lib, tmp_empty) == tmp_empty
        subdir(join_paths('src', 'static_libs', static_lib))
      endif
    endforeach

    #package_c_args definition for lib and module
    package_c_args = [
      '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
      '-DNEED_RUN_IN_TREE=1',
      '-DEFL_BUILD=1',
    ]
    if package[3]
      subdir(join_paths(local_lib, package_name))
      set_variable(package_name + '_eo_files', pub_eo_files)
      set_variable(package_name + '_eot_files', pub_eo_types_files)
      set_variable(package_name + '_header_subdirs', package_header_subdirs)
      set_variable(package_name + '_eo_subdirs', package_eo_subdirs)
      if (package[8] and get_option('efl-one'))
        src = get_variable(package_name+'_src')
        external_deps = get_variable(package_name+'_ext_deps')
        efl_one_include_dirs += [include_directories('.'), include_directories(join_paths(local_lib, package_name))]

        foreach subdirs : package_eo_subdirs
          efl_one_include_dirs += include_directories(join_paths(local_lib, package_name))
        endforeach

        tmp = static_library('efl_one_part_'+package_name,
            src, pub_eo_file_target, priv_eo_file_target,
            include_directories: efl_one_include_dirs,
            dependencies: external_deps + efl_one_eo_deps,
            c_args : package_c_args,
        )
        # dependency for all the .eo file targets
        efl_one_eo_deps += declare_dependency(
          sources: pub_eo_file_target + priv_eo_file_target, #this here *needs* to be public and private, because our binaries and modules do depend on internal headers
        )
        efl_one_deps += external_deps
        efl_one_parts += tmp
      endif
    endif
    #special case for eolian, this is never efl-one, but will be required in the library
    if (package_name == 'eolian')
      package_c_args = [
        '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
        '-DNEED_RUN_IN_TREE=1',
      ]
      if (package[4])
         subdir(join_paths(local_bin, package_name))
      endif
    endif
    if package[6]
       test_dirs += [package_name]
    endif
    if package[7]
       example_dirs += [package_name]
    endif

    set_variable('build_' + package_name.underscorify(), true)

    # generate automatic pc files for libraries
    if automatic_pkgfile and package[3]
      tmp_lib = get_variable(package_name + '_lib')
      tmp_deps = get_variable(package_name + '_deps')
      tmp_pub_deps = get_variable(package_name + '_pub_deps')
      tmp_package_subdirs = []

      foreach subdir : package_header_subdirs
        tmp_package_subdirs += join_paths(package_version_name, subdir)
      endforeach
      if (package[8] and get_option('efl-one'))
        efl_one_sub_dirs += [package_version_name] + tmp_package_subdirs
      endif
      pkgconfig.generate(tmp_lib,
        name : '-'.join(package_name.split('_')),
        subdirs : [package_version_name] + tmp_package_subdirs,
        version : version_major + '.' + version_minor + '.' + version_micro,
        libraries : tmp_pub_deps,
        requires : package[9],
        variables : package_pc_variables
      )
      if package_name == 'ethumb_client'
        pkgconfig.generate(tmp_lib,
          name : package_name,
          subdirs : [package_version_name] + tmp_package_subdirs,
          version : version_major + '.' + version_minor + '.' + version_micro,
          libraries : tmp_pub_deps,
          requires : package[9],
        )
      endif
    endif
  else
    message(package_name+' disabled!')
  endif
endforeach

if (get_option('efl-one'))
  #building efl-one
  efl_one_lib = shared_library('efl-one',
    link_whole : efl_one_parts,
    install : true,
    version : meson.project_version(),
  )

  efl_one = declare_dependency(
    link_with: efl_one_lib,
    include_directories : efl_one_include_dirs,
    dependencies : [thread_dep, intl] + efl_one_eo_deps,
    version: meson.project_version()
  )

  pkgconfig.generate(
    name : 'efl-one',
    subdirs : efl_one_sub_dirs,
    description: 'Configureation for efl in one big fat .so',
    libraries : [efl_one] + eina_pc_deps, #eina is a special case here which drags in m dl & threads
  )

  #overwrite all the dependencies of subprojects with efl-one, in order to link the modules and binaries to the correct .so
  foreach package : subprojects
    package_name = package[0]
    if package[1].length() == 0 or get_option(package[1][0])
      if (package[3])
        if (package[8] and get_option('efl-one'))
          set_variable(package_name, efl_one)
        endif
      endif
    endif
  endforeach
endif

#delayed build of bin, modules, and benchmarks
foreach package : subprojects
  package_name = package[0]
  if package[1].length() == 0 or get_option(package[1][0])
    dir_package_include = join_paths(dir_include, package_version_name)
    dir_package_modules = join_paths(dir_lib, package_name, 'modules')
    package_c_args = [
      '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
      '-DNEED_RUN_IN_TREE=1',
      '-DEFL_BUILD=1',
    ]
    if (package[2])
       subdir(join_paths(local_module, package_name))
    endif
    package_c_args = [
      '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
      '-DNEED_RUN_IN_TREE=1',
    ]
    if (package[4] and package_name != 'eolian')
       subdir(join_paths(local_bin, package_name))
    endif
    if (package[5])
       subdir(join_paths(local_benchmark, package_name))
    endif
  endif
endforeach

#build this later, as the debug services are depending on ecore
subdir(join_paths('src', 'bin', 'efl'))

subdir(join_paths('src', 'generic', 'evas'))
subdir('cmakeconfig')
subdir(join_paths('src', 'bindings'))
subdir(join_paths('src', 'edje_external'))
subdir(join_paths('data'))

if get_option('build-tests')
  check = dependency('check')

  if (check.version() == '0.15.1')
    error('There is a bug in check@0.15.1 which does not allow efl to be compiled with it. Please downgrade / upgrade or disable tests')
  endif

  test_env = environment()
  test_env.set('EFL_RUN_IN_TREE', '1')

  if get_option('b_sanitize') == 'address'
    test_env.set('ASAN_OPTIONS', 'detect_leaks=0:detect_odr_violation=0')
  endif

  subdir(join_paths('src', 'tests'))
  foreach test : test_dirs
    package_c_args = [
      '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, test)+'"',
      '-DNEED_RUN_IN_TREE=1',
    ]
    subdir(join_paths(local_tests, test))
  endforeach
endif

if get_option('build-examples')
  foreach example : example_dirs
    package_c_args = [
      '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, example)+'"',
      '-DNEED_RUN_IN_TREE=1',
    ]
    subdir(join_paths(local_examples, example))
  endforeach
endif

subdir(join_paths(local_scripts))

meson.add_install_script('meson/meson_modules.sh', module_files)

foreach evas_loader_map_inst : evas_loader_map
  evas_loader_original = evas_loader_map_inst[0]
  evas_loader_link_types = evas_loader_map_inst[1]

  meson.add_install_script('meson/evas_loader_conf.sh', evas_loader_original, evas_loader_link_types)
endforeach

doxygen = find_program('doxygen', required : get_option('docs'))
if get_option('docs')
  if not doxygen.found()
    error('Need doxygen for docs')
  endif
  subdir('doc')
endif

#
# Configure files
#

efl_config_h = configuration_data()
efl_config_h.set('EFL_VERSION_MAJOR', version_major)
efl_config_h.set('EFL_VERSION_MINOR', version_minor)
efl_config_h.set('EFL_VERSION_MICRO', version_micro)
efl_config_h.set('EFL_BUILD_ID', get_option('build-id'))

# FIXME placeholder
efl_config_h.set('EFL_API_LEGACY_DEF', '#define EFL_API_LEGACY_DEF "FIXME NOT IMPLEMENTED"')

config_h.set('EFL_MAX_FD_SIZE', get_option('max-fd-size'))

configure_file(
  output: 'config.h',
  configuration: config_h
)

configure_file(
  input: join_paths('src','lib', 'efl', 'Efl_Config.h.in'),
  output: 'Efl_Config.h',
  install_dir : join_paths(dir_include,'eina-'+version_major),
  configuration: efl_config_h
)

configure_file(
  input: 'elm_intro.h.in',
  output: 'elm_intro.h',
  configuration: config_h,
  install_dir : join_paths(dir_include,'elementary-'+version_major)
)

subdir(join_paths('systemd-services'))
subdir(join_paths('dbus-services'))

#output the three new efl-* .pc files
efl_20_pc_files = {
  'efl-ui' : ['elementary'],
  'efl-core' : ['ecore', 'efl', 'emile'],
  'efl-net' : ['ecore', 'ecore-con', 'emile'],
}

foreach name, libraries : efl_20_pc_files
  pkgconfig.generate(
    name : '-'.join(name.split('_')),
    description: name+' configutation file',
    requires : libraries,
  )
endforeach
