#ifndef __MACRO_PLACER_EXTERNAL__
#define __MACRO_PLACER_EXTERNAL__

#include <vector>
#include <string>
#include "parse.h"
#include "lefdefIO.h"
#include "circuit.h"

namespace MacroNetlist {
class Partition;
}

class mplace_external {
public:
  mplace_external();
  ~mplace_external();

  void help();

  void import_lef(const char* lef);
  void import_def(const char* def);
  void export_def(const char* def);
  void export_all_def(const char* location);
//  void set_output(const char* output);
  
  void import_sdc(const char* sdc);
  void import_verilog(const char* verilog);
  void import_lib(const char* lib);

  void import_global_config(const char* global_config);
  void import_local_config(const char* local_config);
  
  bool place_macros();

private:
  EnvFile env;
  Circuit::Circuit ckt;
  MacroCircuit mckt;
  std::vector< std::vector<MacroNetlist::Partition> > allSets;
}; 

#endif
