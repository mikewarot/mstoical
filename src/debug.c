
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

#include <stdarg.h>
#include "kernel.h"

extern const char *t_types[];
extern const char *a_types[];
#ifdef THREADS
	extern const pthread_key_t tsd;
#else
	extern const struct thread_data *mdata;
#endif

/* This file contains various utility functions to aid the debuging process. */

#ifdef DEBUG
	/* Used to print kernel messages. Works like printf */
	void printk(const char *fmt, ... )
	{
		char *nfmt;
		va_list args;
		char pre[] = "kernel: \033[1;32m%s\033[0m\n";
		
		nfmt = malloc( strlen(fmt) + strlen(pre) );

		sprintf( nfmt, pre, fmt );
		
		va_start(args, fmt);
		vfprintf(stderr, nfmt, args);
		va_end(args);

		free( nfmt );
	}
#endif

/* just a breakpoint */
void debug (void) { }

/* Print on stderr a formatted version of the cell pointed to by a */
void st_pcell ( cell *a )
{
	switch ( a->type )
	{
		case T_STR:
		{
			if ( memchr( a->v.s->s, ' ', a->v.s->l ) == NULL )
				fprintf(stderr, "\'%s ", a->v.s->s );
			else
				fprintf(stderr, "\"%s\" ", a->v.s->s );
			break;
		}
		case T_REF:
		{
			if ( (((struct voc_entry*)a->v.p)->type	& A_TMASK)
			      == A_WRD )
			{
				fprintf(stderr, "() %s ",
					((struct voc_entry *)a->v.p)->name
				);
			} else
				{
				fprintf(stderr, "%s(%s) ",
				a_types[
				max(((((struct voc_entry*)a->v.p)->type &
							A_TMASK)
					>> A_OVER),A_MAX)
				], ((struct voc_entry *)a->v.p)->name );
			}
			break;
		}
		case T_FLT:
		{
			fprintf(stderr, "%g ", (double)a->v.f );
			break;
		}
		default:
		{
			fprintf(stderr, "%s(%p) ", 
			t_types[ max(a->type,T_MAX) ],
			a->v.p );
			break;
		}
	}
}
/* Print on stderr a formatted version of the contents of the stack represented
 * by s and m  */
void st_cstack ( cell *s, cell *m )
{

	if ( m > s )
		for (; s < m; s++)
			st_pcell( s );
	else
		fprintf(stderr, "(empty)" );

	fprintf(stderr, "\n");
}

void st_pword ( struct voc_entry *entry, struct voc_entry **high )
{
	struct voc_entry **p, **m;

	if ( entry->type == A_CLS )
		p = (void*)&((struct code *)entry)->parm;
	else
		p = (void *)&entry->parm;

	m = p + entry->size - 1;
	
	for ( ; p < m; p++ )
	{
		if ( p == high )
			fprintf(stderr, "\e[1;31m");

		if ( (*p)->type == A_CLS )
		{
			/* recurse */
			fprintf(stderr, "{ " );
			st_pword( *p, high );
			fprintf(stderr, "} ");
		}
		else if ( strcmp( (*p)->name, "l()" ) == 0 )
		{
			p++;
			st_pcell( (cell*)p );
		}
		else if ( strcmp( (*p)->name, "r()" ) == 0 ||
			  strcmp( (*p)->name, "rs()" ) == 0 )
		{
			p++;
			fprintf(stderr, "\e[1;30m(regex)\e[0m " );
		}
		else
		{
			fprintf(stderr, "%s ", (*p)->name );
		}
		
		if ( p == high )
			fprintf(stderr, "\e[0m");
	}
}
/* Print on stderr the formatted source version of the word 'entry' */
void st_decompile ( struct voc_entry *entry, struct voc_entry **high )
{
	if ( entry->name )
	{
		fprintf(stderr, "'%s : ", entry->name);
		st_pword( entry, high );
		fprintf(stderr, ";");
	}
	else
		st_pword( entry, high );

	fprintf(stderr, "\n");
	/* TODO: maybe print flags, like immediacy too */
}

/*
 * Print diagnostic information.
 */
void diagnostic ( void )
{
	struct thread_data *d;
	struct voc_entry *caller;

#ifdef THREADS
	d = pthread_getspecific( tsd );
#else
	d = (void*)mdata;
#endif
	caller = peek(*(d->rst)).p;
	
	fprintf(stderr, "stack: ");

	st_cstack( d->sstmin, (*((cell**)d->sst))  - 1 );

	fprintf(stderr, "context: ");

	st_decompile( caller, *d->ip );
}
