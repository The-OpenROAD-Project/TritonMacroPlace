#include "timingSta.h"
#include "parse.h"
#include "lefdefIO.h"
#include "partition.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <set>
#include <tcl.h>
#include <boost/icl/interval_map.hpp>
#include <boost/icl/closed_interval.hpp>

#define STRING_EQUAL(a, b) !strcmp((a), (b))

using namespace boost::icl;
using namespace MacroNetlist;
using MacroNetlist::Partition;
vector<pair<Partition, Partition>> GetPart ( 
    CircuitInfo& cInfo,  
    Partition& partition, bool isHorizontal );

void PrintAllSets(FILE* fp, CircuitInfo& cInfo, 
    vector< vector<Partition> >& allSets);

typedef vector<pair<Partition, Partition>> TwoPartitions;
    
// Partition Class --> macroStor's index.
void UpdateMacroPartInfo( MacroCircuit& _mckt,
    MacroNetlist::Partition& part, 
        unordered_map< MacroNetlist::PartClass, vector<int>,
        MyHash<MacroNetlist::PartClass>>& macroPartMap);
 
int main(int argc, char** argv) {
  using namespace std;
  using namespace sta;
  EnvFile _env;

  if( !ParseArgv(argc, argv, _env) ) {
    PrintUsage();
    exit(1);  
  }
 
  _env.Print(); 
  
  cout << "LEF/DEF Parsing... ";
  Circuit::Circuit _ckt(_env.lefStor, _env.def);
  cout << "Done!" << endl;

  double defScale = _ckt.defUnit;
  defiPoints points = _ckt.defDieArea.getPoint();
  if( points.numPoints != 2) {
    cout << "ERROR: The Layout must be rectangle..! "<< endl;
    exit(1);
  }

  if( _ckt.defRowStor.size() == 0) {
    cout << "ERROR: DEF must contain ROW"<< endl;
    exit(1);
  }

  string siteName(_ckt.defRowStor[0].macro());
  auto sitePtr = _ckt.lefSiteMap.find( siteName );
  if( sitePtr == _ckt.lefSiteMap.end()) {
    cout << "ERROR: SITE " << siteName << " not exists in LEF." << endl;
    exit(1);
  }
  
  double siteSizeX = _ckt.lefSiteStor[sitePtr->second].sizeX();
  double siteSizeY = _ckt.lefSiteStor[sitePtr->second].sizeY();

  CircuitInfo _cinfo( 1.0*points.x[0]/defScale, 
        1.0*points.y[0]/defScale,
        1.0*points.x[1]/defScale,
        1.0*points.y[1]/defScale,
        siteSizeX, siteSizeY );

  
  cout << endl;
  cout << "Layout Information" << endl;
  for(int i=0; i<points.numPoints; i++) {
    cout << 1.0*points.x[i]/defScale << " ";
    cout << 1.0*points.y[i]/defScale << endl;
  }

//  cout << "Original Macro List" << endl;

  MacroCircuit _mckt(_ckt, _env, _cinfo);
//  _mckt.GetMacroStor(_ckt);

//  for(auto& curMacro: _mckt.macroStor) {
//    curMacro.Dump();
//  }
 
  bool isHorizontal = true;

  Partition layout(ALL, _cinfo.lx, _cinfo.ly, _cinfo.ux-_cinfo.lx, _cinfo.uy-_cinfo.ly);
  layout.macroStor = _mckt.macroStor;
  
  // Legalize macro on global Structure
  double snapGrid = 0.2f;
  _mckt.StubPlacer(snapGrid);


  /*
  TwoPartitions oneLevelPart = GetPart(_cinfo, layout, isHorizontal);
  TwoPartitions eastStor, westStor;

  vector< vector<Partition> > allSets;


  // Fill the MacroNetlist for ALL circuits
  unordered_map< PartClass, vector<int>, 
    MyHash<PartClass>> globalMacroPartMap;
  UpdateMacroPartInfo( _mckt, layout, globalMacroPartMap );
  layout.FillNetlistTable( _mckt, globalMacroPartMap );
 
  // push to the outer vector 
  vector<Partition> layoutSet;
  layoutSet.push_back(layout);

  // push
  allSets.push_back(layoutSet);

  for(auto& curSet : oneLevelPart ) {
    if( isHorizontal ) {
      CircuitInfo eastInfo(_cinfo, curSet.first);
//      curSet.first.PrintSetFormat(stdout);
      TwoPartitions eastStor = GetPart(eastInfo, curSet.first, !isHorizontal);

      CircuitInfo westInfo(_cinfo, curSet.second);
      TwoPartitions westStor = GetPart(westInfo, curSet.second, !isHorizontal);
//      curSet.second.PrintSetFormat(stdout);
      
//      for(auto& curEastSet : eastStor) {
//        curEastSet.first.PrintSetFormat(stdout);
//        curEastSet.second.PrintSetFormat(stdout);txt
//      } 

//      for(auto& curWestSet : westStor) {
//        curWestSet.first.PrintSetFormat(stdout);
//        curWestSet.second.PrintSetFormat(stdout);
//      }

      // for all possible combinations in partitions
      for(int i=0; i<eastStor.size(); i++) {
        for(int j=0; j<westStor.size(); j++) {

          vector<Partition> oneSet;

          // one set is composed of four subblocks
          oneSet.push_back( eastStor[i].first );
          oneSet.push_back( eastStor[i].second );
          oneSet.push_back( westStor[j].first );
          oneSet.push_back( westStor[j].second );
      
          // Fill Macro Netlist
          // update macroPartMap
          unordered_map< PartClass, vector<int>, 
            MyHash<PartClass>> macroPartMap;
//          for(auto& curSet: oneSet) {
//            UpdateMacroPartInfo( _mckt, curSet, macroPartMap );
//          }

          for(auto& curSet: oneSet) {
            curSet.FillNetlistTable( _mckt, macroPartMap );
          }
           
          allSets.push_back( oneSet );
        }
      } 
    }
  }
  cout << "Total Extracted Sets: " << allSets.size() << endl << endl;


//  srand(_env.seed);

  // TopLevel Macro Location Update
  // For each possible full-layout
  for(auto& curSet: allSets) {

    // For each partitions (four partition)
    for(auto& curPart : curSet) {
      // Annealing
      curPart.DoAnneal();
      // Update _mckt frequently
      _mckt.UpdateMacro(curPart);
    }
      
    // StubPlacer for Macro cells

    // update partitons' macro info
    for(auto& curPart : curSet) { 
      for(auto& curMacro : curPart.macroStor) {
        auto mPtr = _mckt.macroNameMap.find(curMacro.name);
        int macroIdx = mPtr->second;
        curMacro.lx = _mckt.macroStor[macroIdx].lx;
        curMacro.ly = _mckt.macroStor[macroIdx].ly;
      }
    }

    break;
  }
  
  */
  // update _ckt structure
  for(auto& curMacro : _mckt.macroStor) {
    auto cPtr = _ckt.defComponentMap.find( curMacro.name );
    if( cPtr == _ckt.defComponentMap.end()) {
      cout << "ERROR: Cannot find " << curMacro.name << " in defiComponentMap" << endl;
      exit(1);
    }
    int cIdx = cPtr->second;
    int lx = int(curMacro.lx * defScale + 0.5f);
    int ly = int(curMacro.ly * defScale + 0.5f);

    _ckt.defComponentStor[cIdx].setPlacementLocation(lx, ly);
  }

  // top-level layout print
  FILE* fp = fopen(_env.output.c_str(), "w"); 
  if( fp == NULL) { 
    cout << "ERROR: cannot open " << _env.output << " to write output file" << endl;
    exit(1);
  }
  _ckt.WriteDef( fp );
  fclose(fp);

  /*
  // Writing functions for Sets file
  FILE* fp = fopen(_env.output.c_str(), "w");
  if( fp == NULL) { 
    cout << "ERROR: cannot open " << _env.output << " to write output file" << endl;
    exit(1);
  } 

  // Generate *.sets file
  PrintAllSets(fp, _cinfo, allSets);
  fclose(fp);
  */



  // ParquetFormat print
  // only top-level formats
//  layout.PrintParquetFormat(_env.output);

  cout << "Finished" << endl;


  // All Edges... 
//  for(int i=1; i<=numEdge; i++) {
//    sta::Edge* edge = _sta->graph()->edge(i);
//    cout << edge->from(_sta->graph())->name(_sta->network()) << 
//      " -> " << edge->to(_sta->graph())->name(_sta->network()) << endl;
//  }


//  PinSeq pinStor;
//  pinStor.push_back( );

//  for(auto& curSet : allSets) {
//    for(auto& curPart : curSet) {
//      for(auto& curMacro : curPart.macroStor) {
//        cout << curMacro.name << endl;
//        Pin* curPin = _sta->network()->findPin(curMacro.name.c_str()); 
//      }
//    }
//  } 
  
  /* 
  // vertex 
  sta::VertexIterator vertIter(_sta->graph());
  while( vertIter.hasNext()) {
    sta::Vertex* vertex = vertIter.next();
//    if( vertex->isRoot() ) {
//     if ( vertex->isCheckClk() || vertex->isGatedClkEnable() || vertex->isRegClk() ) {
//    if( _sta->network()->isTopLevelPort(vertex->pin())) {
//      cout << vertex->name(_sta->network()) << endl;
//    }
  }

  for(int i=1; i<=numEdge; i++) {
    sta::Edge* edge = _sta->graph()->edge(i);
//    cout << edge->from(_sta->graph())->name(_sta->network()) << 
//      " -> " << edge->to(_sta->graph())->name(_sta->network()) << endl;
  }

  LeafInstanceIterator* leafInstanceIter = _sta->network()->leafInstanceIterator();
  while( leafInstanceIter->hasNext() ) {
    Instance* instance = leafInstanceIter->next();
//    cout << _sta->network()->name(instance) << "/";
    InstancePinIterator* instancePinIter = _sta->network()->pinIterator(instance);
    while( instancePinIter->hasNext()) {
      sta::Pin* pin = instancePinIter->next();
      if( _sta->network()->isDriver(pin) ) {
        cout << "D: ";
      }
      else if( _sta->network()->isLoad(pin) ) {
        cout << "L: ";
      }
      cout << _sta->network()->name(pin) << endl ;
    }
    cout <<endl;
  }
  */
  
//  cout << "top: " << _sta->network()->name(_sta->network()->topInstance()) << endl;
  return 0;
}

