#include "UserCode/OmtfAnalysis/interface/SynchroCheck.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"


#include "Geometry/CommonTopologies/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCChamber.h"
#include "Geometry/CSCGeometry/interface/CSCLayer.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TObjArray.h"
#include "TH1D.h"
#include "TH2D.h"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambPhContainer.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambThContainer.h"

#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include "UserCode/OmtfDataFormats/interface/DetSpecObj.h"
#include "UserCode/OmtfDataFormats/interface/DetBxStatObj.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

SynchroCheck::SynchroCheck(const edm::ParameterSet & cfg, edm::ConsumesCollector cColl)
  : theLctToken( cColl.consumes(cfg.getParameter<edm::InputTag>("srcCSC"))),
    theRpcDigiToken( cColl.consumes(cfg.getParameter<edm::InputTag>("srcRPC"))),
    theDtDigiToken( cColl.consumes(cfg.getParameter<edm::InputTag>("srcDT"))),
    theCscToken( cColl.consumes(edm::InputTag("csc2DRecHits"))),
    theRpcToken( cColl.consumes(edm::InputTag("rpcRecHits"))),
    theGeomteryToken( cColl.esConsumes()), 
    theCscGeomToken( cColl.esConsumes()),
    theRpcGeomToken( cColl.esConsumes()),
    theDtGeomToken( cColl.esConsumes()),
    theFieldToken( cColl.esConsumes()),
    thePropagatorAnyToken( cColl.esConsumes(edm::ESInputTag("","SteppingHelixPropagatorAny")))
{ 
  theDetBxStat = new DetBxStatObj();
  theDetPullXStat = new DetBxStatObj("DetPullXStat");
}

SynchroCheck::~SynchroCheck() { }

void SynchroCheck::initHistos( TObjArray & histos)
{
  hRpcHitDistX = new TH1D("hRpcHitDistX","hRpcHitDistX",100,-100.,100.); histos.Add(hRpcHitDistX);
  hCscHitDistX = new TH1D("hCscHitDistX","hCscHitDistX",100,-100.,100.); histos.Add(hCscHitDistX);

  hRpcHitDistPhi = new TH1D("hRpcHitDistPhi","hRpcDistPhi",180,-M_PI/4.,M_PI/4.); histos.Add(hRpcHitDistPhi);
  hCscHitDistPhi = new TH1D("hCscHitDistPhi","hCscDistPhi",180,-M_PI/4.,M_PI/4.); histos.Add(hCscHitDistPhi);

  hRpcDistX = new TH1D("hRpcDistX","hRpcDistX",100,-25.,25.); histos.Add(hRpcDistX);
  hRpcPullX = new TH1D("hRpcPullX","hRpcPullX",100,-10.,10.); histos.Add(hRpcPullX);
  hCscDistX = new TH1D("hCscDistX","hCscDistX",100,-25.,25.); histos.Add(hCscDistX);
  hCscPullX = new TH1D("hCscPullX","hCscPullX",100,-10.,10.); histos.Add(hCscPullX);

  hME13P_Strip = new TH2D("hME13P_Strip","hME13P_Strip",36,0.5,36.5, 7,-3.5,3.5);  histos.Add(hME13P_Strip);
  hME13N_Strip = new TH2D("hME13N_Strip","hME13N_Strip",36,0.5,36.5, 7,-3.5,3.5);  histos.Add(hME13N_Strip);
  hME13P_Inside = new TH2D("hME13P_Inside","hME13P_Inside",36,0.5,36.5, 7,-3.5,3.5);  histos.Add(hME13P_Inside);
  hME13N_Inside = new TH2D("hME13N_Inside","hME13N_Inside",36,0.5,36.5, 7,-3.5,3.5);  histos.Add(hME13N_Inside);
  hCscDistr = new TH2D("hCscDistr","hCscDistr",100,-100.,100.,120,-120.,120.); histos.Add(hCscDistr);

  hDtBxQ2 = new TH1D("hDtBxQ2","hDtBxQ2",7,-3.5,3.5); histos.Add(hDtBxQ2);
  hDtBxQ4 = new TH1D("hDtBxQ4","hDtBxQ4",7,-3.5,3.5); histos.Add(hDtBxQ4);
  //hDtPhi = new TH2D("hDtPhi","hDtPhi",100,-M_PI,M_PI, 100, -1500.,1500.); histos.Add(hDtPhi);
  hDtPhi = new TH2D("hDtPhi","hDtPhi",90,0.,2*M_PI, 90, 0.,2*M_PI); histos.Add(hDtPhi);
  hDtDPhi = new TH1D("hDtDPhi","hDtDPhi",120,-0.3,0.3); histos.Add(hDtDPhi);

  histos.Add(theDetBxStat);
  histos.Add(theDetPullXStat);
}

