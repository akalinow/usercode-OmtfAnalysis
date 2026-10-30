#ifndef UserCode_L1RpcTriggerAnalysis_SynchroCountsGrabber_H
#define UserCode_L1RpcTriggerAnalysis_SynchroCountsGrabber_H

#include "UserCode/OmtfAnalysis/interface/RPCLinkSynchroStat.h"
#include "CondFormats/RPCObjects/interface/LinkBoardElectronicIndex.h"
#include "DataFormats/RPCDigi/interface/RPCRawSynchro.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "CondFormats/DataRecord/interface/RPCEMapRcd.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "UserCode/OmtfAnalysis/interface/SynchroSelectorMuon.h"
#include "UserCode/OmtfAnalysis/interface/TrackAtSurface.h"

#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/RPCDigi/interface/RPCRawSynchro.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
//#include "UserCode/OmtfAnalysis/interface/TrackAtSurface.h"


namespace edm { class Event; class EventSetup; class Run;}
#include "DataFormats/MuonReco/interface/MuonFwd.h"

class TObjArray;

class SynchroCountsGrabber {
public:
   SynchroCountsGrabber() {}
   SynchroCountsGrabber(const edm::ParameterSet& cfg, edm::ConsumesCollector cColl);
   ~SynchroCountsGrabber();
   RPCRawSynchro::ProdItem counts(const edm::Event &ev, const edm::EventSetup &es);
   RPCRawSynchro::ProdItem counts(const edm::Event &ev, const edm::EventSetup &es, float eta, float phi);
   void setMuon(const reco::Muon *aMuon) { theMuon = aMuon; }
   void initHistos(TObjArray & histos) {theSelector.initHistos(histos); }
private:
  TrackAtSurface trackAtSurface;
  edm::ESWatcher<RPCEMapRcd> theMapWatcher;
  const RPCReadOutMapping * theCabling;
  const reco::Muon * theMuon;
  SynchroSelectorMuon theSelector;
  bool deltaR_MuonToDetUnit_cutoff, checkInside;
  bool theNoSynchroWarning;

//  edm::EDGetTokenT<RPCRawSynchro::ProdItem> rpcRawSynchroProdItemTag_;
};
#endif
