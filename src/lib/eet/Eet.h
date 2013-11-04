/**
   @brief Eet Data Handling Library Public API Calls

   These routines are used for Eet Library interaction

   @page eet_main Eet

   @date 2000 (created)

   @section toc Table of Contents

   @li @ref eet_main_intro
   @li @ref eet_main_compiling
   @li @ref eet_main_next_steps
   @li @ref eet_main_intro_example

   @section eet_main_intro Introduction

   It is a tiny library designed to write an arbitrary set of chunks of data
   to a file and optionally compress each chunk (very much like a zip file)
   and allow fast random-access reading of the file later on. It does not
   do zip as a zip itself has more complexity than is needed, and it was much
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

   @section eet_main_compiling How to compile

   Eet is a library your application links to. The procedure for this is very
   simple. You simply have to compile your application with the appropriate
   compiler flags that the @p pkg-config script outputs. For example:

   Compiling C or C++ files into object files:

   @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eet`
   @endverbatim

   Linking object files into a binary executable:

   @verbatim
   gcc -o my_application main.o `pkg-config --libs eet`
   @endverbatim

   See @ref pkgconfig

   @section eet_main_next_steps Next Steps

   After you understood what Eet is and installed it in your system
   you should proceed understanding the programming interface. We'd
   recommend you to take a while to learn @ref Eina as it is very
   convenient and optimized, and Eet provides integration with it.

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
#include <Efl_Config.h>

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
 * @file Eet.h
 * @brief The file that provides the eet functions.
 *
 * This header provides the Eet management functions.
 *
 */

#define EET_VERSION_MAJOR EFL_VERSION_MAJOR
#define EET_VERSION_MINOR EFL_VERSION_MINOR
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
   int major; /** < major (binary or source incompatible changes) */
   int minor; /** < minor (new features, bugfixes, major improvements version) */
   int micro; /** < micro (bugfix, internal improvements, no new features version) */
   int revision; /** < svn revision (0 if a proper release or the svn revision number Eet is built from) */
} Eet_Version;

EAPI extern Eet_Version *eet_version;

/**
 * @defgroup Eet_Group Top level functions
 * @ingroup Eet
 * Functions that affect Eet as a whole.
 *
 * @{
 */

/**
 * @enum _Eet_Error
 * All the error identifiers known by Eet.
 */
