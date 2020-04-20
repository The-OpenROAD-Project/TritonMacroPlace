%module mplace

%{
#include "openroad/OpenRoad.hh"
#include "tritonmp/TritonMacroPlace.h"

namespace ord {
// Defined in OpenRoad.i
MacroPlace::TritonMacroPlace*
getTritonMp();
}

using ord::getTritonMp;
using MacroPlace::TritonMacroPlace;
%}

%inline %{

void
set_macro_place_global_config_cmd(const char* file) 
{
  TritonMacroPlace* tritonMp = getTritonMp();
  tritonMp->setGlobalConfig(file); 
}

void
set_macro_place_local_config_cmd(const char* file)
{
  TritonMacroPlace* tritonMp = getTritonMp();
  tritonMp->setLocalConfig(file); 
}

void
set_macro_place_die_area_mode_cmd(bool mode)
{
  TritonMacroPlace* tritonMp = getTritonMp();
  tritonMp->setDieAreaMode(mode); 
}

void
place_macros_cmd()
{
  TritonMacroPlace* tritonMp = getTritonMp();
  tritonMp->placeMacros(); 
} 

int
get_macro_place_solution_count_cmd()
{
  TritonMacroPlace* tritonMp = getTritonMp();
  return tritonMp->getSolutionCount();
} 

%} // inline
