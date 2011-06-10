/**
@brief Eet Data Handling Library Public API Calls

These routines are used for Eet Library interaction

@mainpage Eet Library Documentation

@image html  e_big.png

@version 1.0.0
@author Carsten Haitzler <raster@@rasterman.com>
@author David Goodlad <dgoodlad@@gmail.com>
@author Cedric Bail <cedric.bail@@free.fr>
@author Arnaud de Turckheim <quarium@@gmail.com>
@author Luis Felipe Strano Moraes <lfelipe@@profusion.mobi>
@author Chidambar Zinnoury <illogict@@online.fr>
@author Vincent Torri <vtorri@@univ-evry.fr>
@author Gustavo Sverzut Barbieri <barbieri@@profusion.mobi>
@author Raphael Kubo da Costa <kubo@@profusion.mobi>
@author Mathieu Taillefumier <mathieu.taillefumier@@free.fr>
@author Albin "Lutin" Tonnerre <albin.tonnerre@@gmail.com>
@author Adam Simpkins <adam@@adamsimpkins.net>
@date 2000-2011

@section toc Table of Contents

@li @ref intro
@li @ref example
@li @ref format
@li @ref compiling
@li @ref install
@li @ref next_steps
@li @ref intro_example

@section intro What is Eet?

It is a tiny library designed to write an arbitrary set of chunks of data
to a file and optionally compress each chunk (very much like a zip file)
and allow fast random-access reading of the file later on. It does not
do zip as a zip itself has more complexity than is needed, and it was much
simpler to implement this once here.

Eet is extremely fast, small and simple. Eet files can be very small and
highly compressed, making them very optimal for just sending across the
internet without having to archive, compress or decompress and install them.
They allow for lightning-fast random-acess reads once created, making them
perfect for storing data that is written once (or rarely) and read many
times, but the program does not want to have to read it all in at once.

It also can encode and decode data structures in memory, as well as image
data for saving to Eet files or sending across the network to other
machines, or just writing to arbitrary files on the system. All data is
encoded in a platform independent way and can be written and read by any
architecture.

@section example A simple example on using Eet

Here is a simple example on how to use Eet to save a series of strings to a
file and load them again. The advantage of using Eet over just fprintf() and
fscanf() is that not only can these entries be strings, they need no special
parsing to handle delimiter characters or escaping, they can be binary data,
image data, data structures containing integers, strings, other data
structures, linked lists and much more, without the programmer having to
worry about parsing, and best of all, Eet is very fast.

@code
#include <Eet.h>

int
main(int argc, char **argv)
{
  Eet_File *ef;
  int       i;
  char      buf[32];
  char     *ret;
  int       size;
  char     *entries[] =
    {
      "Entry 1",
      "Big text string here compared to others",
      "Eet is cool"
    };

  eet_init();

  // blindly open an file for output and write strings with their NUL char
  ef = eet_open("test.eet", EET_FILE_MODE_WRITE);
  eet_write(ef, "Entry 1", entries[0], strlen(entries[0]) + 1, 0);
  eet_write(ef, "Entry 2", entries[1], strlen(entries[1]) + 1, 1);
  eet_write(ef, "Entry 3", entries[2], strlen(entries[2]) + 1, 0);
  eet_close(ef);

  // open the file again and blindly get the entries we wrote
  ef = eet_open("test.eet", EET_FILE_MODE_READ);
  ret = eet_read(ef, "Entry 1", &size);
  printf("%s\n", ret);
  ret = eet_read(ef, "Entry 2", &size);
  printf("%s\n", ret);
  ret = eet_read(ef, "Entry 3", &size);
  printf("%s\n", ret);
  eet_close(ef);

  eet_shutdown();
}
@endcode

@section format What does an Eet file look like?

The file format is very simple. There is a directory block at the start of
the file listing entries and offsets into the file where they are stored,
their sizes, compression flags etc. followed by all the entry data strung one
element after the other.

All Eet files start with t a 4 byte magic number. It is written using network
byte-order (big endian, or from most significant byte first to least
significant byte last) and is 0x1ee7ff00 (or byte by byte 0:1e 1:e7 2:ff
3:00). The next 4 bytes are an integer (in big endian notation) indicating
how many entries are stored in the Eet file. 0 indicates it is empty. This is
a signed integer and thus values less than 0 are invalid, limiting the number
of entries in an Eet file to 0x7fffffff entries at most. The next 4 bytes is
the size of the directory table, in bytes, encoded in big-endian format. This
is a signed integer and cannot be less than 0.

The directory table for the file follows immediately, with a continuous list
of all entries in the Eet file, their offset in the file etc. The order of
these entries is not important, but convention would have them be from first
to last entry in the file. Each directory entry consiste of 5 integers, one
after the other, each stored as a signed, big endian integer. The first is
the offset in the file that the data for this entry is stored at (based from
the very start of the file, not relative to the end of the directory block).
The second integer holds flags for the entry. currently only the least
significant bit (bit 0) holds any useful information, and it is set to 1 if
the entry is compressed using zlib compression calls, or 0 if it is not
compressed. The next integer is the size of the entry in bytes stored in the
file. The next integer is the size of the data when decompressed (if it was
compressed) in bytes. This may be the same as the previous integer if the
entry was not compressed. The final integer is the number of bytes used by
the string identifier for the entry, without the NUL byte terminator, which
is not stored. The next series of bytes is the string name of the entry, with
the number of bytes being the same as specified in the last integer above.
This list of entries continues until there are no more entries left to list.
To read an entry from an Eet file, simply find the appropriate entry in the
directory table, find it's offset and size, and read it into memory. If it is
compressed, decompress it using zlib and then use that data.

Here is a data map of an Eet file. All integers are encoded using big-endian
notation (most significant byte first) and are signed. There is no alignment
of data, so all data types follow immediately on, one after the other. All
compressed data is compressed using the zlib compress2() function, and
decompressed using the zlib uncompress() function. Please see zlib
documentation for more information as to the encoding of compressed data.

@verbatim
HEADER:
[INT] Magic number (0x1ee7ff00)
[INT] Number of entries in the directory table
[INT] The size of the directory table, in bytes

DIRECTORY TABLE ENTRIES (as many as specified in the header):
[INT] Offest from file start at which entry is stored (in bytes)
[INT] Entry flags (1 = compressed, 0 = not compressed)
[INT] Size of data chunk in file (in bytes)
[INT] Size of the data chunk once decompressed (or the same as above, if not)
[INT] The length of the string itendifier, in bytes, without NUL terminator
[STR] Series of bytes for the string identifier, no NUL terminator
... more directory entries

DATA STORED, ONE AFTER ANOTHER:
[DAT] DATA ENTRY 1...
[DAT] DATA ENTRY 2...
[DAT] DATA ENTRY 3...
... more data chunks
@endverbatim

The contents of each entry in an Eet file has no defined format as such. It
is an opaque chunk of data, that is up to the application to deocde, unless
it is an image, ecoded by Eet, or a data structure encoded by Eet. The data
itself for these entries can be encoded and decoded by Eet with extra helper
functions in Eet. eet_data_image_read() and eet_data_image_write() are used
to handle reading and writing image data from a known Eet file entry name.
eet_data_read() and eet_data_write() are used to decode and encode program
data structures from an Eet file, making the loading and saving of program
information stored in data structures a simple 1 function call process.

Please see src/lib/eet_data.c for information on the format of these
specially encoded data entries in an Eet file (for now).


@section compiling How to compile using Eet ?

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

You simply have to make sure that pkg-config is in your shell's PATH (see
the manual page for your appropriate shell) and eet.pc in /usr/lib/pkgconfig
or its path is in the PKG_CONFIG_PATH environment variable. It's that simple
to link and use Eet once you have written your code to use it.

Since the program is linked to Eet, it is now able to use any advertised
API calls to serialize your data.

You should make sure you add any extra compile and link flags to your
compile commands that your application may need as well. The above example
is only guaranteed to make Eet add it's own requirements.


@section install How is it installed?

Simple:

@verbatim
./configure
make
su -
...
make install
@endverbatim

@section next_steps Next Steps

After you understood what Eet is and installed it in your system you
should proceed understanding the programming interface. We'd recommend
you to take a while to learn Eina
(http://docs.enlightenment.org/auto/eina/) as it is very convenient
and optimized, and Eet provides integration with it.

Recommended reading:

@li @ref Eet_File_Group to know the basics to open and save files.
@li @ref Eet_Data_Group to know the convenient way to serialize and
    parse your data structures automatically. Just create your
    descriptors and let Eet do the work for you.

@section intro_example Introductory Examples

@ref Examples

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
 * @file Eet.h
 * @brief The file that provides the eet functions.
 *
 * This header provides the Eet management functions.
 *
 */

