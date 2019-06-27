
#include <iostream>
using std::cout;
using std::endl;
#include "../sgi_hash_set.h"

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  { return strcmp(s1, s2) == 0; }
};

void lookup(const std::hash_set<const char*, std::hash<const char*>, eqstr>& Set,
            const char* word)
{
  std::hash_set<const char*, std::hash<const char*>, eqstr>::const_iterator it
    = Set.find(word);
  cout << word << ": "
       << (it != Set.end() ? "present" : "not present")
       << endl;
}

int main()
{
  std::hash_set<const char*, std::hash<const char*>, eqstr> Set;
  Set.insert("kiwi");
  Set.insert("plum");
  Set.insert("apple");
  Set.insert("mango");
  Set.insert("apricot");
  Set.insert("banana");

  lookup(Set, "mango");
  lookup(Set, "apple");
  lookup(Set, "durian");
}
