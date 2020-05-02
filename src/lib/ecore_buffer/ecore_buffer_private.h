#ifndef _ECORE_BUFFER_PRIVATE_H_
# define _ECORE_BUFFER_PRIVATE_H_

# ifdef ERR
#  undef ERR
# endif
# ifdef WARN
#  undef WARN
# endif
# ifdef DBG
#  undef DBG
# endif

# define ERR(...)  EINA_LOG_DOM_ERR(_ecore_buffer_queue_log_dom, __VA_ARGS__)
# define DBG(...)  EINA_LOG_DOM_DBG(_ecore_buffer_queue_log_dom, __VA_ARGS__)
# define WARN(...) EINA_LOG_DOM_WARN(_ecore_buffer_queue_log_dom, __VA_ARGS__)

# define CALLBACK_CALL(obj, cbname)           \
   do {                                       \
      if (obj->cb.cbname)                     \
        obj->cb.cbname(obj, obj->cb.data);    \
   } while(0)

extern int _ecore_buffer_queue_log_dom;

const char        *_ecore_buffer_engine_name_get(Ecore_Buffer *buf);

/* NOTE: if Ecore_Export_Type as a return value is EXPORT_TYPE_FD,
 * then caller should close the fd after using it. */
Ecore_Export_Type  _ecore_buffer_export(Ecore_Buffer *buf, int *id);
Ecore_Buffer      *_ecore_buffer_import(const char *engine, int width, int height, Ecore_Buffer_Format format, Ecore_Export_Type type, int export_id, unsigned int flags);
#endif /* _ECORE_BUFFER_PRIVATE_H_ */
