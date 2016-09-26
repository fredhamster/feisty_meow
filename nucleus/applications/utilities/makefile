CONSOLE_MODE = true

include cpp/variables.def

PROJECT = utilities
TYPE = application
ifeq "$(OMIT_VERSIONS)" ""
  SOURCE += util_version.rc 
endif
LOCAL_LIBS_USED = application configuration filesystem loggers mathematics nodes processes textual timely structures basis 
TARGETS = await_app_exit.exe bytedump.exe checker.exe dirtree.exe ini_edit.exe mdate.exe \
  splitter.exe time_set_effective_id.exe time_running_app.exe

include cpp/rules.def

