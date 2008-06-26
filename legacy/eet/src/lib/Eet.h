#ifndef _EET_H
#define _EET_H

#include <stdlib.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EET_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EET_BUILD */
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file Eet.h
 * @brief The file that provides the eet functions.
 *
 * This header provides the Eet management functions.
 *
 */

/***************************************************************************/

#define EET_T_UNKNOW            0 /**< Unknown data encoding type */
#define EET_T_CHAR              1 /**< Data type: char */
#define EET_T_SHORT             2 /**< Data type: short */
#define EET_T_INT               3 /**< Data type: int */
#define EET_T_LONG_LONG         4 /**< Data type: long long */
#define EET_T_FLOAT             5 /**< Data type: float */
#define EET_T_DOUBLE            6 /**< Data type: double */
#define EET_T_UCHAR             7 /**< Data type: unsigned char */
#define EET_T_USHORT            8 /**< Data type: unsigned short */
#define EET_T_UINT              9 /**< Data type: unsigned int */
#define EET_T_ULONG_LONG        10 /**< Data type: unsigned long long */
#define EET_T_STRING            11 /**< Data type: char * */
#define EET_T_INLINED_STRING    12 /**< Data type: char * (but compressed inside the resulting eet) */
#define EET_T_LAST              13 /**< Last data type */

#define EET_G_UNKNOWN    100 /**< Unknown group data encoding type */
#define EET_G_ARRAY      101 /**< Fixed size array group type */
#define EET_G_VAR_ARRAY  102 /**< Variable size array group type */
#define EET_G_LIST       103 /**< Linked list group type */
#define EET_G_HASH       104 /**< Hash table group type */
#define EET_G_LAST       105 /**< Last group type */

/***************************************************************************/

   typedef enum _Eet_File_Mode
     {
	EET_FILE_MODE_INVALID = -1,
	EET_FILE_MODE_READ,
	EET_FILE_MODE_WRITE,
	EET_FILE_MODE_READ_WRITE
     } Eet_File_Mode;

   typedef enum _Eet_Error
     {
	EET_ERROR_NONE,
	EET_ERROR_BAD_OBJECT,
	EET_ERROR_EMPTY,
	EET_ERROR_NOT_WRITABLE,
	EET_ERROR_OUT_OF_MEMORY,
	EET_ERROR_WRITE_ERROR,
	EET_ERROR_WRITE_ERROR_FILE_TOO_BIG,
	EET_ERROR_WRITE_ERROR_IO_ERROR,
	EET_ERROR_WRITE_ERROR_OUT_OF_SPACE,
	EET_ERROR_WRITE_ERROR_FILE_CLOSED
     } Eet_Error;

   typedef struct _Eet_File                  Eet_File;
   typedef struct _Eet_Dictionary            Eet_Dictionary;
   typedef struct _Eet_Data_Descriptor       Eet_Data_Descriptor;

   typedef struct _Eet_Data_Descriptor_Class Eet_Data_Descriptor_Class;

#define EET_DATA_DESCRIPTOR_CLASS_VERSION 2
   struct _Eet_Data_Descriptor_Class
     {
	int         version;
	const char *name;
	int         size;
	struct {
	   void   *(*mem_alloc) (size_t size);
	   void    (*mem_free) (void *mem);
           char   *(*str_alloc) (const char *str);
	   void    (*str_free) (const char *str);
	   void   *(*list_next) (void *l);
	   void   *(*list_append) (void *l, void *d);
	   void   *(*list_data) (void *l);
	   void   *(*list_free) (void *l);
	   void    (*hash_foreach) (void *h, int (*func) (void *h, const char *k, void *dt, void *fdt), void *fdt);
	   void   *(*hash_add) (void *h, const char *k, void *d);
	   void    (*hash_free) (void *h);
           char   *(*str_direct_alloc) (const char *str);
           void    (*str_direct_free) (const char *str);
	} func;
     };