void SynchroCheck::checkInside(const reco::Muon *muon, const edm::Event &ev, const edm::EventSetup &es)
{
  if (!muon) return;

  bool debug = 0;

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(muon->track()), globalGeometry, &magField);
  if (!muTSOS.isValid()) return;

  const CSCCorrelatedLCTDigiCollection & digiCollectionCSC =  ev.get(theLctToken);
  for (const auto & chDigis : digiCollectionCSC) {
    if (debug) std::cout <<"--------------"<< std::endl;
    auto rawId = chDigis.first;
    const GeomDet * det = globalGeometry.idToDet(rawId);
    GlobalPoint detPosition = det->position();
    if (deltaR(muon->eta(), muon->phi(), detPosition.eta(), detPosition.phi()) > 1.0) continue;
    if (debug) std::cout <<"det position  r: "<< detPosition.perp()<<", phi: "<<detPosition.phi()<<", z: "<<detPosition.z() << std::endl;
    const Propagator & propagator = es.getData(thePropagatorAnyToken);
//  Plane::PlanePointer surface = Plane::build(det->position(), det->rotation());
    TrajectoryStateOnSurface stateAtDet =  propagator.propagate(muTSOS, det->surface());
    if (!stateAtDet.isValid()) continue;
    bool inside = det->surface().bounds().inside(stateAtDet.localPosition());  
    if (!inside) continue;
    if (debug) std::cout <<" IS INSIDE "<<std::endl;
    CSCDetId cscDetId(rawId);
    if (debug) std::cout <<"CSC DET ID: "<< cscDetId << std::endl;
    for (auto digi = chDigis.second.first; digi != chDigis.second.second; digi++) {
      if (debug) std::cout << *digi << std::endl;
      if (cscDetId.ring()!=3) continue;
      if (cscDetId.station()!=1) std::cout <<"PROBLEM !!!!Unexpected detector: "<<cscDetId<<std::endl;
//      if ( (cscDetId.endcap()==1 && cscDetId.chamber()==2) || (cscDetId.endcap()==2 && cscDetId.chamber()==5) ) std::cout <<" HERE CHAMBER: "<< cscDetId<<", DIGI: " << *digi <<std::endl;
      TH2D* h = (cscDetId.endcap()==1) ? hME13P_Inside : hME13N_Inside;
//      std::cout <<" HERE FILL INSIDE, event:"<<ev.id().event()<<", det: "<<cscDetId<<", digi:"<<*digi<<std::endl; 
      h->Fill(cscDetId.chamber(),digi->getBX()-8);
      hCscDistr->Fill( stateAtDet.localPosition().x(), stateAtDet.localPosition().y());
    }
  }
}

void SynchroCheck::checkHitRpc(const reco::Muon *muon, const edm::Event &ev, const edm::EventSetup &es)
{
  if (!muon) return;

  bool debug = 0;
  if (debug) std::cout << "------------- HERE CHECK RPC --------------" << std::endl;

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(muon->track()), globalGeometry, &magField);
  if (!muTSOS.isValid()) return;

  const RPCRecHitCollection & rpcDetHits = ev.get(theRpcToken);

  for (const auto & hit: rpcDetHits) {
    if (debug) std::cout <<"--------------"<< std::endl;
    RPCDetId rpcDetId = hit.rpcId();
    if (abs(rpcDetId.region()) != 1 || rpcDetId.ring()!=3 || rpcDetId.station() !=1) continue;
    GlobalPoint detPosition = globalGeometry.idToDet(rpcDetId)->position();
    if (deltaR(muon->eta(), muon->phi(), detPosition.eta(), detPosition.phi()) > 1.0) continue;
    if (debug) std::cout <<"RPC DET ID: "<< rpcDetId << std::endl;
    if (debug) std::cout <<"det position  r: "<< detPosition.perp()<<", phi: "<<detPosition.phi()<<", z: "<<detPosition.z() << std::endl;
    const Propagator & propagator = es.getData(thePropagatorAnyToken);
    TrajectoryStateOnSurface trackAtRPC =  propagator.propagate(muTSOS, globalGeometry.idToDet(rpcDetId)->surface());
    if (!trackAtRPC.isValid()) continue;
    if (debug) std::cout <<"RPC Local  hit   position:"<<hit.localPosition()<<std::endl;
    if (debug) std::cout <<"RPC Local  muon  positino:"<<trackAtRPC.localPosition()<<std::endl;
    if (debug) std::cout <<"RPC Global hit   position:"<<globalGeometry.idToDet(rpcDetId)->toGlobal(hit.localPosition())
                                              <<" phi:"<<globalGeometry.idToDet(rpcDetId)->toGlobal(hit.localPosition()).phi() <<std::endl;
    if (debug) std::cout <<"RPC Global muon  positino:"<<trackAtRPC.globalPosition()<<" phi:"<<trackAtRPC.globalPosition().phi()<<std::endl;
    if (debug) std::cout <<"RPC dist: " << trackAtRPC.localPosition().x()-hit.localPosition().x()<<std::endl;
    hRpcHitDistX->Fill(trackAtRPC.localPosition().x()-hit.localPosition().x());
    hRpcHitDistPhi->Fill(deltaPhi( (double)globalGeometry.idToDet(rpcDetId)->toGlobal(hit.localPosition()).phi(), (double)trackAtRPC.globalPosition().phi()));
  }
}

