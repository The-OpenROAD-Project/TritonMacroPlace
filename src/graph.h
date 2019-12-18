#ifndef __MACRO_NETLIST_GRAPH__
#define __MACRO_NETLIST_GRAPH__ 

#include <iostream>
#include <string>
#include <vector>

namespace sta { 
class Pin; 
}


namespace MacroPlace{

class MacroCircuit;
class Vertex;
class Edge {
  public:
    Vertex* from;
    Vertex* to;
    int weight;

    Edge();
    Edge(Vertex* _from, Vertex* _to, int _weight);
};

enum VertexClass {
  pin, instMacro, instOther, nonInit
};

class PinGroup;
class Vertex {
  public: 
    VertexClass vertexClass;
    std::vector<int> from;
    std::vector<int> to;

    // This can be either PinGroup / sta::Instance*,
    // based on vertexClass
    void* ptr;
      
    Vertex();
    Vertex(void* _ptr, VertexClass _vertexClass);
};

enum PinGroupClass {
  West, East, North, South
};

class PinGroup {
  public:
    PinGroupClass pinGroupClass;
    std::vector<sta::Pin*> pins;
    std::string name();
};


struct PointerPairHash {
  std::size_t operator()(const std::pair<void*, void*> &k) const {
    return std::hash<void*>()(k.first) * 3 + std::hash<void*>()(k.second);
  }
};

struct PointerPairEqual {
  bool operator()(const std::pair<void*, void*> &p1, 
                  const std::pair<void*, void*> &p2) const {
    return p1 == p2;
  }
};

}

#endif
