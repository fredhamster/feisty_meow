#!/bin/bash

# looks for bad usages in source files.

# this script is by no means finished.  it also may not be very accurate yet.

# look for files that provide a copyright year, but are still tied to a
# non-variable end year.  we are replacing all end years with $now, since
# we intend the copyright to last as long as legally valid.
echo "---> Looking for incorrect year attributions..."
find $* -type f -exec grep -li "[0-9][0-9]-[12][90]" {} ';' | grep -v ".*\.svn.*"

# look for includes that are more than one level but which are using double
# quotes instead of angle brackets.
echo "---> Looking for inappropriate double quote usage in #includes now..."
find $* -type f -exec grep -li "#include.*\"[a-zA-Z0-9._]*\/[a-zA-Z0-9._]*\"" {} ';' | grep -v ".*\.svn.*"

# look for includes that are using the dreaded dos style backslash for
# directory separators.  those are non-portable, but forward slash works
# everywhere.
echo "---> Looking for backslash usage in #includes now..."
find $* -type f -exec grep -li "#include[^\"]*\"[^\"]*\\\\[^\"]*\"" {} ';' | grep -v ".*\.svn.*"
find $* -type f -exec grep -li "#include[^<]*<[^>]*\\\\[^>]*>" {} ';' | grep -v ".*\.svn.*"

# check for if statements with dangerous assignments involved.
echo "---> Looking for dangerous assignments in source now..."
find $* -type f -exec grep -li "if[^A-Za-z0-9]*(.*[a-z A-Z()]=[-+a-z A-Z()!].*)" {} ';' | grep -v ".*\.svn.*"
# not quite right; this needs to be done from a parser so we can catch the
# best matching occurrences.

