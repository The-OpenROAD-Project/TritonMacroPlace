#include <iostream>
#include <queue>
#include <chrono>
#include "lefdefIO.h"
#include "parse.h"
#include "timingSta.h"

MacroCircuit::MacroCircuit(Circuit::Circuit& ckt, EnvFile& env, CircuitInfo& cinfo) :
_ckt(ckt), _env(env), _cinfo(cinfo) {
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

    // macro cell update
    macroNameMap[ curComp.id() ] = macroStor.size(); 
    macroStor.push_back( 
        MacroNetlist::Macro( curComp.id(), curComp.name(), 
          1.0*curComp.placementX()/defScale, 
          1.0*curComp.placementY()/defScale, 
          curMacro.sizeX(), curMacro.sizeY())); 

//    cout << curComp.id() << " " 
//      << 1.0*curComp.placementX()/defScale << " " 
//      << 1.0*curComp.placementY()/defScale << " "
//      << curMacro.sizeX() << " " << curMacro.sizeY() << " " 
//      << curComp.name() << endl;
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
  pinGroupStor[(int)PinGroupClass::West].pinGroupClass = PinGroupClass::West;
  pinGroupStor[(int)PinGroupClass::East].pinGroupClass = PinGroupClass::East;
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
  cout << "West: " << pinGroupStor[(int)PinGroupClass::West].pins.size() << endl;
  cout << "East: " << pinGroupStor[(int)PinGroupClass::East].pins.size() << endl;
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
    
    ReplaceStringInPlace( instName, "\\[",  "[" );
    ReplaceStringInPlace( instName, "\\]",  "]" );
    ReplaceStringInPlace( instName, "\\/",  "/" );

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
    cout << "ERROR: " << _sta->network()->name(pin) << " not exists in pinInstVertexMap" << endl;
    exit(1);
  }
  return &vertexStor[vertPtr->second];
}

void MacroCircuit::FillVertexEdge() {
  cout << "Generating Sequantial Graph..." << endl; 
  sta::EdgeIndex numEdge = _sta->graph()->edgeCount();
  sta::VertexIndex numVertex = _sta->graph()->vertexCount();

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

    bool isTopPin = _sta->network()->isTopLevelPort(pin);
    if( isTopPin ) {
      continue;
    }
    // skip for below two cases; Already visited, or non-FF cells
    Instance* inst = _sta->network()->instance(pin);
    if( instMap.find(inst ) != instMap.end()) {
      continue;
    }
    instMap.insert( inst );


    LibertyCell* libCell = _sta->network()->libertyCell(inst);
    if( !libCell -> hasSequentials() && macroInstMap.find(inst) == macroInstMap.end() ) {
      continue;
    }  
    
    cout << "vert: " << _sta->network()->name(inst) << endl;
    cout << "pin: " << _sta->network()->name(pin) << endl << endl;

    pair<void*, VertexClass> vertex = GetPtrClassPair( pin );
    auto vertPtr = pinInstVertexMap.find( vertex.first );
    if( vertPtr == pinInstVertexMap.end()) {
      pinInstVertexMap[ vertex.first ] = vertexStor.size();
      vertexStor.push_back( MacroNetlist::Vertex(vertex.first, vertex.second)); 
    }

//    cout << "vert: " << _sta->network()->name(inst) << endl;

  }
