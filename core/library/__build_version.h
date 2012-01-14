#ifndef BUILD_VERSION_CONFIGURATION
#define BUILD_VERSION_CONFIGURATION

  // This file provides the version macros for this particular build.

  #ifndef __build_major
    #define __build_major "2"
  #endif
  #ifndef __build_minor
    #define __build_minor "108"
  #endif
  #ifndef __build_revision
    #define __build_revision "86"
  #endif
  #ifndef __build_build
    #define __build_build "0"
  #endif

  // calculated macros are dropped in here.

  #define __build_SYSTEM_VERSION "2.108.86.0"

  #define __build_FILE_VERSION_COMMAS 2, 108, 86, 0
  #define __build_FILE_VERSION "2.108.86.0"
  #define __build_PRODUCT_VERSION_COMMAS 2, 108, 0, 0
  #define __build_PRODUCT_VERSION "2.108"

#endif /* outer guard */