#define EET_VERSION_MAJOR 1
#define EET_VERSION_MINOR 4
/**
 * @typedef Eet_Version
 *
 * This is the Eet version information structure that can be used at
 * runtiime to detect which version of eet is being used and adapt
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
 * Note the #if check can be dropped if your program refuses to compile or
 * work with an Eet version less than 1.3.0.
 */
typedef struct _Eet_Version
{
   int major; /** < major (binary or source incompatible changes) */
   int minor; /** < minor (new features, bugfixes, major improvements version) */
   int micro; /** < micro (bugfix, internal improvements, no new features version) */
   int revision; /** < svn revision (0 if a proper rlease or the svn revsion number Eet is built from) */
} Eet_Version;

EAPI extern Eet_Version *eet_version;

/**
 * @defgroup Eet_Group Top level functions
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
   EET_ERROR_NOT_WRITABLE, /**< Could not write to file or fine is #EET_FILE_MODE_READ */
   EET_ERROR_OUT_OF_MEMORY, /**< Could not allocate memory */
   EET_ERROR_WRITE_ERROR, /**< Failed to write data to destination */
   EET_ERROR_WRITE_ERROR_FILE_TOO_BIG, /**< Failed to write file since it is too big */
   EET_ERROR_WRITE_ERROR_IO_ERROR, /**< Failed to write since generic Input/Output error */
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
 * Initialize the EET library.
 *
 * @return The new init count.
 *
 * @since 1.0.0
 * @ingroup Eet_Group
 */
EAPI int
eet_init(void);

/**
 * Shut down the EET library.
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
 * Eet didn't free items by default. If you are under memory
 * presure, just call this function to recall all memory that are
 * not yet referenced anymore.  The cache take care of modification
 * on disk.
 *
 * @since 1.0.0
 * @ingroup Eet_Group
 */
EAPI void
eet_clearcache(void);

/**
 * @defgroup Eet_File_Group Eet File Main Functions
 *
 * Functions to create, destroy and do basic manipulation of
 * #Eet_File handles.
 *
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
 * @see eet_open()
 * @see eet_memopen_read()
 * @see eet_close()
 */
typedef struct _Eet_File         Eet_File;

