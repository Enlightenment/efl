/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Eet.h"
#include "Eet_private.h"

#ifdef HAVE_REALPATH
#undef HAVE_REALPATH
#endif

#define EET_MAGIC_FILE                  0x1ee7ff00
#define EET_MAGIC_FILE_HEADER           0x1ee7ff01

typedef struct _Eet_File_Header         Eet_File_Header;
typedef struct _Eet_File_Node           Eet_File_Node;
typedef struct _Eet_File_Directory      Eet_File_Directory;

struct _Eet_File
{
   int              magic;
   int              references;

   char            *path;

   FILE            *fp;
   Eet_File_Mode    mode;

   unsigned char    writes_pending : 1;
   unsigned char    delete_me_now : 1;

   Eet_File_Header *header;
};

struct _Eet_File_Header
{
   int                 magic;
   Eet_File_Directory *directory;
};

struct _Eet_File_Directory
{
   int             size;
   Eet_File_Node **nodes;
};

struct _Eet_File_Node
{
   char          *name;
   int            offset;
   int            compression;
   int            size;
   int            data_size;
   void          *data;
   Eet_File_Node *next; /* FIXME: make buckets linked lists */
};

#if 0
/* NB: all int's are stored in network byte order on disk */
/* file format: */
int magic; /* magic number ie 0x1ee7ff00 */
int num_directory_entries; /* number of directory entries to follow */
int bytes_directory_entries; /* bytes of directory entries to follow */
struct
{
   int offset; /* bytes offset into file for data chunk */
   int flags; /* flags - for now 0 = uncompressed, 1 = compressed */
   int size; /* size of the data chunk */
   int data_size; /* size of the (uncompressed) data chunk */
   int name_size; /* length in bytes of the name field */
   char name[name_size]; /* name string (variable length) */
} directory[num_directory_entries];
/* and now startes the data stream... */
#endif

/* prototypes of internal calls */
static Eet_File *eet_cache_find(const char *path, Eet_File **cache, int cache_num);
static void      eet_cache_add(Eet_File *ef, Eet_File ***cache, int *cache_num, int *cache_alloc);
static void      eet_cache_del(Eet_File *ef, Eet_File ***cache, int *cache_num, int *cache_alloc);
static int       eet_string_match(const char *s1, const char *s2);
static int       eet_hash_gen(const char *key, int hash_size);
static void      eet_flush(Eet_File *ef);
static Eet_File_Node *find_node_by_name (Eet_File *ef, const char *name);
static int read_data_from_disk(Eet_File *ef, Eet_File_Node *efn, void *buf, int len);

/* cache. i don't expect this to ever be large, so arrays will do */
static int        eet_writers_num     = 0;
static int        eet_writers_alloc   = 0;
static Eet_File **eet_writers         = NULL;
static int        eet_readers_num     = 0;
static int        eet_readers_alloc   = 0;
static Eet_File **eet_readers         = NULL;
static int        eet_initcount       = 0;
static int        eet_cacheburst_mode = 0;

/* find an eet file in the currently in use cache */
static Eet_File *
eet_cache_find(const char *path, Eet_File **cache, int cache_num)
{
   int i;

   /* walk list */
   for (i = 0; i < cache_num; i++)
     {
	/* if matches real path - return it */
	if (eet_string_match(cache[i]->path, path))
	  {
	     if (!cache[i]->delete_me_now) return cache[i];
	  }
     }
   /* not found */
   return NULL;
}

/* add to end of cache */
static void
eet_cache_add(Eet_File *ef, Eet_File ***cache, int *cache_num, int *cache_alloc)
{
   Eet_File **new_cache;
   int new_cache_num, new_cache_alloc;

   new_cache_num = *cache_num;
   if (new_cache_num > 128) /* avoid fd overruns - limit to 128 (most recent) in the cache */
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
	     eet_cacheburst_mode = 0;
	     eet_close(del_ef);
	     eet_cacheburst_mode = 1;
	  }
     }
   new_cache = *cache;
   new_cache_num = *cache_num;
   new_cache_alloc = *cache_alloc;
   new_cache_num++;
   if (new_cache_num > new_cache_alloc)
     {
	new_cache_alloc += 64;
	new_cache = realloc(new_cache, new_cache_alloc * sizeof(Eet_File *));
	if (!new_cache)
	  {
	     fprintf(stderr, "BAD ERROR! Eet realloc of cache list failed. Abort\n");
	     abort();
	  }
     }
   new_cache[new_cache_num - 1] = ef;
   *cache = new_cache;
   *cache_num = new_cache_num;
   *cache_alloc = new_cache_alloc;
}

/* delete from cache */
static void
eet_cache_del(Eet_File *ef, Eet_File ***cache, int *cache_num, int *cache_alloc)
{
   Eet_File **new_cache;
   int new_cache_num, new_cache_alloc;
   int i, j;

   if (eet_cacheburst_mode) return;
   new_cache = *cache;
   new_cache_num = *cache_num;
   new_cache_alloc = *cache_alloc;
   if (new_cache_num <= 0)
     {
	return;
     }
   for (i = 0; i < new_cache_num; i++)
     {
	if (new_cache[i] == ef) break;
     }
   if (i >= new_cache_num)
     {
	return;
     }
   new_cache_num--;
   for (j = i; j < new_cache_num; j++) new_cache[j] = new_cache[j + 1];
   if (new_cache_num < (new_cache_alloc - 64))
     {
	new_cache_alloc -= 64;
	if (new_cache_num > 0)
	  {
	     new_cache = realloc(new_cache, new_cache_alloc * sizeof(Eet_File *));
	     if (!new_cache)
	       {
		  fprintf(stderr, "BAD ERROR! Eet realloc of cache list failed. Abort\n");
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

/* internal string match. bails out at first mismatch - not comparing all */
/* bytes in strings */
static int
eet_string_match(const char *s1, const char *s2)
{
   /* both null- no match */
   if ((!s1) || (!s2)) return 0;
   return (!strcmp(s1, s2));
}

/* caluclate hash table entry valu with bitmask size of hash_size */
static int
eet_hash_gen(const char *key, int hash_size)
{
   int hash_num = 0, i;
   unsigned char *ptr;
   const int masks[9] =
     {
	0x00,
	0x01,
	0x03,
	0x07,
	0x0f,
	0x1f,
	0x3f,
	0x7f,
	0xff
     };

   /* no string - index 0 */
   if (!key) return 0;

   /* calc hash num */
   for (i = 0, ptr = (unsigned char *)key; *ptr; ptr++, i++)
     hash_num ^= ((int)(*ptr) | ((int)(*ptr) << 8)) >> (i % 8);

   /* mask it */
   hash_num &= masks[hash_size];
   /* return it */
   return hash_num;
}

/* flush out writes to an eet file */
static void
eet_flush(Eet_File *ef)
{
   int i, count, size, num, offset;
   int head[3];
   unsigned long int i1, i2;
   Eet_File_Node *efn;

   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE))
     return;
   if (!ef->header) return;
   if (!ef->header->directory) return;
   if ((ef->mode != EET_FILE_MODE_WRITE) && (ef->mode != EET_FILE_MODE_READ_WRITE)) return;
   if (!ef->writes_pending) return;

   /* calculate total size in bytes of directory block */
   size = 0;
   count = 0;
   num = (1 << ef->header->directory->size);
   for (i = 0; i < num; i++)
     {
	for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
	  {
	     if (efn->compression >= 0)
	       {
		  size += 20 + strlen(efn->name);
		  count++;
	       }
	  }
     }
   /* caluclate offsets per entry */
   offset = 0;
   for (i = 0; i < num; i++)
     {
	for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
	  {
	     if (efn->compression >= 0)
	       {
		  efn->offset = 12 + size + offset;
		  offset += efn->size;
	       }
	  }
     }
   /* go thru and write the header */
   i1 = (unsigned long int)EET_MAGIC_FILE;
   i2 = htonl(i1);
   head[0] = (int)i2;
   i1 = (unsigned long int)count;
   i2 = htonl(i1);
   head[1] = (int)i2;
   i1 = (unsigned long int)size;
   i2 = htonl(i1);
   head[2] = (int)i2;
   fseek(ef->fp, 0, SEEK_SET);
   if (fwrite(head, 12, 1, ef->fp) != 1) return;
   offset = 12;
   for (i = 0; i < num; i++)
     {
	for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
	  {
	     if (efn->compression >= 0)
	       {
		  unsigned char *buf;
		  int buf_size;
		  int name_size;

		  name_size = strlen(efn->name);
		  buf_size = 20 + name_size;
		  buf = alloca(buf_size);
		  if (!buf) return;
		  i1 = (unsigned long int)efn->offset;
		  i2 = htonl(i1);
		  *((int *)(buf + 0)) = (int)i2;
		  i1 = (unsigned long int)efn->compression;
		  i2 = htonl(i1);
		  *((int *)(buf + 4)) = (int)i2;
		  i1 = (unsigned long int)efn->size;
		  i2 = htonl(i1);
		  *((int *)(buf + 8)) = (int)i2;
		  i1 = (unsigned long int)efn->data_size;
		  i2 = htonl(i1);
		  *((int *)(buf + 12)) = (int)i2;
		  i1 = (unsigned long int)name_size;
		  i2 = htonl(i1);
		  *((int *)(buf + 16)) = (int)i2;
		  memcpy(buf + 20, efn->name, name_size);
		  if (fwrite(buf, buf_size, 1, ef->fp) != 1) return;
		  offset += buf_size;
	       }
	  }
     }
   /* write data */
   for (i = 0; i < num; i++)
     {
	for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
	  {
	     if (efn->compression >= 0)
	       {
		  if (fwrite(efn->data, efn->size, 1, ef->fp) != 1)
		    return;
	       }
	  }
     }
   /* no more writes pending */
   ef->writes_pending = 0;
}

EAPI int
eet_init(void)
{
   return ++eet_initcount;
}

EAPI int
eet_shutdown(void)
{
   if (--eet_initcount == 0)
     {
	eet_cacheburst(0);
	_eet_memfile_shutdown();
     }

   return eet_initcount;
}

EAPI void
eet_cacheburst(int on)
{
   if (eet_cacheburst_mode == on) return;
   eet_cacheburst_mode = on;
   if (!eet_cacheburst_mode)
     {
	int i;
	int num;
	
	num = 0;
	for (i = 0; i < eet_writers_num; i++)
	  {
	     if (eet_writers[i]->references == 0) num++;
	  }
	for (i = 0; i < eet_readers_num; i++)
	  {
	     if (eet_readers[i]->references == 0) num++;
	  }
	if (num > 0)
	  {
	     Eet_File **closelist = NULL;
	     
	     closelist = alloca(num * sizeof(Eet_File *));
	     num = 0;
	     for (i = 0; i < eet_writers_num; i++)
	       {
		  if (eet_writers[i]->references == 0)
		    {
		       closelist[num] = eet_writers[i];
		       num++;
		    }
	       }
	     for (i = 0; i < eet_readers_num; i++)
	       {
		  if (eet_readers[i]->references == 0)
		    {
		       closelist[num] = eet_readers[i];
		       num++;
		    }
	       }
	     for (i = 0; i < num; i++) eet_close(closelist[i]);
	  }
     }
}

EAPI Eet_File *
eet_open(const char *file, Eet_File_Mode mode)
{
   Eet_File *ef;

   if (!file) return NULL;
   /* find the current file handle in cache*/
   ef = NULL;
   if (mode == EET_FILE_MODE_READ)
     {
	ef = eet_cache_find((char *)file, eet_writers, eet_writers_num);
	if (ef)
	  {
	     eet_flush(ef);
	     ef->delete_me_now = 1;
	  }
	ef = eet_cache_find((char *)file, eet_readers, eet_readers_num);
     }
   else if ((mode == EET_FILE_MODE_WRITE) || (mode == EET_FILE_MODE_READ_WRITE))
     {
	ef = eet_cache_find((char *)file, eet_readers, eet_readers_num);
	if (ef) ef->delete_me_now = 1;
	ef = eet_cache_find((char *)file, eet_writers, eet_writers_num);
     }
   /* we found one */
   if (ef)
     {
	/* reference it up and return it */
	ef->references++;
	return ef;
     }

   /* allocate struct for eet file and have it zero'd out */
   ef = calloc(1, sizeof(Eet_File) + strlen(file) + 1);
   if (!ef) return NULL;

   /* fill some of the members */
   ef->path = ((char *)ef) + sizeof(Eet_File);
   strcpy(ef->path, file);
   ef->magic = EET_MAGIC_FILE;
   ef->references = 1;
   ef->mode = mode;

   /* try open the file based on mode */
   if ((ef->mode == EET_FILE_MODE_READ) || (ef->mode == EET_FILE_MODE_READ_WRITE))
     ef->fp = fopen(ef->path, "rb");
   else if (ef->mode == EET_FILE_MODE_WRITE)
     {
	/* opening for write - delete old copy of file right away */
	unlink(ef->path);
	ef->fp = fopen(ef->path, "wb");
     }
   else
     {
	ef->delete_me_now = 1;
	eet_close(ef);
	return NULL;
     }

   /* if we can't open - bail out */
   if (!ef->fp)
     {
	ef->delete_me_now = 1;
	eet_close(ef);
	return NULL;
     }

   /* if we opened for read or read-write */
//   printf("OPEN!\n");
   if ((mode == EET_FILE_MODE_READ) || (mode == EET_FILE_MODE_READ_WRITE))
     {
	unsigned char buf[12];
	unsigned char *dyn_buf, *p;
	unsigned long int i1, i2;
	int num_entries, byte_entries, i;
	size_t count;

	/* build header table if read mode */
	/* geat header */
	count = fread(buf, 12, 1, ef->fp);
	if (count != 1)
	  {
	     eet_close(ef);
	     return NULL;
	  }
	/* get magic no */
	memcpy(&i1, buf + 0, sizeof(int));
	i2 = ntohl(i1);
	if (i2 != EET_MAGIC_FILE)
	  {
	     eet_close(ef);
	     return NULL;
	  }
	/* get entries count and byte count */
	memcpy(&i1, buf + 4, sizeof(int));
	i2 = ntohl(i1);
	num_entries = (int)i2;
	memcpy(&i1, buf + 8, sizeof(int));
	i2 = ntohl(i1);
	byte_entries = (int)i2;
	/* we cant have <= 0 values here - invalid */
	if ((num_entries <= 0) || (byte_entries <= 0))
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	/* we can't have more entires than minimum bytes for those! invalid! */
	if ((num_entries * 20) > byte_entries)
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	/* allocate dynamic buffer for entire directory block */
	dyn_buf = alloca(byte_entries);
	if (!dyn_buf)
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	/* allocate header */
	ef->header = calloc(1, sizeof(Eet_File_Header));
	if (!ef->header)
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	ef->header->magic = EET_MAGIC_FILE_HEADER;
	/* allocate directory block in ram */
	ef->header->directory = calloc(1, sizeof(Eet_File_Directory));
	if (!ef->header->directory)
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	/* 8 bit hash table (256 buckets) */
	ef->header->directory->size = 8;
	/* allocate base hash table */
	ef->header->directory->nodes = calloc(1, sizeof(Eet_File_Node *) * (1 << ef->header->directory->size));
	if (!ef->header->directory->nodes)
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	/* actually read the directory block - all of it, into ram */
	count = fread(dyn_buf, byte_entries, 1, ef->fp);
	if (count != 1)
	  {
	     ef->delete_me_now = 1;
	     eet_close(ef);
	     return NULL;
	  }
	/* parse directory block */
	p = dyn_buf;
//	printf("entries: %i\n", num_entries);
	for (i = 0; i < num_entries; i++)
	  {
	     int offset;
	     int flags;
	     int size;
	     int data_size;
	     int name_size;
	     int hash;
	     Eet_File_Node *efn;
	     void *data = NULL;

	     /* out directory block is inconsistent - we have oveerun our */
	     /* dynamic block buffer before we finished scanning dir entries */
	     if (p >= (dyn_buf + byte_entries))
	       {
		  ef->delete_me_now = 1;
		  eet_close(ef);
		  return NULL;
	       }
	     /* get entrie header */
	     memcpy(&i1, p + 0, sizeof(int));
	     i2 = ntohl(i1);
	     offset = (int)i2;
	     memcpy(&i1, p + 4, sizeof(int));
	     i2 = ntohl(i1);
	     flags = (int)i2;
	     memcpy(&i1, p + 8, sizeof(int));
	     i2 = ntohl(i1);
	     size = (int)i2;
	     memcpy(&i1, p + 12, sizeof(int));
	     i2 = ntohl(i1);
	     data_size = (int)i2;
	     memcpy(&i1, p + 16, sizeof(int));
	     i2 = ntohl(i1);
	     name_size = (int)i2;
	     /* invalid size */
	     if (size <= 0)
	       {
		  eet_close(ef);
		  return NULL;
	       }
	     /* invalid name_size */
	     if (name_size <= 0)
	       {
		  ef->delete_me_now = 1;
		  eet_close(ef);
		  return NULL;
	       }
	     /* reading name would mean falling off end of dyn_buf - invalid */
	     if ((p + 16 + name_size) > (dyn_buf + byte_entries))
	       {
		  ef->delete_me_now = 1;
		  eet_close(ef);
		  return NULL;
	       }
	     /* allocate all the ram needed for this stored node accounting */
	     efn = malloc(sizeof(Eet_File_Node) + name_size + 1);
	     if (!efn)
	       {
		  ef->delete_me_now = 1;
		  eet_close(ef);
		  return NULL;
	       }
	     efn->name = ((char *)efn) + sizeof(Eet_File_Node);
	     /* copy name in and terminate it */
	     strncpy(efn->name, (char *)p + 20, name_size);
	     efn->name[name_size] = 0;
	     /* get hask bucket it should go in */
	     hash = eet_hash_gen(efn->name, ef->header->directory->size);
	     efn->next = ef->header->directory->nodes[hash];
	     ef->header->directory->nodes[hash] = efn;
	     efn->offset = offset;
	     efn->compression = flags;
	     efn->size = size;
	     efn->data_size = data_size;

	     /* read-only mode, so currently we have no data loaded */
	     if (mode == EET_FILE_MODE_READ)
	       efn->data = NULL;
	     /* read-write mode - read everything into ram */
	     else
	       {
		  data = malloc(size);
		  if (data)
		    {
		       if (fseek(ef->fp, efn->offset, SEEK_SET) < 0)
			 {
			    free(data);
			    data = NULL;
			    /* XXX die gracefully somehow */
			    break;
			 }
		       if (fread(data, size, 1, ef->fp) != 1)
			 {
			    free(data);
			    data = NULL;
			    /* XXX die gracefully somehow */
			    break;
			 }
		    }
                  efn->data = data;
	       }
	     /* advance */
	     p += 20 + name_size;
	  }
     }

   /* we need to delete the original file in read-write mode and re-open for writing */
   if (ef->mode == EET_FILE_MODE_READ_WRITE)
     {
	fclose(ef->fp);
	unlink(ef->path);
	ef->fp = fopen(ef->path, "wb");
     }

   /* add to cache */
   if (ef->references == 1)
     {
	if (ef->mode == EET_FILE_MODE_READ)
	  eet_cache_add(ef, &eet_readers, &eet_readers_num, &eet_readers_alloc);
	else if ((ef->mode == EET_FILE_MODE_WRITE) || (ef->mode == EET_FILE_MODE_READ_WRITE))
	  eet_cache_add(ef, &eet_writers, &eet_writers_num, &eet_writers_alloc);
     }
   return ef;
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

EAPI void
eet_close(Eet_File *ef)
{
   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE))
     return;
   /* deref */
   ef->references--;
   /* if its still referenced - dont go any further */
   if (ef->references > 0) return;
   /* if we are in cacheburst mode - dont free it - leave it in cache */
   if (eet_cacheburst_mode)
     {
	if (!ef->delete_me_now) return;
     }
   /* remove from cache */
   if (ef->mode == EET_FILE_MODE_READ)
     eet_cache_del(ef, &eet_readers, &eet_readers_num, &eet_readers_alloc);
   else if ((ef->mode == EET_FILE_MODE_WRITE) || (ef->mode == EET_FILE_MODE_READ_WRITE))
     eet_cache_del(ef, &eet_writers, &eet_writers_num, &eet_writers_alloc);
   /* flush any writes */
   eet_flush(ef);

   /* free up members */
   if (ef->fp) fclose(ef->fp);

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
			    if (efn->data) free(efn->data);
			    ef->header->directory->nodes[i] = efn->next;
			    free(efn);
			 }
		    }
		  free(ef->header->directory->nodes);
	       }
	     free(ef->header->directory);
	  }
	free(ef->header);
     }

   /* zero out ram for struct - caution tactic against stale memory use */
   memset(ef, 0, sizeof(Eet_File));
   /* free it */
   free(ef);
}

