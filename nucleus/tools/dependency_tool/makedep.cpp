/* $XConsortium: main.c,v 1.84 94/11/30 16:10:44 kaleb Exp $ */
/* $XFree86: xc/config/makedepend/main.c,v 3.3 1995/01/28 15:41:03 dawes Exp $ */
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

#ifdef __WIN32__
  #pragma warning(disable : 4996)
#endif

#include "def.h"

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>

#ifdef hpux
#define sigvec sigvector
#endif /* hpux */

#ifdef X_POSIX_C_SOURCE
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <signal.h>
#undef _POSIX_C_SOURCE
#else
#if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <signal.h>
#else
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#endif
#endif

#if NeedVarargsPrototypes
#include <stdarg.h>
#endif

#ifdef MINIX
#define USE_CHMOD  1
#endif

#ifdef DEBUG
int  _debugmask;
#endif

char *ProgramName;

const char  *directives[] = {
  "if",
  "ifdef",
  "ifndef",
  "else",
  "endif",
  "define",
  "undef",
  "include",
  "line",
  "pragma",
  "error",
  "ident",
  "sccs",
  "elif",
  "eject",
  "import",
  NULL
};

#define MAKEDEPEND
#include "imakemdep.h"  /* from config sources */
#undef MAKEDEPEND

struct  inclist inc_list[ MAXFILES ], *inclistp = inc_list, maininclist;

char  *filelist[ MAXFILES ];
char  *includedirs[ MAXDIRS + 1 ];
char  *excludedirs[ MAXDIRS + 1 ];
char  *notdotdot[ MAXDIRS ];
char  *objprefix = (char *)"";
char  *objsuffix = (char *)".obj"; /* OBJSUFFIX;  */
char  *startat = (char *)"# DO NOT DELETE";
int  width = 78;
bool  append = false;
bool  printed = false;
bool  verbose = false;
bool  show_where_not = false;
bool warn_multiple = false;  /* Warn on multiple includes of same file */

static
#ifdef SIGNALRETURNSINT
int
#else
void
#endif
c_catch(int sig)
{
  fflush (stdout);
  fatalerr ((char *)"got signal %d\n", sig);
}

#if defined(USG) || (defined(i386) && defined(SYSV)) || defined(WIN32) || defined(__EMX__) || defined(__OS2__)
#define USGISH
#endif

#ifndef USGISH
#ifndef _POSIX_SOURCE
#define sigaction sigvec
#define sa_handler sv_handler
#define sa_mask sv_mask
#define sa_flags sv_flags
#endif
struct sigaction sig_act;
#endif /* USGISH */

// turns the cygwin name format into a usable windos filename.
char *translate_cygwin(char *fname)
{
  if (!strncmp(fname, "/cygdrive/", 10)) {
    int oldlen = strlen(fname);
    char *newprefix = (char *)malloc(oldlen); // at least long enough.
    newprefix[0] = fname[10];
    newprefix[1] = ':';
    newprefix[2] = '\0';
    strncat(newprefix, fname + 11, oldlen - 11 + 1);  // one extra for null char.
printf("translate cygwin: new filename is %s\n", newprefix);
    return newprefix;  // ignoring mem leak here.  cannot be helped for quicky fix.
  } else return fname;
}

/* fatty boombalatty, and wrong idea here.

// adds any subdirectories under dirname into the list of
// include directories, so we can get a whole hierarchies set of
// include files.
void add_subdirs(char *dirname, char ** &incp)
{
  directory dir(dirname);
  string_array subdirs = dir.directories();
  for (int i = 0; i < subdirs.length(); i++) {
    istring curr = istring(dirname) + "/" + subdirs[i];
    // add the current subdirectory.
    if (incp >= includedirs + MAXDIRS)
      fatalerr("Too many -I flags.\n");
    *incp++ = strdup(curr.s());
printf((istring("added ") + curr + "\n").s());
    add_subdirs(curr.s(), incp);
  }
}
*/

