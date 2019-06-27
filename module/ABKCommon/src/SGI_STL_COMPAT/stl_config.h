/*
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
 * Copyright (c) 1997
 * Silicon Graphics
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

#ifndef __SGI_STL_CONFIG_H
# define __SGI_STL_CONFIG_H

// SGI basic release
#   define __SGI_STL                                      0x203
// Adaptation version
#   define __SGI_STL_PORT                                 0x203

//# include "stlconf.h"

#ifndef __STL_NAMESPACES
#define __STL_NAMESPACES
#endif

#ifdef __STL_NO_NAMESPACES
#undef __STL_NO_NAMESPACES
#endif

#ifdef __STL_FULL_SPEC_SYNTAX
#undef __STL_FULL_SPEC_SYNTAX
#endif
//==========================================================
// Compatibility section
//==========================================================

// This definition makes SGI reverse_iterator to be compatible with
// other parts of MSVC library.
// Its use is strongly discouraged - for MSVC5.0 configuration, it is being
// set automatically. 
// #    define __STL_MSVC50_COMPATIBILITY 1

//==========================================================
// final workaround tuning based on given flags
//==========================================================

// use other new-style headers only if needed
# if ! defined ( __STL_USE_NEW_STYLE_HEADERS )
#  undef __STL_NEW_HEADER_NAMES
# endif

// HP aCC with +noeh
# ifdef __HPACC_NOEH
#  undef __STL_USE_EXCEPTIONS
#  define __STL_NO_EXCEPTIONS 1
# endif

# if defined (__GNUC__)
// strict ANSI prohibits "long long" ( gcc)
#  if defined ( __STRICT_ANSI__ )
#    undef __STL_LONG_LONG
#  endif
# endif

# if defined (__STL_ARROW_OPERATOR)
#   undef __SGI_STL_NO_ARROW_OPERATOR
# else
#  if defined  (__STL_UNUSED_REQUIRED_BUG)
#   define __SGI_STL_NO_ARROW_OPERATOR 1
#  endif
# endif

# if !defined ( __STL_STATIC_TEMPLATE_DATA )
#   define __STL_STATIC_TEMPLATE_DATA 0
#  if !defined ( __STL_WEAK_ATTRIBUTE )
#   define __STL_WEAK_ATTRIBUTE 0
#  endif
# endif

# if defined (__STL_BASE_TYPEDEF_BUG)
#  undef  __STL_BASE_TYPEDEF_OUTSIDE_BUG
#  define __STL_BASE_TYPEDEF_OUTSIDE_BUG 1
# endif

// comment this section if you want to use BufSize parameter
// of deque (note that no template function taking deque<T,Alloc,BufSize>
// as parameter will compile then)
# if defined (__STL_NON_TYPE_TMPL_PARAM_BUG)
#  undef  __STL_NO_DEFAULT_NON_TYPE_PARAM
#  define __STL_NO_DEFAULT_NON_TYPE_PARAM 1
# endif

// features tuning 
# ifdef __STL_DEBUG
#  define __STL_ASSERTIONS 1
# endif

# ifdef __STL_ASSERTIONS
#  define __stl_assert(expr) \
    if (!(expr)) { fprintf(stderr, "%s:%d STL assertion failure: %s\n", \
			  __FILE__, __LINE__, # expr); abort(); }
# else
#  define __stl_assert(expr)
# endif

# ifdef __STL_DEBUG
#  define __stl_verbose_assert(expr,diagnostic) \
    if (!(expr)) { fprintf(stderr, "%s:%d STL error : %s\n%s:%d STL assertion failure: %s\n",\
			  __FILE__, __LINE__ , diagnostic, __FILE__, __LINE__ , # expr ); abort(); }

#  define __stl_debug_check(expr) __stl_assert(expr)
#  define __stl_debug_do(expr) expr
# else
#  define __stl_verbose_assert(expr,diagnostic)
#  define __stl_debug_check(expr)
#  define __stl_debug_do(expr)
# endif

# ifdef __STL_RAND48
#  define __rand lrand48
# else
#  define __rand rand
# endif

// tuning of static template data members workaround
# if ( __STL_STATIC_TEMPLATE_DATA < 1 )
// ignore __PUT directive in this case
#  if ( __STL_WEAK_ATTRIBUTE > 0 )
#   define __DECLARE_INSTANCE(type,item,init) type item __attribute__ (( weak )) = init
#  else
#   ifdef __PUT_STATIC_DATA_MEMBERS_HERE
#    define __DECLARE_INSTANCE(type,item,init) type item = init
#   else
#    define __DECLARE_INSTANCE(type,item,init)
#   endif /* __PUT_STATIC_DATA_MEMBERS_HERE */
#  endif /* __STL_WEAK_ATTRIBUTE */
# endif /* __STL_STATIC_TEMPLATE_DATA */


