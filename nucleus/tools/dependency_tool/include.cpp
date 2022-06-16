/* $XConsortium: include.c,v 1.17 94/12/05 19:33:08 gildea Exp $ */
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

#include <string.h>

//#ifdef _MSC_VER
//  #undef strcasecmp
//  #undef strncasecmp
//  #define strcasecmp strcmpi
//  #define strncasecmp strnicmp
//#endif

extern inclist inc_list[MAXFILES], *inclistp;
extern char *includedirs[ ];
extern char *excludedirs[ ];
extern char *notdotdot[ ];
extern bool show_where_not;
extern bool warn_multiple;

// forward.
void remove_dotdot(char *path);
int isdot(register char  *p);
int isdotdot(register char *p);
int issymbolic(register char  *dir, register char  *component);
void included_by(register inclist *ip, register inclist *newfile);

inclist *inc_path(register char *file, register char *include, bool dot,
    bool &failure_okay)
{
  static char  path[ BUFSIZ ];
  register char    **pp, *p;
  register inclist  *ip;
  struct stat  st;
  bool  found = false;

//fprintf(stderr, "file=%s include=%s\n", file, include);
  const size_t inclen = strlen(include);
  if (inclen >= 4) {
    register char *cpp_point = include + inclen - 4;
    if (!strcasecmp(".cpp", cpp_point)) {
      // this is a CPP file include, which we skip.
//fprintf(stderr, "!found match at point: %s\n", cpp_point);
//hold      failure_okay = true;
//hold      return NULL;
    }
  }

////////fprintf(stderr, "incpath entry\n");

  /*
   * Check all previously found include files for a path that
   * has already been expanded.
   */
  for (ip = inc_list; ip->i_file; ip++)
    if ((strcmp(ip->i_incstring, include) == 0) && !ip->i_included_sym) {
      found = true;
      break;
    }

  /*
   * If the path was surrounded by "" or is an absolute path,
   * then check the exact path provided.
   */
  if (!found && (dot || *include == '/' || *include == '\\')) {
    if (stat(include, &st) == 0) {
      ip = newinclude(include, include);
      found = true;
    }
    else if (show_where_not)
      warning1("\tnot in %s\n", include);
  }

  /*
   * See if this include file is in the directory of the
   * file being compiled.
   */
  if (!found) {
    for (p=file+strlen(file); p>file; p--)
      if (*p == '/' || *p == '\\')
        break;
    if (p == file)
      strcpy(path, include);
    else {
      strncpy(path, file, (p-file) + 1);
      path[ (p-file) + 1 ] = '\0';
      strcpy(path + (p-file) + 1, include);
    }
    remove_dotdot(path);
    if (stat(path, &st) == 0) {
      ip = newinclude(path, include);
      found = true;
    }
    else if (show_where_not)
      warning1("\tnot in %s\n", path);
  }

  /*
   * Check the include directories specified. (standard include dir
   * should be at the end.)
   */
  if (!found)
    for (pp = includedirs; *pp; pp++) {
      sprintf(path, "%s/%s", *pp, include);
      remove_dotdot(path);
      if (stat(path, &st) == 0) {
        register char **pp2;
        bool exclude_it = false;
        for (pp2 = excludedirs; *pp2; pp2++) {
////////fprintf(stderr, "comparing %s with %s\n", *pp, *pp2);
          if (!strncmp(*pp, *pp2, strlen(*pp2))) {
            // this file is supposed to be excluded.
            exclude_it = true;
            break;
          }
        }
////////if (exclude_it) fprintf(stderr, "excluding path %s\n", path);
        if (exclude_it) {
          failure_okay = true;
          found = false;
        } else {
          ip = newinclude(path, include);
          found = true;
        }
        break;
      }
      else if (show_where_not)
        warning1("\tnot in %s\n", path);
    }

  if (!found)
    ip = NULL;
  return(ip);
}

/*
 * Occasionally, pathnames are created that look like .../x/../y
 * Any of the 'x/..' sequences within the name can be eliminated.
 * (but only if 'x' is not a symbolic link!!)
 */
