#ifndef _ELM_INTERFACE_FILESELECTOR_EO_LEGACY_H_
#define _ELM_INTERFACE_FILESELECTOR_EO_LEGACY_H_

#ifndef _ELM_INTERFACE_FILESELECTOR_EO_CLASS_TYPE
#define _ELM_INTERFACE_FILESELECTOR_EO_CLASS_TYPE

typedef Eo Elm_Interface_Fileselector;

#endif

#ifndef _ELM_INTERFACE_FILESELECTOR_EO_TYPES
#define _ELM_INTERFACE_FILESELECTOR_EO_TYPES

/** Defines how a file selector widget is to layout its contents (file system
 * entries).
 *
 * @ingroup Elm_Fileselector
 */
typedef enum
{
  ELM_FILESELECTOR_LIST = 0, /**< Layout as a list. */
  ELM_FILESELECTOR_GRID, /**< Layout as a grid. */
  ELM_FILESELECTOR_LAST /**< Sentinel value to indicate last enum field during
                         * iteration */
} Elm_Fileselector_Mode;

/** Fileselector sorting modes
 *
 * @ingroup Elm_Fileselector
 */
typedef enum
{
  ELM_FILESELECTOR_SORT_BY_FILENAME_ASC = 0, /**< Alphabetical sort by ascending
                                              * filename, default */
  ELM_FILESELECTOR_SORT_BY_FILENAME_DESC, /**< Alphabetical sorting by
                                           * descending filename */
  ELM_FILESELECTOR_SORT_BY_TYPE_ASC, /**< Sort by file type */
  ELM_FILESELECTOR_SORT_BY_TYPE_DESC, /**< Sort by file type description */
  ELM_FILESELECTOR_SORT_BY_SIZE_ASC, /**< Sort by ascending file size */
  ELM_FILESELECTOR_SORT_BY_SIZE_DESC, /**< Sort by descending file size */
  ELM_FILESELECTOR_SORT_BY_MODIFIED_ASC, /**< Sort by ascending modified date */
  ELM_FILESELECTOR_SORT_BY_MODIFIED_DESC, /**< Sort by descending modified date
                                           */
  ELM_FILESELECTOR_SORT_LAST /**< Sentinel value to indicate last enum field
                              * during iteration */
} Elm_Fileselector_Sort;


#endif























#endif
