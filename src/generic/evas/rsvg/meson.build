generic_src = files([
  'main.c'
])

rsvg = dependency('librsvg-2.0')

if rsvg.version() >= '2.36.0'
  config_h.set('HAVE_SVG_2_36', '1')
endif

# Needed for rsvg_handle_get_intrinsic_size_in_pixels
if rsvg.version() >= '2.51.0'
  config_h.set('HAVE_SVG_2_51', '1')
endif

generic_deps = [rsvg]
generic_support = ['svg', 'svgz', 'svg.gz']
