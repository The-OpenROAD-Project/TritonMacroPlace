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
}
