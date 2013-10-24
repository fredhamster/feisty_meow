CONSOLE_MODE = t

include cpp/variables.def

PROJECT = tests_octopus
TYPE = test
TARGETS = t_bin.exe t_bin_threaded.exe t_entity.exe t_identity.exe \
  t_security.exe t_unpacker.exe t_file_transfer.exe
LOCAL_LIBS_USED = tentacles octopus sockets unit_test application configuration loggers \
  textual timely processes filesystem structures basis 
VCPP_USE_SOCK = t
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

