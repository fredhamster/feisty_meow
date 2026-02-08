CONSOLE_MODE = t

include cpp/variables.def

PROJECT = tests_application
TYPE = test
TARGETS = test_break_signal.exe test_command_line.exe test_path_configuration.exe \
  test_registry_configurator.exe 
DEFINITIONS += USE_FEISTY_MEOW_DLLS
LOCAL_LIBS_USED = unit_test application configuration filesystem loggers \
  mathematics nodes processes structures textual timely structures basis
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

