CONSOLE_MODE = t

include cpp/variables.def

PROJECT = check_versions
TYPE = test
#TARGETS = check_versions.exe
DEFINITIONS += __BUILD_STATIC_APPLICATION__

ifeq "$(OP_SYSTEM)" "WIN32"
 ifeq "$(BOOT_STRAPPING)" ""
  LAST_TARGETS = run_checks
 endif
endif

include cpp/rules.def

run_checks:
	echo version checks temporarily turned off.
#	-$(HIDESH)-c '\
if [ ! -d $(REPOSITORY_DIR)/logs ]; then \
  mkdir $(REPOSITORY_DIR)/logs; \
fi'
#	-$(CATCHER)$(EXECUTABLE_DIR)/check_versions.exe >$(REPOSITORY_DIR)/logs/version_report.txt 2>&1

