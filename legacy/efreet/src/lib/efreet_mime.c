/* vim: set sw=4 ts=4 sts=4 et: */
#include <Efreet.h>
#include <Ecore_Data.h>
#include <Efreet_Mime.h>
#include "efreet_private.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

static Eina_List *globs = NULL;     /* contains Efreet_Mime_Glob structs */
static Eina_List *magics = NULL;    /* contains Efreet_Mime_Magic structs */
static Eina_Hash *wild = NULL;      /* contains *.ext and mime.types globs*/
static Eina_Hash *monitors = NULL;  /* contains file monitors */
static Eina_Hash *mime_icons = NULL; /* contains cache with mime->icons */
static unsigned int _init_count = 0;

/**
 * @internal
 * @brief Holds whether we are big/little endian
 * @note This is set during efreet_mime_init based on
 * a runtime check.
 */
static enum
{
    EFREET_ENDIAN_BIG = 0,
    EFREET_ENDIAN_LITTLE = 1
} efreet_mime_endianess = EFREET_ENDIAN_BIG;

/*
 * Buffer sized used for magic checks.  The default is good enough for the
 * current set of magic rules.  This setting is only here for the future.
 */
#define EFREET_MIME_MAGIC_BUFFER_SIZE 512

/*
 * Minimum timeout in seconds between mime-icons cache flush.
 */
#define EFREET_MIME_ICONS_FLUSH_TIMEOUT 60

/*
 * Timeout in seconds, when older mime-icons items are expired.
 */
#define EFREET_MIME_ICONS_EXPIRE_TIMEOUT 600

/*
 * mime-icons maximum population.
 */
#define EFREET_MIME_ICONS_MAX_POPULATION 512

/*
 * If defined, dump mime-icons statistics after flush.
 */
//#define EFREET_MIME_ICONS_DEBUG

/**
 * Efreet_Mime_Glob
 * @brief A parsed representation of a globs file
 */
typedef struct Efreet_Mime_Glob Efreet_Mime_Glob;
struct Efreet_Mime_Glob
{
    const char *glob;
    const char *mime;
};

/**
 * Efreet_Mime_Magic
 * @brief A parsed representation of a magic file section
 */
typedef struct Efreet_Mime_Magic Efreet_Mime_Magic;
struct Efreet_Mime_Magic
{
    unsigned int priority;
    const char *mime;
    Eina_List *entries;
};

/**
 * Efreet_Mime_Magic_Entry
 * @brief A parsed representation of a magic file entry
 */
typedef struct Efreet_Mime_Magic_Entry Efreet_Mime_Magic_Entry;
struct Efreet_Mime_Magic_Entry
{
    unsigned int indent;
    unsigned int offset;
    unsigned int word_size;
    unsigned int range_len;
    unsigned short value_len;
    char *mask;
    char *value;
};

typedef struct Efreet_Mime_Icon_Entry_Head Efreet_Mime_Icon_Entry_Head;
struct Efreet_Mime_Icon_Entry_Head
{
    Eina_Inlist *list;
    const char *mime;
    time_t timestamp;
};

typedef struct Efreet_Mime_Icon_Entry Efreet_Mime_Icon_Entry;
struct Efreet_Mime_Icon_Entry
{
    EINA_INLIST;
    const char *icon;
    const char *theme;
    unsigned int size;
};


static int efreet_mime_glob_remove(const char *glob);
static void efreet_mime_mime_types_load(const char *file);
static void efreet_mime_shared_mimeinfo_globs_load(const char *file);
static void efreet_mime_shared_mimeinfo_magic_load(const char *file);
static void efreet_mime_shared_mimeinfo_magic_parse(char *data, int size);
static const char *efreet_mime_magic_check_priority(const char *file,
                                                      unsigned int start,
                                                      unsigned int end);
static int efreet_mime_init_files(void);
static const char *efreet_mime_special_check(const char *file);
static const char *efreet_mime_fallback_check(const char *file);
static void efreet_mime_glob_free(void *data);
static void efreet_mime_magic_free(void *data);
static void efreet_mime_magic_entry_free(void *data);
static int efreet_mime_glob_match(const char *str, const char *glob);
static int efreet_mime_glob_case_match(char *str, const char *glob);
static int efreet_mime_endian_check(void);

static void efreet_mime_monitor_add(const char *file);
static void efreet_mime_cb_update_file(void *data,
                                        Ecore_File_Monitor *monitor,
                                        Ecore_File_Event event,
                                        const char *path);

static void efreet_mime_icons_flush(time_t now);
static void efreet_mime_icon_entry_head_free(Efreet_Mime_Icon_Entry_Head *entry);
static void efreet_mime_icon_entry_add(const char *mime,
                                       const char *icon,
                                       const char *theme,
                                       unsigned int size);
static const char *efreet_mime_icon_entry_find(const char *mime,
                                               const char *theme,
                                               unsigned int size);
static void efreet_mime_icons_debug(void);

/**
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the efreet mime settings
 */