/**
 * @typedef Eet_Dictionary
 * Opaque handle that defines a file-backed (mmaped) dictionary of strings.
 */
typedef struct _Eet_Dictionary   Eet_Dictionary;

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
 * Example:
 * @code
 * #include <Eet.h>
 * #include <stdio.h>
 * #include <string.h>
 *
 * int
 * main(int argc, char **argv)
 * {
 *   Eet_File *ef;
 *   char buf[1024], *ret, **list;
 *   int size, num, i;
 *
 *   eet_init();
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
 *   eet_shutdown();
 *
 *   return 0;
 * }
 * @endcode
 *
 * @since 1.0.0
 */
EAPI Eet_File *
eet_open(const char   *file,
         Eet_File_Mode mode);

/**
 * Open an eet file directly from a memory location. The data is not copied,
 * so you must keep it around as long as the eet file is open. There is
 * currently no cache for this kind of Eet_File, so it's reopened every time
 * you use eet_memopen_read.
 *
 * @since 1.1.0
 * @ingroup Eet_File_Group
 */
EAPI Eet_File *
eet_memopen_read(const void *data,
                 size_t      size);

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
 * @ingroup Eet_File_Group
 */
EAPI Eet_Error
eet_close(Eet_File *ef);

/**
 * Sync content of an eet file handle, flushing pending writes.
 * @param ef A valid eet file handle.
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
 *      the #Eet_Data_Descriptor_Class instructrions.
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
 * If the dictionary handle is invlide, the string is NULL or the string is
 * not in the dictionary, 0 is returned.
 *
 * @since 1.0.0
 * @ingroup Eet_File_Group
 */
EAPI int
eet_dictionary_string_check(Eet_Dictionary *ed,
                            const char     *string);

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
eet_read(Eet_File   *ef,
         const char *name,
         int        *size_ret);

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
 * @ingroup Eet_File_Group
 */
EAPI const void *
eet_read_direct(Eet_File   *ef,
                const char *name,
                int        *size_ret);

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
eet_write(Eet_File   *ef,
          const char *name,
          const void *data,
          int         size,
          int         compress);

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
eet_delete(Eet_File   *ef,
           const char *name);

/**
 * Alias a specific section to another one. Destination may exist or not,
 * no check are done.
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param destination Destionation of the alias. eg: "/base/the_real_stuff_i_want".
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * Name and Destination must not be NULL, otherwhise EINA_FALSE will be returned.
 *
 * @since 1.3.3
 * @ingroup Eet_File_Group
 */
