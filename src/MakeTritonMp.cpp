#include <tcl.h>
#include "tritonmp/MakeTritonMp.h"
#include "tritonmp/tritonmp_external.h"

extern "C" {
extern int Mplace_Init(Tcl_Interp* interp);
}

namespace ord {

MacroPlace::tritonmp_external * 
makeTritonmp() 
{
  return new MacroPlace::tritonmp_external; 
}

void 
initTritonmp(OpenRoad *openroad) 
{
//  Tcl_Interp* tcl_interp = openroad->tclInterp();
//  Mplace_Init(tcl_interp);
}

void
deleteTritonmp(MacroPlace::tritonmp_external *tritonmp)
{
  delete tritonmp;
}


}
