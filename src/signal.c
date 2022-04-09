
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

/* This file contains functions specific to signal handling */

#include "kernel.h"
#include <signal.h>

/* TODO: also catch sigfpe */

void sig_segv_h ( int i, siginfo_t *info, void *p )
{
	static int caught;
	
	if ( ! caught )
	{
		caught = 1;
	
		printk("caught SIGSEGV");

		fprintf(stderr, "\nSegmentation fault in:\n" );
	
		diagnostic();

		caught = 0;
	}
		
	exit(1);
}

/* Setup the default signal handlers.
 */
void sig_init ( void )
{
	struct sigaction act;

	act.sa_sigaction = sig_segv_h;
	act.sa_flags	 = SA_SIGINFO;
	
	sigaction( SIGSEGV, &act, NULL ); 
}


