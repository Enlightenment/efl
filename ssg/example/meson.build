
demo_sources = [ 'main.cpp']

demo_dep = []
demo_dep+= dependency('elementary')


executable('ssg_demo',
           demo_sources,
           include_directories : inc,
           link_with : ssglib,
           dependencies : demo_dep)
