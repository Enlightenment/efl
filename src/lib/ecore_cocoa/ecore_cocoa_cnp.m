#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#import <Cocoa/Cocoa.h>
#import "ecore_cocoa_window.h"
#include "ecore_cocoa_private.h"
#import "ecore_cocoa_app.h"

EAPI Eina_Bool
ecore_cocoa_clipboard_set(const void *data,
                          int size,
                          const char *raw_mime_type)
{
   NSMutableArray *objects;
   NSString *str = nil;
   BOOL ok = YES;
   NSString *mime_type = [NSString stringWithUTF8String:raw_mime_type];

   objects = [[NSMutableArray alloc] init];
   if ([mime_type hasPrefix:@"text/"])
     {
        str = [[NSString alloc] initWithBytes: data
                                       length: size
                                     encoding: NSUTF8StringEncoding];
        if (str)
          [objects addObject: str];
     }
   else
     {
        ERR("Mimetype %s is not handled yet", raw_mime_type);
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

EAPI Eina_Bool
ecore_cocoa_clipboard_exists(void)
{
   NSDictionary *options;
   NSPasteboard *pb;
   NSArray *items;
   NSMutableArray *classes;

   classes = [[NSMutableArray alloc] init];
   [classes addObject: [NSString class]]; // we only support strings for now
   pb = [NSPasteboard generalPasteboard];
   options = [NSDictionary dictionary];
   return [pb canReadItemWithDataConformingToTypes: classes];
}

EAPI void *
ecore_cocoa_clipboard_get(int *size,
                          const char *raw_mime_type)
{
   NSMutableArray *classes;
   void *data = NULL;
   NSDictionary *options;
   NSPasteboard *pb;
   NSArray *items;
   unsigned int len;
   BOOL string_class = NO;
   NSString *mime_type = [NSString stringWithUTF8String:raw_mime_type];

   classes = [[NSMutableArray alloc] init];

   if ([mime_type hasPrefix:@"text/"])
     {
        string_class = YES;
        [classes addObject: [NSString class]];
     }
   else
     {
        ERR("Mimetype %s is not handled yet", raw_mime_type);
        goto fail;
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

   if (!data)
     {
        ERR("No types retrieved!");
        goto remove_fail;
     }

   [classes removeAllObjects];

   if (size) *size = len;
   return data;

remove_fail:
   [classes removeAllObjects];
fail:
   if (size) *size = 0;
   return NULL;
}

EAPI void
ecore_cocoa_clipboard_clear(void)
{
   [[NSPasteboard generalPasteboard] clearContents];
}
