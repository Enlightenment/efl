struct _Evas_Object_Textblock2_Format
{
   double               halign;  /**< Horizontal alignment value. */
   double               valign;  /**< Vertical alignment value. */
   struct {
      Evas_Font_Description *fdesc;  /**< Pointer to font description. */
      const char       *source;  /**< Pointer to object from which to search for the font. */
      Evas_Font_Set    *font;  /**< Pointer to font set. */
      Evas_Font_Size    size;  /**< Size of the font. */
   } font;
   struct {
      struct {
	 unsigned char  r, g, b, a;
      } normal, underline, underline2, underline_dash, outline, shadow, glow, glow2, backing,
	strikethrough;
   } color;
   struct {
      int               l, r;
   } margin;  /**< Left and right margin width. */
   int                  ref;  /**< Value of the ref. */
   int                  tabstops;  /**< Value of the size of the tab character. */
   int                  linesize;  /**< Value of the size of the line of the text. */
   int                  linegap;  /**< Value to set the line gap in text. */
   int                  underline_dash_width;  /**< Valule to set the width of the underline dash. */
   int                  underline_dash_gap;  /**< Value to set the gap of the underline dash. */
   double               linerelsize;  /**< Value to set the size of line of text. */
   double               linerelgap;  /**< Value for setting line gap. */
   double               linefill;  /**< The value must be a percentage. */
   double               ellipsis;  /**< The value should be a number. Any value smaller than 0.0 or greater than 1.0 disables ellipsis. A value of 0 means ellipsizing the leftmost portion of the text first, 1 on the other hand the rightmost portion. */
   unsigned char        style;  /**< Value from Evas_Text_Style_Type enum. */
   Eina_Bool            wrap_word : 1;  /**< EINA_TRUE if only wraps lines at word boundaries, else EINA_FALSE. */
   Eina_Bool            wrap_char : 1;  /**< EINA_TRUE if wraps at any character, else EINA_FALSE. */
   Eina_Bool            wrap_mixed : 1;  /**< EINA_TRUE if wrap at words if possible, else EINA_FALSE. */
   Eina_Bool            underline : 1;  /**< EINA_TRUE if a single line under the text, else EINA_FALSE */
   Eina_Bool            underline2 : 1;  /**< EINA_TRUE if two lines under the text, else EINA_FALSE */
   Eina_Bool            underline_dash : 1;  /**< EINA_TRUE if a dashed line under the text, else EINA_FALSE */
   Eina_Bool            strikethrough : 1;  /**< EINA_TRUE if text should be stricked off, else EINA_FALSE */
   Eina_Bool            backing : 1;  /**< EINA_TRUE if enable background color, else EINA_FALSE */
   Eina_Bool            halign_auto : 1;  /**< EINA_TRUE if auto horizontal align, else EINA_FALSE */
};

/* The refcount for the formats. */
static int format_refcount = 0;
/* Holders for the stringshares */
static const char *fontstr = NULL;
static const char *font_fallbacksstr = NULL;
static const char *font_sizestr = NULL;
static const char *font_sourcestr = NULL;
static const char *font_weightstr = NULL;
static const char *font_stylestr = NULL;
static const char *font_widthstr = NULL;
static const char *langstr = NULL;
static const char *colorstr = NULL;
static const char *underline_colorstr = NULL;
static const char *underline2_colorstr = NULL;
static const char *underline_dash_colorstr = NULL;
static const char *outline_colorstr = NULL;
static const char *shadow_colorstr = NULL;
static const char *glow_colorstr = NULL;
static const char *glow2_colorstr = NULL;
static const char *backing_colorstr = NULL;
static const char *strikethrough_colorstr = NULL;
static const char *alignstr = NULL;
static const char *valignstr = NULL;
static const char *wrapstr = NULL;
static const char *left_marginstr = NULL;
static const char *right_marginstr = NULL;
static const char *underlinestr = NULL;
static const char *strikethroughstr = NULL;
static const char *backingstr = NULL;
static const char *stylestr = NULL;
static const char *tabstopsstr = NULL;
static const char *linesizestr = NULL;
static const char *linerelsizestr = NULL;
static const char *linegapstr = NULL;
static const char *linerelgapstr = NULL;
static const char *itemstr = NULL;
static const char *linefillstr = NULL;
static const char *ellipsisstr = NULL;
static const char *underline_dash_widthstr = NULL;
static const char *underline_dash_gapstr = NULL;

static Eina_Bool
_is_space(Eina_Unicode ch)
{
   return (ch == ' ');
}

/**
 * @page evas_textblock2_style_page Evas Textblock2 Style Options
 *
 * @brief This page describes how to style text in an Evas Text Block.
 */

/**
 * @internal
 * Init the format strings.
 */
