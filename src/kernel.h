
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

#include "config.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
// #include <stdarg.h>
#include <unistd.h>
#include <math.h>	/* fabs() */

/* file handling */
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

/* networking */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#ifdef HAVE_SYS_SENDFILE_H
#	include <sys/sendfile.h>
#endif

#ifdef THREADS
/* threads */
#	define _MULTI_THREADED
#	include <pthread.h>
#endif

#ifdef REGEX
/* regular expressions */
#	include <regex.h>
	/* fake reg options that only have meaning to STOICAL */
#	define REG_SUBST	(1 << 16)
#	define REG_GLOBAL	(1 << 17)
#endif

#ifdef READLINE
/* For readline support */
#	include <readline/readline.h>
#	include <readline/history.h>
#endif

/* Default stack and buffer sizes. These are arbitrary, for the most part */
#define SSIZE	256
#define CSIZE	256
#define GSIZE	256
#define NSIZE   40
#define KSIZE   256

/* redefine true and false as they apply to STOICAL. */
#undef TRUE
#undef FALSE
#define TRUE	-1
#define FALSE	0

typedef unsigned char type_t;

/* all strings on the stack will have this format. */
typedef struct {
	unsigned int l;	/* length of string */
	char s[];		/* string itself */
} string;
typedef void(function)();
/* value portion of a stack cell */
typedef union {
	double f;
	void *p;
	string *s;
} value;
/* cell; used on the parameter and loop stacks only */
typedef struct {
	type_t type;
	value v;
} cell;
/* rcell; used on the return stack */
typedef struct {
	struct voc_entry *p;		/* invoking word */
	struct voc_entry **i;		/* invocation of current word */
} rcell;

/* Bit field portion of word types */
#define A_CMP		(0 << 7) /* compile into other words */
#define A_IMM		(1 << 7) /* immediate, execute while compiling */
#define A_SHARE		(1 << 6) /* implicitly locked */
#define A_CHECK		(1 << 5) /* type checked */

/* Word types */
#define A_TMASK		15 /* bitmask to extract the type alone */
#define A_WRD		0 /* word */
#define A_CONST		1 /* constant */
#define A_VAR		2 /* variable */
#define A_ARRAY		3 /* array */
#define A_HASH		4 /* associative array */
#define A_CLS		5 /* clause */
#define A_VOCAB		6 /* vocabulary. name will always end in '<' */
#define A_MAX		7
#define A_OVER		0	 /* bit offset */

/* Data types - May be 255. */
#define T_TMASK	255	/* bitmask to extract just the type */
#define T_MARK  0	/* stack marker */
#define T_NIL	1	/* No type / Don't care */
#define T_FLT	2	/* float */
#define T_STR	3	/* string */
#define T_REF	4	/* pointer to another word ( string, float ) */
#define T_PTR	5	/* pointer to a region of memory. */
#define T_FIL	6	/* file/stream handle */
#define T_ARRAY 7	/* array */
#define T_HASH	8	/* hash table */
#define T_CLS	9	/* clause ( code block ) */
#define T_MAX	10	/* this should always be one more than the last type */
#define T_OVER  0	/* bit offset */


/* Utility macros */
#define max(l,h) ((l) > (h) ? (h) : (l))
#define min(l,h) ((l) < (h) ? (h) : (l))
#define ornull(s)	(s == NULL ? "NULL" : s)

/* Stack manipulation macros */
/* drop is similary to pop, except that it returns no value */
#define drop(s) (--s)
#define pop(s) (*(--s))
#define push(s,v) (*(s++) =v)
#define peek(s) (*(s - 1))
#define idx(s,n) (*(s - (n + 1)))
#define swap(s) ({ \
		cell a, b; \
		a = pop(s); \
		b = pop(s); \
		push(s,a); \
		push(s,b);\
		}) 

/* Macros to manipulte just the values on a stack of type/value pairs */
#define cpop(s) ((--s)->v)
#define ctpush(s,k,t) (*(s++) = (cell){ t, (value)k })

#define cmark(s) (*(s++) = (cell){ T_MARK, { 0 } })
#define cismark(s,n) ( (s - (n + 1))->type == T_MARK )

/* Type specific stack operations. These are only for stacks
 * of cell's. (sstack and lstack) */
#define spush(st,k) (*(st++) = (cell){ T_STR, { .s = k }})
#define fpush(st,k) (*(st++) = (cell){ T_FLT, { .f = k }})
#define ppush(st,k) (*(st++) = (cell){ T_PTR, { .p = k }})
#define rpush(st,k) (*(st++) = (cell){ T_REF, { .p = k }})
#define filpush(st,k) (*(st++) = (cell){ T_FIL, { .p = k }})


#define scpop(st) ( ((--st)->v.s->s) )

#define filpop(s) ((--s)->v.p)
#define ppop(s) ((--s)->v.p)
#define rpop(s) ((--s)->v.p)
#define fpop(s) ((--s)->v.f)
#define spop(st) ((--st)->v.s)

#define begin(name)	s_ ## name: {
#define end()		goto *adr(next); }
#define exec(a)		goto a
#ifdef PROFILE
#	define execute(ad)	({self = ad;self->called++;exec(*self->code);})
#else
#	define execute(ad)	({self = ad; exec(*self->code); })
#endif

