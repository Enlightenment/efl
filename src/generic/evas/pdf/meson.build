generic_src = files([
  'main.cpp'
])

generic_deps = [dependency('poppler-cpp')]
generic_support = ['ppt','pptx','odp','xls','xlsx','ods','doc','docx','odt','rtf']

install_data('evas_generic_pdf_loader.libreoffice',
  install_dir : join_paths(dir_lib, 'evas', 'utils')
)

generic_pdf_loader_types = ['ppt','pptx','odp','xls','xlsx','ods','doc','docx','odt','rtf']

meson.add_install_script( join_paths('..','..','..','..','meson','evas_loader_conf.sh'), join_paths(dir_lib, 'evas', 'utils', 'evas_generic_pdf_loader.libreoffice'), generic_pdf_loader_types)