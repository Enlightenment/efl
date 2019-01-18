/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_FILE_COMMON_H_
# define EINA_FILE_COMMON_H_

#include "eina_file.h"
#include "eina_tmpstr.h"
#include "eina_lock.h"
#include "eina_list.h"
#include "eina_util.h"

#define EINA_FILE_MAGIC 0xFEEDBEEF

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */
/**
 * @addtogroup Eina_File_Group File
 *
 * @brief Functions to handle files and directories.
 *
 * This module performs internal housekeeping and utility tasks for Eina_File.
 * It also provides the underlying data types for things like file handles, file
 * maps and file iterators.
 *
 * @{
 */

/**
 * @typedef Eina_File_Map
 *
 * Type definition for an Eina File Map.
 *
 */
typedef struct _Eina_File_Map Eina_File_Map;

/**
 * @typedef Eina_Lines_Iterator
 *
 * Type definition for an Eina Lines Iterator.
 *
 */
typedef struct _Eina_Lines_Iterator Eina_Lines_Iterator;

/**
 * @struct _Eina_File
 *
 * This is the underlying data structure that represents a file in Eina.
 *
 */
struct _Eina_File
{
   EINA_MAGIC;            /**< Indicates whether Eina Magic should be used. */
   const char *filename;  /**< The absolute path of the file. Note that the path given when calling @ref eina_file_open will be run through @ref eina_file_path_sanitize before it is stored here. */ 
   Eina_Hash *map;        /**< Tracks portions of a file that have been mapped with mmap(2).  The key is a tuple offset/length and the data is a pointer to the mapped region. */
   Eina_Hash *rmap;       /**< Similar function to #map, but used to look up mapped areas by pointer rather than offset/length. */
   void *global_map;      /**< A pointer to the entire contents of the file that have been mapped with mmap(2).  This is the common case, and EFL and is optimized for it. */
   Eina_Lock lock;        /**< A file locking mechanism. */

#ifndef _WIN32
   unsigned long long length;  /**< The length of the file in bytes. */
   time_t mtime;               /**< The last modified time. */
   ino_t inode;                /**< The inode. */
#ifdef _STAT_VER_LINUX
   unsigned long int mtime_nsec; /**< The nano version of the last modified time. */
#endif
#else
   ULONGLONG length;  /**< The length of the file in bytes. */
   ULONGLONG mtime;   /**< The last modified time. */
#endif

   int refcount;        /**< Keeps track of references to #map. */
   int global_refcount; /**< Keeps track of references to #global_map. */

#ifndef _WIN32
   int fd; /**< The file descriptor. */
#else
   HANDLE handle;  /**< A Win32 file handle for this file. */
#endif

   Eina_List *dead_map;          /**< Tracks regions that get a failure from mmap(2). */

   Eina_Bool shared : 1;         /**< Indicates whether this file can be shared */
   Eina_Bool delete_me : 1;      /**< Indicates that this file should be deleted */
   Eina_Bool global_faulty : 1;  /**< Indicates whether #global_map is bad */
   Eina_Bool global_hugetlb : 1; /**< Indicates whether #global_map uses HugeTLB */
   Eina_Bool virtual : 1;        /**< Indicates that this is a virtual file */
   Eina_Bool copied : 1;         /**< Indicates whether this file has copied data */
};

/**
 * @struct _Eina_File_Map
 *
 * This represents a memory mapped region of a file.
 *
 */
struct _Eina_File_Map
{
   void *map;  /**< A pointer to the mapped region */

   unsigned long int offset;  /**< The offset in the file */
   unsigned long int length;  /**< The length of the region */

   int refcount;  /**< Tracks references to this region */

   Eina_Bool hugetlb : 1;  /**< Indicates if we are using HugeTLB */
   Eina_Bool faulty : 1;   /**< Indicates if this region was not mapped correctly (i.e. the call to mmap(2) failed). */
};

/**
 * @struct _Eina_Lines_Iterator
 *
 * This represents a line iterator a file.
 *
 */
struct _Eina_Lines_Iterator
{
   Eina_Iterator iterator;  /**< The iterator itself */

   Eina_File *fp;   /**< The file this iterator is associated with */
   const char *map; /**< A pointer to the head of the file that has been mapped with mmap(2). */
   const char *end; /**< A pointer to the end of the file that has been mapped with mmap(2). */

   int boundary;    /**< Currently hard coded to 4096. */

   Eina_File_Line current;  /**< The current line */
};


#ifndef EINA_LOG_COLOR_DEFAULT
/** Set the color for Eina log entries */
#define EINA_LOG_COLOR_DEFAULT EINA_COLOR_CYAN
#endif

#ifdef ERR
#undef ERR
#endif
/** Macro for logging Eina errors */
#define ERR(...) EINA_LOG_DOM_ERR(_eina_file_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
/** Macro for logging Eina warnings */
#define WRN(...) EINA_LOG_DOM_WARN(_eina_file_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
/** Macro for logging Eina info messages */
#define INF(...) EINA_LOG_DOM_INFO(_eina_file_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
/** Macro for logging Eina debug messages */
#define DBG(...) EINA_LOG_DOM_DBG(_eina_file_log_dom, __VA_ARGS__)

