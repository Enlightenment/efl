#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef _WIN32
# include <winsock2.h>
#endif /* ifdef _WIN32 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <zlib.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif /* ifdef HAVE_EVIL */

#include <Eina.h>

#ifdef HAVE_GNUTLS
# include <gnutls/gnutls.h>
# include <gcrypt.h>
#endif /* ifdef HAVE_GNUTLS */

#ifdef HAVE_OPENSSL
# include <openssl/err.h>
# include <openssl/evp.h>
#endif /* ifdef HAVE_OPENSSL */

#ifdef HAVE_GNUTLS
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif /* ifdef HAVE_GNUTLS */

#include "Eet.h"
#include "Eet_private.h"

#include "lz4.h"
#include "lz4hc.h"

#ifndef O_BINARY
# define O_BINARY 0
#endif

static Eet_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Eet_Version *eet_version = &_version;

#ifdef HAVE_REALPATH
# undef HAVE_REALPATH
#endif /* ifdef HAVE_REALPATH */

#define EET_MAGIC_FILE        0x1ee7ff00
#define EET_MAGIC_FILE_HEADER 0x1ee7ff01

#define EET_MAGIC_FILE2       0x1ee70f42

#define EET_FILE2_HEADER_COUNT           3
#define EET_FILE2_DIRECTORY_ENTRY_COUNT  6
#define EET_FILE2_DICTIONARY_ENTRY_COUNT 5

#define EET_FILE2_HEADER_SIZE            (sizeof(int) * \
                                          EET_FILE2_HEADER_COUNT)
#define EET_FILE2_DIRECTORY_ENTRY_SIZE   (sizeof(int) * \
                                          EET_FILE2_DIRECTORY_ENTRY_COUNT)
#define EET_FILE2_DICTIONARY_ENTRY_SIZE  (sizeof(int) * \
                                          EET_FILE2_DICTIONARY_ENTRY_COUNT)

/* prototypes of internal calls */
static Eet_File *
eet_cache_find(const char *path,
               Eet_File  **cache,
               int         cache_num);
static void
eet_cache_add(Eet_File   *ef,
              Eet_File ***cache,
              int        *cache_num,
              int        *cache_alloc);
static void
eet_cache_del(Eet_File   *ef,
              Eet_File ***cache,
              int        *cache_num,
              int        *cache_alloc);
static int
eet_string_match(const char *s1,
                 const char *s2);
#if 0 /* Unused */
static Eet_Error
eet_flush(Eet_File *ef);
#endif /* if 0 */
static Eet_Error
 eet_flush2(Eet_File *ef);
static Eet_File_Node *
 find_node_by_name(Eet_File  *ef,
                  const char *name);
static int
read_data_from_disk(Eet_File      *ef,
                    Eet_File_Node *efn,
                    void          *buf,
                    int            len);

static Eet_Error
eet_internal_close(Eet_File *ef,
                   Eina_Bool locked);

static Eina_Lock eet_cache_lock;

#define LOCK_CACHE   eina_lock_take(&eet_cache_lock)
#define UNLOCK_CACHE eina_lock_release(&eet_cache_lock)

#define INIT_FILE(File)    eina_lock_new(&File->file_lock)
#define LOCK_FILE(File)    eina_lock_take(&File->file_lock)
#define UNLOCK_FILE(File)  eina_lock_release(&File->file_lock)
#define DESTROY_FILE(File) eina_lock_free(&File->file_lock)

/* cache. i don't expect this to ever be large, so arrays will do */
static int eet_writers_num = 0;
static int eet_writers_alloc = 0;
static Eet_File **eet_writers = NULL;
static int eet_readers_num = 0;
static int eet_readers_alloc = 0;
static Eet_File **eet_readers = NULL;
static int eet_init_count = 0;

/* log domain variable */
int _eet_log_dom_global = -1;

/* Check to see its' an eet file pointer */
static inline int
eet_check_pointer(const Eet_File *ef)
{
   if ((!ef) || (ef->magic != EET_MAGIC_FILE))
     return 1;

   return 0;
}

static inline int
eet_check_header(const Eet_File *ef)
{
   if (!ef->header)
     return 1;

   if (!ef->header->directory)
     return 1;

   return 0;
}

static inline int
eet_test_close(int       test,
               Eet_File *ef)
{
   if (test)
     {
        ef->delete_me_now = 1;
        eet_internal_close(ef, EINA_TRUE);
     }

   return test;
}

/* find an eet file in the currently in use cache */
static Eet_File *
eet_cache_find(const char *path,
               Eet_File  **cache,
               int         cache_num)
{
   int i;

   /* walk list */
   for (i = 0; i < cache_num; i++)
     {
        /* if matches real path - return it */
         if (eet_string_match(cache[i]->path, path))
           if (!cache[i]->delete_me_now)
             return cache[i];
     }

   /* not found */
   return NULL;
}

/* add to end of cache */
/* this should only be called when the cache lock is already held */
static void
eet_cache_add(Eet_File   *ef,
              Eet_File ***cache,
              int        *cache_num,
              int        *cache_alloc)
{
   Eet_File **new_cache;
   int new_cache_num;
   int new_cache_alloc;

   new_cache_num = *cache_num;
   if (new_cache_num >= 64) /* avoid fd overruns - limit to 128 (most recent) in the cache */
     {
        Eet_File *del_ef = NULL;
        int i;

        new_cache = *cache;
        for (i = 0; i < new_cache_num; i++)
          {
             if (new_cache[i]->references == 0)
               {
                  del_ef = new_cache[i];
                  break;
               }
          }

        if (del_ef)
          {
             del_ef->delete_me_now = 1;
             eet_internal_close(del_ef, EINA_TRUE);
          }
     }

   new_cache = *cache;
   new_cache_num = *cache_num;
   new_cache_alloc = *cache_alloc;
   new_cache_num++;
   if (new_cache_num > new_cache_alloc)
     {
        new_cache_alloc += 16;
        new_cache = realloc(new_cache, new_cache_alloc * sizeof(Eet_File *));
        if (!new_cache)
          {
             CRI("BAD ERROR! Eet realloc of cache list failed. Abort");
             abort();
          }
     }

   new_cache[new_cache_num - 1] = ef;
   *cache = new_cache;
   *cache_num = new_cache_num;
   *cache_alloc = new_cache_alloc;
}

/* delete from cache */
/* this should only be called when the cache lock is already held */
static void
eet_cache_del(Eet_File   *ef,
              Eet_File ***cache,
              int        *cache_num,
              int        *cache_alloc)
{
   Eet_File **new_cache;
   int new_cache_num, new_cache_alloc;
   int i, j;

   new_cache = *cache;
   new_cache_num = *cache_num;
   new_cache_alloc = *cache_alloc;
   if (new_cache_num <= 0)
     return;

   for (i = 0; i < new_cache_num; i++)
     {
        if (new_cache[i] == ef)
          break;
     }

   if (i >= new_cache_num)
     return;

   new_cache_num--;
   for (j = i; j < new_cache_num; j++)
     new_cache[j] = new_cache[j + 1];

   if (new_cache_num <= (new_cache_alloc - 16))
     {
        new_cache_alloc -= 16;
        if (new_cache_num > 0)
          {
             new_cache = realloc(new_cache, new_cache_alloc * sizeof(Eet_File *));
             if (!new_cache)
               {
                  CRI("BAD ERROR! Eet realloc of cache list failed. Abort");
                  abort();
               }
          }
        else
          {
             free(new_cache);
             new_cache = NULL;
          }
     }

   *cache = new_cache;
   *cache_num = new_cache_num;
   *cache_alloc = new_cache_alloc;
}

/* internal string match. null friendly, catches same ptr */
static int
eet_string_match(const char *s1,
                 const char *s2)
{
   /* both null- no match */
    if ((!s1) || (!s2))
      return 0;

    if (s1 == s2)
      return 1;

    return !strcmp(s1, s2);
}

