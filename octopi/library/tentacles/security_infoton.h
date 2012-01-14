#ifndef SECURITY_INFOTON_CLASS
#define SECURITY_INFOTON_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : security_infoton                                                  *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <octopus/infoton.h>

namespace octopi {

//! Encapsulates security activities (login, logout, refresh).

class security_infoton : public infoton
{
public:
  enum login_modes {
    LI_LOGIN,   //!< the requester wants to log in as a new entity.
    LI_LOGOUT,  //!< the requester surrenders its login.
    LI_REFRESH  //!< the requester is still alive and wants to keep its login.
  };

  login_modes _mode; //!< what kind of request is being made here?
  basis::outcome _success;  //!< did the request succeed?

  security_infoton();
  security_infoton(login_modes mode, const basis::outcome &success,
          const basis::byte_array &verification);
  security_infoton(const security_infoton &to_copy);

  virtual ~security_infoton();

  DEFINE_CLASS_NAME("security_infoton");

  security_infoton &operator =(const security_infoton &to_copy);

  // observes or modifies the verification token.
  const basis::byte_array &verification() const;
  basis::byte_array &verification();

  static const structures::string_array &security_classifier();
    //!< returns the classifier for this type of infoton.

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  virtual clonable *clone() const;

  virtual int packed_size() const;

  virtual void text_form(basis::base_string &fill) const {
    fill.assign(basis::astring(class_name())
        + basis::a_sprintf(": mode %d, outcome=%d", _mode, _success.value()));
  }

private:
  basis::byte_array *_verification;  //!< anything needed for upper-level verification.
};

} //namespace.

#endif

