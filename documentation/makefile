
export DEFINITIONS
  # ensure that the macros get passed down to the subprocesses.

include cpp/variables.def

PROJECT = Source_Documentation
TYPE = application
FIRST_TARGETS += build_doxygen
CLEANUPS += html

include cpp/rules.def

build_doxygen:
	@echo the defs are $(DEFINITIONS)
	$(HIDESH) -c '\
doxygen $(wildcard *.config); \
if [ $$? -ne 0 ]; then \
  echo failed to run doxygen on our configuration file.; \
  echo perhaps it is not installed?; \
  echo these are the ubuntu install steps:; \
  echo   sudo apt-get install doxygen graphviz; \
echo and something else, uhhh....; \
echo oy; \
fi \
'

