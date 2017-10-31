#include "efl_selection_private.h"


//for dnd: - lam sao khoi tao atomlist
//- lam sao goi tat ca cac callback funcs khi drop
//- lam sao de tiet kiem memory

void
_set_atom_list(Efl_Selection_Manager_Atom *atomlist)
{
   atomlist[SELECTION_ATOM_TARGETS].name = "TARGETS";
   atomlist[SELECTION_ATOM_TARGETS].format = EFL_SELECTION_FORMAT_TARGETS;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_TARGETS].x_converter = _x11_targets_converter;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_TARGETS].wl_converter = _wl_targets_converter;
#endif
   atomlist[SELECTION_ATOM_ATOM].name = "ATOM"; // for opera browser
   atomlist[SELECTION_ATOM_ATOM].format = EFL_SELECTION_FORMAT_TARGETS;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_ATOM].x_converter = _x11_targets_converter;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_ATOM].wl_converter = _wl_targets_converter;
#endif

   atomlist[SELECTION_ATOM_XELM].name = "application/x-elementary-markup";
   atomlist[SELECTION_ATOM_XELM].format = EFL_SELECTION_FORMAT_MARKUP;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_XELM].x_converter = _x11_general_converter;
   atomlist[SELECTION_ATOM_XELM].x_data_preparer = _x11_data_preparer_markup;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_XELM].wl_converter = _wl_general_converter;
   atomlist[SELECTION_ATOM_XELM].wl_data_preparer = _wl_data_preparer_markup;
#endif

   atomlist[SELECTION_ATOM_TEXT_URILIST].name = "text/uri-list";
   atomlist[SELECTION_ATOM_TEXT_URILIST].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_TEXT_URILIST].x_converter = _x11_general_converter;
   atomlist[SELECTION_ATOM_TEXT_URILIST].x_data_preparer = _x11_data_preparer_uri;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_TEXT_URILIST].wl_converter = _wl_general_converter;
   atomlist[SELECTION_ATOM_TEXT_URILIST].wl_data_preparer = _wl_data_preparer_uri;
#endif

   atomlist[SELECTION_ATOM_TEXT_X_VCARD].name = "text/x-vcard";
   atomlist[SELECTION_ATOM_TEXT_X_VCARD].format = EFL_SELECTION_FORMAT_VCARD;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_TEXT_X_VCARD].x_converter = _x11_vcard_send;
   atomlist[SELECTION_ATOM_TEXT_X_VCARD].x_data_preparer = _x11_data_preparer_vcard;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_TEXT_X_VCARD].wl_data_preparer = _wl_data_preparer_vcard;
#endif

   atomlist[SELECTION_ATOM_IMAGE_PNG].name = "image/png";
   atomlist[SELECTION_ATOM_IMAGE_PNG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_PNG].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_PNG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_PNG].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_JPEG].name = "image/jpeg";
   atomlist[SELECTION_ATOM_IMAGE_JPEG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_JPEG].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_JPEG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_JPEG].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_BMP].name = "image/x-ms-bmp";
   atomlist[SELECTION_ATOM_IMAGE_BMP].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_BMP].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_BMP].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_BMP].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_GIF].name = "image/gif";
   atomlist[SELECTION_ATOM_IMAGE_GIF].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_GIF].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_GIF].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_GIF].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_TIFF].name = "image/tiff";
   atomlist[SELECTION_ATOM_IMAGE_TIFF].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_TIFF].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_TIFF].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_TIFF].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_SVG].name = "image/svg+xml";
   atomlist[SELECTION_ATOM_IMAGE_SVG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_SVG].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_SVG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_SVG].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_XPM].name = "image/x-xpixmap";
   atomlist[SELECTION_ATOM_IMAGE_XPM].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_XPM].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_XPM].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_XPM].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_TGA].name = "image/x-tga";
   atomlist[SELECTION_ATOM_IMAGE_TGA].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_TGA].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_TGA].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_TGA].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_IMAGE_PPM].name = "image/x-portable-pixmap";
   atomlist[SELECTION_ATOM_IMAGE_PPM].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_IMAGE_PPM].x_converter = _x11_image_converter;
   atomlist[SELECTION_ATOM_IMAGE_PPM].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_IMAGE_PPM].wl_data_preparer = _wl_data_preparer_image;
#endif

   atomlist[SELECTION_ATOM_UTF8STRING].name = "UTF8_STRING";
   atomlist[SELECTION_ATOM_UTF8STRING].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_UTF8STRING].x_converter = _x11_text_converter;
   atomlist[SELECTION_ATOM_UTF8STRING].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_UTF8STRING].wl_converter = _wl_text_converter;
   atomlist[SELECTION_ATOM_UTF8STRING].wl_data_preparer = _wl_data_preparer_text,
