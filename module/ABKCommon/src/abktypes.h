/**************************************************************************
***    
*** Copyright (c) 1995-2000 Regents of the University of California,
***               Andrew E. Caldwell, Andrew B. Kahng and Igor L. Markov
*** Copyright (c) 2000-2010 Regents of the University of Michigan,
***               Saurabh N. Adya, Jarrod A. Roy, David A. Papa and
***               Igor L. Markov
***
***  Contact author(s): abk@cs.ucsd.edu, imarkov@umich.edu
***  Original Affiliation:   UCLA, Computer Science Department,
***                          Los Angeles, CA 90095-1596 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining 
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation 
***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
***  and/or sell copies of the Software, and to permit persons to whom the 
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/


//! author="Igor Markov, August 18, 1997 "
/*
 This file to be included into all projects in the group
*/

#ifndef  _ABKTYPES_H_
#define  _ABKTYPES_H_
#include <ABKCommon/uofm_alloc.h>

typedef char Str12[13];
typedef char Str31[32];
typedef char Str64[65];
typedef char Str80[81];
typedef char Str127[128];
typedef char Str255[256];

typedef char SByte;
typedef unsigned char UByte;
typedef unsigned char Byte;
typedef short SWord;
typedef unsigned short UWord;
/* int == long on both Solaris SunPro CC and  MS Windows/ MS VC++*/
typedef int SLong;
typedef unsigned int ULong;
#ifdef __GNUC__
  #if( __GNUC__ >= 3)
    typedef std::vector< bool > bit_vector;
  #endif
#endif
#ifdef _MSC_VER
  typedef std::vector< bool > bit_vector;
#endif


#endif 