typedef enum _Eet_Error
{
   EET_ERROR_NONE, /**< No error, it's all fine! */
   EET_ERROR_BAD_OBJECT, /**< Given object or handle is NULL or invalid */
   EET_ERROR_EMPTY, /**< There was nothing to do */
   EET_ERROR_NOT_WRITABLE, /**< Could not write to file or file is #EET_FILE_MODE_READ */
   EET_ERROR_OUT_OF_MEMORY, /**< Could not allocate memory */
   EET_ERROR_WRITE_ERROR, /**< Failed to write data to destination */
   EET_ERROR_WRITE_ERROR_FILE_TOO_BIG, /**< Failed to write file since it is too big */
   EET_ERROR_WRITE_ERROR_IO_ERROR, /**< Failed to write due a generic Input/Output error */
   EET_ERROR_WRITE_ERROR_OUT_OF_SPACE, /**< Failed to write due out of space */
   EET_ERROR_WRITE_ERROR_FILE_CLOSED, /**< Failed to write because file was closed */
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
 * @defgroup Eet_Compression Eet Compression Levels
 * @ingroup Eet
 * Compression modes/levels supported by Eet.
 *
 * @{
 */

/**
 * @enum _Eet_Compression
 * All the compression modes known by Eet.
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
     
   EET_COMPRESSION_LOW2      = 3,  /**< Space filler for compatibility. Don't use it @since 1.7 */
   EET_COMPRESSION_MED1      = 4,  /**< Space filler for compatibility. Don't use it @since 1.7 */
   EET_COMPRESSION_MED2      = 5,  /**< Space filler for compatibility. Don't use it @since 1.7 */
   EET_COMPRESSION_HI1       = 7,  /**< Space filler for compatibility. Don't use it @since 1.7 */
   EET_COMPRESSION_HI2       = 8   /**< Space filler for compatibility. Don't use it @since 1.7 */
} Eet_Compression; /**< Eet compression modes @since 1.7 */
   
/**
 * @}
 */

/**
 * Initialize the EET library.
 *
 * The first time this function is called, it will perform all the internal
 * initialization required for the library to function properly and increment
 * the initialization counter. Any subsequent call only increment this counter
 * and return its new value, so it's safe to call this function more than once.
 *
 * @return The new init count. Will be 0 if initialization failed.
 *
 * @since 1.0.0
 * @ingroup Eet_Group
 */
EAPI int
eet_init(void);

/**
 * Shut down the EET library.
 *
 * If eet_init() was called more than once for the running application,
 * eet_shutdown() will decrement the initialization counter and return its
 * new value, without doing anything else. When the counter reaches 0, all
 * of the internal elements will be shutdown and any memory used freed.
 *
 * @return The new init count.
 *
 * @since 1.0.0
 * @ingroup Eet_Group
 */
EAPI int
eet_shutdown(void);

/**
 * Clear eet cache
 *
 * For a faster access to previously accessed data, Eet keeps an internal
 * cache of files. These files will be freed automatically only when
 * they are unused and the cache gets full, in order based on the last time
 * they were used.
 * On systems with little memory this may present an unnecessary constraint,
 * so eet_clearcache() is available for users to reclaim the memory used by
 * files that are no longer needed. Those that were open using
 * ::EET_FILE_MODE_WRITE or ::EET_FILE_MODE_READ_WRITE and have modifications,
 * will be written down to disk before flushing them from memory.
 *
 * @since 1.0.0
 * @ingroup Eet_Group
 */
EAPI void
eet_clearcache(void);

/**
 * @defgroup Eet_File_Group Eet File Main Functions
 * @ingroup Eet
 *
 * Functions to create, destroy and do basic manipulation of
 * #Eet_File handles.
 *
 * This sections explains how to use the most basic Eet functions, which
 * are used to work with eet files, read data from them, store it back in or
 * take a look at what entries it contains, without making use of the
 * serialization capabilities explained in @ref Eet_Data_Group.
 *
 * The following example will serve as an introduction to most, if not all,
 * of these functions.
 *
 * If you are only using Eet, this is the only header you need to include.
 * @dontinclude eet-file.c
 * @skipline Eet.h
 *
 * Now let's create ourselves an eet file to play with. The following function
 * shows step by step how to open a file and write some data in it.
 * First, we define our file handler and some other things we'll put in it.
 * @line static int
 * @skip Eet_File
 * @until ";
 * @skip eet_open
 *
 * We open a new file in write mode, and if it fails, we just return, since
 * there's not much more we can do about it..
 * @until return
 *
 * Now, we need to write some data in our file. For now, strings will suffice,
 * so let's just dump a bunch of them in there.
 * @until }
 *
 * As you can see, we copied a string into our static buffer, which is a bit
 * bigger than the full length of the string, and then told Eet to write it
 * into the file, compressed, returning the size of the data written into the
 * file.
 * This is all to show that Eet treats data as just data. It doesn't matter
 * what that data represents (for now), it's all just bytes for it. As running
 * the following code will show, we took a string of around 30 bytes and put it
 * in a buffer of 1024 bytes, but the returned size won't be any of those.
 * @until printf
 *
 * Next, we copy into our buffer our set of strings, including their null
 * terminators and write them into the file. No error checking for the sake
 * of brevity. And a call to eet_sync() to make sure all out data is
 * properly written down to disk, even though we haven't yet closed the file.
 * @until eet_sync
 *
 * One more write, this time our large array of binary data and... well, I
 * couldn't come up with a valid use of the last set of strings we stored,
 * so let's take it out from the file with eet_delete().
 * @until eet_delete
 *
 * Finally, we close the file, saving any changes back to disk and return.
 * Notice how, if there's any error closing the file or saving its contents,
 * the return value from the function will be a false one, which later on
 * will make the program exit with an error code.
 * @until return
 *
 * Moving onto our main function, we will open the same file and read it back.
 * Trivial, but it'll show how we can do so in more than one way. We'll skip
 * the variable declarations, as they aren't very different from what we've
 * seen already.
 *
 * We start from the beginning by initializing Eet so things in general work.
 * Forgetting to do so will result in weird results or crashes when calling
 * any eet function, so if you experience something like that, the first thing
 * to look at is whether eet_init() is missing.
 * Then we call our @p create_eet_file function, described above, to make
 * sure we have something to work with. If the function fails it will return
 * 0 and we just exit, since nothing from here onwards will work anyway.
 * @skip eet_init
 * @until return
 *
 * Let's take a look now at what entries our file has. For this, we use
 * eet_list(), which will return a list of strings, each being the name of
 * one entry. Since we skipped before, it may be worth noting that @p list
 * is declared as a @p char **.
 * The @p num parameter will, of course, have the number of entries contained
 * in our file.
 * If everything's fine, we'll get our list and print it to the screen, and
 * once done with it, we free the list. That's just the list, not its contents,
 * as they are internal strings used by Eet and trying to free them will surely
 * break things.
 * @until }
 *
 * Reading back plain data is simple. Just a call to eet_read() with the file
 * to read from, and the name of the entry we are interested in. We get back
 * our data and the passed @p size parameter will contain the size of it. If
 * the data was stored compressed, it will decompressed first.
 * @until }
 *
 * Another simple read for the set of strings from before, except those were
 * deleted, so we should get a NULL return and continue normally.
 * @until }
 *
 * Finally, we'll get our binary data in the same way we got the strings. Once
 * again, it makes no difference for Eet what the data is, it's up to us to
 * know how to handle it.
 * @until {
 *
 * Now some cheating, we know that this data is an Eet file because, well...
 * we just know it. So we are going to open it and take a look at its insides.
 * For this, eet_open() won't work, as it needs to have a file on disk to read
 * from and all we have is some data in RAM.
 *
 * So how do we do? One way would be to create a normal file and write down
 * our data, then open it with eet_open(). Another, faster and more efficient
 * if all we want to do is read the file, is to use eet_memopen_read().
 * @until memopen
 *
 * As you can see, the size we got from our previous read was put to good use
 * this time. Unlike the first one where all we had were strings, the size
 * of the data read only serves to demonstrate that we are reading back the
 * entire size of our original @p buf variable.
 *
 * A little peeking to see how many entries the file has and to make an
 * example of eet_num_entries() to get that number when we don't care about
 * their names.
 * @until printf
 *
 * More cheating follows. Just like we knew this was an Eet file, we also know
 * what key to read from, and ontop of that we know that the data in it is not
 * compressed.
 * Knowing all this allows us to take some shortcuts.
 * @until read_direct
 *
 * That's a direct print of our data, whatever that data is. We don't want
 * to worry about having to free it later, so we just used eet_direct_read()
 * to tell Eet to gives a pointer to the internal data in the file, without
 * duplicating it. Since we said that data was not compressed, we shouldn't
 * worry about printing garbage to the screen (and yes, we also know the data
 * is yet another string).
 * We also don't care about the size of the data as it was stored in the file,
 * so we passed NULL as the size parameter.
 * One very important note about this, however, is that we don't care about
 * the size parameter because the data in the file contains the null
 * terminator for the string. So when using Eet to store strings this way,
 * it's very important to consider whether you will keep that final null
 * byte, or to always get the size read and do the necessary checks and copies.
 * It's up to the user and the particular use cases to decide how this will
 * be done.
 *
 * With everything done, close this second file and free the data used to open
 * it. And this is important, we can't free that data until we are done with
 * the file, as Eet is using it. When opening with eet_memopen_read(), the data
 * passed to it must be available for as long as the the file is open.
 * @until }
 *
 * Finally, we close the first file, shutdown all internal resources used by
 * Eet and leave our main function, thus terminating our program.
 * @until return
 *
 * You can look at the full code of the example @ref eet-file.c "here".
 * @{
 */

/**
 * @enum _Eet_File_Mode
 * Modes that a file can be opened.
 */
typedef enum _Eet_File_Mode
{
   EET_FILE_MODE_INVALID = -1,
   EET_FILE_MODE_READ, /**< File is read-only. */
   EET_FILE_MODE_WRITE, /**< File is write-only. */
   EET_FILE_MODE_READ_WRITE /**< File is for both read and write */
} Eet_File_Mode; /**< Modes that a file can be opened. */

/**
 * @typedef Eet_File
 * Opaque handle that defines an Eet file (or memory).
 *
 * This handle will be returned by the functions eet_open() and
 * eet_memopen_read() and is used by every other function that affects the
 * file in any way. When you are done with it, call eet_close() to close it
 * and, if the file was open for writing, write down to disk any changes made
 * to it.
 *
 * @see eet_open()
 * @see eet_memopen_read()
 * @see eet_close()
 */
typedef struct _Eet_File Eet_File;

/**
 * @typedef Eet_Dictionary
 * Opaque handle that defines a file-backed (mmaped) dictionary of strings.
 */
typedef struct _Eet_Dictionary Eet_Dictionary;

/**
 * @typedef Eet_Entries
 * Eet files may contains multiple Entries per file, this handle describe them. You can get that handle from an iterator given by eet_list_entries().
 *
 * @see eet_list_entries()
 * @since 1.8.0
 */
typedef struct _Eet_Entry Eet_Entry;
struct _Eet_Entry
{
   const char *name; /**< The entry name */

   int offset;       /**< Where it start in the file  */
   int size;         /**< The size on disk */
   int data_size;    /**< The decompressed size if relevant */

   Eina_Bool compression; /**< Is this data compressed ? */
   Eina_Bool ciphered;    /**< Is it ciphered ? */
   Eina_Bool alias;       /**< Is it an alias ? */
};

/**
 * @}
 */

/**
 * Open an eet file on disk, and returns a handle to it.
 * @param file The file path to the eet file. eg: @c "/tmp/file.eet".
 * @param mode The mode for opening. Either #EET_FILE_MODE_READ,
 *        #EET_FILE_MODE_WRITE or #EET_FILE_MODE_READ_WRITE.
 * @return An opened eet file handle.
 * @ingroup Eet_File_Group
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
 * If the same file is opened multiple times, then the same file handle will
 * be returned as eet maintains an internal list of all currently open
 * files. Note that it considers files opened for read only and those opened
 * for read/write and write only as 2 separate sets. Those that do not write
 * to the file and those that do. Eet will allow 2 handles to the same file
 * if they are in the 2 separate lists/groups. That means opening a file for
 * read only looks in the read only set, and returns a handle to that file
 * handle and increments its reference count. If you open a file for read/write
 * or write only it looks in the write set and returns a handle after
 * incrementing the reference count. You need to close an eet file handle
 * as many times as it has been opened to maintain correct reference counts.
 * Files whose modified timestamp or size do not match those of the existing
 * referenced file handles will not be returned and a new handle will be
 * returned instead.
 *
 * @since 1.0.0
 */
EAPI Eet_File *
eet_open(const char *file,
         Eet_File_Mode mode);

/**
 * Open an eet file on disk from an Eina_File handle, and returns a handle to it.
 * @param file The Eina_File handle to map to an eet file.
 * @return An opened eet file handle.
 * @ingroup Eet_File_Group
 *
 * This function will open an exiting eet file for reading, and build
 * the directory table in memory and return a handle to the file, if it
 * exists and can be read, and no memory errors occur on the way, otherwise
 * NULL will be returned.
 *
 * This function can't open file for writing only read only mode is supported for now.
 *
 * If the same file is opened multiple times, then the same file handle will
 * be returned as eet maintains an internal list of all currently open
 * files. That means opening a file for read only looks in the read only set,
 * and returns a handle to that file handle and increments its reference count.
 * You need to close an eet file handle as many times as it has been opened to
 * maintain correct reference counts.
 *
 * @since 1.8.0
 */
EAPI Eet_File *
eet_mmap(const Eina_File *file);

/**
 * Open an eet file directly from a memory location. The data is not copied,
 * so you must keep it around as long as the eet file is open. There is
 * currently no cache for this kind of Eet_File, so it's reopened every time
 * you use eet_memopen_read.
 * @param data Address of file in memory.
 * @param size Size of memory to be read.
 * @return A handle to the file.
 *
 * Files opened this way will always be in read-only mode.
 *
 * @since 1.1.0
 * @ingroup Eet_File_Group
 */
EAPI Eet_File *
eet_memopen_read(const void *data,
                 size_t size);

/**
 * Get the mode an Eet_File was opened with.
 * @param ef A valid eet file handle.
 * @return The mode ef was opened with.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI Eet_File_Mode
eet_mode_get(Eet_File *ef);

/**
 * Close an eet file handle and flush pending writes.
 * @param ef A valid eet file handle.
 * @return An eet error identifier.
 *
 * This function will flush any pending writes to disk if the eet file
 * was opened for write, and free all data associated with the file handle
 * and file, and close the file. If it was opened for read (or read/write),
 * the file handle may still be held open internally for caching purposes.
 * To flush speculatively held eet file handles use eet_clearcache().
 *
 * If the eet file handle is not valid nothing will be done.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 * 
 * @see eet_clearcache()
 */
EAPI Eet_Error
eet_close(Eet_File *ef);

/**
 * Sync content of an eet file handle, flushing pending writes.
 * @param ef A valid eet file handle.
 * @return An eet error identifier.
 *
 * This function will flush any pending writes to disk. The eet file must
 * be opened for write.
 *
 * If the eet file handle is not valid nothing will be done.
 *
 * @since 1.2.4
 * @ingroup Eet_File_Group
 */
EAPI Eet_Error
eet_sync(Eet_File *ef);

/**
 * Return a handle to the shared string dictionary of the Eet file
 * @param ef A valid eet file handle.
 * @return A handle to the dictionary of the file
 *
 * This function returns a handle to the dictionary of an Eet file whose
 * handle is @p ef, if a dictionary exists. NULL is returned otherwise or
 * if the file handle is known to be invalid.
 *
 * @see eet_dictionary_string_check() to know if given string came
 *      from the dictionary or it was dynamically allocated using
 *      the #Eet_Data_Descriptor_Class instructions.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI Eet_Dictionary *
eet_dictionary_get(Eet_File *ef);

/**
 * Check if a given string comes from a given dictionary
 * @param ed A valid dictionary handle
 * @param string A valid 0 byte terminated C string
 * @return 1 if it is in the dictionary, 0 otherwise
 *
 * This checks the given dictionary to see if the given string is actually
 * inside that dictionary (i.e. comes from it) and returns 1 if it does.
 * If the dictionary handle is invalid, the string is NULL or the string is
 * not in the dictionary, 0 is returned.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI int
eet_dictionary_string_check(Eet_Dictionary *ed,
                            const char *string);

/**
 * Return the number of strings inside a dictionary
 * @param ed A valid dictionary handle
 * @return the number of strings inside a dictionary
 *
 * @since 1.6.0
 * @ingroup Eet_File_Group
 */
EAPI int
eet_dictionary_count(const Eet_Dictionary *ed);

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
 * @see eet_read_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI void *
eet_read(Eet_File *ef,
         const char *name,
         int *size_ret);

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
 * encountered or if the data is compressed. The calling program must never
 * call free() on the returned data. The number of bytes in the returned
 * data chunk are placed in size_ret.
 *
 * If the eet file handle is not valid NULL is returned and size_ret is
 * filled with 0.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI const void *
eet_read_direct(Eet_File *ef,
                const char *name,
                int *size_ret);

/**
 * Write a specified entry to an eet file handle
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param data Pointer to the data to be stored.
 * @param size Length in bytes in the data to be stored.
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @return bytes written on successful write, 0 on failure.
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
 * @see eet_write_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI int
eet_write(Eet_File *ef,
          const char *name,
          const void *data,
          int size,
          int compress);

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
 * @ingroup Eet_File_Group
 */
EAPI int
eet_delete(Eet_File *ef,
           const char *name);

/**
 * Alias a specific section to another one. Destination may exist or not,
 * no checks are done.
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the new entry. eg: "/base/file_i_want".
 * @param destination Actual source of the aliased entry eg: "/base/the_real_stuff_i_want".
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * Name and Destination must not be NULL, otherwise EINA_FALSE will be returned.
 * The equivalent of this would be calling 'ln -s destination name'
 *
 * @since 1.3.3
 * @ingroup Eet_File_Group
 */
EAPI Eina_Bool
eet_alias(Eet_File *ef,
          const char *name,
          const char *destination,
          int compress);

/**
 * Retrieve the filename of an Eet_File
 * @param ef A valid eet file handle opened for writing.
 * @return The stringshared file string opened with eet_open(), or NULL on error
 *
 * @note This function will return NULL for files opened with eet_memopen_read()
 *
 * @since 1.6
 * @ingroup Eet_File_Group
 */
EAPI const char *
eet_file_get(Eet_File *ef);

/**
 * Retrieve the destination name of an alias
 * @param ef A valid eet file handle opened for writing
 * @param name Name of the entry. eg: "/base/file_i_want"
 * @return Destination of the alias. eg: "/base/the_real_stuff_i_want", NULL on failure
 *
 * Name must not be NULL, otherwise NULL will be returned.
 *
 * @since 1.5
 * @ingroup Eet_File_Group
 */
EAPI const char *
eet_alias_get(Eet_File *ef,
              const char *name);

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
 * @ingroup Eet_File_Group
 */
EAPI char **
eet_list(Eet_File *ef,
         const char *glob,
         int *count_ret);

/**
 * Return an iterator that will describe each entry of an Eet_File.
 * @param ef A valid eet file handle.
 * @return An interator of Eet_Entry.
 *
 * @since 1.8.0
 * @ingroup Eet_File_Group
 */

EAPI Eina_Iterator *eet_list_entries(Eet_File *ef);

/**
 * Return the number of entries in the specified eet file.
 * @param ef A valid eet file handle.
 * @return Number of entries in ef or -1 if the number of entries
 *         cannot be read due to open mode restrictions.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI int
eet_num_entries(Eet_File *ef);

/**
 * @defgroup Eet_File_Cipher_Group Eet File Ciphered Main Functions
 *
 * Most of the @ref Eet_File_Group have alternative versions that
 * accounts for ciphers to protect their content.
 *
 * @see @ref Eet_Cipher_Group
 *
 * @ingroup Eet_File_Group
 */

/**
 * Read a specified entry from an eet file and return data using a cipher.
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param size_ret Number of bytes read from entry and returned.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_read()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Cipher_Group
 */
EAPI void *
eet_read_cipher(Eet_File *ef,
                const char *name,
                int *size_ret,
                const char *cipher_key);

/**
 * Write a specified entry to an eet file handle using a cipher.
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param data Pointer to the data to be stored.
 * @param size Length in bytes in the data to be stored.
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @param cipher_key The key to use as cipher.
 * @return bytes written on successful write, 0 on failure.
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
 * @see eet_write()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Cipher_Group
 */
EAPI int
eet_write_cipher(Eet_File *ef,
                 const char *name,
                 const void *data,
                 int size,
                 int compress,
                 const char *cipher_key);

/**
 * @defgroup Eet_File_Image_Group Image Store and Load
 * @ingroup Eet
 *
 * Eet efficiently stores and loads images, including alpha
 * channels and lossy compressions.
 *
 * Eet can handle both lossy compression with different levels of quality and
 * non-lossy compression with different compression levels. It's also possible,
 * given an image data, to only read its header to get the image information
 * without decoding the entire content for it.
 *
 * The encode family of functions will take an image raw buffer and its
 * parameters and compress it in memory, returning the new buffer.
 * Likewise, the decode functions will read from the given location in memory
 * and return the uncompressed image.
 *
 * The read and write functions will, respectively, encode and decode to or
 * from an Eet file, under the specified key.
 *
 * These functions are fairly low level and the same functionality can be
 * achieved using Evas and Edje, making it much easier to work with images
 * as well as not needing to worry about things like scaling them.
 */

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
 * @return 1 on successful decode, 0 otherwise
 *
 * Reads and decodes the image header data stored under the given key and
 * Eet file.
 *
 * The information decoded is placed in each of the parameters, which must be
 * provided. The width and height, measured in pixels, will be stored under
 * the variables pointed by @p w and @p h, respectively. If the read or
 * decode of the header fails, this values will be 0. The @p alpha parameter
 * will be 1 or 0, denoting if the alpha channel of the image is used or not.
 * If the image was losslessly compressed, the @p compress parameter will hold
 * the compression amount used, ranging from 0 to 9 and @p lossy will be 0.
 * In the case of lossy compression, @p lossy will be 1, and the compression
 * quality will be placed under @p quality, with a value ranging from 0 to 100.
 *
 * @see eet_data_image_header_decode()
 * @see eet_data_image_header_read_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_header_read(Eet_File *ef,
                           const char *name,
                           unsigned int *w,
                           unsigned int *h,
                           int *alpha,
                           int *compress,
                           int *quality,
                           int *lossy);

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
 * Reads and decodes the image stored in the given Eet file under the named
 * key.
 *
 * The returned pixel data is a linear array of pixels starting from the
 * top-left of the image, scanning row by row from left to right. Each pile
 * is a 32bit value, with the high byte being the alpha channel, the next being
 * red, then green, and the low byte being blue.
 *
 * The rest of the parameters are the same as in eet_data_image_header_read().
 *
 * On success the function returns a pointer to the image data decoded. The
 * calling application is responsible for calling free() on the image data
 * when it is done with it. On failure NULL is returned and the parameter
 * values may not contain any sensible data.
 *
 * @see eet_data_image_header_read()
 * @see eet_data_image_decode()
 * @see eet_data_image_read_cipher()
 * @see eet_data_image_read_to_surface()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI void *
eet_data_image_read(Eet_File *ef,
                    const char *name,
                    unsigned int *w,
                    unsigned int *h,
                    int *alpha,
                    int *compress,
                    int *quality,
                    int *lossy);

/**
 * Read image data from the named key in the eet file and store it in the given buffer.
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
 * Reads and decodes the image stored in the given Eet file, placing the
 * resulting pixel data in the buffer pointed by the user.
 *
 * Like eet_data_image_read(), it takes the image data stored under the
 * @p name key in the @p ef file, but instead of returning a new buffer with
 * the pixel data, it places the result in the buffer pointed by @p d, which
 * must be provided by the user and of sufficient size to hold the requested
 * portion of the image.
 *
 * The @p src_x and @p src_y parameters indicate the top-left corner of the
 * section of the image to decode. These have to be higher or equal than 0 and
 * less than the respective total width and height of the image. The width
 * and height of the section of the image to decode are given in @p w and @p h
 * and also can't be higher than the total width and height of the image.
 *
 * The @p row_stride parameter indicates the length in bytes of each line in
 * the destination buffer and it has to be at least @p w * 4.
 *
 * All the other parameters are the same as in eet_data_image_read().
 *
 * On success the function returns 1, and 0 on failure. On failure the
 * parameter values may not contain any sensible data.
 *
 * @see eet_data_image_read()
 * @see eet_data_image_decode()
 * @see eet_data_image_decode_to_surface()
 * @see eet_data_image_read_to_surface_cipher()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Group
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
                               int *lossy);

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
 * @see eet_data_image_read()
 * @see eet_data_image_encode()
 * @see eet_data_image_write_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
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
                     int lossy);

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
 * This function works exactly like eet_data_image_header_read(), but instead
 * of reading from an Eet file, it takes the buffer of size @p size pointed
 * by @p data, which must be a valid Eet encoded image.
 *
 * On success the function returns 1 indicating the header was read and
 * decoded properly, or 0 on failure.
 *
 * @see eet_data_image_header_read()
 * @see eet_data_image_header_decode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_header_decode(const void *data,
                             int size,
                             unsigned int *w,
                             unsigned int *h,
                             int *alpha,
                             int *compress,
                             int *quality,
                             int *lossy);

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
 * It works exactly like eet_data_image_read(), but it takes the encoded
 * data in the @p data buffer of size @p size, instead of reading from a file.
 * All the others parameters are also the same.
 *
 * On success the function returns a pointer to the image data decoded. The
 * calling application is responsible for calling free() on the image data
 * when it is done with it. On failure NULL is returned and the parameter
 * values may not contain any sensible data.
 *
 * @see eet_data_image_read()
 * @see eet_data_image_decode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI void *
eet_data_image_decode(const void *data,
                      int size,
                      unsigned int *w,
                      unsigned int *h,
                      int *alpha,
                      int *compress,
                      int *quality,
                      int *lossy);

/**
 * Decode Image data into pixel data and stores in the given buffer.
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
 * Like eet_data_image_read_to_surface(), but reading the given @p data buffer
 * instead of a file.
 *
 * On success the function returns 1, and 0 on failure. On failure the
 * parameter values may not contain any sensible data.
 *
 * @see eet_data_image_read_to_surface()
 * @see eet_data_image_decode_to_surface_cipher()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Group
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
                                 int *lossy);

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
 * It works like eet_data_image_write(), but instead of writing the encoded
 * image into an Eet file, it allocates a new buffer of the size required and
 * returns the encoded data in it.
 *
 * On success this function returns a pointer to the encoded data that you
 * can free with free() when no longer needed.
 *
 * @see eet_data_image_write()
 * @see eet_data_image_read()
 * @see eet_data_image_encode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI void *
eet_data_image_encode(const void *data,
                      int *size_ret,
                      unsigned int w,
                      unsigned int h,
                      int alpha,
                      int compress,
                      int quality,
                      int lossy);

/**
 * @defgroup Eet_File_Image_Cipher_Group Image Store and Load using a Cipher
 *
 * Most of the @ref Eet_File_Image_Group have alternative versions
 * that accounts for ciphers to protect their content.
 *
 * @see @ref Eet_Cipher_Group
 *
 * @ingroup Eet_File_Image_Group
 */

/**
 * Read just the header data for an image and dont decode the pixels using a cipher.
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
 * @param w A pointer to the unsigned int to hold the width in pixels.
 * @param h A pointer to the unsigned int to hold the height in pixels.
 * @param alpha A pointer to the int to hold the alpha flag.
 * @param compress A pointer to the int to hold the compression amount.
 * @param quality A pointer to the int to hold the quality amount.
 * @param lossy A pointer to the int to hold the lossiness flag.
 * @return 1 on successful decode, 0 otherwise
 *
 * This function reads an image from an eet file stored under the named
 * key in the eet file and return a pointer to the decompressed pixel data.
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
 * @see eet_data_image_header_read()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
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
                                  int *lossy);

/**
 * Read image data from the named key in the eet file using a cipher.
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_read()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
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
                           int *lossy);

/**
 * Read image data from the named key in the eet file using a cipher.
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_read_to_surface()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Cipher_Group
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
                                      int *lossy);

/**
 * Write image data to the named key in an eet file using a cipher.
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_write()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
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
                            int lossy);

/**
 * Decode Image data header only to get information using a cipher.
 * @param data The encoded pixel data.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_header_decode()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
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
                                    int *lossy);

/**
 * Decode Image data into pixel data using a cipher.
 * @param data The encoded pixel data.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_decode()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
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
                             int *lossy);

/**
 * Decode Image data into pixel data using a cipher.
 * @param data The encoded pixel data.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_decode_to_surface()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Cipher_Group
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
                                        int *lossy);

/**
 * Encode image data for storage or transmission using a cipher.
 * @param data A pointer to the image pixel data.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_image_encode()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
 */
EAPI void *
eet_data_image_encode_cipher(const void *data,
                             const char *cipher_key,
                             unsigned int w,
                             unsigned int h,
                             int alpha,
                             int compress,
                             int quality,
                             int lossy,
                             int *size_ret);

/**
 * @defgroup Eet_Cipher_Group Cipher, Identity and Protection Mechanisms
 * @ingroup Eet
 *
 * Eet allows one to protect entries of an #Eet_File
 * individually. This may be used to ensure data was not tampered or
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
 * @}
 */

/**
 * Callback used to request if needed the password of a private key.
 *
 * @param buffer the buffer where to store the password.
 * @param size the maximum password size (size of buffer, including '@\0').
 * @param rwflag if the buffer is also readable or just writable.
 * @param data currently unused, may contain some context in future.
 * @return 1 on success and password was set to @p buffer, 0 on failure.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
typedef int (*Eet_Key_Password_Callback)(char *buffer, int size, int rwflag, void *data);

/**
 * Create an Eet_Key needed for signing an eet file.
 *
 * The certificate should provide the public that match the private key.
 * No verification is done to ensure that.
 *
 * @param certificate_file The file where to find the certificate.
 * @param private_key_file The file that contains the private key.
 * @param cb Function to callback if password is required to unlock
 *        private key.
 * @return A key handle to use, or @c NULL on failure.
 *
 * @see eet_identity_close()
 *
 * @warning You need to compile signature support in EET.
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI Eet_Key *
eet_identity_open(const char *certificate_file,
                  const char *private_key_file,
                  Eet_Key_Password_Callback cb);

/**
 * Close and release all resource used by an Eet_Key.  An
 * reference counter prevent it from being freed until all file
 * using it are also closed.
 *
 * @param key the key handle to close and free resources.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI void
eet_identity_close(Eet_Key *key);

/**
 * Set a key to sign a file
 *
 * @param ef the file to set the identity.
 * @param key the key handle to set as identity.
 * @return #EET_ERROR_BAD_OBJECT if @p ef is invalid or
 *         #EET_ERROR_NONE on success.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI Eet_Error
eet_identity_set(Eet_File *ef,
                 Eet_Key *key);

/**
 * Display both private and public key of an Eet_Key.
 *
 * @param key the handle to print.
 * @param out where to print.
 *
 * @warning You need to compile signature support in EET.
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI void
eet_identity_print(Eet_Key *key,
                   FILE *out);

/**
 * Get the x509 der certificate associated with an Eet_File. Will return NULL
 * if the file is not signed.
 *
 * @param ef The file handle to query.
 * @param der_length The length of returned data, may be @c NULL.
 * @return the x509 certificate or @c NULL on error.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI const void *
eet_identity_x509(Eet_File *ef,
                  int *der_length);

/**
 * Get the raw signature associated with an Eet_File. Will return NULL
 * if the file is not signed.
 *
 * @param ef The file handle to query.
 * @param signature_length The length of returned data, may be @c NULL.
 * @return the raw signature or @c NULL on error.
 *
 * @ingroup Eet_Cipher_Group
 */
EAPI const void *
eet_identity_signature(Eet_File *ef,
                       int *signature_length);

/**
 * Get the SHA1 associated with a file. Could be the one used to
 * sign the data or if the data where not signed, it will be the
 * SHA1 of the file.
 *
 * @param ef The file handle to query.
 * @param sha1_length The length of returned data, may be @c NULL.
 * @return the associated SHA1 or @c NULL on error.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI const void *
eet_identity_sha1(Eet_File *ef,
                  int *sha1_length);

/**
 * Display the x509 der certificate to out.
 *
 * @param certificate the x509 certificate to print
 * @param der_length The length the certificate.
 * @param out where to print.
 *
 * @warning You need to compile signature support in EET.
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI void
eet_identity_certificate_print(const unsigned char *certificate,
                               int der_length,
                               FILE *out);

/**
 * @defgroup Eet_Data_Group Eet Data Serialization
 * @ingroup Eet
 *
 * Convenience functions to serialize and parse complex data
 * structures to binary blobs.
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
 * quite cumbersome, so we provide lots of macros and convenience
 * functions to aid creating the types.
 *
 * We make now a quick overview of some of the most commonly used elements
 * of this part of the library. A simple example of a configuration system
 * will work as a somewhat real life example that is still simple enough to
 * follow.
 * Only the relevant sections will be shown here, but you can get the full
 * code @ref eet-data-simple.c "here".
 *
 * Ignoring the included headers, we'll begin by defining our configuration
 * struct.
 * @dontinclude eet-data-simple.c
 * @skip typedef
 * @until }
 *
 * When using Eet, you don't think in matters of what data the program needs
 * to run and which you would like to store. It's all the same and if it makes
 * more sense to keep them together, it's perfectly fine to do so. At the time
 * of telling Eet how your data is comprised you can leave out the things
 * that are runtime only and let Eet take care of the rest for you.
 *
 * The key used to store the config follows, as well as the variable used to
 * store our data descriptor.
 * This last one is very important. It's the one thing that Eet will use to
 * identify your data, both at the time of writing it to the file and when
 * loading from it.
 * @skipline MY_CONF
 * @skipline Eet_Data_Descriptor
 *
 * Now we'll see how to create this descriptor, so Eet knows how to handle
 * our data later on.
 * Begin our function by declaring an Eet_Data_Descriptor_Class, which is
 * used to create the actual descriptor. This class contains the name of
 * our data type, its size and several functions that dictate how Eet should
 * handle memory to allocate the necessary bits to bring our data to life.
 * You, as a user, will very hardly set this class' contents directly. The
 * most common scenario is to use one of the provided macros that set it using
 * the Eina data types, so that's what we'll be doing across all our examples.
 * @skip static void
 * @until eet_data_descriptor_stream_new
 *
 * Now that we have our descriptor, we need to make it describe something.
 * We do so by telling it which members of our struct we want it to know about
 * and their types.
 * The eet_data_descriptor_element_add() function takes care of this, but it's
 * too cumbersome for normal use, so several macros are provided that make
 * it easier to handle. Even with them, however, code can get very repetitive
 * and it's not uncommon to define custom macros using them to save on typing.
 * @skip #define
 * @until }
 *
 * Now our descriptor knows about the parts of our structure that we are
 * interesting in saving. You can see that not all of them are there, yet Eet
 * will find those that need saving and do the right thing. When loading our
 * data, any non-described fields in the structure will be zeroed, so there's
 * no need to worry about garbage memory in them.
 * Refer to the documentation of #EET_DATA_DESCRIPTOR_ADD_BASIC to understand
 * what our macro does.
 *
 * We are done with our descriptor init function and it's proper to have the
 * relevant shutdown. Proper coding guidelines indiciate that all memory
 * allocated should be freed when the program ends, and since you will most
 * likely keep your descriptor around for the life or your application, it's
 * only right to free it at the end.
 * @skip static void
 * @until }
 *
 * Not listed here, but included in the full example are functions to create
 * a blank configuration and free it. The first one will only be used when
 * no file exists to load from, or nothing is found in it, but the latter is
 * used regardless of where our data comes from. Unless you are reading direct
 * data from the Eet file, you will be in charge of freeing anything loaded
 * from it.
 *
 * Now it's time to look at how we can load our config from some file.
 * Begin by opening the Eet file normally.
 * @skip static My_Conf_Type
 * @until }
 *
 * And now we need to read the data from the file and decode it using our
 * descriptor. Fortunately, that's all done in one single step.
 * @until goto
 *
 * And that's it for all Eet cares about. But since we are dealing with a
 * common case, as is save and load of user configurations, the next fragment
 * of code shows why we have a version field in our struct, and how you can
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
 * To close, our main function, which doesn't do much. Just take some arguments
 * from the command line with the name of the file to load and another one
 * where to save again. If input file doesn't exist, a new config structure
 * will be created and saved to our output file.
 * @skip int main
 * @until return ret
 * @until }
 *
 * The following is a list of more advanced and detailed examples.
 * @li @ref eet_data_nested_example
 * @li @ref eet_data_file_descriptor
 * @li @ref Example_Eet_Data_File_Descriptor_02
 * @li @ref Example_Eet_Data_Cipher_Decipher
 */

/**
 * @page eet_data_nested_example Nested structures and Eet Data Descriptors
 *
 * We've seen already a simple example of how to use Eet Data Descriptors
 * to handle our structures, but it didn't show how this works when you
 * have structures inside other structures.
 *
 * Now, there's a very simple case of this, for when you have inline structs
 * to keep your big structure more organized, you don't need anything else
 * besides what @ref eet-data-simple.c "this simple example does".
 * Just use something like @p some_struct.sub_struct.member when adding the
 * member to the descriptor and it will work.
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
 * But this is not what we are here for today. When we talk about nested
 * structures, what we really want are things like lists and hashes to be
 * taken into consideration automatically, and all their contents saved and
 * loaded just like ordinary integers and strings are.
 *
 * And of course, Eet can do that, and considering the work it saves you as a
 * programmer, we could say it's even easier to do than handling just integers.
 *
 * Let's begin with our example then, which is not all too different from the
 * simple one introduced earlier.
 *
 * We won't ignore the headers this time to show how easy it is to use Eina
 * data types with Eet, but we'll still skip most of the code that is not
 * pertinent to what we want to show now, but as usual, you can get it full
 * by following @ref eet-data-nested.c "this link".
 *
 * @dontinclude eet-data-nested.c
 * @skipline Eina.h
 * @skipline Eet.h
 * @skip typedef struct
 * @until } My_Conf_Subtype
 *
 * Extremely similar to our previous example. Just a new struct in there, and
 * a pointer to a list in the one we already had. Handling a list of subtypes
 * is easy on our program, but now we'll see what Eet needs to work with them
 * (Hint: it's easy too).
 * @skip _my_conf_descriptor
 * @until _my_conf_sub_descriptor
 *
 * Since we have two structures now, it's only natural that we'll need two
 * descriptors. One for each, which will be defined exactly as before.
 * @skip static void
 * @until eddc
 * @skip EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET
 * @until _my_conf_sub_descriptor
 *
 * We create our descriptors, each for one type, and as before, we are going to
 * use a simple macro to set their contents, to save on typing.
 * @skip #define
 * @until EET_T_UCHAR
 *
 * So far, nothing new. We have our descriptors and we know already how to
 * save them separately. But what we want is to link them together, and even
 * more so, we want our main type to hold a list of more than one of the new
 * sub type. So how do we do that?
 *
 * Simple enough, we tell Eet that our main descriptor will hold a list, of
 * which each node will point to some type described by our new descriptor.
 * @skip EET_DATA_DESCRIPTOR_ADD_LIST
 * @until _my_conf_sub_descriptor
 *
 * And that's all. We are closing the function now so as to not leave dangling
 * curly braces, but there's nothing more to show in this example. Only other
 * additions are the necessary code to free our new data, but you can see it
 * in the full code listing.
 * @until }
 */

/**
 * @page eet_data_file_descriptor Advanced use of Eet Data Descriptors
 *
 * A real life example is usually the best way to see how things are used,
 * but they also involve a lot more code than what needs to be shown, so
 * instead of going that way, we'll be borrowing some pieces from one in
 * the following example. It's been slightly modified from the original
 * source to show more of the varied ways in which Eet can handle our data.
 *
 * @ref eet-data-file_descriptor_01.c "This example" shows a cache of user
 * accounts and messages received, and it's a bit more interactive than
 * previous examples.
 *
 * Let's begin by looking at the structures we'll be using. First we have
 * one to define the messages the user receives and one for the one he posts.
 * Straight forward and nothing new here.
 * @dontinclude eet-data-file_descriptor_01.c
 * @skip typedef
 * @until My_Post
 *
 * One more to declare the account itself. This one will contain a list of
 * all messages received, and the posts we make ourselves will be kept in an
 * array. No special reason other than to show how to use arrays with Eet.
 * @until My_Account
 *
 * Finally, the main structure to hold our cache of accounts. We'll be looking
 * for these accounts by their names, so let's keep them in a hash, using
 * that name as the key.
 * @until My_Cache
 *
 * As explained before, we need one descriptor for each struct we want Eet
 * to handle, but this time we also want to keep around our Eet file and its
 * string dictionary. You will see why in a moment.
 * @skip Eet_Data_Descriptor
 * @until _my_post_descriptor
 * @skip Eet_File
 * @until Eet_Dictionary
 *
 * The differences begin now. They aren't much, but we'll be creating our
 * descriptors differently. Things can be added to our cache, but we won't
 * be modifying the current contents, so we can consider the data read from
 * it to be read-only, and thus allow Eet to save time and memory by not
 * duplicating thins unnecessary.
 * @skip static void
 * @until _my_post_descriptor
 *
 * As the comment in the code explains, we are asking Eet to give us strings
 * directly from the mapped file, which avoids having to load it in memory
 * and data duplication.
 * Of course, there are things to take into account when doing things this
 * way, and they will be mentioned as we encounter those special cases.
 *
 * Next comes the actual description of our data, just like we did in the
 * previous examples.
 * @skip #define
 * @until #undef
 * @until #define
 * @until #undef
 *
 * And the account struct's description doesn't add much new, but it's worth
 * commenting on it.
 * @skip #define
 * @until _my_post_descriptor
 *
 * How to add a list we've seen before, but now we are also adding an array.
 * There's nothing really special about it, but it's important to note that
 * the EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY is used to add arrays of variable
 * length to a descriptor. That is, arrays just like the one we defined.
 * Since there's no way in C to know how long they are, we need to keep
 * track of the count ourselves and Eet needs to know how to do so as well.
 * That's what the @p posts_count member of our struct is for. When adding
 * our array member, this macro will look for another variable in the struct
 * named just like the array, but with @p _count attached to the end.
 * When saving our data, Eet will know how many elements the array contains
 * by looking into this count variable. When loading back from a file, this
 * variable will be set to the right number of elements.
 *
 * Another option for arrays is to use EET_DATA_DESCRIPTOR_ADD_ARRAY, which
 * takes care of fixed sized arrays.
 * For example, let's suppose that we want to keep track of only the last
 * ten posts the user sent, and we declare our account struct as follows
 * @code
 * typedef struct
 * {
 *    unsigned int id;
 *    const char  *name;
 *    Eina_List   *messages;
 *    My_Post      posts[10];
 * } My_Account;
 * @endcode
 * Then we would add the array to our descriptor with
 * @code
 * EET_DATA_DESCRIPTOR_ADD_ARRAY(_my_account_descriptor, My_Account, "posts",
 *                               posts, _my_post_descriptor);
 * @endcode
 *
 * Notice how this time we don't have a @p posts_count variable in our struct.
 * We could have it for the program to keep track of how many posts the
 * array actually contains, but Eet no longer needs it. Being defined that
 * way the array is already taking up all the memory needed for the ten
 * elements, and it is possible in C to determine how much it is in code.
 * When saving our data, Eet will just dump the entire memory blob into the
 * file, regardless of how much of it is really used. So it's important to
 * take into consideration this kind of things when defining your data types.
 * Each has its uses, its advantages and disadvantages and it's up to you
 * to decide which to use.
 *
 * Now, going back to our example, we have to finish adding our data to the
 * descriptors. We are only missing the main one for the cache, which
 * contains our hash of accounts.
 * Unless you are using your own hash functions when setting the descriptor
 * class, always use hashes with string type keys.
 * @skip #define
 * @until }
 *
 * If you remember, we told Eet not to duplicate memory when possible at the
 * time of loading back our data. But this doesn't mean everything will be
 * loaded straight from disk and we don't have to worry about freeing it.
 * Data in the Eet file is compressed and encoded, so it still needs to be
 * decoded and memory will be allocated to convert it back into something we
 * can use. We also need to take care of anything we add in the current
 * instance of the program.
 * To summarize, any string we get from Eet is likely to be a pointer to the
 * internal dictionary, and trying to free it will, in the best case, crash
 * our application right away.
 *
 * So how do we know if we have to free a string? We check if it's part of
 * the dictionary, and if it's not there we can be sure it's safe to get
 * rid of it.
 * @skip static void
 * @skip }
 * @skip static void
 * @until }
 *
 * See how this is used when adding a new message to our cache.
 * @skip static My_Message
 * @until return msg
 * @until free(msg)
 * @until }
 *
 * Skipping all the utility functions used by our program (remember you can
 * look at the full example @ref eet-data-file_descriptor_01.c "here") we get to
 * our cache loading code. Nothing out of the ordinary at first, just the
 * same old open file, read data using our main descriptor to decode it
 * into something we can use and check version of loaded data and if it doesn't
 * match, do something accordingly.
 * @skip static My_Cache
 * @until }
 * @until }
 * @until }
 *
 * Then comes the interesting part. Remember how we kept two more global
 * variables with our descriptors? One of them we already used to check if
 * it was right to free a string or not, but we didn't know where it came from.
 * Loading our data straight from the mmapped file means that we can't close
 * it until we are done using it, so we need to keep its handler around until
 * then. It also means that any changes done to the file can, and will,
 * invalidate all our pointers to the file backed data, so if we add something
 * and save the file, we need to reload our cache.
 *
 * Thus our load function checks if we had an open file, if there is it gets
 * closed and our variable is updated to the new handler. Then we get the
 * string dictionary we use to check if a string is part of it or not.
 * Updating any references to the cache data is up you as a programmer to
 * handle properly, there's nothing Eet can do in this situation.
 * @until }
 *
 * The save function doesn't have anything new, and all that's left after it
 * is the main program, which doesn't really have anything of interest within
 * the scope of what we are learning.
 */

/**
 * @addtogroup Eet_Data_Group
 * @{
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
#define EET_T_VALUE          17 /**< Data type: pointer to Eina_Value @since 1.8 */
#define EET_T_LAST           18 /**< Last data type */

#define EET_G_UNKNOWN        100 /**< Unknown group data encoding type */
#define EET_G_ARRAY          101 /**< Fixed size array group type */
#define EET_G_VAR_ARRAY      102 /**< Variable size array group type */
#define EET_G_LIST           103 /**< Linked list group type */
#define EET_G_HASH           104 /**< Hash table group type */
#define EET_G_UNION          105 /**< Union group type */
#define EET_G_VARIANT        106 /**< Selectable subtype group */
#define EET_G_UNKNOWN_NESTED 107 /**< Unknown nested group type. @since 1.8 */
#define EET_G_LAST           108 /**< Last group type */

#define EET_I_LIMIT          128 /**< Other type exist but are reserved for internal purpose. */

/**
 * @typedef Eet_Data_Descriptor
 *
 * Opaque handle that have information on a type members.
 *
 * Descriptors are created using an #Eet_Data_Descriptor_Class, and they
 * describe the contents of the structure that will be serialized by Eet.
 * Not all members need be described by it, just those that should be handled
 * by Eet. This way it's possible to have one structure with both data to be
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
 * The version of #Eet_Data_Descriptor_Class at the time of the
 * distribution of the sources. One should define this to its
 * version member so it is compatible with abi changes, or at least
 * will not crash with them.
 */
#define EET_DATA_DESCRIPTOR_CLASS_VERSION 4

/**
 * @typedef Eet_Data_Descriptor_Class
 *
 * Instructs Eet about memory management for different needs under
 * serialization and parse process.
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
 * @struct _Eet_Data_Descriptor_Class
 *
 * Instructs Eet about memory management for different needs under
 * serialization and parse process.
 *
 * The list and hash methods match the Eina API, so for a more detailed
 * reference on them, look at the Eina_List and Eina_Hash documentation,
 * respectively.
 * For the most part these will be used with the standard Eina functions,
 * so using EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET() and
 * EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET() will set up everything
 * accordingly.
 */
struct _Eet_Data_Descriptor_Class
{
   int         version;  /**< ABI version. Should always be set to #EET_DATA_DESCRIPTOR_CLASS_VERSION */
   const char *name;  /**< Name of the user data type to be serialized */
   int         size;  /**< Size in bytes of the user data type to be serialized */
   struct
   {
      Eet_Descriptor_Mem_Alloc_Callback        mem_alloc; /**< how to allocate memory (usually malloc()) */
      Eet_Descriptor_Mem_Free_Callback         mem_free; /**< how to free memory (usually free()) */
      Eet_Descriptor_Str_Alloc_Callback        str_alloc; /**< how to allocate a string */
      Eet_Descriptor_Str_Free_Callback         str_free; /**< how to free a string */
      Eet_Descriptor_List_Next_Callback        list_next; /**< how to iterate to the next element of a list. Receives and should return the list node. */
      Eet_Descriptor_List_Append_Callback      list_append; /**< how to append data @p d to list which head node is @p l */
      Eet_Descriptor_List_Data_Callback        list_data; /**< retrieves the data from node @p l */
      Eet_Descriptor_List_Free_Callback        list_free; /**< free all the nodes from the list which head node is @p l */
      Eet_Descriptor_Hash_Foreach_Callback     hash_foreach; /**< iterates over all elements in the hash @p h in no specific order */
      Eet_Descriptor_Hash_Add_Callback         hash_add; /**< add a new data @p d with key @p k in hash @p h */
      Eet_Descriptor_Hash_Free_Callback        hash_free; /**< free all entries from the hash @p h */
      Eet_Descriptor_Str_Direct_Alloc_Callback str_direct_alloc; /**< how to allocate a string directly from file backed/mmaped region pointed by @p str */
      Eet_Descriptor_Str_Direct_Free_Callback  str_direct_free; /**< how to free a string returned by str_direct_alloc */
      Eet_Descriptor_Type_Get_Callback         type_get; /**< get the type, as used in the union or variant mapping, that should be used to store the given data into the eet file. */
      Eet_Descriptor_Type_Set_Callback         type_set; /**< called when loading a mapped type with the given @p type used to describe the type in the descriptor */
      Eet_Descriptor_Array_Alloc_Callback      array_alloc; /**< how to allocate memory for array (usually malloc()) */
      Eet_Descriptor_Array_Free_Callback       array_free; /**< how to free memory for array (usually free()) */
   } func;
};

/**
 * @}
 */

/**
 * Create a new empty data structure descriptor.
 * @param name The string name of this data structure (most be a
 *        global constant and never change).
 * @param size The size of the struct (in bytes).
 * @param func_list_next The function to get the next list node.
 * @param func_list_append The function to append a member to a list.
 * @param func_list_data The function to get the data from a list node.
 * @param func_list_free The function to free an entire linked list.
 * @param func_hash_foreach The function to iterate through all
 *        hash table entries.
 * @param func_hash_add The function to add a member to a hash table.
 * @param func_hash_free The function to free an entire hash table.
 * @return A new empty data descriptor.
 *
 * This function creates a new data descriptor and returns a handle to the
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
 * endian-independent serialised data chunks for transmission across a
 * a network or more.
 *
 * The function pointers to the list and hash table functions are only
 * needed if you use those data types, else you can pass NULL instead.
 *
 * @since 1.0.0
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
 * moving to this api from the old above. this will break things when the
 * move happens - but be warned
 */
EINA_DEPRECATED EAPI Eet_Data_Descriptor *
 eet_data_descriptor2_new(const Eet_Data_Descriptor_Class *eddc);
EINA_DEPRECATED EAPI Eet_Data_Descriptor *
 eet_data_descriptor3_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * This function creates a new data descriptor and returns a handle to the
 * new data descriptor. On creation it will be empty, containing no contents
 * describing anything other than the shell of the data structure.
 * @param eddc The class from where to create the data descriptor.
 * @return A handle to the new data descriptor.
 *
 * You add structure members to the data descriptor using the macros
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB() and
 * EET_DATA_DESCRIPTOR_ADD_LIST(), depending on what type of member you are
 * adding to the description.
 *
 * Once you have described all the members of a struct you want loaded or
 * saved, eet can load and save those members for you, encode them into
 * endian-independent serialised data chunks for transmission across a
 * network or more.
 *
 * This function specially ignores str_direct_alloc and str_direct_free. It
 * is useful when the eet_data you are reading doesn't have a dictionary,
 * like network stream or IPC. It also mean that all string will be allocated
 * and duplicated in memory.
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eet_Data_Descriptor *
eet_data_descriptor_stream_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * This function creates a new data descriptor and returns a handle to the
 * new data descriptor. On creation it will be empty, containing no contents
 * describing anything other than the shell of the data structure.
 * @param eddc The class from where to create the data descriptor.
 * @return A handle to the new data descriptor.
 *
 * You add structure members to the data descriptor using the macros
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB() and
 * EET_DATA_DESCRIPTOR_ADD_LIST(), depending on what type of member you are
 * adding to the description.
 *
 * Once you have described all the members of a struct you want loaded or
 * saved, eet can load and save those members for you, encode them into
 * endian-independent serialised data chunks for transmission across a
 * a network or more.
 *
 * This function uses str_direct_alloc and str_direct_free. It is
 * useful when the eet_data you are reading come from a file and
 * have a dictionary. This will reduce memory use and improve the
 * possibility for the OS to page this string out.
 * However, the load speed and memory saving comes with some drawbacks to keep
 * in mind. If you never modify the contents of the structures loaded from
 * the file, all you need to remember is that closing the eet file will make
 * the strings go away. On the other hand, should you need to free a string,
 * before doing so you have to verify that it's not part of the eet dictionary.
 * You can do this in the following way, assuming @p ef is a valid Eet_File
 * and @p str is a string loaded from said file.
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
 *    // We assume eina_stringshare was used on the descriptor
 *    eina_stringshare_del(str);
 * }
 * @endcode
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eet_Data_Descriptor *
eet_data_descriptor_file_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * This function is an helper that set all the parameters of an
 * Eet_Data_Descriptor_Class correctly when you use Eina data type
 * with a stream.
 * @param eddc The Eet_Data_Descriptor_Class you want to set.
 * @param eddc_size The size of the Eet_Data_Descriptor_Class at the compilation time.
 * @param name The name of the structure described by this class.
 * @param size The size of the structure described by this class.
 * @return EINA_TRUE if the structure was correctly set (The only
 *         reason that could make it fail is if you did give wrong
 *         parameter).
 *
 * @note Unless there's a very specific reason to use this function directly,
 * the EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET macro is recommended.
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eina_Bool
eet_eina_stream_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
                                          unsigned int eddc_size,
                                          const char *name,
                                          int size);

