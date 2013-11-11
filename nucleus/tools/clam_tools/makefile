CONSOLE_MODE = true

include cpp/variables.def

PROJECT = clam_tools
TYPE = application
DEFINITIONS += __BUILD_STATIC_APPLICATION__

# why was that there?
#LIBS_USED += pthread
#

ifeq "$(OMIT_VERSIONS)" ""
  SOURCE += clamtools_version.rc 
endif
TARGETS = value_tagger.exe version_stamper.exe vsts_version_fixer.exe write_build_config.exe

include cpp/rules.def

