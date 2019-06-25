#ifndef __PARSE__
#define __PARSE__ 0

#define MACRO_NETLIST_NAMESPACE_OPEN namespace MacroNetlist {
#define MACRO_NETLIST_NAMESPACE_CLOSE }

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include "lefdefIO.h"
//#include "timingSta.h"

#include <Eigen/Core>
#include <Eigen/SparseCore>

using Eigen::VectorXf;
typedef Eigen::SparseMatrix<int, Eigen::RowMajor> SMatrix;
typedef Eigen::Triplet<int> T;


using std::unordered_map;
using std::unordered_set;

class EnvFile {
  public:
  std::string def;
  std::string verilog;
  std::string design;
  std::string output;
  std::string sdc; 
  std::vector<std::string> lefStor;
  std::vector<std::string> libStor;
  int searchDepth;
  EnvFile() : def(""), verilog(""), design(""), output(""), sdc(""), searchDepth(INT_MIN) {};

  bool IsFilled() { return def != "" && 
      output != "" &&  
      lefStor.size() != 0; };

  void Print() {
    using std::cout;
    using std::endl;
    
    cout << "Design: " << design << endl << endl;
    cout << "Liberty: " << endl;
    for(auto& curLib: libStor) {
      cout << curLib << endl;
    }
    cout << endl;
    cout << "Verilog: " << endl;
    cout << verilog << endl << endl;
    
    cout << endl;
    cout << "Sdc: " << endl;
    cout << sdc << endl << endl;

    cout << "Lef: " << endl;
    for(auto& curLef: lefStor) {
      cout << curLef << endl; 
    }
    cout << endl;
    cout << "Def: " << endl;
    cout << def << endl << endl;
    
    cout << "Output: " << endl;
    cout << output << endl << endl;
    
    cout << "Search Depth: " << endl;
    cout << searchDepth << endl << endl;
  }

};

namespace sta { 
class Sta;
class Instance;
class Pin; 
}
class MacroCircuit;
template <class T> struct MyHash;


MACRO_NETLIST_NAMESPACE_OPEN

class Vertex;

class Macro {
  public:
    string name;
    string type;  
    double lx, ly;
    double w, h;
    Vertex* ptr;
    sta::Instance* instPtr;
    Macro( string _name, string _type, double _lx, double _ly, double _w, double _h,
        Vertex* _ptr, sta::Instance* _instPtr) 
      : name(_name), type(_type), lx(_lx), ly(_ly), w(_w), h(_h), ptr(_ptr), instPtr(_instPtr) {};
    void Dump() {
//      cout << "name: " << name << endl;
//      cout << "type: " << type << endl;
//      cout << "(" << lx << ", " << ly << ") - (" << lx + w << ", " << ly + h << ")" << endl << endl;
//      cout << "MACRO " << name << " " << type << " " << lx - 363.32550 << " " << ly - 96.000 << " " << w << " " << h << endl;
      cout << "MACRO " << name << " " << type << " " << lx << " " << ly << " " << w << " " << h << endl;
    }
};

class Vertex;
class Edge {
  public:
    Vertex* from;
    Vertex* to;
    int weight;
    Edge(): from(0), to(0), weight(0) {}; 
    Edge(Vertex* _from, Vertex* _to, int _weight): 
      from(_from), to(_to), weight(_weight) {}; 
};

enum VertexClass {
  pin, instMacro, instOther, nonInit
};

class PinGroup;
class Vertex {
  public: 
    VertexClass vertexClass;
    vector<int> from;
    vector<int> to;

    // This can be either PinGroup / sta::Instance*,
    // based on vertexClass
    void* ptr;
      
    Vertex(): vertexClass(nonInit), ptr(0) {};
    Vertex(void* _ptr, VertexClass _vertexClass): 
      vertexClass(_vertexClass), ptr(_ptr) {};
};

enum PinGroupClass {
  West, East, North, South
};

class PinGroup {
  public:
    PinGroupClass pinGroupClass;
    vector<sta::Pin*> pins;
    string name() {
      if( pinGroupClass == PinGroupClass::West ) {
        return "West";
      }
      else if( pinGroupClass == PinGroupClass::East ) {
        return "East";
      } 
      else if( pinGroupClass == PinGroupClass::North) {
        return "North";
      } 
      else if( pinGroupClass == PinGroupClass::South) {
        return "South";
      } 
    }
};

enum PartClass {
  S, N, W, E, NW, NE, SW, SE, ALL, None
}; 

class Partition {
  public: 
    PartClass partClass;
    double lx, ly;
    double width, height;
    vector<Macro> macroStor;
    double* netTable;
    int tableCnt;
    unordered_map<string, int> macroMap;
    Partition() : 
      partClass(PartClass::None), lx(FLT_MAX), ly(FLT_MAX), 
      width(FLT_MAX), height(FLT_MAX), netTable(0), tableCnt(0) {};

    Partition(PartClass _partClass, double _lx, double _ly,
        double _width, double _height ) :
      partClass(_partClass), lx(_lx), ly(_ly), 
      width(_width), height(_height), netTable(0), tableCnt(0) {};

    // destructor
    ~Partition() { if( netTable) { delete [] netTable; netTable=0; tableCnt=0;} };

