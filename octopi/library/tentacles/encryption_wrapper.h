#ifndef ENCRYPTION_WRAPPER_CLASS
#define ENCRYPTION_WRAPPER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : encryption_wrapper                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <octopus/entity_defs.h>
#include <octopus/infoton.h>
#include <octopus/tentacle_helper.h>

namespace octopi {

//! Wraps an encrypted infoton when the octopus is in an encrypted mode.
/*!
  The enclosed package will be unwrapped by the encryption tentacle.
*/

class encryption_wrapper : public infoton
{
public:
  basis::byte_array _wrapped;
    //!< the encrypted data that's held here.
    /*!< this must be a packed classifier string array followed by
    the packed infoton. */

  encryption_wrapper(const basis::byte_array &wrapped = basis::byte_array::empty_array());

  encryption_wrapper(const encryption_wrapper &to_copy);

  virtual ~encryption_wrapper();

  DEFINE_CLASS_NAME("encryption_wrapper");

  encryption_wrapper &operator =(const encryption_wrapper &to_copy);

  void text_form(basis::base_string &fill) const {
    fill.assign(basis::astring(class_name()));  // low exposure for vital held info.
  }

  static const structures::string_array &encryption_classifier();
    //!< returns the classifier for this type of infoton.

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

  virtual clonable *clone() const;

  virtual int packed_size() const;
};

//////////////

//! this simple tentacle just unpacks the encryption_wrapper infoton.
/*!
  this object should never be doing more than that.
*/

class unwrapping_tentacle
: public tentacle_helper<encryption_wrapper>
{
public:
  unwrapping_tentacle();
  virtual ~unwrapping_tentacle();

  DEFINE_CLASS_NAME("unwrapping_tentacle");

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, infoton * &reformed);

  virtual basis::outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          basis::byte_array &transformed);
    //!< this should never be called.
};

} //namespace.

#endif  // outer guard.

