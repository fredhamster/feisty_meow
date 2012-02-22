include cpp/variables.def

PROJECT = example_bundle
TYPE = hierarchy
FIRST_TARGETS = build_bundle
CLEANUPS = $(FEISTY_MEOW_DIR)/install/example_bundle$(EXE_END)
export UNIX_BIN = /bin
#hmmm: is UNIX_BIN used anywhere?

include cpp/rules.def

build_bundle:
	$(HIDESH)create_bundle.sh

