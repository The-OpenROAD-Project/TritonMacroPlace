#ifndef __MACRO_PLACE__
#define __MACRO_PLACE__

#include "partition.h"

namespace odb { 
class dbDatabase;
}

namespace sta {
class dbSta;
}

namespace MacroPlace {

class MacroCircuit;
class EnvFile;

void
PlaceMacros(odb::dbDatabase* db, sta::dbSta*, 
    EnvFile& env, MacroCircuit& mckt, int& solCount);

}

#endif
