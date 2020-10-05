
/* STOICAL
 * Copyright (C) 2002 Jonathan Moore Liles. <wantingwaiting@users.sf.net>
 * 
 * STOICAL is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * STOICAL is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with STOICAL; see the file COPYING.  If not,write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#include "kernel.h"
#include "hash.h"

/* Routines to manage hashing and hash tables. Don't you just hate reinventing
 * the wheel? */

/*
 * Convert character string into hash value.
 */
ub4  hash_lookup( k, length, level)
register unsigned char *k;        /* the key */
register ub4  length;   /* the length of the key */
register ub4  level;    /* the previous hash, or an arbitrary value */
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = level;           /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((ub4)k[10]<<24);
   case 10: c+=((ub4)k[9]<<16);
   case 9 : c+=((ub4)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((ub4)k[7]<<24);
   case 7 : b+=((ub4)k[6]<<16);
   case 6 : b+=((ub4)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((ub4)k[3]<<24);
   case 3 : a+=((ub4)k[2]<<16);
   case 2 : a+=((ub4)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}

#define hashsize(n) ((ub4)1 << (n))
#define hashmask(n) ((n) - 1)

/* 'Buckets' should be an optimistic estimate, and not simple the number of
 * initial values for the hash. */
hash_tab *hash_create( int buckets )
{
	hash_tab *h;
	
	/* buckets should be a prime (^2) number */

//	printf("buckets 2^%i\n", buckets);
	
	/* allocate an initialize hash */
	h = malloc( sizeof( hash_tab ) );

	h->buckets = buckets;
	h->size	   = hashsize(buckets);
	h->mask	   = hashmask(h->size);
	h->used	   = 0;
	h->tab	   = calloc( h->size, sizeof( hash_cell ) );
	
//	printf("size %lx\n", h->size );
//	printf("mask %lx\n", h->mask );
		
	return h;
}


/* free all resources allocated to a hash (including strings and keys) */
void hash_destroy( hash_tab *h )
{
	int i;
	hash_cell *p;
	
	/* search for and free strings.. FIXME: maybe we should use a
	 * collectables stack here to avoid doing all this searching, but
	 * that would nearly double the hash's space requirements. */
	for ( i = 0; i < h->buckets; i++ )
	{
		p = &h->tab[i];
		for( ; p; p = p->next )
		{
			if ( p->c.type == T_STR )
				free ( p->c.v.s );
			free( p->key );
		}
	}

	free( h->tab );
	free( h );
}

/* get value from hash. returns a null pointer if the key doesn't exist */
cell *hash_get( hash_tab *h, const char *key )
{
	hash_cell *p;
	
	p = &h->tab[ hash_lookup( key, strlen(key), 1 ) & h->mask ];

	/* search through queue and return a pointer to the matching key's
	 * cell */
	for(; p; p = p->next )
		if ( p->key && ( strcmp( key, p->key ) == 0 ) )
			return &p->c;

	return NULL;
}

/* set the value associated with a hash key. returns 1 if the key already
 * existed and was overwritten. Otherwise, zero. Must handle collisions.
 * If you're adding a cell that points to a string you must make sure that
 * the string has first been protected from the garbage collector.
 */
int hash_put( hash_tab *h, char *key, cell *a )
{
	/* must free overwritten strings */
	hash_cell *p, *tp, *op = NULL;
	int r = 0;
	ub4 hkey;

	/* grow the hash if it has more keys than buckets */
//	if ( h->used > h->size )
//		h = hash_grow( h );

	hkey = hash_lookup( key, strlen(key), 1 );
	p = tp = &h->tab[ hkey & h->mask ];

	/* preincrement usage count */
	h->used++;
	
	for ( ; p; p = p->next )
	{
		if ( p->key && ( strcmp( key, p->key ) == 0 ) )
			r = 1;
		op = p;
	}
	p = op;

	if ( r == 1 )
	{
		/* found existing key.. lets free it's
		 * store and set ourselves up to replace it */
		free( p->key ); p->key = NULL;
		
		/* balance use count */
		h->used--;
		
		if ( p->c.type == T_STR )
			free ( p->c.v.s );
	}
	else if ( p->key )
	{
		/* collision */
		/* allocate space for and append a cell
		 * to the queue. */
		p->next = malloc( sizeof( hash_cell ) );
		p = p->next;
	}
	
	/* proform the actual copy operation */
	p->c	= *a;
	p->hash = hkey;
	p->key	= malloc( strlen(key) + 1 );
	p->next = NULL;
	strcpy( p->key, key );
		
	return r;
}
/* delete key from hash. strings will be freed and the hash will shrink
 * as needed. Returns non zero if key didn't exist */
int hash_delete ( hash_tab *h, char *key )
{
	hash_cell *p;
	hash_cell *op = NULL;
	int r = 1;

	p = &h->tab[ hash_lookup( key, strlen(key), 1 ) & h->mask ];
	
	for( ; p; p = p->next )
	{
		if ( p->key && ( strcmp( key, p->key ) == 0 ) )
		{
			if ( p->c.type == T_STR )
				free ( p->c.v.s );
			r = 0;
			free( p->key );
			p->key = NULL;
			
			h->used--;

			if ( op )
			{
				op->next = p->next;
				free( p );
			}
			else if ( p->next )
			{
				/* if we're the top level and there's
				 * a queue behind us */
				*p = *p->next;
			}
			break;
		}
		op = p;
	}
	
	/* shrink the hash if it is less than 25% full */
	if ( h->used && h->used < h->size >> 2 )
		h = hash_shrink( h );

	return r;
}

/* resize and rebuild the hash ( by doubling it ) */
hash_tab *hash_grow( hash_tab *h )
{
	int i;
	hash_cell *o;	/* pointer to old hash table */
	hash_cell *n;	/* pointer to hash table being built */
	hash_cell *p;	/* pointer to the cell being evaluated */
	hash_cell *np;	/* temporary */

	h->size	   = hashsize(++h->buckets);
	h->mask	   = hashmask(h->size);
	
	/* allocate cleared space for the new table */
	n	= calloc( h->size, sizeof( hash_cell ) );
	
	o	= h->tab;

	/* iterate through the hash backwards, rehashing it into the new
	 * space as we go */
	for ( i = h->size >> 1; i--; )
	{
		p = &o[i];
		
		if ( p->key == NULL )
		{
			/* do nothing. this is an empty head cell. */
		}
		else if ( p->next == NULL )
		{
			/* this top level cell has no children, meaning that
			 * it is impossible for it to cause a collision in the
			 * new hash. therefore we'll just copy it. */
			n[ p->hash & h->mask ] = *p;
		}
		else
		{
			/* this top level cell has children. follow it's list,
			 * letting members of it's queue break out if their
			 * rehashed locoations permit. Cells that do break out
			 * must have there old copies freed. */

			/* handle the head separately */
			np = &n[ p->hash & h->mask ];
			*np = *p;
			np->next = NULL;
			p = p->next;
		
			while ( p )
			{
				/* copy the cell to its rehashed location in
				 * the new hash */
				np = &n[ p->hash & h->mask ];
		
				if ( np->next )
				{
					/* the new cell already has members.
					 * lets join the queue */

					/* find the list's tail */
					do { np = np->next; } while (np->next);
		
					/* copy and append this cell */
					np->next = malloc( sizeof(hash_cell) );
					np = np->next;
				}
				else if ( np->key )
				{
					/* this is also a collision, but
					 * with a cell who doesn't yet have
					 * children. */
					np->next = malloc( sizeof(hash_cell) );
					np = np->next;
				}
				
				*np = *p;
				np->next = NULL;
			
				/* juggle p so that we can free it */
				np = p->next;
				free ( p );
				p = np;
			}
		}

	}
	
	/* free the old table */
	free( o );
	h->tab = n;

	return h;
}

/* shrink a hash to half of its current size. you should avoid shrinking
 * hashes aren't really too big; Shrinking an inadequately sized hash will
 * degrade proformance, and possibly consume more memory. */ 
hash_tab *hash_shrink( hash_tab *h )
{
	int i;
	hash_cell *o;	/* pointer to old hash table */
	hash_cell *n;	/* pointer to hash table being built */
	hash_cell *p;	/* pointer to the cell being evaluated */
	hash_cell *np;	/* temporary */

	ub4 nsize;

	nsize	   = hashsize(--h->buckets);
	h->mask	   = hashmask(nsize);
	
	/* allocate cleared space for the new table */
	n	= calloc( nsize, sizeof( hash_cell ) );
	
	o	= h->tab;
	
	/* iterate through the hash backwards, rehashing it into the new
	 * space as we go. note that, when condsensing the hash it isn't
	 * necessary to free any of the old cells, because there will never
	 * be fewer collisions in a smaller hash. */
	for ( i = h->size; i--; )
	{
		p = &o[i];
		
		if ( p->key )
		{
			while ( p )
			{
				/* copy the cell to its rehashed location in
				 * the new hash */
				np = &n[ p->hash & h->mask ];
		
				if ( np->next )
				{
					/* the new cell already has members.
					 * lets join the queue */

					/* find the list's tail */
					do { np = np->next; } while (np->next);
		
					/* copy and append this cell */
					np->next = malloc( sizeof(hash_cell) );
					np = np->next;
				}
				else if ( np->key )
				{
					/* this is also a collision, but
					 * with a cell who doesn't yet have
					 * children. */

					np->next = malloc( sizeof(hash_cell) );
					np = np->next;
				}
				
				*np = *p;
				np->next = NULL;
				p = p->next;	
			}
		}

	}
	
	/* free the old table */
	free( o );
	h->tab	= n;
	h->size = nsize;	
	
	return h;
}

void foo ( void ) { }

/* return a pointer to the next key in the hash. this a pointer to the
 * actual data stored in the hash, so altering it will cause undefined
 * behavior. 'j' and 'i' are a pointers to unsigned longs used by hash_next
 * as an indices into the internal hash table. both should be initialized to
 * zero before being used with hash_next. Returns a null pointer if there are
 * no more keys in the hash.
 */
char *hash_next ( hash_tab *h, ub4 *member, ub4 *bucket )
{
	ub4 size = h->size;
	hash_cell *p;
	ub4 l;
	
	for ( ; *bucket < size; )
	{
		p = &h->tab[ *bucket ];
		
		if ( *member )
		{
			/* find our previous possition */
			for ( l = 0; l < *member; l++ )
				p = p->next;
		}
			
		if ( p->key )
		{
			if ( p->next )
				(*member)++;
			else
			{
				*member = 0;
				(*bucket)++;
			}
			return p->key;
		}
		else
			(*bucket)++;
	}

	*bucket = 0;
	*member = 0;

	return NULL;
}

void hash_next_reset ( hash_tab *h, ub4 *member, ub4 *bucket )
{
	*bucket = 0;
	*member = 0;
}

