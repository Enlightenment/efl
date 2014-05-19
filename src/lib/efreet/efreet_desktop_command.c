#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <ctype.h>

#ifdef _WIN32
# include <winsock2.h>
#endif

#include <Ecore.h>
#include <Ecore_File.h>

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM _efreet_desktop_log_dom
extern int _efreet_desktop_log_dom;

#include "Efreet.h"
#include "efreet_private.h"

/**
 * @internal
 * The different types of commands in an Exec entry
 */
typedef enum Efreet_Desktop_Command_Flag
{
    EFREET_DESKTOP_EXEC_FLAG_FULLPATH = 0x0001,
    EFREET_DESKTOP_EXEC_FLAG_URI      = 0x0002
} Efreet_Desktop_Command_Flag;

/**
 * @internal
 * Efreet_Desktop_Command
 */
typedef struct Efreet_Desktop_Command Efreet_Desktop_Command;

/**
 * @internal
 * Holds information on a desktop Exec command entry
 */
struct Efreet_Desktop_Command
{
  Efreet_Desktop *desktop;
  int num_pending;

  Efreet_Desktop_Command_Flag flags;

  Efreet_Desktop_Command_Cb cb_command;
  Efreet_Desktop_Progress_Cb cb_progress;
  void *data;

  Eina_List *files; /**< list of Efreet_Desktop_Command_File */
};

/**
 * @internal
 * Efreet_Desktop_Command_File
 */
typedef struct Efreet_Desktop_Command_File Efreet_Desktop_Command_File;

/**
 * @internal
 * Stores information on a file passed to the desktop Exec command
 */
struct Efreet_Desktop_Command_File
{
  Efreet_Desktop_Command *command;
  char *dir;
  char *file;
  char *fullpath;
  char *uri;

  int pending;
};

static int efreet_desktop_command_file_id = 0;

static void *efreet_desktop_exec_cb(void *data, Efreet_Desktop *desktop,
                                            char *exec, int remaining);
static int efreet_desktop_command_flags_get(Efreet_Desktop *desktop);
static void *efreet_desktop_command_execs_process(Efreet_Desktop_Command *command, Eina_List *execs);

static Eina_List *efreet_desktop_command_build(Efreet_Desktop_Command *command);
static void efreet_desktop_command_free(Efreet_Desktop_Command *command);
static char *efreet_desktop_command_append_quoted(char *dest, int *size,
                                                    int *len, char *src);
static char *efreet_desktop_command_append_multiple(char *dest, int *size, int *len,
                                                    Efreet_Desktop_Command *command,
                                                    char type);
static char *efreet_desktop_command_append_single(char *dest, int *size, int *len,
                                                Efreet_Desktop_Command_File *file,
                                                char type);
static char *efreet_desktop_command_append_icon(char *dest, int *size, int *len,
                                                Efreet_Desktop *desktop);

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

static char *efreet_desktop_command_path_absolute(const char *path);

static char *efreet_string_append(char *dest, int *size,
                                    int *len, const char *src);
static char *efreet_string_append_char(char *dest, int *size,
                                        int *len, char c);


EAPI void
efreet_desktop_exec(Efreet_Desktop *desktop, Eina_List *files, void *data)
{
    efreet_desktop_command_get(desktop, files, efreet_desktop_exec_cb, data);
}

EAPI void *
efreet_desktop_command_get(Efreet_Desktop *desktop, Eina_List *files,
                            Efreet_Desktop_Command_Cb func, void *data)
{
    return efreet_desktop_command_progress_get(desktop, files, func, NULL, data);
}

