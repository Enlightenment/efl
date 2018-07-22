#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fnmatch.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
#endif

#include <Ecore.h>
#include <Ecore_File.h>

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM _efreet_mime_log_dom
static int _efreet_mime_log_dom = -1;

#include "Efreet.h"
#include "Efreet_Mime.h"
#include "efreet_private.h"

static Eina_List *magics = NULL;    // contains Efreet_Mime_Magic structs
static Eina_Hash *mime_icons = NULL; // contains cache with mime->icons
static Eina_Inlist *mime_icons_lru = NULL;
static unsigned int _efreet_mime_init_count = 0;

static const char *_mime_inode_symlink = NULL;
static const char *_mime_inode_fifo = NULL;
static const char *_mime_inode_chardevice = NULL;
static const char *_mime_inode_blockdevice = NULL;
static const char *_mime_inode_socket = NULL;
static const char *_mime_inode_mountpoint = NULL;
static const char *_mime_inode_directory = NULL;
static const char *_mime_application_x_executable = NULL;
static const char *_mime_application_octet_stream = NULL;
static const char *_mime_text_plain = NULL;

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

typedef struct Efreet_Mime_Magic Efreet_Mime_Magic;
struct Efreet_Mime_Magic
{
   unsigned int priority;
   const char *mime;
   Eina_List *entries;
};

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
   EINA_INLIST; /* node of mime_icons_lru */
   Eina_Inlist *list;
   const char *mime;
   double timestamp;
};

typedef struct Efreet_Mime_Icon_Entry Efreet_Mime_Icon_Entry;
struct Efreet_Mime_Icon_Entry
{
   EINA_INLIST;
   const char *icon;
   const char *theme;
   unsigned int size;
};

static void efreet_mime_shared_mimeinfo_magic_load(const char *file);
static void efreet_mime_shared_mimeinfo_magic_parse(char *data, int size);
static const char *efreet_mime_magic_check_priority(const char *file,
                                                    unsigned int start,
                                                    unsigned int end);
static int efreet_mime_init_files(void);
static const char *efreet_mime_special_check(const char *file);
static const char *efreet_mime_fallback_check(const char *file);
static void efreet_mime_magic_free(void *data);
static void efreet_mime_magic_entry_free(void *data);
static int efreet_mime_glob_match(const char *str, const char *glob);
static int efreet_mime_glob_case_match(char *str, const char *glob);
static int efreet_mime_endian_check(void);

static void efreet_mime_icons_flush(double now);
static void efreet_mime_icon_entry_head_free(Efreet_Mime_Icon_Entry_Head *entry);
static void efreet_mime_icon_entry_add(const char *mime,
                                       const char *icon,
                                       const char *theme,
                                       unsigned int size);
static const char *efreet_mime_icon_entry_find(const char *mime,
                                               const char *theme,
                                               unsigned int size);
static void efreet_mime_icons_debug(void);












static Eina_File *mimedb = NULL;
static unsigned char *mimedb_ptr = NULL;
static size_t mimedb_size = 0;

static void
_efreet_mimedb_shutdown(void)
{
   if (mimedb)
     {
        if (mimedb_ptr) eina_file_map_free(mimedb, mimedb_ptr);
        eina_file_close(mimedb);
        mimedb = NULL;
        mimedb_ptr = NULL;
        mimedb_size = 0;
     }
}