void SynchroCheck::checkHitCsc(const reco::Muon *muon, const edm::Event &ev, const edm::EventSetup &es)
{
  if (!muon) return;

  bool debug = 0;
  if (debug) std::cout << "------------- HERE CHECK CSC --------------" << std::endl;

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(muon->track()), globalGeometry, &magField);
  if (!muTSOS.isValid()) return;

  const CSCRecHit2DCollection& cscDetHits = ev.get(theCscToken);

  for (const auto & hit: cscDetHits) {
    if (debug) std::cout <<"--------------"<< std::endl;
    CSCDetId cscDetId = hit.cscDetId();
    if (cscDetId.ring()!=3) continue;
    if (cscDetId.station()!=1) std::cout <<"PROBLEM !!!!Unexpected detector: "<<cscDetId<<std::endl;
    GlobalPoint detPosition = globalGeometry.idToDet(cscDetId)->position();
    if (deltaR(muon->eta(), muon->phi(), detPosition.eta(), detPosition.phi()) > 1.0) continue;
    if (debug) std::cout <<"CSC DET ID: "<< cscDetId << std::endl;
    if (debug) std::cout <<"det position  r: "<< detPosition.perp()<<", phi: "<<detPosition.phi()<<", z: "<<detPosition.z() << std::endl;
    const Propagator & propagator = es.getData(thePropagatorAnyToken);
    TrajectoryStateOnSurface trackAtCSC =  propagator.propagate(muTSOS, globalGeometry.idToDet(cscDetId)->surface());
    if (!trackAtCSC.isValid()) continue;
    if (debug) std::cout <<"CSC Local  hit   position:"<<hit.localPosition()<<std::endl;
    if (debug) std::cout <<"CSC Local  muon  positino:"<<trackAtCSC.localPosition()<<std::endl;
    if (debug) std::cout <<"CSC Global hit   position:"<<globalGeometry.idToDet(cscDetId)->toGlobal(hit.localPosition())
                                              <<" phi:"<<globalGeometry.idToDet(cscDetId)->toGlobal(hit.localPosition()).phi() <<std::endl;
    if (debug) std::cout <<"CSC Global muon  positino:"<<trackAtCSC.globalPosition()<<" phi:"<<trackAtCSC.globalPosition().phi()<<std::endl;
    if (debug) std::cout <<"CSC dist: " << trackAtCSC.localPosition().x()-hit.localPosition().x()<<std::endl;
    hCscHitDistX->Fill(trackAtCSC.localPosition().x()-hit.localPosition().x());
    hCscHitDistPhi->Fill(deltaPhi((double)globalGeometry.idToDet(cscDetId)->toGlobal(hit.localPosition()).phi(), (double)trackAtCSC.globalPosition().phi()));
  }
}

