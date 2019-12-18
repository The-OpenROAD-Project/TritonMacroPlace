#include "tritonmp/TritonMacroPlace.h"
#include "macroPlace.h"
#include "parse.h"

namespace MacroPlace {

using std::string;
using std::cout;
using std::endl;
using std::to_string; 
using namespace odb;

TritonMacroPlace::TritonMacroPlace() : db_(0), sta_(0), solCount_(0) {} 
TritonMacroPlace::~TritonMacroPlace() {}

void TritonMacroPlace::setDb(odb::dbDatabase* db) {
  db_ = db;
}
void TritonMacroPlace::setSta(sta::dbSta* sta) {
  sta_ = sta;
}

void TritonMacroPlace::setGlobalConfig(const char* globalConfig) {
  env_.globalConfig = global_config;
}

void TritonMacroPlace::setLocalConfig(const char* localConfig) {
  env_.localConfig = local_config;
}
void TritonMacroPlace::setPlotEnable(bool mode) {
  env_.isPlot = mode;
}

bool TritonMacroPlace::placeMacros() {
  PlaceMacros(db_, sta_, env_, mckt_, solCount_);
  return true;
}

int TritonMacroPlace::getSolutionCount() {
  return solCount_;
}

}