// default parameters as template types derived from arguments ( not always supported )
#  if ! defined (__STL_DEFAULT_TEMPLATE_PARAM)
#   define __DFL_TMPL_PARAM( classname, defval ) class classname
#   define __DFL_TMPL_ARG(classname) , classname
#  else
#   define __STL_DEFAULT_TYPE_PARAM 1
#   define __DFL_TMPL_PARAM( classname, defval ) class classname = defval
#   define __DFL_TMPL_ARG(classname)  
#  endif

// default parameters as complete types
# if defined ( __STL_DEFAULT_TYPE_PARAM )
#   define __DFL_TYPE_PARAM( classname, defval ) class classname = defval
#   define __DFL_NON_TYPE_PARAM(type,name,val) type name = val
#   define __DFL_TYPE_ARG(classname)
# else
#   define __DFL_TYPE_PARAM( classname, defval ) class classname
#   define __DFL_NON_TYPE_PARAM(type,name,val) type name
#   define __DFL_TYPE_ARG(classname) , classname
# endif

// default parameters workaround tuning
#  if defined  ( __STL_DEFAULT_TYPE_PARAM ) 
#    define __WORKAROUND_RENAME(X) X
#  else
#    define __WORKAROUND_RENAME(X) __##X
#  endif

// SGI compatibility
#  if ! defined (__STL_DEFAULT_TEMPLATE_PARAM)
#   define __STL_LIMITED_DEFAULT_TEMPLATES
#  endif

// namespace selection
# if defined (__STL_NAMESPACES) && ! defined (__STL_NO_NAMESPACES)

// change this if don't think that is standard enough ;)
#  define __STL_NAMESPACE std
#  define __BEGIN_STL_NAMESPACE namespace __STL_NAMESPACE {
#  define __STL_USING_NAMESPACE  using namespace __STL_NAMESPACE ;

#  ifdef __STL_NO_USING_STD
#   define __USING_NAMESPACE
#  else
#   define __USING_NAMESPACE using namespace __STL_NAMESPACE ;
#  endif
#  define __END_STL_NAMESPACE }; //__USING_NAMESPACE
    // workaround tuning
#  define __FULL_NAME(X) __WORKAROUND_RENAME(X)
# else /* __STL_NAMESPACES */
#  define __STL_NAMESPACE
#  define __BEGIN_STL_NAMESPACE
#  define __END_STL_NAMESPACE
#  define __STL_USING_NAMESPACE
    // workaround tuning
#  define __FULL_NAME(X) __WORKAROUND_RENAME(X)
# endif  /* __STL_NAMESPACES */

// backwards compat.
#    define __STL_FULL_NAMESPACE __STL_NAMESPACE
#    define __BEGIN_STL_FULL_NAMESPACE
#    define __END_STL_FULL_NAMESPACE

#  define __STL_NAME(name) __STL_NAMESPACE::name  // Lo Russo Graziano <Graziano.LoRusso@CSELT.IT>


// advanced keywords usage
#  ifdef  __STL_NEW_STYLE_CASTS
#   define __CONST_CAST(x,y) const_cast<x>(y)
#  else
#   define __CONST_CAST(x,y) ((x)y)
#  endif

#  ifndef __STL_TYPENAME
#   define typename
#  endif

#  ifndef __STL_EXPLICIT
#   define explicit
#  endif

#  ifdef __STL_MUTABLE
#   define __ASSIGN_MUTABLE(type,x,y) x=y
#  else
#   define __ASSIGN_MUTABLE(type,x,y) __CONST_CAST(type,x)=y
#   define mutable
#  endif

#  if defined (__STL_LOOP_INLINE_PROBLEMS)
#   define INLINE_LOOP
#  else
#   define INLINE_LOOP inline 
#  endif

//#if defined ( __STL_UNINITIALIZABLE_PRIVATE )
#if 1
#  define __PRIVATE public
#  define __PROTECTED public
#else
#  define __PRIVATE private
#  define __PROTECTED protected
#endif

