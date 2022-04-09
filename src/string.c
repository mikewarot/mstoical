
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

/* String manipulation routines. */


/* duplicate string. malloc is used to allocate memory for the new string. */
string *stringdup ( string *s )
{
	long l;
	string *n;
	
	l = sizeof( string ) + s->l + 1;
	
	n = malloc( l );
	
	memcpy( n, s, l );

	return n;
}

/*
 * Interpolate escape codes found in string.
 * Note that this funciton modifies its argument.
 * mode specifies wether interpolate should consider \(character) to
 * mean (character) or \(character).
 */
void
interpolate( string *str, int mode )
{
	int len, i, j;
	int c;
	char *s;

	len = strlen(  ( s = &str->s ) );

	j = 0;
	for (i = 1; i < len; i++ )
	{
		switch ( c = s[i] )
		{
			case '\\':
			{
				if ( ++i < len )
				{
					switch ( c = s[i] )
					{
						case '0' ... '9':
						{
							s[j++] = '\\';
							s[j++] = c;
							break;
						}
						case 'r':
						{
							s[j++] = '\r';
							break;
						}
						case 'n':
						{
							s[j++] = '\n';
							break;
						}
						case 't':
						{
							s[j++] = '\t';
							break;
						}
						case 'e':
						{
							s[j++] = '\033';
							break;
						}
						case 'b':
						{
							s[j++] = '\b';
							break;
						}
						case 'a':
						{
							s[j++] = '\a';
							break;
						}
						case '\\':
						{
							s[j++] = '\\';
							break;
						}
						case '"':
						{
							s[j++] = '"';
							break;
						}
						case 'x':
						{
							char t[3];
							if ( i + 2 < len )
							{
								t[0] = s[++i];
								t[1] = s[++i];
								t[2] = '\0';
								
						s[j++] =
						(int)strtol( t, NULL, 16 );
						
							}
							break;
						}
						default:
						{
							if ( mode )
							{
								s[j++] = '\\';
								s[j++] = c;
							}
							break;
						}
						
					}
					break;
				}
			}
			default:
			{
				s[j++] = s[i];
				break;
			}
		}
	}

	s[j] = '\0';
	str->l = j;
}

/* get next 'word' from the string 'tib'. placing it in the string 'pad',
 * and returning -1 if the end of line was reached */
int st_word( string *tib, int *tibp, string *pad )
{
	char delim[] = " \t"; 
	char c; 
	int len = 0;
	int eol = FALSE;
	for (;;)
	{
		c = (&tib->s)[(*tibp)++];
		if ( c != ' ' && c != '\t' )
			break;
	}
	(*tibp)--;
	for (;;) {
		c = (&tib->s)[(*tibp)++]; 
		(&pad->s)[len++] = c;
		if ( c == '\0' )
		{
			eol = TRUE;
			break;
		}
		if ( c == delim[0] || c == delim[1] )
		{
			if ( (&tib->s)[*tibp - 2] != '\\' ) 
			{ 
				if ( (&tib->s)[*tibp] == '\0' )
					eol = TRUE;
				break;
			} 
			else 
			{ 
				(&pad->s)[len - 2] = c; 
				len--; 
			} 
		}
		else if ( c == '"' )
		{
			delim[0] = '"';
			delim[1] = '"';
		}
		else if ( c == '|' )
		{
			delim[0] = '|';
			delim[1] = '|';
		}
	}
	pad->l = --len;
	(&pad->s)[len] = '\0';
	return eol;
}
