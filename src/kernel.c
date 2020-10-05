
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

const char version[] = VERSION;

#include "kernel.h"
#include "hash.h"
#include "help.h"

#ifdef THREADS
/* Thread Specific Data key. This is used by the thread destructor
 * routines. */
pthread_key_t tsd;
#else
struct thread_data *mdata;
#endif


struct vocabulary *stoical;
char *libroot;

/* Ascii names for cell types */
const char *t_types[] =
{ "mark", "nil", "float", "string", "reference", "pointer", "file",
  "array", "hash", "clause", "unknown!" };
/* Ascii names for word types */
const char *a_types[] =
{ "word", "constant", "variable", "array", "hash", "clause",
  "vocabulary", "unknown!" };

/* st_start
 * This is the entry point for new threads. */
void
st_start (struct thread_data *data)
{

struct vocabulary *voc;
int i, count;
struct voc_item item;
struct voc_entry *entry;

/* Declare stacks. Extra space is to absorb over/underflow */
cell lstack[SSIZE + 8];			/* Loop stack store	*/
ccell ccstack[ (SSIZE / 2) + 8 ];	/* Compile collectable stack */
struct voc_entry **bstack[SSIZE + 8];	/* Block stack		*/
rcell rstack[SSIZE + 8];	/* Return stack		*/
void *gstack[GSIZE];			/* Garbage stack store	*/

/* Declare stack pointers */
cell *sst;
cell *sstmin;
struct vocabulary **vst;
struct vocabulary **vstmin;
	
cell *lst			= &lstack[4];
cell *lstmin			= &lstack[4];
struct voc_entry **cst;
ccell *ccst			= &ccstack[4];

struct voc_entry **cstmin;
struct voc_entry ***bst		= &bstack[4];
struct voc_entry ***bstmin	= &bstack[4];

rcell *rst			= &rstack[4];
rcell *rstmin			= &rstack[4];
ccell *ccstmin			= &ccstack[4];
unsigned char gst;


/* Declare keyboard buffer and scratch pad space */
string *pad;
string *tib;
int tibp = 0;

/* Declare Instruction Pointer and friends */
struct voc_entry **ip;		/* Instruction Pointer */
struct voc_entry **go;		/* Initial Instruction Pointer */
struct voc_entry *self;

/* Current vocabulary. This is where new definitions go. As well as which
 * vocabulary DISREGARD and FORGET should apply to. */
struct vocabulary *current;	

/* Define compiler state variables. */
int depth = 0;		/* nesting depth; called 'check' in STOIC. */
int state = FALSE;	/* compiler state. (TRUEth forces immedacy) */

int eol  = FALSE;	/* end of line? */
int eoc	 = FALSE;	/* end of command? */
function *clause;	/* clause handler */


int argc;
char **argv;
FILE *finput;
FILE *tinput;
FILE *toutput;

/* variables exposed to the language */
struct voc_entry hash_cnt;
struct voc_entry hash_ptr;
struct voc_entry input;

struct voc_entry *line;

struct voc_entry *args = NULL;
struct voc_entry warnings =	{ type: A_CONST, parm:
				{ type: T_FLT, v: { f: TRUE } } };
struct voc_entry radix	  =	{ type: A_CONST, parm:
				{ type: T_FLT, v: { f: 10 } } };
struct iopoint *io;

#ifdef REGEX
/* maximum number of sub matches for regexps */
struct voc_entry hash_match =	{ type: A_CONST, parm:
				{ type: T_FLT, v: { f: 10 } } };
long re_flags = 0;
#endif

finput = tinput	= stdin;
toutput	= stdout;

ginit(gstack, &gst);

sst	= data->sst;
sstmin  = data->sstmin;

data->sst = (void*)(&sst);

vst	= data->vst;
vstmin  = data->vstmin;

data->gstack = gstack;
data->gst    = &gst;

argc	= data->argc;
argv	= data->argv;

io = data->io;

data->ip  = &ip;
data->rst = &rst;

#ifdef THREADS
/* Set our TSD key to point to our resource pack */
pthread_setspecific(tsd, data);
#else
mdata = data;
#endif

pad = malloc( sizeof(string) + KSIZE + 1 );
tib = malloc( sizeof(string) + KSIZE + 1 );

/* build the compile stack */
line = malloc( sizeof( struct voc_entry ) + sizeof( void * ) * ( CSIZE + 4 ) );
cst	= (void*)&line->parm;
cstmin	= (void*)&line->parm;
line->name = NULL;
/* */

sig_init();

goto start;

#include "words.c"

start:

{
		
#include "builtins.h"

	
if ( data->main )
{
	
	{
		int i;
		cell *a;

		/* expose argument vector to the language */
		args = malloc( sizeof( struct voc_entry ) + sizeof( cell ) );
		
		args->type = A_ARRAY;
		args->parm.type	= T_FLT;
		args->parm.v.f	= argc - 1;
		(&args->parm)[1].type = T_PTR;
		a = (&args->parm)[1].v.p = malloc( argc * sizeof( cell ) );
		for ( i = 0; i < argc; i++ )
		{
			int l;
			string *s;
			
			l = strlen( argv[ i ] );
			
			a[i].type	= T_STR;
			a[i].v.s	= malloc( sizeof(string) + l + 1 );
			
			a[i].v.s->l = l;

			s = a[i].v.s;
			
			strcpy( &s->s, argv[i] );
		}
	}

	
	printk("S t o i c a l  %s :", version);
	printk("\tTo be of a STOIC nature; Born from STOICism (FORTHism?),");
	printk("\tStack Oriented Interactive Compiler Adapted to Linux,");
	printk("\tBastard brainchild of Jonathan Moore Liles.");
	printk("\t(read COPYING for important usage information)");

	setvbuf(stdout, (char *)NULL, _IONBF, 0); 
	
	voc = voc_create("stoical<");

	stoical = voc; 
	
	printk("Building STOICAL vocabulary...");

	i = 0;
	while ( builtins[i].code != NULL ) 
		voc_append(voc, &builtins[i++], 0);

	push(vst,voc);

	io->inln.type	= A_CONST;
	io->inln.parm.type	= T_PTR;
	io->inln.parm.v.p	= stt_lookup("ttygetln");
	io->outln.type	= A_CONST;
	io->outln.parm.type	= T_PTR;
	io->outln.parm.v.p	= stt_lookup("ttyputln");
	io->in.type	= A_CONST;
	io->in.parm.type	= T_PTR;
	io->in.parm.v.p	= stt_lookup("ttyget");
	io->out.type	= A_CONST;
	io->out.parm.type	= T_PTR;
	io->out.parm.v.p	= stt_lookup("ttyput");

	input.type = A_CONST;
	input.parm.type = T_FIL;
	input.parm.v.p  = malloc( sizeof( t_file ) );
	*((t_file*)input.parm.v.p) = (t_file){ tinput, fileno( tinput ) };
	
	if ( argc > 1 )
	{
		if ( ( finput = fopen( argv[1], "r" ) ) == NULL )
		{
			fprintf(stderr, 
			"stoical: could not open input file!\n");
			exit(1);
		}
		stt_lookup("rdline")->code = adr(frdline);
		stt_lookup("prompt")->code = adr(next);
	}
	else
		puts("Welcome to Stoical.");
	
	
	printk("Bootstrapping interactive compiler...");

	strcpy(&tib->s, "(:) 'def include" );

	tib->l = strlen(&tib->s);
	tibp = 0;
	eol = FALSE; eoc = TRUE;

	push(cst,stt_lookup("compile"));

	push(cst,stt_lookup("eoc"));
	push(cst,stt_lookup("(if)"));
	push(cst,(void*)7);
	
	push(cst,stt_lookup("check"));
	push(cst,stt_lookup("eqz"));
	push(cst,stt_lookup("(if)"));
	push(cst,(void*)3);

	push(cst,stt_lookup("execc"));
	push(cst,stt_lookup("clearcst"));
	
	push(cst,stt_lookup("prompt"));
	push(cst,stt_lookup("rdline"));
	push(cst,stt_lookup("(else)"));
	push(cst,(void*)-13);


	
	printk("Initializing USER vocabulary...");
	
	current = peek(vst);

	printk("Stage one complete...");
	
}
else
{
	push(cst,data->entry);
	push(cst,stt_lookup("exit"));
	current = peek(vst);

	/* make sure that our collectable items on our new stack are marked */
	stack_mark( sstmin, sst, gstack, &gst );
}

count = cst - cstmin;
cst = cstmin;

/* set the default clause handler */
clause = adr(_left_brace);

item.type = A_CMP;
item.name = "kernel";
item.code = adr(_colon);

entry =	voc_append( current, &item, (count * sizeof(void *)) + 1 );

if ( data->main )
	current = peek(vst);

memcpy( &entry->parm, cst, (count * sizeof(void *)) );

printk("Outer loop has been compiled (%i words). Executing...\n", count );

ip = &entry;
	
go = --ip;

exec(*adr(next));

printk("Outer loop (NEXT) has returned, something is broken!");

	}
}