EAPI int
efreet_mime_init(void)
{
    _init_count++;
    if (_init_count > 1)
        return 1;

    if (!ecore_init())
        return 0;

    if (!ecore_file_init())
        return 0;

    if (!efreet_init())
        return 0;

    efreet_mime_endianess = efreet_mime_endian_check();

    monitors = eina_hash_string_superfast_new(EINA_FREE_CB(ecore_file_monitor_del));

    efreet_mime_type_cache_clear();

    if (!efreet_mime_init_files())
        return 0;

    return 1;
}

/**
 * @return Returns no value
 * @brief Cleans up the efreet mime settings system
 */
EAPI void
efreet_mime_shutdown(void)
{
    if (_init_count == 0)
        return;
    _init_count--;
    if (_init_count > 0)
        return;

    efreet_mime_icons_debug();

    IF_FREE_LIST(globs);
    IF_FREE_LIST(magics);
    IF_FREE_HASH(monitors);
    IF_FREE_HASH(wild);
    IF_FREE_HASH(mime_icons);

    efreet_shutdown();
    ecore_file_shutdown();
    ecore_shutdown();
}

/**
 * @param file: The file to find the mime type
 * @return Returns mime type as a string
 * @brief Retreive the mime type of a file
 */
EAPI const char *
efreet_mime_type_get(const char *file)
{
    const char *type = NULL;

    if ((type = efreet_mime_special_check(file)))
        return type;

    /* Check magics with priority > 80 */
    if ((type = efreet_mime_magic_check_priority(file, 0, 80)))
        return type;

    /* Check globs */
    if ((type = efreet_mime_globs_type_get(file)))
        return type;

    /* Check rest of magics */
    if ((type = efreet_mime_magic_check_priority(file, 80, 0)))
        return type;

    return efreet_mime_fallback_check(file);
}

/**
 * @param mime: The name of the mime type
 * @param theme: The name of the theme to search icons in
 * @param size: The wanted size of the icon
 * @return Returns mime type icon path as a string
 * @brief Retreive the mime type icon for a file
 */
EAPI char *
efreet_mime_type_icon_get(const char *mime, const char *theme, unsigned int size)
{
    char *icon = NULL;
    char *data;
    Eina_List *icons  = NULL;
    const char *env = NULL;
    char *p = NULL, *pp = NULL, *ppp = NULL;
    char buf[PATH_MAX];
    const char *cache;

    if (!mime || !theme || !size)
        return NULL;

    mime = eina_stringshare_add(mime);
    theme = eina_stringshare_add(theme);
    cache = efreet_mime_icon_entry_find(mime, theme, size);
    if (cache)
    {
        eina_stringshare_del(mime);
        eina_stringshare_del(theme);
        return strdup(cache);
    }

    /* Standard icon name */
    p = strdup(mime);
    pp = p;
    while (*pp)
    {
        if (*pp == '/') *pp = '-';
        pp++;
    }
    icons = eina_list_append(icons, p);

    /* Environment Based icon names */
    if ((env = efreet_desktop_environment_get()))
    {
        snprintf(buf, sizeof(buf), "%s-mime-%s", env, p);
        icons = eina_list_append(icons, strdup(buf));

        snprintf(buf, sizeof(buf), "%s-%s", env, p);
        icons = eina_list_append(icons, strdup(buf));
    }

    /* Mime prefixed icon names */
    snprintf(buf, sizeof(buf), "mime-%s", p);
    icons = eina_list_append(icons, strdup(buf));

    /* Generic icons */
    pp = strdup(p);
    while ((ppp = strrchr(pp, '-')))
    {
        *ppp = '\0';

        snprintf(buf, sizeof(buf), "%s-generic", pp);
        icons = eina_list_append(icons, strdup(buf));

        snprintf(buf, sizeof(buf), "%s", pp);
        icons = eina_list_append(icons, strdup(buf));
    }
    FREE(pp);

    /* Search for icons using list */
    icon = efreet_icon_list_find(theme, icons, size);
    while (icons)
    {
        data = eina_list_data_get(icons);
        free(data);
        icons = eina_list_remove_list(icons, icons);
    }

    efreet_mime_icon_entry_add(mime, eina_stringshare_add(icon), theme, size);

    return icon;
}

EAPI void
efreet_mime_type_cache_clear(void)
{
    if (mime_icons)
        eina_hash_free(mime_icons);
    mime_icons = eina_hash_pointer_new(EINA_FREE_CB(efreet_mime_icon_entry_head_free));
}

EAPI void
efreet_mime_type_cache_flush(void)
{
    efreet_mime_icons_flush((time_t)ecore_loop_time_get());
}


/**
 * @param file: The file to check the mime type
 * @return Returns mime type as a string
 * @brief Retreive the mime type of a file using magic
 */
EAPI const char *
efreet_mime_magic_type_get(const char *file)
{
    return efreet_mime_magic_check_priority(file, 0, 0);
}

/**
 * @param file: The file to check the mime type
 * @return Returns mime type as a string
 * @brief Retreive the mime type of a file using globs
 */
