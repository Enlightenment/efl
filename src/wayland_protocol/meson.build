wayland_scanner_bin = find_program('wayland-scanner')

#make it typo proof
_wayland_protocols = dependency('wayland-protocols')
wayland_client = dependency('wayland-client')

wl_protocol_local = [
  'efl-aux-hints.xml',
  'efl-hints.xml',
  'session-recovery.xml',
  'teamwork.xml',	
  'www.xml'
]

wl_unstable_protocol_sys = [
  join_paths('xdg-shell', 'xdg-shell-unstable-v6.xml'),
  join_paths('linux-dmabuf', 'linux-dmabuf-unstable-v1.xml'),
  join_paths('text-input', 'text-input-unstable-v1.xml'),
]

wl_stable_protocol_sys = [
  join_paths('xdg-shell', 'xdg-shell.xml'),
]

foreach sys_protocol : wl_unstable_protocol_sys
	wl_protocol_local += [
		join_paths(_wayland_protocols.get_pkgconfig_variable('pkgdatadir'), 'unstable',
			sys_protocol)]
endforeach

foreach sys_protocol : wl_stable_protocol_sys
  wl_protocol_local += [
    join_paths(_wayland_protocols.get_pkgconfig_variable('pkgdatadir'), 'stable',
      sys_protocol)]
endforeach

gen_obj_c = []
gen_obj_header = []

foreach item : wl_protocol_local
  gen_obj_c += custom_target('wl_code_'+item.underscorify(),
    input: item,
    output : ['@BASENAME@-protocol.c'],
    command: [wayland_scanner_bin, ['code','@INPUT@','@OUTPUT@']]
  )
  gen_obj_header += custom_target('wl_client_header_'+item.underscorify(),
    input: item,
    output : ['@BASENAME@-client-protocol.h'],
    command: [wayland_scanner_bin, ['client-header','@INPUT@','@OUTPUT@']]
  )
  gen_obj_header += custom_target('wl_server_header_'+item.underscorify(),
    input: item,
    output : ['@BASENAME@-server-protocol.h'],
    command: [wayland_scanner_bin, ['server-header','@INPUT@','@OUTPUT@']]
  )
endforeach


wayland_protocol_lib = static_library('wayland_protocol',
    gen_obj_c + gen_obj_header,
)

wayland_protocol = declare_dependency(
	include_directories: include_directories('.'),
	link_with: wayland_protocol_lib,
  sources: gen_obj_header,
  dependencies : [wayland_client]
)
