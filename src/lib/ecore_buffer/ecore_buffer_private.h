#ifndef _ECORE_BUFFER_PRIVATE_H_
# define _ECORE_BUFFER_PRIVATE_H_

#ifdef ERR
#undef ERR
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define ERR(...)  EINA_LOG_DOM_ERR(_ecore_buffer_queue_log_dom, __VA_ARGS__)
#define DBG(...)  EINA_LOG_DOM_DBG(_ecore_buffer_queue_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_ecore_buffer_queue_log_dom, __VA_ARGS__)

#define NFREE(_func_, _data_)                   \
do                                              \
{                                               \
   if (_data_)                                  \
     {                                          \
        _func_(_data_);                         \
        _data_ = NULL;                          \
     }                                          \
} while(0)

#define CALLBACK_CALL(obj, cbname)           \
do {                                         \
     if (obj->cb.cbname)                     \
       obj->cb.cbname(obj, obj->cb.data);    \
} while(0)

extern int _ecore_buffer_queue_log_dom;

const char        *_ecore_buffer_engine_name_get(Ecore_Buffer *buf);
Ecore_Export_Type  _ecore_buffer_export(Ecore_Buffer *buf, int *id);
Ecore_Buffer      *_ecore_buffer_import(const char *engine, int width, int height, Ecore_Buffer_Format format, Ecore_Export_Type type, int export_id, unsigned int flags);
void               _ecore_buffer_user_data_set(Ecore_Buffer *buf, const void *key, const void *data);
void              *_ecore_buffer_user_data_get(Ecore_Buffer *buf, const void *key);

#endif /* _ECORE_BUFFER_PRIVATE_H_ */
