/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

#define DESKTOP_VERSION 1.0

/**
 * The current desktop environment (e.g. "Enlightenment" or "Gnome")
 */
static const char *desktop_environment = NULL;

/**
 * A cache of all loaded desktops, hashed by file name. 
 * Values are Efreet_Desktop structures
 */
static Ecore_Hash *efreet_desktop_cache = NULL;

/**
 * A list of the desktop types available
 */
static Ecore_List *efreet_desktop_types = NULL;

/**
 * A unique id for each tmp file created while building a command
 */
static int efreet_desktop_command_file_id = 0;

static int init = 0;

int EFREET_DESKTOP_TYPE_APPLICATION = 0;
int EFREET_DESKTOP_TYPE_LINK = 0;
int EFREET_DESKTOP_TYPE_DIRECTORY = 0;

/**
 * @internal
 * Information about custom types
 */
typedef struct Efreet_Desktop_Type_Info Efreet_Desktop_Type_Info;
struct Efreet_Desktop_Type_Info
{
    int id;
    char *type;
    Efreet_Desktop_Type_Parse_Cb parse_func;
    Efreet_Desktop_Type_Save_Cb save_func;
    Efreet_Desktop_Type_Free_Cb free_func;
};

static Efreet_Desktop *efreet_desktop_new(const char *file);
static Efreet_Desktop_Type_Info *efreet_desktop_type_parse(const char *type_str);
static Ecore_List *efreet_desktop_string_list_parse(const char *string);
static char *efreet_desktop_string_list_join(Ecore_List *list);
static void *efreet_desktop_application_fields_parse(Efreet_Desktop *desktop, 
                                                    Efreet_Ini *ini);
static void efreet_desktop_application_fields_save(Efreet_Desktop *desktop, 
                                                    Efreet_Ini *ini);
static void *efreet_desktop_link_fields_parse(Efreet_Desktop *desktop,
                                                Efreet_Ini *ini);
static void efreet_desktop_link_fields_save(Efreet_Desktop *desktop, 
                                                Efreet_Ini *ini);
static int efreet_desktop_generic_fields_parse(Efreet_Desktop *desktop, 
                                                Efreet_Ini *ini);
static void efreet_desktop_generic_fields_save(Efreet_Desktop *desktop, 
                                                Efreet_Ini *ini);
static void efreet_desktop_x_fields_parse(Ecore_Hash_Node *node, 
                                            Efreet_Desktop *desktop);
static int efreet_desktop_environment_check(Efreet_Ini *ini);
static char *efreet_string_append(char *dest, int *size, 
                                    int *len, const char *src);
static char *efreet_string_append_char(char *dest, int *size, 
                                        int *len, char c);
static void  efreet_desktop_command_build(Efreet_Desktop_Command *command);
static void efreet_desktop_command_free(Efreet_Desktop_Command *command);
static char *efreet_desktop_command_append_quoted(char *dest, int *size, 
                                                    int *len, char *src);
static char *efreet_desktop_command_append_icon(char *dest, int *size, int *len,
                                                Efreet_Desktop *desktop);
static char *efreet_desktop_command_append_single(char *dest, int *size, int *len, 
                                                Efreet_Desktop_Command_File *file, 
                                                char type);
static char *efreet_desktop_command_append_multiple(char *dest, int *size, int *len, 
                                                    Efreet_Desktop_Command *command, 
                                                    char type);

static char *efreet_desktop_command_path_absolute(const char *path);
static Efreet_Desktop_Command_File *efreet_desktop_command_file_process(
                                                    Efreet_Desktop_Command *command, 
                                                    const char *file);
static const char *efreet_desktop_command_file_uri_process(const char *uri);
static void efreet_desktop_command_file_free(Efreet_Desktop_Command_File *file);

static void efreet_desktop_cb_download_complete(void *data, const char *file, 
                                                                int status);
static int efreet_desktop_cb_download_progress(void *data, const char *file,
                                           long int dltotal, long int dlnow,
                                           long int ultotal, long int ulnow);


static void efreet_desktop_exec_cb(void *data, Efreet_Desktop *desktop, 
                                            char *exec, int remaining);

static void efreet_desktop_type_info_free(Efreet_Desktop_Type_Info *info);

/**
 * @internal
 * @return Returns > 0 on success or 0 on failure
 * @brief Initialize the Desktop parser subsystem
 */
int
efreet_desktop_init(void)
{
    if (init++) return init;
    if (!ecore_string_init()) return --init;
    if (!ecore_file_init()) return --init;

    efreet_desktop_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_set_free_key(efreet_desktop_cache, ECORE_FREE_CB(free));
    ecore_hash_set_free_value(efreet_desktop_cache, 
                                ECORE_FREE_CB(efreet_desktop_free));

    efreet_desktop_types = ecore_list_new();
    ecore_list_set_free_cb(efreet_desktop_types, 
                            ECORE_FREE_CB(efreet_desktop_type_info_free));

    EFREET_DESKTOP_TYPE_APPLICATION = efreet_desktop_type_add("Application", 
                                        efreet_desktop_application_fields_parse, 
                                        efreet_desktop_application_fields_save, 
                                        NULL);
    EFREET_DESKTOP_TYPE_LINK = efreet_desktop_type_add("Link", 
                                    efreet_desktop_link_fields_parse, 
                                    efreet_desktop_link_fields_save, NULL);
    EFREET_DESKTOP_TYPE_DIRECTORY = efreet_desktop_type_add("Directory", NULL, 
                                                                NULL, NULL);

    return init;
}