/**
 * @brief Determines if a path is relative or absolute.
 * The implementation simply checks if the first char in the path is '/'.  If it
 * is not, the path is considered relative.
 *
 * @param[in] path The path to check.
 *
 * @return EINA_TRUE if the path is relative, EINA_FALSE otherwise.
 *
 */
Eina_Bool eina_file_path_relative(const char *path);

/**
 * @brief Gets the current directory and optionally appends a path to it.
 * If a string was passed in via the @p path parameter, it will
 * be appended to the current working directory.  Presumably, this will be a
 * relative path.
 *
 * @param[in] path The path to append to the current directory.
 * @param[in] len The length of @p path.
 *
 * @return A pointer to a string that contains the absolute path to the current
 * working directory plus any path you send in.
 *
 */
Eina_Tmpstr *eina_file_current_directory_get(const char *path, size_t len);

/**
 * @brief Cleans up Eina after a file is no longer needed.
 *
 * @param[in,out] path The path of the file.
 *
 * @return On success, it will return the @p path string.  If @p path is @c NULL,
 * it will return and empty string.
 *
 */
char *eina_file_cleanup(Eina_Tmpstr *path);

/**
 * @brief Closes and cleans up after an Eina file.
 *
 * @param[in,out] file The path of the file.
 *
 */
void eina_file_clean_close(Eina_File *file);

/**
 * @brief Closes a file from the OS perspective.
 *
 * @param[in,out] file The path of the file.
 *
 */
void eina_file_real_close(Eina_File *file);

/**
 * @brief Resets the internal housekeeping structures after a file has changed.
 * Despite the name, this routine does not write anything to disk.  It invalidates
 * the memory maps for the file.  If the file has shrunk, it also adds any mapped
 * regions past the end of the file to the dead_map.
 *
 * @param[in,out] file The file.
 * @param[in] length The current length of the file after the change.
 *
 */
void eina_file_flush(Eina_File *file, unsigned long int length);

/**
 * @brief Removes a mapped region from the file and frees the resources.
 * This routine will remove a previously mapped region from the internal Eina File
 * housekeeping and free the resources associated with it.  In the case where
 * the map is part of the dead_map, @p free_func will be called to handle the actual
 * deallocation.
 *
 * @param[in,out] file The file.
 * @param[in,out] map The memory mapped region that is to be freed.
 * @param[in] free_func A pointer to a function that will be called to
 *            free up the resources used by the map.
 *
 */
void eina_file_common_map_free(Eina_File *file, void *map,
                               void (*free_func)(Eina_File_Map *map));

/** A pointer to the global Eina file cache. */
extern Eina_Hash *_eina_file_cache;

/** The global file lock cache */
extern Eina_Lock _eina_file_lock_cache;

/** The global Eina log file domain. */
extern int _eina_file_log_dom;

// Common function to handle virtual file
/**
 * @brief Map the entire contents of a virtual file to a buffer.
 *
 * @param[in] file The virtual file to map in memory
 * @return The buffer
 */
void *eina_file_virtual_map_all(Eina_File *file);

/**
 * @brief Map a part of a virtual file to a buffer.
 *
 * @param[in,out] file The virtual file to map in memory
 * @param[in] offset The offset inside the file to start mapping
 * @param[in] length The length of the region to map
 * @return The buffer
 */
void *eina_file_virtual_map_new(Eina_File *file,
                                unsigned long int offset, unsigned long int length);

/**
 * @brief Unref and unmap memory if possible.
 *
 * @param[in,out] file The file handler to unmap memory from.
 * @param[in,out] map Memory map to unref and unmap.
 *
 */
void eina_file_virtual_map_free(Eina_File *file, void *map);

// Common hash function
/**
 * @brief Get the length of a map key.
 * @warning This function is not yet implemented.  At present it only returns
 * @code sizeof (unsigned long int) * 2 @endcode
 *
 * @param[in] key The key for which length will be calculated.
 *
 * @return The length of the key.
 *
 */
unsigned int eina_file_map_key_length(const void *key);

/**
 * @brief Compares two map keys.
 * The implementation assumes that @p key1 and @p key2 are both pointers to an
 * array with 2 elements, as is the case with the Eina file map keys.
 *
 * @param[in] key1 The first key.
 * @param[in] key1_length The length of the first key.
 * @param[in] key2 The second key.
 * @param[in] key2_length The length of the second key.
 *
 * @return Positive number if Key1 > Key2, else a negative number.  Will return
 * zero if both elements of the key are exactly the same.
 *
 */
int eina_file_map_key_cmp(const unsigned long long int *key1, int key1_length,
                          const unsigned long long int *key2, int key2_length);

/**
 * @brief Creates a hash from a map key.
 *
 * @param[in] key A pointer to the key.
 * @param[in] key_length The length of the key.
 *
 * @return A key hash.
 *
 */
int eina_file_map_key_hash(const unsigned long long int *key, int key_length);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_FILE_COMMON_H_ */
