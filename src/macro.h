#ifndef __MACRO_PLACER_MACRO__
#define __MACRO_PLACER_MACRO__ 

#include <string>

namespace sta { 
class Instance; 
}

namespace odb {
class dbInst;
}

namespace MacroPlace { 

class Vertex;
class Macro {
  private:  
    odb::dbInst* dbInst_;

  public:
    std::string name;
    std::string type;  
    double lx, ly;
    double w, h;
    double haloX, haloY; 
    double channelX, channelY;
    Vertex* ptr;
    sta::Instance* staInstPtr;

    Macro( std::string _name, std::string _type, 
        double _lx, double _ly, 
        double _w, double _h,
        double _haloX, double _haloY, 
        double _channelX, double _channelY,
        Vertex* _ptr, sta::Instance* _staInstPtr,
        odb::dbInst* dbInst); 

    odb::dbInst* dbInst() const { return dbInst_; }
    void Dump(); 
};

}

class MacroLocalInfo {
private:
  double haloX_, haloY_, channelX_, channelY_;
public:
  MacroLocalInfo(); 
  void setHaloX(double haloX) { haloX_ = haloX; }
  void setHaloY(double haloY) { haloY_ = haloY; }
  void setChannelX(double channelX) { channelX_ = channelX; }
  void setChannelY(double channelY) { channelY_ = channelY; }
  double getHaloX() { return haloX_; }
  double getHaloY() { return haloY_; }
  double getChannelX() { return channelX_; }
  double getChannelY() { return channelY_; }
};


#endif
