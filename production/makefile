include variables.def

PROJECT = feisty_meow_production_dept

BUILD_BEFORE = assign_bases 
# assign_bases is run after all the dynamic libraries have been built.
# on win32, it resets the DLL base addresses, so that we do not try to load where
# everyone else will, at the default location.

BUILD_AFTER = check_versions setup_src
# check_versions ensures that all the version stamps on DLLs match our expectations.
# setup_src hierarchy will build our installers.

include rules.def