static void
_format_command_init(void)
{
   if (format_refcount == 0)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @section evas_textblock2_style_index Index
         *
         * The following styling commands are accepted:
         * @li @ref evas_textblock2_style_font
         * @li @ref evas_textblock2_style_font_fallback
         * @li @ref evas_textblock2_style_font_size
         * @li @ref evas_textblock2_style_font_source
         * @li @ref evas_textblock2_style_font_weight
         * @li @ref evas_textblock2_style_font_style
         * @li @ref evas_textblock2_style_font_width
         * @li @ref evas_textblock2_style_lang
         * @li @ref evas_textblock2_style_color
         * @li @ref evas_textblock2_style_underline_color
         * @li @ref evas_textblock2_style_underline2_color
         * @li @ref evas_textblock2_style_underline_dash_color
         * @li @ref evas_textblock2_style_outline_color
         * @li @ref evas_textblock2_style_shadow_color
         * @li @ref evas_textblock2_style_glow_color
         * @li @ref evas_textblock2_style_glow2_color
         * @li @ref evas_textblock2_style_backing_color
         * @li @ref evas_textblock2_style_strikethrough_color
         * @li @ref evas_textblock2_style_align
         * @li @ref evas_textblock2_style_valign
         * @li @ref evas_textblock2_style_wrap
         * @li @ref evas_textblock2_style_left_margin
         * @li @ref evas_textblock2_style_right_margin
         * @li @ref evas_textblock2_style_underline
         * @li @ref evas_textblock2_style_strikethrough
         * @li @ref evas_textblock2_style_backing
         * @li @ref evas_textblock2_style_style
         * @li @ref evas_textblock2_style_tabstops
         * @li @ref evas_textblock2_style_linesize
         * @li @ref evas_textblock2_style_linerelsize
         * @li @ref evas_textblock2_style_linegap
         * @li @ref evas_textblock2_style_linerelgap
         * @li @ref evas_textblock2_style_item
         * @li @ref evas_textblock2_style_linefill
         * @li @ref evas_textblock2_style_ellipsis
         * @li @ref evas_textblock2_style_password
         * @li @ref evas_textblock2_style_underline_dash_width
         * @li @ref evas_textblock2_style_underline_dash_gap
         *
         * @section evas_textblock2_style_contents Contents
         */
        fontstr = eina_stringshare_add("font");
        font_fallbacksstr = eina_stringshare_add("font_fallbacks");
        font_sizestr = eina_stringshare_add("font_size");
        font_sourcestr = eina_stringshare_add("font_source");
        font_weightstr = eina_stringshare_add("font_weight");
        font_stylestr = eina_stringshare_add("font_style");
        font_widthstr = eina_stringshare_add("font_width");
        langstr = eina_stringshare_add("lang");
        colorstr = eina_stringshare_add("color");
        underline_colorstr = eina_stringshare_add("underline_color");
        underline2_colorstr = eina_stringshare_add("underline2_color");
        underline_dash_colorstr = eina_stringshare_add("underline_dash_color");
        outline_colorstr = eina_stringshare_add("outline_color");
        shadow_colorstr = eina_stringshare_add("shadow_color");
        glow_colorstr = eina_stringshare_add("glow_color");
        glow2_colorstr = eina_stringshare_add("glow2_color");
        backing_colorstr = eina_stringshare_add("backing_color");
        strikethrough_colorstr = eina_stringshare_add("strikethrough_color");
        alignstr = eina_stringshare_add("align");
        valignstr = eina_stringshare_add("valign");
        wrapstr = eina_stringshare_add("wrap");
        left_marginstr = eina_stringshare_add("left_margin");
        right_marginstr = eina_stringshare_add("right_margin");
        underlinestr = eina_stringshare_add("underline");
        strikethroughstr = eina_stringshare_add("strikethrough");
        backingstr = eina_stringshare_add("backing");
        stylestr = eina_stringshare_add("style");
        tabstopsstr = eina_stringshare_add("tabstops");
        linesizestr = eina_stringshare_add("linesize");
        linerelsizestr = eina_stringshare_add("linerelsize");
        linegapstr = eina_stringshare_add("linegap");
        linerelgapstr = eina_stringshare_add("linerelgap");
        itemstr = eina_stringshare_add("item");
        linefillstr = eina_stringshare_add("linefill");
        ellipsisstr = eina_stringshare_add("ellipsis");
        underline_dash_widthstr = eina_stringshare_add("underline_dash_width");
        underline_dash_gapstr = eina_stringshare_add("underline_dash_gap");
     }
   format_refcount++;
}

/**
 * @internal
 * Shutdown the format strings.
 */
