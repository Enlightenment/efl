if get_option('nls')
  i18n = import('i18n')
  i18n.gettext('efl',
             args: [
                '--keyword=_',
                '--keyword=d_:1',
                '--keyword=P_:1,2',
                '--keyword=dP_:1,2',
                '--keyword=N_',
                '--keyword=NP_:1,2',
                '--from-code=UTF-8',
                '--foreign-user'
              ])
  config_h.set('HAVE_GETTEXT', '1')
  config_h.set('ENABLE_NLS', '1')
  intl = cc.find_library('intl', required : false)
  if cc.compiles('#include <libintl.h>', dependencies : intl) == false
    error('libintl.h not found')
  endif
else
  intl = declare_dependency()
endif
