CONSOLE_MODE = t

include cpp/variables.def

PROJECT = tests_application
TYPE = test
TARGETS = test_break_signal.exe test_command_line.exe 

#test_byte_filer.exe 
#  test_directory.exe test_directory_tree.exe test_dirtree_fcopy.exe test_filename.exe \
#  test_huge_file.exe test_ini_configurator.exe test_ini_parser.exe test_logger.exe \
#  test_path_configuration.exe test_registry_configurator.exe test_redirection.exe \
#  test_rendezvous.exe test_system_values.exe test_timer_driver.exe
#ifeq "$(findstring EMBEDDED_BUILD, $(DEFINITIONS))" ""
#  TARGETS += test_shared_memory.exe 
#endif
##LOCAL_LIBS_USED = basis i_library 
DEFINITIONS += USE_FEISTY_MEOW_DLLS
LOCAL_LIBS_USED = unit_test application configuration filesystem loggers \
  mathematics nodes processes structures textual timely structures basis

#VCPP_USE_GUI=t
#USE_HOOPLE_DLLS=t

LAST_TARGETS += test_break_signal_special 
RUN_TARGETS = $(EXECUTABLE_DIR)/test_command_line 

#  test_filename.exe \
#  test_huge_file.exe test_ini_configurator.exe test_ini_parser.exe test_logger.exe \
#  test_path_configuration.exe test_registry_configurator.exe \
#  test_system_values.exe test_timer_driver.exe

include cpp/rules.def

# run the break signal tester with a short timeout, since generally no one's watching the build to hit break.
test_break_signal_special:
	$(EXECUTABLE_DIR)/test_break_signal 1