EAPI Eina_Bool
eet_alias(Eet_File   *ef,
          const char *name,
          const char *destination,
          int         compress);

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
eet_list(Eet_File   *ef,
         const char *glob,
         int        *count_ret);

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
eet_read_cipher(Eet_File   *ef,
                const char *name,
                int        *size_ret,
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
eet_write_cipher(Eet_File   *ef,
                 const char *name,
                 const void *data,
                 int         size,
                 int         compress,
                 const char *cipher_key);

/**
 * @defgroup Eet_File_Image_Group Image Store and Load
 *
 * Eet efficiently stores and loads images, including alpha
 * channels and lossy compressions.
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
 * @see eet_data_image_header_read_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_header_read(Eet_File     *ef,
                           const char   *name,
                           unsigned int *w,
                           unsigned int *h,
                           int          *alpha,
                           int          *compress,
                           int          *quality,
                           int          *lossy);

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
 * @see eet_data_image_read_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI void *
eet_data_image_read(Eet_File     *ef,
                    const char   *name,
                    unsigned int *w,
                    unsigned int *h,
                    int          *alpha,
                    int          *compress,
                    int          *quality,
                    int          *lossy);

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
 * @see eet_data_image_read_to_surface_cipher()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_read_to_surface(Eet_File     *ef,
                               const char   *name,
                               unsigned int  src_x,
                               unsigned int  src_y,
                               unsigned int *d,
                               unsigned int  w,
                               unsigned int  h,
                               unsigned int  row_stride,
                               int          *alpha,
                               int          *compress,
                               int          *quality,
                               int          *lossy);

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
 * @see eet_data_image_write_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_write(Eet_File    *ef,
                     const char  *name,
                     const void  *data,
                     unsigned int w,
                     unsigned int h,
                     int          alpha,
                     int          compress,
                     int          quality,
                     int          lossy);

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
 * @see eet_data_image_header_decode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_header_decode(const void   *data,
                             int           size,
                             unsigned int *w,
                             unsigned int *h,
                             int          *alpha,
                             int          *compress,
                             int          *quality,
                             int          *lossy);

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
 * @see eet_data_image_decode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI void *
eet_data_image_decode(const void   *data,
                      int           size,
                      unsigned int *w,
                      unsigned int *h,
                      int          *alpha,
                      int          *compress,
                      int          *quality,
                      int          *lossy);

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
 * @see eet_data_image_decode_to_surface_cipher()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Group
 */
EAPI int
eet_data_image_decode_to_surface(const void   *data,
                                 int           size,
                                 unsigned int  src_x,
                                 unsigned int  src_y,
                                 unsigned int *d,
                                 unsigned int  w,
                                 unsigned int  h,
                                 unsigned int  row_stride,
                                 int          *alpha,
                                 int          *compress,
                                 int          *quality,
                                 int          *lossy);

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
 * @see eet_data_image_encode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Group
 */
EAPI void *
eet_data_image_encode(const void  *data,
                      int         *size_ret,
                      unsigned int w,
                      unsigned int h,
                      int          alpha,
                      int          compress,
                      int          quality,
                      int          lossy);

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
 * @see eet_data_image_header_read()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
 */
EAPI int
eet_data_image_header_read_cipher(Eet_File     *ef,
                                  const char   *name,
                                  const char   *cipher_key,
                                  unsigned int *w,
                                  unsigned int *h,
                                  int          *alpha,
                                  int          *compress,
                                  int          *quality,
                                  int          *lossy);

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
 * @see eet_data_image_read()
 *
 * @since 1.0.0
 * @ingroup Eet_File_Image_Cipher_Group
 */
EAPI void *
eet_data_image_read_cipher(Eet_File     *ef,
                           const char   *name,
                           const char   *cipher_key,
                           unsigned int *w,
                           unsigned int *h,
                           int          *alpha,
                           int          *compress,
                           int          *quality,
                           int          *lossy);
   
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
 * @see eet_data_image_read_to_surface()
 *
 * @since 1.0.2
 * @ingroup Eet_File_Image_Cipher_Group
 */
EAPI int
eet_data_image_read_to_surface_cipher(Eet_File     *ef,
                                      const char   *name,
                                      const char   *cipher_key,
                                      unsigned int  src_x,
                                      unsigned int  src_y,
                                      unsigned int *d,
                                      unsigned int  w,
                                      unsigned int  h,
                                      unsigned int  row_stride,
                                      int          *alpha,
                                      int          *compress,
                                      int          *quality,
                                      int          *lossy);

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
eet_data_image_write_cipher(Eet_File    *ef,
                            const char  *name,
                            const char  *cipher_key,
                            const void  *data,
                            unsigned int w,
                            unsigned int h,
                            int          alpha,
                            int          compress,
                            int          quality,
                            int          lossy);

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
eet_data_image_header_decode_cipher(const void   *data,
                                    const char   *cipher_key,
                                    int           size,
                                    unsigned int *w,
                                    unsigned int *h,
                                    int          *alpha,
                                    int          *compress,
                                    int          *quality,
                                    int          *lossy);

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
eet_data_image_decode_cipher(const void   *data,
                             const char   *cipher_key,
                             int           size,
                             unsigned int *w,
                             unsigned int *h,
                             int          *alpha,
                             int          *compress,
                             int          *quality,
                             int          *lossy);

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
eet_data_image_decode_to_surface_cipher(const void   *data,
                                        const char   *cipher_key,
                                        int           size,
                                        unsigned int  src_x,
                                        unsigned int  src_y,
                                        unsigned int *d,
                                        unsigned int  w,
                                        unsigned int  h,
                                        unsigned int  row_stride,
                                        int          *alpha,
                                        int          *compress,
                                        int          *quality,
                                        int          *lossy);

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
eet_data_image_encode_cipher(const void   *data,
                             const char   *cipher_key,
                             unsigned int  w,
                             unsigned int  h,
                             int           alpha,
                             int           compress,
                             int           quality,
                             int           lossy,
                             int          *size_ret);

/**
 * @defgroup Eet_Cipher_Group Cipher, Identity and Protection Mechanisms
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
typedef struct _Eet_Key   Eet_Key;

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
typedef int (*Eet_Key_Password_Callback) (char *buffer, int size, int rwflag, void *data);

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
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI Eet_Key *
eet_identity_open(const char                *certificate_file,
                  const char                *private_key_file,
                  Eet_Key_Password_Callback  cb);

/**
 * Close and release all ressource used by an Eet_Key.  An
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
                 Eet_Key  *key);

/**
 * Display both private and public key of an Eet_Key.
 *
 * @param key the handle to print.
 * @param out where to print.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI void
eet_identity_print(Eet_Key *key,
                   FILE    *out);

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
                  int      *der_length);

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
                       int      *signature_length);

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
                  int      *sha1_length);

/**
 * Display the x509 der certificate to out.
 *
 * @param certificate the x509 certificate to print
 * @param der_length The length the certificate.
 * @param out where to print.
 *
 * @since 1.2.0
 * @ingroup Eet_Cipher_Group
 */
EAPI void
eet_identity_certificate_print(const unsigned char *certificate,
                               int                  der_length,
                               FILE                *out);

/**
 * @defgroup Eet_Data_Group Eet Data Serialization
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
 * Example:
 *
 * @code
 * #include <Eina.h>
 * #include <Eet.h>
 *
 * typedef struct _blah2
 * {
 *    char *string;
 * } Blah2;
 *
 * typedef struct _blah3
 * {
 *    char *string;
 * } Blah3;
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
 *    Eina_List *blah3;
 * } Blah;
 *
 * int
 * main(int argc, char **argv)
 * {
 *    Blah blah;
 *    Blah2 blah2;
 *    Blah3 blah3;
 *    Eet_Data_Descriptor *edd, *edd2, *edd3;
 *    Eet_Data_Descriptor_Class eddc, eddc2, eddc3;
 *    void *data;
 *    int size;
 *    FILE *f;
 *    Blah *blah_in;
 *
 *    eet_init();
 *
 *    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc3, Blah3);
 *    edd3 = eet_data_descriptor_stream_new(&eddc3);
 *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd3, Blah3, "string3", string, EET_T_STRING);
 *
 *    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc2, Blah2);
 *    edd2 = eet_data_descriptor_stream_new(&eddc2);
 *    EET_DATA_DESCRIPTOR_ADD_BASIC(edd2, Blah2, "string2", string, EET_T_STRING);
 *
 *    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Blah);
 *    edd = eet_data_descriptor_stream_new(&eddc);
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
 *    blah3.string = "PANTS";
 *
 *    blah2.string = "subtype string here!";
 *
 *    blah.character = '7';
 *    blah.sixteen = 0x7777;
 *    blah.integer = 0xc0def00d;
 *    blah.lots = 0xdeadbeef31337777;
 *    blah.floating = 3.141592654;
 *    blah.floating_lots = 0.777777777777777;
 *    blah.string = "bite me like a turnip";
 *    blah.blah2 = &blah2;
 *    blah.blah3 = eina_list_append(NULL, &blah3);
 *    blah.blah3 = eina_list_append(blah.blah3, &blah3);
 *    blah.blah3 = eina_list_append(blah.blah3, &blah3);
 *    blah.blah3 = eina_list_append(blah.blah3, &blah3);
 *    blah.blah3 = eina_list_append(blah.blah3, &blah3);
 *    blah.blah3 = eina_list_append(blah.blah3, &blah3);
 *    blah.blah3 = eina_list_append(blah.blah3, &blah3);
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
 *         Eina_List *l;
 *         Blah3 *blah3_in;
 *
 *         EINA_LIST_FOREACH(blah_in->blah3, l, blah3_in)
 *           {
 *              printf("%p\n", blah3_in);
 *              printf("  %s\n", blah3_in->string);
 *           }
 *      }
 *    eet_data_descriptor_free(edd);
 *    eet_data_descriptor_free(edd2);
 *    eet_data_descriptor_free(edd3);
 *
 *    eet_shutdown();
 *
 *   return 0;
 * }
 * @endcode
 *
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
#define EET_T_LAST           18 /**< Last data type */

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
 * Opaque handle that have information on a type members.
 *
 * The members are added by means of
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), EET_DATA_DESCRIPTOR_ADD_SUB(),
 * EET_DATA_DESCRIPTOR_ADD_LIST(), EET_DATA_DESCRIPTOR_ADD_HASH()
 * or eet_data_descriptor_element_add().
 *
 * @see eet_data_descriptor_stream_new()
 * @see eet_data_descriptor_file_new()
 * @see eet_data_descriptor_free()
 */
