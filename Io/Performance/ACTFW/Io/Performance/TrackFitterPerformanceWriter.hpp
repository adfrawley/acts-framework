// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "ACTFW/EventData/Track.hpp"
#include "ACTFW/Framework/WriterT.hpp"
#include "ACTFW/Validation/EffPlotTool.hpp"
#include "ACTFW/Validation/ResPlotTool.hpp"

class TFile;
class TTree;

namespace FW {

/// Write out the residual and pull of track parameters and efficiency.
///
/// Efficiency here is the fraction of smoothed tracks compared to all tracks.
///
/// A common file can be provided for to the writer to attach his TTree,
/// this is done by setting the Config::rootFile pointer to an existing file
///
/// Safe to use from multiple writer threads - uses a std::mutex lock.
class TrackFitterPerformanceWriter final : public WriterT<Trajectories>
{
public:
  struct Config
  {
    /// trajectory collection to write
    std::string trackCollection;
    /// truth particle collection
    std::string simulatedEventCollection;
    /// path of the output file
    std::string filePath;
    /// file access mode
    std::string fileMode = "RECREATE";
    /// common root file
    TFile*              rootFile = nullptr;
    ResPlotTool::Config resPlotToolConfig;
    EffPlotTool::Config effPlotToolConfig;
  };

  /// Construct from configuration and log level.
  TrackFitterPerformanceWriter(const Config& cfg, Acts::Logging::Level lvl);
  ~TrackFitterPerformanceWriter() override;

  /// Finalize plots.
  ProcessCode
  endRun() final override;

private:
  ProcessCode
  writeT(const AlgorithmContext& ctx,
         const Trajectories&     trajectories) final override;

  Config m_cfg;
  /// Mutex used to protect multi-threaded writes.
  std::mutex m_writeMutex;
  TFile*     m_outputFile{nullptr};
  /// Plot tool for residuals and pulls.
  ResPlotTool               m_resPlotTool;
  ResPlotTool::ResPlotCache m_resPlotCache;
  /// Plot tool for efficiency
  EffPlotTool               m_effPlotTool;
  EffPlotTool::EffPlotCache m_effPlotCache;
};

}  // namespace FW