/***************************************************************************/

   /**
    * Initialize the EET library.
    *
    * @return The new init count.
    *
    * @since 1.0.0
    */
   EAPI int eet_init(void);

   /**
    * Shut down the EET library.
    *
    * @return The new init count.
    *
    * @since 1.0.0
    */
   EAPI int eet_shutdown(void);

   /**
    * Clear eet cache
    *
    * Eet didn't free items by default. If you are under memory presure, just
    * call this function to recall all memory that are not yet referenced anymore.
    * The cache take care of modification on disk.
    *
    * @since 1.0.0
    */
   EAPI void eet_clearcache(void);

   /**
    * Open an eet file on disk, and returns a handle to it.
    * @param file The file path to the eet file. eg: "/tmp/file.eet".
    * @param mode The mode for opening. Either EET_FILE_MODE_READ, EET_FILE_MODE_WRITE or EET_FILE_MODE_READ_WRITE.
    * @return An opened eet file handle.
    *
    * This function will open an exiting eet file for reading, and build
    * the directory table in memory and return a handle to the file, if it
    * exists and can be read, and no memory errors occur on the way, otherwise
    * NULL will be returned.
    *
    * It will also open an eet file for writing. This will, if successful,
    * delete the original file and replace it with a new empty file, till
    * the eet file handle is closed or flushed. If it cannot be opened for
    * writing or a memory error occurs, NULL is returned.
    *
    * You can also open the file for read/write. If you then write a key that
    * does not exist it will be created, if the key exists it will be replaced
    * by the new data.
    *
    * Example:
    * @code
    * #include <Eet.h>
    * #include <stdio.h>
    *
    * int
    * main(int argc, char **argv)
    * {
    *   Eet_File *ef;
    *   char buf[1024], *ret, **list;
    *   int size, num, i;
    *
    *   strcpy(buf, "Here is a string of data to save!");
    *
    *   ef = eet_open("/tmp/my_file.eet", EET_FILE_MODE_WRITE);
    *   if (!ef) return -1;
    *   if (!eet_write(ef, "/key/to_store/at", buf, 1024, 1))
    *     fprintf(stderr, "Error writing data!\n");
    *   eet_close(ef);
    *
    *   ef = eet_open("/tmp/my_file.eet", EET_FILE_MODE_READ);
    *   if (!ef) return -1;
    *   list = eet_list(ef, "*", &num);
    *   if (list)
    *     {
    *       for (i = 0; i < num; i++)
    *         printf("Key stored: %s\n", list[i]);
    *       free(list);
    *     }
    *   ret = eet_read(ef, "/key/to_store/at", &size);
    *   if (ret)
    *     {
    *       printf("Data read (%i bytes):\n%s\n", size, ret);
    *       free(ret);
    *     }
    *   eet_close(ef);
    *
    *   return 0;
    * }
    * @endcode
    *
    * @since 1.0.0
    */
   EAPI Eet_File *eet_open(const char *file, Eet_File_Mode mode);

   /**
    * Get the mode an Eet_File was opened with.
    * @param ef A valid eet file handle.
    * @return The mode ef was opened with.
    *
    * @since 1.0.0
    */
   EAPI Eet_File_Mode eet_mode_get(Eet_File *ef);

   /**
    * Close an eet file handle and flush and writes pending.
    * @param ef A valid eet file handle.
    *
    * This function will flush any pending writes to disk if the eet file
    * was opened for write, and free all data associated with the file handle
    * and file, and close the file.
    *
    * If the eet file handle is not valid nothing will be done.
    *
    * @since 1.0.0
    */
   EAPI Eet_Error eet_close(Eet_File *ef);


   /**
    * Return a handle to the shared string dictionary of the Eet file
    * @param ef A valid eet file handle.
    * @return A handle to the dictionary of the file
    *
    * This function returns a handle to the dictionary of an Eet file whose
    * handle is @p ef, if a dictionary exists. NULL is returned otherwise or
    * if the file handle is known to be invalid.
    *
    * @since 1.0.0
    */
   EAPI Eet_Dictionary *eet_dictionary_get(Eet_File *ef);

   /**
    * Check if a given string comes from a given dictionary
    * @param ed A valid dictionary handle
    * @param string A valid 0 byte terminated C string
    * @return 1 if it is in the dictionary, 0 otherwise
    *
    * This checks the given dictionary to see if the given string is actually
    * inside that dictionary (i.e. comes from it) and returns 1 if it does.
    * If the dictionary handle is invlide, the string is NULL or the string is
    * not in the dictionary, 0 is returned.
    *
    * @since 1.0.0
    */
   EAPI int eet_dictionary_string_check(Eet_Dictionary *ed, const char *string);

   /**
    * Read a specified entry from an eet file and return data
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param size_ret Number of bytes read from entry and returned.
    * @return The data stored in that entry in the eet file.
    *
    * This function finds an entry in the eet file that is stored under the
    * name specified, and returns that data, decompressed, if successful.
    * NULL is returned if the lookup fails or if memory errors are
    * encountered. It is the job of the calling program to call free() on
    * the returned data. The number of bytes in the returned data chunk are
    * placed in size_ret.
    *
    * If the eet file handle is not valid NULL is returned and size_ret is
    * filled with 0.
    *
    * @since 1.0.0
    */
   EAPI void *eet_read(Eet_File *ef, const char *name, int *size_ret);

   /**
    * Read a specified entry from an eet file and return data
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param size_ret Number of bytes read from entry and returned.
    * @return The data stored in that entry in the eet file.
    *
    * This function finds an entry in the eet file that is stored under the
    * name specified, and returns that data if not compressed and successful.
    * NULL is returned if the lookup fails or if memory errors are
    * encountered or if the data is comrpessed. The calling program must never
    * call free() on the returned data. The number of bytes in the returned
    * data chunk are placed in size_ret.
    *
    * If the eet file handle is not valid NULL is returned and size_ret is
    * filled with 0.
    *
    * @since 1.0.0
    */
   EAPI const void *eet_read_direct(Eet_File *ef, const char *name, int *size_ret);

   /**
    * Write a specified entry to an eet file handle
    * @param ef A valid eet file handle opened for writing.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param data Pointer to the data to be stored.
    * @param size Length in bytes in the data to be stored.
    * @param compress Compression flags (1 == compress, 0 = don't compress).
    * @return Success or failure of the write.
    *
    * This function will write the specified chunk of data to the eet file
    * and return greater than 0 on success. 0 will be returned on failure.
    *
    * The eet file handle must be a valid file handle for an eet file opened
    * for writing. If it is not, 0 will be returned and no action will be
    * performed.
    *
    * Name, and data must not be NULL, and size must be > 0. If these
    * conditions are not met, 0 will be returned.
    *
    * The data will be copied (and optionally compressed) in ram, pending
    * a flush to disk (it will stay in ram till the eet file handle is
    * closed though).
    *
    * @since 1.0.0
    */
   EAPI int eet_write(Eet_File *ef, const char *name, const void *data, int size, int compress);

   /**
    * Delete a specified entry from an Eet file being written or re-written
    * @param ef A valid eet file handle opened for writing.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @return Success or failure of the delete.
    *
    * This function will delete the specified chunk of data from the eet file
    * and return greater than 0 on success. 0 will be returned on failure.
    *
    * The eet file handle must be a valid file handle for an eet file opened
    * for writing. If it is not, 0 will be returned and no action will be
    * performed.
    *
    * Name, must not be NULL, otherwise 0 will be returned.
    *
    * @since 1.0.0
    */
   EAPI int eet_delete(Eet_File *ef, const char *name);

   /**
    * List all entries in eet file matching shell glob.
    * @param ef A valid eet file handle.
    * @param glob A shell glob to match against.
    * @param count_ret Number of entries found to match.
    * @return Pointer to an array of strings.
    *
    * This function will list all entries in the eet file matching the
    * supplied shell glob and return an allocated list of their names, if
    * there are any, and if no memory errors occur.
    *
    * The eet file handle must be valid and glob must not be NULL, or NULL
    * will be returned and count_ret will be filled with 0.
    *
    * The calling program must call free() on the array returned, but NOT
    * on the string pointers in the array. They are taken as read-only
    * internals from the eet file handle. They are only valid as long as
    * the file handle is not closed. When it is closed those pointers in the
    * array are now not valid and should not be used.
    *
    * On success the array returned will have a list of string pointers
    * that are the names of the entries that matched, and count_ret will have
    * the number of entries in this array placed in it.
    *
    * Hint: an easy way to list all entries in an eet file is to use a glob
    * value of "*".
    *
    * @since 1.0.0
    */
   EAPI char **eet_list(Eet_File *ef, const char *glob, int *count_ret);

   /**
    * Return the number of entries in the specified eet file.
    * @param ef A valid eet file handle.
    * @return Number of entries in ef or -1 if the number of entries
    *         cannot be read due to open mode restrictions.
    *
    * @since 1.0.0
    */
   EAPI int eet_num_entries(Eet_File *ef);

