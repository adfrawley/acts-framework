// This file is part of the Acts project.
//
// Copyright (C) 2017-2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <string>

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Utilities/Logger.hpp>

#include "ACTFW/Framework/IReader.hpp"

namespace FW {

/// Read particles in the TrackML comma-separated-value format.
///
/// This reads one file per event in the configured input directory
/// and filename. Files are assumed to be named using the following schema
///
///     event000000001-<stem>.csv
///     event000000002-<stem>.csv
///
/// and each line in the file corresponds to one particle. The
/// input filename can be configured and defaults to `particles.csv`.
class CsvParticleReader : public IReader
{
public:
  struct Config
  {
    /// Tracking geometry required to access surface from geoID
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry;
    /// Which particle collection to read into.
    std::string outputParticles;
    /// Which particle surface sequence collection to read into.
    std::string outputParticleSurfacesMap;
    /// Where to read input files from.
    std::string inputDir;
    /// Input filename stem.
    std::string inputStem = "particles";
    /// Input filename for particle surface sequence.
    std::string inputParticleSurface = "surfaces";
  };

  CsvParticleReader(const Config&        cfg,
                    Acts::Logging::Level level = Acts::Logging::INFO);

  std::string
  name() const final override;

  /// Return the available events range.
  std::pair<size_t, size_t>
  availableEvents() const final override;

  /// Read out data from the input stream.
  ProcessCode
  read(const FW::AlgorithmContext& ctx) final override;

private:
  Config                                           m_cfg;
  std::map<Acts::GeometryID, const Acts::Surface*> m_surfaces;
  std::pair<size_t, size_t>                        m_eventsRange;
  std::unique_ptr<const Acts::Logger>              m_logger;

  const Acts::Logger&
  logger() const
  {
    return *m_logger;
  }
};

}  // namespace FW