/**
 * This macro is an helper that set all the parameter of an
 * Eet_Data_Descriptor_Class correctly when you use Eina data type
 * with stream.
 * @param clas The Eet_Data_Descriptor_Class you want to set.
 * @param type The type of the structure described by this class.
 * @return EINA_TRUE if the structure was correctly set (The only
 *         reason that could make it fail is if you did give wrong
 *         parameter).
 *
 * @see eet_data_descriptor_stream_new
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
#define EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(clas, type) \
  (eet_eina_stream_data_descriptor_class_set(clas, sizeof (*(clas)), # type, sizeof(type)))

/**
 * This function is an helper that set all the parameter of an
 * Eet_Data_Descriptor_Class correctly when you use Eina data type
 * with a file.
 * @param eddc The Eet_Data_Descriptor_Class you want to set.
 * @param eddc_size The size of the Eet_Data_Descriptor_Class at the compilation time.
 * @param name The name of the structure described by this class.
 * @param size The size of the structure described by this class.
 * @return EINA_TRUE if the structure was correctly set (The only
 *         reason that could make it fail is if you did give wrong
 *         parameter).
 *
 * @note Unless there's a very specific reason to use this function directly,
 * the EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET macro is recommended.
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eina_Bool
eet_eina_file_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
                                        unsigned int eddc_size,
                                        const char *name,
                                        int size);

/**
 * This macro is an helper that set all the parameter of an
 * Eet_Data_Descriptor_Class correctly when you use Eina data type
 * with file.
 * @param clas The Eet_Data_Descriptor_Class you want to set.
 * @param type The type of the structure described by this class.
 * @return EINA_TRUE if the structure was correctly set (The only
 *         reason that could make it fail is if you did give wrong
 *         parameter).
 *
 * @see eet_data_descriptor_file_new
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
#define EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(clas, type) \
  (eet_eina_file_data_descriptor_class_set(clas, sizeof (*(clas)), # type, sizeof(type)))

/**
 * This function frees a data descriptor when it is not needed anymore.
 * @param edd The data descriptor to free.
 *
 * This function takes a data descriptor handle as a parameter and frees all
 * data allocated for the data descriptor and the handle itself. After this
 * call the descriptor is no longer valid.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI void
eet_data_descriptor_free(Eet_Data_Descriptor *edd);

/**
 * This function returns the name of a data descriptor.
 *
 * @since 1.8.0
 * @ingroup Eet_Data_Group
 */
