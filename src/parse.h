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
  bool isRandomPlace;

  EnvFile() : def(""), verilog(""), design(""), 
  output(""), sdc(""), 
  globalConfig(""), localConfig(""), 
  searchDepth(3),
  isWestFix(false),
  isPlot(false),
  generateAll(false),
  isRandomPlace(false) {};

  void RaiseError(std::string str) {
    std::cout << "ERROR: " << str << std::endl;
    exit(1);
  }

  bool IsFilled() { 
    if( design == "" ) {
      RaiseError("Please put input design name with -design option");
      return false;
    }
    else if ( def == "" ) {
      RaiseError("Please put input def file with -def option");
      return false;
    }
    else if ( lefStor.size() == 0 ) {
      RaiseError("Please put input lef file with -lef option");
      return false;
    }
    else if ( verilog == "" ) {
      RaiseError("Please put input verilog file with -verilog option");
      return false;
    }
    else if ( output == "" ) {
      RaiseError("Please put input def file with -output option");
      return false;
    }
    else if ( sdc == "" ) {
      RaiseError("Please put input sdc file with -sdc option");
      return false;
    }
    else if ( libStor.size() == 0 ) {
      RaiseError("Please put input lib file with -lib option");
      return false;
    }
    else if ( globalConfig == "" ) {
      RaiseError("Please put globalConfig with -globalConfig option");
      return false;
    }

    return true;
  }

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