/* flush out writes to a v2 eet file */
static Eet_Error
eet_flush2(Eet_File *ef)
{
   Eet_File_Node *efn;
   FILE *fp;
   Eet_Error error = EET_ERROR_NONE;
   int head[EET_FILE2_HEADER_COUNT];
   int num_directory_entries = 0;
   int num_dictionary_entries = 0;
   int bytes_directory_entries = 0;
   int bytes_dictionary_entries = 0;
   int bytes_strings = 0;
   int data_offset = 0;
   int strings_offset = 0;
   int num;
   int i;
   int j;

   if (eet_check_pointer(ef))
     return EET_ERROR_BAD_OBJECT;

   if (eet_check_header(ef))
     return EET_ERROR_EMPTY;

   if (!ef->writes_pending)
     return EET_ERROR_NONE;

   if ((ef->mode == EET_FILE_MODE_READ_WRITE)
       || (ef->mode == EET_FILE_MODE_WRITE))
     {
        int fd;

        /* opening for write - delete old copy of file right away */
        unlink(ef->path);
        fd = open(ef->path, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IRUSR | S_IWUSR);
        if (fd < 0) 
          return EET_ERROR_NOT_WRITABLE;

        fp = fdopen(fd, "wb");
        if (!fp)
          return EET_ERROR_NOT_WRITABLE;

        fcntl(fd, F_SETFD, FD_CLOEXEC);
     }
   else
     return EET_ERROR_NOT_WRITABLE;

   /* calculate string base offset and data base offset */
   num = (1 << ef->header->directory->size);
   for (i = 0; i < num; ++i)
     {
        for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
          {
             num_directory_entries++;
             bytes_strings += strlen(efn->name) + 1;
          }
     }
   if (ef->ed)
     {
        num_dictionary_entries = ef->ed->count;

        for (i = 0; i < num_dictionary_entries; ++i)
          bytes_strings += ef->ed->all[i].len;
     }

   /* calculate section bytes size */
   bytes_directory_entries = EET_FILE2_DIRECTORY_ENTRY_SIZE *
     num_directory_entries + EET_FILE2_HEADER_SIZE;
   bytes_dictionary_entries = EET_FILE2_DICTIONARY_ENTRY_SIZE *
     num_dictionary_entries;

   /* go thru and write the header */
   head[0] = (int)htonl((unsigned int)EET_MAGIC_FILE2);
   head[1] = (int)htonl((unsigned int)num_directory_entries);
   head[2] = (int)htonl((unsigned int)num_dictionary_entries);

   fseek(fp, 0, SEEK_SET);
   if (fwrite(head, sizeof (head), 1, fp) != 1)
     goto write_error;

   /* calculate per entry base offset */
   strings_offset = bytes_directory_entries + bytes_dictionary_entries;
   data_offset = bytes_directory_entries + bytes_dictionary_entries +
     bytes_strings;

   /* write directories entry */
   for (i = 0; i < num; i++)
     {
        for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
          {
             unsigned int flag;
             int ibuf[EET_FILE2_DIRECTORY_ENTRY_COUNT];

             flag = (efn->alias << 2) | (efn->ciphered << 1) | efn->compression;
             flag |= efn->compression_type << 3;

             efn->offset = data_offset;

             ibuf[0] = (int)htonl((unsigned int)data_offset);
             ibuf[1] = (int)htonl((unsigned int)efn->size);
             ibuf[2] = (int)htonl((unsigned int)efn->data_size);
             ibuf[3] = (int)htonl((unsigned int)strings_offset);
             ibuf[4] = (int)htonl((unsigned int)efn->name_size);
             ibuf[5] = (int)htonl((unsigned int)flag);

             strings_offset += efn->name_size;
             data_offset += efn->size;

             if (fwrite(ibuf, sizeof(ibuf), 1, fp) != 1)
               goto write_error;
          }
     }

   /* write dictionary */
   if (ef->ed)
     {
        int offset = strings_offset;

        /* calculate dictionary strings offset */
        ef->ed->offset = strings_offset;

        for (j = 0; j < ef->ed->count; ++j)
          {
             int sbuf[EET_FILE2_DICTIONARY_ENTRY_COUNT];
	     int prev = 0;

             // We still use the prev as an hint for knowing if it is the head of the hash
	     if (ef->ed->hash[ef->ed->all_hash[j]] == j)
	       prev = -1;

             sbuf[0] = (int)htonl((unsigned int)ef->ed->all_hash[j]);
             sbuf[1] = (int)htonl((unsigned int)offset);
             sbuf[2] = (int)htonl((unsigned int)ef->ed->all[j].len);
             sbuf[3] = (int)htonl((unsigned int)prev);
             sbuf[4] = (int)htonl((unsigned int)ef->ed->all[j].next);

             offset += ef->ed->all[j].len;

             if (fwrite(sbuf, sizeof (sbuf), 1, fp) != 1)
               goto write_error;
          }
     }

   /* write directories name */
   for (i = 0; i < num; i++)
     {
        for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
          {
             if (fwrite(efn->name, efn->name_size, 1, fp) != 1)
               goto write_error;
          }
     }

   /* write strings */
   if (ef->ed)
     for (j = 0; j < ef->ed->count; ++j)
       {
          if (fwrite(ef->ed->all[j].str, ef->ed->all[j].len, 1, fp) != 1)
            goto write_error;
       }

   /* write data */
   for (i = 0; i < num; i++)
     {
        for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
          {
             if (fwrite(efn->data, efn->size, 1, fp) != 1)
               goto write_error;
          }
     }

   /* flush all write to the file. */
   fflush(fp);

   /* append signature if required */
   if (ef->key)
     {
        error = eet_identity_sign(fp, ef->key);
        if (error != EET_ERROR_NONE)
          goto sign_error;
     }

   /* no more writes pending */
   ef->writes_pending = 0;

   fclose(fp);

   return EET_ERROR_NONE;

write_error:
   if (ferror(fp))
     {
        switch (errno)
          {
           case EFBIG: error = EET_ERROR_WRITE_ERROR_FILE_TOO_BIG; break;

           case EIO: error = EET_ERROR_WRITE_ERROR_IO_ERROR; break;

           case ENOSPC: error = EET_ERROR_WRITE_ERROR_OUT_OF_SPACE; break;

           case EPIPE: error = EET_ERROR_WRITE_ERROR_FILE_CLOSED; break;

           default: error = EET_ERROR_WRITE_ERROR; break;
          }
     }

sign_error:
   fclose(fp);
   return error;
}

EAPI int
eet_init(void)
{
   if (++eet_init_count != 1)
     return eet_init_count;

   if (!eina_init())
     return --eet_init_count;

   _eet_log_dom_global = eina_log_domain_register("eet", EET_DEFAULT_LOG_COLOR);
   if (_eet_log_dom_global < 0)
     {
        EINA_LOG_ERR("Eet Can not create a general log domain.");
        goto shutdown_eina;
     }

   eina_lock_new(&eet_cache_lock);

   if (!eet_mempool_init())
     {
        EINA_LOG_ERR("Eet: Eet_Node mempool creation failed");
        goto unregister_log_domain;
     }

   if (!eet_node_init())
     {
        EINA_LOG_ERR("Eet: Eet_Node mempool creation failed");
        goto shutdown_mempool;
     }

#ifdef HAVE_GNUTLS
   /* Before the library can be used, it must initialize itself if needed. */
   if (gcry_control(GCRYCTL_ANY_INITIALIZATION_P) == 0)
     {
        gcry_check_version(NULL);
        /* Disable warning messages about problems with the secure memory subsystem.
           This command should be run right after gcry_check_version. */
        if (gcry_control(GCRYCTL_DISABLE_SECMEM_WARN))
          goto shutdown_eet;  /* This command is used to allocate a pool of secure memory and thus
                                 enabling the use of secure memory. It also drops all extra privileges the
                                 process has (i.e. if it is run as setuid (root)). If the argument nbytes
                                 is 0, secure memory will be disabled. The minimum amount of secure memory
                                 allocated is currently 16384 bytes; you may thus use a value of 1 to
                                 request that default size. */

        if (gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0))
          WRN(
            "BIG FAT WARNING: I AM UNABLE TO REQUEST SECMEM, Cryptographic operation are at risk !");
     }

   if (gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread))
     WRN(
       "YOU ARE USING PTHREADS, BUT I CANNOT INITIALIZE THREADSAFE GCRYPT OPERATIONS!");

   if (gnutls_global_init())
     goto shutdown_eet;

#endif /* ifdef HAVE_GNUTLS */
#ifdef HAVE_OPENSSL
   ERR_load_crypto_strings();
   OpenSSL_add_all_algorithms();
#endif /* ifdef HAVE_OPENSSL */

   eina_log_timing(_eet_log_dom_global,
		   EINA_LOG_STATE_STOP,
		   EINA_LOG_STATE_INIT);

   return eet_init_count;

#ifdef HAVE_GNUTLS
shutdown_eet:
   eet_node_shutdown();
#endif
shutdown_mempool:
   eet_mempool_shutdown();
unregister_log_domain:
   eina_log_domain_unregister(_eet_log_dom_global);
   _eet_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --eet_init_count;
}

EAPI int
eet_shutdown(void)
{
   if (eet_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--eet_init_count != 0)
     return eet_init_count;

   eina_log_timing(_eet_log_dom_global,
		   EINA_LOG_STATE_START,
		   EINA_LOG_STATE_SHUTDOWN);

   eet_clearcache();

   if (eet_writers_num || eet_readers_num)
     {
        Eet_File **closelist = NULL;
        int num = 0;
        int i;

        closelist = alloca((eet_writers_num + eet_readers_num)
                           * sizeof(Eet_File *));
        for (i = 0; i < eet_writers_num; i++)
          {
             closelist[num++] = eet_writers[i];
             eet_writers[i]->delete_me_now = 1;
          }

        for (i = 0; i < eet_readers_num; i++)
          {
             closelist[num++] = eet_readers[i];
             eet_readers[i]->delete_me_now = 1;
          }

        for (i = 0; i < num; i++)
          {
             ERR("File '%s' is still open %i times !", closelist[i]->path, closelist[i]->references);
             eet_internal_close(closelist[i], EINA_TRUE);
          }
     }
   eet_node_shutdown();
   eet_mempool_shutdown();

   eina_lock_free(&eet_cache_lock);

#ifdef HAVE_GNUTLS
   /* Note that gnutls has a leak where it doesnt free stuff it alloced
    * on init. valgrind trace here:
    * 21 bytes in 1 blocks are definitely lost in loss record 24 of 194
    *    at 0x4C2B6CD: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
    *    by 0x68AC801: strdup (strdup.c:43)
    *    by 0xD215B6A: p11_kit_registered_module_to_name (in /usr/lib/x86_64-linux-gnu/libp11-kit.so.0.0.0)
    *    by 0x9571574: gnutls_pkcs11_init (in /usr/lib/x86_64-linux-gnu/libgnutls.so.26.21.8)
    *    by 0x955B031: gnutls_global_init (in /usr/lib/x86_64-linux-gnu/libgnutls.so.26.21.8)
    *    by 0x6DFD6D0: eet_init (eet_lib.c:608)
    * 
    * yes - i've tried calling gnutls_pkcs11_deinit() by hand but no luck.
    * the leak is in there.
    */
   gnutls_global_deinit();
#endif /* ifdef HAVE_GNUTLS */
#ifdef HAVE_OPENSSL
   EVP_cleanup();
   ERR_free_strings();
#endif /* ifdef HAVE_OPENSSL */
   eina_log_domain_unregister(_eet_log_dom_global);
   _eet_log_dom_global = -1;
   eina_shutdown();

   return eet_init_count;
}

