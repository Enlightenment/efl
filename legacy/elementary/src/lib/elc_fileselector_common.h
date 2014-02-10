/**
 * Defines how a file selector widget is to layout its contents
 * (file system entries).
 */
typedef enum
{
   ELM_FILESELECTOR_LIST = 0, /**< layout as a list */
   ELM_FILESELECTOR_GRID, /**< layout as a grid */
   ELM_FILESELECTOR_LAST /**< sentinel (helper) value, not used */
} Elm_Fileselector_Mode;

typedef enum
{
   ELM_FILESELECTOR_SORT_BY_FILENAME_ASC = 0,
   ELM_FILESELECTOR_SORT_BY_FILENAME_DESC,
   ELM_FILESELECTOR_SORT_BY_TYPE_ASC,
   ELM_FILESELECTOR_SORT_BY_TYPE_DESC,
   ELM_FILESELECTOR_SORT_BY_SIZE_ASC,
   ELM_FILESELECTOR_SORT_BY_SIZE_DESC,
   ELM_FILESELECTOR_SORT_BY_MODIFIED_ASC,
   ELM_FILESELECTOR_SORT_BY_MODIFIED_DESC,
   ELM_FILESELECTOR_SORT_LAST /**< sentinel (helper) value, not used */
} Elm_Fileselector_Sort;

typedef Eina_Bool (*Elm_Fileselector_Filter_Func)(const char *path, /**< File path */
                                                  Eina_Bool dir, /**< A flag to show if path is a directory or not. True if the path is a directory. */
                                                  void *data /**< A user data that was given by elm_fileselector_custom_filter_append. */);