EAPI const char *eet_data_descriptor_name_get(const Eet_Data_Descriptor *edd);

/**
 * This function is an internal used by macros.
 *
 * This function is used by macros EET_DATA_DESCRIPTOR_ADD_BASIC(),
 * EET_DATA_DESCRIPTOR_ADD_SUB() and EET_DATA_DESCRIPTOR_ADD_LIST(). It is
 * complex to use by hand and should be left to be used by the macros, and
 * thus is not documented.
 *
 * @param edd The data descriptor handle to add element (member).
 * @param name The name of element to be serialized.
 * @param type The type of element to be serialized, like
 *        #EET_T_INT. If #EET_T_UNKNOW, then it is considered to be a
 *        group, list or hash.
 * @param group_type If element type is #EET_T_UNKNOW, then the @p
 *        group_type will specify if it is a list (#EET_G_LIST),
 *        array (#EET_G_ARRAY) and so on. If #EET_G_UNKNOWN, then
 *        the member is a subtype (pointer to another type defined by
 *        another #Eet_Data_Descriptor).
 * @param offset byte offset inside the source memory to be serialized.
 * @param count number of elements (if #EET_G_ARRAY or #EET_G_VAR_ARRAY).
 * @param counter_name variable that defines the name of number of elements.
 * @param subtype If contains a subtype, then its data descriptor.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
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
 * @see eet_data_read_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI void *
eet_data_read(Eet_File *ef,
              Eet_Data_Descriptor *edd,
              const char *name);

/**
 * Write a data structure from memory and store in an eet file.
 * @param ef The eet file handle to write to.
 * @param edd The data descriptor to use when encoding.
 * @param name The key to store the data under in the eet file.
 * @param data A pointer to the data structure to save and encode.
 * @param compress Compression flags for storage.
 * @return bytes written on successful write, 0 on failure.
 *
 * This function is the reverse of eet_data_read(), saving a data structure
 * to an eet file. The file must have been opening in write mode and the data
 * will be kept in memory until the file is either closed or eet_sync() is
 * called to flush any unwritten changes.
 *
 * @see eet_data_write_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI int
eet_data_write(Eet_File *ef,
               Eet_Data_Descriptor *edd,
               const char *name,
               const void *data,
               int compress);

typedef void (*Eet_Dump_Callback)(void *data, const char *str);

/**
 * Dump an eet encoded data structure into ascii text
 * @param data_in The pointer to the data to decode into a struct.
 * @param size_in The size of the data pointed to in bytes.
 * @param dumpfunc The function to call passed a string when new
 *        data is converted to text
 * @param dumpdata The data to pass to the @p dumpfunc callback.
 * @return 1 on success, 0 on failure
 *
 * This function will take a chunk of data encoded by
 * eet_data_descriptor_encode() and convert it into human readable
 * ascii text.  It does this by calling the @p dumpfunc callback
 * for all new text that is generated. This callback should append
 * to any existing text buffer and will be passed the pointer @p
 * dumpdata as a parameter as well as a string with new text to be
 * appended.
 *
 * Example:
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
 * @see eet_data_text_dump_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI int
eet_data_text_dump(const void *data_in,
                   int size_in,
                   Eet_Dump_Callback dumpfunc,
                   void *dumpdata);

/**
 * Take an ascii encoding from eet_data_text_dump() and re-encode in binary.
 * @param text The pointer to the string data to parse and encode.
 * @param textlen The size of the string in bytes (not including 0
 *        byte terminator).
 * @param size_ret This gets filled in with the encoded data blob
 *        size in bytes.
 * @return The encoded data on success, NULL on failure.
 *
 * This function will parse the string pointed to by @p text and return
 * an encoded data lump the same way eet_data_descriptor_encode() takes an
 * in-memory data struct and encodes into a binary blob. @p text is a normal
 * C string.
 *
 * @see eet_data_text_undump_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI void *
eet_data_text_undump(const char *text,
                     int textlen,
                     int *size_ret);

/**
 * Dump an eet encoded data structure from an eet file into ascii text
 * @param ef A valid eet file handle.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param dumpfunc The function to call passed a string when new
 *        data is converted to text
 * @param dumpdata The data to pass to the @p dumpfunc callback.
 * @return 1 on success, 0 on failure
 *
 * This function will take an open and valid eet file from
 * eet_open() request the data encoded by
 * eet_data_descriptor_encode() corresponding to the key @p name
 * and convert it into human readable ascii text. It does this by
 * calling the @p dumpfunc callback for all new text that is
 * generated. This callback should append to any existing text
 * buffer and will be passed the pointer @p dumpdata as a parameter
 * as well as a string with new text to be appended.
 *
 * @see eet_data_dump_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI int
eet_data_dump(Eet_File *ef,
              const char *name,
              Eet_Dump_Callback dumpfunc,
              void *dumpdata);

/**
 * Take an ascii encoding from eet_data_dump() and re-encode in binary.
 * @param ef A valid eet file handle.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param text The pointer to the string data to parse and encode.
 * @param textlen The size of the string in bytes (not including 0
 *        byte terminator).
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @return 1 on success, 0 on failure
 *
 * This function will parse the string pointed to by @p text,
 * encode it the same way eet_data_descriptor_encode() takes an
 * in-memory data struct and encodes into a binary blob.
 *
 * The data (optionally compressed) will be in ram, pending a flush to
 * disk (it will stay in ram till the eet file handle is closed though).
 *
 * @see eet_data_undump_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI int
eet_data_undump(Eet_File *ef,
                const char *name,
                const char *text,
                int textlen,
                int compress);

/**
 * Decode a data structure from an arbitrary location in memory.
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
 * @see eet_data_descriptor_decode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI void *
eet_data_descriptor_decode(Eet_Data_Descriptor *edd,
                           const void *data_in,
                           int size_in);

/**
 * Encode a dsata struct to memory and return that encoded data.
 * @param edd The data  descriptor to use when encoding.
 * @param data_in The pointer to the struct to encode into data.
 * @param size_ret pointer to the an int to be filled with the decoded size.
 * @return NULL on failure, or a valid encoded data chunk on success.
 *
 * This function takes a data structutre in memory and encodes it into a
 * serialised chunk of data that can be decoded again by
 * eet_data_descriptor_decode(). This is useful for being able to transmit
 * data structures across sockets, pipes, IPC or shared file mechanisms,
 * without having to worry about memory space, machine type, endianness etc.
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
 * @see eet_data_descriptor_encode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI void *
eet_data_descriptor_encode(Eet_Data_Descriptor *edd,
                           const void *data_in,
                           int *size_ret);

/**
 * Add a basic data element to a data descriptor.
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param type The type of the member to encode.
 *
 * This macro is a convenience macro provided to add a member to
 * the data descriptor @p edd. The type of the structure is
 * provided as the @p struct_type parameter (for example: struct
 * my_struct). The @p name parameter defines a string that will be
 * used to uniquely name that member of the struct (it is suggested
 * to use the struct member itself).  The @p member parameter is
 * the actual struct member itself (for example: values), and @p type is the
 * basic data type of the member which must be one of: EET_T_CHAR, EET_T_SHORT,
 * EET_T_INT, EET_T_LONG_LONG, EET_T_FLOAT, EET_T_DOUBLE, EET_T_UCHAR,
 * EET_T_USHORT, EET_T_UINT, EET_T_ULONG_LONG or EET_T_STRING.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
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
 * Add a sub-element type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
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
 * @ingroup Eet_Data_Group
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
 * Add a nested sub-element type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param subtype The type of sub-type struct to add.
 *
 * This macro lets you easily add a sub-type: a struct that is nested into
 * this one. If your data is pointed by this element instead of being nested,
 * you should use EET_DATA_DESCRIPTOR_ADD_SUB().
 * All the parameters are the same as for EET_DATA_DESCRIPTOR_ADD_SUB().
 *
 * @since 1.8.0
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(edd, struct_type, name, member, subtype)   \
  do {                                                                         \
       struct_type ___ett;                                                     \
       eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNKNOWN_NESTED, \
                                       (char *)(& (___ett.member)) -           \
                                       (char *)(& (___ett)),                   \
                                       0, /* 0,  */ NULL, subtype);            \
    } while (0)