int main(int argc, char  **argv)
{
  register char  **fp = filelist;
  register char  **incp = includedirs;
  register char  **excp = excludedirs;
  register char  *p;
  register struct inclist  *ip;
  char  *makefile = NULL;
  struct filepointer  *filecontent;
  struct symtab *psymp = predefs;
  char *endmarker = NULL;
  char *defincdir = NULL;

  ProgramName = argv[0];

  while (psymp->s_name)
  {
      define2(psymp->s_name, psymp->s_value, &maininclist);
      psymp++;
  }
  if (argc == 2 && argv[1][0] == '@') {
      struct stat ast;
      int afd;
      char *args;
      char **nargv;
      int nargc;
      char quotechar = '\0';

      nargc = 1;
      if ((afd = open(translate_cygwin(argv[1]+1), O_RDONLY)) < 0)
    fatalerr("cannot open \"%s\"\n", translate_cygwin(argv[1]+1));
      fstat(afd, &ast);
      args = (char *)malloc(ast.st_size + 2);
      if ((ast.st_size = read(afd, args, ast.st_size)) < 0)
    fatalerr("failed to read %s\n", argv[1]+1);
      args[ast.st_size] = '\n';
      args[ast.st_size + 1] = '\0';
      close(afd);
      for (p = args; *p; p++) {
    if (quotechar) {
        if (quotechar == '\\' ||
      (*p == quotechar && p[-1] != '\\'))
      quotechar = '\0';
        continue;
    }
    switch (*p) {
    case '\\':
    case '"':
    case '\'':
        quotechar = *p;
        break;
    case ' ':
    case '\n':
        *p = '\0';
        if (p > args && p[-1])
      nargc++;
        break;
    }
      }
      if (p[-1])
    nargc++;
      nargv = (char **)malloc(nargc * sizeof(char *));
      nargv[0] = argv[0];
      argc = 1;
      for (p = args; argc < nargc; p += strlen(p) + 1)
    if (*p) nargv[argc++] = p;
      argv = nargv;
  }
  for(argc--, argv++; argc; argc--, argv++) {
        /* if looking for endmarker then check before parsing */
    if (endmarker && strcmp (endmarker, *argv) == 0) {
        endmarker = NULL;
        continue;
    }
    if (**argv != '-') {
      /* treat +thing as an option for C++ */
      if (endmarker && **argv == '+')
        continue;
      *fp++ = argv[0];
      continue;
    }
    switch(argv[0][1]) {
    case '-':
      endmarker = &argv[0][2];
      if (endmarker[0] == '\0') endmarker = (char *)"--";
      break;
    case 'D':
      if (argv[0][2] == '\0') {
        argv++;
        argc--;
      }
      for (p=argv[0] + 2; *p ; p++)
        if (*p == '=') {
          *p = ' ';
          break;
        }
      define(argv[0] + 2, &maininclist);
      break;
    case 'i':
      {
        char* delim;
        char* envinclude;
        char* prevdir;
        if (endmarker) break;
        if (argv[0][2] == '\0') {
          argv++;
          argc--;
          envinclude = getenv(argv[0]);
        } else  envinclude = getenv(argv[0]+2);
        if (!envinclude)  break;
        prevdir = envinclude;
        delim = (char*)strchr(envinclude, ';');
        while(delim)
        {
          if (incp >= includedirs + MAXDIRS)  fatalerr("Too many Include directories.\n");
          *delim = '\0';
          delim++;
          *incp++ = prevdir;
          prevdir = delim;
          delim = (char*)strchr(delim, ';');
        }
      }
      break;
    case 'X':
//fprintf(stderr, "adding Xclude %s\n", argv[0]+2);
      // add a directory to the exclusions list.
      if (excp >= excludedirs + MAXDIRS)
          fatalerr("Too many -X flags.\n");
      *excp++ = argv[0]+2;
      if (**(excp-1) == '\0') {
        // fix the prior entry, but don't incremement yet; we'll do that
        // on the include list instead.
        *(excp-1) = *(argv + 1);
      }
      if (incp >= includedirs + MAXDIRS)
          fatalerr("Too many -I flags via -X.\n");
      *incp++ = argv[0]+2;
      if (**(incp-1) == '\0') {
        *(incp-1) = *(++argv);
        argc--;
      }
      break;
    case 'I':
      if (incp >= includedirs + MAXDIRS)
          fatalerr("Too many -I flags.\n");
      *incp++ = argv[0]+2;
      if (**(incp-1) == '\0') {
        *(incp-1) = *(++argv);
        argc--;
      }
///      add_subdirs(*(incp-1), incp);
      break;
    case 'Y':
      defincdir = argv[0]+2;
      break;
    /* do not use if endmarker processing */
    case 'a':
      if (endmarker) break;
      append = true;
      break;
    case 'w':
      if (endmarker) break;
      if (argv[0][2] == '\0') {
        argv++;
        argc--;
        width = atoi(argv[0]);
      } else
        width = atoi(argv[0]+2);
      break;
    case 'o':
      if (endmarker) break;
      if (argv[0][2] == '\0') {
        argv++;
        argc--;
        objsuffix = argv[0];
      } else
        objsuffix = argv[0]+2;
      break;
    case 'p':
      if (endmarker) break;
      if (argv[0][2] == '\0') {
        argv++;
        argc--;
        objprefix = argv[0];
      } else
        objprefix = argv[0]+2;
        objprefix = translate_cygwin(objprefix);
      break;
    case 'v':
      if (endmarker) break;
      verbose = true;
#ifdef DEBUG
      if (argv[0][2])
        _debugmask = atoi(argv[0]+2);
#endif
      break;
    case 's':
      if (endmarker) break;
      startat = argv[0]+2;
      if (*startat == '\0') {
        startat = *(++argv);
        argc--;
      }
      if (*startat != '#')
        fatalerr("-s flag's value should start %s\n",
          "with '#'.");
      break;
    case 'f':
      if (endmarker) break;
      makefile = argv[0]+2;
      if (*makefile == '\0') {
        makefile = *(++argv);
        argc--;
      }
      break;
    case 'm':
      warn_multiple = true;
      break;
      
    /* Ignore -O, -g so we can just pass ${CFLAGS} to
       makedepend
     */
    case 'O':
    case 'g':
      break;
    default:
      if (endmarker) break;
  /*    fatalerr("unknown opt = %s\n", argv[0]); */
      warning("ignoring option %s\n", argv[0]);
      break;
    }
  }

  if (!defincdir) {
#ifdef PREINCDIR
      if (incp >= includedirs + MAXDIRS)
    fatalerr("Too many -I flags.\n");
      *incp++ = PREINCDIR;
#endif
#ifdef INCLUDEDIR
      if (incp >= includedirs + MAXDIRS)
    fatalerr("Too many -I flags.\n");
      *incp++ = INCLUDEDIR;
#endif
#ifdef POSTINCDIR
      if (incp >= includedirs + MAXDIRS)
    fatalerr("Too many -I flags.\n");
      *incp++ = POSTINCDIR;
#endif
  } else if (*defincdir) {
      if (incp >= includedirs + MAXDIRS)
    fatalerr("Too many -I flags.\n");
      *incp++ = defincdir;
  }

  redirect(startat, makefile);

  /*
   * c_catch signals.
   */
#ifdef USGISH
/*  should really reset SIGINT to SIG_IGN if it was.  */
#ifdef SIGHUP
  signal (SIGHUP, c_catch);
#endif
  signal (SIGINT, c_catch);
#ifdef SIGQUIT
  signal (SIGQUIT, c_catch);
#endif
  signal (SIGILL, c_catch);
#ifdef SIGBUS
  signal (SIGBUS, c_catch);
#endif
  signal (SIGSEGV, c_catch);
#ifdef SIGSYS
  signal (SIGSYS, c_catch);
#endif
#else
  sig_act.sa_handler = c_catch;
#ifdef _POSIX_SOURCE
  sigemptyset(&sig_act.sa_mask);
  sigaddset(&sig_act.sa_mask, SIGINT);
  sigaddset(&sig_act.sa_mask, SIGQUIT);
#ifdef SIGBUS
  sigaddset(&sig_act.sa_mask, SIGBUS);
#endif
  sigaddset(&sig_act.sa_mask, SIGILL);
  sigaddset(&sig_act.sa_mask, SIGSEGV);
  sigaddset(&sig_act.sa_mask, SIGHUP);
  sigaddset(&sig_act.sa_mask, SIGPIPE);
#ifdef SIGSYS
  sigaddset(&sig_act.sa_mask, SIGSYS);
#endif
#else
  sig_act.sa_mask = ((1<<(SIGINT -1))
         |(1<<(SIGQUIT-1))
#ifdef SIGBUS
         |(1<<(SIGBUS-1))
#endif
         |(1<<(SIGILL-1))
         |(1<<(SIGSEGV-1))
         |(1<<(SIGHUP-1))
         |(1<<(SIGPIPE-1))
#ifdef SIGSYS
         |(1<<(SIGSYS-1))
#endif
         );
#endif /* _POSIX_SOURCE */
  sig_act.sa_flags = 0;
  sigaction(SIGHUP, &sig_act, (struct sigaction *)0);
  sigaction(SIGINT, &sig_act, (struct sigaction *)0);
  sigaction(SIGQUIT, &sig_act, (struct sigaction *)0);
  sigaction(SIGILL, &sig_act, (struct sigaction *)0);
#ifdef SIGBUS
  sigaction(SIGBUS, &sig_act, (struct sigaction *)0);
#endif
  sigaction(SIGSEGV, &sig_act, (struct sigaction *)0);
#ifdef SIGSYS
  sigaction(SIGSYS, &sig_act, (struct sigaction *)0);
#endif
#endif /* USGISH */

  /*
   * now peruse through the list of files.
   */
  for(fp=filelist; *fp; fp++) {
    filecontent = getfile(*fp);
    ip = newinclude(*fp, (char *)NULL);

    find_includes(filecontent, ip, ip, 0, false);
    freefile(filecontent);
    recursive_pr_include(ip, ip->i_file, base_name(*fp));
    inc_clean();
  }
  if (printed)
    printf("\n");

  return 0;
}

