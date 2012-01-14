
export DEFINITIONS
  # ensure that the macros get passed down to the subprocesses.

include cpp/variables.def

PROJECT = Source_Documentation
TYPE = application
FIRST_TARGETS = build_doxygen
CLEANUPS = html

include cpp/rules.def

build_doxygen:
	@echo the defs are $(DEFINITIONS)
	doxygen $(wildcard *.config)