/**
 * Add a linked list type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param subtype The type of linked list member to add.
 *
 * This macro lets you easily add a linked list of other data types. All the
 * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
 * @p subtype being the exception. This must be the data descriptor of the
 * element that is in each member of the linked list to be stored.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
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
 * Add a linked list of string to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 *
 * This macro lets you easily add a linked list of char *. All the
 * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC().
 *
 * @since 1.5.0
 * @ingroup Eet_Data_Group
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
 * Add a hash type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param subtype The type of hash member to add.
 *
 * This macro lets you easily add a hash of other data types. All the
 * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC(), with the
 * @p subtype being the exception. This must be the data descriptor of the
 * element that is in each member of the hash to be stored.
 * The hash keys must be strings.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
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
 * Add a hash of string to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 *
 * This macro lets you easily add a hash of string elements. All the
 * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_HASH().
 *
 * @since 1.3.4
 * @ingroup Eet_Data_Group
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
 * Add an array of basic data elements to a data descriptor.
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param type The type of the member to encode.
 *
 * This macro lets you easily add a fixed size array of basic data
 * types. All the parameters are the same as for
 * EET_DATA_DESCRIPTOR_ADD_BASIC().
 * The array must be defined with a fixed size in the declaration of the
 * struct containing it.
 *
 * @since 1.5.0
 * @ingroup Eet_Data_Group
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
 * Add a variable array of basic data elements to a data descriptor.
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param type The type of the member to encode.
 *
 * This macro lets you easily add a variable size array of basic data
 * types. All the parameters are the same as for
 * EET_DATA_DESCRIPTOR_ADD_BASIC(). This assumes you have
 * a struct member (of type EET_T_INT) called member_count (note the
 * _count appended to the member) that holds the number of items in
 * the array. This array will be allocated separately to the struct it
 * is in.
 *
 * @since 1.6.0
 * @ingroup Eet_Data_Group
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
 * Add a fixed size array type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param subtype The type of hash member to add.
 *
 * This macro lets you easily add a fixed size array of other data
 * types. All the parameters are the same as for
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @p subtype being the
 * exception. This must be the data descriptor of the element that
 * is in each member of the array to be stored.
 * The array must be defined with a fixed size in the declaration of the
 * struct containing it.
 *
 * @since 1.0.2
 * @ingroup Eet_Data_Group
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
 * Add a variable size array type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param subtype The type of hash member to add.
 *
 * This macro lets you easily add a variable size array of other data
 * types. All the parameters are the same as for
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @p subtype being the
 * exception. This must be the data descriptor of the element that
 * is in each member of the array to be stored. This assumes you have
 * a struct member (of type EET_T_INT) called member_count (note the
 * _count appended to the member) that holds the number of items in
 * the array. This array will be allocated separately to the struct it
 * is in.
 *
 * @since 1.0.2
 * @ingroup Eet_Data_Group
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
 * Add a variable size array type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 *
 * This macro lets you easily add a variable size array of strings. All
 * the parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC().
 *
 * @since 1.4.0
 * @ingroup Eet_Data_Group
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
 * Add an union type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param type_member The member that give hints on what is in the union.
 * @param unified_type Describe all possible type the union could handle.
 *
 * This macro lets you easily add an union with a member that specify what is inside.
 * The @p unified_type is an Eet_Data_Descriptor, but only the entry that match the name
 * returned by type_get will be used for each serialized data. The type_get and type_set
 * callback of unified_type should be defined.
 *
 * @since 1.2.4
 * @ingroup Eet_Data_Group
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
 * Add a automatically selectable type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 * @param type_member The member that give hints on what is in the union.
 * @param unified_type Describe all possible type the union could handle.
 *
 * This macro lets you easily define what the content of @p member points to depending of
 * the content of @p type_member. The type_get and type_set callback of unified_type should
 * be defined. If the the type is not know at the time of restoring it, eet will still call
 * type_set of @p unified_type but the pointer will be set to a serialized binary representation
 * of what eet know. This make it possible, to save this pointer again by just returning the string
 * given previously and telling it by setting unknow to EINA_TRUE.
 *
 * @since 1.2.4
 * @ingroup Eet_Data_Group
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
 * Add a mapping to a data descriptor that will be used by union, variant or inherited type
 * @param unified_type The data descriptor to add the mapping to.
 * @param name The string name to get/set type.
 * @param subtype The matching data descriptor.
 *
 * @since 1.2.4
 * @ingroup Eet_Data_Group
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
 * Add a mapping of a basic type to a data descriptor that will be used by a union type.
 * @param unified_type The data descriptor to add the mapping to.
 * @param name The string name to get/set type.
 * @param basic_type The matching basic type.
 *
 * @since 1.8
 * @ingroup Eet_Data_Group
 * @see Eet_Data_Descriptor_Class
 */
