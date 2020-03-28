#include <iostream>
#include <queue>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set> 

#include "circuit.h"

#include "Machine.hh"
#include "Graph.hh"
#include "Sta.hh"
#include "Network.hh"
#include "Liberty.hh"
#include "Sdc.hh"
#include "PortDirection.hh"
#include "Corner.hh"
#include "PathExpanded.hh"
#include "PathEnd.hh"
#include "PathRef.hh"
#include "Search.hh"
#include "db_sta/dbSta.hh"

namespace MacroPlace {

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::make_pair;
using std::string;

using Eigen::VectorXf;
typedef Eigen::SparseMatrix<int, Eigen::RowMajor> SMatrix;
typedef Eigen::Triplet<int> T;

using namespace odb;


static bool
isNotVisited(MacroPlace::Vertex* vert, 
    vector<MacroPlace::Vertex*> &path);

static bool 
isTerminal(MacroPlace::Vertex* vert, 
    MacroPlace::Vertex* target);


MacroCircuit::MacroCircuit() :
  gHaloX(0), gHaloY(0), 
  gChannelX(0), gChannelY(0), 
  db_(0), sta_(0), 
  lx(0), ly(0), ux(0), uy(0),
  netTable(0) {}

MacroCircuit::MacroCircuit(
    odb::dbDatabase* db,
    sta::dbSta* sta,
    Layout* cinfo) :
  gHaloX(0), gHaloY(0), 
  gChannelX(0), gChannelY(0), 
  db_(db), 
  sta_(sta),
  lx(0), ly(0), ux(0), uy(0),
  netTable(0) {

  Init(db, sta, cinfo);
}

void
MacroCircuit::reset() {
  // TODO
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

void MacroCircuit::Init( 
    odb::dbDatabase* db, 
    sta::dbSta* sta,
    Layout* cinfo) {
  
  db_ = db; 
  sta_ = sta;

  lx = cinfo->lx();
  ly = cinfo->ly();
  ux = cinfo->ux();
  uy = cinfo->uy();

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
  FillVertexEdge();
  UpdateVertexToMacroStor();
  
//  CheckGraphInfo();
  FillMacroPinAdjMatrix();
  FillMacroConnection();
}



void MacroCircuit::FillMacroStor() {
  cout << "Extracting Macro Cells... ";

  dbTech* tech = db_->getTech();

  dbChip* chip = db_->getChip();
  dbBlock* block = chip->getBlock();
  
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

    // for Macro cells
    dbPlacementStatus dps = inst->getPlacementStatus();
    if( dps == dbPlacementStatus::NONE ||
        dps == dbPlacementStatus::UNPLACED ) {
      cout << "ERROR:  Macro: " << inst->getConstName() << " is Unplaced." << endl;
      cout << "        Please use TD-MS-RePlAce to get a initial solution " << endl;
      cout << "        before executing TritonMacroPlace" << endl;
      exit(1); 
    }
    
    double curHaloX =0, curHaloY = 0, curChannelX = 0, curChannelY = 0;
    auto mlPtr = macroLocalMap.find( inst->getConstName() );
    if( mlPtr == macroLocalMap.end() ) {
      curHaloX = gHaloX;
      curHaloY = gHaloY; 
      curChannelX = gChannelX;
      curChannelY = gChannelY;
    }
    else {
      MacroLocalInfo& m = mlPtr->second;
      curHaloX = (m.GetHaloX() == 0)? gHaloX : m.GetHaloX();
      curHaloY = (m.GetHaloY() == 0)? gHaloY : m.GetHaloY();
      curChannelX = (m.GetChannelX() == 0)? gChannelX : m.GetChannelX();
      curChannelY = (m.GetChannelY() == 0)? gChannelY : m.GetChannelY();
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
    cout << "ERROR: Cannot find any macros in this design. " << endl;
    exit(1);
  }
  cout << "Done!" << endl;
  cout << "Extracted # Macros: " << macroStor.size() << endl;
}

static bool isWithIn( int val, int min, int max ) {
  return (( min <= val ) && ( val <= max ));
}


void MacroCircuit::FillPinGroup(){
  dbTech* tech = db_->getTech(); 
  const double dbu = tech->getDbUnitsPerMicron();

  int numEdge = sta_->graph()->edgeCount();
  int numVertex = sta_->graph()->vertexCount();

  cout << "OpenSTA numEdge: " << numEdge << endl;
  cout << "OpenSTA numVertex: " << numVertex << endl;

  int dbuLx = int(lx * dbu +0.5f);
  int dbuLy = int(ly * dbu +0.5f);
  int dbuUx = int(ux * dbu +0.5f);
  int dbuUy = int(uy * dbu +0.5f);

  using MacroPlace::PinGroupLocation;

  unordered_map<string, PinGroupLocation> pinGroupStrMap;

  dbChip* chip = db_->getChip();
  dbBlock* block = chip->getBlock();

  for(dbBTerm* bTerm : block->getBTerms()) {
    
    // pin signal type
    dbSigType psType = bTerm->getSigType();
    if( psType == dbSigType::GROUND ||
        psType == dbSigType::POWER ) {
      continue;
    }

    // pin placement status
    dbPlacementStatus ppStatus = bTerm->getFirstPinPlacementStatus();
    if( ppStatus == dbPlacementStatus::UNPLACED ||
        ppStatus == dbPlacementStatus::NONE ) {
      cout << "**ERROR: PIN " << bTerm->getConstName() << " is not PLACED" << endl;
      exit(1);
    } 
   
    int placeX, placeY; 
    bTerm->getFirstPinLocation(placeX, placeY);
    
    bool isFound = false;
    for(dbBPin* bPin : bTerm->getBPins()) {
      int boxLx = bPin->getBox()->xMin();
      int boxLy = bPin->getBox()->yMin(); 
      int boxUx = bPin->getBox()->xMax();
      int boxUy = bPin->getBox()->yMax();
    
      if( isWithIn( dbuLx, boxLx, boxUx ) ) {
        pinGroupStrMap[ bTerm->getConstName() ] = PinGroupLocation::West; 
        isFound = true;
        break;
      }
      else if( isWithIn( dbuUx, boxLx, boxUx ) ) {
        pinGroupStrMap[ bTerm->getConstName() ] = PinGroupLocation::East; 
        isFound = true;
        break;
      }
      else if( isWithIn( dbuLy, boxLy, boxUy ) ) {
        pinGroupStrMap[ bTerm->getConstName() ] = PinGroupLocation::South; 
        isFound = true;
        break;
      }
      else if( isWithIn( dbuUy, boxLy, boxUy ) ) {
        pinGroupStrMap[ bTerm->getConstName() ] = PinGroupLocation::North; 
        isFound = true;
        break;
      }
    } 
    if( !isFound ) {
      cout << "**ERROR: PIN " << bTerm->getConstName() 
        << " is not PLACED in Border!!" << endl;
      cout << "INFO: Place Information: " << placeX << " " << placeY << endl; 
      cout << "INFO: Border Information: " << dbuLx << " " << dbuLy 
           << " " << dbuUx << " " << dbuUy << endl;
      exit(1);
    } 
  }

  // this is always four array.
  pinGroupStor.resize(4);

  // save PG-Class info in below
  pinGroupStor[(int)PinGroupLocation::East].setPinGroupLocation(East);
  pinGroupStor[(int)PinGroupLocation::West].setPinGroupLocation(West);
  pinGroupStor[(int)PinGroupLocation::North].setPinGroupLocation(North);
  pinGroupStor[(int)PinGroupLocation::South].setPinGroupLocation(South);

  for(int i=1; i<=numVertex; i++) {
    sta::Vertex* vert = sta_->graph()->vertex(i);
    sta::Pin* pin = vert->pin();
    if( !sta_->network()->isTopLevelPort(pin) ) {
      continue;
    }
    
    auto pgPtr = pinGroupStrMap.find(sta_->network()->pathName(pin));
    if( pgPtr == pinGroupStrMap.end() ) {
      cout << "**ERROR: PIN " << sta_->network()->pathName(pin) 
        << " not exist in pinGroupStrMap" << endl;
      exit(1);
    }

    int idx = (int)pgPtr->second;
    pinGroupStor[idx].addPin(pin);
    pinGroupMap.insert( make_pair(pin, idx) );
  }

  cout << endl << "Pin Group Classification Info: " << endl;
  cout << "East: " << pinGroupStor[(int)PinGroupLocation::East].pins().size() << endl;
  cout << "West: " << pinGroupStor[(int)PinGroupLocation::West].pins().size() << endl;
  cout << "North: " << pinGroupStor[(int)PinGroupLocation::North].pins().size() << endl;
  cout << "South: " << pinGroupStor[(int)PinGroupLocation::South].pins().size() << endl;
  
}

void MacroCircuit::FillVertexEdge() {
  cout << "Generating Sequantial Graph..." << endl; 
  int numVertex = sta_->graph()->vertexCount();

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
  for(int i=1; i<=numVertex; i++) {
    sta::Vertex* vert = sta_->graph()->vertex(i); 
    sta::Pin* pin = vert->pin();
   
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
  for(int i=1; i<=numVertex; i++) {
    sta::Vertex* vert = sta_->graph()->vertex(i);
    sta::Pin* pin = vert->pin();
    
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
      sta::PinSet *fanout = sta_->findFanoutPins(&pinStor, false, true ,
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

        /*
         * previous
        auto mapPtr = vertexPairEdgeMap.find( make_pair(curVertex , adjVertex) );
        if( mapPtr == vertexPairEdgeMap.end() ) {
          vertexPairEdgeMap[ make_pair(curVertex, adjVertex) ] = edgeStor.size();
          edgeStor.push_back( MacroPlace::Edge(curVertex, adjVertex, 1) );


          // Vertex Update
          curVertex->to.push_back(edgeStor.size()-1);
          adjVertex->from.push_back(edgeStor.size()-1);
        }
        else {
          // increase the edge weight
          edgeStor[mapPtr->second].weight ++;
        }*/
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

//        cout << sta_->network()->pathName(adjPin) << " -> " << sta_->network()->pathName(pin) << endl;

        /*
         * previous
        auto mapPtr = vertexPairEdgeMap.find( make_pair(adjVertex, curVertex) );
        if( mapPtr == vertexPairEdgeMap.end() ) {
          vertexPairEdgeMap[ make_pair(adjVertex, curVertex) ] = edgeStor.size();
          edgeStor.push_back( MacroPlace::Edge(adjVertex, curVertex, 1) );


          // Vertex Update
          curVertex->from.push_back(edgeStor.size()-1);
          adjVertex->to.push_back(edgeStor.size()-1);
        }
        else {
          // increase the edge weight
          edgeStor[mapPtr->second].weight ++;
        }*/

        tripletList.push_back(T(vertexPtrMap[curVertex], vertexPtrMap[adjVertex], 1));
      }
      delete fanin;
    }
  }
  // Query find_timing_paths 
  for(int i=1; i<=numVertex; i++) {
    sta::Vertex* vert = sta_->graph()->vertex(i);
    sta::Pin* pin = vert->pin();
    
    bool isTopPin = sta_->network()->isTopLevelPort(pin);
    // only query for IO/Pins
    if( !isTopPin ) {
      continue;
    }

    /*
    // Skip For Non-FF Cells
    if( !isTopPin ) {
      Instance* inst = sta_->network()->instance(pin);
      LibertyCell* libCell = sta_->network()->libertyCell(inst);
      if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
        continue;
      }
    }
      
    // skip for output toplevelport;
    PortDirection* dir = sta_->network()->direction(pin);
    if( dir->isAnyOutput() ) {
      if( sta_->network()->isTopLevelPort(pin)) {
        continue;
      }
    }
    // skip for input Non-FF/Macro pins
    else {
      if( !sta_->network()->isTopLevelPort(pin)) {
        continue;
      }
    }
    */

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
//      cout << "ERROR: NO PATH !" << endl;
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

//      cout << startVertPtr << " -> " << endVertPtr << endl;

      /*
       * Previous
      // Edge Update
      auto mapPtr = vertexPairEdgeMap.find( make_pair(startVertPtr, endVertPtr) );
      if( mapPtr == vertexPairEdgeMap.end() ) {
        vertexPairEdgeMap[ make_pair(startVertPtr, endVertPtr) ] = edgeStor.size();
        edgeStor.push_back( MacroPlace::Edge(startVertPtr, endVertPtr, 1) );

        // Vertex Update
        startVertPtr->to.push_back(edgeStor.size()-1);
        endVertPtr->from.push_back(edgeStor.size()-1);
      }
      else {
        // increase the edge weight
        edgeStor[mapPtr->second].weight ++;
      }*/
        
      tripletList.push_back(T(vertexPtrMap[startVertPtr], vertexPtrMap[endVertPtr], 1));
    }
  }

  adjMatrix.setFromTriplets( tripletList.begin(), tripletList.end() );
  
  cout << "Sequential Graph Building is Done!" << endl;
  cout << "Vertex: " << vertexStor.size() << endl;
  cout << "Edge: " << adjMatrix.nonZeros() << endl;
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
    string name = sta_->network()->pathName((sta::Instance*)curVertex.ptr());
    auto mPtr = macroNameMap.find( name );
    if( mPtr == macroNameMap.end() ) {
      cout << "**ERROR: The Macro Name must be in macro NameMap: " << name << endl;
      exit(1);
    } 

    macroStor[mPtr->second].ptr = &curVertex;
  }
}

