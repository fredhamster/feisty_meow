include cpp/variables.def

PROJECT = tests_basis
TYPE = test
SOURCE = checkup.cpp
TARGETS = test_array.exe test_boilerplate.exe test_mutex.exe test_string.exe \
  test_system_preconditions.exe
DEFINITIONS += USE_HOOPLE_DLLS
LOCAL_LIBS_USED = unit_test application processes loggers configuration mathematics nodes \
  structures textual timely filesystem structures basis 
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

