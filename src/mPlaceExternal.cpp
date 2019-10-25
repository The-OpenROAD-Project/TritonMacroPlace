#include "mPlaceExternal.h"
#include "mPlace.h"

mplace_external::mplace_external() {}; 
mplace_external::~mplace_external() {};

void mplace_external::help() {};
void mplace_external::import_lef(const char* lef) {
  env.lefStor.push_back(lef);
}

void mplace_external::import_def(const char* def) {
  env.def = def; 
}

void mplace_external::export_def(const char* def) { 

}

void mplace_external::export_all_def(const char* location) { 

}

void mplace_external::import_sdc(const char* sdc) {
  env.sdc = sdc;
}

void mplace_external::import_verilog(const char* verilog) {
  env.verilog = verilog;
}

void mplace_external::import_lib(const char* lib) {
  env.libStor.push_back(lib);
}

void mplace_external::import_global_config(const char* global_config) {
  env.globalConfig = global_config;
}

void mplace_external::import_local_config(const char* local_config) {
  env.localConfig = local_config;
}

bool mplace_external::place_macros() {
  PlaceMacros(env, ckt);
}
