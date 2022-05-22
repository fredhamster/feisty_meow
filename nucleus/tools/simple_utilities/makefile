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

include cpp/rules.def

