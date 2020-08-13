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

#ifndef __MACRO_PLACER_PARTITION__
#define __MACRO_PLACER_PARTITION__


#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace MacroPlace {

class MacroCircuit;
class Macro;
class Logger;

enum PartClass {
  S, N, W, E, NW, NE, SW, SE, ALL, None
};

struct PartClassHash;
struct PartClassEqual;

class Partition {
  public:
    PartClass partClass;
    double lx, ly;
    double width, height;
    std::vector<Macro> macroStor;
    double* netTable;
    int tableCnt;
    std::unordered_map<std::string, int> macroMap;


    Partition();
    Partition(PartClass _partClass, double _lx, double _ly,
        double _width, double _height );

    // destructor
    ~Partition();

    // copy constructor
    Partition(const Partition& prev);

    // assign operator overloading
    Partition& operator= (const Partition& prev);

    void FillNetlistTable(MacroCircuit& mckt,
        std::unordered_map<PartClass, std::vector<int>,
        PartClassHash, PartClassEqual>& macroPartMap);

    void Dump();

    // Call Parquet to have annealing solution
    bool DoAnneal(std::shared_ptr<Logger>);

    // Update Macro location from MacroCircuit
    void UpdateMacroCoordi(MacroCircuit& mckt);

    // Writing functions
    void PrintSetFormat(FILE* fp);

    // Parquet print functions
    void PrintParquetFormat(std::string origName);
    void WriteBlkFile(std::string blkName);
    void WriteNetFile(std::vector< std::pair<int, int>> & netStor, std::string netName);
    void WriteWtsFile(std::vector<int>& costStor, std::string wtsName );
    void WritePlFile(std::string plName);

    std::string GetName(int macroIdx);

  private:
    void FillNetlistTableIncr();
    void FillNetlistTableDesc();
};

struct PartClassHash {
  std::size_t operator()(const MacroPlace::PartClass &k) const {
    return k;
  }
};

struct PartClassEqual {
  bool operator()(const MacroPlace::PartClass &p1,
      const MacroPlace::PartClass &p2) const {
    return p1 == p2;
  }
};

}



#endif
