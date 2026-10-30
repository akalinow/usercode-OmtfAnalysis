#include "UserCode/OmtfAnalysis/interface/TrackAtSurface.h"

#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"

#include "Geometry/CommonTopologies/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"


#include "DataFormats/GeometrySurface/interface/BoundCylinder.h"
#include "DataFormats/GeometrySurface/interface/SimpleCylinderBounds.h"
#include "DataFormats/GeometrySurface/interface/BoundDisk.h"
#include "DataFormats/GeometrySurface/interface/SimpleDiskBounds.h"

#include "Geometry/CommonTopologies/interface/GeomDet.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"


TrackAtSurface::TrackAtSurface(edm::ConsumesCollector cColl) 
  : theGeomteryToken(cColl.esConsumes()), theFieldToken(cColl.esConsumes()), 
    thePropagatorAnyToken(cColl.esConsumes(edm::ESInputTag("","SteppingHelixPropagatorAny"))) { 

}

/*
void TrackAtSurface::prepare(const TrajectoryStateOnSurface &state, const edm::Event &ev, const edm::EventSetup &es) 
{
  theState = state;
}
*/

//
//
// FIXME sprawdzic jak dziala - moze wystarczy wziac theState i go propagowac?
TrajectoryStateOnSurface TrackAtSurface::atStation2(const reco::Muon* mu,const edm::Event &ev, const edm::EventSetup &es)
{
  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & magField       = es.getData(theFieldToken);

//
// find tracjectory closest to muon
//
//refit global muons, take tracjctory colsest to Mu.
/*
  typedef std::vector<Trajectory> Trajectories;
  edm::Handle<Trajectories> trajectories;
  edm::InputTag refitTag("refittedMuons","Refitted");
  ev.getByLabel(refitTag,trajectories);
  double minDR =  0.3;
  for (Trajectories::const_iterator it=trajectories->begin(); it != trajectories->end(); ++it) {
    //FIXME dlaczego proownuje z innermost state a nie at vertex? Przyblizenie?
    double diff = deltaR(mu->track()->eta(), mu->track()->phi(), it->geometricalInnermostState().globalMomentum().eta(), it->geometricalInnermostState().globalMomentum().phi());
    if (diff < minDR) { minDR = diff; theTrajectory = *it; }
  }
*/

  //
  // find muons TSOS at the end of tracker
  // 
  TrajectoryStateOnSurface theState = trajectoryStateTransform::outerStateOnSurface(*(mu->track()), globalGeometry, &magField);

  double eta = mu->eta();
  double theta = 2.*atan(exp(-eta));
  double rho   = 500.;
  double zet   = 790.;
  if (fabs(eta) > 1.24) rho  = zet * tan(fabs(theta)); else zet = rho/tan(theta);
  GlobalPoint point( GlobalPoint::Polar(theta, mu->phi(), sqrt( pow(rho,2)+pow(zet,2)) ) );

  //
  // traj closest to stright-line point at station2
  //
//  TrajectoryStateOnSurface muTSOS  = theTrajectory.empty() ? theState : theTrajectory.closestMeasurement(point).updatedState();
  TrajectoryStateOnSurface muTSOS  = theState;

  //
  // second station surface
  //
  bool barrel = fabs(point.z()) < 700. ? true : false;
  ReferenceCountingPointer<Surface> surface = barrel ?
      ReferenceCountingPointer<Surface>( new  BoundCylinder( GlobalPoint(0.,0.,0.), TkRotation<float>(), SimpleCylinderBounds( point.perp(),  point.perp(), -780., 780. ) ))
    : ReferenceCountingPointer<Surface>( new  BoundDisk( GlobalPoint(0.,0.,point.z()), TkRotation<float>(), SimpleDiskBounds( 260., 810., -0.0001, 0.0001 ) ) );

  //
  // propagator towards station2
  //
  auto const & propagator = es.getData(thePropagatorAnyToken);
//  edm::ESHandle<Propagator> propagator;
//  theEs.get<TrackingComponentsRecord>().get("SteppingHelixPropagatorAny", propagator);
  TrajectoryStateOnSurface result =  propagator.propagate(muTSOS, *surface);
  return result;

}

/*
void TrackAtSurface::prepare(const reco::Muon* mu,const edm::Event &ev, const edm::EventSetup &es)
{
//  theEv = ev;
//  theEs = es;
  std::cout <<" HERE HERE A" << std::endl;
  auto const & globalGeometry = es.getData(theGeomteryToken); 
  auto const & magField       = es.getData(theFieldToken);
  std::cout <<" HERE HERE E" << std::endl;


//refit global muons, take tracjctory colsest to Mu. 
  typedef std::vector<Trajectory> Trajectories;
  edm::Handle<Trajectories> trajectories;
//  edm::InputTag refitTag("globalMuons","Refitted");
  edm::InputTag refitTag("refittedMuons","Refitted");
  ev.getByLabel(refitTag,trajectories);
  double minDR =  0.3;
  for (Trajectories::const_iterator it=trajectories->begin(); it != trajectories->end(); ++it) {
    double diff = deltaR(mu->track()->eta(), mu->track()->phi(), it->geometricalInnermostState().globalMomentum().eta(), it->geometricalInnermostState().globalMomentum().phi());
    if (diff < minDR) { minDR = diff; theTrajectory = *it; }
  }
  theState = trajectoryStateTransform::outerStateOnSurface(*(mu->track()), globalGeometry, &magField);

}
*/

