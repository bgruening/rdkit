//
// Copyright (C) 2003-2021 Greg Landrum and other RDKit contributors
//
//  @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
/*! \file Dict.h

  \brief Defines the Dict class

*/
#include <RDGeneral/export.h>
#ifndef RD_DICT_H_012020
#define RD_DICT_H_012020

#include <map>
#include <string>
#include <vector>
#include "RDValue.h"
#include "Exceptions.h"
#include <RDGeneral/BoostStartInclude.h>
#include <boost/lexical_cast.hpp>
#include <RDGeneral/BoostEndInclude.h>

namespace RDKit {
typedef std::vector<std::string> STR_VECT;

//! \brief The \c Dict class can be used to store objects of arbitrary
//!        type keyed by \c strings.
//!
//!  The actual storage is done using \c RDValue objects.
//!
class RDKIT_RDGENERAL_EXPORT Dict {
 public:
  struct Pair {
    std::string key;
    RDValue val;

    Pair() : key(), val() {}
    explicit Pair(std::string s) : key(std::move(s)), val() {}
    Pair(std::string s, const RDValue &v) : key(std::move(s)), val(v) {}
    // In the case you are holding onto an rdvalue outside of a dictionary
    //  or other container, you kust call cleanup to release non POD memory.
    void cleanup() { RDValue::cleanup_rdvalue(val); }
  };

  typedef std::vector<Pair> DataType;

  Dict() {}

  Dict(const Dict &other) : _data(other._data) {
    _hasNonPodData = other._hasNonPodData;
    if (other._hasNonPodData) {  // other has non pod data, need to copy
      std::vector<Pair> data(other._data.size());
      _data.swap(data);
      for (size_t i = 0; i < _data.size(); ++i) {
        _data[i].key = other._data[i].key;
        copy_rdvalue(_data[i].val, other._data[i].val);
      }
    }
  }

  Dict(Dict &&other) noexcept = default;

  ~Dict() {
    reset();  // to clear pointers if necessary
  }

  void update(const Dict &other, bool preserveExisting = false) {
    if (!preserveExisting) {
      *this = other;
    } else {
      if (other._hasNonPodData) {
        _hasNonPodData = true;
      }
      for (const auto &opair : other._data) {
        Pair *target = nullptr;
        for (auto &dpair : _data) {
          if (dpair.key == opair.key) {
            target = &dpair;
            break;
          }
        }

        if (!target) {
          // need to create blank entry and copy
          _data.push_back(Pair(opair.key));
          copy_rdvalue(_data.back().val, opair.val);
        } else {
          // just copy
          copy_rdvalue(target->val, opair.val);
        }
      }
    }
  }

  Dict &operator=(const Dict &other) {
    if (this == &other) {
      return *this;
    }
    if (_hasNonPodData) {
      reset();
    }

    if (other._hasNonPodData) {
      std::vector<Pair> data(other._data.size());
      _data.swap(data);
      for (size_t i = 0; i < _data.size(); ++i) {
        _data[i].key = other._data[i].key;
        copy_rdvalue(_data[i].val, other._data[i].val);
      }
    } else {
      _data = other._data;
    }
    _hasNonPodData = other._hasNonPodData;
    return *this;
  }

  Dict &operator=(Dict &&other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_hasNonPodData) {
      reset();
    }
    _hasNonPodData = other._hasNonPodData;
    other._hasNonPodData = false;
    _data = std::move(other._data);
    return *this;
  }

  //----------------------------------------------------------
  //! \brief Access to the underlying non-POD containment flag
  //! This is meant to be used only in bulk updates of _data.
  bool &getNonPODStatus() { return _hasNonPodData; }

  //----------------------------------------------------------
  //! \brief Access to the underlying data.
  const DataType &getData() const { return _data; }
  DataType &getData() { return _data; }

  //----------------------------------------------------------

  //! \brief Returns whether or not the dictionary contains a particular
  //!        key.
  bool hasVal(const std::string &what) const {
    for (const auto &data : _data) {
      if (data.key == what) {
        return true;
      }
    }
    return false;
  }

  //----------------------------------------------------------
  //! Returns the set of keys in the dictionary
  /*!
     \return  a \c STR_VECT
  */
  STR_VECT keys() const {
    STR_VECT res;
    res.reserve(_data.size());
    for (const auto &item : _data) {
      res.push_back(item.key);
    }
    return res;
  }

  //----------------------------------------------------------
  //! \brief Gets the value associated with a particular key
  /*!
     \param what  the key to lookup
     \param res   a reference used to return the result

     <b>Notes:</b>
      - If \c res is a \c std::string, every effort will be made
        to convert the specified element to a string using the
        \c boost::lexical_cast machinery.
      - If the dictionary does not contain the key \c what,
        a KeyErrorException will be thrown.
  */
  template <typename T>
  void getVal(const std::string &what, T &res) const {
    res = getVal<T>(what);
  }

