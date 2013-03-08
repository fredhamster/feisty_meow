#ifndef HASH_TABLE_CLASS
#define HASH_TABLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : hash_table                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "amorph.h"

#include <basis/array.h>
#include <basis/byte_array.h>
#include <basis/contracts.h>
#include <basis/enhance_cpp.h>
#include <basis/functions.h>

#include <math.h>

namespace structures {

// forward.
template <class key, class contents> class internal_hash_array;

//! A hashing algorithm takes a key and derives a related integer from it.
/*!
  The hashing_algorithm is used in hash_tables for placing objects into slots
  that can be easily found again.  The numerical hash should be faster than
  actually doing a search on what might be unsorted data otherwise.
*/

class hashing_algorithm : public virtual basis::clonable
{
public:
  virtual basis::un_int hash(const void *key_data, int key_length) const = 0;
    //!< returns a hash value based on the "key_data" and "key_length".
    /*!< the "key_length" is provided from the sizeof() of the key type used
    in the hash_table (below).  it is up to the implementor to create a hash
    value that spreads the keys to be hashed appropriately.  if similar keys
    create same or similar hash values, then the efficiency of the hash_table
    is compromised. */

  virtual hashing_algorithm *clone() const = 0;
    //!< supports virtual construction of new algorithm objects.
};

//////////////

//! Implements hashing into buckets for quick object access.
/*!
  The buckets are currently simple lists, but if the hashing algorithm is well
  chosen, then that's not a major problem.  This makes lookups a lot faster
  than a linear search, but no particular performance guarantees are made at
  this time.
*/

template <class key_type, class contents>
  // the key_type must support valid copy constructor, assignment operator and
  // equality operators.  the contents need not support any special operations;
  // it is always considered as just a pointer.
class hash_table : public virtual basis::nameable
{
public:
  hash_table(const hashing_algorithm &hasher, int estimated_elements);
    //!< Creates a table using the "hasher" that is ready to store "estimated_elements".
    /*!< the "hasher" must provide the hashing algorithm for the two types
    specified.  if the implementation is thread safe, then one object can
    be constructed to use with all the same types of hash tables.
    note that "hasher" must exist longer than any classes based on it; do
    not let "hasher" go out of scope or the hash table will explode. */

  virtual ~hash_table();
    //!< destroys any objects left in the hash_table.

  DEFINE_CLASS_NAME("hash_table");

  void rehash(int estimated_elements);
    //!< resizes the hashing structures to optimise for a new size of "estimated_elements".
    /*!< this is a potentially expensive operation. */

  enum outcomes {
    IS_NEW = basis::common::IS_NEW,
    EXISTING = basis::common::EXISTING
  };

  static int calculate_num_slots(int estimated_elements);
    //!< a helper method that lets us know what n is for how many 2^n slots we should have.

  int elements() const;
    //!< the number of valid items we found by traversing the hash table.
    /*!< this is a very costly method. */

  int estimated_elements() const { return c_estim_elements; }
    //!< returns the size of table we're optimized for.

  basis::outcome add(const key_type &key, contents *to_store);
    //!< Stores "to_store" into the table given its "key" for hashing.
    /*!< This places an entry in the hash table with the contents "to_store",
    using the "key" structure to identify it.  the return value reports whether
    the "key" was already in the list or not.  if the "key" was already in use,
    then the contents for it get replaced with "to_store".  note that the hash
    table assumes ownership of the "to_store" object but just makes a copy of
    the key.  thus, if an item is replaced, the previous contents are
    destroyed. */

  basis::outcome fast_dangerous_add(const key_type &key, contents *to_store);
    //!< Like the add method above, but doesn't check for duplicates.
    /*!< This should only ever be used when one has already guaranteed that
    the table doesn't have a duplicate item for the "key" specified. */

  bool find(const key_type &key, contents * &item_found) const;
    //!< locates the item specified by the "key", if possible.
    /*!< true is returned on success and the "item_found" is filled in.  the
    "item_found" is a pointer to the actual data held in the table, so do not
    destroy or otherwise damage the "item_found". */