static void
_format_command_shutdown(void)
{
   if (--format_refcount > 0) return;

   eina_stringshare_del(fontstr);
   eina_stringshare_del(font_fallbacksstr);
   eina_stringshare_del(font_sizestr);
   eina_stringshare_del(font_sourcestr);
   eina_stringshare_del(font_weightstr);
   eina_stringshare_del(font_stylestr);
   eina_stringshare_del(font_widthstr);
   eina_stringshare_del(langstr);
   eina_stringshare_del(colorstr);
   eina_stringshare_del(underline_colorstr);
   eina_stringshare_del(underline2_colorstr);
   eina_stringshare_del(underline_dash_colorstr);
   eina_stringshare_del(outline_colorstr);
   eina_stringshare_del(shadow_colorstr);
   eina_stringshare_del(glow_colorstr);
   eina_stringshare_del(glow2_colorstr);
   eina_stringshare_del(backing_colorstr);
   eina_stringshare_del(strikethrough_colorstr);
   eina_stringshare_del(alignstr);
   eina_stringshare_del(valignstr);
   eina_stringshare_del(wrapstr);
   eina_stringshare_del(left_marginstr);
   eina_stringshare_del(right_marginstr);
   eina_stringshare_del(underlinestr);
   eina_stringshare_del(strikethroughstr);
   eina_stringshare_del(backingstr);
   eina_stringshare_del(stylestr);
   eina_stringshare_del(tabstopsstr);
   eina_stringshare_del(linesizestr);
   eina_stringshare_del(linerelsizestr);
   eina_stringshare_del(linegapstr);
   eina_stringshare_del(linerelgapstr);
   eina_stringshare_del(itemstr);
   eina_stringshare_del(linefillstr);
   eina_stringshare_del(ellipsisstr);
   eina_stringshare_del(underline_dash_widthstr);
   eina_stringshare_del(underline_dash_gapstr);
}

/**
 * @internal
 * Copies str to dst while removing the \\ char, i.e unescape the escape sequences.
 *
 * @param[out] dst the destination string - Should not be NULL.
 * @param[in] src the source string - Should not be NULL.
 */
static int
_format_clean_param(Eina_Tmpstr *s)
{
   Eina_Tmpstr *ss;
   char *ds;
   int len = 0;

   ds = (char*) s;
   for (ss = s; *ss; ss++, ds++, len++)
     {
        if ((*ss == '\\') && *(ss + 1)) ss++;
        if (ds != ss) *ds = *ss;
     }
   *ds = 0;

   return len;
}

/**
 * @internal
 * Parses the cmd and parameter and adds the parsed format to fmt.
 *
 * @param obj the evas object - should not be NULL.
 * @param fmt The format to populate - should not be NULL.
 * @param[in] cmd the command to process, should be stringshared.
 * @param[in] param the parameter of the command.
 */