  //! \overload
  template <typename T>
  T getVal(const std::string &what) const {
    for (auto &data : _data) {
      if (data.key == what) {
        return from_rdvalue<T>(data.val);
      }
    }
    throw KeyErrorException(what);
  }

  //! \overload
  void getVal(const std::string &what, std::string &res) const {
    for (const auto &i : _data) {
      if (i.key == what) {
        rdvalue_tostring(i.val, res);
        return;
      }
    }
    throw KeyErrorException(what);
  }

  //----------------------------------------------------------
  //! \brief Potentially gets the value associated with a particular key
  //!        returns true on success/false on failure.
  /*!
     \param what  the key to lookup
     \param res   a reference used to return the result

     <b>Notes:</b>
      - If \c res is a \c std::string, every effort will be made
        to convert the specified element to a string using the
        \c boost::lexical_cast machinery.
      - If the dictionary does not contain the key \c what,
        a KeyErrorException will be thrown.
  */
  template <typename T>
  bool getValIfPresent(const std::string &what, T &res) const {
    for (const auto &data : _data) {
      if (data.key == what) {
        res = from_rdvalue<T>(data.val);
        return true;
      }
    }
    return false;
  }

  //! \overload
  bool getValIfPresent(const std::string &what, std::string &res) const {
    for (const auto &i : _data) {
      if (i.key == what) {
        rdvalue_tostring(i.val, res);
        return true;
      }
    }
    return false;
  }

  //----------------------------------------------------------
  //! \brief Sets the value associated with a key
  /*!

     \param what the key to set
     \param val  the value to store

     <b>Notes:</b>
        - If \c val is a <tt>const char *</tt>, it will be converted
           to a \c std::string for storage.
        - If the dictionary already contains the key \c what,
          the value will be replaced.
  */
  template <typename T>
  void setVal(const std::string &what, T &val) {
    static_assert(!std::is_same_v<T, std::string_view>,
                  "T cannot be string_view");
    _hasNonPodData = true;
    for (auto &&data : _data) {
      if (data.key == what) {
        RDValue::cleanup_rdvalue(data.val);
        data.val = val;
        return;
      }
    }
    _data.push_back(Pair(what, val));
  }

  template <typename T>
  void setPODVal(const std::string &what, T val) {
    static_assert(!std::is_same_v<T, std::string_view>,
                  "T cannot be string_view");
    // don't change the hasNonPodData status
    for (auto &&data : _data) {
      if (data.key == what) {
        RDValue::cleanup_rdvalue(data.val);
        data.val = val;
        return;
      }
    }
    _data.push_back(Pair(what, val));
  }

  void setVal(const std::string &what, bool val) { setPODVal(what, val); }

  void setVal(const std::string &what, double val) { setPODVal(what, val); }

  void setVal(const std::string &what, float val) { setPODVal(what, val); }

  void setVal(const std::string &what, int val) { setPODVal(what, val); }

  void setVal(const std::string &what, unsigned int val) {
    setPODVal(what, val);
  }

  //! \overload
  void setVal(const std::string &what, const char *val) {
    std::string h(val);
    setVal(what, h);
  }

  //----------------------------------------------------------
  //! \brief Clears the value associated with a particular key,
  //!     removing the key from the dictionary.
  /*!

     \param what the key to clear

  */
  void clearVal(const std::string &what) {
    for (DataType::iterator it = _data.begin(); it < _data.end(); ++it) {
      if (it->key == what) {
        if (_hasNonPodData) {
          RDValue::cleanup_rdvalue(it->val);
        }
        _data.erase(it);
        return;
      }
    }
  }

  //----------------------------------------------------------
  //! \brief Clears all keys (and values) from the dictionary.
  //!
  void reset() {
    if (_hasNonPodData) {
      for (auto &&data : _data) {
        RDValue::cleanup_rdvalue(data.val);
      }
    }
    DataType data;
    _data.swap(data);
  }

 private:
  DataType _data{};            //!< the actual dictionary
  bool _hasNonPodData{false};  // if true, need a deep copy
                               //  (copy_rdvalue)
};

template <>
inline std::string Dict::getVal<std::string>(const std::string &what) const {
  std::string res;
  getVal(what, res);
  return res;
}

// Utility class for holding a Dict::Pair
//  Dict::Pairs require containers for memory management
//  This utility class covers cleanup and copying
class PairHolder : public Dict::Pair {
public:
 PairHolder() : Pair() {}
  
  explicit PairHolder(const PairHolder &p) : Pair(p.key) {
    copy_rdvalue(this->val, p.val);
  }

  explicit PairHolder(PairHolder&&p) : Pair(p.key) {
    this->val = p.val;
    p.val.type = RDTypeTag::EmptyTag;
  }

  explicit PairHolder(Dict::Pair&&p) : Pair(p.key) {
    this->val = p.val;
    p.val.type = RDTypeTag::EmptyTag;
  }

  ~PairHolder() {
    RDValue::cleanup_rdvalue(this->val);
  }

};
}  // namespace RDKit
#endif