EAPI void *
eet_read(Eet_File *ef, const char *name, int *size_ret)
{
   void *data = NULL;
   int size = 0;
   Eet_File_Node *efn;
   
   if (size_ret) *size_ret = 0;

   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!name) ||
       ((ef->mode != EET_FILE_MODE_READ) &&
        (ef->mode != EET_FILE_MODE_READ_WRITE)))
     {
	return NULL;
     }
   /* no header, return NULL */
   if (!ef->header) return NULL;
   /* no directory, return NULL */
   if (!ef->header->directory) return NULL;

   /* hunt hash bucket */
   efn = find_node_by_name(ef, name);
   if (!efn) return NULL;

   /* get size (uncompressed, if compressed at all) */
   size = efn->data_size;

   /* allocate data */
   data = malloc(size);
   if (!data) return NULL;

   /* uncompressed data */
   if (efn->compression == 0)
     {
	/* if we alreayd have the data in ram... copy that */
	if (efn->data)
	  memcpy(data, efn->data, efn->size);
	else if (!read_data_from_disk(ef, efn, data, size))
	  {
	     free(data);
	     return NULL;
	  }
     }
   /* compressed data */
   else
     {
	void *tmp_data;
	int free_tmp = 0, compr_size = efn->size;
	uLongf dlen;

	/* if we already have the data in ram... copy that */
	if (efn->data)
	  tmp_data = efn->data;
	else
	  {
	     tmp_data = malloc(compr_size);
	     if (!tmp_data)
	       {
		  free(data);
		  return NULL;
	       }

	     free_tmp = 1;

	     if (!read_data_from_disk(ef, efn, tmp_data, compr_size))
	       {
		  free(data);
		  return NULL;
	       }
	  }

	/* decompress it */
	dlen = size;
	if (uncompress((Bytef *)data, &dlen,
		 tmp_data, (uLongf)compr_size))
	  {
	     free(data);
	     return NULL;
	  }

	if (free_tmp) free(tmp_data);
     }

   /* fill in return values */
   if (size_ret) *size_ret = size;
   return data;
}