/***************************************************************************/

   /**
    * Read just the header data for an image and dont decode the pixels.
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param w A pointer to the unsigned int to hold the width in pixels.
    * @param h A pointer to the unsigned int to hold the height in pixels.
    * @param alpha A pointer to the int to hold the alpha flag.
    * @param compress A pointer to the int to hold the compression amount.
    * @param quality A pointer to the int to hold the quality amount.
    * @param lossy A pointer to the int to hold the lossiness flag.
    * @return 1 on successfull decode, 0 otherwise
    *
    * This function reads an image from an eet file stored under the named
    * key in the eet file and return a pointer to the decompressed pixel data.
    *
    * The other parameters of the image (width, height etc.) are placed into
    * the values pointed to (they must be supplied). The pixel data is a linear
    * array of pixels starting from the top-left of the image scanning row by
    * row from left to right. Each pile is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denotes that it is significant.
    * Compress is filled with the compression value/amount the image was
    * stored with. The quality value is filled with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    *
    * On success the function returns 1 indicating the header was read and
    * decoded properly, or 0 on failure.
    *
    * @since 1.0.0
    */
   EAPI int eet_data_image_header_read(Eet_File *ef, const char *name, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy);

   /**
    * Read image data from the named key in the eet file.
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param w A pointer to the unsigned int to hold the width in pixels.
    * @param h A pointer to the unsigned int to hold the height in pixels.
    * @param alpha A pointer to the int to hold the alpha flag.
    * @param compress A pointer to the int to hold the compression amount.
    * @param quality A pointer to the int to hold the quality amount.
    * @param lossy A pointer to the int to hold the lossiness flag.
    * @return The image pixel data decoded
    *
    * This function reads an image from an eet file stored under the named
    * key in the eet file and return a pointer to the decompressed pixel data.
    *
    * The other parameters of the image (width, height etc.) are placed into
    * the values pointed to (they must be supplied). The pixel data is a linear
    * array of pixels starting from the top-left of the image scanning row by
    * row from left to right. Each pile is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denotes that it is significant.
    * Compress is filled with the compression value/amount the image was
    * stored with. The quality value is filled with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    *
    * On success the function returns a pointer to the image data decoded. The
    * calling application is responsible for calling free() on the image data
    * when it is done with it. On failure NULL is returned and the parameter
    * values may not contain any sensible data.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_image_read(Eet_File *ef, const char *name, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy);

   /**
    * Read image data from the named key in the eet file.
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param src_x The starting x coordinate from where to dump the stream.
    * @param src_y The starting y coordinate from where to dump the stream.
    * @param d A pointer to the pixel surface.
    * @param w The expected width in pixels of the pixel surface to decode.
    * @param h The expected height in pixels of the pixel surface to decode.
    * @param row_stride The length of a pixels line in the destination surface.
    * @param alpha A pointer to the int to hold the alpha flag.
    * @param compress A pointer to the int to hold the compression amount.
    * @param quality A pointer to the int to hold the quality amount.
    * @param lossy A pointer to the int to hold the lossiness flag.
    * @return 1 on success, 0 otherwise.
    *
    * This function reads an image from an eet file stored under the named
    * key in the eet file and return a pointer to the decompressed pixel data.
    *
    * The other parameters of the image (width, height etc.) are placed into
    * the values pointed to (they must be supplied). The pixel data is a linear
    * array of pixels starting from the top-left of the image scanning row by
    * row from left to right. Each pile is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denotes that it is significant.
    * Compress is filled with the compression value/amount the image was
    * stored with. The quality value is filled with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    *
    * On success the function returns 1, and 0 on failure. On failure the
    * parameter values may not contain any sensible data.
    *
    * @since 1.0.2
    */
  EAPI int eet_data_image_read_to_surface(Eet_File *ef, const char *name, unsigned int src_x, unsigned int src_y, unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride, int *alpha, int *compress, int *quality, int *lossy);

   /**
    * Write image data to the named key in an eet file.
    * @param ef A valid eet file handle opened for writing.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param data A pointer to the image pixel data.
    * @param w The width of the image in pixels.
    * @param h The height of the image in pixels.
    * @param alpha The alpha channel flag.
    * @param compress The compression amount.
    * @param quality The quality encoding amount.
    * @param lossy The lossiness flag.
    * @return Success if the data was encoded and written or not.
    *
    * This function takes image pixel data and encodes it in an eet file
    * stored under the supplied name key, and returns how many bytes were
    * actually written to encode the image data.
    *
    * The data expected is the same format as returned by eet_data_image_read.
    * If this is not the case weird things may happen. Width and height must
    * be between 1 and 8000 pixels. The alpha flags can be 0 or 1 (0 meaning
    * the alpha values are not useful and 1 meaning they are). Compress can
    * be from 0 to 9 (0 meaning no compression, 9 meaning full compression).
    * This is only used if the image is not lossily encoded. Quality is used on
    * lossy compression and should be a value from 0 to 100. The lossy flag
    * can be 0 or 1. 0 means encode losslessly and 1 means to encode with
    * image quality loss (but then have a much smaller encoding).
    *
    * On success this function returns the number of bytes that were required
    * to encode the image data, or on failure it returns 0.
    *
    * @since 1.0.0
    */
   EAPI int eet_data_image_write(Eet_File *ef, const char *name, const void *data, unsigned int w, unsigned int h, int alpha, int compress, int quality, int lossy);

   /**
    * Decode Image data header only to get information.
    * @param data The encoded pixel data.
    * @param size The size, in bytes, of the encoded pixel data.
    * @param w A pointer to the unsigned int to hold the width in pixels.
    * @param h A pointer to the unsigned int to hold the height in pixels.
    * @param alpha A pointer to the int to hold the alpha flag.
    * @param compress A pointer to the int to hold the compression amount.
    * @param quality A pointer to the int to hold the quality amount.
    * @param lossy A pointer to the int to hold the lossiness flag.
    * @return 1 on success, 0 on failure.
    *
    * This function takes encoded pixel data and decodes it into raw RGBA
    * pixels on success.
    *
    * The other parameters of the image (width, height etc.) are placed into
    * the values pointed to (they must be supplied). The pixel data is a linear
    * array of pixels starting from the top-left of the image scanning row by
    * row from left to right. Each pixel is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denotes that it is significant.
    * Compress is filled with the compression value/amount the image was
    * stored with. The quality value is filled with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    *
    * On success the function returns 1 indicating the header was read and
    * decoded properly, or 0 on failure.
    *
    * @since 1.0.0
    */
   EAPI int eet_data_image_header_decode(const void *data, int size, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy);

   /**
    * Decode Image data into pixel data.
    * @param data The encoded pixel data.
    * @param size The size, in bytes, of the encoded pixel data.
    * @param w A pointer to the unsigned int to hold the width in pixels.
    * @param h A pointer to the unsigned int to hold the height in pixels.
    * @param alpha A pointer to the int to hold the alpha flag.
    * @param compress A pointer to the int to hold the compression amount.
    * @param quality A pointer to the int to hold the quality amount.
    * @param lossy A pointer to the int to hold the lossiness flag.
    * @return The image pixel data decoded
    *
    * This function takes encoded pixel data and decodes it into raw RGBA
    * pixels on success.
    *
    * The other parameters of the image (width, height etc.) are placed into
    * the values pointed to (they must be supplied). The pixel data is a linear
    * array of pixels starting from the top-left of the image scanning row by
    * row from left to right. Each pixel is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denotes that it is significant.
    * Compress is filled with the compression value/amount the image was
    * stored with. The quality value is filled with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    *
    * On success the function returns a pointer to the image data decoded. The
    * calling application is responsible for calling free() on the image data
    * when it is done with it. On failure NULL is returned and the parameter
    * values may not contain any sensible data.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_image_decode(const void *data, int size, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy);

   /**
    * Decode Image data into pixel data.
    * @param data The encoded pixel data.
    * @param size The size, in bytes, of the encoded pixel data.
    * @param src_x The starting x coordinate from where to dump the stream.
    * @param src_y The starting y coordinate from where to dump the stream.
    * @param d A pointer to the pixel surface.
    * @param w The expected width in pixels of the pixel surface to decode.
    * @param h The expected height in pixels of the pixel surface to decode.
    * @param row_stride The length of a pixels line in the destination surface.
    * @param alpha A pointer to the int to hold the alpha flag.
    * @param compress A pointer to the int to hold the compression amount.
    * @param quality A pointer to the int to hold the quality amount.
    * @param lossy A pointer to the int to hold the lossiness flag.
    * @return 1 on success, 0 otherwise.
    *
    * This function takes encoded pixel data and decodes it into raw RGBA
    * pixels on success.
    *
    * The other parameters of the image (alpha, compress etc.) are placed into
    * the values pointed to (they must be supplied). The pixel data is a linear
    * array of pixels starting from the top-left of the image scanning row by
    * row from left to right. Each pixel is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denotes that it is significant.
    * Compress is filled with the compression value/amount the image was
    * stored with. The quality value is filled with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    *
    * On success the function returns 1, and 0 on failure. On failure the
    * parameter values may not contain any sensible data.
    *
    * @since 1.0.2
    */
   EAPI int eet_data_image_decode_to_surface(const void *data, int size, unsigned int src_x, unsigned int src_y, unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride, int *alpha, int *compress, int *quality, int *lossy);

   /**
    * Encode image data for storage or transmission.
    * @param data A pointer to the image pixel data.
    * @param size_ret A pointer to an int to hold the size of the returned data.
    * @param w The width of the image in pixels.
    * @param h The height of the image in pixels.
    * @param alpha The alpha channel flag.
    * @param compress The compression amount.
    * @param quality The quality encoding amount.
    * @param lossy The lossiness flag.
    * @return The encoded image data.
    *
    * This function stakes image pixel data and encodes it with compression and
    * possible loss of quality (as a trade off for size) for storage or
    * transmission to another system.
    *
    * The data expected is the same format as returned by eet_data_image_read.
    * If this is not the case weird things may happen. Width and height must
    * be between 1 and 8000 pixels. The alpha flags can be 0 or 1 (0 meaning
    * the alpha values are not useful and 1 meaning they are). Compress can
    * be from 0 to 9 (0 meaning no compression, 9 meaning full compression).
    * This is only used if the image is not lossily encoded. Quality is used on
    * lossy compression and should be a value from 0 to 100. The lossy flag
    * can be 0 or 1. 0 means encode losslessly and 1 means to encode with
    * image quality loss (but then have a much smaller encoding).
    *
    * On success this function returns a pointer to the encoded data that you
    * can free with free() when no longer needed.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_image_encode(const void *data, int *size_ret, unsigned int w, unsigned int h, int alpha, int compress, int quality, int lossy);

/***************************************************************************/

   /**
    * Create a new empty data structure descriptor.
    * @param name The string name of this data structure (most be a global constant and never change).
    * @param size The size of the struct (in bytes).
    * @param func_list_next The function to get the next list node.
    * @param func_list_append The function to append a member to a list.
    * @param func_list_data The function to get the data from a list node.
    * @param func_list_free The function to free an entire linked list.
    * @param func_hash_foreach The function to iterate through all hash table entries.
    * @param func_hash_add The function to add a member to a hash table.
    * @param func_hash_free The function to free an entire hash table.
    * @return A new empty data descriptor.
    *
    * This function creates a new data descriptore and returns a handle to the
    * new data descriptor. On creation it will be empty, containing no contents
    * describing anything other than the shell of the data structure.
    *
    * You add structure members to the data descriptor using the macros
    * EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB() and
    * EET_DATA_DESCRIPTOR_ADD_LIST(), depending on what type of member you are
    * adding to the description.
    *
    * Once you have described all the members of a struct you want loaded, or
    * saved eet can load and save those members for you, encode them into
    * endian-independant serialised data chunks for transmission across a
    * a network or more.
    *
    * The function pointers to the list and hash table functions are only 
    * needed if you use those data types, else you can pass NULL instead.
    *
    * Example:
    *
    * @code
    * #include <Eet.h>
    * #include <Evas.h>
    *
    * typedef struct _blah2
    * {
    *    char *string;
    * }
    * Blah2;
    *
    * typedef struct _blah3
    * {
    *    char *string;
    * }
    * Blah3;
    *
    * typedef struct _blah
    * {
    *    char character;
    *    short sixteen;
    *    int integer;
    *    long long lots;
    *    float floating;
    *    double floating_lots;
    *    char *string;
    *    Blah2 *blah2;
    *    Evas_List *blah3;
    * }
    * Blah;
    *
    * int
    * main(int argc, char **argv)
    * {
    *    Blah blah;
    *    Blah2 blah2;
    *    Blah3 blah3;
    *    Eet_Data_Descriptor *edd, *edd2, *edd3;
    *    void *data;
    *    int size;
    *    FILE *f;
    *    Blah *blah_in;
    *
    *    edd3 = eet_data_descriptor_new("blah3", sizeof(Blah3),
    *                                   evas_list_next,
    *                                   evas_list_append,
    *                                   evas_list_data,
    *                                   evas_list_free,
    *                                   evas_hash_foreach,
    *                                   evas_hash_add,
    *                                   evas_hash_free);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd3, Blah3, "string3", string, EET_T_STRING);
    *
    *    edd2 = eet_data_descriptor_new("blah2", sizeof(Blah2),
    *                                   evas_list_next,
    *                                   evas_list_append,
    *                                   evas_list_data,
    *                                   evas_list_free,
    *                                   evas_hash_foreach,
    *                                   evas_hash_add,
    *                                   evas_hash_free);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd2, Blah2, "string2", string, EET_T_STRING);
    *
    *    edd = eet_data_descriptor_new("blah", sizeof(Blah),
    *                                   evas_list_next,
    *                                   evas_list_append,
    *                                   evas_list_data,
    *                                   evas_list_free,
    *                                   evas_hash_foreach,
    *                                   evas_hash_add,
    *                                   evas_hash_free);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "character", character, EET_T_CHAR);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "sixteen", sixteen, EET_T_SHORT);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "integer", integer, EET_T_INT);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "lots", lots, EET_T_LONG_LONG);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "floating", floating, EET_T_FLOAT);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "floating_lots", floating_lots, EET_T_DOUBLE);
    *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Blah, "string", string, EET_T_STRING);
    *    EET_DATA_DESCRIPTOR_ADD_SUB(edd, Blah, "blah2", blah2, edd2);
    *    EET_DATA_DESCRIPTOR_ADD_LIST(edd, Blah, "blah3", blah3, edd3);
    *
    *    blah3.string="PANTS";
    *
    *    blah2.string="subtype string here!";
    *
    *    blah.character='7';
    *    blah.sixteen=0x7777;
    *    blah.integer=0xc0def00d;
    *    blah.lots=0xdeadbeef31337777;
    *    blah.floating=3.141592654;
    *    blah.floating_lots=0.777777777777777;
    *    blah.string="bite me like a turnip";
    *    blah.blah2 = &blah2;
    *    blah.blah3 = evas_list_append(NULL, &blah3);
    *    blah.blah3 = evas_list_append(blah.blah3, &blah3);
    *    blah.blah3 = evas_list_append(blah.blah3, &blah3);
    *    blah.blah3 = evas_list_append(blah.blah3, &blah3);
    *    blah.blah3 = evas_list_append(blah.blah3, &blah3);
    *    blah.blah3 = evas_list_append(blah.blah3, &blah3);
    *    blah.blah3 = evas_list_append(blah.blah3, &blah3);
    *
    *    data = eet_data_descriptor_encode(edd, &blah, &size);
    *    printf("-----DECODING\n");
    *    blah_in = eet_data_descriptor_decode(edd, data, size);
    *
    *    printf("-----DECODED!\n");
    *    printf("%c\n", blah_in->character);
    *    printf("%x\n", (int)blah_in->sixteen);
    *    printf("%x\n", blah_in->integer);
    *    printf("%lx\n", blah_in->lots);
    *    printf("%f\n", (double)blah_in->floating);
    *    printf("%f\n", (double)blah_in->floating_lots);
    *    printf("%s\n", blah_in->string);
    *    printf("%p\n", blah_in->blah2);
    *    printf("  %s\n", blah_in->blah2->string);
    *      {
    *         Evas_List *l;
    *
    *         for (l = blah_in->blah3; l; l = l->next)
    *           {
    *              Blah3 *blah3_in;
    *
    *              blah3_in = l->data;
    *              printf("%p\n", blah3_in);
    *              printf("  %s\n", blah3_in->string);
    *           }
    *      }
    *    eet_data_descriptor_free(edd);
    *    eet_data_descriptor_free(edd2);
    *    eet_data_descriptor_free(edd3);
    *
    *   return 0;
    * }
    *
    * @endcode
    *
    * @since 1.0.0
    */
   EAPI Eet_Data_Descriptor *eet_data_descriptor_new(const char *name, int size, void *(*func_list_next) (void *l), void *(*func_list_append) (void *l, void *d), void *(*func_list_data) (void *l), void *(*func_list_free) (void *l), void  (*func_hash_foreach) (void *h, int (*func) (void *h, const char *k, void *dt, void *fdt), void *fdt), void *(*func_hash_add) (void *h, const char *k, void *d), void  (*func_hash_free) (void *h));
   /*
    * FIXME:
    *
    * moving to this api from the old above. this will break things when the
    * move happens - but be warned
    */
   EAPI Eet_Data_Descriptor *eet_data_descriptor2_new(Eet_Data_Descriptor_Class *eddc);
   EAPI Eet_Data_Descriptor *eet_data_descriptor3_new(Eet_Data_Descriptor_Class *eddc);

   /**
    * This function frees a data descriptor when it is not needed anymore.
    * @param edd The data descriptor to free.
    *
    * This function takes a data descriptor handle as a parameter and frees all
    * data allocated for the data descriptor and the handle itself. After this
    * call the descriptor is no longer valid.
    *
    * @since 1.0.0
    */
   EAPI void eet_data_descriptor_free(Eet_Data_Descriptor *edd);

   /**
    * This function is an internal used by macros.
    *
    * This function is used by macros EET_DATA_DESCRIPTOR_ADD_BASIC(),
    * EET_DATA_DESCRIPTOR_ADD_SUB() and EET_DATA_DESCRIPTOR_ADD_LIST(). It is
    * complex to use by hand and should be left to be used by the macros, and
    * thus is not documented.
    *
    * @since 1.0.0
    */
   EAPI void eet_data_descriptor_element_add(Eet_Data_Descriptor *edd, const char *name, int type, int group_type, int offset, /* int count_offset,  */int count, const char *counter_name, Eet_Data_Descriptor *subtype);

   /**
    * Read a data structure from an eet file and decodes it.
    * @param ef The eet file handle to read from.
    * @param edd The data descriptor handle to use when decoding.
    * @param name The key the data is stored under in the eet file.
    * @return A pointer to the decoded data structure.
    *
    * This function decodes a data structure stored in an eet file, returning
    * a pointer to it if it decoded successfully, or NULL on failure. This
    * can save a programmer dozens of hours of work in writing configuration
    * file parsing and writing code, as eet does all that work for the program
    * and presents a program-friendly data structure, just as the programmer
    * likes. Eet can handle members being added or deleted from the data in
    * storage and safely zero-fills unfilled members if they were not found
    * in the data. It checks sizes and headers whenever it reads data, allowing
    * the programmer to not worry about corrupt data.
    *
    * Once a data structure has been described by the programmer with the
    * fields they wish to save or load, storing or retrieving a data structure
    * from an eet file, or from a chunk of memory is as simple as a single
    * function call.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_read(Eet_File *ef, Eet_Data_Descriptor *edd, const char *name);

   /**
    * Write a data structure from memory and store in an eet file.
    * @param ef The eet file handle to write to.
    * @param edd The data descriptor to use when encoding.
    * @param name The key to store the data under in the eet file.
    * @param data A pointer to the data structure to ssave and encode.
    * @param compress Compression flags for storage.
    * @return 1 on successful write, 0 on failure.
    *
    * This function is the reverse of eet_data_read(), saving a data structure
    * to an eet file.
    *
    * @since 1.0.0
    */
   EAPI int eet_data_write(Eet_File *ef, Eet_Data_Descriptor *edd, const char *name, const void *data, int compress);

   /**
    * Dump an eet encoded data structure into ascii text
    * @param data_in The pointer to the data to decode into a struct.
    * @param size_in The size of the data pointed to in bytes.
    * @param dumpfunc The function to call passed a string when new data is converted to text
    * @param dumpdata The data to pass to the @p dumpfunc callback.
    * @return 1 on success, 0 on failure
    *
    * This function will take a chunk of data encoded by
    * eet_data_descriptor_encode() and convert it into human readable ascii text.
    * It does this by calling the @p dumpfunc callback for all new text that is
    * generated. This callback should append to any existing text buffer and
    * will be passed the pointer @p dumpdata as a parameter as well as a string
    * with new text to be appended.
    *
    * Example:
    *
    * @code
    *
    * void output(void *data, const char *string)
    * {
    *   printf("%s", string);
    * }
    *
    * void dump(const char *file)
    * {
    *   FILE *f;
    *   int len;
    *   void *data;
    *
    *   f = fopen(file, "r");
    *   fseek(f, 0, SEEK_END);
    *   len = ftell(f);
    *   rewind(f);
    *   data = malloc(len);
    *   fread(data, len, 1, f);
    *   fclose(f);
    *   eet_data_text_dump(data, len, output, NULL);
    * }
    * @endcode
    *
    * @since 1.0.0
    */
   EAPI int eet_data_text_dump(const void *data_in, int size_in, void (*dumpfunc) (void *data, const char *str), void *dumpdata);

   /**
    * Take an ascii encoding from eet_data_text_dump() and re-encode in binary.
    * @param text The pointer to the string data to parse and encode.
    * @param textlen The size of the string in bytes (not including 0 byte terminator).
    * @param size_ret This gets filled in with the encoded data blob size in bytes.
    * @return The encoded data on success, NULL on failure.
    *
    * This function will parse the string pointed to by @p text and return
    * an encoded data lump the same way eet_data_descriptor_encode() takes an
    * in-memory data struct and encodes into a binary blob. @p text is a normal
    * C string.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_text_undump(const char *text, int textlen, int *size_ret);

   /**
    * Dump an eet encoded data structure from an eet file into ascii text
    * @param ef A valid eet file handle.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param dumpfunc The function to call passed a string when new data is converted to text
    * @param dumpdata The data to pass to the @p dumpfunc callback.
    * @return 1 on success, 0 on failure
    *
    * This function will take an open and valid eet file from eet_open() request
    * the data encoded by eet_data_descriptor_encode() corresponding to the key @p name
    * and convert it into human readable ascii text. It does this by calling the
    * @p dumpfunc callback for all new text that is generated. This callback should
    * append to any existing text buffer and will be passed the pointer @p dumpdata
    * as a parameter as well as a string with new text to be appended.
    *
    * @since 1.0.0
    */
   EAPI int eet_data_dump(Eet_File *ef, const char *name, void (*dumpfunc) (void *data, const char *str), void *dumpdata);

   /**
    * Take an ascii encoding from eet_data_dump() and re-encode in binary.
    * @param ef A valid eet file handle.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param text The pointer to the string data to parse and encode.
    * @param textlen The size of the string in bytes (not including 0 byte terminator).
    * @param compress Compression flags (1 == compress, 0 = don't compress).
    * @return 1 on success, 0 on failure
    *
    * This function will parse the string pointed to by @p text, encode it the same
    * way eet_data_descriptor_encode() takes an in-memory data struct and encodes into a
    * binary blob.
    *
    * The data (optionally compressed) will be in ram, pending a flush to
    * disk (it will stay in ram till the eet file handle is closed though).
    *
    * @since 1.0.0
    */
   EAPI int eet_data_undump(Eet_File *ef, const char *name, const char *text, int textlen, int compress);

   /**
    * Decode a data structure from an arbitary location in memory.
    * @param edd The data  descriptor to use when decoding.
    * @param data_in The pointer to the data to decode into a struct.
    * @param size_in The size of the data pointed to in bytes.
    * @return NULL on failure, or a valid decoded struct pointer on success.
    *
    * This function will decode a data structure that has been encoded using
    * eet_data_descriptor_encode(), and return a data structure with all its
    * elements filled out, if successful, or NULL on failure.
    *
    * The data to be decoded is stored at the memory pointed to by @p data_in,
    * and is described by the descriptor pointed to by @p edd. The data size is
    * passed in as the value to @p size_in, ande must be greater than 0 to
    * succeed.
    *
    * This function is useful for decoding data structures delivered to the
    * application by means other than an eet file, such as an IPC or socket
    * connection, raw files, shared memory etc.
    *
    * Please see eet_data_read() for more information.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_descriptor_decode(Eet_Data_Descriptor *edd, const void *data_in, int size_in);

   /**
    * Encode a dsata struct to memory and return that encoded data.
    * @param edd The data  descriptor to use when encoding.
    * @param data_in The pointer to the struct to encode into data.
    * @param size_ret A pointer to the an int to be filled with the decoded size.
    * @return NULL on failure, or a valid encoded data chunk on success.
    *
    * This function takes a data structutre in memory and encodes it into a
    * serialised chunk of data that can be decoded again by
    * eet_data_descriptor_decode(). This is useful for being able to transmit
    * data structures across sockets, pipes, IPC or shared file mechanisms,
    * without having to worry about memory space, machine type, endianess etc.
    *
    * The parameter @p edd must point to a valid data descriptor, and
    * @p data_in must point to the right data structure to encode. If not, the
    * encoding may fail.
    *
    * On success a non NULL valid pointer is returned and what @p size_ret
    * points to is set to the size of this decoded data, in bytes. When the
    * encoded data is no longer needed, call free() on it. On failure NULL is
    * returned and what @p size_ret points to is set to 0.
    *
    * Please see eet_data_write() for more information.
    *
    * @since 1.0.0
    */
   EAPI void *eet_data_descriptor_encode(Eet_Data_Descriptor *edd, const void *data_in, int *size_ret);

   /**
    * Add a basic data element to a data descriptor.
    * @param edd The data descriptor to add the type to.
    * @param struct_type The type of the struct.
    * @param name The string name to use to encode/decode this member (must be a constant global and never change).
    * @param member The struct member itself to be encoded.
    * @param type The type of the member to encode.
    *
    * This macro is a convenience macro provided to add a member to the data
    * descriptor @p edd. The type of the structure is provided as the
    * @p struct_type parameter (for example: struct my_struct). The @p name
    * parameter defines a string that will be used to uniquely name that
    * member of the struct (it is suggested to use the struct member itself).
    * The @p member parameter is the actual struct member itself (for
eet_dictionary_string_check    * example: values), and @p type is the basic data type of the member which
    * must be one of: EET_T_CHAR, EET_T_SHORT, EET_T_INT, EET_T_LONG_LONG,
    * EET_T_FLOAT, EET_T_DOUBLE, EET_T_UCHAR, EET_T_USHORT, EET_T_UINT,
    * EET_T_ULONG_LONG or EET_T_STRING.
    *
    * @since 1.0.0
    */