void remove_dotdot(char *path)
{
  register char  *end, *from, *to, **cp;
  char    *components[ MAXFILES ], newpath[ BUFSIZ ];
  bool    component_copied;

  /*
   * slice path up into components.
   */
  to = newpath;
  if (*path == '/' || *path == '\\')
    *to++ = '/';
  *to = '\0';
  cp = components;
  for (from=end=path; *end; end++)
    if (*end == '/' || *end == '\\') {
      while (*end == '/' || *end == '\\')
        *end++ = '\0';
      if (*from)
        *cp++ = from;
      from = end;
    }
  *cp++ = from;
  *cp = NULL;

  /*
   * Recursively remove all 'x/..' component pairs.
   */
  cp = components;
  while(*cp) {
    if (!isdot(*cp) && !isdotdot(*cp) && isdotdot(*(cp+1))
        && !issymbolic(newpath, *cp))
    {
        char **fp = cp + 2;
        char **tp = cp;

        do 
      *tp++ = *fp; /* move all the pointers down */
        while (*fp++);
        if (cp != components)
      cp--;  /* go back and check for nested ".." */
    } else {
        cp++;
    }
  }
  /*
   * Concatenate the remaining path elements.
   */
  cp = components;
  component_copied = false;
  while(*cp) {
    if (component_copied)
      *to++ = '/';
    component_copied = true;
    for (from = *cp; *from; )
      *to++ = *from++;
    *to = '\0';
    cp++;
  }
  *to++ = '\0';

  /*
   * copy the reconstituted path back to our pointer.
   */
  strcpy(path, newpath);
}

int isdot(register char  *p)
{
  if(p && *p++ == '.' && *p++ == '\0')
    return(true);
  return(false);
}

int isdotdot(register char *p)
{
  if(p && *p++ == '.' && *p++ == '.' && *p++ == '\0')
    return(true);
  return(false);
}

int issymbolic(register char  *dir, register char  *component)
{
#ifdef S_IFLNK
  struct stat  st;
  char  buf[ BUFSIZ ], **pp;

  sprintf(buf, "%s%s%s", dir, *dir ? "/" : "", component);
  for (pp=notdotdot; *pp; pp++)
    if (strcmp(*pp, buf) == 0)
      return (true);
  if (lstat(buf, &st) == 0
  && (st.st_mode & S_IFMT) == S_IFLNK) {
    *pp++ = copy(buf);
    if (pp >= &notdotdot[ MAXDIRS ])
      fatalerr("out of .. dirs, increase MAXDIRS\n");
    return(true);
  }
#endif
  return(false);
}

/*
 * Add an include file to the list of those included by 'file'.
 */
inclist *newinclude(register char *newfile, register char *incstring)
{
  register inclist  *ip;

  /*
   * First, put this file on the global list of include files.
   */
  ip = inclistp++;
  if (inclistp == inc_list + MAXFILES - 1)
    fatalerr("out of space: increase MAXFILES\n");
  ip->i_file = copy(newfile);
  ip->i_included_sym = false;
  if (incstring == NULL)
    ip->i_incstring = ip->i_file;
  else
    ip->i_incstring = copy(incstring);

  return(ip);
}

void included_by(register inclist *ip, register inclist *newfile)
{
  register int i;

  if (ip == NULL)
    return;
  /*
   * Put this include file (newfile) on the list of files included
   * by 'file'.  If 'file' is NULL, then it is not an include
   * file itself (i.e. was probably mentioned on the command line).
   * If it is already on the list, don't stick it on again.
   */
  if (ip->i_list == NULL)
    ip->i_list = (inclist **)
      malloc(sizeof(inclist *) * ++ip->i_listlen);
  else {
    for (i=0; i<ip->i_listlen; i++)
      if (ip->i_list[ i ] == newfile) {
          i = int(strlen(newfile->i_file));
          if (!ip->i_included_sym &&
        !(i > 2 &&
          newfile->i_file[i-1] == 'c' &&
          newfile->i_file[i-2] == '.'))
          {
        /* only complain if ip has */
        /* no #include SYMBOL lines  */
        /* and is not a .c file */
        if (warn_multiple)
        {
          warning("%s includes %s more than once!\n",
            ip->i_file, newfile->i_file);
          warning1("Already have\n");
          for (i=0; i<ip->i_listlen; i++)
            warning1("\t%s\n", ip->i_list[i]->i_file);
        }
          }
          return;
      }
    ip->i_list = (inclist **) realloc(ip->i_list,
      sizeof(inclist *) * ++ip->i_listlen);
  }
  ip->i_list[ ip->i_listlen-1 ] = newfile;
}

void inc_clean()
{
  register inclist *ip;

  for (ip = inc_list; ip < inclistp; ip++) {
    ip->i_marked = false;
  }
}

