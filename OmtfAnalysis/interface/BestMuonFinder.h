#ifndef L1RpcTriggerAnalysis_BestMuonFinder_H
#define L1RpcTriggerAnalysis_BestMuonFinder_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/RunID.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "UserCode/OmtfDataFormats/interface/MuonObj.h"
#include "UserCode/OmtfAnalysis/interface/TrackAtSurface.h"

namespace edm { class Event; class EventSetup; }
#include "DataFormats/MuonReco/interface/MuonFwd.h"
namespace edm { class EDAnalyzer; }

#include <vector>

class TH1D;
class TH2D;
class TObjArray;

class BestMuonFinder {

public:
  BestMuonFinder( const edm::ParameterSet& cfg, edm::ConsumesCollector cColl);
 
  const reco::Muon* result( const edm::Event &ev, const edm::EventSetup &es) { run(ev,es); return theMuon; }
  std::vector<MuonObj> muons( const edm::Event &ev, const edm::EventSetup &es) { run(ev,es); return theMuonObjs; }
  bool isUnique( const edm::Event &ev, const edm::EventSetup &es) { run(ev,es); return theUnique;}
  unsigned int numberOfAllMuons( const edm::Event &ev, const edm::EventSetup &es) { run(ev,es); return theAllMuons; }
  void initHistos( TObjArray & histos);

  unsigned int numberOfValidMuonRPCHits() const { return theRPCHits; }
  unsigned int numberOfValidMuonDTHits()  const { return theDTHits; }
  unsigned int numberOfValidMuonCSCHits() const { return theCSCHits; }
  unsigned int numberOfValidTrackerHits() const { return theTrackerHits; } 
  bool isLoose() const { return theIsLoose; }
  bool isMedium() const { return theIsMedium; }
  bool isTight() const { return theIsTight; }

private:
  bool run(const edm::Event &ev, const edm::EventSetup &es);

private:
  edm::EventNumber_t lastEvent;
  edm::RunNumber_t   lastRun;
  edm::ParameterSet  theConfig;

  TrackAtSurface muAtSurface;

  bool theUnique, theIsLoose, theIsMedium, theIsTight;
  unsigned int theAllMuons;
  const reco::Muon* theMuon;
  std::vector<MuonObj> theMuonObjs; 

  unsigned int theTrackerHits, theRPCHits, theDTHits, theCSCHits;

  TH1D *hMuChi2Tk, *hMuChi2Gl, *hMuNHitsTk ;
  TH2D *hMuPtVsEta, *hMuHitsRPCvsCSC, *hMuHitsRPCvsDT; 
  TH1D *hMuonPt_BMF, *hMuonEta_BMF, *hMuonPhi_BMF;

};
#endif
