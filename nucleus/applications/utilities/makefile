CONSOLE_MODE = true

include cpp/variables.def

PROJECT = utilities
TYPE = application
ifeq "$(OMIT_VERSIONS)" ""
  SOURCE += util_version.rc 
endif
LOCAL_LIBS_USED = application loggers mathematics processes textual timely configuration filesystem structures basis 
TARGETS = await_app_exit.exe bytedump.exe checker.exe ini_edit.exe mdate.exe splitter.exe 
#check_address.exe check_eol.exe create_guid.exe cstrip.exe \
  empty.exe find_non_ascii.exe find_text.exe insert_before.exe \
  lexcaser.exe machine_name.exe merge_inis.exe net_calc.exe pi.exe \
  replace_text.exe show_args.exe show_versions.exe to_lower.exe uucat.exe
#VCPP_USE_SOCK=t

include cpp/rules.def

