include cpp/variables.def

PROJECT = tests_timely
TYPE = test
TARGETS = test_earth_time.exe test_stopwatch.exe
DEFINITIONS += USE_FEISTY_MEOW_DLLS
LOCAL_LIBS_USED = unit_test application processes loggers configuration mathematics nodes \
  structures textual timely filesystem structures basis 
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

