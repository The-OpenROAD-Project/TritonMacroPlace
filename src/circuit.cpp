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

#include <iostream>
#include <queue>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set> 

#include "circuit.h"
#include "logger.h"

// None of these are necessary because they are included in circuit.h -cherry
#include "sta/Graph.hh"
#include "sta/Sta.hh"
#include "sta/Network.hh"
#include "sta/Liberty.hh"
#include "sta/Sdc.hh"
#include "sta/PortDirection.hh"
#include "sta/Corner.hh"
#include "sta/PathExpanded.hh"
#include "sta/PathEnd.hh"
#include "sta/PathRef.hh"
#include "sta/Search.hh"

#include "db_sta/dbSta.hh"
#include "db_sta/dbNetwork.hh"

namespace MacroPlace {

// None of these are necessary because they are included in circuit.h -cherry
using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::to_string;
using std::cout;
using std::endl;

// None of these are necessary because they are included in circuit.h -cherry
using Eigen::VectorXf;
typedef Eigen::SparseMatrix<int, Eigen::RowMajor> SMatrix;
typedef Eigen::Triplet<int> T;

// Really bad to use a namespace -cherry
using namespace odb;

using sta::VertexIterator;


static bool
isNotVisited(MacroPlace::Vertex* vert, 
    vector<MacroPlace::Vertex*> &path);

static bool 
isTerminal(MacroPlace::Vertex* vert, 
    MacroPlace::Vertex* target);

static size_t 
TrimWhiteSpace(char *out, size_t len, 
    const char *str);

static MacroPlace::PinGroupLocation
getPinGroupLocation(
    int cx, int cy, 
    int dieLx, int dieLy, int dieUx, int dieUy);

static std::string
getPinGroupLocationString(PinGroupLocation pg);

static bool 
isMacroType(odb::dbMasterType mType);

static bool
isMissingLiberty(sta::Sta* sta,
    vector<Macro>& macroStor);

MacroCircuit::MacroCircuit() 
  : db_(nullptr), sta_(nullptr), 
  log_(nullptr),
  isTiming_(false),
  isPlot_(false),
  lx_(0), ly_(0), ux_(0), uy_(0),
  siteSizeX_(0), siteSizeY_(0),
  haloX_(0), haloY_(0), 
  channelX_(0), channelY_(0), 
  netTable_(nullptr),
  verbose_(1),
  fenceRegionMode_(false) {}

MacroCircuit::MacroCircuit(
    odb::dbDatabase* db,
    sta::dbSta* sta)
  : MacroCircuit() {

  db_ = db;
  sta_ = sta;
  init();
}

void
MacroCircuit::reset() {
  db_ = nullptr;
  sta_ = nullptr;
  isTiming_ = false;
  isPlot_ = false;
  lx_ = ly_ = ux_ = uy_ = 0;
  siteSizeX_ = siteSizeY_ = 0; 
  haloX_ = haloY_ = 0;
  channelX_ = channelY_ = 0;
  netTable_ = nullptr;
  globalConfig_ = localConfig_ = "";
  verbose_ = 1;
  fenceRegionMode_ = false; 
}

void 
MacroCircuit::setDb(odb::dbDatabase* db) {
  db_ = db; 
}

void
MacroCircuit::setSta(sta::dbSta* sta) {
  sta_ = sta; 
}

void
MacroCircuit::setGlobalConfig(const char* globalConfig) {
  globalConfig_ = globalConfig;
}

void
MacroCircuit::setLocalConfig(const char* localConfig) {
  localConfig_ = localConfig; 
}

void 
MacroCircuit::setPlotEnable(bool mode) {
  isPlot_ = true;
}

void
MacroCircuit::setVerboseLevel(int verbose) {
  verbose_ = verbose;
}

void
MacroCircuit::setFenceRegion(double lx, double ly, double ux, double uy) {
  fenceRegionMode_ = true;
  lx_ = lx; 
  ly_ = ly; 
  ux_ = ux;
  uy_ = uy;
}

void MacroCircuit::init() {
  log_ = std::make_shared<Logger>("MAPL", verbose_);

  dbBlock* block = db_->getChip()->getBlock();

  dbSet<dbRow> rows = block->getRows();
  if( rows.size() == 0 ) { 
    log_->error("DEF must contain ROW", 1);
    exit(1);
  }

  const double dbu = db_->getTech()->getDbUnitsPerMicron();

  dbSite* site = rows.begin()->getSite();

  siteSizeX_ = site->getWidth() / dbu;
  siteSizeY_ = site->getHeight() / dbu;

  // if fenceRegion is not set
  // (lx, ly) - (ux, uy) becomes core area
  if( !fenceRegionMode_ ) {
    odb::Rect coreRect;
    block->getCoreArea(coreRect);

    lx_ = coreRect.xMin() / dbu;
    ly_ = coreRect.yMin() / dbu;
    ux_ = coreRect.xMax() / dbu;
    uy_ = coreRect.yMax() / dbu;
  }

  // parsing from cfg file
  // global config
  ParseGlobalConfig(globalConfig_);
  // local config (optional)
  if( localConfig_ != "" ) {
    ParseLocalConfig(localConfig_);
  }

  sta_->updateTiming(0);

  FillMacroStor(); 
  FillPinGroup(); 
  UpdateInstanceToMacroStor();

  // Timing-related feature will be skipped
  // if there is some of liberty is missing.
  isTiming_ = !isMissingLiberty(sta_, macroStor);

  if( isTiming_ ) {
    FillVertexEdge();
    UpdateVertexToMacroStor();
    FillMacroPinAdjMatrix();
    FillMacroConnection();
  }
  else {
    string msg = "Missing Liberty Detected.\n";
    msg += "       TritonMP will place macros without timing information";
    log_->warn(msg, 1); 

    UpdateVertexToMacroStor();
  }
}

static bool 
isMacroType(odb::dbMasterType mType) {
  return mType.isBlock();
}

void MacroCircuit::FillMacroStor() {
  log_->procBegin("Extracting Macro Cells");

  dbTech* tech = db_->getTech();
  dbBlock* block = db_->getChip()->getBlock();
  
  dbSet<dbRow> rows = block->getRows();

  Rect dieBox;
  rows.begin()->getBBox(dieBox);

  int cellHeight = dieBox.dy();
  const int dbu = tech->getDbUnitsPerMicron();

  for(dbInst* inst : block->getInsts() ){ 
    // Skip for standard cells
    if( (int)inst->getBBox()->getDY() <= cellHeight) { 
      continue;
    }

    // only concerns about macroType cells.
    if( !isMacroType(inst->getMaster()->getType()) ) {
      continue;
    }

    // for Macro cells
    dbPlacementStatus dps = inst->getPlacementStatus();
    if( dps == dbPlacementStatus::NONE ||
        dps == dbPlacementStatus::UNPLACED ) {
      string msg = "  Macro : " + string(inst->getConstName()) + " is Unplaced.\n";
      msg += "        Please use TD-MS-RePlAce to get a initial solution\n";
      msg += "        before executing TritonMacroPlace\n";
      log_->error(msg, 2);
      exit(1); 
    }
    
    double curHaloX =0, curHaloY = 0, curChannelX = 0, curChannelY = 0;
    auto mlPtr = macroLocalMap.find( inst->getConstName() );
    if( mlPtr == macroLocalMap.end() ) {
      curHaloX = haloX_;
      curHaloY = haloY_; 
      curChannelX = channelX_;
      curChannelY = channelY_;
    }
    else {
      MacroLocalInfo& m = mlPtr->second;
      curHaloX = (m.GetHaloX() == 0)? haloX_ : m.GetHaloX();
      curHaloY = (m.GetHaloY() == 0)? haloY_ : m.GetHaloY();
      curChannelX = (m.GetChannelX() == 0)? channelX_ : m.GetChannelX();
      curChannelY = (m.GetChannelY() == 0)? channelY_ : m.GetChannelY();
    }

    macroNameMap[ inst->getConstName() ] = macroStor.size();
    
    int placeX, placeY;
    inst->getLocation( placeX, placeY );
     
    MacroPlace::Macro 
      tmpMacro( inst->getConstName(), 
          inst->getMaster()->getConstName(), 
          1.0*placeX/dbu, 
          1.0*placeY/dbu,
          1.0*inst->getBBox()->getDX()/dbu, 
          1.0*inst->getBBox()->getDY()/dbu, 
          curHaloX, curHaloY, 
          curChannelX, curChannelY,  
          nullptr, nullptr, inst );
    macroStor.push_back( tmpMacro ); 
  }

  if( macroStor.size() == 0 ) {
    log_->error("Cannot find any macros in this design.\n", 3);
    exit(1);
  }

  log_->procEnd("Extracting Macro Cells");
  log_->infoInt("NumMacros", macroStor.size());
}

static bool 
isWithIn( int val, int min, int max ) {
  return (( min <= val ) && ( val <= max ));
}


void 
MacroCircuit::FillPinGroup(){
  dbTech* tech = db_->getTech(); 
  const double dbu = tech->getDbUnitsPerMicron();

  log_->infoInt("NumEdgeInSta", sta_->graph()->edgeCount());
  log_->infoInt("NumVertexInSta", sta_->graph()->vertexCount());

  int dbuCoreLx = static_cast<int>(round(lx_ * dbu));
  int dbuCoreLy = static_cast<int>(round(ly_ * dbu));
  int dbuCoreUx = static_cast<int>(round(ux_ * dbu));
  int dbuCoreUy = static_cast<int>(round(uy_ * dbu));

  // never makes sense to 'using' inside a function -cherry
  using MacroPlace::PinGroupLocation;

  // this is always four array.
  pinGroupStor.resize(4);

  // save PG-Class info in below
  pinGroupStor[static_cast<int>(East)].setPinGroupLocation(East);
  pinGroupStor[static_cast<int>(West)].setPinGroupLocation(West);
  pinGroupStor[static_cast<int>(North)].setPinGroupLocation(North);
  pinGroupStor[static_cast<int>(South)].setPinGroupLocation(South);
  
  dbBlock* block = db_->getChip()->getBlock();

  for(dbBTerm* bTerm : block->getBTerms()) {
    
    // pin signal type
    dbSigType psType = bTerm->getSigType();
    if( psType == dbSigType::GROUND ||
        psType == dbSigType::POWER ) {
      continue;
    }

    // pin placement status
    dbPlacementStatus ppStatus = bTerm->getFirstPinPlacementStatus();

    // unplaced BTerms 
    if( ppStatus == dbPlacementStatus::UNPLACED ||
        ppStatus == dbPlacementStatus::NONE ) {
      string msg = string(bTerm->getConstName()) 
        + " toplevel port is not placed!\n";
      msg += "       TritonMP will regard " 
        + string(bTerm->getConstName()) + " is placed on West side"; 
      log_->warn(msg, 1);
          
      // update pinGroups on West 
      sta::Pin* pin = sta_->getDbNetwork()->dbToSta(bTerm);
      pinGroupStor[static_cast<int>(West)].addPin(pin);
      staToPinGroup[pin] = static_cast<int>(West);
    } 
    else {
      int placeX = 0, placeY = 0;
      bTerm->getFirstPinLocation(placeX, placeY);
      PinGroupLocation pgLoc;

      bool isAxisFound = false;
      for(dbBPin* bPin : bTerm->getBPins()) {
        int boxLx = bPin->getBox()->xMin();
        int boxLy = bPin->getBox()->yMin(); 
        int boxUx = bPin->getBox()->xMax();
        int boxUy = bPin->getBox()->yMax();

        if( isWithIn( dbuCoreLx, boxLx, boxUx ) ) {
          pgLoc = West;
          isAxisFound = true;
          break;
        }
        else if( isWithIn( dbuCoreUx, boxLx, boxUx ) ) {
          pgLoc = East;
          isAxisFound = true;
          break;
        }
        else if( isWithIn( dbuCoreLy, boxLy, boxUy ) ) {
          pgLoc = South;
          isAxisFound = true;
          break;
        }
        else if( isWithIn( dbuCoreUy, boxLy, boxUy ) ) {
          pgLoc = North;
          isAxisFound = true;
          break;
        }
      } 
      if( !isAxisFound ) {
        dbBPin* bPin = *(bTerm->getBPins().begin());
        int boxLx = bPin->getBox()->xMin();
        int boxLy = bPin->getBox()->yMin(); 
        int boxUx = bPin->getBox()->xMax();
        int boxUy = bPin->getBox()->yMax();
        pgLoc = getPinGroupLocation( 
            (boxLx + boxUx)/2, (boxLy + boxUy)/2,
            dbuCoreLx, dbuCoreLy, dbuCoreUx, dbuCoreUy); 
      } 
        
      // update pinGroups 
      sta::Pin* pin = sta_->getDbNetwork()->dbToSta(bTerm);
      pinGroupStor[static_cast<int>(pgLoc)].addPin(pin);
      staToPinGroup[pin] = static_cast<int>(pgLoc);
    }
  }

  log_->infoInt("NumEastPins", pinGroupStor[static_cast<int>(East)].pins().size());
  log_->infoInt("NumWestPins", pinGroupStor[static_cast<int>(West)].pins().size());
  log_->infoInt("NumNorthPins", pinGroupStor[static_cast<int>(North)].pins().size());
  log_->infoInt("NumSouthPins", pinGroupStor[static_cast<int>(South)].pins().size());
}

void MacroCircuit::FillVertexEdge() {
  log_->procBegin("Generating Sequential Graph"); 

  Eigen::setNbThreads(8);
  unordered_set<sta::Instance*> instMap;

  // Fill Vertex for Four IO cases.
  for(int i=0; i<4; i++) {
    pinInstVertexMap[ (void*) &pinGroupStor[i] ] 
      = vertexStor.size();

    vertexStor.push_back( 
        MacroPlace::Vertex(&pinGroupStor[i]) );
  }

  // Fill Vertex for FF/Macro cells 
  VertexIterator vIter1(sta_->graph());

  while(vIter1.hasNext()) {
    sta::Vertex* staVertex = vIter1.next();
    sta::Pin* pin = staVertex->pin();
   
    // skip for top-level port 
    bool isTopPin = sta_->network()->isTopLevelPort(pin);
    if( isTopPin ) {
      continue;
    }
    
    // skip for below two cases; non-FF cells
    sta::Instance* inst 
      = sta_->network()->instance(pin);
    sta::LibertyCell* libCell 
      = sta_->network()->libertyCell(inst);

    if( !libCell -> hasSequentials() 
        && macroInstMap.find(inst) == macroInstMap.end() ) {
      continue;
    }  
    
    // skip for below two cases; non visited
    if( instMap.find(inst ) != instMap.end()) {
      continue;
    }
    instMap.insert( inst );

    pair<void*, VertexType> vertex = GetPtrClassPair( pin );
    auto vertPtr = pinInstVertexMap.find( vertex.first );

    if( vertPtr == pinInstVertexMap.end()) {
      pinInstVertexMap[ vertex.first ] = vertexStor.size();
      vertexStor.push_back( 
          MacroPlace::Vertex(vertex.first, vertex.second)); 
    }
  }
  
  // VertexPtrMap Initialize
  for(auto& curVertex: vertexStor) {
    vertexPtrMap[&curVertex] = &curVertex - &vertexStor[0];
  }
  
  adjMatrix.resize( vertexStor.size(), vertexStor.size() ); 
  vector< T > tripletList;

  // Query Get_FanIn/ Get_FanOut
  VertexIterator vIter2(sta_->graph());

  while(vIter2.hasNext()) {
    sta::Vertex* staVertex = vIter2.next();
    sta::Pin* pin = staVertex->pin();
    
    bool isTopPin = sta_->network()->isTopLevelPort(pin);
    // !!!!!!!!!!!!!!!!
    // Future support of OpenSTA
    if( isTopPin ) {
      continue;
    }

    // Skip For Non-FF Cells
    if( !isTopPin ) {
      sta::Instance* inst = sta_->network()->instance(pin);
      sta::LibertyCell* libCell = sta_->network()->libertyCell(inst);
      if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
        continue;
      }
    }

    //skip for clock pin
    if( sta_->network()->isCheckClk(pin) || sta_->sdc()->isClock(pin) ) {
      continue;
    }

    sta::PinSeq pinStor;
    pinStor.push_back(pin);
  
    sta::PortDirection* dir = sta_->network()->direction(pin);
    MacroPlace::Vertex* curVertex = GetVertex(pin);


    // Query for get_fanin/get_fanout
    if( dir->isAnyOutput() ) {
      sta::PinSet *fanout = sta_->findFanoutPins(&pinStor, false, true,
          500, 700,
          false, false);
      for(auto& adjPin: *fanout) {
        // Skip For Non-FF Pin 
        if( !sta_->network()->isTopLevelPort(adjPin) ) {
          sta::Instance* inst = sta_->network()->instance(adjPin);
          sta::LibertyCell* libCell = sta_->network()->libertyCell(inst);
          if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
            continue;
          }
        }
        
        MacroPlace::Vertex* adjVertex = GetVertex(adjPin);

        if( adjVertex == curVertex ) {
          continue;
        }

        //skip for clock pin
        if( sta_->network()->isCheckClk(adjPin) || sta_->sdc()->isClock(adjPin) ) {
          continue;
        }

        tripletList.push_back(T(vertexPtrMap[curVertex], vertexPtrMap[adjVertex], 1));
      }
      delete fanout;
    }
    else {
      sta::PinSet *fanin = sta_->findFaninPins(&pinStor, false, true ,
          500, 700,
          false, false);
      for(auto& adjPin: *fanin) {
        // Skip For Non-FF Pin 
        if( !sta_->network()->isTopLevelPort(adjPin) ) {
          sta::Instance* inst = sta_->network()->instance(adjPin);
          sta::LibertyCell* libCell = sta_->network()->libertyCell(inst);
          if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
            continue;
          }
        }
        
        MacroPlace::Vertex* adjVertex = GetVertex(adjPin);

        if( adjVertex == curVertex ) {
          continue;
        }
        
        //skip for clock pin
        if( sta_->network()->isCheckClk(adjPin) || sta_->sdc()->isClock(adjPin) ) {
          continue;
        }

        tripletList.push_back(T(vertexPtrMap[curVertex], vertexPtrMap[adjVertex], 1));
      }
      delete fanin;
    }
  }
  // Query find_timing_paths 
  VertexIterator vIter3(sta_->graph());

  while(vIter3.hasNext()) {
    sta::Vertex* staVertex = vIter3.next();
    sta::Pin* pin = staVertex->pin();
    
    bool isTopPin = sta_->network()->isTopLevelPort(pin);
    // only query for IO/Pins
    if( !isTopPin ) {
      continue;
    }

    // Survived
    // FF/Macro with non-clock output pins
    // topLevelPort(I). i.e. input top-level-port.
    
    //skip for clock pin
    if( sta_->network()->isCheckClk(pin) || sta_->sdc()->isClock(pin) ) {
      continue;
    }
    if( string(sta_->network()->pathName(pin)) == "reset_i" ) {
      continue;
    }

    sta::Corner* corner = sta_->corners()->findCorner(0);

    sta::PinSet* pSet = new sta::PinSet;
    pSet->insert(pin);

    // Get Pin direction
    sta::PortDirection* dir = sta_->network()->direction(pin);

    sta::ExceptionFrom* from = (!dir->isAnyOutput())? 
      sta_->sdc()->makeExceptionFrom(pSet, nullptr, nullptr, 
        sta::RiseFallBoth::riseFall()) : nullptr;

    sta::ExceptionTo* to = (dir->isAnyOutput())? 
      sta_->sdc()->makeExceptionTo(pSet, nullptr, nullptr, 
        sta::RiseFallBoth::riseFall(), 
        sta::RiseFallBoth::riseFall()) : nullptr;

    sta::PathEndSeq *ends = sta_->findPathEnds(from, nullptr, to, //from, thru, to
        false,
        corner, sta::MinMaxAll::max(), // corner, delay_min_max
        INT_MAX, 1, false, //group_count, endpoint_count, unique_pins
        -sta::INF, sta::INF, //slack_min, slack_max
        false, nullptr, //sort_by_slack, group_name
        true, true, // setup, hold
        true, true, // recovery, removal
        true, true); // clk gating setup, hold

    if (ends->empty()) {
      continue;
    }

    sta::PathEndSeq::Iterator tmpIter(ends), pathEndIter(ends), pathEndIter2(ends);
    int edgeCnt = 0;
    while( tmpIter.hasNext() ) {
      tmpIter.next();
      edgeCnt ++; 
    }

    while( pathEndIter.hasNext()) {
      sta::PathEnd *end = pathEndIter.next();
      //TimingPathPrint( sta_, end );

      sta::PathExpanded expanded(end->path(), sta_);

      // get Un-clockpin 
      int startIdx = 0;
      sta::PathRef *startPath = expanded.path(startIdx);
      while( startIdx < (int)expanded.size()-1 && startPath->isClock(sta_->search())) {
        startPath = expanded.path(++startIdx);
      }

      sta::PathRef *endPath = expanded.path(expanded.size()-1);
      sta::Vertex *startVert = startPath->vertex(sta_);
      sta::Vertex *endVert = endPath->vertex(sta_);

      sta::Pin* startPin = startVert->pin();
      sta::Pin* endPin = endVert->pin();

      MacroPlace::Vertex* startVertPtr = GetVertex( startPin );
      MacroPlace::Vertex* endVertPtr = GetVertex( endPin );


      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // OpenSTA could return Null Vertex:
      // This means that non-Timing cells would be returned by findPathEnds command...
      if( startVertPtr == nullptr || endVertPtr == nullptr) {
        continue;
      }

      tripletList.push_back(T(vertexPtrMap[startVertPtr], vertexPtrMap[endVertPtr], 1));
    }
  }

  adjMatrix.setFromTriplets( tripletList.begin(), tripletList.end() );
  
  log_->procEnd("Generating Sequential Graph"); 
  log_->infoInt("NumVertexSeqGraph", vertexStor.size());
  log_->infoInt("NumEdgeSeqGraph", adjMatrix.nonZeros());
}