struct filepointer *getfile(char  *file)
{
  register int  fd;
  struct filepointer  *content;
  struct stat  st;

  content = (struct filepointer *)malloc(sizeof(struct filepointer));
  content->f_name = strdup(translate_cygwin(file));
  if ((fd = open(file, O_RDONLY)) < 0) {
    warning("cannot open \"%s\"\n", translate_cygwin(file));
    content->f_p = content->f_base = content->f_end = (char *)malloc(1);
    *content->f_p = '\0';
    return(content);
  }
  fstat(fd, &st);
  content->f_base = (char *)malloc(st.st_size+1);
  if (content->f_base == NULL)
    fatalerr("cannot allocate mem\n");
  if ((st.st_size = read(fd, content->f_base, st.st_size)) < 0)
    fatalerr("failed to read %s\n", translate_cygwin(file));
  close(fd);
  content->f_len = st.st_size+1;
  content->f_p = content->f_base;
  content->f_end = content->f_base + st.st_size;
  *content->f_end = '\0';
  content->f_line = 0;
  return(content);
}

void freefile(struct filepointer  *fp)
{
  free(fp->f_name);
  free(fp->f_base);
  free(fp);
}

char *copy(register char  *str)
{
  register char  *p = (char *)malloc(strlen(str) + 1);

  strcpy(p, str);
  return(p);
}

