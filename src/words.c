
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

/*
 * This file defines all of the words available from the language itself.
 * Unfortunately, due to the extreme limitations of the C language, this
 * file must be include in kernel.c by a preprocessor directive.
 * Bare this in mind. Also note that the comments in this source file are
 * used to automatically generate the language documentation, and their format
 * should be respected.
 */

/* description of word catatories */

/*** thread		Thread handling words */
/*** stack		Stack manipulation words */
/*** var		Load an store operations */
/*** array		Array handling words */
/*** constants		Constants defined by the compiler */
/*** variables		Variables defined by the compiler */
/*** binary		Binary operators */
/*** unary		Unary operators */
/*** iterative		Words for constructing iterations */
/*** conditional	Words for constructing conditionals */
/*** literal		Literal handling words */
/*** source		Words that change how STOICAL sees program source */
/*** dictionary		Dictionary manipulation words */
/*** io			Input / Output words */
/*** error		Error handling words */
/*** regex		Regular expression words */
/*** system		Words for interacting with Unix */
/*** string		String handling words */
/*** compiler		Compiler words */
/*** math		Arithmetic words */

#ifdef THREADS
/**(thread) thread
 * "() foo THREAD detach"
 * Launch a new thread.
 * Takes as an argument the address of the word that the thread should begin
 * execution with. Returns a reference to the new thread. In order for a
 * thread's resources to be reclaimed, it is necessary to either JOIN or
 * DETACH it.
 */
begin(thread)
	struct thread_data *data;
	
	data	= calloc( 1, sizeof( struct thread_data ) );

	data->entry = ppop(sst);
	
	data->sstmin = calloc( SSIZE + 8, sizeof( cell ) );
	data->vstmin = calloc( SSIZE + 8, sizeof( struct vocabulary * ) );
	data->sstmin += 4;
	data->vstmin += 4;
	
	stack_dup( data->sstmin, sstmin, 1 + sst - sstmin );
	
	memcpy( data->vstmin, vstmin, ((vst - vstmin) + 1 ) * sizeof( struct vocabulary * ) );
			
	data->sst = data->sstmin + ( sst - sstmin );
	data->vst = data->vstmin + ( vst - vstmin );

	/* Push a null onto the copied vstack to serve as a marker, so
	 * that the destructor knows how much of the vstack to reclaim. */
	push(data->vst,NULL);
	push(data->vst,voc_create("private<"));

	/* allocate space for, and copy, io pointers */
	data->io	= malloc( sizeof( struct iopoint ) );
	memcpy( data->io, io, sizeof( struct iopoint ) );
	
	pthread_create( &data->thread, NULL, (void*)st_start, data );

	printk("launching thread %i", (ub4)data->thread);

	fpush(sst,(ub4)data->thread);
end()
/**(thread) detach
 * "() foo thread DETACH"
 * Detach the thread referred to by TOS. Once a thread has been detached
 * you cannot retrieve it's return value.
 */
begin(detach)
	pthread_detach( (pthread_t)((ub4)fpop(sst)) );
end()
/**(thread) join
 * "() foo thread JOIN"
 * Wait for thread referred to by TOS to end, and then detach it, freeing
 * its resources. Returns the threads exit status.
 */
begin(join)
	void *status;
	pthread_join( (pthread_t)((ub4)fpop(sst)), &status );
	fpush(sst,(long)status);
end()
/**(thread) me
 * "ME cancel"
 * Push a reference to the calling thread onto the stack
 */
begin(me)
	fpush(sst,(ub4)pthread_self());
end()
/**(thread) cancel
 * "me CANCEL"
 * Cancel the thread referred to by TOS.
 */
begin(cancel)
	pthread_cancel( (pthread_t)((ub4)fpop(sst)) );
end()
#endif
/**(thread) delay
 * "1 0 delay"
 * Delay calling thread (not entire process) for TOS-1 seconds and TOS
 * nanoseconds.
 */
begin(delay)
	struct timespec t;

	t.tv_nsec = fpop(sst);
	t.tv_sec  = fpop(sst);
#ifdef PTHREAD_DELAY
	pthread_delay_np(&t);
#else
#	ifdef HAVE_PSELECT
	pselect(0, NULL, NULL, NULL, &t, NULL);
#	else
	{
		struct timeval tv;

		tv.tv_sec  = t.tv_sec;
		tv.tv_usec = t.tv_nsec > 0 ? t.tv_nsec / 1000 : 0;

		select(0, NULL, NULL, NULL, &tv);
	}
#	endif	
#endif
end()
/**(compiler) exit
 * Exit thread. Calling exit from the main thread, or with threads disabled,
 * causes program termination. 
 */
begin(exit)
#ifdef THREADS
	pthread_exit((void*)((ub4)fpop(sst)));
#else
	exit((int)fpop(sst));
#endif
end()
#ifdef SAFE
#define st_errch() { \
	if ( sst < sstmin ) \
		error("stack empty\n"); \
	else if ( sst > sstmin + SSIZE ) \
		error("stack full\n"); \
	\
	if ( rst > rstmin + SSIZE ) \
		error("return stack full"); \
	\
	if ( vst == vstmin ) \
	{ \
		push(vst,stoical); \
		error("vocabulary stack empty\n"); \
	} \
	else if ( vst > vstmin + SSIZE ) \
	{ \
		drop(vst); \
		error("vocabulary stack full\n"); \
	} \
	\
	if ( lst < lstmin ) \
		error("loop stack empty\n");  \
	else if ( lst > lstmin + SSIZE ) \
		error ("loop stack full\n"); \
 \
}
#else
#define st_errch()
#endif
/**(error) errch
 * Test for under/overflow on all stacks and print appropriate error
 * message.
 */
begin(errch)
	st_errch();
end()
/**(math) rand
 * Return a pseudo-random number.
 */
begin(rand)
	fpush(sst,(int)rand());
	exec(*adr(int));
end()
/**(math) srand
 * "123 srand"
 * Reset the random seed to the value of TOS.
 */
begin(srand)
	srand(fpop(sst));
end()
/*
 * Okay, the typing system is currently a bit hard to explain.
 * So, forgive me if it seems at all unclear.
 */
begin(dispatch)
	type_head *head;
	type_vec *tvec, *vec;
	ub4 i, type = 0;

	/* the parameter field of a typechecked word looks like :
	 *
	 * max type-mask type-vec {clause} type-mask type-vec {clause} ...
	 */
	head = (void*)&self->parm;
	
	/* generate vector address from stack types */
	for ( i = 0; i < head->max; i++ )
		type |= (ub4) (idx(sst,i).type) << (i * 8);
		
	/* search until we find a match */
	
	vec = head->vec;
		
	for ( i = 0; i < head->num; i++ )
	{
		tvec = &vec[ i ];

		printk("comparing type %lx with %lx", tvec->type,
						      type & tvec->mask);
				
		if ( tvec->type == (type & tvec->mask) )
		{
			/* found the correct entry */
			/* save IP on the return stack */
			push(rst,((rcell){ self, ip }));
			
			execute((struct voc_entry *)tvec->clause);
		}
	}

	/* falling through means that a suitable type couldn't be found */
	exec(*adr(badtype));
end()
/**(stack) cells
 * "CELLS idrop"
 * Push the number of occupied parameter stack cells onto the stack.
 */
begin(cells)
	fpush(sst,(sst - sstmin));
end()
/**(stack) mark
 * Leave a mark on the stack that is unique from any other object that
 * might occupy a stack cell. This mark can later be used to restore the
 * stack to the level it was at when the mark was placed.
 */
begin(mark)
	cmark(sst);
end()
/**(conditional) mark?
 * Return TRUE or FALSE based on whether TOS is a mark.
 */
begin(mark_query)
	if ( pop(sst).type == T_MARK )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/* Public version */
begin(lookup)
	struct voc_entry *entry;

	if ( (entry = stt_lookup(scpop(sst))) != NULL )
	{
		rpush(sst,entry);
		fpush(sst,TRUE);
	}
	else
		fpush(sst,FALSE);
end()
/**(compiler) ^ - (*)
 * Clear the end-of-command flag. Thereby allowing the continuation of,
 * otherwise executable, lines.
 */
begin(circumflex)
	printk("s_circumflex");

	eoc = FALSE;
end()
/**(compiler) % and #! (*)
 * "#! /usr/bin/stoical"
 * Set the end-of-line flag to true. This causes the remainder of the
 * line to be ignored and discarded. Used for comments.
 */
begin(comment)
	printk("s_comment()");

	eol = TRUE;
end()
/**(compiler) %% (*)
 * "%% line of text"
 * Call MSG against the remainder of the line, including a newline character.
 */
begin(print)
	eol = TRUE;

	/* copy the remainder of the line into the pad */
	memcpy( &pad->s, &tib->s + tibp, 1 + tib->l - tibp );

	/* set the length of the new pad and append a newline */
	pad->l = strlen( &pad->s );

	(&pad->s)[pad->l]	= '\n';
	(&pad->s)[++pad->l]	= '\0';

	/* push the address of pad, and call MSG */
	spush(sst,pad);
	exec(*adr(msg));
end()
/**(constants) true
 * Push -1.
 */
begin(true)
	fpush(sst,TRUE);
end()
/**(constants) false
 * Push 0.
 */
begin(false)
	fpush(sst,FALSE);
end()	
begin(one)
	fpush(sst,1);
end()
begin(two)
	fpush(sst,2);
end()
begin(zero)
	fpush(sst,0);
end()
/**(unary) 1+
 * Increment TOS by one.
 */
begin(one_plus)
	peek(sst).v.f++;
end()
/**(unary) 1-
 * Decrement TOS by one.
 */
begin(one_minus)
	peek(sst).v.f--;
end()
/**(binary) add
 * "1 2 +"
 * Perform floating point addition of TOS and TOS-1.
 */
begin(add)
	float v;
	
	v = fpop(sst) + fpop(sst);
		
	fpush(sst,v);
end()
/**(binary) -
 * "4 2 - ( 2 )"
 * Subtract TOS from TOS-1.
 */
begin(sub)
	float v;

	v = idx(sst,1).v.f - fpop(sst);
	drop(sst);
	
	fpush(sst,v);
end()
/**(binary) /
 * "10 5 / ( 5 )"
 * Perform floating point division of TOS-1 by TOS.
 */
begin(div)
	float v;

	v = idx(sst,1).v.f / fpop(sst);
	drop(sst);

	fpush(sst,v);
end()
/**(binary) *
 * "5 2 * ( 10 )"
 * Multiply TOS-1 by TOS.
 */
begin(mul)
	float v;

	v = fpop(sst) * fpop(sst);

	fpush(sst,v);
end()
/**(binary) mod
 * "10 4 mod ( 2 )"
 * Integer divide TOS-1 by TOS, and
 * return the remainder.
 */
begin(mod)
	float v;

	v = (long)idx(sst,1).v.f % (long)fpop(sst);
	drop(sst);

	fpush(sst,v);
end()
/**(binary) /mod
 * "10 5 /mod - ( 2 0 )"
 * Integer divide. Leaves both quotient and remainer on stack.
 */ 
begin(divmod)
	float a, b;

	b = fpop(sst);
	a = fpop(sst);
	
	fpush(sst,((long)a / (long)b));
	fpush(sst,((long)a % (long)b));
end()
/**(unary) int
 * "( 1.3 - 1 )"
 * Round float at TOS to an integer value. Not very pretty, as the gritty bits
 * are left to your specific C compiler. But, since I've been GCC-ific so far,
 * why stop now? FIXME: My name is pandora. This is my box.
 */
begin(int)
	peek(sst).v.f = (float)((long)(peek(sst).v.f));
end()
/**(unary) abs
 * "( -3 - 3 )"
 * Return absolute value of TOS.
 */
begin(abs)
	peek(sst).v.f = fabsf(peek(sst).v.f);
end()	
/**(var) @
 * "foo @"
 * Load the contents of the parameter field of the word who's address is at
 * TOS. If threads are enabled and the variable has been marked by the SHARE
 * word, then access to it's value will be serialized in a 'one writer, many
 * reader' scheme with @ playing the reader.
 */
begin(at)
	struct voc_entry *entry;
	entry = ppop(sst);

#ifdef THREADS
	if ( entry->type & A_SHARE )
	{	
		printk("variable shared.. requesting write lock");

		pthread_rwlock_rdlock( &entry->lock );

		push(sst,entry->parm);

		pthread_rwlock_unlock( &entry->lock );
	}
	else
#endif
	push(sst,entry->parm);
end()
/**(var) !
 * "1 foo !"
 * Store TOS-1 in the parameter field of the variable pointed to by TOS.
 * If threads are enabled and the variable has been marked by the SHARE word,
 * then access to it's value will be serialized in a 'one writer, many reader'
 * scheme with ! playing the writer.
 */
begin(bang)
	cell *ptr;
	cell a;
	struct voc_entry *entry;
	
	entry	= ppop(sst);

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
	ptr	= &entry->parm;
	
	if ( ptr->type == T_STR )
		/* Address we're storing to is a string.
		 * Free it's memory. */
		free( ptr->v.s );
	
	a = pop(sst);

	if ( a.type == T_STR ) 
	{
		/* source is a string. store a copy its value in the
		 * word */
		ptr->type = T_STR;
		ptr->v.s = stringdup( a.v.s );
	}
	else
		*ptr = a;

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()
/**(array) ]@
 * "ary[ 1 ]@"
 * Push element number TOS from array TOS-1 onto the stack. If TOS is negative,
 * index from the end of the array.
 */
