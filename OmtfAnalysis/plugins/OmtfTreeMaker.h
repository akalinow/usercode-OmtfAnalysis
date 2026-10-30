#ifndef OmtfTreeMaker_H
#define OmtfTreeMaker_H

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
//#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"

#include "UserCode/OmtfDataFormats/interface/EventObj.h"
#include "UserCode/OmtfDataFormats/interface/TrackObj.h"
#include "UserCode/OmtfDataFormats/interface/MuonObj.h"
#include "UserCode/OmtfDataFormats/interface/MuonObjColl.h"
#include "UserCode/OmtfDataFormats/interface/GenObj.h"
#include "UserCode/OmtfDataFormats/interface/GenObjColl.h"
#include "UserCode/OmtfDataFormats/interface/L1Obj.h"
#include "UserCode/OmtfDataFormats/interface/L1ObjColl.h"

#include "UserCode/OmtfDataFormats/interface/TriggerMenuResultObj.h"
#include "UserCode/OmtfDataFormats/interface/SynchroCountsObjVect.h"

#include "UserCode/OmtfAnalysis/interface/MenuInspector.h"
#include "UserCode/OmtfAnalysis/interface/BestMuonFinder.h"
#include "UserCode/OmtfAnalysis/interface/L1ObjMaker.h"

#include "UserCode/OmtfAnalysis/interface/ClosestTrackFinder.h"
#include "UserCode/OmtfAnalysis/interface/GenParticleFinder.h"
#include "UserCode/OmtfAnalysis/interface/SynchroCheck.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "UserCode/OmtfAnalysis/interface/SynchroCountsGrabber.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"

#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"

#include <vector>
#include "TObjArray.h"

namespace edm { class Event; class EventSetup; }
#include "DataFormats/MuonReco/interface/MuonFwd.h"
 class TTree;
class TFile;

class OmtfTreeMaker : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
//class OmtfTreeMaker : public edm::one::EDAnalyzer<edm::one::SharedResources> {
//class OmtfTreeMaker : public edm::EDAnalyzer {
public:
  explicit OmtfTreeMaker(const edm::ParameterSet& cfg);
  virtual ~OmtfTreeMaker();
  virtual void beginJob();
  virtual void beginRun(const edm::Run &ru, const edm::EventSetup &es);
  virtual void analyze(const edm::Event &ev, const edm::EventSetup &es);
  virtual void endRun(edm::Run const&, edm::EventSetup const&) {}
  virtual void endJob();

  //call back to register consumes. Alternatively possible to declare friend class and call it therein
  //template<typename T> edm::EDGetTokenT<T> initConsumes(const edm::InputTag & t) { return consumes<T>(t); }

private:
  edm::ParameterSet theConfig;
  unsigned int theCounter;

  TFile *theFile;
  TTree *theTree;

  TriggerMenuResultObj *bitsL1;
  TriggerMenuResultObj *bitsHLT;

  EventObj* event;
  MuonObjColl* muonColl;
  L1ObjColl * l1ObjColl;
  GenObjColl * genColl;
  SynchroCountsObjVect * synchroCounts;
  TrackObj* closestTrack;
								    
  TObjArray      theHelper;

  MenuInspector theMenuInspector;
  BestMuonFinder theBestMuonFinder;
  L1ObjMaker theL1ObjMaker;
  GenParticlefinder theGenParticleFinder;
  ClosestTrackFinder theClosestTrackFinder;
  SynchroCheck  theSynchroCheck; 

  edm::EDGetTokenT<edm::SimTrackContainer> inputSim;
  edm::EDGetTokenT<edm::SimVertexContainer> vertexSim;

};
#endif
