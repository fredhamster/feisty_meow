CONSOLE_MODE = t

include cpp/variables.def

PROJECT = tests_octopus
TYPE = test
TARGETS = test_bin.exe test_bin_threaded.exe test_entity.exe test_identity.exe \
  test_security.exe test_unpacker.exe test_file_transfer.exe
LOCAL_LIBS_USED = tentacles octopus sockets unit_test application configuration loggers \
  textual octopus timely configuration nodes processes filesystem structures basis 
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

