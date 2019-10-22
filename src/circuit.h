#ifndef __MACRO_CIRCUIT__
#define __MACRO_CIRCUIT__ 

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Eigen/Core>
#include <Eigen/SparseCore>


#include "parse.h"
#include "graph.h"
#include "partition.h"
#include "macro.h"
#include "lefdefIO.h"


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
    
    std::vector<MacroNetlist::Vertex> vertexStor;
    std::vector<MacroNetlist::Edge> edgeStor;
    
    // macro Information
    std::vector<MacroNetlist::Macro> macroStor;

    // pin Group Information
    std::vector<MacroNetlist::PinGroup> pinGroupStor;

    // pin Group Map;
    // Pin* --> pinGroupStor's index.
    std::unordered_map<sta::Pin*, int> pinGroupMap;

    // macro name -> macroStor's index. 
    std::unordered_map<std::string, int> macroNameMap; 

    // macro idx/idx pair -> give each 
    std::vector< std::vector<int> > macroWeight;

    std::string GetEdgeName(MacroNetlist::Edge* edge);
    std::string GetVertexName(MacroNetlist::Vertex* vertex);
    
    // sta::Instance* --> macroStor's index stor
    std::unordered_map<sta::Instance*, int> macroInstMap;

    // Update Macro Location from Partition info
    void UpdateMacroLoc(MacroNetlist::Partition& part);

    // parsing function
    void ParseGlobalConfig(std::string fileName);
    void ParseLocalConfig(std::string fileName);

    // initialize
    double gHaloX, gHaloY;
    double gChannelX, gChannelY;

    // save LocalCfg into this structure
    std::unordered_map< std::string, MacroLocalInfo > macroLocalMap;

    // plotting 
    void Plot(std::string outputFile, std::vector<MacroNetlist::Partition>& set);

    // netlist  
    void UpdateNetlist(MacroNetlist::Partition& layout);

    // return weighted wire-length to get best solution
    double GetWeightedWL();


    // 
    void StubPlacer(double snapGrid);

  private:
    void FillMacroStor();
    void FillPinGroup();
    void FillVertexEdge();
    void FillMacroConnection();
    
    void UpdateVertexToMacroStor();
    void UpdateInstanceToMacroStor();
//    std::unordered_map<MacroNetlist::Edge*, int> edgeMap;

    // either Pin*, Inst* -> vertexStor's index.
    std::unordered_map<void*, int> pinInstVertexMap;
    std::unordered_map<MacroNetlist::Vertex*, int> vertexPtrMap;

    Eigen::SparseMatrix<int, Eigen::RowMajor> adjMatrix;
    
    // pair of <StartVertex*, EndVertex*> --> edgeStor's index
    std::unordered_map< std::pair<MacroNetlist::Vertex*, MacroNetlist::Vertex*>, 
      int, MyHash<std::pair<void*, void*>> > vertexPairEdgeMap;
    
    std::pair<void*, MacroNetlist::VertexClass> GetPtrClassPair(sta::Pin* pin);

    MacroNetlist::Vertex* GetVertex(sta::Pin* pin); 

    int GetPathWeight( MacroNetlist::Vertex* from, 
        MacroNetlist::Vertex* to, int limit );
    // Matrix version
    int GetPathWeightMatrix ( Eigen::SparseMatrix<int, Eigen::RowMajor> & mat, 
        MacroNetlist::Vertex* from, 
        MacroNetlist::Vertex* to );

    double* netTable; 
};

class CircuitInfo {
  public:
    double lx, ly, ux, uy;
    double siteSizeX, siteSizeY;

    CircuitInfo();

    CircuitInfo( double _lx, double _ly, double _ux, double _uy, 
        double _siteSizeX, double _siteSizeY );

    CircuitInfo( CircuitInfo& orig, MacroNetlist::Partition& part );
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