static void
_efreet_mimedb_update(void)
{
   char buf[PATH_MAX];

   if (mimedb)
     {
        if (mimedb_ptr) eina_file_map_free(mimedb, mimedb_ptr);
        eina_file_close(mimedb);
        mimedb = NULL;
        mimedb_ptr = NULL;
        mimedb_size = 0;
     }
#ifdef WORDS_BIGENDIAN
   snprintf(buf, sizeof(buf), "%s/efreet/mime_cache_%s.be.dat",
            efreet_cache_home_get(), efreet_hostname_get());
#else
   snprintf(buf, sizeof(buf), "%s/efreet/mime_cache_%s.le.dat",
            efreet_cache_home_get(), efreet_hostname_get());
#endif
   mimedb = eina_file_open(buf, EINA_FALSE);
   if (mimedb)
     {
        mimedb_ptr = eina_file_map_all(mimedb, EINA_FILE_POPULATE);
        if (mimedb_ptr)
          {
             mimedb_size = eina_file_size_get(mimedb);
             if ((mimedb_size >= (16 + 4  + 4  + 4) &&
                 (!strncmp((char *)mimedb_ptr, "EfrEeT-MiMeS-001", 16))))
               {
                  // load ok - magic fine. more sanity checks?
               }
             else
               {
                  eina_file_map_free(mimedb, mimedb_ptr);
                  mimedb_ptr = NULL;
                  eina_file_close(mimedb);
                  mimedb = NULL;
                  mimedb_size = 0;
               }
          }
        else
          {
             eina_file_close(mimedb);
             mimedb = NULL;
          }
     }
}

static const char *
_efreet_mimedb_str_get(unsigned int offset)
{
   if (offset < (16 + 4 + 4 + 4)) return NULL;
   if (offset >= mimedb_size) return NULL;
   return (const char *)(mimedb_ptr + offset);
}

static unsigned int
_efreet_mimedb_uint_get(unsigned int index)
// index is the unit NUMBER AFTER the header
{
   unsigned int *ptr;
   ptr = ((unsigned int *)(mimedb_ptr + 16)) + index;
   if ((size_t)(((unsigned char *)ptr) - mimedb_ptr) >= (mimedb_size - 4))
     return 0;
   return *ptr;
}

static unsigned int
_efreet_mimedb_mime_count(void)
{
   return _efreet_mimedb_uint_get(0);
}

/**** currently unused - here for symmetry and future use
static const char *
_efreet_mimedb_mime_get(unsigned int num)
{
   unsigned int offset = _efreet_mimedb_uint_get
     (1 + num);
   return  _efreet_mimedb_str_get(offset);
}
*/

static unsigned int
_efreet_mimedb_extn_count(void)
{
   return _efreet_mimedb_uint_get(1 + _efreet_mimedb_mime_count());
}

static const char *
_efreet_mimedb_extn_get(unsigned int num)
{
   unsigned int offset = _efreet_mimedb_uint_get
     (1 + _efreet_mimedb_mime_count() + 1 + (num * 2));
   return  _efreet_mimedb_str_get(offset);
}

static const char *
_efreet_mimedb_extn_mime_get(unsigned int num)
{
   unsigned int offset = _efreet_mimedb_uint_get
     (1 + _efreet_mimedb_mime_count() + 1 + (num * 2) + 1);
   return  _efreet_mimedb_str_get(offset);
}

static const char *
_efreet_mimedb_extn_find(const char *extn)
{
   unsigned int i, begin, end;
   const char *s;

   // binary search keys to get value
   begin = 0;
   end = _efreet_mimedb_extn_count();
   i = (begin + end) / 2;
   for (;;)
     {
        s = _efreet_mimedb_extn_get(i);
        if (s)
          {
             int v = strcmp(extn, s);
             if (v < 0)
               {
                  end = i;
                  i = (begin + end) / 2;
                  if ((end - begin) == 0) break;
               }
             else if (v > 0)
               {
                  if ((end - begin) > 1)
                    {
                       begin = i;
                       i = (begin + end) / 2;
                       if (i == end) break;
                    }
                  else
                    {
                       if ((end - begin) == 0) break;
                       begin = end;
                       i = end;
                    }
               }
             else if (v == 0)
               return _efreet_mimedb_extn_mime_get(i);
          }
        else
          {
             break;
          }
     }
   return NULL;
}

static unsigned int
_efreet_mimedb_glob_count(void)
{
   return _efreet_mimedb_uint_get
     (1 + _efreet_mimedb_mime_count() +
      1 + (_efreet_mimedb_extn_count() * 2));
}

static const char *
_efreet_mimedb_glob_get(unsigned int num)
{
   unsigned int offset = _efreet_mimedb_uint_get
     (1 + _efreet_mimedb_mime_count() +
      1 + (_efreet_mimedb_extn_count() * 2) +
      1 + (num * 2));
   return  _efreet_mimedb_str_get(offset);
}

