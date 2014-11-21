/**
 * @internal
   @defgroup Eet_Group Eet
   @ingroup EFL_Group

   @brief Eet Data Handling Library Public API Calls

   These routines are used for Eet Library interaction.

   @page eet_main Eet

   @date 2000 (created)

   @section toc Table of Contents

   @li @ref eet_main_intro
   @li @ref eet_main_next_steps
   @li @ref eet_main_intro_example

   @section eet_main_intro Introduction

   It is a tiny library designed to write an arbitrary set of chunks of data
   to a file and optionally compress each chunk (very much like a zip file)
   and allow fast random-access reading of the file later on. It does not
   do zip, as a zip itself has more complexity than is needed, and it is much
   simpler to implement this once here.

   Eet is extremely fast, small and simple. Eet files can be very small and
   highly compressed, making them very optimal for just sending across the
   internet without having to archive, compress or decompress and install them.
   They allow for lightning-fast random-access reads once created, making them
   perfect for storing data that is written once (or rarely) and read many
   times, but the program does not want to have to read it all in at once.

   It also can encode and decode data structures in memory, as well as image
   data for saving to Eet files or sending across the network to other
   machines, or just writing to arbitrary files on the system. All data is
   encoded in a platform independent way and can be written and read by any
   architecture.

   @section eet_main_next_steps Next Steps

   After you understood what Eet is and installed it in your system you
   should understand the programming interface. We would recommend
   you to take a while to learn @ref Eina_Group as it is very convenient and 
   optimized, and Eet provides integration with it.

   Recommended reading:

   @li @ref Eet_File_Group to know the basics to open and save files.
   @li @ref Eet_Data_Group to know the convenient way to serialize and
       parse your data structures automatically. Just create your
       descriptors and let Eet do the work for you.

   @section eet_main_intro_example Introductory Examples

   Here is a simple example on how to use Eet to save a series of strings to a
   file and load them again. The advantage of using Eet over just
   fprintf() and
   fscanf() is that not only can these entries be strings, they need no special
   parsing to handle delimiter characters or escaping, they can be binary data,
   image data, data structures containing integers, strings, other data
   structures, linked lists and much more, without the programmer having to
   worry about parsing, and best of all, Eet is very fast.

   This is just a very simple example that doesn't show all of the capabilities
   of Eet, but it serves to illustrate its simplicity.

   @include eet-basic.c

   More examples can be found at @ref eet_examples.

   @todo Document data format for images and data structures.

 */

#ifndef _EET_H
#define _EET_H

#include <stdlib.h>
#include <stdio.h>
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif /* ifdef EAPI */

#ifdef _WIN32
# ifdef EFL_EET_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else /* ifdef DLL_EXPORT */
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else /* ifdef EFL_EET_BUILD */
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EET_BUILD */
#else /* ifdef _WIN32 */
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else /* if __GNUC__ >= 4 */
#   define EAPI
#  endif /* if __GNUC__ >= 4 */
# else /* ifdef __GNUC__ */
#  define EAPI
# endif /* ifdef __GNUC__ */
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @internal
 * @file Eet.h
 * @brief The file that provides the eet functions.
 *
 * @details This header provides the Eet management functions.
 *
 */

/**
 * @internal
 * @defgroup Eet_General_Group Top level functions
 * @ingroup Eet_Group
 *
 * @brief This group provides functions that affect Eet as a whole.
 *
 * @{
 */

#define EET_VERSION_MAJOR 1
#define EET_VERSION_MINOR 8
/**
 * @typedef Eet_Version
 *
 * This is the Eet version information structure that can be used at
 * runtime to detect which version of eet is being used and adapt
 * appropriately as follows for example:
 *
 * @code
 * #if defined(EET_VERSION_MAJOR) && (EET_VERSION_MAJOR >= 1) && defined(EET_VERSION_MINOR) && (EET_VERSION_MINOR > 2)
 * printf("Eet version: %i.%i.%i\n",
 *        eet_version->major,
 *        eet_version->minor,
 *        eet_version->micro);
 * if (eet_version->revision > 0)
 *   {
 *     printf("  Built from SVN revision # %i\n", eet_version->revision);
 *   }
 * #endif
 * @endcode
 *
 * Note the \#if check can be dropped if your program refuses to compile or
 * work with an Eet version less than 1.3.0.
 */
typedef struct _Eet_Version
{
   int major; /**< Major (binary or source incompatible changes) */
   int minor; /**< Minor (new features, bugfixes, major improvements version) */
   int micro; /**< Micro (bugfix, internal improvements, no new features version) */
   int revision; /**< svn revision (@c 0 if a proper release or the svn revision number Eet is built from) */
} Eet_Version;

EAPI extern Eet_Version *eet_version;

/**
 * @internal
 * @enum _Eet_Error
 * @brief Enumeration of all the error identifiers known by Eet.
 */