EAPI Eet_Error
eet_sync(Eet_File *ef)
{
   Eet_Error ret;

   if (eet_check_pointer(ef))
     return EET_ERROR_BAD_OBJECT;

   if ((ef->mode != EET_FILE_MODE_WRITE) &&
       (ef->mode != EET_FILE_MODE_READ_WRITE))
     return EET_ERROR_NOT_WRITABLE;

   if (!ef->writes_pending)
     return EET_ERROR_NONE;

   LOCK_FILE(ef);

   ret = eet_flush2(ef);

   UNLOCK_FILE(ef);
   return ret;
}

EAPI void
eet_clearcache(void)
{
   int num = 0;
   int i;

   /*
    * We need to compute the list of eet file to close separately from the cache,
    * due to eet_close removing them from the cache after each call.
    */
   LOCK_CACHE;
   for (i = 0; i < eet_writers_num; i++)
     {
        if (eet_writers[i]->references <= 0)
          num++;
     }

   for (i = 0; i < eet_readers_num; i++)
     {
        if (eet_readers[i]->references <= 0)
          num++;
     }

   if (num > 0)
     {
        Eet_File **closelist = NULL;

        closelist = alloca(num * sizeof(Eet_File *));
        num = 0;
        for (i = 0; i < eet_writers_num; i++)
          {
             if (eet_writers[i]->references <= 0)
               {
                  closelist[num] = eet_writers[i];
                  eet_writers[i]->delete_me_now = 1;
                  num++;
               }
          }

        for (i = 0; i < eet_readers_num; i++)
          {
             if (eet_readers[i]->references <= 0)
               {
                  closelist[num] = eet_readers[i];
                  eet_readers[i]->delete_me_now = 1;
                  num++;
               }
          }

        for (i = 0; i < num; i++)
          {
             eet_internal_close(closelist[i], EINA_TRUE);
          }
     }

   UNLOCK_CACHE;
}

/* FIXME: MMAP race condition in READ_WRITE_MODE */
static Eet_File *
eet_internal_read2(Eet_File *ef)
{
   const int *data = (const int *)ef->data;
   const char *start = (const char *)ef->data;
   int idx = 0;
   unsigned long int bytes_directory_entries;
   unsigned long int bytes_dictionary_entries;
   unsigned long int signature_base_offset;
   unsigned long int num_directory_entries;
   unsigned long int num_dictionary_entries;
   unsigned int i;

   idx += sizeof(int);
   if (eet_test_close((int)ntohl(*data) != EET_MAGIC_FILE2, ef))
     return NULL;

   data++;

#define GET_INT(Value, Pointer, Index) \
  {                                    \
     Value = ntohl(*Pointer);          \
     Pointer++;                        \
     Index += sizeof(int);             \
  }

   /* get entries count and byte count */
   GET_INT(num_directory_entries, data, idx);
   /* get dictionary count and byte count */
   GET_INT(num_dictionary_entries, data, idx);

   bytes_directory_entries = EET_FILE2_DIRECTORY_ENTRY_SIZE *
     num_directory_entries + EET_FILE2_HEADER_SIZE;
   bytes_dictionary_entries = EET_FILE2_DICTIONARY_ENTRY_SIZE *
     num_dictionary_entries;

   /* we can't have > 0x7fffffff values here - invalid */
   if (eet_test_close((num_directory_entries > 0x7fffffff), ef))
     return NULL;

   /* we can't have more bytes directory and bytes in dictionaries than the size of the file */
   if (eet_test_close((bytes_directory_entries + bytes_dictionary_entries) >
                      ef->data_size, ef))
     return NULL;

   /* allocate header */
   ef->header = eet_file_header_calloc(1);
   if (eet_test_close(!ef->header, ef))
     return NULL;

   ef->header->magic = EET_MAGIC_FILE_HEADER;

   /* allocate directory block in ram */
   ef->header->directory = eet_file_directory_calloc(1);
   if (eet_test_close(!ef->header->directory, ef))
     return NULL;

   /* 8 bit hash table (256 buckets) */
   ef->header->directory->size = 8;
   /* allocate base hash table */
   ef->header->directory->nodes =
     calloc(1, sizeof(Eet_File_Node *) * (1 << ef->header->directory->size));
   if (eet_test_close(!ef->header->directory->nodes, ef))
     return NULL;

   signature_base_offset = 0;
   if (num_directory_entries == 0)
     {
        signature_base_offset = ef->data_size;
     }

   /* actually read the directory block - all of it, into ram */
   for (i = 0; i < num_directory_entries; ++i)
     {
        const char *name;
        Eet_File_Node *efn;
        unsigned long int name_offset;
        unsigned long int name_size;
        int hash;
        int flag;

        /* out directory block is inconsistent - we have overrun our */
        /* dynamic block buffer before we finished scanning dir entries */
        efn = eet_file_node_malloc(1);
        if (eet_test_close(!efn, ef))
          {
             if (efn) eet_file_node_mp_free(efn);  /* yes i know - we only get here if
                                   * efn is null/0 -> trying to shut up
                                   * warning tools like cppcheck */
             return NULL;
          }

        /* get entrie header */
        GET_INT(efn->offset, data, idx);
        GET_INT(efn->size, data, idx);
        GET_INT(efn->data_size, data, idx);
        GET_INT(name_offset, data, idx);
        GET_INT(name_size, data, idx);
        GET_INT(flag, data, idx);

        efn->compression = flag & 0x1 ? 1 : 0;
        efn->ciphered = flag & 0x2 ? 1 : 0;
        efn->alias = flag & 0x4 ? 1 : 0;
        efn->compression_type = (flag >> 3) & 0xff;

#define EFN_TEST(Test, Ef, Efn) \
  if (eet_test_close(Test, Ef)) \
    {                           \
       eet_file_node_mp_free(Efn);               \
       return NULL;             \
    }

        /* check data pointer position */
        EFN_TEST(!((efn->size > 0)
                   && (efn->offset + efn->size <= ef->data_size)
                   && (efn->offset > bytes_dictionary_entries +
                       bytes_directory_entries)), ef, efn);

        /* check name position */
        EFN_TEST(!((name_size > 0)
                   && (name_offset + name_size < ef->data_size)
                   && (name_offset >= bytes_dictionary_entries +
                       bytes_directory_entries)), ef, efn);

        name = start + name_offset;

        /* check '\0' at the end of name string */
        EFN_TEST(name[name_size - 1] != '\0', ef, efn);

        efn->free_name = 0;
        efn->name = (char *)name;
        efn->name_size = name_size;

        hash = _eet_hash_gen(efn->name, ef->header->directory->size);
        efn->next = ef->header->directory->nodes[hash];
        ef->header->directory->nodes[hash] = efn;

        /* read-only mode, so currently we have no data loaded */
        if (ef->mode == EET_FILE_MODE_READ)
          efn->data = NULL;  /* read-write mode - read everything into ram */
        else
          {
             efn->data = malloc(efn->size);
             if (efn->data)
               memcpy(efn->data, ef->data + efn->offset, efn->size);
          }

        /* compute the possible position of a signature */
        if (signature_base_offset < efn->offset + efn->size)
          signature_base_offset = efn->offset + efn->size;
     }

   ef->ed = NULL;

   if (num_dictionary_entries)
     {
        const int *dico = (const int *)ef->data +
          EET_FILE2_DIRECTORY_ENTRY_COUNT * num_directory_entries +
          EET_FILE2_HEADER_COUNT;
        int j;

        if (eet_test_close((num_dictionary_entries *
                            (int)EET_FILE2_DICTIONARY_ENTRY_SIZE + idx) >
                           (bytes_dictionary_entries + bytes_directory_entries),
                           ef))
          return NULL;

        ef->ed = eet_dictionary_add();
        if (eet_test_close(!ef->ed, ef))
          return NULL;

        INF("loading dictionary for '%s' with %lu entries of size %zu",
            ef->path, num_dictionary_entries, sizeof(Eet_String));

        ef->ed->all = calloc(1, num_dictionary_entries * sizeof(Eet_String));
        if (eet_test_close(!ef->ed->all, ef))
          return NULL;

	ef->ed->all_hash = calloc(1, num_dictionary_entries * sizeof (unsigned char));
	if (eet_test_close(!ef->ed->all_hash, ef))
	  return NULL;

	ef->ed->all_allocated = calloc(1, ((num_dictionary_entries >> 3) + 1) * sizeof (unsigned char));
	if (eet_test_close(!ef->ed->all_allocated, ef))
	  return NULL;

        ef->ed->count = num_dictionary_entries;
        ef->ed->total = num_dictionary_entries;
        ef->ed->start = start + bytes_dictionary_entries +
          bytes_directory_entries;
        ef->ed->end = ef->ed->start;

        for (j = 0; j < ef->ed->count; ++j)
          {
             unsigned int offset;
	     int prev;
             int hash;

             GET_INT(hash, dico, idx);
             GET_INT(offset, dico, idx);
             GET_INT(ef->ed->all[j].len, dico, idx);
             GET_INT(prev, dico, idx); // Let's ignore prev link for dictionary, use it only as an hint to head
             GET_INT(ef->ed->all[j].next, dico, idx);

             /* Hash value could be stored on 8bits data, but this will break alignment of all the others data.
                So stick to int and check the value. */
             if (eet_test_close(hash & 0xFFFFFF00, ef))
               return NULL;

             /* Check string position */
             if (eet_test_close(!((ef->ed->all[j].len > 0)
                                  && (offset >
                                      (bytes_dictionary_entries +
                                       bytes_directory_entries))
                                  && (offset + ef->ed->all[j].len <
                                      ef->data_size)), ef))
               return NULL;

             ef->ed->all[j].str = start + offset;

             if (ef->ed->all[j].str + ef->ed->all[j].len > ef->ed->end)
               ef->ed->end = ef->ed->all[j].str + ef->ed->all[j].len;

             /* Check '\0' at the end of the string */
             if (eet_test_close(ef->ed->all[j].str[ef->ed->all[j].len - 1] !=
                                '\0', ef))
               return NULL;

             ef->ed->all_hash[j] = hash;
             if (prev == -1)
               ef->ed->hash[hash] = j;

             /* compute the possible position of a signature */
             if (signature_base_offset < offset + ef->ed->all[j].len)
               signature_base_offset = offset + ef->ed->all[j].len;
          }
     }

   /* Check if the file is signed */
   ef->x509_der = NULL;
   ef->x509_length = 0;
   ef->signature = NULL;
   ef->signature_length = 0;

   if (signature_base_offset < ef->data_size)
     {
#ifdef HAVE_SIGNATURE
        const unsigned char *buffer = ((const unsigned char *)ef->data) +
          signature_base_offset;
        ef->x509_der = eet_identity_check(ef->data,
                                          signature_base_offset,
                                          &ef->sha1,
                                          &ef->sha1_length,
                                          buffer,
                                          ef->data_size - signature_base_offset,
                                          &ef->signature,
                                          &ef->signature_length,
                                          &ef->x509_length);

        if (eet_test_close(!ef->x509_der, ef))
          return NULL;

#else /* ifdef HAVE_SIGNATURE */
        ERR(
          "This file could be signed but you didn't compile the necessary code to check the signature.");
#endif /* ifdef HAVE_SIGNATURE */
     }

   /* At this stage we have a valid eet file, let's tell the system we are likely to need most of its data */
   if (ef->readfp && ef->ed)
     {
        unsigned long int offset;

        offset = (unsigned char*) ef->ed->start - (unsigned char*) ef->data;
        eina_file_map_populate(ef->readfp, EINA_FILE_WILLNEED, ef->data,
                               offset, ef->data_size - offset);
     }

   return ef;
}