#   define __STL_FULL_SPECIALIZATION template<>
/* ILM: May 14, 2000
  #  ifdef __STL_FULL_SPEC_SYNTAX
  #   define __STL_FULL_SPECIALIZATION template<>
  #  else
  #   define __STL_FULL_SPECIALIZATION
  #  endif
*/

#  define __IMPORT_CONTAINER_TYPEDEFS(super)                            \
    typedef typename super::value_type value_type;                               \
    typedef typename super::reference reference;                                 \
    typedef typename super::size_type size_type;                                 \
    typedef typename super::const_reference const_reference;                     \
    typedef typename super::difference_type difference_type;

#  define __IMPORT_ITERATORS(super)                                     \
    typedef typename super::iterator iterator;                                   \
    typedef typename super::const_iterator const_iterator; 

#  define __IMPORT_REVERSE_ITERATORS(super)                             \
    typedef typename super::const_reverse_iterator  const_reverse_iterator;      \
    typedef typename super::reverse_iterator reverse_iterator;

#define  __IMPORT_SUPER_COPY_ASSIGNMENT(__derived_name)         \
    __derived_name(const self& x) : super(x) {}                 \
    __derived_name(const super& x) : super(x) {}                \
    self& operator=(const self& x) {                            \
        super::operator=(x);                                    \
        return *this;                                           \
    }                                                           \
    self& operator=(const super& x) {                           \
        super::operator=(x);                                    \
        return *this;                                           \
    }

# if defined (__STL_BASE_TYPEDEF_OUTSIDE_BUG) || defined (__STL_NESTED_TYPE_PARAM_BUG)
#   define __CONTAINER_SUPER_TYPEDEFS \
 __IMPORT_CONTAINER_TYPEDEFS(super) __IMPORT_ITERATORS(super) __IMPORT_REVERSE_ITERATORS(super)
# else
#   define __CONTAINER_SUPER_TYPEDEFS
# endif

# if defined (__STL_TRIVIAL_CONSTRUCTOR_BUG) 
#  define __TRIVIAL_CONSTRUCTOR(type) type() {}  
# else
#  define __TRIVIAL_CONSTRUCTOR(type)
# endif
# if defined (__STL_TRIVIAL_DESTRUCTOR_BUG)
#  define __TRIVIAL_DESTRUCTOR(type) ~type() {}  
# else
#  define __TRIVIAL_DESTRUCTOR(type) 
# endif

#  define __TRIVIAL_STUFF(type)  \
  __TRIVIAL_CONSTRUCTOR(type) __TRIVIAL_DESTRUCTOR(type)

# if ! defined ( __STL_NAMESPACES )
#  define __STL_NO_NAMESPACES 1
# endif 

# if ! defined ( __STL_USE_EXCEPTIONS )
#  define __STL_NO_EXCEPTIONS 1
# endif 

# if defined ( __STL_NO_EXCEPTIONS )
#  undef __STL_USE_EXCEPTIONS
# endif 

# if defined ( __STL_USE_EXCEPTION_SPEC )
#  define __STL_THROWS(x) throw x
# else
#  define __STL_THROWS(x)
# endif

#if !defined(_PTHREADS) && !defined(__STL_SOLARIS_THREADS) && !defined(_NOTHREADS) \
 && !defined(__STL_SGI_THREADS) && !defined(__STL_WIN32THREADS)
#  define _NOTHREADS
#endif

# if (defined  (__STL_WIN32THREADS) && ! defined (_NOTHREADS)) && \
     (defined (__STL_DEBUG) ||                                     \
     !(defined(__STL_USE_MALLOC) || defined (__STL_USE_NEWALLOC)))
#   define __STL_WINDOWS_H_INCLUDED
#   define NOMINMAX
#   include <windows.h>
#   undef min
#   undef max
# endif

# if defined(__STL_YVALS_H)
#  include <yvals.h>
# else
#  if ! defined(__STL_BOOL_KEYWORD)
#   if defined (__STL_RESERVED_BOOL_KEYWORD)
#    define bool int
#   else
//   typedef int bool;
#   endif
#    define true 1
#    define false 0
#  endif /* __STL_BOOL_KEYWORD */
# endif

# undef __STL_BOOL_KEYWORD
# undef __STL_RESERVED_BOOL_KEYWORD
# undef __STL_YVALS_H
# undef __STL_LOOP_INLINE_PROBLEMS
# undef __STL_TYPENAME
# undef __STL_EXPLICIT
# undef __AUTO_CONFIGURED
# undef __STL_FULL_SPEC_SYNTAX

#endif /* __STL_CONFIG_H */
