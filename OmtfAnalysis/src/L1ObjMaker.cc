#include "UserCode/OmtfAnalysis/interface/L1ObjMaker.h"

#include <iostream>
#include <sstream>

#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuRegionalCand.h"
#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuGMTCand.h"
#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuGMTExtendedCand.h"
#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuGMTReadoutCollection.h"
#include "L1Trigger/RPCTrigger/interface/RPCConst.h"

#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1TMuonPhase2/interface/TrackerMuon.h"

using namespace std;
namespace {
  edm::EDGetTokenT<l1t::RegionalMuonCandBxCollection> theOmtfEmulToken, theOmtfDataToken, theEmtfDataToken, theBmtfDataToken;
  edm::EDGetTokenT<l1t::MuonBxCollection> theGmtDataToken, theGmtEmulToken;
  edm::EDGetTokenT<l1t::TrackerMuonCollection> theGmtPhase2EmulToken;
}

L1ObjMaker::L1ObjMaker(const  edm::ParameterSet & cfg, edm::ConsumesCollector&& cColl)
  :  theConfig(cfg),
     lastEvent(0),lastRun(0)
{
  if (theConfig.exists("omtfEmulSrc")) theOmtfEmulToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("omtfEmulSrc") );
  if (theConfig.exists("omtfDataSrc")) theOmtfDataToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("omtfDataSrc") );
  if (theConfig.exists("bmtfDataSrc")) theBmtfDataToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("bmtfDataSrc") );
  if (theConfig.exists("emtfDataSrc")) theEmtfDataToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("emtfDataSrc") );
  if (theConfig.exists("gmtDataSrc"))  theGmtDataToken  =  cColl.consumes<l1t::MuonBxCollection>( theConfig.getParameter<edm::InputTag>("gmtDataSrc") );
  if (theConfig.exists("gmtEmulSrc"))  theGmtEmulToken  =  cColl.consumes<l1t::MuonBxCollection>( theConfig.getParameter<edm::InputTag>("gmtEmulSrc") );
  if (theConfig.exists("gmtPhase2EmulSrc"))  theGmtPhase2EmulToken  =  cColl.consumes<l1t::TrackerMuonCollection>(theConfig.getParameter<edm::InputTag>("gmtPhase2EmulSrc"));  
 
}

void L1ObjMaker::run(const edm::Event &ev)
{
  if (lastEvent==ev.id().event() && lastRun==ev.run()) return;
  lastEvent = ev.id().event() ;
  lastRun = ev.run();
  theL1Objs.clear();

  if ( !theGmtDataToken.isUninitialized())  makeGmtCandidates(ev, L1Obj::uGMT    , theL1Objs);
  if ( !theGmtEmulToken.isUninitialized())  makeGmtCandidates(ev, L1Obj::uGMT_emu, theL1Objs);
  if (!theOmtfDataToken.isUninitialized())  makeRegCandidates(ev, L1Obj::OMTF    , theL1Objs);
  if (!theOmtfEmulToken.isUninitialized())  makeRegCandidates(ev, L1Obj::OMTF_emu, theL1Objs);
  if (!theBmtfDataToken.isUninitialized())  makeRegCandidates(ev, L1Obj::BMTF    , theL1Objs);
  if (!theEmtfDataToken.isUninitialized())  makeRegCandidates(ev, L1Obj::EMTF    , theL1Objs);
  if (!theGmtPhase2EmulToken.isUninitialized())  makeGmtPhase2Candidates(ev, L1Obj::uGMTPhase2_emu, theL1Objs);
}