#endif

   atomlist[SELECTION_ATOM_STRING].name = "STRING";
   atomlist[SELECTION_ATOM_STRING].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_STRING].x_converter = _x11_text_converter;
   atomlist[SELECTION_ATOM_STRING].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_STRING].wl_converter = _wl_text_converter;
   atomlist[SELECTION_ATOM_STRING].wl_data_preparer = _wl_data_preparer_text;
#endif

   atomlist[SELECTION_ATOM_COMPOUND_TEXT].name = "COMPOUND_TEXT";
   atomlist[SELECTION_ATOM_COMPOUND_TEXT].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_COMPOUND_TEXT].x_converter = _x11_text_converter;
   atomlist[SELECTION_ATOM_COMPOUND_TEXT].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_COMPOUND_TEXT].wl_converter = _wl_text_converter;
   atomlist[SELECTION_ATOM_COMPOUND_TEXT].wl_data_preparer = _wl_data_preparer_text;
#endif

   atomlist[SELECTION_ATOM_TEXT].name = "TEXT";
   atomlist[SELECTION_ATOM_TEXT].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_TEXT].x_converter = _x11_text_converter;
   atomlist[SELECTION_ATOM_TEXT].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_TEXT].wl_converter = _wl_text_converter;
   atomlist[SELECTION_ATOM_TEXT].wl_data_preparer = _wl_data_preparer_text;
#endif

   atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].name = "text/plain;charset=utf-8";
   atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].x_converter = _x11_text_converter;
   atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].wl_converter = _wl_text_converter;
   atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].wl_data_preparer = _wl_data_preparer_text;
#endif

   atomlist[SELECTION_ATOM_TEXT_PLAIN].name = "text/plain";
   atomlist[SELECTION_ATOM_TEXT_PLAIN].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   atomlist[SELECTION_ATOM_TEXT_PLAIN].x_converter = _x11_text_converter;
   atomlist[SELECTION_ATOM_TEXT_PLAIN].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   atomlist[SELECTION_ATOM_TEXT_PLAIN].wl_converter = _wl_text_converter;
   atomlist[SELECTION_ATOM_TEXT_PLAIN].wl_data_preparer = _wl_data_preparer_text;
#endif


#ifdef HAVE_ELEMENTARY_X
   int i;
   for (i = 0; i < SELECTION_N_ATOMS; i++)
      {
         atomlist[i].x_atom = ecore_x_atom_get(atomlist[i].name);
         ecore_x_selection_converter_atom_add
            (atomlist[i].x_atom, atomlist[i].x_converter);
      }
#endif


}

void
_set_selection_list(X11_Cnp_Selection *sellist)
{
   sellist[EFL_SELECTION_TYPE_PRIMARY].debug = "Primary";
   sellist[EFL_SELECTION_TYPE_PRIMARY].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   sellist[EFL_SELECTION_TYPE_PRIMARY].set = ecore_x_selection_primary_set;
   sellist[EFL_SELECTION_TYPE_PRIMARY].clear = ecore_x_selection_primary_clear;
   sellist[EFL_SELECTION_TYPE_PRIMARY].request = ecore_x_selection_primary_request;

   sellist[EFL_SELECTION_TYPE_SECONDARY].debug = "Secondary";
   sellist[EFL_SELECTION_TYPE_SECONDARY].ecore_sel = ECORE_X_SELECTION_SECONDARY;
   sellist[EFL_SELECTION_TYPE_SECONDARY].set = ecore_x_selection_secondary_set;
   sellist[EFL_SELECTION_TYPE_SECONDARY].clear = ecore_x_selection_secondary_clear;
   sellist[EFL_SELECTION_TYPE_SECONDARY].request = ecore_x_selection_secondary_request;

   sellist[EFL_SELECTION_TYPE_DND].debug = "DnD";
   sellist[EFL_SELECTION_TYPE_DND].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   sellist[EFL_SELECTION_TYPE_DND].request = ecore_x_selection_xdnd_request;

   sellist[EFL_SELECTION_TYPE_CLIPBOARD].debug = "Clipboard";
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].ecore_sel = ECORE_X_SELECTION_CLIPBOARD;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].set = ecore_x_selection_clipboard_set;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].clear = ecore_x_selection_clipboard_clear;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].request = ecore_x_selection_clipboard_request;
}


