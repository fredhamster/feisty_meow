include cpp/variables.def

PROJECT = Whole_Build_Pack
FIRST_TARGETS += create_package
CLEANUPS += $(RUNTIME_PATH)/install/whole_build* $(RUNTIME_PATH)/install/sources_*
TYPE = hierarchy
export TARGET=/tmp/zz_wholebuild_$(USER)

include cpp/rules.def

create_package:
ifeq "$(NO_BUILD_PACK)" ""
#will it fail?	$(HIDE)-mkdir -p $(TARGET)/waste
	$(HIDESH)create_whole_build_pack.sh "$(major).$(minor).$(revision).$(build)"
endif

