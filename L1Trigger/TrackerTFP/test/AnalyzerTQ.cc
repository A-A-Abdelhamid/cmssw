#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Common/interface/Handle.h"

#include "SimTracker/TrackTriggerAssociation/interface/StubAssociation.h"
#include "L1Trigger/TrackTrigger/interface/Setup.h"
#include "L1Trigger/TrackerTFP/interface/DataFormats.h"

#include <TProfile.h>
#include <TH1F.h>

#include <vector>
#include <deque>
#include <set>
#include <cmath>
#include <numeric>
#include <sstream>

using namespace std;
using namespace edm;
using namespace tt;

namespace trackerTFP {

  /*! \class  trackerTFP::AnalyzerTQ
   *  \brief  Class to analyze hardware like structured track Collection generated by Duplicate Removal
   *  \author Thomas Schuh
   *  \date   2023, Feb
   */
  class AnalyzerTQ : public one::EDAnalyzer<one::WatchRuns, one::SharedResources> {
  public:
    AnalyzerTQ(const ParameterSet& iConfig);
    void beginJob() override {}
    void beginRun(const Run& iEvent, const EventSetup& iSetup) override;
    void analyze(const Event& iEvent, const EventSetup& iSetup) override;
    void endRun(const Run& iEvent, const EventSetup& iSetup) override {}
    void endJob() override;

  private:
    //
    void formTracks(const StreamsTrack& streamsTrack,
                    const StreamsStub& streamsStubs,
                    vector<vector<TTStubRef>>& tracks,
                    int channel) const;
    //
    void associate(const vector<vector<TTStubRef>>& tracks,
                   const StubAssociation* ass,
                   set<TPPtr>& tps,
                   int& sum,
                   bool perfect = true) const;
    // ED input token of stubs
    EDGetTokenT<StreamsStub> edGetTokenStubs_;
    // ED input token of tracks
    EDGetTokenT<StreamsTrack> edGetTokenTracks_;
    // ED input token of TTStubRef to TPPtr association for tracking efficiency
    EDGetTokenT<StubAssociation> edGetTokenSelection_;
    // ED input token of TTStubRef to recontructable TPPtr association
    EDGetTokenT<StubAssociation> edGetTokenReconstructable_;
    // Setup token
    ESGetToken<Setup, SetupRcd> esGetTokenSetup_;
    // DataFormats token
    ESGetToken<DataFormats, DataFormatsRcd> esGetTokenDataFormats_;
    // stores, calculates and provides run-time constants
    const Setup* setup_ = nullptr;
    // helper class to extract structured data from tt::Frames
    const DataFormats* dataFormats_ = nullptr;
    // enables analyze of TPs
    bool useMCTruth_;
    //
    int nEvents_ = 0;

    // Histograms

    TProfile* prof_;
    TProfile* profChannel_;
    TProfile* profTracks_;
    TH1F* hisChannel_;
    TH1F* hisTracks_;

    // printout
    stringstream log_;
  };

  AnalyzerTQ::AnalyzerTQ(const ParameterSet& iConfig) : useMCTruth_(iConfig.getParameter<bool>("UseMCTruth")) {
    usesResource("TFileService");
    // book in- and output ED products
    const string& label = iConfig.getParameter<string>("OutputLabelTQ");
    const string& branchStubs = iConfig.getParameter<string>("BranchStubs");
    const string& branchTracks = iConfig.getParameter<string>("BranchTracks");
    edGetTokenStubs_ = consumes<StreamsStub>(InputTag(label, branchStubs));
    edGetTokenTracks_ = consumes<StreamsTrack>(InputTag(label, branchTracks));
    if (useMCTruth_) {
      const auto& inputTagSelecttion = iConfig.getParameter<InputTag>("InputTagSelection");
      const auto& inputTagReconstructable = iConfig.getParameter<InputTag>("InputTagReconstructable");
      edGetTokenSelection_ = consumes<StubAssociation>(inputTagSelecttion);
      edGetTokenReconstructable_ = consumes<StubAssociation>(inputTagReconstructable);
    }
    // book ES products
    esGetTokenSetup_ = esConsumes<Setup, SetupRcd, Transition::BeginRun>();
    esGetTokenDataFormats_ = esConsumes<DataFormats, DataFormatsRcd, Transition::BeginRun>();
    // log config
    log_.setf(ios::fixed, ios::floatfield);
    log_.precision(4);
  }