// macroStr & macroInstMap update
void MacroCircuit::UpdateInstanceToMacroStor() {
  sta::VertexId numVertex = sta_->graph()->vertexCount();
  for(size_t i=1; i<=numVertex; i++) {
    sta::Vertex* vert = sta_->graph()->vertex(i);
    sta::Pin* pin = vert->pin();
    if( sta_->network()->isTopLevelPort(pin) ) {
      continue;
    }
    sta::Instance* inst = sta_->network()->instance(pin);
    string instName = sta_->network()->pathName(inst);

    auto mnPtr = macroNameMap.find(instName); 
    if( mnPtr == macroNameMap.end()) {
      continue; 
    }
    
    // macro & macroInstMap update
    macroStor[mnPtr->second].staInstPtr = inst;
    macroInstMap[inst] = mnPtr->second; 
  }
}

pair<void*, VertexType> 
MacroCircuit::GetPtrClassPair( sta::Pin* pin ) {

  pair<void*, VertexType> ret;
  bool isTopPin = sta_->network()->isTopLevelPort(pin);

  // toplevel pin
  if( isTopPin ) {
    auto pgPtr = pinGroupMap.find( pin );
    if( pgPtr == pinGroupMap.end()) {
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
//  cout << "Macro Legalizing process... ";
  cout << "Macro Stub Placement process... ";

  snapGrid *= 10;

  int sizeX = (int)( (ux - lx) / snapGrid + 0.5f);
  int sizeY = (int)( (uy - ly) / snapGrid + 0.5f);

  int** checker = new int* [sizeX];
  for(int i=0; i<sizeX; i++) {
    checker[i] = new int [sizeY];
    for(int j=0; j<sizeY; j++) {
      checker[i][j] = -1; // uninitialize
    }
  }
  cout << "cinfo: " << ux << " " << uy << endl;
  cout << "GRID Width: " << sizeX << " Height: " << sizeY << endl;


  bool isOverlap = true;
  do {

    for(auto& curMacro: macroStor) { 
      // Macro Projection in (llx, lly, urx, ury)
      if( curMacro.lx < lx ) curMacro.lx = lx;
      if( curMacro.ly < ly ) curMacro.ly = ly;
      if( curMacro.lx + curMacro.w > ux ) curMacro.lx = ux - curMacro.w;
      if( curMacro.ly + curMacro.h > uy ) curMacro.ly = uy - curMacro.h;

      if( curMacro.lx < lx || curMacro.lx + curMacro.w > ux ) {
        cout << "ERROR: Macro Legalizer detects width is not enough" << endl;
        exit(1);
      }
      if( curMacro.ly < lx || curMacro.ly + curMacro.h > uy ) {
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


// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.

// referenced from 
// https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way 

size_t TrimWhiteSpace(char *out, size_t len, const char *str)
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


void MacroCircuit::ParseGlobalConfig(string fileName) {
//  cout << "Parsing globalConfig: " << fileName << " ... " ;
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
      gHaloY = val;
    }
    else if( IS_STRING_EXIST( varName, "HALO_WIDTH_H") ) {
      gHaloX = val;
    }
    else if( IS_STRING_EXIST( varName, "CHANNEL_WIDTH_V") ) {
      gChannelY = val;
    }
    else if( IS_STRING_EXIST( varName, "CHANNEL_WIDTH_H") ) {
      gChannelX = val;
    }
    else {
      cout << "ERROR: Cannot parse : " << varName << endl;
      exit(1);
    }
  }
//  cout << "Done!" << endl;
}

void MacroCircuit::ParseLocalConfig(string fileName) {
  cout << "Parsing localConfig: " << fileName << " ... " ;
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
  cout << "Done!" << endl;
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

  gpOut << "set xrange[" << lx << ":" << ux << "]" << endl;
  gpOut << "set yrange[" << ly << ":" << uy << "]" << endl;

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

  if( netTable ) {
    delete[] netTable;
    netTable = 0;
  }
 
  assert( layout.macroStor.size() == macroStor.size() );
  size_t tableSize = (macroStor.size()+4) * (macroStor.size()+4);

  netTable = new double[tableSize];
  for(size_t i=0; i<tableSize; i++) {
    netTable[i] = layout.netTable[i];
  }
}

#define EAST_IDX (macroStor.size())
#define WEST_IDX (macroStor.size()+1)
#define NORTH_IDX (macroStor.size()+2)
#define SOUTH_IDX (macroStor.size()+3)

double MacroCircuit::GetWeightedWL() {
  double wwl = 0.0f;

  double width = ux - lx;
  double height = uy - ly; 

  for(size_t i=0; i<macroStor.size()+4; i++) {
    for(size_t j=0; j<macroStor.size()+4; j++) {
      if( j >= i ) {
        continue;
      }

      double pointX1 = 0, pointY1 = 0;
      if( i == EAST_IDX ) {
        pointX1 = lx;
        pointY1 = ly + height /2.0;
      }
      else if( i == WEST_IDX) {
        pointX1 = lx + width;
        pointY1 = ly + height /2.0;
      }
      else if( i == NORTH_IDX ) { 
        pointX1 = lx + width / 2.0;
        pointY1 = ly + height;
      }
      else if( i == SOUTH_IDX ) {
        pointX1 = lx + width / 2.0;
        pointY1 = ly;
      }
      else {
        pointX1 = macroStor[i].lx + macroStor[i].w;
        pointY1 = macroStor[i].ly + macroStor[i].h;
      }

      double pointX2 = 0, pointY2 = 0;
      if( j == EAST_IDX ) {
        pointX2 = lx;
        pointY2 = ly + height /2.0;
      }
      else if( j == WEST_IDX) {
        pointX2 = lx + width;
        pointY2 = ly + height /2.0;
      }
      else if( j == NORTH_IDX ) { 
        pointX2 = lx + width / 2.0;
        pointY2 = ly + height;
      }
      else if( j == SOUTH_IDX ) {
        pointX2 = lx + width / 2.0;
        pointY2 = ly;
      }
      else {
        pointX2 = macroStor[j].lx + macroStor[j].w;
        pointY2 = macroStor[j].ly + macroStor[j].h;
      }
     
      wwl += netTable[ i*(macroStor.size())+j ] *
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

}