bool ParseArgv(int argc, char** argv, EnvFile& _env) {
  for(int i=1; i<argc; i++) {
    if (STRING_EQUAL("-lef", argv[i])){
      i++;
      _env.lefStor.push_back(argv[i]);
    }
    else if (STRING_EQUAL("-def", argv[i])){
      i++;
      _env.def = string(argv[i]);
    }
    else if (STRING_EQUAL("-output", argv[i])){
      i++;
      _env.output = string(argv[i]);
    }
  }

  return _env.IsFilled();
}

void PrintUsage() {
  cout << "usage: ./stubPlacer -lef tech.lef -lef ... -def design.def -output output.def" << endl;
}


void CutRoundUp( CircuitInfo& cInfo, double& cutLine, bool isHorizontal ) {
  if( isHorizontal ) {
    int integer = (1.0* cutLine / cInfo.siteSizeX) + 0.5f;
    cutLine = integer * cInfo.siteSizeX;
    cutLine = (cutLine > cInfo.ux)? cInfo.ux : cutLine;
  }
  else {
    int integer = (1.0* cutLine / cInfo.siteSizeY) + 0.5f;
    cutLine = integer * cInfo.siteSizeY;
    cutLine = (cutLine > cInfo.uy)? cInfo.uy : cutLine;
  }
}