EAPI const char *
efreet_mime_globs_type_get(const char *file)
{
    Eina_List *l;
    Efreet_Mime_Glob *g;
    char *sl, *p;
    const char *s;
    char *ext, *mime;

    /* Check in the extension hash for the type */
    ext = strchr(file, '.');
    if (ext)
    {
        sl = alloca(strlen(ext) + 1);
        for (s = ext, p = sl; *s; s++, p++) *p = tolower(*s);
        *p = 0;
        p = sl;
        while (p)
        {
            p++;
            if (p && (mime = eina_hash_find(wild, p))) return mime;
            p = strchr(p, '.');
        }
    }

    /* Fallback to the other globs if not found */
    EINA_LIST_FOREACH(globs, l, g)
    {
        if (efreet_mime_glob_match(file, g->glob))
            return g->mime;
    }

    ext = alloca(strlen(file) + 1);
    for (s = file, p = ext; *s; s++, p++) *p = tolower(*s);
    *p = 0;
    EINA_LIST_FOREACH(globs, l, g)
    {
        if (efreet_mime_glob_case_match(ext, g->glob))
            return g->mime;
    }
    return NULL;
}

/**
 * @param file: The file to check the mime type
 * @return Returns mime type as a string
 * @brief Retreive the special mime type of a file
 */
EAPI const char *
efreet_mime_special_type_get(const char *file)
{
    return efreet_mime_special_check(file);
}

/**
 * @param file: The file to check the mime type
 * @return Returns mime type as a string
 * @brief Retreive the fallback mime type of a file
 */
EAPI const char *
efreet_mime_fallback_type_get(const char *file)
{
    return efreet_mime_fallback_check(file);
}

/**
 * @internal
 * @return Returns the endianess
 * @brief Retreive the endianess of the machine
 */
static int
efreet_mime_endian_check(void)
{
    int test = 1;
    return (*((char*)(&test)));
}

/**
 * @internal
 * @param file: File to monitor
 * @return Returns no value.
 * @brief Creates a new file monitor if we aren't already monitoring the
 * given file
 */
static void
efreet_mime_monitor_add(const char *file)
{
    Ecore_File_Monitor *fm = NULL;

    /* if this is already in our hash then we're already monitoring so no
     * reason to re-monitor */
    if (eina_hash_find(monitors, file))
        return;

    if ((fm = ecore_file_monitor_add(file, efreet_mime_cb_update_file, NULL)))
      {
	 eina_hash_del(monitors, file, NULL);
	 eina_hash_add(monitors, file, fm);
      }
}

/**
 * @internal
 * @param datadirs: List of XDG data dirs
 * @param datahome: Path to XDG data home directory
 * @return Returns no value
 * @brief Read all glob files in XDG data/home dirs.
 * Also reads the /etc/mime.types file.
 */
static void
efreet_mime_load_globs(Eina_List *datadirs, const char *datahome)
{
    Eina_List *l;
    char buf[4096];
    const char *datadir = NULL;

    IF_FREE_HASH(wild);
    wild = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
    while (globs)
    {
        efreet_mime_glob_free(eina_list_data_get(globs));
        globs = eina_list_remove_list(globs, globs);
    }

    /*
     * This is here for legacy reasons.  It is mentioned briefly
     * in the spec and seems to still be quite valid.  It is
     * loaded first so the globs files will override anything
     * in here.
    */
    efreet_mime_mime_types_load("/etc/mime.types");

    datadir = datahome;
        snprintf(buf, sizeof(buf), "%s/mime/globs", datadir);
        efreet_mime_shared_mimeinfo_globs_load(buf);

    EINA_LIST_FOREACH(datadirs, l, datadir)
    {
        snprintf(buf, sizeof(buf), "%s/mime/globs", datadir);
        efreet_mime_shared_mimeinfo_globs_load(buf);
    }
}

/**
 * @internal
 * @param datadirs: List of XDG data dirs
 * @param datahome: Path to XDG data home directory
 * @return Returns no value
 * @brief Read all magic files in XDG data/home dirs.
 */
static void
efreet_mime_load_magics(Eina_List *datadirs, const char *datahome)
{
    Eina_List *l;
    char buf[4096];
    const char *datadir = NULL;

    while (magics)
    {
        efreet_mime_magic_free(eina_list_data_get(magics));
        magics = eina_list_remove_list(magics, magics);
    }

    datadir = datahome;
        snprintf(buf, sizeof(buf), "%s/mime/magic", datadir);
        efreet_mime_shared_mimeinfo_magic_load(buf);

    EINA_LIST_FOREACH(datadirs, l, datadir)
    {
        snprintf(buf, sizeof(buf), "%s/mime/magic", datadir);
        efreet_mime_shared_mimeinfo_magic_load(buf);
    }
}

/**
 * @internal
 * @param data: Data pointer passed to monitor_add
 * @param monitor: Ecore_File_Monitor associated with this event
 * @param event: The type of event
 * @param path: Path to the file that was updated
 * @return Returns no value
 * @brief Callback for all file monitors.  Just reloads the appropriate
 * list depending on which file changed.  If it was a magic file
 * only the magic list is updated.  If it was a glob file or /etc/mime.types,
 * the globs are updated.
 */
static void
efreet_mime_cb_update_file(void *data __UNUSED__,
                    Ecore_File_Monitor *monitor __UNUSED__,
                    Ecore_File_Event event __UNUSED__,
                    const char *path)
{
    Eina_List *datadirs = NULL;
    const char *datahome = NULL;

    if (!(datahome = efreet_data_home_get()))
        return;

    if (!(datadirs = efreet_data_dirs_get()))
        return;

    if (strstr(path, "magic"))
        efreet_mime_load_magics(datadirs, datahome);
    else
        efreet_mime_load_globs(datadirs, datahome);
}

