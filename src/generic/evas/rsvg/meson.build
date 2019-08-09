generic_src = files([
  'main.c'
])

rsvg = dependency('librsvg-2.0')

if rsvg.version() >= '2.36.0'
  config_h.set('HAVE_SVG_2_36', '1')
endif

generic_deps = [rsvg]
generic_support = ['svgz', 'svg.gz']