#if EET_OLD_EET_FILE_FORMAT
static Eet_File *
eet_internal_read1(Eet_File *ef)
{
   const unsigned char *dyn_buf = NULL;
   const unsigned char *p = NULL;
   unsigned long int byte_entries;
   unsigned long int num_entries;
   unsigned int i;
   int idx = 0;

   WRN(
     "EET file format of '%s' is deprecated. You should just open it one time with mode == EET_FILE_MODE_READ_WRITE to solve this issue.",
     ef->path);

   /* build header table if read mode */
   /* geat header */
   idx += sizeof(int);
   if (eet_test_close((int)ntohl(*((int *)ef->data)) != EET_MAGIC_FILE, ef))
     return NULL;

#define EXTRACT_INT(Value, Pointer, Index)       \
  {                                              \
     int tmp;                                    \
     memcpy(&tmp, Pointer + Index, sizeof(int)); \
     Value = ntohl(tmp);                         \
     Index += sizeof(int);                       \
  }

   /* get entries count and byte count */
   EXTRACT_INT(num_entries, ef->data, idx);
   EXTRACT_INT(byte_entries, ef->data, idx);

   /* we can't have <= 0 values here - invalid */
   if (eet_test_close((num_entries > 0x7fffffff) ||
                      (byte_entries > 0x7fffffff), ef))
     return NULL;

   /* we can't have more entires than minimum bytes for those! invalid! */
   if (eet_test_close((num_entries * 20) > byte_entries, ef))
     return NULL;

   /* check we will not outrun the file limit */
   if (eet_test_close(((byte_entries + (int)(sizeof(int) * 3)) >
                       ef->data_size), ef))
     return NULL;

   /* allocate header */
   ef->header = eet_file_header_calloc(1);
   if (eet_test_close(!ef->header, ef))
     return NULL;

   ef->header->magic = EET_MAGIC_FILE_HEADER;

   /* allocate directory block in ram */
   ef->header->directory = eet_file_directory_calloc(1);
   if (eet_test_close(!ef->header->directory, ef))
     return NULL;

   /* 8 bit hash table (256 buckets) */
   ef->header->directory->size = 8;
   /* allocate base hash table */
   ef->header->directory->nodes =
     calloc(1, sizeof(Eet_File_Node *) * (1 << ef->header->directory->size));
   if (eet_test_close(!ef->header->directory->nodes, ef))
     return NULL;

   /* actually read the directory block - all of it, into ram */
   dyn_buf = ef->data + idx;

   /* parse directory block */
   p = dyn_buf;

   for (i = 0; i < num_entries; i++)
     {
        Eet_File_Node *efn;
        void *data = NULL;
        int indexn = 0;
        int name_size;
        int hash;
        int k;

#define HEADER_SIZE (sizeof(int) * 5)

        /* out directory block is inconsistent - we have overrun our */
        /* dynamic block buffer before we finished scanning dir entries */
        if (eet_test_close(p + HEADER_SIZE >= (dyn_buf + byte_entries), ef))
          return NULL;

        /* allocate all the ram needed for this stored node accounting */
        efn = eet_file_node_malloc(1);
        if (eet_test_close(!efn, ef))
          {
             if (efn) eet_file_node_mp_free(efn);  /* yes i know - we only get here if
                                   * efn is null/0 -> trying to shut up
                                   * warning tools like cppcheck */
             return NULL;
          }

        /* get entrie header */
        EXTRACT_INT(efn->offset, p, indexn);
        EXTRACT_INT(efn->compression, p, indexn);
        EXTRACT_INT(efn->size, p, indexn);
        EXTRACT_INT(efn->data_size, p, indexn);
        EXTRACT_INT(name_size, p, indexn);

        efn->name_size = name_size;
        efn->ciphered = 0;
        efn->alias = 0;

        /* invalid size */
        if (eet_test_close(efn->size <= 0, ef))
          {
             eet_file_node_mp_free(efn);
             return NULL;
          }

        /* invalid name_size */
        if (eet_test_close(name_size <= 0, ef))
          {
             eet_file_node_mp_free(efn);
             return NULL;
          }

        /* reading name would mean falling off end of dyn_buf - invalid */
        if (eet_test_close((p + 16 + name_size) > (dyn_buf + byte_entries), ef))
          {
             eet_file_node_mp_free(efn);
             return NULL;
          }

        /* This code is useless if we dont want backward compatibility */
        for (k = name_size;
             k > 0 && ((unsigned char)*(p + HEADER_SIZE + k)) != 0; --k)
          ;

        efn->free_name = ((unsigned char)*(p + HEADER_SIZE + k)) != 0;

        if (efn->free_name)
          {
             efn->name = malloc(sizeof(char) * name_size + 1);
             if (eet_test_close(!efn->name, ef))
               {
                  eet_file_node_mp_free(efn);
                  return NULL;
               }

             strncpy(efn->name, (char *)p + HEADER_SIZE, name_size);
             efn->name[name_size] = 0;

             WRN(
               "File: %s is not up to date for key \"%s\" - needs rebuilding sometime",
               ef->path,
               efn->name);
          }
        else
          /* The only really useful peace of code for efn->name (no backward compatibility) */
          efn->name = (char *)((unsigned char *)(p + HEADER_SIZE));

        /* get hash bucket it should go in */
        hash = _eet_hash_gen(efn->name, ef->header->directory->size);
        efn->next = ef->header->directory->nodes[hash];
        ef->header->directory->nodes[hash] = efn;

        /* read-only mode, so currently we have no data loaded */
        if (ef->mode == EET_FILE_MODE_READ)
          efn->data = NULL;  /* read-write mode - read everything into ram */
        else
          {
             data = malloc(efn->size);
             if (data)
               memcpy(data, ef->data + efn->offset, efn->size);

             efn->data = data;
          }

        /* advance */
        p += HEADER_SIZE + name_size;
     }
   return ef;
}

#endif /* if EET_OLD_EET_FILE_FORMAT */

/*
 * this should only be called when the cache lock is already held
 * (We could drop this restriction if we add a parameter to eet_test_close
 * that indicates if the lock is held or not.  For now it is easiest
 * to just require that it is always held.)
 */
static Eet_File *
eet_internal_read(Eet_File *ef)
{
   const int *data = (const int *)ef->data;

   if (eet_test_close((ef->data == (void *)-1) || (!ef->data), ef))
     return NULL;

   if (eet_test_close(ef->data_size < (int)sizeof(int) * 3, ef))
     return NULL;

   switch (ntohl(*data))
     {
#if EET_OLD_EET_FILE_FORMAT
      case EET_MAGIC_FILE:
        return eet_internal_read1(ef);

#endif /* if EET_OLD_EET_FILE_FORMAT */
      case EET_MAGIC_FILE2:
        return eet_internal_read2(ef);

      default:
        ef->delete_me_now = 1;
        eet_internal_close(ef, EINA_TRUE);
        break;
     }

   return NULL;
}

