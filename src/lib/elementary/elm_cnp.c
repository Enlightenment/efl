#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Ecore_Evas.h>

#include "elm_priv.h"
#include "elm_entry_eo.h"

static inline Ecore_Evas_Selection_Buffer
_elm_sel_type_to_ee_type(Elm_Sel_Type type)
{
   if (type == ELM_SEL_TYPE_PRIMARY)
     return ECORE_EVAS_SELECTION_BUFFER_SELECTION_BUFFER;
   if (type == ELM_SEL_TYPE_XDND)
     return ECORE_EVAS_SELECTION_BUFFER_DRAG_AND_DROP_BUFFER;
   if (type == ELM_SEL_TYPE_CLIPBOARD)
     return ECORE_EVAS_SELECTION_BUFFER_COPY_AND_PASTE_BUFFER;
   return ECORE_EVAS_SELECTION_BUFFER_LAST;
}

static inline Eina_Array*
_elm_sel_format_to_mime_type(Elm_Sel_Format format)
{
   Eina_Array *ret = eina_array_new(10);
   if (format & ELM_SEL_FORMAT_TEXT)
     eina_array_push(ret, "text/plain;charset=utf-8");
   if (format & ELM_SEL_FORMAT_MARKUP)
      eina_array_push(ret, "application/x-elementary-markup");
   if (format & ELM_SEL_FORMAT_IMAGE)
     {
        eina_array_push(ret, "image/png");
        eina_array_push(ret, "image/jpeg");
        eina_array_push(ret, "image/x-ms-bmp");
        eina_array_push(ret, "image/gif");
        eina_array_push(ret, "image/tiff");
        eina_array_push(ret, "image/svg+xml");
        eina_array_push(ret, "image/x-xpixmap");
        eina_array_push(ret, "image/x-tga");
        eina_array_push(ret, "image/x-portable-pixmap");
     }
   if (format & ELM_SEL_FORMAT_VCARD)
     eina_array_push(ret, "text/vcard");
   if (format & ELM_SEL_FORMAT_HTML)
     eina_array_push(ret, "application/xhtml+xml");

   if (eina_array_count(ret) == 0)
     ERR("Specified mime type is not available");

   return ret;
}

typedef struct {
  const unsigned char image_sequence[16];
  const size_t image_sequence_len;
  const char *mimetype;
} Mimetype_Content_Matcher;

static const Mimetype_Content_Matcher matchers[] = {
  {{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}, 8, "image/png"},
  {{0xFF, 0xD8}, 2, "image/jpeg"},
  {{0x42, 0x4D}, 2, "image/x-ms-bmp"},
  {{0x47, 0x49, 0x46, 0x38, 0x37, 0x61}, 6, "image/gif"},
  {{0x47, 0x49, 0x46, 0x38, 0x39, 0x61}, 6, "image/gif"},
  {{0x49, 0x49, 0x2A, 00}, 4, "image/tiff"},
  {{0x49, 0x4D, 0x00, 0x2A}, 4, "image/tiff"},
  {{0},0, NULL}
};

static inline Eina_Array*
_elm_sel_from_content_to_mime_type(const void *buf, size_t buflen)
{
   Eina_Array *ret = eina_array_new(10);

   for (int i = 0; matchers[i].mimetype && eina_array_count(ret) == 0; ++i)
     {
        if (matchers[i].image_sequence_len >= buflen) continue;
        for (size_t j = 0; j < matchers[i].image_sequence_len; ++j)
          {
             if (((const unsigned  char*)buf)[j] == matchers[i].image_sequence[j])
               {
                  eina_array_push(ret, matchers[i].mimetype);
                  break;
               }
          }
     }

   if (eina_array_count(ret) != 1)
     ERR("Specified mime type is not available");

   return ret;
}

static inline Elm_Sel_Format
_mime_type_to_elm_sel_format(const char *mime_type)
{
   if (eina_streq(mime_type, "text/vcard"))
     return ELM_SEL_FORMAT_VCARD;
   else if (eina_streq(mime_type, "application/x-elementary-markup"))
     return ELM_SEL_FORMAT_MARKUP;
   else if (eina_streq(mime_type, "application/xhtml+xml"))
     return ELM_SEL_FORMAT_HTML;
   else if (!strncmp(mime_type, "text/", strlen("text/")))
     return ELM_SEL_FORMAT_TEXT;
   else if (!strncmp(mime_type, "image/", strlen("image/")))
     return ELM_SEL_FORMAT_IMAGE;

   return ELM_SEL_FORMAT_NONE;
}

static int
_default_seat(const Eo *obj)
{
   return evas_device_seat_id_get(evas_default_device_get(evas_object_evas_get(obj), EVAS_DEVICE_CLASS_SEAT));
}

EAPI Eina_Bool
elm_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type selection,
                                     Elm_Sel_Format format,
                                     const void *buf, size_t buflen)
{
   Eina_Content *content;
   Ecore_Evas *ee;
   const char *mime_type;
   Eina_Slice data;
   Eina_Array *tmp;
   unsigned char *mem_buf = NULL;

   if (format == ELM_SEL_FORMAT_TEXT && ((char*)buf)[buflen - 1] != '\0')
     {
        mem_buf = eina_memdup((unsigned char *)buf, buflen, EINA_TRUE);
        data.mem = mem_buf;
        data.len = buflen + 1;
     }
   else
     {
        data.mem = buf;
        data.len = buflen;
     }

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));

   if (format == ELM_SEL_FORMAT_IMAGE)
     {
        tmp = _elm_sel_from_content_to_mime_type(buf, buflen);
     }
   else
     {
        tmp = _elm_sel_format_to_mime_type(format);
     }


   if (eina_array_count(tmp) != 1)
     {
        ERR("You cannot specify more than one format when setting selection");
     }
   mime_type = eina_array_data_get(tmp, 0);
   eina_array_free(tmp);
   content = eina_content_new(data, mime_type);
   _register_selection_changed(obj);

   if (mem_buf != NULL)
     free(mem_buf);

   return ecore_evas_selection_set(ee, _default_seat(obj), _elm_sel_type_to_ee_type(selection), content);
}