/**
 * @internal
 * @returns the number of initializations left for this system
 * @brief Attempts to shut down the subsystem if nothing else is using it
 */
int
efreet_desktop_shutdown(void)
{
    if (--init) return init;
    ecore_file_shutdown();
    ecore_string_shutdown();

    IF_RELEASE(desktop_environment);
    IF_FREE_HASH(efreet_desktop_cache);
    IF_FREE_LIST(efreet_desktop_types);

    return init;
}

/**
 * @internal
 * @param desktop: The desktop to check
 * @return Returns 1 if the cache is still valid, 0 otherwise
 * @brief This will check if the desktop cache is still valid. 
 */
static int
efreet_desktop_cache_check(Efreet_Desktop *desktop)
{
    struct stat buf;

    if (!desktop) return 0;

    /* have we modified this file since we last read it in? */
    if (stat(desktop->orig_path, &buf) || (buf.st_mtime > desktop->load_time))
        return 0;

    return 1;
}

/**
 * @param file: The file to get the Efreet_Desktop from
 * @return Returns a reference to a cached Efreet_Desktop on success, NULL 
 * on failure. This reference should not be freed.
 * @brief Gets a reference to an Efreet_Desktop structure representing the
 * contents of @a file or NULL if @a file is not a valid .desktop file.
 */
Efreet_Desktop *
efreet_desktop_get(const char *file)
{
    Efreet_Desktop *desktop;

    if (efreet_desktop_cache) 
    {
        desktop = ecore_hash_get(efreet_desktop_cache, file);
        if (desktop)
        {
            if (efreet_desktop_cache_check(desktop))
                return desktop;

            ecore_hash_remove(efreet_desktop_cache, file);
            efreet_desktop_free(desktop);
        }
    }

    desktop = efreet_desktop_new(file);
    if (!desktop) return NULL;

    ecore_hash_set(efreet_desktop_cache, strdup(file), desktop);
    return desktop;

}

/**
 * @param file: The file to create the Efreet_Desktop from
 * @return Returns a new empty_Efreet_Desktop on success, NULL on failure
 * @brief Creates a new empty Efreet_Desktop structure or NULL on failure
 */
Efreet_Desktop *
efreet_desktop_empty_new(const char *file)
{
    Efreet_Desktop *desktop;

    desktop = NEW(Efreet_Desktop, 1);
    if (!desktop) return NULL;

    desktop->orig_path = strdup(file);
    desktop->load_time = ecore_time_get();
    return desktop;
}

/**
 * @internal
 * @param file: The file to create the Efreet_Desktop from
 * @return Returns a new Efreet_Desktop on success, NULL on failure
 * @brief Creates a new Efreet_Desktop structure initialized from the
 * contents of @a file or NULL on failure
 */
static Efreet_Desktop *
efreet_desktop_new(const char *file)
{
    Efreet_Desktop *desktop;
    Efreet_Ini *ini;
    int error = 0;
    int ok;

    desktop = NEW(Efreet_Desktop, 1);
    if (!desktop) return NULL;

    desktop->orig_path = strdup(file);
    desktop->load_time = ecore_time_get();

    ini = efreet_ini_new(file);
    if (!ini->data) 
    {
        efreet_ini_free(ini);
        IF_FREE(desktop->orig_path);
        free(desktop);
        return NULL;
    }

    ok = efreet_ini_section_set(ini, "Desktop Entry");
    if (!ok) ok = efreet_ini_section_set(ini, "KDE Desktop Entry");
    if (!ok) 
    {
        printf("efreet_desktop_new error: no Desktop Entry section\n");
        error = 1;
    }

    if (!error)
    {
        Efreet_Desktop_Type_Info *info;

        info = efreet_desktop_type_parse(efreet_ini_string_get(ini, "Type"));
        if (info)
        {
            desktop->type = info->id;
            desktop->version = efreet_ini_double_get(ini, "Version");

            if (info->parse_func)
                desktop->type_data = info->parse_func(desktop, ini); 
        }
        else
            error = 1;
    }

    if (!error && !efreet_desktop_environment_check(ini)) error = 1;
    if (!error && !efreet_desktop_generic_fields_parse(desktop, ini)) error = 1;
    if (!error)
        ecore_hash_for_each_node(ini->section,
                ECORE_FOR_EACH(efreet_desktop_x_fields_parse), desktop);

    efreet_ini_free(ini);

    if (!error) 
      return desktop;

    efreet_desktop_free(desktop);
    return NULL;
}

/**
 * @param desktop: The desktop file to save
 * @return Returns 1 on success or 0 on failure
 * @brief Saves any changes made to @a desktop back to the file on the
 * filesystem
 */
