#include "mPlaceExternal.h"
#include "mPlace.h"
#include "opendb/lefin.h"
#include "opendb/defin.h"

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
void mplace_external::set_db(odb::dbDatabase* db) {
  db_id = db->getId();
}

void mplace_external::import_lef(const char* lef) {
  dbDatabase* db = odb::dbDatabase::getDatabase(db_id);
  odb::lefin lefReader(db, false);
  lefReader.createTechAndLib("testlib", lef);
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
  odb::dbChip* chip = defReader.createChip( search_libs,  def );
}

void mplace_external::export_def(const char* def) { 
  solCount = 0;
  int bestSetIdx = 0;
  double bestWwl = DBL_MIN;
  for(auto& curSet: allSets) {
    // skip for top-layout partition
    if( curSet.size() == 1) {
      continue;
    }
    // For each partitions (four partition)
    //
    bool isFailed = false;
    for(auto& curPart : curSet) {
      // Annealing based on ParquetFP Engine
      if( !curPart.DoAnneal() ) {
        isFailed = true;
        break;
      }
      // Update mckt frequently
      mckt.UpdateMacroCoordi(curPart);
    }
    if( isFailed ) {
      continue;
    }

    // update partitons' macro info
    for(auto& curPart : curSet) { 
      curPart.UpdateMacroCoordi(mckt);
    }
      
    double curWwl = mckt.GetWeightedWL();
    cout << "Set " << &curSet - &allSets[0] << ": WWL = " << curWwl << endl;

    if( curWwl > bestWwl ) {
      bestWwl = curWwl;
      bestSetIdx = &curSet - &allSets[0]; 
    }
    solCount++;
  }
  
  // bestset DEF writing
  std::vector<MacroNetlist::Partition> bestSet = allSets[bestSetIdx];
  dbDatabase* db = odb::dbDatabase::getDatabase(db_id);
  UpdateCircuitCoordi(db, env, mckt); 

  // check plotting
  if( env.isPlot ) {
    mckt.Plot(string(def) + "_best.plt", bestSet );
  }
}

void mplace_external::export_all_def(const char* location) {
 /* 
  solCount = 0;
  for(auto& curSet: allSets) {
    // skip for top-layout partition
    if( curSet.size() == 1) {
      continue;
    }

    bool isFailed = false;
    // For each partitions (four partition)
    for(auto& curPart : curSet) {
      // Annealing based on ParquetFP Engine
      if( !curPart.DoAnneal() ) {
        isFailed = true;
        break;
      }
      // Update mckt frequently
      mckt.UpdateMacroCoordi(curPart);
    }
    
    if( isFailed ) {
      continue;
    }

    // update partitons' macro info
    for(auto& curPart : curSet) { 
      curPart.UpdateMacroCoordi(mckt);
    }

    // update ckt structure
    UpdateCircuitCoordi(env, mckt, ckt);

    // check plotting
    if( env.isPlot ) {
      mckt.Plot(string(location) + "_" + std::to_string(solCount) + ".plt", curSet);
    }

    string fileName = string(string(location) + "_" 
            + std::to_string(solCount) + ".def");

    // top-level layout print
    FILE* fp = fopen(fileName.c_str(), "w"); 

    if( fp == NULL) { 
      cout << "ERROR: cannot open " << fileName << " to write output file" << endl;
      exit(1);
    }

    ckt.WriteDef( fp );
    fclose(fp);
    
    solCount++;
  }
  */
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
  allSets = PlaceMacros(db, env, mckt);
  return true;
}

int mplace_external::get_solution_count() {
  return solCount;
}
