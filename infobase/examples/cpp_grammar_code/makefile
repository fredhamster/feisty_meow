#
#	Title:		Make file for Cxx Grammar tester.
#
#	Author:		E.D.Willink
#
#	SCCS:		%W% %G%
#
#	Description:
#			This makefile wrapper redirects all activity to makefile.unix after first ensuring that
#			any .INIT provided by ../import.make has executed to load required source files.
#
#	Targets:	
#			executable, normal
#					builds $(ARCH)o/grammar
#			sun
#					builds $(ARCH)o/grammar using sun compiler, yacc and lex
#			gnu
#					builds $(ARCH)o/grammar using gnu compiler, bison and flex
#			clean
#					eliminates $(ARCH)o* intermediates
#			realclean
#					eliminates $(ARCH)o* intermediates and executables
#			source_kit
#					generates the distribution kits
#
#	Switch settings are appropriate for Sun C++ 4.2.
#	Commented settings indicate what might be appropriate for gcc once it supports templates plausibly.
#
#	Latest Modification:
# EDW		Date:	14-Jun-2001	    Original
#END

.SUFFIXES:

SRCDIR = sources
ARCH = sun4
OBJ_DIR = $(ARCH)o
OBJ_DIR_PI = $(ARCH)o_pi
OBJ_DIR_G = $(ARCH)o_g
OBJ_DIR_PI_G = $(ARCH)o_pi_g

default : executable

#../import.make is empty by default, but may be provided to copy sources from somewhere.
../import.make :
	echo > $@
IMPORT_PATH = grammar
include ../import.make

clean executable gnu realclean source_kit sun %.tar \
$(SRCDIR)/% $(OBJ_DIR)/% $(OBJ_DIR_PI)/% $(OBJ_DIR_G)/% $(OBJ_DIR_PI_G)/% \
	: makefile makefile.unix .FORCE
	$(MAKE) -f makefile.unix $(MFLAGS) $(MAKEFLAGS) $@ 

.FORCE: