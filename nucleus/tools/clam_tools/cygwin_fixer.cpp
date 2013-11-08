//need header here.

// make ms be quiet about strncat.
#define _CRT_SECURE_NO_WARNINGS

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// turns the cygwin name format into a usable windos filename.
char *translate_cygwin(char *fname)
{
  int oldlen = strlen(fname);
  if (!strncmp(fname, "/cygdrive/", 10) && (oldlen > 10) ) {
    // in the first case the filename has /cygdrive in it, right at the front.
    char *newprefix = (char *)malloc(oldlen);
    // build the drive letter first.
    newprefix[0] = fname[10];
    newprefix[1] = ':';
    newprefix[2] = '\0';
    // concatenate the filename without cygdrive in it.
    strncat(newprefix, fname + 11, oldlen - 11 + 1);  // one extra for null char.
    return newprefix;  // mem leak here; cannot be helped for quick fix using functional style.
  } else if ( (fname[0] == '-') && (oldlen > 12)
      && (!strncmp(fname + 2, "/cygdrive/", 10)) ) {
    // in the second case we are looking for command line options.  this code handles a parameter
    // that starts with a single dash and has a single flag character after that.
    char *newprefix = (char *)malloc(oldlen);
    newprefix[0] = fname[0];
    newprefix[1] = fname[1];
    newprefix[2] = fname[12];
    newprefix[3] = ':';
    newprefix[4] = '\0';
    // now concatenate the useful filename portion, offset by the flag found.
    strncat(newprefix, fname + 13, oldlen - 13 + 1);  // one extra for null char.
    return newprefix;
  } else {
    return fname;
  }
}

 
/*

function dossify_and_run_commands()
{

 
  declare -a darc_commands=()
  for i in "$@"; do
    // we only mess with the command line on windows.
    if [ "$OS" == "Windows_NT" ]; then
      if [[ "$i" =~ ^-[a-zA-z][/\"].* ]]; then
#echo matched on our pattern for parameters
        flag="${i:0:2}"
        filename="$(unix_to_dos_path ${i:2})"

#echo "first two chars are $flag"
#echo "last after that are $filename"
#combined="$flag$filename"
#echo combined is $combined
      
        darc_commands+=("$flag$filename")
      else 
        darc_commands+=($(unix_to_dos_path $i))
      fi
    else
      darc_commands+=("$i")
    fi
  done

}

*/

int main(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    printf("%s", translate_cygwin(argv[i]));
  }
  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
#endif // __BUILD_STATIC_APPLICATION__