#define EET_DATA_DESCRIPTOR_ADD_MAPPING_BASIC(unified_type, name, basic_type) \
  eet_data_descriptor_element_add(unified_type,                               \
                                  name,                                       \
                                  basic_type,                                 \
                                  EET_G_UNKNOWN,                              \
                                  0,                                          \
                                  0,                                          \
                                  NULL,                                       \
                                  NULL)
/**
 * @defgroup Eet_Data_Cipher_Group Eet Data Serialization using A Ciphers
 *
 * Most of the @ref Eet_Data_Group have alternative versions that
 * accounts for ciphers to protect their content.
 *
 * @see @ref Eet_Cipher_Group
 *
 * @ingroup Eet_Data_Group
 */

/**
 * Read a data structure from an eet file and decodes it using a cipher.
 * @param ef The eet file handle to read from.
 * @param edd The data descriptor handle to use when decoding.
 * @param name The key the data is stored under in the eet file.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_read()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI void *
eet_data_read_cipher(Eet_File *ef,
                     Eet_Data_Descriptor *edd,
                     const char *name,
                     const char *cipher_key);

/**
 * Read a data structure from an eet extended attribute and decodes it using a cipher.
 * @param filename The file to extract the extended attribute from.
 * @param attribute The attribute to get the data from.
 * @param edd The data descriptor handle to use when decoding.
 * @param cipher_key The key to use as cipher.
 * @return A pointer to the decoded data structure.
 *
 * This function decodes a data structure stored in an eet extended attribute,
 * returning a pointer to it if it decoded successfully, or NULL on failure.
 * Eet can handle members being added or deleted from the data in
 * storage and safely zero-fills unfilled members if they were not found
 * in the data. It checks sizes and headers whenever it reads data, allowing
 * the programmer to not worry about corrupt data.
 *
 * Once a data structure has been described by the programmer with the
 * fields they wish to save or load, storing or retrieving a data structure
 * from an eet file, from a chunk of memory or from an extended attribute
 * is as simple as a single function call.
 *
 * @since 1.5.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI void *
eet_data_xattr_cipher_get(const char *filename,
                          const char *attribute,
                          Eet_Data_Descriptor *edd,
                          const char *cipher_key);

/**
 * Write a data structure from memory and store in an eet file
 * using a cipher.
 * @param ef The eet file handle to write to.
 * @param edd The data descriptor to use when encoding.
 * @param name The key to store the data under in the eet file.
 * @param cipher_key The key to use as cipher.
 * @param data A pointer to the data structure to save and encode.
 * @param compress Compression flags for storage.
 * @return bytes written on successful write, 0 on failure.
 *
 * This function is the reverse of eet_data_read_cipher(), saving a data structure
 * to an eet file.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI int
eet_data_write_cipher(Eet_File *ef,
                      Eet_Data_Descriptor *edd,
                      const char *name,
                      const char *cipher_key,
                      const void *data,
                      int compress);

/**
 * Write a data structure from memory and store in an eet extended attribute
 * using a cipher.
 * @param filename The file to write the extended attribute to.
 * @param attribute The attribute to store the data to.
 * @param edd The data descriptor to use when encoding.
 * @param cipher_key The key to use as cipher.
 * @param data A pointer to the data structure to save and encode.
 * @param flags The policy to use when setting the data.
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * This function is the reverse of eet_data_xattr_cipher_get(), saving a data structure
 * to an eet extended attribute.
 *
 * @since 1.5.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI Eina_Bool
eet_data_xattr_cipher_set(const char *filename,
                          const char *attribute,
                          Eet_Data_Descriptor *edd,
                          const char *cipher_key,
                          const void *data,
                          Eina_Xattr_Flags flags);

/**
 * Dump an eet encoded data structure into ascii text using a cipher.
 * @param data_in The pointer to the data to decode into a struct.
 * @param cipher_key The key to use as cipher.
 * @param size_in The size of the data pointed to in bytes.
 * @param dumpfunc The function to call passed a string when new
 *        data is converted to text
 * @param dumpdata The data to pass to the @p dumpfunc callback.
 * @return 1 on success, 0 on failure
 *
 * This function will take a chunk of data encoded by
 * eet_data_descriptor_encode() and convert it into human readable
 * ascii text.  It does this by calling the @p dumpfunc callback
 * for all new text that is generated. This callback should append
 * to any existing text buffer and will be passed the pointer @p
 * dumpdata as a parameter as well as a string with new text to be
 * appended.
 *
 * Example:
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
 * @see eet_data_text_dump()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI int
eet_data_text_dump_cipher(const void *data_in,
                          const char *cipher_key,
                          int size_in,
                          Eet_Dump_Callback dumpfunc,
                          void *dumpdata);

/**
 * Take an ascii encoding from eet_data_text_dump() and re-encode
 * in binary using a cipher.
 * @param text The pointer to the string data to parse and encode.
 * @param cipher_key The key to use as cipher.
 * @param textlen The size of the string in bytes (not including 0
 *        byte terminator).
 * @param size_ret This gets filled in with the encoded data blob
 *        size in bytes.
 * @return The encoded data on success, NULL on failure.
 *
 * This function will parse the string pointed to by @p text and return
 * an encoded data lump the same way eet_data_descriptor_encode() takes an
 * in-memory data struct and encodes into a binary blob. @p text is a normal
 * C string.
 *
 * @see eet_data_text_undump()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI void *
eet_data_text_undump_cipher(const char *text,
                            const char *cipher_key,
                            int textlen,
                            int *size_ret);

/**
 * Dump an eet encoded data structure from an eet file into ascii
 * text using a cipher.
 * @param ef A valid eet file handle.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
 * @param dumpfunc The function to call passed a string when new
 *        data is converted to text
 * @param dumpdata The data to pass to the @p dumpfunc callback.
 * @return 1 on success, 0 on failure
 *
 * This function will take an open and valid eet file from
 * eet_open() request the data encoded by
 * eet_data_descriptor_encode() corresponding to the key @p name
 * and convert it into human readable ascii text. It does this by
 * calling the @p dumpfunc callback for all new text that is
 * generated. This callback should append to any existing text
 * buffer and will be passed the pointer @p dumpdata as a parameter
 * as well as a string with new text to be appended.
 *
 * @see eet_data_dump()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI int
eet_data_dump_cipher(Eet_File *ef,
                     const char *name,
                     const char *cipher_key,
                     Eet_Dump_Callback dumpfunc,
                     void *dumpdata);

/**
 * Take an ascii encoding from eet_data_dump() and re-encode in
 * binary using a cipher.
 * @param ef A valid eet file handle.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
 * @param text The pointer to the string data to parse and encode.
 * @param textlen The size of the string in bytes (not including 0
 *        byte terminator).
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @return 1 on success, 0 on failure
 *
 * This function will parse the string pointed to by @p text,
 * encode it the same way eet_data_descriptor_encode() takes an
 * in-memory data struct and encodes into a binary blob.
 *
 * The data (optionally compressed) will be in ram, pending a flush to
 * disk (it will stay in ram till the eet file handle is closed though).
 *
 * @see eet_data_undump()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI int
eet_data_undump_cipher(Eet_File *ef,
                       const char *name,
                       const char *cipher_key,
                       const char *text,
                       int textlen,
                       int compress);

/**
 * Decode a data structure from an arbitrary location in memory
 * using a cipher.
 * @param edd The data  descriptor to use when decoding.
 * @param data_in The pointer to the data to decode into a struct.
 * @param cipher_key The key to use as cipher.
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
 * @see eet_data_descriptor_decode()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI void *
eet_data_descriptor_decode_cipher(Eet_Data_Descriptor *edd,
                                  const void *data_in,
                                  const char *cipher_key,
                                  int size_in);

/**
 * Encode a data struct to memory and return that encoded data
 * using a cipher.
 * @param edd The data  descriptor to use when encoding.
 * @param data_in The pointer to the struct to encode into data.
 * @param cipher_key The key to use as cipher.
 * @param size_ret pointer to the an int to be filled with the decoded size.
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
 * @see eet_data_descriptor_encode()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI void *
eet_data_descriptor_encode_cipher(Eet_Data_Descriptor *edd,
                                  const void *data_in,
                                  const char *cipher_key,
                                  int *size_ret);

/**
 * @defgroup Eet_Node_Group Low-level Serialization Structures.
 * @ingroup Eet
 *
 * Functions that create, destroy and manipulate serialization nodes
 * used by @ref Eet_Data_Group.
 *
 * @{
 */

