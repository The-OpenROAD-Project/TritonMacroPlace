///////////////////////////////////////////////////////////////////////////////
// Authors: Mingyu Woo
//          (Ph.D. Advisor: Andrew. B. Kahng)
//
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

#ifndef __MACRO_PLACER_MACRO__
#define __MACRO_PLACER_MACRO__ 

#include <string>

namespace sta { 
class Instance; 
}

namespace odb {
class dbInst;
}

namespace MacroPlace { 

class Vertex;
class Macro {
  public:
    std::string name;
    std::string type;  
    double lx, ly;
    double w, h;
    double haloX, haloY; 
    double channelX, channelY;
    Vertex* ptr;
    sta::Instance* staInstPtr;
    odb::dbInst* dbInstPtr;
    Macro( std::string _name, std::string _type, 
        double _lx, double _ly, 
        double _w, double _h,
        double _haloX, double _haloY, 
        double _channelX, double _channelY,
        Vertex* _ptr, sta::Instance* _staInstPtr,
        odb::dbInst* _dbInstPtr);
    void Dump(); 
};

}

class MacroLocalInfo {
private:
  double haloX_, haloY_, channelX_, channelY_;
public:
  MacroLocalInfo(); 
  void putHaloX(double haloX) { haloX_ = haloX; }
  void putHaloY(double haloY) { haloY_ = haloY; }
  void putChannelX(double channelX) { channelX_ = channelX; }
  void putChannelY(double channelY) { channelY_ = channelY; }
  double GetHaloX() { return haloX_; }
  double GetHaloY() { return haloY_; }
  double GetChannelX() { return channelX_; }
  double GetChannelY() { return channelY_; }
};


#endif
