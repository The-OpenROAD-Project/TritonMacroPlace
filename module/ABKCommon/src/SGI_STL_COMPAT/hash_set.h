/*
 * Copyright (c) 1996
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Moscow Center for SPARC Technology makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __SGI_STL_HASH_SET_H
#define __SGI_STL_HASH_SET_H

#ifndef __SGI_STL_HASHTABLE_H
#include <memory>
#include <algorithm>
#include <functional>
#include <utility>
#include "ABKCommon/SGI_STL_COMPAT/stl_config.h"
#include "ABKCommon/SGI_STL_COMPAT/hashtable.h"
#endif /* __SGI_STL_HASHTABLE_H */

__BEGIN_STL_NAMESPACE

# define  hash_set      __WORKAROUND_RENAME(hash_set)
# define  hash_multiset __WORKAROUND_RENAME(hash_multiset)

template <class Value, __DFL_TMPL_PARAM(HashFcn,hash<Value>),
          __DFL_TMPL_PARAM(EqualKey,equal_to<Value>),
          __DFL_TYPE_PARAM(Alloc,alloc) >
class hash_set
{
private:
  typedef hashtable<Value, Value, HashFcn, identity<Value>, 
      EqualKey, Alloc> ht;
  typedef hash_set<Value, HashFcn, EqualKey, Alloc> self;
  typedef typename ht::iterator ht_iterator;
public:
  typedef typename ht::key_type key_type;
  typedef typename ht::value_type value_type;
  typedef typename ht::hasher hasher;
  typedef typename ht::key_equal key_equal;

  typedef typename ht::size_type size_type;
  typedef typename ht::difference_type difference_type;
  typedef typename ht::const_pointer pointer;
  typedef typename ht::const_pointer const_pointer;
  typedef typename ht::const_reference reference;
  typedef typename ht::const_reference const_reference;

  // SunPro bug
  typedef typename ht::const_iterator const_iterator;
  typedef const_iterator iterator;

  hasher hash_funct() const { return rep.hash_funct(); }
  key_equal key_eq() const { return rep.key_eq(); }

private:
  ht rep;

public:
  hash_set() : rep(100, hasher(), key_equal()) {}
  explicit hash_set(size_type n) : rep(n, hasher(), key_equal()) {}
  hash_set(size_type n, const hasher& hf) : rep(n, hf, key_equal()) {}
  hash_set(size_type n, const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) {}

#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  hash_set(InputIterator f, InputIterator l)
    : rep(100, hasher(), key_equal()) { rep.insert_unique(f, l); }
  template <class InputIterator>
  hash_set(InputIterator f, InputIterator l, size_type n)
    : rep(n, hasher(), key_equal()) { rep.insert_unique(f, l); }
  template <class InputIterator>
  hash_set(InputIterator f, InputIterator l, size_type n,
           const hasher& hf)
    : rep(n, hf, key_equal()) { rep.insert_unique(f, l); }
  template <class InputIterator>
  hash_set(InputIterator f, InputIterator l, size_type n,
           const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) { rep.insert_unique(f, l); }
#else

  hash_set(const value_type* f, const value_type* l)
    : rep(100, hasher(), key_equal()) { rep.insert_unique(f, l); }
  hash_set(const value_type* f, const value_type* l, size_type n)
    : rep(n, hasher(), key_equal()) { rep.insert_unique(f, l); }
  hash_set(const value_type* f, const value_type* l, size_type n,
           const hasher& hf)
    : rep(n, hf, key_equal()) { rep.insert_unique(f, l); }
  hash_set(const value_type* f, const value_type* l, size_type n,
           const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) { rep.insert_unique(f, l); }

  hash_set(const_iterator f, const_iterator l)
    : rep(100, hasher(), key_equal()) { rep.insert_unique(f, l); }
  hash_set(const_iterator f, const_iterator l, size_type n)
    : rep(n, hasher(), key_equal()) { rep.insert_unique(f, l); }
  hash_set(const_iterator f, const_iterator l, size_type n,
           const hasher& hf)
    : rep(n, hf, key_equal()) { rep.insert_unique(f, l); }
  hash_set(const_iterator f, const_iterator l, size_type n,
           const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) { rep.insert_unique(f, l); }
#endif /*__STL_MEMBER_TEMPLATES */

public:
  size_type size() const { return rep.size(); }
  size_type max_size() const { return rep.max_size(); }
  bool empty() const { return rep.empty(); }
  void swap(hash_set<Value, HashFcn, EqualKey, Alloc>& hs) { rep.swap(hs.rep); }
  friend inline bool operator==(const hash_set<Value,HashFcn,EqualKey,Alloc>&,
                                const hash_set<Value,HashFcn,EqualKey,Alloc>&);

