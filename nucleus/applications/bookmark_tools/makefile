CONSOLE_MODE = true

include cpp/variables.def

PROJECT = bookmark_tools
TYPE = application
SOURCE = bookmark_tree.cpp bookmark_version.rc 
LOCAL_LIBS_USED += application configuration filesystem loggers mathematics nodes \
  processes textual timely structures basis 
TARGETS = js_marks_maker.exe link_parser.exe marks_checker.exe marks_maker.exe marks_sorter.exe
USE_CURL = t

include cpp/rules.def

