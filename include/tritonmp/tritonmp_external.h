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

class tritonmp_external {
public:
  tritonmp_external ();
  ~tritonmp_external ();

  void help();
  void set_db(odb::dbDatabase* db);
  void set_sta(sta::dbSta* sta);
  void import_global_config(const char* global_config);
  void import_local_config(const char* local_config);
  
  void set_plot_enable(bool mode);
  bool place_macros();
  int get_solution_count();

private:
  odb::dbDatabase* db_;
  sta::dbSta* sta_;
  EnvFile env_;
  MacroCircuit mckt_;
  int solCount_;
}; 

}

#endif