begin(bracket_at)
	struct voc_entry *entry;
	int i;
	cell *a;
	
	i	= fpop(sst);
	entry	= ppop(sst);

	if ( i < 0 )
		i += entry->parm.v.f;
#ifdef SAFE
	else if ( i >= entry->parm.v.f )
		error("array index out of bounds\n");
#endif
		
	a = (&entry->parm)[1].v.p;

	i++;
	
#ifdef THREADS
	if ( entry->type & A_SHARE )
	{	
		printk("variable shared.. requesting write lock");

		pthread_rwlock_rdlock( &entry->lock );

		push(sst,a[i]);

		pthread_rwlock_unlock( &entry->lock );
	}
	else
#endif
	push(sst,a[i]);
end()
/**(array) ]!
 * "0 ary[ 3 ]!"
 * Store TOS-2 in element TOS of array TOS-1. If TOS is negative, then it is
 * taken to be an index from the last element of the array.
 */
begin(bracket_bang)
	cell *ptr;
	cell a;
	int i;
	struct voc_entry *entry;

	i	= fpop(sst);
	entry	= ppop(sst);
	
#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
		
	if ( i < 0 )
		i += entry->parm.v.f;
#ifdef SAFE
	else if ( i >= entry->parm.v.f )
		error("array index out of bounds\n");
#endif
	
	i++;
	
	ptr	= &((cell*)(&entry->parm)[1].v.p)[i];
	
	if ( ptr->type == T_STR )
		/* Address we're storing to is a string.
		 * Free it's memory. */
		free( ptr->v.s );
	
	a = pop(sst);

	if ( a.type == T_STR ) 
	{
		/* source is a string. store a copy its value in the
		 * word */
		ptr->type = T_STR;
		ptr->v.s = malloc( sizeof(string) + a.v.s->l + 2 );
		memcpy( ptr->v.s, a.v.s, sizeof(string) + a.v.s->l + 2 );
	}
	else
		*ptr = a;

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()
/**(array) ]push
 * "'foo ary[ ]push"
 * Extend array TOS to include TOS-1.
 */
begin(bracket_push)
	struct voc_entry *entry;
	int i;
	cell a;
	cell *ptr;
	
	entry = ppop(sst);

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
	a = (&entry->parm)[1];
	
	if ( (long)(++entry->parm.v.f) % 128 == 0 )
		/* Time to allocate some more memory. */
		(&entry->parm)[1].v.p = realloc( (&entry->parm)[1].v.p,
						 (entry->parm.v.f + 128) *
						  sizeof( cell) );
		
	/* now to actually append */	
	i	= entry->parm.v.f;
	
	ptr	= &((cell*)(&entry->parm)[1].v.p)[i];

	a = pop(sst);

	if ( a.type == T_STR ) 
	{
		/* source is a string. store a copy its value in the
		 * word */
		ptr->type = T_STR;
		ptr->v.s = malloc( sizeof(string) + a.v.s->l + 2 );
		memcpy( ptr->v.s, a.v.s, sizeof(string) + a.v.s->l + 2 );
	}
	else
		*ptr = a;

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()
/**(array) pop
 * "ary[ ]pop ="
 * Shorten array at TOS by one, leaving the orphaned element on the stack.
 * If there are no elements left to pop, then return a stack marker.
 */
begin(bracket_pop)
	struct voc_entry *entry;
	int i;
	cell *a;
	
	entry	= ppop(sst);

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
	
	if ( ( i = --entry->parm.v.f ) < 0 )
	{
		entry->parm.v.f = 0;
		cmark(sst);
	}
	else
	{	
		i++;
		a = (&entry->parm)[1].v.p;

		if ( a[i].type == T_STR )
			/* we're popping a string. it must be exposed to
			 * the garbage collector. */
			gmark( a[i].v.s, gstack, &gst );

		
		if ( (long)(entry->parm.v.f) % 128 == 0 )
		/* Time to free some memory. */
		(&entry->parm)[1].v.p = realloc( (&entry->parm)[1].v.p,
						 (entry->parm.v.f) *
						  sizeof( cell) );
			
		push(sst,a[i]);
	}

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()
/**(array) ]delete
 * "1 ary[ 3 ]delete"
 * Remove TOS-2 elements from array TOS-1, starting at index TOS and moving
 * tword the end of the array. Array will be resized as necessary.
 */
begin(bracket_delete)
	struct voc_entry *entry;
	int i, n, j;
	cell *ptr;
	
	i	= fpop(sst);
	entry	= ppop(sst);
	n	= fpop(sst);
	
#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
		
	if ( i < 0 )
		i += entry->parm.v.f;
	
	i++;

	/* protect against boundary */
	n = max(n,((entry->parm.v.f - i) + 1));
	
	ptr	= &((cell*)(&entry->parm)[1].v.p)[i];

	/* free strings */
	for ( j = 0; j < n; j++ )
		if ( (ptr + j)->type == T_STR )
			free( (ptr + j)->v.s );

	memmove( ptr, ptr + n, ( entry->parm.v.f - i) * sizeof( cell ) );

	entry->parm.v.f -= n;
	
	if ( (long)(entry->parm.v.f) % 128 == 0 )
	/* Time to free some memory. */
	(&entry->parm)[1].v.p = realloc( (&entry->parm)[1].v.p,
					 (entry->parm.v.f) *
					  sizeof( cell) );
#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()	
/**(array) ]insert
 * "[ 1 2 3 ] ary[ 3 ]insert"
 * Insert TOS-2 many elements from the stack into the array TOS-1 starting at
 * offset TOS. Array will be resized as necessary.
 */
begin(bracket_insert)


end()	
/**(array) )@
 * "hash( 'foo )@"
 * Lookup the key named by TOS in the associative array at TOS-1. Return
 * value.
 */
begin(paren_at)
	struct voc_entry *entry;
	hash_tab *h;
	cell *a;
	char *key;
	
	key	= scpop(sst);
	entry	= ppop(sst);

	h = (&entry->parm)[1].v.p;

#ifdef THREADS
	if ( entry->type & A_SHARE )
	{	
		printk("variable shared.. requesting write lock");

		pthread_rwlock_rdlock( &entry->lock );

		a = hash_get( h, key );
		if ( a )
			push(sst,*a);
		else
			cmark(sst);

		pthread_rwlock_unlock( &entry->lock );
	}
	else
#endif
	{
		a = hash_get( h, key );
		if ( a )
			push(sst,*a);
		else
			cmark(sst);
	}
end()
/**(array) )!
 * "0 hash( 'foo )!"
 * Store TOS-2 as the value of key TOS in the hash TOS-1. If the key already
 * exists, it's value will be replaced.
 */
begin(paren_bang)
	cell a;
	char *key;
	string *s;
	struct voc_entry *entry;
	hash_tab *h;
	
	key	= scpop(sst);
	entry	= ppop(sst);
	
#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
	
	h = (&entry->parm)[1].v.p;

	a = pop(sst);
	
	if ( a.type == T_STR ) 
	{
		/* source is a string. copy its value to protect it from the
		 * garbage collector. */
		s = malloc( sizeof(string) + a.v.s->l + 1 );
		memcpy( s, a.v.s, sizeof(string) + a.v.s->l + 1 );
		a.v.s = s;
	}
		
	hash_put( h, key, &a );

	/* update the number of keys */
	entry->parm.v.f = h->used;

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()
/**(array) )delete
 * "hash( 'foo )delete"
 * Delete the key named at TOS from the associative array at TOS-1.
 */
begin(paren_delete)
	char *key;
	struct voc_entry *entry;
	hash_tab *h;
	
	key	= scpop(sst);
	entry	= ppop(sst);
	
#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_wrlock( &entry->lock );
#endif
	
	h = (&entry->parm)[1].v.p;

	hash_delete( h, key );

	/* update the number of keys */
	entry->parm.v.f = h->used;

#ifdef THREADS
	if ( entry->type & A_SHARE )
		pthread_rwlock_unlock( &entry->lock );
#endif
end()
/**(stack) swap
 * "( a b - b a )"
 * Swap TOS with TOS-1.
 */
begin(swap)
	swap(sst);
end()
/**(stack) drop
 * "( a - )"
 * Discard the value on top of the stack.
 */
begin(drop)
	drop(sst);
end()
/**(stack) dup
 * "( a - a a )"
 * Duplicate TOS.
 */
begin(dup)
	*(sst) = peek(sst);
	 sst++;
end()
/**(stack) over
 * "( a b - a b a )"
 * Duplicate TOS-1.
 */
begin(over)
	push(sst,idx(sst,1));	
end()
/**(stack) nip - (new)
 * "( a b - b )"
 * Remove TOS-1.
 * Note: STOIC calls this word UNDER (which is decieving). In my view UNDER
 * should work like TUCK.
 */
begin(nip)
	sst--;
	*(sst - 1) = *(sst);
end()
/**(stack) tuck - (new)
 * "( a b c - a c b c )"
 * Tuck inserts a copy of TOS before TOS-1.
 */
begin(tuck)
	cell a;

	a = peek(sst);

	swap(sst);

	push(sst,a);
end()
/**(stack) -rot
 * "( a b c - c a b )"
 * Rotate the top three stack cells backwards by one position.
 */
begin(minus_rot)
	cell a, b, c;

	c = pop(sst);
	b = pop(sst);
	a = pop(sst);

	push(sst,b);
	push(sst,c);
	push(sst,a);
end()
/**(stack) +rot
 * "( a b c - b c a )"
 * Rotate the top three stack cells forwards by one position.
 */
begin(plus_rot)
	cell a, b, c;

	c = pop(sst);
	a = pop(sst);
	b = pop(sst);

	push(sst,c);
	push(sst,b);
	push(sst,a);
end();
/**(stack) flip
 * "( a b c - c b a )"
 * Swap TOS and TOS-2.
 */
begin(flip)
	cell a, b, c;

	c = pop(sst);
	b = pop(sst);
	a = pop(sst);

	push(sst,c);
	push(sst,b);
	push(sst,a);
end()
/**(stack) idup - (new)
 * "( a b c 3 - a b c a )"
 * Duplicate the TOSth element of the stack.
 */
begin(idup)
	int n;
	
	n = fpop(sst);

	push(sst,*(sst - n));
end()
/**(stack) idrop - (new)
 * "( a b c 3 - b c )"
 * Discard the TOSth element of the stack.
 */
begin(idrop)
	int n;
	
	n = fpop(sst);

	/* shift the stack down a cell */
	memmove((sst - n) - 1, sst - n, (sst - sstmin) * sizeof( cell ) );
	sst--;
end()
/* Print a pointer */
begin(pdot)
	printf("(%p) ", ppop(sst));
end()
begin(fdot)
	printf("%g ", (double)fpop(sst)); 
end()

/**(variables) pad - (new)
 * Push the address of the scratch Pad area. The PAD is in the form of a
 * string.
 */
begin(pad)
	spush(sst,pad);
end()
begin(ttyout)
	putchar(fpop(sst));
end()
begin(ttyin)
	fpush(sst,getchar());
end()
/**(string) cat
 * "'foo 'bar cat"
 * Concatenate two strings.
 * Note: use + instead.
 */
begin(cat)
	int len;
	string *s;
	string *one;
	string *two;
	
	two = spop(sst);
	one = spop(sst);
	len = one->l + two->l;

	s = gmalloc( sizeof(string) + len + 1, gstack, &gst );

	memcpy( &s->s, &one->s, one->l );
	memcpy( &s->s + one->l, &two->s, two->l + 1);

	s->l = len;
	
//	strcpy( s + 1, one + 1 );
//	strcat( s + 1, two + 1 );

	spush(sst,s);
end()
/**(string) count
 * "'foo COUNT"
 * Leaves the address of the preceding string plus one at TOS-1 and it's count
 * byte at TOS.
 */
begin(count)
	string *s;

	s = spop(sst);
	ppush(sst,&s->s);
	fpush(sst,s->l);
end()
/**(string) type
 * "'foo count TYPE"
 * Output a string who's address is at TOS-1 and who's length is at TOS.
 */
begin(type)
	exec(*adr(putln));
end()
/**(string) stype
 * "'foo count STYPE"
 * Takes the same input as TYPE, but leaves its output on the stack instead
 * of printing it. That is to say, given a pointer and a count byte, it will
 * return a pointer to a new string beginning with that count byte, and with
 * the same contents as the source string. 
 */
begin(stype)
	int len;
	char *s;
	string *n;
	
	len = fpop(sst);
	s = (char*)spop(sst);

	n = gmalloc( sizeof(string) + len + 1, gstack, &gst );

	memcpy( &n->s, s, len + 1 );

	n->l = len;

	spush(sst,n);
end()
/**(string) msg
 * "'foo msg"
 * Print the string addressed by TOS.
 * Note: use = instead.
 */
begin(msg)
	string *s;
	
	printk("s_msg()");

	s = spop(sst);

	if ( s->l )
	{
		ppush(sst,&s->s);
		fpush(sst,s->l);

		exec(*adr(putln));
	}
