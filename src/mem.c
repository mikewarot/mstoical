
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

/* Memory managemnt / Garbage collection */

#ifdef THREADS
	extern pthread_key_t tsd;
#endif

/* 
 * Allocate memory. Works like malloc().  All memories allocated will be
 * subject to later destruction by the garbage collector. This routine should
 * not be used for any memory that must persist (ie, strings inside colon
 * definitions).  Such things will be freed, if necessary, by the vocabulary
 * handling code.
 */
void *
gmalloc( int size, void **gstack, unsigned char *gst )
{
	void *mem;

	if ( gstack[ *gst ] != NULL )
	{
		free( gstack[ *gst ] );
		gstack[ *gst ] = NULL;
	}

	mem = malloc( size );
	gstack[ *gst ] = mem;
	*gst += 1;

	return mem;
}
/* 
 * Similar to gmalloc(), except that it marks memory that has alredy been
 * allocated.
 */
void *
gmark ( void *mem, void **gstack, unsigned char *gst )
{
	if ( gstack[ *gst ] != NULL )
	{
		free( gstack[ *gst ] );
		gstack[ *gst ] = NULL;
	}

	gstack[ *gst ] = mem;
	*gst += 1;

	return mem;
}
ccell *
free_collectables ( ccell *ccstmin, ccell **ccst )
{
	int i;
	ccell col;
	
	i = *ccst - ccstmin;
	
	for (; i; i-- )
	{
		col = pop(*ccst);
		(col.handler)(col.ptr);
	}

	return *ccst;
}

/* initialize gstack to all null pointers. */
void ginit ( void **gstack, unsigned char *gst )
{
	void **gstmax;
	gstmax = gstack + GSIZE - 1;
	for ( ; gstack < gstmax; gstack++ )
		*gstack = NULL;
}
/* Free all memories listed on the gstack. */
void 
gfree ( void **gstack, unsigned char *gst )
{
	void **gstmax;
	gstmax = gstack + GSIZE - 1;
	for (; gstack < gstmax; gstack++ )
	{
		if ( *gstack != NULL )
			free( *gstack );
	}
}


#ifdef THREADS
/* Thread destructor function. Called when a thread ends.
 * Should free all the resources associated with the thread.
 * This includes its stack copies, as well as the contents
 * of its gstack, new vocabularies, and variables. */
void
thread_destroy( struct thread_data *data )
{
	struct vocabulary *v;

//	if ( data->main )
//		return;

	printk("destroying thread:");

	printk("\tfreeing vocabularies");
	/* Free vocabularies allocated to thread by descending the vstack
	 * until we hit a NULL (that THREAD left for us) */
	while ( (v = pop(data->vst) ) )
			voc_destroy( v );
	
	printk("\tfreeing stacks");
	free( data->sstmin - 4 );
	free( data->vstmin - 4 );
	gfree( data->gstack, data->gst );		
	
	printk("\tfreeing misc thread data");

	pthread_setspecific(tsd, NULL);
	
	free( data );
}
#endif

/* duplicate a portion of the given stack.. basically a memcpy that also copies
 * the things that cells may to.
 */
void
stack_dup( cell *dst, cell *src, int n )
{
	for ( ; n; )
	{
		n--;
		/* copy the cell */
		dst[n] = src[n];
			
		/* dupliate strings */
		if ( ( dst[n].type & T_TMASK ) == T_STR )	
			dst[n].v.s = stringdup( dst[n].v.s );

		/* FIXME: maybe file handles should be duplicated or reopened..
		 * Should threads really be sharing such things anyway? */
	}
}

/*
 * Assume that all collectable items on the given cell stack are NOT already
 * marked for collection, and mark them. Care should be taken not doubly mark
 * any pointers.
 */
void
stack_mark( cell *stmin, cell *st, void **gstack, unsigned char *gst )
{
	long t;
	for (; st > stmin; st-- )
	{
		t = st->type & T_TMASK;	
		if ( t == T_STR || t == T_CLS )
			gmark ( st->v.p, gstack, gst );
	}
}

