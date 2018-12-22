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
  EnvFile() : def(""), verilog(""), design(""), output(""), sdc("") {};
  bool IsFilled() { return design != "" && def != "" && verilog != "" && output != "" && sdc != "" && lefStor.size() != 0 && libStor.size() != 0; };
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
  }

};

namespace sta { 
class Sta;
class Instance;
class Pin; 
}


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
    Macro( string _name, string _type, double _lx, double _ly, double _w, double _h ) 
      : name(_name), type(_type), lx(_lx), ly(_ly), w(_w), h(_h), ptr(0), instPtr(0) {};
    void Dump() {
//      cout << "name: " << name << endl;
//      cout << "type: " << type << endl;
//      cout << "(" << lx << ", " << ly << ") - (" << lx + w << ", " << ly + h << ")" << endl << endl;
//      cout << "MACRO " << name << " " << type << " " << lx - 363.32550 << " " << ly - 96.000 << " " << w << " " << h << endl;
      cout << "MACRO " << name << " " << type << " " << lx << " " << ly << " " << w << " " << h << endl;
    }
};

enum IoClass{ 
  IoS, IoE, IoW, IoN
};

class Io {
  public: 
    IoClass ioClass;
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
    vector<Edge*> from;
    vector<Edge*> to;

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
};


enum PartClass {
  S, N, W, E, NW, NE, SW, SE, None
}; 

class Partition {
  public: 
    PartClass partClass;
    double lx, ly;
    double width, height;
    vector<Macro> macroStor;
    double* netTable;
    int tableCnt;
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
      macroStor(prev.macroStor), tableCnt(prev.tableCnt) {
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

    Partition& operator= (const Partition& prev) {
      partClass = prev.partClass;
      lx = prev.lx;
      ly = prev.ly;
      width = prev.width;
      height = prev.height;
      macroStor = prev.macroStor;
      tableCnt = prev.tableCnt;

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

    void FillNetlistTable() {
      tableCnt = (macroStor.size()+4)*(macroStor.size()+4);
      netTable = new double[tableCnt];
      for(int i=0; i<tableCnt; i++) {
        netTable[i] = 0.0f;
      }
//      FillNetlistTableIncr();
      FillNetlistTableDesc();
    }

    void Dump() {
      cout << "partClass: " << partClass << endl;
      cout << lx << " " << ly << " " << width << " " << height << endl;
      for(auto& curMacro : macroStor) {
        curMacro.Dump();
      }
      cout << endl;
    }
    void PrintSetFormat(FILE* fp);

  private:
    void FillNetlistTableIncr() {
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
    
    void FillNetlistTableDesc() {
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
}; 

MACRO_NETLIST_NAMESPACE_CLOSE

template <class T> struct MyHash;

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

typedef vector< pair<MacroNetlist::Vertex*, int> > VertexInfo;
class CircuitInfo;

class MacroCircuit {
  public:
    MacroCircuit(Circuit::Circuit& ckt, EnvFile& env, CircuitInfo& cinfo);

    Circuit::Circuit& _ckt;
    EnvFile& _env;
    sta::Sta* _sta;
    CircuitInfo& _cinfo;
//    using MacroNetlist::Vertex;
//    using MacroNetlist::Edge;
//    using MacroNetlist::Macro;
    // layout information
    double llx, lly, urx, ury;
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

    void SetEdgeName(MacroNetlist::Edge* edge, string& name);
    void SetVertexName(MacroNetlist::Vertex* vertex, string& name);
    void SetEdgeWeight(MacroNetlist::Edge* edge, double weight);
    void AddEdgeFromVertex(MacroNetlist::Edge* edge, MacroNetlist::Vertex* vertex);
    void AddEdgeToVertex(MacroNetlist::Edge* edge, MacroNetlist::Vertex* vertex);

  private:
    void FillMacroStor();
    void FillPinGroup();
    void FillVertexEdge();
    void FillMacroConnection();
    
    void UpdateVertexToMacroStor();
    void UpdateInstanceToMacroStor();
//    unordered_map<MacroNetlist::Edge*, int> edgeMap;

    // sta::Instance* --> macroStor's index
    unordered_map<sta::Instance*, int> macroInstMap;

    // either Pin*, Inst* -> vertexStor's index.
    unordered_map<void*, int> pinInstVertexMap;
    unordered_map<MacroNetlist::Vertex*, int> vertexPtrMap;

    // pair of <StartVertex*, EndVertex*> --> edgeStor's index
    unordered_map< pair<MacroNetlist::Vertex*, MacroNetlist::Vertex*>, 
      int, MyHash<pair<void*, void*>> > vertexPairEdgeMap;
    
    pair<void*, MacroNetlist::VertexClass> GetPtrClassPair(sta::Pin* pin);
    MacroNetlist::Vertex* GetVertex(sta::Pin* pin); 


//    VertexInfo GetAdjVertexList( VertexInfo candiVertex,
//       unordered_set<MacroNetlist::Vertex*> &vertexVisit );


  int GetPathWeight( MacroNetlist::Vertex* from, MacroNetlist::Vertex* to, int limit);

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
