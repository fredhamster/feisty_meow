include variables.def

PROJECT = Setup_Hierarchy
TYPE = hierarchy

BUILD_BEFORE += bundle_example

ifeq "$(NO_BUILD_PACK)" ""
  BUILD_BEFORE += whole_build_package
endif

include rules.def