static Eet_Error
eet_internal_close(Eet_File *ef,
                   Eina_Bool locked)
{
   Eet_Error err;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return EET_ERROR_BAD_OBJECT;

   if (!locked)
     LOCK_CACHE;

   /* deref */
   ef->references--;
   /* if its still referenced - dont go any further */
   if (ef->references > 0)
     {
        /* flush any writes */
         if ((ef->mode == EET_FILE_MODE_WRITE) ||
             (ef->mode == EET_FILE_MODE_READ_WRITE))
           eet_sync(ef);
         goto on_error;
     }

   err = eet_flush2(ef);

   eet_identity_unref(ef->key);
   ef->key = NULL;

   /* if not urgent to delete it - dont free it - leave it in cache */
   if ((!ef->delete_me_now) && (ef->mode == EET_FILE_MODE_READ))
     goto on_error;

   /* remove from cache */
   if (ef->mode == EET_FILE_MODE_READ)
     eet_cache_del(ef, &eet_readers, &eet_readers_num, &eet_readers_alloc);
   else if ((ef->mode == EET_FILE_MODE_WRITE) ||
            (ef->mode == EET_FILE_MODE_READ_WRITE))
     eet_cache_del(ef, &eet_writers, &eet_writers_num, &eet_writers_alloc);

   /* we can unlock the cache now */
   if (!locked)
     UNLOCK_CACHE;

   DESTROY_FILE(ef);

   /* free up data */
   if (ef->header)
     {
        if (ef->header->directory)
          {
             if (ef->header->directory->nodes)
               {
                  int i, num;

                  num = (1 << ef->header->directory->size);
                  for (i = 0; i < num; i++)
                    {
                       Eet_File_Node *efn;

                       while ((efn = ef->header->directory->nodes[i]))
                         {
                            if (efn->data)
                              free(efn->data);

                            ef->header->directory->nodes[i] = efn->next;

                            if (efn->free_name)
                              free(efn->name);

                            eet_file_node_mp_free(efn);
                         }
                    }
                  free(ef->header->directory->nodes);
               }

             eet_file_directory_mp_free(ef->header->directory);
          }

        eet_file_header_mp_free(ef->header);
     }

   eet_dictionary_free(ef->ed);

   if (ef->sha1)
     free(ef->sha1);

   if (ef->readfp && ef->readfp_owned)
     {
        if (ef->data)
          eina_file_map_free(ef->readfp, (void *)ef->data);

        eina_file_close(ef->readfp);
     }

   /* zero out ram for struct - caution tactic against stale memory use */
   memset(ef, 0, sizeof(Eet_File));

   /* free it */
   eina_stringshare_del(ef->path);
   eet_file_mp_free(ef);
   return err;

on_error:
   if (!locked)
     UNLOCK_CACHE;

   return EET_ERROR_NONE;
}

EAPI Eet_File *
eet_memopen_read(const void *data,
                 size_t      size)
{
   Eet_File *ef;

   if (!data || size == 0)
     return NULL;

   ef = eet_file_malloc(1);
   if (!ef)
     return NULL;

   INIT_FILE(ef);
   ef->ed = NULL;
   ef->path = NULL;
   ef->key = NULL;
   ef->magic = EET_MAGIC_FILE;
   ef->references = 1;
   ef->mode = EET_FILE_MODE_READ;
   ef->header = NULL;
   ef->delete_me_now = 1;
   ef->readfp = NULL;
   ef->data = data;
   ef->data_size = size;
   ef->sha1 = NULL;
   ef->sha1_length = 0;
   ef->readfp_owned = EINA_FALSE;

   /* eet_internal_read expects the cache lock to be held when it is called */
   LOCK_CACHE;
   ef = eet_internal_read(ef);
   UNLOCK_CACHE;
   return ef;
}

EAPI const char *
eet_file_get(Eet_File *ef)
{
   if (eet_check_pointer(ef)) return NULL;
   return ef->path;
}

EAPI Eet_File *
eet_mmap(const Eina_File *file)
{
   Eet_File *ef = NULL;
   const char *path;

   path = eina_file_filename_get(file);

   LOCK_CACHE;
   ef = eet_cache_find(path, eet_writers, eet_writers_num);
   if (ef)
     {
        eet_sync(ef);
        ef->references++;
        ef->delete_me_now = 1;
        eet_internal_close(ef, EINA_TRUE);
     }

   ef = eet_cache_find(path, eet_readers, eet_readers_num);
   if (ef && ef->readfp == file)
     {
        ef->references++;
        goto done;
     }

   /* Allocate struct for eet file and have it zero'd out */
   ef = eet_file_malloc(1);
   if (!ef) goto on_error;

   /* fill some of the members */
   INIT_FILE(ef);
   ef->ed = NULL;
   ef->key = NULL;
   ef->readfp = eina_file_dup(file);
   ef->path = eina_stringshare_add(path);
   ef->magic = EET_MAGIC_FILE;
   ef->references = 1;
   ef->mode = EET_FILE_MODE_READ;
   ef->header = NULL;
   ef->writes_pending = 0;
   ef->delete_me_now = 0;
   ef->data = NULL;
   ef->data_size = 0;
   ef->sha1 = NULL;
   ef->sha1_length = 0;
   ef->readfp_owned = EINA_TRUE;

   ef->data_size = eina_file_size_get(ef->readfp);
   ef->data = eina_file_map_all(ef->readfp, EINA_FILE_SEQUENTIAL);
   if (eet_test_close((ef->data == NULL), ef))
     goto on_error;

   ef = eet_internal_read(ef);
   if (!ef)
     goto on_error;

   if (ef->mode == EET_FILE_MODE_READ)
     eet_cache_add(ef, &eet_readers, &eet_readers_num, &eet_readers_alloc);
   
 done:
   UNLOCK_CACHE;
   return ef;

 on_error:
   UNLOCK_CACHE;
   return NULL;
}

EAPI Eet_File *
eet_open(const char   *file,
         Eet_File_Mode mode)
{
   Eina_File *fp;
   Eet_File *ef;
   int file_len;
   unsigned long int size;

   if (!file)
     return NULL;

   /* find the current file handle in cache*/
   ef = NULL;
   LOCK_CACHE;
   if (mode == EET_FILE_MODE_READ)
     {
        ef = eet_cache_find((char *)file, eet_writers, eet_writers_num);
        if (ef)
          {
             eet_sync(ef);
             ef->references++;
             ef->delete_me_now = 1;
             eet_internal_close(ef, EINA_TRUE);
          }

        ef = eet_cache_find((char *)file, eet_readers, eet_readers_num);
     }
   else if ((mode == EET_FILE_MODE_WRITE) ||
            (mode == EET_FILE_MODE_READ_WRITE))
     {
        ef = eet_cache_find((char *)file, eet_readers, eet_readers_num);
        if (ef)
          {
             ef->delete_me_now = 1;
             ef->references++;
             eet_internal_close(ef, EINA_TRUE);
          }

        ef = eet_cache_find((char *)file, eet_writers, eet_writers_num);
     }

   /* try open the file based on mode */
   if ((mode == EET_FILE_MODE_READ) || (mode == EET_FILE_MODE_READ_WRITE))
     {
        /* Prevent garbage in futur comparison. */
         fp = eina_file_open(file, EINA_FALSE);
         if (!fp)
           {
              size = 0;
              goto open_error;
           }

         size = eina_file_size_get(fp);

         if (size < ((int)sizeof(int) * 3))
           {
              eina_file_close(fp);
              fp = NULL;

              size = 0;

              goto open_error;
           }

open_error:
         if (!fp && mode == EET_FILE_MODE_READ)
           goto on_error;
     }
   else
     {
        if (mode != EET_FILE_MODE_WRITE)
          {
            UNLOCK_CACHE;
            return NULL;
          }

        size = 0;

        fp = NULL;
     }

   /* We found one */
   if (ef && ef->readfp != fp)
     {
        ef->delete_me_now = 1;
        ef->references++;
        eet_internal_close(ef, EINA_TRUE);
        ef = NULL;
     }

   if (ef)
     {
        /* reference it up and return it */
         if (fp)
           eina_file_close(fp);

         ef->references++;
         UNLOCK_CACHE;
         return ef;
     }

   file_len = strlen(file) + 1;

   /* Allocate struct for eet file and have it zero'd out */
   ef = eet_file_malloc(1);
   if (!ef)
     goto on_error;

   /* fill some of the members */
   INIT_FILE(ef);
   ef->key = NULL;
   ef->readfp = fp;
   ef->path = eina_stringshare_add_length(file, file_len);
   ef->magic = EET_MAGIC_FILE;
   ef->references = 1;
   ef->mode = mode;
   ef->header = NULL;
   ef->writes_pending = 0;
   ef->delete_me_now = 0;
   ef->data = NULL;
   ef->data_size = 0;
   ef->sha1 = NULL;
   ef->sha1_length = 0;
   ef->readfp_owned = EINA_TRUE;

   ef->ed = (mode == EET_FILE_MODE_WRITE)
     || (!ef->readfp && mode == EET_FILE_MODE_READ_WRITE) ?
     eet_dictionary_add() : NULL;

   if (!ef->readfp &&
       (mode == EET_FILE_MODE_READ_WRITE || mode == EET_FILE_MODE_WRITE))
     goto empty_file;

   /* if we can't open - bail out */
   if (eet_test_close(!ef->readfp, ef))
     goto on_error;

   /* if we opened for read or read-write */
   if ((mode == EET_FILE_MODE_READ) || (mode == EET_FILE_MODE_READ_WRITE))
     {
        ef->data_size = size;
        ef->data = eina_file_map_all(fp, EINA_FILE_SEQUENTIAL);
        if (eet_test_close((ef->data == NULL), ef))
          goto on_error;

        ef = eet_internal_read(ef);
        if (!ef)
          goto on_error;
     }

empty_file:
   /* add to cache */
   if (ef->references == 1)
     {
        if (ef->mode == EET_FILE_MODE_READ)
          eet_cache_add(ef, &eet_readers, &eet_readers_num, &eet_readers_alloc);
        else if ((ef->mode == EET_FILE_MODE_WRITE) ||
                 (ef->mode == EET_FILE_MODE_READ_WRITE))
          eet_cache_add(ef, &eet_writers, &eet_writers_num, &eet_writers_alloc);
     }

   UNLOCK_CACHE;
   return ef;

on_error:
   UNLOCK_CACHE;
   return NULL;
}

