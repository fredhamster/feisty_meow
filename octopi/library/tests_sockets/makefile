include cpp/variables.def

PROJECT = tests_sockets
TYPE = test
SOURCE = bcast_spocketer.cpp spocket_tester.cpp
TARGETS = test_address.exe test_bcast_spocket.exe test_sequence_tracker.exe \
  test_span_manager.exe test_spocket.exe test_ucast_spocket.exe 
ifneq "$(OS_SUBCLASS)" "darwin"
  TARGETS += test_enum_adapters.exe 
endif
LOCAL_LIBS_USED = sockets unit_test application configuration loggers textual timely \
  processes filesystem structures basis 
VCPP_USE_SOCK = t
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def
