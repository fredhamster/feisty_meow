
export DEFINITIONS
  # ensure that the macros get passed down to the subprocesses.

include cpp/variables.def

PROJECT = Source_Documentation
TYPE = application
FIRST_TARGETS += build_docs

include cpp/rules.def

build_docs:
#	@echo the defs are $(DEFINITIONS)
	@echo Regenerating documentation for feisty meow with doxygen.  This could take a while...
	$(HIDESH) -c '\
outfile="$$TEMPORARIES_PILE/doxygen_run_$$(date +"%Y%m%d%H%M%S").log"; \
doxygen $(wildcard *.config) &> "$$outfile"; \
if [ $$? -ne 0 ]; then \
  echo failed to run doxygen on our configuration file.; \
  echo perhaps doxygen is not properly installed?; \
  echo these are the ubuntu install steps:; \
  echo   sudo apt install doxygen graphviz; \
fi; \
\rm -f "$$outfile" \
'
	@echo ...done regenerating the feisty meow docs.

clean_docs:
	@echo dropping code_guide contents.
	$(HIDESH) -c '\
for dir in ../production/code_guide; do \
  for patt in html map md5 png js css ttf dot ; do \
    rm -f $$dir/*.$$patt; \
  done; \
done \
'


