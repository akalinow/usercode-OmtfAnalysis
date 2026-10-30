#include "UserCode/OmtfAnalysis/interface/L1ObjMaker.h"

#include <charconv>
#include <cmath>
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
#include "DataFormats/L1TMuonPhase2/interface/SAMuon.h"
#include "DataFormats/L1TMuonPhase2/interface/EMTFTrack.h"
#include "DataFormats/L1TMuonPhase2/interface/Constants.h"
#include "L1Trigger/L1TMuonEndCapPhase2/interface/Utils/TPUtils.h"

using namespace std;
namespace {
  edm::EDGetTokenT<l1t::RegionalMuonCandBxCollection> theOmtfEmulToken, theOmtfDataToken, theEmtfDataToken, theBmtfDataToken;
  edm::EDGetTokenT<l1t::phase2::EMTFTrackCollection> theEmtfPhase2DataToken;
  edm::EDGetTokenT<l1t::MuonBxCollection> theGmtDataToken, theGmtEmulToken;
  edm::EDGetTokenT<l1t::TrackerMuonCollection> theGmtPhase2DefaultToken;
  edm::EDGetTokenT<l1t::TrackerMuonCollection> theGmtPhase2ExtendedToken;
  edm::EDGetTokenT<std::vector<l1t::SAMuon>> theSAMToken;
  edm::EDGetTokenT<std::vector<l1t::SAMuon>> theDisplacedSAMToken;
}

L1ObjMaker::L1ObjMaker(const  edm::ParameterSet & cfg, edm::ConsumesCollector&& cColl)
  :  theConfig(cfg),
     lastEvent(0),lastRun(0)
{
  if (theConfig.exists("omtfEmulSrc")) theOmtfEmulToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("omtfEmulSrc") );
  if (theConfig.exists("omtfDataSrc")) theOmtfDataToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("omtfDataSrc") );
  if (theConfig.exists("bmtfDataSrc")) theBmtfDataToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("bmtfDataSrc") );
  if (theConfig.exists("emtfDataSrc")) theEmtfDataToken =  cColl.consumes<l1t::RegionalMuonCandBxCollection>(  theConfig.getParameter<edm::InputTag>("emtfDataSrc") );
  if (theConfig.exists("emtfPhase2DataSrc")) theEmtfPhase2DataToken = cColl.consumes<l1t::phase2::EMTFTrackCollection>(theConfig.getParameter<edm::InputTag>("emtfPhase2DataSrc"));
  if (theConfig.exists("gmtDataSrc"))  theGmtDataToken  =  cColl.consumes<l1t::MuonBxCollection>( theConfig.getParameter<edm::InputTag>("gmtDataSrc") );
  if (theConfig.exists("gmtEmulSrc"))  theGmtEmulToken  =  cColl.consumes<l1t::MuonBxCollection>( theConfig.getParameter<edm::InputTag>("gmtEmulSrc") );
  if (theConfig.exists("gmtPhase2DefaultSrc"))  theGmtPhase2DefaultToken  =  cColl.consumes<l1t::TrackerMuonCollection>(theConfig.getParameter<edm::InputTag>("gmtPhase2DefaultSrc"));
  if (theConfig.exists("gmtPhase2ExtendedSrc"))  theGmtPhase2ExtendedToken  =  cColl.consumes<l1t::TrackerMuonCollection>(theConfig.getParameter<edm::InputTag>("gmtPhase2ExtendedSrc"));
  if (theConfig.exists("samSrc"))  theSAMToken  =  cColl.consumes<std::vector<l1t::SAMuon>>(theConfig.getParameter<edm::InputTag>("samSrc"));
  if (theConfig.exists("displacedSamSrc"))  theDisplacedSAMToken  =  cColl.consumes<std::vector<l1t::SAMuon>>(theConfig.getParameter<edm::InputTag>("displacedSamSrc"));
  
}

void L1ObjMaker::run(const edm::Event &ev)
{
  if (lastEvent==ev.id().event() && lastRun==ev.run()) return;
  lastEvent = ev.id().event() ;
  lastRun = ev.run();
  theL1Objs.clear();

  if (!theOmtfDataToken.isUninitialized())  makeRegCandidates(ev, L1Obj::OMTF    , theL1Objs);
  if (!theOmtfEmulToken.isUninitialized())  makeRegCandidates(ev, L1Obj::OMTF_emu, theL1Objs);
  if (!theBmtfDataToken.isUninitialized())  makeRegCandidates(ev, L1Obj::BMTF    , theL1Objs);
  if (!theEmtfDataToken.isUninitialized())  makeRegCandidates(ev, L1Obj::EMTF    , theL1Objs);
  if (!theEmtfPhase2DataToken.isUninitialized()) makeEmtfPhase2Candidates(ev, L1Obj::EMTFPhase2, theL1Objs);
  if ( !theGmtDataToken.isUninitialized())  makeGmtCandidates(ev, L1Obj::uGMT    , theL1Objs);
  if ( !theGmtEmulToken.isUninitialized())  makeGmtCandidates(ev, L1Obj::uGMT_emu, theL1Objs);
  if (!theGmtPhase2DefaultToken.isUninitialized())  makeGmtPhase2Candidates(ev, L1Obj::uGMTPhase2Default, theL1Objs);
  if (!theGmtPhase2ExtendedToken.isUninitialized())  makeGmtPhase2Candidates(ev, L1Obj::uGMTPhase2Extended, theL1Objs);
  if (!theSAMToken.isUninitialized())  makeSAMCandidates(ev, L1Obj::SAMuon, theL1Objs);
  if (!theDisplacedSAMToken.isUninitialized())  makeSAMCandidates(ev, L1Obj::DisplacedSAMuon, theL1Objs);

  //Add a dummy L1Obj to avoid empty vector problems downstream
  if (theL1Objs.size() == 0) {
    L1Obj obj;
    obj.type = L1Obj::NONE;
    theL1Objs.push_back(obj); 
  }
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

    //std::cout<<"GMT: " << obj << std::endl;
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
    // if (type == 10){
    //   std::cout << "OMTF: " << std::endl;
    //   std::cout << "pt: " << (obj.pt - 1) / 2.0 << " GeV" << std::endl;
    //   std::cout << "eta: " << obj.eta / 240.0 * 2.61 << std::endl;
    //   std::cout << "phi: " << ((15.0 + obj.iProcessor * 60.0) / 360.0 + obj.phi / 576.0) * 2 * M_PI << " rad" << std::endl;
    //   std::cout << "hwPt: " << obj.pt << std::endl;
    //   std::cout << "hwEta: " << obj.eta << std::endl;
    //   std::cout << "hwPhi: " << obj.phi << std::endl;

    // }
//std::cout<<"Regional: " << obj << std::endl;
  }    


  }

  return true;
}