// Two partitioning functions:
// first : lower part
// second : upper part
// 
// cutLine is sweeping from lower to upper coordinates in x / y
vector<pair<Partition, Partition>> GetPart(
    CircuitInfo &cInfo,  
    Partition& partition, 
    bool isHorizontal ) {

  // Return vector
  vector<pair<Partition, Partition>> ret;
  
  typedef std::set<int> MacroSetT;
  interval_map<double, MacroSetT> macroMap;

  double maxWidth = DBL_MIN;
  double maxHeight = DBL_MIN;

  // in parent partition, traverse macros
  //
  for(auto& curMacro: partition.macroStor) {
    MacroSetT macroInfo;
    macroInfo.insert( &curMacro - &partition.macroStor[0]);
    
    macroMap.add( 
        make_pair(
          interval<double>::closed(
            (isHorizontal)? curMacro.lx : curMacro.ly,
            (isHorizontal)? curMacro.lx + curMacro.w : 
                curMacro.ly + curMacro.h ), macroInfo));
    maxWidth = ( maxWidth < curMacro.w)? curMacro.w: maxWidth;
    maxHeight = ( maxHeight < curMacro.h)? curMacro.h: maxHeight;
  }

  double cutLineLimit = (isHorizontal)? maxWidth * 0.25 : maxHeight * 0.25;


  double prevPushLimit = DBL_MIN;
  bool isFirst = true;
  vector<double> cutLineStor;
  for( interval_map<double, MacroSetT>::iterator 
      it = macroMap.begin();
      it != macroMap.end(); *it++) {
    interval<double>::type macroInterval = it->first;
    MacroSetT result = it->second;

    if( isFirst ) {
      cutLineStor.push_back( macroInterval.lower() );
      prevPushLimit = macroInterval.lower();
      isFirst = false;
    }
    else if( abs(macroInterval.lower()-prevPushLimit) > cutLineLimit ) {
      cutLineStor.push_back( macroInterval.lower() );
      prevPushLimit = macroInterval.lower();
    }
  }
  
  // Macro checker array
  // 0 for uninitialize
  // 1 for lower
  // 2 for upper
  // 3 for both
  int* chkArr = new int[partition.macroStor.size()];
  
  for(auto& cutLine : cutLineStor ) {
    cout << cutLine << endl;
    CutRoundUp(cInfo, cutLine, isHorizontal);
//    cout << "updated cutLine: " << cutLine << endl;

   
    // chkArr initialize 
    for(int i=0; i<partition.macroStor.size(); i++) {
      chkArr[i] = 0;
    }
  
    bool isImpossible = false;
    for(auto& curMacro : partition.macroStor) {
      int i = &curMacro - &partition.macroStor[0];
      if( isHorizontal ) {
        // lower is possible
        if( curMacro.w <= cutLine ) {
          chkArr[i] += 1;
        }
        // upper is possible
        if ( curMacro.w <= partition.width - cutLine) {
          chkArr[i] += 2; 
        }
        // none of them
        if( chkArr[i] == 0 ) {
          isImpossible = true;
          break;
        }
      }
      else {
        // lower is possible
        if( curMacro.h <= cutLine ) {
          chkArr[i] += 1;
        }
        // upper is possible
        if (curMacro.h <= partition.height - cutLine) {
          chkArr[i] += 2;
        }
        // none of 
        if( chkArr[i] == 0 ) {
          isImpossible = true;
          break;
        }
      }
    } 
    // impossible cuts, then skip 
    if( isImpossible ) {
      continue;
    }

    // Fill in the Partitioning information
    PartClass lClass, uClass;
    if( partition.partClass == ALL ) {
      lClass = (isHorizontal)? W : S;
      uClass = (isHorizontal)? E : N;
    }

    if( partition.partClass == W) {
      lClass = SW;
      uClass = NW;
    
    }
    if( partition.partClass == E) {
      lClass = SE;
      uClass = NE;
    }

    if( partition.partClass == N) {
      lClass = NW;
      uClass = NE;
    }

    if( partition.partClass == S) {
      lClass = SW;
      uClass = SE;
    }

    Partition lowerPart( lClass, 
      partition.lx, 
      partition.ly, 
      (isHorizontal)? cutLine - partition.lx : partition.width, 
      (isHorizontal)? partition.height : cutLine - partition.ly); 

    Partition upperPart( uClass, 
      (isHorizontal)? cutLine : partition.lx, 
      (isHorizontal)? partition.ly :cutLine,
      (isHorizontal)? partition.lx + partition.width - cutLine : partition.width, 
      (isHorizontal)? partition.height : partition.ly + partition.height - cutLine);



    // cout << it->first << " " << it->second << endl;
    //
    // Fill in child partitons' macroStor
    for(auto& curMacro : partition.macroStor) {
      int i=&curMacro - &partition.macroStor[0];
      if( chkArr[i] == 1 ) {
        lowerPart.macroStor.push_back( 
            Macro( curMacro.name, curMacro.type,
              curMacro.lx, curMacro.ly,
              curMacro.w, curMacro.h,
              curMacro.ptr, curMacro.instPtr )) ; 
      }
      else if( chkArr[i] == 2 ) {
        upperPart.macroStor.push_back(
            Macro( curMacro.name, curMacro.type,
              (isHorizontal)? curMacro.lx-cutLine : curMacro.lx, 
              (isHorizontal)? curMacro.ly : curMacro.ly-cutLine,
              curMacro.w, curMacro.h,
              curMacro.ptr, curMacro.instPtr));
      }
      else if( chkArr[i] == 3 ) {
        double centerPoint = 
          (isHorizontal)? 
          curMacro.lx + curMacro.w/2.0 :
          curMacro.ly + curMacro.h/2.0;

        if( centerPoint < cutLine ) {
          lowerPart.macroStor.push_back( 
              Macro( curMacro.name, curMacro.type,
                curMacro.lx, curMacro.ly,
                curMacro.w, curMacro.h,
                curMacro.ptr, curMacro.instPtr )) ; 
        
        }
        else {
          upperPart.macroStor.push_back(
              Macro( curMacro.name, curMacro.type,
                (isHorizontal)? curMacro.lx-cutLine : curMacro.lx, 
                (isHorizontal)? curMacro.ly : curMacro.ly-cutLine,
                curMacro.w, curMacro.h,
                curMacro.ptr, curMacro.instPtr));
        }
      }
    }
    
    double lowerArea = lowerPart.width * lowerPart.height;
    double upperArea = upperPart.width * upperPart.height;

    double upperMacroArea = 0.0f;
    double lowerMacroArea = 0.0f;

    for(auto& curMacro : upperPart.macroStor) {
      upperMacroArea += curMacro.w * curMacro.h;
    } 
    for(auto& curMacro : lowerPart.macroStor) {
      lowerMacroArea += curMacro.w * curMacro.h;
    }
//    cout << upperMacroArea << " " << upperArea << endl;
//    cout << lowerMacroArea << " " << lowerArea << endl;
    // impossible partitioning
    if( upperMacroArea > upperArea || lowerMacroArea > lowerArea) {
      cout << "it was impossible partition" << endl;
      continue;
    }

// Previous code
//    if( partition.partClass == N || 
//        partition.partClass == E ||
//        partition.partClass == W ||
//        partition.partClass == S ) {
//      lowerPart.FillNetlistTable();
//      upperPart.FillNetlistTable();
//    }
    
    pair<Partition, Partition> curPart( lowerPart, upperPart );
    ret.push_back( curPart );
//    cout << endl;
  }
  delete[] chkArr; 
  
  return ret; 
}

