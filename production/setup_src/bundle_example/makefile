include cpp/variables.def

PROJECT = example_bundle
TYPE = hierarchy
FIRST_TARGETS = build_bundle
CLEANUPS = $(FEISTY_MEOW_DIR)/install/example_bundle$(EXE_END)
# set the unix bin variable so the bundle finds the apps.
export UNIX_BIN = /bin

include cpp/rules.def

build_bundle:
	$(HIDESH)create_bundle.sh