EAPI Eet_File_Mode
eet_mode_get(Eet_File *ef)
{
   /* check to see its' an eet file pointer */
    if ((!ef) || (ef->magic != EET_MAGIC_FILE))
      return EET_FILE_MODE_INVALID;
    else
      return ef->mode;
}

EAPI const void *
eet_identity_x509(Eet_File *ef,
                  int      *der_length)
{
   if (eet_check_pointer(ef))
     return NULL;

   if (!ef->x509_der)
     return NULL;

   if (der_length)
     *der_length = ef->x509_length;

   return ef->x509_der;
}

EAPI const void *
eet_identity_signature(Eet_File *ef,
                       int      *signature_length)
{
   if (eet_check_pointer(ef))
     return NULL;

   if (!ef->signature)
     return NULL;

   if (signature_length)
     *signature_length = ef->signature_length;

   return ef->signature;
}

EAPI const void *
eet_identity_sha1(Eet_File *ef,
                  int      *sha1_length)
{
   if (eet_check_pointer(ef))
     return NULL;

   if (!ef->sha1)
     ef->sha1 = eet_identity_compute_sha1(ef->data,
                                          ef->data_size,
                                          &ef->sha1_length);

   if (sha1_length)
     *sha1_length = ef->sha1_length;

   return ef->sha1;
}

EAPI Eet_Error
eet_identity_set(Eet_File *ef,
                 Eet_Key  *key)
{
   Eet_Key *tmp;

   if (!ef)
     return EET_ERROR_BAD_OBJECT;

   tmp = ef->key;
   ef->key = key;
   eet_identity_ref(ef->key);
   eet_identity_unref(tmp);

   /* flags that writes are pending */
   ef->writes_pending = 1;

   return EET_ERROR_NONE;
}

EAPI Eet_Error
eet_close(Eet_File *ef)
{
   return eet_internal_close(ef, EINA_FALSE);
}

EAPI void *
eet_read_cipher(Eet_File   *ef,
                const char *name,
                int        *size_ret,
                const char *cipher_key)
{
   Eet_File_Node *efn;
   char *data = NULL;
   unsigned long int size = 0;

   if (size_ret)
     *size_ret = 0;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return NULL;

   if (!name)
     return NULL;

   if ((ef->mode != EET_FILE_MODE_READ) &&
       (ef->mode != EET_FILE_MODE_READ_WRITE))
     return NULL;

   /* no header, return NULL */
   if (eet_check_header(ef))
     return NULL;

   LOCK_FILE(ef);

   /* hunt hash bucket */
   efn = find_node_by_name(ef, name);
   if (!efn)
     goto on_error;

   /* get size (uncompressed, if compressed at all) */
   size = efn->data_size;

   /* allocate data */
   data = malloc(size);
   if (!data)
     goto on_error;

   /* uncompressed data */
   if (efn->compression == 0)
     {
        void *data_deciphered = NULL;
        unsigned int data_deciphered_sz = 0;
        /* if we already have the data in ram... copy that */

        if (efn->ciphered && efn->size > size)
          {
             size = efn->size;
             data = realloc(data, efn->size);
          }

        if (efn->data)
          memcpy(data, efn->data, size);
        else
        if (!read_data_from_disk(ef, efn, data, size))
          goto on_error;

        if (efn->ciphered && cipher_key)
          {
             if (eet_decipher(data, efn->size, cipher_key, strlen(cipher_key),
                              &data_deciphered, &data_deciphered_sz))
               {
                  if (data_deciphered)
                    free(data_deciphered);

                  goto on_error;
               }

             free(data);
             data = data_deciphered;
             size = data_deciphered_sz;
          }
     }
   /* compressed data */
   else
     {
        void *tmp_data = NULL;
        void *data_deciphered = NULL;
        unsigned int data_deciphered_sz = 0;
        int free_tmp = 0, ret;
        int compr_size = efn->size;
        uLongf dlen;

        /* if we already have the data in ram... copy that */
        if (efn->data)
          tmp_data = efn->data;
        else
          {
             tmp_data = malloc(compr_size);
             if (!tmp_data)
               goto on_error;

             free_tmp = 1;

             if (!read_data_from_disk(ef, efn, tmp_data, compr_size))
               {
                  free(tmp_data);
                  goto on_error;
               }
          }

        if (efn->ciphered && cipher_key)
          {
             if (eet_decipher(tmp_data, compr_size, cipher_key,
                              strlen(cipher_key), &data_deciphered,
                              &data_deciphered_sz))
               {
                  if (free_tmp)
                    free(tmp_data);

                  if (data_deciphered)
                    free(data_deciphered);

                  goto on_error;
               }

             if (free_tmp)
               free(tmp_data);
             free_tmp = 1;
             tmp_data = data_deciphered;
             compr_size = data_deciphered_sz;
          }

        /* decompress it */
        dlen = size;
        switch (efn->compression_type)
          {
           case EET_COMPRESSION_VERYFAST:
           case EET_COMPRESSION_SUPERFAST:
             ret = LZ4_uncompress(tmp_data, data, dlen);
             if (ret != compr_size)
               {
                  if (free_tmp)
                    free(tmp_data);
                  goto on_error;
               }
             break;
           default:
             if (uncompress((Bytef *)data, &dlen,
                            tmp_data, (uLongf)compr_size) != Z_OK)
               {
                  if (free_tmp)
                    free(tmp_data);
                  goto on_error;
               }
             break;
          }

        if (free_tmp)
          free(tmp_data);
     }

   UNLOCK_FILE(ef);

   /* handle alias */
   if (efn->alias)
     {
        void *tmp;

        if (data[size - 1] != '\0')
          goto on_error;

        tmp = eet_read_cipher(ef, data, size_ret, cipher_key);

        free(data);

        data = tmp;
     }
   else
   /* fill in return values */
   if (size_ret)
     *size_ret = size;

   return data;

on_error:
   UNLOCK_FILE(ef);
   free(data);
   return NULL;
}

EAPI void *
eet_read(Eet_File   *ef,
         const char *name,
         int        *size_ret)
{
   return eet_read_cipher(ef, name, size_ret, NULL);
}

EAPI const void *
eet_read_direct(Eet_File   *ef,
                const char *name,
                int        *size_ret)
{
   Eet_File_Node *efn;
   const char *data = NULL;
   int size = 0, ret;

   if (size_ret)
     *size_ret = 0;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return NULL;

   if (!name)
     return NULL;

   if ((ef->mode != EET_FILE_MODE_READ) &&
       (ef->mode != EET_FILE_MODE_READ_WRITE))
     return NULL;

   /* no header, return NULL */
   if (eet_check_header(ef))
     return NULL;

   LOCK_FILE(ef);

   /* hunt hash bucket */
   efn = find_node_by_name(ef, name);
   if (!efn)
     goto on_error;

   /* trick to detect data in memory instead of mmaped from disk */
   if (efn->offset > ef->data_size && !efn->data)
     goto on_error;

   /* get size (uncompressed, if compressed at all) */
   size = efn->data_size;

   if (efn->alias)
     {
        data = efn->data ? efn->data : ef->data + efn->offset;

        /* handle alias case */
        if (efn->compression)
          {
             const void *retptr;
             char *tmp;
             int compr_size = efn->size;
             uLongf dlen;
             
             tmp = malloc(compr_size);
             if (!tmp) goto on_error;
             switch (efn->compression_type)
               {
                case EET_COMPRESSION_VERYFAST:
                case EET_COMPRESSION_SUPERFAST:
                  ret = LZ4_uncompress(data, tmp, size);
                  if (ret != compr_size)
                    {
                       free(tmp);
                       goto on_error;
                    }
                  break;
                default:
                  dlen = size;
                  
                  if (uncompress((Bytef *)tmp, &dlen, (Bytef *)data,
                                 (uLongf)compr_size))
                    {
                       free(tmp);
                       goto on_error;
                    }
               }
             
             if (tmp[compr_size - 1] != '\0')
               {
                  free(tmp);
                  goto on_error;
               }
             
             UNLOCK_FILE(ef);
             
             retptr = eet_read_direct(ef, tmp, size_ret);
             free(tmp);
             return retptr;
          }

        if (!data)
          goto on_error;

        if (data[size - 1] != '\0')
          goto on_error;

        UNLOCK_FILE(ef);

        return eet_read_direct(ef, data, size_ret);
     }
   else
   /* uncompressed data */
   if ((efn->compression == 0) && (efn->ciphered == 0))
     data = efn->data ? efn->data : ef->data + efn->offset;  /* compressed data */
   else
     data = NULL;

   /* fill in return values */
   if (size_ret)
     *size_ret = size;

   UNLOCK_FILE(ef);

   return data;

on_error:
   UNLOCK_FILE(ef);
   return NULL;
}

