enum node_type;

/* different kinds of things that can appear in the value field
   of a hash node.  Actually, this may be useless now. */
union hashval {
   int                 ival;
   char               *cpval;
   DEFINITION         *defn;
};

struct hashnode {
   struct hashnode    *next;	/* double links for easy deletion */
   struct hashnode    *prev;
   struct hashnode   **bucket_hdr;	/* also, a back pointer to this node's hash
					 * chain is kept, in case the node is the head
					 * of the chain and gets deleted. */
   enum node_type      type;	/* type of special token */
   int                 length;	/* length of token, for quick comparison */
   char               *name;	/* the actual name */
   union hashval       value;	/* pointer to expansion, or whatever */
};

typedef struct hashnode HASHNODE;

/* Some definitions for the hash table.  The hash function MUST be
   computed as shown in hashf () below.  That is because the rescan
   loop computes the hash value `on the fly' for most tokens,
   in order to avoid the overhead of a lot of procedure calls to
   the hashf () function.  Hashf () only exists for the sake of
   politeness, for use when speed isn't so important. */

#define HASHSIZE 1403
#define HASHSTEP(old, c) ((old << 2) + c)
#define MAKE_POS(v) (v & 0x7fffffff)	/* make number positive */

extern int          hashf(const char *name, int len, int hashsize);
extern HASHNODE    *cpp_lookup(const char *name, int len, int hash);
extern void         delete_macro(HASHNODE * hp);
extern HASHNODE    *install(const char *name, int len, enum node_type type,
			    int ivalue, char *value, int hash);
extern void         cpp_hash_cleanup(cpp_reader * pfile);