void MacroCircuit::CheckGraphInfo() {
  
  vector<MacroPlace::Vertex*> searchVert;

  for(auto& curMacro: macroStor) {
    searchVert.push_back( curMacro.ptr );
  }

  for(int i=0; i<4; i++) {
    searchVert.push_back( &vertexStor[i] );
  }
  
#define CHECK_LEVEL_MAX 3

  
  vector<int> vertexCover(vertexStor.size(), -1);
  for(int i=0; i<4; i++) {
    vertexCover[i] = 0;
  }
  for(auto& curMacro: macroStor) {
    auto vpPtr = vertexPtrMap.find(curMacro.ptr);
    vertexCover[vpPtr->second] = 0;
  }


  for(int level = 1; level <= CHECK_LEVEL_MAX;  level++) {
    vector<MacroPlace::Vertex*> newVertex;

    for(auto& curVertex1: searchVert) {
      // for all other vertex
      for(auto& curVertex2: vertexStor) {
        int idx2 = &curVertex2 - &vertexStor[0];

        // skip for same pointer
        if( curVertex1->ptr() == &curVertex2) {
          continue;
        }

        if( vertexCover[idx2] == -1 && 
            GetPathWeightMatrix(adjMatrix, curVertex1, idx2) ) {
          vertexCover[idx2] = level;
          newVertex.push_back( &vertexStor[idx2]);
        }
      }
    }
    newVertex.swap(searchVert);
  }

  int sumArr[CHECK_LEVEL_MAX+1] = {0, };
  for(size_t i=0; i<vertexStor.size(); i++) {
    if( vertexCover[i] != -1 ) {
      sumArr[ vertexCover[i] ] ++;
    }
  }

  for(int i=0; i<=CHECK_LEVEL_MAX; i++) {
    cout << "level " << i << " " << sumArr[i] << endl;
  }
}