typedef struct _Eet_Data_Descriptor         Eet_Data_Descriptor;

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
typedef struct _Eet_Data_Descriptor_Class   Eet_Data_Descriptor_Class;

typedef int         (*Eet_Descriptor_Hash_Foreach_Callback_Callback)(void *h, const char *k, void *dt, void *fdt);
  
typedef void       *(*Eet_Descriptor_Mem_Alloc_Callback)(size_t size);
typedef void        (*Eet_Descriptor_Mem_Free_Callback)(void *mem);
typedef char       *(*Eet_Descriptor_Str_Alloc_Callback)(const char *str);
typedef void        (*Eet_Descriptor_Str_Free_Callback)(const char *str);
typedef void       *(*Eet_Descriptor_List_Next_Callback)(void *l);
typedef void       *(*Eet_Descriptor_List_Append_Callback)(void *l, void *d);
typedef void       *(*Eet_Descriptor_List_Data_Callback)(void *l);
typedef void       *(*Eet_Descriptor_List_Free_Callback)(void *l);
typedef void        (*Eet_Descriptor_Hash_Foreach_Callback)(void *h, Eet_Descriptor_Hash_Foreach_Callback_Callback func, void *fdt);
typedef void       *(*Eet_Descriptor_Hash_Add_Callback)(void *h, const char *k, void *d);
typedef void        (*Eet_Descriptor_Hash_Free_Callback)(void *h);
typedef char       *(*Eet_Descriptor_Str_Direct_Alloc_Callback)(const char *str);
typedef void        (*Eet_Descriptor_Str_Direct_Free_Callback)(const char *str);
typedef const char *(*Eet_Descriptor_Type_Get_Callback)(const void *data, Eina_Bool *unknow);
typedef Eina_Bool   (*Eet_Descriptor_Type_Set_Callback)(const char *type, void *data, Eina_Bool unknow);
typedef void       *(*Eet_Descriptor_Array_Alloc_Callback)(size_t size);
typedef void        (*Eet_Descriptor_Array_Free_Callback)(void *mem);
/**
 * @struct _Eet_Data_Descriptor_Class
 *
 * Instructs Eet about memory management for different needs under
 * serialization and parse process.
 *
 * If using Eina data types, it is advised to use the helpers
 * EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET() and
 * EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET().
 */
