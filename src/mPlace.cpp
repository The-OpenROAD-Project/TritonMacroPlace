#include "mPlace.h"
#include "circuit.h"
#include "lefdefIO.h"
#include "partition.h"

#include <boost/icl/interval_map.hpp>
#include <boost/icl/closed_interval.hpp>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using MacroNetlist::Partition;
using namespace MacroNetlist;
using namespace boost::icl;

typedef vector<pair<Partition, Partition>> TwoPartitions;

void CutRoundUp( CircuitInfo& cInfo, double& cutLine, bool isHorizontal ) {
  if( isHorizontal ) {
    int integer = (1.0* cutLine / cInfo.siteSizeX) + 0.5f;
    cutLine = integer * cInfo.siteSizeX;
    cutLine = (cutLine > cInfo.ux)? cInfo.ux : cutLine;
  }
  else {
    int integer = (1.0* cutLine / cInfo.siteSizeY) + 0.5f;
    cutLine = integer * cInfo.siteSizeY;
    cutLine = (cutLine > cInfo.uy)? cInfo.uy : cutLine;
  }
}

//
void PrintAllSets(FILE* fp, CircuitInfo& cInfo, 
    vector< vector<Partition> >& allSets) {
  cout << "Writing sets file... ";
  for(auto& curSet : allSets) {
    fprintf(fp,"BEGIN SET ;\n" );
    fprintf(fp,"  WIDTH %f ;\n", cInfo.ux - cInfo.lx );
    fprintf(fp,"  HEIGHT %f ;\n", cInfo.uy - cInfo.ly );
    for(auto& curSlice : curSet) {
      if( curSlice.macroStor.size() == 0) {
        continue;
      }
      curSlice.PrintSetFormat(fp);
    }
    fprintf(fp,"END SET ;\n");
  }
  fflush(fp);
  cout << "Done!" << endl;
}