end()
/**(source) load
 * "'filename load"
 * Transfer execution to the STOICAL source file named by TOS. Internally,
 * load will the current input file pointer, rdline Code Address, and prompt
 * CA on the loop stack, and then begin executing the file named by TOS.
 */
begin(load)
	struct voc_entry *rdline, *prompt;
	
	rdline = stt_lookup("rdline");
	prompt = stt_lookup("prompt");

	ppush(lst,finput);
	ppush(lst,rdline->code);
	ppush(lst,prompt->code);

	if ( ( finput = fopen( scpop(sst), "r" ) ) == NULL )
		error("file not found\n");
	
	rdline->code = adr(frdline);
	prompt->code = adr(next);
end()
/**(source) include
 * "'filename include"
 * This is the same as load, with the filename being relative to the stoical
 * library root. (/usr/local/lib/stoical/)
 */
begin(include)
	string *s;
	string *n;
	int l;
	s = spop(sst);

	l = strlen(libroot);
	
	/* create new string */
	n = gmalloc( sizeof(string) + s->l + l + 1, gstack, &gst );

	/* copy library root and filename into the new string */
	memcpy( &n->s, libroot, l );
	(&n->s)[l] = '/';
	memcpy( &n->s + l + 1, &s->s, s->l + 1 );

	spush(sst,n);

	/* let load figure out the rest */
	exec(*adr(load));
end()
/**(source) ;f
 * Close the current input file, and restore input, rdline and prompt
 * from the loop stack to their values at the last LOAD.
 */
begin(semicolon_f)
	struct voc_entry *rdline, *prompt;

	if ( lst - lstmin < 3 )
		/* LOAD either wasn't called or the loop stack
		 * has been corrupted. Maybe input is from the
		 * keyboard and the user means RTN. */
		exec(*adr(bye));
	
	rdline = stt_lookup("rdline");
	prompt = stt_lookup("prompt");
	
	fclose(finput);

	prompt->code	= ppop(lst);
	rdline->code	= ppop(lst);
	finput		= ppop(lst);
end()
/**(io) chdir
 * "'.. chdir"
 * Change to the directory named by TOS. Returns a boolean value indicating
 * success
 */
begin(chdir)
	int v;

	v = chdir(scpop(sst)) == 0 ? TRUE : FALSE;

	fpush(sst,v);
end()
/**(io) mkdir
 * "'foo 0 mkdir"
 * Create a new directory. With name at TOS-1, and mode at TOS. Returns a
 * boolean value indicating success.
 */
begin(mkdir)
	int v, m;

	m = fpop(sst);
	
	v = mkdir( scpop(sst), m) == 0 ? TRUE : FALSE;
	
	fpush(sst,v);
end()
/**(io) rmdir
 * "'foo rmdir"
 * Remove the directory named in the string at TOS. Will fail if the directory
 * is not empty. Returns a boolean success.
 */
begin(rmdir)
	int v;

	v = rmdir(scpop(sst)) == 0 ? TRUE : FALSE;

	fpush(sst,v);
end()
/**(io) unlink
 * "'foo unlink"
 * Remove file named by TOS from the file system. Returns a boolean success.
 */
begin(unlink)
	int v;

	v = unlink(scpop(sst)) == 0 ? TRUE : FALSE;

	fpush(sst,v);
end();
/**(io) mkfifo
 * "'foo mkfifo"
 * Create a special file of the FIFO variety with the name at TOS-1, and the
 * permissions at TOS. Returns boolean success.
 */
begin(mkfifo)
	int v, m;

	m = (ub4)fpop(sst);

	v = mknod( scpop(sst), S_IFIFO | m, 0 ) == 0 ? TRUE : FALSE;

	fpush(sst,v);
end()
/**(io) umask
 * "0 umask"
 * Set the process's new file permissions mask to the value at TOS.
 */
begin(umask)
	umask(fpop(sst));
end()
/**(io) open
 * "'foo 'r+ open"
 * Open the file named by TOS-1 with the mode spelled out in the string at
 * TOS. Modes are of the form "r+", etc. Returns a handle for the file, and
 * a boolean success.
 */
begin(open)
	char *m;
	FILE *fp;
	
	m = scpop(sst);

	fp = fopen( scpop(sst), m );

	if ( fp != NULL )
	{
		t_file *file;

		file = malloc( sizeof( t_file ) );
		
		file->fp = fp;
		file->fd = fileno(fp);
		
		filpush(sst,file);
		fpush(sst,TRUE);
	}
	else
		fpush(sst,FALSE);
end()
/**(io) close
 * "'foo 'r open drop CLOSE"
 * Close the file handle at TOS and free its resources
 */
begin(close)
	t_file *f;

	f = ppop(sst);
	
	fclose( f->fp );

	free( f );
end()
/**(io) write
 * "address count handle WRITE"
 * Copy 'count' bytes from 'address' to the file or socket described by 'handle'
 * Returns the number of bytes actually transfered.
 */
begin(write)
	t_file *f;
	int c;
	void *p;

	f = ppop(sst);
	c = fpop(sst);
	p = ppop(sst);
	
	fpush(sst,fwrite( p, 1, c, f->fp ));
end()
/**(io) read
 * "address count handle READ"
 * Copy 'count' bytes form the file or socket described by handle, into the
 * memory at 'address' Returns the number of bytes actually transfered.
 */
begin(read)
	t_file *f;
	int c;
	void* p;
	
	if ( peek(sst).type != T_FIL )
		exec(*adr(badtype));
	
	f = ppop(sst);
	c = fpop(sst);
	p = ppop(sst);
	
	fpush(sst,fread( p, 1, c, f->fp ));
end()
/**(io) writeln
 * "'foo handle WRITELN"
 * Write the string at TOS-1 to the file or socket described by the handle
 * at TOS.
 */
begin(writeln)
	t_file *f;
	string *s;

	f = ppop(sst);
	s = spop(sst);
	
	fpush(sst,fwrite( &s->s, 1, s->l, f->fp ));
end()
/**(io) readln
 * "handle READLN"
 * Read a line of text from a file/socket handle. Return the text (including
 * newline) and TRUE if a string could be read, or FALSE if end of file was
 * reached and no bytes were read.
 */
begin(readln)
	t_file *f;
	int l, eof;
	string *n;

	f = ppop(sst);
	
	eof = l = 0;
	n = NULL;

	/* take input until an newline is reached, extending the
	 * buffer to accommodate. */
	for (;;)
	{
		n = realloc( n, sizeof(string) + l + BUFSIZ );
		
		/* setup overflow detector */
		(&n->s)[ l + BUFSIZ ] = 'x';

		if ( fgets( &n->s + l, BUFSIZ, f->fp ) == NULL )
		{
			eof = 1;
			break;
		}

		if ( (&n->s)[ l + BUFSIZ ] == 'x' )
			/* didn't overflow. done. */
				break;

		l += BUFSIZ;
	}

	if ( ! eof ) {
		/* resize and mark the string for collection */
		n->l = strlen( &n->s );
		n = realloc( n, sizeof(string) + n->l + 1 );
		n = gmark( n, gstack, &gst );

		spush(sst,n);
		fpush(sst,TRUE);
	}
	else
	{	
		free(n);
		fpush(sst,FALSE);
	}
end()
/**(io) flush
 * "handle flush"
 * Flush the buffers for the file/socket represented by the handle at TOS.
 */
begin(flush)
	fflush( ((t_file *)ppop(sst))->fp );
end()
/**(io) seek
 * "offset whence file seek"
 * Change the possition indicator of the given stream.  TOS-3 is the number of
 * bytes to move. TOS-2 is one of 1, 0 or -1, for makeing the offset relative
 * to the start of the file, the current possition, or the end of the file,
 * respectively.
 */
begin(seek)
	t_file *f;
	int whence;

	f = ppop(sst);
	whence = fpop(sst);
	whence = whence == -1 ? SEEK_END :
		 whence ==  0 ? SEEK_CUR :
		 whence == 1 && SEEK_SET ;

	fseek( f->fp, fpop(sst), whence );
end()
/**(io) stat
 * "'filename [ 'mode 'atime ... ] stat"
 * Get file statistics. TOS (n) is number of items in attribute list.
 * TOS - n is the name of the file to stat. Leaves stat values in the order
 * specified on the stack and TRUE if all went well. Otherwise, just return
 * FALSE.
 *
 * The attribute list may contain any number of the following:
 *
 * 'dev		- device
 * 'ino		- inode
 * 'mode	- permissions
 * 'nlink	- number of hard links
 * 'uid		- user id of owner
 * 'gid		- group id of owner
 * 'rdev	- device type
 * 'blksize	- block size for file i/o
 * 'blocks	- number of blocks allocated to file
 * 'atime	- time of last access
 * 'mtime	- time of last modification
 * 'ctime	- time of last change
 */
begin(stat)
	struct stat buf;
	char *name;
	char *s;
	int n;
	int i;
	
	n = fpop(sst) + 1;
	name = (char*)&(idx(sst,(n - 1)).v.s->s);

	printk("stat'ing file %s", name );
	
	if ( stat( name, &buf ) == 0 )
	{
		for ( i = 0; i < n; i++ )
		{
			s = scpop(sst);
			
			if ( strcmp( s, "dev" ) == 0 )
				fpush(lst,buf.st_dev);
			else if ( strcmp( s, "ino" ) == 0 )
				fpush(lst,buf.st_ino);
			else if ( strcmp( s, "mode" ) == 0 )
				fpush(lst,buf.st_mode);
			else if ( strcmp( s, "nlink" ) == 0 )
				fpush(lst,buf.st_nlink);
			else if ( strcmp( s, "uid" ) == 0 )
				fpush(lst,buf.st_uid);
			else if ( strcmp( s, "gid" ) == 0 )
				fpush(lst,buf.st_gid);
			else if ( strcmp( s, "rdev" ) == 0 )
				fpush(lst,buf.st_rdev);
			else if ( strcmp( s, "size" ) == 0 )
				fpush(lst,buf.st_size);
			else if ( strcmp( s, "blksize" ) == 0 )
				fpush(lst,buf.st_blksize);
			else if ( strcmp( s, "blocks" ) == 0 )
				fpush(lst,buf.st_blocks);
			else if ( strcmp( s, "atime" ) == 0 )
				fpush(lst,buf.st_atime);
			else if ( strcmp( s, "mtime" ) == 0 )
				fpush(lst,buf.st_mtime);
			else if ( strcmp( s, "ctime" ) == 0 )
				fpush(lst,buf.st_ctime);
			else
				fpush(lst,0);
		}
		/* discard filename */
		drop(sst);
		
		for ( i = 0; i < n; i++ )
			push(sst,pop(lst));
		
		fpush(sst,TRUE);
	}
 	else 
	{
		/* discard filename */
		drop(sst);
		fpush(sst,FALSE);
	}

end()
/**(io) (get)
 * Variable containing address of current character input word.
 */
begin(_get)
	rpush(sst,&io->in);
end()
/**(io) (put)
 * Variable containing address of current character output word.
 */
begin(_put)
	rpush(sst,&io->out);
end()
/**(io) (getln)
 * Variable containing address of current line input word.
 */
begin(_getln)
	rpush(sst,&io->inln);
end()
/**(io) (putln)
 * Variable containing address of current line output word.
 */
begin(_putln)
	rpush(sst,&io->outln);
end()
/**(io) get
 * Execute word who's address is stored in the variable IN. The word to be
 * executed should read a single character from the input device, and return
 * it an integer representing the ascii value entered.
 */
begin(get)
	execute(io->in.parm.v.p);
end()
/**(io) put
 * "48 put"
 * Execute word who's address is stored in the variable OUT. The word to be
 * executed should print the single character represented by the ascii
 * code at TOS, to the output device.
 */
begin(put)
	execute(io->out.parm.v.p);
end()
/**(io) ttyput
 * "48 ttyput"
 * Output character to terminal
 */
begin(ttyput)
	fputc( fpop(sst), toutput );
end()
/**(io) ttyget
 * Input a character from terminal.
 */
begin(ttyget)
	fpush(sst,fgetc( tinput ));
end()
	
/**(io) getln
 * Execute word who's address is stored in the variable INLN. The word to be
 * executed should read a line from the input device, and return it as a
 * string striped of its newline.
 */
begin(getln)
	execute(io->inln.parm.v.p);
end()
/**(io) putln
 * "'foo count putln"
 * Execute word who's address is stored in the variable OUTLN. The word to be
 * executed should output TOS many bytes of the memory at TOS-1 to the output
 * device.
 */
begin(putln)
	execute(io->outln.parm.v.p);
end()
/**(io) ttyputln
 * "'foo count ttyputln"
 * Output string to terminal
 */
begin(ttyputln)
	int c;
	void *p;

	c = fpop(sst);
	p = ppop(sst);
	
	fwrite( p, c, 1, toutput );
end()
/**(io) ttygetln
 * Input line from the terminal (strip newline)
 */
begin(ttygetln)
	filpush(sst,input.parm.v.p);

	exec(*adr(readln));
