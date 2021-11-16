pals = [
  'default',
  'light',
  'mauve-sunset',
  'candy-mint',
  'ebony-brass',
  'white-pill'
]

foreach p: pals
  custom_target('colors_' + p + '.ecl',
                input: p + '.src',
                output: p + '.pal',
                command: [eet_bin, '-e', '@OUTPUT@', 'palette', '@INPUT@', '1'],
                install: true,
                install_dir: join_paths(dir_data, 'elementary', 'colors'),
                )
endforeach
