#include <iostream>
#include <queue>
#include <chrono>
#include <fstream>
#include <sstream>

#include "lefdefIO.h"
#include "parse.h"
#include "circuit.h"
#include "timingSta.h"



MacroCircuit::MacroCircuit(
    Circuit::Circuit& ckt, 
    EnvFile& env, 
    CircuitInfo& cinfo) :
  _ckt(ckt), _env(env), _cinfo(cinfo),
  gHaloX(0), gHaloY(0), 
  gChannelX(0), gChannelY(0) {

  // parsing from cfg file
  // global config
  ParseGlobalConfig(_env.globalConfig);
  // local config (optional)
  if( _env.localConfig != "" ) {
    ParseLocalConfig(_env.localConfig);
  }

  FillMacroStor(); 
  FillPinGroup(); 
  UpdateInstanceToMacroStor();
  FillVertexEdge();
  UpdateVertexToMacroStor();
  FillMacroConnection();
}

//MACRO_NETLIST_NAMESPACE_OPEN

using namespace sta;
using MacroNetlist::VertexClass;
using MacroNetlist::PinGroup;
using MacroNetlist::PinGroupClass;
using std::cout;
using std::endl;
using std::make_pair;

void MacroCircuit::FillMacroStor() {
  cout << "Extracting Macro Cells... ";

  double cellHeight = FLT_MIN;
  for(auto& curSite : _ckt.lefSiteStor) {
    if( strcmp( curSite.siteClass(), "CORE" ) == 0 ) {
      cellHeight = curSite.sizeY();
      break;
    }
  }
  double defScale = _ckt.defUnit;
//  cout << "Normal cellHeight: " << cellHeight << endl;
//  cout << "DEF scale down: " << defScale << endl;

  defiPoints points = _ckt.defDieArea.getPoint();
//  cout << points.numPoints << endl;

//  for(int i=0; i<points.numPoints; i++ ) {
//    cout << 1.0*points.x[i]/defScale << " " 
//      << 1.0*points.y[i]/defScale << endl;
//  }
  // cout << _ckt.defDieArea.xl() << " " << _ckt.defDieArea.yl() << " "
  //  << _ckt.defDieArea.xh() << " " << _ckt.defDieArea.yh() << endl;


  for(auto& curComp : _ckt.defComponentStor) {
    auto macroPtr = _ckt.lefMacroMap.find( string(curComp.name()) );
    if( macroPtr == _ckt.lefMacroMap.end() ) {
      cout << "\n** ERROR : Cannot find MACRO cell in lef files: " 
        << curComp.name() << endl;
      exit(1);
    }

    lefiMacro& curMacro = _ckt.lefMacroStor[ macroPtr->second ];

    if( !curMacro.hasSize() ) {
      cout << "\n** ERROR : Cannot find MACRO SIZE in lef files: " 
        << curComp.name() << endl;
      exit(1);
    }

    // std cell skip
    if( abs( curMacro.sizeY() - cellHeight) 
        <= std::numeric_limits<double>::epsilon() ) {
      continue;
    }

    double curHaloX =0, curHaloY = 0, curChannelX = 0, curChannelY = 0;
    auto mlPtr = macroLocalMap.find( curComp.name() );
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

    // macro cell update
    string macroName = curComp.id();
//    ReplaceStringInPlace( macroName, "\\[",  "[" );
//    ReplaceStringInPlace( macroName, "\\]",  "]" );
//    ReplaceStringInPlace( macroName, "\\/",  "/" );

    macroNameMap[ macroName ] = macroStor.size();


    int macroOrient = 
      (_env.isWestFix)? 1 : curComp.placementOrient();

    double realSizeX = 0, realSizeY = 0;
    switch( macroOrient ) {
      case 0:
      case 2:
      case 4:
      case 6:
        realSizeX = curMacro.sizeX();
        realSizeY = curMacro.sizeY(); 
        break;
      case 1:
      case 3:
      case 5:
      case 7:
        realSizeX = curMacro.sizeY();
        realSizeY = curMacro.sizeX();
        break;
    }
    
    MacroNetlist::Macro 
      tmpMacro( macroName, curComp.name(), 
          1.0*curComp.placementX()/defScale, 
          1.0*curComp.placementY()/defScale,
          realSizeX, realSizeY, 
          curHaloX, curHaloY, 
          curChannelX, curChannelY,  
          NULL, NULL );

    macroStor.push_back( tmpMacro ); 

  }
  cout << "Done!" << endl;
}

