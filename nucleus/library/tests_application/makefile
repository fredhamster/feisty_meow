CONSOLE_MODE = t

include cpp/variables.def

PROJECT = tests_application
TYPE = test
TARGETS =    test_command_line.exe \
#test_break_signal.exe test_byte_filer.exe 
#  test_directory.exe test_directory_tree.exe test_dirtree_fcopy.exe test_filename.exe \
#  test_huge_file.exe test_ini_configurator.exe test_ini_parser.exe test_logger.exe \
#  test_path_configuration.exe test_registry_configurator.exe test_redirection.exe \
#  test_rendezvous.exe test_system_values.exe test_timer_driver.exe
#ifeq "$(findstring EMBEDDED_BUILD, $(DEFINITIONS))" ""
#  TARGETS += test_shared_memory.exe 
#endif
##LOCAL_LIBS_USED = basis i_library 
DEFINITIONS += USE_FEISTY_MEOW_DLLS
LOCAL_LIBS_USED = unit_test application 
#configuration filesystem loggers \
#  mathematics nodes processes structures textual timely structures basis  \
#loggers 

#VCPP_USE_GUI=t
#USE_HOOPLE_DLLS=t

#RUN_TARGETS = $(ACTUAL_TARGETS)
# run every test except test_rendezvous, which waits for a key.
RUN_TARGETS =    test_command_line.exe \

#test_break_signal.exe test_byte_filer.exe 
#  test_directory.exe test_directory_tree.exe test_filename.exe \
#  test_huge_file.exe test_ini_configurator.exe test_ini_parser.exe test_logger.exe \
#  test_path_configuration.exe test_registry_configurator.exe \
#  test_system_values.exe test_timer_driver.exe

include cpp/rules.def

