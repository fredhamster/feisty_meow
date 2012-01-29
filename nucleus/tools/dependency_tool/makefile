CONSOLE_MODE = true

include cpp/variables.def

STRICT_WARNINGS =

PROJECT = dependency_tool
TYPE = application
SOURCE = cppsetup.cpp ifparser.cpp include.cpp parse.cpp pr.cpp
#DEFINITIONS += __BUILD_STATIC_APPLICATION__
ifeq "$(OP_SYSTEM)" "WIN32"
  SOURCE += makedep_version.rc 
endif
TARGETS = makedep.exe

include cpp/rules.def

