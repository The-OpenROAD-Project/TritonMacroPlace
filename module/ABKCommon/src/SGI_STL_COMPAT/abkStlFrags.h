
#ifndef __ABK_STL_FRAGMENTS_H
#define __ABK_STL_FRAGMENTS_H

#include <functional>

namespace std
{

template <class T1, class T2>
inline void construct(T1* p, const T2& value) 
{ new (p) T1(value); } 

template <class T>
inline void destroy(T* pointer)
{ pointer->~T(); }

template<class _Tp, class _Alloc>
class simple_alloc {

#ifdef _MSC_VER
    std::allocator<_Tp> a; // in this case _Alloc is ignored
#else
    _Alloc a;
#endif

public:
    _Tp* allocate(size_t __n)
#ifdef _MSC_VER
        // the NULL is not used
      { return 0 == __n ? 0 : (_Tp*) a.allocate(__n * sizeof (_Tp),NULL); }
#else
      { return 0 == __n ? 0 : (_Tp*) a.allocate(__n * sizeof (_Tp)); }
#endif
    _Tp* allocate()
#ifdef _MSC_VER
        // the NULL is not used
      { return (_Tp*) a.allocate(sizeof (_Tp),NULL); }
#else
      { return (_Tp*) a.allocate(sizeof (_Tp)); }
#endif
    void deallocate(_Tp* __p, size_t __n)
      { if (0 != __n) a.deallocate(__p, __n * sizeof (_Tp)); }
    void deallocate(_Tp* __p)
      { a.deallocate(__p, sizeof (_Tp)); }
    void construct(_Tp* __p,const _Tp& val)
      { a.construct(__p,val);}
    void destroy(_Tp* __p)
      { a.destroy(__p);}

}; 

//struct __true_type { };
//struct __false_type { };
}

//using std::simple_alloc;
//using std::__true_type;
//using std::__false_type;

#endif