EAPI int
eet_write(Eet_File *ef, const char *name, void *data, int size, int compress)
{
   int data_size;
   int hash;
   Eet_File_Node *efn;
   void *data2;
   int exists_already = 0;

   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!name) || (!data) || 
       (size <= 0) || ((ef->mode != EET_FILE_MODE_WRITE) &&
		       (ef->mode != EET_FILE_MODE_READ_WRITE)))
     return 0;

   if (!ef->header)
     {
	/* allocate header */
	ef->header = calloc(1, sizeof(Eet_File_Header));
	if (!ef->header) return 0;
	ef->header->magic = EET_MAGIC_FILE_HEADER;
	/* allocate directory block in ram */
	ef->header->directory = calloc(1, sizeof(Eet_File_Directory));
	if (!ef->header->directory) return 0;
	/* 8 bit hash table (256 buckets) */
	ef->header->directory->size = 8;
	/* allocate base hash table */
	ef->header->directory->nodes = calloc(1, sizeof(Eet_File_Node *) * (1 << ef->header->directory->size));
	if (!ef->header->directory->nodes) return 0;
     }

   /* figure hash bucket */
   hash = eet_hash_gen(name, ef->header->directory->size);

   /* dup data */
   data_size = size;

   /* have bigger buffer for compress */
   if (compress) data_size = 12 + ((size * 101) / 100);

   data2 = malloc(data_size);
   if (!data2) return 0;

   /* if we want to compress */
   if (compress)
     {
	uLongf buflen;

	/* compress the data with max compression */
	buflen = (uLongf)data_size;
	if (compress2((Bytef *)data2, &buflen, (Bytef *)data,
			   (uLong)size, Z_BEST_COMPRESSION) != Z_OK)
	  {
	     free(data2);
	     return 0;
	  }
	/* record compressed chunk size */
	data_size = (int)buflen;
	if (data_size < 0 || data_size >= size)
	  {
	     compress = 0;
	     data_size = size;
	  }
	else
	  {
	     void *data3;

	     data3 = realloc(data2, data_size);
	     if (data3) data2 = data3;
	  }
     }
   if (!compress)
     memcpy(data2, data, size);

   /* Does this node already exist? */
   if (ef->mode == EET_FILE_MODE_READ_WRITE)
     {
	for (efn = ef->header->directory->nodes[hash]; efn; efn = efn->next)
	  {
	     /* if it matches */
	     if (eet_string_match(efn->name, name))
	       {
		  free(efn->data);
		  efn->compression = !!compress;
		  efn->size = data_size;
		  efn->data_size = size;
		  efn->data = data2;
		  exists_already = 1;
		  break;
	       }
	  }
     }
   if (!exists_already)
     {
	efn = malloc(sizeof(Eet_File_Node) + strlen(name) + 1);
	if (!efn)
	  {
	     free(data2);
	     return 0;
	  }
	efn->name = (char *)efn + sizeof(Eet_File_Node);
	strcpy(efn->name, name);
	efn->next = ef->header->directory->nodes[hash];
	ef->header->directory->nodes[hash] = efn;
	efn->offset = 0;
	efn->compression = !!compress;
	efn->size = data_size;
	efn->data_size = size;
	efn->data = data2;
     }

   /* flags that writes are pending */
   ef->writes_pending = 1;
   return data_size;
}