static void
_format_command(Evas_Object *eo_obj, Evas_Object_Textblock2_Format *fmt, const char *cmd, Eina_Tmpstr *param)
{
   int len;

   len = _format_clean_param(param);

   /* If we are changing the font, create the fdesc. */
   if ((cmd == font_weightstr) || (cmd == font_widthstr) ||
         (cmd == font_stylestr) || (cmd == langstr) ||
         (cmd == fontstr) || (cmd == font_fallbacksstr))
     {
        if (!fmt->font.fdesc)
          {
             fmt->font.fdesc = evas_font_desc_new();
          }
        else if (!fmt->font.fdesc->is_new)
          {
             Evas_Font_Description *old = fmt->font.fdesc;
             fmt->font.fdesc = evas_font_desc_dup(fmt->font.fdesc);
             if (old) evas_font_desc_unref(old);
          }
     }


   if (cmd == fontstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font Font
         *
         * This sets the name of the font to be used.
         * @code
         * font=<font name>
         * @endcode
         */
        evas_font_name_parse(fmt->font.fdesc, param);
     }
   else if (cmd == font_fallbacksstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font_fallback Font fallback
         *
         * This sets the name of the fallback font to be used. This font will
         * be used if the primary font is not available.
         * @code
         * font_fallbacks=<font name>
         * @endcode
         */
        eina_stringshare_replace(&(fmt->font.fdesc->fallbacks), param);
     }
   else if (cmd == font_sizestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font_size Font size
         *
         * This sets the the size of font in points to be used.
         * @code
         * font_size=<size>
         * @endcode
         */
        int v;

        v = atoi(param);
        if (v != fmt->font.size)
          {
             fmt->font.size = v;
          }
     }
   else if (cmd == font_sourcestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font_source Font source
         *
         * Specify an object from which to search for the font.
         * @code
         * font_source=<source>
         * @endcode
         */
        if ((!fmt->font.source) ||
              ((fmt->font.source) && (strcmp(fmt->font.source, param))))
          {
             eina_stringshare_replace(&(fmt->font.source), param);
          }
     }
   else if (cmd == font_weightstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font_weight Font weight
         *
         * Sets the weight of the font. The value must be one of:
         * @li "normal"
         * @li "thin"
         * @li "ultralight"
         * @li "light"
         * @li "book"
         * @li "medium"
         * @li "semibold"
         * @li "bold"
         * @li "ultrabold"
         * @li "black"
         * @li "extrablack"
         * @code
         * font_weight=<weight>
         * @endcode
         */
        fmt->font.fdesc->weight = evas_font_style_find(param,
                                                       param + len,
                                                       EVAS_FONT_STYLE_WEIGHT);
     }
   else if (cmd == font_stylestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font_style Font style
         *
         * Sets the style of the font. The value must be one of:
         * @li "normal"
         * @li "oblique"
         * @li "italic"
         * @code
         * font_style=<style>
         * @endcode
         */
        fmt->font.fdesc->slant = evas_font_style_find(param,
                                                      param + len,
                                                      EVAS_FONT_STYLE_SLANT);
     }
   else if (cmd == font_widthstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_font_width Font width
         *
         * Sets the width of the font. The value must be one of:
         * @li "normal"
         * @li "ultracondensed"
         * @li "extracondensed"
         * @li "condensed"
         * @li "semicondensed"
         * @li "semiexpanded"
         * @li "expanded"
         * @li "extraexpanded"
         * @li "ultraexpanded"
         * @code
         * font_width=<width>
         * @endcode
         */
        fmt->font.fdesc->width = evas_font_style_find(param,
                                                      param + len,
                                                      EVAS_FONT_STYLE_WIDTH);
     }
   else if (cmd == langstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_lang Language
         *
         * Sets the language of the text for FontConfig.
         * @code
         * lang=<language>
         * @endcode
         */
        eina_stringshare_replace(&(fmt->font.fdesc->lang), param);
     }
   else if (cmd == colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_color Color
      *
      * Sets the color of the text. The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.normal.r), &(fmt->color.normal.g),
           &(fmt->color.normal.b), &(fmt->color.normal.a));
   else if (cmd == underline_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_underline_color Underline Color
      *
      * Sets the color of the underline. The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * underline_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.underline.r), &(fmt->color.underline.g),
           &(fmt->color.underline.b), &(fmt->color.underline.a));
   else if (cmd == underline2_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_underline2_color Second Underline Color
      *
      * Sets the color of the second line of underline(when using underline
      * mode "double"). The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * underline2_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.underline2.r), &(fmt->color.underline2.g),
           &(fmt->color.underline2.b), &(fmt->color.underline2.a));
   else if (cmd == underline_dash_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_underline_dash_color Underline Dash Color
      *
      * Sets the color of dashed underline. The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * underline_dash_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.underline_dash.r), &(fmt->color.underline_dash.g),
           &(fmt->color.underline_dash.b), &(fmt->color.underline_dash.a));
   else if (cmd == outline_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_outline_color Outline Color
      *
      * Sets the color of the outline of the text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * outline_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.outline.r), &(fmt->color.outline.g),
           &(fmt->color.outline.b), &(fmt->color.outline.a));
   else if (cmd == shadow_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_shadow_color Shadow Color
      *
      * Sets the color of the shadow of the text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * shadow_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.shadow.r), &(fmt->color.shadow.g),
           &(fmt->color.shadow.b), &(fmt->color.shadow.a));
   else if (cmd == glow_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_glow_color First Glow Color
      *
      * Sets the first color of the glow of text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * glow_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.glow.r), &(fmt->color.glow.g),
           &(fmt->color.glow.b), &(fmt->color.glow.a));
   else if (cmd == glow2_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_glow2_color Second Glow Color
      *
      * Sets the second color of the glow of text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * glow2_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.glow2.r), &(fmt->color.glow2.g),
           &(fmt->color.glow2.b), &(fmt->color.glow2.a));
   else if (cmd == backing_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_backing_color Backing Color
      *
      * Sets a background color for text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * backing_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.backing.r), &(fmt->color.backing.g),
           &(fmt->color.backing.b), &(fmt->color.backing.a));
   else if (cmd == strikethrough_colorstr)
     /**
      * @page evas_textblock2_style_page Evas Textblock2 Style Options
      *
      * @subsection evas_textblock2_style_strikethrough_color Strikethrough Color
      *
      * Sets the color of text that is striked through. The following formats
      * are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * strikethrough_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.strikethrough.r), &(fmt->color.strikethrough.g),
           &(fmt->color.strikethrough.b), &(fmt->color.strikethrough.a));
   else if (cmd == alignstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_align Horizontal Align
         *
         * Sets the horizontal alignment of the text. The value can either be
         * a number, a percentage or one of several presets:
         * @li "auto" - Respects LTR/RTL settings
         * @li "center" - Centers the text in the line
         * @li "middle" - Alias for "center"
         * @li "left" - Puts the text at the left of the line
         * @li "right" - Puts the text at the right of the line
         * @li <number> - A number between 0.0 and 1.0 where 0.0 represents
         * "left" and 1.0 represents "right"
         * @li <number>% - A percentage between 0% and 100% where 0%
         * represents "left" and 100% represents "right"
         * @code
         * align=<value or preset>
         * @endcode
         */
        if (len == 4 && !strcmp(param, "auto"))
          {
             fmt->halign_auto = EINA_TRUE;
          }
        else
          {
             static const struct {
                const char *param;
                int len;
                double halign;
             } halign_named[] = {
               { "middle", 6, 0.5 },
               { "center", 6, 0.5 },
               { "left", 4, 0.0 },
               { "right", 5, 1.0 },
               { NULL, 0, 0.0 }
             };
             unsigned int i;

             for (i = 0; halign_named[i].param; i++)
               if (len == halign_named[i].len &&
                   !strcmp(param, halign_named[i].param))
                 {
                    fmt->halign = halign_named[i].halign;
                    break;
                 }

             if (halign_named[i].param == NULL)
               {
                  char *endptr = NULL;
                  double val = strtod(param, &endptr);
                  if (endptr)
                    {
                       while (*endptr && _is_space(*endptr))
                         endptr++;
                       if (*endptr == '%')
                         val /= 100.0;
                    }
                  fmt->halign = val;
                  if (fmt->halign < 0.0) fmt->halign = 0.0;
                  else if (fmt->halign > 1.0) fmt->halign = 1.0;
               }
             fmt->halign_auto = EINA_FALSE;
          }
     }
   else if (cmd == valignstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_valign Vertical Align
         *
         * Sets the vertical alignment of the text. The value can either be
         * a number or one of the following presets:
         * @li "top" - Puts text at the top of the line
         * @li "center" - Centers the text in the line
         * @li "middle" - Alias for "center"
         * @li "bottom" - Puts the text at the bottom of the line
         * @li "baseline" - Baseline
         * @li "base" - Alias for "baseline"
         * @li <number> - A number between 0.0 and 1.0 where 0.0 represents
         * "top" and 1.0 represents "bottom"
         * @li <number>% - A percentage between 0% and 100% where 0%
         * represents "top" and 100% represents "bottom"
         * @code
         * valign=<value or preset>
         * @endcode
         *
         * See explanation of baseline at:
         * https://en.wikipedia.org/wiki/Baseline_%28typography%29
         */
        static const struct {
           const char *param;
           int len;
           double valign;
        } valign_named[] = {
          { "top", 3, 0.0 },
          { "middle", 6, 0.5 },
          { "center", 6, 0.5 },
          { "bottom", 6, 1.0 },
          { "baseline", 8, -1.0 },
          { "base", 4, -1.0 },
          { NULL, 0, 0 }
        };
        unsigned int i;

        for (i = 0; valign_named[i].param; i++)
          if (len == valign_named[i].len &&
              !strcmp(valign_named[i].param, param))
            {
               fmt->valign = valign_named[i].valign;
               break;
            }

        if (valign_named[i].param == NULL)
          {
             char *endptr = NULL;
             double val = strtod(param, &endptr);
             if (endptr)
               {
                  while (*endptr && _is_space(*endptr))
                    endptr++;
                  if (*endptr == '%')
                    val /= 100.0;
               }
             fmt->valign = val;
             if (fmt->valign < 0.0) fmt->valign = 0.0;
             else if (fmt->valign > 1.0) fmt->valign = 1.0;
          }
     }
   else if (cmd == wrapstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_wrap Wrap
         *
         * Sets the wrap policy of the text. The value must be one of the
         * following:
         * @li "word" - Only wraps lines at word boundaries
         * @li "char" - Wraps at any character
         * @li "mixed" - Wrap at words if possible, if not at any character
         * @li "" - Don't wrap
         * @code
         * wrap=<value or preset>
         * @endcode
         */
        static const struct {
           const char *param;
           int len;
           Eina_Bool wrap_word;
           Eina_Bool wrap_char;
           Eina_Bool wrap_mixed;
        } wrap_named[] = {
          { "word", 4, 1, 0, 0 },
          { "char", 4, 0, 1, 0 },
          { "mixed", 5, 0, 0, 1 },
          { NULL, 0, 0, 0, 0 }
        };
        unsigned int i;

        fmt->wrap_word = fmt->wrap_mixed = fmt->wrap_char = 0;
        for (i = 0; wrap_named[i].param; i++)
          if (wrap_named[i].len == len &&
              !strcmp(wrap_named[i].param, param))
            {
               fmt->wrap_word = wrap_named[i].wrap_word;
               fmt->wrap_char = wrap_named[i].wrap_char;
               fmt->wrap_mixed = wrap_named[i].wrap_mixed;
               break;
            }
     }
   else if (cmd == left_marginstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_left_margin Left margin
         *
         * Sets the left margin of the text. The value can be a number, an
         * increment, decrement or "reset":
         * @li +<number> - Increments existing left margin by <number>
         * @li -<number> - Decrements existing left margin by <number>
         * @li <number> - Sets left margin to <number>
         * @li "reset" - Sets left margin to 0
         * @code
         * left_margin=<value or reset>
         * @endcode
         */
        if (len == 5 && !strcmp(param, "reset"))
          fmt->margin.l = 0;
        else
          {
             if (param[0] == '+')
               fmt->margin.l += atoi(&(param[1]));
             else if (param[0] == '-')
               fmt->margin.l -= atoi(&(param[1]));
             else
               fmt->margin.l = atoi(param);
             if (fmt->margin.l < 0) fmt->margin.l = 0;
          }
     }
   else if (cmd == right_marginstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_right_margin Right margin
         *
         * Sets the right margin of the text. The value can be a number, an
         * increment, decrement or "reset":
         * @li +<number> - Increments existing right margin by <number>
         * @li -<number> - Decrements existing right margin by <number>
         * @li <number> - Sets left margin to <number>
         * @li "reset" - Sets left margin to 0
         * @code
         * right_margin=<value or reset>
         * @endcode
         */
        if (len == 5 && !strcmp(param, "reset"))
          fmt->margin.r = 0;
        else
          {
             if (param[0] == '+')
               fmt->margin.r += atoi(&(param[1]));
             else if (param[0] == '-')
               fmt->margin.r -= atoi(&(param[1]));
             else
               fmt->margin.r = atoi(param);
             if (fmt->margin.r < 0) fmt->margin.r = 0;
          }
     }
   else if (cmd == underlinestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_underline Underline
         *
         * Sets if and how a text will be underlined. The value must be one of
         * the following:
         * @li "off" - No underlining
         * @li "single" - A single line under the text
         * @li "on" - Alias for "single"
         * @li "double" - Two lines under the text
         * @li "dashed" - A dashed line under the text
         * @code
         * underline=off/single/on/double/dashed
         * @endcode
         */
        static const struct {
           const char *param;
           int len;
           Eina_Bool underline;
           Eina_Bool underline2;
           Eina_Bool underline_dash;
        } underlines_named[] = {
          { "off", 3, 0, 0, 0 },
          { "on", 2, 1, 0, 0 },
          { "single", 6, 1, 0, 0 },
          { "double", 6, 1, 1, 0 },
          { "dashed", 6, 0, 0, 1 },
          { NULL, 0, 0, 0, 0 }
        };
        unsigned int i;

        fmt->underline = fmt->underline2 = fmt->underline_dash = 0;
        for (i = 0; underlines_named[i].param; ++i)
          if (underlines_named[i].len == len &&
              !strcmp(underlines_named[i].param, param))
            {
               fmt->underline = underlines_named[i].underline;
               fmt->underline2 = underlines_named[i].underline2;
               fmt->underline_dash = underlines_named[i].underline_dash;
               break;
            }
     }
   else if (cmd == strikethroughstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_strikethrough Strikethrough
         *
         * Sets if the text will be striked through. The value must be one of
         * the following:
         * @li "off" - No strikethrough
         * @li "on" - Strikethrough
         * @code
         * strikethrough=on/off
         * @endcode
         */
        if (len == 3 && !strcmp(param, "off"))
          fmt->strikethrough = 0;
        else if (len == 2 && !strcmp(param, "on"))
          fmt->strikethrough = 1;
     }
   else if (cmd == backingstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_backing Backing
         *
         * Sets if the text will have backing. The value must be one of
         * the following:
         * @li "off" - No backing
         * @li "on" - Backing
         * @code
         * backing=on/off
         * @endcode
         */
        if (len == 3 && !strcmp(param, "off"))
          fmt->backing = 0;
        else if (len == 2 && !strcmp(param, "on"))
          fmt->backing = 1;
     }
   else if (cmd == stylestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_style Style
         *
         * Sets the style of the text. The value must be a string composed of
         * two comma separated parts. The first part of the value sets the
         * appearance of the text, the second the position.
         *
         * The first part may be any of the following values:
         * @li "plain"
         * @li "off" - Alias for "plain"
         * @li "none" - Alias for "plain"
         * @li "shadow"
         * @li "outline"
         * @li "soft_outline"
         * @li "outline_shadow"
         * @li "outline_soft_shadow"
         * @li "glow"
         * @li "far_shadow"
         * @li "soft_shadow"
         * @li "far_soft_shadow"
         * The second part may be any of the following values:
         * @li "bottom_right"
         * @li "bottom"
         * @li "bottom_left"
         * @li "left"
         * @li "top_left"
         * @li "top"
         * @li "top_right"
         * @li "right"
         * @code
         * style=<appearance>,<position>
         * @endcode
         */
        const char *p;
        char *p1, *p2, *pp;

        p2 = alloca(len + 1);
        *p2 = 0;
        /* no comma */
        if (!strstr(param, ",")) p1 = (char*) param;
        else
          {
             p1 = alloca(len + 1);
             *p1 = 0;

             /* split string "str1,str2" into p1 and p2 (if we have more than
              * 1 str2 eg "str1,str2,str3,str4" then we don't care. p2 just
              * ends up being the last one as right now it's only valid to have
              * 1 comma and 2 strings */
             pp = p1;
             for (p = param; *p; p++)
               {
                  if (*p == ',')
                    {
                       *pp = 0;
                       pp = p2;
                       continue;
                    }
                  *pp = *p;
                  pp++;
               }
             *pp = 0;
          }
        if      (!strcmp(p1, "off"))                 fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(p1, "none"))                fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(p1, "plain"))               fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(p1, "shadow"))              fmt->style = EVAS_TEXT_STYLE_SHADOW;
        else if (!strcmp(p1, "outline"))             fmt->style = EVAS_TEXT_STYLE_OUTLINE;
        else if (!strcmp(p1, "soft_outline"))        fmt->style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
        else if (!strcmp(p1, "outline_shadow"))      fmt->style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
        else if (!strcmp(p1, "outline_soft_shadow")) fmt->style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
        else if (!strcmp(p1, "glow"))                fmt->style = EVAS_TEXT_STYLE_GLOW;
        else if (!strcmp(p1, "far_shadow"))          fmt->style = EVAS_TEXT_STYLE_FAR_SHADOW;
        else if (!strcmp(p1, "soft_shadow"))         fmt->style = EVAS_TEXT_STYLE_SOFT_SHADOW;
        else if (!strcmp(p1, "far_soft_shadow"))     fmt->style = EVAS_TEXT_STYLE_FAR_SOFT_SHADOW;
        else                                         fmt->style = EVAS_TEXT_STYLE_PLAIN;

        if (*p2)
          {
             if      (!strcmp(p2, "bottom_right")) EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT);
             else if (!strcmp(p2, "bottom"))       EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM);
             else if (!strcmp(p2, "bottom_left"))  EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT);
             else if (!strcmp(p2, "left"))         EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT);
             else if (!strcmp(p2, "top_left"))     EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT);
             else if (!strcmp(p2, "top"))          EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP);
             else if (!strcmp(p2, "top_right"))    EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT);
             else if (!strcmp(p2, "right"))        EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT);
             else                                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT);
          }
     }
   else if (cmd == tabstopsstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_tabstops Tabstops
         *
         * Sets the size of the tab character. The value must be a number
         * greater than one.
         * @code
         * tabstops=<number>
         * @endcode
         */
        fmt->tabstops = atoi(param);
        if (fmt->tabstops < 1) fmt->tabstops = 1;
     }
   else if (cmd == linesizestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_linesize Line size
         *
         * Sets the size of line of text. The value should be a number.
         * @warning Setting this value sets linerelsize to 0%!
         * @code
         * linesize=<number>
         * @endcode
         */
        fmt->linesize = atoi(param);
        fmt->linerelsize = 0.0;
     }
   else if (cmd == linerelsizestr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_linerelsize Relative line size
         *
         * Sets the relative size of line of text. The value must be a
         * percentage.
         * @warning Setting this value sets linesize to 0!
         * @code
         * linerelsize=<number>%
         * @endcode
         */
        char *endptr = NULL;
        double val = strtod(param, &endptr);
        if (endptr)
          {
             while (*endptr && _is_space(*endptr))
               endptr++;
             if (*endptr == '%')
               {
                  fmt->linerelsize = val / 100.0;
                  fmt->linesize = 0;
                  if (fmt->linerelsize < 0.0) fmt->linerelsize = 0.0;
               }
          }
     }
   else if (cmd == linegapstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_linegap Line gap
         *
         * Sets the size of the line gap in text. The value should be a
         * number.
         * @warning Setting this value sets linerelgap to 0%!
         * @code
         * linegap=<number>
         * @endcode
         */
        fmt->linegap = atoi(param);
        fmt->linerelgap = 0.0;
     }
   else if (cmd == linerelgapstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_linerelgap Relative line gap
         *
         * Sets the relative size of the line gap in text. The value must be
         * a percentage.
         * @warning Setting this value sets linegap to 0!
         * @code
         * linerelgap=<number>%
         * @endcode
         */
        char *endptr = NULL;
        double val = strtod(param, &endptr);
        if (endptr)
          {
             while (*endptr && _is_space(*endptr))
               endptr++;
             if (*endptr == '%')
               {
                  fmt->linerelgap = val / 100.0;
                  fmt->linegap = 0;
                  if (fmt->linerelgap < 0.0) fmt->linerelgap = 0.0;
               }
          }
     }
   else if (cmd == itemstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_item Item
         *
         * Not implemented! Does nothing!
         * @code
         * item=<anything>
         * @endcode
         */
        // itemstr == replacement object items in textblock2 - inline imges
        // for example
     }
   else if (cmd == linefillstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_linefill Line fill
         *
         * Sets the size of the line fill in text. The value must be a
         * percentage.
         * @code
         * linefill=<number>%
         * @endcode
         */
        char *endptr = NULL;
        double val = strtod(param, &endptr);
        if (endptr)
          {
             while (*endptr && _is_space(*endptr))
               endptr++;
             if (*endptr == '%')
               {
                  fmt->linefill = val / 100.0;
                  if (fmt->linefill < 0.0) fmt->linefill = 0.0;
               }
          }
     }
   else if (cmd == ellipsisstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_ellipsis Ellipsis
         *
         * Sets ellipsis mode. The value should be a number. Any value smaller
         * than 0.0 or greater than 1.0 disables ellipsis.
         * A value of 0 means ellipsizing the leftmost portion of the text
         * first, 1 on the other hand the rightmost portion.
         * @code
         * ellipsis=<number>
         * @endcode
         */
        char *endptr = NULL;
        fmt->ellipsis = strtod(param, &endptr);
        if ((fmt->ellipsis < 0.0) || (fmt->ellipsis > 1.0))
          fmt->ellipsis = -1.0;
        else
          {
             Evas_Textblock2_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
             o->have_ellipsis = 1;
          }
     }
   else if (cmd == underline_dash_widthstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_underline_dash_width Underline dash width
         *
         * Sets the width of the underline dash. The value should be a number.
         * @code
         * underline_dash_width=<number>
         * @endcode
         */
        fmt->underline_dash_width = atoi(param);
        if (fmt->underline_dash_width <= 0) fmt->underline_dash_width = 1;
     }
   else if (cmd == underline_dash_gapstr)
     {
        /**
         * @page evas_textblock2_style_page Evas Textblock2 Style Options
         *
         * @subsection evas_textblock2_style_underline_dash_gap Underline dash gap
         *
         * Sets the gap of the underline dash. The value should be a number.
         * @code
         * underline_dash_gap=<number>
         * @endcode
         */
        fmt->underline_dash_gap = atoi(param);
        if (fmt->underline_dash_gap <= 0) fmt->underline_dash_gap = 1;
     }
}

