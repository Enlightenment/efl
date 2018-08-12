shared_module(module_name,
    module_name+'.c',
    dependencies: [eina, eeze, ecore],
    install: true,
    install_dir : join_paths(dir_package_modules, 'sensor', module_name, version_name),
)

module_files += join_paths(dir_package_modules, 'sensor', module_name, version_name, 'lib' + module_name + '.' + sys_lib_extension)
