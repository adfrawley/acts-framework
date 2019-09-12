// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>

#include "ACTFW/EventData/Barcode.hpp"
#include "ACTFW/EventData/DataContainers.hpp"
#include "ACTFW/EventData/SimHit.hpp"
#include "ACTFW/EventData/SimParticle.hpp"
#include "ACTFW/EventData/SimSourceLink.hpp"
#include "ACTFW/EventData/SimVertex.hpp"
#include "ACTFW/Framework/ProcessCode.hpp"
#include "ACTFW/Framework/RandomNumbers.hpp"
#include "ACTFW/Framework/WhiteBoard.hpp"
#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackState.hpp"
#include "Acts/Fitter/KalmanFitter.hpp"
#include "Acts/Geometry/GeometryID.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"

namespace FW {

template <typename kalman_Fitter_t>
class FittingAlgorithm : public BareAlgorithm
{
public:
  // A few initialisations and definitionas
  using Identifier  = Data::SimSourceLink;
  using TrackState  = Acts::TrackState<Identifier, Acts::BoundParameters>;
  using TrackMap    = std::map<barcode_type, std::vector<TrackState>>;
  using ParticleMap = std::map<barcode_type, Data::SimParticle>;

  /// Nested configuration struct
  struct Config
  {

    Config(kalman_Fitter_t fitter) : kFitter(std::move(fitter)) {}
    /// input hit collection
    std::string simulatedHitCollection = "";
    /// input event collection
    std::string simulatedEventCollection = "";
    /// output track collection
    std::string trackCollection = "";
    /// kalmanFitter instance
    kalman_Fitter_t kFitter;
    /// FW random number service
    std::shared_ptr<RandomNumbers> randomNumberSvc = nullptr;
    /// Gaussian sigma used to smear the truth track parameter
    std::vector<double> parameterSigma = {10, 10, 0.02, 0.02, 1};
    /// Gaussian sigma used to smear the truth hit
    std::vector<double> measurementSigma = {30, 30};
  };

  /// Constructor of the fitting algorithm
  ///
  /// @param cfg is the config struct to configure the algorihtm
  /// @param level is the logging level
  FittingAlgorithm(Config               cfg,
                   Acts::Logging::Level level = Acts::Logging::INFO);

  /// Framework execute method of the fitting algorithm
  ///
  /// @param ctx is the algorithm context that holds event-wise information
  /// @return a process code to steer the algporithm flow
  FW::ProcessCode
  execute(const FW::AlgorithmContext& ctx) const final override;

private:
  Config m_cfg;  /// config struct
};

}  // namespace FW

#include "FittingAlgorithm.ipp"
