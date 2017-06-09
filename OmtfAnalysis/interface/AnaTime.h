#ifndef UserCode_OmtfAnalysis_AnaTime_H
#define UserCode_OmtfAnalysis_AnaTime_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
class TObjArray;
class MuonObj;
class L1ObjColl;
class EventObj;
#include <vector>

class AnaTime {
public:
  AnaTime(const edm::ParameterSet& cfg);
  void init(TObjArray& histos);
  void run( const EventObj* ev, const MuonObj* muon, const L1ObjColl * l1Objs);
  bool debug;
private:
  edm::ParameterSet theCfg;
};
#endif

