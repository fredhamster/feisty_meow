include cpp/variables.def

PROJECT = rebaser
#ifeq "$(OPERATING_SYSTEM)" "WIN32"
#  TARGETS = perform_rebasing
#endif
TYPE = hierarchy

include cpp/rules.def

perform_rebasing:
	$(HIDESH)./rebaser.sh