static const char *
_efreet_mimedb_glob_mime_get(unsigned int num)
{
   unsigned int offset = _efreet_mimedb_uint_get
     (1 + _efreet_mimedb_mime_count() +
      1 + (_efreet_mimedb_extn_count() * 2) +
      1 + (num * 2) + 1);
   return  _efreet_mimedb_str_get(offset);
}

/** --------------------------------- **/

EAPI int
efreet_mime_init(void)
{
   if (++_efreet_mime_init_count != 1)
     return _efreet_mime_init_count;

   if (!ecore_init())
     return --_efreet_mime_init_count;

   if (!ecore_file_init())
     goto shutdown_ecore;

   if (!efreet_init())
     goto shutdown_ecore_file;

   _efreet_mime_log_dom = eina_log_domain_register
      ("efreet_mime", EFREET_DEFAULT_LOG_COLOR);

   if (_efreet_mime_log_dom < 0)
     {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreet_mime.");
        goto shutdown_efreet;
     }

   efreet_mime_endianess = efreet_mime_endian_check();
   efreet_mime_type_cache_clear();

   _efreet_mimedb_update();

   if (!efreet_mime_init_files())
     goto unregister_log_domain;

   _efreet_mime_update_func = _efreet_mimedb_update;

   return _efreet_mime_init_count;

unregister_log_domain:
   eina_log_domain_unregister(_efreet_mime_log_dom);
   _efreet_mime_log_dom = -1;
shutdown_efreet:
   efreet_shutdown();
shutdown_ecore_file:
   ecore_file_shutdown();
shutdown_ecore:
   ecore_shutdown();

   return --_efreet_mime_init_count;
}

EAPI int
efreet_mime_shutdown(void)
{
   if (_efreet_mime_init_count == 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--_efreet_mime_init_count != 0)
     return _efreet_mime_init_count;

   _efreet_mimedb_shutdown();
   _efreet_mime_update_func = NULL;

   efreet_mime_icons_debug();

   IF_RELEASE(_mime_inode_symlink);
   IF_RELEASE(_mime_inode_fifo);
   IF_RELEASE(_mime_inode_chardevice);
   IF_RELEASE(_mime_inode_blockdevice);
   IF_RELEASE(_mime_inode_socket);
   IF_RELEASE(_mime_inode_mountpoint);
   IF_RELEASE(_mime_inode_directory);
   IF_RELEASE(_mime_application_x_executable);
   IF_RELEASE(_mime_application_octet_stream);
   IF_RELEASE(_mime_text_plain);

   IF_FREE_LIST(magics, efreet_mime_magic_free);
   IF_FREE_HASH(mime_icons);
   eina_log_domain_unregister(_efreet_mime_log_dom);
   _efreet_mime_log_dom = -1;
   efreet_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();

   return _efreet_mime_init_count;
}

