include cpp/variables.def

PROJECT = Whole_Build_Pack
FIRST_TARGETS = create_package
CLEANUPS = $(PRODUCTION_DIR)/install/whole_build*
TYPE = hierarchy

include cpp/rules.def

create_package:
ifeq "$(NO_BUILD_PACK)" ""
	$(HIDESH)create_whole_build_pack.sh "$(major).$(minor).$(revision).$(build)"
endif

