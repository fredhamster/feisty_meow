CONSOLE_MODE = true

include cpp/variables.def

PROJECT = security
TYPE = library
ifeq "$(OPERATING_SYSTEM)" "WIN32"
  SOURCE = nt_security.cpp win32_security.cpp windows_firewall.cpp
endif
TARGETS = security.lib

include cpp/rules.def