//  exit(1);

  auto endTime = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
  cout << "Vertex Building: " << elapsed.count()<< "s" << endl;


  startTime = std::chrono::system_clock::now();

  // Query Get_FanIn/ Get_FainOut
  for(int i=1; i<=numVertex; i++) {
    Vertex* vert = _sta->graph()->vertex(i);
    Pin* pin = vert->pin();
    
    bool isTopPin = _sta->network()->isTopLevelPort(pin);
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
    if( _sta->network()->isCheckClk(pin) ) {
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
   

    cout << "[" << _sta->network()->name(pin) << "]" << endl;
    
    PinSeq pinStor;
    pinStor.push_back(pin);
  
    PortDirection* dir = _sta->network()->direction(pin);
    MacroNetlist::Vertex* curVertex = GetVertex(pin);



    if( dir->isAnyOutput() ) {
      cout << "FanOut!" << endl;
      PinSet *fanout = _sta->findFanoutPins(&pinStor, false, true ,
          500, 700,
          false, false);
      for(auto& adjPin: *fanout) {
        cout << _sta->network()->name(pin) << " -> " << _sta->network()->name(adjPin) << endl;
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
//        cout << _sta->network()->name(pin) << " -> " << _sta->network()->name(adjPin) << endl;

        auto mapPtr = vertexPairEdgeMap.find( make_pair(curVertex , adjVertex) );
        if( mapPtr == vertexPairEdgeMap.end() ) {
          vertexPairEdgeMap[ make_pair(curVertex, adjVertex) ] = edgeStor.size();
          edgeStor.push_back( MacroNetlist::Edge(curVertex, adjVertex, 1) );

          // Vertex Update
          curVertex->to.push_back(&edgeStor[edgeStor.size()-1]);
          adjVertex->from.push_back(&edgeStor[edgeStor.size()-1]);
        }
        else {
          // increase the edge weight
          edgeStor[mapPtr->second].weight ++;
        }
      }
    }
    else {
      cout << "FanIn!" << endl;
      PinSet *fanin = _sta->findFaninPins(&pinStor, false, true ,
          500, 700,
          false, false);
      for(auto& adjPin: *fanin) {
        cout <<  _sta->network()->name(adjPin) << " -> " << _sta->network()->name(pin) << endl;
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
//        cout << _sta->network()->name(adjPin) << " -> " << _sta->network()->name(pin) << endl;

        auto mapPtr = vertexPairEdgeMap.find( make_pair(adjVertex, curVertex) );
        if( mapPtr == vertexPairEdgeMap.end() ) {
          vertexPairEdgeMap[ make_pair(adjVertex, curVertex) ] = edgeStor.size();
          edgeStor.push_back( MacroNetlist::Edge(adjVertex, curVertex, 1) );

          // Vertex Update
          curVertex->from.push_back(&edgeStor[edgeStor.size()-1]);
          adjVertex->to.push_back(&edgeStor[edgeStor.size()-1]);
        }
        else {
          // increase the edge weight
          edgeStor[mapPtr->second].weight ++;
        }
      }
    }
  }
  endTime = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
  cout << "Edge Building: " << elapsed.count() << "s" << endl;


  cout << "Sequential Graph Building is Done!" << endl;
  cout << "Vertex: " << vertexStor.size() << endl;
  cout << "Edge: " << edgeStor.size() << endl;




  /*
   * Previous Version

  Corner* corner = _sta->corners()->defaultCorner();

  cout << -INF << " " << INF << endl;
  PathEndSeq *ends = _sta->findPathEnds(NULL, NULL, NULL, //from, thru, to
      corner, MinMaxAll::max(), // corner, delay_min_max
      INT_MAX, 1, false, //group_count, endpoint_count, unique_pins
      -INF, INF, //slack_min, slack_max
      true, NULL, //sort_by_slack, group_name
      true, false, // setup, hold
      true, true, // recovery, removal
      true, false); // clk gating setup, hold

  if (ends->empty()) cout << "NO PATH !" << endl;

  PathEndSeq::Iterator tmpIter(ends), pathEndIter(ends), pathEndIter2(ends);
  int edgeCnt = 0;
  while( tmpIter.hasNext() ) {
    tmpIter.next();
    edgeCnt ++; 
  }
  
  cout << "Total Possible Path: " << edgeCnt << endl;

  // below is for generating vertex 
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

    Instance* startInst = _sta->network()->instance(startPin);
    Instance* endInst = _sta->network()->instance(endPin);

    void* startPtr = (_sta->network()->isTopLevelPort(startPin))? 
      (void*) startPin : (void*) startInst;
    void* endPtr = (_sta->network()->isTopLevelPort(endPin))?
      (void*) endPin : (void*) endInst;

    // below two var is only used when each is not the topLevelPorts
    string startName = _sta->network()->name(startInst);
    string endName = _sta->network()->name(endInst);

    ReplaceStringInPlace( startName, "\\[",  "[" );
    ReplaceStringInPlace( startName, "\\]",  "]" );
    ReplaceStringInPlace( startName, "\\/",  "/" );
    
    ReplaceStringInPlace( endName, "\\[",  "[" );
    ReplaceStringInPlace( endName, "\\]",  "]" );
    ReplaceStringInPlace( endName, "\\/",  "/" );

    VertexClass startClass = (_sta->network()->isTopLevelPort(startPin))?
      VertexClass::pin : 
      (macroNameMap.find( startName ) != macroNameMap.end())? 
        VertexClass::instMacro : VertexClass::instOther;
    VertexClass endClass = (_sta->network()->isTopLevelPort(endPin))?
      VertexClass::pin : 
      (macroNameMap.find( endName ) != macroNameMap.end())? 
        VertexClass::instMacro : VertexClass::instOther;

//    if( startClass == VertexClass::instMacro ) { 
//      cout << _sta->network()->name(startInst) << endl;
//    }

    // Vertex Update
    auto mapPtr = pinInstVertexMap.find(startPtr);
    if( mapPtr == pinInstVertexMap.end()) {
      pinInstVertexMap[ startPtr ] = vertexStor.size();
      vertexStor.push_back( MacroNetlist::Vertex(startPtr, startClass) );
    }
    
    mapPtr = pinInstVertexMap.find(endPtr);
    if( mapPtr == pinInstVertexMap.end()) {
      pinInstVertexMap[ endPtr ] = vertexStor.size();
      vertexStor.push_back( MacroNetlist::Vertex(endPtr, endClass) );
    }
    
//    if( _sta->network()->isTopLevelPort(startPin) ||
//        _sta->network()->isTopLevelPort(endPin)) {
    cout << _sta->network()->name(startPin) << " -> " << _sta->network()->name(endPin) << endl;
//      } 
  }

  // VertexPtrMap Initialize
  for(auto& curVertex: vertexStor) {
    vertexPtrMap[&curVertex] = &curVertex - &vertexStor[0];
  }

  cout << "Total Vertex: " << vertexStor.size() << endl;

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
    
    
//    mapPtr = pinInstVertexMap.find(endPtr);
//    if( mapPtr == pinInstVertexMap.end()) {
//      pinInstVertexMap[ endPtr ] = vertexStor.size();
//      vertexStor.push_back( Vertex(endPtr, endClass) );
//    }
    
//    if( _sta->network()->isTopLevelPort(startPin) ||
//        _sta->network()->isTopLevelPort(endPin)) {
//      cout << _sta->network()->name(startPin) << " -> " << _sta->network()->name(endPin) << endl; 
//    } 
  }
    */

  cout << "Total Edge: " << edgeStor.size() << endl;
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

    ReplaceStringInPlace(instName, "\\[", "[");
    ReplaceStringInPlace(instName, "\\]", "]");
    ReplaceStringInPlace(instName, "\\/", "/");

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
    ReplaceStringInPlace(name, "\\[", "[");
    ReplaceStringInPlace(name, "\\]", "]");
    ReplaceStringInPlace(name, "\\/", "/");

    auto mPtr = macroNameMap.find( name );
    if( mPtr == macroNameMap.end() ) {
      cout << "**ERROR: The Macro Name must be in macro NameMap: " << name << endl;
      exit(1);
    } 

    macroStor[mPtr->second].ptr = &curVertex;
  }
}

