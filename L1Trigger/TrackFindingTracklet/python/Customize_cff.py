# functions to alter configurations

import FWCore.ParameterSet.Config as cms

# configures track finding s/w to behave as track finding f/w
def fwConfig(process):
  process.l1tTTTracksFromTrackletEmulation.Fakefit = True
  process.TrackTriggerSetup.TrackFinding.MaxEta =  2.5
  process.TrackTriggerSetup.GeometricProcessor.ChosenRofZ = 57.76
  process.l1tTTTracksFromTrackletEmulation.RemovalType = ""
  process.l1tTTTracksFromTrackletEmulation.DoMultipleMatches = False
  process.l1tTTTracksFromTrackletEmulation.StoreTrackBuilderOutput = True

# configures track finding s/w to behave as a subchain of processing steps
def reducedConfig(process):
  #fwConfig(process) disabled for now as new KF not working with reduced config
  process.TrackTriggerSetup.Firmware.FreqBEHigh = 240 # Frequency of DTC & KF (determines truncation)
  process.TrackTriggerSetup.KalmanFilter.NumWorker = 1
  process.ChannelAssignment.SeedTypes = cms.vstring( "L5L6" )
  process.ChannelAssignment.SeedTypesSeedLayers = cms.PSet( L5L6 = cms.vint32( 5,  6 ) )
  process.ChannelAssignment.SeedTypesProjectionLayers = cms.PSet( L5L6 = cms.vint32(  1,  2,  3,  4 ) )
  # this are tt::Setup::dtcId in order as in process.l1tTTTracksFromTrackletEmulation.processingModulesFile translated by 
  # reverssing naming logic described in L1FPGATrackProducer
  # TO DO: Eliminate cfg param IRChannelsIn by taking this info from Tracklet wiring map.
  process.ChannelAssignment.IRChannelsIn = cms.vint32( 0, 1, 25, 2, 26, 4, 5, 29, 6, 30, 7, 31, 8, 9, 33 )
  process.l1tTTTracksFromTrackletEmulation.Reduced = True
  process.l1tTTTracksFromTrackletEmulation.DoMultipleMatches = False
  process.l1tTTTracksFromTrackletEmulation.memoryModulesFile = 'L1Trigger/TrackFindingTracklet/data/memorymodules_reduced.dat'
  process.l1tTTTracksFromTrackletEmulation.processingModulesFile = 'L1Trigger/TrackFindingTracklet/data/processingmodules_reduced.dat'
  process.l1tTTTracksFromTrackletEmulation.wiresFile = 'L1Trigger/TrackFindingTracklet/data/wires_reduced.dat'

# configures pure tracklet algorithm (as opposed to Hybrid algorithm)
def trackletConfig(process):
  process.l1tTTTracksFromTrackletEmulation.fitPatternFile = cms.FileInPath('L1Trigger/TrackFindingTracklet/data/fitpattern.txt') 

