CONSOLE_MODE = true

include cpp/variables.def

PROJECT = utilities
TYPE = application
ifeq "$(OMIT_VERSIONS)" ""
  SOURCE += util_version.rc 
endif
LOCAL_LIBS_USED = application loggers mathematics processes textual timely configuration filesystem nodes structures basis 
TARGETS = await_app_exit.exe bytedump.exe checker.exe dirtree.exe ini_edit.exe mdate.exe \
  splitter.exe 

include cpp/rules.def

