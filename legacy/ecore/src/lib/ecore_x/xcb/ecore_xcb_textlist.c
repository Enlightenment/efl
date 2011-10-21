#include "ecore_xcb_private.h"
//#include "Ecore_X_Atoms.h"
#include <langinfo.h>
#ifdef HAVE_ICONV
# include <iconv.h>
#endif
#ifndef CODESET
# define CODESET "INVALID"
#endif

static int _ecore_xcb_textlist_get_buffer_size(Eina_Bool is_wide,
                                               void     *list,
                                               int       count);
static int   _ecore_xcb_textlist_get_wc_len(wchar_t *wstr);
static void *_ecore_xcb_textlist_alloc_list(Eina_Bool is_wide,
                                            int       count,
                                            int       nitems);
static void _ecore_xcb_textlist_copy_list(Eina_Bool is_wide,
                                          void     *text,
                                          char    **list,
                                          int       count);
static wchar_t *_ecore_xcb_textlist_copy_wchar(wchar_t *str1,
                                               wchar_t *str2);
static int      _ecore_xcb_textlist_len_wchar(wchar_t *str);

#ifdef HAVE_ICONV
Eina_Bool
_ecore_xcb_utf8_textlist_to_textproperty(char                   **list,
                                         int                      count,
                                         Ecore_Xcb_Encoding_Style style,
                                         Ecore_Xcb_Textproperty  *ret)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_textlist_to_textproperty("utf8string", list, count,
                                              style, ret);
}

#endif

Eina_Bool
_ecore_xcb_mb_textlist_to_textproperty(char                   **list,
                                       int                      count,
                                       Ecore_Xcb_Encoding_Style style,
                                       Ecore_Xcb_Textproperty  *ret)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_textlist_to_textproperty("multiByte", list, count,
                                              style, ret);
}

/* NB: This Function May Not Be Correct !!!
 * (as I do not know text conversion, locales, etc, etc very well)
 *
 * Portions were ripped from libX11 XTextListToTextProperty
 */
Eina_Bool
_ecore_xcb_textlist_to_textproperty(const char              *type,
                                    char                   **list,
                                    int                      count,
                                    Ecore_Xcb_Encoding_Style style,
                                    Ecore_Xcb_Textproperty  *ret)
{
   Eina_Bool is_wide = EINA_FALSE;
   Ecore_X_Atom encoding;
   int len = 0, nitems = 0, i = 0;
   size_t from_left = 0, to_left = 0;
   int unconv_num = 0, val = 0;
   char *buff, *to, *value, *from;
   const char *to_type, *from_type;
   char **mb = NULL;
   wchar_t **wc = NULL;
#ifdef HAVE_ICONV
   iconv_t conv;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!strcmp("wideChar", type)) is_wide = EINA_TRUE;
   len = _ecore_xcb_textlist_get_buffer_size(is_wide, list, count);
   if (!(buff = (char *)malloc(len * sizeof(char)))) return EINA_FALSE;
   from_type = nl_langinfo(CODESET);
   switch (style)
     {
      case XcbStringStyle:
      case XcbStdICCTextStyle:
        encoding = ECORE_X_ATOM_STRING;
        to_type = nl_langinfo(CODESET);
//        to_type = "string";
        break;

      case XcbUTF8StringStyle:
        encoding = ECORE_X_ATOM_UTF8_STRING;
        to_type = "UTF-8";
        break;

      case XcbCompoundTextStyle:
        encoding = ECORE_X_ATOM_COMPOUND_TEXT;
        to_type = nl_langinfo(CODESET);
//        to_type = "compoundText";
        break;

      case XcbTextStyle:
        encoding = ECORE_X_ATOM_TEXT;
        to_type = nl_langinfo(CODESET);
//        to_type = "multiByte";
        if (!is_wide)
          {
             nitems = 0;
             mb = (char **)list;
             to = buff;
             for (i = 0; ((i < count) && (len > 0)); i++)
               {
                  if (*mb) strcpy(to, *mb);
                  else *to = '\0';
                  from_left = (*mb ? strlen(*mb) : 0) + 1;
                  nitems += from_left;
                  to += from_left;
                  mb++;
               }
             unconv_num = 0;
             goto done;
          }
        break;

      default:
        free(buff);
        return EINA_FALSE;
        break;
     }

   if (count < 1)
     {
        nitems = 0;
        goto done;
     }

retry:
#ifdef HAVE_ICONV
   conv = iconv_open(to_type, from_type);
