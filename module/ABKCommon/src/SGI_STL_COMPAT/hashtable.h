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
 * Adaptation:
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

#ifndef __SGI_STL_HASHTABLE_H
#define __SGI_STL_HASHTABLE_H

// Hashtable class, used to implement the hashed associative containers
// hash_set, hash_map, hash_multiset, and hash_multimap.

//# ifndef __SGI_STL_ALGO_H
#  include <memory>
#  include <algorithm>
//# endif
//# ifndef __SGI_STL_VECTOR_H
#  include <vector>
//# endif
#  include <utility>

#  include <iterator>
#include "ABKCommon/SGI_STL_COMPAT/abkStlFrags.h"
#include "ABKCommon/SGI_STL_COMPAT/type_traits.h"

static const  unsigned __stl_num_primes=28;
extern const unsigned long __stl_prime_list[__stl_num_primes];
/*
{
  53,         97,         193,       389,       769,
  1543,       3079,       6151,      12289,     24593,
  49157,      98317,      196613,    393241,    786433,
  1572869,    3145739,    6291469,   12582917,  25165843,
  50331653,   100663319,  201326611, 402653189, 805306457,
  1610612741, 3221225473, 4294967291
};
*/

inline unsigned long __stl_next_prime(unsigned long n)
{
  const unsigned long* first = __stl_prime_list;
  const unsigned long* last = __stl_prime_list + __stl_num_primes;
  const unsigned long* pos = std::lower_bound(first, last, n);
  return pos == last ? *(last - 1) : *pos;
}

//using std::pair;
//using std::vector;
__BEGIN_STL_NAMESPACE

# if ! defined (__STL_COMPILE_TEMPLATE_BODY_ONLY)

# if defined ( __STL_USE_ABBREVS )
#  define __hashtable_iterator         _hT__It
#  define __hashtable_const_iterator   _hT__cIt
#  define __hashtable_node             _hT__N
#  define __hashtable_base             _hT__B
#  define hashtable                    _h__T
# endif

template <class Key> struct hash { };

inline size_t __stl_hash_string(const char* s)
{
  unsigned long h = 0; 
  for ( ; *s; ++s)
    h = 5*h + *s;
  
  return size_t(h);
}

__STL_FULL_SPECIALIZATION
struct hash<char*>
{
  size_t operator()(const char* s) const { return __stl_hash_string(s); }
};

__STL_FULL_SPECIALIZATION
struct hash<const char*>
{
  size_t operator()(const char* s) const { return __stl_hash_string(s); }
};