void MacroCircuit::FillMacroPinAdjMatrix() {

  macroPinAdjMatrixMap.resize(vertexStor.size(), -1);
  int macroPinAdjIdx = 0;

  vector<int> searchVertIdx;
  // 
  // for each pin vertex
  //
  // macroPinAdjMatrix's 0, 1, 2, 3 is equal to original adjMatrix' index.
  // e.g. pin index is the exactly same.
  //
  for(int i=0; i<4; i++) {
    searchVertIdx.push_back(i);
    macroPinAdjMatrixMap[i] = macroPinAdjIdx++;
  }
  
  // 
  // for each macro vertex
  //
  // Update macroPinAdjMatrixMap 
  // to have macroIdx --> updated macroPinAdjMatrix's index.
  //
  for(auto& curMacro: macroStor) {
    auto vpPtr = vertexPtrMap.find(curMacro.ptr);
    int macroVertIdx = vpPtr->second;
    searchVertIdx.push_back( macroVertIdx );
    macroPinAdjMatrixMap[macroVertIdx] = macroPinAdjIdx++;
  }

  // Do BFS search by LEVEL 3
#define MPLACE_BFS_MAX_LEVEL 3

  const int EmptyVert = -1, SearchVert = -2;

  // return adjMatrix triplet candidates.
  vector< T > tripletList;

  // for each macro/pin vertex 
  for(auto& startVertIdx: searchVertIdx) {
   
    // initial starting points 
    vector<int> candiVert;
    candiVert.push_back(startVertIdx);

    // initialize vertexWeight and vertexCover.
    vector<int> vertexWeight(vertexStor.size(), 0);
    vector<int> vertexCover(vertexStor.size(), EmptyVert);

    for(auto& curVertIdx : searchVertIdx) {
      vertexCover[curVertIdx] = SearchVert; 
    }
    vertexWeight[startVertIdx] = 1;
    

    // BFS search up to MPLACE_BFS_MAX_LEVEL  
    for(int level = 1; level <= MPLACE_BFS_MAX_LEVEL; level++) {
      vector<int> nextCandiVert;

      for(auto& idx1: candiVert) {

        // for all other vertex
        for(auto& curVertex2: vertexStor) {
          int idx2 = &curVertex2 - &vertexStor[0];

          // skip for same vertex
          if( idx1 == idx2 ) {
            continue;
          }

          // if( vertexCover[idx2] == -1 && 
          // GetPathWeightMatrix(adjMatrix, idx1, idx2) ) 
          if(vertexCover[idx2] == EmptyVert || vertexCover[idx2] == SearchVert) {
            int pathWeight = GetPathWeightMatrix(adjMatrix, idx1, idx2);
            if( pathWeight == 0 ) { 
              continue;
            }
            vertexWeight[idx2] += pathWeight * vertexWeight[idx1];

            // update vertexCover only when vertex is FFs.
            if( vertexCover[idx2] == EmptyVert ) { 
              vertexCover[idx2] = level;
            }

            // prevent multi-search for vertex itself
            if( vertexCover[idx2] != SearchVert ) {
              nextCandiVert.push_back( idx2 );
            }
          }
        }
      }
      // for next level search,
      nextCandiVert.swap(candiVert);
    }
    
    for(auto& curCandiVert: searchVertIdx) {
      if( curCandiVert == startVertIdx ) {
        continue;
      }
      tripletList.push_back( 
          T(macroPinAdjMatrixMap[startVertIdx], 
            macroPinAdjMatrixMap[curCandiVert], 
            vertexWeight[curCandiVert]));
    }
  } 

  // Fill in all of vertex weights into compacted adjMatrix
  macroPinAdjMatrix.resize( searchVertIdx.size(), searchVertIdx.size() );
  macroPinAdjMatrix.setFromTriplets(tripletList.begin(), tripletList.end());
}

