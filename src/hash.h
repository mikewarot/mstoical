
struct hash_cell_s {
	char *key;		/* this cell's ascii key ( name ) */
	ub4 hash;		/* the hashed value of that key */
	struct hash_cell_s *next;/* fellow behind us in the queue. */
	cell c;		 	/* our hashes contain STOICAL cells (which may
				   point elsewhere. */
};
typedef struct hash_cell_s hash_cell;

typedef struct {
	int buckets;		/* buckets allocated ( 1 >> x ) */
	ub4 size;		/* actual number of buckets */
	ub4 mask;		/* bitmask for hashed keys */
	ub4 used;		/* buckets defined */
	hash_cell *tab;	/* pointer to the table of cells that make up
				 * this hash. */
} hash_tab;

hash_tab *hash_create( int buckets );
void hash_destroy( hash_tab *h );
cell *hash_get( hash_tab *h, const char *key );
int hash_put( hash_tab *h, char *key, cell *a );
hash_tab *hash_grow( hash_tab *h );
hash_tab *hash_shrink( hash_tab *h );
int hash_delete ( hash_tab *h, char *key );
char *hash_next ( hash_tab *h, ub4 *member, ub4 *bucket );
void hash_next_reset ( hash_tab *h, ub4 *member, ub4 *bucket );

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}