end()
/**(io) opendir
 * "'. opendir"
 * Open directory named by TOS. Leaves on the stack a directory pointer and
 * TRUE if call succeeds, otherwise it just leaves FALSE.
 */ 
begin(opendir)
	DIR *d;

	if ( ( d = opendir(scpop(sst))) != NULL )
	{
		t_file *f;
		
		f = malloc( sizeof( t_file ));

		f->fp = (FILE *)d;
		f->fd = fileno((FILE *)d);

		filpush(sst,f);
		fpush(sst,TRUE);
	}
	else
		fpush(sst,FALSE);
end()
/**(io) closedir
 * "dir close"
 * Close the directory referenced by the handle at TOS.
 */
begin(closedir)
	t_file *f;

	f = ppop(sst);
	
	closedir( (DIR *)f->fp );

	free( f );
end()
/**(io) readdir
 * "dir readdir"
 * Push the name of the next entry in directory pointed to by TOS onto
 * the stack and TRUE. If end of directoy is hit, just push FALSE.
 */
begin(readdir)
	t_file *f;
	string *n;
	int len;
	struct dirent *ent;
	
	f = ppop(sst);

	if ( ( ent = readdir( (DIR *)f->fp ) ) != NULL )
	{
		len = strlen( ent->d_name );

		n = gmalloc( sizeof(string) + len + 1, gstack, &gst );
		n->l = len;
		memcpy( &n->s, ent->d_name, len );
		(&n->s)[len] = '\0';
		
		spush(sst,n);
		fpush(sst,TRUE);
	}
	else
		fpush(sst,FALSE);
end()
/**(io) socket - (new)
 * "'inet 'stream 'tcp socket"
 * Create an endpoint for communication. This can later be used by connect
 * and listen to create a connection. 
 * 
 * 'domain 'type 'protocol socket
 * 
 * Domain should be a string matching one of the following:
 * "unix"	local (unix) communication
 * "inet"	ipv4 protocols
 * "inet6"	ipv6 protocols
 * "packet"	low level packet interface.
 *
 * Type should match one of:
 * "stream"	reliable, two-way, and connection based.
 * "dgram"	connectionless, unreliable and fixed length.
 * "seq"	a hybrid of stream and dgram.
 * "raw"	raw protocol access.
 *
 * Protocol can be any protocol that the system supports.
 * These should be listed in the /etc/protocols file on most systems.
 * Those most likely to be used include "tcp", "udp", and "icmp".
 *
 * FIXME: Some more error checking wouldn't hurt.
 */
begin(socket)
	int proto, type, domain, af, v;
	struct protoent *p;
	char *s = NULL;

	/* Deduce protocol */
	p = getprotobyname( scpop(sst) );
	if ( p == NULL )
		error("invalid socket protocol (%s)\n", s);

	proto = p->p_proto;
	

	/* Deduce  Type */
	s = scpop(sst);
	if ( strcmp(s,"stream") == 0 )
		type = SOCK_STREAM;
	else if ( strcmp(s,"dgram") == 0 )
		type = SOCK_DGRAM;
	else if ( strcmp(s,"seq") == 0 )
		type = SOCK_SEQPACKET;
	else if ( strcmp(s,"raw") == 0 )
		type = SOCK_RAW;
	else
		error("invalid socket type (%s)\n", s);
		
	/* Deduce domain */
	s = scpop(sst);
	if ( strcmp(s,"unix") == 0 )
	{
		domain = PF_UNIX;
		af = AF_UNIX;
	}
	else if ( strcmp(s,"inet") == 0 )
	{
		domain = PF_INET;
		af = AF_INET;
	}
	else if ( strcmp(s,"inet6") == 0 )
	{
		domain = PF_INET6;
		af = AF_INET6;
	}
#if 0
	else if ( strcmp(s,"packet") == 0 )
	{
		domain = PF_PACKET;
		af = AF_PACKET;
	}
#endif
	else
		error("invalid socket domain (%s)\n", s);
	
	proto = type == SOCK_STREAM ? 0 : proto;
	
	v = socket( domain, type, proto );

	if ( v == -1 )
		fpush(sst,FALSE);
	else
	{
		t_sock *sock;
		
		sock = malloc( sizeof( t_sock ) );
		
		sock->fp = fdopen(v, "w+");
		sock->fd = v;
		sock->pf = domain;
		sock->af = af; 
		sock->st = type;
		sock->p	 = proto;
		
		filpush(sst,sock);
		
		fpush(sst,TRUE);
	}
	
end()
/**(io) bind - (new)
 * "'port sock bind"
 * Bind socket at TOS to port at TOS-1 ( or unix name as the case may be ) in 
 * the domain at TOS-2.
 */