void MacroCircuit::FillMacroConnection() {
  
  vector<int> searchVertIdx;
  for(int i=0; i<4; i++) {
    searchVertIdx.push_back( i );
  }

  for(auto& curMacro: macroStor) {
    auto vpPtr = vertexPtrMap.find(curMacro.ptr);
    int macroVertIdx = vpPtr->second;
    searchVertIdx.push_back( macroVertIdx );
  }

  // macroNetlistWeight Initialize
  macroWeight.resize(searchVertIdx.size());
  for(size_t i=0; i<searchVertIdx.size(); i++) {
    macroWeight[i] = vector<int> (searchVertIdx.size(), 0);
  }

  for(auto& curVertex1: searchVertIdx) {
    for(auto& curVertex2: searchVertIdx) {
      if( curVertex1 == curVertex2 ) { 
        continue;
      }

      VertexType class1 = vertexStor[curVertex1].vertexType();
      VertexType class2 = vertexStor[curVertex2].vertexType();

      // no need to fill in PIN -> PIN connections
      if( class1 == VertexType::PinGroupType && class2 == VertexType::PinGroupType) {
        continue;
      }
     
      void* ptr1 = vertexStor[curVertex1].ptr();
      void* ptr2 = vertexStor[curVertex2].ptr();

      string name1 = (class1 == VertexType::PinGroupType)? 
        ((PinGroup*)ptr1)->name() : sta_->network()->pathName((sta::Instance*)ptr1);
      string name2 = (class2 == VertexType::PinGroupType)?
        ((PinGroup*)ptr2)->name() : sta_->network()->pathName((sta::Instance*)ptr2);

      macroWeight[macroPinAdjMatrixMap[curVertex1]][macroPinAdjMatrixMap[curVertex2]] 
        = GetPathWeightMatrix( 
            macroPinAdjMatrix, 
            macroPinAdjMatrixMap[curVertex1], 
            macroPinAdjMatrixMap[curVertex2]);
    }
  }
}