int
efreet_desktop_save(Efreet_Desktop *desktop)
{
    Efreet_Desktop_Type_Info *info;
    Efreet_Ini *ini;
    int ok = 1;

    ini = efreet_ini_new(desktop->orig_path);
    efreet_ini_section_add(ini, "Desktop Entry");
    efreet_ini_section_set(ini, "Desktop Entry");

    info = ecore_list_goto_index(efreet_desktop_types, desktop->type);
    if (info) 
    {
        efreet_ini_string_set(ini, "Type", info->type);
        if (info->save_func) info->save_func(desktop, ini);
    }
    else
        ok = 0;

    if (ok)
    {
        char *val;

        if (desktop->only_show_in)
        {
            val = efreet_desktop_string_list_join(desktop->only_show_in);
            efreet_ini_string_set(ini, "OnlyShowIn", val);
            FREE(val);
        }
        if (desktop->not_show_in)
        {
            val = efreet_desktop_string_list_join(desktop->not_show_in);
            efreet_ini_string_set(ini, "NotShowIn", val);
            FREE(val);
        }
        efreet_desktop_generic_fields_save(desktop, ini);
        /* When we save the file, it should be updated to the
         * latest version that we support! */
        efreet_ini_double_set(ini, "Version", DESKTOP_VERSION);

        if (!efreet_ini_save(ini, desktop->orig_path)) ok = 0;
        else
        {
            if (desktop != ecore_hash_get(efreet_desktop_cache, desktop->orig_path))
                ecore_hash_set(efreet_desktop_cache, 
                                strdup(desktop->orig_path), desktop);
        }
    }
    efreet_ini_free(ini);
    return ok;
}

/**
 * @param desktop: The desktop file to save
 * @param file: The filename to save as
 * @return Returns 1 on success or 0 on failure
 * @brief Saves @a desktop to @a file
 */
int
efreet_desktop_save_as(Efreet_Desktop *desktop, const char *file)
{
    if (desktop == ecore_hash_get(efreet_desktop_cache, desktop->orig_path))
        ecore_hash_remove(efreet_desktop_cache, desktop->orig_path);
    FREE(desktop->orig_path);
    desktop->orig_path = strdup(file);
    return efreet_desktop_save(desktop);
}

/**
 * @internal
 * @param desktop: The Efreet_Desktop to work with
 * @return Returns no value
 * @brief Frees the Efreet_Desktop structure and all of it's data
 */
void
efreet_desktop_free(Efreet_Desktop *desktop)
{
    if (!desktop) return;

    IF_FREE(desktop->orig_path);

    IF_FREE(desktop->name);
    IF_FREE(desktop->generic_name);
    IF_FREE(desktop->comment);
    IF_FREE(desktop->icon);
    IF_FREE(desktop->url);

    IF_FREE(desktop->try_exec);
    IF_FREE(desktop->exec);
    IF_FREE(desktop->path);
    IF_FREE(desktop->startup_wm_class);

    IF_FREE_LIST(desktop->only_show_in);
    IF_FREE_LIST(desktop->not_show_in);
    IF_FREE_LIST(desktop->categories);
    IF_FREE_LIST(desktop->mime_types);

    IF_FREE_HASH(desktop->x);

    if (desktop->type_data)
    {
        Efreet_Desktop_Type_Info *info;
        info = ecore_list_goto_index(efreet_desktop_types, desktop->type);
        if (info->free_func)
            info->free_func(desktop->type_data); 
    }

    FREE(desktop);
}

/**
 * @param desktop: The desktop file to work with
 * @param files: The files to be substituted into the exec line
 * @aparam data: The data pointer to pass
 * @return Returns the Ecore_Exce for @a desktop
 * @brief Parses the @a desktop exec line and returns an Ecore_Exe.
 */
void
efreet_desktop_exec(Efreet_Desktop *desktop, Ecore_List *files, void *data)
{
    efreet_desktop_command_get(desktop, files, efreet_desktop_exec_cb, data);
}

static void
efreet_desktop_exec_cb(void *data, Efreet_Desktop *desktop __UNUSED__, 
                                char *exec, int remaining __UNUSED__)
{
    ecore_exe_run(exec, data);
    free(exec);
}

/**
 * @internal
 * @param environment: the environment name
 * @brief sets the global desktop environment name
 */
void
efreet_desktop_environment_set(const char *environment)
{
    if (desktop_environment) ecore_string_release(desktop_environment);
    if (environment) ecore_string_instance(environment);
    else desktop_environment = NULL;
}

/**
 * @param desktop: The desktop to work with
 * @return Returns the number of categories assigned to this desktop
 * @brief Retrieves the number of categories the given @a desktop belongs
 * too
 */
unsigned int
efreet_desktop_category_count_get(Efreet_Desktop *desktop)
{
    if (!desktop || !desktop->categories) return 0;
    return ecore_list_nodes(desktop->categories);
}

/**
 * @param desktop: the desktop
 * @param category: the category name
 * @brief add a category to a desktop
 */
void
efreet_desktop_category_add(Efreet_Desktop *desktop, const char *category)
{
    if (!desktop) return;

    if (!desktop->categories)
    {
        desktop->categories = ecore_list_new();
        ecore_list_set_free_cb(desktop->categories, 
                                ECORE_FREE_CB(ecore_string_release));
    }
    else
        if (ecore_list_find(desktop->categories, 
                                ECORE_COMPARE_CB(strcmp), category)) return;

    ecore_list_append(desktop->categories, 
                        (void *)ecore_string_instance(category));
}

/**
 * @param desktop: the desktop
 * @param category: the category name
 * @brief removes a category from a desktop
 * @return 1 if the desktop had his category listed, 0 otherwise
 */
int
efreet_desktop_category_del(Efreet_Desktop *desktop, const char *category)
{
    int found = 0;
    if (!desktop || !desktop->categories) return 0;

    if (ecore_list_find(desktop->categories, 
                            ECORE_COMPARE_CB(strcmp), category)) 
    {
        found = 1;
        ecore_list_remove(desktop->categories);
    }

    return found;
}