begin(bind)
	int s;
	char *port;
	t_sock *sock;

	sock	= ppop(sst);
	s	= sock->fd;
	port	= scpop(sst);

	if ( sock->pf == PF_UNIX )
	{
		struct sockaddr_un *sun;
		int size = sizeof( sa_family_t ) + *(port - 1);
		
		sun = calloc( 1, size );

		sun->sun_family	= sock->af;

		memcpy(&sun->sun_path, port, *(port - 1) );

		if ( bind( s, (struct sockaddr *)sun, size ) < 0 )
		{
			free(sun);
			error("bind failed\n");
		}
		free(sun);
	}
	else
	{
		struct sockaddr_in sin;
		struct servent *ent;
		
		if ( isdigit( port[ 0 ] ) )
			sin.sin_port = htons(atoi(port));
		else
		{
			if ( ( ent = getservbyname(port,
				     getprotobynumber(sock->p)->p_name
			) ) == NULL )
				error("invalid port name\n");
			
			sin.sin_port = ent->s_port;
		}
		
		sin.sin_family		= sock->af;
		sin.sin_addr.s_addr	= htonl(INADDR_ANY);

		if ( bind( s, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
			error("bind failed\n");
	}
end()
/**(io) listen - (new)
 * "4 sock listen"
 * Initialize socket at TOS for listening. Queue TOS-1 many requests.
 */
begin(listen)
	t_sock *s;

	s = ppop(sst);
	
	listen(s->fd, fpop(sst));
end()
/**(io) accept
 * "sock accept"
 * Accept a connection on the socket at TOS. Returns a new socket
 * representing the connection accepted.
 */
begin(accept)
	struct sockaddr_in sin;
	int s, len;
	t_sock *nsock;
	t_sock *sock;

	sock = ppop(sst);
	nsock = malloc( sizeof( t_sock ) );

	memcpy(nsock, sock, sizeof( t_sock ) );

	s = accept( sock->fd, (struct sockaddr *)&sin, &len );

	nsock->fd = s;
	nsock->fp = fdopen(s, "w+" );
	
	filpush(sst,nsock);
end()
/**(io) connect
 * "'name 'port sock connect"
 * Initiate a connection to 'name at port 'port on the socket sock.
 */
begin(connect)
	char *addr, *port;
	int s;
	t_sock *sock;
	
	sock	= ppop(sst);
	s	= sock->fd;
	port	= scpop(sst);
	addr	= scpop(sst);

	if ( sock->pf == PF_UNIX )
	{
		struct sockaddr_un *sun;
		int size = sizeof( sa_family_t ) + *(addr - 1);
		
		sun = calloc( 1, size );

		sun->sun_family	= sock->af;

		memcpy(&sun->sun_path, addr, *(addr - 1) );

		if ( connect( s, (struct sockaddr *)sun, size ) < 0 )
		{
			free(sun);
			error("uconnect failed\n");
		}
		free(sun);
	}
	else
	{
		struct sockaddr_in sin;
		struct hostent *ent;
		struct servent *sent;

		if ( ( ent = gethostbyname(addr) ) == NULL )
			error("hostname lookup failed.\n");
	
		sin.sin_addr	= *((struct in_addr*)ent->h_addr);

		if ( isdigit(port[0]) )
			sin.sin_port = htons(atoi(port));
		else
		{
			if ( ( sent = getservbyname(port,
				     getprotobynumber(sock->p)->p_name
			) ) == NULL )
				
				error("invalid port name\n");
			
			sin.sin_port = sent->s_port;
		}
		
		sin.sin_family		= sock->af;

		if ( connect( s, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
			error("connect failed\n");
	}
end()
/**(io) shutdown - (new)
 * "sock 1 shutdown"
 * Shut down one direction of a full-duplex connection.
 * Socket is at TOS. TOS-1 specifies which direction:
 * 0 - incoming.
 * 1 - outgoing.
 * 2 - both.
 */
begin(shutdown)
	/* FIXME: When should I close this stream? */
	shutdown(((t_file *)ppop(sst))->fd, fpop(sst));
end()
#ifdef HAVE_SYS_SENDFILE_H
/**(io) sendfile - (new)
 * "start count dest src SENDFILE"
 * Copy data between file descriptors. DST should point to a file or socket
 * opened for writing. SRC should point to a file opened for reading.
 * Copying will begin at offset START in SRC, COUNT many bytes will be
 * transfered. Returns the number of bytes actually copied. 
 */
begin(sendfile)
	int src, dst;
	long count;
	off_t start;

	src	= ((t_file *)ppop(sst))->fd;
	dst	= ((t_file *)ppop(sst))->fd;
	count	= fpop(sst);
	start	= fpop(sst);

	fpush(sst,sendfile( dst, src, &start, count ));
end()
#endif
/**(conditional) check
 * Push current nesting depth.
 */
begin(check)	
	fpush(sst,depth);
end()
/**(conditional) +check
 * Increment nesting depth.
 */
begin(plus_check)
	depth++;
end()
/**(conditional) -check
 * Decrement nesting depth, generating a syntax error if the result is less
 * then zero.
 */
begin(minus_check)
	st_minus_check();
end()
/**(variables) warnings
 * Change compiler warning verbosity.
 * Currently the only meaningful value is FALSE, to disable warnings on word
 * redefinitions and the like.
 */ 
begin(warnings)
	rpush(sst,&warnings);
end()
/**(variables) current
 * Return a pointer to the current vocabulary for new definitions.
 */
begin(current)
	ppush(sst,current);
end()
/* Number conversion */
begin(hash_ptr)
	rpush(sst,&hash_ptr);
end()
begin(hash_cnt)
	rpush(sst,&hash_cnt);
end()
begin(left_angle_hash)
	/* Initialize conversion count and pointer */
	hash_cnt.parm.type = T_FLT;
	hash_cnt.parm.v.f = 0;
	hash_ptr.parm.type = T_PTR;
	hash_ptr.parm.v.p = &pad->s + 32;
end()
begin(hash_right_angle)
	/* Discard TOS. Push the contents of hash_ptr. Push the
	 * contents of hash_cnt. */
	drop(sst);

	/* Set the length byte in the string */
	*((char*)hash_ptr.parm.v.p - 1) = hash_cnt.parm.v.f;
	
	push(sst,hash_ptr.parm);
	push(sst,hash_cnt.parm);
end()
begin(hash_put)
	/* Place character represented by the value at TOS into
	 * the buffer. Decrement the pointer, and increment the count. */
	*(--(char*)hash_ptr.parm.v.p) = fpop(sst);
	
	hash_cnt.parm.v.f++;
end()
begin(hash_a)
	/* Convert the number at TOS to an ascii digit. */
	int i;

	i = fpop(sst);

	fpush(sst,( i > 10 ? i + 48 + 7 : i + 48));
end()
begin(hash_match)
	rpush(sst,&hash_match);
end()
/**(system) args[
 * 
 * Array containing the command line arguments passed by the shell. 
 * Element zero is the name of the program.
 */
begin(args_bracket)
	rpush(sst,args);
end()
/**(math) radix
 * Current number conversion radix.
 */
begin(radix)
	rpush(sst,&radix);
end()
/**(compiler) eoc
 * End of command flag.
 */
begin(eoc)
	fpush(sst,eoc);
end()
/**(compiler) eol
 * End of line flag.
 */
begin(eol)
	fpush(sst,eol);
end()
/**(unary) bool
 * "3 bool not if"
 * Convert TOS to TRUE either FALSE. This word is used in situations where
 * you would like to use AND/OR/NOT in their logical capacity, but the value
 * you are working with may never be TRUE in the stoical sense.
 */
begin(bool)
	if ( peek(sst).v.f != 0 )
		peek(sst).v.f = TRUE;
end()
/**(unary) not
 * "false NOT"
 * Negate each bit of TOS.
 */
begin(not)
	peek(sst).v.f = ! peek(sst).v.f;
end()
/**(binary) and
 * Perform bitwise AND on TOS and TOS-1.
 */
begin(and)
	long v;

	v = (long)fpop(sst);

	peek(sst).v.f = (long)peek(sst).v.f & v;
end()
/**(binary) or
 * Perform bitwise OR on TOS and TOS-1
 */
begin(or)
	long v;

	v = (long)fpop(sst);

	peek(sst).v.f = (long)peek(sst).v.f | v;
end()
/**(binary) xor
 * Perform bitwise eXclusive OR on TOS and TOS-1.
 */
begin(xor)
	long v;

	v = (long)fpop(sst);

	peek(sst).v.f = (long)peek(sst).v.f ^ v;
end()
/**(unary) eqz
 * FALSE if TOS does not equal zero.
 */
begin(eqz)
	peek(sst).v.f = peek(sst).v.f == 0 ? TRUE : FALSE;
end()
/**(unary) ltz
 * FALSE if TOS does is not less than zero.
 */
begin(ltz)
	peek(sst).v.f = peek(sst).v.f < 0 ? TRUE : FALSE;
end()
/**(binary) feq
 * "1 1 feq"
 * Test TOS and TOS-1 for equality.
 */
begin(feq)
	if ( fpop(sst) == fpop(sst) )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(string) $eq - (new)
 * Compare two strings.
 */
begin(dolar_eq)
	if ( strcmp(&spop(sst)->s,&spop(sst)->s) == 0 )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(binary) fne
 * "1 2 fne"
 * Test TOS and TOS-1 for inequality.
 */
begin(fne)
	if ( fpop(sst) != fpop(sst) )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(string) $ne - (new)
 * Compare two strings.
 */
begin(dolar_ne)
	if ( strcmp(&spop(sst)->s,&spop(sst)->s) != 0 )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(binary) lt
 * "1 2 lt"
 * TRUE if TOS-1 is less than TOS.
 */
begin(lt)
	if ( fpop(sst) > fpop(sst) )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(binary) gt
 * "2 1 gt"
 * TRUE if TOS-1 is greater than TOS.
 */
begin(gt)
	if ( fpop(sst) < fpop(sst) )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(binary) le
 * "1 1 le"
 * TRUE if TOS-1 is less than or equal to TOS.
 */
begin(le)
	if ( fpop(sst) >= fpop(sst) )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(binary) ge
 * "1 1 ge"
 * TRUE if TOS-1 is greater than or equal to TOS.
 */
begin(ge)
	if ( fpop(sst) <= fpop(sst) )
		fpush(sst,TRUE);
	else
		fpush(sst,FALSE);
end()
/**(iterative) i
 * "4 ( i = )"
 * Push innermost loop index onto the parameter stack.
 */
begin(i)
	push(sst,*(lst - 1));
end()
/**(iterative) j
 * "4 ( 4 ( j = ) )"
 * Push next innermost loop index onto the parameter stack.
 */
begin(j)
	push(sst,*(lst - 4));
end()
/**(iterative) k
 * "4 ( 4 ( 4 ( k = ) ) )"
 * Push next, next innermost loop index onto the parameter stack.
 */
begin(k)
	push(sst,*(lst - 7));
end()
/**(unary) -i 
 * "4 ( -i = )"
 * Push TOS-2 + TOS-1 - TOS - 1 of the loopstack. The effect of this is an
 * index like i, but moving in the other direction. In STOICAL, -i can be used
 * from with in parenthesis loops, where in STOIC this was erroneous.
 * Note: STOIC calls this i' (i prime). I think -i makes the word's function
 * more obvious. 
 */	
begin(minus_i)
	fpush(sst, idx(lst,2).v.f + idx(lst,1).v.f - peek(lst).v.f - 1);
end()
/**(unary) -j 
 * Similar to -i, but for the next innermost index.
 */
begin(minus_j)
	fpush(sst, idx(lst,5).v.f + idx(lst,4).v.f - idx(lst,3).v.f - 1);
end()
/**(unary) -k 
 * Similar to -i, but for the next, next innermost index.
 */
begin(minus_k)
	fpush(sst, idx(lst,8).v.f + idx(lst,7).v.f - idx(lst,6).v.f - 1);
end()
/**(stack) <l
 * "1 <l"
 * Transfer TOS to the loop stack.
 */
begin(left_angle_l)
	push(lst,*(--sst));
end()
/**(stack) l>
 * "1 <l L>"
 * Push the top of the loop stack. 
 */
begin(l_right_angle)
	push(sst,*(--lst));
end()
/**(stack) <c
 * "() foo <C"
 * Transfer pointer value at TOS to the compile stack.
 * Only pointers may exist on the compile stack, anything else is in error. 
 * Note: STOIC calls this C,. 
 */
begin(left_angle_c)
	push(cst,ppop(sst));
end()
/**(stack) c>
 * Push the top of the compile stack.
 */
begin(c_right_angle)
	rpush(sst,pop(cst));
end()
/**(variables) .c
 * Push the number of cells used by the compile stack.
 * Note: STOIC would have pushed the compile stack pointer.
 */
begin(dot_c)
	fpush(sst,(cst - cstmin));
end()
/**(variables) .l
 * Push the number of cells used by the loop stack.
 * Note: STOIC would have pushed the loop stack pointer.
 */
begin(dot_l) 
	fpush(sst,(lst - lstmin));
end()
/**(variables) .r
 * Push the number of cells used by the return stack.
 * Note: STOIC would have pushed the return stack pointer.
 */
begin(dot_r) /* .r - return stack pointer */
	fpush(sst,(rst - rstmin));
end()
/**(variables) .v
 * Push the number of cells used by the vocabulary stack.
 * Note: STOIC would have pushed the vocabulary stack pointer.
 */
begin(dot_v) 
	fpush(sst,(vst - vstmin));
end()
/**(variables) .d 
 * Return a pointer to the bare memory portion of the scratch PAD area.
 * 
 * The original STOIC would return a pointer to the end of the dictionary;
 * Usually using it as a temporary space. However, in STOICAL there is no such
 * space.
 */
begin(dot_d)
	ppush(sst,&pad->s);
end()
/**(compiler) stack - (new)
 * "1 2 3 stack"
 * Non-destructively print the parameter stacks contents.
 */
begin(stack)
	st_cstack( sstmin, sst );
end()
/**(compiler) stack - (new)
 * "1 2 3 stack"
 * Non-destructively print the loop stacks contents.
 */
begin(lstack)
	st_cstack( lstmin, lst );
end()
/**(compiler) vocab - (new)
 * Print contents of the vocabulary on top of the vocabulary stack.
 */
begin(vocab)
	int i;
	struct vocabulary *voc;
	struct voc_entry *entry;

	voc = peek(vst);
	entry = voc->last;
	
	printf("Contents of vocabulary %s (%i words):\n",
		voc->name, voc->words);
	
	for ( i = voc->words; i > 0; i-- )
	{
	
#ifdef PROFILE
		printf("\'%-20s : size=%i, called=%i\n",  ornull(entry->name),
							 entry->size *
							 sizeof( ip ),
							 entry->called );
#else
		printf("\'%-20s :\n",  ornull(entry->name) );
#endif
		entry = entry->last;
	}
	
	putchar('\n');
end()
/**(compiler) decompile
 * "() foo DECOMPILE"
 * Print a source version of the word addressed by TOS.
 */
begin(decompile)
	st_decompile( ppop(sst), NULL );
end()
/**(compiler) vstack (new)
 * List the vocabularies on the vocabulary stack.
 */
begin(vstack)
	int i;

	i = vst - vstmin - 1;

	for (; i >= 0; i--)
	{
		printf("%s\n", idx(vst,i)->name );
	}
end()
/**(compiler) rtn
 * Exite the process with the return value of TOS.
 */
begin(rtn)
	printk("Goodbye, Dave.");
	return;
end()
/**(compiler) bye
 * Exit the entire process. (with return value of zero);
 */
begin(bye)
	exec(*adr(rtn));
end()
/**(error) abort
 * Reset compiler to a sane state, and return control to the keyboard.
 */
begin(abort)
	printk("abort: reseting compiler");
	
	sst = sstmin;
	rst = rstmin;
	lst = lstmin;
	cst = cstmin;
	bst = bstmin;
	depth = 0;
	state = 0;
	eol   = FALSE;
	eoc   = TRUE;

	strcpy( &tib->s, "(:) nop" );
	tib->l = 7;
	tibp = 0;

	peek(sst).type = 0;

	ip = go;
end()
/**(dictionary) (:)
 * Push the Instruction Pointer onto the return stack. Set IP to
 * point just before the first location in the current word's parameter field.
 */ 
begin(_colon) 
	printk("(:)");

	/* save IP on the return stack */
	push(rst,((rcell){ self, ip }));
	
	/* Set the IP to point right before the parm field */
	ip = ((struct voc_entry **)&self->parm) - 1;
end()
/**(dictionary) ({:)
 * Execute a clause as if it were a word.
 */
begin(_brace_colon)
	printk("({:)");

	/* save IP on the return stack */
	push(rst,((rcell){ self, ip }));

	exec(*adr(__brace_colon));
end()
begin(__brace_colon)
	struct code *c;
	
	/* Set the IP to point right before the parm field */
	c = (struct code*)self;
	
	ip = &c->parm - 1;
end()
#ifdef PROFILE
#	define st_enter_end \
	current->last->called = 0; \
	current->last->size = 0;
#else
#	define st_enter_end
#endif
#define st_enter(extra) { \
	char *name; \
	struct voc_item item; \
	printk("enter()"); \
	name = scpop(sst); \
	if ( warnings.parm.v.f != FALSE && stt_lookup(name) != NULL ) \
		printf("redefining %s\n", name); \
	item.name = name; \
	item.code = NULL; \
	voc_append( current, &item, extra ); \
	st_enter_end \
} 
/**(dictionary) :{ (*)
 * Begin a colon definition.  Push the address of (:{) onto the compile stack,
 * and open a new clause.
 */
begin(colon_brace)
	push(cst,stt_lookup("(:{)"));
	exec(*adr(left_brace));
end();
/**(dictionary) (:{)
 * "'foo :{ bar }"
 * Create a new executable word in the dictionary consising of the following
 * clause, and named by the string at TOS.
 */
begin(_colon_brace)
	int size, cols;
	struct code *c;
	ccell col;

	printk("(:{)");
	c = (struct code*)(*(++ip));
	
	size = c->size;
	cols = c->cols;
	
	st_enter((c->size * sizeof ( void * )));

	current->last->code = adr(_colon);
	current->last->type = A_WRD;

	/* Copy code */
	memcpy( &current->last->parm, &c->parm, size * sizeof( void * ) ); 

	/* change the (}) at the end of the instruction stream to (;) */
((struct voc_entry **)&current->last->parm)[size - 1] = stt_lookup("(;)");
	
	current->last->size = size;
		
	/* move the compile stack collectables over into the new definition */
	for (; cols; cols-- ) 
	{
		col = pop(ccst);
		entry_collectable( current->last, col.handler, col.ptr );
	}
end()
/**(dictionary) ):{
 * "'foo :( s f ):{ bar }"
 * Begin a type checked definition.  Pop the top entry off of the vocabulary
 * stack. Push the address of ():{) onto the compile stack. Open a new clause.
 */
begin(paren_colon_brace)
	drop(vst);
	push(cst,stt_lookup("():{)"));
	exec(*adr(left_brace));
end()
/**(dictionary) ():{)
 * Begin typed colon definition.  There should be a type-number list
 * representing the named word's type mask on the stack, terminated by a
 * marker. Name follows this. The new clause will be appended to the word's
 * clause table, or the word will be created if it doesn't exist. A new word
 * will also be created if the previously defined version wasn't defined with
 * type-checking enabled.
 */
begin(_paren_colon_brace)
	int i, size, cols;
	ub4 type, mask;
	struct code *c;
	string *name;
	struct voc_entry *entry;
	type_head *head = NULL;
	type_vec *tvec;
	ccell col;

	type = 0;
	mask = 0;

	/* generate vector address and mask from stack types */
	for ( i = 0; ; i += 8 )
	{
		if ( peek(sst).type == 0 )
		{
			/* end of list */
			drop(sst);
			break;
		}
		
		/* this is a "don't care" value.. update the mask */
		if ( peek(sst).v.f == 0 )
			mask |= (ub4)0xFF << i;

		type |= (ub4)fpop(sst) << i;
	}

	if ( i == 0 || i > 4 * 8 )
		error("wrong number of types in definition\n");

	/* negate the mask */
	mask = ~mask;
	
	c = (struct code*)(*(++ip));
	
	size = c->size;
	cols = c->cols;

	name = spop(sst);

	entry = stt_lookup(&name->s);
	
	if ( entry == NULL || ! (entry->type & A_CHECK) )
	{
		spush(sst,name);
		st_enter( sizeof( type_head ) );
		entry = current->last;
	
		entry->type = A_WRD | A_CHECK;
		entry->code = adr(dispatch);

		head = (void*)&entry->parm;
		head->num = 0;
		head->max = i / 8;
		head->vec = NULL;
	}
	else
	{
		head = (void*)&entry->parm;

		if ( i / 8 != head->max )
			error("wrong number of types in definition\n");
		
		/* pop the old collectable pointer, because realloc will
		 * be changing it */
		entry_pop_collectable( entry );			
	}

	head->vec = realloc( head->vec, (1 + head->num) * sizeof( type_vec ) );

	tvec = &head->vec[ head->num ];
	
	tvec->type	= type;
	tvec->mask	= mask;
	tvec->clause	= c;
	
	/* Copy code */

	c->code = adr(__brace_colon);

	/* change the (}) at the end of the instruction stream to (;) */
	(&c->parm)[size - 1] = stt_lookup("(;)");
	entry->size = size;

	head->num += 1;
	
	/* move the compile stack collectables over into the new definition */
	for (; cols; cols-- ) 
	{
		col = pop(ccst);
		entry_collectable( entry, col.handler, col.ptr );
	}
	
	/* note this clause as being collecablte */
	entry_collectable( entry, free, c );
	entry_collectable( entry, free, head->vec );

end()
/**(dictionary) enter
 * "'name enter"
 * Create an entry 'name in the vocabulary pointed to by CURRENT.
 */
begin(enter)
	st_enter(0);
end()
/**(conditional) { (*)
 * Begin clause
 */
begin(left_brace)
	
	depth++;

	/* leave the code field pointer for the clause to be created on the
	 * block stack. } will used this when allocating the block. Note
	 * that this is a compile-time behavior. */
	push(bst,(void*)clause);
	clause = adr(_left_brace);
	
	ppush(sst,cst);
end()
/**(conditional) } (*)
 * End clause. 
 */
begin(right_brace)
	int size;
	struct code *c;
	struct voc_entry **oldcst;

	printk("(})");

	st_minus_check();

	oldcst  = ppop(sst);

	size = cst - oldcst;
	
	/* allocate space */
	c = malloc( sizeof( struct code ) + size * sizeof( void * ) );

	/* Copy code */
	memcpy( &c->parm, oldcst, size * sizeof(void *) );
	
	/* pop the code we copied off of the compile stack */
	cst -= size;
	
	(&c->parm)[size++] = stt_lookup("(})");
	
	c->size = size;
	c->code = (void*)pop(bst);
	c->type	= A_CLS;

	/* note where this block begins */
	// block = cst;
	
	/* push the code block invoker, and the code block pointer itself */
	push(cst,(void*)c);

	/* mark the code block as collectable */
	push(ccst,((ccell){ free, c}));

	/* note the depth of the collectables stack. */
	c->cols = ccst - ccstmin;
end()
/**(dictionary) ({)
 * Push IP onto the block stack. Set ip to point just before the first
 * instruction of this clause.
 */
begin(_left_brace)
	struct code *c;

	printk("({)");

	/* save IP on the block stack */
	push(bst,ip);
	
	c = (struct code*)self;
	
	ip = &c->parm - 1;
end()
/**(dictionary) (b{)
 * Similar to ({), except that IP - 3 is stored on the block stack instead.
 * This results in the clause returning to a point two instructions head of
 * where it was invoked. WHILE and friends exploit this behavior.
 */
begin(_b_left_brace)
	struct code *c;

	printk("(b{)");

	/* save IP on the block stack */
	push(bst,ip - 3);
	
	c = (struct code*)self;
	
	ip = &c->parm - 1;
end()
/**(dictionary) (})
 * Return from clause. Pop IP from the block stack.
 */
begin(_right_brace)
	printk("(})");

	/* set IP to tos of the block stack */
	ip = pop(bst);
end()
/**(dictionary) {}> (*)
 * Move the clause at the top of the compile stack onto the parameter stack,
 * marking it for GC.
 */
begin(bb_right_angle)
	struct code *c;

	c = (void*)pop(cst);
	
	/* take it off the collectables stack */
	drop(ccst);

	/* mark it for collection by the garbage collector */
	c = gmark( c, gstack, &gst );
	
	push(sst,((cell){ T_CLS, { p: c } }));
	st_lit();
end()
/**(dictionary) (;)
 * Pops an instruction pointer off the return stack. Used to terminate a colon
 * definition's instruction portion.
 */
begin(_semicolon) 
	printk("(;)");

	ip = pop(rst).i;
end()
#define word() (fpush(sst,(eol = st_word( tib, &tibp, pad ))))
/**(string) word
 * Scan keyboard buffer for the next word. Skip leading white space.
 * Treat Everything between quotes as a single word. Set eol to true if the
 * end of line was reached while scanning.
 */
begin(word)
	word();
end()
/**(dictionary) immediate
 * "'foo : 1 = ; immediate"
 * Set the immediacy flag of the most recently defined word.
 */
begin(immediate)
	current->last->type |= A_IMM;	
end()
/**(compiler) inspect - (new)
 * Display some memory usage information.
 */
begin(inspect)
#if 0
	char **s;
	int i = 0;

	puts("The following strings remain uncollected.");
	
	for ( s = &gstack[0]; s < &gstack[255]; s++, i++ )
	{
		if ( *s != NULL )
			printf( "%i: \"%s\"\n", i, (*s) + 1 ); 
	}
#endif
end()	
/**(literal) l()
 * Push the literal following us in the intruction stream onto the stack,
 * and skip to the intruction following it. */
begin(l_)
	cell *a;

	ip++;
	a = (cell*)ip;
	
	((cell*)ip)++;
	ip--;

	push(sst,*a);
end()
#ifdef REGEX
/**(regex) x - (*) (new)
 * Set option flags for the next regular expression. Takes next word in the
 * keyboard buffer and evaluates each leter as a flag, logically OR'ing them
 * together into re_flags.
 * 
 * flag	meaning						Expression Modifiers
 * ---------------
 * b	use basic regex syntax instead of extended
 * s	perform substitution
 * i	ignore case
 * g	match globally (ie, match all occurrences)
 * m	treat as multiple lines
 * n	need not return the matches, just a boolean
 */
begin(x)
	int i;

	word();
	drop(sst);

	re_flags = 0;
	for ( i = 0; i < pad->l; i++ )
	{
		switch ( (&pad->s)[i] )
		{
			case 'b': { re_flags &= ! REG_EXTENDED;	break; };
			case 'i': { re_flags |= REG_ICASE;	break; };
			case 'm': { re_flags |= REG_NEWLINE;	break; };
			case 'n': { re_flags |= REG_NOSUB;	break; };
			case 's': { re_flags |= REG_SUBST;	break; };
			case 'g': { re_flags |= REG_GLOBAL;	break; };
		};
	
	}
end()
/**(regex) rs() - (new)
 * Regular expression substitution. Replacement string is at TOS, search
 * string at TOS-1, and a pointer to the precompiled RE follows us in the
 * instruction stream. Returns the new string and the number of times the
 * pattern matched if a substitution could be made, otherwise it returns
 * the string searched (unchanged) and FALSE.
 */
begin(rs_)
	regex_t *reg;
	regmatch_t p[10];
	char *s, *r, *n, *np, *sp, *nr, *nrp;
	string *ns, *os;
	int rl, sl, count = 0;
	long flags;

	ip++;
	flags	= (long)(long*)*ip++;
	reg	= (regex_t *)*ip;

	printk("rs()");

	rl	= peek(sst).v.s->l;
	r	= scpop(sst);
	os	= spop(sst);
	sl	= os->l;
	s	= &os->s;
	
	sp = s;

	if ( regexec( reg, s, 10, p, 0 ) == 0 )
	{	
		{
		char *i;
		int c;
		int siz;
		/* found a match */
		nrp = nr = malloc( siz = rl + 128 );
		for (;;)
		{
			if ( ( i = (char*)memccpy( nr, r, '\\', rl ) ) == NULL )
			{
				r	= nrp;
				rl	= (nr - nrp) > 0 ?
					  ((nr - nrp) + rl) - 1 : rl;
				break;
			}
			
			r  += i - nr;
			rl -= i - nr;
			nr = i - 1;
			
			/* found backreference. */
			if ( *r >= '0' && *r <= '9' )
			{
				int len;
				
				/* interpolate reference */
				c = *r - '0';

				if ( (nr - nrp ) +
				     (len = p[c].rm_eo - p[c].rm_so) > siz
				)
				{
					int ofs;
					ofs = nr - nrp;
					nrp = realloc( nrp, siz += len + 128 );
					nr = nrp + ofs;
				}
				
				memcpy( nr, s + p[c].rm_so, len );
				nr += len;
				r++;
			}
		}
		}
	
		{
		int siz;
		/* Guess what length the result will be, and allocate memory
		 * for it. This will be extended automatically, if
		 * insufficient. */
		ns = malloc( sizeof(string) + (siz = sl + rl + 128) );
		n = np = &ns->s; 
	
		for (;;)
		{
		/* find additional matches. */
			if ( ( regexec( reg, s, 1, p, 0 ) == 0 )
			     && 
			     ( flags & REG_GLOBAL || count < 1 )
			)
			{		
			/* first match in list represents the entire matching
			 * portion of the expression. This is what we replace.
			 */
				if ( p[0].rm_so != -1 )
				{

					if ( (n - np) + rl + p[0].rm_so > siz )
					{
						int ofs;
						ofs = n - np;
						ns = realloc( ns,
							      sizeof(string) +
							      (siz += 128 +
							      rl + p[0].rm_so) );
						np = &ns->s;
						n = np + ofs;
					}
					
					/* copy the stuff between this match and
					 * the last one into the string. */
					if ( p[0].rm_so > 0 )
					{
						memcpy( n, s, p[0].rm_so );
						s += p[0].rm_so;
						n += p[0].rm_so;
					}
					/* insert replacement */
					memcpy( n, r, rl );
					n += rl;
				
					/* advance past this match */
					s += (p[0].rm_eo - p[0].rm_so);
				}
				count++;
			}
			else
			{
				if ( (n - np) + (sl - (s - sp)) > siz )
				{
					int ofs;
					ofs = n - np;
					ns = realloc( ns,
						      sizeof(string) +
						      (siz += 128 +
						      (sl - ( s - sp ))));
					np = &ns->s;
					n = np + ofs;
				}
				
				/* last mactch. finish output string and 
				 * add it to the stack */

				/* copy non matching trailing characters. */
				memcpy( n, s, sl - (s - sp) ); 
				n += sl - (s - sp);
			
				ns->l = (n - np);
				*n = '\0';
				
				/* shrink buffer down to the actual size
				 * of the string */
				ns = realloc( ns, sizeof(string) + ns->l + 1 );
				ns = gmark( ns, gstack, &gst );
				
				spush(sst,ns);
				fpush(sst,count);
				break;
			}
		}
		}
		free(r);
	}
	else
	{
		/* not a match */
		spush(sst,os);
		fpush(sst,FALSE);
	}
end()	
/**(regex) r() - (new)
 * Attempt match the string at TOS against the precompiled regular expression
 * pointed to by the next item of instruction stream. Leaves submatches on
 * the stack, followed by a count, or FALSE if no matches could be found.
 */	
begin(r_)
	regex_t *reg;
	int matches = hash_match.parm.v.f;
	regmatch_t p[matches];
	char *s;

	p[0].rm_so = -1;
	
	s = scpop(sst);

	ip++;
	reg = (regex_t *)*ip;

	if ( regexec( reg, s, matches, p, 0 ) == 0 )
	{
	 	int i, c;
		string *n;

		if ( p[0].rm_so == -1 )
			fpush(sst,TRUE);
		else
		{
			/* found a match */
			for ( i = c = 0; i < matches; i++ )
			{
				if ( p[i].rm_so != -1 )
				{
					int len;
					c++;

					len = p[i].rm_eo - p[i].rm_so;
					
					n = gmalloc( sizeof(string) + 1 + len,
							gstack, &gst );

					memcpy( &n->s, &s[ p[i].rm_so ], len );
					
					n->l		= len;
					(&n->s)[len]	= '\0';
					
					spush(sst,n);
				}
			}
			fpush(sst,c);
		}
	}
	else 
		fpush(sst,FALSE);	
	
end()
#endif
	
#define st_iliteral() {  \
	float v; \
	char *s; \
	char *end; \
 \
	end = s = scpop(sst); \
	 \
	v = strtol( s, &end, radix.parm.v.f ); \
 \
	if ( end != s ) \
	{ \
		if ( *end == '.' ) \
		{ \
			/* Looks like a float. \
			 * FIXME: Too bad we can only \
			 * read floats in base 10. */ \
			v = strtof( s, &end ); \
 \
			if ( *end == '\0' ) \
			{ \
				fpush(sst,v); \
				fpush(sst,TRUE); \
			} \
			else \
				fpush(sst,FALSE); \
		} \
		else if ( *end == '\0' ) \
		{ \
			fpush(sst,v); \
			fpush(sst,TRUE); \
		} \
		else \
			fpush(sst,FALSE); \
	} \
	else \
		fpush(sst,FALSE); \
 \
}
/**(string) iliteral
 * "( '1.23 - 1.23 )"
 * Evaluate the string at TOS. If the string represents an integer or
 * floating point literal then leave its value at TOS-1 and TRUE at TOS.
 * Otherwise discard the string and leave FALSE at TOS. */
begin(iliteral)
	st_iliteral();
end()
begin(next)
	/* check stack bounds */
	st_errch();

	/* Increment IP and then call the thing pointed to by
	 * the code field of that entry in the vocabulary. */

	ip++;
#ifdef DEBUG
	if ( *ip == NULL )
		error("null instruction! forgot to add it to the vocabulary?\n");
#endif
	execute(*ip);
end()
/**(dictionary) (constant)
 * Push the contents of the current word's parameter field onto the stack.
 */
begin(_constant)
	push(sst,self->parm);
end()
#define st_constant() {\
	st_enter(0); \
	current->last->code = adr(_constant); \
	current->last->parm = pop(sst); \
	current->last->type = A_CONST; \
}
/**(dictionary) fconstant
 * "1 'foo constant"
 * Define TOS to be a constant, with the value of TOS-1.
 *
 * Execute ENTER. Add set the code field of the new dictionary entry to point
 * to (constant), place the value from TOS-1 in the word's parameter field.
 */
begin(fconstant)
	st_constant();
end()
/* $variable
 * allocate memory for and initialize string constant */
#define st_dolar_constant() {\
	string *s, *n; \
	st_constant(); \
	s = current->last->parm.v.s; \
	n = malloc( sizeof(string) + s->l + 1 ); \
	memcpy( n, s, sizeof(string) + s->l + 1 ); \
	current->last->parm.v.s = n; \
	current->last->code = adr(_constant); \
}
begin(dolar_constant)
	st_dolar_constant();
