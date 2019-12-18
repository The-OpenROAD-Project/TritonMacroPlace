#include <tcl.h>
#include "StaMain.hh"
#include "openroad/OpenRoad.hh"
#include "tritonmp/MakeTritonMp.h"
#include "tritonmp/tritonmp_external.h"


namespace sta {
extern const char *tritonmp_tcl_inits[];
}

extern "C" {
extern int Mplace_Init(Tcl_Interp* interp);
}

namespace ord {

MacroPlace::tritonmp_external * 
makeTritonMp() 
{
  return new MacroPlace::tritonmp_external; 
}

void 
initTritonMp(OpenRoad *openroad) 
{
  Tcl_Interp* tcl_interp = openroad->tclInterp();
  Mplace_Init(tcl_interp);
  sta::evalTclInit(tcl_interp, sta::tritonmp_tcl_inits);
  openroad->getTritonMp()->set_db(openroad->getDb());
  openroad->getTritonMp()->set_sta(openroad->getSta());
}

void
deleteTritonMp(MacroPlace::tritonmp_external *tritonmp)
{
  delete tritonmp;
}


}
