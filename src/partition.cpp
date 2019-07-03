//#include "partition.h"
//#include "macro.h"
#include "circuit.h"
#include <iostream>
#include <climits>
#include <cfloat>
#include <fstream>

using std::cout;
using std::endl;

MACRO_NETLIST_NAMESPACE_OPEN

Partition::Partition() 
  : partClass(PartClass::None), 
  lx(FLT_MAX), ly(FLT_MAX), 
  width(FLT_MAX), height(FLT_MAX), 
  netTable(0), tableCnt(0) {}

Partition::Partition(
    PartClass _partClass, 
    double _lx, double _ly, 
    double _width, double _height ) :
  partClass(_partClass), lx(_lx), ly(_ly), 
  width(_width), height(_height), 
  netTable(0), tableCnt(0) {}
    
Partition::~Partition(){ 
  if( netTable) { 
    delete [] netTable; 
    netTable=0; 
    tableCnt=0;
  } 
}

    
Partition::Partition(const Partition& prev)   
  : partClass(prev.partClass), 
  lx(prev.lx), ly(prev.ly),
  width(prev.width), height(prev.height),
  macroStor(prev.macroStor), 
  tableCnt(prev.tableCnt), 
  macroMap(prev.macroMap) {
    if( prev.netTable ) {
      netTable = new double[tableCnt];
      for(int i=0; i<tableCnt; i++) {
        netTable[i] = prev.netTable[i];
      }
    }
    else {
      netTable = 0;
    }
  }


Partition& Partition::operator= (const Partition& prev) {
  partClass = prev.partClass;
  lx = prev.lx;
  ly = prev.ly;
  width = prev.width;
  height = prev.height;
  macroStor = prev.macroStor;
  tableCnt = prev.tableCnt;
  macroMap = prev.macroMap; 

  if( prev.netTable ) {
    netTable = new double[tableCnt];
    for(int i=0; i<tableCnt; i++) {
      netTable[i] = prev.netTable[i];
    }
  }
  else {
    netTable = 0;
  }
}

void Partition::Dump() {    
  cout << "partClass: " << partClass << endl;
  cout << lx << " " << ly << " " << width << " " << height << endl;
  for(auto& curMacro : macroStor) {
    curMacro.Dump();
  }
  cout << endl;
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

MACRO_NETLIST_NAMESPACE_CLOSE