EAPI const char *
eet_alias_get(Eet_File   *ef,
              const char *name)
{
   Eet_File_Node *efn;
   const char *data = NULL;
   int size = 0, ret;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return NULL;

   if (!name)
     return NULL;

   if ((ef->mode != EET_FILE_MODE_READ) &&
       (ef->mode != EET_FILE_MODE_READ_WRITE))
     return NULL;

   /* no header, return NULL */
   if (eet_check_header(ef))
     return NULL;

   LOCK_FILE(ef);

   /* hunt hash bucket */
   efn = find_node_by_name(ef, name);
   if (!efn)
     goto on_error;

   /* trick to detect data in memory instead of mmaped from disk */
   if (efn->offset > ef->data_size && !efn->data)
     goto on_error;

   /* get size (uncompressed, if compressed at all) */
   size = efn->data_size;

   if (!efn->alias) goto on_error;
   data = efn->data ? efn->data : ef->data + efn->offset;

   /* handle alias case */
   if (efn->compression)
     {
        const char *retptr;
        char *tmp;
        int compr_size = efn->size;
        uLongf dlen;
                  
        tmp = malloc(compr_size);
        if (!tmp) goto on_error;
        switch (efn->compression_type)
          {
           case EET_COMPRESSION_VERYFAST:
           case EET_COMPRESSION_SUPERFAST:
             ret = LZ4_uncompress(data, tmp, size);
             if (ret != compr_size)
               {
                  free(tmp);
                  goto on_error;
               }
             break;
           default:
             dlen = size;
             
             if (uncompress((Bytef *)tmp, &dlen, (Bytef *)data,
                            (uLongf)compr_size))
               {
                  free(tmp);
                  goto on_error;
               }
          }
        
        if (tmp[compr_size - 1] != '\0')
          {
             free(tmp);
             goto on_error;
          }

        UNLOCK_FILE(ef);

        retptr = eina_stringshare_add(tmp);
        free(tmp);
        return retptr;
     }

   if (!data)
     goto on_error;

   if (data[size - 1] != '\0')
     goto on_error;

   UNLOCK_FILE(ef);

   return eina_stringshare_add(data);

on_error:
   UNLOCK_FILE(ef);
   return NULL;
}

EAPI Eina_Bool
eet_alias(Eet_File   *ef,
          const char *name,
          const char *destination,
          int         comp)
{
   Eet_File_Node *efn;
   void *data2;
   Eina_Bool exists_already = EINA_FALSE;
   int data_size, ret, hash, slen;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return EINA_FALSE;

   if ((!name) || (!destination))
     return EINA_FALSE;

   if ((ef->mode != EET_FILE_MODE_WRITE) &&
       (ef->mode != EET_FILE_MODE_READ_WRITE))
     return EINA_FALSE;

   LOCK_FILE(ef);

   if (!ef->header)
     {
        /* allocate header */
         ef->header = eet_file_header_calloc(1);
         if (!ef->header)
           goto on_error;

         ef->header->magic = EET_MAGIC_FILE_HEADER;
         /* allocate directory block in ram */
         ef->header->directory = eet_file_directory_calloc(1);
         if (!ef->header->directory)
           {
              eet_file_header_mp_free(ef->header);
              ef->header = NULL;
              goto on_error;
           }

         /* 8 bit hash table (256 buckets) */
         ef->header->directory->size = 8;
         /* allocate base hash table */
         ef->header->directory->nodes =
           calloc(1, sizeof(Eet_File_Node *) *
                  (1 << ef->header->directory->size));
         if (!ef->header->directory->nodes)
           {
              eet_file_directory_mp_free(ef->header->directory);
              ef->header = NULL;
              goto on_error;
           }
     }

   /* figure hash bucket */
   hash = _eet_hash_gen(name, ef->header->directory->size);

   slen = strlen(destination) + 1;
   data_size = comp ?
     12 + ((slen * 101) / 100)
     : slen;
   if (comp)
     {
        ret = LZ4_compressBound(slen);
        if ((ret > 0) && (ret > data_size)) data_size = ret;
     }
   
   data2 = malloc(data_size);
   if (!data2) goto on_error;

   /* if we want to compress */
   if (comp)
     {
        switch (comp)
          {
           case EET_COMPRESSION_VERYFAST:
             ret = LZ4_compressHC((const char *)destination, (char *)data2,
                                  slen);
             if (ret <= 0)
               {
                  free(data2);
                  goto on_error;
               }
             data_size = ret;
             break;
           case EET_COMPRESSION_SUPERFAST:
             ret = LZ4_compress((const char *)destination, (char *)data2,
                                slen);
             if (ret <= 0)
               {
                  free(data2);
                  goto on_error;
               }
             data_size = ret;
             break;
           default:
               {
                  uLongf buflen;
                  
                  /* compress the data with max compression */
                  buflen = (uLongf)data_size;
                  if (compress2((Bytef *)data2, &buflen, 
                                (const Bytef *)destination,
                                (uLong)slen, Z_BEST_COMPRESSION) != Z_OK)
                    {
                       free(data2);
                       goto on_error;
                    }
                  /* record compressed chunk size */
                  data_size = (int)buflen;
               }
             break;
          }
        if ((data_size < 0) || 
            (data_size >= (int)(strlen(destination) + 1)))
          {
             comp = 0;
             data_size = strlen(destination) + 1;
          }
        else
          {
             void *data3;

             data3 = realloc(data2, data_size);
             if (data3) data2 = data3;
          }
     }
   
   if (!comp) memcpy(data2, destination, data_size);

   /* Does this node already exist? */
   for (efn = ef->header->directory->nodes[hash]; efn; efn = efn->next)
     {
        /* if it matches */
         if ((efn->name) && (eet_string_match(efn->name, name)))
           {
              free(efn->data);
              efn->alias = 1;
              efn->ciphered = 0;
              efn->compression = !!comp;
              efn->compression_type = comp;
              efn->size = data_size;
              efn->data_size = strlen(destination) + 1;
              efn->data = data2;
              /* Put the offset above the limit to avoid direct access */
              efn->offset = ef->data_size + 1;
              exists_already = EINA_TRUE;
              break;
           }
     }
   if (!exists_already)
     {
        efn = eet_file_node_malloc(1);
        if (!efn)
          {
             free(data2);
             goto on_error;
          }

        efn->name = strdup(name);
        efn->name_size = strlen(efn->name) + 1;
        efn->free_name = 1;

        efn->next = ef->header->directory->nodes[hash];
        ef->header->directory->nodes[hash] = efn;
        /* Put the offset above the limit to avoid direct access */
        efn->offset = ef->data_size + 1;
        efn->alias = 1;
        efn->ciphered = 0;
        efn->compression = !!comp;
        efn->compression_type = comp;
        efn->size = data_size;
        efn->data_size = strlen(destination) + 1;
        efn->data = data2;
     }

   /* flags that writes are pending */
   ef->writes_pending = 1;

   UNLOCK_FILE(ef);
   return EINA_TRUE;

on_error:
   UNLOCK_FILE(ef);
   return EINA_FALSE;
}

EAPI int
eet_write_cipher(Eet_File   *ef,
                 const char *name,
                 const void *data,
                 int         size,
                 int         comp,
                 const char *cipher_key)
{
   Eet_File_Node *efn;
   void *data2 = NULL;
   int exists_already = 0, data_size, hash, ret;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return 0;

   if ((!name) || (!data) || (size <= 0))
     return 0;

   if ((ef->mode != EET_FILE_MODE_WRITE) &&
       (ef->mode != EET_FILE_MODE_READ_WRITE))
     return 0;

   LOCK_FILE(ef);

   if (!ef->header)
     {
        /* allocate header */
         ef->header = eet_file_header_calloc(1);
         if (!ef->header)
           goto on_error;

         ef->header->magic = EET_MAGIC_FILE_HEADER;
         /* allocate directory block in ram */
         ef->header->directory = eet_file_directory_calloc(1);
         if (!ef->header->directory)
           {
              eet_file_header_mp_free(ef->header);
              ef->header = NULL;
              goto on_error;
           }

         /* 8 bit hash table (256 buckets) */
         ef->header->directory->size = 8;
         /* allocate base hash table */
         ef->header->directory->nodes =
           calloc(1, sizeof(Eet_File_Node *) *
                  (1 << ef->header->directory->size));
         if (!ef->header->directory->nodes)
           {
              eet_file_directory_mp_free(ef->header->directory);
              ef->header = NULL;
              goto on_error;
           }
     }

   /* figure hash bucket */
   hash = _eet_hash_gen(name, ef->header->directory->size);

   UNLOCK_FILE(ef);
   
   data_size = comp ? 12 + ((size * 101) / 100) : size;
   if (comp)
     {
        ret = LZ4_compressBound(size);
        if ((ret > 0) && (ret > data_size)) data_size = ret;
     }
   
   if (comp || !cipher_key)
     {
        data2 = malloc(data_size);
        if (!data2)
          goto on_error;
     }

   /* if we want to compress */
   if (comp)
     {
        switch (comp)
          {
           case EET_COMPRESSION_VERYFAST:
             ret = LZ4_compressHC((const char *)data, (char *)data2, size);
             if (ret <= 0)
               {
                  free(data2);
                  LOCK_FILE(ef);
                  goto on_error;
               }
             data_size = ret;
             break;
           case EET_COMPRESSION_SUPERFAST:
             ret = LZ4_compress((const char *)data, (char *)data2, size);
             if (ret <= 0)
               {
                  free(data2);
                  LOCK_FILE(ef);
                  goto on_error;
               }
             data_size = ret;
             break;
           default:
               {
                  uLongf buflen;

                  /* compress the data with max compression */
                  buflen = (uLongf)data_size;
                  if (compress2((Bytef *)data2, &buflen, (Bytef *)data,
                                (uLong)size, Z_BEST_COMPRESSION) != Z_OK)
                    {
                       free(data2);
                       LOCK_FILE(ef);
                       goto on_error;
                    }
                  /* record compressed chunk size */
                  data_size = (int)buflen;
               }
          }
        if ((data_size < 0) || (data_size >= size))
          {
             comp = 0;
             data_size = size;
          }
        else
          {
             void *data3;

             data3 = realloc(data2, data_size);
             if (data3)
               data2 = data3;
          }
     }

   if (cipher_key)
     {
        void *data_ciphered = NULL;
        unsigned int data_ciphered_sz = 0;
        const void *tmp;

        tmp = comp ? data2 : data;
        if (!eet_cipher(tmp, data_size, cipher_key, strlen(cipher_key),
                        &data_ciphered, &data_ciphered_sz))
          {
             if (data2)
               free(data2);

             data2 = data_ciphered;
             data_size = data_ciphered_sz;
          }
        else
          {
             if (data_ciphered)
               free(data_ciphered);

             cipher_key = NULL;
          }
     }
   else
   if (!comp)
     memcpy(data2, data, size);

   LOCK_FILE(ef);
   /* Does this node already exist? */
   for (efn = ef->header->directory->nodes[hash]; efn; efn = efn->next)
     {
        /* if it matches */
         if ((efn->name) && (eet_string_match(efn->name, name)))
           {
              free(efn->data);
              efn->alias = 0;
              efn->ciphered = cipher_key ? 1 : 0;
              efn->compression = !!comp;
              efn->compression_type = comp;
              efn->size = data_size;
              efn->data_size = size;
              efn->data = data2;
              /* Put the offset above the limit to avoid direct access */
              efn->offset = ef->data_size + 1;
              exists_already = 1;
              break;
           }
     }
   if (!exists_already)
     {
        efn = eet_file_node_malloc(1);
        if (!efn)
          {
             free(data2);
             goto on_error;
          }

        efn->name = strdup(name);
        efn->name_size = strlen(efn->name) + 1;
        efn->free_name = 1;

        efn->next = ef->header->directory->nodes[hash];
        ef->header->directory->nodes[hash] = efn;
        /* Put the offset above the limit to avoid direct access */
        efn->offset = ef->data_size + 1;
        efn->alias = 0;
        efn->ciphered = cipher_key ? 1 : 0;
        efn->compression = !!comp;
        efn->compression_type = comp;
        efn->size = data_size;
        efn->data_size = size;
        efn->data = data2;
     }

   /* flags that writes are pending */
   ef->writes_pending = 1;
   UNLOCK_FILE(ef);
   return data_size;

on_error:
   UNLOCK_FILE(ef);
   return 0;
}

