CONSOLE_MODE = true

include cpp/variables.def

PROJECT = app_bundle
TYPE = application
SOURCE = common_bundle.cpp 
ifeq "$(OMIT_VERSIONS)" ""
  SOURCE += bundler_version.rc
endif
DEFINITIONS += __BUILD_STATIC_APPLICATION__=t
TARGETS = bundle_creator.exe
LAST_TARGETS += make_stub
ifeq "$(OP_SYSTEM)" "WIN32"
  LOCAL_HEADERS += $(THIRD_PARTY_DIR)/zlib/include
  LOCAL_LIBRARIES += $(THIRD_PARTY_DIR)/zlib/lib
  LIBS_USED += zlib.lib
endif
ifeq "$(OP_SYSTEM)" "UNIX"
  LIBS_USED += z
endif

include cpp/rules.def

make_stub:
	$(MAKE) -f makefile.stub

