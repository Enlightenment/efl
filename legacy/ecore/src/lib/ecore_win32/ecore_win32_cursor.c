#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Ecore_Win32_Group Ecore_Win32 library
 *
 * @{
 */

/**
 * @brief Create a new cursor.
 *
 * @param pixels_and The array of bytes containing the bit values for
 * the AND mask of the cursor.
 * @param pixels_xor The array of bytes containing the bit values for
 * the XOR mask of the cursor.
 * @param width The width of the cursor.
 * @param height The height of the cursor.
 * @param hot_x The horizontal position of the cursor's hot spot.
 * @param hot_y The vertical position of the cursor's hot spot.
 * @return A newly user-defined cursor.
 *
 * This function creates a new cursor of size @p width and @p
 * height. They must be valid size. To determine the valid size of a
 * cursor, useecore_win32_cursor_size_get(). @p pixels_and is an array
 * of bytes (unsigned char) containing the bits of the cursor that
 * will be visible. @p pixels_xor is similar but will allow the cursor
 * to have a shape. Here is the truth table for the masks:
 *
 * <table border=1>
 * <tr><td>AND mask</td><td>XOR mask</td><td>Display</td></tr>
 * <tr><td>0</td>       <td>0</td>       <td>Black</td></tr>
 * <tr><td>0</td>       <td>1</td>       <td>White</td></tr>
 * <tr><td>1</td>       <td>0</td>       <td>Screen</td></tr>
 * <tr><td>1</td>       <td>1</td>       <td>Reverse screen</td></tr>
 * </table>
 *
 * @p hot_x and @p hot_y are the position of the hot spot of the
 * cursor. If @p pixels_and or @p pixels_xor are @c NULL, the function
 * returns NULL. If @p width or @p height does not match the valid
 * size of a cursor, the function returns @c NULL. On success, the
 * function creates a user-defined cursor, otherwise it returns
 * @c NULL.
 *
 * Once the cursor is not used anymore, use ecore_win32_cursor_free()
 * to free the ressources.
 *
 * Example of use:
 *
 * @code
 * unsigned char pixels_and[] ={
 * 0xFF, 0xFC, 0x3F, 0xFF,   // line 1
 * 0xFF, 0xC0, 0x1F, 0xFF,   // line 2
 * 0xFF, 0x00, 0x3F, 0xFF,   // line 3
 * 0xFE, 0x00, 0xFF, 0xFF,   // line 4
 *
 * 0xF7, 0x01, 0xFF, 0xFF,   // line 5
 * 0xF0, 0x03, 0xFF, 0xFF,   // line 6
 * 0xF0, 0x03, 0xFF, 0xFF,   // line 7
 * 0xE0, 0x07, 0xFF, 0xFF,   // line 8
 *
 * 0xC0, 0x07, 0xFF, 0xFF,   // line 9
 * 0xC0, 0x0F, 0xFF, 0xFF,   // line 10
 * 0x80, 0x0F, 0xFF, 0xFF,   // line 11
 * 0x80, 0x0F, 0xFF, 0xFF,   // line 12
 *
 * 0x80, 0x07, 0xFF, 0xFF,   // line 13
 * 0x00, 0x07, 0xFF, 0xFF,   // line 14
 * 0x00, 0x03, 0xFF, 0xFF,   // line 15
 * 0x00, 0x00, 0xFF, 0xFF,   // line 16
 *
 * 0x00, 0x00, 0x7F, 0xFF,   // line 17
 * 0x00, 0x00, 0x1F, 0xFF,   // line 18
 * 0x00, 0x00, 0x0F, 0xFF,   // line 19
 * 0x80, 0x00, 0x0F, 0xFF,   // line 20
 *
 * 0x80, 0x00, 0x07, 0xFF,   // line 21
 * 0x80, 0x00, 0x07, 0xFF,   // line 22
 * 0xC0, 0x00, 0x07, 0xFF,   // line 23
 * 0xC0, 0x00, 0x0F, 0xFF,   // line 24
 *
 * 0xE0, 0x00, 0x0F, 0xFF,   // line 25
 * 0xF0, 0x00, 0x1F, 0xFF,   // line 26
 * 0xF0, 0x00, 0x1F, 0xFF,   // line 27
 * 0xF8, 0x00, 0x3F, 0xFF,   // line 28
 *
 * 0xFE, 0x00, 0x7F, 0xFF,   // line 29
 * 0xFF, 0x00, 0xFF, 0xFF,   // line 30
 * 0xFF, 0xC3, 0xFF, 0xFF,   // line 31
 * 0xFF, 0xFF, 0xFF, 0xFF    // line 32
 * };
 *
 * unsigned char pixels_xor[] =
 * {
 * 0x00, 0x00, 0x00, 0x00,   // line 1
 * 0x00, 0x03, 0xC0, 0x00,   // line 2
 * 0x00, 0x3F, 0x00, 0x00,   // line 3
 * 0x00, 0xFE, 0x00, 0x00,   // line 4
 *
 * 0x0E, 0xFC, 0x00, 0x00,   // line 5
 * 0x07, 0xF8, 0x00, 0x00,   // line 6
 * 0x07, 0xF8, 0x00, 0x00,   // line 7
 * 0x0F, 0xF0, 0x00, 0x00,   // line 8
 *
 * 0x1F, 0xF0, 0x00, 0x00,   // line 9
 * 0x1F, 0xE0, 0x00, 0x00,   // line 10
 * 0x3F, 0xE0, 0x00, 0x00,   // line 11
 * 0x3F, 0xE0, 0x00, 0x00,   // line 12
 *
 * 0x3F, 0xF0, 0x00, 0x00,   // line 13
 * 0x7F, 0xF0, 0x00, 0x00,   // line 14
 * 0x7F, 0xF8, 0x00, 0x00,   // line 15
 * 0x7F, 0xFC, 0x00, 0x00,   // line 16
 *
 * 0x7F, 0xFF, 0x00, 0x00,   // line 17
 * 0x7F, 0xFF, 0x80, 0x00,   // line 18
 * 0x7F, 0xFF, 0xE0, 0x00,   // line 19
 * 0x3F, 0xFF, 0xE0, 0x00,   // line 20
 *
 * 0x3F, 0xC7, 0xF0, 0x00,   // line 21
 * 0x3F, 0x83, 0xF0, 0x00,   // line 22
 * 0x1F, 0x83, 0xF0, 0x00,   // line 23
 * 0x1F, 0x83, 0xE0, 0x00,   // line 24
 *
 * 0x0F, 0xC7, 0xE0, 0x00,   // line 25
 * 0x07, 0xFF, 0xC0, 0x00,   // line 26
 * 0x07, 0xFF, 0xC0, 0x00,   // line 27
 * 0x01, 0xFF, 0x80, 0x00,   // line 28
 *
 * 0x00, 0xFF, 0x00, 0x00,   // line 29
 * 0x00, 0x3C, 0x00, 0x00,   // line 30
 * 0x00, 0x00, 0x00, 0x00,   // line 31
 * 0x00, 0x00, 0x00, 0x00    // line 32
 * };
 *
 * Ecore_Win32_Cursor cursor = ecore_win32_cursor_new(pixels_and, pixels_xor, 32, 32, 19, 2);
 * @endcode
 */
EAPI Ecore_Win32_Cursor *
ecore_win32_cursor_new(const void *pixels_and,
                       const void *pixels_xor,
                       int         width,
                       int         height,
                       int         hot_x,
                       int         hot_y)
{
   Ecore_Win32_Cursor *cursor = NULL;
   int                 cursor_width;
   int                 cursor_height;

   INF("creating cursor");

   if (!pixels_and || !pixels_xor)
     return NULL;

   cursor_width = GetSystemMetrics(SM_CXCURSOR);
   cursor_height = GetSystemMetrics(SM_CYCURSOR);

   if ((cursor_width != width) ||
       (cursor_height != height))
     return NULL;

   if (!(cursor = CreateCursor(_ecore_win32_instance,
                               hot_x, hot_y,
                               width, height,
                               pixels_and,
                               pixels_xor)))
     return NULL;

   return cursor;
}

/**
 * @brief Free the given cursor.
 *
 * @param cursor The cursor to free.
 *
 * This function free @p cursor. @p cursor must have been obtained
 * with ecore_win32_cursor_new().
 */
EAPI void
ecore_win32_cursor_free(Ecore_Win32_Cursor *cursor)
{
   INF("destroying cursor");

   DestroyCursor(cursor);
}

/**
 * @brief Create a cursor from a Windows ressource.
 *
 * @param shape The pre-defined shape of the cursor.
 * @return The new cursor.
 *
 * This function returns a pre-defined cursor with a specified
 * @p shape. This cursor does not need to be freed, as it is loaded
 * from an existing resource.
 */
EAPI Ecore_Win32_Cursor *
ecore_win32_cursor_shaped_new(Ecore_Win32_Cursor_Shape shape)
{
   Ecore_Win32_Cursor *cursor = NULL;
   const char         *cursor_name;

   INF("geting shape cursor");

   switch (shape)
     {
       case ECORE_WIN32_CURSOR_SHAPE_APP_STARTING:
         cursor_name = IDC_APPSTARTING;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_ARROW:
         cursor_name = IDC_ARROW;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_CROSS:
         cursor_name = IDC_CROSS;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_HAND:
         cursor_name = IDC_HAND;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_HELP:
         cursor_name = IDC_HELP;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_I_BEAM:
         cursor_name = IDC_IBEAM;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_NO:
         cursor_name = IDC_NO;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_ALL:
         cursor_name = IDC_SIZEALL;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_NESW:
         cursor_name = IDC_SIZENESW;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_NS:
         cursor_name = IDC_SIZENS;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_NWSE:
         cursor_name = IDC_SIZENWSE;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_WE:
         cursor_name = IDC_SIZEWE;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_UP_ARROW:
         cursor_name = IDC_UPARROW;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_WAIT:
         cursor_name = IDC_WAIT;
         break;
     default:
         return NULL;
     }

   if (!(cursor = LoadCursor(NULL, cursor_name)))
     return NULL;

   return cursor;
}

/**
 * @brief Retrieve the size of a valid cursor.
 *
 * @param width The width of a valid cursor.
 * @param height The height of a valid cursor.
 *
 * This function returns the size of a cursor that must be passed to
 * ecore_win32_cursor_new(). @p width and @p height are buffers that
 * will be filled with the correct size. They can be @c NULL.
 */
EAPI void
ecore_win32_cursor_size_get(int *width, int *height)
{
   INF("geting size cursor");

   if (*width) *width = GetSystemMetrics(SM_CXCURSOR);
   if (*height) *height = GetSystemMetrics(SM_CYCURSOR);
}

/**
 * @}
 */