/**
 * @internal
 * @param datadirs: List of XDG data dirs
 * @param datahome: Path to XDG data home directory
 * @return Returns 1 on success, 0 on failure
 * @brief Initializes globs, magics, and monitors lists.
 */
static int
efreet_mime_init_files(void)
{
    Eina_List *l;
    Eina_List *datadirs = NULL;
    char buf[PATH_MAX];
    const char *datahome, *datadir = NULL;

    if (!(datahome = efreet_data_home_get()))
        return 0;

    if (!(datadirs = efreet_data_dirs_get()))
        return 0;

    /*
     * Add our file monitors
     * We watch the directories so we can watch for new files
     */
    datadir = datahome;
        snprintf(buf, PATH_MAX, "%s/mime", datadir);
        efreet_mime_monitor_add(buf);

    EINA_LIST_FOREACH(datadirs, l, datadir)
    {
        snprintf(buf, PATH_MAX, "%s/mime", datadir);
        efreet_mime_monitor_add(buf);
    }
    efreet_mime_monitor_add("/etc/mime.types");

    /* Load our mime information */
    efreet_mime_load_globs(datadirs, datahome);
    efreet_mime_load_magics(datadirs, datahome);

    return 1;
}

/**
 * @internal
 * @param file: File to examine
 * @return Returns mime type if special file, else NULL
 * @brief Returns a mime type based on the stat of a file.
 * This is used first to catch directories and other special
 * files.  A NULL return doesn't necessarily mean failure, but
 * can also mean the file is regular.
 * @note Mapping of file types to mime types:
 * Stat Macro   File Type           Mime Type
 * S_IFREG      regular             NULL
 * S_IFIFO      named pipe (fifo)   inode/fifo
 * S_IFCHR      character special   inode/chardevice
 * S_IFDIR      directory           inode/directory
 * S_IFBLK      block special       inode/blockdevice
 * S_IFLNK      symbolic link       inode/symlink
 * S_IFSOCK     socket              inode/socket
 *
 * This function can also return inode/mount-point.
 * This is calculated by comparing the st_dev of the directory
 * against that of it's parent directory.  If they differ it
 * is considered a mount point.
 */
static const char *
efreet_mime_special_check(const char *file)
{
    struct stat s;
    int path_len = 0;

    if (!lstat(file, &s))
    {
        if (S_ISREG(s.st_mode))
            return NULL;

        if (S_ISLNK(s.st_mode))
            return "inode/symlink";

        if (S_ISFIFO(s.st_mode))
            return "inode/fifo";

        if (S_ISCHR(s.st_mode))
            return "inode/chardevice";

        if (S_ISBLK(s.st_mode))
            return "inode/blockdevice";

        if (S_ISSOCK(s.st_mode))
            return "inode/socket";

        if (S_ISDIR(s.st_mode))
        {
            struct stat s2;
            char parent[PATH_MAX];
            char path[PATH_MAX];

            strncpy(path, file, PATH_MAX);

            path_len = strlen(file);
            strncpy(parent, path, PATH_MAX);

            /* Kill any trailing slash */
            parent[--path_len] = '\0';

            /* Truncate to last slash */
            while (parent[--path_len] != '/') parent[path_len] = '\0';

            if (!lstat(parent, &s2))
            {
                if (s.st_dev != s2.st_dev)
                    return "inode/mount-point";
            }

            return "inode/directory";
        }

        return NULL;
    }

    return NULL;
}

/**
 * @internal
 * @param file: File to examine
 * @return Returns mime type or NULL if the file doesn't exist
 * @brief Returns text/plain if the file appears to contain text and
 * returns application/octet-stream if it appears to be binary.
 */
static const char *
efreet_mime_fallback_check(const char *file)
{
    FILE *f = NULL;
    char buf[32];
    int i;

    if (!(f = fopen(file, "r"))) return NULL;

    i = fread(buf, 1, sizeof(buf), f);
    fclose(f);

    if (i == 0) return "application/octet-stream";

    /*
     * Check for ASCII control characters in the first 32 bytes.
     * Line Feeds, carriage returns, and tabs are ignored as they are
     * quite common in text files in the first 32 chars.
     */
    for (i -= 1; i >= 0; --i)
    {
        if ((buf[i] < 0x20) &&
            (buf[i] != '\n') &&     /* Line Feed */
            (buf[i] != '\r') &&     /* Carriage Return */
            (buf[i] != '\t'))       /* Tab */
            return "application/octet-stream";
    }

    return "text/plain";
}

/**
 * @internal
 * @param glob: Glob to search for
 * @return Returns 1 on success, 0 on failure
 * @brief Removes a glob from the list
 */
static int
efreet_mime_glob_remove(const char *glob)
{
    Efreet_Mime_Glob *mime = NULL;

    if ((mime = eina_list_search_unsorted(globs, (Eina_Compare_Cb)strcmp, glob)))
        {
        globs = eina_list_remove(globs, mime);
            IF_RELEASE(mime->glob);
            IF_RELEASE(mime->mime);
            FREE(mime);
            return 1;
        }

    return 0;
}

