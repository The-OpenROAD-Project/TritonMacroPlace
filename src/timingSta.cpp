#include <timingSta.h>

sta::Sta* GetStaObject(EnvFile& _env ) {
  using namespace sta;
  
  // STA object create
  Sta* _sta = new Sta;
  Tcl_Interp* _interp = Tcl_CreateInterp();

  // define swig commands
  Sta_Init(_interp);

  // load encoded TCL functions
  evalTclInit(_interp, tcl_inits);
  // initialize TCL commands
  Tcl_Eval(_interp, "sta::show_splash");
  Tcl_Eval(_interp, "sta::define_sta_cmds");
  Tcl_Eval(_interp, "namespace import sta::*");

  // initialize STA objects
  initSta();
  Sta::setSta(_sta);
  _sta->makeComponents();
  _sta->setTclInterp(_interp);
//  _sta->setThreadCount(8);

  // environment settings

  string cornerName="wst";
  //    string cornerNameFF="bst";

  StringSet cornerNameSet;
  cornerNameSet.insert(cornerName.c_str());
  //    cornerNameSet.insert(cornerNameFF.c_str());

  //define_corners
  _sta->makeCorners(&cornerNameSet);
  Corner *corner = _sta->findCorner(cornerName.c_str());
  //    Corner *corner_FF = _sta->findCorner(cornerNameFF.c_str());


  // read_liberty
  for(auto& libName : _env.libStor) {
    _sta->readLiberty(libName.c_str(), corner, MinMaxAll::max(), false);
  }

  //read_netlist
  NetworkReader *network = _sta->networkReader();
  bool readVerilog = false;
  if (network) {
    _sta->readNetlistBefore();       
    readVerilog = 
      readVerilogFile(_env.verilog.c_str(), _sta->report(), 
          _sta->debug(), _sta->networkReader());
  }

  //link_design
  bool link = _sta->linkDesign(_env.design.c_str());
  bool isLinked = network->isLinked();
  if( !isLinked ) {
    cout << "ERROR on OpenSTA's linking..!" << endl;
    exit(1);
  }

  Tcl_Eval(_interp, string( "sta::read_sdc " + _env.sdc).c_str() ); 
//  bool parasitics = 
//    _sta->readParasitics(_env.spef.c_str(), 
//        _sta->currentInstance(), 
//        MinMaxAll::max(), false, true, 0.0, 
//        reduce_parasitics_to_pi_elmore, false, true, true);

/*  
  string _clkName = "clk"; 
  float _clkPeriod = 2e-10;

  FloatSeq *waveform = new FloatSeq;
  waveform->push_back(0.0);
  waveform->push_back(_clkPeriod/2);

  PinSet *pins = new PinSet;
  Pin *pin = _sta->network()->findPin(_sta->currentInstance(),_clkName.c_str());
  pins->insert(pin);

  _sta->makeClock(_clkName.c_str(),pins,true,_clkPeriod,waveform, NULL);
  _sta->findClock(_clkName.c_str());
*/
  _sta->updateTiming(true);
  return _sta;
}

