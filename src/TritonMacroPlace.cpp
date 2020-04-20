#include "tritonmp/TritonMacroPlace.h"
#include "circuit.h"

namespace MacroPlace {

using namespace odb;

TritonMacroPlace::TritonMacroPlace() 
  : solCount_(0) { 
  std::unique_ptr<MacroCircuit> mckt(new MacroCircuit());
  mckt_ = std::move(mckt);
} 

TritonMacroPlace::~TritonMacroPlace() {}

void 
TritonMacroPlace::setDb(odb::dbDatabase* db) {
  mckt_->setDb(db);
}
void 
TritonMacroPlace::setSta(sta::dbSta* sta) {
  mckt_->setSta(sta);
}

void 
TritonMacroPlace::setGlobalConfig(const char* globalConfig) {
  mckt_->setGlobalConfig(globalConfig);
}

void 
TritonMacroPlace::setLocalConfig(const char* localConfig) {
  mckt_->setLocalConfig(localConfig);
}

void
TritonMacroPlace::setDieAreaMode(bool mode) {
  mckt_->setDieAreaMode(mode);
}

void 
TritonMacroPlace::setPlotEnable(bool mode) {
  mckt_->setPlotEnable(mode);
}



bool 
TritonMacroPlace::placeMacros() {
  mckt_->PlaceMacros(solCount_);
  return true;
}

int 
TritonMacroPlace::getSolutionCount() {
  return solCount_;
}

}