void Partition::PrintSetFormat(FILE* fp) {
  string sliceStr = "";
  if( partClass == ALL ) {
    sliceStr = "ALL";
  }
  else if( partClass == NW ) {
    sliceStr = "NW";
  }
  else if( partClass == NE ) {
    sliceStr = "NE";
  }
  else if (partClass == SW) {
    sliceStr = "SW";
  }
  else if (partClass == SE) {
    sliceStr = "SE";
  }
  else if (partClass == E) {
    sliceStr = "E";
  }
  else if (partClass == W) {
    sliceStr = "W";
  }
  else if (partClass == S) {
    sliceStr = "S";
  }
  else if (partClass == N) {
    sliceStr = "N";
  }

  fprintf(fp,"  BEGIN SLICE %s %d ;\n", sliceStr.c_str(), macroStor.size() );
  fprintf(fp,"    LX %f ;\n", lx);
  fprintf(fp,"    LY %f ;\n", ly);
  fprintf(fp,"    WIDTH %f ;\n", width);
  fprintf(fp,"    HEIGHT %f ;\n", height);
  for(auto& curMacro : macroStor) {
    fprintf(fp,"    MACRO %s %s %f %f %f %f ;\n", 
        curMacro.name.c_str(), curMacro.type.c_str(), 
        curMacro.lx, curMacro.ly, curMacro.w, curMacro.h);
  }

  if( netTable ) {
    fprintf(fp, "    NETLISTTABLE \n    ");
    for(int i=0; i<macroStor.size()+4; i++) {
      for(int j=0; j<macroStor.size()+4; j++) {
        fprintf(fp, "%.3f ", netTable[(macroStor.size()+4)*i + j]);
      }
      if( i == macroStor.size()+3 ) {
        fprintf(fp, "; \n");
      }
      else {
        fprintf(fp, "\n    ");
      }
    }
  }
  else {
    fprintf(fp,"    NETLISTTABLE ;\n");
  }
  
  fprintf(fp,"  END SLICE ;\n");
  fflush(fp);
}