/**
 * @param type: The type to add to the list of matching types
 * @param parse_func: a function to parse out custom fields
 * @param save_func: a function to save data returned from @a parse_func
 * @param free_func: a function to free data returned from @a parse_func
 * @return Returns the id of the new type 
 * @brief Adds the given type to the list of types in the system
 */
int
efreet_desktop_type_add(const char *type, Efreet_Desktop_Type_Parse_Cb parse_func,
                        Efreet_Desktop_Type_Save_Cb save_func,
                        Efreet_Desktop_Type_Free_Cb free_func)
{
    int id;
    Efreet_Desktop_Type_Info *info;

    info = NEW(Efreet_Desktop_Type_Info, 1);
    if (!info) return 0;

    id = ecore_list_nodes(efreet_desktop_types);

    info->id = id;
    info->type = strdup(type);
    info->parse_func = parse_func;
    info->save_func = save_func;
    info->free_func = free_func;

    ecore_list_append(efreet_desktop_types, info);

    return id;
}

/**
 * @internal
 * @brief Free an Efreet Desktop_Type_Info struct
 */
static void
efreet_desktop_type_info_free(Efreet_Desktop_Type_Info *info)
{
    if (!info) return;
    IF_FREE(info->type);
    free(info);
}

/**
 * @brief get type specific data for custom desktop types
 * @param desktop the desktop
 * @return type specific data, or NULL if there is none
 */
void *
efreet_desktop_type_data_get(Efreet_Desktop *desktop)
{
    return desktop->type_data;
}

/**
 * @internal
 * @param type_str: the type as a string
 * @return the parsed type
 * @brief parse the type string into an Efreet_Desktop_Type
 */
static Efreet_Desktop_Type_Info *
efreet_desktop_type_parse(const char *type_str)
{
    Efreet_Desktop_Type_Info *info;

    if (!type_str) return NULL;

    ecore_list_goto_first(efreet_desktop_types);
    while ((info = ecore_list_next(efreet_desktop_types)))
    {
        if (!strcmp(info->type, type_str))
            return info;
    }

    return NULL;
}

/**
 * @internal
 * @param string: the raw string list
 * @return an Ecore_List of ecore string's
 * @brief Parse ';' separate list of strings according to the desktop spec
 */
static Ecore_List *
efreet_desktop_string_list_parse(const char *string)
{
    Ecore_List *list;
    char *tmp;
    char *s, *p;

    if (!string) return NULL;

    list = ecore_list_new();
    if (!list) return NULL;

    ecore_list_set_free_cb(list, ECORE_FREE_CB(ecore_string_release));

    tmp = strdup(string);
    s = tmp;

    while ((p = strchr(s, ';')))
    {
        if (p > tmp && *(p-1) == '\\') continue;
        *p = '\0';
        ecore_list_append(list, (void *)ecore_string_instance(s));
        s = p + 1;
    }
    /* If this is true, the .desktop file does not follow the standard */
    if (*s)
    {
#if STRICT_SPEC
        printf("[Efreet]: Found a string list without ';' "
                "at the end: %s\n", string);
#endif
        ecore_list_append(list, (void *)ecore_string_instance(s));
    }

    free(tmp);

    return list;
}

/**
 * @internal
 * @param list: Ecore_List with strings
 * @return a raw string list
 * @brief Create a ';' separate list of strings according to the desktop spec
 */
