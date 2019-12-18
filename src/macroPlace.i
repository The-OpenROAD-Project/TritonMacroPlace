%module mplace

%{
#include "openroad/OpenRoad.hh"
#include "tritonmp/tritonmp_external.h"
%}

%include "tritonmp/tritonmp_external.h"

%inline %{

MacroPlace::tritonmp_external *
get_tritonmp()
{
  return ord::OpenRoad::openRoad()->getTritonMp();
}

%} // inline
