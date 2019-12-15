#include "parse.h"

namespace MacroPlace {

EnvFile::EnvFile(): def(""), verilog(""), design(""), 
  sdc(""), 
  globalConfig(""), localConfig(""), 
  searchDepth(3),
  isWestFix(false),
  isPlot(false),
  generateAll(false),
  isRandomPlace(false) {};

void EnvFile::RaiseError(std::string str) {
  std::cout << "ERROR: " << str << std::endl;
  exit(1);
}

bool EnvFile::IsFilled() { 
  if ( def == "" ) {
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

void EnvFile::Print() {
  using std::cout;
  using std::endl;

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

  cout << "Search Depth: " << endl;
  cout << searchDepth << endl << endl;

  cout << "globalConfig: " << endl;
  cout << globalConfig << endl << endl;

  cout << "localConfig: " << endl;
  cout << localConfig << endl << endl;
}

}