static char *
efreet_desktop_string_list_join(Ecore_List *list)
{
    const char *tmp;
    char *string;
    size_t size, pos, len;

    if (ecore_list_is_empty(list)) return strdup("");

    size = 1024;
    string = malloc(size);
    pos = 0;

    ecore_list_goto_first(list);
    while ((tmp = ecore_list_next(list)))
    {
        len = strlen(tmp);
        /* +1 for ';' */
        if ((len + pos + 1) >= size)
        {
            size += 1024;
            string = realloc(string, size);
        }
        strcpy(string + pos, tmp);
        pos += len;
        strcpy(string + pos, ";");
        pos += 1;
    }
    return string;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to store parsed fields in
 * @param ini: the Efreet_Ini to parse fields from
 * @return No value
 * @brief Parse application specific desktop fields
 */
static void *
efreet_desktop_application_fields_parse(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    val = efreet_ini_string_get(ini, "TryExec");
    if (val) desktop->try_exec = strdup(val);

    val = efreet_ini_string_get(ini, "Exec");
    if (val) desktop->exec = strdup(val);

    val = efreet_ini_string_get(ini, "Path");
    if (val) desktop->path = strdup(val);

    val = efreet_ini_string_get(ini, "StartupWMClass");
    if (val) desktop->startup_wm_class = strdup(val);

    desktop->categories = efreet_desktop_string_list_parse(
                                efreet_ini_string_get(ini, "Categories"));
    desktop->mime_types = efreet_desktop_string_list_parse(
                                efreet_ini_string_get(ini, "MimeType"));

    desktop->terminal = efreet_ini_boolean_get(ini, "Terminal");
    desktop->startup_notify = efreet_ini_boolean_get(ini, "StartupNotify");

    return NULL;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to save fields from
 * @param ini: the Efreet_Ini to save fields to
 * @return Returns no value
 * @brief Save application specific desktop fields
 */
static void
efreet_desktop_application_fields_save(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    char *val;

    if (desktop->try_exec) 
        efreet_ini_string_set(ini, "TryExec", desktop->try_exec);

    if (desktop->exec) 
        efreet_ini_string_set(ini, "Exec", desktop->exec);

    if (desktop->path) 
        efreet_ini_string_set(ini, "Path", desktop->path);

    if (desktop->startup_wm_class)
        efreet_ini_string_set(ini, "StartupWMClass", desktop->startup_wm_class);

    if (desktop->categories)
    {
        val = efreet_desktop_string_list_join(desktop->categories);
        efreet_ini_string_set(ini, "Categories", val);
        FREE(val);
    }

    if (desktop->mime_types)
    {
        val = efreet_desktop_string_list_join(desktop->mime_types);
        efreet_ini_string_set(ini, "MimeType", val);
        FREE(val);
    }

    efreet_ini_boolean_set(ini, "Terminal", desktop->terminal);
    efreet_ini_boolean_set(ini, "StartupNotify", desktop->startup_notify);
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to store parsed fields in
 * @param ini: the Efreet_Ini to parse fields from
 * @return Returns no value
 * @brief Parse link specific desktop fields
 */
static void *
efreet_desktop_link_fields_parse(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    val = efreet_ini_string_get(ini, "URL");
    if (val) desktop->url = strdup(val);
    return NULL;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to save fields from
 * @param ini: the Efreet_Ini to save fields in
 * @return Returns no value
 * @brief Save link specific desktop fields
 */
static void
efreet_desktop_link_fields_save(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    if (desktop->url) efreet_ini_string_set(ini, "URL", desktop->url);
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to store parsed fields in
 * @param ini: the Efreet_Ini to parse fields from
 * @return 1 if parsed succesfully, 0 otherwise
 * @brief Parse desktop fields that all types can include
 */
static int
efreet_desktop_generic_fields_parse(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    val = efreet_ini_localestring_get(ini, "Name");
    if (val) desktop->name = strdup(val);
    else 
    {
        printf("efreet_desktop_generic_fields_parse error: no Name\n");
        return 0;
    }

    val = efreet_ini_localestring_get(ini, "GenericName");
    if (val) desktop->generic_name = strdup(val);

    val = efreet_ini_localestring_get(ini, "Comment");
    if (val) desktop->comment = strdup(val);

    val = efreet_ini_localestring_get(ini, "Icon");
    if (val) desktop->icon = strdup(val);

    desktop->no_display = efreet_ini_boolean_get(ini, "NoDisplay");
    desktop->hidden = efreet_ini_boolean_get(ini, "Hidden");

    return 1;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to save fields from
 * @param ini: the Efreet_Ini to save fields to
 * @return Returns nothing
 * @brief Save desktop fields that all types can include
 */
static void
efreet_desktop_generic_fields_save(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    if (desktop->name)
    {
        efreet_ini_localestring_set(ini, "Name", desktop->name);
        val = efreet_ini_string_get(ini, "Name");
        if (!val)
            efreet_ini_string_set(ini, "Name", desktop->name);
    }
    if (desktop->generic_name)
    {
        efreet_ini_localestring_set(ini, "GenericName", desktop->generic_name);
        val = efreet_ini_string_get(ini, "GenericName");
        if (!val)
            efreet_ini_string_set(ini, "GenericName", desktop->generic_name);
    }
    if (desktop->comment)
    {
        efreet_ini_localestring_set(ini, "Comment", desktop->comment);
        val = efreet_ini_string_get(ini, "Comment");
        if (!val)
            efreet_ini_string_set(ini, "Comment", desktop->comment);
    }
    if (desktop->icon)
    {
        efreet_ini_localestring_set(ini, "Icon", desktop->icon);
        val = efreet_ini_string_get(ini, "Icon");
        if (!val)
            efreet_ini_string_set(ini, "Icon", desktop->icon);
    }

    efreet_ini_boolean_set(ini, "NoDisplay", desktop->no_display);
    efreet_ini_boolean_set(ini, "Hidden", desktop->hidden);
}

/**
 * @internal
 * @param node: The node to work with
 * @param desktop: The desktop file to work with
 * @return Returns no value
 * @brief Parses out an X- deys from @a node and stores in @a desktop
 */
static void
efreet_desktop_x_fields_parse(Ecore_Hash_Node *node, Efreet_Desktop *desktop)
{
    if (strncmp(node->key, "X-", 2)) return;

    if (!desktop->x)
    {
        desktop->x = ecore_hash_new(ecore_str_hash, ecore_str_compare);
        ecore_hash_set_free_key(desktop->x, 
                            ECORE_FREE_CB(ecore_string_release));
        ecore_hash_set_free_value(desktop->x, 
                            ECORE_FREE_CB(ecore_string_release));
    }
    ecore_hash_set(desktop->x, (void *)ecore_string_instance(node->key),
            (void *)ecore_string_instance(node->value));
}


/**
 * @internal
 * @param ini: The Efreet_Ini to parse values from
 * @return 1 if desktop should be included in current environement, 0 otherwise
 * @brief Determines if a desktop should be included in the current environment, 
 * based on the values of the OnlyShowIn and NotShowIn fields
 */
static int
efreet_desktop_environment_check(Efreet_Ini *ini)
{
    Ecore_List *list;
    const char *val;

    list = efreet_desktop_string_list_parse(efreet_ini_string_get(ini, "OnlyShowIn"));
    if (list)
    {
        int found = 0;

        if (desktop_environment) 
        {
            ecore_list_goto_first(list);
            while ((val = ecore_list_next(list)))
            {
                if (!strcmp(val, desktop_environment)) 
                {
                    found = 1;
                    break;
                }
            }
        }

        ecore_list_destroy(list);
        return found;
    }

    if (desktop_environment) 
    {
        int found = 0;

        list = efreet_desktop_string_list_parse(efreet_ini_string_get(ini, "NotShowIn"));
        if (list)
        {
            ecore_list_goto_first(list);
            while ((val = ecore_list_next(list)))
            {
                if (!strcmp(val, desktop_environment)) 
                {
                    found = 1;
                    break;
                }
            }
            ecore_list_destroy(list);
        }

        return !found;
    }

    return 1;
}


/**
 * @param desktop: the desktop entry
 * @param files: an ecore list of file names to execute, as either absolute paths,
 * relative paths, or uris
 * @param func: a callback to call for each prepared command line
 * @param data: user data passed to the callback
 * @return Returns 1 on success or 0 on failure
 * @brief Get a command to use to execute a desktop entry.
 */
int
efreet_desktop_command_get(Efreet_Desktop *desktop, Ecore_List *files, 
                            Efreet_Desktop_Command_Cb func, void *data)
{
    return efreet_desktop_command_progress_get(desktop, files, func, NULL, data);
}


/**
 * @param desktop: the desktop entry
 * @param files: an ecore list of file names to execute, as either absolute paths, 
 * relative paths, or uris
 * @param cb_command: a callback to call for each prepared command line
 * @param cb_progress: a callback to get progress for the downloads
 * @param data: user data passed to the callback
 * @return Returns 1 on success or 0 on failure
 * @brief Get a command to use to execute a desktop entry, and receive progress 
 * updates for downloading of remote URI's passed in.
 */
int
efreet_desktop_command_progress_get(Efreet_Desktop *desktop, Ecore_List *files, 
                                    Efreet_Desktop_Command_Cb cb_command, 
                                    Efreet_Desktop_Progress_Cb cb_progress,  
                                    void *data)
{
    char *p;
    Efreet_Desktop_Command *command;
    char *file;

    if (!desktop || !cb_command || !desktop->exec) return 0;

    command = NEW(Efreet_Desktop_Command, 1);
    if (!command) return 0;

    command->cb_command = cb_command;
    command->cb_progress = cb_progress;
    command->data = data;
    command->files = ecore_list_new();
    command->desktop = desktop;

    ecore_list_set_free_cb(command->files, 
                            ECORE_FREE_CB(efreet_desktop_command_file_free));

    /* first, determine which fields are present in the Exec string */
    p = strchr(desktop->exec, '%');
    while (p)
    {
        p++;
        switch(*p)
        {
            case 'f':
            case 'F':
                command->flags |= EFREET_DESKTOP_EXEC_FLAG_FULLPATH;
                break;
            case 'u':
            case 'U':
                command->flags |= EFREET_DESKTOP_EXEC_FLAG_URI;
                break;
            case 'd':
            case 'D':
                command->flags |= EFREET_DESKTOP_EXEC_FLAG_DIR;
                break;
            case 'n':
            case 'N':
                command->flags |= EFREET_DESKTOP_EXEC_FLAG_FILE;
                break;
            case '%':
                p++;
                break;
            default:
                break;
        }

        p = strchr(p, '%');
    }

    /* get the required info for each file passed in */
    if (files)
    {
        ecore_list_goto_first(files);
        while ((file = ecore_list_next(files)))
        {
            Efreet_Desktop_Command_File *dcf;

            dcf = efreet_desktop_command_file_process(command, file);
            if (!dcf) continue;
            ecore_list_append(command->files, dcf);
            command->num_pending += dcf->pending;
        }
    }

    if (command->num_pending == 0) efreet_desktop_command_build(command);
    
    return 1;
}


/**
 * @brief Builds the actual exec string from the raw string and a list of 
 * processed filename information. The callback passed in to 
 * efreet_desktop_command_get is called for each exec string created.
 *
 * @param command: the command to build
 * @return Nothing is returned
 */
static void
efreet_desktop_command_build(Efreet_Desktop_Command *command)
{
    Efreet_Desktop_Command_File *file = NULL;
    int first = 1;
    int num = 0;
    Ecore_List *execs;
    char *exec;

    execs = ecore_list_new();

    ecore_list_goto_first(command->files);

    /* if the Exec field appends multiple, that will run the list to the end, 
     * causing this loop to only run once. otherwise, this loop will generate a 
     * command for each file in the list. if the list is empty, this 
     * will run once, removing any file field codes */
    while ((file = ecore_list_next(command->files)) || first)
    {
        const char *p;
        int len = 0;
        int size = PATH_MAX;
        int file_added = 0;

        first = 0;

        exec = malloc(size);
        p = command->desktop->exec;
        len = 0;

        while (*p)
        {
            if (len >= size - 1) 
            {
                size += 1024;
                exec = realloc(exec, size);
            }

            /* XXX handle fields inside quotes? */
            if (*p == '%')
            {
                p++;
                switch (*p)
                {
                    case 'f':
                    case 'u': 
                    case 'd': 
                    case 'n': 
                        if (file) 
                        {
                            exec = efreet_desktop_command_append_single(exec, &size, 
                                                                    &len, file, *p);
                            file_added = 1;
                        }
                        break;
                    case 'F': 
                    case 'U': 
                    case 'D': 
                    case 'N': 
                        if (file) 
                        {
                            exec = efreet_desktop_command_append_multiple(exec, &size, 
                                                                    &len, command, *p);
                            file_added = 1;
                        }
                        break;
                    case 'i': 
                        exec = efreet_desktop_command_append_icon(exec, &size, &len, 
                                                                    command->desktop);
                        break;
                    case 'c': 
                        exec = efreet_desktop_command_append_quoted(exec, &size, &len,
                                                                command->desktop->name);
                        break;
                    case 'k': 
                        exec = efreet_desktop_command_append_quoted(exec, &size, &len, 
                                                            command->desktop->orig_path);
                        break;
                    case 'v': 
                    case 'm': 
                        printf("[Efreet]: Deprecated conversion char: '%c' in file '%s'\n", 
                                                            *p, command->desktop->orig_path);
                        break;
                    case '%':
                        exec[len++] = *p;
                        break;
                    default:
#if STRICT_SPEC
                        printf("[Efreet]: Unknown conversion character: '%c'\n", *p);
#endif
                        break;
                }
            }
            else exec[len++] = *p;
            p++;
        }

        exec[len++] = '\0';

        ecore_list_append(execs, exec);
        num++;

        /* If no file was added, then the Exec field doesn't contain any file 
         * fields (fFuUdDnN). We only want to run the app once in this case. */
        if (!file_added) break;
    }

    ecore_list_goto_first(execs);
    while ((exec = ecore_list_next(execs)))
    {
        command->cb_command(command->data, command->desktop, exec, --num);
    }

    efreet_desktop_command_free(command);
    ecore_list_destroy(execs);
}

static void
efreet_desktop_command_free(Efreet_Desktop_Command *command)
{
    if (!command) return;

    IF_FREE_LIST(command->files); 
    FREE(command);
}

static char *
efreet_desktop_command_append_quoted(char *dest, int *size, int *len, char *src)
{
    if (!src) return dest;
    dest = efreet_string_append(dest, size, len, "'");

    /* single quotes in src need to be escaped */
    if (strchr(src, '\''))
    {
        char *p;
        p = src;
        while (*p)
        {
            if (*p == '\'')
                dest = efreet_string_append_char(dest, size, len, '\\');
                
            dest = efreet_string_append_char(dest, size, len, *p);
            p++;
        }
    }
    else
        dest = efreet_string_append(dest, size, len, src);

    dest = efreet_string_append(dest, size, len, "'");

    return dest;
}


static char *
efreet_desktop_command_append_multiple(char *dest, int *size, int *len, 
                                        Efreet_Desktop_Command *command, 
                                        char type)
{
    Efreet_Desktop_Command_File *file;
    int first = 1;

    if (!command->files) return dest;

    ecore_list_goto_first(command->files);
    while ((file = ecore_list_next(command->files)))
    {
        if (first)
            first = 0;
        else
            dest = efreet_string_append_char(dest, size, len, ' ');

        dest = efreet_desktop_command_append_single(dest, size, len, 
                                                    file, tolower(type));
    }

    return dest;
}

static char *
efreet_desktop_command_append_single(char *dest, int *size, int *len, 
                                        Efreet_Desktop_Command_File *file, 
                                        char type)
{
    char *str;
    switch(type)
    {
        case 'f':
            str = file->fullpath;
            break;
        case 'u':
            str = file->uri;
            break;
        case 'd':
            str = file->dir;
            break;
        case 'n':
            str = file->file;
            break;
        default:
            printf("Invalid type passed to efreet_desktop_command_append_single:"
                                                                " '%c'\n", type);
            return dest;
    }

    if (!str) return dest;

    dest = efreet_desktop_command_append_quoted(dest, size, len, str);

    return dest;
}

static char *
efreet_desktop_command_append_icon(char *dest, int *size, int *len, 
                                            Efreet_Desktop *desktop)
{
    if (!desktop->icon || !desktop->icon[0]) return dest;

    dest = efreet_string_append(dest, size, len, "--icon ");
    dest = efreet_desktop_command_append_quoted(dest, size, len, desktop->icon);

    return dest;
}


/**
 * Append a string to a buffer, reallocating as necessary.
 */
static char *
efreet_string_append(char *dest, int *size, int *len, const char *src)
{
    int l;
    int off = 0;

    l = ecore_strlcpy(dest + *len, src, *size - *len);

    while (l > *size - *len) 
    {
        /* we successfully appended this much */
        off += *size - *len - 1;
        *len = *size - 1;
        *size += 1024;
        dest = realloc(dest, *size);
        *(dest + *len) = '\0';

        l = ecore_strlcpy(dest + *len, src + off, *size - *len);
    }
    *len += l;

    return dest;
}

static char *
efreet_string_append_char(char *dest, int *size, int *len, char c)
{
    if (*len >= *size - 1)
    {
        *size += 1024;
        dest = realloc(dest, *size);
    }

    dest[(*len)++] = c;
    dest[*len] = '\0';

    return dest;
}

/**
 * @param command: the Efreet_Desktop_Comand that this file is for
 * @param file: the filname as either an absolute path, relative path, or URI
 */
static Efreet_Desktop_Command_File *
efreet_desktop_command_file_process(Efreet_Desktop_Command *command, const char *file)
{
    Efreet_Desktop_Command_File *f;
    const char *uri, *base;
    int nonlocal = 0;
/*
    printf("FLAGS: %d, %d, %d, %d\n", 
        command->flags & EFREET_DESKTOP_EXEC_FLAG_FULLPATH ? 1 : 0,
        command->flags & EFREET_DESKTOP_EXEC_FLAG_URI ? 1 : 0,
        command->flags & EFREET_DESKTOP_EXEC_FLAG_DIR ? 1 : 0,
        command->flags & EFREET_DESKTOP_EXEC_FLAG_FILE ? 1 : 0);
*/
    f = NEW(Efreet_Desktop_Command_File, 1);
    if (!f) return NULL;

    f->command = command;

    /* handle uris */
    if(!strncmp(file, "http://", 7) || !strncmp(file, "ftp://", 6))
    {
        uri = file;
        base = ecore_file_get_file(file);

        nonlocal = 1;
    }
    else if (!strncmp(file, "file:", 5))
    {
        file = efreet_desktop_command_file_uri_process(file);
        if (!file)
        {
            efreet_desktop_command_file_free(f);
            return NULL;
        }
    }

    if (nonlocal)
    {
        /* process non-local uri */
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_FULLPATH)
        {
            char buf[PATH_MAX];

            snprintf(buf, PATH_MAX, "/tmp/%d-%d-%s", getpid(), 
                            efreet_desktop_command_file_id++, base);
            printf("nonlocal fullpath: %s\n", buf);
            f->fullpath = strdup(buf);
            f->pending = 1;

            ecore_file_download(uri, f->fullpath, efreet_desktop_cb_download_complete, 
                                            efreet_desktop_cb_download_progress, f);
        }

        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_URI) 
            f->uri = strdup(uri);
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_DIR) 
            f->dir = strdup("/tmp");
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_FILE) 
            f->file = strdup(base);
    }
    else
    {
        char *abs = efreet_desktop_command_path_absolute(file);
        /* process local uri/path */
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_FULLPATH)
            f->fullpath = strdup(abs);

        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_URI)
        {
            char buf[PATH_MAX];
            snprintf(buf, sizeof(buf), "file://%s", abs);
            f->uri = strdup(buf);
        }
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_DIR) 
            f->dir = ecore_file_get_dir(abs);
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_FILE) 
            f->file = strdup(ecore_file_get_file(file));

        free(abs);
    }