/**
 * @internal
 * @param file: mime.types file to load
 * @return Returns no value
 * @brief Loads values from a mime.types style file
 * into the globs list.
 * @note Format:
 * application/msaccess     mdb
 * application/msword       doc dot
 */
static void
efreet_mime_mime_types_load(const char *file)
{
    FILE *f = NULL;
    char buf[4096], mimetype[4096];
    char ext[4096], *p = NULL, *pp = NULL;

    f = fopen(file, "rb");
    if (!f) return;
    while (fgets(buf, sizeof(buf), f))
    {
        p = buf;
        while (isspace(*p) && (*p != 0) && (*p != '\n')) p++;

        if (*p == '#') continue;
        if ((*p == '\n') || (*p == 0)) continue;

        pp = p;
        while (!isspace(*p) && (*p != 0) && (*p != '\n')) p++;

        if ((*p == '\n') || (*p == 0)) continue;
        strncpy(mimetype, pp, (p - pp));
        mimetype[p - pp] = 0;

        do
        {
            while (isspace(*p) && (*p != 0) && (*p != '\n')) p++;

            if ((*p == '\n') || (*p == 0)) break;

            pp = p;
            while (!isspace(*p) && (*p != 0) && (*p != '\n')) p++;

            strncpy(ext, pp, (p - pp));
            ext[p - pp] = 0;

	    eina_hash_del(wild, ext, NULL);
            eina_hash_add(wild, ext, (void*)eina_stringshare_add(mimetype));
        }
        while ((*p != '\n') && (*p != 0));
    }
    fclose(f);
}

/**
 * @internal
 * @param file: globs file to load
 * @return Returns no value
 * @brief Loads values from a mime.types style file
 * into the globs list.
 * @note Format:
 * text/vnd.wap.wml:*.wml
 * application/x-7z-compressed:*.7z
 * application/vnd.corel-draw:*.cdr
 * text/spreadsheet:*.sylk
 */
static void
efreet_mime_shared_mimeinfo_globs_load(const char *file)
{
    FILE *f = NULL;
    char buf[4096], mimetype[4096], ext[4096], *p, *pp;
    Efreet_Mime_Glob *mime = NULL;

    f = fopen(file, "rb");
    if (!f) return;

    while (fgets(buf, sizeof(buf), f))
    {
        p = buf;
        while (isspace(*p) && (*p != 0) && (*p != '\n')) p++;

        if (*p == '#') continue;
        if ((*p == '\n') || (*p == 0)) continue;

        pp = p;
        while ((*p != ':') && (*p != 0) && (*p != '\n')) p++;

        if ((*p == '\n') || (*p == 0)) continue;
        strncpy(mimetype, pp, (p - pp));
        mimetype[p - pp] = 0;
        p++;
        pp = ext;

        while ((*p != 0) && (*p != '\n'))
        {
            *pp = *p;
            pp++;
            p++;
        }

        *pp = 0;

        if (ext[0] == '*' && ext[1] == '.')
        {
            eina_hash_del(wild, &(ext[2]), NULL);
            eina_hash_add(wild, &(ext[2]),
			  (void*)eina_stringshare_add(mimetype));
        }
        else
        {
            mime = NEW(Efreet_Mime_Glob, 1);
            if (mime)
            {
                mime->mime = eina_stringshare_add(mimetype);
                mime->glob = eina_stringshare_add(ext);
                if ((!mime->mime) || (!mime->glob))
                {
                    IF_RELEASE(mime->mime);
                    IF_RELEASE(mime->glob);
                    FREE(mime);
                }
                else
                {
                    efreet_mime_glob_remove(ext);
                    globs = eina_list_append(globs, mime);
                }
            }
        }
    }

    fclose(f);
}

/**
 * @internal
 * @param in: Number to count the digits
 * @return Returns number of digits
 * @brief Calculates and returns the number of digits
 * in a number.
 */
static int
efreet_mime_count_digits(int in)
{
    int i = 1, j = in;

    if (j < 10) return 1;
    while ((j /= 10) > 0) ++i;

    return i;
}

/**
 * @internal
 * @param file: File to parse
 * @return Returns no value
 * @brief Loads a magic file and adds information to magics list
 */
static void
efreet_mime_shared_mimeinfo_magic_load(const char *file)
{
    int fd = -1, size;
    char *data = (void *)-1;

    if (!file) return;

    size = ecore_file_size(file);
    if (size <= 0) return;

    fd = open(file, O_RDONLY);
    if (fd == -1) return;

    data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        close(fd);
        return;
    }

    efreet_mime_shared_mimeinfo_magic_parse(data, size);

    munmap(data, size);
    close(fd);
}

