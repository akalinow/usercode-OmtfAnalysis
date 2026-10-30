#ifndef OmtfAnalysis_ClosestTrackFinder_H
#define OmtfAnalysis_ClosestTrackFinder_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/RunID.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "UserCode/OmtfDataFormats/interface/MuonObj.h"

namespace edm { class Event; class EventSetup; }
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include <vector>

class TH1D;
class TH2D;
class TObjArray;

class ClosestTrackFinder {

public:
  ClosestTrackFinder(){}
  ClosestTrackFinder( const edm::ParameterSet& cfg, edm::ConsumesCollector&& cColl);

  reco::Track result(const edm::Event &ev, const edm::EventSetup &es, double eta, double phi);
  void initHistos( TObjArray & histos);

private:
  edm::ParameterSet  theConfig;
};
#endif

