/* vim: set sw=4 ts=4 sts=4 et: */
#include <Efreet.h>
#include <Ecore_Data.h>
#include <Efreet_Mime.h>
#include "efreet_private.h"
#include <arpa/inet.h>

static Ecore_List *globs = NULL;     /* contains Efreet_Mime_Glob structs */
static Ecore_List *magics = NULL;    /* contains Efreet_Mime_Magic structs */
static Ecore_List *monitors = NULL;  /* contains Efreet_Mime_Monitor structs */

/**
 * @internal
 * @brief Holds whether we are big/little endian
 * @note This is set during efreet_mime_init based on
 * a runtime check.
 */
static enum 
{ 
    BIG = 0, 
    LITTLE = 1 
} efreet_mime_endianess = BIG;        


/*
 * Buffer sized used for magic checks.  The default is good enough for the 
 * current set of magic rules.  This setting is only here for the future.
 */
#define EFREET_MIME_MAGIC_BUFFER_SIZE 512

typedef struct Efreet_Mime_Monitor Efreet_Mime_Monitor;
struct Efreet_Mime_Monitor
{
    const char *file;
    Ecore_File_Monitor *monitor;
};

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
    Ecore_List *entries;
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

static int          efreet_mime_glob_remove(const char *glob);
static void         efreet_mime_mime_types_load(const char *file);
static void         efreet_mime_shared_mimeinfo_globs_load(const char *file);
static void         efreet_mime_shared_mimeinfo_magic_load(const char *file);
static const char * efreet_mime_magic_check_priority(const char *file, 
                                                      unsigned int start, 
                                                      unsigned int end);
static void         efreet_mime_update_file_cb(void *data, 
                                        Ecore_File_Monitor *monitor,
                                        Ecore_File_Event event, 
                                        const char *path);
static int          efreet_mime_init_files(void);
static const char * efreet_mime_special_check(const char *file);
static void         efreet_mime_glob_free(void *data);
static void         efreet_mime_magic_free(void *data);
static int          efreet_mime_glob_match(const char *str, const char *glob);
static int          efreet_mime_glob_case_match(char *str, const char *glob);
static int          efreet_mime_endian_check(void);

static void         efreet_mime_monitor_del(const char *file);
static void         efreet_mime_monitor_free(void *data);

/**
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the efreet mime settings
 */
int
efreet_mime_init(void)
{            
    Efreet_Mime_Monitor *mm = NULL;

    if (!ecore_init())
        return 0;
    
    if (!ecore_file_init())
        return 0;
  
    if (!efreet_init())
        return 0;
    
    efreet_mime_endianess = efreet_mime_endian_check();
    
    monitors = ecore_list_new();
    ecore_list_set_free_cb(monitors, efreet_mime_monitor_free);
    
    if (!efreet_mime_init_files())
        return 0;
    
    /* Can be used to debug monitors*/
    if (monitors)
    {
        ecore_list_goto_first(monitors);
        while ((mm = ecore_list_next(monitors)))
            printf("Watching: %s\n", mm->file);
    }
    
    return 1;
}

/**
 * @return Returns no value
 * @brief Cleans up the efreet mime settings system
 */
void
efreet_mime_shutdown(void)
{
    IF_FREE_LIST(magics);
    IF_FREE_LIST(globs);
    IF_FREE_LIST(monitors);
    
    efreet_shutdown();
    ecore_file_shutdown();
    ecore_shutdown();
}

/**
 * @param file: The file to check the mime type
 * @param methods: The methods to use, see Efreet_Mime_Method structure
 * @return Returns mime type as a string
 * @brief Retreive the mime type of a file
 */
