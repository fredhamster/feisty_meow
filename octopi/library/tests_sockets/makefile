include cpp/variables.def

PROJECT = tests_sockets
TYPE = test
SOURCE = bcast_spocketer.cpp spocket_tester.cpp
TARGETS = test_address.exe test_bcast_spocket.exe test_enum_adapters.exe \
  test_sequence_tracker.exe test_span_manager.exe test_spocket.exe test_ucast_spocket.exe 
LOCAL_LIBS_USED = sockets unit_test application configuration loggers textual timely \
  processes filesystem structures basis 
VCPP_USE_SOCK = t
ifeq "$(OP_SYSTEM)" "WIN32"
  LIBS_USED += ws2_32.lib
endif
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def