  void AnalyzerTQ::beginRun(const Run& iEvent, const EventSetup& iSetup) {
    // helper class to store configurations
    setup_ = &iSetup.getData(esGetTokenSetup_);
    // helper class to extract structured data from tt::Frames
    dataFormats_ = &iSetup.getData(esGetTokenDataFormats_);
    // book histograms
    Service<TFileService> fs;
    TFileDirectory dir;
    dir = fs->mkdir("TQ");
    prof_ = dir.make<TProfile>("Counts", ";", 12, 0.5, 12.5);
    prof_->GetXaxis()->SetBinLabel(1, "Stubs");
    prof_->GetXaxis()->SetBinLabel(2, "Tracks");
    prof_->GetXaxis()->SetBinLabel(4, "Matched Tracks");
    prof_->GetXaxis()->SetBinLabel(5, "All Tracks");
    prof_->GetXaxis()->SetBinLabel(6, "Found TPs");
    prof_->GetXaxis()->SetBinLabel(7, "Found selected TPs");
    prof_->GetXaxis()->SetBinLabel(9, "All TPs");
    prof_->GetXaxis()->SetBinLabel(10, "states");
    prof_->GetXaxis()->SetBinLabel(12, "max tp");
    // channel occupancy
    constexpr int maxOcc = 180;
    const int numChannels = dataFormats_->numChannel(Process::dr);
    hisChannel_ = dir.make<TH1F>("His Channel Occupancy", ";", maxOcc, -.5, maxOcc - .5);
    profChannel_ = dir.make<TProfile>("Prof Channel Occupancy", ";", numChannels, -.5, numChannels - .5);
    // track occupancy
    hisTracks_ = dir.make<TH1F>("His Track Occupancy", ";", maxOcc, -.5, maxOcc - .5);
    profTracks_ = dir.make<TProfile>("Prof Track Occupancy", ";", numChannels, -.5, numChannels - .5);
  }

  void AnalyzerTQ::analyze(const Event& iEvent, const EventSetup& iSetup) {
    // read in ht products
    Handle<StreamsStub> handleStubs;
    iEvent.getByToken<StreamsStub>(edGetTokenStubs_, handleStubs);
    const StreamsStub& acceptedStubs = *handleStubs;
    Handle<StreamsTrack> handleTracks;
    iEvent.getByToken<StreamsTrack>(edGetTokenTracks_, handleTracks);
    const StreamsTrack& acceptedTracks = *handleTracks;
    // read in MCTruth
    const StubAssociation* selection = nullptr;
    const StubAssociation* reconstructable = nullptr;
    if (useMCTruth_) {
      Handle<StubAssociation> handleSelection;
      iEvent.getByToken<StubAssociation>(edGetTokenSelection_, handleSelection);
      selection = handleSelection.product();
      prof_->Fill(9, selection->numTPs());
      Handle<StubAssociation> handleReconstructable;
      iEvent.getByToken<StubAssociation>(edGetTokenReconstructable_, handleReconstructable);
      reconstructable = handleReconstructable.product();
    }
    // analyze ht products and associate found tracks with reconstrucable TrackingParticles
    set<TPPtr> tpPtrs;
    set<TPPtr> tpPtrsSelection;
    set<TPPtr> tpPtrsMax;
    int allMatched(0);
    int allTracks(0);
    for (int region = 0; region < setup_->numRegions(); region++) {
      vector<vector<TTStubRef>> tracks;
      formTracks(acceptedTracks, acceptedStubs, tracks, region);
      hisTracks_->Fill(tracks.size());
      profTracks_->Fill(region, tracks.size());
      const int nTracks = tracks.size();
      const int nStubs = accumulate(tracks.begin(), tracks.end(), 0, [](int sum, const vector<TTStubRef>& track) {
        return sum += (int)track.size();
      });
      allTracks += tracks.size();
      if (!useMCTruth_)
        continue;
      int tmp(0);
      associate(tracks, selection, tpPtrsSelection, tmp);
      associate(tracks, reconstructable, tpPtrs, allMatched, false);
      associate(tracks, selection, tpPtrsMax, tmp, false);
      const int size = acceptedTracks[region].size();
      hisChannel_->Fill(size);
      profChannel_->Fill(region, size);
      prof_->Fill(1, nStubs);
      prof_->Fill(2, nTracks);
    }
    prof_->Fill(4, allMatched);
    prof_->Fill(5, allTracks);
    prof_->Fill(6, tpPtrs.size());
    prof_->Fill(7, tpPtrsSelection.size());
    prof_->Fill(12, tpPtrsMax.size());
    nEvents_++;
  }

