custom_target('colors_default.ecl',
              input: 'default.src',
              output: 'default.pal',
              command: [eet_bin, '-e', '@OUTPUT@', 'palette', '@INPUT@', '1'],
              install: true,
              install_dir: join_paths(dir_data, 'elementary', 'colors'),
              )
custom_target('colors_light.ecl',
              input: 'light.src',
              output: 'light.pal',
              command: [eet_bin, '-e', '@OUTPUT@', 'palette', '@INPUT@', '1'],
              install: true,
              install_dir: join_paths(dir_data, 'elementary', 'colors'),
              )