__STL_FULL_SPECIALIZATION
struct hash<char> {
  size_t operator()(char x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<unsigned char> {
  size_t operator()(unsigned char x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<signed char> {
  size_t operator()(unsigned char x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<short> {
  size_t operator()(short x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<unsigned short> {
  size_t operator()(unsigned short x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<int> {
  size_t operator()(int x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<unsigned int> {
  size_t operator()(unsigned int x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<long> {
  size_t operator()(long x) const { return x; }
};
__STL_FULL_SPECIALIZATION
struct hash<unsigned long> {
  size_t operator()(unsigned long x) const { return x; }
};

template <class Value>
struct __hashtable_node
{
  typedef __hashtable_node<Value> self;
  self* next;
  Value val;
};  

// some compilers require the names of template parameters to be the same
template <class V, class K, class HF, class ExK, class EqK, class A>
class hashtable;

template <class V, class K, class HF, class ExK, class EqK, class A>
struct __hashtable_iterator;

template <class V, class K, class HF, class ExK, class EqK, class A>
struct __hashtable_const_iterator;

template <class V, class K, class HF, class ExK, class EqK, class A>
struct __hashtable_iterator 
# if defined ( __STL_DEBUG )
    : public __safe_base 
# endif
{
  typedef hashtable<V, K, HF, ExK, EqK, A>
          hash_table;
  typedef __hashtable_iterator<V, K, HF, ExK, EqK, A>
          iterator;
  typedef __hashtable_const_iterator<V, K, HF, ExK, EqK, A>
          const_iterator;
  typedef __hashtable_node<V> node;

  typedef forward_iterator_tag iterator_category;
  typedef V value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef V& reference;
  typedef V* pointer;

  node* cur;
  hash_table* ht;

# if defined ( __STL_DEBUG )
  const node* get_iterator() const { return cur; }
  const hash_table* owner() const { return (const hash_table*)__safe_base::owner(); }
  __hashtable_iterator(node* n, hash_table* tab) : 
      __safe_base(tab), cur(n), ht(tab) {}
  __hashtable_iterator() : __safe_base(0) {}
# else
  __hashtable_iterator(node* n, hash_table* tab) : cur(n), ht(tab) {}
  __hashtable_iterator() {}
# endif
  reference operator*() const { 
        __stl_verbose_assert(valid() && cur!=0,__STL_MSG_NOT_DEREFERENCEABLE);
        return cur->val; 
  }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */
  iterator& operator++();
  iterator operator++(int);
  bool operator==(const iterator& it) const { 
      __stl_debug_check(__check_same_owner(*this,it));
      return cur == it.cur; 
  }
  bool operator!=(const iterator& it) const { 
      __stl_debug_check(__check_same_owner(*this,it));
      return cur != it.cur; 
  }
};


template <class V, class K, class HF, class ExK, class EqK, class A>
struct __hashtable_const_iterator 
# if defined ( __STL_DEBUG )
    : public __safe_base 
# endif
{
  typedef hashtable<V, K, HF, ExK, EqK, A>
          hash_table;
  typedef __hashtable_iterator<V, K, HF, ExK, EqK, A>
          iterator;
  typedef __hashtable_const_iterator<V, K, HF, ExK, EqK, A>
          const_iterator;
  typedef __hashtable_node<V> node;

  typedef forward_iterator_tag iterator_category;
  typedef V value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef const V& reference;
  typedef const V* pointer;

  const node* cur;
  const hash_table* ht;

# if defined ( __STL_DEBUG )
  const hash_table* owner() const { return (const hash_table*)__safe_base::owner(); }
  const node* get_iterator() const { return cur; }
  __hashtable_const_iterator(const node* n, const hash_table* tab)
    : __safe_base(tab), cur(n), ht(tab) {}
  __hashtable_const_iterator() : __safe_base(0) {}
  __hashtable_const_iterator(const iterator& it) : __safe_base(it), cur(it.cur), ht(it.ht) {}
# else
  __hashtable_const_iterator(const node* n, const hash_table* tab)
    : cur(n), ht(tab) {}
  __hashtable_const_iterator() {}
  __hashtable_const_iterator(const iterator& it) : cur(it.cur), ht(it.ht) {}
# endif

  reference operator*() const { 
      __stl_verbose_assert(valid() && cur!=0,__STL_MSG_NOT_DEREFERENCEABLE);
      return cur->val; 
  }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */
  const_iterator& operator++();
  const_iterator operator++(int);
  bool operator==(const const_iterator& it) const { 
      __stl_debug_check(__check_same_owner(*this,it));
      return cur == it.cur; 
  }
  bool operator!=(const const_iterator& it) const { 
      __stl_debug_check(__check_same_owner(*this,it));
      return cur != it.cur; 
  }
};

// inline unsigned long __stl_next_prime(unsigned long n);

#ifndef _MSC_VER
class allocator<void>;
#endif

// some compilers require the names of template parameters to be the same
template <class V, class K, class HF, class ExK, class EqK, class A>
# if defined ( __STL_DEBUG )
class hashtable : public __safe_server {
# else
class hashtable 
{
# endif
  typedef hashtable<V, K, HF, ExK, EqK, A> self;
public:
  typedef K key_type;
  typedef V value_type;
  typedef HF hasher;
  typedef EqK key_equal;

  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;

  hasher hash_funct() const { return hashfun; }
  key_equal key_eq() const { return equals; }

private:
  hasher hashfun;
  key_equal equals;
  ExK       get_key;
public:
  typedef __hashtable_node<V> node;
  typedef simple_alloc<node, A> node_allocator;
private:
#ifndef _MSC_VER
  vector<node*,A> buckets;
#else
  vector<node*> buckets;
#endif

  size_type num_elements;
  node_allocator node_allocator_object;

public:
  typedef __hashtable_iterator<V, K, HF, ExK, EqK, A> iterator;
  typedef __hashtable_const_iterator<V, K, HF, ExK, EqK, A> const_iterator;
  friend struct __hashtable_iterator<V, K, HF, ExK, EqK, A>;
  friend struct __hashtable_const_iterator<V, K, HF, ExK, EqK, A>;

public:
  hashtable(size_type n,
            const HF&    hf,
            const EqK&   eql,
            const ExK& ext)
    : hashfun(hf), equals(eql), get_key(ext), num_elements(0)
  {
    initialize_buckets(n);
  }

  hashtable(size_type n,
            const HF&    hf,
            const EqK&   eql)
    : hashfun(hf), equals(eql), get_key(ExK()), num_elements(0)
  {
    initialize_buckets(n);
    __stl_debug_do(safe_init(this));
  }

  hashtable(const self& ht)
    : hashfun(ht.hashfun), equals(ht.equals), get_key(ht.get_key), num_elements(0)
  {
    __stl_debug_do(safe_init(0));
    copy_from(ht);
    __stl_debug_do(safe_init(this));
  }

  self& operator= (const self& ht)
  {
    if (&ht != this) {
      clear();
      hashfun = ht.hashfun;
      equals = ht.equals;
      get_key = ht.get_key;
      copy_from(ht);
    }
    return *this;
  }

  ~hashtable() { clear(); __stl_debug_do(invalidate()); }

  size_type size() const { return num_elements; }
  size_type max_size() const { return size_type(-1); }
  bool empty() const { return size() == 0; }

  void swap(self& ht)
  {
    __STL_NAMESPACE::swap(hashfun, ht.hashfun);
    __STL_NAMESPACE::swap(equals, ht.equals);
    __STL_NAMESPACE::swap(get_key, ht.get_key);
    buckets.swap(ht.buckets);
    __STL_NAMESPACE::swap(num_elements, ht.num_elements);
    __stl_debug_do(swap_owners(ht));
  }

  iterator begin()
  { 
    for (size_type n = 0; n < buckets.size(); ++n)
      if (buckets[n])
        return iterator(buckets[n], this);
    return end();
  }

  iterator end() { return iterator((node*)0, this); }

  const_iterator begin() const
  {
    for (size_type n = 0; n < buckets.size(); ++n)
      if (buckets[n])
        return const_iterator(buckets[n], this);
    return end();
  }

  const_iterator end() const { return const_iterator((node*)0, this); }

  friend bool operator== (const self&, const self&);

public:

  size_type bucket_count() const { return buckets.size(); }

  size_type max_bucket_count() const
    { return 4294967291; }//__stl_prime_list[__stl_num_primes - 1]; } 

  size_type elems_in_bucket(size_type bucket) const
  {
    size_type result = 0;
    for (node* cur = buckets[bucket]; cur; cur = cur->next)
      result += 1;
    return result;
  }

  pair<iterator, bool> insert_unique(const value_type& obj)
  {
    resize(num_elements + 1);
    return insert_unique_noresize(obj);
  }

  iterator insert_equal(const value_type& obj)
  {
    resize(num_elements + 1);
    return insert_equal_noresize(obj);
  }

  pair<iterator, bool> insert_unique_noresize(const value_type& obj);
  iterator insert_equal_noresize(const value_type& obj);
 
#ifdef __STL_MEMBER_TEMPLATES
  template <class InputIterator>
  void insert_unique(InputIterator f, InputIterator l)
  {
    insert_unique(f, l, iterator_category(f));
  }

  template <class InputIterator>
  void insert_equal(InputIterator f, InputIterator l)
  {
    insert_equal(f, l, iterator_category(f));
  }

  template <class InputIterator>
  void insert_unique(InputIterator f, InputIterator l,
                     input_iterator_tag)
  {
    for ( ; f != l; ++f)
      insert_unique(*f);
  }

  template <class InputIterator>
  void insert_equal(InputIterator f, InputIterator l,
                    input_iterator_tag)
  {
    for ( ; f != l; ++f)
      insert_equal(*f);
  }

  template <class ForwardIterator>
  void insert_unique(ForwardIterator f, ForwardIterator l,
                     forward_iterator_tag)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_unique_noresize(*f);
  }

  template <class ForwardIterator>
  void insert_equal(ForwardIterator f, ForwardIterator l,
                    forward_iterator_tag)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_equal_noresize(*f);
  }

#else /* __STL_MEMBER_TEMPLATES */
  void insert_unique(const value_type* f, const value_type* l)
  {
    size_type n = l - f;
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_unique_noresize(*f);
  }

  void insert_equal(const value_type* f, const value_type* l)
  {
    size_type n = l - f;
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_equal_noresize(*f);
  }

  void insert_unique(const_iterator f, const_iterator l)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_unique_noresize(*f);
  }

  void insert_equal(const_iterator f, const_iterator l)
  {
    size_type n = 0;
    distance(f, l, n);
    resize(num_elements + n);
    for ( ; n > 0; --n, ++f)
      insert_equal_noresize(*f);
  }
#endif /*__STL_MEMBER_TEMPLATES */

  reference find_or_insert(const value_type& obj);

  iterator find(const key_type& key) 
  {
    size_type n = bkt_num_key(key);
    node* first;
    for ( first = buckets[n];
          first && !equals(get_key(first->val), key);
          first = first->next)
      {}
    return iterator(first, this);
  } 

  const_iterator find(const key_type& key) const
  {
    size_type n = bkt_num_key(key);
    const node* first;
    for ( first = buckets[n];
          first && !equals(get_key(first->val), key);
          first = first->next)
      {}
    return const_iterator(first, this);
  } 

  size_type count(const key_type& key) const
  {
    const size_type n = bkt_num_key(key);
    size_type result = 0;

    for (const node* cur = buckets[n]; cur; cur = cur->next)
      if (equals(get_key(cur->val), key))
        ++result;
    return result;
  }

  pair<iterator, iterator> equal_range(const key_type& key);
  pair<const_iterator, const_iterator> equal_range(const key_type& key) const;

  size_type erase(const key_type& key);
  void erase(const iterator& it);
  void erase(iterator first, iterator last);

  void erase(const const_iterator& it);
  void erase(const_iterator first, const_iterator last);

  void resize(size_type num_elements_hint);
  void clear();
# if defined (__STL_DEBUG)
    void invalidate_node(node* it) { 
        __invalidate_iterator(this,it,iterator());
    }
# endif

private:
  size_type next_size(size_type n) const { return __stl_next_prime(n); }

  void initialize_buckets(size_type n)
  {
    const size_type n_buckets = next_size(n);
    buckets.reserve(n_buckets);
    buckets.insert(buckets.end(), n_buckets, (node*) 0);
    num_elements = 0;
    __stl_debug_do(safe_init(this));
  }

  size_type bkt_num_key(const key_type& key) const
  {
    return bkt_num_key(key, buckets.size());
  }

  size_type bkt_num(const value_type& obj) const
  {
    return bkt_num_key(get_key(obj));
  }

  size_type bkt_num_key(const key_type& key, size_t n) const
  {
    return hashfun(key) % n;
  }

  size_type bkt_num(const value_type& obj, size_t n) const
  {
    return bkt_num_key(get_key(obj), n);
  }
 
  node* __new_node(const value_type& obj, __false_type, node* nxt = 0)
  {
    node* n = node_allocator_object.allocate();
#       ifdef __STL_USE_EXCEPTIONS
    try {
#       endif /* __STL_USE_EXCEPTIONS */
//    A::construct(&n->val, obj);
      construct(&n->val, obj);
      n->next=nxt;
      return n;
#       ifdef __STL_USE_EXCEPTIONS
    }
    catch(...) {
      node_allocator_object.deallocate(n);
      throw;
    }
#       endif /* __STL_USE_EXCEPTIONS */
    return n;
  }

  node* __new_node(const value_type& obj, __true_type, node* nxt = 0)
  {
    node* n = node_allocator_object.allocate();
    construct(&n->val, obj);
    n->next=nxt;
    return n;
  }

  node* new_node(const value_type& obj, node* nxt = 0) {
      typedef typename __type_traits<value_type>::is_POD_type is_POD_type;
      return __new_node(obj, is_POD_type(),nxt);
  }
  
  void delete_node(node* n)
  {
    __stl_debug_do(invalidate_node(n));
       destroy(&n->val);
    node_allocator_object.deallocate(n);
  }

  void erase_bucket(const size_type n, node* first, node* last)
  {
	  node* cur = buckets[n];
	  if (cur == first)
	    erase_bucket(n, last);
	  else 
          {
	    node* next;
	    for (next = cur->next; next != first; cur = next, next = cur->next)
	      ;
	    while (next)
            {
	      cur->next = next->next;
	      delete_node(next);
	      next = cur->next;
	      --num_elements;
	    }
	  }
  }

  void erase_bucket(const size_type n, node* last)
  {
    node* cur = buckets[n];
    while (cur != last) 
    {
      node* next = cur->next;
      delete_node(cur);
      cur = next;
      buckets[n] = cur;
      --num_elements;
    }
  }

  void copy_from(const self& ht)
//template <class V, class K, class HF, class ExK, class EqK, class A>
//void hashtable<V, K, HF, ExK, EqK, A>::copy_from(const hashtable<V, K, HF, ExK, EqK, A>& ht)
  {
	  buckets.clear();
	  buckets.reserve(ht.buckets.size());
	  buckets.insert(buckets.end(), ht.buckets.size(), (node*) 0);
	#   ifdef __STL_USE_EXCEPTIONS
	  try {
	#   endif /* __STL_USE_EXCEPTIONS */
	    for (size_type i = 0; i < ht.buckets.size(); ++i) {
	      const node* cur = ht.buckets[i];
	      if (cur) {
		node* copy = new_node(cur->val);
		buckets[i] = copy;
 	        for (node* next = cur->next; next; cur = next, next = cur->next)
                {
		  copy->next = new_node(next->val);
		  copy = copy->next;
		}
	//        copy->next = 0;
	      }
	    }
	    num_elements = ht.num_elements;
	#   ifdef __STL_USE_EXCEPTIONS
	  }
	  catch(...) {
	    clear();
	    throw;
	  }
	#   endif /* __STL_USE_EXCEPTIONS */
  }
};

template <class V, class K, class HF, class ExK, class EqK, class A>
bool operator== (const hashtable<V, K, HF, ExK, EqK, A>&, 
                 const hashtable<V, K, HF, ExK, EqK, A>&);

# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY */

// fbp: these defines are for outline methods definitions.
// needed to definitions to be portable. Should not be used in method bodies.

# if defined ( __STL_NESTED_TYPE_PARAM_BUG )
#  define __difference_type__ ptrdiff_t
#  define __size_type__       size_t
#  define __value_type__      V
#  define __key_type__        K
#  define __node__            __hashtable_node<V>
#  define __reference__       V&
#  define __iterator__        __hashtable_iterator<V, K, HF, ExK, EqK, A>
#  define __const_iterator__  __hashtable_const_iterator<V, K, HF, ExK, EqK, A>
# else
#  define __difference_type__  hashtable<V, K, HF, ExK, EqK, A>::difference_type
#  define __size_type__        hashtable<V, K, HF, ExK, EqK, A>::size_type
#  define __value_type__       hashtable<V, K, HF, ExK, EqK, A>::value_type
#  define __key_type__         hashtable<V, K, HF, ExK, EqK, A>::key_type
#  define __node__             hashtable<V, K, HF, ExK, EqK, A>::node
#  define __reference__        hashtable<V, K, HF, ExK, EqK, A>::reference
#  define __iterator__         hashtable<V, K, HF, ExK, EqK, A>::iterator
#  define __const_iterator__   hashtable<V, K, HF, ExK, EqK, A>::const_iterator
# endif

# if defined (__STL_COMPILE_TEMPLATE_BODY_ONLY) || \
   ! defined (__STL_SEPARATE_TEMPLATE_BODY)
template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++()
{
  const node* old = cur;
  __stl_verbose_assert(old!=0,__STL_MSG_INVALID_ADVANCE);
  cur = cur->next;
  if (!cur) {
    size_type bucket = ht->bkt_num(old->val);
    while (!cur && ++bucket < ht->buckets.size())
      cur = ht->buckets[bucket];
  }
  return *this;
}
# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY) || ! __STL_SEPARATE_TEMPLATE_BODY */

# if ! defined (__STL_COMPILE_TEMPLATE_BODY_ONLY)
template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_iterator<V, K, HF, ExK, EqK, A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++(int)
{
  iterator tmp = *this;
  ++*this;
  return tmp;
}
# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY */

# if defined (__STL_COMPILE_TEMPLATE_BODY_ONLY) || \
   ! defined (__STL_SEPARATE_TEMPLATE_BODY)
template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>::operator++()
{
  const node* old = cur;
  __stl_verbose_assert(old!=0,__STL_MSG_INVALID_ADVANCE);
  cur = cur->next;
  if (!cur) {
    size_type bucket = ht->bkt_num(old->val);
    while (!cur && ++bucket < ht->buckets.size())
      cur = ht->buckets[bucket];
  }
  return *this;
}
# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY) || ! __STL_SEPARATE_TEMPLATE_BODY */

#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION
# if ! defined (__STL_COMPILE_TEMPLATE_BODY_ONLY)
template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_const_iterator<V, K, HF, ExK, EqK, A>
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>::operator++(int)
{
  const_iterator tmp = *this;
  ++*this;
  return tmp;
}

/*
template <class V, class K, class HF, class ExK, class EqK, class A>
inline forward_iterator_tag
iterator_category(const __hashtable_iterator<V, K, HF, ExK, EqK, A>&)
{
  return forward_iterator_tag();
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline V* value_type(const __hashtable_iterator<V, K, HF, ExK, EqK, A>&)
{
  return (V*) 0;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline ptrdiff_t*
distance_type(const __hashtable_iterator<V, K, HF, ExK, EqK, A>&)
{
  return (__difference_type__*) 0;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline forward_iterator_tag
iterator_category(const __hashtable_const_iterator<V, K, HF, ExK, EqK, A>&)
{
  return forward_iterator_tag();
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline V* 
value_type(const __hashtable_const_iterator<V, K, HF, ExK, EqK, A>&)
{
  return (V*) 0;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline ptrdiff_t*
distance_type(const __hashtable_const_iterator<V, K, HF, ExK, EqK, A>&)
{
  return (__difference_type__*) 0;
}
*/

# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY */
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

# if defined (__STL_COMPILE_TEMPLATE_BODY_ONLY) || \
   ! defined (__STL_SEPARATE_TEMPLATE_BODY)
template <class V, class K, class HF, class ExK, class EqK, class A>
bool operator==(const hashtable<V, K, HF, ExK, EqK, A>& ht1,
                const hashtable<V, K, HF, ExK, EqK, A>& ht2)
{
  typedef typename hashtable<V, K, HF, ExK, EqK, A>::node node;
  if (ht1.buckets.size() != ht2.buckets.size())
    return false;
  for (size_t n = 0; n < ht1.buckets.size(); ++n) {
    node* cur1 = ht1.buckets[n];
    node* cur2 = ht2.buckets[n];
    for ( ; cur1 && cur2 && cur1->val == cur2->val;
          cur1 = cur1->next, cur2 = cur2->next)
      {}
    if (cur1 || cur2)
      return false;
  }
  return true;
}  

template <class V, class K, class HF, class ExK, class EqK, class A>
pair<__iterator__, bool> 
hashtable<V, K, HF, ExK, EqK, A>::insert_unique_noresize(const __value_type__& obj)
{
  const size_type n = bkt_num(obj);
  node* first = buckets[n];

  for (node* cur = first; cur; cur = cur->next) 
    if (equals(get_key(cur->val), get_key(obj)))
      return pair<iterator, bool>(iterator(cur, this), false);

  node* tmp = new_node(obj,first);
//  tmp->next = first;
  buckets[n] = tmp;
  ++num_elements;
  return pair<iterator, bool>(iterator(tmp, this), true);
}

template <class V, class K, class HF, class ExK, class EqK, class A>
__iterator__ 
hashtable<V, K, HF, ExK, EqK, A>::insert_equal_noresize(const __value_type__& obj)
{
  const size_type n = bkt_num(obj);
  node* first = buckets[n];

  for (node* cur = first; cur; cur = cur->next) 
    if (equals(get_key(cur->val), get_key(obj))) {
      node* tmp = new_node(obj, cur->next);
//      tmp->next = cur->next;
      cur->next = tmp;
      ++num_elements;
      return iterator(tmp, this);
    }

  node* tmp = new_node(obj,first);
//  tmp->next = first;
  buckets[n] = tmp;
  ++num_elements;
  return iterator(tmp, this);
}

template <class V, class K, class HF, class ExK, class EqK, class A>
__reference__ 
hashtable<V, K, HF, ExK, EqK, A>::find_or_insert(const __value_type__& obj)
{
  resize(num_elements + 1);

  size_type n = bkt_num(obj);
  node* first = buckets[n];

  for (node* cur = first; cur; cur = cur->next)
  {
    if (equals(get_key(cur->val), get_key(obj)))
      return cur->val;
  }

  node* tmp = new_node(obj, first);
//  tmp->next = first;
  buckets[n] = tmp;
  ++num_elements;
  return tmp->val;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
pair<__iterator__,
     __iterator__> 
hashtable<V, K, HF, ExK, EqK, A>::equal_range(const __key_type__& key)
{
  typedef pair<iterator, iterator> pii;
  const size_type n = bkt_num_key(key);

  for (node* first = buckets[n]; first; first = first->next) {
    if (equals(get_key(first->val), key)) {
      for (node* cur = first->next; cur; cur = cur->next)
        if (!equals(get_key(cur->val), key))
          return pii(iterator(first, this), iterator(cur, this));
      for (size_type m = n + 1; m < buckets.size(); ++m)
        if (buckets[m])
          return pii(iterator(first, this),
                     iterator(buckets[m], this));
      return pii(iterator(first, this), end());
    }
  }
  return pii(end(), end());
}

template <class V, class K, class HF, class ExK, class EqK, class A>
pair<__const_iterator__, 
     __const_iterator__> 
hashtable<V, K, HF, ExK, EqK, A>::equal_range(const __key_type__& key) const
{
  typedef pair<const_iterator, const_iterator> pii;
  const size_type n = bkt_num_key(key);

  for (const node* first = buckets[n] ; first; first = first->next) {
    if (equals(get_key(first->val), key)) {
      for (const node* cur = first->next; cur; cur = cur->next)
        if (!equals(get_key(cur->val), key))
          return pii(const_iterator(first, this),
                     const_iterator(cur, this));
      for (size_type m = n + 1; m < buckets.size(); ++m)
        if (buckets[m])
          return pii(const_iterator(first, this),
                     const_iterator(buckets[m], this));
      return pii(const_iterator(first, this), end());
    }
  }
  return pii(end(), end());
}

template <class V, class K, class HF, class ExK, class EqK, class A>
__size_type__ 
hashtable<V, K, HF, ExK, EqK, A>::erase(const __key_type__& key)
{
  const size_type n = bkt_num_key(key);
  node* first = buckets[n];
  size_type erased = 0;

  if (first) {
    node* cur = first;
    node* next = cur->next;
    while (next) {
      if (equals(get_key(next->val), key)) {
        cur->next = next->next;
        delete_node(next);
        next = cur->next;
        ++erased;
        --num_elements;
      }
      else {
        cur = next;
        next = cur->next;
      }
    }
    if (equals(get_key(first->val), key)) {
      buckets[n] = first->next;
      delete_node(first);
      ++erased;
      --num_elements;
    }
  }
  return erased;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
void hashtable<V, K, HF, ExK, EqK, A>::erase(const __iterator__& it)
{
  __stl_verbose_assert(it.owner()==this, __STL_MSG_NOT_OWNER);
  node* const p = it.cur;
  if (p) {
    const size_type n = bkt_num(p->val);
    node* cur = buckets[n];

    if (cur == p) {
      buckets[n] = cur->next;
      delete_node(cur);
      --num_elements;
    }
    else {
      node* next = cur->next;
      while (next) {
        if (next == p) {
          cur->next = next->next;
          delete_node(next);
          --num_elements;
          break;
        }
        else {
          cur = next;
          next = cur->next;
        }
      }
    }
  }
}

template <class V, class K, class HF, class ExK, class EqK, class A>
void hashtable<V, K, HF, ExK, EqK, A>::erase(__iterator__ first, __iterator__ last)
{
  size_type f_bucket = first.cur ? bkt_num(first.cur->val) : buckets.size();
  size_type l_bucket = last.cur ? bkt_num(last.cur->val) : buckets.size();
  __stl_debug_check(__check_if_owner(this,first)&&__check_if_owner(this,last));
  __stl_verbose_assert(f_bucket <= l_bucket, __STL_MSG_INVALID_RANGE);
  if (first.cur == last.cur)
    return;
  else if (f_bucket == l_bucket)
    erase_bucket(f_bucket, first.cur, last.cur);
  else {
    erase_bucket(f_bucket, first.cur, 0);
    for (size_type n = f_bucket + 1; n < l_bucket; ++n)
      erase_bucket(n, 0);
    if (l_bucket != buckets.size())
      erase_bucket(l_bucket, last.cur);
  }
}
# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY) || ! __STL_SEPARATE_TEMPLATE_BODY */

# if ! defined (__STL_COMPILE_TEMPLATE_BODY_ONLY)
template <class V, class K, class HF, class ExK, class EqK, class A>
inline void
hashtable<V, K, HF, ExK, EqK, A>::erase(__const_iterator__ first,
                                      __const_iterator__ last)
{
  erase(iterator(__CONST_CAST(node*,first.cur),
                 __CONST_CAST(self*,first.ht)),
        iterator(__CONST_CAST(node*,last.cur),
                 __CONST_CAST(self*,last.ht)));
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline void
hashtable<V, K, HF, ExK, EqK, A>::erase(const __const_iterator__& it)
{
  erase(iterator(__CONST_CAST(node*,it.cur),
                 __CONST_CAST(self*,it.ht)));
}
# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY */

# if defined (__STL_COMPILE_TEMPLATE_BODY_ONLY) || \
   ! defined (__STL_SEPARATE_TEMPLATE_BODY)
template <class V, class K, class HF, class ExK, class EqK, class A>
void hashtable<V, K, HF, ExK, EqK, A>::resize(__size_type__ num_elements_hint)
{
  const size_type old_n = buckets.size();
  if (num_elements_hint > old_n) {
    const size_type n = next_size(num_elements_hint);
    if (n > old_n) {
      //__vector__<node*, A> tmp(n, (node*) 0);
#ifndef _MSC_VER
      vector<node*, A> tmp(n, (node*) 0); // changed mro; hope it's ok
#else
      vector<node*> tmp(n, (node*) 0); // changed mro; hope it's ok
#endif
#         ifdef __STL_USE_EXCEPTIONS
      try {
#         endif /* __STL_USE_EXCEPTIONS */
        for (size_type bucket = 0; bucket < old_n; ++bucket) {
          node* first = buckets[bucket];
          while (first) {
            size_type new_bucket = bkt_num(first->val, n);
            buckets[bucket] = first->next;
            first->next = tmp[new_bucket];
            tmp[new_bucket] = first;
            first = buckets[bucket];          
          }
        }
        buckets.swap(tmp);
#         ifdef __STL_USE_EXCEPTIONS
      }
      catch(...) {
        for (size_type bucket = 0; bucket < tmp.size(); ++bucket) {
          while (tmp[bucket]) {
            node* next = tmp[bucket]->next;
            delete_node(tmp[bucket]);
            tmp[bucket] = next;
          }
        }
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
    }
  }
}

template <class V, class K, class HF, class ExK, class EqK, class A>
void hashtable<V, K, HF, ExK, EqK, A>::clear()
{
  for (size_type i = 0; i < buckets.size(); ++i) {
    node* cur = buckets[i];
    while (cur != 0) {
      node* next = cur->next;
      delete_node(cur);
      cur = next;
    }
    buckets[i] = 0;
  }
  num_elements = 0;
  __stl_debug_do(invalidate_all());
}

    

# endif /* __STL_COMPILE_TEMPLATE_BODY_ONLY) || ! __STL_SEPARATE_TEMPLATE_BODY */

# undef __iterator__ 
# undef __const_iterator__ 
# undef __difference_type__ 
# undef __size_type__       
# undef __value_type__      
# undef __key_type__        
# undef __node__            

__END_STL_NAMESPACE

#endif /* __SGI_STL_HASHTABLE_H */