/**
 * @param data: The data from the file
 * @return Returns no value
 * @brief Parses a magic file
 * @note Format:
 *
 * ----------------------------------------------------------------------
 * |                     HEX                         |    ASCII         |
 * ----------------------------------------------------------------------
 * |4D 49 4D 45 2D 4D 61 67 69 63 00 0A 5B 39 30 3A  | MIME-Magic..[90: |
 * |61 70 70 6C 69 63 61 74 69 6F 6E 2F 64 6F 63 62  | application/docb |
 * |6F 6F 6B 2B 78 6D 6C 5D 0A 3E 30 3D 00 05 3C 3F  | ook+xml].>0=..<? |
 * |78 6D 6C 0A 31 3E 30 3D 00 19 2D 2F 2F 4F 41 53  | xml.1>0=..-//OAS |
 * |49 53 2F 2F 44 54 44 20 44 6F 63 42 6F 6F 6B 20  | IS//DTD DocBook  |
 * |58 4D 4C 2B 31 30 31 0A 31 3E 30 3D 00 17 2D 2F  | XML+101.1>0=..-/ |
 * ----------------------------------------------------------------------
 *
 * indent
 *   The nesting depth of the rule, corresponding to the number of '>'
 *   characters in the traditional file format.
 * ">" start-offset
 *     The offset into the file to look for a match.
 * "=" value
 *     Two bytes giving the (big-endian) length of the value, followed by the
 *     value itself.
 * "&" mask
 *     The mask, which (if present) is exactly the same length as the value.
 * "~" word-size
 *     On little-endian machines, the size of each group to byte-swap.
 * "+" range-length
 *     The length of the region in the file to check.
 *
 * The indent, range-length, word-size and mask components are optional.
 * If missing, indent defaults to 0, range-length to 1, the word-size to 1,
 * and the mask to all 'one' bits.  In our case, mask is null as it is
 * quicker, uses less memory and will acheive the same exact effect.
 */
static void
efreet_mime_shared_mimeinfo_magic_parse(char *data, int size)
{
    Efreet_Mime_Magic *mime = NULL;
    Efreet_Mime_Magic_Entry *entry = NULL;
    char *ptr;

    ptr = data;

    /* make sure we're a magic file */
    if (!ptr || (size < 12) || strncmp(ptr, "MIME-Magic\0\n", 12))
        return;

    ptr += 12;

    for (; (ptr - data) < size; )
    {
        if (*ptr == '[')
        {
            char *val, buf[512];

            mime = NEW(Efreet_Mime_Magic, 1);
            magics = eina_list_append(magics, mime);

            val = ++ptr;
            while ((*val != ':')) val++;
            memcpy(&buf, ptr, val - ptr);
            buf[val - ptr] = '\0';

            mime->priority = atoi(buf);
            ptr = ++val;

            while ((*val != ']')) val++;
            memcpy(&buf, ptr, val - ptr);
            buf[val - ptr] = '\0';

            mime->mime = eina_stringshare_add(buf);
            ptr = ++val;

            while (*ptr != '\n') ptr++;
        }
        else
        {
            short tshort;

            if (!mime) continue;
            if (!entry)
            {
                if (!(entry = NEW(Efreet_Mime_Magic_Entry, 1)))
                {
                    IF_FREE_LIST(magics);
                    return;
                }

                entry->indent = 0;
                entry->offset = 0;
                entry->value_len = 0;
                entry->word_size = 1;
                entry->range_len = 1;
                entry->mask = NULL;
                entry->value = NULL;
                ptr++;

                mime->entries = eina_list_append(mime->entries, entry);
           }

            switch(*ptr)
            {
                case '>':
                    ptr ++;
                    entry->offset = atoi(ptr);
                    ptr += efreet_mime_count_digits(entry->offset);
                    break;

                case '=':
                    ptr++;
       
                    memcpy(&tshort, ptr, sizeof(short));
                    entry->value_len = ntohs(tshort);
                    ptr += 2;

                    entry->value = NEW(1, entry->value_len);
                    memcpy(entry->value, ptr, entry->value_len);
                    ptr += entry->value_len;
                    break;

                case '&':
                    ptr++;
                    entry->mask = NEW(1, entry->value_len);
                    memcpy(entry->mask, ptr, entry->value_len);
                    ptr += entry->value_len;
                    break;

                case '~':
                    ptr++;
                    entry->word_size = atoi(ptr);
                    if (((entry->word_size != 0)
                            && (entry->word_size != 1)
                            && (entry->word_size != 2)
                            && (entry->word_size != 4))
                            || (entry->value_len % entry->word_size))
                    {
                        /* Invalid, Destroy */
                        FREE(entry->value);
                        FREE(entry->mask);
                        FREE(entry);

                        while (*ptr != '\n') ptr++;
                        break;
                    }

                    if (efreet_mime_endianess == EFREET_ENDIAN_LITTLE)
                    {
                        int j;

                        for (j = 0; j < entry->value_len; j += entry->word_size)
                        {
                            if (entry->word_size == 2)
                            {
                                ((short*)entry->value)[j] =
                                              ntohs(((short*)entry->value)[j]);

                                if (entry->mask)
                                    ((short*)entry->mask)[j] =
                                              ntohs(((short*)entry->mask)[j]);
                            }
                            else if (entry->word_size == 4)
                            {
                                ((int*)entry->value)[j] =
                                              ntohl(((int*)entry->value)[j]);

                                if (entry->mask)
                                    ((int*)entry->mask)[j] =
                                              ntohl(((int*)entry->mask)[j]);
                            }
                        }
                    }

                    ptr += efreet_mime_count_digits(entry->word_size);
                    break;

                case '+':
                    ptr++;
                    entry->range_len = atoi(ptr);
                    ptr += efreet_mime_count_digits(entry->range_len);
                    break;

                case '\n':
                    ptr++;
                    entry = NULL;
                    break;

                default:
                    if (isdigit(*ptr))
                    {
                        entry->indent = atoi(ptr);
                        ptr += efreet_mime_count_digits(entry->indent);
                    }
                    break;
            }
        }
    }

    if (entry)
    {
        IF_FREE(entry->value);
        IF_FREE(entry->mask);
        FREE(entry);
    }
}

