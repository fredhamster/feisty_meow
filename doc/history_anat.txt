
This is the "feisty meow® concerns" codebase top folder.

This software is licensed *per* *file* using either the GNU General Public
License (GNU GPL) version 3 *or* the Apache Software License version 2.0.
If a file does not have a license statement attached and it is part of the
feisty meow® codebase (rather than attributed to some other author), then the
default license is the GNU GPL.

For information on building the source code, see the file "building.txt".

####

Feisty Meow Concerns Ltd. is a small software shop that concentrates on
building high quality, portable, open source projects in a variety of
languages (primarily bash, perl, C++, Java, and python).  Here are some
highlights of our main products:

  Bookmark Processing Tools - can take a mozilla bookmark file or arbitrary
     web page and extract all the links out of it, building a csv database of
     web links.  Using that database, a variety of output formats are
     provided, including one that outputs a mozilla bookmark file again.
     The most useful feature is probably the marks checker that operates on
     our csv format and that locates all unreachable links in the file and
     separates them out.

  CROMP protocol - supports transmission of objects across the network and
     provides a platform independent method for RPC and other types of
     communication.

  Octopus design pattern - the underpinning of the CROMP protocol.  An octopus
     has an arbitrary number of tentacles (no, not just eight) which are each
     responsible for consuming a different type of object (or datum).

  Fast Templates and Portable Abstractions - the class libraries of Feisty Meow
     provide numerous different data structures and programming language
     abstractions (like threads and state machines).  There are also some
     fairly ancient templates (in use since late 80s) which in many cases
     perform faster than their STL analogues.

  CLAM System - Feisty Meow is the home site of the CLAM makefile system.  The
     CLAM system is a flexible and extensible method for building C++ and
     C# files using makefiles.

####

Historical note:
Feisty Meow® Concerns codebase comprises some but probably not all of several projects that
have come before.  These include the Twain.COM library scripts, the Yeti / YetiCode library
scripts, the HOOPLE 1 and HOOPLE 2 C++ Codebases, the Fred Huffhines open source LSL scripts,
the CLAM build system, and a variety of other efforts.

####

Directories and files of interest here...

scripts/
    Stores the script files that we use a lot.  These are usually written in interpreted
    languages, such as bash and perl.  These were the core of the YetiCode scripts library.

nucleus/
    Source for the basic modules of the Feisty Meow® codebase.  This includes generic
    data structures, useful utilities, and the like.  This hierarchy, plus octopi and graphiq,
    formed the major parts of hoople1 and hoople2.

huffware/
    The library of LSL scripts for osgrid and Second Life written by the avatar named Fred
    Huffhines.  Fred Huffhines is a regular contributor and a well-loved member of our
    development team.  He also happens to be a fictional sock-puppet for adventuring around
    the grids, but a lot of code has been written in his name.  We are going to release the
    full corpus of the Huffhines work over time.

octopi/
    Octopus design pattern and CROMP protocol reference implementations.  Assorted applications
    based on these.

database/
    Some files considered critical to the operations of Feisty Meow Concerns Ltd.  This includes
    the database of fortunes used by the Nechung Oracle Program.

doc/
    Assorted documentation files for Feisty Meow and a code-scanning documentation generator
    based on doxygen.

examples/
    Some files that show how to get work done with Feisty Meow or that show how to do certain
    tasks in different scripting / programming languages.

kona/
    Our burgeoning Java libraries.  Not much to see here yet, but there is some code piling
    up for these that we will try to release soon.

osgi/
    Some example bundles for Java OSGi.  These may eventually fill out with real material,
    but currently they are just tracking the investigation we did into creating bundles and
    running using them.

production/
    Machinery for getting releases out the door, including the generated files such as binaries
    and build system tools.

graphiq/
    GUI tools and components, somewhat busted or just absent right now.

webby/
    Source code specifically for web programming and building sites.  Mainly a few javascript
    files we keep around.

####

Notable Prerequisites for Using the Feisty Meow® codebase:

  Software required to compile under Linux:
    Gnu C++
    A few free source libraries (see "building.txt" for more information).

  Software required to compiler under MS-windows:
    The free Microsoft compiler should build the codebase but it is untested.
    The full version of MS Visual Studio 2010 (version 10) is supported.
      --but this has fallen into disrepair.
    Gnu C++ should compile the codebase but it is also untested recently.

####

More information is available at the official site http://feistymeow.org

####