#define EET_DATA_DESCRIPTOR_ADD_BASIC(edd, struct_type, name, member, type) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, type, EET_G_UNKNOWN, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, /* 0,  */NULL, NULL); \
     }

   /**
    * Add a sub-element type to a data descriptor
    * @param edd The data descriptor to add the type to.
    * @param struct_type The type of the struct.
    * @param name The string name to use to encode/decode this member (must be a constant global and never change).
    * @param member The struct member itself to be encoded.
    * @param subtype The type of sub-type struct to add.
    *
    * This macro lets you easily add a sub-type (a struct that's pointed to
    * by this one). All the parameters are the same as for
    * EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @p subtype being the exception.
    * This must be the data descriptor of the struct that is pointed to by
    * this element.
    *
    * @since 1.0.0
    */
#define EET_DATA_DESCRIPTOR_ADD_SUB(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNKNOWN, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, /* 0,  */NULL, subtype); \
     }

   /**
    * Add a linked list type to a data descriptor
    * @param edd The data descriptor to add the type to.
    * @param struct_type The type of the struct.
    * @param name The string name to use to encode/decode this member (must be a constant global and never change).
    * @param member The struct member itself to be encoded.
    * @param subtype The type of linked list member to add.
    *
    * This macro lets you easily add a linked list of other data types. All the
    * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
    * @p subtype being the exception. This must be the data descriptor of the
    * element that is in each member of the linked list to be stored.
    *
    * @since 1.0.0
    */
