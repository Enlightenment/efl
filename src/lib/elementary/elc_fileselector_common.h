typedef Eina_Bool (*Elm_Fileselector_Filter_Func)(const char *path, /**< File path */
                                                  Eina_Bool dir, /**< A flag to show if path is a directory or not. True if the path is a directory. */
                                                  void *data /**< A user data that was given by elm_fileselector_custom_filter_append. */);

EAPI extern Eina_Error ELM_FILESELECTOR_ERROR_UNKNOWN;
EAPI extern Eina_Error ELM_FILESELECTOR_ERROR_INVALID_MODEL;