// macroStor Update
void MacroCircuit::UpdateVertexToMacroStor() {
  for(auto& curVertex: vertexStor) {
    if( curVertex.vertexType() != VertexType::MacroInstType ) {
      continue;
    }

    sta::Instance* staInst = (sta::Instance*) curVertex.ptr();
    auto mPtr = macroInstMap.find( staInst );
    if( mPtr == macroInstMap.end() ) {
      cout << "**ERROR: The Macro Name must be in macro NameMap" <<  endl;
      exit(1);
    } 

    macroStor[mPtr->second].ptr = &curVertex;
  }
}

// macroStr & macroInstMap update
void MacroCircuit::UpdateInstanceToMacroStor() {
  for(auto& macro: macroStor) {
    sta::Instance* staInst = sta_->getDbNetwork()->dbToSta(macro.dbInstPtr);
    macro.staInstPtr = staInst;
    macroInstMap[staInst] = &macro - &macroStor[0];
  }
}

pair<void*, VertexType> 
MacroCircuit::GetPtrClassPair( sta::Pin* pin ) {

  pair<void*, VertexType> ret;
  bool isTopPin = sta_->network()->isTopLevelPort(pin);

  // toplevel pin
  if( isTopPin ) {
    auto pgPtr = staToPinGroup.find( pin );
    if( pgPtr == staToPinGroup.end()) {
      cout << "ERROR: " << sta_->network()->pathName(pin) << " not exists in PinGroupMap" << endl;
      exit(1);
    }

    // pinGroupPointer
    ret.first = (void*) &pinGroupStor[pgPtr->second];
    ret.second = VertexType::PinGroupType; 
  }
  else {
    sta::Instance* inst = sta_->network()->instance(pin);
    string instName = sta_->network()->pathName(inst);
    
    ret.first = (void*) inst;
    ret.second = (macroNameMap.find( instName ) != macroNameMap.end())? 
      VertexType::MacroInstType : VertexType::OtherInstType;
  }
  return ret;
}


int 
MacroCircuit::GetPathWeight(MacroPlace::Vertex* from, MacroPlace::Vertex* to, int limit ) {

  std::queue< vector<MacroPlace::Vertex*> > q;
  vector<MacroPlace::Vertex*> path;

  path.reserve(limit+2);
  path.push_back(from);

  q.push(path);
  vector< vector<MacroPlace::Vertex*> > result;

  int pathDepth = 1;
  cout << "Depth: " << 1 << endl;

  while(!q.empty()) {
    path = q.front();
    q.pop();
    MacroPlace::Vertex* last = path[path.size()-1];

    if( pathDepth < (int)path.size() ){
      cout << "Depth: " << path.size() << endl;
      pathDepth = path.size();
    }

    if( last == to ) { 
      result.push_back(path);
      continue;
    }

    for(auto& curOutEdge: last->to()) {
      MacroPlace::Vertex* nextVert = edgeStor[curOutEdge].to();

      if( !isTerminal(nextVert, to) && (int)path.size() < limit 
          && isNotVisited(nextVert, path)) {
        vector<MacroPlace::Vertex*> newPath(path);

        newPath.push_back(nextVert);
        q.push(newPath);
      } 
    }
    for(auto& curInEdge: last->from()) {
      MacroPlace::Vertex* nextVert = edgeStor[curInEdge].from();

      if( !isTerminal(nextVert, to) && (int)path.size() < limit 
          && isNotVisited(nextVert, path)) {
        vector<MacroPlace::Vertex*> newPath(path);

        newPath.push_back(nextVert);
        q.push(newPath);
      } 
    }
  }

  int ret = 0;
  for(auto& curPath: result) {
    for(size_t i=0; i<curPath.size()-1; i++) {
      auto vPtr = vertexPairEdgeMap.find( make_pair(curPath[i], curPath[i+1]) );
      if( vPtr == vertexPairEdgeMap.end() ) {
        vPtr = vertexPairEdgeMap.find( make_pair(curPath[i+1], curPath[i]));
      }
      if( vPtr == vertexPairEdgeMap.end() ) {
        cout << "**ERROR: vertex Pair Edge Map is wrong!" << endl;
        exit(1);
      }
      ret += edgeStor[vPtr->second].weight();
    }
    cout << " " ;
    for(auto& curVert: curPath) {
      MacroPlace::PinGroup* ptr = (MacroPlace::PinGroup*) curVert->ptr();

      string name = (curVert->vertexType() == VertexType::PinGroupType)?
        ptr->name() :
        sta_->network()->pathName((sta::Instance*)curVert->ptr());
      cout << name << " -> ";
    }
    cout << endl;
  }
  return ret;
}