  iterator begin() const { return rep.begin(); }
  iterator end() const { return rep.end(); }

public:
  pair<iterator, bool> insert(const value_type& obj)
    {
      pair<ht_iterator, bool> p = rep.insert_unique(obj);
      return pair<iterator, bool>(p.first, p.second);
    }
#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  void insert(InputIterator f, InputIterator l) { rep.insert_unique(f,l); }
#else
  void insert(const value_type* f, const value_type* l) {
    rep.insert_unique(f,l);
  }
  void insert(const_iterator f, const_iterator l) {rep.insert_unique(f, l); }
#endif /*__STL_MEMBER_TEMPLATES */
  pair<iterator, bool> insert_noresize(const value_type& obj)
    {
      pair<ht_iterator, bool> p = rep.insert_unique_noresize(obj);
      return pair<iterator, bool>(p.first, p.second);
    }

  iterator find(const key_type& key) const { return rep.find(key); }

  size_type count(const key_type& key) const { return rep.count(key); }
  
  pair<iterator, iterator> equal_range(const key_type& key) const
    { return rep.equal_range(key); }

  size_type erase(const key_type& key) {return rep.erase(key); }
  void erase(iterator it) { rep.erase(it); }
  void erase(iterator f, iterator l) { rep.erase(f, l); }
  void clear() { rep.clear(); }

public:
  void resize(size_type hint) { rep.resize(hint); }
  size_type bucket_count() const { return rep.bucket_count(); }
  size_type max_bucket_count() const { return rep.max_bucket_count(); }
  size_type elems_in_bucket(size_type n) const
    { return rep.elems_in_bucket(n); }
};


template <class Value, __DFL_TMPL_PARAM(HashFcn,hash<Value>),
          __DFL_TMPL_PARAM(EqualKey,equal_to<Value>),
          __DFL_TYPE_PARAM(Alloc,alloc) >
class hash_multiset
{
private:
  typedef hashtable<Value, Value, HashFcn, identity<Value>, 
      EqualKey, Alloc> ht;
  typedef hash_multiset<Value, HashFcn, EqualKey, Alloc> self;

public:
  typedef typename ht::key_type key_type;
  typedef typename ht::value_type value_type;
  typedef typename ht::hasher hasher;
  typedef typename ht::key_equal key_equal;

  typedef typename ht::size_type size_type;
  typedef typename ht::difference_type difference_type;
  typedef typename ht::const_pointer pointer;
  typedef typename ht::const_pointer const_pointer;
  typedef typename ht::const_reference reference;
  typedef typename ht::const_reference const_reference;

  typedef typename ht::const_iterator const_iterator;
  // SunPro bug
  typedef const_iterator iterator;

  hasher hash_funct() const { return rep.hash_funct(); }
  key_equal key_eq() const { return rep.key_eq(); }
private:
  ht rep;

public:
  hash_multiset() : rep(100, hasher(), key_equal()) {}
  explicit hash_multiset(size_type n) : rep(n, hasher(), key_equal()) {}
  hash_multiset(size_type n, const hasher& hf) : rep(n, hf, key_equal()) {}
  hash_multiset(size_type n, const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) {}

#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  hash_multiset(InputIterator f, InputIterator l)
    : rep(100, hasher(), key_equal()) { rep.insert_equal(f, l); }
  template <class InputIterator>
  hash_multiset(InputIterator f, InputIterator l, size_type n)
    : rep(n, hasher(), key_equal()) { rep.insert_equal(f, l); }
  template <class InputIterator>
  hash_multiset(InputIterator f, InputIterator l, size_type n,
                const hasher& hf)
    : rep(n, hf, key_equal()) { rep.insert_equal(f, l); }
  template <class InputIterator>
  hash_multiset(InputIterator f, InputIterator l, size_type n,
                const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) { rep.insert_equal(f, l); }
#else

  hash_multiset(const value_type* f, const value_type* l)
    : rep(100, hasher(), key_equal()) { rep.insert_equal(f, l); }
  hash_multiset(const value_type* f, const value_type* l, size_type n)
    : rep(n, hasher(), key_equal()) { rep.insert_equal(f, l); }
  hash_multiset(const value_type* f, const value_type* l, size_type n,
                const hasher& hf)
    : rep(n, hf, key_equal()) { rep.insert_equal(f, l); }
  hash_multiset(const value_type* f, const value_type* l, size_type n,
                const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) { rep.insert_equal(f, l); }

