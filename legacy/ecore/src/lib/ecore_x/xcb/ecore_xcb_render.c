#include "ecore_xcb_private.h"
#include <ctype.h> // for isupper/tolower
#ifdef ECORE_XCB_RENDER
# include <xcb/render.h>
# include <xcb/xcb_renderutil.h>
#endif

/* local function prototypes */
static Eina_Bool _ecore_xcb_render_parse_boolean(char *v);

/* local variables */
static Eina_Bool _render_avail = EINA_FALSE;
static Eina_Bool _render_argb = EINA_FALSE;
static Eina_Bool _render_anim = EINA_FALSE;

void 
_ecore_xcb_render_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_RENDER
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_render_id);
#endif
}

void 
_ecore_xcb_render_finalize(void) 
{
#ifdef ECORE_XCB_RENDER
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_RENDER
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_render_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_render_query_version_cookie_t cookie;
        xcb_render_query_version_reply_t *reply;

        cookie = 
          xcb_render_query_version_unchecked(_ecore_xcb_conn, 
                                             XCB_RENDER_MAJOR_VERSION, 
                                             XCB_RENDER_MINOR_VERSION);
        reply = xcb_render_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
//        if ((reply->major_version >= XCB_RENDER_MAJOR_VERSION) && 
             if (reply->minor_version >= XCB_RENDER_MINOR_VERSION) 
               {
                  char *v = NULL;

                  _render_avail = EINA_TRUE;
                  _ecore_xcb_xdefaults_init();
                  if ((reply->major_version > 0) || (reply->minor_version >= 5)) 
                    {
                       _render_argb = EINA_TRUE;
                       v = getenv("XCURSOR_CORE");
                       if (!v) 
                         v = _ecore_xcb_xdefaults_string_get("Xcursor", "core");
                       if ((v) && (_ecore_xcb_render_parse_boolean(v)))
                         _render_argb = EINA_FALSE;
                    }
                  if ((_render_argb) && 
                      ((reply->major_version > 0) || (reply->minor_version >= 8)))
                    {
                       _render_anim = EINA_TRUE;
                       v = getenv("XCURSOR_ANIM");
                       if (!v) 
                         v = _ecore_xcb_xdefaults_string_get("Xcursor", "anim");
                       if ((v) && (_ecore_xcb_render_parse_boolean(v)))
                         _render_anim = EINA_FALSE;
                    }
                  _ecore_xcb_xdefaults_shutdown();
               }
          }
        free(reply);
     }
#endif
}

Eina_Bool 
_ecore_xcb_render_avail_get(void) 
{
   return _render_avail;
}

Eina_Bool 
_ecore_xcb_render_argb_get(void) 
{
   return _render_argb;
}

Eina_Bool 
_ecore_xcb_render_anim_get(void) 
{
   return _render_anim;
}

Eina_Bool 
_ecore_xcb_render_visual_supports_alpha(Ecore_X_Visual visual) 
{
   Eina_Bool ret = EINA_FALSE;
#ifdef ECORE_XCB_RENDER
   const xcb_render_query_pict_formats_reply_t *reply;
   xcb_render_pictvisual_t *vis;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!visual) return EINA_FALSE;
   if (!_render_avail) return EINA_FALSE;

#ifdef ECORE_XCB_RENDER
   reply = xcb_render_util_query_formats(_ecore_xcb_conn);
   if (!reply) return EINA_FALSE;

   vis = 
     xcb_render_util_find_visual_format(reply, 
                                        ((xcb_visualtype_t *)visual)->visual_id);
   if (vis) 
     {
        xcb_render_pictforminfo_t temp;
        xcb_render_pictforminfo_t *format;

        temp.id = vis->format;
        format = 
          xcb_render_util_find_format(reply, XCB_PICT_FORMAT_ID, &temp, 0);

        if ((format->type == XCB_RENDER_PICT_TYPE_DIRECT) && 
            (format->direct.alpha_mask))
          ret = EINA_TRUE;
     }

#endif

   return ret;
}

uint32_t 
_ecore_xcb_render_find_visual_id(int type, Eina_Bool check_alpha) 
{
#ifdef ECORE_XCB_RENDER
   const xcb_render_query_pict_formats_reply_t *reply;
   xcb_render_pictvisual_t *visual = NULL;
   xcb_render_pictscreen_iterator_t screens;
   xcb_render_pictdepth_iterator_t depths;
   xcb_render_pictvisual_iterator_t visuals;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_render_avail) return 0;

#ifdef ECORE_XCB_RENDER
   reply = xcb_render_util_query_formats(_ecore_xcb_conn);
   if (!reply) return 0;

   for (screens = xcb_render_query_pict_formats_screens_iterator(reply);
        screens.rem; xcb_render_pictscreen_next(&screens)) 
     {
        for (depths = xcb_render_pictscreen_depths_iterator(screens.data); 
             depths.rem; xcb_render_pictdepth_next(&depths)) 
          {
             for (visuals = xcb_render_pictdepth_visuals_iterator(depths.data);
                  visuals.rem; xcb_render_pictvisual_next(&visuals)) 
               {
                  xcb_render_pictforminfo_t temp;
                  xcb_render_pictforminfo_t *format;

                  visual = visuals.data;
                  temp.id = visual->format;

                  format = 
                    xcb_render_util_find_format(reply, XCB_PICT_FORMAT_ID, 
                                                &temp, 0);
                  if (!format) continue;
                  if (format->type == type) 
                    {
                       if (check_alpha) 
                         {
                            if (format->direct.alpha_mask)
                              return visual->visual;
                         }
                       else
                         return visual->visual;
                    }
               }
          }
     }
#endif

   return 0;
}

/* local function prototypes */
static Eina_Bool 
_ecore_xcb_render_parse_boolean(char *v) 
{
   char c;

   c = *v;
   if (isupper((int)c))
     c = tolower(c);
   if ((c == 't') || (c == 'y') || (c == '1'))
     return EINA_TRUE;
   if ((c == 'f') || (c == 'n') || (c == '0'))
     return EINA_FALSE;
   if (c == 'o') 
     {
        char d;

        d = v[1];
        if (isupper((int)d))
          d = tolower(d);
        if (d == 'n') return EINA_TRUE;
        if (d == 'f') return EINA_FALSE;
     }
   return EINA_FALSE;
}