#if 0
    printf("  fullpath: %s\n", f->fullpath);
    printf("  uri: %s\n", f->uri);
    printf("  dir: %s\n", f->dir);
    printf("  file: %s\n", f->file);
#endif
    return f;
}

/**
 * @brief Find the local path portion of a file uri.
 * @param uri: a uri beginning with "file:"
 * @return the location of the path portion of the uri, 
 * or NULL if the file is not on this machine
 */
static const char *
efreet_desktop_command_file_uri_process(const char *uri)
{
    const char *path = NULL;
    int len = strlen(uri);

    /* uri:foo/bar => relative path foo/bar*/
    if (len >= 4 && uri[5] != '/') 
        path = uri + strlen("file:");

    /* uri:/foo/bar => absolute path /foo/bar */
    else if (len >= 5 && uri[6] != '/')
        path = uri + strlen("file:");

    /* uri://foo/bar => absolute path /bar on machine foo */
    else if (len >= 6 && uri[7] != '/')
    {
        char *tmp, *p;
        char hostname[PATH_MAX];
        tmp = strdup(uri + 7);
        p = strchr(tmp, '/');
        if (p)
        {
            *p = '\0';
            if (!strcmp(tmp, "localhost")) 
                path = uri + strlen("file://localhost");
            else 
            {
                int ret;

                ret = gethostname(hostname, PATH_MAX); 
                if ((ret == 0) && !strcmp(tmp, hostname)) 
                    path = uri + strlen("file://") + strlen(hostname);
            }
        }
        free(tmp);
    }

    /* uri:///foo/bar => absolute path /foo/bar on local machine */
    else if (len >= 7)
        path = uri + strlen("file://");

    return path;
}