EAPI int
eet_delete(Eet_File *ef, char *name)
{
   int hash;
   int exists_already = 0;
   Eet_File_Node *efn, *pefn;
   
   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!name))
     return 0;

   /* deleting keys is only possible in RW or WRITE mode */
   if (ef->mode == EET_FILE_MODE_READ) return 0;

   if (!ef->header) return 0;

   /* figure hash bucket */
   hash = eet_hash_gen(name, ef->header->directory->size);

   /* Does this node already exist? */
   for (pefn = NULL, efn = ef->header->directory->nodes[hash]; efn; pefn = efn, efn = efn->next)
     {
	/* if it matches */
	if (eet_string_match(efn->name, name))
	  {
	     if (efn->data) free(efn->data);
	     if (efn == ef->header->directory->nodes[hash])
	       ef->header->directory->nodes[hash] = efn->next;
	     else
	       pefn->next = efn->next;
	     free(efn);
	     exists_already = 1;
	     break;
	  }
     }
   /* flags that writes are pending */
   if (exists_already) ef->writes_pending = 1;
   /* update access time */
   return exists_already;
}

EAPI char **
eet_list(Eet_File *ef, char *glob, int *count_ret)
{
   char **list_ret = NULL;
   int list_count = 0;
   int list_count_alloc = 0;
   int i, num;
   Eet_File_Node *efn;

   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) || (!glob) ||
       (!ef->header) || (!ef->header->directory) ||
       ((ef->mode != EET_FILE_MODE_READ) &&
        (ef->mode != EET_FILE_MODE_READ_WRITE)))
     {
	if (count_ret) *count_ret = 0;
	return NULL;
     }
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
	     if ((!strcmp (glob, "*")) || !fnmatch(glob, efn->name, 0))
	       {
		  char **new_list;

		  /* add it to our list */
		  list_count++;
		  /* only realloc in 32 entry chunks */
		  if (list_count > list_count_alloc)
		    {
		       list_count_alloc += 64;
		       new_list = realloc(list_ret, list_count_alloc * (sizeof(char *)));
		       if (!new_list)
			 {
			    free(list_ret);
			    if (count_ret) *count_ret = 0;
			    return NULL;
			 }
		       list_ret = new_list;
		    }
		  /* put pointer of name string in */
		  list_ret[list_count - 1] = efn->name;
	       }
	  }
     }
   /* return count and list */
   if (count_ret) *count_ret = list_count;
   return list_ret;
}

