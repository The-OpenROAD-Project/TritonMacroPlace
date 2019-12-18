#include "parse.h"

namespace MacroPlace {

EnvFile::EnvFile(): 
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
  if ( globalConfig == "" ) {
    RaiseError("Please put globalConfig with -globalConfig option");
    return false;
  }

  return true;
}

void EnvFile::Print() {
  using std::cout;
  using std::endl;

  cout << "Search Depth: " << endl;
  cout << searchDepth << endl << endl;

  cout << "globalConfig: " << endl;
  cout << globalConfig << endl << endl;

  cout << "localConfig: " << endl;
  cout << localConfig << endl << endl;
}

}
