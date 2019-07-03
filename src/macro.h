#ifndef __MACRO_MACRO__
#define __MACRO_MACRO__ 0

#define MACRO_NETLIST_NAMESPACE_OPEN namespace MacroNetlist {
#define MACRO_NETLIST_NAMESPACE_CLOSE }

#include <iostream>

namespace sta { 
class Instance; 
}

MACRO_NETLIST_NAMESPACE_OPEN

class Vertex;
class Macro {
  public:
    string name;
    string type;  
    double lx, ly;
    double w, h;
    double haloX, haloY; 
    double channelX, channelY;
    Vertex* ptr;
    sta::Instance* instPtr;
    Macro( string _name, string _type, 
        double _lx, double _ly, 
        double _w, double _h,
        double _haloX, double _haloY, 
        double _channelX, double _channelY,
        Vertex* _ptr, sta::Instance* _instPtr) 
      : name(_name), type(_type), 
      lx(_lx), ly(_ly), 
      w(_w), h(_h),
      haloX(_haloX), haloY(_haloY),
      channelX(_channelX), channelY(_channelY), 
      ptr(_ptr), instPtr(_instPtr) {};

    void Dump() {
      std::cout << "MACRO " << name << " " 
        << type << " " 
        << lx << " " << ly << " " 
        << w << " " << h << std::endl;
      std::cout << haloX << " " << haloY << " " 
        << channelX << " " << channelY << std::endl;
    }
};

MACRO_NETLIST_NAMESPACE_CLOSE

class MacroLocalInfo {
private:
  double haloX_, haloY_, channelX_, channelY_;
public:
  MacroLocalInfo() : 
    haloX_(0), haloY_(0), 
    channelX_(0), channelY_(0) {}
  void putHaloX(double haloX) {haloX_ = haloX; }
  void putHaloY(double haloY) {haloY_ = haloY; }
  void putChannelX(double channelX) {channelX_ = channelX; }
  void putChannelY(double channelY) {channelY_ = channelY; }
  double GetHaloX() { return haloX_; }
  double GetHaloY() { return haloY_; }
  double GetChannelX() { return channelX_; }
  double GetChannelY() { return channelY_; }
};


#endif
