#ifndef __MACRO_PLACER_PARSE__
#define __MACRO_PLACER_PARSE__ 

#include <iostream>
#include <climits>
#include <vector>
#include <string>

namespace MacroPlace {

class EnvFile {
  public:
  std::string globalConfig;
  std::string localConfig; 
  int searchDepth;
  bool isWestFix;
  bool isPlot;
  bool generateAll;
  bool isRandomPlace;

  EnvFile();

  void RaiseError(std::string str);
  bool IsFilled();
  void Print();
  
};

}

#endif
