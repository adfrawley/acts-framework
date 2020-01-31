// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <string>
#include "ACTFW/EventData/Barcode.hpp"
#include "ACTFW/EventData/SimHit.hpp"
#include "ACTFW/EventData/SimParticle.hpp"
#include "ACTFW/EventData/SimSourceLink.hpp"
#include "ACTFW/Utilities/Helpers.hpp"
#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackState.hpp"
#include "Acts/Geometry/GeometryID.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"

namespace FW {

// Tools to make track info plots to show tracking track info.
class TrackSummaryPlotTool
{
public:
  /// @brief The nested configuration struct
  struct Config
  {
    std::map<std::string, PlotHelpers::Binning> varBinning
        = {{"Eta", PlotHelpers::Binning("#eta", 40, -4, 4)},
           {"Phi", PlotHelpers::Binning("#phi", 100, -3.15, 3.15)},
           {"Pt", PlotHelpers::Binning("pT [GeV/c]", 20, 0, 100)},
           {"Multiplicity", PlotHelpers::Binning("N", 30, -0.5, 29.5)}};
  };

  /// @brief Nested Cache struct
  struct RecoTrackPlotCache
  {
    TProfile* nStates_vs_eta;        ///< number of total states vs eta
    TProfile* nMeasurements_vs_eta;  ///< number of non-outlier measurements
    TProfile* nHoles_vs_eta;         ///< number of holes vs eta
    TProfile* nOutliers_vs_eta;      ///< number of outliers vs eta
    TProfile* nStates_vs_pt;         ///< number of total states vs pt
    TProfile*
              nMeasurements_vs_pt;  ///< number of non-outlier measurements vs pt
    TProfile* nHoles_vs_pt;         ///< number of holes vs pt
    TProfile* nOutliers_vs_pt;      ///< number of outliers vs pt
  };

  /// Constructor
  ///
  /// @param cfg Configuration struct
  /// @param level Message level declaration
  TrackSummaryPlotTool(const Config&        cfg,
                       Acts::Logging::Level level = Acts::Logging::INFO);

  /// @brief book the track info plots
  /// @param recoTrackPlotCache the cache for track info plots
  void
  book(RecoTrackPlotCache& recoTrackPlotCache) const;

  /// @brief fill track info
  ///
  /// @param recoTrackPlotCache cache object for track info plots
  /// @param truthParticle truth particle for this track
  /// @param nStates number of track states
  /// @param nMeasurements number of measurements
  /// @param nOutliers number of outliers
  /// @param nHoles number of holes
  void
  fill(RecoTrackPlotCache&      recoTrackPlotCache,
       const Data::SimParticle& truthParticle,
       const size_t&            nStates,
       const size_t&            nMeasurments,
       const size_t&            Outliers,
       const size_t&            nHoles) const;

  /// @brief write the track info plots to file
  /// @param recoTrackPlotCache cache object for track info plots
  void
  write(const RecoTrackPlotCache& recoTrackPlotCache) const;

  /// @brief delete the track info plots
  /// @param recoTrackPlotCache cache object for track info plots
  void
  clear(RecoTrackPlotCache& recoTrackPlotCache) const;

private:
  Config                              m_cfg;     ///< The Config class
  std::unique_ptr<const Acts::Logger> m_logger;  ///< The logging instance

  /// The logger
  const Acts::Logger&
  logger() const
  {
    return *m_logger;
  }
};

}  // namespace FW