/**
 * @internal
 * @param file: File to check
 * @param start: Start priority, if 0 start at beginning
 * @param end: End priority, should be less then start
 * unless start
 * @return Returns mime type for file if found, NULL if not
 * @brief Applies magic rules to a file given a start and end priority
 */
static const char *
efreet_mime_magic_check_priority(const char *file,
                                  unsigned int start,
                                  unsigned int end)
{
    Efreet_Mime_Magic *m = NULL;
    Efreet_Mime_Magic_Entry *e = NULL;
    Eina_List *l, *ll;
    FILE *f = NULL;
    unsigned int i = 0, offset = 0,level = 0, match = 0, bytes_read = 0;
    const char *last_mime = NULL;
    char c, v, buf[EFREET_MIME_MAGIC_BUFFER_SIZE];

    f = fopen(file, "rb");
    if (!f) return NULL;

    if (!magics)
    {
        fclose(f);
        return NULL;
    }

    if ((bytes_read = fread(buf, 1, sizeof(buf), f)) == 0)
    {
        fclose(f);
        return NULL;
    }

    EINA_LIST_FOREACH(magics, l, m)
    {
        if ((start != 0) && (m->priority > start))
            continue;

        if (m->priority < end)
            break;

        EINA_LIST_FOREACH(m->entries, ll, e)
        {
            if ((level < e->indent) && !match)
                continue;

            if ((level >= e->indent) && !match)
                level = e->indent;

            else if ((level > e->indent) && match)
            {
                fclose(f);
                if (last_mime) return last_mime;
            }

            for (offset = e->offset; offset < e->offset + e->range_len; offset++)
            {
                if (((offset + e->value_len) > bytes_read) &&
                        (fseek(f, offset, SEEK_SET) == -1))
                    break;

                match = 1;
                for (i = 0; i < e->value_len; ++i)
                {
                    if (offset + e->value_len > bytes_read)
                        c = fgetc(f);
                    else
                        c = buf[offset + i];

                    v = e->value[i];
                    if (e->mask) v &= e->mask[i];

                    if (!(c == v))
                    {
                        match = 0;
                        break;
                    }
                }

                if (match)
                {
                    level += 1;
                    last_mime = m->mime;
                    break;
                }
            }
        }
    }
    fclose(f);

    return NULL;
}

/**
 * @internal
 * @param data: Data pointer that is being destroyed
 * @return Returns no value
 * @brief Callback for globs destroy
 */
static void
efreet_mime_glob_free(void *data)
{
    Efreet_Mime_Glob *m = data;

    IF_RELEASE(m->mime);
    IF_RELEASE(m->glob);
    IF_FREE(m);
}

/**
 * @internal
 * @param data: Data pointer that is being destroyed
 * @return Returns no value
 * @brief Callback for magics destroy
 */
static void
efreet_mime_magic_free(void *data)
{
    Efreet_Mime_Magic *m = data;
    Efreet_Mime_Magic_Entry *entry = NULL;

    IF_RELEASE(m->mime);
    while (m->entries)
    {
        entry = eina_list_data_get(m->entries);
        efreet_mime_magic_entry_free(entry);
        m->entries = eina_list_remove_list(m->entries, m->entries);
    }
    IF_FREE(m);
}

/**
 * @internal
 * @param data: Data pointer that is being destroyed
 * @return Returns no value
 * @brief Callback for magic entry destroy
 */
static void
efreet_mime_magic_entry_free(void *data)
{
    Efreet_Mime_Magic_Entry *e = data;

    IF_FREE(e->mask);
    IF_FREE(e->value);
    IF_FREE(e);
}


/**
 * @internal
 * @param str: String (filename) to match
 * @param glob: Glob to match str to
 * @return Returns 1 on success, 0 on failure
 * @brief Compares str to glob, case sensitive
 */
static int
efreet_mime_glob_match(const char *str, const char *glob)
{
    if (!str || !glob) return 0;
    if (glob[0] == 0)
    {
        if (str[0] == 0) return 1;
        return 0;
    }
    if (!fnmatch(glob, str, 0)) return 1;
    return 0;
}

/**
 * @internal
 * @param str: String (filename) to match
 * @param glob: Glob to match str to
 * @return Returns 1 on success, 0 on failure
 * @brief Compares str to glob, case insensitive (expects str already in lower case)
 */
static int
efreet_mime_glob_case_match(char *str, const char *glob)
{
    const char *p;
    char *tglob, *tp;

    if (!str || !glob) return 0;
    if (glob[0] == 0)
    {
        if (str[0] == 0) return 1;
        return 0;
    }
    tglob = alloca(strlen(glob) + 1);
    for (tp = tglob, p = glob; *p; p++, tp++) *tp = tolower(*p);
    *tp = 0;
    if (!fnmatch(str, tglob, 0)) return 1;
    return 0;
}