static void
efreet_desktop_command_file_free(Efreet_Desktop_Command_File *file)
{
    if (!file) return;

    IF_FREE(file->fullpath);
    IF_FREE(file->uri);
    IF_FREE(file->dir);
    IF_FREE(file->file);

    FREE(file);
}


static void
efreet_desktop_cb_download_complete(void *data, const char *file __UNUSED__, 
                                                        int status __UNUSED__)
{
    Efreet_Desktop_Command_File *f;
    f = data;

    /* XXX check status... error handling, etc */
    f->pending = 0;
    f->command->num_pending--;

    if (f->command->num_pending <= 0)
        efreet_desktop_command_build(f->command);
}

static int
efreet_desktop_cb_download_progress(void *data,
                                    const char *file __UNUSED__,
                                    long int dltotal, long int dlnow,
                                    long int ultotal __UNUSED__,
                                    long int ulnow __UNUSED__)
{
    Efreet_Desktop_Command_File *dcf;

    dcf = data;
    if (dcf->command->cb_progress)
        return dcf->command->cb_progress(dcf->command->data, 
                                        dcf->command->desktop, 
                                        dcf->uri, dltotal, dlnow);

    return 0;
}

/**
 * @brief Build an absolute path from an absolute or relative one.
 * @param path: an absolute or relative path
 * @return an allocated absolute path (must be freed)
 */
static char *
efreet_desktop_command_path_absolute(const char *path)
{
    char *buf;
    int size = PATH_MAX;
    int len = 0;

    /* relative url */
    if (path[0] != '/')
    {
        buf = malloc(size);
        if (!getcwd(buf, size)) return NULL;
        len = strlen(buf);

        if (buf[len-1] != '/') buf = efreet_string_append(buf, &size, &len, "/");
        buf = efreet_string_append(buf, &size, &len, path);

        return buf;
    }

    /* just dup an alreaady absolute buffer */
    return strdup(path);
}