  contents *find(const key_type &key) const
          { contents *c = NIL; return find(key, c)? c : NIL; }
    //!< simplified form of above find() method.

  contents *acquire(const key_type &key);
    //!< retrieves the contents held for "key" out of the table.
    /*!< afterwards, the contents pointer is the caller's responsibility; it
    is no longer in the table and must be destroyed externally.  if no item
    was found for the "key", then NIL is returned. */

  bool zap(const key_type &key);
    //!< removes the entry with the "key" specified.
    /*!<  true is returned if the item was found and destroyed. */

  void reset();
    //!< removes all entries in the table and returns it to a pristine state.

  typedef bool apply_function(const key_type &key, contents &current,
          void *data_link);
    //!< the "apply_function" is what a user of the "apply" method must supply.
    /*!< the function will be called on every item in the table unless one of
    the invocations returns false; this causes the apply process to stop.
    the "data_link" provides a way for the function to refer back to an
    parent class of some sort. */

  void apply(apply_function *to_apply, void *data_link);
    //!< Invokes the function "to_apply" on every entry in the table.
    /*!< This calls the "to_apply" function on every item in the catalog
    until the function returns false.  The "data_link" pointer is available to
    the applied method and can be used to communicate an object for use during
    the iteration.  NOTE: it is NOT safe to rearrange or manipulate the table
    in any way from your "to_apply" function. */

  basis::outcome add(key_type *key, contents *to_store, bool check_dupes = true);
    //!< specialized add for a pre-existing pointer "key".
    /*!< responsibility for the "key" is taken over by the hash table, as of
    course it is for the "to_store" pointer.  this just allows others to
    generate new keys and hand them over to us when it might otherwise be
    very costly to copy the key structure.  if "check_dupes" is not true,
    then that asserts that you have independently verified that there's no
    need to check whether the key is already present. */

  bool verify() const;
    //!< returns true if the hash table is internally consistent.
    /*!< this is mainly for debugging. */

private:
  int c_estim_elements;  //!< expected running size for the hash table.
  hashing_algorithm *_hasher;  //!< algorithm for getting hash value.
  internal_hash_array<key_type, contents> *_table;  //!< storage area.
  int _last_iter;
    //!< tracks where we left off iterating.  we restart just after that spot.

  hash_table(const hash_table &to_copy);
    //!< not allowed; use the copy_hash_table function below.
  hash_table &operator =(const hash_table &to_copy);
    //!< not allowed; use the copy_hash_table function below.

public:
  internal_hash_array<key_type, contents> &table_access() const;
    //!< special accessor for the copy_hash_table method only.
};

//////////////

//! Copies the entire hash table, given a contents type that supports copying.
/*!
  Provides a copy operation on hash tables where the contents object supports
  a copy constructor, which is not appropriate to require in general.  The
  hash_table held in "target" will be wiped out and replaced with items
  equivalent to those in "source". */

template <class key_type, class contents>
void copy_hash_table(hash_table<key_type, contents> &target,
    const hash_table<key_type, contents> &source);

//////////////

// implementations for longer methods below....

// this is a very special micro-managed class.  it holds two pointers which
// it neither creates nor destroys.  thus all interaction with it must be
// careful about removing these objects at the appropriate time.  we don't
// want automatic memory management since we want a cheap copy on the items
// in the buckets.

template <class key_type, class contents>
class hash_wrapper : public virtual basis::root_object
{
public:
  key_type *_id;
  contents *_data;

