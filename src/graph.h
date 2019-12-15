#ifndef __MACRO_NETLIST_GRAPH__
#define __MACRO_NETLIST_GRAPH__ 

#include <iostream>
#include <string>
#include <vector>
#include <boost/functional/hash.hpp>

namespace sta { 
class Pin; 
}


namespace MacroPlace{

class MacroCircuit;
class Vertex;
template <class T> struct MyHash;
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


// hash function definition for two pairs.
// this will enable unordered_map/unordered_set usages.
template<>
struct MyHash< std::pair<void*, void*> > {
  std::size_t operator() ( const std::pair<void*, void*>& k ) const {
    size_t seed = 0;
    boost::hash_combine(seed, k.first);
    boost::hash_combine(seed, k.second);
    return seed; 
  }
};

}

#endif
