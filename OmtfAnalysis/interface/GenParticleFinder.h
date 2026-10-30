#ifndef L1RpcTriggerAnalysis_GenParticlefinder_H
#define L1RpcTriggerAnalysis_GenParticlefinder_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/RunID.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "UserCode/OmtfDataFormats/interface/GenObj.h"
#include "UserCode/OmtfDataFormats/interface/GenObjColl.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"


namespace edm { class Event; class EventSetup; }
#include "DataFormats/MuonReco/interface/MuonFwd.h"
namespace edm { class EDAnalyzer; }

#include <vector>

class TObjArray;

class GenParticlefinder {

public:
  GenParticlefinder( const edm::ParameterSet& cfg, edm::ConsumesCollector&& cColl);
 
  const reco::GenParticle* result( const edm::Event &ev, const edm::EventSetup &es) { run(ev,es); return theGenPart; }
  std::vector<GenObj> genparticles( const edm::Event &ev, const edm::EventSetup &es) { run(ev,es); return theGenObjs; }


private:

  void getSimVertex(const edm::Event &ev);
  void getGenParticles(const edm::Event &ev);
  void getTrackingParticles(const edm::Event &ev);
  
  bool run(const edm::Event &ev, const edm::EventSetup &es);

private:
  edm::EventNumber_t lastEvent;
  edm::RunNumber_t   lastRun;
  edm::ParameterSet  theConfig;
  
  unsigned int theAllParticles;
  const reco::GenParticle* theGenPart;
  const SimVertex theSimVertex;
  std::vector<GenObj> theGenObjs; 


};
#endif
