#ifndef __MACRO_PARTITION__
#define __MACRO_PARTITION__ 0

#define MACRO_NETLIST_NAMESPACE_OPEN namespace MacroNetlist {
#define MACRO_NETLIST_NAMESPACE_CLOSE }

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>

using std::string;
using std::vector;
using std::unordered_map;
using std::pair;

template <class T> struct MyHash;

class MacroCircuit;

MACRO_NETLIST_NAMESPACE_OPEN

enum PartClass {
  S, N, W, E, NW, NE, SW, SE, ALL, None
};

class Macro;

class Partition {
  public: 
    PartClass partClass;
    double lx, ly;
    double width, height;
    vector<Macro> macroStor;
    double* netTable;
    int tableCnt;
    unordered_map<string, int> macroMap;


    Partition();
    Partition(PartClass _partClass, double _lx, double _ly,
        double _width, double _height );
   
    // destructor
    ~Partition();

    // copy constructor
    Partition(const Partition& prev);

    // assign operator overloading
    Partition& operator= (const Partition& prev);

    void FillNetlistTable(MacroCircuit& _mckt,
        unordered_map<PartClass, vector<int>, MyHash<PartClass>>& macroPartMap);

    void Dump();

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
struct MyHash< MacroNetlist::PartClass > {
  std::size_t operator()( const MacroNetlist::PartClass & k ) const {
    using boost::hash_combine;
    size_t seed = 0;
    hash_combine(seed, (int)k);
    return seed; 
  }
};


#endif