int main (int argc, char **argv)
{
	struct thread_data *data;
	int n = 0;
	int tty;

	libroot = LIBROOT;

	data    = calloc( 1, sizeof( struct thread_data ) );
	data->sstmin = calloc( SSIZE + 8, sizeof( cell ) );
	data->vstmin = calloc( SSIZE + 8, sizeof( struct vocabulary * ) );
	data->sstmin += 4;
	data->vstmin += 4;
	
	if ( argc > 1 )
	{
		int i;
		for ( i = 1; i < argc; i++ )
		{
			if ( strcmp( argv[i], "-l" ) == 0 )
			{
				/* change library root */
				libroot = argv[i + 1];
				n += 2;
			}
			else if ( strcmp( argv[i], "-f" ) == 0 )
			{
				/* protect the following file name and stop
				 * parsing for options. */
				n += 1;
				break;
			}
			else if ( strncmp( argv[i], "-h", 2 ) == 0 )
			{
				printf(help, version);
				exit(0);
			}
		}
	}
	
	data->sst	= data->sstmin;
	data->vst	= data->vstmin;
	data->argc	= argc - n;
	data->argv	= &argv[ n ];
	data->main	= 1;
	data->io	= malloc( sizeof( struct iopoint ));
	
	srand((ub4)&data);	/* just a lame seed for rand */

#ifdef THREADS
	/* Initialize TSD key ) */
	pthread_key_create( &tsd, (void*)thread_destroy );
#endif
	if ( ( tty = isatty(0) ) )
	{
		term_save();
		term_cbreak(); 
	}

	st_start( data );
	
	if ( tty )
		term_restore();
	
	return(0);
}


