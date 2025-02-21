// TrackletProcessorDisplaced: This class performs the tasks of the TrackletEngineDisplaced+TripletEngine+TrackletCalculatorDisplaced.
#ifndef L1Trigger_TrackFindingTracklet_interface_TrackletProcessorDisplaced_h
#define L1Trigger_TrackFindingTracklet_interface_TrackletProcessorDisplaced_h

#include "L1Trigger/TrackFindingTracklet/interface/TrackletCalculatorBase.h"
#include "L1Trigger/TrackFindingTracklet/interface/TrackletCalculatorDisplaced.h"
#include "L1Trigger/TrackFindingTracklet/interface/TrackletLUT.h"
#include "L1Trigger/TrackFindingTracklet/interface/CircularBuffer.h"
#include "L1Trigger/TrackFindingTracklet/interface/TrackletParametersMemory.h"
#include "L1Trigger/TrackFindingTracklet/interface/TrackletProjectionsMemory.h"
#include "L1Trigger/TrackFindingTracklet/interface/TripletEngineUnit.h"

#include <vector>
#include <tuple>
#include <map>

namespace trklet {

  class Settings;
  class Globals;
  class MemoryBase;
  class AllStubsMemory;
  class AllInnerStubsMemory;
  class VMStubsTEMemory;
  class StubPairsMemory;

  class TrackletProcessorDisplaced : public TrackletCalculatorDisplaced {
  public:
    TrackletProcessorDisplaced(std::string name, Settings const& settings, Globals* globals);

    ~TrackletProcessorDisplaced() override = default;

    void addOutputProjection(TrackletProjectionsMemory*& outputProj, MemoryBase* memory);

    void addOutput(MemoryBase* memory, std::string output) override;

    void addInput(MemoryBase* memory, std::string input) override;

    void execute(unsigned int iSector, double phimin, double phimax);

  private:
    int iTC_;
    unsigned int maxStep_;

    std::tuple<CircularBuffer<TrpEData>, unsigned int, unsigned int, unsigned int, unsigned int> trpbuffer_;
    std::vector<TripletEngineUnit> trpunits_;

    unsigned int layerdisk1_;
    unsigned int layerdisk2_;
    unsigned int layerdisk3_;

    int firstphibits_;
    int secondphibits_;
    int thirdphibits_;

    int nbitszfinebintable_;
    int nbitsrfinebintable_;

    TrackletLUT innerTable_;       //projection to next layer/disk
    TrackletLUT innerThirdTable_;  //projection to third disk/layer

    std::vector<VMStubsTEMemory*> innervmstubs_;
    std::vector<VMStubsTEMemory*> outervmstubs_;
  };

};  // namespace trklet
#endif
