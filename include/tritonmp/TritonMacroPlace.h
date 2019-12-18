#ifndef __MACRO_PLACER_EXTERNAL__
#define __MACRO_PLACER_EXTERNAL__

#include "parse.h"
#include "circuit.h"

namespace odb {
class dbDatabase;
}

namespace sta {
class dbSta;
}

namespace MacroPlace {

class MacroCircuit;
class EnvFile;

class TritonMacroPlace {
public:
  TritonMacroPlace ();
  ~TritonMacroPlace ();

  void setDb(odb::dbDatabase* db);
  void setSta(sta::dbSta* sta);
  void setGlobalConfig(const char* globalConfig);
  void setLocalConfig(const char* localConfig);
  
  void setPlotEnable(bool mode);
  bool placeMacros();
  int getSolutionCount();

private:
  odb::dbDatabase* db_;
  sta::dbSta* sta_;
  EnvFile env_;
  MacroCircuit mckt_;
  int solCount_;
}; 

}

#endif
