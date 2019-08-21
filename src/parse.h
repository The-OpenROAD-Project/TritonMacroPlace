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

  EnvFile();

  void RaiseError(std::string str);
  bool IsFilled();
  void Print();
  
};

#endif
