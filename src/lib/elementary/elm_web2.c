#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_web.h"

#define MY_CLASS elm_web_class_get()

#define MY_CLASS_NAME "Elm_Web"
#define MY_CLASS_NAME_LEGACY "elm_web"

typedef struct _Elm_Web_Module Elm_Web_Module;
struct _Elm_Web_Module
{
   void (*unneed_web)(void);
   Eina_Bool (*need_web)(void);

   void (*window_features_ref)(Elm_Web_Window_Features *wf);
   void (*window_features_unref)(Elm_Web_Window_Features *wf);
   Eina_Bool (*window_features_property_get)(const Elm_Web_Window_Features *wf,
                                             Elm_Web_Window_Feature_Flag flag);
   void (*window_features_region_get)(const Elm_Web_Window_Features *wf,
                                      Evas_Coord *x,
                                      Evas_Coord *y,
                                      Evas_Coord *w,
                                      Evas_Coord *h);

   const Efl_Class *(*class_get)(void);

   Eina_Module *m;
};

static Elm_Web_Module ewm = {
  NULL,
  NULL,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL
};

static const char SIG_URI_CHANGED[] = "uri,changed"; // deprecated, use "url,changed" instead.
static const char SIG_URL_CHANGED[] = "url,changed";

static const Evas_Smart_Cb_Description _elm_web_smart_callbacks[] = {
   { SIG_URI_CHANGED, "s" },
   { SIG_URL_CHANGED, "s" },
   { SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   { SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   { NULL, NULL }
};

// FIXME: init/shutdown module below
void
_elm_unneed_web(void)
{
   if (!ewm.unneed_web) return ;
   ewm.unneed_web();
}

EAPI Eina_Bool
elm_need_web(void)
{
   if (!ewm.need_web) return EINA_FALSE;
   return ewm.need_web();
}

EAPI Evas_Object *
elm_web_add(Evas_Object *parent)
{
   if (!parent || !ewm.class_get) return NULL;

   return efl_add(ewm.class_get(), parent, efl_canvas_object_legacy_ctor(efl_added));
}

EAPI const Efl_Class *
elm_web_real_class_get(void)
{
   if (!ewm.class_get) return NULL;

   return ewm.class_get();
}

EOLIAN static Eo *
_elm_web_efl_object_constructor(Eo *obj, Elm_Web_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _elm_web_smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_HTML_CONTAINER);

   return obj;
}

EAPI Eina_Bool
elm_web_uri_set(Evas_Object *obj, const char *url)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return elm_obj_web_url_set(obj, url);
}

EAPI const char *
elm_web_uri_get(const Evas_Object *obj)
{
   return elm_obj_web_url_get((Eo *) obj);
}

// FIXME: override with module function
EAPI void
elm_web_window_features_ref(Elm_Web_Window_Features *wf)
{
   if (!ewm.window_features_ref) return ;
   ewm.window_features_ref(wf);
}

EAPI void
elm_web_window_features_unref(Elm_Web_Window_Features *wf)
{
   if (!ewm.window_features_unref) return ;
   ewm.window_features_unref(wf);
}

EAPI Eina_Bool
elm_web_window_features_property_get(const Elm_Web_Window_Features *wf,
                                     Elm_Web_Window_Feature_Flag flag)
{
   if (!ewm.window_features_property_get) return EINA_FALSE;
   return ewm.window_features_property_get(wf, flag);
}

EAPI void
elm_web_window_features_region_get(const Elm_Web_Window_Features *wf,
                                   Evas_Coord *x,
                                   Evas_Coord *y,
                                   Evas_Coord *w,
                                   Evas_Coord *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;

   if (!ewm.window_features_region_get) return;
   ewm.window_features_region_get(wf, x, y, w, h);
}

static void
_elm_web_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#if defined(_WIN32) || defined(__CYGWIN__)
# define EFL_SHARED_EXTENSION ".dll"
#else
# define EFL_SHARED_EXTENSION ".so"
#endif

Eina_Bool
_elm_web_init(const char *engine)
{
   char buf[PATH_MAX];

#ifdef NEED_RUN_IN_TREE
   if (getenv("ELM_RUN_IN_TREE"))
     snprintf(buf, sizeof(buf),
              ELM_TOP_BUILD_DIR"/src/modules/web/%s/.libs/module"EFL_SHARED_EXTENSION,
              engine);
   else
#endif
     snprintf(buf, sizeof(buf),
              "%s/elementary/modules/web/%s/%s/module"EFL_SHARED_EXTENSION,
              _elm_lib_dir, engine, MODULE_ARCH);

   if (ewm.m)
     {
        // Check if the module is already open
        if (!strcmp(buf, eina_module_file_get(ewm.m)))
          return EINA_TRUE;

        // We are leaking reference on purpose here, as we can't be sure that
        // the web engine is not leaking state around preventing a clean exit.
        // Only future elm_web object created from now will use the new engine.
        ewm.unneed_web = NULL;
        ewm.need_web = NULL;
        ewm.window_features_ref = NULL;
        ewm.window_features_unref = NULL;
        ewm.window_features_property_get = NULL;
        ewm.window_features_region_get = NULL;
        ewm.class_get = NULL;
     }

   ewm.m = eina_module_new(buf);
   if (!ewm.m) return EINA_FALSE;

   if (!eina_module_load(ewm.m))
     {
        eina_module_free(ewm.m);
        ewm.m = NULL;
        return EINA_FALSE;
     }

   ewm.unneed_web = eina_module_symbol_get(ewm.m, "ewm_unneed_web");
   ewm.need_web = eina_module_symbol_get(ewm.m, "ewm_need_web");
   ewm.window_features_ref = eina_module_symbol_get(ewm.m, "ewm_window_features_ref");
   ewm.window_features_unref = eina_module_symbol_get(ewm.m, "ewm_window_features_unref");
   ewm.window_features_property_get = eina_module_symbol_get(ewm.m, "ewm_window_features_property_get");
   ewm.window_features_region_get = eina_module_symbol_get(ewm.m, "ewm_window_features_region_get");
   ewm.class_get = eina_module_symbol_get(ewm.m, "ewm_class_get");

   // Only the class_get is mandatory
   if (!ewm.class_get) return EINA_FALSE;
   return EINA_TRUE;
}

#undef ELM_WEB_CLASS
#define ELM_WEB_CLASS elm_web_class_get()

#include "elm_web.eo.c"
