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


#ifndef MMAPSTREAMBUF_H
#define MMAPSTREAMBUF_H

#include <iostream>
#include "uofm_alloc.h"

/*
 * mmap stream buffer class to plug into the MMapIStream class
 * When the MMapIStream is initialized with a filename, it passes the 
 * filename to this class to be constructed into a streambuf. 
 * mmap() calls are RAII-ed.
 *
 * (check out sstream, fstream, streambuf and istream header files for more info)
 *
 * -- aaronnn
 */

class MMapStreamBuf : public std::streambuf
{
 public:
	MMapStreamBuf(const char *fileName);
	~MMapStreamBuf();

 protected:
	char *_M_buf;
	size_t _M_buf_size;

	virtual std::streambuf* setbuf(char *buf, unsigned sz)
	{
		_M_buf = buf;	
		_M_buf_size = sz;	
		this->setg(_M_buf, _M_buf + 0, _M_buf + _M_buf_size);
		this->setp(_M_buf, _M_buf + _M_buf_size);

		return this;
	}

 private:
	unsigned _fileSize;
	unsigned _mapSize;
	std::string _fileName;

	MMapStreamBuf();
};

#endif
