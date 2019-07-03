#ifndef __MACRO_GRAPH__
#define __MACRO_GRAPH__ 0

#define MACRO_NETLIST_NAMESPACE_OPEN namespace MacroNetlist {
#define MACRO_NETLIST_NAMESPACE_CLOSE }

#include <iostream>
#include <string>
#include <vector>
#include <boost/functional/hash.hpp>

namespace sta { 
class Pin; 
}

class MacroCircuit;
template <class T> struct MyHash;

using std::string;
using std::vector;
using std::pair;

MACRO_NETLIST_NAMESPACE_OPEN

class Vertex;
class Edge {
  public:
    Vertex* from;
    Vertex* to;
    int weight;
    Edge(): from(0), to(0), weight(0) {}; 
    Edge(Vertex* _from, Vertex* _to, int _weight): 
      from(_from), to(_to), weight(_weight) {}; 
};

enum VertexClass {
  pin, instMacro, instOther, nonInit
};

class PinGroup;
class Vertex {
  public: 
    VertexClass vertexClass;
    vector<int> from;
    vector<int> to;

    // This can be either PinGroup / sta::Instance*,
    // based on vertexClass
    void* ptr;
      
    Vertex(): vertexClass(nonInit), ptr(0) {};
    Vertex(void* _ptr, VertexClass _vertexClass): 
      vertexClass(_vertexClass), ptr(_ptr) {};
};

enum PinGroupClass {
  West, East, North, South
};

class PinGroup {
  public:
    PinGroupClass pinGroupClass;
    vector<sta::Pin*> pins;
    string name() {
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
};

MACRO_NETLIST_NAMESPACE_CLOSE

template<>
struct MyHash< std::pair<void*, void*> > {
  std::size_t operator()( const pair<void*, void*>& k ) const {
    using boost::hash_combine;
    size_t seed = 0;
    hash_combine(seed, k.first);
    hash_combine(seed, k.second);
    return seed; 
  }
};


#endif