bool L1ObjMaker::makeEmtfPhase2Candidates(const edm::Event &iEvent, L1Obj::TYPE type, std::vector<L1Obj> &result)
{
  edm::Handle<l1t::phase2::EMTFTrackCollection> candidates;
  iEvent.getByToken(theEmtfPhase2DataToken, candidates);

  for (const auto &track : *candidates) {
    L1Obj obj;
    obj.type = type;
    obj.iProcessor = track.sector() - 1;
    obj.position = track.endcap() == 1 ? 1 : -1;

    const double localPhi =
        emtf::phase2::tp::calcPhiLocRadFromInt(track.modelPhi());
    const double globalPhi =
        emtf::phase2::tp::calcPhiGlobRadFromLoc(track.sector(), localPhi);
    const double theta = emtf::phase2::tp::calcThetaRadFromInt(track.modelEta());
    const double eta = -std::log(std::tan(theta / 2.)) * track.endcap();
    const double pt = track.emtfPt() * Phase2L1GMT::LSBpt;
    //const double processorPhiOffset = (15. + obj.iProcessor * 60.) / 360.;

    // Store Phase-2 tracks in the legacy EMTF hardware-like coordinates.
    obj.phi = globalPhi;
    obj.eta = eta;
    obj.pt = pt;
    obj.ptUnconstrained = track.unconstrained() ? pt : 0.;
    obj.charge = track.emtfQ();
    obj.q = track.emtfQuality()*track.valid()*(track.emtfRels()==127); //  max relevance score (127) as in Phase2L1TGMTFwdMuonTranslator::ConvertEMTFTrack
    obj.bx = track.bx();
    obj.d0 = track.emtfD0();
    obj.z0 = track.emtfZ0();
    result.push_back(obj);
  }
  return true;
}


bool L1ObjMaker::makeGmtPhase2Candidates(const edm::Event &iEvent,  L1Obj::TYPE type, std::vector<L1Obj> &result)
{  
  edm::Handle<l1t::TrackerMuonCollection> candidates;
  switch (type) {
    case  L1Obj::uGMTPhase2Default    : { iEvent.getByToken(theGmtPhase2DefaultToken, candidates); break; }
    case  L1Obj::uGMTPhase2Extended    : { iEvent.getByToken(theGmtPhase2ExtendedToken, candidates); break; }
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
    obj.q   = aCand.hwQual();                             
    obj.bx = 0;
    obj.charge = aCand.phCharge();
    obj.hwBeta = aCand.hwBeta();
    result.push_back(obj);
  }
  return true; 
}

bool L1ObjMaker::makeSAMCandidates(const edm::Event &iEvent, L1Obj::TYPE type, std::vector<L1Obj> &result)
{
  edm::Handle<std::vector<l1t::SAMuon>> candidates;
  switch (type) {
    case L1Obj::SAMuon: {
      iEvent.getByToken(theSAMToken, candidates);
      break;
    }
    case L1Obj::DisplacedSAMuon: {
      iEvent.getByToken(theDisplacedSAMToken, candidates);
      break;
    }
    default: {
      std::cout << "Invalid type: " << type << std::endl;
      abort();
    }
  }

  for (const auto &aCand : *candidates.product()) {

    L1Obj obj;
    obj.type = type;
    obj.phi = aCand.phPhi();
    obj.eta = aCand.phEta();
    obj.pt = aCand.phPt();
    obj.charge = aCand.phCharge();
    obj.z0 = aCand.phZ0();
    obj.d0 = aCand.phD0();
    obj.hwBeta = aCand.hwBeta();
    obj.commonStubCount = aCand.commonStubCount();
    obj.totalStubQuality = aCand.totalStubQuality();
    obj.totalStubCount = aCand.totalStubCount();
    obj.commonStubQuality = aCand.commonStubQuality();
    obj.q   = aCand.hwQual();                        
    obj.bx = 0;
    result.push_back(obj);
  }
  return true;
}