void SynchroCheck::checkDigiDt(const reco::Muon *muon, const edm::Event &ev, const edm::EventSetup &es)
{
  if (!muon) return;

  bool debug = 0;
  if (debug) std::cout << "------------- HERE SYNCHRO DT--------------" << std::endl;

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & geomDt         = es.getData(theDtGeomToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(muon->track()), globalGeometry, &magField);
  if (!muTSOS.isValid()) return;
  const L1MuDTChambPhContainer & digiCollectionDT =  ev.get(theDtDigiToken);
  if (debug) std::cout <<" #### DTPh digis from BMTF " << digiCollectionDT.getContainer()->size()<< std::endl;
  for (const auto & chDigi : *digiCollectionDT.getContainer()) {
    if (abs(chDigi.whNum()) != 2) continue;
    if (chDigi.stNum() ==4) continue;
    if (chDigi.code()==7) continue;
    DTChamberId dtChamberId(chDigi.whNum(),chDigi.stNum(),chDigi.scNum()+1);
    GlobalPoint detPosition = globalGeometry.idToDet(dtChamberId)->position();
    if (deltaR(muon->eta(), muon->phi(), detPosition.eta(), detPosition.phi()) > 1.0) continue;
    if (debug) std::cout <<"det position  r: "<< detPosition.perp()<<", phi: "<<detPosition.phi()<<", z: "<<detPosition.z() << std::endl;
    const DTChamber * chamber = geomDt.chamber(dtChamberId);
    const Propagator & propagator = es.getData(thePropagatorAnyToken);
    TrajectoryStateOnSurface stateAtLayer =  propagator.propagate(muTSOS, chamber->surface());
    if (!stateAtLayer.isValid()) continue;
    bool inside = chamber->surface().bounds().inside(stateAtLayer.localPosition());
    if (inside && chDigi.code() >=2 ) {
      if (debug) std::cout <<" SegmentPhi: "<<chDigi.phi() 
                <<" global: "<<stateAtLayer.globalPosition().phi()
                <<" local: "<<stateAtLayer.localPosition().phi() << std::endl;
      double phiStub = chDigi.phi()/1024.*M_PI/12+(chDigi.scNum()+0)*M_PI/6;
      if (phiStub<0.) phiStub += 2.*M_PI;
      if (phiStub>2*M_PI) phiStub -= 2.*M_PI;
      double phiGlb = stateAtLayer.globalPosition().phi();
      if (phiGlb <0.) phiGlb += 2*M_PI;
      hDtPhi->Fill(phiGlb, phiStub);
      double deltaPhi = reco::deltaPhi(phiGlb,phiStub);
      hDtDPhi->Fill(deltaPhi);
      if (fabs(deltaPhi) < 0.05) { 
        DetSpecObj det(DetSpecObj::DET::DT,chDigi.whNum(),chDigi.stNum(),chDigi.scNum());
        theDetBxStat->bxStat(det).incrementBx(chDigi.bxNum()); 
        if (chDigi.code() >=2 ) hDtBxQ2->Fill(chDigi.bxNum());
        if (chDigi.code() >=4 ) hDtBxQ4->Fill(chDigi.bxNum());
      }
    }
  }
}