#define EET_DATA_DESCRIPTOR_ADD_LIST(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_LIST, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, /* 0,  */NULL, subtype); \
     }

   /**
    * Add a hash type to a data descriptor
    * @param edd The data descriptor to add the type to.
    * @param struct_type The type of the struct.
    * @param name The string name to use to encode/decode this member (must be a constant global and never change).
    * @param member The struct member itself to be encoded.
    * @param subtype The type of hash member to add.
    *
    * This macro lets you easily add a hash of other data types. All the
    * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
    * @p subtype being the exception. This must be the data descriptor of the
    * element that is in each member of the hash to be stored.
    *
    * @since 1.0.0
    */
#define EET_DATA_DESCRIPTOR_ADD_HASH(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_HASH, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, /* 0,  */NULL, subtype); \
     }

   /**
    * Add a fixed size array type to a data descriptor
    * @param edd The data descriptor to add the type to.
    * @param struct_type The type of the struct.
    * @param name The string name to use to encode/decode this member (must be a constant global and never change).
    * @param member The struct member itself to be encoded.
    * @param subtype The type of hash member to add.
    *
    * This macro lets you easily add a fixed size array of other data types. All the
    * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
    * @p subtype being the exception. This must be the data descriptor of the
    * element that is in each member of the hash to be stored.
    *
    * @since 1.0.2
    */
#define EET_DATA_DESCRIPTOR_ADD_ARRAY(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_ARRAY, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					/* 0,  */sizeof(___ett.member)/sizeof(___ett.member[0]), NULL, subtype); \
     }

   /**
    * Add a variable size array type to a data descriptor
    * @param edd The data descriptor to add the type to.
    * @param struct_type The type of the struct.
    * @param name The string name to use to encode/decode this member (must be a constant global and never change).
    * @param member The struct member itself to be encoded.
    * @param subtype The type of hash member to add.
    *
    * This macro lets you easily add a fixed size array of other data types. All the
    * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
    * @p subtype being the exception. This must be the data descriptor of the
    * element that is in each member of the hash to be stored.
    *
    * @since 1.0.2
    */
#define EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_VAR_ARRAY, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					(char *)(&(___ett.member ## _count)) - (char *)(&(___ett)), /* 0,  */NULL, subtype); \
     }

/***************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
