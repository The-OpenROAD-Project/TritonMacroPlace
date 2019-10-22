#include "lefdefIO.h"

using std::string;
using std::vector;
using std::unordered_map; 
using std::cout;
using std::endl;

namespace Circuit { 

Circuit::Circuit()
  : lefManufacturingGrid(DBL_MIN) {};

Circuit::Circuit(vector<string>& lefStor, 
      string defFilename, string verilogFilename )
  : lefManufacturingGrid(DBL_MIN) {
    Init( lefStor, defFilename, verilogFilename ); 
};

void Circuit::Init( vector<string>& lefStor, string defFilename, 
    string verilogFilename ) {
  ParseLef(lefStor);
  ParseDef(defFilename);
  if( verilogFilename != "" ) {
    // ParseVerilog(verilogFilename);   
  }
}

NetInfo::NetInfo( int _macroIdx, int _compIdx, int _pinIdx) 
    : macroIdx(_macroIdx), compIdx(_compIdx), pinIdx(_pinIdx) {};
}
  
DieRect::DieRect() : llx(INT_MAX), lly(INT_MAX), urx(INT_MIN), ury(INT_MIN) {};

void DieRect::Dump() {
  cout << "(" << llx << ", " << lly << ") - (" << urx << ", " << ury << ")" << endl;
}

bool DieRect::isNotInitialize () {
    return ( llx == INT_MAX || lly == INT_MAX
        || urx == INT_MIN || ury == INT_MIN);
}
