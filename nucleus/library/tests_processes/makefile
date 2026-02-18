CONSOLE_MODE = t

include cpp/variables.def

PROJECT = test_processes
TYPE = test
SOURCE = 
TARGETS = find_window.exe test_safe_callback.exe test_state_machine.exe zing_window.exe
LOCAL_LIBS_USED = unit_test application configuration filesystem loggers \
  nodes processes structures textual timely structures basis 
#filesystem  mathematics 
RUN_TARGETS = $(ACTUAL_TARGETS)
LAST_TARGETS += build_shutdowner

include cpp/rules.def

build_shutdowner:
	$(MAKE) -f makefile.shutdowner

