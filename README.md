# Preface (2022)

This is an experiment in software archeology, someone on some orange
forum nerdsniped me by asking what it would take to compile some
ancient Forth-like language written in C that hadn't been run in
twenty years on a modern machine.

Turns out, is is not a good idea to store a pthread_t in a float
variable...

I make not guarantees about this code except that it compile on my
machine. I don't think that the interpreter ever frees memory
allocated for strings, and it segfaults when one of the directories
the call in `make doc` wants to write to is write protected.  So there
is still much fun to be had if you want to use this.  Share and enjoy.

# README (2002)

## S  T  O  I  C  A  L					http://stoical.sf.net

(c) 2002 Jonathan Moore Liles

### Structure

STOICAL's implementation is based largely on my own feelings about efficiency.
I have written the source code in C so that it can interface with the libraries
that are at the heart of the Unix system. C, though, was not designed to make
writing threaded compilers easy. So, needless to say, the code isn't as pretty
now as it was when I started. STOICAL is still a very new language, despite its
roots. Surprisingly however, It possesses many features rarely found in even the
most modern languages, and even more surprisingly, it provides them in
combination. These include: POSIX Threading, POSIX Regular Expressions,
associative arrays, dynamic memory allocation, socket based networking,
floating point arithmetic, passive garbage collection, passive type-checking,
and more. A full description of these features can be found in the FEATURES
distribution file.

Of all the Forths that I have benchmarked, STOICAL has been the fastest by far.
However, for the sake of reasonable comparison these were all Forths written in
C. 

The system can run code written for STOIC, provided that it includes no CODE<
definitions, and does a minimum amount of pointer arithmetic. Practically, this
isn't a problem due to the fact that virtually no software has ever been
written for STOIC, and conceptually it is part of STOICAL's progressive outlook
on the future.

STOICAL has a fascinating history, I suggest you read all of the included
documentation.

### Compilation and Installation

STOICAL was developed with the help of Debian GNU/Linux. However, any other
modern Linux distribution should do fine, provided that you have the
appropriate development files and libraries installed. If you have any trouble
in this regard, then please utilize the forums and mailing lists, which may be
accessed through the web site at http://stoical.sf.net.

Use ./configure --help to see a list of available build options.  In a perfect
world all that you should be required to do is:

	./configure
	make
	su -c 'make install'
	(superuser password)

There are also some files that, unfortunately, must be installed by hand. These
are the Vim syntax highlighting and file type detection files located under
the vim/ directory of this distribution. You are encouraged to install these
in your ~/.vim directory, as they will greatly enhance your experience with
STOICAL. Emacs users have an editor that is a language in and of itself, and
therefore have no need for STOICAL; Or friends, for that matter ;-)

### Platforms

STOICAL has been built and tested on the following platforms:

Sparc - Linux 2.2 Debian 2.2
Alpha - Linux 2.2 Debain 2.2
PPC RS/6000 Linux 2.4 Debian 2.2
Intel x86/SMP Linux 2.4 Debian 2.3
Intel x86 - FreeBSD 4.5-STABLE (no threads)

If you would like to build STOICAL for a platform not listed here, please send
an email explaining your situation to the following address:

"stoical-support" <stoical-support@lists.sf.net>

### Documentation

During the build process, STOICAL will generate documentation from its C and
STOICAL sources. This, and other, more general documentation can be found under
the doc/ subdirectory.

### Examples

The examples/ tree contains sample source code that you should be able to run
immediately after building STOICAL (assuming that your build options were
compatible with the needs of the examples)

Of course there will be more to come. As always, documentation is a work in
progress.

