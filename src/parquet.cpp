//#include "allparquet.h"
#include "Parquet.h"
#include "mixedpackingfromdb.h"
#include "btreeanneal.h"
#include "parse.h"

using MacroNetlist::Partition;

// Call ParquetFP
void Partition::DoAnneal() {
  // No macro, no need to execute
  if( macroStor.size() == 0 ) {
    return;
  }
  
  cout << "Parquet is starting... " << endl;
  
  
  // Preprocessing in macroPlacer side
  // For nets and wts 
  vector< pair<int, int> > netStor;
  vector<int> costStor;

  netStor.reserve( (macroStor.size()+4)*(macroStor.size()+3)/2 );
  costStor.reserve( (macroStor.size()+4)*(macroStor.size()+3)/2 );
  for(int i=0; i<macroStor.size()+4; i++) {
    for(int j=i+1; j<macroStor.size()+4; j++) {
      int cost = netTable[ i*(macroStor.size()+4) + j] + 
        netTable[ j*(macroStor.size()+4) + i ];
      if( cost != 0 ) {
        netStor.push_back( std::make_pair( std::min(i,j), std::max(i,j) ) );
        costStor.push_back(cost);
      }
    }
  }

  using namespace parquetfp;
  using uofm::string;
  using uofm::vector;

  // Populating DB structure
  // Instantiate Parquet DB structure
  DB db;
  Nodes* nodes = db.getNodes();
  Nets* nets = db.getNets();


  //////////////////////////////////////////////////////
  // Feed node structure: macro Info
  for(auto& curMacro : macroStor) {
  
    double padMacroWidth = curMacro.w + 2*(curMacro.haloX + curMacro.channelX);
    double padMacroHeight = curMacro.h + 2*(curMacro.haloY + curMacro.channelY);
//    cout << curMacro.w << " -> " << padMacroWidth << endl;
//    exit(1);

    Node tmpMacro ( curMacro.name.c_str() , padMacroWidth * padMacroHeight, 
        padMacroWidth/padMacroHeight, padMacroWidth/padMacroHeight,
        &curMacro - &macroStor[0], false);

    tmpMacro.addSubBlockIndex(&curMacro - &macroStor[0]);

    // TODO
    // tmpMacro.putSnapX();
    // tmpMacro.putHaloX();
    // tmpMacro.putChannelX();

    nodes->putNewNode(tmpMacro);
  }

  // Feed node structure: terminal Info
  int indexTerm = 0;
  string pinNames[4] = {"West", "East", "North", "South"};
  double posX[4] = {0.0,         width,      width/2.0,  width/2.0};
  double posY[4] = {height/2.0,  height/2.0, height,     0.0f };
  for(int i=0; i<4; i++) {
    Node tmpPin(pinNames[i], 0, 1, 1, indexTerm++, true); 
    tmpPin.putX(posX[i]);
    tmpPin.putY(posY[i]);
    nodes->putNewTerm(tmpPin);
  }

  
  //////////////////////////////////////////////////////
  // Feed net / weight structure
  for(auto& curNet : netStor) {
    int idx = &curNet - &netStor[0];
    Net tmpEdge;

    parquetfp::pin tempPin1( GetName(curNet.first).c_str(), true, 0, 0, idx );
    parquetfp::pin tempPin2( GetName(curNet.second).c_str(), true, 0, 0, idx );

    tmpEdge.addNode(tempPin1);
    tmpEdge.addNode(tempPin2);
    tmpEdge.putIndex(idx);
    tmpEdge.putName(std::string("n"+std::to_string(idx)).c_str());
    tmpEdge.putWeight(costStor[idx]);

    nets->putNewNet(tmpEdge);
  }

  nets->updateNodeInfo(*nodes);
  nodes->updatePinsInfo(*nets);


  // Populate MixedBlockInfoType object
  // It is from DB object
  MixedBlockInfoTypeFromDB dbBlockInfo(db);
  MixedBlockInfoType* blockInfo = reinterpret_cast<MixedBlockInfoType*> (&dbBlockInfo);
 
  // Command_Line object populate
  Command_Line param;
  param.minWL = true;
  param.noRotation = true;
  param.FPrep = "BTree";
  param.seed = 100;
  param.scaleTerms = false;

  // Fixed-outline mode in Parquet
  param.nonTrivialOutline = parquetfp::BBox(0, 0, width, height);
  param.reqdAR = width/height;
  param.maxWS = 0;

  // Instantiate BTreeAnnealer Object
  BTreeAreaWireAnnealer* annealer = 
    new BTreeAreaWireAnnealer(*blockInfo, const_cast<Command_Line*>(&param), &db);

  annealer->go();
  delete annealer;

  // 
  // flip info initialization for each partition
  bool isFlipX = false, isFlipY = false;
  switch(partClass) {
    // y flip
    case NW:
      isFlipY = true;
      break;
    // x, y flip
    case NE:
      isFlipX = isFlipY = true; 
      break;
    // NonFlip 
    case SW:
      break;
    // x flip
    case SE: 
      isFlipX = true;
      break;
    // very weird
    default: 
      break;
  }

  // update back into macroPlacer 
  for(int i=0; i<nodes->getNumNodes(); i++) {
    Node& curNode = nodes->getNode(i);

    double macroLx = 0, macroLy = 0;
    
    macroStor[i].lx = (isFlipX)? 
      width - curNode.getX() - curNode.getWidth() + lx :
      curNode.getX() + lx;
    macroStor[i].ly = (isFlipY)?
      height - curNode.getY() - curNode.getHeight() + ly :
      curNode.getY() + ly;

    macroStor[i].lx += (macroStor[i].haloX + macroStor[i].channelX);
    macroStor[i].ly += (macroStor[i].haloY + macroStor[i].channelY); 
  }

//  db.plot( "out.plt", 0, 0, 0, 0, 0,
//      0, 1, 1,  // slack, net, name 
//      true, 
//      0, 0, width, height);

  cout << "Done" << endl; 

}