  void AnalyzerTQ::endJob() {
    if (nEvents_ == 0)
      return;
    // printout DR summary
    const double totalTPs = prof_->GetBinContent(9);
    const double numStubs = prof_->GetBinContent(1);
    const double numTracks = prof_->GetBinContent(2);
    const double totalTracks = prof_->GetBinContent(5);
    const double numTracksMatched = prof_->GetBinContent(4);
    const double numTPsAll = prof_->GetBinContent(6);
    const double numTPsEff = prof_->GetBinContent(7);
    const double numTPsEffMax = prof_->GetBinContent(12);
    const double errStubs = prof_->GetBinError(1);
    const double errTracks = prof_->GetBinError(2);
    const double fracFake = (totalTracks - numTracksMatched) / totalTracks;
    const double fracDup = (numTracksMatched - numTPsAll) / totalTracks;
    const double eff = numTPsEff / totalTPs;
    const double errEff = sqrt(eff * (1. - eff) / totalTPs / nEvents_);
    const double effMax = numTPsEffMax / totalTPs;
    const double errEffMax = sqrt(effMax * (1. - effMax) / totalTPs / nEvents_);
    const vector<double> nums = {numStubs, numTracks};
    const vector<double> errs = {errStubs, errTracks};
    const int wNums = ceil(log10(*max_element(nums.begin(), nums.end()))) + 5;
    const int wErrs = ceil(log10(*max_element(errs.begin(), errs.end()))) + 5;
    log_ << "                         TQ  SUMMARY                         " << endl;
    log_ << "number of stubs       per TFP = " << setw(wNums) << numStubs << " +- " << setw(wErrs) << errStubs << endl;
    log_ << "number of tracks      per TFP = " << setw(wNums) << numTracks << " +- " << setw(wErrs) << errTracks
         << endl;
    log_ << "          tracking efficiency = " << setw(wNums) << eff << " +- " << setw(wErrs) << errEff << endl;
    log_ << "      max tracking efficiency = " << setw(wNums) << effMax << " +- " << setw(wErrs) << errEffMax << endl;
    log_ << "                    fake rate = " << setw(wNums) << fracFake << endl;
    log_ << "               duplicate rate = " << setw(wNums) << fracDup << endl;
    log_ << "=============================================================";
    LogPrint(moduleDescription().moduleName()) << log_.str();
  }

  //
  void AnalyzerTQ::formTracks(const StreamsTrack& streamsTrack,
                              const StreamsStub& streamsStubs,
                              vector<vector<TTStubRef>>& tracks,
                              int region) const {
    const int offset = region * setup_->numLayers();
    const StreamTrack& streamTrack = streamsTrack[region];
    const int numTracks = accumulate(streamTrack.begin(), streamTrack.end(), 0, [](int sum, const FrameTrack& frame) {
      return sum += (frame.first.isNonnull() ? 1 : 0);
    });
    tracks.reserve(numTracks);
    for (int frame = 0; frame < (int)streamTrack.size(); frame++) {
      const FrameTrack& frameTrack = streamTrack[frame];
      if (frameTrack.first.isNull())
        continue;
      deque<TTStubRef> stubs;
      for (int layer = 0; layer < setup_->numLayers(); layer++) {
        const FrameStub& stub = streamsStubs[offset + layer][frame];
        if (stub.first.isNonnull())
          stubs.push_back(stub.first);
      }
      tracks.emplace_back(stubs.begin(), stubs.end());
    }
  }

  //
  void AnalyzerTQ::associate(const vector<vector<TTStubRef>>& tracks,
                             const StubAssociation* ass,
                             set<TPPtr>& tps,
                             int& sum,
                             bool perfect) const {
    for (const vector<TTStubRef>& ttStubRefs : tracks) {
      const vector<TPPtr>& tpPtrs = perfect ? ass->associateFinal(ttStubRefs) : ass->associate(ttStubRefs);
      if (tpPtrs.empty())
        continue;
      sum++;
      copy(tpPtrs.begin(), tpPtrs.end(), inserter(tps, tps.begin()));
    }
  }

}  // namespace trackerTFP

DEFINE_FWK_MODULE(trackerTFP::AnalyzerTQ);