  hash_multiset(const_iterator f, const_iterator l)
    : rep(100, hasher(), key_equal()) { rep.insert_equal(f, l); }
  hash_multiset(const_iterator f, const_iterator l, size_type n)
    : rep(n, hasher(), key_equal()) { rep.insert_equal(f, l); }
  hash_multiset(const_iterator f, const_iterator l, size_type n,
                const hasher& hf)
    : rep(n, hf, key_equal()) { rep.insert_equal(f, l); }
  hash_multiset(const_iterator f, const_iterator l, size_type n,
                const hasher& hf, const key_equal& eql)
    : rep(n, hf, eql) { rep.insert_equal(f, l); }
#endif /*__STL_MEMBER_TEMPLATES */

public:
  size_type size() const { return rep.size(); }
  size_type max_size() const { return rep.max_size(); }
  bool empty() const { return rep.empty(); }
  void swap(hash_multiset<Value, HashFcn, EqualKey, Alloc>& hs) { rep.swap(hs.rep); }
  friend inline bool operator==(const hash_multiset<Value,HashFcn,EqualKey,Alloc>&,
                                const hash_multiset<Value,HashFcn,EqualKey,Alloc>&);

  iterator begin() const { return rep.begin(); }
  iterator end() const { return rep.end(); }

public:
  iterator insert(const value_type& obj) { return rep.insert_equal(obj); }
#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  void insert(InputIterator f, InputIterator l) { rep.insert_equal(f,l); }
#else
  void insert(const value_type* f, const value_type* l) {
    rep.insert_equal(f,l);
  }
  void insert(const_iterator f, const_iterator l) { rep.insert_equal(f, l); }
#endif /*__STL_MEMBER_TEMPLATES */
  iterator insert_noresize(const value_type& obj)
    { return rep.insert_equal_noresize(obj); }    

  iterator find(const key_type& key) const { return rep.find(key); }

  size_type count(const key_type& key) const { return rep.count(key); }
  
  pair<iterator, iterator> equal_range(const key_type& key) const
    { return rep.equal_range(key); }

  size_type erase(const key_type& key) {return rep.erase(key); }
  void erase(iterator it) { rep.erase(it); }
  void erase(iterator f, iterator l) { rep.erase(f, l); }
  void clear() { rep.clear(); }

public:
  void resize(size_type hint) { rep.resize(hint); }
  size_type bucket_count() const { return rep.bucket_count(); }
  size_type max_bucket_count() const { return rep.max_bucket_count(); }
  size_type elems_in_bucket(size_type n) const
    { return rep.elems_in_bucket(n); }
};


// do a cleanup
#  undef hash_set
#  undef hash_multiset
// provide a uniform way to access full funclionality 
#  define __hash_set__       __FULL_NAME(hash_set)
#  define __hash_multiset__  __FULL_NAME(hash_multiset)

template <class Value, class HashFcn, class EqualKey, class Alloc>
inline bool operator==(const __hash_set__<Value, HashFcn, EqualKey, Alloc>& hs1,
                       const __hash_set__<Value, HashFcn, EqualKey, Alloc>& hs2)
{
  return hs1.rep == hs2.rep;
}

template <class Value, class HashFcn, class EqualKey, class Alloc>
inline bool operator==(const __hash_multiset__<Value, HashFcn, EqualKey, Alloc>& hs1,
                       const __hash_multiset__<Value, HashFcn, EqualKey, Alloc>& hs2)
{
  return hs1.rep == hs2.rep;
}

# if defined (__STL_CLASS_PARTIAL_SPECIALIZATION )
template <class Value, class HashFcn, class EqualKey, class Alloc>
inline void swap(__hash_multiset__<Value, HashFcn, EqualKey, Alloc>& a,
                 __hash_multiset__<Value, HashFcn, EqualKey, Alloc>& b) { a.swap(b); }
template <class Value, class HashFcn, class EqualKey, class Alloc>
inline void swap(__hash_set__<Value, HashFcn, EqualKey, Alloc>& a,
                 __hash_set__<Value, HashFcn, EqualKey, Alloc>& b) { a.swap(b); }
# endif

typedef std::allocator<void> alloc;

# ifndef __STL_DEFAULT_TYPE_PARAM
// provide a "default" hash_set adaptor
template <class Value, class HashFcn,
          class EqualKey >
