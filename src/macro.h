#ifndef __MACRO_MACRO__
#define __MACRO_MACRO__ 

#include <iostream>

namespace sta { 
class Instance; 
}

namespace MacroNetlist { 

class Vertex;
class Macro {
  public:
    std::string name;
    std::string type;  
    double lx, ly;
    double w, h;
    double haloX, haloY; 
    double channelX, channelY;
    Vertex* ptr;
    sta::Instance* instPtr;
    Macro( std::string _name, std::string _type, 
        double _lx, double _ly, 
        double _w, double _h,
        double _haloX, double _haloY, 
        double _channelX, double _channelY,
        Vertex* _ptr, sta::Instance* _instPtr);
    void Dump(); 
};

}

class MacroLocalInfo {
private:
  double haloX_, haloY_, channelX_, channelY_;
public:
  MacroLocalInfo(); 
  void putHaloX(double haloX) { haloX_ = haloX; }
  void putHaloY(double haloY) { haloY_ = haloY; }
  void putChannelX(double channelX) { channelX_ = channelX; }
  void putChannelY(double channelY) { channelY_ = channelY; }
  double GetHaloX() { return haloX_; }
  double GetHaloY() { return haloY_; }
  double GetChannelX() { return channelX_; }
  double GetChannelY() { return channelY_; }
};


#endif
