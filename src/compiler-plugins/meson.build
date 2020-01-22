cc = meson.get_compiler('c')
plugin_dev_path_result = run_command(cc.cmd_array(), '-print-file-name=plugin')
plugin_dev_path = plugin_dev_path_result.stdout().strip()
plugin_inc = include_directories([join_paths(plugin_dev_path, 'include')])

myplugin = shared_module('myplugin',
	'myplugin.cc',
	cpp_args: [ '-fno-rtti' ],
	include_directories: plugin_inc
)

myplugin_dep = custom_target('myplugin',
	input: myplugin,
        output: 'myplugin_dep.h',
        command: ['echo'],
        capture: true
)

compiler_plugin = declare_dependency()

test_1 = executable('test-1', 'test-1.c', myplugin_dep,
	c_args: [ '-fdump-passes', '-fplugin=' + myplugin.full_path() ],
	native: true,
)
test('test_1', test_1)
