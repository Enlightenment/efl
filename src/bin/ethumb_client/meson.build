ethumbd = executable('ethumbd',
        'ethumbd.c',
        install: true,
        dependencies : [ecore, ethumb, ethumb_client, ecore, eldbus],
        include_directories : config_dir,
        c_args : package_c_args,
)

ethumbd_client = executable('ethumbd_client',
        'ethumbd_client.c',
        install: true,
        dependencies : [ecore, ethumb, ethumb_client, ecore],
        include_directories : config_dir,
        c_args : package_c_args,
)

ethumbd_slave = executable('ethumbd_slave',
        'ethumbd_slave.c',
        install: true,
        install_dir : join_paths(dir_lib, 'ethumb_client', 'utils', version_name),
        dependencies : [ecore, ethumb, ethumb_client, ecore],
        include_directories : config_dir,
        c_args : package_c_args,
)
