// This file is part of the Acts project.
//
// Copyright (C) 2017-2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Utilities/Logger.hpp>

#include "ACTFW/Framework/IWriter.hpp"

namespace Acts {
class TrackingVolume;
}

namespace FW {

/// Write out the geometry for all sensitive detector surfaces.
///
/// This writes a `detectors.csv` file at the end of the run using the
/// default context to determine the geometry. If configured, it also writes
/// an additional file for each event using the following schema
///
///     event000000001-detectors.csv
///     event000000002-detectors.csv
///     ...
///
/// that uses the per-event context to determine the geometry.
class CsvTrackingGeometryWriter : public IWriter
{
public:
  struct Config
  {
    /// The tracking geometry that should be written.
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry;
    /// Where to place output files.
    std::string outputDir;
    /// Number of decimal digits for floating point precision in output.
    std::size_t outputPrecision = 6;
    /// Whether to write the per-event file.
    bool writePerEvent = false;
  };

  /// Constructor
  /// @param cfg is the configuration class
  CsvTrackingGeometryWriter(const Config&        cfg,
                            Acts::Logging::Level lvl = Acts::Logging::INFO);

  std::string
  name() const final override;

  /// Write geometry using the per-event context (optional).
  ProcessCode
  write(const AlgorithmContext& context) final override;

  /// Write geometry using the default context.
  ProcessCode
  endRun() final override;

private:
  Config                              m_cfg;
  const Acts::TrackingVolume*         m_world;
  std::unique_ptr<const Acts::Logger> m_logger;

  const Acts::Logger&
  logger() const
  {
    return *m_logger;
  }
};

}  // namespace FW