EAPI const char *
efreet_mime_type_get(const char *file)
{
   const char *type = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   if (!mimedb_ptr) return NULL;

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

EAPI const char *
efreet_mime_type_icon_get(const char *mime, const char *theme, unsigned int size)
{
   const char *icon = NULL;
   char *data;
   Eina_List *icons  = NULL;
   const char *env = NULL;
   char *p = NULL, *pp = NULL, *ppp = NULL;
   char buf[PATH_MAX];
   const char *cache;

   EINA_SAFETY_ON_NULL_RETURN_VAL(mime, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(theme, NULL);

   mime = eina_stringshare_add(mime);
   theme = eina_stringshare_add(theme);
   cache = efreet_mime_icon_entry_find(mime, theme, size);
   if (cache)
     {
        eina_stringshare_del(mime);
        eina_stringshare_del(theme);
        return cache;
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

        snprintf(buf, sizeof(buf), "%s-x-generic", pp);
        icons = eina_list_append(icons, strdup(buf));

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
     {
        eina_hash_free(mime_icons);
        mime_icons_lru = NULL;
     }
   mime_icons = eina_hash_stringshared_new(EINA_FREE_CB(efreet_mime_icon_entry_head_free));
}

EAPI void
efreet_mime_type_cache_flush(void)
{
   efreet_mime_icons_flush(ecore_loop_time_get());
}


EAPI const char *
efreet_mime_magic_type_get(const char *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   return efreet_mime_magic_check_priority(file, 0, 0);
}

EAPI const char *
efreet_mime_globs_type_get(const char *file)
{
   char *sl, *p;
   const char *s, *mime;
   char *ext;
   unsigned int i, n;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   if (!mimedb_ptr) return NULL;

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
             if (p && (mime = _efreet_mimedb_extn_find(p))) return mime;
             p = strchr(p, '.');
          }
     }

   // Fallback to the other globs if not found
   n = _efreet_mimedb_glob_count();
   for (i = 0; i < n; i++)
     {
        s = _efreet_mimedb_glob_get(i);
        if (efreet_mime_glob_match(file, s))
          return _efreet_mimedb_glob_mime_get(i);
     }
   ext = alloca(strlen(file) + 1);
   for (s = file, p = ext; *s; s++, p++) *p = tolower(*s);
   *p = 0;
   for (i = 0; i < n; i++)
     {
        s = _efreet_mimedb_glob_get(i);
        if (efreet_mime_glob_case_match(ext, s))
          return _efreet_mimedb_glob_mime_get(i);
     }
   return NULL;
}

EAPI const char *
efreet_mime_special_type_get(const char *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   return efreet_mime_special_check(file);
}

EAPI const char *
efreet_mime_fallback_type_get(const char *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
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
 * @param datadirs List of XDG data dirs
 * @param datahome Path to XDG data home directory
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
 * @param datadirs List of XDG data dirs
 * @param datahome Path to XDG data home directory
 * @return Returns 1 on success, 0 on failure
 * @brief Initializes globs, magics, and monitors lists.
 */
static int
efreet_mime_init_files(void)
{
   Eina_List *datadirs = NULL;
   const char *datahome;

   if (!(datahome = efreet_data_home_get()))
     return 0;

   if (!(datadirs = efreet_data_dirs_get()))
     return 0;

   efreet_mime_load_magics(datadirs, datahome);

   _mime_inode_symlink            = eina_stringshare_add("inode/symlink");
   _mime_inode_fifo               = eina_stringshare_add("inode/fifo");
   _mime_inode_chardevice         = eina_stringshare_add("inode/chardevice");
   _mime_inode_blockdevice        = eina_stringshare_add("inode/blockdevice");
   _mime_inode_socket             = eina_stringshare_add("inode/socket");
   _mime_inode_mountpoint         = eina_stringshare_add("inode/mountpoint");
   _mime_inode_directory          = eina_stringshare_add("inode/directory");
   _mime_application_x_executable = eina_stringshare_add("application/x-executable");
   _mime_application_octet_stream = eina_stringshare_add("application/octet-stream");
   _mime_text_plain               = eina_stringshare_add("text/plain");

   return 1;
}

/**
 * @internal
 * @param file File to examine
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

   /* no symlink on Windows */
#ifdef _WIN32
   if (!stat(file, &s))
#else
     if (!lstat(file, &s))
#endif
       {
          if (S_ISREG(s.st_mode))
            return NULL;

#ifndef _WIN32
          if (S_ISLNK(s.st_mode))
            return _mime_inode_symlink;
#endif

          if (S_ISFIFO(s.st_mode))
            return _mime_inode_fifo;

          if (S_ISCHR(s.st_mode))
            return _mime_inode_chardevice;

          if (S_ISBLK(s.st_mode))
            return _mime_inode_blockdevice;

#ifndef _WIN32
          if (S_ISSOCK(s.st_mode))
            return _mime_inode_socket;
#endif

          if (S_ISDIR(s.st_mode))
            {
               struct stat s2;
               char parent[PATH_MAX];
               char path[PATH_MAX];

               strncpy(path, file, PATH_MAX);
               path[PATH_MAX - 1] = '\0';

               path_len = strlen(file);
               strncpy(parent, path, PATH_MAX);
               parent[PATH_MAX - 1] = '\0';

               /* Kill any trailing slash */
               if (parent[path_len - 1] == '/')
                 parent[--path_len] = '\0';

               /* Truncate to last slash */
               while ((path_len > 0) &&
                      (parent[--path_len] != '/'))
                 parent[path_len] = '\0';

#ifdef _WIN32
               if (!stat(file, &s2))
#else
               if (!lstat(parent, &s2))
#endif
                 {
                    if (s.st_dev != s2.st_dev)
                      return _mime_inode_mountpoint;
                 }

               return _mime_inode_directory;
            }

          return NULL;
       }

   return NULL;
}

/**
 * @internal
 * @param file File to examine
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

   if (ecore_file_can_exec(file))
     return _mime_application_x_executable;

   if (!(f = fopen(file, "rb"))) return NULL;

   i = fread(buf, 1, sizeof(buf), f);
   fclose(f);

   if (i == 0) return _mime_application_octet_stream;

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
          return _mime_application_octet_stream;
     }

   return _mime_text_plain;
}

/**
 * @internal
 * @param in Number to count the digits
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
 * @param file File to parse
 * @return Returns no value
 * @brief Loads a magic file and adds information to magics list
 */
static void
efreet_mime_shared_mimeinfo_magic_load(const char *file)
{
   Eina_File *f;
   void *data;

   if (!file) return;

   f = eina_file_open(file, EINA_FALSE);
   if (!f) return ;

   data = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!data) goto end;

   efreet_mime_shared_mimeinfo_magic_parse(data, eina_file_size_get(f));

   eina_file_map_free(f, data);
 end:
   eina_file_close(f);
}

