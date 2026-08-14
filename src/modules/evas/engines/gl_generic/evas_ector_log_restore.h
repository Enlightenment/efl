/*
 * Restore Evas log macros after ector_software_private.h clobbers them.
 * ector_private.h redefines ERR/WRN/etc. to use _ector_log_dom_global
 * which is not linked into the engine module.
 */
#undef ERR
#undef WRN
#undef INF
#undef DBG
#undef CRI
#define ERR(...)  EINA_LOG_ERR(__VA_ARGS__)
#define WRN(...)  EINA_LOG_WARN(__VA_ARGS__)
#define INF(...)  EINA_LOG_INFO(__VA_ARGS__)
#define DBG(...)  EINA_LOG_DBG(__VA_ARGS__)
#define CRI(...)  EINA_LOG_CRIT(__VA_ARGS__)
