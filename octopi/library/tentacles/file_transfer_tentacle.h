#ifndef FILE_TRANSFER_TENTACLE_CLASS
#define FILE_TRANSFER_TENTACLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : file_transfer_tentacle                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "file_transfer_infoton.h"

#include <basis/mutex.h>
#include <filesystem/directory_tree.h>
#include <filesystem/filename_list.h>
#include <octopus/tentacle_helper.h>
#include <timely/time_stamp.h>

namespace octopi {

class file_transfer_cleaner;
class file_transfer_status;

//! Manages the transferrence of directory trees from one place to another.
/*!
  Note: this is a fairly heavy-weight header due to the inclusion of the
  file transfer infoton header.  It is better to forward declare the
  objects in both file transfer headers when using the types in other
  headers.
*/

class file_transfer_tentacle
: public tentacle_helper<file_transfer_infoton>
{
public:
  enum transfer_modes {
    ONLY_REPORT_DIFFS = 0x1,  //!< no actual file transfer, just reports.
    COMPARE_SIZE_AND_TIME = 0x2,  //!< uses size and time to see differences.
    COMPARE_CONTENT_SAMPLE = 0x4,  //!< samples parts of file for comparison.
    COMPARE_ALL = 0x6  //!< compares all of the file size, file time, and contents.
  };


  file_transfer_tentacle(int maximum_transfer, transfer_modes mode_of_transfer);
    //!< constructs a tentacle for either transfers or comparisons.
    /*!< the "maximum_transfer" is the largest chunk of data we will try to
    sling across the network at a time.  the "mode_of_transfer" selects how
    to perform the operation.  if "ONLY_REPORT_DIFFS" is set, then there
    will only be a report of differences and no files will be copied.  if
    COMPARE_SIZE_AND_TIME is set, then the comparison will use the file's
    size and its access time for determining if it has changed.  if the
    COMPARE_CONTENT_SAMPLE flag is set, then the file will be sampled at some
    key locations and that will decide differences.  the comparison modes
    can be mixed together.  if there are no comparison modes, then the files
    will always be copied. */

  virtual ~file_transfer_tentacle();

  DEFINE_CLASS_NAME("file_transfer_tentacle");

  basis::astring text_form() const;
    //!< returns a string representing the current state of transfers.

  filesystem::directory_tree *lock_directory(const basis::astring &source_mapping);
    //!< provides a view of the tentacle's current state.
  void unlock_directory();
    //!< unlock MUST be called when one is done looking at the tree.

  // these methods are for the "server" side--the side that has files to offer.

  basis::outcome add_correspondence(const basis::astring &source_mapping,
          const basis::astring &source_root, int refresh_interval);
    //!< adds a file transfer correspondence.
    /*!< this is a "source_mapping" which is a short string that is made
    available to the other side for transfer requests.  when they specify the
    "source_mapping", it will be translated on this side to the "source_root",
    which must be a valid filesystem path.  the "refresh_interval" dictates
    how frequently, in milliseconds, the source will be scanned to update the
    internal directory tree.  this is done the first time the "source_mapping"
    is set up also.  if a previous identical "source_mapping" existed, then it
    is removed and replaced with the information from the new invocation. */

  basis::outcome remove_correspondence(const basis::astring &source_mapping);
    //!< takes out the "source_mapping" which was previously added.
    /*!< this keeps any transfers from occurring on that name, and will cause
    aborted transfers if any were still ongoing. */

  basis::outcome refresh_now(const basis::astring &source_mapping);
    //!< refreshes the "source_mapping" right now, regardless of the interval.
    /*!< the mapping must already have been created with add_correspondence().
    */

  bool add_path(const basis::astring &source_mapping, const basis::astring &new_path);
    //!< inserts the "new_path" into a registered correspondence.
    /*!< the "source_mapping" must already be registered. */

  bool remove_path(const basis::astring &source_mapping, const basis::astring &old_path);
    //!< deletes the "old_path" out of an existing correspondence.

  // these methods are for the client side--the side that wants to get files.

  basis::outcome register_file_transfer(const octopus_entity &ent,
          const basis::astring &src_root, const basis::astring &dest_root,
          const structures::string_array &include);
    //!< records a transfer that is going to commence.
    /*!< the side that wishes to download files must invoke this before
    starting the transfer.  if the "include" list is not empty, then only
    those files will be transferred.  they have to match the suffix of the
    files that would have been transferred and wildcards are not currently
    supported. */

  basis::outcome cancel_file_transfer(const octopus_entity &ent,
          const basis::astring &src_root, const basis::astring &dest_root);
    //!< tosses a previously registered file transfer.
    /*!< this will be done automatically after a time-out period, but it is
    better to clean it up as soon as one is finished with the transfer. */

  bool status(const octopus_entity &ent, const basis::astring &src,
        const basis::astring &dest, double &total_size, int &total_files,
        double &current_size, int &current_files, bool &done,
        timely::time_stamp &last_active);
    //!< locates the transfer specified and returns information about it.
    /*!< the transfer is designated by the "ent", "src" and "dest" parameters
    and returns the current progress.  files refers to how many files are
    being transferred and size refers to their combined weight in bytes.  the
    "done" flag is set if the transfer seems finished.  note that this will
    not set any values until the first reply comes back from the server. */

  bool get_differences(const octopus_entity &ent, const basis::astring &src,
        const basis::astring &dest, filesystem::filename_list &diffs);
    //!< accesses the list of difference for an ongoing transfer.
    /*!< the progress is stored in "diffs". */

  // required tentacle methods...

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, infoton * &reformed);
    //!< recreates a "reformed" infoton from its packed form.
    /*!< this requires the "classifier" and packed infoton data in
    "packed_form".  this will only succeed if the classifier's first name
    is understood here. */

  virtual basis::outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          basis::byte_array &transformed);
    //!< processes the "to_chow" infoton as a file transfer request.

  virtual void expunge(const octopus_entity &to_remove);
    //!< throws out any transfers occurring for the entity "to_remove".

  // internal use only.

  void periodic_actions();  //!< drops timed out transfers.

private:
  int _maximum_transfer;  //!< largest chunk to send at a time.
  file_transfer_status *_transfers;  //!< our record of ongoing transfers.
  file_transfer_status *_correspondences;  //!< the synonyms for mapping.
  basis::mutex *_lock;  //!< protects our lists.
  file_transfer_cleaner *_cleaner;  //!< cleans up dead transfers.
  int _mode;  //!< how will the comparison be done?

  // these process the request and response infotons that are passed to us.
  basis::outcome handle_tree_compare_request(file_transfer_infoton &req,
          const octopus_request_id &item_id);
  basis::outcome handle_tree_compare_response(file_transfer_infoton &req,
          const octopus_request_id &item_id);
  basis::outcome handle_storage_request(file_transfer_infoton &req,
          const octopus_request_id &item_id);
  basis::outcome handle_storage_response(file_transfer_infoton &req,
          const octopus_request_id &item_id);
  basis::outcome conclude_storage_request(file_transfer_infoton &req,
          const octopus_request_id &item_id);
  basis::outcome conclude_storage_response(file_transfer_infoton &req,
          const octopus_request_id &item_id);
};

} //namespace.

#endif