#define adr(name)	&&s_ ## name

#define stt_lookup(n)	dict_lookup(n,vst,vstmin)
#define st_minus_check() if ( --depth < 0 ) error("syntax error\n")
#define error(...)	({fprintf(stderr, __VA_ARGS__); diagnostic(); exec(*adr(abort));})

/* take the cell ontop of the sstack and move it to the compile buffer. */
#define st_lit()	({ \
				push(cst,(stt_lookup("l()"))); \
				*((cell*)cst) = pop(sst); \
				(cell*)cst++; \
				cst++; \
			})
#ifndef HAVE_STRTOF
/* emulate strtof() */
#	define strtof(s,nptr) ((*(nptr) = (s) + strspn((s),"-0123456789eE."))\
				, atof((s)) )
#endif

	
struct voc_item {
	type_t type;
	char *name;
	function *code;
};

/* collectable cell.. used on the collectable stack */
typedef struct {
	function *handler;	/* cleanup function to call */
	void *ptr;		/* pointer to give it as an argument */
} ccell;
	
struct voc_entry {
	function *code;
	type_t type;
	int size;		/* size of entry in instructions */
#ifdef PROFILE
	int called;		/* number of invocations */
#endif
#ifdef THREADS
	pthread_rwlock_t lock;
#endif
	int len;
	char *name;
	int collectables;	/* length of collectable array */
	ccell *collectable;	/* array of pointers and destructors */
	struct voc_entry *last;
	/* Dictionaries are doubly linked in STOICAL to support
	 * the simple removal of individual entries. */
	struct voc_entry *next;
	cell parm;		/* parameter field, will be extended later */
};

/* the fisrt three elements of this structure must match those of voc_entry */
struct code {
	function *code;
	type_t type;
	int size;	/* size of code block in instructions */
	int cols;	/* collectables stack depth upon entry */
	struct voc_entry *parm;
};

typedef struct {
	ub4 type;
	ub4 mask;
	struct code *clause;
} type_vec;

typedef struct {
	ub4 num;
	ub4 max;
	type_vec *vec;	
} type_head;

struct vocabulary {
	int words;
	char *name;
	long type;
	struct voc_entry *last;
};

struct iopoint {
	struct voc_entry in, out, inln, outln;
};

struct thread_data {
	/* Pointers to stack copies that we can use. */
	cell *sst;
	cell *sstmin;
	struct vocabulary **vst;
	struct vocabulary **vstmin;
	void **gstack;
	unsigned char *gst;

	/* Entry point for thread. Can point to any word. */
	struct voc_entry *entry;

	/* Terminal io pointers. */
	struct iopoint *io;

	/* Virtual command line arguments for this thread */
	int argc;
	char **argv;
	int main;

	struct voc_entry ***ip;
	rcell **rst;
	
#ifdef THREADS
	pthread_t thread;
#endif
};

/* file structures */

typedef struct {
	FILE	*fp;		/* Stream pointer */
	int	fd;		/* File descriptor */
} t_file;
typedef struct {
	FILE	*fp;		/* Stream pointer */
	int	fd;		/* File descriptor */
	int	pf;		/* protocol family */
	int	af;		/* adress family */
	int	st;		/* socket type */
	int	p;		/* protocol */
} t_sock;

/* dict.c function prototypes */

struct vocabulary *voc_create (char *name);
struct voc_entry *voc_append (struct vocabulary *voc, struct voc_item *item, int extra);
void voc_destroy (struct vocabulary *voc);
void entry_delete (struct vocabulary *voc, struct voc_entry *entry);
struct voc_entry *voc_lookup (struct vocabulary *voc, int len, char *name);
struct voc_entry *dict_lookup (char *name, struct vocabulary **vst, struct vocabulary **vstmin);
void entry_free( struct voc_entry *entry );
struct voc_entry *entry_resize( struct voc_entry *entry, int size );
void entry_collectable ( struct voc_entry *entry, function *handler, void *ptr );

void entry_pop_collectable ( struct voc_entry *entry );

/* mem.c function prototypes */

void *gmalloc( int size, void **gstack, unsigned char *gst );
void *gmark( void *mem, void **gstack, unsigned char *gst );
void ginit( void **gstack, unsigned char *gst );
void gfree( void **gstack, unsigned char *gst );
void thread_destroy( struct thread_data *value );
ccell *free_collectables ( ccell *ccstmin, ccell **ccst );
void stack_dup( cell *dst, cell *src, int n );
void stack_mark( cell *stmin, cell *st, void **gstack, unsigned char *gst );

/* string.c function prototypes */

string* stringdup( string *s );
void interpolate( string *str, int mode );
int st_word( string *tib, int *tibp, string *pad );

/* signal.c function prototypes */

void sig_init ( void );

/* debug.c */

void debug (void);
#ifdef DEBUG
void printk(const char *fmt, ... );
#endif
void st_pcell ( cell *a );
void st_cstack ( cell *s, cell *m );
void st_decompile ( struct voc_entry *entry, struct voc_entry **high );
void diagnostic ( void );

/* term.c */
void term_save(void);
void term_restore(void);
void term_cbreak(void);