EAPI Eina_Bool
elm_object_cnp_selection_clear(Evas_Object *obj,
                                              Elm_Sel_Type selection)
{
   Ecore_Evas *ee;

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   return ecore_evas_selection_set(ee, _default_seat(obj), _elm_sel_type_to_ee_type(selection), NULL);
}

EAPI Eina_Bool
elm_cnp_clipboard_selection_has_owner(Evas_Object *obj)
{
   Ecore_Evas *ee;

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   return ecore_evas_selection_exists(ee, _default_seat(obj), ECORE_EVAS_SELECTION_BUFFER_COPY_AND_PASTE_BUFFER);

}

typedef struct _Sel_Lost_Data Sel_Lost_Data;
struct _Sel_Lost_Data
{
   const Evas_Object *obj;
   Elm_Sel_Type type;
   void *udata;
   Elm_Selection_Loss_Cb loss_cb;
};

static void
_selection_changed_cb(void *data, const Efl_Event *ev)
{
   Sel_Lost_Data *ldata = data;
   Efl_Ui_Wm_Selection_Changed *changed = ev->info;

   if (changed->buffer == EFL_UI_CNP_BUFFER_SELECTION && ldata->type != ELM_SEL_TYPE_PRIMARY)
     return;

   if (changed->buffer == EFL_UI_CNP_BUFFER_COPY_AND_PASTE && ldata->type != ELM_SEL_TYPE_CLIPBOARD)
     return;

   if (ldata->obj == changed->caused_by)
     return;

   ldata->loss_cb(ldata->udata, ldata->type);
   efl_event_callback_del(ev->object, ev->desc, _selection_changed_cb, data);
   free(data);
}

EAPI void
elm_cnp_selection_loss_callback_set(Evas_Object *obj, Elm_Sel_Type type, Elm_Selection_Loss_Cb func, const void *data)
{
   Sel_Lost_Data *ldata = calloc(1, sizeof(Sel_Lost_Data));

   if (!ldata) return;
   ldata->obj = obj;
   ldata->type = type;
   ldata->udata = (void *)data;
   ldata->loss_cb = func;
   efl_event_callback_add(obj, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, _selection_changed_cb, ldata);
}

typedef struct {
   Elm_Drop_Cb data_cb;
   void *data;
   Elm_Sel_Format format;
} Callback_Storage;

static Eina_Value
_callback_storage_deliver(Eo *obj, void *data, const Eina_Value value)
{
   Callback_Storage *cb_storage = data;
   Eina_Content *content = eina_value_to_content(&value);
   Elm_Sel_Format format = _mime_type_to_elm_sel_format(eina_content_type_get(content));
   Eina_Slice cdata;

   cdata = eina_content_data_get(content);
   Elm_Selection_Data d = { 0 };
   d.data = eina_memdup((unsigned char*)cdata.bytes, cdata.len, EINA_FALSE);
   d.len = cdata.len;
   d.format = _mime_type_to_elm_sel_format(eina_content_type_get(content));

   if (cb_storage->data_cb)
     {
        cb_storage->data_cb(cb_storage->data, obj, &d);
     }
   else
     {
        EINA_SAFETY_ON_FALSE_GOTO(format == ELM_SEL_FORMAT_TEXT || format == ELM_SEL_FORMAT_MARKUP || format == ELM_SEL_FORMAT_HTML, end);

        _elm_entry_entry_paste(obj, (const char *) d.data);
     }

end:
   free(d.data);

   return EINA_VALUE_EMPTY;
}

static Eina_Value
_callback_storage_error(Eo *obj EINA_UNUSED, void *data EINA_UNUSED, Eina_Error error)
{
   ERR("Content cound not be received because of %s.", eina_error_msg_get(error));
   return EINA_VALUE_EMPTY;
}

static void
_callback_storage_free(Eo *obj EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   free(data);
}

EAPI Eina_Bool
elm_cnp_selection_get(const Evas_Object *obj, Elm_Sel_Type selection,
                                     Elm_Sel_Format format,
                                     Elm_Drop_Cb data_cb, void *udata)
{
   Ecore_Evas *ee;
   Eina_Array *mime_types;
   Eina_Future *future;
   Callback_Storage *storage;

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   mime_types = _elm_sel_format_to_mime_type(format);
   future = ecore_evas_selection_get(ee, _default_seat(obj), _elm_sel_type_to_ee_type(selection), eina_array_iterator_new(mime_types));
   storage = calloc(1,sizeof(Callback_Storage));
   storage->data_cb = data_cb;
   storage->data = udata;
   storage->format = format;

   efl_future_then(obj, future, _callback_storage_deliver, _callback_storage_error, _callback_storage_free, EINA_VALUE_TYPE_CONTENT, storage);

   return EINA_TRUE;
}