end()
/**(dictionary) (variable)
 * Push the address of the current word onto the stack.
 */
begin(_variable)
	rpush(sst,self);
end()
/**(dictionary) ((variable))
 *
 */
begin(__variable)
	current->last->type = A_VAR;
	current->last->code = adr(_variable);
end()
/**(array) array
 * "[ 1 2 3 ] 'foo array"
 * Define an array. Name is at TOS, count at TOS-1, and first element at TOS-n
 */
begin(array)
	int n, i;
	cell *a;
	cell b;
	struct voc_entry *entry;
	
	st_enter( sizeof( cell ) );
	entry = current->last;
	
	n = fpop(sst);
	
	a = malloc( (n + 128) * sizeof( cell ) );

	entry->type = A_ARRAY;
	entry->code = adr(_variable);

	a += n;

	/* elements must be protected from the gc */
	for ( i = n; i > 0; i-- )
	{
		b = pop(sst);
		if ( b.type == T_STR )
		{
			string *s;
			/* string must be copied and therefore protected
			 * from the garbage collector. */
			
			s = malloc( sizeof(string) + b.v.s->l + 1 );
			memcpy( s, b.v.s, sizeof(string) + b.v.s->l + 1 );

			b.v.s = s;
		}
		
		*(a--) = b;
	}

	/* record the array's length */
	entry->parm.type	= T_FLT;
	entry->parm.v.f		= n;

	(&entry->parm)[1].type = T_PTR;
	(&entry->parm)[1].v.p  = a;

	printk("defining array (%i elements)", n);