/**
 * @internal
 * Returns @c EINA_TRUE if the item is a format parameter, @c EINA_FALSE
 * otherwise.
 *
 * @param[in] item the item to check - Not NULL.
 */
static Eina_Bool
_format_is_param(const char *item)
{
   if (strchr(item, '=')) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * @internal
 * Parse the format item and populate key and val with the stringshares that
 * corrospond to the formats parsed.
 * It expects item to be of the structure:
 * "key=val"
 *
 * @param[in] item the item to parse - Not NULL.
 * @param[out] key where to store the key at - Not NULL.
 * @param[out] val where to store the value at - Not NULL.
 */
static void
_format_param_parse(const char *item, const char **key, Eina_Tmpstr **val)
{
   const char *start, *end;
   char *tmp, *s, *d;
   size_t len;

   start = strchr(item, '=');
   if (!start) return ;
   *key = eina_stringshare_add_length(item, start - item);
   start++; /* Advance after the '=' */
   /* If we can find a quote as the first non-space char,
    * our new delimiter is a quote, not a space. */
   while (*start == ' ')
      start++;

   if (*start == '\'')
     {
        start++;
        end = strchr(start, '\'');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, '\'');
     }
   else
     {
        end = strchr(start, ' ');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, ' ');
     }

   /* Null terminate before the spaces */
   if (end) len = end - start;
   else len = strlen(start);

   tmp = (char*) eina_tmpstr_add_length(start, len);
   if (!tmp) goto end;

   for (d = tmp, s = tmp; *s; s++)
     {
        if (*s != '\\')
          {
             *d = *s;
             d++;
          }
     }
   *d = '\0';

