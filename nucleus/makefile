# Top-level makefile for library, application and test components.

include variables.def

# Set default locations for the following variables.  If the build_variables
# have been sourced in, that takes care of setting these.
ifeq "$(FEISTY_MEOW_DIR)" ""
  export FEISTY_MEOW_DIR := $(CURRENT_DIR)
endif
ifeq "$(PRODUCTION_DIR)" ""
  export PRODUCTION_DIR = $(FEISTY_MEOW_DIR)/production
endif
ifeq "$(CLAM_DIR)" ""
  export CLAM_DIR = $(FEISTY_MEOW_SCRIPTS)/clam
endif

include $(CLAM_DIR)/cpp/variables.def

PROJECT = feisty_meow_nucleus
TYPE = hierarchy
FIRST_TARGETS = do_make
BUILD_AFTER = library applications tools

include $(CLAM_DIR)/cpp/rules.def

do_make: start_make $(PRODUCTION_DIR)/binaries/manifest.txt end_make

start_make: show_date.start

end_make: show_date.end

$(PRODUCTION_DIR)/binaries/manifest.txt: $(PARAMETER_FILE)
	$(HIDESH) -c '\
if [ ! -d "$(EXECUTABLE_DIR)" ]; then mkdir -p "$(EXECUTABLE_DIR)"; fi; \
if [ $$? -ne 0 ]; then \
  echo build failure while creating executable directory.; \
  exit 1; \
fi; \
cp -f "$(PRODUCTION_DIR)/paths.ini" "$(EXECUTABLE_DIR)"; \
if [ $$? -ne 0 ]; then \
  echo build failure while copying paths initialization file.; \
  exit 1; \
fi; \
echo cmd is: $(CLAM_BIN)/value_tagger$(EXE_END) $(PRODUCTION_DIR)/codescan.ini; \
$(CLAM_BIN)/value_tagger$(EXE_END) $(PRODUCTION_DIR)/codescan.ini; \
if [ $$? -ne 0 ]; then \
  echo build failure during value tagging.; \
  exit 1; \
fi; \
$(CLAM_BIN)/write_build_config$(EXE_END); \
if [ $$? -ne 0 ]; then \
  echo build failure while writing config.; \
  exit 1; \
fi; \
  '