end()
/**(array) hash
 * "[ 'key1 1 'key2 2 'key3 3 ] 'foo hash"
 * Define an associative array (hash table). The hash will grow and shrink
 * as keys are added and deleted. There may be no duplicate keys, and the
 * order in which keys were added cannot be retrieved. It is an error to
 * initialize a hash with an odd number of elements.
 * 
 * Name is at TOS, count at TOS-1, and first element at TOS-n
 */
begin(hash)
	struct voc_entry *entry;
	int n;
	hash_tab *h;
	cell a;

	/* enter hash name in the dictionary */
	st_enter( sizeof( cell ) );
	entry = current->last;
	
	entry->type = A_HASH;
	entry->code = adr(_variable);
	
	n    = fpop(sst);

	if ( n % 2 != 0 )
		error("unbalanced elements in hash initialization\n");

	h = hash_create( 8 );
//	h = hash_create( log( n ) );

	printk("defining hash (%i key/value pairs)", n / 2);
	
	/* add initial key/value pairs to hash */
	for ( ; n > 0; n -= 2 )
	{
		a = pop(sst);
		hash_put( h, scpop(sst), &a );
	}

	/* record the hash's length */
	/* FIXME: This must be tracked somehow. */
	entry->parm.type	= T_FLT;
	entry->parm.v.f		= h->used;

	(&entry->parm)[1].type = T_REF;
	(&entry->parm)[1].v.p  = h;
end()
#ifdef THREADS
/**(thread) shared - (new)
 * "'foo 'bar variable shared"
 * Declare the most recently defined word to be shared among threads.  This
 * means that STOICAL will serialize access to it in a 'one writer, many
 * readers' fashion.
 */
begin(shared)
	pthread_rwlock_init( &current->last->lock, NULL );
	current->last->type |= A_SHARE;
end()
#endif
/**(string) string
 * Return a pointer to an emtpy string of length TOS.
 */
begin(string)
	string *s;
	int l;

	l = fpop(sst);
	
	s = gmalloc( sizeof(string) + l + 1, gstack, &gst );
	s->l = l;
	
	spush(sst,s);
end()
/**(dictionary) definitions
 * Copy the top of the vocabulary stack into "current".
 */
begin(definitions)
	current = peek(vst);
end()
/**(dictionary) stoical< (*)
 * Push a pointer to the STOICAL kernel vocabulary onto the vocabulary stack.
 */
begin(stoical)
	push(vst,stoical);
end()
/**(dictionary) (branch) - (new)
 * Push the pointer stored in the calling word's parameter field onto
 * the vocabulary stack.
 */
begin(_branch)
	push(vst,self->parm.v.p);
end()
/**(dictionary) branch
 * "'foo branch"
 * Create a branch off of the current vocabulary, with the name found in TOS.
 */
begin(branch)
	string *s, *n;
	
	s = spop(sst);

	n = malloc( sizeof(string) + s->l + 2 );

	memcpy( &n->s, &s->s, s->l + 1 );

	(&n->s)[ s->l ] = '<';
	(&n->s)[ ++s->l ] = '\0';
	n->l = s->l;
	
	/* Create a vocabulary entry with the name "NAME"<
	 * and with a value being a pointer to the new branch. */
	
	rpush(sst,voc_create(&n->s));
	spush(sst,n);
	st_constant();

	/* Set the code address to point to a routine which will push
	 * the pointer onto the vocabulary stack */
	current->last->code = adr(_branch);
	current->last->type = A_IMM | A_VOCAB;
	
	free(n);
end()
/**(dictionary) disregard - (new)
 * "'foo disregard"
 * Works like FORGET, except that it removes ONLY the specified vocabulary
 * entry. (only searches CURRENT for NAME)
 */
begin(disregard)
	struct voc_entry *entry;
	string *s;
	
	s = spop(sst);

	if ( ( entry = voc_lookup( current, s->l, &s->s ) ) != NULL )
		entry_delete(current, entry);
end()
begin(badtype)
	error("type mismatch\n");
end()
begin(undefined)
	printk("undefined");
	printf("%s undefined\n", &pad->s );
	exec(*adr(abort));
end()
/**(io) frdline
 * Just line rdline, but when input is from something other than a 
 * terminal. Routines that change the input stream must also change
 * the code pointer in the rdline vocabulary entry accordingly.
 * (This is to avoid constantly checking where input is coming from)
 */
begin(frdline)
	int len;
	
	eol = FALSE;
	eoc = TRUE;

	if ( fgets( &tib->s, KSIZE - 2, finput ) == NULL )
		/* eof - return control to the file
		 * that loaded us (that's ;f's job) */
		exec(*adr(semicolon_f));

	len = strlen( &tib->s );
	
	if ( len >= KSIZE - 2 )
		error("input buffer full");

	tib->l = len;
	(&tib->s)[len - 1] = '\0';
	tibp = 0;
end()
/**(io) rdline
 * Read a line of input into the Terminal Input Buffer.
 */
begin(rdline)
	int len;
	
	/* FIXME: Add buffer modification tracking to emulate stoic's
	 * escape press reaction of recompiling the last line.
	 */

	eol = FALSE;
	eoc = TRUE;

#ifdef READLINE
	{ char *str = NULL;
	if ( ( str = readline("") ) == NULL )
		exec(*adr(semicolon_f));

	len = strlen(str);
#else
	fgets( &tib->s, KSIZE, stdin );
	len = strlen( &tib->s );
	/* remove the newline */
	(&tib->s)[ --len ] = '\0';
#endif

	if ( len >= KSIZE - 1) 
		error("keyboard buffer full");
		
#ifdef READLINE	
	add_history( str );
	strncpy( &tib->s, str, len + 1 );
	free(str);
	}
#endif
	tib->l = len;
	tibp = 0;
end()
/**(dictionary) > - (*)
 * Pop off the top of the vocabulary stack.
 */
begin(right_angle)
	drop(vst);
	exec(*adr(errch));
end()
/**(dictionary) address
 * "'foo address"
 * 
 * Lookup the address of the string TOS in the dictionary. Error if
 * undefined.
 */
begin(address)
	struct voc_entry *entry;
	
	if ( ( entry = stt_lookup(scpop(sst)) ) == NULL )
				exec(*adr(undefined));

	rpush(sst,entry);
end()
/**(dictionary) () - (*)
 * This word looks up the address of the following word in the vocabulary
 * and replaces itself with code to push that address onto the stack at
 * runtime. Lookup next word. Push address of l() onto the compile stack. Push
 * address we found onto compile stack. (in a cell).
 */
begin(_)
	word();
	drop(sst);

	rpush(sst,stt_lookup(&pad->s));
	st_lit();

	if ( peek(cst) == NULL )
		exec(*adr(undefined));
end()
/**(string) ascii - (*)
 * "ascii f"
 * Replace self with the literal value of the first character of the next
 * word in the TIB
 */
begin(ascii)
	word();
	drop(sst);

	fpush(sst,*( &pad->s ));
	st_lit();
end()
/**(dictionary) // - (*)
 * "'foo : // 1 = // ;"
 * Complement compiler state. A True state causes all words to act as Immediate.
 */
begin(fslash_fslash)
	state = !state;
end()
/**(stack) .t 
 * Replace TOS with the type of what we replaced. 
 */
begin(dott)
	peek(sst).v.f	= peek(sst).type;
	peek(sst).type	= T_FLT;
end()
/**(unary) exec
 * Execute word who's address is at TOS.
 */
begin(exec)
	self = ppop(sst);

	execute(self);
end()
/**(dictionary) self
 * Push a pointer to the vocabulary entry for the calling word onto the stack.
 */
begin(self)
	rpush(sst,peek(rst).p);
end()
/**(iterative) redo 
 * "'foo : 1 = redo ;"
 * Cause the calling word to begin execution over at its first instruction.
 * This effectively creates tail-recursive calls.
 *
 * Execute the word addressed by the self pointer on top of the return stack.
 * (the word being executed)
 */
begin(redo)
	struct voc_entry *entry;

	entry = pop(rst).p;

	execute(entry);
end()
/**(stack) retype
 * "'foo 3 retype"
 * Set type of TOS-1 to the value of TOS. 
 */
begin(retype)
	idx(sst,1).type = fpop(sst);
end()
/**(compiler) prompt
 * Display the compiler prompt on the output device.
 */
begin(prompt)
	int i;

	if ( toutput == stdout )
	{
//		if ( column != 0 )
//			putchar('\n');
		
		printf("\033[1;30m(\033[0m%i\033[1;30m)\033[37m", depth);

		for ( i = depth; i > 0; i-- )
			putchar('\t');

		printf("->\033[0m ");

		fflush(stdout);
	}
end()
/**(conditional) (else)
 * Increments instruction pointer by the contents of the next
 * value in the instruction steam.
 */
begin(_else)
	ip += (ub4)*(ip + 1);
end()
/**(conditional) (if)
 * Tests TOS. If zero, the instruction pointer is incremented. Otherwise,
 * instruction pointer is incremented by the contents of the next value in the
 * instruction stream.
 */  
begin(_if)
	if ( fpop(sst) != FALSE )
		ip++;
	else
		/* Doesn't return! */
		exec(*adr(_else));
end()

/**(iterative) ({do) 
 * Setup loop stack in anticipation of ({loop)
 */
begin(_brace_do)
	/* tos >= tos-1 ? */
	if ( peek(sst).v.f >= idx(sst,1).v.f )
		/* indces are bogus. skip the entire loop */
		ip += 3;
	else
	{
		/* stack: 10 0 do ... loop */
		/* Move tos-1, followed by two copies of tos, onto the loop
		 * stack */
		push(lst,idx(sst,1));
		push(lst,peek(sst));
		push(lst,pop(sst));
		drop(sst);

		/* make the first iteration */
		ip += 2;
	}
end()
/**(iterative) ({loop) (new)
 * Inrement the top of the loop stack and compare the result against TOP-2
 * of the loop stack. Result is grater than or equal to TOP-2, then the
 * loop stack is cleaned up and the loop execution continues at the next
 * instruction. Otherwise, the loop is incomplete, so we jump back one
 * instruction to the begining of the loop.
 */
begin(_brace_loop) 
	/* Increment the top of the loop stack and compare the result
	 * against top-2 of the loop stack. */
	if ( ++((lst - 1)->v.f) >= idx(lst,2).v.f )
	{
		/* Loop is complete. Pop top three items from the
		 * loop stack, and increment ip by 2. */
		lst -= 3;
		ip += 2;
	}
	else
		/* Still more to go. Increment IP */
		ip++;
end()
/**(iterative) ({+loop)
 * Add TOS to top of loop stack, otherwise the same as loop.
 */
begin(_brace_plus_loop)
	if ( ( ((lst - 1)->v.f) += fpop(sst) ) >= idx(lst,2).v.f )
	{
		lst -= 3;
		ip += 2;
	}
	else
		ip++;
end()
/**(iterative) {loop - (*)
 * "hi lo {loop { ... }"
 * Rewrite the compile stack to look like:
 *
 * ({do) ({loop) nop nop b{ ... b}
 */
begin(brace_loop)
	push(cst,stt_lookup("({do)"));
	push(cst,stt_lookup("({loop)"));
	push(cst,stt_lookup("..."));
	
	clause = adr(_b_left_brace);	