#endif

   if (is_wide)
     wc = (wchar_t **)list;
   else
     mb = (char **)list;

   to = buff;
   to_left = len;
   unconv_num = 0;
   for (i = 1; to_left > 0; i++)
     {
        if (is_wide)
          {
             from = (char *)*wc;
             from_left = _ecore_xcb_textlist_get_wc_len(*wc);
             wc++;
          }
        else
          {
             from = *mb;
             from_left = (*mb ? strlen(*mb) : 0);
             mb++;
          }

#ifdef HAVE_ICONV
        val = iconv(conv, &from, &from_left, &to, &to_left);
#endif
        if (val < 0) continue;
        if ((val > 0) && (style == XcbStdICCTextStyle) &&
            (encoding == ECORE_X_ATOM_STRING))
          {
#ifdef HAVE_ICONV
             iconv_close(conv);
#endif
             encoding = ECORE_X_ATOM_COMPOUND_TEXT;
             goto retry;
          }

        unconv_num += val;
        *to++ = '\0';
        to_left--;
        if (i >= count) break;
     }

#ifdef HAVE_ICONV
   iconv_close(conv);
#endif
   nitems = (to - buff);

done:
   if (nitems <= 0) nitems = 1;
   if (!(value = (char *)malloc(nitems * sizeof(char))))
     {
        free(buff);
        return EINA_FALSE;
     }
   if (nitems == 1)
     *value = 0;
   else
     memcpy(value, buff, nitems);
   nitems--;
   free(buff);

   ret->value = value;
   ret->encoding = encoding;
   ret->format = 8;
   ret->nitems = nitems;

   return EINA_TRUE;
}

#ifdef HAVE_ICONV
Eina_Bool
_ecore_xcb_utf8_textproperty_to_textlist(const Ecore_Xcb_Textproperty *text_prop,
                                         char                       ***list_ret,
                                         int                          *count_ret)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_textproperty_to_textlist(text_prop, "utf8String",
                                              list_ret, count_ret);
}

#endif

Eina_Bool
_ecore_xcb_mb_textproperty_to_textlist(const Ecore_Xcb_Textproperty *text_prop,
                                       char                       ***list_ret,
                                       int                          *count_ret)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_textproperty_to_textlist(text_prop, "multiByte",
                                              list_ret, count_ret);
}

Eina_Bool
_ecore_xcb_textproperty_to_textlist(const Ecore_Xcb_Textproperty *text_prop,
                                    const char                   *type,
                                    char                       ***list_ret,
                                    int                          *count_ret)
{
   Eina_Bool is_wide = EINA_FALSE;
   Eina_Bool do_strcpy = EINA_FALSE;
   const char *from_type;
   char *buff, *to, *from;
   char *lptr, *sptr;
   int nitems = 0, len = 0, num = 0, ret = 0;
   size_t from_left = 0, to_left = 0;
#ifdef HAVE_ICONV
   iconv_t conv = 0;
#endif

   *list_ret = NULL;
   *count_ret = 0;
   if (!strcmp("wideChar", type)) is_wide = EINA_TRUE;

   nitems = text_prop->nitems;
   if (nitems <= 0) return EINA_TRUE;

   if (text_prop->format != 8) return EINA_FALSE;

   from_type = nl_langinfo(CODESET);
   if (text_prop->encoding == ECORE_X_ATOM_UTF8_STRING)
     from_type = "UTF-8";

   if (is_wide)
     len = (text_prop->nitems + 1) * sizeof(wchar_t);
   else
     {
        if (!strcmp(type, "utf8String"))
          len = text_prop->nitems * 6 + 1;
        else
          len = text_prop->nitems * MB_CUR_MAX + 1;
     }

   buff = (char *)malloc(len * sizeof(char));
   if (!buff) return EINA_FALSE;

   to = buff;
   to_left = len;

   if (!strcmp(from_type, type))
     do_strcpy = EINA_TRUE;
   else
     {
#ifdef HAVE_ICONV
        conv = iconv_open(type, from_type);
#endif
        if (!conv)
          {
             free(buff);
             return EINA_FALSE;
          }
     }

   lptr = sptr = text_prop->value;
   num = *count_ret = 0;
   while (1)
     {
        if ((nitems == 0) || (*sptr == 0))
          {
             from = lptr;
             from_left = sptr - lptr;
             lptr = sptr;
             if (do_strcpy)
               {
                  int l = 0;

                  l = MIN(from_left, to_left);
                  strncpy(to, from, l);
                  from += len;
                  to += len;
                  from_left -= l;
                  to_left -= l;
                  ret = 0;
               }
             else
               ret = iconv(conv, &from, &from_left, &to, &to_left);

             if (ret < 0) continue;
             num += ret;
             (*count_ret)++;
             if (nitems == 0) break;
             lptr = ++sptr;
             if (is_wide)
               {
                  *((wchar_t *)to) = (wchar_t)0;
                  to += sizeof(wchar_t);
                  to_left -= sizeof(wchar_t);
               }
             else
               {
                  *((char *)to) = '\0';
                  to++;
                  to_left--;
               }
          }
        else
          sptr++;

        nitems--;
     }

#if HAVE_ICONV
   if (!do_strcpy) iconv_close(conv);
#endif

   if (is_wide)
     {
        *((wchar_t *)to) = (wchar_t)0;
        to_left -= sizeof(wchar_t);
     }
   else
     {
        *((char *)to) = '\0';
        to_left--;
     }

   *list_ret =
     _ecore_xcb_textlist_alloc_list(is_wide, *count_ret, (len - to_left));
   if (*list_ret)
     _ecore_xcb_textlist_copy_list(is_wide, buff, *list_ret, *count_ret);

   free(buff);

   return EINA_TRUE;
}