    // copy constructor
    Partition(const Partition& prev) :
      partClass(prev.partClass), lx(prev.lx), ly(prev.ly),
      width(prev.width), height(prev.height),
      macroStor(prev.macroStor), tableCnt(prev.tableCnt), macroMap(prev.macroMap) {
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

    // assign operator overloading
    Partition& operator= (const Partition& prev) {
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

    void FillNetlistTable(MacroCircuit& _mckt,
        unordered_map<PartClass, vector<int>, MyHash<PartClass>>& macroPartMap);

    void Dump() {
      cout << "partClass: " << partClass << endl;
      cout << lx << " " << ly << " " << width << " " << height << endl;
      for(auto& curMacro : macroStor) {
        curMacro.Dump();
      }
      cout << endl;
    }

    // Call Parquet to have annealing solution
    void DoAnneal();

    // Writing functions
    void PrintSetFormat(FILE* fp);

    // Parquet print functions
    void PrintParquetFormat(string origName);
    void WriteBlkFile(string blkName);
    void WriteNetFile(vector< pair<int, int>> & netStor, string netName);
    void WriteWtsFile(vector<int>& costStor, string wtsName );
    void WritePlFile(string plName);

    string GetName(int macroIdx);

  private:
    void FillNetlistTableIncr();
    void FillNetlistTableDesc();
}; 

MACRO_NETLIST_NAMESPACE_CLOSE


template<>
struct MyHash< std::pair<void*, void*> > {
  std::size_t operator()( const pair<void*, void*>& k ) const {
    using boost::hash_combine;
    size_t seed = 0;
    hash_combine(seed, k.first);
    hash_combine(seed, k.second);
    return seed; 
  }
};

template<>
struct MyHash< MacroNetlist::PartClass > {
  std::size_t operator()( const MacroNetlist::PartClass & k ) const {
    using boost::hash_combine;
    size_t seed = 0;
    hash_combine(seed, (int)k);
    return seed; 
  }
};

class CircuitInfo;
class MacroCircuit {
  public:
    MacroCircuit(Circuit::Circuit& ckt, EnvFile& env, CircuitInfo& cinfo);

    Circuit::Circuit& _ckt;
    EnvFile& _env;
    // Timer
    sta::Sta* _sta;
    // layout information
    CircuitInfo& _cinfo;
//    using MacroNetlist::Vertex;
//    using MacroNetlist::Edge;
//    using MacroNetlist::Macro;
    vector<MacroNetlist::Vertex> vertexStor;
    vector<MacroNetlist::Edge> edgeStor;
    
    // macro Information
    vector<MacroNetlist::Macro> macroStor;

    // pin Group Information
    vector<MacroNetlist::PinGroup> pinGroupStor;

    // pin Group Map;
    // Pin* --> pinGroupStor's index.
    unordered_map<sta::Pin*, int> pinGroupMap;

    // macro name -> macroStor's index. 
    unordered_map<string, int> macroNameMap; 

    // macro idx/idx pair -> give each 
    vector< vector<int> > macroWeight;

    string GetEdgeName(MacroNetlist::Edge* edge);
    string GetVertexName(MacroNetlist::Vertex* vertex);
    
    // sta::Instance* --> macroStor's index stor
    unordered_map<sta::Instance*, int> macroInstMap;

    // Update Macro Location from Partition info
    void UpdateMacro(MacroNetlist::Partition& part);

    // snapping Macros
    void StubPlacer(double snapGrid);

  private:
    void FillMacroStor();
    void FillPinGroup();
    void FillVertexEdge();
    void FillMacroConnection();
    
    void UpdateVertexToMacroStor();
    void UpdateInstanceToMacroStor();
//    unordered_map<MacroNetlist::Edge*, int> edgeMap;

    // either Pin*, Inst* -> vertexStor's index.
    unordered_map<void*, int> pinInstVertexMap;
    unordered_map<MacroNetlist::Vertex*, int> vertexPtrMap;

    SMatrix adjMatrix;
//    SMatrix adjMatrixTwo;
//    SMatrix adjMatrixFour;

    // pair of <StartVertex*, EndVertex*> --> edgeStor's index
    unordered_map< pair<MacroNetlist::Vertex*, MacroNetlist::Vertex*>, 
      int, MyHash<pair<void*, void*>> > vertexPairEdgeMap;
    
    pair<void*, MacroNetlist::VertexClass> GetPtrClassPair(sta::Pin* pin);
    MacroNetlist::Vertex* GetVertex(sta::Pin* pin); 

    int GetPathWeight( MacroNetlist::Vertex* from, MacroNetlist::Vertex* to, int limit );
    // Matrix version
    int GetPathWeightMatrix ( SMatrix& mat, MacroNetlist::Vertex* from, MacroNetlist::Vertex* to );
};

class CircuitInfo {
  public:
    double lx, ly, ux, uy;
    double siteSizeX, siteSizeY;
    CircuitInfo() : lx(FLT_MIN), ly(FLT_MIN), 
      ux(FLT_MIN), uy(FLT_MIN),
      siteSizeX(FLT_MIN), siteSizeY(FLT_MIN) {};

    CircuitInfo( double _lx, double _ly, double _ux, double _uy, 
        double _siteSizeX, double _siteSizeY ) :
      lx(_lx), ly(_ly), ux(_ux), uy(_uy),
      siteSizeX(_siteSizeX), siteSizeY(_siteSizeY) {};

    CircuitInfo( CircuitInfo& orig, MacroNetlist::Partition& part ) :
      lx(part.lx), ly(part.ly), ux(part.lx+part.width), uy(part.ly+part.height),
      siteSizeX(orig.siteSizeX), siteSizeY(orig.siteSizeY) {};
};


bool ParseArgv(int argc, char** argv, EnvFile& _env);
void PrintUsage();
//void GetMacroStor(Circuit::Circuit& _ckt, MacroNetlist::Circuit& _mckt);

// for string escape
inline bool ReplaceStringInPlace( std::string& subject, 
        const std::string& search,
        const std::string& replace) {
    size_t pos = 0;
    bool isFound = false;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
         isFound = true; 
    }
    return isFound; 
}

#endif
