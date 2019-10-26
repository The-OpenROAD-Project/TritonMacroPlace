#ifndef __MACRO_PARTITION__
#define __MACRO_PARTITION__ 0


#include <string>
#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>


template <class T> struct MyHash;

class MacroCircuit;

namespace MacroNetlist {

enum PartClass {
  S, N, W, E, NW, NE, SW, SE, ALL, None
};

class Macro;

class Partition {
  public: 
    PartClass partClass;
    double lx, ly;
    double width, height;
    std::vector<Macro> macroStor;
    double* netTable;
    int tableCnt;
    std::unordered_map<std::string, int> macroMap;


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
        std::unordered_map<PartClass, std::vector<int>, MyHash<PartClass>>& macroPartMap);

    void Dump();

    // Call Parquet to have annealing solution
    void DoAnneal();

    // Update Macro location from MacroCircuit
    void UpdateMacroCoordi(MacroCircuit& mckt);

    // Writing functions
    void PrintSetFormat(FILE* fp);

    // Parquet print functions
    void PrintParquetFormat(std::string origName);
    void WriteBlkFile(std::string blkName);
    void WriteNetFile(std::vector< std::pair<int, int>> & netStor, std::string netName);
    void WriteWtsFile(std::vector<int>& costStor, std::string wtsName );
    void WritePlFile(std::string plName);

    std::string GetName(int macroIdx);

  private:
    void FillNetlistTableIncr();
    void FillNetlistTableDesc();
}; 

}

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
