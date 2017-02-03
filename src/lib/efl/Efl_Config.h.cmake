#ifndef EFL_CONFIG_H
#define EFL_CONFIG_H

/* Add -DEFL_API_OVERRIDE ot your CFLAGS to override the default
 * installed api set and then fter that -DEFL_NOLEGACY_API_SUPPORT
 * and/or -DEFL_EO_API_SUPPORT as desired
 */
#ifndef EFL_API_OVERRIDE

#cmakedefine EFL_API_LEGACY_DEF
#cmakedefine EFL_BETA_API_SUPPORT
/* You can't disable Eo API anymore as EFL use it everywhere internally
   and we don't want to expose it publicly just yet, so keep it hidden
   and require an explicit request to get it. */
/* #define EFL_EO_API_SUPPORT */
#endif

#define EFL_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define EFL_VERSION_MINOR @PROJECT_VERSION_MINOR@
#define EFL_BUILD_ID "@PROJECT_VERSION_TWEAK@"
#endif
