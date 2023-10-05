#ifndef EARTH_TIME_GROUP
#define EARTH_TIME_GROUP

/*
  Name   : earth_time
  Author : Chris Koeritz

  Copyright (c) 1999-$now By Author.  This program is free software; you can
  redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either version 2 of
  the License or (at your option) any later version.  This is online at:
      http://www.fsf.org/copyleft/gpl.html
  Please send any updates to: fred@gruntose.com
*/

#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>
//#include <basis/definitions.h>
#include <structures/object_packers.h>

///#include <time.h>

//! A set of methods for rendering calendrical and clock times.
/*!
  It is based on the Gregorian calendar currently in use by the USA and other
  countries.
*/
namespace timely {

  typedef basis::signed_long time_number;

  enum days { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };
    //!< The names of the days of the week.

  days day_now();  //!< Returns the current local day.

  const char *day_name(days to_name);
    //!< Returns the name of the day "to_name".

  enum months { JANUARY, FEBRUARY, MARCH, APRIL, MAY, JUNE, JULY, AUGUST,
      SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER };
    //!< The names of the months in our calendar.

  months month_now();  //!< returns the local month.

  const char *month_name(months to_name);
    //!< Returns the name of the month "to_name".
  const char *short_month_name(months to_name);
    //!< Returns a shorter, constant-length (3 characters) month name.

  extern const time_number days_in_month[12];
    //!< The number of days in each month in the standard year.
  extern const time_number leap_days_in_month[12];
    //!< The number of days in each month in a leap year.

  extern const time_number julian_days_in_month[12];
    //!< Number of days in each month based on the julian calendar.
  extern const time_number julian_leap_days_in_month[12];
    //!< Number of days in each month of a leap year in the julian calendar.

  const time_number SECONDS_IN_MINUTE = 60;  //!< Number of seconds in one minute.
  const time_number MINUTES_IN_HOUR = 60;  //!< Number of minutes in an hour.
  const time_number SECONDS_IN_HOUR = 3600;  //!< Number of seconds in an hour.
  const time_number HOURS_IN_DAY = 24;  //!< Number of hours in a day.
  const time_number MINUTES_IN_DAY = 1440;  //!< Number of minutes in a day.
  const time_number SECONDS_IN_DAY = 86400;  //!< Number of seconds in a day.
  const time_number DAYS_IN_YEAR = 365;  //!< Number of days in a standard year.
  const time_number LEAP_DAYS_IN_YEAR = 366;  //!< Number of days in a leap year.
  const double APPROX_DAYS_IN_YEAR = 365.2424;
    //!< A more accurate measure of the number of days in a year.
    /*!< This is the more accurate mean length of time in 24 hour days between
    vernal equinoxes.  it's about 11 minutes shy of 365.25 days. */

  //! An enumeration of time zones, both relative and absolute.
  enum time_zones {
    LOCAL_ZONE,      //!< The time zone this computer is configured to report.
    GREENWICH_ZONE   //!< The time zone of Greenwich Mean Time.
  };

  // now some structures for representing time...

  //! A specific point in time as represented by a 24 hour clock.
  class clock_time : public virtual basis::packable
  {
  public:
    time_number hour;  //!< The hour represented in military time: 0 through 23.
    time_number minute;  //!< The number of minutes after the hour.
    time_number second;  //!< The number of seconds after the current minute.
    time_number millisecond;  //!< The number of milliseconds elapsed in this second.
    time_number microsecond;  //!< Number of microseconds elapsed in this millisecond.

    //! Constructs a clock_time object given all the parts.
    clock_time(time_number h = 0, time_number m = 0, time_number s = 0, time_number ms = 0, time_number us = 0)
             : hour(h), minute(m), second(s), millisecond(ms),
               microsecond(us) {}
    ~clock_time() {}

    int packed_size() const { return 5 * structures::PACKED_SIZE_INT32; }

    virtual void pack(basis::byte_array &packed_form) const;
      //!< Packs a clock time into an array of bytes.
    virtual bool unpack(basis::byte_array &packed_form);
      //!< Unpacks a clock time from an array of bytes.

    bool operator < (const clock_time &to_compare) const;
      //!< Returns true if this clock_time is earlier than "to_compare"
    bool operator == (const clock_time &to_compare) const;
      //!< Returns true if this clock_time is equal to "to_compare"

    //! An enumeration of time formatting modes used when printing the time.
    enum time_formats {
      MERIDIAN = 0x1,  //!< default: uses 12 hour with AM/PM and no seconds.
      MILITARY = 0x2,  //!< use military 24 hour time.
      NO_AM_PM = 0x4,  //!< use 12 hour time but don't include AM/PM.
      SECONDS = 0x8,   //!< include the number of seconds as a third field.
      MILLISECONDS = 0x10  //!< milliseconds are fourth field (after secs).
    };

    basis::astring text_form(int how = MERIDIAN) const;
      //!< Prints the clock_time according to "how".
      /*!< "how" is a combination of time_formats. */
    void text_form(basis::astring &to_stuff, int how = MERIDIAN) const;
      //!< Prints the time into "to_stuff" given "how".
      /*!< note that "to_stuff" will be appended to; the existing contents
      are retained. */

