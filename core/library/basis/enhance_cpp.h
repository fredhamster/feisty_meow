#ifndef ENHANCE_CPP_GROUP
#define ENHANCE_CPP_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : enhance_cpp                                                       *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/definitions.h>

namespace basis {

//! Provides missing language features in C++.
/*!
  The most noticeable missing thing in C++ when trying to build debugging
  and tracking subsystems with it is *reflection*.  This header attempts to
  ameliorate some of the worst missing parts, such as the fact that a function
  cannot get its own name, and other really helpful features.
*/

//hmmm: temporary to hide missing code.
#define frame_tracking_instance
#define __trail_of_function(a, b, c, d, e)

class enhance_cpp : public virtual root_object
{
public:
  // this class is an encapsulator; any macros are not actual members.

//////////////

  //! Defines the name of a class by providing a couple standard methods.
  /*! This provides a virtual function functionality slice for the class name,
  as well as a static version that can be used when no instances of the
  class exist yet. */
  #define DEFINE_CLASS_NAME(objname) \
    static const char *static_class_name() { return (objname); } \
    virtual const char *class_name() const { return static_class_name(); }

//////////////

  //! FUNCDEF sets the name of a function (and plugs it into the callstack).
  /*! This macro establishes the function name and should be used at the top
  of functions that wish to participate in class based logged as well as the
  callstack tracing capability of hoople.  A new variable is created on the
  stack to track the function's presence until the function exits, at which
  time the stack will no longer show it as active. */
  #define FUNCDEF(func_in) \
    const char *func = (const char *)func_in; \
    frame_tracking_instance __trail_of_function(static_class_name(), func, \
        __FILE__, __LINE__, true)

//////////////

  //! A macro used within the FUNCTION macro to do most of the work.
  #define BASE_FUNCTION(func) astring just_function = astring(func); \
    astring function_name = static_class_name(); \
    function_name += astring("::") + just_function
  //! This macro sets up a descriptive variable called "function_name".
  /*! The variable includes the object's name (static_class_name() must be
  implemented for the current object) and the current function's name within
  that object (the macro "func" must be defined with that name). */
  #define FUNCTION(func) BASE_FUNCTION(func); \
    function_name += ": "; \
    update_current_stack_frame_line_number(__LINE__)

  //! A macro used within the INSTANCE_FUNCTION macro.
  #define BASE_INSTANCE_FUNCTION(func) astring just_function = astring(func); \
    astring function_name = instance_name(); \
    function_name += astring("::") + just_function
  //! A function macro that contains more information.
  /*! This macro is similar to FUNCTION but it uses the class's instance_name()
  method (see root_object).  The instance function usually will provide more
  information about the class. */
  #define INSTANCE_FUNCTION(func) BASE_INSTANCE_FUNCTION(func); \
    function_name += ": "; \
    update_current_stack_frame_line_number(__LINE__)

//////////////

  //! __WHERE__ is a macro that combines the file and line number macros.
  /*! These are available to most compilers as automatically updated macros
  called __FILE__ and __LINE__.  This macro can be used anywhere an astring can
  be used and reports the current file name and line number. */
  #define __WHERE__ basis::a_sprintf("%s [line %d]", __FILE__, __LINE__)

};

} //namespace.

#endif