EAPI Eina_List *
efreet_desktop_command_local_get(Efreet_Desktop *desktop, Eina_List *files)
{
    Efreet_Desktop_Command *command;
    char *file;
    Eina_List *execs, *l;

    EINA_SAFETY_ON_NULL_RETURN_VAL(desktop, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(desktop->exec, NULL);

    command = NEW(Efreet_Desktop_Command, 1);
    if (!command) return 0;

    command->desktop = desktop;

    command->flags = efreet_desktop_command_flags_get(desktop);
    /* get the required info for each file passed in */
    if (files)
    {
        EINA_LIST_FOREACH(files, l, file)
        {
            Efreet_Desktop_Command_File *dcf;

            dcf = efreet_desktop_command_file_process(command, file);
            if (!dcf) continue;
            if (dcf->pending)
            {
                efreet_desktop_command_file_free(dcf);
                continue;
            }
            command->files = eina_list_append(command->files, dcf);
        }
    }

    execs = efreet_desktop_command_build(command);
    efreet_desktop_command_free(command);

    return execs;
}

EAPI void *
efreet_desktop_command_progress_get(Efreet_Desktop *desktop, Eina_List *files,
                                    Efreet_Desktop_Command_Cb cb_command,
                                    Efreet_Desktop_Progress_Cb cb_progress,
                                    void *data)
{
    Efreet_Desktop_Command *command;
    Eina_List *l;
    char *file;
    void *ret = NULL;

    EINA_SAFETY_ON_NULL_RETURN_VAL(desktop, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(desktop->exec, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(cb_command, NULL);

    command = NEW(Efreet_Desktop_Command, 1);
    if (!command) return NULL;

    command->cb_command = cb_command;
    command->cb_progress = cb_progress;
    command->data = data;
    command->desktop = desktop;
    command->num_pending = 0;

    command->flags = efreet_desktop_command_flags_get(desktop);
    /* get the required info for each file passed in */
    if (files)
    {
        EINA_LIST_FOREACH(files, l, file)
        {
            Efreet_Desktop_Command_File *dcf;

            dcf = efreet_desktop_command_file_process(command, file);
            if (!dcf) continue;
            command->files = eina_list_append(command->files, dcf);
            command->num_pending += dcf->pending;
        }
    }

    if (command->num_pending == 0)
    {
        Eina_List *execs;

        execs = efreet_desktop_command_build(command);
        if (execs)
        {
            ret = efreet_desktop_command_execs_process(command, execs);
            eina_list_free(execs);
        }
        efreet_desktop_command_free(command);
    }

    return ret;
}

static void *
efreet_desktop_exec_cb(void *data,
                       Efreet_Desktop *desktop EINA_UNUSED,
                       char *exec,
                       int remaining EINA_UNUSED)
{
    ecore_exe_run(exec, data);
    free(exec);

    return NULL;
}

/**
 * @internal
 *
 * @brief Determine which file related field codes are present in the Exec string of a .desktop
 * @params desktop and Efreet Desktop
 * @return a bitmask of file field codes present in exec string
 */
static int
efreet_desktop_command_flags_get(Efreet_Desktop *desktop)
{
    int flags = 0;
    const char *p;
    /* first, determine which fields are present in the Exec string */
    p = strchr(desktop->exec, '%');
    while (p)
    {
        p++;
        switch(*p)
        {
            case 'f':
            case 'F':
                flags |= EFREET_DESKTOP_EXEC_FLAG_FULLPATH;
                break;
            case 'u':
            case 'U':
                flags |= EFREET_DESKTOP_EXEC_FLAG_URI;
                break;
            case '%':
                p++;
                break;
            default:
                break;
        }

        p = strchr(p, '%');
    }
#ifdef SLOPPY_SPEC
    /* NON-SPEC!!! this is to work around LOTS of 'broken' .desktop files that
     * do not specify %U/%u, %F/F etc. etc. at all. just a command. this is
     * unlikely to be fixed in distributions etc. in the long run as gnome/kde
     * seem to have workarounds too so no one notices.
     */
    if (!flags) flags |= EFREET_DESKTOP_EXEC_FLAG_FULLPATH;
#endif

    return flags;
}


/**
 * @internal
 *
 * @brief Call the command callback for each exec in the list
 * @param command
 * @param execs
 */
static void *
efreet_desktop_command_execs_process(Efreet_Desktop_Command *command, Eina_List *execs)
{
    Eina_List *l;
    char *exec;
    int num;
    void *ret = NULL;

    num = eina_list_count(execs);
    EINA_LIST_FOREACH(execs, l, exec)
    {
        ret = command->cb_command(command->data, command->desktop, exec, --num);
    }
    return ret;
}


/**
 * @brief Builds the actual exec string from the raw string and a list of
 * processed filename information. The callback passed in to
 * efreet_desktop_command_get is called for each exec string created.
 *
 * @param command the command to build
 * @return a list of executable strings
 */
static Eina_List *
efreet_desktop_command_build(Efreet_Desktop_Command *command)
{
    Eina_List *execs = NULL;
    const Eina_List *l;
    char *exec;

    /* if the Exec field appends multiple, that will run the list to the end,
     * causing this loop to only run once. otherwise, this loop will generate a
     * command for each file in the list. if the list is empty, this
     * will run once, removing any file field codes */
    l = command->files;
    do
    {
        const char *p;
        int len = 0;
        int size = PATH_MAX;
        int file_added = 0;
        Efreet_Desktop_Command_File *file = eina_list_data_get(l);
       int single;

        exec = malloc(size);
        if (!exec) goto error;
        p = command->desktop->exec;
        len = 0;

        single = 0;
        while (*p)
        {
            if (len >= size - 1)
            {
                char *tmp;

                size = len + 1024;
                tmp = realloc(exec, size);
                if (!tmp) goto error;
                exec = tmp;
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
                            if (!exec) goto error;
                            file_added = 1;
                            single = 1;
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
                           fprintf(stderr, "EXE: '%s'\n", exec);
                            if (!exec) goto error;
                            file_added = 1;
                        }
                        break;
                    case 'i':
                        exec = efreet_desktop_command_append_icon(exec, &size, &len,
                                command->desktop);
                        if (!exec) goto error;
                        break;
                    case 'c':
                        exec = efreet_desktop_command_append_quoted(exec, &size, &len,
                                command->desktop->name);
                        if (!exec) goto error;
                        break;
                    case 'k':
                        exec = efreet_desktop_command_append_quoted(exec, &size, &len,
                                command->desktop->orig_path);
                        if (!exec) goto error;
                        break;
                    case 'v':
                    case 'm':
                        WRN("[Efreet]: Deprecated conversion char: '%c' in file '%s'",
                                *p, command->desktop->orig_path);
                        break;
                    case '%':
                        exec[len++] = *p;
                        break;
                    default:
#ifdef STRICT_SPEC
                        WRN("[Efreet_desktop]: Unknown conversion character: '%c'", *p);
#endif
                        break;
                }
            }
            else exec[len++] = *p;
            p++;
        }

#ifdef SLOPPY_SPEC
        /* NON-SPEC!!! this is to work around LOTS of 'broken' .desktop files that
         * do not specify %U/%u, %F/F etc. etc. at all. just a command. this is
         * unlikely to be fixed in distributions etc. in the long run as gnome/kde
         * seem to have workarounds too so no one notices.
         */
        if ((file) && (!file_added))
        {
            WRN("Efreet_desktop: %s\n"
                "  command: %s\n"
                "  has no file path/uri spec info for executing this app WITH a\n"
                "  file/uri as a parameter. This is unlikely to be the intent.\n"
                "  please check the .desktop file and fix it by adding a %%U or %%F\n"
                "  or something appropriate.",
                command->desktop->orig_path, command->desktop->exec);
            if (len >= size - 1)
            {
                char *tmp;
                size = len + 1024;
                tmp = realloc(exec, size);
                if (!tmp) goto error;
                exec = tmp;
            }
            exec[len++] = ' ';
            exec = efreet_desktop_command_append_multiple(exec, &size,
                    &len, command, 'F');
            if (!exec) goto error;
            file_added = 1;
        }
#endif
        exec[len++] = '\0';

       if ((single) || (!execs))
         {
            execs = eina_list_append(execs, exec);
            exec = NULL;
         }

        /* If no file was added, then the Exec field doesn't contain any file
         * fields (fFuUdDnN). We only want to run the app once in this case. */
        if (!file_added) break;
    }
    while ((l = eina_list_next(l)));

    return execs;
error:
    IF_FREE(exec);
    EINA_LIST_FREE(execs, exec)
        free(exec);
    return NULL;
}

static void
efreet_desktop_command_free(Efreet_Desktop_Command *command)
{
    Efreet_Desktop_Command_File *dcf;

    if (!command) return;

    while (command->files)
    {
        dcf = eina_list_data_get(command->files);
        efreet_desktop_command_file_free(dcf);
        command->files = eina_list_remove_list(command->files,
                                               command->files);
    }
    FREE(command);
}

static char *
efreet_desktop_command_append_quoted(char *dest, int *size, int *len, char *src)
{
    if (!src) return dest;
    dest = efreet_string_append(dest, size, len, "'");
    if (!dest) return NULL;

    /* single quotes in src need to be escaped */
    if (strchr(src, '\''))
    {
        char *p;
        p = src;
        while (*p)
        {
            if (*p == '\'')
            {
                dest = efreet_string_append(dest, size, len, "\'\\\'");
                if (!dest) return NULL;
            }

            dest = efreet_string_append_char(dest, size, len, *p);
            if (!dest) return NULL;
            p++;
        }
    }
    else
    {
        dest = efreet_string_append(dest, size, len, src);
        if (!dest) return NULL;
    }

    dest = efreet_string_append(dest, size, len, "'");
    if (!dest) return NULL;

    return dest;
}

static char *
efreet_desktop_command_append_multiple(char *dest, int *size, int *len,
                                        Efreet_Desktop_Command *command,
                                        char type)
{
    Efreet_Desktop_Command_File *file;
    Eina_List *l;
    int first = 1;

    if (!command->files) return dest;

    EINA_LIST_FOREACH(command->files, l, file)
    {
        if (first)
            first = 0;
        else
        {
            dest = efreet_string_append_char(dest, size, len, ' ');
            if (!dest) return NULL;
        }

        dest = efreet_desktop_command_append_single(dest, size, len,
                                                    file, tolower(type));
        if (!dest) return NULL;
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
            ERR("Invalid type passed to efreet_desktop_command_append_single:"
                                                                " '%c'", type);
            return dest;
    }

    if (!str) return dest;

    dest = efreet_desktop_command_append_quoted(dest, size, len, str);
    if (!dest) return NULL;

    return dest;
}

static char *
efreet_desktop_command_append_icon(char *dest, int *size, int *len,
                                            Efreet_Desktop *desktop)
{
    if (!desktop->icon || !desktop->icon[0]) return dest;

    dest = efreet_string_append(dest, size, len, "--icon ");
    if (!dest) return NULL;
    dest = efreet_desktop_command_append_quoted(dest, size, len, desktop->icon);
    if (!dest) return NULL;

    return dest;
}

/**
 * @param command the Efreet_Desktop_Comand that this file is for
 * @param file the filname as either an absolute path, relative path, or URI
 */
static Efreet_Desktop_Command_File *
efreet_desktop_command_file_process(Efreet_Desktop_Command *command, const char *file)
{
    Efreet_Desktop_Command_File *f;
    const char *uri, *base;
    int nonlocal = 0;
/*
    DBG("FLAGS: %d, %d, %d, %d\n",
        command->flags & EFREET_DESKTOP_EXEC_FLAG_FULLPATH ? 1 : 0,
        command->flags & EFREET_DESKTOP_EXEC_FLAG_URI ? 1 : 0);
*/
    f = NEW(Efreet_Desktop_Command_File, 1);
    if (!f) return NULL;

    f->command = command;

    /* handle uris */
    if (!strncmp(file, "file:", 5))
    {
        file = efreet_desktop_command_file_uri_process(file);
        if (!file)
        {
            efreet_desktop_command_file_free(f);
            return NULL;
        }
    }
    else if (strstr(file, ":"))
    {
        uri = file;
        base = ecore_file_file_get(file);

        nonlocal = 1;
    }

    if (nonlocal)
    {
        /* process non-local uri */
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_FULLPATH)
        {
            char buf[PATH_MAX];

            snprintf(buf, sizeof(buf), "/tmp/%d-%d-%s", getpid(),
                            efreet_desktop_command_file_id++, base);
            f->fullpath = strdup(buf);
            f->pending = 1;

            ecore_file_download(uri, f->fullpath, efreet_desktop_cb_download_complete,
                                            efreet_desktop_cb_download_progress, f, NULL);
        }

        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_URI)
            f->uri = strdup(uri);
    }
    else
    {
        char *absol = efreet_desktop_command_path_absolute(file);
        if (!absol) goto error;
        /* process local uri/path */
        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_FULLPATH)
            f->fullpath = strdup(absol);

        if (command->flags & EFREET_DESKTOP_EXEC_FLAG_URI)
        {
            const char *buf;
            Efreet_Uri ef_uri;
            ef_uri.protocol = "file";
            ef_uri.hostname = "";
            ef_uri.path = absol;
            buf = efreet_uri_encode(&ef_uri);

            f->uri = strdup(buf);

            eina_stringshare_del(buf);
        }

        free(absol);
    }