//
void PrintAllSets(FILE* fp, CircuitInfo& cInfo, 
    vector< vector<Partition> >& allSets) {
  cout << "Writing sets file... ";
  for(auto& curSet : allSets) {
    fprintf(fp,"BEGIN SET ;\n" );
    fprintf(fp,"  WIDTH %f ;\n", cInfo.ux - cInfo.lx );
    fprintf(fp,"  HEIGHT %f ;\n", cInfo.uy - cInfo.ly );
    for(auto& curSlice : curSet) {
      if( curSlice.macroStor.size() == 0) {
        continue;
      }
      curSlice.PrintSetFormat(fp);
    }
    fprintf(fp,"END SET ;\n");
  }
  fflush(fp);
  cout << "Done!" << endl;
}

#define EAST_IDX (macroStor.size())
#define WEST_IDX (macroStor.size()+1)
#define NORTH_IDX (macroStor.size()+2)
#define SOUTH_IDX (macroStor.size()+3)

#define GLOBAL_EAST_IDX (_mckt.macroStor.size())
#define GLOBAL_WEST_IDX (_mckt.macroStor.size()+1)
#define GLOBAL_NORTH_IDX (_mckt.macroStor.size()+2)
#define GLOBAL_SOUTH_IDX (_mckt.macroStor.size()+3)