int MacroCircuit::GetPathWeightMatrix(
    SMatrix& mat, MacroPlace::Vertex* from, MacroPlace::Vertex* to) {

  auto vpPtr = vertexPtrMap.find(from);
  if( vpPtr == vertexPtrMap.end()) {
    exit(1);
  }
  int idx1 = vpPtr->second;
  
  auto vpPtr2 = vertexPtrMap.find(to);
  if( vpPtr2 == vertexPtrMap.end()) {
    exit(1);
  }
  int idx2 = vpPtr2->second;

  return mat.coeff(idx1, idx2);
}

int MacroCircuit::GetPathWeightMatrix(
    SMatrix& mat, MacroPlace::Vertex* from, int toIdx) {

  auto vpPtr = vertexPtrMap.find(from);
  if( vpPtr == vertexPtrMap.end()) {
    exit(1);
  }
  int idx1 = vpPtr->second;
  return mat.coeff(idx1, toIdx);
}

int MacroCircuit::GetPathWeightMatrix(
    SMatrix& mat, int fromIdx, int toIdx) {
  return mat.coeff(fromIdx, toIdx);
}

static float getRoundUpFloat( float x, float unit ) { 
  int roundVal = static_cast<int>(x / unit + 0.5f);
  return static_cast<float>(roundVal) * unit;
}

// 
// Update Macro Location
// from partition
void MacroCircuit::UpdateMacroCoordi( MacroPlace::Partition& part) {
  dbTech* tech = db_->getTech();
  dbTechLayer* fourLayer = tech->findRoutingLayer( 4 );
  if( !fourLayer ) {
    cout << "WARNING: Metal 4 not exist! " << endl;
    cout << "         Macro snapping will not be applied on Metal4 pitch" << endl;
  }

  const float pitchX = static_cast<float>(fourLayer->getPitchX()) 
    / static_cast<float>(tech->getDbUnitsPerMicron());
  const float pitchY = static_cast<float>(fourLayer->getPitchY()) 
    / static_cast<float>(tech->getDbUnitsPerMicron());

  for(auto& curMacro : part.macroStor) {
    auto mnPtr = macroNameMap.find(curMacro.name);
    if( mnPtr == macroNameMap.end() ) {
      cout << "ERROR: Macro not exists in MacroCircuit " << curMacro.name << endl;
      exit(1);
    }

    // update macro coordi
    float macroX = (fourLayer)? getRoundUpFloat( curMacro.lx, pitchX ) : curMacro.lx;
    float macroY = (fourLayer)? getRoundUpFloat( curMacro.ly, pitchY ) : curMacro.ly;

    // Update Macro Location
    int macroIdx = mnPtr->second;
    macroStor[macroIdx].lx = macroX;
    macroStor[macroIdx].ly = macroY;
  }
}

// Legalizer for macro locations
void MacroCircuit::StubPlacer(double snapGrid) {
  cout << "Macro Stub Placement process... ";

  snapGrid *= 10;

  int sizeX = (int)( (ux_ - lx_) / snapGrid + 0.5f);
  int sizeY = (int)( (uy_ - ly_) / snapGrid + 0.5f);

  int** checker = new int* [sizeX];
  for(int i=0; i<sizeX; i++) {
    checker[i] = new int [sizeY];
    for(int j=0; j<sizeY; j++) {
      checker[i][j] = -1; // uninitialize
    }
  }

  bool isOverlap = true;
  do {

    for(auto& curMacro: macroStor) { 
      // Macro Projection in (llx, lly, urx, ury)
      if( curMacro.lx < lx_ ) curMacro.lx = lx_;
      if( curMacro.ly < ly_ ) curMacro.ly = ly_;
      if( curMacro.lx + curMacro.w > ux_ ) curMacro.lx = ux_ - curMacro.w;
      if( curMacro.ly + curMacro.h > uy_ ) curMacro.ly = uy_ - curMacro.h;

      if( curMacro.lx < lx_ || curMacro.lx + curMacro.w > ux_ ) {
        cout << "ERROR: Macro Legalizer detects width is not enough" << endl;
        exit(1);
      }
      if( curMacro.ly < lx_ || curMacro.ly + curMacro.h > uy_ ) {
        cout << "ERROR: Macro Legalizer detects height is not enough" << endl;
        exit(1);
      }

      // do random placement
      int macroWidthGrid = int( curMacro.w / snapGrid + 0.5f);
      int macroHeightGrid = int( curMacro.h / snapGrid + 0.5f);

      // possible range
      int macroRangeX = sizeX - macroWidthGrid;
      int macroRangeY = sizeY - macroHeightGrid;

      // extract random lx, ly location
      int macroGridLx = rand() % macroRangeX;
      int macroGridLy = rand() % macroRangeY;

      curMacro.lx = int( macroGridLx * snapGrid +0.5f);
      curMacro.ly = int( macroGridLy * snapGrid +0.5f); 

      // follow initial placement
      // curMacro.lx = int( curMacro.lx / snapGrid + 0.5f) * snapGrid;
      // curMacro.ly = int( curMacro.ly / snapGrid + 0.5f) * snapGrid;

      // Do someth to avoid overlap
      // ...
      //    curMacro.Dump();


      isOverlap = false;
      for(int i= macroGridLx ; i <= macroGridLx + macroWidthGrid; i++) {
        for(int j= macroGridLy ; j <= macroGridLy + macroHeightGrid; j++) {
          if( checker[i][j] != -1 ) {
            cout << i << " " << j << " prev: " << checker[i][j] 
              << " cur: " << &curMacro - &macroStor[0] << endl;
      

            isOverlap = true; 
            break;
          }
          // insert a macro placer
          checker[i][j] = &curMacro - &macroStor[0];
        }
        if( isOverlap ) {
          break;
        }
      }
      if( isOverlap ) {
        break;
      }
    }

    if( isOverlap) {
      cout << "overlap!!" << endl;
    }

    for(int i=0; i<sizeX; i++) {
      for(int j=0; j<sizeY; j++) {
        checker[i][j] = -1;
      }
    }
  } while( isOverlap );

  cout << "Done" << endl;
}