#if 0
    INF("  fullpath: %s", f->fullpath);
    INF("  uri: %s", f->uri);
    INF("  dir: %s", f->dir);
    INF("  file: %s", f->file);
#endif
    return f;
error:
    IF_FREE(f);
    return NULL;
}

/**
 * @brief Find the local path portion of a file uri.
 * @param uri a uri beginning with "file"
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
        size_t len2;

        len2 = strlen(uri + 7) + 1;
        tmp = alloca(len2);
        memcpy(tmp, uri + 7, len2);
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
efreet_desktop_cb_download_complete(void *data, const char *file EINA_UNUSED,
                                                        int status EINA_UNUSED)
{
    Efreet_Desktop_Command_File *f;

    f = data;

    /* XXX check status... error handling, etc */
    f->pending = 0;
    f->command->num_pending--;

    if (f->command->num_pending <= 0)
    {
        Eina_List *execs;

        execs = efreet_desktop_command_build(f->command);
        if (execs)
        {
            /* TODO: Need to handle the return value from efreet_desktop_command_execs_process */
            efreet_desktop_command_execs_process(f->command, execs);
            eina_list_free(execs);
        }
        efreet_desktop_command_free(f->command);
    }
}

static int
efreet_desktop_cb_download_progress(void *data,
                                    const char *file EINA_UNUSED,
                                    long int dltotal, long int dlnow,
                                    long int ultotal EINA_UNUSED,
                                    long int ulnow EINA_UNUSED)
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
 * @param path an absolute or relative path
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
        if (!(buf = malloc(size))) return NULL;
        if (!getcwd(buf, size))
        {
            FREE(buf);
            return NULL;
        }
        len = strlen(buf);

        if (buf[len-1] != '/') buf = efreet_string_append(buf, &size, &len, "/");
        if (!buf) return NULL;
        buf = efreet_string_append(buf, &size, &len, path);
        if (!buf) return NULL;

        return buf;
    }

    /* just dup an already absolute buffer */
    return strdup(path);
}

/**
 * Append a string to a buffer, reallocating as necessary.
 */
static char *
efreet_string_append(char *dest, int *size, int *len, const char *src)
{
    int l;
    int off = 0;

    l = eina_strlcpy(dest + *len, src, *size - *len);

    while (l > *size - *len)
    {
        char *tmp;
        /* we successfully appended this much */
        off += *size - *len - 1;
        *len = *size - 1;
        *size += 1024;
        tmp = realloc(dest, *size);
        if (!tmp)
        {
            free(dest);
            return NULL;
        }
        dest = tmp;
        *(dest + *len) = '\0';

        l = eina_strlcpy(dest + *len, src + off, *size - *len);
    }
    *len += l;

    return dest;
}

static char *
efreet_string_append_char(char *dest, int *size, int *len, char c)
{
    if (*len >= *size - 1)
    {
        char *tmp;
        *size += 1024;
        tmp = realloc(dest, *size);
        if (!tmp)
        {
            free(dest);
            return NULL;
        }
        dest = tmp;
    }

    dest[(*len)++] = c;
    dest[*len] = '\0';

    return dest;
}

