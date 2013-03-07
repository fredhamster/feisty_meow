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

//extern struct  inclist  inc_list[ MAXFILES ], *inclistp;
extern char  *objprefix;
extern char  *objsuffix;
extern int  width;
extern bool  printed;
extern bool  verbose;
extern bool  show_where_not;

void add_include(filepointer *filep, inclist  *file,
    inclist  *file_red, char  *include, bool dot, bool failOK)
{
  register struct inclist  *newfile;
  register struct filepointer  *content;

  /*
   * First decide what the pathname of this include file really is.
   */
  newfile = inc_path(file->i_file, include, dot, failOK);
  if (newfile == NULL) {
    if (failOK)
        return;
    if (file != file_red)
      warning("%s (reading %s, line %d): ",
        file_red->i_file, file->i_file, filep->f_line);
    else
      warning("%s, line %d: ", file->i_file, filep->f_line);
    warning1("cannot find include file \"%s\"\n", include);
fatalerr("cannot find include file \"%s\"\n", include);
    show_where_not = true;
    newfile = inc_path(file->i_file, include, dot, failOK);
    show_where_not = false;
  }

  if (newfile) {
    included_by(file, newfile);
    if (!newfile->i_searched) {
      newfile->i_searched = true;
      content = getfile(newfile->i_file);
      find_includes(content, newfile, file_red, 0, failOK);
      freefile(content);
    }
  }
}

void recursive_pr_include(register struct inclist  *head, register char  *file,
    register char  *base)
{
  register int  i;

  if (head->i_marked)
    return;
  head->i_marked = true;
  if (head->i_file != file) {
    bool rc_file = false;
    if ((strlen(file) >= 3) && !strcmp(file + strlen(file) - 3, ".rc"))
      rc_file = true;
    pr(head, file, base, rc_file);
  }
  for (i=0; i<head->i_listlen; i++)
    recursive_pr_include(head->i_list[ i ], file, base);
}

void pr(register struct inclist *ip, char *file, char *base, bool rc_file)
{
  static char  *lastfile;
  static int  current_len;
  register int  len, i;
  char  buf[ BUFSIZ ];

  printed = true;
  len = int(strlen(ip->i_file)+1);
  if (current_len + len > width || file != lastfile) {
    lastfile = file;
    char *temp_suff = objsuffix;
    if (rc_file) temp_suff = (char *)".res";
    sprintf(buf, "\n%s%s%s: %s ", objprefix, base, temp_suff, ip->i_file);
    len = current_len = int(strlen(buf));
  }
  else {
    strcpy(buf, ip->i_file);
//printf("ip->file=%s\n", ip->i_file);
    char tmp[2] = { ' ', '\0' };
//printf("tmp=%s\n", tmp);
    strcat(buf, tmp);
//printf("buf=%s\n", buf);
    current_len += len;
  }
  fwrite(buf, len, 1, stdout);

  // If verbose is set, then print out what this file includes.
  if (! verbose || ip->i_list == NULL || ip->i_notified)
    return;
  ip->i_notified = true;
  lastfile = NULL;
  printf("\n# %s includes:", ip->i_file);
  for (i=0; i<ip->i_listlen; i++)
    printf("\n#\t%s", ip->i_list[ i ]->i_incstring);
}