void Partition::PrintParquetFormat(string origName){
  string blkName = origName + ".blocks";
  string netName = origName + ".nets";
  string wtsName = origName + ".wts"; 
  string plName = origName + ".pl";
  
  // For *.nets and *.wts writing
  vector< pair<int, int> > netStor;
  vector<int> costStor;

  netStor.reserve( (macroStor.size()+4)*(macroStor.size()+3)/2 );
  costStor.reserve( (macroStor.size()+4)*(macroStor.size()+3)/2 );
  for(int i=0; i<macroStor.size()+4; i++) {
    for(int j=i+1; j<macroStor.size()+4; j++) {
      int cost = netTable[ i*(macroStor.size()+4) + j] + 
        netTable[ j*(macroStor.size()+4) + i ];
      if( cost != 0 ) {
        netStor.push_back( std::make_pair( std::min(i,j), std::max(i,j) ) );
        costStor.push_back(cost);
      }
    }
  }
  
  WriteBlkFile( blkName );
  WriteNetFile( netStor, netName );
  WriteWtsFile( costStor, wtsName );
  WritePlFile( plName );
  
}

void Partition::WriteBlkFile( string blkName ) {
  std::ofstream blkFile(blkName);
  if( !blkFile.good() ) {
    cout << "** ERROR: Cannot Open BlkFile to write : " << blkName << endl;
    exit(1);
  }

  std::stringstream feed;
  feed << "UCSC blocks 1.0" << endl;
  feed << "# Created" << endl;
  feed << "# User" << endl;
  feed << "# Platform" << endl << endl;

  feed << "NumSoftRectangularBlocks : 0" << endl;
  feed << "NumHardRectilinearBlocks : " << macroStor.size() << endl;
  feed << "NumTerminals : 4" << endl << endl;

  for(auto& curMacro : macroStor) {
    feed << curMacro.name << " hardrectilinear 4 ";
    feed << "(" << curMacro.lx << ", " << curMacro.ly << ") ";
    feed << "(" << curMacro.lx << ", " << curMacro.ly + curMacro.h << ") ";
    feed << "(" << curMacro.lx + curMacro.w << ", " << curMacro.ly + curMacro.h << ") ";
    feed << "(" << curMacro.lx + curMacro.w << ", " << curMacro.ly << ") " << endl;
  }

  feed << endl;

  feed << "West terminal" << endl;
  feed << "East terminal" << endl;
  feed << "North terminal" << endl;
  feed << "South terminal" << endl;
  
  blkFile << feed.str();
  blkFile.close();
  feed.clear();  
}

string Partition::GetName(int macroIdx ) {
  if( macroIdx < macroStor.size()) {
      return macroStor[macroIdx].name;
  }
  else {
    if( macroIdx == EAST_IDX ) {
      return "East"; 
    }
    else if( macroIdx == WEST_IDX) {
      return "West";
    }
    else if( macroIdx == NORTH_IDX) {
      return "North";
    }
    else if( macroIdx == SOUTH_IDX) { 
      return "South";
    }
    else {
      return "None";
    }
  }
}

void Partition::WriteNetFile( vector< pair<int, int> >& netStor, string netName ) {
  std::ofstream netFile(netName);
  if( !netFile.good() ) {
    cout << "** ERROR: Cannot Open NetFile to write : " << netName << endl;
    exit(1);
  }

  std::stringstream feed;
  feed << "UCLA nets 1.0" << endl;
  feed << "# Created" << endl;
  feed << "# User" << endl;
  feed << "# Platform" << endl << endl;


  feed << "NumNets : " << netStor.size() << endl;
  feed << "NumPins : " << 2 * netStor.size() << endl;

  for(auto& curNet : netStor) { 
    int idx = &curNet - &netStor[0];
    feed << "NetDegree : 2  n" << std::to_string(idx) << endl;
    feed << GetName( curNet.first ) << " B : %0.0 %0.0" << endl;
    feed << GetName( curNet.second ) << " B : %0.0 %0.0" << endl;
  }

  feed << endl;

  netFile << feed.str();
  netFile.close();
  feed.clear();  

}

void Partition::WriteWtsFile( vector< int >& costStor, string wtsName ) {
  std::ofstream wtsFile(wtsName);
  if( !wtsFile.good() ) {
    cout << "** ERROR: Cannot Open WtsFile to write : " << wtsName << endl;
    exit(1);
  }

  std::stringstream feed;
  feed << "UCLA wts 1.0" << endl;
  feed << "# Created" << endl;
  feed << "# User" << endl;
  feed << "# Platform" << endl << endl;


  for(auto& curWts : costStor ) { 
    int idx = &curWts - &costStor[0];
    feed << "n" << std::to_string(idx) << " " << curWts << endl;
  }

  feed << endl;

  wtsFile << feed.str();
  wtsFile.close();
  feed.clear();  

}