EAPI int
eet_num_entries(Eet_File *ef)
{
   int i, num, ret = 0;
   Eet_File_Node *efn;

   /* check to see its' an eet file pointer */
   if ((!ef) || (ef->magic != EET_MAGIC_FILE) ||
       (!ef->header) || (!ef->header->directory) ||
       ((ef->mode != EET_FILE_MODE_READ) &&
        (ef->mode != EET_FILE_MODE_READ_WRITE)))
     {
	return -1;
     }

   /* loop through all entries */
   num = (1 << ef->header->directory->size);
   for (i = 0; i < num; i++)
     {
	for (efn = ef->header->directory->nodes[i]; efn; efn = efn->next)
	  {
	     ret++;
	  }
     }

   return ret;
}

static Eet_File_Node *
find_node_by_name(Eet_File *ef, const char *name)
{
   Eet_File_Node *efn;
   int hash;

   /* get hash bucket this should be in */
   hash = eet_hash_gen(name, ef->header->directory->size);

   for (efn = ef->header->directory->nodes[hash]; efn; efn = efn->next)
     if (eet_string_match(efn->name, name)) return efn;

   return NULL;
}

static int
read_data_from_disk(Eet_File *ef, Eet_File_Node *efn, void *buf, int len)
{
   /* seek to data location */
   if (fseek(ef->fp, efn->offset, SEEK_SET) < 0) return 0;

   /* read it */
   return (fread(buf, len, 1, ef->fp) == 1);
}
