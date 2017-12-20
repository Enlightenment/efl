project('efl', ['c','cpp'], version: '1.20.99')

pkgconfig = import('pkgconfig')

version_arr = meson.project_version().split('.')

version_major = version_arr[0]
version_minor = version_arr[1]
version_micro = version_arr[2]

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

module_arch = '@0@-@1@-@2@'.format(host_os, host_machine.cpu_family(), meson.project_version())

#install paths
dir_prefix    = get_option('prefix')
dir_sysconf   = join_paths(dir_prefix, get_option('sysconfdir'))
dir_bin       = join_paths(dir_prefix, get_option('bindir'))
dir_data      = join_paths(dir_prefix, get_option('datadir'))
dir_include   = join_paths(dir_prefix, get_option('includedir'))
dir_lib       = join_paths(dir_prefix, get_option('libdir'))

#local paths
local_lib = join_paths('src', 'lib')
local_bin = join_paths('src', 'bin')
local_module = join_paths('src', 'modules')
local_tests = join_paths('src', 'tests')
local_benchmark = join_paths('src', 'benchmarks')
local_examples = join_paths('src', 'examples')

add_global_arguments('-DHAVE_CONFIG_H=1', language: 'c')
add_global_arguments('-D_GNU_SOURCE=1', language: 'c')
add_global_arguments('-DEFL_EO_API_SUPPORT=1', language: 'c')
add_global_arguments('-DEFL_BETA_API_SUPPORT=1', language: 'c')

config_h = configuration_data()
config_h.set_quoted('MODULE_ARCH', module_arch)
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
config_h.set_quoted('PACKAGE_BUILD_DIR', meson.current_source_dir())
config_h.set_quoted('PACKAGE_DATA_DIR', join_paths(dir_data, meson.project_name()))
config_h.set_quoted('PACKAGE_SYSCONF_DIR', dir_sysconf)
config_h.set_quoted('BINDIR', dir_bin)
config_h.set_quoted('DATADIR', dir_data)
config_h.set10('EFL_HAVE_THREADS', true)

config_dir = [include_directories('.')]

subdir('header_checks')

subdir(join_paths(local_lib, 'efl'))

subdir(join_paths(local_module, 'eina'))
subdir(join_paths(local_lib, 'eina'))
subdir(join_paths(local_bin, 'eina'))
subdir(join_paths(local_benchmark, 'eina'))

if get_option('build-tests')
  subdir(join_paths(local_tests))
endif

if get_option('build-examples')
  subdir(join_paths(local_examples))
endif

configure_file(output: 'config.h', install: false, configuration: config_h)