void MacroCircuit::FillMacroConnection() {

  for(auto& curMacro1: macroStor) {
    int idx1 = &curMacro1 - &macroStor[0];
    for(auto& curMacro2: macroStor) {
      int idx2 = &curMacro2- &macroStor[0];
      if( idx1 == idx2 ) { 
        continue;
      }
     
      cout << "[ " << curMacro1.name << " --> " << curMacro2.name << " ]" <<endl; 
      cout << GetPathWeight( curMacro1.ptr, curMacro2.ptr, 20) << endl;
    }
  }

//  VertexInfo vertexList;
//  vertexList.push_back( make_pair(&curVertex, 1)  );

//  unordered_set<MacroNetlist::Vertex*> vertexChk;

//  // Search up to depth 3.
//  for(int i=0; i<5; i++) {
//    vertexList = GetAdjVertexList( vertexList, vertexChk );
//  }
//
//  cout << _sta->network()->name( (Instance*)curVertex.ptr ) << endl;
//
//  // Get Macro Cells Only
//  for(auto& curVertex: vertexList) {
//    // Macro-Pins filtering
//    if( curVertex.first->vertexClass == VertexClass::instOther ) {
//      continue;
//    }
//
//    string name = (curVertex.first->vertexClass == VertexClass::pin)? 
//      _sta->network()->name( (Pin*) curVertex.first->ptr ) :
//      _sta->network()->name( (Instance*) curVertex.first->ptr );
//    cout << curVertex.first << " " 
//      << name << " " 
//      << curVertex.second << endl;
//  }
}