  hash_wrapper(key_type *id = NIL, contents *data = NIL)
      : _id(id), _data(data) {}
};

//////////////

template <class key_type, class contents>
class bucket
    : public basis::array<hash_wrapper<key_type, contents> >,
      public virtual basis::root_object
{
public:
  bucket() : basis::array<hash_wrapper<key_type, contents> >(0, NIL,
      basis::byte_array::SIMPLE_COPY | basis::byte_array::EXPONE
      | basis::byte_array::FLUSH_INVISIBLE) {}

  int find(const key_type &to_find) {
    for (int i = 0; i < this->length(); i++) {
      key_type *curr_key = this->get(i)._id;
//hmmm: if curr key is not set, is that a logic error?  it seems like we
//      are seeing the potential for this.
      if (curr_key && (to_find == *curr_key))
        return i;
    }
    return basis::common::NOT_FOUND;
  }
};

//////////////

template <class key_type, class contents>
class internal_hash_array : public amorph<bucket<key_type, contents> >
{
public:
  internal_hash_array(int estimated_elements)
      : amorph<bucket<key_type, contents> >
           (hash_table<key_type, contents>::calculate_num_slots(estimated_elements)) {}
};

//////////////

template <class key_type, class contents>
hash_table<key_type, contents>::hash_table(const hashing_algorithm &hasher, int estimated_elements)
: c_estim_elements(estimated_elements),
  _hasher(hasher.clone()),
  _table(new internal_hash_array<key_type, contents>(estimated_elements)),
  _last_iter(0)
{}

template <class key_type, class contents>
hash_table<key_type, contents>::~hash_table()
{
#ifdef EXTREME_CHECKING
  FUNCDEF("destructor");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  reset();
  basis::WHACK(_table);
  basis::WHACK(_hasher);
}

template <class key_type, class contents>
int hash_table<key_type, contents>::calculate_num_slots(int estimated_elements)
{
//printf("elems wanted = %d\n", estimated_elements);
  int log_2_truncated = int(log(float(estimated_elements)) / log(2.0));
//printf("log 2 of elems, truncated = %d\n", log_2_truncated);
  int slots_needed_for_elems = (int)pow(2.0, double(log_2_truncated + 1));
//printf("slots needed = %d\n", slots_needed_for_elems );
  return slots_needed_for_elems;
}

// the specialized copy operation.
template <class key_type, class contents>
void copy_hash_table(hash_table<key_type, contents> &target,
    const hash_table<key_type, contents> &source)
{
#ifdef EXTREME_CHECKING
  FUNCDEF("copy_hash_table");
  if (!source.verify())
    deadly_error(class_name(), func, "source state did not verify.");
#endif
  target.reset();
  for (int i = 0; i < source.table_access().elements(); i++) {
    bucket<key_type, contents> *buck = source.table_access().borrow(i);
    if (!buck) continue;
    for (int j = 0; j < buck->length(); j++) {
      target.add(*((*buck)[j]._id), new contents(*((*buck)[j]._data)));
    }
  }
#ifdef EXTREME_CHECKING
  if (!target.verify())
    deadly_error(class_name(), func, "target state did not verify.");
#endif
  #undef class_name
}

template <class key_type, class contents>
void hash_table<key_type, contents>::reset()
{
#ifdef EXTREME_CHECKING
  FUNCDEF("reset");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  // iterate over the list whacking the content items in the buckets.
  for (int i = 0; i < _table->elements(); i++) {
    bucket<key_type, contents> *buck = _table->acquire(i);
    if (!buck) continue;
    for (int j = 0; j < buck->length(); j++) {
      basis::WHACK((*buck)[j]._data);  // eliminate the stored data.
      basis::WHACK((*buck)[j]._id);  // eliminate the stored key.
      buck->zap(j, j);  // remove the element.
      j--;  // skip back before whack happened.
    }
    basis::WHACK(buck);
  }
#ifdef EXTREME_CHECKING
  if (!verify())
    deadly_error(class_name(), func, "state did not verify afterwards.");
#endif
}

template <class key_type, class contents>
bool hash_table<key_type, contents>::verify() const
{
  for (int i = 0; i < _table->elements(); i++) {
    const bucket<key_type, contents> *buck = _table->borrow(i);
    if (!buck) continue;  // that's acceptable.
    for (int j = 0; j < buck->length(); j++) {
      const hash_wrapper<key_type, contents> &wrap = (*buck)[j];
      if (!wrap._data) {
//        program_wide_logger::get().log(a_sprintf("hash table: no data segment at position %d.", j));
        return false;
      }
      if (!wrap._id) {
//        program_wide_logger::get().log(a_sprintf("hash table: no identifier at position %d.", j));
        return false;
      }
    }
  }
  return true;
}

template <class key_type, class contents>
internal_hash_array<key_type, contents> &hash_table<key_type, contents>
    ::table_access() const
{ return *_table; }

template <class key_type, class contents>
void hash_table<key_type, contents>::rehash(int estimated_elements)
{
#ifdef EXTREME_CHECKING
  FUNCDEF("rehash");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  typedef bucket<key_type, contents> buckie;
  hash_table<key_type, contents> new_hash(*_hasher, estimated_elements);
    // this is the new table that will be used.

  // scoot through the existing table so we can move items into the new one.
  for (int i = 0; i < _table->elements(); i++) {
    buckie *b = _table->borrow(i);  // look at the current element.
    if (!b) continue;  // nothing here, keep going.
    for (int j = b->length() - 1; j >= 0; j--) {
      key_type *k = b->use(j)._id;
      contents *c = b->use(j)._data;
      new_hash.add(k, c);
    }
    b->reset();
      // clean out any items in the buckets here now that they've moved.
  }

  // now flip the contents of the new guy into "this".
  _table->reset();  // toss previous stuff.
  _table->adjust(new_hash._table->elements());
  for (int q = 0; q < new_hash._table->elements(); q++)
    _table->put(q, new_hash._table->acquire(q));
  // reset other data members.
  c_estim_elements = new_hash.c_estim_elements;
  _last_iter = 0;
#ifdef EXTREME_CHECKING
  if (!verify())
    deadly_error(class_name(), func, "state did not verify afterwards.");
#endif
}

template <class key_type, class contents>
basis::outcome hash_table<key_type, contents>::add(const key_type &key,
    contents *to_store)
{ return add(new key_type(key), to_store); }

template <class key_type, class contents>
basis::outcome hash_table<key_type, contents>::add(key_type *key,
    contents *to_store, bool check_dupes)
{
#ifdef EXTREME_CHECKING
  FUNCDEF("add");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  // get a hash value.
  basis::un_int hashed = _hasher->hash((const void *)key, sizeof(key_type));
  // make the value appropriate for our table.
  hashed = hashed % _table->elements();
  // see if the key already exists there.
  bucket<key_type, contents> *buck = _table->borrow(hashed);
  if (!buck) {
    // this slot doesn't exist yet.
    buck = new bucket<key_type, contents>;
    _table->put(hashed, buck);
  }
  if (!check_dupes) {
    // we aren't even going to check for its existence.
    *buck += hash_wrapper<key_type, contents>(key, to_store);
    return IS_NEW;
  }
  int indy = buck->find(*key);
  if (basis::negative(indy)) {
    // that value was not seen yet, so we're adding a new entry.
    *buck += hash_wrapper<key_type, contents>(key, to_store);
#ifdef EXTREME_CHECKING
    if (!verify())
      deadly_error(class_name(), func, "state did not verify afterwards.");
#endif
    return IS_NEW;
  }
  // that key already existed, so we'll re-use its slot with the new data.
  basis::WHACK((*buck)[indy]._data);
  basis::WHACK(key);  // toss since we're not using.
  (*buck)[indy]._data = to_store;
#ifdef EXTREME_CHECKING
  if (!verify())
    deadly_error(class_name(), func, "state did not verify afterwards.");
#endif
  return EXISTING;
}

template <class key_type, class contents>
basis::outcome hash_table<key_type, contents>::fast_dangerous_add
    (const key_type &key, contents *to_store)
{ return add(new key_type(key), to_store, false); }

template <class key_type, class contents>
bool hash_table<key_type, contents>::find(const key_type &key,
    contents * &item_found) const
{
#ifdef EXTREME_CHECKING
  FUNCDEF("find");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  item_found = NIL;
  // get a hash value.
  basis::un_int hashed = _hasher->hash((const void *)&key, sizeof(key));
  // make the value appropriate for our table.
  hashed = hashed % _table->elements();
  // see if the key exists in the bucket.
  bucket<key_type, contents> *buck = _table->borrow(hashed);
  if (!buck) return false;
  int indy = buck->find(key);
  if (basis::negative(indy)) return false;  // not there.
  // was there, so retrieve the data.
  item_found = (*buck)[indy]._data;
  return true;
}

template <class key_type, class contents>
contents *hash_table<key_type, contents>::acquire(const key_type &key)
{
#ifdef EXTREME_CHECKING
  FUNCDEF("acquire");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  // get a hash value.
  basis::un_int hashed = _hasher->hash((const void *)&key, sizeof(key));
  // make the value appropriate for our table.
  hashed = hashed % _table->elements();
  // see if the key exists in the bucket.
  bucket<key_type, contents> *buck = _table->borrow(hashed);
  if (!buck) return NIL;
  int indy = buck->find(key);
  if (basis::negative(indy)) return NIL;  // nope, not there.
  contents *to_return = (*buck)[indy]._data;
  basis::WHACK((*buck)[indy]._id);  // clean the id.
  buck->zap(indy, indy);  // toss the storage blob for the item.
#ifdef EXTREME_CHECKING
  if (!verify())
    deadly_error(class_name(), func, "state did not verify afterwards.");
#endif
  return to_return;
}

template <class key_type, class contents>
bool hash_table<key_type, contents>::zap(const key_type &key)
{
#ifdef EXTREME_CHECKING
  FUNCDEF("zap");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  // get a hash value.
  basis::un_int hashed = _hasher->hash((const void *)&key, sizeof(key));
  // make the value appropriate for our table.
  hashed = hashed % _table->elements();
  // see if the key exists in the bucket.
  bucket<key_type, contents> *buck = _table->borrow(hashed);
  if (!buck) return false;
  int indy = buck->find(key);
  if (basis::negative(indy)) {
    // nope, not there.
    return false;
  }
  basis::WHACK((*buck)[indy]._data);  // delete the data held.
  basis::WHACK((*buck)[indy]._id);  // delete the data held.
  buck->zap(indy, indy);  // toss the storage blob for the item.
  if (!buck->length()) {
    // clean up this bucket now.
    buck = _table->acquire(hashed);
    basis::WHACK(buck);
  }
#ifdef EXTREME_CHECKING
  if (!verify())
    deadly_error(class_name(), func, "state did not verify afterwards.");
#endif
  return true;
}

template <class key_type, class contents>
void hash_table<key_type, contents>::apply(apply_function *to_apply,
    void *data_link)
{
#ifdef EXTREME_CHECKING
  FUNCDEF("apply");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  typedef bucket<key_type, contents> buckie;
  int bucks_seen = 0;
  int posn = _last_iter;  // start at the same place we left.
  while (bucks_seen++ < _table->elements()) {
    if ( (posn < 0) || (posn >= _table->elements()) )
      posn = 0;
    buckie *b = _table->borrow(posn);
    _last_iter = posn++;
      // record where the iteration last touched and increment next position.
      // we must do this before we check if the bucket exists or we'll rotate
      // on this same place forever.
    if (!b) continue;  // nothing here, keep going.
    for (int j = 0; j < b->length(); j++) {
      contents *c = b->use(j)._data;
      if (!c) {
#ifdef EXTREME_CHECKING
        deadly_error("hash_table", "apply", "logic error--missing pointer");
#endif
        continue;
      }
      if (!to_apply(*b->use(j)._id, *c, data_link)) return;
    }
  }
}

template <class key_type, class contents>
int hash_table<key_type, contents>::elements() const
{
#ifdef EXTREME_CHECKING
  FUNCDEF("elements");
  if (!verify()) deadly_error(class_name(), func, "state did not verify.");
#endif
  typedef bucket<key_type, contents> buckie;
  int to_return = 0;
  for (int i = 0; i < _table->elements(); i++) {
    bucket<key_type, contents> *buck = _table->borrow(i);
    if (!buck) continue;  // nothing to count.
    to_return += buck->length();
  }
  return to_return;
}

#undef static_class_name

} //namespace.

#endif // outer guard.