struct _Eet_Data_Descriptor_Class
{
   int         version;  /**< ABI version as #EET_DATA_DESCRIPTOR_CLASS_VERSION */
   const char *name;  /**< Name of data type to be serialized */
   int         size;  /**< Size in bytes of data type to be serialized */
   struct {
     Eet_Descriptor_Mem_Alloc_Callback mem_alloc; /**< how to allocate memory (usually malloc()) */
     Eet_Descriptor_Mem_Free_Callback mem_free; /**< how to free memory (usually free()) */
     Eet_Descriptor_Str_Alloc_Callback str_alloc; /**< how to allocate a string */
     Eet_Descriptor_Str_Free_Callback str_free; /**< how to free a string */
     Eet_Descriptor_List_Next_Callback list_next; /**< how to iterate to the next element of a list. Receives and should return the list node. */
     Eet_Descriptor_List_Append_Callback list_append; /**< how to append data @p d to list which head node is @p l */
     Eet_Descriptor_List_Data_Callback list_data; /**< retrieves the data from node @p l */
     Eet_Descriptor_List_Free_Callback list_free; /**< free all the nodes from the list which head node is @p l */
     Eet_Descriptor_Hash_Foreach_Callback hash_foreach; /**< iterates over all elements in the hash @p h in no specific order */
     Eet_Descriptor_Hash_Add_Callback hash_add; /**< add a new data @p d as key @p k in hash @p h */
     Eet_Descriptor_Hash_Free_Callback hash_free; /**< free all entries from the hash @p h */
     Eet_Descriptor_Str_Direct_Alloc_Callback str_direct_alloc; /**< how to allocate a string directly from file backed/mmaped region pointed by @p str */
     Eet_Descriptor_Str_Direct_Free_Callback str_direct_free; /**< how to free a string returned by str_direct_alloc */
     Eet_Descriptor_Type_Get_Callback type_get; /**< convert any kind of data type to a name that define an Eet_Data_Element. */
     Eet_Descriptor_Type_Set_Callback type_set; /**< set the type at a particular address */
     Eet_Descriptor_Array_Alloc_Callback array_alloc; /**< how to allocate memory for array (usually malloc()) */
     Eet_Descriptor_Array_Free_Callback array_free; /**< how to free memory for array (usually free()) */
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
 * This function creates a new data descriptore and returns a handle to the
 * new data descriptor. On creation it will be empty, containing no contents
 * describing anything other than the shell of the data structure.
 * @param eddc The data descriptor to free.
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
 * This function specially ignore str_direct_alloc and str_direct_free. It
 * is useful when the eet_data you are reading don't have a dictionnary
 * like network stream or ipc. It also mean that all string will be allocated
 * and duplicated in memory.
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eet_Data_Descriptor *
eet_data_descriptor_stream_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * This function creates a new data descriptore and returns a handle to the
 * new data descriptor. On creation it will be empty, containing no contents
 * describing anything other than the shell of the data structure.
 * @param eddc The data descriptor to free.
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
 * This function use str_direct_alloc and str_direct_free. It is
 * useful when the eet_data you are reading come from a file and
 * have a dictionnary. This will reduce memory use, improve the
 * possibility for the OS to page this string out. But be carrefull
 * all EET_T_STRING are pointer to a mmapped area and it will point
 * to nowhere if you close the file. So as long as you use this
 * strings, you need to have the Eet_File open.
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eet_Data_Descriptor *
eet_data_descriptor_file_new(const Eet_Data_Descriptor_Class *eddc);

/**
 * This function is an helper that set all the parameter of an
 * Eet_Data_Descriptor_Class correctly when you use Eina data type
 * with a stream.
 * @param eddc The Eet_Data_Descriptor_Class you want to set.
 * @param name The name of the structure described by this class.
 * @param eddc_size The size of the Eet_Data_Descriptor_Class at the compilation time.
 * @param size The size of the structure described by this class.
 * @return EINA_TRUE if the structure was correctly set (The only
 *         reason that could make it fail is if you did give wrong
 *         parameter).
 *
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eina_Bool
eet_eina_stream_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
                                          unsigned int		     eddc_size,
                                          const char                *name,
                                          int                        size);

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
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
#define EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(clas, type)\
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
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
EAPI Eina_Bool
eet_eina_file_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
                                        unsigned int               eddc_size,
                                        const char                *name,
                                        int                        size);

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
 * @since 1.2.3
 * @ingroup Eet_Data_Group
 */
#define EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(clas, type)\
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
 *        group_type will speficy if it is a list (#EET_G_LIST),
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
                                const char          *name,
                                int                  type,
                                int                  group_type,
                                int                  offset,
                                /* int                  count_offset, */
                                int                  count,
                                const char          *counter_name,
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
eet_data_read(Eet_File            *ef,
              Eet_Data_Descriptor *edd,
              const char          *name);

/**
 * Write a data structure from memory and store in an eet file.
 * @param ef The eet file handle to write to.
 * @param edd The data descriptor to use when encoding.
 * @param name The key to store the data under in the eet file.
 * @param data A pointer to the data structure to ssave and encode.
 * @param compress Compression flags for storage.
 * @return bytes written on successful write, 0 on failure.
 *
 * This function is the reverse of eet_data_read(), saving a data structure
 * to an eet file.
 *
 * @see eet_data_write_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI int
eet_data_write(Eet_File            *ef,
               Eet_Data_Descriptor *edd,
               const char          *name,
               const void          *data,
               int                  compress);

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
eet_data_text_dump(const void  *data_in,
                   int          size_in,
                   Eet_Dump_Callback dumpfunc,
                   void        *dumpdata);

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
                     int         textlen,
                     int        *size_ret);

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
eet_data_dump(Eet_File    *ef,
              const char  *name,
              Eet_Dump_Callback dumpfunc,
              void        *dumpdata);

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
eet_data_undump(Eet_File   *ef,
                const char *name,
                const char *text,
                int         textlen,
                int         compress);

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
                           const void          *data_in,
                           int                  size_in);

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
 * @see eet_data_descriptor_encode_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
EAPI void *
eet_data_descriptor_encode(Eet_Data_Descriptor *edd,
                           const void          *data_in,
                           int                 *size_ret);

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
 * the actual struct member itself (for eet_dictionary_string_check
 * example: values), and @p type is the basic data type of the
 * member which must be one of: EET_T_CHAR, EET_T_SHORT, EET_T_INT,
 * EET_T_LONG_LONG, EET_T_FLOAT, EET_T_DOUBLE, EET_T_UCHAR,
 * EET_T_USHORT, EET_T_UINT, EET_T_ULONG_LONG or EET_T_STRING.
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_BASIC(edd, struct_type, name, member, type) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, type, EET_G_UNKNOWN, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      0, /* 0,  */ NULL, NULL); \
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
#define EET_DATA_DESCRIPTOR_ADD_SUB(edd, struct_type, name, member, subtype) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNKNOWN, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      0, /* 0,  */ NULL, subtype); \
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
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_LIST, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      0, /* 0,  */ NULL, subtype); \
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
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_STRING, EET_G_LIST, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      0, /* 0,  */ NULL, NULL); \
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
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_HASH(edd, struct_type, name, member, subtype) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_HASH, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      0, /* 0,  */ NULL, subtype); \
   } while (0)