static int
_ecore_xcb_textlist_get_buffer_size(Eina_Bool is_wide,
                                    void     *list,
                                    int       count)
{
   int len = 0;
   char **mb;
   wchar_t **wc;

   if (!list) return 0;
   if (is_wide)
     {
        wc = (wchar_t **)list;
        for (; count-- > 0; wc++)
          if (*wc) len += _ecore_xcb_textlist_get_wc_len(*wc) + 1;
        len *= 5;
     }
   else
     {
        mb = (char **)list;
        for (; count-- > 0; mb++)
          if (*mb) len += strlen(*mb) + 1;
        len *= 3;
     }
   len = (len / 2048 + 1) * 2048;
   return len;
}

static int
_ecore_xcb_textlist_get_wc_len(wchar_t *wstr)
{
   wchar_t *ptr;

   ptr = wstr;
   while (*ptr)
     ptr++;

   return ptr - wstr;
}

static void *
_ecore_xcb_textlist_alloc_list(Eina_Bool is_wide,
                               int       count,
                               int       nitems)
{
   if (is_wide)
     {
        wchar_t **list;

        list = (wchar_t **)malloc(count * sizeof(wchar_t *));
        if (!list) return NULL;
        *list = (wchar_t *)malloc(nitems * sizeof(wchar_t));
        if (!*list)
          {
             free(list);
             return NULL;
          }
        return *list;
     }
   else
     {
        char **list;

        list = (char **)malloc(count * sizeof(char *));
        if (!list) return NULL;
        *list = (char *)malloc(nitems * sizeof(char));
        if (!*list)
          {
             free(list);
             return NULL;
          }
        return *list;
     }
}

static void
_ecore_xcb_textlist_copy_list(Eina_Bool is_wide,
                              void     *text,
                              char    **list,
                              int       count)
{
   int len = 0;

   if (is_wide)
     {
        wchar_t *txt, *str, **wlist;

        txt = (wchar_t *)text;
        wlist = (wchar_t **)list;
        for (str = *wlist; count > 0; count--, wlist++)
          {
             _ecore_xcb_textlist_copy_wchar(str, txt);
             *wlist = str;
             len = (_ecore_xcb_textlist_len_wchar(str) + 1);
             str += len;
             txt += len;
          }
     }
   else
     {
        char *txt, *str, **slist;

        txt = (char *)text;
        slist = (char **)list;
        for (str = *slist; count > 0; count--, slist++)
          {
             strcpy(str, txt);
             *slist = str;
             len = strlen(str) + 1;
             str += len;
             txt += len;
          }
     }
}

static wchar_t *
_ecore_xcb_textlist_copy_wchar(wchar_t *str1,
                               wchar_t *str2)
{
   wchar_t *tmp;

   tmp = str1;
   while ((*str1++ = *str2++))
     ;
   return tmp;
}

static int
_ecore_xcb_textlist_len_wchar(wchar_t *str)
{
   wchar_t *ptr;

   ptr = str;
   while (*ptr)
     ptr++;
   return ptr - str;
}

