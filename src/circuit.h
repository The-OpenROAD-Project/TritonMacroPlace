#ifndef __MACRO_CIRCUIT__
#define __MACRO_CIRCUIT__ 0

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Eigen/Core>
#include <Eigen/SparseCore>

using Eigen::VectorXf;
typedef Eigen::SparseMatrix<int, Eigen::RowMajor> SMatrix;
typedef Eigen::Triplet<int> T;

#include "parse.h"
#include "graph.h"
#include "partition.h"
#include "macro.h"
#include "lefdefIO.h"
//#include "timingSta.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;


namespace sta { 
class Sta;
}

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
    void UpdateMacroLoc(MacroNetlist::Partition& part);

    // snapping Macros
    void StubPlacer(double snapGrid);

    // parsing function
    void ParseGlobalConfig(string fileName);
    void ParseLocalConfig(string fileName);

    // initialize
    double gHaloX, gHaloY;
    double gChannelX, gChannelY;

    // save LocalCfg into this structure
    unordered_map< string, MacroLocalInfo > macroLocalMap;


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