end:
   *val = tmp;
}

/**
 * @internal
 * This function parses the format passed in *s and advances s to point to the
 * next format item, while returning the current one as the return value.
 * @param s The current and returned position in the format string.
 * @return the current item parsed from the string.
 */
static const char *
_format_parse(const char **s)
{
   const char *p;
   const char *s1 = NULL, *s2 = NULL;
   Eina_Bool quote = EINA_FALSE;

   p = *s;
   if (*p == 0) return NULL;
   for (;;)
     {
        if (!s1)
          {
             if (*p != ' ') s1 = p;
             if (*p == 0) break;
          }
        else if (!s2)
          {
             if (*p == '\'')
               {
                  quote = !quote;
               }

             if ((p > *s) && (p[-1] != '\\') && (!quote))
               {
                  if (*p == ' ') s2 = p;
               }
             if (*p == 0) s2 = p;
          }
        p++;
        if (s1 && s2)
          {
             *s = s2;
             return s1;
          }
     }
   *s = p;
   return NULL;
}

/**
 * @internal
 * Parse the format str and populate fmt with the formats found.
 *
 * @param obj The evas object - Not NULL.
 * @param[out] fmt The format to populate - Not NULL.
 * @param[in] str the string to parse.- Not NULL.
 */
static void
_format_fill(Evas_Object *eo_obj, Evas_Object_Textblock2_Format *fmt, const char *str)
{
   const char *s;
   const char *item;

   s = str;

   /* get rid of any spaces at the start of the string */
   while (*s == ' ') s++;

   while ((item = _format_parse(&s)))
     {
        if (_format_is_param(item))
          {
             const char *key = NULL;
             Eina_Tmpstr *val = NULL;

             _format_param_parse(item, &key, &val);
             if ((key) && (val)) _format_command(eo_obj, fmt, key, val);
             eina_stringshare_del(key);
             eina_tmpstr_del(val);
          }
        else
          {
             /* immediate - not handled here */
          }
     }
}

/**
 * @internal
 * Duplicate a format and return the duplicate.
 *
 * @param obj The evas object - Not NULL.
 * @param[in] fmt The format to duplicate - Not NULL.
 * @return the copy of the format.
 */
static Evas_Object_Textblock2_Format *
_format_dup(Evas_Object *eo_obj, const Evas_Object_Textblock2_Format *fmt)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Object_Textblock2_Format *fmt2;

   fmt2 = calloc(1, sizeof(Evas_Object_Textblock2_Format));
   memcpy(fmt2, fmt, sizeof(Evas_Object_Textblock2_Format));
   fmt2->ref = 1;
   if (fmt->font.fdesc) fmt2->font.fdesc = evas_font_desc_ref(fmt->font.fdesc);

   if (fmt->font.source) fmt2->font.source = eina_stringshare_add(fmt->font.source);

   /* FIXME: just ref the font here... */
   fmt2->font.font = evas_font_load(obj->layer->evas->evas, fmt2->font.fdesc,
         fmt2->font.source, (int)(((double) fmt2->font.size) * obj->cur->scale));
   return fmt2;
}
