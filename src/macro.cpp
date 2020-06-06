#include "macro.h"
#include "opendb/db.h"
#include <iostream>

namespace MacroPlace{ 


Macro::Macro( 
        double _lx, double _ly, 
        double _w, double _h,
        double _haloX, double _haloY, 
        double _channelX, double _channelY,
        Vertex* vertex, sta::Instance* staInst,
        odb::dbInst* dbInst) 
      : lx(_lx), ly(_ly), 
      w(_w), h(_h),
      haloX(_haloX), haloY(_haloY),
      channelX(_channelX), channelY(_channelY), 
      vertex_(vertex), staInst_(staInst),
      dbInst_(dbInst) {}

std::string Macro::name() {
  return dbInst()->getConstName();
}

std::string Macro::type() {
  return dbInst()->getMaster()->getConstName();
}

void Macro::Dump() {
  std::cout << "MACRO " << name() << " " 
    << type() << " " 
    << lx << " " << ly << " " 
    << w << " " << h << std::endl;
  std::cout << haloX << " " << haloY << " " 
    << channelX << " " << channelY << std::endl;
}
}

MacroLocalInfo::MacroLocalInfo() : 
    haloX_(0), haloY_(0), 
    channelX_(0), channelY_(0) {}
