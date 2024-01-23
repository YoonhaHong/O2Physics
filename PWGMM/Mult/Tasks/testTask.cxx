// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "CCDB/BasicCCDBManager.h"
#include "Common/CCDB/EventSelectionParams.h"
#include "Common/Core/TrackSelection.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Framework/ASoAHelpers.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/AnalysisTask.h"
#include "Framework/Configurable.h"
#include "Framework/HistogramRegistry.h"
#include "Framework/O2DatabasePDGPlugin.h"
#include "Framework/RuntimeError.h"
#include "Framework/runDataProcessing.h"
#include "Selections.h"

#include "Common/DataModel/EventSelection.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;
using namespace o2::aod::track;
using namespace pwgmm::mult;

struct tutorialTask {
  HistogramRegistry registry{"registry", {}, OutputObjHandlingPolicy::AnalysisObject};

  void init(InitContext&)
  {

    if (doprocessTest) {
      registry.add("h_track_eta", "hist_name", {HistType::kTH1F, {{1000, -4, 4}}});
      registry.add("h_collision_posZ", "hist_name", {HistType::kTH1F, {{1000, -20, 20}}});
      registry.add("h_track_collision_posZ", "hist_name", {HistType::kTH1F, {{1000, -20, 20}}});
    }
  }

  void processDummy(
    aod::McCollisions const& mccollisions)
  {
  }
  PROCESS_SWITCH(tutorialTask, processDummy, "dummy Tasks", true);

  Filter fTrackSelectionITS = ncheckbit(aod::track::v001::detectorMap, (uint8_t)o2::aod::track::ITS) &&
                              ncheckbit(aod::track::trackCutFlag, trackSelectionITS);
  Filter fTrackSelectionTPC = ifnode(ncheckbit(aod::track::v001::detectorMap, (uint8_t)o2::aod::track::TPC),
                                     ncheckbit(aod::track::trackCutFlag, trackSelectionTPC), true);

  Filter trackfilter = nabs(o2::aod::track::eta) < 1.5f;

  Preslice<aod::Tracks> slicecolId = o2::aod::track::collisionId;
  using JoinTracks = soa::Filtered<soa::Join<aod::Tracks, aod::TracksExtra, aod::TrackSelection>>;

  void processTest(
    JoinTracks const& tracks,
    soa::Join<aod::Collisions, aod::EvSels> const& collisions)
  {
    for (auto& collision : collisions) {

      if (!collision.sel8()) {
        continue;
      }
      registry.fill(HIST("h_collision_posZ"), collision.posZ());

      auto trackpercol = tracks.sliceBy(slicecolId, collision.globalIndex());
      for (auto& track : trackpercol) {
        registry.fill(HIST("h_track_eta"), track.eta());
        if (track.has_collision()) {
          registry.fill(HIST("h_track_collision_posZ"), track.collision_as<soa::Join<aod::Collisions, aod::EvSels>>().posZ());
        }
      }
    }
  }
  PROCESS_SWITCH(tutorialTask, processTest, "test Tasks", false);

  // void processTest(
  //   aod::Tracks const& tracks,
  //   aod::Collision const& collision)
  // {
  //     if (nabs(collision.posZ()) > 20) {
  //       continue;
  //     }
  // registry.fill(HIST("h_collision_posZ"), collision.posZ());
  //     for (auto& track : tracks) {
  //       registry.fill(HIST("h_track_eta"), track.eta());
  //  if (track.has_collision()) {
  // registry.fill(HIST("h_track_collision_posZ"), track.collision_as<soa::Join<aod::Collisions, aod::EvSels>>().posZ());
  // }
  //     }
  //   }
  // }
  // PROCESS_SWITCH(tutorialTask, processTest, "test Tasks", false);
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{adaptAnalysisTask<tutorialTask>(cfgc)};
}
