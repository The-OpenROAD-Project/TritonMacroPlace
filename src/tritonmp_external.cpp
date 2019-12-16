#include "tritonmp/tritonmp_external.h"
#include "macroPlace.h"
#include "opendb/lefin.h"
#include "opendb/defin.h"
#include "opendb/defout.h"

namespace MacroPlace {

using std::string;
using std::cout;
using std::endl;
using std::to_string; 
using namespace odb;

mplace_external::mplace_external() : solCount(0) {
  odb::dbDatabase* db = odb::dbDatabase::create();
  db_id = db->getId();
} 
mplace_external::~mplace_external() {}

void mplace_external::help() {}
//void mplace_external::set_db(odb::dbDatabase* db) {
//  db_id = db->getId();
//}

void mplace_external::import_lef(const char* lef) {
  dbDatabase* db = odb::dbDatabase::getDatabase(db_id);
  odb::lefin lefReader(db, false);
  if( db->getTech() == nullptr ) { 
    lefReader.createTechAndLib(lef, lef);
  }
  else {
    lefReader.createLib(lef, lef);
  }
}

void mplace_external::import_def(const char* def) {
  dbDatabase* db = odb::dbDatabase::getDatabase(db_id);
  odb::defin defReader(db);

  std::vector<odb::dbLib *> search_libs;
  odb::dbSet<odb::dbLib> libs = db->getLibs();
  odb::dbSet<odb::dbLib>::iterator itr;
  for( itr = libs.begin(); itr != libs.end(); ++itr ) {
    search_libs.push_back(*itr);
  }
  defReader.createChip( search_libs,  def );
}

void mplace_external::export_def(const char* def) { 
  defout writer;
  writer.setVersion( defout::DEF_5_6 );
  dbDatabase* db = odb::dbDatabase::getDatabase(db_id);
  dbChip* chip = db->getChip();
  dbBlock* block = chip->getBlock();
  writer.writeBlock( block, def );
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
void mplace_external::set_plot_enable(bool mode) {
  env.isPlot = mode;
}

bool mplace_external::place_macros() {
  odb::dbDatabase* db = odb::dbDatabase::getDatabase(db_id);
  PlaceMacros(db, NULL, env, mckt, solCount);
  return true;
}

int mplace_external::get_solution_count() {
  return solCount;
}

}
