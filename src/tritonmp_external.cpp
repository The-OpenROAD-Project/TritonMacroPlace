#include "tritonmp/tritonmp_external.h"
#include "macroPlace.h"
#include "parse.h"

namespace MacroPlace {

using std::string;
using std::cout;
using std::endl;
using std::to_string; 
using namespace odb;

tritonmp_external::tritonmp_external() : db_(0), sta_(0), solCount_(0) {} 
tritonmp_external::~tritonmp_external() {}

void tritonmp_external::help() {}

void tritonmp_external::set_db(odb::dbDatabase* db) {
  db_ = db;
}
void tritonmp_external::set_sta(sta::Sta* sta) {
  sta_ = sta;
}

void tritonmp_external::import_global_config(const char* global_config) {
  env_.globalConfig = global_config;
}

void tritonmp_external::import_local_config(const char* local_config) {
  env_.localConfig = local_config;
}
void tritonmp_external::set_plot_enable(bool mode) {
  env_.isPlot = mode;
}

bool tritonmp_external::place_macros() {
  PlaceMacros(db_, sta_, env_, mckt_, solCount_);
  return true;
}

int tritonmp_external::get_solution_count() {
  return solCount_;
}

}
