#ifndef _EET_H
#define _EET_H

/***************************************************************************/

/*
 * EET - E file chunk reading/writing library
 * 
 * What is it?
 * It is a tiny library designed to write an arbitary set of chunks of data
 * to a file and optionally compress each chunk (very much like a zip file)
 * and allow fast random-access reading of the file later on. It does not
 * do zip as zip itself has more complexity than we need, and it was much
 * simpler to impliment this once here.
 * 
 */

/***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

#define EET_T_UNKNOW     0
#define EET_T_CHAR       1
#define EET_T_SHORT      2
#define EET_T_INT        3
#define EET_T_LONG_LONG  4
#define EET_T_FLOAT      5
#define EET_T_DOUBLE     6
#define EET_T_UCHAR      7
#define EET_T_USHORT     8
#define EET_T_UINT       9
#define EET_T_ULONG_LONG 10
#define EET_T_STRING     11
#define EET_T_LAST       12

#define EET_G_UNKNOWN    100
#define EET_G_ARRAY      101
#define EET_G_VAR_ARRAY  102
#define EET_G_LIST       103
#define EET_G_HASH       104
#define EET_G_LAST       105

/***************************************************************************/

   enum _Eet_File_Mode
     {
	EET_FILE_MODE_READ,
	EET_FILE_MODE_WRITE
     };
   
   typedef enum _Eet_File_Mode             Eet_File_Mode;
   
   typedef struct _Eet_File                Eet_File;
   typedef struct _Eet_Data_Descriptor     Eet_Data_Descriptor;


/***************************************************************************/
   
   /**
    * Open an eet file on disk, and returns a handle to it.
    * @param file The file path to the eet file. eg: "/tmp/file.eet".
    * @param mode The mode for opening. Either EET_FILE_MODE_READ or EET_FILE_MODE_WRITE, but not both.
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
    */   
   Eet_File *eet_open  (char *file, Eet_File_Mode mode);
   /**
    * Close an eet file handle and flush and writes pending.
    * @param ef A valid eet file handle.
    * 
    * This function will flush any pending writes to disk if the eet file
    * was opened for write, and free all data associated with the file handle
    * and file, and close the file.
    * 
    * If the eet file handle is not valid nothing will be done.
    */
   void      eet_close (Eet_File *ef);
   /**
    * Read a specified entry from an eet file and return data
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param size_ret Number of bytes read from entry and returned.
    * @return The data stored in that entry in the eet file.
    * 
    * This function finds an entry in the eet file that is stored under the
    * name specified, and returns that data, decompressed, if successfule.
    * NULL is retuurned if the lookup fails or if memory errors are
    * encountered. It is the job of the calling program to call free() on
    * the returned data. The number of bytes in the returned data chunk are
    * placed in size_ret.
    * 
    * If the eet file handle is not valid NULl is returned and size_ret is
    * filled with 0.
    */
   void     *eet_read  (Eet_File *ef, char *name, int *size_ret);
   /**
    * Write a specified entry to an eet file handle
    * @param ef A valid eet file handle opened for writing.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param data Pointer to the data to be stored.
    * @param size Length in bytes in the data to be stored.
    * @param compress Compression flags (1 == compress, 0 = don't compress).
    * @retrun Success or failure of the write.
    * 
    * This function will write the specified chunk of data to the eet file
    * and return 1 on success. 0 will be returned on failure.
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
    */
   int       eet_write (Eet_File *ef, char *name, void *data, int size, int compress);
   /**
    * List all entries in eet file matching shell glob.
    * @param ef A valid eet file handle.
    * @param glob A shell glob to match against.
    * @param count_ret Number of entries foudn to match.
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
    */
   char    **eet_list  (Eet_File *ef, char *glob, int *count_ret);