typedef enum _Eet_Error
{
   EET_ERROR_NONE, /**< No error, it is all fine! */
   EET_ERROR_BAD_OBJECT, /**< Given object or handle is @c NULL or invalid */
   EET_ERROR_EMPTY, /**< There is nothing to do */
   EET_ERROR_NOT_WRITABLE, /**< Could not write to file or file is #EET_FILE_MODE_READ */
   EET_ERROR_OUT_OF_MEMORY, /**< Could not allocate memory */
   EET_ERROR_WRITE_ERROR, /**< Failed to write data to destination */
   EET_ERROR_WRITE_ERROR_FILE_TOO_BIG, /**< Failed to write file since it is too big */
   EET_ERROR_WRITE_ERROR_IO_ERROR, /**< Failed to write due a generic Input/Output error */
   EET_ERROR_WRITE_ERROR_OUT_OF_SPACE, /**< Failed to write due out of space */
   EET_ERROR_WRITE_ERROR_FILE_CLOSED, /**< Failed to write because file is closed */
   EET_ERROR_MMAP_FAILED, /**< Could not mmap file */
   EET_ERROR_X509_ENCODING_FAILED, /**< Could not encode using X509 */
   EET_ERROR_SIGNATURE_FAILED, /**< Could not validate signature */
   EET_ERROR_INVALID_SIGNATURE, /**< Signature is invalid */
   EET_ERROR_NOT_SIGNED, /**< File or contents are not signed */
   EET_ERROR_NOT_IMPLEMENTED, /**< Function is not implemented */
   EET_ERROR_PRNG_NOT_SEEDED, /**< Could not introduce random seed */
   EET_ERROR_ENCRYPT_FAILED, /**< Could not encrypt contents */
   EET_ERROR_DECRYPT_FAILED /**< Could not decrypt contents */
} Eet_Error; /**< Eet error identifiers */

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_Compression_Group Eet Compression Levels
 * @ingroup Eet_General_Group
 *
 * Compression modes/levels supported by Eet.
 *
 * @{
 */

/**
 * @enum _Eet_Compression
 * @brief Enumeration for the compression modes known by Eet.
 */

typedef enum _Eet_Compression
{
   EET_COMPRESSION_NONE      = 0,  /**< No compression at all @since 1.7 */
   EET_COMPRESSION_DEFAULT   = 1,  /**< Default compression (Zlib) @since 1.7 */
   EET_COMPRESSION_LOW       = 2,  /**< Fast but minimal compression (Zlib) @since 1.7 */
   EET_COMPRESSION_MED       = 6,  /**< Medium compression level (Zlib) @since 1.7 */
   EET_COMPRESSION_HI        = 9,  /**< Slow but high compression level (Zlib) @since 1.7 */
   EET_COMPRESSION_VERYFAST  = 10, /**< Very fast, but lower compression ratio (LZ4HC) @since 1.7 */
   EET_COMPRESSION_SUPERFAST = 11, /**< Very fast, but lower compression ratio (faster to compress than EET_COMPRESSION_VERYFAST)  (LZ4) @since 1.7 */

   EET_COMPRESSION_LOW2      = 3,  /**< Space filler for compatibility. Do not use it @since 1.7 */
   EET_COMPRESSION_MED1      = 4,  /**< Space filler for compatibility. Do not use it @since 1.7 */
   EET_COMPRESSION_MED2      = 5,  /**< Space filler for compatibility. Do not use it @since 1.7 */
   EET_COMPRESSION_HI1       = 7,  /**< Space filler for compatibility. Do not use it @since 1.7 */
   EET_COMPRESSION_HI2       = 8   /**< Space filler for compatibility. Do not use it @since 1.7 */
} Eet_Compression; /**< Eet compression modes @since 1.7 */

/**
 * @}
 */

/**
 * @brief   Initializes the EET library.
 * @since   1.0.0
 *
 * @remarks The first time this function is called, it performs all the internal
 *          initialization required for the library to function properly and increment
 *          the initialization counter. Any subsequent call only increment this counter
 *          and return its new value. So it is safe to call this function more than once.
 *
 * @return  The new init count, \n 
 *          otherwise @c 0 if initialization failed
 *
 * @ingroup Eet_General_Group
 */
EAPI int
eet_init(void);

/**
 * @brief   Shuts down the EET library.
 * @since   1.0.0
 *
 * @remarks If eet_init() is called more than once for the running application,
 *          eet_shutdown() decrements the initialization counter and return its
 *          new value, without doing anything else. When the counter reaches @c 0, all
 *          of the internal elements are shutdown and any memory used is freed.
 *
 * @return  The new init count
 * @ingroup Eet_Group
 */
EAPI int
eet_shutdown(void);

/**
 * @brief   Clears eet cache.
 * @since   1.0.0
 *
 * @remarks For a faster access to previously accessed data, Eet keeps an internal
 *          cache of files. These files are freed automatically only when
 *          they are unused and the cache gets full, in order based on the last time
 *          they were used. On systems with little memory this may present an unnecessary 
 *          constraint. So eet_clearcache() is available for users to reclaim the memory 
 *          used by files that are no longer needed. Those that were open using
 *          ::EET_FILE_MODE_WRITE or ::EET_FILE_MODE_READ_WRITE and have modifications,
 *          are written down to disk before flushing them from memory.
 *
 * @ingroup Eet_General_Group
 */
EAPI void
eet_clearcache(void);

/**
 * @internal
 * @defgroup Eet_File_Group Eet File Main Functions
 * @ingroup Eet_Group
 *
 * @brief   This group provides functions to create, destroy, and do basic manipulation of
 *          #Eet_File handles.
 *
 * This sections explains how to use the most basic Eet functions, which
 * are used to work with eet files, read data from them, store it back in, or
 * take a look at what entries it contains, without making use of the
 * serialization capabilities explained in @ref Eet_Data_Group.
 *
 * The following example serves as an introduction to most, if not all,
 * of these functions.
 *
 * If you are only using Eet, this is the only header you need to include.
 * @dontinclude eet-file.c
 * @skipline Eet.h
 *
 * Now create an eet file to play with. The following function
 * shows step by step how to open a file and write some data in it.
 * First, define your file handler and some other things that you put in it.
 * @line static int
 * @skip Eet_File
 * @until ";
 * @skip eet_open
 *
 * Open a new file in write mode, and if it fails, just return, since
 * there is not much more you can do about it.
 * @until return
 *
 * Now, write some data in your file. For now, strings suffice.
 * So just dump a bunch of them in there.
 * @until }
 *
 * As you can see, you copied a string into your static buffer, which is a bit
 * bigger than the full length of the string, and then told Eet to write it
 * into the file, compressed, returning the size of the data written into the
 * file.
 * This is all to show that Eet treats data as just data. It does not matter
 * what that data represents (for now). It is all just bytes for it. As running
 * the following code shows, you took a string of around 30 bytes and put it
 * in a buffer of 1024 bytes, but the returned size is not any of those.
 * @until printf
 *
 * Next, copy into your buffer your set of strings, including their null
 * terminators and write them into the file. No error checking for the sake
 * of brevity. And a call to eet_sync() to make sure all out data is
 * properly written down to disk, even though you have not yet closed the file.
 * @until eet_sync
 *
 * One more write, this time your large array of binary data and... well, 
 * a valid use of the last set of strings you stored is not identified here,
 * so take it out from the file with eet_delete().
 * @until eet_delete
 *
 * Finally, close the file, saving any changes back to disk and return.
 * Notice how, if there is any error closing the file or saving its contents,
 * the return value from the function is a false one, which later on
 * makes the program exit with an error code.
 * @until return
 *
 * Moving onto your main function, you open the same file and read it back.
 * Trivial, but it shows how you can do so in more than one way. You skip
 * the variable declarations, as they are not very different from what you have
 * seen already.
 *
 * You start from the beginning by initializing Eet so things in general work.
 * Forgetting to do so results in weird results or crashes when calling
 * any eet function, so if you experience something like that, the first thing
 * to look at is whether eet_init() is missing.
 * Then call your @a create_eet_file function, described above, to make
 * sure you have something to work with. If the function fails it returns
 * @c 0 and just exits, since nothing from here onwards works anyway.
 * @skip eet_init
 * @until return
 *
 * Take a look now at what entries our file has. For this, use
 * eet_list(), which returns a list of strings, each being the name of
 * one entry. Since this is skipped before, it may be worth noting that @a list
 * is declared as a @a char **.
 * The @a num parameter, of course, has the number of entries contained
 * in our file.
 * If everything is fine, you get your list and print it to the screen, and
 * once done with it, free the list. That is just the list, not its contents,
 * as they are internal strings used by Eet and trying to free them surely
 * breaks things.
 * @until }
 *
 * Reading back plain data is simple. Just a call to eet_read() with the file
 * to read from, and the name of the entry you are interested in. You get back
 * your data and the passed @a size parameter contains the size of it. If
 * the data is stored compressed, it decompresses first.
 * @until }
 *
 * Another simple read for the set of strings from before, except those were
 * deleted, so you should get a @c NULL return and continue normally.
 * @until }
 *
 * Finally, you get your binary data in the same way you got the strings. Once
 * again, it makes no difference for Eet what the data is, it is up to you to
 * know how to handle it.
 * @until {
 *
 * Now some cheating, you know that this data is an Eet file because, well...
 * you just know it. So you are going to open it and take a look at its insides.
 * For this, eet_open() does not work, as it needs to have a file on disk to read
 * from and all you have is some data in RAM.
 *
 * So how to do this? One way would be to create a normal file and write down
 * your data, then open it with eet_open(). Another, faster and more efficient way,
 * if all you want to do is read the file, is to use eet_memopen_read().
 * @until memopen
 *
 * As you can see, the size you got from your previous read is put to good use
 * this time. Unlike the first one where all you had were strings, the size
 * of the data read only serves to demonstrate that you are reading back the
 * entire size of your original @a buf variable.
 *
 * A little peeking to see how many entries the file has and to make an
 * example of eet_num_entries() to get that number when you do not care about
 * their names.
 * @until printf
 *
 * More cheating follows. Just like you knew this is an Eet file, you also know
 * what key to read from, and on top of that you know that the data in it is not
 * compressed.
 * Knowing all this allows you to take some shortcuts.
 * @until read_direct
 *
 * That is a direct print of your data, whatever that data is. We do not want
 * to worry about having to free it later, so just used eet_direct_read()
 * to tell Eet to gives a pointer to the internal data in the file, without
 * duplicating it. Since you said that data is not compressed, you should not
 * worry about printing garbage to the screen (and yes, you also know the data
 * is yet another string).
 * You also do not care about the size of the data as it is stored in the file,
 * so you passed @c NULL as the size parameter.
 * One very important note about this, however, is that you do not care about
 * the size parameter because the data in the file contains the null
 * terminator for the string. So when using Eet to store strings this way,
 * it is very important to consider whether you keep that final null
 * byte, or to always get the size read and do the necessary checks and copies.
 * It is up to the user and the particular use cases to decide how this is
 * done.
 *
 * With everything done, close this second file and free the data used to open
 * it. And this is important, you cannot free that data until you are done with
 * the file, as Eet is using it. When opening with eet_memopen_read(), the data
 * passed to it must be available for as long as the the file is open.
 * @until }
 *
 * Finally, close the first file, shutdown all internal resources used by
 * Eet and leave our main function, thus terminating our program.
 * @until return
 *
 * You can look at the full code of the example @ref eet-file.c "here".
 * @{
 */

/**
 * @enum _Eet_File_Mode
 * @brief Enumeration for modes in which a file can be opened.
 */
typedef enum _Eet_File_Mode
{
   EET_FILE_MODE_INVALID = -1,
   EET_FILE_MODE_READ, /**< File is read-only */
   EET_FILE_MODE_WRITE, /**< File is write-only */
   EET_FILE_MODE_READ_WRITE /**< File is for both read and write */
} Eet_File_Mode; /**< Modes that a file can be opened */

/**
 * @enum _Eet_Image_Encoding
 * @brief Enumeration for lossy encoding for image.
 * @remarks	Backported from EFL 1.10 and EFL 1.11
 */
typedef enum _Eet_Image_Encoding
{
   EET_IMAGE_LOSSLESS = 0,
   EET_IMAGE_JPEG = 1,
   EET_IMAGE_ETC1 = 2,
   EET_IMAGE_ETC2_RGB = 3,
   EET_IMAGE_ETC2_RGBA = 4,
   EET_IMAGE_ETC1_ALPHA = 5,
} Eet_Image_Encoding;

typedef enum _Eet_Colorspace
{
   EET_COLORSPACE_ARGB8888 = 0,
   /* The number between are reserved to preserve compatibility with evas */
   EET_COLORSPACE_ETC1 = 9,
   EET_COLORSPACE_RGB8_ETC2 = 10,
   EET_COLORSPACE_RGBA8_ETC2_EAC = 11,
   EET_COLORSPACE_ETC1_ALPHA = 12
} Eet_Colorspace;

/**
 * @typedef Eet_File
 * @brief   The structure type containing an opaque handle that defines an Eet file (or memory).
 *
 * This handle is returned by the functions eet_open() and
 * eet_memopen_read() and is used by every other function that affects the
 * file in any way. When you are done with it, call eet_close() to close it
 * and, if the file is open for writing, write down to disk any changes made
 * to it.
 *
 * @see eet_open()
 * @see eet_memopen_read()
 * @see eet_close()
 */
typedef struct _Eet_File Eet_File;

/**
 * @typedef Eet_Dictionary
 * @brief The structure type containing an opaque handle that defines a file-backed (mmaped) dictionary of strings.
 */
typedef struct _Eet_Dictionary Eet_Dictionary;

/**
 * @brief   Opens an eet file on disk, and returns a handle to it.
 * @since   1.0.0
 *
 * @remarks This function opens an exiting eet file for reading, and build
 *          the directory table in memory and return a handle to the file, if it
 *          exists and can be read, and no memory errors occur on the way, otherwise
 *          @c NULL is returned.
 *
 * @remarks It also opens an eet file for writing. This, if successful,
 *          deletes the original file and replace it with a new empty file, till
 *          the eet file handle is closed or flushed. If it cannot be opened for
 *          writing or a memory error occurs, @c NULL is returned.
 *
 * @remarks You can also open the file for read/write. If you then write a key that
 *          does not exist it is created, if the key exists it is replaced
 *          by the new data.
 *
 * @remarks If the same file is opened multiple times, then the same file handle
 *          is returned as eet maintains an internal list of all currently open
 *          files. Note that it considers files opened for read only and those opened
 *          for read/write and write only as two separate sets. Those that do not write
 *          to the file and those that do. Eet allows two handles to the same file
 *          if they are in the two separate lists/groups. That means opening a file for
 *          read only looks in the read only set, and returns a handle to that file
 *          handle and increments its reference count. If you open a file for read/write
 *          or write only, it looks in the write set and returns a handle after
 *          incrementing the reference count. You need to close an eet file handle
 *          as many times as it has been opened to maintain correct reference counts.
 *          Files whose modified timestamp or size do not match those of the existing
 *          referenced file handles are not returned and a new handle is returned instead.
 *
 * @param[in]   file  The file path to the eet file. eg: @c "/tmp/file.eet"
 * @param[in]   mode  The mode for opening. Either #EET_FILE_MODE_READ,
 *                #EET_FILE_MODE_WRITE or #EET_FILE_MODE_READ_WRITE
 * @return  An opened eet file handle
 */
EAPI Eet_File *
eet_open(const char *file,
         Eet_File_Mode mode);

/**
 * @brief   Opens an eet file directly from a memory location. 
 *
 * @since   1.1.0
 *
 * @remarks The data is not copied, so you must keep it around 
 *          as long as the eet file is open. There is
 *          currently no cache for this kind of Eet_File, 
 *          so it is reopened every time you use eet_memopen_read.
 *
 * @remarks Files opened this way are always in read-only mode.
 *
 * @param[in]   data    The address of file in memory
 * @param[in]   size    The size of memory to be read
 * @return  A handle to the file
 */
EAPI Eet_File *
eet_memopen_read(const void *data,
                 size_t size);

/**
 * @brief   Gets the mode an Eet_File is opened with.
 *
 * @since   1.0.0
 *
 * @param[in]   ef  A valid eet file handle
 * @return  The mode ef is opened with
 */
EAPI Eet_File_Mode
eet_mode_get(Eet_File *ef);

/**
 * @brief   Closes an eet file handle and flush pending writes.
 *
 * @details This function flushes any pending writes to disk if the eet file
 *          is opened for write, and free all data associated with the file handle
 *          and file, and close the file. If it is opened for read (or read/write),
 *          the file handle may still be held open internally for caching purposes.
 *          To flush speculatively held eet file handles use eet_clearcache().
 *
 * @since   1.0.0
 *
 * @remarks If the eet file handle is not valid nothing is done.
 *
 * @param[in]   ef  A valid eet file handle
 * @return  An eet error identifier
 * 
 * @see eet_clearcache()
 */
EAPI Eet_Error
eet_close(Eet_File *ef);

/**
 * @brief   Syncs content of an eet file handle, flushing pending writes.
 * @details This function flushes any pending writes to disk. The eet file must
 *          be opened for write.
 *
 * @since   1.2.4
 *
 * @remarks If the eet file handle is not valid nothing is done.
 *
 * @param[in]   ef  A valid eet file handle
 * @return  An eet error identifier
 */
EAPI Eet_Error
eet_sync(Eet_File *ef);

/**
 * @brief   Gets a handle to the shared string dictionary of the Eet file.
 *
 * @details This function returns a handle to the dictionary of an Eet file whose
 *          handle is @a ef, if a dictionary exists. @c NULL is returned otherwise or
 *          if the file handle is known to be invalid.
 *
 * @since   1.0.0
 *
 * @param[in]   ef  A valid eet file handle
 * @return  A handle to the dictionary of the file
 * @see eet_dictionary_string_check() to know if given string came
 *      from the dictionary or it is dynamically allocated using
 *      the #Eet_Data_Descriptor_Class instructions.
 */
EAPI Eet_Dictionary *
eet_dictionary_get(Eet_File *ef);

/**
 * @brief   Checks whether a given string comes from a given dictionary.
 *
 * @details This checks the given dictionary to see if the given string is actually
 *          inside that dictionary (i.e. comes from it) and returns @c 1 if it does.
 *          If the dictionary handle is invalid, the string is @c NULL or the string is
 *          not in the dictionary, @c 0 is returned.
 *
 * @since   1.0.0
 *
 * @param[in]   ed      A valid dictionary handle
 * @param[in]   string  A valid @c 0 byte terminated C string
 * @return  @c 1 if it is in the dictionary, \n
 *          otherwise @c 0 if it is not in the dictionary
 */
EAPI int
eet_dictionary_string_check(Eet_Dictionary *ed,
                            const char *string);

/**
 * @brief   Gets the number of strings inside a dictionary.
 * @since   1.6.0
 *
 * @param[in]   ed  A valid dictionary handle
 * @return  The number of strings inside a dictionary
 */
EAPI int
eet_dictionary_count(const Eet_Dictionary *ed);

/**
 * @brief   Reads a specified entry from an eet file and returns data.
 *
 * @details This function finds an entry in the eet file that is stored under the
 *          name specified, and returns that data, decompressed, if successful.
 *          @c NULL is returned if the lookup fails or if memory errors are
 *          encountered. It is the job of the calling program to call free() on
 *          the returned data. The number of bytes in the returned data chunk are
 *          placed in size_ret.
 *
 * @since   1.0.0
 *
 * @remarks If the eet file handle is not valid @c NULL is returned and @a size_ret is
 *          filled with @c 0.
 *
 * @param[in]   ef        A valid eet file handle opened for reading
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[out]   size_ret  The number of bytes read from entry and returned
 * @return  The data stored in that entry in the eet file
 *
 * @see eet_read_cipher()
 */
EAPI void *
eet_read(Eet_File *ef,
         const char *name,
         int *size_ret);

/**
 * @brief   Reads a specified entry from an eet file and returns the data.
 *
 * @details This function finds an entry in the eet file that is stored under the
 *          name specified, and returns that data if not compressed and successful.
 *          @c NULL is returned if the lookup fails or if memory errors are
 *          encountered or if the data is compressed. The calling program must never
 *          call free() on the returned data. The number of bytes in the returned
 *          data chunk are placed in size_ret.
 * @since   1.0.0
 *
 * @remarks If the eet file handle is not valid, @c NULL is returned and @a size_ret is
 *          filled with @a 0.
 * @param[in]   ef        A valid eet file handle opened for reading.
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[out]   size_ret  The number of bytes read from entry and returned
 * @return  The data stored in that entry in the eet file
 */
EAPI const void *
eet_read_direct(Eet_File *ef,
                const char *name,
                int *size_ret);

/**
 * @brief   Writes a specified entry to an eet file handle.
 *
 * @details This function writes the specified chunk of data to the eet file
 *          and return greater than @c 0 on success. @c 0 is returned on failure.
 *
 * @since   1.0.0
 *
 * @remarks The eet file handle must be a valid file handle for an eet file opened
 *          for writing. If it is not, @c 0 is returned and no action is
 *          performed.
 *
 * @remarks Name and data must not be @c NULL, and size must be > @c 0. If these
 *          conditions are not met, @c 0 is returned.
 *
 * @remarks The data is copied (and optionally compressed) in RAM, pending
 *          a flush to disk (it stays in RAM till the eet file handle is
 *          closed though).
 *
 * @param[in]   ef        A valid eet file handle opened for writing
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[in]   data      The pointer to the data to be stored
 * @param[in]   size      The length in bytes in the data to be stored
 * @param[in]   compress  The compression flags (1 = compress, 0 = do not compress)
 * @return  The bytes written on successful write, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_write_cipher()
 */
EAPI int
eet_write(Eet_File *ef,
          const char *name,
          const void *data,
          int size,
          int compress);

/**
 * @brief   Deletes a specified entry from an Eet file being written or re-written.
 *
 * @details This function deletes the specified chunk of data from the eet file
 *          and return greater than @c 0 on success. @c 0 is returned on failure.
 *
 * @since   1.0.0
 *
 * @remarks The eet file handle must be a valid file handle for an eet file opened
 *          for writing. If it is not, @c 0 is returned and no action is performed.
 *
 * @remarks Name must not be @c NULL, otherwise @c 0 is returned.
 *
 * @param[in]   ef    A valid eet file handle opened for writing
 * @param[in]   name  The name of the entry \n
 *                For example: "/base/file_i_want".
 * @return  A number > @c 0 if the entry is deleted successfully, \n 
 *          otherwise @c 0 on failure
 */
EAPI int
eet_delete(Eet_File *ef,
           const char *name);

/**
 * @brief   Aliases a specific section to another one. 
 * @since   1.3.3
 *
 * @remarks Destination may exist or not, no checks are done.
 *
 * @remarks Name and Destination must not be @c NULL, otherwise @c EINA_FALSE is returned.
 *          The equivalent of this would be calling 'ln -s destination name'.
 *
 * @param[in]   ef           A valid eet file handle opened for writing
 * @param[in]   name         The name of the new entry \n 
 *                       For example: "/base/file_i_want".
 * @param[in]   destination  The actual source of the aliased entry \n
 *                       For example: "/base/the_real_stuff_i_want".
 * @param[in]   compress     The compression flags (1 == compress, 0 = do not compress)
 * @return  @c EINA_TRUE if it is aliased successfully, \n
 *          otherwise @c EINA_FALSE on failure
 */
EAPI Eina_Bool
eet_alias(Eet_File *ef,
          const char *name,
          const char *destination,
          int compress);

/**
 * @brief   Gets the filename of an Eet_File.
 * @since   1.6
 *
 * @remarks This function returns @c NULL for files opened with eet_memopen_read()
 *
 * @param[in]   ef  A valid eet file handle opened for writing
 * @return  The stringshared file string opened with eet_open(), \n
 *          otherwise @c NULL on error
 */
EAPI const char *
eet_file_get(Eet_File *ef);

/**
 * @brief   Gets the destination name of an alias.
 *
 * @since   1.5
 *
 * @remarks Name must not be @c NULL, otherwise @c NULL is returned.
 *
 * @param[in]   ef    A valid eet file handle opened for writing
 * @param[in]   name  The name of the entry \n 
 *                For example: "/base/file_i_want"
 * @return  The destination of the alias, example: "/base/the_real_stuff_i_want", \n
 *          otherwise @c NULL on failure
 */
EAPI const char *
eet_alias_get(Eet_File *ef,
              const char *name);

/**
 * @brief   Lists all entries in eet file matching shell glob.
 *
 * @details This function lists all entries in the eet file matching the
 *          supplied shell glob and return an allocated list of their names, if
 *          there are any, and if no memory errors occur.
 *
 * @since   1.0.0
 *
 * @remarks The eet file handle must be valid and glob must not be @c NULL, or @c NULL
 *          is returned and @a count_ret is filled with @c 0.
 *
 * @remarks The calling program must call free() on the array returned, but NOT
 *          on the string pointers in the array. They are taken as read-only
 *          internals from the eet file handle. They are only valid as long as
 *          the file handle is not closed. When it is closed those pointers in the
 *          array are now not valid and should not be used.
 *
 * @remarks On success the array returned has a list of string pointers
 *          that are the names of the entries that matched, and @a count_ret has
 *          the number of entries in this array placed in it.
 *
 * @remarks Hint: an easy way to list all entries in an eet file is to use a glob
 *          value of "*".
 *
 * @param[in]   ef         A valid eet file handle
 * @param[in]   glob       A shell glob to match against
 * @param[out]   count_ret  The number of entries found to match
 * @return  The pointer to an array of strings
 */
EAPI char **
eet_list(Eet_File *ef,
         const char *glob,
         int *count_ret);

/**
 * @brief   Gets the number of entries in the specified eet file.
 * @since   1.0.0
 *
 * @param[in]   ef  A valid eet file handle
 * @return  The number of entries in ef, \n 
 *          otherwise @c -1 if the number of entries cannot be read due to open mode restrictions
 */
EAPI int
eet_num_entries(Eet_File *ef);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_File_Cipher_Group Eet File Ciphered Main Functions
 * @ingroup Eet_File_Group
 *
 * Most of the @ref Eet_File_Group have alternative versions that
 * accounts for ciphers to protect their content.
 *
 * @see @ref Eet_Cipher_Group
 *
 * @{
 */

/**
 * @brief   Reads a specified entry from an eet file and returns the data using a cipher.
 *
 * @details This function finds an entry in the eet file that is stored under the
 *          name specified, and returns that data, decompressed, if successful.
 *          @c NULL is returned if the lookup fails or if memory errors are
 *          encountered. It is the job of the calling program to call free() on
 *          the returned data. The number of bytes in the returned data chunk are
 *          placed in size_ret.
 * @since   1.0.0
 *
 * @remarks If the eet file handle is not valid, @c NULL is returned and @a size_ret is
 *          filled with @c 0.
 *
 * @param[in]   ef          A valid eet file handle opened for reading
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[out]   size_ret    The number of bytes read from entry and returned
 * @param[in]   cipher_key  The key to use as cipher
 * @return  The data stored in that entry in the eet file, \n
 *          otherwise @c NULL on error
 *
 * @see eet_read()
 */
EAPI void *
eet_read_cipher(Eet_File *ef,
                const char *name,
                int *size_ret,
                const char *cipher_key);

/**
 * @brief   Writes a specified entry to an eet file handle using a cipher.
 *
 * @details This function writes the specified chunk of data to the eet file
 *          and return greater than @c 0 on success. @c 0 is returned on failure.
 * @since   1.0.0
 *
 * @remarks The eet file handle must be a valid file handle for an eet file opened
 *          for writing. If it is not, @c 0 is returned and no action is performed.
 *
 * @remarks @a name and @a data must not be @c NULL, and size must be > @c 0. If these
 *          conditions are not met, @c 0 is returned.
 *
 * @remarks The data is copied (and optionally compressed) in RAM, pending
 *          a flush to disk (it stays in RAM till the eet file handle is
 *          closed though).
 *
 * @param[in]   ef          A valid eet file handle opened for writing
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   data        The pointer to the data to be stored
 * @param[in]   size        The length in bytes in the data to be stored
 * @param[in]   compress    The compression flags (1 == compress, 0 = do not compress)
 * @param[in]   cipher_key  The key to use as cipher
 * @return  The bytes written on successful write, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_write()
 */
EAPI int
eet_write_cipher(Eet_File *ef,
                 const char *name,
                 const void *data,
                 int size,
                 int compress,
                 const char *cipher_key);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_File_Image_Group Image Store and Load
 * @ingroup Eet_File_Group
 *
 * @brief  This group provides functions for image storing and loading.
 *
 * Eet efficiently stores and loads images, including alpha
 * channels and lossy compressions.
 *
 * Eet can handle both lossy compression with different levels of quality and
 * non-lossy compression with different compression levels. It is also possible,
 * given an image data, to only read its header to get the image information
 * without decoding the entire content for it.
 *
 * The encode family of functions takes an image raw buffer and its
 * parameters and compress it in memory, returning the new buffer.
 * Likewise, the decode functions reads from the given location in memory
 * and return the uncompressed image.
 *
 * The read and write functions, respectively, encode and decode to or
 * from an Eet file, under the specified key.
 *
 * These functions are fairly low level and the same functionality can be
 * achieved using Evas and Edje, making it much easier to work with images
 * as well as not needing to worry about things like scaling them.
 *
 * @{
 */

/**
 * @brief   Reads the header data for an image but does not decode the pixels.
 *
 * @details This function reads and decodes the image header data stored under 
 *          the given key and Eet file.
 *
 * @since   1.0.0
 *
 * @remarks The information decoded is placed in each of the parameters, which must be
 *          provided. The width and height, measured in pixels, is stored under
 *          the variables pointed by @a w and @a h, respectively. If the read or
 *          decode of the header fails, this value is @c 0. The @a alpha parameter
 *          is @c 1 or @c 0, denoting if the alpha channel of the image is used or not.
 *          If the image is losslessly compressed, the @a compress parameter holds
 *          the compression amount used, ranging from @c 0 to @c 9 and @a lossy is @c 0.
 *          In the case of lossy compression, @a lossy is @c 1, and the compression
 *          quality is placed under @a quality, with a value ranging from @c 0 to @c 100.
 *
 * @param[in]   ef        A valid eet file handle opened for reading
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[out]   w         A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h         A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha     A pointer to the int to hold the alpha flag
 * @param[out]   compress  A pointer to the int to hold the compression amount
 * @param[out]   quality   A pointer to the int to hold the quality amount
 * @param[out]   lossy     A pointer to the int to hold the lossiness flag
 * @return  @c 1 if decoded successfully, \n 
 *          otherwise @c 0 otherwise
 *
 * @see eet_data_image_header_decode()
 * @see eet_data_image_header_read_cipher()
 */
EAPI int
eet_data_image_header_read(Eet_File *ef,
                           const char *name,
                           unsigned int *w,
                           unsigned int *h,
                           int *alpha,
                           int *compress,
                           int *quality,
                           Eet_Image_Encoding *lossy);

/**
 * @brief   Reads the image data from the named key in the eet file.
 *
 * @details This function reads and decodes the image stored in the given Eet file under the named key.
 *
 * @since   1.0.0
 *
 * @remarks The returned pixel data is a linear array of pixels starting from the
 *          top-left of the image, scanning row by row from left to right. Each pile
 *          is a 32bit value, with the high byte being the alpha channel, the next being
 *          red, then green, and the low byte being blue.
 *
 * @remarks The rest of the parameters are the same as in eet_data_image_header_read().
 *
 * @remarks On success, the function returns a pointer to the image data decoded. The
 *          calling application is responsible for calling free() on the image data
 *          when it is done with it. On failure @c NULL is returned and the parameter
 *          values may not contain any sensible data.
 *
 * @param[in]   ef        A valid eet file handle opened for reading
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[out]   w         A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h         A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha     A pointer to the int to hold the alpha flag
 * @param[out]   compress  A pointer to the int to hold the compression amount
 * @param[out]   quality   A pointer to the int to hold the quality amount
 * @param[out]   lossy     A pointer to the int to hold the lossiness flag
 * @return  The image pixel data decoded
 *
 * @see eet_data_image_header_read()
 * @see eet_data_image_decode()
 * @see eet_data_image_read_cipher()
 * @see eet_data_image_read_to_surface()
 */
EAPI void *
eet_data_image_read(Eet_File *ef,
                    const char *name,
                    unsigned int *w,
                    unsigned int *h,
                    int *alpha,
                    int *compress,
                    int *quality,
                    Eet_Image_Encoding *lossy);

/**
 * @brief   Reads image data from the named key in the eet file and stores it in the given buffer.
 *
 * @details This function reads and decodes the image stored in the given Eet file, placing the
 *          resulting pixel data in the buffer pointed by the user.
 *
 * @since   1.0.2
 *
 * @remarks Like eet_data_image_read(), it takes the image data stored under the
 *          @a name key in the @a ef file, but instead of returning a new buffer with
 *          the pixel data, it places the result in the buffer pointed by @a d, which
 *          must be provided by the user and of sufficient size to hold the requested
 *          portion of the image.
 *
 * @remarks The @a src_x and @a src_y parameters indicate the top-left corner of the
 *          section of the image to decode. These have to be higher or equal than @c 0 and
 *          less than the respective total width and height of the image. The width
 *          and height of the section of the image to decode are given in @a w and @a h
 *          and also cannot be higher than the total width and height of the image.
 *
 * @remarks The @a row_stride parameter indicates the length in bytes of each line in
 *          the destination buffer and it has to be at least @a w * 4.
 *
 * @remarks All the other parameters are the same as in eet_data_image_read().
 *
 * @remarks On success, the function returns @c 1, and @c 0 on failure. On failure, the
 *          parameter values may not contain any sensible data.
 *
 * @param[in]   ef          A valid eet file handle opened for reading
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   src_x       The starting x coordinate from where to dump the stream
 * @param[in]   src_y       The starting y coordinate from where to dump the stream
 * @param[out]   d           A pointer to the pixel surface
 * @param[in]   w           The expected width in pixels of the pixel surface to decode
 * @param[in]   h           The expected height in pixels of the pixel surface to decode
 * @param[in]   row_stride  The length of a pixels line in the destination surface
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if the image data is read and stored successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_read()
 * @see eet_data_image_decode()
 * @see eet_data_image_decode_to_surface()
 * @see eet_data_image_read_to_surface_cipher()
 * @see eet_data_image_decode_to_cspace_surface_cipher()
 */
EAPI int
eet_data_image_read_to_surface(Eet_File *ef,
                               const char *name,
                               unsigned int src_x,
                               unsigned int src_y,
                               unsigned int *d,
                               unsigned int w,
                               unsigned int h,
                               unsigned int row_stride,
                               int *alpha,
                               int *compress,
                               int *quality,
                               Eet_Image_Encoding *lossy);

/**
 * @brief   Writes image data to the named key in an eet file.
 *
 * @details This function takes image pixel data and encodes it in an eet file
 *          stored under the supplied name key, and returns how many bytes were
 *          actually written to encode the image data.
 *
 * @since   1.0.0
 *
 * @remarks The data expected is the same format as returned by eet_data_image_read.
 *          If this is not the case, weird things may happen. Width and height must
 *          be between @c 1 and @c 8000 pixels. The alpha flags can be @c 0 or @c 1 (@c 0 meaning
 *          the alpha values are not useful and @c 1 meaning they are). Compress can
 *          be from @c 0 to @c 9 (@c 0 meaning no compression, @c 9 meaning full compression).
 *          This is only used if the image is not lossily encoded. Quality is used on
 *          lossy compression and should be a value from @c 0 to @c 100. The lossy flag
 *          can be @c 0 or @c 1. @c 0 means encode losslessly and @c 1 means to encode with
 *          image quality loss (but then have a much smaller encoding).
 *
 * @remarks On success, this function returns the number of bytes that were required
 *          to encode the image data, or on failure it returns @c 0.
 *
 * @param[in]   ef        A valid eet file handle opened for writing
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[in]   data      A pointer to the image pixel data
 * @param[in]   w         The width of the image in pixels
 * @param[in]   h         The height of the image in pixels
 * @param[in]   alpha     The alpha channel flag
 * @param[in]   compress  The compression amount
 * @param[in]   quality   The quality encoding amount
 * @param[in]   lossy     The lossiness flag
 * @return  The number of bytes to encode the image data, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_read()
 * @see eet_data_image_encode()
 * @see eet_data_image_write_cipher()
 */
EAPI int
eet_data_image_write(Eet_File *ef,
                     const char *name,
                     const void *data,
                     unsigned int w,
                     unsigned int h,
                     int alpha,
                     int compress,
                     int quality,
                     Eet_Image_Encoding lossy);

/**
 * @brief   Decodes image data header to get information.
 *
 * @details This function works exactly like eet_data_image_header_read(), but instead
 *          of reading from an Eet file, it takes the buffer of size @a size pointed
 *          by @a data, which must be a valid Eet encoded image.
 *
 * @since   1.0.0
 *
 * @remarks On success, the function returns @c 1 indicating the header is read and
 *          decoded properly, or @c 0 on failure.
 *
 * @param[in]   data      The encoded pixel data
 * @param[in]   size      The size, in bytes, of the encoded pixel data
 * @param[out]   w         A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h         A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha     A pointer to the int to hold the alpha flag
 * @param[out]   compress  A pointer to the int to hold the compression amount
 * @param[out]   quality   A pointer to the int to hold the quality amount
 * @param[out]   lossy     A pointer to the int to hold the lossiness flag
 * @return  @c 1 if the image data header is decoded successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_header_read()
 * @see eet_data_image_header_decode_cipher()
 */
EAPI int
eet_data_image_header_decode(const void *data,
                             int size,
                             unsigned int *w,
                             unsigned int *h,
                             int *alpha,
                             int *compress,
                             int *quality,
                             Eet_Image_Encoding *lossy);

/**
 * @brief   Decodes image data into pixel data.
 *
 * @details This function takes encoded pixel data and decodes it into raw RGBA
 *          pixels on success.
 * @since   1.0.0
 *
 * @remarks It works exactly like eet_data_image_read(), but it takes the encoded
 *          data in the @a data buffer of size @a size, instead of reading from a file.
 *          All the others parameters are also the same.
 *
 * @remarks On success, the function returns a pointer to the image data decoded. The
 *          calling application is responsible for calling free() on the image data
 *          when it is done with it. On failure, @c NULL is returned and the parameter
 *          values may not contain any sensible data.
 *
 * @param[in]   data      The encoded pixel data
 * @param[in]   size      The size, in bytes, of the encoded pixel data
 * @param[out]   w         A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h         A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha     A pointer to the int to hold the alpha flag
 * @param[out]   compress  A pointer to the int to hold the compression amount
 * @param[out]   quality   A pointer to the int to hold the quality amount
 * @param[out]   lossy     A pointer to the int to hold the lossiness flag
 * @return  The image pixel data decoded, \n
 *          @c otherwise @c NULL on failure
 *
 * @see eet_data_image_read()
 * @see eet_data_image_decode_cipher()
 */
EAPI void *
eet_data_image_decode(const void *data,
                      int size,
                      unsigned int *w,
                      unsigned int *h,
                      int *alpha,
                      int *compress,
                      int *quality,
                      Eet_Image_Encoding *lossy);

/**
 * @brief   Decodes image data into pixel data and stores in the given buffer.
 * @since   1.0.2
 *
 * @remarks Like eet_data_image_read_to_surface(), but reading the given @a data buffer
 *          instead of a file.
 *
 * @remarks On success, the function returns @c 1, and @c 0 on failure. On failure, the
 *          parameter values may not contain any sensible data.
 *
 * @param[in]   data        The encoded pixel data
 * @param[in]   size        The size, in bytes, of the encoded pixel data
 * @param[in]   src_x       The starting x coordinate from where to dump the stream
 * @param[in]   src_y       The starting y coordinate from where to dump the stream
 * @param[out]   d           A pointer to the pixel surface
 * @param[in]   w           The expected width in pixels of the pixel surface to decode
 * @param[in]   h           The expected height in pixels of the pixel surface to decode
 * @param[in]   row_stride  The length of a pixels line in the destination surface
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if the image is decoded successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_read_to_surface()
 * @see eet_data_image_decode_to_surface_cipher()
 */
EAPI int
eet_data_image_decode_to_surface(const void *data,
                                 int size,
                                 unsigned int src_x,
                                 unsigned int src_y,
                                 unsigned int *d,
                                 unsigned int w,
                                 unsigned int h,
                                 unsigned int row_stride,
                                 int *alpha,
                                 int *compress,
                                 int *quality,
                                 Eet_Image_Encoding *lossy);

/**
 * @brief   Encodes image data for storage or transmission.
 *
 * @details This function encodes image pixel data with compression and
 *          possible loss of quality (as a trade off for size) for storage or
 *          transmission to another system.
 * @since   1.0.0
 *
 * @remarks It works like eet_data_image_write(), but instead of writing the encoded
 *          image into an Eet file, it allocates a new buffer of the size required and
 *          returns the encoded data in it.
 *
 * @remarks On success, this function returns a pointer to the encoded data that you
 *          can free with free() when no longer needed.
 *
 * @param[in]   data      A pointer to the image pixel data
 * @param[out]   size_ret  A pointer to an int to hold the size of the returned data
 * @param[in]   w         The width of the image in pixels
 * @param[in]   h         The height of the image in pixels
 * @param[in]   alpha     The alpha channel flag
 * @param[in]   compress  The compression amount
 * @param[in]   quality   The quality encoding amount
 * @param[in]   lossy     The lossiness flag
 * @return  The encoded image data
 *
 * @see eet_data_image_write()
 * @see eet_data_image_read()
 * @see eet_data_image_encode_cipher()
 */
EAPI void *
eet_data_image_encode(const void *data,
                      int *size_ret,
                      unsigned int w,
                      unsigned int h,
                      int alpha,
                      int compress,
                      int quality,
                      Eet_Image_Encoding lossy);

/**
 * @brief   Gets the colorspace that Eet can decode into of a given eet image resource.
 *
 * @since 1.10.0
 *
 * @param[in]   ef       A valid eet file handle opened for reading
 * @param[in]   name     The name of the entry \n 
 *                   For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   cspaces  The returned pointer by Eet to a list of possible decoding colorspace 
 *                   finished by @c EET_COLORSPACE_ARGB8888 \n 
 *                   If @c NULL, only EET_COLORSPACE_ARGB8888 is supported.
 * @return  @c 1 if the colorspace is obtained successfully, \n
 *          otherwise @c 0 on failure
 */
EAPI int
eet_data_image_colorspace_get(Eet_File *ef,
                              const char *name,
                              const char *cipher_key,
                              const Eet_Colorspace **cspaces);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_File_Image_Cipher_Group Image Store and Load using a Cipher
 * @ingroup Eet_File_Image_Group
 *
 * Most of the @ref Eet_File_Image_Group have alternative versions
 * that accounts for ciphers to protect their content.
 *
 * @see @ref Eet_Cipher_Group
 *
 * @{
 */

/**
 * @brief   Reads only the header data for an image and does not decode the pixels using a cipher.
 *
 * @details This function reads an image from an eet file stored under the named
 *          key in the eet file and returns a pointer to the decompressed pixel data.
 *
 * @since   1.0.0
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value or amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success the function returns @c 1 indicating the header is read and
 *          decoded properly, or @c 0 on failure.
 *
 * @param[in]   ef          A valid eet file handle opened for reading
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[out]   w           A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h           A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 is the image is read successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_header_read()
 */
EAPI int
eet_data_image_header_read_cipher(Eet_File *ef,
                                  const char *name,
                                  const char *cipher_key,
                                  unsigned int *w,
                                  unsigned int *h,
                                  int *alpha,
                                  int *compress,
                                  int *quality,
                                  Eet_Image_Encoding *lossy);

/**
 * @brief   Reads image data from the named key in the eet file using a cipher.
 *
 * @details This function reads an image from an eet file stored under the named
 *          key in the eet file and return a pointer to the decompressed pixel data.
 *
 * @since   1.0.0
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value or amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success, the function returns a pointer to the image data decoded. The
 *          calling application is responsible for calling free() on the image data
 *          when it is done with it. On failure, @c NULL is returned and the parameter
 *          values may not contain any sensible data.
 *
 * @param[in]   ef          A valid eet file handle opened for reading
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[out]   w           A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h           A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  The image pixel data decoded
 *
 * @see eet_data_image_read()
 */
EAPI void *
eet_data_image_read_cipher(Eet_File *ef,
                           const char *name,
                           const char *cipher_key,
                           unsigned int *w,
                           unsigned int *h,
                           int *alpha,
                           int *compress,
                           int *quality,
                           Eet_Image_Encoding *lossy);

/**
 * @brief   Reads image data from the named key in the eet file using a cipher.
 *
 * @details This function reads an image from an eet file stored under the named
 *          key in the eet file and store the decompressed pixel data in the provided
 *          surface with an @c EET_COLORSPACE_ARGB8888 colorspace.
 *
 * @since   1.0.2
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value/amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success, the function returns @c 1, and @c 0 on failure. On failure, the
 *          parameter values may not contain any sensible data.
 *
 * @param[in]   ef          A valid eet file handle opened for reading
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   src_x       The starting x coordinate from where to dump the stream
 * @param[in]   src_y       The starting y coordinate from where to dump the stream
 * @param[out]   d           A pointer to the pixel surface
 * @param[in]   w           The expected width in pixels of the pixel surface to decode
 * @param[in]   h           The expected height in pixels of the pixel surface to decode
 * @param[in]   row_stride  The length of a pixels line in the destination surface
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if image data is read successfully, 
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_read_to_surface()
 * @see eet_data_image_decode_to_cspace_surface_cipher()
 */
EAPI int
eet_data_image_read_to_surface_cipher(Eet_File *ef,
                                      const char *name,
                                      const char *cipher_key,
                                      unsigned int src_x,
                                      unsigned int src_y,
                                      unsigned int *d,
                                      unsigned int w,
                                      unsigned int h,
                                      unsigned int row_stride,
                                      int *alpha,
                                      int *compress,
                                      int *quality,
                                      Eet_Image_Encoding *lossy);


/**
 * @brief   Reads image data from the named key in the eet file using a cipher.
 *
 * @details This function reads an image from an eet file stored under the named
 *          key in the eet file and store the decompressed pixel data in the provided
 *          surface colorspace.
 *
 * @since   1.10.0
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value/amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success the function returns @c 1, and @c 0 on failure. On failure the
 *          parameter values may not contain any sensible data.
 *
 * @param[in]   ef          A valid eet file handle opened for reading
 * @param[in]   name        The name of the entry \n 
 *                          For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   src_x       The starting x coordinate from where to dump the stream
 * @param[in]   src_y       The starting y coordinate from where to dump the stream
 * @param[out]  d           A pointer to the pixel surface
 * @param[in]   w           The expected width in pixels of the pixel surface to decode
 * @param[in]   h           The expected height in pixels of the pixel surface to decode
 * @param[in]   row_stride  The length of a pixels line in the destination surface
 * @param[in]   cspace      The color space of the pixels bsurface
 * @param[out]  alpha       A pointer to the int to hold the alpha flag
 * @param[out]  comp        A pointer to the int to hold the compression amount
 * @param[out]  quality     A pointer to the int to hold the quality amount
 * @param[out]  lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if image data is read successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_read_to_surface()
 * @see eet_data_image_decode_to_cspace_surface_cipher()
 * @see eet_data_image_read_to_surface_cipher()
 */
EAPI int
eet_data_image_read_to_cspace_surface_cipher(Eet_File     *ef,
                                             const char   *name,
                                             const char   *cipher_key,
                                             unsigned int  src_x,
                                             unsigned int  src_y,
                                             unsigned int *d,
                                             unsigned int  w,
                                             unsigned int  h,
                                             unsigned int  row_stride,
                                             Eet_Colorspace cspace,
                                             int          *alpha,
                                             int          *comp,
                                             int          *quality,
                                             Eet_Image_Encoding *lossy);


/**
 * @brief   Reads image data from the named key in the eet file using a cipher.
 *
 * @details This function reads an image from an eet file stored under the named
 *          key in the eet file and store the decompressed pixels in the specified
 *          color space inside the given surface.
 *
 * @since   1.10.0
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value or amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success the function returns @c 1, and @c 0 on failure. On failure the
 *          parameter values may not contain any sensible data.
 *
 * @param[in]   data        A pointer to the image pixel data
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size        The size, in bytes, of the encoded pixel data
 * @param[in]   src_x       The starting x coordinate from where to dump the stream
 * @param[in]   src_y       The starting y coordinate from where to dump the stream
 * @param[out]  d           A pointer to the pixel surface
 * @param[in]   w           The expected width in pixels of the pixel surface to decode
 * @param[in]   h           The expected height in pixels of the pixel surface to decode
 * @param[in]   row_stride  The length of a pixels line in the destination surface
 * @param[in]   cspace      The color space of the pixel surface
 * @param[out]  alpha       A pointer to the int to hold the alpha flag
 * @param[out]  comp        A pointer to the int to hold the compression amount
 * @param[out]  quality     A pointer to the int to hold the quality amount
 * @param[out]  lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if image data is read successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_read_to_surface()
 * @see eet_data_image_read_to_surface_cipher()
 */

EAPI int
eet_data_image_decode_to_cspace_surface_cipher(const void   *data,
                                               const char   *cipher_key,
                                               int           size,
                                               unsigned int  src_x,
                                               unsigned int  src_y,
                                               unsigned int *d,
                                               unsigned int  w,
                                               unsigned int  h,
                                               unsigned int  row_stride,
                                               Eet_Colorspace cspace,
                                               int          *alpha,
                                               int          *comp,
                                               int          *quality,
                                               Eet_Image_Encoding *lossy);

/**
 * @brief   Writes image data to the named key in an eet file using a cipher.
 *
 * @details This function takes image pixel data and encodes it in an eet file
 *          stored under the supplied name key, and returns how many bytes were
 *          actually written to encode the image data.
 * @since   1.0.0
 *
 * @remarks The data expected is the same format as returned by eet_data_image_read.
 *          If this is not the case weird things may happen. Width and height must
 *          be between @c 1 and @c 8000 pixels. The alpha flags can be @c 0 or @c 1 (@c 0 meaning
 *          the alpha values are not useful and @c 1 meaning they are). Compress can
 *          be from @c 0 to @c 9 (@c 0 meaning no compression, @c 9 meaning full compression).
 *          This is only used if the image is not lossily encoded. Quality is used on
 *          lossy compression and should be a value from @c 0 to @c 100. The lossy flag
 *          can be @c 0 or @c 1. @c 0 means encode losslessly and @c 1 means to encode with
 *          image quality loss (but then have a much smaller encoding).
 *
 * @remarks On success, this function returns the number of bytes that were required
 *          to encode the image data, or on failure it returns @c 0.
 *
 * @param[in]   ef          A valid eet file handle opened for writing
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   data        A pointer to the image pixel data
 * @param[in]   w           The width of the image in pixels
 * @param[in]   h           The height of the image in pixels
 * @param[in]   alpha       The alpha channel flag
 * @param[in]   compress    The compression amount
 * @param[in]   quality     The quality encoding amount
 * @param[in]   lossy       The lossiness flag
 * @return  The number of bytes to encode the image data, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_write()
 */
EAPI int
eet_data_image_write_cipher(Eet_File *ef,
                            const char *name,
                            const char *cipher_key,
                            const void *data,
                            unsigned int w,
                            unsigned int h,
                            int alpha,
                            int compress,
                            int quality,
                            Eet_Image_Encoding lossy);

/**
 * @brief   Decodes image data header only to get information using a cipher.
 *
 * @details This function takes encoded pixel data and decodes it into raw RGBA
 *          pixels on success.
 * @since   1.0.0
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value or amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success, the function returns @c 1 indicating the header is read and
 *          decoded properly, or @c 0 on failure.
 *
 * @param[in]   data        The encoded pixel data
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size        The size, in bytes, of the encoded pixel data
 * @param[out]   w           A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h           A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if the header is decoded successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_header_decode()
 */
EAPI int
eet_data_image_header_decode_cipher(const void *data,
                                    const char *cipher_key,
                                    int size,
                                    unsigned int *w,
                                    unsigned int *h,
                                    int *alpha,
                                    int *compress,
                                    int *quality,
                                    Eet_Image_Encoding *lossy);

/**
 * @brief   Decodes image data into pixel data using a cipher.
 *
 * @details This function takes encoded pixel data and decodes it into raw RGBA
 *          pixels on success.
 * @since   1.0.0
 *
 * @remarks The other parameters of the image (width, height etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value or amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success, the function returns a pointer to the image data decoded. The
 *          calling application is responsible for calling free() on the image data
 *          when it is done with it. On failure, @c NULL is returned and the parameter
 *          values may not contain any sensible data.
 *
 * @param[in]   data        The encoded pixel data
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size        The size, in bytes, of the encoded pixel data
 * @param[out]   w           A pointer to the unsigned int to hold the width in pixels
 * @param[out]   h           A pointer to the unsigned int to hold the height in pixels
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  The image pixel data decoded, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_image_decode()
 */
EAPI void *
eet_data_image_decode_cipher(const void *data,
                             const char *cipher_key,
                             int size,
                             unsigned int *w,
                             unsigned int *h,
                             int *alpha,
                             int *compress,
                             int *quality,
                             Eet_Image_Encoding *lossy);

/**
 * @brief   Decodes image data into pixel data using a cipher.
 *
 * @details This function takes encoded pixel data and decodes it into raw RGBA
 *          pixels on success.
 * @since   1.0.2
 *
 * @remarks The other parameters of the image (alpha, compress etc.) are placed into
 *          the values pointed to (they must be supplied). The pixel data is a linear
 *          array of pixels starting from the top-left of the image scanning row by
 *          row from left to right. Each pixel is a 32bit value, with the high byte
 *          being the alpha channel, the next being red, then green, and the low byte
 *          being blue. The width and height are measured in pixels and is
 *          greater than @c 0 when returned. The alpha flag is either @c 0 or @c 1. @c 0 denotes
 *          that the alpha channel is not used. @c 1 denotes that it is significant.
 *          Compress is filled with the compression value or amount the image is
 *          stored with. The quality value is filled with the quality encoding of
 *          the image file (0 - 100). The lossy flags is either @c 0 or @c 1 as to if
 *          the image is encoded lossily or not.
 *
 * @remarks On success, the function returns @c 1, and @c 0 on failure. On failure, the
 *          parameter values may not contain any sensible data.
 *
 * @param[in]   data        The encoded pixel data
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size        The size, in bytes, of the encoded pixel data
 * @param[in]   src_x       The starting x coordinate from where to dump the stream
 * @param[in]   src_y       The starting y coordinate from where to dump the stream
 * @param[out]   d           A pointer to the pixel surface
 * @param[in]   w           The expected width in pixels of the pixel surface to decode
 * @param[in]   h           The expected height in pixels of the pixel surface to decode
 * @param[in]   row_stride  The length of a pixels line in the destination surface
 * @param[out]   alpha       A pointer to the int to hold the alpha flag
 * @param[out]   compress    A pointer to the int to hold the compression amount
 * @param[out]   quality     A pointer to the int to hold the quality amount
 * @param[out]   lossy       A pointer to the int to hold the lossiness flag
 * @return  @c 1 if the image data is decoded successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_image_decode_to_surface()
 */
EAPI int
eet_data_image_decode_to_surface_cipher(const void *data,
                                        const char *cipher_key,
                                        int size,
                                        unsigned int src_x,
                                        unsigned int src_y,
                                        unsigned int *d,
                                        unsigned int w,
                                        unsigned int h,
                                        unsigned int row_stride,
                                        int *alpha,
                                        int *compress,
                                        int *quality,
                                        Eet_Image_Encoding *lossy);

/**
 * @brief   Encodes image data for storage or transmission using a cipher.
 *
 * @details This function encodes image pixel data with compression and
 *          possible loss of quality (as a trade off for size) for storage or
 *          transmission to another system.
 * @since   1.0.0
 *
 * @remarks The data expected is the same format as returned by eet_data_image_read.
 *          If this is not the case weird things may happen. Width and height must
 *          be between @c 1 and @c 8000 pixels. The alpha flags can be @c 0 or @c 1 (@c 0 meaning
 *          the alpha values are not useful and @c 1 meaning they are). Compress can
 *          be from @c 0 to @c 9 (@c 0 meaning no compression, @c 9 meaning full compression).
 *          This is only used if the image is not lossily encoded. Quality is used on
 *          lossy compression and should be a value from @c 0 to @c 100. The lossy flag
 *          can be @c 0 or @c 1. @c 0 means encode losslessly and @c 1 means to encode with
 *          image quality loss (but then have a much smaller encoding).
 *
 * @remarks On success, this function returns a pointer to the encoded data that you
 *          can free with free() when no longer needed.
 *
 * @param[in]   data        A pointer to the image pixel data
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size_ret    A pointer to an int to hold the size of the returned data
 * @param[in]   w           The width of the image in pixels
 * @param[in]   h           The height of the image in pixels
 * @param[in]   alpha       The alpha channel flag
 * @param[in]   compress    The compression amount
 * @param[in]   quality     The quality encoding amount
 * @param[in]   lossy       The lossiness flag
 * @return  The encoded image data
 *
 * @see eet_data_image_encode()
 */
EAPI void *
eet_data_image_encode_cipher(const void *data,
                             const char *cipher_key,
                             unsigned int w,
                             unsigned int h,
                             int alpha,
                             int compress,
                             int quality,
                             Eet_Image_Encoding lossy,
                             int *size_ret);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_Cipher_Group Cipher, Identity and Protection Mechanisms
 * @ingroup Eet_Group
 *
 * Eet allows one to protect entries of an #Eet_File
 * individually. This may be used to ensure data is not tampered or
 * that third party does not read your data.
 *
 * @see @ref Eet_File_Cipher_Group
 * @see @ref Eet_File_Image_Cipher_Group
 *
 * @{
 */

/**
 * @typedef Eet_Key
 * Opaque handle that defines an identity (also known as key)
 * in Eet's cipher system.
 */
typedef struct _Eet_Key Eet_Key;

/**
 * @brief   Callback to request whether a password is needed for a private key.
 * @since   1.2.0
 *
 * @param   buffer  The buffer to store the password
 * @param   size    The maximum password size \n
 *                  The size of buffer, including '@\0'.
 * @param   rwflag  Indicator for the buffer mode \n
 *                  Whether it is also readable or just writeable.
 * @param   data    Currently unused, may contain some context in future
 * @return  @c 1 on success and password is set to @a buffer, \n
 *          otherwise @c 0 on failure
 * @ingroup Eet_Cipher_Group
 */
typedef int (*Eet_Key_Password_Callback)(char *buffer, int size, int rwflag, void *data);

/**
 * @brief   Creates an Eet_Key needed for signing an eet file.
 * @since   1.2.0
 *
 * @remarks The certificate should provide the public key that matches the private key.
 *          No verification is done to ensure that.
 *
 * @remarks You need to compile signature support in EET.
 *
 * @param[in]   certificate_file  The file to find the certificate
 * @param[in]   private_key_file  The file that contains the private key
 * @param[in]   cb                The callback to check whether password is required to unlock
 *                            private key
 * @return  A key handle to use, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_identity_close()
 */
EAPI Eet_Key *
eet_identity_open(const char *certificate_file,
                  const char *private_key_file,
                  Eet_Key_Password_Callback cb);

/**
 * @brief   Closes and releases all resources used by an Eet_Key. 
 * @since   1.2.0
 *
 * @remarks A reference counter prevents it from being freed until all the files
 *          using it are also closed.
 *
 * @param[in]   key  The key handle to close and free resources
 */
EAPI void
eet_identity_close(Eet_Key *key);

/**
 * @brief   Sets a key to sign a file.
 * @since   1.2.0
 *
 * @param[in]   ef   The file to set the identity
 * @param[in]   key  The key handle to set as identity
 * @return  #EET_ERROR_NONE is the key is set successfully, \n
 *          otherwise #EET_ERROR_BAD_OBJECT if @a ef is invalid
 */
EAPI Eet_Error
eet_identity_set(Eet_File *ef,
                 Eet_Key *key);

/**
 * @brief   Displays both private and public key of an Eet_Key.
 * @since   1.2.0
 *
 * @remarks You need to compile signature support in EET.
 *
 * @param[in]   key  The handle to print
 * @param[in]   out  The output destination to print
 */
EAPI void
eet_identity_print(Eet_Key *key,
                   FILE *out);

/**
 * @brief   Gets the x509 der certificate associated with an Eet_File. 
 * @details This function returns @c NULL if the file is not signed.
 * @since   1.2.0
 *
 * @param[in]   ef          The file handle to query
 * @param[out]   der_length  The length of returned data \n 
 *                      This may be @c NULL.
 * @return  The x509 certificate, \n
 *          otherwise @c NULL on error
 */
EAPI const void *
eet_identity_x509(Eet_File *ef,
                  int *der_length);

/**
 * @brief   Gets the raw signature associated with an Eet_File. 
 * @details This function returns @c NULL if the file is not signed.
 *
 * @param[in]   ef                The file handle to query
 * @param[out]   signature_length  The length of returned data \n 
 *                            This may be @c NULL
 * @return  The raw signature, \n 
 *          otherwise @c NULL on error
 */
EAPI const void *
eet_identity_signature(Eet_File *ef,
                       int *signature_length);

/**
 * @brief   Gets the SHA1 associated with a file. 
 * @since   1.2.0
 *
 * @remarks This could be used to sign the data or if the data where not signed, 
 *          it is the SHA1 of the file.
 *
 * @param[in]   ef           The file handle to query
 * @param[out]   sha1_length  The length of returned data \n 
 *                       This may be @c NULL.
 * @return  The associated SHA1, \n
 *          otherwise @c NULL on error
 */
EAPI const void *
eet_identity_sha1(Eet_File *ef,
                  int *sha1_length);

/**
 * @brief   Displays the x509 der certificate on @a out.
 * @since   1.2.0
 *
 * @remarks You need to compile signature support in EET.
 *
 * @param[in]  certificate  The x509 certificate to print
 * @param[in]  der_length   The length of the certificate
 * @param[in]  out          The output destination
 */
EAPI void
eet_identity_certificate_print(const unsigned char *certificate,
                               int der_length,
                               FILE *out);
/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_Data_Group Eet Data Serialization
 * @ingroup Eet_Group
 *
 * @brief  This group provides convenience functions to serialize and parse complex data
 *         structures to binary blobs.
 *
 * While Eet core just handles binary blobs, it is often required
 * to save some structured data of different types, such as
 * strings, integers, lists, hashes and so on.
 *
 * Eet can serialize and then parse data types given some
 * construction instructions. These are defined in two levels:
 *
 * - #Eet_Data_Descriptor_Class to tell generic memory handling,
 *   such as the size of the type, how to allocate memory, strings,
 *   lists, hashes and so on.
 *
 * - #Eet_Data_Descriptor to tell inside such type, the members and
 *   their offsets inside the memory blob, their types and
 *   names. These members can be simple types or other
 *   #Eet_Data_Descriptor, allowing hierarchical types to be
 *   defined.
 *
 * Given that C provides no introspection, this process can be
 * quite cumbersome, so lots of macros and convenience
 * functions are provided to aid creating the types.
 *
 * The following is a quick overview of some of the most commonly used elements
 * of this part of the library. A simple example of a configuration system
 * works as a somewhat real life example that is still simple enough to
 * follow.
 * Only the relevant sections are shown here, but you can get the full
 * code @ref eet-data-simple.c "here".
 *
 * Ignoring the included headers, you begin by defining your configuration
 * struct.
 * @dontinclude eet-data-simple.c
 * @skip typedef
 * @until }
 *
 * When using Eet, you do not think in matters of what data the program needs
 * to run and which you would like to store. It is all the same and if it makes
 * more sense to keep them together, it is perfectly fine to do so. At the time
 * of telling Eet how your data is comprised you can leave out the things
 * that are runtime only and let Eet take care of the rest for you.
 *
 * The key used to store the config follows, as well as the variable used to
 * store the data descriptor.
 * This last one is very important. It is the one thing that Eet uses to
 * identify your data, both at the time of writing it to the file and when
 * loading from it.
 * @skipline MY_CONF
 * @skipline Eet_Data_Descriptor
 *
 * Now you see how to create this descriptor, so Eet knows how to handle
 * your data later on.
 * Begin the function by declaring an Eet_Data_Descriptor_Class, which is
 * used to create the actual descriptor. This class contains the name of
 * your data type, its size and several functions that dictate how Eet should
 * handle memory to allocate the necessary bits to bring your data to life.
 * You, as a user, very hardly set this class' contents directly. The
 * most common scenario is to use one of the provided macros that set it using
 * the Eina data types, so that is what you are doing across all your examples.
 * @skip static void
 * @until eet_data_descriptor_stream_new
 *
 * Now that you have your descriptor, you need to make it describe something.
 * Do so by telling it which members of your struct you want it to know about
 * and their types.
 * The eet_data_descriptor_element_add() function takes care of this, but it is
 * too cumbersome for normal use, so several macros are provided that make
 * it easier to handle. Even with them, however, code can get very repetitive
 * and it is not uncommon to define custom macros using them to save on typing.
 * @skip #define
 * @until }
 *
 * Now your descriptor knows about the parts of your structure that you are
 * interesting in saving. You can see that not all of them are there, yet Eet
 * finds those that need saving and do the right thing. When loading your
 * data, any non-described fields in the structure are zeroed, so there is
 * no need to worry about garbage memory in them.
 * See the documentation of #EET_DATA_DESCRIPTOR_ADD_BASIC to understand
 * what your macro does.
 *
 * You are done with your descriptor init function and it is proper to have the
 * relevant shutdown. Proper coding guidelines indiciate that all memory
 * allocated should be freed when the program ends, and since you must most
 * likely keep your descriptor around for the life or your application, it is
 * only right to free it at the end.
 * @skip static void
 * @until }
 *
 * Not listed here, but included in the full example are functions to create
 * a blank configuration and free it. The first one is only used when
 * no file exists to load from, or nothing is found in it, but the latter is
 * used regardless of where your data comes from. Unless you are reading direct
 * data from the Eet file, you are in charge of freeing anything loaded
 * from it.
 *
 * Now it is time to look at how you can load your config from some file.
 * Begin by opening the Eet file normally.
 * @skip static My_Conf_Type
 * @until }
 *
 * And now you need to read the data from the file and decode it using your
 * descriptor. Fortunately, that is all done in one single step.
 * @until goto
 *
 * And that is it for all Eet cares about. But since you are dealing with a
 * common case, as is save and load of user configurations, the next fragment
 * of code shows why you have a version field in our struct, and how you can
 * use it to load older configuration files and update them as needed.
 * @until }
 *
 * Finally, close the file and return the newly loaded config data.
 * @until }
 *
 * Saving data is just as easy. The full version of the following function
 * includes code to save to a temporary file first, so you can be sure not
 * to lose all your data in the case of a failure mid-writing. You can look
 * at it @ref eet-data-simple.c "here".
 * @skip static Eina_Bool
 * @until {
 * @skipline Eina_Bool ret
 * @skip eet_open
 * @until eet_close
 * @skip return
 * @until }
 *
 * To close, your main function, which does not do much. Just take some arguments
 * from the command line with the name of the file to load and another one
 * where to save again. If input file does not exist, a new config structure
 * is created and saved to our output file.
 * @skip int main
 * @until return ret
 * @until }
 *
 * The following is a list of more advanced and detailed examples.
 * @li @ref eet_data_nested_example
 * @li @ref eet_data_file_descriptor
 * @li @ref Example_Eet_Data_File_Descriptor_02
 * @li @ref Example_Eet_Data_Cipher_Decipher
 *
 * @{
 */

/**
 * @internal
 * @page eet_data_nested_example Nested structures and Eet Data Descriptors
 *
 * You have seen already a simple example of how to use Eet Data Descriptors
 * to handle your structures, but it did not show how this works when you
 * have structures inside other structures.
 *
 * Now, there is a very simple case of this, for when you have inline structs
 * to keep your big structure more organized, you do not need anything else
 * besides what @ref eet-data-simple.c "this simple example does".
 * Just use something like @a some_struct.sub_struct.member when adding the
 * member to the descriptor and it works.
 *
 * For example:
 * @code
 * typedef struct
 * {
 *    int a_number;
 *    char *a_string;
 *    struct {
 *       int other_num;
 *       int one_more;
 *    } sub;
 * } some_struct;
 *
 * void some_function()
 * {
 *    ...
 *    my_desc = eet_data_descriptor_stream_new(&eddc);
 *    EET_DATA_DESCRIPTOR_ADD_BASIC(my_desc, some_struct, "a_number",
 *                                  a_number, EET_T_INT);
 *    EET_DATA_DESCRIPTOR_ADD_BASIC(my_desc, some_struct, "a_string",
 *                                  a_string, EET_T_STRING);
 *    EET_DATA_DESCRIPTOR_ADD_BASIC(my_desc, some_struct, "sub.other_num",
 *                                  sub.other_num, EET_T_INT);
 *    EET_DATA_DESCRIPTOR_ADD_BASIC(my_desc, some_struct, "sub.one_more",
 *                                  sub.one_more", EET_T_INT);
 *    ...
 * }
 * @endcode
 *
 * But this is not what you are going to do. When you talk about nested
 * structures, what you really want are things like lists and hashes to be
 * taken into consideration automatically, and all their contents saved and
 * loaded just like ordinary integers and strings are.
 *
 * And of course, Eet can do that, and considering the work it saves you as a
 * programmer, you could say it is even easier to do than handling just integers.
 *
 * Let us begin with your example then, which is not all too different from the
 * simple one introduced earlier.
 *
 * You do not ignore the headers this time to show how easy it is to use Eina
 * data types with Eet, but you still skip most of the code that is not
 * pertinent to what you want to show now, but as usual, you can get it full
 * by following @ref eet-data-nested.c "this link".
 *
 * @dontinclude eet-data-nested.c
 * @skipline Eina.h
 * @skipline Eet.h
 * @skip typedef struct
 * @until } My_Conf_Subtype
 *
 * Extremely similar to your previous example. Just a new struct in there, and
 * a pointer to a list in the one you already had. Handling a list of subtypes
 * is easy on your program, but now you see what Eet needs to work with them
 * (Hint: it's easy too).
 * @skip _my_conf_descriptor
 * @until _my_conf_sub_descriptor
 *
 * Since you have two structures now, it is only natural that you need two
 * descriptors. One for each, which are defined exactly as before.
 * @skip static void
 * @until eddc
 * @skip EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET
 * @until _my_conf_sub_descriptor
 *
 * Create your descriptors, each for one type, and as before, you are going to
 * use a simple macro to set their contents, to save on typing.
 * @skip #define
 * @until EET_T_UCHAR
 *
 * So far, nothing new. You have your descriptors and you know already how to
 * save them separately. But what you want is to link them together, and even
 * more so, you want your main type to hold a list of more than one of the new
 * sub type. So how do you do that?
 *
 * Simple enough, you tell Eet that your main descriptor holds a list, of
 * which each node points to some type described by your new descriptor.
 * @skip EET_DATA_DESCRIPTOR_ADD_LIST
 * @until _my_conf_sub_descriptor
 *
 * And that is all. You are closing the function now so as to not leave dangling
 * curly braces, but there is nothing more to show in this example. Only other
 * additions are the necessary code to free your new data, but you can see it
 * in the full code listing.
 * @until }
 */

/**
 * @internal
 * @page eet_data_file_descriptor Advanced use of Eet Data Descriptors
 *
 * A real life example is usually the best way to see how things are used,
 * but they also involve a lot more code than what needs to be shown, so
 * instead of going that way, you are borrowing some pieces from one in
 * the following example. It is slightly modified from the original
 * source to show more of the varied ways in which Eet can handle your data.
 *
 * @ref eet-data-file_descriptor_01.c "This example" shows a cache of user
 * accounts and messages received, and it is a bit more interactive than
 * previous examples.
 *
 * Begin by looking at the structures that you are using. First you have
 * one to define the messages the user receives and one for the one the user posts.
 * Straight forward and nothing new here.
 * @dontinclude eet-data-file_descriptor_01.c
 * @skip typedef
 * @until My_Post
 *
 * One more to declare the account itself. This one contains a list of
 * all messages received, and the posts you make ourselves are kept in an
 * array. No special reason other than to show how to use arrays with Eet.
 * @until My_Account
 *
 * Finally, the main structure to hold your cache of accounts. You are looking
 * for these accounts by their names, so keep them in a hash, using
 * that name as the key.
 * @until My_Cache
 *
 * As explained before, you need one descriptor for each struct that you want Eet
 * to handle, but this time you also want to keep around your Eet file and its
 * string dictionary. You see why in a moment.
 * @skip Eet_Data_Descriptor
 * @until _my_post_descriptor
 * @skip Eet_File
 * @until Eet_Dictionary
 *
 * The differences begin now. They are not much, but you are creating your
 * descriptors differently. Things can be added to your cache, but you are not
 * modifying the current contents, so you can consider the data read from
 * it to be read-only, and thus allow Eet to save time and memory by not
 * duplicating things unnecessarily.
 * @skip static void
 * @until _my_post_descriptor
 *
 * As the comment in the code explains, you are asking Eet to give strings
 * directly from the mapped file, which avoids having to load it in memory
 * and data duplication.
 * Of course, there are things to take into account when doing things this
 * way, and they are mentioned as you encounter those special cases.
 *
 * Next comes the actual description of your data, just like you did in the
 * previous examples.
 * @skip #define
 * @until #undef
 * @until #define
 * @until #undef
 *
 * And the account struct's description does not add much new, but it is worth
 * commenting on it.
 * @skip #define
 * @until _my_post_descriptor
 *
 * You have seen before how to add a list, but now you are also adding an array.
 * There is nothing really special about it, but it is important to note that
 * the EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY is used to add arrays of variable
 * length to a descriptor. That is, arrays just like the one you defined.
 * Since there is no way in C to know how long they are, you need to keep
 * track of the count yourselves and Eet needs to know how to do so as well.
 * That is what the @a posts_count member of your struct is for. When adding
 * your array member, this macro looks for another variable in the struct
 * named just like the array, but with @a _count attached to the end.
 * When saving your data, Eet knows how many elements the array contains
 * by looking into this count variable. When loading back from a file, this
 * variable is set to the right number of elements.
 *
 * Another option for arrays is to use EET_DATA_DESCRIPTOR_ADD_ARRAY, which
 * takes care of fixed sized arrays.
 * For example, suppose that you want to keep track of only the last
 * ten posts the user sent, and declare your account struct as follows:
 * @code
 * typedef struct
 * {
 *    unsigned int id;
 *    const char  *name;
 *    Eina_List   *messages;
 *    My_Post      posts[10];
 * } My_Account;
 * @endcode
 * Then add the array to your descriptor with
 * @code
 * EET_DATA_DESCRIPTOR_ADD_ARRAY(_my_account_descriptor, My_Account, "posts",
 *                               posts, _my_post_descriptor);
 * @endcode
 *
 * Notice how this time you do not have a @a posts_count variable in your struct.
 * You could have it for the program to keep track of how many posts the
 * array actually contains, but Eet no longer needs it. Having defined
 * the way the array is already taking up all the memory needed for the ten
 * elements, it is possible in C to determine how much it is in code.
 * When saving your data, Eet just dumps the entire memory blob into the
 * file, regardless of how much of it is really used. So it is important to
 * take into consideration this kind of things when defining your data types.
 * Each has its uses, its advantages and disadvantages and it is up to you
 * to decide which to use.
 *
 * Now, going back to your example, you have to finish adding your data to the
 * descriptors. We are only missing the main one for the cache, which
 * contains your hash of accounts.
 * Unless you are using your own hash functions when setting the descriptor
 * class, always use hashes with string type keys.
 * @skip #define
 * @until }
 *
 * If you remember, you told Eet not to duplicate memory when possible at the
 * time of loading back your data. But this does not mean everything is
 * loaded straight from disk and you do not have to worry about freeing it.
 * Data in the Eet file is compressed and encoded, so it still needs to be
 * decoded and memory is allocated to convert it back into something that you
 * can use. You also need to take care of anything you add in the current
 * instance of the program.
 * To summarize, any string you get from Eet is likely to be a pointer to the
 * internal dictionary, and trying to free it, in the best case, crashes
 * your application right away.
 *
 * So how do you know if you have to free a string? You check if it is part of
 * the dictionary, and if it is not there, you can be sure it is safe to get
 * rid of it.
 * @skip static void
 * @skip }
 * @skip static void
 * @until }
 *
 * See how this is used when adding a new message to your cache.
 * @skip static My_Message
 * @until return msg
 * @until free(msg)
 * @until }
 *
 * Skipping all the utility functions used by your program (remember you can
 * look at the full example @ref eet-data-file_descriptor_01.c "here") you get to
 * your cache loading code. Nothing out of the ordinary at first, just the
 * same old open file, read data using your main descriptor to decode it
 * into something you can use and check version of loaded data and if it does not
 * match, do something accordingly.
 * @skip static My_Cache
 * @until }
 * @until }
 * @until }
 *
 * Then comes the interesting part. Remember how you kept two more global
 * variables with your descriptors? One of them you already used to check if
 * it is right to free a string or not, but you did not know where it came from.
 * Loading your data straight from the mmapped file means that you cannot close
 * it until you are done using it, so you need to keep its handler around until
 * then. It also means that any changes done to the file can, and do,
 * invalidate all your pointers to the file backed data, so if you add something
 * and save the file, you need to reload your cache.
 *
 * Thus your load function checks if you had an open file, if there is, it gets
 * closed and your variable is updated to the new handler. Then you get the
 * string dictionary you use to check whether a string is part of it or not.
 * Updating any references to the cache data is up to you as a programmer to
 * handle properly; there is nothing Eet can do in this situation.
 * @until }
 *
 * The save function does not have anything new, and all that is left after it
 * is the main program, which does not really have anything of interest within
 * the scope of what you are learning.
 */

#define EET_T_UNKNOW         0 /**< Unknown data encoding type */
#define EET_T_CHAR           1 /**< Data type: char */
#define EET_T_SHORT          2 /**< Data type: short */
#define EET_T_INT            3 /**< Data type: int */
#define EET_T_LONG_LONG      4 /**< Data type: long long */
#define EET_T_FLOAT          5 /**< Data type: float */
#define EET_T_DOUBLE         6 /**< Data type: double */
#define EET_T_UCHAR          7 /**< Data type: unsigned char */
#define EET_T_USHORT         8 /**< Data type: unsigned short */
#define EET_T_UINT           9 /**< Data type: unsigned int */
#define EET_T_ULONG_LONG     10 /**< Data type: unsigned long long */
#define EET_T_STRING         11 /**< Data type: char * */
#define EET_T_INLINED_STRING 12 /**< Data type: char * (but compressed inside the resulting eet) */
#define EET_T_NULL           13 /**< Data type: (void *) (only use it if you know why) */
#define EET_T_F32P32         14 /**< Data type: fixed point 32.32 */
#define EET_T_F16P16         15 /**< Data type: fixed point 16.16 */
#define EET_T_F8P24          16 /**< Data type: fixed point 8.24 */
#define EET_T_LAST           17 /**< Last data type */

#define EET_G_UNKNOWN        100 /**< Unknown group data encoding type */
#define EET_G_ARRAY          101 /**< Fixed size array group type */
#define EET_G_VAR_ARRAY      102 /**< Variable size array group type */
#define EET_G_LIST           103 /**< Linked list group type */
#define EET_G_HASH           104 /**< Hash table group type */
#define EET_G_UNION          105 /**< Union group type */
#define EET_G_VARIANT        106 /**< Selectable subtype group */
#define EET_G_LAST           107 /**< Last group type */

#define EET_I_LIMIT          128 /**< Other type exist but are reserved for internal purpose. */

/**
 * @typedef Eet_Data_Descriptor
 *
 * @brief The structure type containing an opaque handle that have information on a type members.
 *
 * Descriptors are created using an #Eet_Data_Descriptor_Class, and they
 * describe the contents of the structure that is serialized by Eet.
 * Not all members need to be described by it, just those that should be handled
 * by Eet. This way it is possible to have one structure with both data to be
 * saved to a file, like application configuration, and runtime information
 * that would be meaningless to store, but is appropriate to keep together
 * during the program execution.
 * The members are added by means of
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB(),
 * EET_DATA_DESCRIPTOR_ADD_LIST(), EET_DATA_DESCRIPTOR_ADD_HASH()
 * or eet_data_descriptor_element_add().
 *
 * @see eet_data_descriptor_stream_new()
 * @see eet_data_descriptor_file_new()
 * @see eet_data_descriptor_free()
 */
typedef struct _Eet_Data_Descriptor Eet_Data_Descriptor;

/**
 * @def EET_DATA_DESCRIPTOR_CLASS_VERSION
 * @brief Definition of the version of #Eet_Data_Descriptor_Class at the time of the
 *        distribution of the sources. One should define this to its
 *        version member so it is compatible with API changes, or at least
 *        do not crash with them.
 */
#define EET_DATA_DESCRIPTOR_CLASS_VERSION 4

/**
 * @typedef Eet_Data_Descriptor_Class
 *
 * @brief The structure type that instructs Eet about memory management for different needs under
 *        serialization and parse process.
 */
typedef struct _Eet_Data_Descriptor_Class Eet_Data_Descriptor_Class;

typedef int                             (*Eet_Descriptor_Hash_Foreach_Callback_Callback)(void *h, const char *k, void *dt, void *fdt);

typedef void *                          (*Eet_Descriptor_Mem_Alloc_Callback)(size_t size);
typedef void                            (*Eet_Descriptor_Mem_Free_Callback)(void *mem);
typedef char *                          (*Eet_Descriptor_Str_Alloc_Callback)(const char *str);
typedef void                            (*Eet_Descriptor_Str_Free_Callback)(const char *str);
typedef void *                          (*Eet_Descriptor_List_Next_Callback)(void *l);
typedef void *                          (*Eet_Descriptor_List_Append_Callback)(void *l, void *d);
typedef void *                          (*Eet_Descriptor_List_Data_Callback)(void *l);
typedef void *                          (*Eet_Descriptor_List_Free_Callback)(void *l);
typedef void                            (*Eet_Descriptor_Hash_Foreach_Callback)(void *h, Eet_Descriptor_Hash_Foreach_Callback_Callback func, void *fdt);
typedef void *                          (*Eet_Descriptor_Hash_Add_Callback)(void *h, const char *k, void *d);
typedef void                            (*Eet_Descriptor_Hash_Free_Callback)(void *h);
typedef char *                          (*Eet_Descriptor_Str_Direct_Alloc_Callback)(const char *str);
typedef void                            (*Eet_Descriptor_Str_Direct_Free_Callback)(const char *str);
typedef const char *                    (*Eet_Descriptor_Type_Get_Callback)(const void *data, Eina_Bool *unknow);
typedef Eina_Bool                       (*Eet_Descriptor_Type_Set_Callback)(const char *type, void *data, Eina_Bool unknow);
typedef void *                          (*Eet_Descriptor_Array_Alloc_Callback)(size_t size);
typedef void                            (*Eet_Descriptor_Array_Free_Callback)(void *mem);

/**
 * @internal
 * @struct _Eet_Data_Descriptor_Class
 *
 * @brief The structure type that instructs Eet about memory management for different needs under
 *        serialization and parse process.
 *
 * The list and hash methods match the Eina API, so for a more detailed
 * reference on them, look at the Eina_List and Eina_Hash documentation,
 * respectively.
 * For the most part these are used with the standard Eina functions,
 * so using EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET() and
 * EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET() set up everything
 * accordingly.
 */
struct _Eet_Data_Descriptor_Class
{
   int         version;  /**< API version. Should always be set to #EET_DATA_DESCRIPTOR_CLASS_VERSION */
   const char *name;  /**< Name of the user data type to be serialized */
   int         size;  /**< Size in bytes of the user data type to be serialized */
   struct
   {
      Eet_Descriptor_Mem_Alloc_Callback        mem_alloc; /**< Called when allocating memory (usually malloc()) */
      Eet_Descriptor_Mem_Free_Callback         mem_free; /**< Called when freeing memory (usually free()) */
      Eet_Descriptor_Str_Alloc_Callback        str_alloc; /**< Called when allocating a string */
      Eet_Descriptor_Str_Free_Callback         str_free; /**< Called when freeing a string */
      Eet_Descriptor_List_Next_Callback        list_next; /**< Called when iterating to the next element of a list. Receives and should return the list node. */
      Eet_Descriptor_List_Append_Callback      list_append; /**< Called when appending data @a d to list with head node @a l */
      Eet_Descriptor_List_Data_Callback        list_data; /**< Called when retrieving the data from node @a l */
      Eet_Descriptor_List_Free_Callback        list_free; /**< Called when freeing all the nodes from the list with head node @a l */
      Eet_Descriptor_Hash_Foreach_Callback     hash_foreach; /**< Called when iterating over all elements in the hash @a h in no specific order */
      Eet_Descriptor_Hash_Add_Callback         hash_add; /**< Called when adding a new data @a d with key @a k in hash @a h */
      Eet_Descriptor_Hash_Free_Callback        hash_free; /**< Called when freeing all entries from the hash @a h */
      Eet_Descriptor_Str_Direct_Alloc_Callback str_direct_alloc; /**< Called when allocating a string directly from file backed or mmaped region pointed by @a str */
      Eet_Descriptor_Str_Direct_Free_Callback  str_direct_free; /**< Called when freeing a string returned by str_direct_alloc */
      Eet_Descriptor_Type_Get_Callback         type_get; /**< Called when getting the type, as used in the union or variant mapping, that should be used to store the given data into the eet file */
      Eet_Descriptor_Type_Set_Callback         type_set; /**< Called when loading a mapped type with the given @a type used to describe the type in the descriptor */
      Eet_Descriptor_Array_Alloc_Callback      array_alloc; /**< Called when allocating memory for array (usually malloc()) */
      Eet_Descriptor_Array_Free_Callback       array_free; /**< Called when freeing memory for array (usually free()) */
   } func;
};

/**
 * @brief   Creates a new empty data structure descriptor.
 *
 * @details This function creates a new data descriptor and returns a handle to the
 *          new data descriptor. On creation it is empty, containing no contents
 *          describing anything other than the shell of the data structure.
 * @since   1.0.0
 * @remarks You add structure members to the data descriptor using the macros
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB() and
 *          EET_DATA_DESCRIPTOR_ADD_LIST(), depending on what type of member you are
 *          adding to the description.
 *
 * @remarks Once you have described all the members of a struct you want loaded, or
 *          saved eet can load and save those members for you, encode them into
 *          endian-independent serialised data chunks for transmission across a
 *          a network or more.
 *
 * @remarks The function pointers to the list and hash table functions are only
 *          needed if you use those data types, else you can pass @c NULL instead.
 *
 * @param   name               The string name of this data structure \n
 *                             A global constant and never changes.
 * @param   size               The size of the struct (in bytes)
 * @param   func_list_next     The function to get the next list node
 * @param   func_list_append   The function to append a member to a list
 * @param   func_list_data     The function to get the data from a list node
 * @param   func_list_free     The function to free an entire linked list
 * @param   func_hash_foreach  The function to iterate through all hash table entries
 * @param   func_hash_add      The function to add a member to a hash table
 * @param   func_hash_free     The function to free an entire hash table
 * @return  A new empty data descriptor
 *
 * @ingroup Eet_Data_Group
 *
 * @deprecated use eet_data_descriptor_stream_new() or
 *             eet_data_descriptor_file_new()
 */
EINA_DEPRECATED EAPI Eet_Data_Descriptor *
eet_data_descriptor_new(const char *name,
                        int size,
                        Eet_Descriptor_List_Next_Callback func_list_next,
                        Eet_Descriptor_List_Append_Callback func_list_append,
                        Eet_Descriptor_List_Data_Callback func_list_data,
                        Eet_Descriptor_List_Free_Callback func_list_free,
                        Eet_Descriptor_Hash_Foreach_Callback func_hash_foreach,
                        Eet_Descriptor_Hash_Add_Callback func_hash_add,
                        Eet_Descriptor_Hash_Free_Callback func_hash_free);
/*
 * FIXME:
 *
 * Moving to this API from the old above. this breaks things when the
 * move happens - but be warned
 */
EINA_DEPRECATED EAPI Eet_Data_Descriptor *
 eet_data_descriptor2_new(const Eet_Data_Descriptor_Class *eddc);
EINA_DEPRECATED EAPI Eet_Data_Descriptor *
 eet_data_descriptor3_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * @brief   Creates a new data descriptor and returns a handle to the
 *          new data descriptor. 
 *
 * @since   1.2.3
 *
 * @remarks On creation it is empty, containing no contents
 *          describing anything other than the shell of the data structure.
 *
 * @remarks You add structure members to the data descriptor using the macros
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB() and
 *          EET_DATA_DESCRIPTOR_ADD_LIST(), depending on what type of member you are
 *          adding to the description.
 *
 * @remarks Once you have described all the members of a struct you want loaded or
 *          saved, eet can load and save those members for you, encode them into
 *          endian-independent serialised data chunks for transmission across a
 *          network or more.
 *
 * @remarks This function specially ignores str_direct_alloc and str_direct_free. It
 *          is useful when the eet_data you are reading does not have a dictionary,
 *          like network stream or IPC. It also means that all strings are allocated
 *          and duplicated in memory.
 *
 * @param[in]   eddc  The class from where to create the data descriptor
 * @return  A handle to the new data descriptor
 */
EAPI Eet_Data_Descriptor *
eet_data_descriptor_stream_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * @brief   Creates a new data descriptor and returns a handle to the
 *          new data descriptor. 
 * @since   1.2.3
 *
 * @remarks On creation it is empty, containing no contents
 *          describing anything other than the shell of the data structure.
 *
 * @remarks You add structure members to the data descriptor using the macros
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB() and
 *          EET_DATA_DESCRIPTOR_ADD_LIST(), depending on what type of member you are
 *          adding to the description.
 *
 * @remarks Once you have described all the members of a struct you want loaded or
 *          saved, eet can load and save those members for you, encode them into
 *          endian-independent serialised data chunks for transmission across a
 *          a network or more.
 *
 * @remarks This function uses str_direct_alloc and str_direct_free. It is
 *          useful when the eet_data you are reading come from a file and
 *          have a dictionary. This reduces memory use and improve the
 *          possibility for the OS to page this string out.
 *          However, the load speed and memory saving comes with some drawbacks to keep
 *          in mind. If you never modify the contents of the structures loaded from
 *          the file, all you need to remember is that closing the eet file makes
 *          the strings go away. On the other hand, should you need to free a string,
 *          before doing so you have to verify that it is not part of the eet dictionary.
 *          You can do this in the following way, assuming @a ef is a valid Eet_File
 *          and @a str is a string loaded from said file.
 *
 * @code
 * void eet_string_free(Eet_File *ef, const char *str)
 * {
 *    Eet_Dictionary *dict = eet_dictionary_get(ef);
 *    if (dict && eet_dictionary_string_check(dict, str))
 *      {
 *         // The file contains a dictionary and the given string is a part of
 *         // of it, so we can't free it, just return.
 *         return;
 *      }
 *    // We assume eina_stringshare is used on the descriptor
 *    eina_stringshare_del(str);
 * }
 * @endcode
 *
 * @param[in]   eddc  The class from where to create the data descriptor
 * @return  A handle to the new data descriptor
 */
EAPI Eet_Data_Descriptor *
eet_data_descriptor_file_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * @brief   Sets all the parameters of an Eet_Data_Descriptor_Class correctly
 *          when you use Eina data type with a stream.
 *
 * @details This function is a helper.
 * @since   1.2.3
 *
 * @remarks Unless there is a very specific reason to use this function directly,
 *          use the EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET() macro.
 *
 * @param[in]   eddc       The Eet_Data_Descriptor_Class you want to set
 * @param[in]   eddc_size  The size of the Eet_Data_Descriptor_Class at the compilation time
 * @param[in]   name       The name of the structure described by this class
 * @param[in]   size       The size of the structure described by this class
 * @return  @c EINA_TRUE if the structure is correctly set, \n 
 *          otherwise @c EINA_FALSE on failure \n
 *          The only reason that could make it fail is if you did give wrong parameter.
 */
EAPI Eina_Bool
eet_eina_stream_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
                                          unsigned int eddc_size,
                                          const char *name,
                                          int size);

