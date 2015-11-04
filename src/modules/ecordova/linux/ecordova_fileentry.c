#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_fileentry_private.h"
#include "ecordova_entry_private.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

/* logging support */
extern int _ecordova_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_ecordova_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_ecordova_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ecordova_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ecordova_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ecordova_log_dom, __VA_ARGS__)

#define MY_CLASS ECORDOVA_FILEENTRY_CLASS
#define MY_CLASS_NAME "Ecordova_FileEntry"

static void _file_notify(Eo *, void *);
static void _writer_create(Eo *, void *);

static Eo_Base *
_ecordova_fileentry_eo_base_constructor(Eo *obj, Ecordova_FileEntry_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   ecordova_entry_file_is_set(EINA_TRUE);
   return obj;
}

static void
_ecordova_fileentry_eo_base_destructor(Eo *obj,
                                       Ecordova_FileEntry_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_fileentry_writer_create(Eo *obj,
                                  Ecordova_FileEntry_Data *pd EINA_UNUSED)
{
   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(super);

   _metadata_get(super, _writer_create, _error_notify);
}

static void
_ecordova_fileentry_file(Eo *obj, Ecordova_FileEntry_Data *pd EINA_UNUSED)
{
   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(super);

   _metadata_get(super, _file_notify, _error_notify);
}

static Ecordova_File *
_create_file(Eo *obj, const Ecordova_Metadata *metadata)
{
   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(super, NULL);

   return eo_add(ECORDOVA_FILE_CLASS, NULL,
     ecordova_file_constructor(super->name,
                              super->native,
                              NULL, //< TODO: mime-type?
                              metadata->modification_date,
                              metadata->size));
}

static void
_file_notify(Eo *obj, void *data)
{
   const Ecordova_Metadata *metadata = data;

   Ecordova_File *file = _create_file(obj, metadata);

   eo_do(obj, eo_event_callback_call(ECORDOVA_FILEENTRY_EVENT_FILE, file));
   eo_unref(file);
}

static void
_writer_create(Eo *obj, void *data)
{
   const Ecordova_Metadata *metadata = data;
   Ecordova_File *file = _create_file(obj, metadata);
   Ecordova_FileWriter *writer = eo_add(ECORDOVA_FILEWRITER_CLASS, NULL,
     ecordova_filewriter_file_set(file));

   eo_do(obj,
         eo_event_callback_call(ECORDOVA_FILEENTRY_EVENT_CREATE_WRITER, writer));

   eo_unref(writer);
   eo_unref(file);
}

#undef EOAPI
#define EOAPI EAPI

#include "undefs.h"

#define ecordova_fileentry_class_get ecordova_fileentry_impl_class_get

#include "ecordova_fileentry.eo.c"
