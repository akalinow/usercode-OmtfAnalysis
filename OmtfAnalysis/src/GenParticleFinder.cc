#include "UserCode/OmtfAnalysis/interface/GenParticleFinder.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"


#include "UserCode/OmtfAnalysis/interface/Utilities.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "TObjArray.h"
#include "TH1D.h"
#include "TH2D.h"

#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

GenParticlefinder::GenParticlefinder(const edm::ParameterSet& cfg, edm::ConsumesCollector&& cColl)
  : lastEvent(0), lastRun(0), theConfig(cfg), 
    theAllParticles(0), theGenPart(0)
{ 

  if (theConfig.exists("genColl")){
    edm::InputTag genCollTag =  theConfig.getParameter<edm::InputTag>("genColl");
    cColl.consumes<reco::GenParticleCollection>(genCollTag);
  }

  if (theConfig.exists("trackingParticle")){
    edm::InputTag trackingPartTag =  theConfig.getParameter<edm::InputTag>("trackingParticle");
    cColl.consumes<TrackingParticleCollection>(trackingPartTag);
  }  
}

void GenParticlefinder::getGenParticles(const edm::Event &ev){

//get Muon
  edm::Handle<reco::GenParticleCollection> genparticles;
  edm::InputTag genCollTag =  theConfig.getParameter<edm::InputTag>("genColl");
  ev.getByLabel( genCollTag, genparticles);
  if (!genparticles.isValid()) return;
  
  
  theAllParticles = genparticles->size();
  
  for (reco::GenParticleCollection::const_iterator im = genparticles->begin(); im != genparticles->end(); ++im) {
    
    
    GenObj genObj(im->pt(),im->eta(),im->phi(),im->mass(),im->charge(),
		  im->pdgId(),im->status(),im->vx(),im->vy(),im->vz(),im->p4().Beta());
    theGenObjs.push_back(genObj);
    // std::cout << genObj << std::endl;
  }  
}

void GenParticlefinder::getTrackingParticles(const edm::Event &ev){

  edm::Handle<TrackingParticleCollection> trackingParticleHandle;
  edm::InputTag trackingPartTag =  theConfig.getParameter<edm::InputTag>("trackingParticle");
  ev.getByLabel(trackingPartTag, trackingParticleHandle);
  if (!trackingParticleHandle.isValid()) return;

  for (TrackingParticleCollection::const_iterator iTP = trackingParticleHandle->begin();
       iTP!=trackingParticleHandle->end();++iTP){

    if(iTP->eventId().bunchCrossing() != 0) continue;
    if (abs(iTP->pdgId()) != 13) continue;
    
    GenObj genObj(iTP->pt(), iTP->eta(), iTP->phi(), iTP->mass(), iTP->charge(),
		  iTP->pdgId(), iTP->status(), iTP->vx(), iTP->vy(), iTP->vz(), iTP->p4().Beta());
    theGenObjs.push_back(genObj);
    std::cout << "beta " << iTP->p4().Beta() << std::endl;
    // std::cout << genObj << std::endl;

  }  
}

bool GenParticlefinder::run(const edm::Event &ev, const edm::EventSetup &es)
{

  if (lastEvent==ev.id().event() && lastRun==ev.run()) return false;
  lastEvent = ev.id().event() ;
  lastRun = ev.run();
  theGenPart = 0;
  theGenObjs.clear();


  if(theConfig.exists("genColl")) getGenParticles(ev);
  if(theConfig.exists("trackingParticle")) getTrackingParticles(ev);
     
  //
  // sort resulting container by pT, in descending order
  //
  std::sort(theGenObjs.begin(), theGenObjs.end(), []( const GenObj &m1, const GenObj &m2) { return m1.pt() > m2.pt(); } );
  
  return true;
}