void PlaceMacros(EnvFile& env, Circuit::Circuit& ckt) {
  env.Print(); 
  
  cout << "PROC: LEF/DEF Parsing... ";
  ckt.Init(env.lefStor, env.def);
  cout << "Done!" << endl;

  double defScale = ckt.defUnit;
  defiPoints points = ckt.defDieArea.getPoint();
  if( points.numPoints != 2) {
    cout << "ERROR: The Layout must be rectangle..! "<< endl;
    exit(1);
  }

  if( ckt.defRowStor.size() == 0) {
    cout << "ERROR: DEF must contain ROW"<< endl;
    exit(1);
  }

  string siteName(ckt.defRowStor[0].macro());
  auto sitePtr = ckt.lefSiteMap.find( siteName );
  if( sitePtr == ckt.lefSiteMap.end()) {
    cout << "ERROR: SITE " << siteName << " not exists in LEF." << endl;
    exit(1);
  }
  
  double siteSizeX = ckt.lefSiteStor[sitePtr->second].sizeX();
  double siteSizeY = ckt.lefSiteStor[sitePtr->second].sizeY();

  CircuitInfo _cinfo( 1.0*points.x[0]/defScale, 
        1.0*points.y[0]/defScale,
        1.0*points.x[1]/defScale,
        1.0*points.y[1]/defScale,
        siteSizeX, siteSizeY );

  
  cout << endl;
//  cout << "Layout Information" << endl;
//  for(int i=0; i<points.numPoints; i++) {
//    cout << 1.0*points.x[i]/defScale << " ";
//    cout << 1.0*points.y[i]/defScale << endl;
//  }

//  cout << "Original Macro List" << endl;

  MacroCircuit _mckt(ckt, env, _cinfo);
    
  
  //  RandomPlace for special needs. 
  //  Really not recommended to execute this functioning 
  if( env.isRandomPlace == true ) {
    double snapGrid = 0.02f;
    _mckt.StubPlacer(snapGrid);
  }


//  _mckt.GetMacroStor(ckt);

//  for(auto& curMacro: _mckt.macroStor) {
//    curMacro.Dump();
//  }
 
  bool isHorizontal = (_cinfo.ux-_cinfo.lx) > (_cinfo.uy-_cinfo.ly);

  Partition layout(PartClass::ALL, 
      _cinfo.lx, _cinfo.ly, _cinfo.ux-_cinfo.lx, _cinfo.uy-_cinfo.ly);
  layout.macroStor = _mckt.macroStor;

  cout << "PROC: Begin One Level Partition ... " << endl; 
  TwoPartitions oneLevelPart = GetPart(_cinfo, layout, isHorizontal);
  cout << "PROC: End One Level Partition" << endl << endl;
  TwoPartitions eastStor, westStor;

  vector< vector<Partition> > allSets;


  // Fill the MacroNetlist for ALL circuits
  unordered_map< PartClass, vector<int>, 
    MyHash<PartClass>> globalMacroPartMap;
  UpdateMacroPartInfo( _mckt, layout, globalMacroPartMap );
  layout.FillNetlistTable( _mckt, globalMacroPartMap );
  _mckt.UpdateNetlist(layout);
 
  // push to the outer vector 
  vector<Partition> layoutSet;
  layoutSet.push_back(layout);

  // push
  allSets.push_back(layoutSet);

  for(auto& curSet : oneLevelPart ) {
    if( isHorizontal ) {
      cout << "PROC: Begin Horizontal Cuts" << endl;
      CircuitInfo eastInfo(_cinfo, curSet.first);
      CircuitInfo westInfo(_cinfo, curSet.second);

      cout << "PROC: Begin 1st Second Level Partition" << endl;
      TwoPartitions eastStor = GetPart(eastInfo, curSet.first, !isHorizontal);
      cout << "PROC: End 1st Second Level Partition" << endl << endl;

      cout << "PROC: Begin 2nd Second Level Partition" << endl;
      TwoPartitions westStor = GetPart(westInfo, curSet.second, !isHorizontal);
      cout << "PROC: End 2nd Second Level Partition" << endl << endl;

     
      // Zero case handling when eastStor = 0 
      if( eastStor.size() == 0 && westStor.size() != 0 ) {
        for(size_t i=0; i<westStor.size(); i++) {
          vector<Partition> oneSet;

          // one set is composed of two subblocks
          oneSet.push_back( westStor[i].first );
          oneSet.push_back( westStor[i].second );

          // Fill Macro Netlist
          // update macroPartMap
          unordered_map< PartClass, vector<int>, 
            MyHash<PartClass>> macroPartMap;
          for(auto& curSet: oneSet) {
            UpdateMacroPartInfo( _mckt, curSet, macroPartMap );
          }

          for(auto& curSet: oneSet) {
            curSet.FillNetlistTable( _mckt, macroPartMap );
          }

          allSets.push_back( oneSet );
        }
      }
      // Zero case handling when westStor = 0 
      else if( eastStor.size() != 0 && westStor.size() == 0 ) {
        for(size_t i=0; i<eastStor.size(); i++) {
          vector<Partition> oneSet;

          // one set is composed of two subblocks
          oneSet.push_back( eastStor[i].first );
          oneSet.push_back( eastStor[i].second );

          // Fill Macro Netlist
          // update macroPartMap
          unordered_map< PartClass, vector<int>, 
            MyHash<PartClass>> macroPartMap;
          for(auto& curSet: oneSet) {
            UpdateMacroPartInfo( _mckt, curSet, macroPartMap );
          }

          for(auto& curSet: oneSet) {
            curSet.FillNetlistTable( _mckt, macroPartMap );
          }

          allSets.push_back( oneSet );
        } 
      }
      else {
        // for all possible combinations in partitions
        for(size_t i=0; i<eastStor.size(); i++) {
          for(size_t j=0; j<westStor.size(); j++) {

            vector<Partition> oneSet;

            // one set is composed of four subblocks
            oneSet.push_back( eastStor[i].first );
            oneSet.push_back( eastStor[i].second );
            oneSet.push_back( westStor[j].first );
            oneSet.push_back( westStor[j].second );

            // Fill Macro Netlist
            // update macroPartMap
            unordered_map< PartClass, vector<int>, 
              MyHash<PartClass>> macroPartMap;
            for(auto& curSet: oneSet) {
              UpdateMacroPartInfo( _mckt, curSet, macroPartMap );
            }

            for(auto& curSet: oneSet) {
              curSet.FillNetlistTable( _mckt, macroPartMap );
            }

            allSets.push_back( oneSet );
          }
        }
      } 
    }
    else {
      cout << "PROC: Vertical Cuts" << endl;
    }
  }
  cout << "INFO: Total Extracted Sets = " << allSets.size() << endl << endl;


  // Legalize macro on global Structure
//  double snapGrid = 0.002f;

  // TopLevel Macro Location Update
  // For each possible full-layout
  
  int setCnt = 0;
  int bestSetIdx = 0;
  double bestWwl = DBL_MIN;
  for(auto& curSet: allSets) {

    // skip for top-layout partition
    if( curSet.size() == 1) {
      continue;
    }

    // For each partitions (four partition)
//    cout << "curSet.size: " << curSet.size() << endl;
    for(auto& curPart : curSet) {
      // Annealing based on ParquetFP Engine
      curPart.DoAnneal();
      // Update _mckt frequently
      _mckt.UpdateMacroLoc(curPart);
    }
      

    // update partitons' macro info
    for(auto& curPart : curSet) { 
      for(auto& curMacro : curPart.macroStor) {
        auto mPtr = _mckt.macroNameMap.find(curMacro.name);
        int macroIdx = mPtr->second;
        curMacro.lx = _mckt.macroStor[macroIdx].lx;
        curMacro.ly = _mckt.macroStor[macroIdx].ly;
      }
    }

    if( env.generateAll ) {
      // update ckt structure
      for(auto& curMacro : _mckt.macroStor) {
        auto cPtr = ckt.defComponentMap.find( curMacro.name );
        if( cPtr == ckt.defComponentMap.end()) {
          cout << "ERROR: Cannot find " << curMacro.name 
            << " in defiComponentMap" << endl;
          exit(1);
        }
        int cIdx = cPtr->second;
        int lx = int(curMacro.lx * defScale + 0.5f);
        int ly = int(curMacro.ly * defScale + 0.5f);
        
        int orient = -1;
        if( ckt.defComponentStor[cIdx].placementStatus() != 0 
            && ckt.defComponentStor[cIdx].placementStatus() != DEFI_COMPONENT_UNPLACED ) {
          // Follow original orient 
          orient = ckt.defComponentStor[cIdx].placementOrient(); 
        }
        else {
          // Default is North
          orient = 0;
        }

        if( env.isWestFix ) {
          orient = 1;
        }

        ckt.defComponentStor[cIdx].
          setPlacementLocation(lx, ly, orient);

        ckt.defComponentStor[cIdx].
          setPlacementStatus(DEFI_COMPONENT_FIXED);
      }

      // check plotting
      if( env.isPlot ) {
        _mckt.Plot(env.output + "_" + std::to_string(setCnt) + ".plt", curSet);
      }

      // top-level layout print
      FILE* fp = 
        fopen(string(env.output + "_" 
              + std::to_string(setCnt) + ".def").c_str(), "w"); 

      if( fp == NULL) { 
        cout << "ERROR: cannot open " << env.output << " to write output file" << endl;
        exit(1);
      }

      ckt.WriteDef( fp );
      fclose(fp);
    }
    else {
      double curWwl = _mckt.GetWeightedWL();
      if( curWwl > bestWwl ) {
        bestWwl = curWwl;
        bestSetIdx = &curSet - &allSets[0]; 
      }
    }
    setCnt ++;
  }

  // bestset DEF writing
  vector<Partition> bestSet = allSets[bestSetIdx];
  if( !env.generateAll ) {
    for(auto& curPart : bestSet) { 
      for(auto& curMacro : curPart.macroStor) {
        auto mPtr = _mckt.macroNameMap.find(curMacro.name);
        int macroIdx = mPtr->second;
        curMacro.lx = _mckt.macroStor[macroIdx].lx;
        curMacro.ly = _mckt.macroStor[macroIdx].ly;
      }
    }
     
    unordered_set<int> macroIdxMap;

    // update ckt structure
    for(auto& curMacro : _mckt.macroStor) {
      auto cPtr = ckt.defComponentMap.find( curMacro.name );
      if( cPtr == ckt.defComponentMap.end()) {
        cout << "ERROR: Cannot find " << curMacro.name 
          << " in defiComponentMap" << endl;
        exit(1);
      }
      int cIdx = cPtr->second;
      macroIdxMap.insert(cIdx);
      int lx = int(curMacro.lx * defScale + 0.5f);
      int ly = int(curMacro.ly * defScale + 0.5f);

      int orient = -1;
      if( ckt.defComponentStor[cIdx].placementStatus() != 0 
          && ckt.defComponentStor[cIdx].placementStatus() != DEFI_COMPONENT_UNPLACED ) {
        // Follow original orient 
        orient = ckt.defComponentStor[cIdx].placementOrient(); 
      }
      else {
        // Default is North
        orient = 0;
      }

      if( env.isWestFix ) {
        orient = 1;
      }

      ckt.defComponentStor[cIdx].
        setPlacementLocation(lx, ly, orient);

      ckt.defComponentStor[cIdx].
        setPlacementStatus(DEFI_COMPONENT_FIXED);
    }

    // reset other cells's location not to have bug in other tools
    for(size_t i=0; i<ckt.defComponentStor.size(); i++) {
      // continue for MacroIndex
      if( macroIdxMap.find(i) != macroIdxMap.end() ) {
        continue;
      }
      
      ckt.defComponentStor[i].setPlacementStatus(0);
      ckt.defComponentStor[i].setPlacementLocation(-1,-1,-1);
    }

    // check plotting
    if( env.isPlot ) {
      _mckt.Plot(env.output + "_best.plt", bestSet );
    }

    // top-level layout print
    FILE* fp = 
      fopen(string(env.output + "_best.def").c_str(), "w"); 

    if( fp == NULL) { 
      cout << "ERROR: cannot open " << env.output << " to write output file" << endl;
      exit(1);
    }

    ckt.WriteDef( fp );
    fclose(fp);
  }
   
  

  /*
  // Writing functions for Sets file
  FILE* fp = fopen(_env.output.c_str(), "w");
  if( fp == NULL) { 
    cout << "ERROR: cannot open " << _env.output << " to write output file" << endl;
    exit(1);
  } 

  // Generate *.sets file
  PrintAllSets(fp, _cinfo, allSets);
  fclose(fp);
  */



  // ParquetFormat print
  // only top-level formats
//  layout.PrintParquetFormat(_env.output);

  cout << "PROC: End TritonMacroPlacer" << endl;


  // All Edges... 
//  for(int i=1; i<=numEdge; i++) {
//    sta::Edge* edge = _sta->graph()->edge(i);
//    cout << edge->from(_sta->graph())->name(_sta->network()) << 
//      " -> " << edge->to(_sta->graph())->name(_sta->network()) << endl;
//  }


//  PinSeq pinStor;
//  pinStor.push_back( );

//  for(auto& curSet : allSets) {
//    for(auto& curPart : curSet) {
//      for(auto& curMacro : curPart.macroStor) {
//        cout << curMacro.name << endl;
//        Pin* curPin = _sta->network()->findPin(curMacro.name.c_str()); 
//      }
//    }
//  } 
  
  /* 
  // vertex 
  sta::VertexIterator vertIter(_sta->graph());
  while( vertIter.hasNext()) {
    sta::Vertex* vertex = vertIter.next();
//    if( vertex->isRoot() ) {
//     if ( vertex->isCheckClk() || vertex->isGatedClkEnable() || vertex->isRegClk() ) {
//    if( _sta->network()->isTopLevelPort(vertex->pin())) {
//      cout << vertex->name(_sta->network()) << endl;
//    }
  }

  for(int i=1; i<=numEdge; i++) {
    sta::Edge* edge = _sta->graph()->edge(i);
//    cout << edge->from(_sta->graph())->name(_sta->network()) << 
//      " -> " << edge->to(_sta->graph())->name(_sta->network()) << endl;
  }

  LeafInstanceIterator* leafInstanceIter = _sta->network()->leafInstanceIterator();
  while( leafInstanceIter->hasNext() ) {
    Instance* instance = leafInstanceIter->next();
//    cout << _sta->network()->name(instance) << "/";
    InstancePinIterator* instancePinIter = _sta->network()->pinIterator(instance);
    while( instancePinIter->hasNext()) {
      sta::Pin* pin = instancePinIter->next();
      if( _sta->network()->isDriver(pin) ) {
        cout << "D: ";
      }
      else if( _sta->network()->isLoad(pin) ) {
        cout << "L: ";
      }
      cout << _sta->network()->name(pin) << endl ;
    }
    cout <<endl;
  }
  */
  
//  cout << "top: " << _sta->network()->name(_sta->network()->topInstance()) << endl;
}


