#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <set>
#include <tcl.h>

#include "timingSta.h"
#include "parse.h"
#include "circuit.h"
#include "lefdefIO.h"

#define STRING_EQUAL(a, b) !strcmp((a), (b))

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using MacroNetlist::Partition;
using MacroNetlist::PartClass;

extern "C" {
extern int Mplace_Init(Tcl_Interp *interp);
}

int 
mplaceTclAppInit(Tcl_Interp *interp) {

  if (Tcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }

  if( Mplace_Init(interp) == TCL_ERROR) {
    return TCL_ERROR; 
  }
  
  string command = "";

  command = "";
  command += "puts \"MacroPlacer Version: 1.0.0\"";
  Tcl_Eval(interp, command.c_str());
}
 
int main(int argc, char** argv) {
  Tcl_Main(1, argv, mplaceTclAppInit);
  return 0;
}


bool ParseArgv(int argc, char** argv, EnvFile& _env) {
  for(int i=1; i<argc; i++) {
    if(STRING_EQUAL("-verilog", argv[i])){
      i++;
      _env.verilog = string(argv[i]);
    }
    else if (STRING_EQUAL("-lib", argv[i])){
      i++;
      _env.libStor.push_back(argv[i]);
    }
    else if (STRING_EQUAL("-lef", argv[i])){
      i++;
      _env.lefStor.push_back(argv[i]);
    }
    else if (STRING_EQUAL("-def", argv[i])){
      i++;
      _env.def = string(argv[i]);
    }
    else if (STRING_EQUAL("-sdc", argv[i])){
      i++;
      _env.sdc= string(argv[i]);
    }
    else if (STRING_EQUAL("-design", argv[i])){
      i++;
      _env.design= string(argv[i]);
    }
    else if (STRING_EQUAL("-output", argv[i])){
      i++;
      _env.output = string(argv[i]);
    }
    else if (STRING_EQUAL("-depth", argv[i])){
      i++;
      _env.searchDepth = atoi(argv[i]);
    }
    else if (STRING_EQUAL("-globalConfig", argv[i])){
      i++;
      _env.globalConfig = string(argv[i]);
    }
    else if (STRING_EQUAL("-localConfig", argv[i])) {
      i++;
      _env.localConfig = string(argv[i]);
    }
    else if (STRING_EQUAL("-westFix", argv[i])) {
      _env.isWestFix = true;
    }
    else if (STRING_EQUAL("-plot", argv[i])) {
      _env.isPlot = true;
    }
    else if (STRING_EQUAL("-generateAll", argv[i])) {
      _env.generateAll = true;
    }
    else if (STRING_EQUAL("-randomPlace", argv[i])) {
      _env.isRandomPlace= true;
    }
  }

  return _env.IsFilled();
}

//void PrintUsage() {
//  cout << "macroPlacer" << endl;
//  cout << "./macroPlacer -lib one.lib -lib two.lib ... -verilog netlist.v" << endl;
//}