void MacroCircuit::FillPinGroup(){
  cout << "OpenSTA Object Starting... ";
  _sta = GetStaObject( _env );
  cout << "Done!" << endl;
 
  sta::EdgeIndex numEdge = _sta->graph()->edgeCount();
  sta::VertexIndex numVertex = _sta->graph()->vertexCount();

  cout << "OpenSTA numEdge: " << numEdge << endl;
  cout << "OpenSTA numVertex: " << numVertex << endl;

  int lx = int(_cinfo.lx*_ckt.defUnit+0.5f);
  int ly = int(_cinfo.ly*_ckt.defUnit+0.5f);
  int ux = int(_cinfo.ux*_ckt.defUnit+0.5f);
  int uy = int(_cinfo.uy*_ckt.defUnit+0.5f);

  using MacroNetlist::PinGroupClass;

  unordered_map<string, PinGroupClass> pinGroupStrMap;
  for(auto& curPin: _ckt.defPinStor) {
    if( curPin.hasUse() && 
        (strcmp(curPin.use(), "POWER") == 0 || strcmp(curPin.use(), "GROUND") == 0) ) {
      continue;
    }
    if( !curPin.hasPlacement() ) {
      cout << "**ERROR: PIN " << curPin.pinName() << " is not PLACED" << endl;
      exit(1);
    }

//    cout << curPin.pinName() << endl;
//    cout << curPin.placementX() << " " << curPin.placementY() << endl;
    if( curPin.placementX() == lx ) {
      pinGroupStrMap[ curPin.pinName() ] = PinGroupClass::West; 
    }
    else if( curPin.placementX() == ux ) {
      pinGroupStrMap[ curPin.pinName() ] = PinGroupClass::East; 
    }
    else if( curPin.placementY() == ly) {
      pinGroupStrMap[ curPin.pinName() ] = PinGroupClass::South; 
    }
    else if( curPin.placementY() == uy) {
      pinGroupStrMap[ curPin.pinName() ] = PinGroupClass::North; 
    }
    else {
      cout << "**ERROR: PIN " << curPin.pinName() << " is not PLACED in Border!!" << endl;
      cout << "INFO: Border Information: " << lx << " " << ly << " " << ux << " " << uy << endl;
      exit(1);
    }
  }

  // this is always four array.
  pinGroupStor.resize(4);

  // save PG-Class info in below
  pinGroupStor[(int)PinGroupClass::East].pinGroupClass = PinGroupClass::East;
  pinGroupStor[(int)PinGroupClass::West].pinGroupClass = PinGroupClass::West;
  pinGroupStor[(int)PinGroupClass::North].pinGroupClass = PinGroupClass::North;
  pinGroupStor[(int)PinGroupClass::South].pinGroupClass = PinGroupClass::South;


  for(int i=1; i<=numVertex; i++) {
    sta::Vertex* vert = _sta->graph()->vertex(i);
    sta::Pin* pin = vert->pin();
    if( !_sta->network()->isTopLevelPort(pin) ) {
      continue;
    }
    
    auto pgPtr = pinGroupStrMap.find(_sta->network()->name(pin));
    if( pgPtr == pinGroupStrMap.end() ) {
      cout << "**ERROR: PIN " << _sta->network()->name(pin) << " not exist in pinGroupStrMap" << endl;
      exit(1);
    }

    int idx = (int)pgPtr->second;
    pinGroupStor[idx].pins.push_back(pin);
    pinGroupMap.insert( make_pair(pin, idx) );
  }

  cout << endl << "Pin Group Classification Info: " << endl;
  cout << "East: " << pinGroupStor[(int)PinGroupClass::East].pins.size() << endl;
  cout << "West: " << pinGroupStor[(int)PinGroupClass::West].pins.size() << endl;
  cout << "North: " << pinGroupStor[(int)PinGroupClass::North].pins.size() << endl;
  cout << "South: " << pinGroupStor[(int)PinGroupClass::South].pins.size() << endl;
  
}

pair<void*, VertexClass> MacroCircuit::GetPtrClassPair( sta::Pin* pin ) {
  pair<void*, VertexClass> ret;
  bool isTopPin = _sta->network()->isTopLevelPort(pin);

  // toplevel pin
  if( isTopPin ) {
    auto pgPtr = pinGroupMap.find( pin);
    if( pgPtr == pinGroupMap.end()) {
      cout << "ERROR: " << _sta->network()->name(pin) << " not exists in PinGroupMap" << endl;
      exit(1);
    }

    // pinGroupPointer
    ret.first = (void*) &pinGroupStor[pgPtr->second];
    ret.second = VertexClass::pin; 
  }
  else {
    sta::Instance* inst = _sta->network()->instance(pin);
    string instName = _sta->network()->name(inst);
    
//    ReplaceStringInPlace( instName, "\\[",  "[" );
//    ReplaceStringInPlace( instName, "\\]",  "]" );
//    ReplaceStringInPlace( instName, "\\/",  "/" );

    ret.first = (void*) inst;
    ret.second = (macroNameMap.find( instName ) != macroNameMap.end())? 
      VertexClass::instMacro : VertexClass::instOther;
  }
  return ret;
}

MacroNetlist::Vertex* MacroCircuit::GetVertex( sta::Pin *pin) {
  pair<void*, VertexClass> vertInfo = GetPtrClassPair( pin);
  auto vertPtr = pinInstVertexMap.find(vertInfo.first);
  if( vertPtr == pinInstVertexMap.end() )  {
    cout << "WARNING: " << _sta->network()->name(pin) << " not exists in pinInstVertexMap" << endl;
    return NULL;
  }
//  cout << "GetVertex: " << &vertexStor[vertPtr->second] << endl;;
  return &vertexStor[vertPtr->second];
}

