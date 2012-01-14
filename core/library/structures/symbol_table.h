#ifndef SYMBOL_TABLE_CLASS
#define SYMBOL_TABLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : symbol_table                                                      *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "pointer_hash.h"
#include "string_hash.h"
#include "symbol_table.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>

namespace structures {

template <class contents> class internal_symbol_indexer;
template <class contents> class internal_symbol_info;
template <class contents> class internal_symbol_list;

//! Maintains a list of names, where each name has a type and some contents.

template <class contents>
class symbol_table
{
public:
  //! constructs a symbol table with sufficient size for "estimated_elements".
  /*! the "estimated_elements" dictates how large the symbol table's key space is.  the
  number of keys that can be stored without collisions (assuming perfect distribution
  from the hash function) will be close to the number of elements specified. */
  symbol_table(int estimated_elements = 100);

  symbol_table(const symbol_table<contents> &to_copy);

  ~symbol_table();

  int symbols() const;
    //!< returns the number of symbols listed in the table.

  int estimated_elements() const;
    //!< returns the number of symbols the table is optimized for.

  void rehash(int estimated_elements);
    //!< resizes underlying table to support "estimated_elements".

  void hash_appropriately(int estimated_elements);
    //!< Resizes the number of table slots to have space for "estimated_elements".

  symbol_table &operator =(const symbol_table<contents> &to_copy);

  basis::outcome add(const basis::astring &name, const contents &storage);
    //!< Enters a symbol name into the table along with some contents.
    /*!< If the name already exists in the table, then the previous contents
    are replaced with these, but EXISTING is returned.  If this is a new entry,
    then IS_NEW is returned instead. */

  const basis::astring &name(int index) const;
    //!< returns the name held at the "index".
    /*!< if the index is invalid, then a bogus name is returned. */

  void names(string_set &to_fill) const;
    //!< returns the names of all the symbols currently held.

  contents &operator [] (int index);
    //!< provides access to the symbol_table's contents at the "index".
  const contents &operator [] (int index) const;
    //!< provides a constant peek at the contents at the "index".

  const contents &get(int index) const { return operator[](index); }
    //!< named equivalent for the bracket operator.
  contents &use(int index) { return operator[](index); }
    //!< named equivalent for the bracket operator.

  contents *find(const basis::astring &name) const;
    //!< returns the contents held for "name" or NIL if it wasn't found.

  contents *find(const basis::astring &name,
          basis::string_comparator_function *comparator) const;
    //!< Specialized search via a comparison method "comparator".
    /*!< Searches for a symbol by its "name" but uses a special comparison
    function "comparator" to determine if the name is really equal.  This
    method is by its nature slower than the main find method, since all buckets
    must be searched until a match is found.  It is just intended to provide
    extensibility. */

  int dep_find(const basis::astring &name) const;
    //!< Searches for a symbol by its "name".
    /*!< Returns the index or NOT_FOUND.  NOTE: this is deprecated; it is far
    faster to use the first find method above. */

  //! Locates the symbol at position "index" and stores it to the parameters.
  /*! retrieve() accesses the "index"th symbol in the table and returns the
      held contents for it as well as its name.  if the outcome is OKAY, then
      the returned information is valid.  otherwise, the search failed. */
  basis::outcome retrieve(int index, basis::astring &symbol_name, contents &contains) const;

  basis::outcome whack(const basis::astring &name);
    //!< removes a symbol from the table.
    /*!< this succeeds only if the symbol name is present in the table. */

  basis::outcome zap_index(int index);
    //!< zaps the entry at the specified index.  slower than whack().

  void reset();  //<! removes all symbols from the table.

private:
  internal_symbol_list<contents> *_symbol_list;  //!< our table of symbols.
  internal_symbol_indexer<contents> *_tracker;  //!< indexed lookup support.

  internal_symbol_info<contents> *get_index(int index) const;
    //!< returns the info item at "index" or NIL.
};

//////////////

template <class contents>
bool symbol_table_compare(const symbol_table<contents> &a,
    const symbol_table<contents> &b);
  //!< returns true if table "a" and table "b" have the same contents.

//////////////

// implementations below...

//#define DEBUG_SYMBOL_TABLE
  // uncomment for noisier debugging.

#ifdef DEBUG_SYMBOL_TABLE
  #undef LOG
  #define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)
#endif

//////////////

// this structure keeps track of our symbol table elements.

template <class contents>
class internal_symbol_info
{
public:
  contents _content;  // the object provided by the user.
  basis::astring _name;  // symbol's name.