void Partition::WritePlFile( string plName ) {
  std::ofstream plFile(plName);
  if( !plFile.good() ) {
    cout << "** ERROR: Cannot Open NetFile to write : " << plName << endl;
    exit(1);
  }

  std::stringstream feed;
  feed << "UCLA pl 1.0" << endl;
  feed << "# Created" << endl;
  feed << "# User" << endl;
  feed << "# Platform" << endl << endl;

  for(auto& curMacro : macroStor) {
    feed << curMacro.name << " 0 0" << endl;
  }

  feed << "East " << lx + width << " " << ly + height/2.0 << endl;
  feed << "West " << lx << " " << ly + height/2.0 << endl;
  feed << "North " << lx + width/2.0 << " " << ly + height << endl;
  feed << "South " << lx + width/2.0 << " " << ly << endl;

  feed << endl;

  plFile << feed.str();
  plFile.close();
  feed.clear();  
}

   


void Partition::FillNetlistTable(MacroCircuit& _mckt, 
    unordered_map<PartClass, vector<int>, MyHash<PartClass>>& macroPartMap ) {
  tableCnt = (macroStor.size()+4)*(macroStor.size()+4);
  netTable = new double[tableCnt];
  for(int i=0; i<tableCnt; i++) {
    netTable[i] = 0.0f;
  }
  // FillNetlistTableIncr();
  // FillNetlistTableDesc();
  
  auto mpPtr = macroPartMap.find( partClass );
  if( mpPtr == macroPartMap.end()) {
    cout << "ERROR: Partition: " << partClass << " not exists MacroCell (macroPartMap)" << endl;
    exit(1);
  }

  // Just Copy to the netlistTable.
  if( partClass == ALL ) {
    for(int i=0; i< (macroStor.size()+4); i++) {
      for(int j=0; j< macroStor.size()+4; j++) {
        netTable[ i*(macroStor.size()+4)+j ] = (double)_mckt.macroWeight[i][j]; 
      }
    }
    return; 
  }

  // row
  for(int i=0; i<macroStor.size()+4; i++) {
    // column
    for(int j=0; j<macroStor.size()+4; j++) {
      if( i == j ) {
        continue;
      }

      // from: macro case
      if ( i < macroStor.size() ) {
        auto mPtr = _mckt.macroNameMap.find( macroStor[i].name );
        if( mPtr == _mckt.macroNameMap.end()) {
          cout << "ERROR on macroNameMap: " << endl;
          exit(1);
        }
        int globalIdx1 = mPtr->second;

        // to macro case
        if( j < macroStor.size() ) {
          auto mPtr = _mckt.macroNameMap.find( macroStor[j].name );
          if( mPtr == _mckt.macroNameMap.end()) {
            cout << "ERROR on macroNameMap: " << endl;
            exit(1);
          }
          int globalIdx2 = mPtr->second;
          netTable[ i*(macroStor.size()+4) + j ] = _mckt.macroWeight[globalIdx1][globalIdx2];
        }
        // to IO-west case
        else if( j == WEST_IDX ) {
          int westSum = _mckt.macroWeight[globalIdx1][GLOBAL_WEST_IDX];

          if( partClass == PartClass::NE ) {
            auto mpPtr = macroPartMap.find( PartClass::NW );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              westSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          if( partClass == PartClass::SE ) {
            auto mpPtr = macroPartMap.find( PartClass::SW );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              westSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          netTable[ i*(macroStor.size()+4) + j] = westSum;
        }
        else if (j == EAST_IDX ) {
          int eastSum = _mckt.macroWeight[globalIdx1][GLOBAL_EAST_IDX];

          if( partClass == PartClass::NW ) {
            auto mpPtr = macroPartMap.find( PartClass::NE );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              eastSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          if( partClass == PartClass::SW ) {
            auto mpPtr = macroPartMap.find( PartClass::SE );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              eastSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          netTable[ i*(macroStor.size()+4) + j] = eastSum ;
        }
        else if (j == NORTH_IDX ) { 
          int northSum = _mckt.macroWeight[globalIdx1][GLOBAL_NORTH_IDX];

          if( partClass == PartClass::SE ) {
            auto mpPtr = macroPartMap.find( PartClass::NE );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              northSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          if( partClass == PartClass::SW ) {
            auto mpPtr = macroPartMap.find( PartClass::NW );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              northSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          netTable[ i*(macroStor.size()+4) + j] = northSum ;
        }
        else if (j == SOUTH_IDX ) {
          int southSum = _mckt.macroWeight[globalIdx1][GLOBAL_SOUTH_IDX];

          if( partClass == PartClass::NE ) {
            auto mpPtr = macroPartMap.find( PartClass::SE );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              southSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          if( partClass == PartClass::NW ) {
            auto mpPtr = macroPartMap.find( PartClass::SW );
            for(auto& curMacroIdx : mpPtr->second) {
              int curGlobalIdx = _mckt.macroNameMap[_mckt.macroStor[curMacroIdx].name];
              southSum += _mckt.macroWeight[globalIdx1][curGlobalIdx]; 
            }
          }
          netTable[ i*(macroStor.size()+4) + j] = southSum;
        }
      }
      //from IO
      else if( i == WEST_IDX ){
        // to Macro
        if( j < macroStor.size() ) {
          auto mPtr = _mckt.macroNameMap.find( macroStor[j].name );
          if( mPtr == _mckt.macroNameMap.end()) {
            cout << "ERROR on macroNameMap: " << endl;
            exit(1);
          }
          int globalIdx2 = mPtr->second;
          netTable[ i*(macroStor.size()+4) + j] = 
            _mckt.macroWeight[GLOBAL_WEST_IDX][globalIdx2];
        }
      }
      else if( i == EAST_IDX) {
        // to Macro
        if( j < macroStor.size() ) {
          auto mPtr = _mckt.macroNameMap.find( macroStor[j].name );
          if( mPtr == _mckt.macroNameMap.end()) {
            cout << "ERROR on macroNameMap: " << endl;
            exit(1);
          }
          int globalIdx2 = mPtr->second;
          netTable[ i*(macroStor.size()+4) + j] = 
            _mckt.macroWeight[GLOBAL_EAST_IDX][globalIdx2];
        }
      }
      else if(i == NORTH_IDX) {
        // to Macro
        if( j < macroStor.size() ) {
          auto mPtr = _mckt.macroNameMap.find( macroStor[j].name );
          if( mPtr == _mckt.macroNameMap.end()) {
            cout << "ERROR on macroNameMap: " << endl;
            exit(1);
          }
          int globalIdx2 = mPtr->second;
          netTable[ i*(macroStor.size()+4) + j] = 
            _mckt.macroWeight[GLOBAL_NORTH_IDX][globalIdx2];
        }
      }
      else if(i == SOUTH_IDX ) {
        // to Macro
        if( j < macroStor.size() ) {
          auto mPtr = _mckt.macroNameMap.find( macroStor[j].name );
          if( mPtr == _mckt.macroNameMap.end()) {
            cout << "ERROR on macroNameMap: " << endl;
            exit(1);
          }
          int globalIdx2 = mPtr->second;
          netTable[ i*(macroStor.size()+4) + j] = 
            _mckt.macroWeight[GLOBAL_SOUTH_IDX][globalIdx2];
        }
      }
    }
  }
}

void Partition::FillNetlistTableIncr() {
  //      if( macroStor.size() <= 1 ) {
  //        return;
  //      }

  for(int i=0; i<macroStor.size()+4; i++) {
    for(int j=0; j<macroStor.size()+4; j++) {
      double val = (i + j + 1)* 100;
      if( i == j || 
          i >= macroStor.size() && j >= macroStor.size() ) {
        val = 0;
      }
      netTable[ i*(macroStor.size()+4) + j] = val;
    }
  } 
}
    
void Partition::FillNetlistTableDesc() {
  //      if( macroStor.size() <= 1 ) {
  //        return;
  //      }

  for(int i=0; i<macroStor.size()+4; i++) {
    for(int j=0; j<macroStor.size()+4; j++) {
      double val = (2*macroStor.size()+8-(i+j)) * 100;
      if( i == j || 
          i >= macroStor.size() && j >= macroStor.size() ) {
        val = 0;
      }
      netTable[ i*(macroStor.size()+4) + j] = val;
    }
  } 
}

// Call ParquetFP
void Partition::DoAnneal() {}


void UpdateMacroPartInfo( 
    MacroCircuit& _mckt,
    MacroNetlist::Partition& part, 
    unordered_map<MacroNetlist::PartClass, vector<int>, 
    MyHash<MacroNetlist::PartClass>>& macroPartMap ) {

  auto mpPtr = macroPartMap.find( part.partClass );
  if( mpPtr == macroPartMap.end() ) {
    vector<int> curMacroStor;
    // convert macro Information into macroIdx
    for(auto& curMacro: part.macroStor) {
      auto miPtr = _mckt.macroInstMap.find( curMacro.instPtr );
      if( miPtr == _mckt.macroInstMap.end() ) {
        cout << "ERROR: macro " << curMacro.name << " not exists in macroInstMap: " << curMacro.instPtr << endl;
        exit(1);
      }
      curMacroStor.push_back( miPtr->second) ;
    }
    macroPartMap[ part.partClass ] = curMacroStor; 
  }
  else {
    cout << "ERROR: Partition- " << part.partClass << " already updated (UpdateMacroPartInfo)" << endl; 
    exit(1);
  }
}