class hash_set : public __hash_set__<Value, HashFcn, EqualKey, alloc>
{
  typedef hash_set<Value, HashFcn, EqualKey> self;
public:
  typedef __hash_set__<Value, HashFcn, EqualKey, alloc> super;
  __IMPORT_CONTAINER_TYPEDEFS(super)
  __IMPORT_ITERATORS(super)
  typedef typename super::key_type key_type;
  typedef typename super::hasher hasher;
  typedef typename super::key_equal key_equal;
  typedef typename super::pointer pointer;
  typedef typename super::const_pointer const_pointer;
  hash_set() {}
  hash_set(size_type n) : super(n) {}
  hash_set(size_type n, const hasher& hf) : super(n, hf) {}
  hash_set(size_type n, const hasher& hf, const key_equal& eql): super(n, hf, eql) {}

  hash_set(const value_type* f, const value_type* l) : super(f,l) {}
  hash_set(const value_type* f, const value_type* l, size_type n): super(f,l,n) {}
  hash_set(const value_type* f, const value_type* l, size_type n,
           const hasher& hf) : super(f,l,n,hf) {}
  hash_set(const value_type* f, const value_type* l, size_type n,
           const hasher& hf, const key_equal& eql) : super(f,l,n,hf, eql) {}

  hash_set(const_iterator f, const_iterator l) : super(f,l) { }
  hash_set(const_iterator f, const_iterator l, size_type n) : super(f,l,n) { }
  hash_set(const_iterator f, const_iterator l, size_type n,
           const hasher& hf) : super(f, l, n, hf) { }
  hash_set(const_iterator f, const_iterator l, size_type n,
           const hasher& hf, const key_equal& eql) : super(f, l, n, hf, eql) { }
# if defined (__STL_BASE_MATCH_BUG)
    friend bool operator==(const self& hs1, const self& hs2);
# endif
};

# if defined (__STL_BASE_MATCH_BUG)
template <class Value, class HashFcn,class EqualKey >
inline bool operator==(const hash_set<Value, HashFcn,EqualKey>& hs1, 
                       const hash_set<Value, HashFcn,EqualKey>& hs2)
{
    typedef typename hash_set<Value, HashFcn,EqualKey>::super super;
    return (const super&)hs1 == (const super&)hs2; 
}
# endif

// provide a "default" hash_multiset adaptor
template <class Value, class HashFcn,
          class EqualKey >
class hash_multiset : public __hash_multiset__<Value, HashFcn, EqualKey, alloc>
{
  typedef hash_multiset<Value, HashFcn, EqualKey> self;
public:
  typedef __hash_multiset__<Value, HashFcn, EqualKey, alloc> super;
  __IMPORT_CONTAINER_TYPEDEFS(super)
  __IMPORT_ITERATORS(super)
  typedef typename super::key_type key_type;
  typedef typename super::hasher hasher;
  typedef typename super::key_equal key_equal;
  typedef typename super::pointer pointer;
  typedef typename super::const_pointer const_pointer;

  hash_multiset() {}
  hash_multiset(size_type n) : super(n) {}
  hash_multiset(size_type n, const hasher& hf) : super(n, hf) {}
  hash_multiset(size_type n, const hasher& hf, const key_equal& eql): super(n, hf, eql) {}

  hash_multiset(const value_type* f, const value_type* l) : super(f,l) {}
  hash_multiset(const value_type* f, const value_type* l, size_type n): super(f,l,n) {}
  hash_multiset(const value_type* f, const value_type* l, size_type n,
           const hasher& hf) : super(f,l,n,hf) {}
  hash_multiset(const value_type* f, const value_type* l, size_type n,
           const hasher& hf, const key_equal& eql) : super(f,l,n,hf, eql) {}

  hash_multiset(const_iterator f, const_iterator l) : super(f,l) { }
  hash_multiset(const_iterator f, const_iterator l, size_type n) : super(f,l,n) { }
  hash_multiset(const_iterator f, const_iterator l, size_type n,
           const hasher& hf) : super(f, l, n, hf) { }
  hash_multiset(const_iterator f, const_iterator l, size_type n,
           const hasher& hf, const key_equal& eql) : super(f, l, n, hf, eql) { }
# if defined (__STL_BASE_MATCH_BUG)
  friend bool operator==(const self& hs1, const self& hs2);
# endif
};

# if defined (__STL_BASE_MATCH_BUG)
template <class Value, class HashFcn,class EqualKey >
inline bool operator==(const hash_multiset<Value, HashFcn,EqualKey>& hs1, 
                       const hash_multiset<Value, HashFcn,EqualKey>& hs2)
{
    // THAT is not a bug - just a workaround ;)
    typedef __hash_set__<Value, HashFcn, EqualKey, alloc> s;
    return __STL_NAMESPACE::operator==((const s&)hs1,(const s&)hs2);
}
# endif

# endif /*  __STL_DEFAULT_TEMPLATE_PARAM */

__END_STL_NAMESPACE

#endif /* __SGI_STL_HASH_SET_H */