/**
 * @param data The data from the file
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
 * quicker, uses less memory and will achieve the same exact effect.
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
             ptr++;
          }
        else
          {
             short tshort;

             if (!mime) continue;
             if (!entry)
               {
                  if (!(entry = NEW(Efreet_Mime_Magic_Entry, 1)))
                    {
                       IF_FREE_LIST(magics, efreet_mime_magic_free);
                       return;
                    }

                  entry->indent = 0;
                  entry->offset = 0;
                  entry->value_len = 0;
                  entry->word_size = 1;
                  entry->range_len = 1;
                  entry->mask = NULL;
                  entry->value = NULL;

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

                   tshort = 0;
                   memcpy(&tshort, ptr, sizeof(short));
                   entry->value_len = ntohs(tshort);
                   ptr += 2;

                   entry->value = NEW(char, entry->value_len);
                   memcpy(entry->value, ptr, entry->value_len);
                   ptr += entry->value_len;
                   break;

                case '&':
                   ptr++;
                   entry->mask = NEW(char, entry->value_len);
                   memcpy(entry->mask, ptr, entry->value_len);
                   ptr += entry->value_len;
                   break;

                case '~':
                   ptr++;
                   entry->word_size = atoi(ptr);
                   if ((entry->word_size != 0) && (((entry->word_size != 1)
                                                    && (entry->word_size != 2)
                                                    && (entry->word_size != 4))
                                                   || (entry->value_len % entry->word_size)))
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
   /*
      if (entry)
      {
      IF_FREE(entry->value);
      IF_FREE(entry->mask);
      FREE(entry);
      }
    */
}

/**
 * @internal
 * @param file File to check
 * @param start Start priority, if 0 start at beginning
 * @param end End priority, should be less then start
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
   Eina_File *f = NULL;
   const char *mem = NULL;
   size_t sz;
   unsigned int i = 0, offset = 0,level = 0, match = 0;
   const char *last_mime = NULL;
   int c;
   char v;

   if (!magics) return NULL;

   f = eina_file_open(file, EINA_FALSE);
   if (!f) return NULL;

   mem = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!mem) goto end;

   sz = eina_file_size_get(f);

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
               {
                  level = e->indent;
               }
             else if ((level > e->indent) && match)
               {
                  goto end;
               }

             for (offset = e->offset; offset < e->offset + e->range_len; offset++)
               {
                  if (offset + e->value_len >= sz) break;

                  match = 1;
                  for (i = 0; i < e->value_len; ++i)
                    {
                       c = mem[offset + i];

                       v = e->value[i];
                       if (e->mask) c &= e->mask[i];

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

        if (match) break;
     }

 end:
   if (mem) eina_file_map_free(f, (void*) mem);

   eina_file_close(f);

   return last_mime;
}

/**
 * @internal
 * @param data Data pointer that is being destroyed
 * @return Returns no value
 * @brief Callback for magics destroy
 */