/**
 * Add a hash of string to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 *
 * This macro lets you easily add a hash of string. All the
 * parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC().
 *
 * @since 1.3.4
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_HASH_STRING(edd, struct_type, name, member) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_STRING, EET_G_HASH, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      0, /* 0,  */ NULL, NULL); \
   } while (0)

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
 * is in each member of the hash to be stored.
 *
 * @since 1.0.2
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_ARRAY(edd, struct_type, name, member, subtype) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_ARRAY, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      /* 0,  */ sizeof(___ett.member) / \
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
 * This macro lets you easily add a fixed size array of other data
 * types. All the parameters are the same as for
 * EET_DATA_DESCRIPTOR_ADD_BASIC(), with the @p subtype being the
 * exception. This must be the data descriptor of the element that
 * is in each member of the hash to be stored. This assumes you have
 * a struct member (of type EET_T_INT) called member_count (note the
 * _count appended to the member) that holds the number of items in
 * the array. This array will be allocated separately to the struct it
 * is in.
 *
 * @since 1.0.2
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(edd, struct_type, name, member, subtype) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, \
                                      name, \
                                      EET_T_UNKNOW, \
                                      EET_G_VAR_ARRAY, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      (char *)(& (___ett.member ## _count)) - \
                                      (char *)(& (___ett)), \
                                      /* 0,  */ NULL, \
                                      subtype); \
   } while (0)

/**
 * Add a variable size array type to a data descriptor
 * @param edd The data descriptor to add the type to.
 * @param struct_type The type of the struct.
 * @param name The string name to use to encode/decode this member
 *        (must be a constant global and never change).
 * @param member The struct member itself to be encoded.
 *
 * This macro lets you easily add a fixed size array of string. All
 * the parameters are the same as for EET_DATA_DESCRIPTOR_ADD_BASIC().
 *
 * @since 1.4.0
 * @ingroup Eet_Data_Group
 */
#define EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING(edd, struct_type, name, member) \
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, \
                                      name, \
                                      EET_T_STRING, \
                                      EET_G_VAR_ARRAY, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      (char *)(& (___ett.member ## _count)) - \
                                      (char *)(& (___ett)), \
                                      /* 0,  */ NULL, \
                                      NULL); \
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
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_UNION, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      (char *)(& (___ett.type_member)) - \
                                      (char *)(& (___ett)), \
                                      NULL, unified_type); \
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
   do { \
      struct_type ___ett; \
      eet_data_descriptor_element_add(edd, name, EET_T_UNKNOW, EET_G_VARIANT, \
                                      (char *)(& (___ett.member)) - \
                                      (char *)(& (___ett)), \
                                      (char *)(& (___ett.type_member)) - \
                                      (char *)(& (___ett)), \
                                      NULL, unified_type); \
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
   eet_data_descriptor_element_add(unified_type, \
                                   name, \
                                   EET_T_UNKNOW, \
                                   EET_G_UNKNOWN, \
                                   0, \
                                   0, \
                                   NULL, \
                                   subtype)

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
eet_data_read_cipher(Eet_File            *ef,
                     Eet_Data_Descriptor *edd,
                     const char          *name,
                     const char          *cipher_key);

/**
 * Write a data structure from memory and store in an eet file
 * using a cipher.
 * @param ef The eet file handle to write to.
 * @param edd The data descriptor to use when encoding.
 * @param name The key to store the data under in the eet file.
 * @param cipher_key The key to use as cipher.
 * @param data A pointer to the data structure to ssave and encode.
 * @param compress Compression flags for storage.
 * @return bytes written on successful write, 0 on failure.
 *
 * This function is the reverse of eet_data_read(), saving a data structure
 * to an eet file.
 *
 * @see eet_data_write_cipher()
 *
 * @since 1.0.0
 * @ingroup Eet_Data_Cipher_Group
 */
EAPI int
eet_data_write_cipher(Eet_File            *ef,
                      Eet_Data_Descriptor *edd,
                      const char          *name,
                      const char          *cipher_key,
                      const void          *data,
                      int                  compress);

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
                            int         textlen,
                            int        *size_ret);

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
eet_data_dump_cipher(Eet_File    *ef,
                     const char  *name,
                     const char  *cipher_key,
                     Eet_Dump_Callback dumpfunc,
                     void        *dumpdata);

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
eet_data_undump_cipher(Eet_File   *ef,
                       const char *name,
                       const char *cipher_key,
                       const char *text,
                       int         textlen,
                       int         compress);

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
                                  const void          *data_in,
                                  const char          *cipher_key,
                                  int                  size_in);

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
                                  const void          *data_in,
                                  const char          *cipher_key,
                                  int                 *size_ret);