int match(register const char *str, register const char **list)
{
  register int  i;

  for (i=0; *list; i++, list++)
    if (strcmp(str, *list) == 0)
      return i;
  return -1;
}

/*
 * Get the next line.  We only return lines beginning with '#' since that
 * is all this program is ever interested in.
 */
char *getline(register struct filepointer  *filep)
{
  register char  *p,  /* walking pointer */
      *eof,  /* end of file pointer */
      *bol;  /* beginning of line pointer */
  register  int lineno;  /* line number */

  eof = filep->f_end;
////  if (p >= eof) return NULL;
  lineno = filep->f_line;
  bol = filep->f_p;

  // p is always pointing at the "beginning of a line" when we start this loop.
  // this means that we must start considering the stuff on that line as
  // being a useful thing to look at.
  for (p = filep->f_p; p < eof; p++) {
if (bol > p) fatalerr("somehow bol got ahead of p.");
    if (*p == '/' && *(p+1) == '*') {
      /* consume C-style comments */
      *p++ = ' '; *p++ = ' ';  // skip the two we've already seen.
      while (p < eof) {
        if (*p == '*' && *(p+1) == '/') {
          *p++ = ' '; *p = ' ';
            // skip one of these last two, let the loop skip the next.
          break;
        } else if (*p == '\n') lineno++;
        p++;  // skip the current character.
      }
      continue;
    } else if (*p == '/' && *(p+1) == '/') {
      /* consume C++-style comments */
      *p++ = ' '; *p++ = ' ';  // skip the comment characters.
      while (p < eof && (*p != '\n') ) *p++ = ' ';
        // we scan until we get to the end of line.
///no count, since we'll see it again.      lineno++;
      p--;  // skip back to just before \n.
      continue;  // let the loop skip past the eol that we saw.
    } else if (*p == '\\') {
      // handle an escape character.
      if (*(p+1) == '\n') {
        // we modify the stream so we consider the line correctly.
        *p = ' ';
        *(p+1) = ' ';
        lineno++;
      }
    } else if (*p == '\n') {
      // we've finally reached the end of the line.
      lineno++;
      *p = '\0';  // set the end of line to be a end of string now.
      if (bol < p) {
        // it's not at the same position as the end of line, so it's worth
        // considering.
        while ( (bol < p) && ((*bol == ' ') || (*bol == '\t')) ) bol++;
////fprintf(stderr, "%s: %s\n", filep->f_name, bol);
////fflush(stderr);
        if (*bol == '#') {
          register char *cp;
          /* punt lines with just # (yacc generated) */
          for (cp = bol+1; *cp && (*cp == ' ' || *cp == '\t'); cp++) {}
          if (*cp) { p++; goto done; }
        }
      }
      // this is a failure now.  we reset the beginning of line.
      bol = p+1;
    }
  }
  if (*bol != '#') bol = NULL;
done:
  filep->f_p = p;
  filep->f_line = lineno;
  return bol;
}

