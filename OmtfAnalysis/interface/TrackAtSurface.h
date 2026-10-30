#ifndef L1RpcTriggerAnalysis_TrackAtSurface_H
#define L1RpcTriggerAnalysis_TrackAtSurface_H

#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "Geometry/CommonTopologies/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"


namespace edm {class Event; class EventSetup; }
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

class TrackAtSurface {
public:
  TrackAtSurface(){}
  TrackAtSurface(edm::ConsumesCollector cColl);

//  //use that if possible
//  void prepare( const reco::Muon* mu, const edm::Event &ev, const edm::EventSetup &es);

//  //note: if possible use the constructor with muon (here no access to trajectory) 
//  void prepare( const TrajectoryStateOnSurface &state, const edm::Event &ev, const edm::EventSetup &es);

  TrajectoryStateOnSurface atStation2( const reco::Muon* mu, const edm::Event &ev, const edm::EventSetup &es);
  TrajectoryStateOnSurface atDetFromClose(const reco::Muon* mu, const RPCDetId& rpcDet,  const GlobalPoint& point, const edm::EventSetup &es) const;

//  TrajectoryStateOnSurface atDetFromTrack( const RPCDetId& rpcDet) const;
//  TrajectoryStateOnSurface atPoint(double eta, double phi) const;
//  TrajectoryStateOnSurface atPoint( const GlobalPoint& point) const;

  //note: propagation "Along from tsos"
//  TrajectoryStateOnSurface atStation2( float eta) const;      
private:
   const edm::ESGetToken<GlobalTrackingGeometry, GlobalTrackingGeometryRecord> theGeomteryToken;
   const edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> theFieldToken;
   const edm::ESGetToken<Propagator, TrackingComponentsRecord> thePropagatorAnyToken;
private:
//   Trajectory               theTrajectory;
//   TrajectoryStateOnSurface theState; 
//   const edm::Event & theEv;
//   const edm::EventSetup & theEs;
};
#endif