// Two partitioning functions:
// first : lower part
// second : upper part
// 
// cutLine is sweeping from lower to upper coordinates in x / y
vector<pair<Partition, Partition>> GetPart(
    CircuitInfo &cInfo,  
    Partition& partition, 
    bool isHorizontal ) {
  cout << "PROC: Begin Partition ... " << endl;
  cout << "INFO: Partitions' #Macro = " << partition.macroStor.size() << endl;


  // Return vector
  vector<pair<Partition, Partition>> ret;
  
  typedef std::set<int> MacroSetT;
  interval_map<double, MacroSetT> macroMap;

  double maxWidth = DBL_MIN;
  double maxHeight = DBL_MIN;

  // in parent partition, traverse macros
  //
  for(auto& curMacro: partition.macroStor) {
    MacroSetT macroInfo;
    macroInfo.insert( &curMacro - &partition.macroStor[0]);
    
    macroMap.add( 
        make_pair(
          interval<double>::closed(
            (isHorizontal)? curMacro.lx : curMacro.ly,
            (isHorizontal)? curMacro.lx + curMacro.w : 
                curMacro.ly + curMacro.h ), macroInfo));
    maxWidth = ( maxWidth < curMacro.w)? curMacro.w: maxWidth;
    maxHeight = ( maxHeight < curMacro.h)? curMacro.h: maxHeight;
  }

  double cutLineLimit = (isHorizontal)? maxWidth * 0.25 : maxHeight * 0.25;


  double prevPushLimit = DBL_MIN;
  bool isFirst = true;
  vector<double> cutLineStor;
 
  // less than 4
  if( partition.macroStor.size() <= 4 ) {
    for( interval_map<double, MacroSetT>::iterator 
        it = macroMap.begin();
        it != macroMap.end(); *it++) {
      interval<double>::type macroInterval = it->first;
      MacroSetT result = it->second;

      if( isFirst ) {
        cutLineStor.push_back( macroInterval.lower() );
        prevPushLimit = macroInterval.lower();
        isFirst = false;
      }
      else if( abs(macroInterval.lower()-prevPushLimit) > cutLineLimit ) {
        cutLineStor.push_back( macroInterval.lower() );
        prevPushLimit = macroInterval.lower();
      }
    }
  }
  // more than 4
  else {
    int hardLimit = int( sqrt( 1.0*partition.macroStor.size()/3.0 ) + 0.5f);
    for(int i=0; i<=hardLimit; i++) {
      cutLineStor.push_back( (isHorizontal)? 
          cInfo.lx + 1.0*(cInfo.ux - cInfo.lx)/hardLimit * i :
          cInfo.ly + 1.0*(cInfo.uy - cInfo.ly)/hardLimit * i );
    }
  }
  cout << "INFO: NumCutline = " << cutLineStor.size() << endl;
  
  // Macro checker array
  // 0 for uninitialize
  // 1 for lower
  // 2 for upper
  // 3 for both
  int* chkArr = new int[partition.macroStor.size()];
  
  for(auto& cutLine : cutLineStor ) {
    cout << "INFO: CutLine = " << cutLine << endl;
    CutRoundUp(cInfo, cutLine, isHorizontal);
    cout << "INFO: Updated CutLine = " << cutLine << endl;

   
    // chkArr initialize 
    for(size_t i=0; i<partition.macroStor.size(); i++) {
      chkArr[i] = 0;
    }
  
    bool isImpossible = false;
    for(auto& curMacro : partition.macroStor) {
      int i = &curMacro - &partition.macroStor[0];
      if( isHorizontal ) {
        // lower is possible
        if( curMacro.w <= cutLine ) {
          chkArr[i] += 1;
        }
        // upper is possible
        if ( curMacro.w <= partition.width - cutLine) {
          chkArr[i] += 2; 
        }
        // none of them
        if( chkArr[i] == 0 ) {
          isImpossible = true;
          break;
        }
      }
      else {
        // lower is possible
        if( curMacro.h <= cutLine ) {
          chkArr[i] += 1;
        }
        // upper is possible
        if (curMacro.h <= partition.height - cutLine) {
          chkArr[i] += 2;
        }
        // none of 
        if( chkArr[i] == 0 ) {
          isImpossible = true;
          break;
        }
      }
    } 
    // impossible cuts, then skip 
    if( isImpossible ) {
      continue;
    }

    // Fill in the Partitioning information
    PartClass lClass = None, uClass = None;
    if( partition.partClass == ALL ) {
      lClass = (isHorizontal)? W : S;
      uClass = (isHorizontal)? E : N;
    }

    if( partition.partClass == W) {
      lClass = SW;
      uClass = NW;
    
    }
    if( partition.partClass == E) {
      lClass = SE;
      uClass = NE;
    }

    if( partition.partClass == N) {
      lClass = NW;
      uClass = NE;
    }

    if( partition.partClass == S) {
      lClass = SW;
      uClass = SE;
    }

    Partition lowerPart( lClass, 
      partition.lx, 
      partition.ly, 
      (isHorizontal)? cutLine - partition.lx : partition.width, 
      (isHorizontal)? partition.height : cutLine - partition.ly); 

    Partition upperPart( uClass, 
      (isHorizontal)? cutLine : partition.lx, 
      (isHorizontal)? partition.ly :cutLine,
      (isHorizontal)? partition.lx + partition.width - cutLine : partition.width, 
      (isHorizontal)? partition.height : partition.ly + partition.height - cutLine);



    // cout << it->first << " " << it->second << endl;
    //
    // Fill in child partitons' macroStor
    for(auto& curMacro : partition.macroStor) {
      int i=&curMacro - &partition.macroStor[0];
      if( chkArr[i] == 1 ) {
        lowerPart.macroStor.push_back( 
            Macro( curMacro.name, curMacro.type,
              curMacro.lx, curMacro.ly,
              curMacro.w, curMacro.h,
              curMacro.haloX, curMacro.haloY,
              curMacro.channelX, curMacro.channelY, 
              curMacro.ptr, curMacro.instPtr )) ; 
      }
      else if( chkArr[i] == 2 ) {
        upperPart.macroStor.push_back(
            Macro( curMacro.name, curMacro.type,
              (isHorizontal)? curMacro.lx-cutLine : curMacro.lx, 
              (isHorizontal)? curMacro.ly : curMacro.ly-cutLine,
              curMacro.w, curMacro.h,
              curMacro.haloX, curMacro.haloY,
              curMacro.channelX, curMacro.channelY, 
              curMacro.ptr, curMacro.instPtr));
      }
      else if( chkArr[i] == 3 ) {
        double centerPoint = 
          (isHorizontal)? 
          curMacro.lx + curMacro.w/2.0 :
          curMacro.ly + curMacro.h/2.0;

        if( centerPoint < cutLine ) {
          lowerPart.macroStor.push_back( 
              Macro( curMacro.name, curMacro.type,
                curMacro.lx, curMacro.ly,
                curMacro.w, curMacro.h,
                curMacro.haloX, curMacro.haloY,
                curMacro.channelX, curMacro.channelY, 
                curMacro.ptr, curMacro.instPtr )) ; 
        
        }
        else {
          upperPart.macroStor.push_back(
              Macro( curMacro.name, curMacro.type,
                (isHorizontal)? curMacro.lx-cutLine : curMacro.lx, 
                (isHorizontal)? curMacro.ly : curMacro.ly-cutLine,
                curMacro.w, curMacro.h,
                curMacro.haloX, curMacro.haloY,
                curMacro.channelX, curMacro.channelY, 
                curMacro.ptr, curMacro.instPtr));
        }
      }
    }
    
    double lowerArea = lowerPart.width * lowerPart.height;
    double upperArea = upperPart.width * upperPart.height;

    double upperMacroArea = 0.0f;
    double lowerMacroArea = 0.0f;

    for(auto& curMacro : upperPart.macroStor) {
      upperMacroArea += curMacro.w * curMacro.h;
    } 
    for(auto& curMacro : lowerPart.macroStor) {
      lowerMacroArea += curMacro.w * curMacro.h;
    }
//    cout << upperMacroArea << " " << upperArea << endl;
//    cout << lowerMacroArea << " " << lowerArea << endl;
    // impossible partitioning
    if( upperMacroArea > upperArea || lowerMacroArea > lowerArea) {
      cout << "PROC: Impossible partition, continue" << endl;
      continue;
    }

// Previous code
//    if( partition.partClass == N || 
//        partition.partClass == E ||
//        partition.partClass == W ||
//        partition.partClass == S ) {
//      lowerPart.FillNetlistTable();
//      upperPart.FillNetlistTable();
//    }
    
    pair<Partition, Partition> curPart( lowerPart, upperPart );

    cout << "INFO: NumMacro in LowerPart[" << ret.size() << "] = " 
      << lowerPart.macroStor.size() << endl;
    cout << "INFO: NumMacro in UpperPart[" << ret.size() << "] = " 
      << upperPart.macroStor.size() << endl;
    
    ret.push_back( curPart );
  }
  delete[] chkArr;
  cout << "PROC: End Partition " << endl << endl;
  
  return ret; 
}

void UpdateMacroPartInfo( 
    MacroCircuit& _mckt,
    MacroNetlist::Partition& part, 
    unordered_map<MacroNetlist::PartClass, vector<int>, 
    MyHash<MacroNetlist::PartClass>>& macroPartMap ) {

  auto mpPtr = macroPartMap.find( part.partClass );
  if( mpPtr == macroPartMap.end() ) {
    vector<int> curMacroStor;
    // convert macro Information into macroIdx
    for(auto& curMacro: part.macroStor) {
      auto miPtr = _mckt.macroInstMap.find( curMacro.instPtr );
      if( miPtr == _mckt.macroInstMap.end() ) {
        cout << "ERROR: macro " << curMacro.name 
          << " not exists in macroInstMap: " << curMacro.instPtr << endl;
        exit(1);
      }
      curMacroStor.push_back( miPtr->second) ;
    }
    macroPartMap[ part.partClass ] = curMacroStor; 
  }
  else {
    cout << "ERROR: Partition- " << part.partClass 
      << " already updated (UpdateMacroPartInfo)" << endl; 
    exit(1);
  }
}