end()	
/**(iterative) {+loop - (*)
 * "hi low {+loop { ... incr }"
 * Rewrite the compile stack to look like:
 * 
 * hi low (do) (+loop) nop nop b{ ... incr b}
 */
begin(brace_plus_loop)
	push(cst,stt_lookup("({do)"));
	push(cst,stt_lookup("({+loop)"));
	push(cst,stt_lookup("..."));
	
	clause = adr(_b_left_brace);	
end()
/**(iterative) ({()
 * Note that this version differs from STOIC's in that it sets up the loop
 * stack so that '-i' produces sane results.
 *
 * Increment IP by the value of the next item in the instruction stream if TOS
 * is less than, or equal to 0. This handles the case that the iteration count
 * given to us amounts to 0 times. Otherwise, push a zero and the value of TOS
 * plus 2, followed by TOS, onto the loop stack.
 */
begin(_brace_left_paren)
	if ( (peek(sst).v.f) <= 0 )
		/* This handles the case that the iteration count
		 * given us is less than or equal to zero. We therefore
		 * execute the loop zero times */
		ip++;
	else
	{
		/* Increment top of loop stack twice.
		 * Move TOS onto loop stack. And increment IP. */
		fpush(lst,0);
		fpush(lst,peek(sst).v.f + 2);

		push(lst,pop(sst));
	}
end()
/**(iterative) ({)) 
 * Decrement the value on top of the loop stack. If the result is zero, then
 * the loop is over. Clean up the loop stack.
 * Otherwise, decrement the instruction pointer to repeat the loop.
 */
begin(_brace_right_paren)
	if ( --(peek(lst).v.f) == 0 )
	{
		/* Pop top three values off of the loop stack and
		 * increment IP */
		lst -= 3;
	}
	else
		/* there's still more to go. Jump back an instruction. */
		ip -= 2;
end()
/**(iterative) {( - (*)
 * "time {( { ... }"
 * Rewrite the compile stack to look like:
 *
 * ({() { ... } ({))
 */
begin(brace_left_paren)
	push(cst,stt_lookup("({()"));

	clause = adr(_left_brace);
end()	
/**(stack) [[ - (new)
 * "[[ 1 2 3 ]]"
 * Leave a mark on the stack.
 */
begin(lleft_bracket)
	cmark(sst);
end()
/**(stack) ]] - (new)
 * "[[ 1 2 3 ]]"
 * Pop items, up to and including a mark, from the stack.
 */
begin(rright_bracket)
	int i, size;

	size = sst - sstmin;

	for ( i = 0; i < size; i++ )
		if ( pop(sst).type == T_MARK )
			break;
end()	
/**(stack) l]] - (new)
 * Pop items, up to and including a mark, from the loop stack.
 */
begin(l_rright_bracket)
	int i, size; 

	size = lst - lstmin;

	for ( i = 0; i < size; i++ )
		if ( pop(lst).type == T_MARK )
			break;
end()	
/**(stack) ([)
 * "[ 1 2 3 ]"
 * Push parameter stack pointer onto the loop stack. ']' will use this
 * later.
 */
begin(_left_bracket)
	ppush(lst,sst);
end()
/**(stack) (])
 * "[ 1 2 3 ]"
 * Calculate how many items have been added to the parameter stack since
 * '[' left the pointer for us on the loop stack. Push the result. 
 */
begin(_right_bracket)
	cell *oldsst;
	oldsst = ppop(lst);
	fpush(sst,(long)(sst - oldsst));
end()
/**(stack) [ - (*)
 * Execute +check. Push the address of ([) onto the compile stack.
 */
begin(left_bracket)
	depth++;
	push(cst,stt_lookup("([)"));
end()
/**(stack) ] - (*)
 * Execute -check. Push the address of (]) onto the compile stack.
 */
begin(right_bracket)
	st_minus_check();
	push(cst,stt_lookup("(])"));
end()
/**(iterative) ()do) (new)
 * Push an empty cell onto the loop stack. Move the hash pointer at TOS onto
 * the loopstack, and follow it with an empty string.
 */
begin(_paren_do)
	/* the type and value of this cell will be used as indices into the
	 * hash */	
	printk("()do)");

	ppush(lst,NULL);
	/* move the hash pointer at TOS onto the loop stack */
	ppush(lst,((cell*)&(((struct voc_entry*)(pop(sst).v.p))->parm))[1].v.p);
	
	spush(lst,NULL);

	ppush(lst,NULL);
	ppush(lst,NULL);
	/* put another throw-away value on the loop stack. (}loop) will
	 * replace this with a pointer to the current hash key */
	spush(lst,NULL);
	
	hash_next_reset( idx(lst,4).v.p, (ub4*)&(idx(lst,2).v),
					(ub4*)&(idx(lst,1).v) );
	
end()
/**(iterative) (){each) (new)
 * Place the next key from the hash at LST-1 on top of loop stack.  If there
 * are no more keys, then clean up the loop stack and skip over the next code
 * block to terminate the loop. Likewise terminate the loop if BREAK has been
 * called. Otherwise, jump back to the begining of the loop.
 */
begin(_paren_brace_each)
		char *s;

		printk("(){each)");
		if ( peek(lst).v.f == 1 )
		{
		/* someone called BREAK. clean up and leave loop */
			lst -= 6;
			ip += 2;
		}
		else
		{	
			s = hash_next( idx(lst,4).v.p,
				       (ub4*)&(idx(lst,2).v),
				       (ub4*)&(idx(lst,1).v) );

			if ( s != NULL )
			{
				string *n;
				int l;

				l = strlen(s);
				
				n = gmalloc( sizeof(string) + l + 1, gstack, &gst );
				n->l = l;
				memcpy( &n->s, s, l + 1 );

				peek(lst).type	= T_STR;
				peek(lst).v.s	= n;
		
				ip++;
			}
			else
			{
				/* no more keys in the hash. we're done.
				 * clean up the loop stack and exit. */
				lst -= 6;
				ip += 2;
			}
		}
end()
/**(iterative) ){each - (*) (new)
 * "hash( ){each ... }"
 * Rewrite the compile stack to look like:
 *
 * "()do) (){each) nop { ... }"
 */
begin(paren_brace_each) 
	printk("){each");
	
	clause = adr(_b_left_brace);

	push(cst,stt_lookup("()do)"));
	push(cst,stt_lookup("(){each)"));
	push(cst,stt_lookup("..."));
end()
/**(iterative) {until (new)
 * "{ ... } {until"
 * Backup an instruction if TOS is FALSE.
 */
begin(brace_until)
	if ( fpop(sst) == FALSE )
		ip -= 2;
end()
/**(iterative) {while (*) (new)
 * "{ ... } {while { ... }"
 * Rewrite the compile stack to look like this:
 * 
 * { ... } {if b{ b}
 */
begin(brace_while)
	clause = adr(_b_left_brace);

	push(cst,stt_lookup("{if"));
end()
/**(iterative) break
 * "5 ( i 3 eq if BREAK then i = )"
 * STOIC calls this EXIT, but that seems ambiguous.
 * Break is used to unconditionally terminate the inntermost looping construct
 * upon the next iteration. That is, the example above will print "5 4 1"
 * before ending.
 *
 * Set TOS and TOS-2 of loop stack to 1.
 */
begin(break)
	peek(lst).v.f	= 1;
	idx(lst,2).v.f	= 1;
end()
/**iterative fail
 * "5 ( fail )"
 * Synonym for break.
 */
begin(fail)
	exec(*adr(break));
end()
/**(conditional) {if
 * "n {IF { ... }"
 * Conditionally execute the following clause.
 * Increment IP if TOS is equal to the FALSE value.
 */
begin(brace_if) 
	if ( fpop(sst) == FALSE )
		ip++;	
end()
/**(conditional) {else - (*)
 * "n {if { ... } {else { ... }"
 * Rewrite the compile stack to look like:
 *
 * "{ifelse { ... } ({else) { ... }"
 */
begin(brace_else)

	if ( ( peek(cst)->type & A_TMASK ) != A_CLS )
		error("syntax error");

	idx(cst,1) = stt_lookup("{ifelse");

	push(cst,stt_lookup("({else)"));
end()
/**(conditional) (else)
 * Skip over the clause following us in the instruction stream
 */
begin(_brace_else)
	ip++;
end()
/**(conditional) {ifelse (new)
 * "{ifelse { true clause } ({else) { false clause }"
 */
begin(brace_ifelse)
	if ( fpop(sst) == FALSE )
		ip += 2;
end()
/**(variables) tib - (new)
 * Push the address of the Terminal Input Buffer onto the stack. The
 * TIB looks like any other string.
 */
begin(tib)
	spush(sst,tib);
end()
/**(compiler) compile
 * Compile textual instruction from the TIB, stacking them onto the compile
 * stack as we go.
 */
begin(compile)
	struct voc_entry *entry;

	printk("compile:");

	for (;;)
	{
		if ( eol != FALSE )
			break;
		
		word();	
		drop(sst);

		if ( pad->l < 1 )
			break;
		
		if ( ( entry = stt_lookup( &pad->s ) ) != NULL )
		{	
			/* Is a word */
			if ( state || entry->type & A_IMM )
			{
				/* Word is IMMEDIATE, execute it */
				printk("\timmediate word");

				/* Set the instruction pointer to point back to
				 * us so that after the immediate word passes
				 * control to NEXT we'll start running again.
				 * */
				ip--;
				execute(entry);
			}
			else
			{	
				printk("\tword");
				push(cst,entry);
			}
		}
		else if ( *( &pad->s ) == '\"' || *( &pad->s ) == '\'' )
		{
			string *s;
			printk("\tstring literal");
			/* Literal - string */

			/* Interpolate escape codes */
			interpolate( pad, 0 );

			s = stringdup( pad );

			/* push a pointer to the new string onto the compile
			 * stack */
			
			spush(sst,s);
			st_lit();
			
			/* mark the pointer on the collectables stack */
			push(ccst,((ccell){ free, s }));
		}
#ifdef REGEX
		else if ( *( &pad->s ) == '|' )
		{
			int v;
			regex_t *reg;

			reg = malloc( sizeof( regex_t ) );

			/* interpolate escape sequences */
			interpolate( pad, 1 );
			
			v = regcomp( reg, &pad->s,
				     REG_EXTENDED ^ re_flags );
							
			/* FIXME: check for errors... */

			if ( re_flags & REG_SUBST )
			{
				printk("\tregular expression substitution");
				push(cst,(stt_lookup("rs()")));
				push(cst,(void*)re_flags);
			}
			else
			{
				printk("\tregular expression");
				push(cst,(stt_lookup("r()")));
			}

			push(cst,(void*)reg);
			
			/* mark as pointer as collectable */
			push(ccst,((ccell){ regfree, reg }));

			/* clear re_flags that 'x' may have set */
			re_flags = 0;
		}
#endif
		else
		{
			spush(sst,pad);
			st_iliteral();
			
			if ( fpop(sst) != FALSE )
				st_lit();	
			else
				exec(*adr(undefined));
		}
	}

end()
/**(compiler) clearcst
 * clear the compile stack
 */
begin(clearcst)
	/* Clear compile stack */
	cst = cstmin;

	if ( ccst - ccstmin )
		ccst = free_collectables( ccstmin, &ccst );
	
	push(cst,(stt_lookup("(:)")));
end()
/**(system) system
 * "'ls system"
 * Execute a shell to run the command reflected in the string at TOS.
 * Leaves the shell's return value on the stack.
 */
begin(system)	
	int rval;
	rval = system(scpop(sst));

	fpush(sst,rval);

end()
/**(system) env>
 * "'PWD env>"
 * Replace string at TOS with the STRING value of the environment variable by
 * that name. (Or FALSE if not found)
 */
begin(env_right_angle)
	char *s;
	string *n;
	int len;
	
	printk("getenv:");
	s = getenv(scpop(sst));

	if ( s != NULL )
	{
		len = strlen(s);

		n = gmalloc( sizeof(string) + len + 1, gstack, &gst );

		memcpy( &n->s, s, len + 1 );
		n->l = len;

		spush(sst,n);
	}
	else
	{
		fpush(sst,FALSE);
	}	
end()
/**(system) <env
 * "'/ 'HOME <env"
 * Set environment variable who's name matches the string at TOS to the
 * string value at TOS-1
 */
begin(left_angle_env)
	printk("setenv()");
	setenv(scpop(sst), scpop(sst), 1); /* 1 means overwrite */
end()
/**(compiler) eval
 * Execute STOICAL source code in string at TOS.
 */
begin(eval)
	/* FIXME: There are numerous promlems with this setup. Just one being
	 * the fact that (eval) will never be called if the evaluated string
	 * contains the comment character. And only single lines can be
	 * evaluated at a time. Maybe this is a symptom of a larger problem
	 * with the input system ... */
end()
/**(compiler) execc
 * Execute the contents of the compile stack.
 */
begin(execc)
//	push(rst,((rcell){ self, ip }));
	push(rst,((rcell){ line, ip }));
	push(cst,(stt_lookup("(;)")));
	
	line->size = cst - cstmin;

	ip = (void *)cstmin;
end()