    static time_number normalize(clock_time &to_fix);
      // ensures that the units in each field are in the proper range by
      // promoting them upwards.  if the clock_time goes above the maximum hour,
      // then it rolls around.  zero is returned for no rollover or a positive
      // integer is returned for the number of rollovers that occurred.  if
      // there are any negative fields, they are rolled backwards.
      // the returned rollovers are measured in days.
  };

  //! An object that represents a particular day in a year.
  class day_in_year : public virtual basis::packable
  {
  public:
    months month;  //!< The current month.
    time_number day_in_month;  //!< The day number within the month (starting at one).
    days day_of_week;  //!< The day of the week.
    time_number day_of_year;  //!< Numerical day, where January 1st is equal to zero.

    int packed_size() const { return 4 * structures::PACKED_SIZE_INT64; }

    //! Constructs a representation of the day specified.
    day_in_year(months m = JANUARY, time_number dim = 1, days dow = SUNDAY,
            time_number day_o_year = 1) : month(m), day_in_month(dim),
            day_of_week(dow), day_of_year(day_o_year) {}

    virtual void pack(basis::byte_array &packed_form) const;
      //!< Packs a day object into an array of bytes.
    virtual bool unpack(basis::byte_array &packed_form);
      //!< Unpacks a day object from an array of bytes.

    bool operator < (const day_in_year &to_compare) const;
      //!< Returns true if this day is earlier than "to_compare"
      /*!< Note that this only compares the month and day in the month. */
    bool operator == (const day_in_year &to_compare) const;
      //!< Returns true if this day is equal to "to_compare"
      /*!< Note that this only compares the month and day in the month. */

    //! An enumeration of ways to print out the current date.
    enum date_formats {
      // note: these classes may need to be revised in the year 9999.
      SHORT_MONTH = 0x1,    //!< default: three letter month.
      LONG_MONTH = 0x2,     //!< uses full month name.
      INCLUDE_DAY = 0x4     //!< adds the name of the day.
    };

    basis::astring text_form(int how = SHORT_MONTH) const;
      //!< Prints the day according to "how".
    void text_form(basis::astring &to_stuff, int how = SHORT_MONTH) const;
      //!< Prints the day according to "how" and stores it in "to_stuff".

    static time_number normalize(day_in_year &to_fix, bool leap_year = false);
      //!< normalizes the day as needed and returns the adjustment in years.
      /*!< note that this only adjusts the day_in_month and month members
      currently.  the other counters are not changed. */
  };

  //! An object that represents a particular point in time.
  /*! It contains both a time of day and the day in the year. */
  class time_locus : public clock_time, public day_in_year,
      public virtual basis::hoople_standard
  {
  public:
    time_number year;  //!< The year, using the gregorian calendar.

    time_locus() : clock_time(), day_in_year(), year() {}

    DEFINE_CLASS_NAME("time_locus");

    //! Constructs a location in time given its components.
    time_locus(const clock_time &ct, const day_in_year &ytd, time_number year_in)
            : clock_time(ct), day_in_year(ytd), year(year_in) {}

    int packed_size() const { return clock_time::packed_size()
            + day_in_year::packed_size() + structures::PACKED_SIZE_INT32; }

    virtual void pack(basis::byte_array &packed_form) const;
      //!< Packs a time_locus object into an array of bytes.
    virtual bool unpack(basis::byte_array &packed_form);
      //!< Unpacks a time_locus object from an array of bytes.

    // these implement the orderable and equalizable interfaces.
    virtual bool equal_to(const basis::equalizable &s2) const;
    virtual bool less_than(const basis::orderable &s2) const;
//old    bool operator < (const time_locus &to_compare) const;
      //!< Returns true if this time_locus is earlier than "to_compare"
//old    bool operator == (const time_locus &to_compare) const;
      //!< Returns true if this time_locus is equal to "to_compare"

    //! Enumerates the ways to show the year.
    enum locus_formats {
      LONG_YEAR = 0x1,  //!< default: full four digit year (problems in 9999).
      SHORT_YEAR = 0x2  //!< use only last two digits of year.  ugh--Y2K danger.
    };

    // fulfills obligation for text_formable.
    virtual void text_form(basis::base_string &state_fill) const {
      state_fill.assign(text_form_long(clock_time::MERIDIAN, day_in_year::SHORT_MONTH, LONG_YEAR));
    }

    basis::astring text_form_long(int t = clock_time::MERIDIAN,
            int d = day_in_year::SHORT_MONTH, int y = LONG_YEAR) const;
      //! Prints out the time_locus given the way to print each component.
      /*< "t" is a combination of time_formats, "d" is a combination of
      date_formats and "y" is a combination of locus_formats. */
    void text_form_long(basis::astring &to_stuff, int t = clock_time::MERIDIAN,
            int d = day_in_year::SHORT_MONTH, int y = LONG_YEAR) const;
      //! Same as text_form() above, but stores into "to_stuff".

    static time_number normalize(time_locus &to_fix);
      //!< normalizes the time_locus for its clock time and date.
//hmmm: what are rollovers measured in?
  };

  time_number year_now();  //!< what year is it?
  clock_time time_now();  //!< what time is it?
  day_in_year date_now();  //!< what day on the calendar is it?
  time_locus now();  //!< returns our current locus in the time continuum.
  time_locus greenwich_now();  //!< returns Greenwich Mean Time (their now).
} // namespace.

#endif