  internal_symbol_info(const contents &content, const basis::astring &name)
  : _content(content), _name(name) {}
};

//////////////

// this is our tracking system that allows us to offer array like services.
// each symbol held has an address as one of our wrappers.  thus we can
// list those addresses in a hash table along with listing the contents
// in the main hash table.  the pointer_hash gives us a list of ids set, so
// we can have some ordering for the items in the table.

template <class contents>
class internal_pointer_hider
{
public:
  internal_symbol_info<contents> *_held;

  internal_pointer_hider(internal_symbol_info<contents> *held) : _held(held) {}
};

template <class contents>
class internal_symbol_indexer
: public pointer_hash<internal_pointer_hider<contents> >
{
public:
  internal_symbol_indexer(int elems)
      : pointer_hash<internal_pointer_hider<contents> >(elems) {}
};

//////////////

// this object maintains the symbol table's contents.

template <class contents>
class internal_symbol_list
: public string_hash<internal_symbol_info<contents> >
{
public:
  internal_symbol_list(int elems)
      : string_hash<internal_symbol_info<contents> >(elems) {}
};

//////////////

// the main algorithms class implementing the external interface.

#undef static_class_name
#define static_class_name() "symbol_table"

template <class contents>
symbol_table<contents>::symbol_table(int estimated_elements)
: _symbol_list(new internal_symbol_list<contents>(estimated_elements)),
  _tracker(new internal_symbol_indexer<contents>(estimated_elements))
{}

template <class contents>
symbol_table<contents>::symbol_table(const symbol_table<contents> &to_copy)
: _symbol_list(new internal_symbol_list<contents>
      (to_copy._symbol_list->estimated_elements())),
  _tracker(new internal_symbol_indexer<contents>(to_copy.estimated_elements()))
{ *this = to_copy; }

template <class contents>
symbol_table<contents>::~symbol_table()
{
  WHACK(_symbol_list);
  WHACK(_tracker);
}

template <class contents>
int symbol_table<contents>::estimated_elements() const
{ return _symbol_list->estimated_elements(); }

template <class contents>
void symbol_table<contents>::rehash(int estimated_elements)
{
  _symbol_list->rehash(estimated_elements);
  _tracker->rehash(estimated_elements);
}

template <class contents>
void symbol_table<contents>::hash_appropriately(int new_elements)
{ rehash(new_elements); }

template <class contents>
int symbol_table<contents>::symbols() const
{ return _tracker->ids().elements(); }

template <class contents>
symbol_table<contents> &symbol_table<contents>::
    operator =(const symbol_table &to_copy)
{
  if (this == &to_copy) return *this;
  reset();
  for (int i = 0; i < to_copy.symbols(); i++) {
    internal_symbol_info<contents> *info = to_copy.get_index(i);
    if (info) {
      internal_symbol_info<contents> *new_info
          = new internal_symbol_info<contents>(*info);
      _symbol_list->add(info->_name, new_info);
      internal_pointer_hider<contents> *new_track = 
          new internal_pointer_hider<contents>(new_info);
      _tracker->add(new_info, new_track);
    }
  }
  return *this;
}

template <class contents>
void symbol_table<contents>::reset()
{
  _symbol_list->reset();
  _tracker->reset();
}

template <class contents>
const basis::astring &symbol_table<contents>::name(int index) const
{
  bounds_return(index, 0, symbols() - 1, basis::bogonic<basis::astring>());
  return get_index(index)->_name;
}

template <class contents>
void symbol_table<contents>::names(string_set &to_fill) const
{
  to_fill.reset();
  for (int i = 0; i < symbols(); i++)
    to_fill += get_index(i)->_name;
}

template <class contents>
internal_symbol_info<contents> *symbol_table<contents>::
    get_index(int index) const
{
  bounds_return(index, 0, symbols() - 1, NIL);
  return _tracker->find(_tracker->ids()[index])->_held;
}

template <class contents>
const contents &symbol_table<contents>::operator [] (int index) const
{
  bounds_return(index, 0, symbols() - 1, basis::bogonic<contents>());
  internal_symbol_info<contents> *found = get_index(index);
  if (!found) return basis::bogonic<contents>();
  return found->_content;
}

template <class contents>
contents &symbol_table<contents>::operator [] (int index)
{
  bounds_return(index, 0, symbols() - 1, basis::bogonic<contents>());
  internal_symbol_info<contents> *found = get_index(index);
  if (!found) return basis::bogonic<contents>();
  return found->_content;
}

template <class contents>
contents *symbol_table<contents>::find(const basis::astring &name) const
{ 
//  FUNCDEF("find [name]");
  internal_symbol_info<contents> *found = _symbol_list->find(name);
  if (!found) return NIL;
  return &found->_content;
}

template <class contents>
int symbol_table<contents>::dep_find(const basis::astring &name) const
{
  internal_symbol_info<contents> *entry = _symbol_list->find(name);
  if (!entry) return basis::common::NOT_FOUND;

  for (int i = 0; i < symbols(); i++) {
    if (_tracker->ids()[i] == entry) return i;
  }
  return basis::common::NOT_FOUND;  // this is bad; it should have been found.
}

template <class contents>
struct sym_tab_apply_data
{
  basis::string_comparator_function *_scf;
  contents *_found;
  basis::astring _to_find;