/**
 * @brief   Sets all the parameter of an Eet_Data_Descriptor_Class correctly 
 *          when you use Eina data type with stream.
 *
 * @details This is a macro.
 * @since   1.2.3
 *
 * @param   clas  The Eet_Data_Descriptor_Class you want to set
 * @param   type  The type of the structure described by this class
 * @return  @c EINA_TRUE if the structure is correctly set, \n
 *          otherwise @c EINA_FALSE on failure \n 
 *          The only reason that could make it fail is if you did give wrong parameter.
 *
 * @see eet_data_descriptor_stream_new
 */
#define EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(clas, type) \
  (eet_eina_stream_data_descriptor_class_set(clas, sizeof (*(clas)), # type, sizeof(type)))

/**
 * @brief   Sets all the parameter of an Eet_Data_Descriptor_Class correctly 
 *          when you use Eina data type with a file.
 * @details This is a helper function.
 * @since   1.2.3
 *
 * @remarks Unless there is a very specific reason to use this function directly,
 *          use the EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET macro.
 *
 * @param[in]   eddc       The Eet_Data_Descriptor_Class you want to set
 * @param[in]   eddc_size  The size of the Eet_Data_Descriptor_Class at the compilation time
 * @param[in]   name       The name of the structure described by this class
 * @param[in]   size       The size of the structure described by this class
 * @return  @c EINA_TRUE if the structure is correctly set, \n
 *          otherwise @c EINA_FALSE on failure \n
 *          The only reason that could make it fail is if you did give wrong parameter.
 */
EAPI Eina_Bool
eet_eina_file_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
                                        unsigned int eddc_size,
                                        const char *name,
                                        int size);

/**
 * @brief   Sets all the parameter of an Eet_Data_Descriptor_Class correctly 
 *          when you use Eina data type with file.
 *
 * @details This macro is a helper.
 * @since   1.2.3
 *
 * @param   clas  The Eet_Data_Descriptor_Class you want to set
 * @param   type  The type of the structure described by this class
 * @return  @c EINA_TRUE if the structure is correctly set, \n
 *          otherwise @c EINA_FALSE on failure \n
 *          The only reason that could make it fail is if you did give wrong parameter.
 *
 * @see eet_data_descriptor_file_new
 */
#define EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(clas, type) \
  (eet_eina_file_data_descriptor_class_set(clas, sizeof (*(clas)), # type, sizeof(type)))

/**
 * @brief   Frees a data descriptor when it is not needed anymore.
 *
 * @details This function takes a data descriptor handle as a parameter and frees all
 *          data allocated for the data descriptor and the handle itself. After this
 *          call the descriptor is no longer valid.
 * @since   1.0.0
 *
 * @param[in]   edd  The data descriptor to free
 */
EAPI void
eet_data_descriptor_free(Eet_Data_Descriptor *edd);

/**
 * @brief   This function is an internal used by macros.
 *
 * @details This function is used by macros EET_DATA_DESCRIPTOR_ADD_BASIC(),
 *          EET_DATA_DESCRIPTOR_ADD_SUB() and EET_DATA_DESCRIPTOR_ADD_LIST(). 
 * @since   1.0.0
 *
 * @remarks It is complex to use by hand and should be left to be used by the macros, and
 *          thus is not documented.
 *
 * @param[in]   edd           The data descriptor handle to add element (member)
 * @param[in]   name          The name of element to be serialized
 * @param[in]   type          The type of element to be serialized, like #EET_T_INT \n
 *                        If #EET_T_UNKNOW, then it is considered to be a
 *                        group, list or hash.
 * @param[in]   group_type    If element type is #EET_T_UNKNOW, then the @a
 *                        group_type specifies if it is a list (#EET_G_LIST),
 *                        array (#EET_G_ARRAY) and so on \n 
 *                        If #EET_G_UNKNOWN, then the member is a subtype (pointer 
 *                        to another type defined by another #Eet_Data_Descriptor).
 * @param[in]   offset        The byte offset inside the source memory to be serialized
 * @param[in]   count         The number of elements (if #EET_G_ARRAY or #EET_G_VAR_ARRAY)
 * @param[in]   counter_name  The number of elements
 * @param[in]   subtype       The data descriptor, if this contains a subtype
 */
EAPI void
eet_data_descriptor_element_add(Eet_Data_Descriptor *edd,
                                const char *name,
                                int type,
                                int group_type,
                                int offset,
     /* int                  count_offset, */
                                int count,
                                const char *counter_name,
                                Eet_Data_Descriptor *subtype);

/**
 * @brief   Reads a data structure from an eet file and decodes it.
 * @since   1.0.0
 *
 * @remarks This function decodes a data structure stored in an eet file, returning
 *          a pointer to it if it decoded successfully, or @c NULL on failure. This
 *          can save a programmer dozens of hours of work in writing configuration
 *          file parsing and writing code, as eet does all that work for the program
 *          and presents a program-friendly data structure, just as the programmer
 *          likes. Eet can handle members being added or deleted from the data in
 *          storage and safely zero-fills unfilled members if they were not found
 *          in the data. It checks sizes and headers whenever it reads data, allowing
 *          the programmer to not worry about corrupt data.
 *
 * @remarks Once a data structure has been described by the programmer with the
 *          fields they wish to save or load, storing or retrieving a data structure
 *          from an eet file, or from a chunk of memory is as simple as a single
 *          function call.
 *
 * @param[in]   ef    The eet file handle to read from
 * @param[in]   edd   The data descriptor handle to use when decoding
 * @param[in]   name  The key the data is stored under in the eet file
 * @return  A pointer to the decoded data structure
 *
 * @see eet_data_read_cipher()
 */
EAPI void *
eet_data_read(Eet_File *ef,
              Eet_Data_Descriptor *edd,
              const char *name);

/**
 * @brief   Writes a data structure from memory and store in an eet file.
 *
 * @since   1.0.0
 *
 * @remarks This function is the reverse of eet_data_read(), saving a data structure
 *          to an eet file. The file must have been opening in write mode and the data
 *          is kept in memory until the file is either closed or eet_sync() is
 *          called to flush any unwritten changes.
 *
 * @param[in]   ef        The eet file handle to write to
 * @param[in]   edd       The data descriptor to use when encoding
 * @param[in]   name      The key to store the data under in the eet file
 * @param[in]   data      A pointer to the data structure to save and encode
 * @param[in]   compress  The compression flags for storage
 * @return  The bytes written on successful write, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_write_cipher()
 */
EAPI int
eet_data_write(Eet_File *ef,
               Eet_Data_Descriptor *edd,
               const char *name,
               const void *data,
               int compress);

typedef void (*Eet_Dump_Callback)(void *data, const char *str);

/**
 * @brief   Dumps an eet encoded data structure into ascii text.
 * @details This function takes a chunk of data encoded by
 *          eet_data_descriptor_encode() and convert it into human readable
 *          ascii text.  It does this by calling the @a dumpfunc callback
 *          for all new text that is generated. This callback should append
 *          to any existing text buffer and is passed the pointer @a
 *          dumpdata as a parameter as well as a string with new text to be
 *          appended.
 * @since   1.0.0
 *
 * @remarks The following is an example:
 *
 * @code
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
 * @param[in]   data_in   The pointer to the data to decode into a struct
 * @param[in]   size_in   The size of the data pointed to in bytes
 * @param[in]   dumpfunc  The function to call passed a string when new
 *                    data is converted to text
 * @param[in]   dumpdata  The data to pass to the @a dumpfunc callback
 * @return  @c 1 if the data structure is dumped into ascii successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_text_dump_cipher()
 */
EAPI int
eet_data_text_dump(const void *data_in,
                   int size_in,
                   Eet_Dump_Callback dumpfunc,
                   void *dumpdata);

/**
 * @brief   Takes an ascii encoding from eet_data_text_dump() and re-encodes in binary.
 *
 * @details This function parses the string pointed to by @a text and return
 *          an encoded data lump the same way eet_data_descriptor_encode() takes an
 *          in-memory data struct and encodes into a binary blob. @a text is a normal
 *          C string.
 * @since   1.0.0
 *
 * @param[in]   text      The pointer to the string data to parse and encode
 * @param[in]   textlen   The size of the string in bytes (not including @c 0
 *                    byte terminator)
 * @param[out]   size_ret  This gets filled in with the encoded data blob
 *                    size in bytes
 * @return  The encoded data on success, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_text_undump_cipher()
 */
EAPI void *
eet_data_text_undump(const char *text,
                     int textlen,
                     int *size_ret);

/**
 * @brief   Dumps an eet encoded data structure from an eet file into ascii text.
 *
 * @details This function takes an open and valid eet file from
 *          eet_open() request the data encoded by
 *          eet_data_descriptor_encode() corresponding to the key @a name
 *          and convert it into human readable ascii text. It does this by
 *          calling the @a dumpfunc callback for all new text that is
 *          generated. This callback should append to any existing text
 *          buffer and is passed the pointer @a dumpdata as a parameter
 *          as well as a string with new text to be appended.
 * @since   1.0.0
 *
 * @param[in]   ef        A valid eet file handle
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[in]   dumpfunc  The function to call passed a string when new
 *                    data is converted to text
 * @param[in]   dumpdata  The data to pass to the @a dumpfunc callback
 * @return  @c 1 if the data structure is dumped into ascii text successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_dump_cipher()
 */
EAPI int
eet_data_dump(Eet_File *ef,
              const char *name,
              Eet_Dump_Callback dumpfunc,
              void *dumpdata);

/**
 * @brief   Takes an ascii encoding from eet_data_dump() and re-encodes in binary.
 *
 * @details This function parses the string pointed to by @a text,
 *          encode it the same way eet_data_descriptor_encode() takes an
 *          in-memory data struct and encodes into a binary blob.
 * @since   1.0.0
 *
 * @remarks The data (optionally compressed) is in RAM, pending a flush to
 *          disk (it stays in RAM till the eet file handle is closed though).
 *
 * @param[in]   ef        A valid eet file handle
 * @param[in]   name      The name of the entry \n 
 *                    For example: "/base/file_i_want".
 * @param[in]   text      The pointer to the string data to parse and encode
 * @param[in]   textlen   The size of the string in bytes (not including @c 0
 *                    byte terminator)
 * @param[in]   compress  The compression flags (1 = compress, 0 = do not compress)
 * @return  @c 1 if the ascii encoding is re-encoded successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_undump_cipher()
 */
EAPI int
eet_data_undump(Eet_File *ef,
                const char *name,
                const char *text,
                int textlen,
                int compress);

/**
 * @brief   Decodes a data structure from an arbitrary location in memory.
 *
 * @details This function decodes a data structure that has been encoded using
 *          eet_data_descriptor_encode(), and return a data structure with all its
 *          elements filled out, if successful, or @c NULL on failure.
 * @since   1.0.0
 *
 * @remarks The data to be decoded is stored at the memory pointed to by @a data_in,
 *          and is described by the descriptor pointed to by @a edd. The data size is
 *          passed in as the value to @a size_in, and must be greater than @c 0 to
 *          succeed.
 *
 * @remarks This function is useful for decoding data structures delivered to the
 *          application by means other than an eet file, such as an IPC or socket
 *          connection, raw files, and shared memory.
 *
 * @remarks See eet_data_read() for more information.
 *
 * @param[in]   edd      The data descriptor to use when decoding
 * @param[in]   data_in  The pointer to the data to decode into a struct
 * @param[in]   size_in  The size of the data pointed to in bytes
 * @return  A valid decoded struct pointer on success, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_descriptor_decode_cipher()
 */
EAPI void *
eet_data_descriptor_decode(Eet_Data_Descriptor *edd,
                           const void *data_in,
                           int size_in);

/**
 * @brief   Encodes a data struct to memory and returns that encoded data.
 *
 * @details This function takes a data structutre in memory and encodes it into a
 *          serialised chunk of data that can be decoded again by
 *          eet_data_descriptor_decode(). This is useful for being able to transmit
 *          data structures across sockets, pipes, IPC or shared file mechanisms,
 *          without having to worry about memory space, machine type, endianness, and so on.
 * @since   1.0.0
 *
 * @remarks The parameter @a edd must point to a valid data descriptor, and
 *          @a data_in must point to the right data structure to encode. If not, the
 *          encoding may fail.
 *
 * @remarks On success, a non-NULL valid pointer is returned and what @a size_ret
 *          points to is set to the size of this decoded data, in bytes. When the
 *          encoded data is no longer needed, call free() on it. On failure @c NULL is
 *          returned and what @a size_ret points to is set to @c 0.
 *
 * @remarks See eet_data_write() for more information.
 *
 * @param[in]   edd       The data  descriptor to use when encoding
 * @param[in]   data_in   The pointer to the struct to encode into data
 * @param[out]   size_ret  The pointer to the an int to be filled with the decoded size
 * @return  A valid encoded data chunk on success, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_descriptor_encode_cipher()
 */
EAPI void *
eet_data_descriptor_encode(Eet_Data_Descriptor *edd,
                           const void *data_in,
                           int *size_ret);

/**
 * @brief   Adds a basic data element to a data descriptor.
 *
 * @details This macro is a convenience macro provided to add a member to
 *          the data descriptor @a edd. The type of the structure is
 *          provided as the @a struct_type parameter (for example: struct
 *          my_struct). The @a name parameter defines a string that is
 *          used to uniquely name that member of the struct (it is suggested
 *          to use the struct member itself). The @a member parameter is
 *          the actual struct member itself (for example: values), and @a type is the
 *          basic data type of the member. The valid values for @a type are: EET_T_CHAR, EET_T_SHORT,
 *          EET_T_INT, EET_T_LONG_LONG, EET_T_FLOAT, EET_T_DOUBLE, EET_T_UCHAR,
 *          EET_T_USHORT, EET_T_UINT, EET_T_ULONG_LONG or EET_T_STRING.
 * @since   1.0.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never change.
 * @param   member       The struct member itself to be encoded
 * @param   type         The type of the member to encode
 */
#define EET_DATA_DESCRIPTOR_ADD_BASIC(edd, struct_type, name, member, type) \
  do {                                                                      \
       struct_type ___ett;                                                  \
       eet_data_descriptor_element_add(edd, name, type, EET_G_UNKNOWN,      \
                                       (char *)(& (___ett.member)) -        \
                                       (char *)(& (___ett)),                \
                                       0, /* 0,  */ NULL, NULL);            \
    } while(0)

/**
 * @brief   Adds a sub-element type to a data descriptor.
 *
 * @details This macro lets you easily add a sub-type (a struct that is pointed to
 *          by this one). All the parameters are the same as for
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @a subtype being the exception.
 *          This must be the data descriptor of the struct that is pointed to by
 *          this element.
 * @since   1.0.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   subtype      The type of sub-type struct to add
 */
#define EET_DATA_DESCRIPTOR_ADD_SUB(edd, struct_type, name, member, subtype)   \
  do {                                                                         \
       struct_type ___ett;                                                     \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNKNOWN, \
                                       (char *)(& (___ett.member)) -           \
                                       (char *)(& (___ett)),                   \
                                       0, /* 0,  */ NULL, subtype);            \
    } while (0)

/**
 * @brief   Adds a linked list type to a data descriptor.
 *
 * @details This macro lets you easily add a linked list of other data types. All the
 *          parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
 *          @a subtype being the exception. This must be the data descriptor of the
 *          element that is in each member of the linked list to be stored.
 * @since   1.0.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   subtype      The type of linked list member to add
 */
#define EET_DATA_DESCRIPTOR_ADD_LIST(edd, struct_type, name, member, subtype) \
  do {                                                                        \
       struct_type ___ett;                                                    \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_LIST,   \
                                       (char *)(& (___ett.member)) -          \
                                       (char *)(& (___ett)),                  \
                                       0, /* 0,  */ NULL, subtype);           \
    } while (0)

/**
 * @brief   Adds a linked list of string to a data descriptor.
 *
 * @details This macro lets you easily add a linked list of char *. All the
 *          parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC().
 * @since   1.5.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 */
#define EET_DATA_DESCRIPTOR_ADD_LIST_STRING(edd, struct_type, name, member) \
  do {                                                                      \
       struct_type ___ett;                                                  \
       eet_data_descriptor_element_add(edd, name, EET_T_STRING, EET_G_LIST, \
                                       (char *)(& (___ett.member)) -        \
                                       (char *)(& (___ett)),                \
                                       0, /* 0,  */ NULL, NULL);            \
    } while (0)

/**
 * @brief   Adds a hash type to a data descriptor.
 *
 * @details This macro lets you easily add a hash of other data types. All the
 *          parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
 *          @a subtype being the exception. This must be the data descriptor of the
 *          element that is in each member of the hash to be stored.
 *          The hash keys must be strings.
 * @since   1.0.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   subtype      The type of hash member to add
 */
#define EET_DATA_DESCRIPTOR_ADD_HASH(edd, struct_type, name, member, subtype) \
  do {                                                                        \
       struct_type ___ett;                                                    \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_HASH,   \
                                       (char *)(& (___ett.member)) -          \
                                       (char *)(& (___ett)),                  \
                                       0, /* 0,  */ NULL, subtype);           \
    } while (0)

