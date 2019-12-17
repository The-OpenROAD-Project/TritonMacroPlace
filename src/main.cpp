#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <set>
#include <tcl.h>

#include "timingSta.h"
#include "parse.h"
#include "circuit.h"

using std::string;

extern "C" {
extern int Mplace_Init(Tcl_Interp *interp);
}

static int mplace_argc = 0;
static char** mplace_argv = 0;

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
  
  int argc = mplace_argc;
  char** argv = mplace_argv;
  
  if( argc == 2 ) {
    command = "source " + string(argv[1]);
    Tcl_Eval(interp, command.c_str());
    return TCL_OK;
  }
  else if( argc >= 3) {
    Tcl_Eval(interp, "tritonmp_external mp; mp help;");
    exit(1);
  }
  return TCL_OK;
}
 
int main(int argc, char** argv) {
  mplace_argc = argc;
  mplace_argv = argv;
  Tcl_Main(1, argv, mplaceTclAppInit);
  return 0;
}

