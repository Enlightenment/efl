epp_src = [
  'cpplib.h',
  'cpphash.h',
  'cppalloc.c',
  'cpperror.c',
  'cppexp.c',
  'cpphash.c',
  'cpplib.c',
  'cppmain.c'
]

epp = executable('epp',
        epp_src,
        dependencies: eolian,
        install: true,
        install_dir : join_paths(dir_lib, 'edje', 'utils', version_name),
        c_args : [
          package_c_args,
          '-DHAVE_CONFIG_H',
          '-DHAVE_STRERROR',
          '-DFATAL_EXIT_CODE=1',
          '-DSUCCESS_EXIT_CODE=0',
          '-DGCC_INCLUDE_DIR="/usr/include"',
          '-DGPLUSPLUS_INCLUDE_DIR="/usr/include"',
          '-DTOOL_INCLUDE_DIR="/usr/bin"',
          '-DHOST_BITS_PER_LONG=32',
          '-DBITS_PER_UNIT=8',
          '-DHOST_BITS_PER_INT=32',
          '-DBITS_PER_WORD=16',
          '-DTARGET_BELL=7',
          '-DTARGET_BS=8',
          '-DTARGET_FF=12',
          '-DTARGET_NEWLINE=10',
          '-DTARGET_CR=13',
          '-DTARGET_TAB=9',
          '-DTARGET_VT=11'
        ],
)
