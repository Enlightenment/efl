#fixed dependencies by efl
png = dependency('libpng')
tiff = dependency('libtiff-4')
giflib = cc.find_library('gif')

webp = dependency('libwebp', required: get_option('evas-loaders-disabler').contains('webp') == false)

#there are a few modules that should NEVER be build as a module but rather be build as static lib and linked in later
evas_static_list = []

subdir('image_loaders')
subdir('image_savers')
subdir('model_savers')
subdir('model_loaders')
subdir('vg_savers')
subdir('vg_loaders')
