#ifndef L1ObjMaker_H
#define L1ObjMaker_H

#include <vector>
#include "UserCode/OmtfDataFormats/interface/L1Obj.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/RunID.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/L1TMuonPhase2/interface/SAMuon.h"

namespace edm {class Event; }

class L1ObjMaker {
public:
  L1ObjMaker(const  edm::ParameterSet & cfg, edm::ConsumesCollector&& cColl);
  const std::vector<L1Obj> & operator()(const edm::Event &ev) { run(ev); return theL1Objs; }

private:
  void run(const edm::Event &ev);
  bool makeRegCandidates(const edm::Event &ev, L1Obj::TYPE t, std::vector<L1Obj> &result);
  bool makeGmtCandidates(const edm::Event &ev, L1Obj::TYPE t, std::vector<L1Obj> &result);
  bool makeGmtPhase2Candidates(const edm::Event &ev, L1Obj::TYPE t, std::vector<L1Obj> &result);
  bool makeSAMuonMatch(const edm::Event &iEvent, L1Obj::TYPE type, std::vector<L1Obj> &result);

private:

private:
  edm::ParameterSet   theConfig;
  std::vector<L1Obj>  theL1Objs;
  edm::EventNumber_t lastEvent;
  edm::RunNumber_t   lastRun;

};
#endif

