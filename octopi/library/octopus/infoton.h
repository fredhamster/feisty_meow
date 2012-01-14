#ifndef INFOTON_CLASS
#define INFOTON_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : infoton                                                           *
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

#include <basis/contracts.h>
#include <structures/string_array.h>

namespace octopi {

//! An infoton is an individual request parcel with accompanying information.
/*!
  This is the unit of data exchange in the octopus scheme.
*/

class infoton
: public virtual basis::packable,
  public virtual basis::clonable,
  public virtual basis::text_formable
{
public:
  infoton(const structures::string_array &classifier);
    //!< creates an infoton with the "classifier".
    /*!< keep in mind that although anything can be passed in here, the
    consistency of one's collection of octopi depends on a regular
    classification scheme.  it is recommended that the "classifier" be
    effectively constant.  also, classifiers that begin with the octothorpe
    (aka the pound sign '#') are reserved for octopus internal usage. */

  // takes care of the most common cases of 1, 2 & 3 level classifiers.
  infoton(const basis::astring &class_1);
  infoton(const basis::astring &class_1, const basis::astring &class_2);
  infoton(const basis::astring &class_1, const basis::astring &class_2, const basis::astring &cl_3);

  infoton(const infoton &to_copy);
    //!< copies only the base class portion of the infoton.
    /*!< clone() is the proper method for copying an instantiated infoton--
    this constructor only supports copying the base's information. */

  virtual ~infoton();

  DEFINE_CLASS_NAME("infoton");

  infoton &operator =(const infoton &to_copy);
    //!< assigns only the base class portion.
    /*!< clone() is the proper method for copying an instantiated infoton. */

  const structures::string_array &classifier() const;
    //!< this array of strings is the "name" for this infoton.
    /*!< the naming scheme for an infoton hierarchically and uniquely
    identifies the exact type of this object.  the last string (at the end()
    index) is the most specific name for this object, while the preceding
    names describe the object's membership in groups.  the outermost group
    name is at the zeroth index in the array.  a classifier can have one or
    more elements. */

  void set_classifier(const structures::string_array &new_classifier);
    //!< sets the infoton's classifier to the "new_classifier".
    /*!< do not do this unless you know what you're doing; changing the
    classifier may keep an infoton from being recognized properly. */

  // these are also dangerous if you're not careful; they mimic the
  // string constructors.
  void set_classifier(const basis::astring &class_1);
  void set_classifier(const basis::astring &class_1, const basis::astring &class_2);
  void set_classifier(const basis::astring &class_1, const basis::astring &class_2,
          const basis::astring &cl_3);

  bool check_classifier(const basis::astring &class_name, const basis::astring &caller);
    //!< checks that the classifier seems valid.
    /*!< the "class_name" and "caller" should be set to the location where
    the check is being done. */

  virtual void pack(basis::byte_array &packed_form) const = 0;
    //!< stuffs the data in the infoton into the "packed_form".
    /*!< the derived method must know how to pack this particular type
    of infoton. */
  virtual bool unpack(basis::byte_array &packed_form) = 0;
    //!< restores an infoton from a packed form.
    /*!< the unpack() method will be utilized by tentacles that support
    this type of object. */

  virtual void text_form(basis::base_string &state_fill) const = 0;
    //!< requires derived infotons to be able to show their state as a string.

  virtual clonable *clone() const = 0;
    //!< must be provided to allow creation of a copy of this object.

  virtual int packed_size() const = 0;
    //!< reports how large the infoton will be when packed.
    /*!< must be overridden by derived classes to provide a guess at how
    large the packed size of this will be.  this is important to estimate
    accurately. */

  //! local version just makes text_form() more functional.
  virtual basis::astring text_form() const { basis::astring fill; text_form(fill); return fill; }

  //////////////

  // This defines the wire format for a flattened infoton.  It is in essence
  // a packet header format which all infotons must adhere to to ensure that
  // they can be successfully unflattened when appropriate item managers are
  // available.
  static void fast_pack(basis::byte_array &packed_form, const infoton &to_pack);
    //!< flattens an infoton "to_pack" into the byte array "packed_form".

  static bool fast_unpack(basis::byte_array &packed_form, structures::string_array &classifier,
          basis::byte_array &info);
    //!< undoes a previous fast_pack to restore the previous information.
    /*!< extracts the data from a packed infoton in "packed_form" into the
    "classifier" and "info" that are contained therein. */

  static bool test_fast_unpack(const basis::byte_array &packed_form,
          int &packed_length);
    //!< checks that the "packed_form" could hold a valid packed infoton.
    /*!< tests that the smallest prefix of the "packed_form" looks like an
    appropriate packed classifier and packet length.  the "packed_length"
    is set to the length found in the packet.  note that the byte array
    does not need to contain the complete packed infoton yet; just the first
    portion where the header info is located must be present.  this method
    does not disturb the data in the packed array. */

  static int fast_pack_overhead(const structures::string_array &classifier);
    //!< reports how much space is needed to pack the "classifier".
    /*!< returns the overhead in bytes that will be added to an infoton's
    packed size when it is packed with fast_pack().  the "classifier" is the
    name of the infoton in question and must be accurate or the overhead will
    not be calculated properly. */

private:
  structures::string_array *_classifier;  //!< our classifier held.
};

//////////////

//! a templated method for cloning any infoton with a valid copy constructor.

template <class contents>
basis::clonable *cloner(const contents &this_obj) { return new contents(this_obj); }

} //namespace.

#endif

