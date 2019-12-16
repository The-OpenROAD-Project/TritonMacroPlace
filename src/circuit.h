#ifndef __MACRO_PLACER_CIRCUIT__
#define __MACRO_PLACER_CIRCUIT__ 

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <opendb/db.h>

#include "parse.h"
#include "graph.h"
#include "partition.h"
#include "macro.h"


namespace sta { 
class Sta;
}

namespace MacroPlace{ 

class CircuitInfo;

class MacroCircuit {
  public:
    MacroCircuit();
    MacroCircuit(odb::dbDatabase* db, sta::Sta* sta, EnvFile* env, CircuitInfo* cinfo);
    
    void Init(odb::dbDatabase* db, 
        sta::Sta* sta, 
        EnvFile* env, 
        CircuitInfo* cinfo);
    
    std::vector<MacroPlace::Vertex> vertexStor;

    std::vector<MacroPlace::Edge> edgeStor;

    
    // macro Information
    std::vector<MacroPlace::Macro> macroStor;


    // pin Group Information
    std::vector<MacroPlace::PinGroup> pinGroupStor;


    // pin Group Map;
    // Pin* --> pinGroupStor's index.
    std::unordered_map<sta::Pin*, int> pinGroupMap;

    // macro name -> macroStor's index. 
    std::unordered_map<std::string, int> macroNameMap; 

    // macro idx/idx pair -> give each 
    std::vector< std::vector<int> > macroWeight;

    std::string GetEdgeName(MacroPlace::Edge* edge);

    std::string GetVertexName(MacroPlace::Vertex* vertex);

    
    // sta::Instance* --> macroStor's index stor
    std::unordered_map<sta::Instance*, int> macroInstMap;

    // Update Macro Location from Partition info
    void UpdateMacroCoordi(MacroPlace::Partition& part);


    // parsing function
    void ParseGlobalConfig(std::string fileName);
    void ParseLocalConfig(std::string fileName);

    // initialize
    double gHaloX, gHaloY;
    double gChannelX, gChannelY;

    // save LocalCfg into this structure
    std::unordered_map< std::string, MacroLocalInfo > macroLocalMap;

    // plotting 
    void Plot(std::string outputFile, std::vector<MacroPlace::Partition>& set);


    // netlist  
    void UpdateNetlist(MacroPlace::Partition& layout);


    // return weighted wire-length to get best solution
    double GetWeightedWL();


    // 
    void StubPlacer(double snapGrid);

  private:
    odb::dbDatabase* _db;
    sta::Sta* _sta;
    EnvFile* _env;

    double lx, ly, ux, uy;

    void FillMacroStor();
    void FillPinGroup();
    void FillVertexEdge();
    void CheckGraphInfo();
    void FillMacroPinAdjMatrix();
    void FillMacroConnection();
    
    void UpdateVertexToMacroStor();
    void UpdateInstanceToMacroStor();
//    std::unordered_map<MacroPlace::Edge*, int> edgeMap;


    // either Pin*, Inst* -> vertexStor's index.
    std::unordered_map<void*, int> pinInstVertexMap;
    std::unordered_map<MacroPlace::Vertex*, int> vertexPtrMap;


    // adjacency matrix for whole(macro/pins/FFs) graph
    Eigen::SparseMatrix<int, Eigen::RowMajor> adjMatrix;
   
    // vertex idx --> macroPinAdjMatrix idx. 
    std::vector< int > macroPinAdjMatrixMap;

    // adjacency matrix for macro/pins graph
    Eigen::SparseMatrix<int, Eigen::RowMajor> macroPinAdjMatrix;
    
    // pair of <StartVertex*, EndVertex*> --> edgeStor's index
    std::unordered_map< std::pair<MacroPlace::Vertex*, MacroPlace::Vertex*>, 

      int, MyHash<std::pair<void*, void*>> > vertexPairEdgeMap;
    
    std::pair<void*, MacroPlace::VertexClass> GetPtrClassPair(sta::Pin* pin);


    MacroPlace::Vertex* GetVertex(sta::Pin* pin); 


    int GetPathWeight( MacroPlace::Vertex* from, 

        MacroPlace::Vertex* to, int limit );

    // Matrix version
    int GetPathWeightMatrix ( Eigen::SparseMatrix<int, Eigen::RowMajor> & mat, 
        MacroPlace::Vertex* from, 

        MacroPlace::Vertex* to );

    
    // Matrix version
    int GetPathWeightMatrix ( Eigen::SparseMatrix<int, Eigen::RowMajor> & mat, 
        MacroPlace::Vertex* from, 

        int toIdx );
    
    // Matrix version
    int GetPathWeightMatrix ( Eigen::SparseMatrix<int, Eigen::RowMajor> & mat, 
        int fromIdx, int toIdx );

    double* netTable; 
};

class CircuitInfo {
  public:
    double lx, ly, ux, uy;
    double siteSizeX, siteSizeY;

    CircuitInfo();

    CircuitInfo( double _lx, double _ly, double _ux, double _uy, 
        double _siteSizeX, double _siteSizeY );

    CircuitInfo( CircuitInfo& orig, MacroPlace::Partition& part );

};

bool ParseArgv(int argc, char** argv, EnvFile& _env);
void PrintUsage();

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

}

#endif