/*
TrajectoryStateOnSurface TrackAtSurface::atPoint( double eta, double phi) const
{
  double theta = 2.*atan(exp(-eta));
  double rho   = 500.;
  double zet   = 790.;
  if (fabs(eta) > 1.24) rho  = zet * tan(fabs(theta)); else zet = rho/tan(theta);
  GlobalPoint point( GlobalPoint::Polar(theta, phi, sqrt( pow(rho,2)+pow(zet,2)) ) );
  return atPoint(point);
}
*/

/* FIXME
TrajectoryStateOnSurface TrackAtSurface::atPoint( const GlobalPoint& point) const
{
  return TrajectoryStateOnSurface();
  TrajectoryStateOnSurface muTSOS  = theTrajectory.empty() ? theState : theTrajectory.closestMeasurement(point).updatedState();

  bool barrel = fabs(point.z()) < 700. ? true : false;
  ReferenceCountingPointer<Surface> surface = barrel ?
      ReferenceCountingPointer<Surface>( new  BoundCylinder( GlobalPoint(0.,0.,0.), TkRotation<float>(), SimpleCylinderBounds( point.perp(),  point.perp(), -780., 780. ) ))
    : ReferenceCountingPointer<Surface>( new  BoundDisk( GlobalPoint(0.,0.,point.z()), TkRotation<float>(), SimpleDiskBounds( 260., 810., -0.0001, 0.0001 ) ) );

  edm::ESHandle<Propagator> propagator;
  theEs.get<TrackingComponentsRecord>().get("SteppingHelixPropagatorAny", propagator);
  TrajectoryStateOnSurface result =  propagator->propagate(muTSOS, *surface);

  return result;
}
*/ 

TrajectoryStateOnSurface TrackAtSurface::atDetFromClose(const reco::Muon* mu, const RPCDetId& rpc, const GlobalPoint& point, const edm::EventSetup &es) const
{

  auto const & globalGeometry = es.getData(theGeomteryToken);
  auto const & magField       = es.getData(theFieldToken);

  TrajectoryStateOnSurface muTSOS = trajectoryStateTransform::outerStateOnSurface(*(mu->track()), globalGeometry, &magField);
  //if (!theTrajectory.empty()) muTSOS = theTrajectory.closestMeasurement(point).updatedState();

  const GeomDet * det = globalGeometry.idToDet(rpc);
  Plane::PlanePointer surface = Plane::build(det->position(), det->rotation());
  TrajectoryStateOnSurface result;
  auto const & propagator = es.getData(thePropagatorAnyToken);
  if (muTSOS.isValid()) result =  propagator.propagate(muTSOS, *surface);

  return result;
}

/* FIXME
TrajectoryStateOnSurface TrackAtSurface::atDetFromTrack( const RPCDetId& rpc) const
{
  edm::ESHandle<GlobalTrackingGeometry> globalGeometry;
  theEs.get<GlobalTrackingGeometryRecord>().get(globalGeometry);
  edm::ESHandle<Propagator> propagator;

  const GeomDet * det = globalGeometry->idToDet(rpc);
  TrajectoryStateOnSurface muTSOS = theState;
  theEs.get<TrackingComponentsRecord>().get("SteppingHelixPropagatorAlong",propagator);
  Plane::PlanePointer surface = Plane::build(det->position(), det->rotation());
  TrajectoryStateOnSurface result;
  if (muTSOS.isValid()) result =  propagator->propagate(muTSOS, *surface);

  return result;
}
*/
/* FIXME
TrajectoryStateOnSurface TrackAtSurface::atStation2( float eta) const
{
  return TrajectoryStateOnSurface();
  ReferenceCountingPointer<Surface> rpc;
  if (eta < -1.24)       rpc = ReferenceCountingPointer<Surface>(new  BoundDisk( GlobalPoint(0.,0.,-790.),  TkRotation<float>(), SimpleDiskBounds( 300., 810., -10., 10. ) ) );
  else if (eta < 1.24)   rpc = ReferenceCountingPointer<Surface>(new  BoundCylinder( GlobalPoint(0.,0.,0.), TkRotation<float>(), SimpleCylinderBounds( 500, 500, -900, 900 ) ) );
  else                   rpc = ReferenceCountingPointer<Surface>(new  BoundDisk( GlobalPoint(0.,0.,790.),   TkRotation<float>(), SimpleDiskBounds( 300., 810., -10., 10. ) ) );
//  if (eta < -1.04)       rpc = ReferenceCountingPointer<Surface>(new  BoundDisk( GlobalPoint(0.,0.,-790.), TkRotation<float>(), SimpleDiskBounds( 300., 710., -10., 10. ) ) );
//  else if (eta < -0.72)  rpc = ReferenceCountingPointer<Surface>(new  BoundCylinder( GlobalPoint(0.,0.,0.), TkRotation<float>(), SimpleCylinderBounds( 520, 520, -700, 700 ) ) );
//  else if (eta < 0.72)   rpc = ReferenceCountingPointer<Surface>(new  BoundCylinder( GlobalPoint(0.,0.,0.), TkRotation<float>(), SimpleCylinderBounds( 500, 500, -700, 700 ) ) );
//  else if (eta < 1.04)   rpc = ReferenceCountingPointer<Surface>(new  BoundCylinder( GlobalPoint(0.,0.,0.), TkRotation<float>(), SimpleCylinderBounds( 520, 520, -700, 700 ) ) );
//  else                      rpc = ReferenceCountingPointer<Surface>(new  BoundDisk( GlobalPoint(0.,0.,790.), TkRotation<float>(), SimpleDiskBounds( 300., 710., -10., 10. ) ) );
  edm::ESHandle<Propagator> propagator;
  theEs.get<TrackingComponentsRecord>().get("SteppingHelixPropagatorAlong", propagator);
  TrajectoryStateOnSurface trackAtRPC =  propagator->propagate( theState, *rpc);
  return trackAtRPC;
}
*/
