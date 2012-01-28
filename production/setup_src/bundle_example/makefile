include cpp/variables.def

PROJECT = example_bundle
TYPE = hierarchy
FIRST_TARGETS = build_bundle
CLEANUPS = $(FEISTY_MEOW_DIR)/install/example_bundle$(EXE_END)
export UNIX_BIN = /bin
ifeq "$(OP_SYSTEM)" "WIN32"
  export UNIX_BIN = $(BUILDING_HIERARCHY)/msys/bin
endif

include cpp/rules.def

build_bundle:
	$(HIDESH)create_bundle.sh

