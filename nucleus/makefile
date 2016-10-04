# Top-level makefile for library, application and test components.

include variables.def

# Set default locations for the following variables.  If the build_variables
# have been sourced in, that takes care of setting these.
ifeq "$(FEISTY_MEOW_APEX)" ""
  export FEISTY_MEOW_APEX := $(CURRENT_DIR)
endif
#ifeq "$(PRODUCTION_STORE)" ""
#  export PRODUCTION_STORE = $(FEISTY_MEOW_APEX)/production
#endif
ifeq "$(CLAM_SCRIPTS)" ""
  export CLAM_SCRIPTS = $(FEISTY_MEOW_SCRIPTS)/clam
endif

include $(CLAM_SCRIPTS)/cpp/variables.def

PROJECT = feisty_meow_nucleus
TYPE = hierarchy
FIRST_TARGETS += do_make
BUILD_AFTER = library applications tools

include $(CLAM_SCRIPTS)/cpp/rules.def

do_make: start_make $(RUNTIME_PATH)/binaries/manifest.txt end_make

start_make: show_date.start

end_make: show_date.end

$(RUNTIME_PATH)/binaries/manifest.txt: $(PARAMETER_FILE)
	$(HIDESH) -c '\
if [ ! -d "$(EXECUTABLE_DIR)" ]; then mkdir -p "$(EXECUTABLE_DIR)"; fi; \
if [ $$? -ne 0 ]; then \
  echo build failure while creating executable directory.; \
  exit 1; \
fi; \
cp -f "$(PRODUCTION_STORE)/paths.ini" "$(EXECUTABLE_DIR)"; \
if [ $$? -ne 0 ]; then \
  echo build failure while copying paths initialization file.; \
  exit 1; \
fi; \
bash $(BUILD_SCRIPTS_PATH)/wrapdoze.sh $(CLAM_BINARIES)/value_tagger$(EXE_END) $(PRODUCTION_STORE)/codescan.ini; \
if [ $$? -ne 0 ]; then \
  echo build failure during value tagging.; \
  exit 1; \
fi; \
bash $(BUILD_SCRIPTS_PATH)/wrapdoze.sh $(CLAM_BINARIES)/write_build_config$(EXE_END); \
if [ $$? -ne 0 ]; then \
  echo build failure while writing config.; \
  exit 1; \
fi; \
  '