/***************************************************************************/

   /**
    * Read image data from the named key in the eet file.
    * @param ef A valid eet file handle opened for reading.
    * @param name Name of the entry. eg: "/base/file_i_want".
    * @param w A pointer to the int to hold the width in pixels.
    * @param h A pointer to the int to hold the height in pixels.
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
    * row from left to right. Each piel is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denoties that it is significant.
    * Compress is fiulled with the compression value/amount the image was
    * stored with. The quality value si fileld with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    * 
    * On success the function returns a pointer to the image data decoded. The
    * calling application is responsible for calling free() on the image data
    * when it is done with it. On failure NULL is returned and the parameter
    * values may not contain any sensible data.
    */
   void     *eet_data_image_read(Eet_File *ef, char *name, int *w, int *h, int *alpha, int *compress, int *quality, int *lossy);
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
    * If this is not the case wierd things may happen. Width and height must
    * be between 1 and 8000 pixels. The alpha flags can be 0 or 1 (0 meaning
    * the alpha values are not useful and 1 meaning they are). Compress can
    * be from 0 to 9 (0 meaning no compression, 9 meaning full compression).
    * This is only used if the image is not lossily encoded. Quality is used on
    * lossy compression and shoudl be a value from 0 to 100. The lossy flag
    * can be 0 or 1. 0 means encode losslessly and 1 means to encode with
    * image quality loss (but then have a much smaller encoding).
    * 
    * On success this function rtuens the numebr fo bytes that were required
    * to encode the image data, or on failure it returns 0.
    */   
   int       eet_data_image_write(Eet_File *ef, char *name, void *data, int w, int h, int alpha, int compress, int quality, int lossy);
   /**
    * Decode Image data into pixel data.
    * @param data The encoded pixel data.
    * @param size The size, in bytes, of the encoded pixel data.
    * @param w A pointer to the int to hold the width in pixels.
    * @param h A pointer to the int to hold the height in pixels.
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
    * row from left to right. Each piel is a 32bit value, with the high byte
    * being the alpha channel, the next being red, then green, and the low byte
    * being blue. The width and height are measured in pixels and will be
    * greater than 0 when returned. The alpha flag is either 0 or 1. 0 denotes
    * that the alpha channel is not used. 1 denoties that it is significant.
    * Compress is fiulled with the compression value/amount the image was
    * stored with. The quality value si fileld with the quality encoding of
    * the image file (0 - 100). The lossy flags is either 0 or 1 as to if
    * the image was encoded lossily or not.
    * 
    * On success the function returns a pointer to the image data decoded. The
    * calling application is responsible for calling free() on the image data
    * when it is done with it. On failure NULL is returned and the parameter
    * values may not contain any sensible data.
    */
   void     *eet_data_image_decode(void *data, int size, int *w, int *h, int *alpha, int *compress, int *quality, int *lossy);
   /**
    * Encode image data for sotrage or transmission.
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
    * possible loss of quality (as a trade off for size) for sotrage or
    * transmision to another system.
    * 
    * The data expected is the same format as returned by eet_data_image_read.
    * If this is not the case wierd things may happen. Width and height must
    * be between 1 and 8000 pixels. The alpha flags can be 0 or 1 (0 meaning
    * the alpha values are not useful and 1 meaning they are). Compress can
    * be from 0 to 9 (0 meaning no compression, 9 meaning full compression).
    * This is only used if the image is not lossily encoded. Quality is used on
    * lossy compression and shoudl be a value from 0 to 100. The lossy flag
    * can be 0 or 1. 0 means encode losslessly and 1 means to encode with
    * image quality loss (but then have a much smaller encoding).
    * 
    * On success this function returns a pointer to the encoded data that you
    * can free with free() wehn no longer needed.
    */   
   void     *eet_data_image_encode(void *data, int *size_ret, int w, int h, int alpha, int compress, int quality, int lossy);
/***************************************************************************/
   
   /* 
    * To Be Documented
    */
   Eet_Data_Descriptor *eet_data_descriptor_new(char *name, int size, void *(*func_list_next) (void *l), void *(*func_list_append) (void *l, void *d), void *(*func_list_data) (void *l), void  (*func_hash_foreach) (void *h, int (*func) (void *h, const char *k, void *dt, void *fdt), void *fdt), void *(*func_hash_add) (void *h, const char *k, void *d));
   void                 eet_data_descriptor_free(Eet_Data_Descriptor *edd);
   
   void  eet_data_descriptor_element_add(Eet_Data_Descriptor *edd, char *name, int type, int group_type, int offset, int count, char *counter_name, Eet_Data_Descriptor *subtype);
   
   void *eet_data_read(Eet_File *ef, Eet_Data_Descriptor *edd, char *name);
   int   eet_data_write(Eet_File *ef, Eet_Data_Descriptor *edd, char *name, void *data, int compress);
   
   void *eet_data_descriptor_decode(Eet_Data_Descriptor *edd, void *data_in, int size_in);
   void *eet_data_descriptor_encode(Eet_Data_Descriptor *edd, void *data_in, int *size_ret);

#define EET_DATA_DESCRIPTOR_ADD_BASIC(edd, struct_type, name, member, type) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, type, EET_G_UNKNOWN, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, NULL, NULL); \
     }
#define EET_DATA_DESCRIPTOR_ADD_SUB(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNKNOWN, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, NULL, subtype); \
     }
#define EET_DATA_DESCRIPTOR_ADD_LIST(edd, struct_type, name, member, subtype) \
     { \
	struct_type ___ett; \
	\
	eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_LIST, \
					(char *)(&(___ett.member)) - (char *)(&(___ett)), \
					0, NULL, subtype); \
     }
   
/***************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
