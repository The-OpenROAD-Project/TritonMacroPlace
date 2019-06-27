
#include <iostream.h>
#include "ABKCommon/sgi_hash_map.h"
//#include "hash_set.h"

using std::hash_map;

// Note: assumes long is at least 32 bits.
// fbp: try to avoid intances in every module

/*
inline unsigned long __stl_next_prime(unsigned long n)
{
  const unsigned long* first = __stl_prime_list;
  const unsigned long* last = __stl_prime_list + __stl_num_primes;
  const unsigned long* pos = std::lower_bound(first, last, n);
  return pos == last ? *(last - 1) : *pos;
}
*/

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
//  cout << " eqstr call  : " << unsigned(s1) << " " << unsigned(s2) << endl;
    return strcmp(s1, s2) == 0;
  }
};

int main()
{
  hash_map<const char*, int, std::hash<const char*>, eqstr> months;
 
  months["january"] = 31;
  months["february"] = 28;
  months["march"] = 31;
  months["april"] = 30;
  months["may"] = 31;
  months["june"] = 30;
  months["july"] = 31;
  months["august"] = 31;
  months["september"] = 30;
  months["october"] = 31;
  months["november"] = 30;
  months["december"] = 31;

  cout << " Days in months: " << endl; 
  cout << "september -> " << months["september"] << endl;
  cout << "april     -> " << months["april"] << endl;
  cout << "june      -> " << months["june"] << endl;
  cout << "november  -> " << months["november"] << endl;
return 0;
}