/**
 * @brief   Adds a hash of string to a data descriptor.
 *
 * @details This macro lets you easily add a hash of string elements. All the
 *          parameters are the same as for EET_DATA_DESCRIPTOR_ADD_HASH().
 * @since   1.3.4
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 */
#define EET_DATA_DESCRIPTOR_ADD_HASH_STRING(edd, struct_type, name, member) \
  do {                                                                      \
       struct_type ___ett;                                                  \
       eet_data_descriptor_element_add(edd, name, EET_T_STRING, EET_G_HASH, \
                                       (char *)(& (___ett.member)) -        \
                                       (char *)(& (___ett)),                \
                                       0, /* 0,  */ NULL, NULL);            \
    } while (0)

/**
 * @brief   Adds an array of basic data elements to a data descriptor.
 *
 * @details This macro lets you easily add a fixed size array of basic data
 *          types. All the parameters are the same as for
 *          EET_DATA_DESCRIPTOR_ADD_BASIC().
 *          The array must be defined with a fixed size in the declaration of the
 *          struct containing it.
 * @since   1.5.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   type         The type of the member to encode
 */
#define EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(edd, struct_type, name, member, type) \
  do {                                                                            \
       struct_type ___ett;                                                        \
       eet_data_descriptor_element_add(edd, name, type, EET_G_ARRAY,              \
                                       (char *)(& (___ett.member)) -              \
                                       (char *)(& (___ett)),                      \
                                       sizeof(___ett.member) /                    \
                                       sizeof(___ett.member[0]),                  \
                                       NULL, NULL);                               \
    } while(0)

