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


#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <cmath>

#include "mmapStreamBuf.h"
#include "uofm_alloc.h"

using std::cout;
using std::endl;
using uofm::string;


MMapStreamBuf::MMapStreamBuf(const char *inputFileName)
{
	_fileName = string(inputFileName);

	struct stat s;
	if (stat(_fileName.c_str(), &s)) {
		cout << "[MMapStreamBuf] " << "stat failed" << endl;
		throw;
	}

	_fileSize = s.st_size;

	int fd;
	if ((fd = open(_fileName.c_str(), O_RDONLY)) < 0) {
		cout << "[MMapStreamBuf] " << "open failed" << endl;
		throw;
	}

	int pageSize = getpagesize();
	_mapSize = unsigned(ceil(double(_fileSize)/double(pageSize))) * pageSize;
	unsigned offset = 0;
	void *buf = mmap(NULL, _mapSize, PROT_READ, MAP_PRIVATE, fd, offset);

	close(fd);

	if (buf == MAP_FAILED) {
		cout << "[MMapStreamBuf] " << "mmap failed" << endl;
		throw;
	}

	setbuf((char *)buf, _fileSize);
}

MMapStreamBuf::~MMapStreamBuf()
{
	munmap(_M_buf, _mapSize);
}