# configures KF simulation in emulation chain
def oldKFConfig(process):
  process.ProducerKF.Hybrid                                   = True
  process.ProducerKF.DeadModuleOpts.KillScenario              = 0
  process.ProducerKF.DeadModuleOpts.KillRecover               = False
  process.ProducerKF.HTArraySpecRphi.HoughMinPt               = 2.
  process.ProducerKF.TrackDigi.KF_skipTrackDigi               = True
  process.ProducerKF.StubDigitize.EnableDigitize              = False
  process.ProducerKF.GeometricProc.UseApproxB                 = True
  process.ProducerKF.GeometricProc.BApprox_gradient           = 0.886454
  process.ProducerKF.GeometricProc.BApprox_intercept          = 0.504148
  process.ProducerKF.PhiSectors.NumPhiSectors                 = 9
  process.ProducerKF.PhiSectors.NumPhiNonants                 = 9
  process.ProducerKF.PhiSectors.ChosenRofPhi                  = 55.
  process.ProducerKF.EtaSectors.EtaRegions                    = [-2.4, -2.08, -1.68, -1.26, -0.90, -0.62, -0.41, -0.20, 0.0, 0.20, 0.41, 0.62, 0.90, 1.26, 1.68, 2.08, 2.4]
  process.ProducerKF.EtaSectors.ChosenRofZ                    = 50.0
  process.ProducerKF.TrackFitSettings.KalmanMinNumStubs       = 4
  process.ProducerKF.TrackFitSettings.KalmanMaxNumStubs       = 6
  process.ProducerKF.TrackFitSettings.KalmanMaxSkipLayersHard = 1
  process.ProducerKF.TrackFitSettings.KalmanMaxSkipLayersEasy = 2
  process.ProducerKF.TrackFitSettings.KalmanMaxStubsEasy      = 10
  process.ProducerKF.TrackFitSettings.KalmanMaxStubsPerLayer  = 4
  process.ProducerKF.TrackFitSettings.KalmanMultiScattTerm    = 0.00075
  process.ProducerKF.TrackFitSettings.KalmanChi2RphiScale     = 8
  process.ProducerKF.TrackFitSettings.KFUseMaybeLayers        = True
  process.ProducerKF.TrackFitSettings.KalmanRemove2PScut      = True
  process.ProducerKF.TrackFitSettings.KFLayerVsPtToler        = [999., 999., 0.1, 0.1, 0.05, 0.05, 0.05]
  process.ProducerKF.TrackFitSettings.KFLayerVsD0Cut5         = [999., 999., 999., 10., 10., 10., 10.]
  process.ProducerKF.TrackFitSettings.KFLayerVsZ0Cut5         = [999., 999., 25.5, 25.5, 25.5, 25.5, 25.5]
  process.ProducerKF.TrackFitSettings.KFLayerVsZ0Cut4         = [999., 999., 15., 15., 15., 15., 15.]
  process.ProducerKF.TrackFitSettings.KFLayerVsChiSq5         = [999., 999., 10., 30., 80., 120., 160.]
  process.ProducerKF.TrackFitSettings.KFLayerVsChiSq4         = [999., 999., 10., 30., 80., 120., 160.]
  process.ProducerKF.TrackFitSettings.KalmanAddBeamConstr     = False
  process.ProducerKF.TrackFitSettings.KalmanHOfw              = False
  process.ProducerKF.TrackFitSettings.KalmanHOtilted          = True
  process.ProducerKF.TrackFitSettings.KalmanHOprojZcorr       = 1
  process.ProducerKF.TrackFitSettings.KalmanHOalpha           = 0
  process.ProducerKF.TrackFitSettings.KalmanHOhelixExp        = True
  process.ProducerKF.TrackFitSettings.KalmanDebugLevel        = 0


def newKFConfig(process):
    """
    Configure HYBRID_NEWKF_DISPLACED:
      - Use extended seeding
      - Enable full 5-parameter Kalman fit
      - Use seed wiring map for triplet layers
      - Load lookup tables for extended KF (TED/TRE)
      - Emulate firmware-like configuration
    """

    # Tell the tracklet producer to use extended (triplet) seeding
    process.TrackFindingTrackletProducer_params.Extended = cms.bool(True)

    # Use 5-parameter Kalman Filter fit
    process.TrackFindingTrackletProducer_params.Use5ParameterFit = cms.bool(True)

    # Provide the wiring map to enable triplet seeding
    process.TrackFindingTrackletProducer_params.Reduced = cms.bool(False)
    process.TrackFindingTrackletProducer_params.wiresJSONFile = cms.string(
        "L1Trigger/TrackFindingTracklet/data/seedWiring.json"
    )

    # Load TED/TRE tables used in displaced KF
    process.TrackFindingTrackletProducer_params.tableTEDFile = cms.FileInPath(
        "L1Trigger/TrackFindingTracklet/data/table_TED/table_TED_D1PHIA1_D2PHIA1.txt"
    )
    process.TrackFindingTrackletProducer_params.tableTREFile = cms.FileInPath(
        "L1Trigger/TrackFindingTracklet/data/table_TRE/table_TRE_D1AD2A_1.txt"
    )

    # These settings emulate hardware/firmware behavior
    process.TrackFindingTrackletProducer_params.Fakefit = cms.bool(True)
    process.TrackFindingTrackletProducer_params.RemovalType = cms.string("")
    process.TrackFindingTrackletProducer_params.DoMultipleMatches = cms.bool(False)
    process.TrackFindingTrackletProducer_params.StoreTrackBuilderOutput = cms.bool(True)

    # Max eta and chosen z-coordinate for beamspot used in geometrical calculations
    process.TrackTriggerSetup.TrackFinding.MaxEta = cms.double(2.5)
    process.TrackTriggerSetup.GeometricProcessor.ChosenRofZ = cms.double(57.76)