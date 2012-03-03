
This is the "feisty meow (TM) concerns" codebase top folder.

Feisty Meow (TM) Concerns codebase comprises some but probably not all of several projects that
have come before.  These include the Twain.COM library scripts, the Yeti / YetiCode library
scripts, the HOOPLE 1 and HOOPLE 2 C++ Codebases, the Fred Huffhines opensource LSL scripts,
the CLAM build system, and a variety of other efforts.

Directories and files of interest here...

scripts/
    Stores the script files that we use a lot.  These are usually written in interpreted
    languages, such as bash and perl.  These were the core of the YetiCode scripts library.

nucleus/
    Source for the basic modules of the Feisty Meow (TM) codebase.  This includes generic
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

production/
    Machinery for getting releases out the door, including the generated files such as binaries
    and build system tools.

graphiq/
    GUI tools and components, somewhat busted or just absent right now.

webby/
    Source code specifically for web programming and building sites.  Mainly a few javascript
    files we keep around.

