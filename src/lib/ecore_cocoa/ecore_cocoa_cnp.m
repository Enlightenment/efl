#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#import <Cocoa/Cocoa.h>
#import "ecore_cocoa_window.h"
#include "ecore_cocoa_private.h"
#import "ecore_cocoa_app.h"

EAPI Eina_Bool
ecore_cocoa_clipboard_set(const void           *data,
                          int                   size,
                          Ecore_Cocoa_Cnp_Type  type)
{
   NSMutableArray *objects;
   NSString *str = nil;
   BOOL ok = YES;

   objects = [[NSMutableArray alloc] init];
   if (type & ECORE_COCOA_CNP_TYPE_STRING)
     {
        str = [[NSString alloc] initWithBytes: data
                                       length: size
                                     encoding: NSUTF8StringEncoding];
        if (str)
          [objects addObject: str];
     }
   if (type & ECORE_COCOA_CNP_TYPE_MARKUP)
     {
         WRN("Markup CNP: NOT IMPLEMENTED");
     }

   if (type & ECORE_COCOA_CNP_TYPE_IMAGE)
     {
         WRN("Image CNP: NOT IMPLEMENTED");
     }
   if (type & ECORE_COCOA_CNP_TYPE_HTML)
     {
         WRN("HTML CNP: NOT IMPLEMENTED");
     }

   /* Write to pasteboard */
   if ([objects count] > 0)
     {
        NSPasteboard *pb;

        pb = [NSPasteboard generalPasteboard];
        [pb clearContents];
        ok = [pb writeObjects: objects];
        [objects removeAllObjects];
     }

   return (ok) ? EINA_TRUE : EINA_FALSE;
}


EAPI void *
ecore_cocoa_clipboard_get(int                  *size,
                          Ecore_Cocoa_Cnp_Type  type,
                          Ecore_Cocoa_Cnp_Type *retrieved_types)
{
   NSMutableArray *classes;
   void *data;
   NSDictionary *options;
   NSPasteboard *pb;
   NSArray *items;
   unsigned int len;
   BOOL string_class = NO;
   Ecore_Cocoa_Cnp_Type types = 0;

   classes = [[NSMutableArray alloc] init];

   if (type & ECORE_COCOA_CNP_TYPE_STRING)
     {
        string_class = YES;
        [classes addObject: [NSString class]];
     }
   if (type & ECORE_COCOA_CNP_TYPE_IMAGE)
     {
         WRN("Image CNP: NOT IMPLEMENTED");
     }
   if (type & ECORE_COCOA_CNP_TYPE_HTML)
     {
         WRN("HTML CNP: NOT IMPLEMENTED");
     }

   if ([classes count] <= 0)
     {
        ERR("No registered classes... got nothing from pasteboard");
        goto fail;
     }

   pb = [NSPasteboard generalPasteboard];
   options = [NSDictionary dictionary];
   items = [pb readObjectsForClasses: classes
                             options: options];
   if (!items)
     {
        ERR("No items in the clipboard");
        goto remove_fail;
     }
   if ([items count] != 1)
     {
        ERR("%lu items in pasteboard. Only one at the time can be handled",
            [items count]);
        goto fail;
     }

   if (string_class)
     {
        NSString *str = [items objectAtIndex: 0];
        data = (void *)[str UTF8String];
        len = [str lengthOfBytesUsingEncoding: NSUTF8StringEncoding];
        data = eina_strndup((const char *)data, len);

        if (EINA_UNLIKELY(!data))
          {
             CRI("Failed to strndup() string \"%s\" (len: %u)",
                 (const char *)data, len);
             goto remove_fail;
          }
        types |= ECORE_COCOA_CNP_TYPE_STRING;

#if 0
        if (type & ECORE_COCOA_CNP_TYPE_MARKUP)
          {
             char *markup;
             markup = evas_textblock_text_utf8_to_markup(NULL, data);
             free(data);
             data = markup;
             if (EINA_UNLIKELY(!data))
               {
                  CRI("Failed to retrieve markup from UTF8");
                  goto remove_fail;
               }
             len = strlen(markup);
          }
#endif
     }

   if (!types)
     {
        ERR("No types retrieved!");
        goto remove_fail;
     }

   [classes removeAllObjects];

   if (size) *size = len;
   if (retrieved_types) *retrieved_types = types;
   return data;

remove_fail:
   [classes removeAllObjects];
fail:
   if (size) *size = 0;
   if (retrieved_types) *retrieved_types = 0;
   return NULL;
}

EAPI void
ecore_cocoa_clipboard_clear(void)
{
   [[NSPasteboard generalPasteboard] clearContents];
}
