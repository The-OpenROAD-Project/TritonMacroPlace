#ifndef __MACRO_PLACE__
#define __MACRO_PLACE__

#include "partition.h"
#include "opendb/db.h"

namespace MacroPlace {

class MacroCircuit;
class CircuitInfo;
class EnvFile;

void
PlaceMacros(odb::dbDatabase* db, 
    EnvFile& env, MacroCircuit& mckt, int& solCount);

void UpdateOpendbCoordi(odb::dbDatabase* db, 
    EnvFile& env, MacroCircuit& mckt);

}

#endif
