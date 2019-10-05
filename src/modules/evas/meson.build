#fixed dependencies by efl
png = dependency('libpng')
tiff = dependency('libtiff-4', required: get_option('evas-loaders-disabler').contains('tiff') == false)
giflib = cc.find_library('gif')
json = dependency('rlottie', required: get_option('evas-loaders-disabler').contains('json') == false)
webp = dependency('libwebp', required: get_option('evas-loaders-disabler').contains('webp') == false)

evas_loader_saver_mod_bmp     = ['bmp',     'shared', []]
evas_loader_saver_mod_eet     = ['eet',     'static', [eet]]
evas_loader_saver_mod_generic = ['generic', 'shared', [rt]]
evas_loader_saver_mod_gif     = ['gif',     'shared', [giflib]]
evas_loader_saver_mod_ico     = ['ico',     'shared', []]
evas_loader_saver_mod_jpeg    = ['jpeg',    'static', [jpeg]]
evas_loader_saver_mod_pmaps   = ['pmaps',   'shared', []]
evas_loader_saver_mod_png     = ['png',     'static', [png]]
evas_loader_saver_mod_psd     = ['psd',     'shared', []]
evas_loader_saver_mod_tga     = ['tga',     'shared', []]
evas_loader_saver_mod_tgv     = ['tgv',     'shared', [rg_etc, lz4]]
evas_loader_saver_mod_tiff    = ['tiff',    'shared', [tiff]]
evas_loader_saver_mod_wbmp    = ['wbmp',    'shared', []]
evas_loader_saver_mod_webp    = ['webp',    'shared', [webp]]
evas_loader_saver_mod_xpm     = ['xpm',     'shared', []]

#there are a few modules that should NEVER be build as a module but rather be build as static lib and linked in later
evas_static_list = []

subdir('image_loaders')
subdir('image_savers')
subdir('model_savers')
subdir('model_loaders')
subdir('vg_savers')
subdir('vg_loaders')