void SynchroCheck::checkStripRpc(const reco::Muon *muon, const edm::Event &ev, const edm::EventSetup &es)
{
  if (!muon) return;

  bool debug = 0;
  if (debug) std::cout << "------------- HERE SYNCHRO RPC --------------" << std::endl;

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & geomRpc        = es.getData(theRpcGeomToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(muon->track()), globalGeometry, &magField);
  if (!muTSOS.isValid()) return;
  const RPCDigiCollection & digiCollectionRPC =  ev.get(theRpcDigiToken);
  for (const auto & chDigis : digiCollectionRPC) {
    if (debug) std::cout <<"--------------"<< std::endl;
    auto rawId = chDigis.first;
    GlobalPoint detPosition = globalGeometry.idToDet(rawId)->position();
    if (deltaR(muon->eta(), muon->phi(), detPosition.eta(), detPosition.phi()) > 1.0) continue;
    if (debug) std::cout <<"det position  r: "<< detPosition.perp()<<", phi: "<<detPosition.phi()<<", z: "<<detPosition.z() << std::endl;

    RPCDetId rpcDetId(rawId);
    if (debug) std::cout <<"RPC DET ID: "<< rpcDetId << std::endl;
    for (auto digi = chDigis.second.first; digi != chDigis.second.second; digi++) {
      int strip = digi->strip();
      if (debug) std::cout << *digi <<" strip: "<< strip <<std::endl;
      const RPCRoll * roll = geomRpc.roll(rpcDetId);
      const Propagator & propagator = es.getData(thePropagatorAnyToken);
      TrajectoryStateOnSurface stateAtLayer =  propagator.propagate(muTSOS, roll->surface());
      if (!stateAtLayer.isValid()) continue;
      LocalPoint stripLocalPosition = roll->centreOfStrip(strip);
      double distX = stateAtLayer.localPosition().x() - stripLocalPosition.x();
      double pullX = distX/sqrt(stateAtLayer.localError().positionError().xx());
      bool inside = roll->surface().bounds().inside(stateAtLayer.localPosition());
      hRpcDistX->Fill(distX);
      hRpcPullX->Fill(pullX);
      if ( inside && fabs(distX) < 10. && fabs(pullX) < 1.) {
        DetSpecObj det =(rpcDetId.region()==0) ? DetSpecObj(DetSpecObj::DET::RPCb, rpcDetId.ring(),                     rpcDetId.station(), rpcDetId.sector()) 
                                               : DetSpecObj(DetSpecObj::DET::RPCe, rpcDetId.region()*rpcDetId.station(), rpcDetId.ring(), (rpcDetId.sector()-1)*6+rpcDetId.subsector());
        theDetBxStat->bxStat(det).incrementBx(digi->bx());
      } 
    }
  }
}
void SynchroCheck::checkStripCsc(const reco::Muon *muon, const edm::Event &ev, const edm::EventSetup &es)
{
  if (!muon) return;

  bool debug = 0;
//  phiCscGlob = -10*M_PI;
  if (debug) std::cout << "------------- HERE SYNCHRO CSC --------------" << std::endl;

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & geomCsc        = es.getData(theCscGeomToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(muon->track()), globalGeometry, &magField);
  if (!muTSOS.isValid()) return;

  const CSCCorrelatedLCTDigiCollection & digiCollectionCSC =  ev.get(theLctToken);
  for (const auto & chDigis : digiCollectionCSC) {
    if (debug) std::cout <<"--------------"<< std::endl;
    auto rawId = chDigis.first;
    GlobalPoint detPosition = globalGeometry.idToDet(rawId)->position();
    if (deltaR(muon->eta(), muon->phi(), detPosition.eta(), detPosition.phi()) > 1.0) continue;
    if (debug) std::cout <<"det position  r: "<< detPosition.perp()<<", phi: "<<detPosition.phi()<<", z: "<<detPosition.z() << std::endl;

    CSCDetId cscDetId(rawId);
    if (debug) std::cout <<"CSC DET ID: "<< cscDetId << std::endl;
    for (auto digi = chDigis.second.first; digi != chDigis.second.second; digi++) {
      if (debug) std::cout << *digi << std::endl;
//      if (cscDetId.ring()!=3) continue;
//      if (cscDetId.station()!=1) std::cout <<"PROBLEM !!!!Unexpected detector: "<<cscDetId<<std::endl;
      int halfStrip = digi->getStrip();
      const CSCChamber * chamber = geomCsc.chamber(cscDetId);
      const CSCLayer * layer = chamber->layer(3);
      const Propagator & propagator = es.getData(thePropagatorAnyToken);
      TrajectoryStateOnSurface stateAtLayer =  propagator.propagate(muTSOS, layer->surface());
      if (!stateAtLayer.isValid()) continue;
      GlobalPoint stripGlobalPosition = layer->centerOfStrip(halfStrip/2);
      LocalPoint   stripLocalPosition = layer->toLocal(stripGlobalPosition);
//      if (phiCscGlob< -9*M_PI) phiCscGlob = stripGlobalPosition.phi(); else phiCscGlob = -5*M_PI;
      double distX = stateAtLayer.localPosition().x() - stripLocalPosition.x();
      double pullX = distX/sqrt(stateAtLayer.localError().positionError().xx());

      hCscDistX->Fill(distX); 
      hCscPullX->Fill(pullX); 

      if (debug) std::cout <<"CSC Local  strip position:"<<stripLocalPosition<<std::endl;
      if (debug) std::cout <<"CSC Local  muon  positino:"<<stateAtLayer.localPosition()<<std::endl;
      if (debug) std::cout <<"CSC Global strip position:"<<stripGlobalPosition<<" phi:"<<stripGlobalPosition.phi()<<std::endl;
      if (debug) std::cout <<"CSC Global muon  positino:"<<stateAtLayer.globalPosition()  <<" phi:"<<stateAtLayer.globalPosition().phi()<<std::endl;

      bool inside = layer->surface().bounds().inside(stateAtLayer.localPosition());
      DetSpecObj::DET type = DetSpecObj::DET::CSC;
      int wd = cscDetId.station()*(-1)*(cscDetId.endcap()*2-3);
      int sr = cscDetId.ring();
      int sc = cscDetId.chamber();
      DetSpecObj detSpec(type, wd, sr, sc);
      if (inside) theDetPullXStat->bxStat(detSpec).incrementBx(pullX);
      if (inside && fabs(distX) < 10. && fabs(pullX) < 1.) {
        theDetBxStat->bxStat(detSpec).incrementBx(digi->getBX()-8);

        if (cscDetId.ring()==3) {
          TH2D* h = (cscDetId.endcap()==1) ? hME13P_Strip : hME13N_Strip;
          if (debug) std::cout <<" HERE FILL STRIP event:"<<ev.id().event()<<" Dist: "<<distX<<std::endl; //", det: "<<cscDetId<<", digi:"<<*digi<<std::endl; 
          h->Fill(cscDetId.chamber(),digi->getBX()-8);
        }
      } 
    }
  }
}