/**
 * @typedef Eet_Node
 * Opaque handle to manage serialization node.
 */
typedef struct _Eet_Node Eet_Node;

/**
 * @typedef Eet_Node_Data
 * Contains an union that can fit any kind of node.
 */
typedef struct _Eet_Node_Data Eet_Node_Data;

/**
 * @struct _Eet_Node_Data
 * Contains an union that can fit any kind of node.
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
 * @}
 */

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_char_new(const char *name,
                  char c);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_short_new(const char *name,
                   short s);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_int_new(const char *name,
                 int i);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_long_long_new(const char *name,
                       long long l);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_float_new(const char *name,
                   float f);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_double_new(const char *name,
                    double d);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_char_new(const char *name,
                           unsigned char uc);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_short_new(const char *name,
                            unsigned short us);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_int_new(const char *name,
                          unsigned int ui);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_long_long_new(const char *name,
                                unsigned long long l);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_string_new(const char *name,
                    const char *str);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_inlined_string_new(const char *name,
                            const char *str);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_null_new(const char *name);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_list_new(const char *name,
                  Eina_List *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_array_new(const char *name,
                   int count,
                   Eina_List *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_var_array_new(const char *name,
                       Eina_List *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_hash_new(const char *name,
                  const char *key,
                  Eet_Node *node);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_struct_new(const char *name,
                    Eina_List *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_struct_child_new(const char *parent,
                          Eet_Node *child);

/**
 * @brief Get a node's child nodes
 * @param node The node
 * @return The first child node which contains a pointer to the
 * next child node and the parent.
 * @since 1.5
 */
EAPI Eet_Node *
eet_node_children_get(Eet_Node *node);

/**
 * @brief Get the next node in a list of nodes
 * @param node The node
 * @return A node which contains a pointer to the
 * next child node and the parent.
 * @since 1.5
 */
EAPI Eet_Node *
eet_node_next_get(Eet_Node *node);

/**
 * @brief Get the parent node of a node
 * @param node The node
 * @return The parent node of @p node
 * @since 1.5
 */
EAPI Eet_Node *
eet_node_parent_get(Eet_Node *node);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_list_append(Eet_Node *parent,
                     const char *name,
                     Eet_Node *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_struct_append(Eet_Node *parent,
                       const char *name,
                       Eet_Node *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_hash_add(Eet_Node *parent,
                  const char *name,
                  const char *key,
                  Eet_Node *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_dump(Eet_Node *n,
              int dumplevel,
              Eet_Dump_Callback dumpfunc,
              void *dumpdata);

/**
 * @brief Return the type of a node
 * @param node The node
 * @return The node's type (EET_T_$TYPE)
 * @since 1.5
 */
EAPI int
eet_node_type_get(Eet_Node *node);

/**
 * @brief Return the node's data
 * @param node The node
 * @return The data contained in the node
 * @since 1.5
 */
EAPI Eet_Node_Data *
eet_node_value_get(Eet_Node *node);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_del(Eet_Node *n);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void *
eet_data_node_encode_cipher(Eet_Node *node,
                            const char *cipher_key,
                            int *size_ret);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_data_node_decode_cipher(const void *data_in,
                            const char *cipher_key,
                            int size_in);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_data_node_read_cipher(Eet_File *ef,
                          const char *name,
                          const char *cipher_key);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
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
 * Describes how to walk trees of #Eet_Node.
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
 * @struct _Eet_Node_Walk
 * Describes how to walk trees of #Eet_Node.
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

/*******/

/**
 * @defgroup Eet_Connection_Group Helper function to use eet over a network link
 * @ingroup Eet
 *
 * Function that reconstruct and prepare packet of @ref Eet_Data_Group to be send.
 *
 */

/**
 * @typedef Eet_Connection
 * Opaque handle to track paquet for a specific connection.
 *
 * @ingroup Eet_Connection_Group
 */
typedef struct _Eet_Connection Eet_Connection;

/**
 * @typedef Eet_Read_Cb
 * Called back when an @ref Eet_Data_Group has been received completely and could be used.
 *
 * @ingroup Eet_Connection_Group
 */
typedef Eina_Bool Eet_Read_Cb (const void *eet_data, size_t size, void *user_data);

/**
 * @typedef Eet_Write_Cb
 * Called back when a packet containing @ref Eet_Data_Group data is ready to be send.
 *
 * @ingroup Eet_Connection_Group
 */
typedef Eina_Bool Eet_Write_Cb (const void *data, size_t size, void *user_data);

/**
 * Instanciate a new connection to track.
 * @param eet_read_cb Function to call when one Eet_Data packet has been fully assemble.
 * @param eet_write_cb Function to call when one Eet_Data packet is ready to be send over the wire.
 * @param user_data Pointer provided to both functions to be used as a context handler.
 * @return NULL on failure, or a valid Eet_Connection handler.
 *
 * For every connection to track you will need a separate Eet_Connection provider.
 *
 * @since 1.2.4
 * @ingroup Eet_Connection_Group
 */
EAPI Eet_Connection *
eet_connection_new(Eet_Read_Cb *eet_read_cb,
                   Eet_Write_Cb *eet_write_cb,
                   const void *user_data);

/**
 * Process a raw packet received over the link
 * @param conn Connection handler to track.
 * @param data Raw data packet.
 * @param size The size of that packet.
 * @return 0 on complete success, any other value indicate where in the stream it got wrong (It could be before that packet).
 *
 * Every time you receive a packet related to your connection, you should pass
 * it to that function so that it could process and assemble packet has you
 * receive it. It will automatically call Eet_Read_Cb when one is fully received.
 *
 * @since 1.2.4
 * @ingroup Eet_Connection_Group
 */
EAPI int
eet_connection_received(Eet_Connection *conn,
                        const void *data,
                        size_t size);

/**
 * Tell if the Eet_Connection as received some partial data.
 * @param conn Connection handler to request.
 * @return EINA_TRUE if there is some data pending inside, EINA_FALSE otherwise.
 *
 * Eet_Connection buffer data until the received data can be unserialized correctly. This
 * function let you know if there is some data inside that buffer waiting for more data to
 * be received before being processed.
 *
 * @since 1.7
 * @ingroup Eet_Connection_Group
 */
EAPI Eina_Bool eet_connection_empty(Eet_Connection *conn);

/**
 * Convert a complex structure and prepare it to be send.
 * @param conn Connection handler to track.
 * @param edd The data descriptor to use when encoding.
 * @param data_in The pointer to the struct to encode into data.
 * @param cipher_key The key to use as cipher.
 * @return EINA_TRUE if the data where correctly send, EINA_FALSE if they don't.
 *
 * This function serialize data_in with edd, assemble the packet and call
 * Eet_Write_Cb when ready. The data passed Eet_Write_Cb are temporary allocated
 * and will vanish just after the return of the callback.
 *
 * @see eet_data_descriptor_encode_cipher
 *
 * @since 1.2.4
 * @ingroup Eet_Connection_Group
 */
EAPI Eina_Bool
eet_connection_send(Eet_Connection *conn,
                    Eet_Data_Descriptor *edd,
                    const void *data_in,
                    const char *cipher_key);

/**
 * Convert a Eet_Node tree and prepare it to be send.
 * @param conn Connection handler to track.
 * @param node The data tree to use when encoding.
 * @param cipher_key The key to use as cipher.
 * @return EINA_TRUE if the data where correctly send, EINA_FALSE if they don't.
 *
 * This function serialize node, assemble the packet and call
 * Eet_Write_Cb when ready. The data passed Eet_Write_Cb are temporary allocated
 * and will vanish just after the return of the callback.
 *
 * @see eet_data_node_encode_cipher
 *
 * @since 1.2.4
 * @ingroup Eet_Connection_Group
 */
EAPI Eina_Bool
eet_connection_node_send(Eet_Connection *conn,
                         Eet_Node *node,
                         const char *cipher_key);

/**
 * Close a connection and lost its track.
 * @param conn Connection handler to close.
 * @param on_going Signal if a partial packet wasn't completed.
 * @return the user_data passed to both callback.
 *
 * @since 1.2.4
 * @ingroup Eet_Connection_Group
 */
EAPI void *
eet_connection_close(Eet_Connection *conn,
                     Eina_Bool *on_going);

/***************************************************************************/

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef _EET_H */
