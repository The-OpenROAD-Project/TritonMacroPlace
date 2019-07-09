#ifndef __PARSE__
#define __PARSE__ 0

#include <iostream>
#include <climits>
#include <vector>
#include <string>

class EnvFile {
  public:
  std::string def;
  std::string verilog;
  std::string design;
  std::string output;
  std::string sdc;
  std::string globalConfig;
  std::string localConfig; 
  std::vector<std::string> lefStor;
  std::vector<std::string> libStor;
  int searchDepth;
  bool isWestFix;
  bool isPlot;
  bool generateAll;

  EnvFile() : def(""), verilog(""), design(""), 
  output(""), sdc(""), 
  globalConfig(""), localConfig(""), 
  searchDepth(INT_MIN),
  isWestFix(false),
  isPlot(false),
  generateAll(false) {};

  bool IsFilled() { return design != "" && def != "" && 
      verilog != "" && output != "" && sdc != "" && 
      lefStor.size() != 0 && libStor.size() != 0 &&
      searchDepth!= INT_MIN && globalConfig != ""; };

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

    cout << "globalConfig: " << endl;
    cout << globalConfig << endl << endl;

    cout << "localConfig: " << endl;
    cout << localConfig << endl << endl;
  }
};

#endif
