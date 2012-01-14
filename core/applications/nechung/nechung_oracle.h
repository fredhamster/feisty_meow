#ifndef NECHUNG_CLASS
#define NECHUNG_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : nechung_oracle                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    This is the root nechung functionality.  It provides a means of          *
*  randomly selecting an item out of a specially formatted file.  If no index *
*  file has previously been built for the file, then one is created.  The     *
*  index file makes choosing a fortune randomly very quick; only a seek on    *
*  the much smaller index is needed in order to find the position of the      *
*  fortune to be shown.                                                       *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

// Nechung works with a particular form of data file and will extract a random
// and hopefully auspicious message out of that file.  An example of the file
// format is:
//   msg1
//   ~
//   msg2
//   ~
//   (... more messages and tildes...)
// The tilde is the separation character mentioned below.

#include <basis/astring.h>
#include <mathematics/chaos.h>

//#define DEBUG_NECHUNG
  // uncomment this to get the debugging version.  it is used by several files
  // that are part of nechung.

const char NECHUNG_SEPARATION_CHARACTER = '~';
  // this character separates the entries in the fortunes database.

class nechung_oracle
{
public:
  nechung_oracle(const basis::astring &data_filename, const basis::astring &index_filename);
    // the constructor needs the name of a nechung format data file in
    // "data_filename" and the name of the index file to be used for that data
    // file in "index_filename".

  virtual ~nechung_oracle();

  DEFINE_CLASS_NAME("nechung_oracle");

  basis::astring pick_random();
    // returns a randomly chosen fortune.

  void display_random();
    // selects an oracular pronouncement from the file and then shows it on
    // standard output.

private:
  mathematics::chaos c_randomizer;  // the random number generator we use.
  basis::astring c_filename_held;  // the data file's name.
  basis::astring c_index_held;  // the index file's name.
  int c_number_of_fortunes;  // how many fortunes exist in the file.

  void parse_file();
    // given the data file and index file, this will ensure that the index
    // file is made up to date.  it creates, if necessary, the file that
    // contains the positions of fortunes in the data file.  this is what
    // we'll use to find the start of each fortune.  if the file already
    // exists, then it will just retrieve the number of fortunes from the index
    // file.  after this method, the pick_random() and display_random() methods
    // are available.

  // disallowed.
  nechung_oracle(const nechung_oracle &);
  nechung_oracle &operator =(const nechung_oracle &);
};

#endif

