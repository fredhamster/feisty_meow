include cpp/variables.def

PROJECT = tests_filesystem
TYPE = test
TARGETS = test_byte_filer.exe test_directory.exe test_directory_tree.exe test_file_info.exe \
  test_file_time.exe test_filename.exe test_huge_file.exe
DEFINITIONS += USE_FEISTY_MEOW_DLLS
LOCAL_LIBS_USED = unit_test application configuration filesystem loggers \
  mathematics nodes processes structures textual timely structures basis  \
loggers 
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

