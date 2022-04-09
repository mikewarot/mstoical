
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

/* Vocabulary handling functions. */

/* 
 * Create new vocabulary. Returns a pointer to the vocabulary created.
 * Note that this function will not add the vocabulary to the vocabulary
 * stack for you.
 */
struct vocabulary*
voc_create (char *name)
{
	struct vocabulary *voc;

	voc = malloc( sizeof(struct vocabulary) );

	voc->name = malloc( strlen( name ) + 1 );
	strcpy( voc->name, name );

	voc->words = 0;
	voc->last  = NULL;

	return voc;
}
/*
 * Append an entry to the specified vocabulary (usually the one pointed
 * to by 'current'). extra is an additional amount of memory that should
 * be allocated along with the entry. It is specified in bytes.
 */
struct voc_entry *
voc_append (struct vocabulary *voc, struct voc_item *item,
	     int extra)
{
	int siz;
	struct voc_entry *entry;

	entry = malloc( sizeof(struct voc_entry) + extra );

	entry->type = item->type;
	entry->code = item->code;

	entry->len  = strlen( item->name );

	siz = max(entry->len,NSIZE);

	entry->name = malloc( siz + 1 );
	
	strncpy( entry->name, item->name, siz );
	entry->name[siz] = '\0';

	if ( ( entry->last = voc->last ) != NULL )
		entry->last->next = entry;

	voc->last = entry;
	entry->next = NULL;

	entry->collectable	= NULL;
	entry->collectables	= 0;
	
	voc->words++;

	return entry;
}

/*
 * Recursively destroy a vocinoary. All memories allocated to it and its
 * child branches should be freed. Beware, this function calls entry_free
 * recusively in the case of vocabulary sub branches. 
 */
void 
voc_destroy (struct vocabulary *voc)
{
	struct voc_entry *last;
	struct voc_entry *entry;

	entry = voc->last;

	while ( entry != NULL )
	{
		last = entry->last;
		entry_free( entry );
		entry = last;
	}
		
	free( voc->name );
	free( voc );
}

/* 
 * Free storage associated with entry. Does not remove the entry
 * from the vocabulary linkage
 */
void
entry_free( struct voc_entry *entry )
{
	ccell col;
	/* Free all resources that can be associated to this
	 * entry */
#ifdef THREADS
	if ( entry->type & A_SHARE )
	{
		pthread_rwlock_destroy((void*)(&entry->parm + sizeof( cell )) );
	}
#endif

	if ( (entry->type == A_VAR || entry->type == A_CONST)
			&& entry->parm.type == T_STR )
		/* Free string constants and variables */
		free( (void*)entry->parm.v.s );
	else if ( ( entry->type & A_TMASK ) == A_VOCAB )
		/* Entry is a vocabulary branch. Destroy it */
		voc_destroy( (void*)entry->parm.v.p );
	else if ( ( entry->type & A_TMASK ) == A_HASH )
		hash_destroy( (&entry->parm)[1].v.p );

	/* collect collectables */
	for ( ; entry->collectables; entry->collectables-- )
	{
		col = entry->collectable[ entry->collectables ];

		(col.handler)(col.ptr);
	}
	
	free( entry->collectable );
	free( entry->name );
	free( entry );
}

/* 
 * Push a handler and pointer onto the specified entry's collectable list.
 * Later, when the entry is destroyed, the handler will be called with the
 * pointer as its argument.
 */
void
entry_collectable ( struct voc_entry *entry, function *handler, void *ptr )
{
	entry->collectable =
	realloc( entry->collectable, (entry->collectables + 1) *
				     sizeof( ccell ));

	entry->collectable[ entry->collectables++ ] = (ccell){ handler, ptr };
}

/* pop an entry from the given word's collectabels list.. doesn't actually
 * free anything */
void entry_pop_collectable ( struct voc_entry *entry )
{
	entry->collectable =
	realloc( entry->collectable, (--entry->collectables) *
				     sizeof( ccell ));
}


/*
 * Remove an entry from a vocabulary
 */
void
entry_delete (struct vocabulary *voc, struct voc_entry *entry)
{
	/* Remove this entry from the link chain */
	if ( entry->next != NULL )
		entry->next->last = entry->last;
	if ( entry->last != NULL )
		entry->last->next = entry->next;
	if ( voc->last == entry )
		voc->last = entry->last;

	voc->words--;
	
	entry_free(entry);	
}
/* Look up named entry in specified vocabulary */
struct voc_entry *
voc_lookup (struct vocabulary *voc, int len, char *name)
{
	struct voc_entry *entry;
	entry = voc->last;

	for (;;)
	{
		if ( entry == NULL )
			break;

		if ( ( entry->len == len ) && 
		     ( strncmp( entry->name, name, 
				max(len, NSIZE) )
		     == 0 ) )
		{
			/* Found word */
			return entry;
		}
		else
			entry = entry->last;
	}
	
	return NULL;
}

/* Lookup name in the dictionary */
/* Private version */
struct voc_entry *
dict_lookup (char *name, struct vocabulary **vst, struct vocabulary **vstmin)
{
	int i;
	int count;
	int len;
	struct vocabulary *voc;
	struct voc_entry *entry;

	len = strlen(name);

	count = vst - vstmin;
	
	for ( i = 0; i < count; )
	{
		if ( ( voc = idx(vst,i++) ) != NULL)
			if ( ( entry = voc_lookup(voc, len, name) ) != NULL )
				return entry;
	}

	return(NULL);
}
/* Alter the size of the parameter space allocated to entry */
struct voc_entry *
entry_resize ( struct voc_entry *entry, int size )
{
	entry = realloc( entry, size + sizeof( struct voc_entry ) );
	if ( entry->last != NULL )
		entry->last->next = entry;
	return entry;
}