bool L1ObjMaker::makeGmtCandidates(const edm::Event &iEvent,  L1Obj::TYPE type, std::vector<L1Obj> &result)
{
  edm::Handle<l1t::MuonBxCollection> candidates;
  switch (type) {
    case  L1Obj::uGMT     : { iEvent.getByToken(theGmtDataToken, candidates); break; }
    case  L1Obj::uGMT_emu : { iEvent.getByToken(theGmtEmulToken, candidates); break; }
    default: { std::cout <<"Invalid type : " << type << std::endl; abort(); }
  }
  for (int bxNumber=candidates->getFirstBX(); bxNumber<=candidates->getLastBX(); bxNumber++) {
  for (l1t::MuonBxCollection::const_iterator it = candidates.product()->begin(bxNumber);
      it != candidates.product()->end(bxNumber);
      ++it) {
    L1Obj obj;
    obj.type =  type;
    obj.phi = it->hwPhi(); 
    obj.eta = it->hwEta();        // eta = hwEta/240.*2.61
    obj.pt  = it->hwPt();         // pt = (hwPt-1.)/2.
    obj.q   = it->hwQual();                             
    obj.bx = bxNumber;
    obj.charge = it->hwCharge();  // charge  =  pow(-1,hwSign)
    result.push_back(obj);
  }
  }
  return true; 
}

bool L1ObjMaker::makeRegCandidates(const edm::Event &iEvent,  L1Obj::TYPE type, std::vector<L1Obj> &result)
{
  edm::Handle<l1t::RegionalMuonCandBxCollection> candidates;
  switch (type) {
    case  L1Obj::OMTF_emu: { iEvent.getByToken(theOmtfEmulToken, candidates); break; }
    case  L1Obj::OMTF    : { iEvent.getByToken(theOmtfDataToken, candidates); break; }
    case  L1Obj::BMTF    : { iEvent.getByToken(theBmtfDataToken, candidates); break; }
    case  L1Obj::EMTF    : { iEvent.getByToken(theEmtfDataToken, candidates); break; }
    default: { std::cout <<"Invalid type : " << type << std::endl; abort(); }
  }
  for (int bxNumber=candidates->getFirstBX(); bxNumber<=candidates->getLastBX(); bxNumber++) {    
    for (l1t::RegionalMuonCandBxCollection::const_iterator it = candidates.product()->begin(bxNumber);
       it != candidates.product()->end(bxNumber);
       ++it) {

    L1Obj obj;
    obj.type =  type;
    obj.iProcessor = it->processor(); // [0..5]

    obj.position =   (it->trackFinderType() == l1t::omtf_neg || it->trackFinderType() == l1t::emtf_neg) ? -1 :
                   ( (it->trackFinderType() == l1t::omtf_pos || it->trackFinderType() == l1t::emtf_pos) ? +1 : 0 );

    obj.phi = it->hwPhi();  // phi_Rad = [ (15.+processor*60.)/360. + hwPhi/576. ] *2*M_PI
    obj.eta = it->hwEta();  // eta = hwEta/240.*2.61
    obj.pt = it->hwPt();         // pt = (hwPt-1.)/2.
    obj.charge = it->hwSign();   // charge=  pow(-1,hwSign)
    obj.ptUnconstrained =  it->hwPtUnconstrained(); //pt = hwPt -1 (GeV)
    obj.d0 = it->hwDXY(); //(4 bits) 0-75, 75-150, 150-225, 225-inf
    std::map<int, int> hwAddrMap = it->trackAddress();
    obj.q   = it->hwQual();
    obj.hits   = hwAddrMap[0];
    obj.bx = bxNumber;
    obj.refLayer = hwAddrMap[1];    
    obj.disc = hwAddrMap[2];    
    result.push_back(obj);   
  }
  }
  return true;
}


bool L1ObjMaker::makeGmtPhase2Candidates(const edm::Event &iEvent,  L1Obj::TYPE type, std::vector<L1Obj> &result)
{  
  edm::Handle<l1t::TrackerMuonCollection> candidates;
  switch (type) {
    case  L1Obj::uGMTPhase2_emu    : { iEvent.getByToken(theGmtPhase2EmulToken, candidates); break; }    
    default: { std::cout <<"Invalid type : " << type << std::endl; abort(); }
  }

  for(const auto & aCand: *candidates.product()){
    L1Obj obj;
    obj.type =  type;
    obj.z0 = aCand.phZ0();
    obj.d0 = aCand.phD0();
    obj.phi = aCand.phPhi();
    obj.eta = aCand.phEta();
    obj.pt = aCand.phPt();
    obj.q   = 12;                             
    obj.bx = 0;
    obj.charge = aCand.phCharge();
    result.push_back(obj);
  }
  return true; 
}