/*
 * Strip the file name down to what we want to see in the Makefile.
 * It will have objprefix and objsuffix around it.
 */
char *base_name(register char  *file)
{
  register char  *p;

  file = copy(file);
  for(p=file+strlen(file); p>file && *p != '.'; p--) ;

  if (*p == '.')
    *p = '\0';
  return(file);
}

#if defined(USG) && !defined(CRAY) && !defined(SVR4) && !defined(__EMX__)
int rename(char *from, char *to)
{
    (void) unlink (to);
    if (link (from, to) == 0) {
  unlink (from);
  return 0;
    } else {
  return -1;
    }
}
#endif /* USGISH */

void redirect(char  *line, char  *makefile)
{
  struct stat  st;
  FILE  *fdin, *fdout;
  char  backup[ BUFSIZ ],
    buf[ BUFSIZ ];
  bool  found = false;
  int  len;

  /*
   * if makefile is "-" then let it pour onto stdout.
   */
  if (makefile && *makefile == '-' && *(makefile+1) == '\0')
    return;

  /*
   * use a default makefile is not specified.
   */
  if (!makefile) {
    if (stat("Makefile", &st) == 0)
      makefile = (char *)"Makefile";
    else if (stat("makefile", &st) == 0)
      makefile = (char *)"makefile";
    else
      fatalerr("[mM]akefile is not present\n");
  }
  else
      stat(makefile, &st);
  if ((fdin = fopen(translate_cygwin(makefile), "r")) == NULL)
    fatalerr("cannot open \"%s\"\n", translate_cygwin(makefile));
  sprintf(backup, "%s.bak", makefile);
  unlink(backup);
#if defined(WIN32) || defined(__EMX__) || defined(__OS2__)
  fclose(fdin);
#endif
  if (rename(translate_cygwin(makefile), translate_cygwin(backup)) < 0)
    fatalerr("cannot rename %s to %s\n", translate_cygwin(makefile), translate_cygwin(backup));
#if defined(WIN32) || defined(__EMX__) || defined(__OS2__)
  if ((fdin = fopen(translate_cygwin(backup), "r")) == NULL)
    fatalerr("cannot open \"%s\"\n", translate_cygwin(backup));
#endif
  if ((fdout = freopen(translate_cygwin(makefile), "w", stdout)) == NULL)
    fatalerr("cannot open \"%s\"\n", translate_cygwin(backup));
  len = int(strlen(line));
  while (!found && fgets(buf, BUFSIZ, fdin)) {
    if (*buf == '#' && strncmp(line, buf, len) == 0)
      found = true;
    fputs(buf, fdout);
  }
  if (!found) {
    if (verbose)
    warning("Adding new delimiting line \"%s\" and dependencies...\n",
      line);
    puts(line); /* same as fputs(fdout); but with newline */
  } else if (append) {
      while (fgets(buf, BUFSIZ, fdin)) {
    fputs(buf, fdout);
      }
  }
  fflush(fdout);
#if defined(USGISH) || defined(_SEQUENT_) || defined(USE_CHMOD)
  chmod(makefile, st.st_mode);
#else
        fchmod(fileno(fdout), st.st_mode);
#endif /* USGISH */
}

#if NeedVarargsPrototypes
void fatalerr(const char *msg, ...)
#else
/*VARARGS*/
void fatalerr(char *msg,x1,x2,x3,x4,x5,x6,x7,x8,x9)
#endif
{
#if NeedVarargsPrototypes
  va_list args;
#endif
  fprintf(stderr, "%s: error:  ", ProgramName);
#if NeedVarargsPrototypes
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
#else
  fprintf(stderr, msg,x1,x2,x3,x4,x5,x6,x7,x8,x9);
#endif
  exit (1);
}

#if NeedVarargsPrototypes
void warning(const char *msg, ...)
#else
/*VARARGS0*/
void warning(const char *msg,x1,x2,x3,x4,x5,x6,x7,x8,x9)
#endif
{
#if NeedVarargsPrototypes
  va_list args;
#endif
  fprintf(stderr, "%s: warning:  ", ProgramName);
#if NeedVarargsPrototypes
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
#else
  fprintf(stderr, msg,x1,x2,x3,x4,x5,x6,x7,x8,x9);
#endif
}

#if NeedVarargsPrototypes
void warning1(const char *msg, ...)
#else
/*VARARGS0*/
void warning1(const char *msg,x1,x2,x3,x4,x5,x6,x7,x8,x9)
#endif
{
#if NeedVarargsPrototypes
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
#else
  fprintf(stderr, msg,x1,x2,x3,x4,x5,x6,x7,x8,x9);
#endif
}