/*
VertexInfo MacroCircuit::GetAdjVertexList( VertexInfo candiVertex, 
    unordered_set<MacroNetlist::Vertex*> &vertexVisit) {  
  unordered_map< MacroNetlist::Vertex*, int > vertexCheck;
  for(auto& curVertex: candiVertex ) {
    vertexCheck[curVertex.first] = curVertex.second;
  }

  // forward
  for(auto& curVertex: candiVertex ) {
    for(auto& curEdge : curVertex.first->to) {
      // skip for null
      if( ! curEdge->to ) {
        continue;
      }

      if( vertexVisit.find( curEdge->to ) != vertexVisit.end() ) {
        continue;
      }
//      vertexVisit.insert(curEdge->to);

      auto vertPtr = vertexCheck.find( curEdge->to );
      // new Vertex
      if( vertPtr == vertexCheck.end() ) {
        vertexCheck[curEdge->to] = curVertex.second * curEdge->weight;
      }
      // previous Vertex
      else {
        vertPtr->second += curEdge->weight * curVertex.second;
      }
    }
  }  
  
  // backward 
  for(auto& curVertex: candiVertex ) {
    for(auto& curEdge : curVertex.first->from) {
      // skip for null
      if( ! curEdge->from ) {
        continue;
      }
      
      if( vertexVisit.find( curEdge->from ) != vertexVisit.end() ) {
        continue;
      }
//      vertexVisit.insert(curEdge->from);

      auto vertPtr = vertexCheck.find( curEdge->from );
      // new Vertex
      if( vertPtr == vertexCheck.end() ) {
        vertexCheck[curEdge->from] = curVertex.second * curEdge->weight;
      }
      // previous Vertex
      else {
        vertPtr->second += curEdge->weight * curVertex.second;
      }
    }
  }  
 
  VertexInfo ().swap(candiVertex);
  candiVertex.reserve( vertexCheck.size() );

  for(auto& curPair: vertexCheck) {
//    cout << curPair.first << " " << curPair.second << endl;
    candiVertex.push_back( curPair );
    vertexVisit.insert(curPair.first);
  }
  return candiVertex;
}
*/


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

int MacroCircuit::GetPathWeight(MacroNetlist::Vertex* from, MacroNetlist::Vertex* to, int limit ) {
  queue< vector<MacroNetlist::Vertex*> > q;
  vector<MacroNetlist::Vertex*> path;
  path.reserve(limit+2);
  path.push_back(from);

  q.push(path);

  vector< vector<MacroNetlist::Vertex*> > result;
  while(!q.empty()) {
//    if( i > limit ) {
//      break;
//    }

    path = q.front();
    q.pop();
    MacroNetlist::Vertex* last = path[path.size()-1];

    if( last == to ) { 
      result.push_back(path);
    }

    for(auto& curOutEdge: last->to) {
      MacroNetlist::Vertex* nextVert = curOutEdge->to;
      if( !isTerminal(nextVert, to) && path.size()+1 < limit 
          && isNotVisited(nextVert, path)) {
//      cout << "ToConnected: " << nextVert << endl;
        vector<MacroNetlist::Vertex*> newPath(path);
        newPath.push_back(nextVert);
        q.push(newPath);
      } 
    }
    for(auto& curInEdge: last->from) {
      MacroNetlist::Vertex* nextVert = curInEdge->from;
      if( !isTerminal(nextVert, to) && path.size()+1 < limit 
          && isNotVisited(nextVert, path)) {
//      cout << "FromConnected: " << nextVert << endl;
        vector<MacroNetlist::Vertex*> newPath(path);
        newPath.push_back(nextVert);
        q.push(newPath);
      } 
    }
  }
  int ret = 0;
  for(auto& curPath: result) {
    for(int i=0; i<curPath.size()-1; i++) {
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
      string name = (curVert->vertexClass == VertexClass::pin)?
        _sta->network()->name((sta::Pin*)curVert->ptr) :
        _sta->network()->name((sta::Instance*)curVert->ptr);
      cout << name << " -> ";
    }
    cout << endl;
  }
  return ret;
}
//MACRO_NETLIST_NAMESPACE_CLOSE
