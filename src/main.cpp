#include "timingSta.h"
#include "parse.h"
#include "lefdefIO.h"
#include "partition.h"
#include <iostream>
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

//  for(auto& curMacro: macroStor) {
//    curMacro.Dump();
//  }
 
  bool isHorizontal = true;

  Partition layout(None, _cinfo.lx, _cinfo.ly, _cinfo.ux-_cinfo.lx, _cinfo.uy-_cinfo.ly);
  layout.macroStor = _mckt.macroStor;

  TwoPartitions oneLevelPart = GetPart(_cinfo, layout, isHorizontal);
  TwoPartitions eastStor, westStor;

  vector< vector<Partition> > allSets;
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

      for(int i=0; i<eastStor.size(); i++) {
        for(int j=0; j<westStor.size(); j++) {
          vector<Partition> oneSet;
          oneSet.push_back( eastStor[i].first );
          oneSet.push_back( eastStor[i].second );
          oneSet.push_back( westStor[j].first );
          oneSet.push_back( westStor[j].second );
          allSets.push_back( oneSet );
        }
      } 
    }
  }

  FILE* fp = fopen(_env.output.c_str(), "w");
  if( fp == NULL) { 
    cout << "ERROR: cannot open " << _env.output << " to write output file" << endl;
    exit(1);
  } 

  cout << "Total Extracted Sets: " << allSets.size() << endl << endl;
  PrintAllSets(fp, _cinfo, allSets);
  fclose(fp);

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
    if(STRING_EQUAL("-verilog", argv[i])){
      i++;
      _env.verilog = string(argv[i]);
    }
    else if (STRING_EQUAL("-lib", argv[i])){
      i++;
      _env.libStor.push_back(argv[i]);
    }
    else if (STRING_EQUAL("-lef", argv[i])){
      i++;
      _env.lefStor.push_back(argv[i]);
    }
    else if (STRING_EQUAL("-def", argv[i])){
      i++;
      _env.def = string(argv[i]);
    }
    else if (STRING_EQUAL("-sdc", argv[i])){
      i++;
      _env.sdc= string(argv[i]);
    }
    else if (STRING_EQUAL("-design", argv[i])){
      i++;
      _env.design= string(argv[i]);
    }
    else if (STRING_EQUAL("-output", argv[i])){
      i++;
      _env.output = string(argv[i]);
    }
  }

  return _env.IsFilled();
}

void PrintUsage() {
  cout << "usage: ./macro-netlist -lib one.lib -lib two.lib ... -verilog netlist.v" << endl;
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

vector<pair<Partition, Partition>> GetPart(
    CircuitInfo &cInfo,  
    Partition& partition, 
    bool isHorizontal ) {

  vector<pair<Partition, Partition>> ret;
//  vector<double> eventPoints;
  typedef std::set<int> MacroSetT;
  interval_map<double, MacroSetT> macroMap;

  for(auto& curMacro: partition.macroStor) {
    MacroSetT macroInfo;
    macroInfo.insert( &curMacro - &partition.macroStor[0]);
    
    macroMap.add( 
        make_pair(
          interval<double>::closed(
            (isHorizontal)? curMacro.lx : curMacro.ly,
            (isHorizontal)? curMacro.lx + curMacro.w : 
                curMacro.ly + curMacro.h ), macroInfo));
  }

  
  vector<double> cutLineStor;
  for( interval_map<double, MacroSetT>::iterator 
      it = macroMap.begin();
      it != macroMap.end(); *it++) {
    interval<double>::type macroInterval = it->first;
    MacroSetT result = it->second;

    cutLineStor.push_back( macroInterval.lower() );
  }
  
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
    // impossible cuts. 
    if( isImpossible ) {
      continue;
    }

    PartClass lClass, uClass;
    if( partition.partClass == None ) {
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
    for(auto& curMacro : partition.macroStor) {
      int i=&curMacro - &partition.macroStor[0];
      if( chkArr[i] == 1 ) {
        lowerPart.macroStor.push_back( 
            Macro( curMacro.name, curMacro.type,
              curMacro.lx, curMacro.ly,
              curMacro.w, curMacro.h )) ; 
      }
      else if( chkArr[i] == 2 ) {
        upperPart.macroStor.push_back(
            Macro( curMacro.name, curMacro.type,
              (isHorizontal)? curMacro.lx-cutLine : curMacro.lx, 
              (isHorizontal)? curMacro.ly : curMacro.ly-cutLine,
              curMacro.w, curMacro.h));
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
                curMacro.w, curMacro.h )) ; 
        
        }
        else {
          upperPart.macroStor.push_back(
              Macro( curMacro.name, curMacro.type,
                (isHorizontal)? curMacro.lx-cutLine : curMacro.lx, 
                (isHorizontal)? curMacro.ly : curMacro.ly-cutLine,
                curMacro.w, curMacro.h));
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

    if( partition.partClass == N || 
        partition.partClass == E ||
        partition.partClass == W ||
        partition.partClass == S ) {
      lowerPart.FillNetlistTable();
      upperPart.FillNetlistTable();
    }
    
    pair<Partition, Partition> curPart( lowerPart, upperPart );
    ret.push_back( curPart );
//    cout << endl;
  }
  delete[] chkArr; 
  
  return ret; 
}

void Partition::PrintSetFormat(FILE* fp) {
  string sliceStr = "";
  if( partClass == NW ) {
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
    fprintf(fp,"    MACRO %s %s %f %f %f %f ;\n", curMacro.name.c_str(), curMacro.type.c_str(), 
        curMacro.lx, curMacro.ly, curMacro.w, curMacro.h);
  }

  if( netTable ) {
    fprintf(fp, "    NETLISTTABLE \n    ");
    for(int i=0; i<macroStor.size()+4; i++) {
      for(int j=0; j<macroStor.size()+4; j++) {
        fprintf(fp, "%.3f ", netTable[(macroStor.size()+4)*i + j]);
      }
      if( i == macroStor.size()+3 ) {
        fprintf(fp, ";");
      }
      fprintf(fp, "\n    ");
    }
  }
  else {
    fprintf(fp,"    NETLISTTABLE ;\n");
  }
  
  fprintf(fp,"  END SLICE ;\n");
  fflush(fp);
}

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
