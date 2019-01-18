embryo_cc_src = [
  'embryo_cc_amx.h',
  'embryo_cc_osdefs.h',
  'embryo_cc_sc.h',
  'embryo_cc_sc1.c',
  'embryo_cc_sc2.c',
  'embryo_cc_sc3.c',
  'embryo_cc_sc4.c',
  'embryo_cc_sc5.c',
  'embryo_cc_sc6.c',
  'embryo_cc_sc7.c',
  'embryo_cc_scexpand.c',
  'embryo_cc_sclist.c',
  'embryo_cc_scvars.c',
  'embryo_cc_prefix.c',
  'embryo_cc_prefix.h'
]

embryo_cc = executable('embryo_cc',
        embryo_cc_src,
        dependencies: [embryo, m, buildsystem],
        install: true,
        c_args : package_c_args,
)
