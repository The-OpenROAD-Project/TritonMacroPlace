#include "graph.h"


namespace MacroNetlist {

Edge::Edge(): from(0), to(0), weight(0) {}; 
Edge::Edge(Vertex* _from, Vertex* _to, int _weight): 
      from(_from), to(_to), weight(_weight) {}; 

Vertex::Vertex(): vertexClass(nonInit), ptr(0) {};
Vertex::Vertex(void* _ptr, VertexClass _vertexClass): 
      vertexClass(_vertexClass), ptr(_ptr) {};

    
std::string PinGroup::name() {
  if( pinGroupClass == PinGroupClass::West ) {
    return "West";
  }
  else if( pinGroupClass == PinGroupClass::East ) {
    return "East";
  } 
  else if( pinGroupClass == PinGroupClass::North) {
    return "North";
  } 
  else if( pinGroupClass == PinGroupClass::South) {
    return "South";
  }
  return ""; 
}

}