/**
 * @brief   Adds a variable array of basic data elements to a data descriptor.
 *
 * @details This macro lets you easily add a variable size array of basic data
 *          types. All the parameters are the same as for
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(). This assumes you have
 *          a struct member (of type EET_T_INT) called member_count (note the
 *          _count appended to the member) that holds the number of items in
 *          the array. This array is allocated separately to the struct it is in.
 *
 * @since   1.6.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   type         The type of the member to encode
 */
#define EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(edd, struct_type, name, member, type) \
  do {                                                                                \
       struct_type ___ett;                                                            \
       eet_data_descriptor_element_add(edd, name, type, EET_G_VAR_ARRAY,              \
                                       (char *)(& (___ett.member)) -                  \
                                       (char *)(& (___ett)),                          \
                                       (char *)(& (___ett.member ## _count)) -        \
                                       (char *)(& (___ett)),                          \
                                       NULL,                                          \
                                       NULL);                                         \
    } while(0)

/**
 * @brief   Adds a fixed size array type to a data descriptor.
 *
 * @details This macro lets you easily add a fixed size array of other data
 *          types. All the parameters are the same as for
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @a subtype being the
 *          exception. This must be the data descriptor of the element that
 *          is in each member of the array to be stored.
 *          The array must be defined with a fixed size in the declaration of the
 *          struct containing it.
 * @since   1.0.2
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   subtype      The type of hash member to add
 */
#define EET_DATA_DESCRIPTOR_ADD_ARRAY(edd, struct_type, name, member, subtype)   \
  do {                                                                           \
       struct_type ___ett;                                                       \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_ARRAY,     \
                                       (char *)(& (___ett.member)) -             \
                                       (char *)(& (___ett)),                     \
                                       /* 0,  */ sizeof(___ett.member) /         \
                                       sizeof(___ett.member[0]), NULL, subtype); \
    } while (0)

/**
 * @brief   Adds a variable size array type to a data descriptor.
 *
 * @details This macro lets you easily add a variable size array of other data
 *          types. All the parameters are the same as for
 *          EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @a subtype being the
 *          exception. This must be the data descriptor of the element that
 *          is in each member of the array to be stored. This assumes you have
 *          a struct member (of type EET_T_INT) called member_count (note the
 *          _count appended to the member) that holds the number of items in
 *          the array. This array is allocated separately to the struct it is in.
 * @since   1.0.2
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 * @param   subtype      The type of hash member to add
 */
#define EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(edd, struct_type, name, member, subtype) \
  do {                                                                             \
       struct_type ___ett;                                                         \
       eet_data_descriptor_element_add(edd,                                        \
                                       name,                                       \
                                       EET_T_UNKNOW,                               \
                                       EET_G_VAR_ARRAY,                            \
                                       (char *)(& (___ett.member)) -               \
                                       (char *)(& (___ett)),                       \
                                       (char *)(& (___ett.member ## _count)) -     \
                                       (char *)(& (___ett)),                       \
                                       /* 0,  */ NULL,                             \
                                       subtype);                                   \
    } while (0)

/**
 * @brief   Adds a variable size array type to a data descriptor.
 *
 * @details This macro lets you easily add a variable size array of strings. All
 *          the parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC().
 * @since   1.4.0
 *
 * @param   edd          The data descriptor to add the type to
 * @param   struct_type  The type of the struct
 * @param   name         The string name to use to encode or decode this member \n
 *                       This must be a constant global and never changes.
 * @param   member       The struct member itself to be encoded
 */
#define EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING(edd, struct_type, name, member) \
  do {                                                                           \
       struct_type ___ett;                                                       \
       eet_data_descriptor_element_add(edd,                                      \
                                       name,                                     \
                                       EET_T_STRING,                             \
                                       EET_G_VAR_ARRAY,                          \
                                       (char *)(& (___ett.member)) -             \
                                       (char *)(& (___ett)),                     \
                                       (char *)(& (___ett.member ## _count)) -   \
                                       (char *)(& (___ett)),                     \
                                       /* 0,  */ NULL,                           \
                                       NULL);                                    \
    } while (0)

/**
 * @brief   Adds a union type to a data descriptor.
 *
 * @details This macro lets you easily add a union with a member that specifies what is inside.
 *          The @a unified_type is an Eet_Data_Descriptor, but only the entry that matches the name
 *          returned by type_get is used for each serialized data. The type_get and type_set
 *          callback of @a unified_type should be defined.
 * @since   1.2.4
 *
 * @param   edd           The data descriptor to add the type to
 * @param   struct_type   The type of the struct
 * @param   name          The string name to use to encode or decode this member \n
 *                        This must be a constant global and never changes.
 * @param   member        The struct member itself to be encoded
 * @param   type_member   The member that give hints on what is in the union
 * @param   unified_type  The union type
 *
 * @see Eet_Data_Descriptor_Class
 */
#define EET_DATA_DESCRIPTOR_ADD_UNION(edd, struct_type, name, member, type_member, unified_type) \
  do {                                                                                           \
       struct_type ___ett;                                                                       \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNION,                     \
                                       (char *)(& (___ett.member)) -                             \
                                       (char *)(& (___ett)),                                     \
                                       (char *)(& (___ett.type_member)) -                        \
                                       (char *)(& (___ett)),                                     \
                                       NULL, unified_type);                                      \
    } while (0)

/**
 * @brief   Adds an automatically selectable type to a data descriptor.
 *
 * @details This macro lets you easily define what the content of @a member points to depending of
 *          the content of @a type_member. The type_get and type_set callback of @a unified_type should
 *          be defined. If the the type is not known at the time of restoring it, eet still calls
 *          type_set of @a unified_type but the pointer is set to a serialized binary representation
 *          of what eet know. This makes it possible, to save this pointer again by just returning the string
 *          given previously and telling it by setting unknown to @c EINA_TRUE.
 * @since   1.2.4
 *
 * @param   edd           The data descriptor to add the type to
 * @param   struct_type   The type of the struct
 * @param   name          The string name to use to encode or decode this member \n
 *                        This must be a constant global and never changes.
 * @param   member        The struct member itself to be encoded.
 * @param   type_member   The member that give hints on what is in the union.
 * @param   unified_type  Describe all possible type the union could handle.
 *
 * @see Eet_Data_Descriptor_Class
 */
#define EET_DATA_DESCRIPTOR_ADD_VARIANT(edd, struct_type, name, member, type_member, unified_type) \
  do {                                                                                             \
       struct_type ___ett;                                                                         \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_VARIANT,                     \
                                       (char *)(& (___ett.member)) -                               \
                                       (char *)(& (___ett)),                                       \
                                       (char *)(& (___ett.type_member)) -                          \
                                       (char *)(& (___ett)),                                       \
                                       NULL, unified_type);                                        \
    } while (0)

/**
 * brief    Adds a mapping to a data descriptor that is used by union, variant or inherited type.
 * @since   1.2.4
 * @param   unified_type  The data descriptor to add the mapping to
 * @param   name          The string name to get/set type
 * @param   subtype       The matching data descriptor
 * @see Eet_Data_Descriptor_Class
 */
#define EET_DATA_DESCRIPTOR_ADD_MAPPING(unified_type, name, subtype) \
  eet_data_descriptor_element_add(unified_type,                      \
                                  name,                              \
                                  EET_T_UNKNOW,                      \
                                  EET_G_UNKNOWN,                     \
                                  0,                                 \
                                  0,                                 \
                                  NULL,                              \
                                  subtype)

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_Data_Cipher_Group Eet Data Serialization using A Ciphers
 * @ingroup Eet_Data_Group
 *
 * Most of the @ref Eet_Data_Group have alternative versions that
 * accounts for ciphers to protect their content.
 *
 * @see @ref Eet_Cipher_Group
 *
 * @{
 */

/**
 * @brief   Reads a data structure from an eet file and decodes it using a cipher.
 *
 * @details This function decodes a data structure stored in an eet file, returning
 *          a pointer to it if it decoded successfully, or @c NULL on failure. This
 *          can save a programmer dozens of hours of work in writing configuration
 *          file parsing and writing code, as eet does all that work for the program
 *          and presents a program-friendly data structure, just as the programmer
 *          likes. Eet can handle members being added or deleted from the data in
 *          storage and safely zero-fills unfilled members if they were not found
 *          in the data. It checks sizes and headers whenever it reads data, allowing
 *          the programmer to not worry about corrupt data.
 * @since   1.0.0
 *
 * @remarks Once a data structure has been described by the programmer with the
 *          fields they wish to save or load, storing or retrieving a data structure
 *          from an eet file, or from a chunk of memory is as simple as a single
 *          function call.
 *
 * @param[in]   ef          The eet file handle to read from
 * @param[in]   edd         The data descriptor handle to use when decoding
 * @param[in]   name        The key the data is stored under in the eet file
 * @param[in]   cipher_key  The key to use as cipher
 * @return  A pointer to the decoded data structure
 *
 * @see eet_data_read()
 */
EAPI void *
eet_data_read_cipher(Eet_File *ef,
                     Eet_Data_Descriptor *edd,
                     const char *name,
                     const char *cipher_key);

/**
 * @brief   Reads a data structure from an eet file and decodes it into a buffer using a cipher.
 *
 * @details This function decodes a data structure stored in an eet file, returning
 *          a pointer to it if it decoded successfully, or NULL on failure. This
 *          can save a programmer dozens of hours of work in writing configuration
 *          file parsing and writing code, as eet does all that work for the program
 *          and presents a program-friendly data structure, just as the programmer
 *          likes. Eet can handle members being added or deleted from the data in
 *          storage and safely zero-fills unfilled members if they were not found
 *          in the data. It checks sizes and headers whenever it reads data, allowing
 *          the programmer to not worry about corrupt data.
 * @since   1.10.0
 *
 * @remarks Once a data structure has been described by the programmer with the
 *          fields they wish to save or load, storing or retrieving a data structure
 *          from an eet file, or from a chunk of memory is as simple as a single
 *          function call.
 *
 * @param[in]   ef          The eet file handle to read from
 * @param[in]   edd         The data descriptor handle to use when decoding
 * @param[in]   name        The key the data is stored under in the eet file
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   buffer      The buffer
 * @param[in]   buffer_size The buffer size
 * @return  A pointer to buffer if successful, \n
 *          otherwise @c NULL on error
 *
 * @see eet_data_read_cipher()
 */
EAPI void *
eet_data_read_cipher_buffer(Eet_File            *ef,
                            Eet_Data_Descriptor *edd,
                            const char          *name,
                            const char          *cipher_key,
                            char                *buffer,
                            int                 buffer_size);

/**
 * @brief   Reads a data structure from an eet extended attribute and decodes it using a cipher.
 *
 * @details This function decodes a data structure stored in an eet extended attribute,
 *          returning a pointer to it if it decoded successfully, or @c NULL on failure.
 *          Eet can handle members being added or deleted from the data in
 *          storage and safely zero-fills unfilled members if they were not found
 *          in the data. It checks sizes and headers whenever it reads data, allowing
 *          you to not worry about corrupt data.
 * @since   1.5.0
 *
 * @remarks Once you have described a data structure with the
 *          fields you wish to save or load, storing or retrieving a data structure
 *          from an eet file, from a chunk of memory or from an extended attribute
 *          is as simple as a single function call.
 *
 * @param[in]   filename    The file to extract the extended attribute from
 * @param[in]   attribute   The attribute to get the data from
 * @param[in]   edd         The data descriptor handle to use when decoding
 * @param[in]   cipher_key  The key to use as cipher
 * @return  A pointer to the decoded data structure, \n
 *          otherwise @c NULL on failure
 */
EAPI void *
eet_data_xattr_cipher_get(const char *filename,
                          const char *attribute,
                          Eet_Data_Descriptor *edd,
                          const char *cipher_key);

/**
 * @brief   Writes a data structure from memory and store in an eet file
 *          using a cipher.
 *
 * @details This function is the reverse of eet_data_read_cipher(), saving a data structure
 *          to an eet file.
 *
 * @since   1.0.0
 *
 * @param[in]   ef          The eet file handle to write to
 * @param[in]   edd         The data descriptor to use when encoding
 * @param[in]   name        The key to store the data under in the eet file
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   data        A pointer to the data structure to save and encode
 * @param[in]   compress    The compression flags for storage
 * @return  The bytes written on successful write, \n
 *          otherwise @c 0 on failure
 */
EAPI int
eet_data_write_cipher(Eet_File *ef,
                      Eet_Data_Descriptor *edd,
                      const char *name,
                      const char *cipher_key,
                      const void *data,
                      int compress);

/**
 * @brief   Writes a data structure from memory and store in an eet extended attribute
 *          using a cipher.
 *
 * @details This function is the reverse of eet_data_xattr_cipher_get(), saving a data structure
 *          to an eet extended attribute.
 * @since   1.5.0
 *
 * @param[in]   filename    The file to write the extended attribute to
 * @param[in]   attribute   The attribute to store the data to
 * @param[in]   edd         The data descriptor to use when encoding
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   data        A pointer to the data structure to save and encode
 * @param[in]   flags       The policy to use when setting the data
 * @return  @c EINA_TRUE if structure is written successfully, \n
 *          otherwise @c EINA_FALSE on failure
 */
EAPI Eina_Bool
eet_data_xattr_cipher_set(const char *filename,
                          const char *attribute,
                          Eet_Data_Descriptor *edd,
                          const char *cipher_key,
                          const void *data,
                          Eina_Xattr_Flags flags);

/**
 * @brief   Dumps an eet encoded data structure into ascii text using a cipher.
 *
 * @details This function takes a chunk of data encoded by
 *          eet_data_descriptor_encode() and convert it into human readable
 *          ascii text. It does this by calling the @a dumpfunc callback
 *          for all new text that is generated. This callback should append
 *          to any existing text buffer and is passed the pointer @a
 *          dumpdata as a parameter as well as a string with new text to be
 *          appended.
 * @since   1.0.0
 *
 * @remarks The following is an example:
 *
 * @code
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
 *   eet_data_text_dump_cipher(data, cipher_key, len, output, NULL);
 * }
 * @endcode
 *
 * @param[in]   data_in     The pointer to the data to decode into a struct
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size_in     The size of the data pointed to in bytes
 * @param[in]   dumpfunc    The function to call passed a string when new
 *                      data is converted to text
 * @param[in]   dumpdata    The data to pass to the @a dumpfunc callback
 * @return  @c 1 if the structure is dumped into ascii successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_text_dump()
 */
EAPI int
eet_data_text_dump_cipher(const void *data_in,
                          const char *cipher_key,
                          int size_in,
                          Eet_Dump_Callback dumpfunc,
                          void *dumpdata);

/**
 * @brief   Takes an ascii encoding from eet_data_text_dump() and re-encodes 
 *          in binary using a cipher.
 *
 * @details This function parses the string pointed to by @a text and return
 *          an encoded data lump the same way eet_data_descriptor_encode() takes an
 *          in-memory data struct and encodes into a binary blob. @a text is a normal
 *          C string.
 * @since   1.0.0
 *
 * @param[in]   text        The pointer to the string data to parse and encode
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   textlen     The size of the string in bytes (not including @c 0 byte terminator)
 * @param[out]   size_ret    This gets filled in with the encoded data blob size in bytes
 * @return  The encoded data, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_text_undump()
 */
EAPI void *
eet_data_text_undump_cipher(const char *text,
                            const char *cipher_key,
                            int textlen,
                            int *size_ret);

/**
 * @brief   Dumps an eet encoded data structure from an eet file into ascii
 *          text using a cipher.
 *
 * @details This function takes an open and valid eet file from
 *          eet_open() request the data encoded by
 *          eet_data_descriptor_encode() corresponding to the key @a name
 *          and convert it into human readable ascii text. It does this by
 *          calling the @a dumpfunc callback for all new text that is
 *          generated. This callback should append to any existing text
 *          buffer and is passed the pointer @a dumpdata as a parameter
 *          as well as a string with new text to be appended.
 * @since   1.0.0
 *
 * @param[in]   ef          A valid eet file handle
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   dumpfunc    The function to call passed a string when new
 *                      data is converted to text
 * @param[in]   dumpdata    The data to pass to the @a dumpfunc callback
 * @return  @c 1 if the encoded data structure is dumped successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_dump()
 */
EAPI int
eet_data_dump_cipher(Eet_File *ef,
                     const char *name,
                     const char *cipher_key,
                     Eet_Dump_Callback dumpfunc,
                     void *dumpdata);

/**
 * @brief   Takes an ascii encoding from eet_data_dump() and re-encodes in
 *          binary using a cipher.
 *
 * @details This function parses the string pointed to by @a text,
 *          encode it the same way eet_data_descriptor_encode() takes an
 *          in-memory data struct and encodes into a binary blob.
 * @since   1.0.0
 *
 * @remarks The data (optionally compressed) is in RAM, pending a flush to
 *          disk (it stays in RAM till the eet file handle is closed though).
 *
 * @param[in]   ef          A valid eet file handle
 * @param[in]   name        The name of the entry \n 
 *                      For example: "/base/file_i_want".
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   text        The pointer to the string data to parse and encode
 * @param[in]   textlen     The size of the string in bytes (not including @c 0 byte terminator)
 * @param[in]   compress    The compression flags (1 = compress, 0 = do not compress)
 * @return  @c 1 if the ascii encoding is re-encoded successfully, \n
 *          otherwise @c 0 on failure
 *
 * @see eet_data_undump()
 */
EAPI int
eet_data_undump_cipher(Eet_File *ef,
                       const char *name,
                       const char *cipher_key,
                       const char *text,
                       int textlen,
                       int compress);

/**
 * @brief   Decodes a data structure from an arbitrary location in memory
 *          using a cipher.
 *
 * @details This function decodes a data structure that has been encoded using
 *          eet_data_descriptor_encode(), and return a data structure with all its
 *          elements filled out, if successful, or @c NULL on failure.
 * @since   1.0.0
 *
 * @remarks The data to be decoded is stored at the memory pointed to by @a data_in,
 *          and is described by the descriptor pointed to by @a edd. The data size is
 *          passed in as the value to @a size_in, and must be greater than @c 0 to succeed.
 *
 * @remarks This function is useful for decoding data structures delivered to the
 *          application by means other than an eet file, such as an IPC or socket
 *          connection, raw files, and shared memory.
 *
 * @remarks See eet_data_read() for more information.
 *
 * @param[in]   edd         The data descriptor to use when decoding
 * @param[in]   data_in     The pointer to the data to decode into a struct
 * @param[in]   cipher_key  The key to use as cipher
 * @param[in]   size_in     The size of the data pointed to in bytes
 * @return  A valid decoded struct pointer on success, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_descriptor_decode()
 */
EAPI void *
eet_data_descriptor_decode_cipher(Eet_Data_Descriptor *edd,
                                  const void *data_in,
                                  const char *cipher_key,
                                  int size_in);

/**
 * @brief   Encodes a data struct to memory and return that encoded data
 *          using a cipher.
 *
 * @details This function takes a data structure in memory and encodes it into a
 *          serialised chunk of data that can be decoded again by
 *          eet_data_descriptor_decode(). This is useful for being able to transmit
 *          data structures across sockets, pipes, IPC or shared file mechanisms,
 *          without having to worry about memory space, machine type, endianess, and so on.
 * @since   1.0.0
 *
 * @remarks The parameter @a edd must point to a valid data descriptor, and
 *          @a data_in must point to the right data structure to encode. If not, the
 *          encoding may fail.
 *
 * @remarks On success a non NULL valid pointer is returned and what @a size_ret
 *          points to is set to the size of this decoded data, in bytes. When the
 *          encoded data is no longer needed, call free() on it. On failure @c NULL is
 *          returned and what @a size_ret points to is set to @c 0.
 *
 * @remarks See eet_data_write() for more information.
 *
 * @param[in]   edd         The data descriptor to use when encoding
 * @param[in]   data_in     The pointer to the struct to encode into data
 * @param[in]   cipher_key  The key to use as cipher
 * @param[out]   size_ret    The pointer to the an int to be filled with the decoded size
 * @return  A valid encoded data chunk, \n
 *          otherwise @c NULL on failure
 *
 * @see eet_data_descriptor_encode()
 */
EAPI void *
eet_data_descriptor_encode_cipher(Eet_Data_Descriptor *edd,
                                  const void *data_in,
                                  const char *cipher_key,
                                  int *size_ret);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_Node_Group Low-level Serialization Structures.
 * @ingroup Eet_Group
 *
 * @brief This group provides functions that create, destroy and manipulate serialization nodes
 *        used by @ref Eet_Data_Group.
 *
 * @{
 */

/**
 * @typedef Eet_Node
 * @brief The structure type containing an opaque handle to manage serialization node.
 */
typedef struct _Eet_Node Eet_Node;

/**
 * @typedef Eet_Node_Data
 * @brief The structure type containing a union that can fit any kind of node.
 */
typedef struct _Eet_Node_Data Eet_Node_Data;

/**
 * @internal
 * @struct _Eet_Node_Data
 * @brief The structure type containing a union that can fit any kind of node.
 */
struct _Eet_Node_Data
{
   union {
      char               c;
      short              s;
      int                i;
      long long          l;
      float              f;
      double             d;
      unsigned char      uc;
      unsigned short     us;
      unsigned int       ui;
      unsigned long long ul;
      const char        *str;
   } value;
};

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_char_new(const char *name,
                  char c);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_short_new(const char *name,
                   short s);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_int_new(const char *name,
                 int i);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_long_long_new(const char *name,
                       long long l);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_float_new(const char *name,
                   float f);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_double_new(const char *name,
                    double d);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_unsigned_char_new(const char *name,
                           unsigned char uc);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_unsigned_short_new(const char *name,
                            unsigned short us);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_unsigned_int_new(const char *name,
                          unsigned int ui);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_unsigned_long_long_new(const char *name,
                                unsigned long long l);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_string_new(const char *name,
                    const char *str);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_inlined_string_new(const char *name,
                            const char *str);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_null_new(const char *name);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_list_new(const char *name,
                  Eina_List *nodes);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_array_new(const char *name,
                   int count,
                   Eina_List *nodes);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_var_array_new(const char *name,
                       Eina_List *nodes);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_hash_new(const char *name,
                  const char *key,
                  Eet_Node *node);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_struct_new(const char *name,
                    Eina_List *nodes);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_node_struct_child_new(const char *parent,
                          Eet_Node *child);

/**
 * @brief   Gets a node's child nodes.
 * @since   1.5
 *
 * @param[in]   node  The node
 * @return  The first child node which contains a pointer to the
 *          next child node and the parent
 */
EAPI Eet_Node *
eet_node_children_get(Eet_Node *node);

/**
 * @brief   Gets the next node in a list of nodes.
 * @since   1.5
 *
 * @param[in]   node  The node
 * @return  A node which contains a pointer to the
 *          next child node and the parent
 */
EAPI Eet_Node *
eet_node_next_get(Eet_Node *node);

/**
 * @brief   Gets the parent node of a node.
 * @since   1.5
 *
 * @param[in]   node  The node
 * @return  The parent node of @a node
 */
EAPI Eet_Node *
eet_node_parent_get(Eet_Node *node);

/**
 * TODO FIX ME
 */
EAPI void
eet_node_list_append(Eet_Node *parent,
                     const char *name,
                     Eet_Node *child);

/**
 * TODO FIX ME
 */
EAPI void
eet_node_struct_append(Eet_Node *parent,
                       const char *name,
                       Eet_Node *child);

/**
 * TODO FIX ME
 */
EAPI void
eet_node_hash_add(Eet_Node *parent,
                  const char *name,
                  const char *key,
                  Eet_Node *child);

/**
 * TODO FIX ME
 */
EAPI void
eet_node_dump(Eet_Node *n,
              int dumplevel,
              Eet_Dump_Callback dumpfunc,
              void *dumpdata);

/**
 * @brief   Gets the type of a node.
 * @since   1.5
 *
 * @param[in]   node  The node
 * @return  The node's type (EET_T_$TYPE)
 */
EAPI int
eet_node_type_get(Eet_Node *node);

/**
 * @brief   Gets the node's data.
 * @since   1.5
 *
 * @param[in]   node  The node
 * @return  The data contained in the node
 */
EAPI Eet_Node_Data *
eet_node_value_get(Eet_Node *node);

/**
 * TODO FIX ME
 */
EAPI void
eet_node_del(Eet_Node *n);

/**
 * TODO FIX ME
 */
EAPI void *
eet_data_node_encode_cipher(Eet_Node *node,
                            const char *cipher_key,
                            int *size_ret);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_data_node_decode_cipher(const void *data_in,
                            const char *cipher_key,
                            int size_in);

/**
 * TODO FIX ME
 */
EAPI Eet_Node *
eet_data_node_read_cipher(Eet_File *ef,
                          const char *name,
                          const char *cipher_key);

/**
 * TODO FIX ME
 */
EAPI int
eet_data_node_write_cipher(Eet_File *ef,
                           const char *name,
                           const char *cipher_key,
                           Eet_Node *node,
                           int compress);

/* EXPERIMENTAL: THIS API MAY CHANGE IN THE FUTURE, USE IT ONLY IF YOU KNOW WHAT YOU ARE DOING. */

/**
 * @typedef Eet_Node_Walk
 * @brief The structure type that describes how to walk trees of #Eet_Node.
 */
typedef struct _Eet_Node_Walk Eet_Node_Walk;

typedef void *              (*Eet_Node_Walk_Struct_Alloc_Callback)(const char *type, void *user_data);
typedef void                (*Eet_Node_Walk_Struct_Add_Callback)(void *parent, const char *name, void *child, void *user_data);
typedef void *              (*Eet_Node_Walk_Array_Callback)(Eina_Bool variable, const char *name, int count, void *user_data);
typedef void                (*Eet_Node_Walk_Insert_Callback)(void *array, int index, void *child, void *user_data);
typedef void *              (*Eet_Node_Walk_List_Callback)(const char *name, void *user_data);
typedef void                (*Eet_Node_Walk_Append_Callback)(void *list, void *child, void *user_data);
typedef void *              (*Eet_Node_Walk_Hash_Callback)(void *parent, const char *name, const char *key, void *value, void *user_data);
typedef void *              (*Eet_Node_Walk_Simple_Callback)(int type, Eet_Node_Data *data, void *user_data);

/**
 * @internal
 * @struct _Eet_Node_Walk
 * @brief The structure type that describes how to walk trees of #Eet_Node.
 */
struct _Eet_Node_Walk
{
   Eet_Node_Walk_Struct_Alloc_Callback struct_alloc;
   Eet_Node_Walk_Struct_Add_Callback   struct_add;
   Eet_Node_Walk_Array_Callback        array;
   Eet_Node_Walk_Insert_Callback       insert;
   Eet_Node_Walk_List_Callback         list;
   Eet_Node_Walk_Append_Callback       append;
   Eet_Node_Walk_Hash_Callback         hash;
   Eet_Node_Walk_Simple_Callback       simple;
};

EAPI void *
eet_node_walk(void *parent,
              const char *name,
              Eet_Node *root,
              Eet_Node_Walk *cb,
              void *user_data);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eet_Connection_Group Helper function to use eet over a network link
 * @ingroup Eet_Group
 *
 * @remarks  This group provides function that reconstructs and prepares packet of @ref Eet_Data_Group to be send.
 *
 * @{
 */

/**
 * @typedef Eet_Connection
 * @brief   The structure type containing an opaque handle to track packet for a specific connection.
 */
typedef struct _Eet_Connection Eet_Connection;

/**
 * @typedef Eet_Read_Cb
 * @brief   Called when an @ref Eet_Data_Group has been received completely and could be used.
 */
typedef Eina_Bool Eet_Read_Cb (const void *eet_data, size_t size, void *user_data);

/**
 * @typedef Eet_Write_Cb
 * @brief   Called when a packet containing @ref Eet_Data_Group data is ready to be sent.
 */
typedef Eina_Bool Eet_Write_Cb (const void *data, size_t size, void *user_data);

/**
 * @brief   Instantiates a new connection to track.
 * @since   1.2.4
 *
 * @remarks For every connection to track, you need a separate Eet_Connection provider.
 *
 * @param[in]   eet_read_cb   The function to call when one Eet_Data packet has been fully assembled
 * @param[in]   eet_write_cb  The function to call when one Eet_Data packet is ready to be sent over the wire
 * @param[in]   user_data     The pointer provided to both functions to be used as a context handler
 * @return  A valid Eet_Connection handler, \n
 *          otherwise @c NULL on failure
 */
EAPI Eet_Connection *
eet_connection_new(Eet_Read_Cb *eet_read_cb,
                   Eet_Write_Cb *eet_write_cb,
                   const void *user_data);

/**
 * @brief   Processes a raw packet received over the link.
 * @since   1.2.4
 *
 * @remarks Every time you receive a packet related to your connection, you should pass
 *          it to that function so that it could process and assemble packet that you
 *          receive. It automatically calls Eet_Read_Cb when one is fully received.
 *
 * @param[in]   conn  The connection handler to track
 * @param[in]   data  The raw data packet
 * @param[in]   size  The size of that packet
 * @return  @c 0 on complete success, \n
 *          otherwise any other value which indicates the stream it got wrong \n
 *          It could be before that packet.
 */
EAPI int
eet_connection_received(Eet_Connection *conn,
                        const void *data,
                        size_t size);

/**
 * @brief   Checks whether the Eet_Connection has received some partial data.
 * @since   1.7
 *
 * @remarks Eet_Connection buffer data until the received data can be unserialized correctly. This
 *          function lets you know if there is some data inside that buffer waiting for more data to
 *          be received before being processed.
 * @param[in]   conn  The connection handler to request
 * @return  @c EINA_TRUE if there is some data pending inside, \n
 *          otherwise @c EINA_FALSE 
 */
EAPI Eina_Bool eet_connection_empty(Eet_Connection *conn);

/**
 * @brief   Converts a complex structure and prepares it to be send.
 *
 * @details This function serializes data_in with edd, assembles the packet and calls
 *          Eet_Write_Cb when ready. The data passed Eet_Write_Cb is temporary allocated
 *          and vanishes just after the return of the callback.
 * @since   1.2.4
 *
 * @param[in]   conn        The connection handler to track
 * @param[in]   edd         The data descriptor to use when encoding
 * @param[in]   data_in     The pointer to the struct to encode into data
 * @param[in]   cipher_key  The key to use as cipher
 * @return  @c EINA_TRUE if the complex structure is converted successfully, \n
 *          otherwise @c EINA_FALSE on failure
 *
 * @see eet_data_descriptor_encode_cipher
 */
EAPI Eina_Bool
eet_connection_send(Eet_Connection *conn,
                    Eet_Data_Descriptor *edd,
                    const void *data_in,
                    const char *cipher_key);

/**
 * @brief   Converts a Eet_Node tree and prepares it to be send.
 *
 * @details This function serializes node, assembles the packet and calls
 *          Eet_Write_Cb when ready. The data passed Eet_Write_Cb is temporary allocated
 *          and vanishes just after the return of the callback.
 * @since   1.2.4
 *
 * @param[in]   conn        The connection handler to track
 * @param[in]   node        The data tree to use when encoding
 * @param[in]   cipher_key  The key to use as cipher
 * @return  @c EINA_TRUE if the tree is converted successfully, \n
 *          otherwise @c EINA_FALSE on failure
 *
 * @see eet_data_node_encode_cipher
 */
EAPI Eina_Bool
eet_connection_node_send(Eet_Connection *conn,
                         Eet_Node *node,
                         const char *cipher_key);

/**
 * @brief   Closes a connection and loses its track.
 * @since   1.2.4
 *
 * @param[in]   conn      The connection handler to close
 * @param[in]   on_going  The signal if a partial packet is not completed
 * @return  The user_data passed to both callback
 */
EAPI void *
eet_connection_close(Eet_Connection *conn,
                     Eina_Bool *on_going);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef _EET_H */