void MacroCircuit::ParseGlobalConfig(string fileName) {
  std::ifstream gConfFile (fileName);
  if( !gConfFile.is_open() ) {
    cout << "ERROR: Cannot open file: " << fileName << endl;
    exit(1);
  } 

  string lineStr = "";
  while(getline(gConfFile, lineStr)) {
    char trimChar[256] = {0, };
    TrimWhiteSpace(trimChar, lineStr.length()+1, lineStr.c_str());
    string trimStr (trimChar);

    std::stringstream oStream(trimStr);
    string buf1, varName; 
    double val = 0.0f;

    oStream >> buf1;
    string skipStr = "//";
    // skip for slash
    if( buf1.substr(0, skipStr.size()) == skipStr) {
      continue;
    }
    if( buf1 != "set" ) {
      cout << "ERROR: Cannot parse : " << buf1 << endl;
      exit(1);
    }
    
    oStream >> varName >> val;

#define IS_STRING_EXIST(varname, str) ( (varName).find((str)) != std::string::npos)
    if( IS_STRING_EXIST(varName, "FIN_PITCH") ) {
      // TODO
      // ?
    }
    else if( IS_STRING_EXIST( varName, "ROW_HEIGHT") ) {
      // TODO
      // No Need
    }
    else if( IS_STRING_EXIST( varName, "SITE_WIDTH") ) {
      // TODO
      // No Need
    }
    else if( IS_STRING_EXIST( varName, "HALO_WIDTH_V") ) {
      haloY_ = val;
    }
    else if( IS_STRING_EXIST( varName, "HALO_WIDTH_H") ) {
      haloX_ = val;
    }
    else if( IS_STRING_EXIST( varName, "CHANNEL_WIDTH_V") ) {
      channelY_ = val;
    }
    else if( IS_STRING_EXIST( varName, "CHANNEL_WIDTH_H") ) {
      channelX_ = val;
    }
    else {
      cout << "ERROR: Cannot parse : " << varName << endl;
      exit(1);
    }
  }
  log_->procEnd("Parsing Global Config");
}

void MacroCircuit::ParseLocalConfig(string fileName) {
  std::ifstream gConfFile (fileName);
  if( !gConfFile.is_open() ) {
    cout << "ERROR: Cannot open file: " << fileName << endl;
    exit(1);
  } 

  string lineStr = "";
  while(getline(gConfFile, lineStr)) {
    char trimChar[256] = {0, };
    TrimWhiteSpace(trimChar, lineStr.length()+1, lineStr.c_str());
    string trimStr (trimChar);

    std::stringstream oStream(trimStr);
    string buf1, varName, masterName; 
    double val = 0.0f;

    oStream >> buf1;
    string skipStr = "//";
    // skip for slash
    if( buf1.substr(0, skipStr.size()) == skipStr) {
      continue;
    }

    if( buf1 == "" ) { 
      continue;
    }
    if( buf1 != "set" ) {
      cout << "ERROR: Cannot parse : " << buf1 << endl;
      exit(1);
    }
    
    oStream >> varName >> masterName >> val;

#define IS_STRING_EXIST(varname, str) ( (varName).find((str)) != std::string::npos)
    if( IS_STRING_EXIST( varName, "ROW_HEIGHT") ) {
      // TODO
      // No Need
    }
    else if( IS_STRING_EXIST( varName, "HALO_WIDTH_V") ) {
      macroLocalMap[ masterName ].putHaloY(val);
    }
    else if( IS_STRING_EXIST( varName, "HALO_WIDTH_H") ) {
      macroLocalMap[ masterName ].putHaloX(val);
    }
    else if( IS_STRING_EXIST( varName, "CHANNEL_WIDTH_V") ) {
      macroLocalMap[ masterName ].putChannelY(val);
    }
    else if( IS_STRING_EXIST( varName, "CHANNEL_WIDTH_H") ) {
      macroLocalMap[ masterName ].putChannelX(val);
    }
    else {
      cout << "ERROR: Cannot parse : " << varName << endl;
      exit(1);
    }
  }
  log_->procEnd("Parsing Local Config");
}

void 
MacroCircuit::
Plot(string fileName, vector<MacroPlace::Partition>& set) {

  cout<<"OutPut Plot file is "<<fileName<<endl;
  std::ofstream gpOut(fileName);
  if (!gpOut.good()) {
    cout << "Warning: output file " << fileName
      << " can't be opened" << endl;
  }
  gpOut <<"set terminal png size 1024,768" << endl;

  gpOut<<"#Use this file as a script for gnuplot"<<endl;
  gpOut<<"#(See http://www.gnuplot.info/ for details)"<<endl;
  gpOut << "set nokey"<<endl;
  
  gpOut << "set size ratio -1" << endl;
  gpOut << "set title '' " << endl; 

  gpOut << "set xrange[" << lx_ << ":" << ux_ << "]" << endl;
  gpOut << "set yrange[" << ly_ << ":" << uy_ << "]" << endl;

  int objCnt = 0; 
  for(auto& curMacro : macroStor) {
    // rect box
    gpOut << "set object " << ++objCnt
      << " rect from " << curMacro.lx << "," << curMacro.ly 
      << " to " << curMacro.lx + curMacro.w << "," 
      << curMacro.ly + curMacro.h
      << " fc rgb \"gold\"" << endl;

    // name
    gpOut<<"set label '"<< curMacro.name 
      << "(" << &curMacro - &macroStor[0] << ")"
      << "'noenhanced at "
      << curMacro.lx + curMacro.w/5<<" , "
      << curMacro.ly + curMacro.h/4<<endl;
  }

  // just print boundary for each sets
  for(auto& curSet : set) {
    gpOut << "set object " << ++objCnt
      << " rect from " << curSet.lx << "," << curSet.ly 
      << " to " << curSet.lx + curSet.width << "," 
      << curSet.ly + curSet.height
      << " fc rgb \"#FFFFFF\"" << endl;
  }

  gpOut << "plot '-' w l" << endl;
  gpOut << "EOF" << endl;
  gpOut.close();

}

void MacroCircuit::UpdateNetlist(MacroPlace::Partition& layout) {

  if( netTable_ ) {
    delete[] netTable_;
    netTable_ = 0;
  }
 
  assert( layout.macroStor.size() == macroStor.size() );
  size_t tableSize = (macroStor.size()+4) * (macroStor.size()+4);

  netTable_ = new double[tableSize];
  for(size_t i=0; i<tableSize; i++) {
    netTable_[i] = layout.netTable[i];
  }
}

