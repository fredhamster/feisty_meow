CONSOLE_MODE = t

include cpp/variables.def

PROJECT = simplistic_utils
TYPE = application
TARGETS = create_guid.exe playsound.exe short_path.exe sleep_ms.exe \
  zap_process.exe 
#run_as_service.exe 
ifeq "$(OMIT_VERSIONS)" ""
  SOURCE += simple_utils_version.rc
endif
DEFINITIONS += __BUILD_STATIC_APPLICATION__
UNDEFINITIONS += ENABLE_MEMORY_HOOK ENABLE_CALLSTACK_TRACKING
ifeq "$(OP_SYSTEM)" "WIN32"
  # static C runtime support...
#hmmm: resurrect this as a particular build type, so makefiles don't need to know this...
  COMPILER_FLAGS += -MT
  LIBS_USED += netapi32.lib
  ifeq "$(DEBUG)" ""
    LIBS_USED += libcmt.lib
  else
    LIBS_USED += libcmtd.lib
  endif
endif

include cpp/rules.def