/**
 * @defgroup Eet_Node_Group Low-level Serialization Structures.
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
typedef struct _Eet_Node        Eet_Node;

/**
 * @typedef Eet_Node_Data
 * Contains an union that can fit any kind of node.
 */
typedef struct _Eet_Node_Data   Eet_Node_Data;

/**
 * @struct _Eet_Node_Data
 * Contains an union that can fit any kind of node.
 */
struct _Eet_Node_Data
{
   union {
      char                c;
      short               s;
      int                 i;
      long long           l;
      float               f;
      double              d;
      unsigned char       uc;
      unsigned short      us;
      unsigned int        ui;
      unsigned long long  ul;
      const char         *str;
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
                   char        c);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_short_new(const char *name,
                   short       s);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_int_new(const char *name,
                 int         i);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_long_long_new(const char *name,
                       long long   l);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_float_new(const char *name,
                   float       f);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_double_new(const char *name,
                    double      d);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_char_new(const char    *name,
                           unsigned char  uc);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_short_new(const char     *name,
                            unsigned short  us);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_int_new(const char   *name,
                          unsigned int  ui);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_unsigned_long_long_new(const char         *name,
                                unsigned long long  l);

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
                  Eina_List  *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_array_new(const char *name,
                   int         count,
                   Eina_List  *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_var_array_new(const char *name,
                       Eina_List  *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_hash_new(const char *name,
                  const char *key,
                  Eet_Node   *node);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_struct_new(const char *name,
                    Eina_List  *nodes);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_node_struct_child_new(const char *parent,
                          Eet_Node   *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_list_append(Eet_Node   *parent,
                     const char *name,
                     Eet_Node   *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_struct_append(Eet_Node   *parent,
                       const char *name,
                       Eet_Node   *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_hash_add(Eet_Node   *parent,
                  const char *name,
                  const char *key,
                  Eet_Node   *child);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI void
eet_node_dump(Eet_Node  *n,
              int        dumplevel,
              Eet_Dump_Callback dumpfunc,
              void      *dumpdata);

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
eet_data_node_encode_cipher(Eet_Node   *node,
                            const char *cipher_key,
                            int        *size_ret);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_data_node_decode_cipher(const void *data_in,
                            const char *cipher_key,
                            int         size_in);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI Eet_Node *
eet_data_node_read_cipher(Eet_File   *ef,
                          const char *name,
                          const char *cipher_key);

/**
 * TODO FIX ME
 * @ingroup Eet_Node_Group
 */
EAPI int
eet_data_node_write_cipher(Eet_File   *ef,
                           const char *name,
                           const char *cipher_key,
                           Eet_Node   *node,
                           int         compress);

/* EXPERIMENTAL: THIS API MAY CHANGE IN THE FUTURE, USE IT ONLY IF YOU KNOW WHAT YOU ARE DOING. */

/**
 * @typedef Eet_Node_Walk
 * Describes how to walk trees of #Eet_Node.
 */
typedef struct _Eet_Node_Walk   Eet_Node_Walk;

typedef void *(*Eet_Node_Walk_Struct_Alloc_Callback)(const char *type, void *user_data);
typedef void  (*Eet_Node_Walk_Struct_Add_Callback)(void *parent, const char *name, void *child, void *user_data);
typedef void *(*Eet_Node_Walk_Array_Callback)(Eina_Bool variable, const char *name, int count, void *user_data);
typedef void  (*Eet_Node_Walk_Insert_Callback)(void *array, int index, void *child, void *user_data);
typedef void *(*Eet_Node_Walk_List_Callback)(const char *name, void *user_data);
typedef void  (*Eet_Node_Walk_Append_Callback)(void *list, void *child, void *user_data);
typedef void *(*Eet_Node_Walk_Hash_Callback)(void *parent, const char *name, const char *key, void *value, void *user_data);
typedef void *(*Eet_Node_Walk_Simple_Callback)(int type, Eet_Node_Data *data, void *user_data);
  
/**
 * @struct _Eet_Node_Walk
 * Describes how to walk trees of #Eet_Node.
 */
struct _Eet_Node_Walk
{
   Eet_Node_Walk_Struct_Alloc_Callback struct_alloc;
   Eet_Node_Walk_Struct_Add_Callback struct_add;
   Eet_Node_Walk_Array_Callback array;
   Eet_Node_Walk_Insert_Callback insert;
   Eet_Node_Walk_List_Callback list;
   Eet_Node_Walk_Append_Callback append;
   Eet_Node_Walk_Hash_Callback hash;
   Eet_Node_Walk_Simple_Callback simple;
};

EAPI void *
eet_node_walk(void          *parent,
              const char    *name,
              Eet_Node      *root,
              Eet_Node_Walk *cb,
              void          *user_data);

/*******/

/**
 * @defgroup Eet_Connection_Group Helper function to use eet over a network link
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
typedef struct _Eet_Connection   Eet_Connection;

/**
 * @typedef Eet_Read_Cb
 * Called back when an @ref Eet_Data_Group has been received completly and could be used.
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
eet_connection_new(Eet_Read_Cb  *eet_read_cb,
                   Eet_Write_Cb *eet_write_cb,
                   const void   *user_data);

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
                        const void     *data,
                        size_t          size);

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
eet_connection_send(Eet_Connection      *conn,
                    Eet_Data_Descriptor *edd,
                    const void          *data_in,
                    const char          *cipher_key);

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
                         Eet_Node       *node,
                         const char     *cipher_key);

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
                     Eina_Bool      *on_going);

/***************************************************************************/

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef _EET_H */