#define EAST_IDX (macroStor.size())
#define WEST_IDX (macroStor.size()+1)
#define NORTH_IDX (macroStor.size()+2)
#define SOUTH_IDX (macroStor.size()+3)

double MacroCircuit::GetWeightedWL() {
  double wwl = 0.0f;

  double width = ux_ - lx_;
  double height = uy_ - ly_; 

  for(size_t i=0; i<macroStor.size()+4; i++) {
    for(size_t j=0; j<macroStor.size()+4; j++) {
      if( j >= i ) {
        continue;
      }

      double pointX1 = 0, pointY1 = 0;
      if( i == EAST_IDX ) {
        pointX1 = lx_ + width;
        pointY1 = ly_ + height /2.0;
      }
      else if( i == WEST_IDX) {
        pointX1 = lx_;
        pointY1 = ly_ + height /2.0;
      }
      else if( i == NORTH_IDX ) { 
        pointX1 = lx_ + width / 2.0;
        pointY1 = ly_ + height;
      }
      else if( i == SOUTH_IDX ) {
        pointX1 = lx_ + width / 2.0;
        pointY1 = ly_;
      }
      else {
        pointX1 = macroStor[i].lx + macroStor[i].w;
        pointY1 = macroStor[i].ly + macroStor[i].h;
      }

      double pointX2 = 0, pointY2 = 0;
      if( j == EAST_IDX ) {
        pointX2 = lx_ + width;
        pointY2 = ly_ + height /2.0;
      }
      else if( j == WEST_IDX) {
        pointX2 = lx_;
        pointY2 = ly_ + height /2.0;
      }
      else if( j == NORTH_IDX ) { 
        pointX2 = lx_ + width / 2.0;
        pointY2 = ly_ + height;
      }
      else if( j == SOUTH_IDX ) {
        pointX2 = lx_ + width / 2.0;
        pointY2 = ly_;
      }
      else {
        pointX2 = macroStor[j].lx + macroStor[j].w;
        pointY2 = macroStor[j].ly + macroStor[j].h;
      }

      float edgeWeight = 0.0f;
      if( isTiming_ ) {
        edgeWeight = netTable_[ i*(macroStor.size())+j ];
      }
      else {
        edgeWeight = 1;
      }

      wwl += edgeWeight *
        sqrt( (pointX1-pointX2)*(pointX1-pointX2) + 
            (pointY1-pointY2)*(pointY1-pointY2) );
    }
  }

  return wwl;
}

MacroPlace::Vertex* 
MacroCircuit::GetVertex( sta::Pin *pin ) {
  pair<void*, VertexType> vertInfo = GetPtrClassPair( pin);
  auto vertPtr = pinInstVertexMap.find(vertInfo.first);
  if( vertPtr == pinInstVertexMap.end() )  {
    cout << "WARNING: " << sta_->network()->pathName(pin) 
      << " not exists in pinInstVertexMap" << endl;
    return nullptr;
  }
  return &vertexStor[vertPtr->second];
}


Layout::Layout() 
  : lx_(0), ly_(0), ux_(0), uy_(0) {}
    
Layout::Layout( double lx, double ly, 
    double ux, double uy) 
  : lx_(lx), ly_(ly), ux_(ux), uy_(uy) {}
      
Layout::Layout( Layout& orig, MacroPlace::Partition& part ) 
  : lx_(part.lx), ly_(part.ly), ux_(part.lx+part.width), uy_(part.ly+part.height) {}

void
Layout::setLx(double lx) {
  lx_ = lx;
}

void
Layout::setLy(double ly) {
  ly_ = ly;
}

void
Layout::setUx(double ux) {
  ux_ = ux;
}

void
Layout::setUy(double uy) {
  uy_ = uy;
}

///////////////////////////////////////////////////
//  static funcs


static bool
isNotVisited(MacroPlace::Vertex* vert, vector<MacroPlace::Vertex*> &path) {
  for(auto& curVert: path) {
    if( curVert == vert ) {
      return false;
    }
  }
  return true;
}

static bool 
isTerminal(MacroPlace::Vertex* vert, 
    MacroPlace::Vertex* target ) {

  return ( vert != target && 
      (vert->vertexType() == VertexType::PinGroupType || 
      vert->vertexType() == VertexType::MacroInstType) );
}

// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.

// referenced from 
// https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way 

static size_t 
TrimWhiteSpace(char *out, size_t len, const char *str)
{
  if(len == 0)
    return 0;

  const char *end;
  size_t out_size;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
  {
    *out = 0;
    return 1;
  }

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end++;

  // Set output size to minimum of trimmed string length and buffer size minus 1
  out_size = (end - str) < ((int)len-1) ? (end - str) : ((int)len-1);

  // Copy trimmed string and add null terminator
  memcpy(out, str, out_size);
  out[out_size] = 0;

  return out_size;
}

static MacroPlace::PinGroupLocation
getPinGroupLocation(
    int cx, int cy, 
    int dieLx, int dieLy, int dieUx, int dieUy) {
  int lxDx = abs(cx - dieLx);
  int uxDx = abs(cx - dieUx);

  int lyDy = abs(cy - dieLy);
  int uyDy = abs(cy - dieUy);

  int minDiff = std::min(lxDx, std::min(uxDx, std::min(lyDy, uyDy)));
  if( minDiff == lxDx ) {
    return West;
  }
  else if( minDiff == uxDx ) {
    return East;
  }
  else if( minDiff == lyDy ) {
    return South;
  }
  else if( minDiff == uyDy ) {
    return North;
  }
  return West;
}

static std::string
getPinGroupLocationString(PinGroupLocation pg) {
  if( pg == West ) {
    return "West";
  }
  else if( pg == East ) {
    return "East";
  }
  else if( pg == North ) {
    return "North";
  }
  else if( pg == South ) {
    return "South";
  }
  else
    return "??";
}

static bool
isMissingLiberty(sta::Sta* sta, vector<Macro>& macroStor) {
  sta::LeafInstanceIterator *instIter 
    = sta->network()->leafInstanceIterator();
  while (instIter->hasNext()) {
    sta::Instance *inst = instIter->next();
    if ( !sta->network()->libertyCell(inst) ) {
      delete instIter;
      return true;
    }
  }
  delete instIter;
  
  for(auto& macro: macroStor) {
    sta::Instance* staInst = macro.staInstPtr;
    sta::LibertyCell* libCell 
      = sta->network()->libertyCell(staInst);
    if( !libCell ) {
      return true;
    }
  }
  return false;
}

} // namespace MacroPlace
