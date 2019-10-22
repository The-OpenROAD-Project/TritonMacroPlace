///////////////////////////////////////////////////////////////////////////////
// Authors: Ilgweon Kang and Lutong Wang
//          (respective Ph.D. advisors: Chung-Kuan Cheng, Andrew B. Kahng),
//          based on Dr. Jingwei Lu with ePlace and ePlace-MS
//
//          Many subsequent improvements were made by Mingyu Woo
//          leading up to the initial release.
//
// BSD 3-Clause License
//
// Copyright (c) 2018, The Regents of the University of California
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
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//
//  This code was implemented by mgwoo at 18.01.17
//          
//          http://mgwoo.github.io/
//
///////////////////////////////////////////////////////

#ifndef __circuit__
#define __circuit__ 

#ifndef CIRCUIT_FPRINTF
#define CIRCUIT_FPRINTF(fmt, ...) {if(fmt) { fprintf(fmt, ##__VA_ARGS__); }}
#endif

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <climits>
#include <cfloat>
#include <limits>
#include <queue>
#include <unordered_map>

#include <stdint.h>

#include <lefrReader.hpp>
#include <lefwWriter.hpp>
#include <lefiDebug.hpp>
#include <lefiEncryptInt.hpp>
#include <lefiUtil.hpp>

#include <defrReader.hpp>
#include <defiAlias.hpp>

#define INIT_STR "!@#!@#"


namespace Circuit {

class Circuit {
  public:
    Circuit();
    // 
    // LEF/DEF is essential, 
    // but verilog is optional
    //
    Circuit(std::vector<std::string>& lefStor, 
        std::string defFilename, std::string verilogFilename = "" );

    void Init( std::vector<std::string>& lefStor, std::string defFilename, 
        std::string verilogFilename = "" );
   
    void WriteLef( FILE* _fout );
    void WriteDef( FILE* _fout );

    /////////////////////////////////////////////////////
    // LEF parsing
    //
    double lefVersion;
    std::string lefDivider;
    std::string lefBusBitChar;

    lefiUnits lefUnit;
    double lefManufacturingGrid;
    std::vector<lefiMacro> lefMacroStor;
    std::vector<lefiLayer> lefLayerStor;
    std::vector<lefiVia> lefViaStor;
    std::vector<lefiSite> lefSiteStor;

    // Macro, via, Layer's unique name -> index of lefXXXStor.
    std::unordered_map<std::string, int> lefMacroMap;
    std::unordered_map<std::string, int> lefViaMap;
    std::unordered_map<std::string, int> lefLayerMap;
    std::unordered_map<std::string, int> lefSiteMap;

    // this will maps 
    // current lefMacroStor's index
    // -> lefiPin, lefiObstruction
    //
    // below index is same with lefMacroStor
    std::vector<std::vector<lefiPin>> lefPinStor; // macroIdx -> pinIdx -> pinObj
    std::vector<std::unordered_map<std::string, int>> lefPinMapStor; // macroIdx -> pinName -> pinIdx
    std::vector<std::vector<lefiObstruction>> lefObsStor; // macroIdx -> obsIdx -> obsObj

    /////////////////////////////////////////////////////
    // DEF parsing
    //
    std::string defVersion;
    std::string defDividerChar;
    std::string defBusBitChar;
    std::string defDesignName;

    std::vector<defiProp> defPropStor;

    double defUnit;
    defiBox defDieArea;
    std::vector<defiRow> defRowStor;
    std::vector<defiTrack> defTrackStor;
    std::vector<defiGcellGrid> defGcellGridStor;
    std::vector<defiVia> defViaStor;

    defiComponentMaskShiftLayer defComponentMaskShiftLayer;
    std::vector<defiComponent> defComponentStor;
    std::vector<defiPin> defPinStor;
    std::vector<defiNet> defNetStor;
    std::vector<defiNet> defSpecialNetStor;

    // Component's unique name -> index of defComponentStor.
    std::unordered_map<std::string, int> defComponentMap;
    std::unordered_map<std::string, int> defPinMap;

    // ROW's Y coordinate --> Orient info
    std::unordered_map<int, int> defRowY2OrientMap;

    // this will maps
    // current defComponentStor's index + std::string pin Name
    // -> defNetStor indexes.
    //
    // below index is same with defComponentStor
    std::vector<std::unordered_map<std::string, int>> defComponentPinToNet;

  private:
    // Parsing function
    void ParseLef(std::vector<std::string>& lefStor);
    void ParseDef(std::string filename);
    // void ParseVerilog(std::string filename);


    /////////////////////////////////////////////////////
    // LEF Writing 
    //

    // for Dump Lef
    void DumpLefVersion();
    void DumpLefDivider();
    void DumpLefBusBitChar();

    void DumpLefUnit();
    void DumpLefManufacturingGrid();
    void DumpLefLayer();
    void DumpLefSite();
    void DumpLefMacro();

    void DumpLefPin(lefiPin* pin);
    void DumpLefObs(lefiObstruction* obs);

    void DumpLefVia();
    void DumpLefDone();


    /////////////////////////////////////////////////////
    // DEF Writing 
    //

    void DumpDefVersion();
    void DumpDefDividerChar();
    void DumpDefBusBitChar();
    void DumpDefDesignName();

    void DumpDefProp();
    void DumpDefUnit();

    void DumpDefDieArea(); 
    void DumpDefRow();
    void DumpDefTrack();
    void DumpDefGcellGrid();
    void DumpDefVia();

    void DumpDefComponentMaskShiftLayer();
    void DumpDefComponent();
    void DumpDefPin();
    void DumpDefSpecialNet();
    void DumpDefNet();
    void DumpDefDone();

    void DumpDefComponentPinToNet();

};

// Verilog net Info Storage
struct NetInfo {
  int macroIdx;
  int compIdx;
  int pinIdx;

  NetInfo( int _macroIdx, int _compIdx, int _pinIdx);
};

extern Circuit __ckt; 

}

struct DieRect {
  int llx, lly, urx, ury;
  DieRect();
  bool isNotInitialize ();
  void Dump();
};

DieRect GetDieFromProperty();
DieRect GetDieFromDieArea();

void ParseInput();
void ParseLefDef();

void ReadPl(const char* fileName);
void ReadPlLefDef(const char* fileName);

void WriteDef(const char* defOutput);

void GenerateModuleTerminal(Circuit::Circuit& __ckt);
void GenerateRow(Circuit::Circuit& __ckt);

void GenerateNetDefOnly(Circuit::Circuit& __ckt);
void GenerateNetDefVerilog(Circuit::Circuit& __ckt);

// custom scale down parameter setting during the stage
void SetUnitY(float  _unitY);
void SetUnitY(double _unitY);

#endif

