#ifndef __MACRO_PLACE__
#define __MACRO_PLACE__

#include "partition.h"
#include "opendb/db.h"

class MacroCircuit;
class CircuitInfo;
class EnvFile;

namespace Circuit{
  class Circuit;
}

std::vector<std::pair<MacroPlace::Partition, MacroPlace::Partition>> GetPart ( 

    CircuitInfo& cInfo,  
    MacroPlace::Partition& partition, bool isHorizontal );


// Partition Class --> macroStor's index.
void UpdateMacroPartMap( MacroCircuit& mckt,
    MacroPlace::Partition& part, 

        std::unordered_map< MacroPlace::PartClass, std::vector<int>,

        MyHash<MacroPlace::PartClass>>& macroPartMap);


void
PlaceMacros(odb::dbDatabase* db, 
    EnvFile& env, MacroCircuit& mckt, int& solCount);

void UpdateOpendbCoordi(odb::dbDatabase* db, 
    EnvFile& env, MacroCircuit& mckt);


#endif
