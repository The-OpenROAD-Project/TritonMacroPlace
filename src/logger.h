///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (c) 2020, The Regents of the University of California
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#ifndef __REPLACE_LOGGER__
#define __REPLACE_LOGGER__

#include <string>

namespace MacroPlace {

class Logger {

public:
  Logger(std::string name, int verbose);

  // Print functions
  void proc(std::string input, int verbose = 0);
  void procBegin(std::string input, int verbose = 0);
  void procEnd(std::string input, int verbose = 0);

  void error(std::string input, int code, int verbose = 0);

  void warn(std::string input, int code, int verbose = 0);

  void infoInt(std::string input, int val, int verbose = 0);
  void infoIntPair(std::string input, int val1, int val2, int verbose = 0);

  void infoInt64(std::string input, int64_t val, int verbose = 0);

  void infoFloat(std::string input, float val, int verbose = 0);
  void infoFloatPair(std::string input, float val1, float val2, int verbose = 0);
  void infoFloatSignificant(std::string input, float val, int verbose = 0);
  void infoFloatSignificantPair(std::string input, float val1, float val2, int verbose = 0);


  void infoString(std::string input, int verbose = 0);
  void infoString(std::string input, std::string val, int verbose = 0);

  void infoRuntime(std::string input, double runtime, int verbose = 0);


private:
  int verbose_;
  std::string name_;

};

}

#endif