const char *
efreet_mime_get(const char *file, int methods)
{   
    const char *type = NULL;
  
    if (!methods)
        methods = EFREET_MIME_FLAG_GLOB | EFREET_MIME_FLAG_TYPES;
      
    if ((type = efreet_mime_special_check(file)))
        return type;
  
    if (methods & EFREET_MIME_FLAG_MAGIC)
    {
      if ((type = efreet_mime_magic_check_priority(file, 0, 80)))
          return type;
    }
    
    if ((methods & EFREET_MIME_FLAG_GLOB) 
            || (methods & EFREET_MIME_FLAG_TYPES))
    {
        Efreet_Mime_Glob *g;
        char *s;
        
        ecore_list_goto_first(globs);
        while ((g = ecore_list_next(globs)))
        {
            if (efreet_mime_glob_match(file, g->glob))
                return g->mime;
        }

        s = strdup(file);
        ecore_list_goto_first(globs);
        while ((g = ecore_list_next(globs)))
        {                
            if (efreet_mime_glob_case_match(s, g->glob))
                return g->mime;
        }
        FREE(s);
    }
    
    if (methods & EFREET_MIME_FLAG_MAGIC)
    {
        if ((type = efreet_mime_magic_check_priority(file, 80, 0)))
            return type;
    }
    
    return NULL;
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
 * @return Returns the monitor, see Efreet_Mime_Monitor
 * @brief Create/Add a new file monitor
 */
static Efreet_Mime_Monitor *
efreet_mime_monitor_add(const char *file) 
{
    Efreet_Mime_Monitor *mm = NULL;
    Ecore_File_Monitor *fm = NULL;

    efreet_mime_monitor_del(file);
    mm = NEW(Efreet_Mime_Monitor, 1);
    
    if ((fm = ecore_file_monitor_add(file, efreet_mime_update_file_cb, mm)))
    {      
        mm->file = strdup(file);
        mm->monitor = fm;
        
        ecore_list_append(monitors, mm);
    }
    else
        FREE(mm);
    
    return mm;
}

/**
 * @internal
 * @param file: File to discontinue monitoring
 * @return Returns no value
 * @brief Delete a file monitor
 */
static void
efreet_mime_monitor_del(const char *file)
{
    Efreet_Mime_Monitor *mm = NULL;
    
    ecore_list_goto_first(monitors);
    while ((mm = ecore_list_current(monitors)))
    {
        if (!strcmp(mm->file, file))
        {
            ecore_list_remove_destroy(monitors);
            break;
        }
        ecore_list_next(monitors);
    }
}

/**
 * @internal
 * @param data: Pointer to the monitor to free
 * @return Returns no value
 * @brief Free a file monitor structure.
 * Destroy callback for monitors.
 */
static void
efreet_mime_monitor_free(void *data)
{
    Efreet_Mime_Monitor *mm = NULL;

    mm = data;
    if (mm->monitor) ecore_file_monitor_del(mm->monitor);
    FREE(mm);
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
efreet_mime_load_globs(Ecore_List *datadirs, const char *datahome)
{
    char buf[4096];
    const char *datadir = NULL;
        
    IF_FREE_LIST(globs);
    globs = ecore_list_new();    
    ecore_list_set_free_cb(globs, efreet_mime_glob_free);
    
    ecore_list_goto_first(datadirs);
    while ((datadir = ecore_list_next(datadirs)))
    {
        snprintf(buf, sizeof(buf), "%s/mime/globs", datadir);
        efreet_mime_shared_mimeinfo_globs_load(buf);
    }
    
    if (datahome && (datahome[0] != '\0'))
    {
        snprintf(buf, sizeof(buf), "%s/mime/globs", datahome);
        efreet_mime_shared_mimeinfo_globs_load(buf);
    }
    
    /*
     * This is here for legacy reasons.  It is mentioned briefly
     * in the spec and seems to still be quite valid.
    */
    efreet_mime_mime_types_load("/etc/mime.types");
}

/**
 * @internal
 * @param datadirs: List of XDG data dirs
 * @param datahome: Path to XDG data home directory
 * @return Returns no value
 * @brief Read all magic files in XDG data/home dirs.
 */
static void 
efreet_mime_load_magics(Ecore_List *datadirs, const char *datahome)
{
    char buf[4096];
    const char *datadir = NULL;
    
    IF_FREE_LIST(magics);
    magics = ecore_list_new();
    ecore_list_set_free_cb(magics, efreet_mime_magic_free);    
    
    ecore_list_goto_first(datadirs);
    while ((datadir = ecore_list_next(datadirs)))
    {
        snprintf(buf, sizeof(buf), "%s/mime/magic", datadir);
        efreet_mime_shared_mimeinfo_magic_load(buf);
    }

    if (datahome && (datahome[0] != '\0'))
    {    
        snprintf(buf, sizeof(buf), "%s/mime/magic", datahome);
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
efreet_mime_update_file_cb(void *data __UNUSED__, 
                    Ecore_File_Monitor *monitor __UNUSED__,
                    Ecore_File_Event event __UNUSED__, 
                    const char *path)
{
    Ecore_List *datadirs = NULL; 
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
    Ecore_List *datadirs = NULL; 
    char buf[PATH_MAX];
    const char *datahome = NULL, *datadir = NULL;
    
    if (!(datahome = efreet_data_home_get()))
        return 0;
    
    if (!(datadirs = efreet_data_dirs_get()))
        return 0;
    
    /* Add our file monitors */
    /* We watch the directories so we can watch for new files? */
    ecore_list_goto_first(datadirs);
    while ((datadir = ecore_list_next(datadirs)))
    {
        snprintf(buf,PATH_MAX,"%s/mime", datadir);
        efreet_mime_monitor_add(buf);
    }
    
    snprintf(buf,PATH_MAX,"%s/mime", datahome);
    efreet_mime_monitor_add(buf);
    
    efreet_mime_monitor_add("/etc/mime.types");
    
    /*
     * Load our mime information
     */
    efreet_mime_load_globs(datadirs, datahome);
    efreet_mime_load_magics(datadirs, datahome);
    
    return 1;
}

/**
 * @internal
 * @param datadirs: List of XDG data dirs
 * @param datahome: Path to XDG data home directory
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
 * @param glob: Glob to search for
 * @return Returns 1 on success, 0 on failure
 * @brief Removes a glob from the list
 */
static int
efreet_mime_glob_remove(const char *glob)
{
    Efreet_Mime_Glob *mime = NULL;
  
    mime = ecore_list_goto_first(globs);
    while ((mime = ecore_list_current(globs)))
    {
        if (!strcmp(glob, mime->glob))
        {
            ecore_list_remove(globs);
            IF_RELEASE(mime->glob);
            IF_RELEASE(mime->mime);
            FREE(mime);
            return 1;
        }
        ecore_list_next(globs);
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
    char buf[4096], buf2[4096], mimetype[4096];
    char ext[4096], *p = NULL, *pp = NULL;
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

            mime = NEW(Efreet_Mime_Glob,1);
            if (mime)
            {
                mime->mime = ecore_string_instance(mimetype);
                snprintf(buf2, sizeof(buf2), "*.%s", ext);
                mime->glob = ecore_string_instance(buf2);
                if ((!mime->mime) || (!mime->glob))
                {
                    IF_RELEASE(mime->mime);
                    IF_RELEASE(mime->glob);
                    FREE(mime);
                }
                else
                {
                    efreet_mime_glob_remove(buf2);
                    ecore_list_append(globs, mime);
                }
            }
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

        mime = NEW(Efreet_Mime_Glob, 1);
        if (mime)
        {
            mime->mime = ecore_string_instance(mimetype);
            mime->glob = ecore_string_instance(ext);
            if ((!mime->mime) || (!mime->glob))
            {
                IF_RELEASE(mime->mime);
                IF_RELEASE(mime->glob);
                FREE(mime);
            }
            else
            {
                efreet_mime_glob_remove(ext);
                ecore_list_append(globs, mime);
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
  
  if (j < 10)
      return 1;
  
  while ((j /= 10) > 0) ++i;
    
  return i;
}

/**
 * @internal
 * @param file: File to parse
 * @return Returns number of digits
 * @brief Parses a magic file and adds information to magics list
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
efreet_mime_shared_mimeinfo_magic_load(const char *file)
{
    FILE *f = NULL;
    int priority = 0, i = 0;
    enum {INDENT=0, OFFSET, VALUE, MASK, 
          WORD_SIZE, RANGE_LEN} magic_state = 0;
    char buf[4096], mimetype[4096];
    Efreet_Mime_Magic *mime = NULL;
    Efreet_Mime_Magic_Entry *entry = NULL;
    int bytes_read = 0, last_section = 0;
    
    f = fopen(file, "rb");
    if (!f) return;
    
    /* Check for magic string.  Will also move us to first section */
    if (fread(buf, 1, 12, f))
      if (memcmp(buf, "MIME-Magic\0\n", 12)) return;
      
    /* Begin reading in sections and mime data */
    while ((bytes_read = fread(buf, 1, sizeof(buf), f)))
    {        
        for (i=0; i < bytes_read;)
        {        
            if (buf[i] == '[')
            {
                IF_FREE(entry);
                
                last_section = i;
                i++;
                priority = atoi(strtok(&(buf[i]), ":"));
                strncpy(mimetype, strtok(NULL, "]"), sizeof(mimetype));
  
                mime = NEW(Efreet_Mime_Magic, 1);
                mime->mime = ecore_string_instance(mimetype);
                mime->priority = priority;
                mime->entries = ecore_list_new();
                
                while (i < bytes_read && buf[i] != '\n') ++i;
            }
            else
            {
                if (!mime) continue;
                
                switch(buf[i])
                {
                    case '>':
                        magic_state = OFFSET;
                        break;

                    case '=':
                        magic_state = VALUE;
                        break;

                    case '&':
                        magic_state = MASK;
                        break;

                    case '~':
                        magic_state = WORD_SIZE;
                        break;

                    case '+':
                        magic_state = RANGE_LEN;
                        break;

                    default:
                        if (isdigit(buf[i])) magic_state = INDENT;
                        break;
                }    
                
                if (magic_state != INDENT)
                    ++i;
                
                switch(magic_state)
                {
                    case INDENT:
                        entry->indent = atoi(&buf[i]);
                        i += efreet_mime_count_digits(entry->indent);
                        break;

                    case OFFSET:
                        entry->offset = atoi(&buf[i]);
                        i += efreet_mime_count_digits(entry->offset);
                        break;

                    case VALUE:
                        entry->value_len = ntohs(buf[i + 1] << 8 | (short)(buf[i]));
                        i += 2;

                        entry->value = NEW(1, entry->value_len);        
                        memcpy(entry->value, &(buf[i]), entry->value_len);
                        i += entry->value_len;
                        break;

                    case MASK:
                        entry->mask = NEW(1, entry->value_len);
                        memcpy(entry->mask, &(buf[i]), entry->value_len);
                        i += entry->value_len;
                        break;

                    case WORD_SIZE:
                        entry->word_size = atoi(&(buf[i]));
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
                            while ((i < bytes_read) && (buf[i] != '\n')) ++i;
                            break;
                        }
                        
                        if (efreet_mime_endianess == LITTLE)
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
                                    ((long*)entry->value)[j] = 
                                              ntohl(((long*)entry->value)[j]);
                                  
                                    if (entry->mask)
                                        ((long*)entry->mask)[j] = 
                                              ntohl(((long*)entry->mask)[j]);
                                }
                            }
                        }
                        
                        i += efreet_mime_count_digits(entry->word_size);
                        break;

                    case RANGE_LEN:
                        entry->range_len = atoi(&(buf[i]));
                        i += efreet_mime_count_digits(entry->range_len);
                        break;

                    default:
                        break;
                }
            }            
            
            /*
             * Reached the end of the file.  Need to go back to the last 
             * section.  This can probably be elmiminated if the file is
             * read another way which allows random access without having
             * to read the file in chunks.  This works fine for now though.
             */
            if (i >= 4096)        
            {
                /* If we have a mime, clean it up */
                if (mime)
                {
                    Efreet_Mime_Magic *m;
                    m = ecore_list_goto_last(magics);
                  
                    if (m && !(strcmp(m->mime,mimetype)))
                    {
                        efreet_mime_magic_free(m);
                        ecore_list_remove(magics);
                    }
                
                    FREE(mime);
                }
                
                /*
                 * If we finished in the middle of an entry, make sure to
                 * clean it up as well.
                 */
                IF_FREE(entry);
                
                fseek(f, last_section-4096, SEEK_CUR);
                break;
            }
            
            /* Create our new structure */
            if (buf[i] == '\n')
            {                                    
                if (entry) ecore_list_append(mime->entries, entry);

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
                ++i;
            }
            
            /* Append Mime to list of magics */
            if (mime && ((buf[i] == '[') || (i >= bytes_read)))
                ecore_list_append(magics, mime);
        }
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
    FILE *f = NULL;
    unsigned int i = 0, offset = 0,level = 0, match = 0, bytes_read = 0;
    const char *last_mime = NULL;
    char c, v, buf[EFREET_MIME_MAGIC_BUFFER_SIZE];
              
    f = fopen(file, "rb");
    if (!f) return NULL;
      
    if (!(m = ecore_list_goto_first(magics)))
    {
        fclose(f);
        return NULL;
    }
    
    bytes_read = fread(buf, 1, sizeof(buf), f);
    while ((m = ecore_list_next(magics)))
    {
        if ((start != 0) && (m->priority > start))
            continue;
        
        if (m->priority < end)                        
            break;
        
        ecore_list_goto_first(m->entries);
        while ((e = ecore_list_next(m->entries)))
        {                    
            if ((level < e->indent) && !match)
                continue;
            if ((level >= e->indent) && !match)
                level = e->indent;
            else if ((level > e->indent) && match)
            {
                fclose(f);
              
                if (last_mime)
                    return last_mime;
            }
            
            for (offset = e->offset; offset < e->offset+e->range_len; ++offset)
            {
                if (((offset+e->value_len) > bytes_read) && 
                    (fseek(f, offset, SEEK_SET) == -1))
                    break;
                
                match = 1;
                for (i = 0; i < e->value_len; ++i)
                {
                    if (offset+e->value_len > bytes_read)
                        c = fgetc(f);
                    else
                        c = buf[offset + i];
                    
                    v = e->value[i];
                    
                    if (e->mask)
                        v &= e->mask[i];

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
    Efreet_Mime_Magic_Entry *e;
    
    IF_RELEASE(m->mime);
    
    if (m->entries)
    {
        ecore_list_goto_first(m->entries);
        while ((e = ecore_list_next(m->entries)))
        {
            IF_FREE(e->mask);
            IF_FREE(e->value);
        }

        IF_FREE_LIST(m->entries);
    }
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
    if (!str || !glob)
        return 0;
    
    if (glob[0] == '\0')
    {
        if (str[0] == '\0') return 1;
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
 * @brief Compares str to glob, case insensitive
 */
static int
efreet_mime_glob_case_match(char *str, const char *glob)
{
    const char *p;
    char *tglob, *tp;

    if (glob[0] == '\0')
    {
        if (str[0] == '\0') return 1;
        return 0;
    }
      
    for (tp = str; *tp != '\0'; tp++) *tp = tolower(*tp);
    
    tglob = NEW(1, strlen(glob) + 1);
    for (tp = tglob, p = glob; *p != 0; p++, tp++) *tp = tolower(*p);
    *tp = 0;
    
    if (!fnmatch(str, p, 0)) return 1;
      
    IF_FREE(tglob);
    
    return 0;
}