static void
efreet_mime_magic_free(void *data)
{
   Efreet_Mime_Magic *m = data;

   IF_RELEASE(m->mime);
   IF_FREE_LIST(m->entries, efreet_mime_magic_entry_free);
   IF_FREE(m);
}

/**
 * @internal
 * @param data Data pointer that is being destroyed
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
 * @param str String (filename) to match
 * @param glob Glob to match str to
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
 * @param str String (filename) to match
 * @param glob Glob to match str to
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

static void
efreet_mime_icons_flush(double now)
{
   Eina_Inlist *l;
   static double old = 0;
   int todo;

   if (now - old < EFREET_MIME_ICONS_FLUSH_TIMEOUT)
     return;
   old = now;

   todo = eina_hash_population(mime_icons) - EFREET_MIME_ICONS_MAX_POPULATION;
   if (todo <= 0)
     return;

   l = mime_icons_lru->last; /* mime_icons_lru is not NULL, since todo > 0 */
   for (; todo > 0; todo--)
     {
        Efreet_Mime_Icon_Entry_Head *entry = (Efreet_Mime_Icon_Entry_Head *)l;
        Eina_Inlist *prev = l->prev;

        mime_icons_lru = eina_inlist_remove(mime_icons_lru, l);
        eina_hash_del_by_key(mime_icons, entry->mime);
        l = prev;
     }

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
     {
        Eina_Inlist *l;

        l = EINA_INLIST_GET(n);
        entry->list = eina_inlist_prepend(entry->list, l);

        l = EINA_INLIST_GET(entry);
        mime_icons_lru = eina_inlist_promote(mime_icons_lru, l);
     }
   else
     {
        Eina_Inlist *l;

        entry = malloc(sizeof(*entry));
        if (!entry)
          {
             efreet_mime_icon_entry_free(n);
             return;
          }

        l = EINA_INLIST_GET(n);
        entry->list = eina_inlist_prepend(NULL, l);
        entry->mime = mime;
        eina_hash_direct_add(mime_icons, mime, entry);

        l = EINA_INLIST_GET(entry);
        mime_icons_lru = eina_inlist_prepend(mime_icons_lru, l);
     }

   entry->timestamp = ecore_loop_time_get();
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
             Eina_Inlist *l;

             l = EINA_INLIST_GET(n);
             if (entry->list != l)
               entry->list = eina_inlist_promote(entry->list, l);

             l = EINA_INLIST_GET(entry);
             if (mime_icons_lru != l)
               mime_icons_lru = eina_inlist_promote(mime_icons_lru, l);

             entry->timestamp = ecore_loop_time_get();
             return n->icon;
          }
     }

   return NULL;
}

#ifdef EFREET_MIME_ICONS_DEBUG
static void
efreet_mime_icons_debug(void)
{
   double now = ecore_loop_time_get();
   Efreet_Mime_Icon_Entry_Head *entry;
   EINA_INLIST_FOREACH(mime_icons_lru, entry)
     {
        Efreet_Mime_Icon_Entry *n;

        if ((now > 0) &&
            (now - entry->timestamp >= EFREET_MIME_ICONS_EXPIRE_TIMEOUT))
          {
             puts("*** FOLLOWING ENTRIES ARE AGED AND CAN BE EXPIRED ***");
             now = 0;
          }

        DBG("mime-icon entry: '%s' last used: %s",
            entry->mime, ctime(&entry->timestamp));

        EINA_INLIST_FOREACH(entry->list, n)
           DBG("\tsize: %3u theme: '%s' icon: '%s'",
               n->theme, n->size, n->icon);
     }
}
#else
static void
efreet_mime_icons_debug(void)
{
}
#endif