  sym_tab_apply_data() : _scf(NIL), _found(NIL) {}
};

template <class contents>
bool sym_tab_finder_apply(const basis::astring &key, contents &current,
    void *data_link)
{
#ifdef DEBUG_SYMBOL_TABLE
  FUNCDEF("sym_tab_finder_apply");
#endif
  sym_tab_apply_data<contents> *package
      = (sym_tab_apply_data<contents> *)data_link;
#ifdef DEBUG_SYMBOL_TABLE
  LOG(basis::astring("  checking ") + key);
#endif
  bool equals = package->_scf(key, package->_to_find);
  if (equals) {
    package->_found = &current;
    return false;  // done.
  }
  return true;  // keep going.
}

template <class contents>
contents *symbol_table<contents>::find(const basis::astring &name,
    basis::string_comparator_function *comparator) const
{
#ifdef DEBUG_SYMBOL_TABLE
  FUNCDEF("find [comparator]");
#endif
  if (!comparator) return find(name);  // devolve to simplified call.
#ifdef DEBUG_SYMBOL_TABLE
  LOG(basis::astring("looking for ") + name);
#endif
  sym_tab_apply_data<contents> pack;
  pack._to_find = name;
  pack._scf = comparator;
  // iterate across all the items in the hash.
  _symbol_list->apply(sym_tab_finder_apply, &pack);
  return pack._found;
}

template <class contents>
basis::outcome symbol_table<contents>::add(const basis::astring &name, const contents &to_add)
{
//  FUNCDEF("add");
  internal_symbol_info<contents> *found = _symbol_list->find(name);
  if (!found) {
    // not there already.
    internal_symbol_info<contents> *new_item
        = new internal_symbol_info<contents>(to_add, name);
    _symbol_list->add(name, new_item);
    internal_pointer_hider<contents> *new_track = 
        new internal_pointer_hider<contents>(new_item);
    _tracker->add(new_item, new_track);
    return basis::common::IS_NEW;
  }
  // overwrite the existing contents.
  found->_content = to_add;
  return basis::common::EXISTING;
}

template <class contents>
basis::outcome symbol_table<contents>::zap_index(int index)
{
  basis::astring dead_name = name(index);
  return whack(dead_name);
}

template <class contents>
basis::outcome symbol_table<contents>::whack(const basis::astring &name)
{
//  FUNCDEF("whack");
  internal_symbol_info<contents> *sep_ind = _symbol_list->find(name);
    // we need this pointer so we can find the tracker entry easily.
  bool found_it = _symbol_list->zap(name);
  if (found_it) {
    _tracker->zap(sep_ind);
  }
  return found_it? basis::common::OKAY : basis::common::NOT_FOUND;
}

template <class contents>
basis::outcome symbol_table<contents>::retrieve(int index, basis::astring &name,
    contents &got) const
{
  bounds_return(index, 0, symbols() - 1, basis::common::NOT_FOUND);
  internal_symbol_info<contents> *found = get_index(index);
  name = found->_name;
  got = found->_content;
  return basis::common::OKAY;
}

//////////////

template <class contents>
bool symbol_table_compare(const symbol_table<contents> &a,
    const symbol_table<contents> &b)
{
//  FUNCDEF("symbol_table_compare");

  string_set names_a;
  a.names(names_a);
  string_set names_b;
  b.names(names_b);
  if (names_a != names_b) return false;

  for (int i = 0; i < names_a.elements(); i++) {
    const basis::astring &current_key = names_a[i];
    const contents *a_value = a.find(current_key);
    const contents *b_value = b.find(current_key);
    if (!a_value || !b_value) continue;  // not good.
    if (*a_value != *b_value) return false;
  }
  return true;
}

#ifdef DEBUG_SYMBOL_TABLE
  #undef LOG
#endif

#undef static_class_name

} //namespace.

#endif