void MacroCircuit::FillVertexEdge() {

  cout << "Generating Sequantial Graph..." << endl; 
  sta::EdgeIndex numEdge = _sta->graph()->edgeCount();
  sta::VertexIndex numVertex = _sta->graph()->vertexCount();

  Eigen::setNbThreads(8);

  unordered_set<Instance*> instMap;
  unordered_set<void*> vertexDupChk;

  // Fill Vertex for Four IO cases.
  for(int i=0; i<4; i++) {
    pinInstVertexMap[ (void*) &pinGroupStor[i] ] = vertexStor.size();
    vertexStor.push_back( MacroNetlist::Vertex((void*)&pinGroupStor[i], VertexClass::pin ));
  }

  auto startTime = std::chrono::system_clock::now();

  // Fill Vertex for FF/Macro cells 
  for(int i=1; i<=numVertex; i++) {
    Vertex* vert = _sta->graph()->vertex(i); 
    Pin* pin = vert->pin();
    
//    cout << "vert: " << _sta->graph()->name(vert) << endl;
//    cout << "pin: " << _sta->network()->name(pin) << endl ;

    bool isTopPin = _sta->network()->isTopLevelPort(pin);
    if( isTopPin ) {
      continue;
    }
//    cout << "passed level1 "<< endl;

    // skip for below two cases; non-FF cells
    Instance* inst = _sta->network()->instance(pin);
    LibertyCell* libCell = _sta->network()->libertyCell(inst);
    if( !libCell -> hasSequentials() 
        && macroInstMap.find(inst) == macroInstMap.end() ) {
      continue;
    }  
    
//    cout << "passed level2 "<< endl << endl;
    
    // skip for below two cases; non visited
    if( instMap.find(inst ) != instMap.end()) {
      continue;
    }
    instMap.insert( inst );

//    cout << "passed level3 "<< endl;

    pair<void*, VertexClass> vertex = GetPtrClassPair( pin );
    auto vertPtr = pinInstVertexMap.find( vertex.first );
    if( vertPtr == pinInstVertexMap.end()) {
      pinInstVertexMap[ vertex.first ] = vertexStor.size();
      vertexStor.push_back( MacroNetlist::Vertex(vertex.first, vertex.second)); 
    }

//    cout << "vert: " << _sta->network()->name(inst) << endl;
  }
  
  // VertexPtrMap Initialize
  for(auto& curVertex: vertexStor) {
    vertexPtrMap[&curVertex] = &curVertex - &vertexStor[0];
  }
  
  adjMatrix.resize( vertexStor.size(), vertexStor.size() ); 
  vector< T > tripletList;

  auto endTime = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
  cout << "Vertex Building: " << elapsed.count()<< "s" << endl;

  
  startTime = std::chrono::system_clock::now();

  // Query Get_FanIn/ Get_FanOut
  for(int i=1; i<=numVertex; i++) {
    Vertex* vert = _sta->graph()->vertex(i);
    Pin* pin = vert->pin();
    
    bool isTopPin = _sta->network()->isTopLevelPort(pin);
    // !!!!!!!!!!!!!!!!
    // Future support of OpenSTA
    if( isTopPin ) {
      continue;
    }

    // Skip For Non-FF Cells
    if( !isTopPin ) {
      Instance* inst = _sta->network()->instance(pin);
      LibertyCell* libCell = _sta->network()->libertyCell(inst);
      if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
//        cout << "is not: " << _sta->network()->name(pin) << endl; 
        continue;
      }
    }

    //skip for clock pin
    if( _sta->network()->isCheckClk(pin) || _sta->sdc()->isClock(pin) ) {
      continue;
    }

    MacroNetlist::Vertex* mVert = GetVertex(pin);

//TmpInstanceSet *
//find_fanin_insts(PinSeq *to,
//     bool flat,
//     bool startpoints_only,
//     int inst_levels,
//     int pin_levels,
//     bool thru_disabled,
//     bool thru_constants)
//{

//    cout << "[" << _sta->network()->name(pin) << "]" << endl;
    
    PinSeq pinStor;
    pinStor.push_back(pin);
  
    PortDirection* dir = _sta->network()->direction(pin);
    MacroNetlist::Vertex* curVertex = GetVertex(pin);

    // Query for get_fanin/get_fanout
    if( dir->isAnyOutput() ) {
//      cout << "FanOut!" << endl;
      PinSet *fanout = _sta->findFanoutPins(&pinStor, false, true ,
          500, 700,
          false, false);
      for(auto& adjPin: *fanout) {
//        cout << _sta->network()->name(pin) << " -> " << _sta->network()->name(adjPin) << endl;
        // Skip For Non-FF Pin 
        if( !_sta->network()->isTopLevelPort(adjPin) ) {
          Instance* inst = _sta->network()->instance(adjPin);
          LibertyCell* libCell = _sta->network()->libertyCell(inst);
          if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
            continue;
          }
        }
        
        MacroNetlist::Vertex* adjVertex = GetVertex(adjPin);
        if( adjVertex == curVertex ) {
          continue;
        }

        //skip for clock pin
        if( _sta->network()->isCheckClk(adjPin) || _sta->sdc()->isClock(adjPin) ) {
          continue;
        }

//        cout << _sta->network()->name(pin) << " -> " << _sta->network()->name(adjPin) << endl;

        /*
         * previous
        auto mapPtr = vertexPairEdgeMap.find( make_pair(curVertex , adjVertex) );
        if( mapPtr == vertexPairEdgeMap.end() ) {
          vertexPairEdgeMap[ make_pair(curVertex, adjVertex) ] = edgeStor.size();
          edgeStor.push_back( MacroNetlist::Edge(curVertex, adjVertex, 1) );

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
    }
    else {
//      cout << "FanIn!" << endl;
      PinSet *fanin = _sta->findFaninPins(&pinStor, false, true ,
          500, 700,
          false, false);
      for(auto& adjPin: *fanin) {
//        cout <<  _sta->network()->name(adjPin) << " -> " << _sta->network()->name(pin) << endl;
        // Skip For Non-FF Pin 
        if( !_sta->network()->isTopLevelPort(adjPin) ) {
          Instance* inst = _sta->network()->instance(adjPin);
          LibertyCell* libCell = _sta->network()->libertyCell(inst);
          if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
            continue;
          }
        }
        
        MacroNetlist::Vertex* adjVertex = GetVertex(adjPin);
        if( adjVertex == curVertex ) {
          continue;
        }
        
        //skip for clock pin
        if( _sta->network()->isCheckClk(adjPin) || _sta->sdc()->isClock(adjPin) ) {
          continue;
        }

//        cout << _sta->network()->name(adjPin) << " -> " << _sta->network()->name(pin) << endl;

        /*
         * previous
        auto mapPtr = vertexPairEdgeMap.find( make_pair(adjVertex, curVertex) );
        if( mapPtr == vertexPairEdgeMap.end() ) {
          vertexPairEdgeMap[ make_pair(adjVertex, curVertex) ] = edgeStor.size();
          edgeStor.push_back( MacroNetlist::Edge(adjVertex, curVertex, 1) );

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
    }
  }
  endTime = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
  cout << "Edge Building for Non-FF: " << elapsed.count() << "s" << endl;



  startTime = std::chrono::system_clock::now();

  // Query find_timing_paths 
  for(int i=1; i<=numVertex; i++) {
    Vertex* vert = _sta->graph()->vertex(i);
    Pin* pin = vert->pin();
    
    bool isTopPin = _sta->network()->isTopLevelPort(pin);
    // only query for IO/Pins
    if( !isTopPin ) {
      continue;
    }

    /*
    // Skip For Non-FF Cells
    if( !isTopPin ) {
      Instance* inst = _sta->network()->instance(pin);
      LibertyCell* libCell = _sta->network()->libertyCell(inst);
      if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
        continue;
      }
    }
      
    // skip for output toplevelport;
    PortDirection* dir = _sta->network()->direction(pin);
    if( dir->isAnyOutput() ) {
      if( _sta->network()->isTopLevelPort(pin)) {
        continue;
      }
    }
    // skip for input Non-FF/Macro pins
    else {
      if( !_sta->network()->isTopLevelPort(pin)) {
        continue;
      }
    }
    */

    // Survived
    // FF/Macro with non-clock output pins
    // topLevelPort(I). i.e. input top-level-port.
    
    //skip for clock pin
    if( _sta->network()->isCheckClk(pin) || _sta->sdc()->isClock(pin) ) {
      continue;
    }
    if( string(_sta->network()->name(pin)) == "reset_i" ) {
      continue;
    }

    MacroNetlist::Vertex* mVert = GetVertex(pin);

//    cout << "[" << _sta->network()->name(pin) << "]" << endl;

//  virtual PathEndSeq *findPathEnds(ExceptionFrom *from,
//           ExceptionThruSeq *thrus,
//           ExceptionTo *to,
//           // Use corner NULL to report timing
//           // for all corners.
//           const Corner *corner,
//           // max for setup checks.
//           // min for hold checks.
//           // min_max for setup and hold checks.
//           const MinMaxAll *min_max,
//           // Number of path ends to report in
//           // each group.
//           int group_count,
//           // Number of paths to report for
//           // each endpoint.
//           int endpoint_count,
//           // endpoint_count paths report unique pins
//           // without rise/fall variations.
//           bool unique_pins,
//           // Min/max bounds for slack of
//           // returned path ends.
//           float slack_min,
//           float slack_max,
//           // Sort path ends by slack ignoring path groups.
//           bool sort_by_slack,
//           // Path groups to report.
//           // Null or empty list reports all groups.
//           PathGroupNameSet *group_names,
//           // Predicates to filter the type of path
//           // ends returned.
//           bool setup,
//           bool hold,
//           bool recovery,
//           bool removal,
//           bool clk_gating_setup,
//           bool clk_gating_hold);
    
    
    Corner* corner = _sta->corners()->findCorner(0);

    PinSet* pSet = new PinSet;
    pSet->insert(pin);

//    InstanceSet* instSet = new InstanceSet;
//  ExceptionFrom(PinSet *pins,
//    ClockSet *clks,
//    InstanceSet *insts,
//    const TransRiseFallBoth *tr,
//    bool own_pts);
    
    // Get Pin direction
    PortDirection* dir = _sta->network()->direction(pin);

    ExceptionFrom* from = (!dir->isAnyOutput())? 
      _sta->sdc()->makeExceptionFrom(pSet, NULL, NULL, 
        TransRiseFallBoth::riseFall()) : NULL;

    ExceptionTo* to = (dir->isAnyOutput())? 
      _sta->sdc()->makeExceptionTo(pSet, NULL, NULL, 
        TransRiseFallBoth::riseFall(), 
        TransRiseFallBoth::riseFall()) : NULL;

    PathEndSeq *ends = _sta->findPathEnds(from, NULL, to, //from, thru, to
        false,
        corner, MinMaxAll::max(), // corner, delay_min_max
        INT_MAX, 1, false, //group_count, endpoint_count, unique_pins
        -INF, INF, //slack_min, slack_max
        false, NULL, //sort_by_slack, group_name
        true, true, // setup, hold
        true, true, // recovery, removal
        true, true); // clk gating setup, hold

    if (ends->empty()) {
//      cout << "ERROR: NO PATH !" << endl;
      continue;
    }

    PathEndSeq::Iterator tmpIter(ends), pathEndIter(ends), pathEndIter2(ends);
    int edgeCnt = 0;
    while( tmpIter.hasNext() ) {
      tmpIter.next();
      edgeCnt ++; 
    }

//    delete pSet;

//    cout << "Total Possible Path: " << edgeCnt << endl;

    while( pathEndIter.hasNext()) {
      PathEnd *end = pathEndIter.next();
      //TimingPathPrint( _sta, end );

      PathExpanded expanded(end->path(), _sta);

      // get Un-clockpin 
      int startIdx = 0;
      PathRef *startPath = expanded.path(startIdx);
      while( startIdx < expanded.size()-1 && startPath->isClock(_sta->search())) {
        startPath = expanded.path(++startIdx);
      }

      PathRef *endPath = expanded.path(expanded.size()-1);

      sta::Vertex *startVert = startPath->vertex(_sta);
      sta::Vertex *endVert = endPath->vertex(_sta);

      Pin* startPin = startVert->pin();
      Pin* endPin = endVert->pin();

//      cout << _sta->network()->name(startPin) << " -> " << _sta->network()->name(endPin) << endl;
      MacroNetlist::Vertex* startVertPtr = GetVertex( startPin );
      MacroNetlist::Vertex* endVertPtr = GetVertex( endPin );

      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // OpenSTA could return Null Vertex:
      // This means that non-Timing cells would be returned by findPathEnds command...
      if( startVertPtr == NULL || endVertPtr == NULL) {
        continue;
      }

//      cout << startVertPtr << " -> " << endVertPtr << endl;

      /*
       * Previous
      // Edge Update
      auto mapPtr = vertexPairEdgeMap.find( make_pair(startVertPtr, endVertPtr) );
      if( mapPtr == vertexPairEdgeMap.end() ) {
        vertexPairEdgeMap[ make_pair(startVertPtr, endVertPtr) ] = edgeStor.size();
        edgeStor.push_back( MacroNetlist::Edge(startVertPtr, endVertPtr, 1) );
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
  
  endTime = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
  cout << "Edge Building for IO: " << elapsed.count()<< "s" << endl;

//  for(auto& curEdge: edgeStor) {
//    cout << "EdgePrint: " << &curEdge << " " << curEdge.from << " " << curEdge.to << endl;
//  }

          /*
          // This MUST be done, otherwise the memory address would be different
          edgeStor.reserve(edgeCnt);

          // below is for generating Edges 
          while( pathEndIter2.hasNext()) {
      PathEnd *end = pathEndIter2.next();
      //TimingPathPrint( _sta, end );

      PathExpanded expanded(end->path(), _sta);

      // get Un-clockpin 
      int startIdx = 0;
      PathRef *startPath = expanded.path(startIdx);
      while( startIdx < expanded.size()-1 && startPath->isClock(_sta->search())) {
        startPath = expanded.path(++startIdx);
      }

      PathRef *endPath = expanded.path(expanded.size()-1);

      sta::Vertex *startVert = startPath->vertex(_sta);
      sta::Vertex *endVert = endPath->vertex(_sta);

      Pin* startPin = startVert->pin();
      Pin* endPin = endVert->pin();

      Instance* startInst = _sta->network()->instance(startPin);
      Instance* endInst = _sta->network()->instance(endPin);

      void* startPtr = (_sta->network()->isTopLevelPort(startPin))? 
        (void*) startPin : (void*) startInst;
      void* endPtr = (_sta->network()->isTopLevelPort(endPin))?
        (void*) endPin : (void*) endInst;

      MacroNetlist::Vertex* startVertPtr = &vertexStor[pinInstVertexMap[startPtr]];
      MacroNetlist::Vertex* endVertPtr = &vertexStor[pinInstVertexMap[endPtr]];

      // Edge Update
      auto mapPtr = vertexPairEdgeMap.find( make_pair(startVertPtr, endVertPtr) );
      if( mapPtr == vertexPairEdgeMap.end() ) {
        vertexPairEdgeMap[ make_pair(startVertPtr, endVertPtr) ] = edgeStor.size();
        edgeStor.push_back( MacroNetlist::Edge(startVertPtr, endVertPtr, 1) );
        // Vertex Update
        startVertPtr->to.push_back(&edgeStor[edgeStor.size()-1]);
        endVertPtr->from.push_back(&edgeStor[edgeStor.size()-1]);
      }
      else {
        // increase the edge weight
        edgeStor[mapPtr->second].weight ++;
      }
    }
    */
    
    
//    mapPtr = pinInstVertexMap.find(endPtr);
//    if( mapPtr == pinInstVertexMap.end()) {
//      pinInstVertexMap[ endPtr ] = vertexStor.size();
//      vertexStor.push_back( Vertex(endPtr, endClass) );
//    }
    
//    if( _sta->network()->isTopLevelPort(startPin) ||
//        _sta->network()->isTopLevelPort(endPin)) {
//      cout << _sta->network()->name(startPin) << " -> " << _sta->network()->name(endPin) << endl; 
//    } 
  cout << "Sequential Graph Building is Done!" << endl;
  cout << "Vertex: " << vertexStor.size() << endl;
//  cout << "Edge: " << edgeStor.size() << endl;
  cout << "Edge: " << adjMatrix.nonZeros() << endl;
}


// macroStr & macroInstMap update
void MacroCircuit::UpdateInstanceToMacroStor() {
  sta::VertexIndex numVertex = _sta->graph()->vertexCount();
  for(int i=1; i<=numVertex; i++) {
    sta::Vertex* vert = _sta->graph()->vertex(i);
    sta::Pin* pin = vert->pin();
    if( _sta->network()->isTopLevelPort(pin) ) {
      continue;
    }
    sta::Instance* inst = _sta->network()->instance(pin);
    string instName = _sta->network()->name(inst);

//    ReplaceStringInPlace(instName, "\\[", "[");
//    ReplaceStringInPlace(instName, "\\]", "]");
//    ReplaceStringInPlace(instName, "\\/", "/");

    auto mnPtr = macroNameMap.find(instName); 
    if( mnPtr == macroNameMap.end()) {
      continue; 
    }

    // macro & macroInstMap update
    macroStor[mnPtr->second].instPtr = inst;
    macroInstMap[inst] = mnPtr->second; 
  }
}

// macroStor Update
void MacroCircuit::UpdateVertexToMacroStor() {
  for(auto& curVertex: vertexStor) {
    if( curVertex.vertexClass != VertexClass::instMacro ) {
      continue;
    }
    string name = _sta->network()->name((sta::Instance*)curVertex.ptr);
//    cout << "Macro: " << name << endl;
//    ReplaceStringInPlace(name, "\\[", "[");
//    ReplaceStringInPlace(name, "\\]", "]");
//    ReplaceStringInPlace(name, "\\/", "/");

    auto mPtr = macroNameMap.find( name );
    if( mPtr == macroNameMap.end() ) {
      cout << "**ERROR: The Macro Name must be in macro NameMap: " << name << endl;
      exit(1);
    } 

    macroStor[mPtr->second].ptr = &curVertex;
  }
}

void MacroCircuit::FillMacroConnection() {

  vector<MacroNetlist::Vertex*> searchVert;
  for(auto& curMacro: macroStor) {
    searchVert.push_back( curMacro.ptr );
  }

  for(int i=0; i<4; i++) {
    searchVert.push_back( &vertexStor[i] );
  }

  cout << "searchVertSize: " << searchVert.size() << endl;
  // macroNetlistWeight Initialize
  macroWeight.resize(searchVert.size());
  for(size_t i=0; i<searchVert.size(); i++) {
    macroWeight[i] = vector<int> (searchVert.size(), 0);
  }


  SMatrix calMatrix = adjMatrix;
  if( _env.searchDepth == 2) {
    calMatrix = calMatrix * calMatrix; 
  }
  else if( _env.searchDepth == 3) {
    calMatrix = calMatrix * calMatrix; 
    calMatrix = calMatrix * adjMatrix; 
  }
  else if( _env.searchDepth == 4) {
    calMatrix = calMatrix * calMatrix; 
    calMatrix = calMatrix * calMatrix; 
  }
  else if(_env.searchDepth == 5){
    calMatrix = calMatrix * calMatrix; 
    calMatrix = calMatrix * calMatrix; 
    calMatrix = calMatrix * adjMatrix; 
  }


  auto startTime = std::chrono::system_clock::now();
  for(auto& curVertex1: searchVert) {
    int idx1 = &curVertex1 - &searchVert[0];
    for(auto& curVertex2: searchVert) {
      int idx2 = &curVertex2 - &searchVert[0];
      if( idx1 == idx2 ) { 
        continue;
      }

      if( curVertex1->vertexClass == VertexClass::pin 
          && curVertex2->vertexClass == VertexClass::pin) {
        continue;
      }
     
//      cout << "[ " << curMacro1.name << " --> " << curMacro2.name << " ]" <<endl; 
//      cout << "[ " << curMacro1.ptr << " --> " << curMacro2.ptr << " ]" <<endl;
     
      PinGroup* ptr1 = (PinGroup*)curVertex1->ptr;
      PinGroup* ptr2 = (PinGroup*)curVertex2->ptr;

      string name1 = (curVertex1->vertexClass == VertexClass::pin)? 
        ptr1->name() : _sta->network()->name((sta::Instance*)curVertex1->ptr);
      string name2 = (curVertex2->vertexClass == VertexClass::pin)?
        ptr2->name() : _sta->network()->name((sta::Instance*)curVertex2->ptr);
      cout << "[ " << name1 << " --> " << name2 << " ]" <<endl;

      macroWeight[idx1][idx2] = GetPathWeightMatrix( calMatrix, curVertex1, curVertex2);
//      cout << GetPathWeight( curVertex1, curVertex2, 3) << endl;
    }
  }
  auto endTime = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
  cout << "BFS Search: " << elapsed.count()<< "s" << endl;
  for(size_t i=0; i<searchVert.size(); i++) {
    for(size_t j=0; j<searchVert.size(); j++) {
      cout << macroWeight[i][j] << " ";
    }
    cout << endl;
  }

}

using std::queue;

bool isNotVisited(MacroNetlist::Vertex* vert, vector<MacroNetlist::Vertex*> &path) {
  for(auto& curVert: path) {
    if( curVert == vert ) {
      return false;
    }
  }
  return true;
}

bool isTerminal(MacroNetlist::Vertex* vert, 
    MacroNetlist::Vertex* target ) {
  return ( vert != target && 
      (vert->vertexClass == VertexClass::pin || 
      vert->vertexClass == VertexClass::instMacro) );
}

int MacroCircuit::GetPathWeightMatrix(
    SMatrix& mat, MacroNetlist::Vertex* from, MacroNetlist::Vertex* to) {
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

int MacroCircuit::GetPathWeight(MacroNetlist::Vertex* from, MacroNetlist::Vertex* to, int limit ) {
  queue< vector<MacroNetlist::Vertex*> > q;
  vector<MacroNetlist::Vertex*> path;
  path.reserve(limit+2);
  path.push_back(from);

  q.push(path);
//  for(auto& curEdge: edgeStor) {
//    cout << "EdgePrint2: " << curEdge.from << " " << curEdge.to << endl;
//  }

//  for(auto& curAdj : from->to) {
//    cout << "Adj: " << curAdj << " " << curAdj->to << endl;
//  }

  vector< vector<MacroNetlist::Vertex*> > result;
  int pathDepth = 1;
  cout << "Depth: " << 1 << endl;
  while(!q.empty()) {
//    if( i > limit ) {
//      break;
//    }

    path = q.front();
    q.pop();
    MacroNetlist::Vertex* last = path[path.size()-1];

//    cout << path.size() << endl;
//    for(auto& curVert: path){
//      cout << "currentVert: " << curVert <<" ";
//    }
//  for(auto& curAdj : from->to) {
//    cout << "Adj2: " << curAdj->to << endl;
//  }
//    cout << endl;
//  for(auto& curAdj : last->to) {
//    cout << "Adj3: " << curAdj << endl;
//  }

    if( pathDepth < path.size() ){
      cout << "Depth: " << path.size() << endl;
      pathDepth = path.size();
    }


    if( last == to ) { 
      result.push_back(path);
      continue;
    }
//    cout << "dead" << endl;
//  for(auto& curAdj : last->to) {
//    cout << "Adj4: " << curAdj->to << endl;
//  }

    for(auto& curOutEdge: last->to) {
      MacroNetlist::Vertex* nextVert = edgeStor[curOutEdge].to;
//      cout << "PrevToConnected: " << nextVert << endl;
      if( !isTerminal(nextVert, to) && path.size() < limit 
          && isNotVisited(nextVert, path)) {
//        cout << "ToConnected: " << nextVert << endl;
        vector<MacroNetlist::Vertex*> newPath(path);
        newPath.push_back(nextVert);
        q.push(newPath);
      } 
    }
    for(auto& curInEdge: last->from) {
      MacroNetlist::Vertex* nextVert = edgeStor[curInEdge].from;
//      cout << "PrevFromConnected: " << nextVert << endl;
      if( !isTerminal(nextVert, to) && path.size() < limit 
          && isNotVisited(nextVert, path)) {
//        cout << "FromConnected: " << nextVert << endl;
        vector<MacroNetlist::Vertex*> newPath(path);
        newPath.push_back(nextVert);
        q.push(newPath);
      } 
    }
//    for(auto& curVert: path) {
//      MacroNetlist::PinGroup* ptr = (MacroNetlist::PinGroup*) curVert->ptr;
//      string name = (curVert->vertexClass == VertexClass::pin)?
//        ptr->name() :
//        _sta->network()->name((sta::Instance*)curVert->ptr);
//      cout << name << " -> ";
//    }
//    cout << endl;
//    cout << "qSize: " << q.size() << " " << path.size() << endl;
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
      ret += edgeStor[vPtr->second].weight;
    }
    cout << " " ;
    for(auto& curVert: curPath) {
      MacroNetlist::PinGroup* ptr = (MacroNetlist::PinGroup*) curVert->ptr;
      string name = (curVert->vertexClass == VertexClass::pin)?
        ptr->name() :
        _sta->network()->name((sta::Instance*)curVert->ptr);
      cout << name << " -> ";
    }
    cout << endl;
  }
  return ret;
}


// 
// Update Macro Location
// from partition
void MacroCircuit::UpdateMacroLoc( MacroNetlist::Partition& part) {
  for(auto& curMacro : part.macroStor) {
    auto mnPtr = macroNameMap.find(curMacro.name);
    if( mnPtr == macroNameMap.end() ) {
      cout << "ERROR: Macro not exists in MacroCircuit " << curMacro.name << endl;
      exit(1);
    }

    // Update Macro Location
    int macroIdx = mnPtr->second;
    macroStor[macroIdx].lx = curMacro.lx;
    macroStor[macroIdx].ly = curMacro.ly;  
  }
}

// Legalizer for macro locations
void MacroCircuit::StubPlacer(double snapGrid) {
//  cout << "Macro Legalizing process... ";
  cout << "Macro Stub Placement process... ";

  snapGrid *= 10;

  int sizeX = (int)( (_cinfo.ux - _cinfo.lx) / snapGrid + 0.5f);
  int sizeY = (int)( (_cinfo.uy - _cinfo.ly) / snapGrid + 0.5f);

  int** checker = new int* [sizeX];
  for(int i=0; i<sizeX; i++) {
    checker[i] = new int [sizeY];
    for(int j=0; j<sizeY; j++) {
      checker[i][j] = -1; // uninitialize
    }
  }
  cout << "cinfo: " << _cinfo.ux << " " << _cinfo.uy << endl;
  cout << "GRID Width: " << sizeX << " Height: " << sizeY << endl;


  bool isOverlap = true;
  do {

    for(auto& curMacro: macroStor) { 
      // Macro Projection in (llx, lly, urx, ury)
      if( curMacro.lx < _cinfo.lx ) curMacro.lx = _cinfo.lx;
      if( curMacro.ly < _cinfo.ly ) curMacro.ly = _cinfo.ly;
      if( curMacro.lx + curMacro.w > _cinfo.ux ) curMacro.lx = _cinfo.ux - curMacro.w;
      if( curMacro.ly + curMacro.h > _cinfo.uy ) curMacro.ly = _cinfo.uy - curMacro.h;

      if( curMacro.lx < _cinfo.lx || curMacro.lx + curMacro.w > _cinfo.ux ) {
        cout << "ERROR: Macro Legalizer detects width is not enough" << endl;
        exit(1);
      }
      if( curMacro.ly < _cinfo.lx || curMacro.ly + curMacro.h > _cinfo.uy ) {
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
  out_size = (end - str) < len-1 ? (end - str) : len-1;

  // Copy trimmed string and add null terminator
  memcpy(out, str, out_size);
  out[out_size] = 0;

  return out_size;
}


void MacroCircuit::ParseGlobalConfig(string fileName) {
  cout << "Parsing globalConfig: " << fileName << " ... " ;
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
  cout << "Done!" << endl;
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
Plot(string fileName, vector<MacroNetlist::Partition>& set) {
  cout<<"OutPut Plot file is "<<fileName<<endl;
  std::ofstream gpOut(fileName);
  if (!gpOut.good()) {
    cout << "Warning: output file " << fileName
      << " can't be opened" << endl;
  }
  //   gpOut <<"set terminal png size 1024,768" << endl;

  gpOut<<"#Use this file as a script for gnuplot"<<endl;
  gpOut<<"#(See http://www.gnuplot.info/ for details)"<<endl;
  gpOut << "set nokey"<<endl;
  
  gpOut << "set size ratio -1" << endl;
  gpOut << "set title '' " << endl; 

  gpOut << "set xrange[" << _cinfo.lx << ":" << _cinfo.ux << "]" << endl;
  gpOut << "set yrange[" << _cinfo.ly << ":" << _cinfo.uy << "]" << endl;

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
      << " fc rgb \"#FFFFFFFF\"" << endl;
  }

  gpOut << "plot '-' w l" << endl;
  gpOut << "EOF" << endl;
  gpOut << "pause -1 'Press any key' "<<endl;
  gpOut.close();

}
