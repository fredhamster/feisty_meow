/* $XConsortium: def.h,v 1.25 94/04/17 20:10:33 gildea Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#include <stdio.h>
#include "Xosdefs.h"
#ifdef WIN32
#endif
#ifdef __OS2__
#define __STDC__ 1
#include "Xos2defs.h"
#endif
#include "Xfuncproto.h"
#include <ctype.h>
#ifndef X_NOT_POSIX
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAXDEFINES 512
#define MAXFILES   4000
#define MAXDIRS    640
#define SYMTABINC  10  /* must be > 1 for define() to work right */

/* the following must match the directives table in main.c */
#define IF               0
#define IFDEF            1
#define IFNDEF           2
#define ELSE             3
#define ENDIF            4
#define DEFINE           5
#define UNDEF            6
#define INCLUDE          7
#define LINE             8
#define PRAGMA           9
#define ERROR           10
#define IDENT           11
#define SCCS            12
#define ELIF            13
#define EJECT           14
#define IMPORT          15

#define IFFALSE         16     /* pseudo value --- never matched */
#define ELIFFALSE       17     /* pseudo value --- never matched */
#define INCLUDEDOT      18     /* pseudo value --- never matched */
#define IFGUESSFALSE    19     /* pseudo value --- never matched */
#define ELIFGUESSFALSE  20     /* pseudo value --- never matched */

///#define DEBUG
  // uncomment this for debugging version.

#ifdef DEBUG
extern int  _debugmask;
/*
 * debug levels are:
 * 
 *     0  show ifn*(def)*,endif
 *     1  trace defined/!defined
 *     2  show #include
 *     3  show #include SYMBOL
 *     4-6  unused
 */
#define debug(level,arg) { if (_debugmask & (1 << level)) warning arg; }
#else
#define  debug(level,arg) {}
#endif /* DEBUG */

struct symtab {
  char  *s_name;
  char  *s_value;
};

struct inclist {
  char    *i_incstring;  /* string from #include line */
  char    *i_file;  /* path name of the include file */
  inclist          **i_list;  /* list of files it itself includes */
  int    i_listlen;  /* length of i_list */
  symtab  *i_defs;  /* symbol table for this file */
  int    i_ndefs;  /* current # defines */
  int    i_deflen;  /* amount of space in table */
  bool    i_defchecked;  /* whether defines have been checked */
  bool    i_notified;  /* whether we have revealed includes */
  bool    i_marked;  /* whether it's in the makefile */
  bool    i_searched;  /* whether we have read this */
  bool         i_included_sym; /* whether #include SYMBOL was found */
          /* Can't use i_list if true */
};

struct filepointer {
  char  *f_p;
  char  *f_base;
  char  *f_end;
  long  f_len;
  long  f_line;
  char  *f_name;
};

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#if defined(macII) && !defined(__STDC__)  /* stdlib.h fails to define these */
char *malloc(), *realloc();
#endif /* macII */
#else
char      *malloc();
char      *realloc();
#endif

/*
char      *getline();
symtab    *slookup();
symtab    *isdefined();
symtab    *fdefined();
filepointer  *getfile();
inclist *newinclude(register char *, register char *);
inclist    *inc_path();
*/

// cppsetup.cpp:
int cppsetup(register char *line, register filepointer *filep,
    register inclist *inc);

// include.cpp
inclist *newinclude(register char *newfile, register char *incstring);
void inc_clean();
inclist *inc_path(register char *file, register char *include, bool dot,
    bool &failure_okay);
void included_by(register inclist *ip, register inclist *newfile);

// main.cpp:
char *base_name(register char  *file);
char *copy(register char *str);
filepointer *getfile(char  *file);
void freefile(filepointer  *fp);
char *getline(register filepointer  *filep);
int match(register const char *str, register const char **list);
void redirect(char  *line, char  *makefile);
#if NeedVarargsPrototypes
  void fatalerr(const char *, ...);
  void warning(const char *, ...);
  void warning1(const char *, ...);
#endif

// parse.cpp:
void define(char  *def, inclist  *file);
void define2(char  *name, char  *val, inclist  *file);
int deftype(register char  *line, register filepointer *filep,
    register inclist *file_red, register inclist *file,
    int parse_it);
symtab *fdefined(register char *symbol, inclist *file, inclist **srcfile);
int find_includes(filepointer *filep, inclist *file,
    inclist *file_red, int recursion, bool failOK);
int gobble(register filepointer *filep, inclist *file,
    inclist *file_red);
symtab *isdefined(register char *symbol, inclist *file,
    inclist  **srcfile);
symtab *slookup(register char  *symbol, register inclist *file);
void undefine(char  *symbol, register inclist *file);
int zero_value(register char  *exp, register filepointer *filep,
    register inclist *file_red);

// pr.cpp:
void add_include(filepointer *filep, inclist  *file,
    inclist  *file_red, char  *include, bool dot, bool failOK);
void pr(register inclist *ip, char *file, char *base, bool rc_file);
void recursive_pr_include(register inclist *head, register char *file,
    register char *base);

