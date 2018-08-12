efreetd_src = [
  'efreetd.c',
  'efreetd.h',
  'efreetd_ipc.h',
  'efreetd_ipc.c',
  'efreetd_cache.h',
  'efreetd_cache.c'
]

config_h.set('STRICT_SPEC', '1')

efreetd = executable('efreetd',
        efreetd_src,
        dependencies: [efreet, ecore, ecore_ipc, ecore_file, eio],
        install: true,
        c_args : package_c_args,
)

efreet_desktop_cache_create = executable('efreet_desktop_cache_create',
        'efreet_desktop_cache_create.c',
        dependencies: [efreet, ecore, eet, ecore_file],
        install: true,
        c_args : package_c_args,
)

efreet_icon_cache_create = executable('efreet_icon_cache_create',
        'efreet_icon_cache_create.c',
        dependencies: [efreet, ecore, eet, ecore_file],
        install: true,
        c_args : package_c_args,
)

efreet_mime_cache_create = executable('efreet_mime_cache_create',
        'efreet_mime_cache_create.c',
        dependencies: [efreet, ecore, eet, ecore_file],
        install: true,
        c_args : package_c_args,
)