EAPI int
eet_write(Eet_File   *ef,
          const char *name,
          const void *data,
          int         size,
          int         comp)
{
   return eet_write_cipher(ef, name, data, size, comp, NULL);
}

EAPI int
eet_delete(Eet_File   *ef,
           const char *name)
{
   Eet_File_Node *efn;
   Eet_File_Node *pefn;
   int hash;
   int exists_already = 0;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef))
     return 0;

   if (!name)
     return 0;

   /* deleting keys is only possible in RW or WRITE mode */
   if (ef->mode == EET_FILE_MODE_READ)
     return 0;

   if (eet_check_header(ef))
     return 0;

   LOCK_FILE(ef);

   /* figure hash bucket */
   hash = _eet_hash_gen(name, ef->header->directory->size);

   /* Does this node already exist? */
   for (pefn = NULL, efn = ef->header->directory->nodes[hash];
        efn;
        pefn = efn, efn = efn->next)
     {
        /* if it matches */
         if (eet_string_match(efn->name, name))
           {
              if (efn->data)
                free(efn->data);

              if (!pefn)
                ef->header->directory->nodes[hash] = efn->next;
              else
                pefn->next = efn->next;

              if (efn->free_name)
                free(efn->name);

              eet_file_node_mp_free(efn);
              exists_already = 1;
              break;
           }
     }
   /* flags that writes are pending */
   if (exists_already)
     ef->writes_pending = 1;

   UNLOCK_FILE(ef);

   /* update access time */
   return exists_already;
}

EAPI Eet_Dictionary *
eet_dictionary_get(Eet_File *ef)
{
   if (eet_check_pointer(ef))
     return NULL;

   return ef->ed;
}

EAPI char **
eet_list(Eet_File   *ef,
         const char *glob,
         int        *count_ret)
{
   Eet_File_Node *efn;
   char **list_ret = NULL;
   int list_count = 0;
   int list_count_alloc = 0;
   int i, num;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef) || eet_check_header(ef) ||
       (!glob) ||
       ((ef->mode != EET_FILE_MODE_READ) &&
        (ef->mode != EET_FILE_MODE_READ_WRITE)))
     {
        if (count_ret)
          *count_ret = 0;

        return NULL;
     }

   if (!strcmp(glob, "*"))
     glob = NULL;

   LOCK_FILE(ef);

   /* loop through all entries */
   num = (1 << ef->header->directory->size);
   for (i = 0; i < num; i++)
     {
        for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
          {
             /* if the entry matches the input glob
              * check for * explicitly, because on some systems, * isn't well
              * supported
              */
               if ((!glob) || !fnmatch(glob, efn->name, 0))
                 {
     /* add it to our list */
                     list_count++;

     /* only realloc in 32 entry chunks */
                     if (list_count > list_count_alloc)
                       {
                          char **new_list = NULL;

                          list_count_alloc += 64;
                          new_list =
                            realloc(list_ret, list_count_alloc * (sizeof(char *)));
                          if (!new_list)
                            {
                               free(list_ret);

                               goto on_error;
                            }

                          list_ret = new_list;
                       }

     /* put pointer of name string in */
                     list_ret[list_count - 1] = efn->name;
                 }
          }
     }

   UNLOCK_FILE(ef);

   /* return count and list */
   if (count_ret)
     *count_ret = list_count;

   return list_ret;

on_error:
   UNLOCK_FILE(ef);

   if (count_ret)
     *count_ret = 0;

   return NULL;
}

EAPI int
eet_num_entries(Eet_File *ef)
{
   int i, num, ret = 0;
   Eet_File_Node *efn;

   /* check to see its' an eet file pointer */
   if (eet_check_pointer(ef) || eet_check_header(ef) ||
       ((ef->mode != EET_FILE_MODE_READ) &&
        (ef->mode != EET_FILE_MODE_READ_WRITE)))
     return -1;

   LOCK_FILE(ef);

   /* loop through all entries */
   num = (1 << ef->header->directory->size);
   for (i = 0; i < num; i++)
     {
        for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
          ret++;
     }

   UNLOCK_FILE(ef);

   return ret;
}

typedef struct _Eet_Entries_Iterator Eet_Entries_Iterator;
struct _Eet_Entries_Iterator
{
   Eina_Iterator iterator;

   Eet_File *ef;
   Eet_File_Node *efn;
   int index;

   Eet_Entry entry;

   Eina_Bool locked;
};

Eina_Bool
_eet_entries_iterator_next(Eet_Entries_Iterator *it, void **data)
{
   if (it->efn == NULL)
     {
        int num;

        num = (1 << it->ef->header->directory->size);

        do
          {
             it->index++;

             if (!(it->index < num))
               return EINA_FALSE;

             it->efn = it->ef->header->directory->nodes[it->index];
          }
        while (!it->efn);
     }

   /* copy info in public header */
   it->entry.name = it->efn->name;
   it->entry.offset = it->efn->offset;
   it->entry.size = it->efn->size;
   it->entry.data_size = it->efn->data_size;
   it->entry.compression = it->efn->compression;
   it->entry.ciphered = it->efn->ciphered;
   it->entry.alias = it->efn->alias;

   *data = &it->entry;
   it->efn = it->efn->next;
   return EINA_TRUE;
}

void *
_eet_entries_iterator_container(Eet_Entries_Iterator *it)
{
   return it->ef;
}

void
_eet_entries_iterator_free(Eet_Entries_Iterator *it)
{
   if (it->locked)
     {
        CRI("Iterator still LOCKED !");
        UNLOCK_FILE(it->ef);
     }
}

Eina_Bool
_eet_entries_iterator_lock(Eet_Entries_Iterator *it)
{
   if (it->locked)
     {
        CRI("Iterator already LOCKED !");
        return EINA_TRUE;
     }

   LOCK_FILE(it->ef);
   it->locked = EINA_TRUE;
   return EINA_TRUE;
}

Eina_Bool
_eet_entries_iterator_unlock(Eet_Entries_Iterator *it)
{
   if (!it->locked)
     {
        CRI("Iterator already UNLOCKED !");
        return EINA_TRUE;
     }

   UNLOCK_FILE(it->ef);
   it->locked = EINA_FALSE;
   return EINA_TRUE;
}

EAPI Eina_Iterator *
eet_list_entries(Eet_File *ef)
{
   Eet_Entries_Iterator *it;

   it = malloc(sizeof (Eet_Entries_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->ef = ef;
   it->efn = NULL;
   it->index = -1;
   it->locked = EINA_FALSE;
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eet_entries_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eet_entries_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eet_entries_iterator_free);
   it->iterator.lock = FUNC_ITERATOR_LOCK(_eet_entries_iterator_lock);
   it->iterator.unlock = FUNC_ITERATOR_LOCK(_eet_entries_iterator_unlock);

   return &it->iterator;
}

static Eet_File_Node *
find_node_by_name(Eet_File   *ef,
                  const char *name)
{
   Eet_File_Node *efn;
   int hash;

   /* get hash bucket this should be in */
   hash = _eet_hash_gen(name, ef->header->directory->size);

   for (efn = ef->header->directory->nodes[hash]; efn; efn = efn->next)
     {
        if (eet_string_match(efn->name, name))
          return efn;
     }

   return NULL;
}

static int
read_data_from_disk(Eet_File      *ef,
                    Eet_File_Node *efn,
                    void          *buf,
                    int            len)
{
   if (efn->offset > ef->data_size)
     return 0;

   if (!ef->data)
     return 0;

   if ((efn->offset + len) > ef->data_size)
     return 0;

   memcpy(buf, ef->data + efn->offset, len);

   return len;
}