struct Efreet_Mime_Icons_Flush_Data
{
    time_t now;
    int todo;
    Eina_List *removal;
};

static Eina_Bool
efreet_mime_icons_flush_cb(const Eina_Hash *hash __UNUSED__,
                           const void *key __UNUSED__,
                           void *data,
                           void *fdata)
{
    struct Efreet_Mime_Icons_Flush_Data *d = fdata;
    Efreet_Mime_Icon_Entry_Head *entry = data;

    if (d->now - entry->timestamp < EFREET_MIME_ICONS_EXPIRE_TIMEOUT)
        return 1;

    d->removal = eina_list_append(d->removal, entry->mime);
    d->todo--;

    return d->todo > 0;
}

static void
efreet_mime_icons_flush(time_t now)
{
    struct Efreet_Mime_Icons_Flush_Data data;
    static time_t old = 0;
    int todo;
    const char *key;

    if (now - old < EFREET_MIME_ICONS_FLUSH_TIMEOUT)
        return;

    todo = eina_hash_population(mime_icons) - EFREET_MIME_ICONS_MAX_POPULATION;
    if (todo <= 0)
        return;

    data.now = now;
    data.todo = todo;
    data.removal = NULL;
    eina_hash_foreach(mime_icons, efreet_mime_icons_flush_cb, &data);

    EINA_LIST_FREE(data.removal, key)
        eina_hash_del_by_key(mime_icons, key);

    efreet_mime_icons_debug();
}

static void
efreet_mime_icon_entry_free(Efreet_Mime_Icon_Entry *node)
{
    eina_stringshare_del(node->icon);
    eina_stringshare_del(node->theme);
    free(node);
}

static void
efreet_mime_icon_entry_head_free(Efreet_Mime_Icon_Entry_Head *entry)
{
    while (entry->list)
    {
        Efreet_Mime_Icon_Entry *n = (Efreet_Mime_Icon_Entry *)entry->list;
        entry->list = eina_inlist_remove(entry->list, entry->list);
        efreet_mime_icon_entry_free(n);
    }

    eina_stringshare_del(entry->mime);
    free(entry);
}

static Efreet_Mime_Icon_Entry *
efreet_mime_icon_entry_new(const char *icon,
                           const char *theme,
                           unsigned int size)
{
    Efreet_Mime_Icon_Entry *entry;

    entry = malloc(sizeof(*entry));
    if (!entry)
        return NULL;

    entry->icon = icon;
    entry->theme = theme;
    entry->size = size;

    return entry;
}

static void
efreet_mime_icon_entry_add(const char *mime,
                           const char *icon,
                           const char *theme,
                           unsigned int size)
{
    Efreet_Mime_Icon_Entry_Head *entry;
    Efreet_Mime_Icon_Entry *n;

    n = efreet_mime_icon_entry_new(icon, theme, size);
    if (!n)
        return;
    entry = eina_hash_find(mime_icons, mime);

    if (entry)
        entry->list = eina_inlist_prepend(entry->list, EINA_INLIST_GET(n));
    else
    {
        entry = malloc(sizeof(*entry));
        if (!entry)
        {
            efreet_mime_icon_entry_free(n);
            return;
        }
        entry->list = eina_inlist_prepend(NULL, EINA_INLIST_GET(n));
        entry->mime = mime;
        eina_hash_direct_add(mime_icons, mime, entry);
    }

    entry->timestamp = (time_t)ecore_loop_time_get();
    efreet_mime_icons_flush(entry->timestamp);
}

static const char *
efreet_mime_icon_entry_find(const char *mime,
                            const char *theme,
                            unsigned int size)
{
    Efreet_Mime_Icon_Entry_Head *entry;
    Efreet_Mime_Icon_Entry *n;

    entry = eina_hash_find(mime_icons, mime);
    if (!entry)
        return NULL;

    EINA_INLIST_FOREACH(entry->list, n)
    {
        if ((n->theme == theme) && (n->size == size))
        {
            Eina_Inlist *l = EINA_INLIST_GET(n);
            if (entry->list != l)
                entry->list = eina_inlist_promote(entry->list, l);
            entry->timestamp = (time_t)ecore_loop_time_get();
            return n->icon;
        }
    }

    return NULL;
}

#ifdef EFREET_MIME_ICONS_DEBUG
static Eina_Bool
efreet_mime_icons_debug_cb(const Eina_Hash *hash __UNUSED__,
                           const void *key __UNUSED__,
                           void *data,
                           void *fdata __UNUSED__)
{
    Efreet_Mime_Icon_Entry_Head *entry = data;
    Efreet_Mime_Icon_Entry *n;

    printf("mime-icon entry: '%s' last used: %s",
           entry->mime, ctime(&entry->timestamp));

    EINA_INLIST_FOREACH(entry->list, n)
           printf("\tsize: %3u theme: '%s' icon: '%s'\n",
                  n->theme, n->size, n->icon);

    return 1;
}

static void
efreet_mime_icons_debug(void)
{
    eina_hash_foreach(mime_icons, efreet_mime_icons_debug_cb, NULL);
}
#else
static void
efreet_mime_icons_debug(void)
{
}
#endif
