// This file is part of the Acts project.
//
// Copyright (C) 2017 Acts project team
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

#include "ACTFW/Barcode/Barcode.hpp"
#include "ACTFW/EventData/DataContainers.hpp"
#include "ACTFW/EventData/SimHit.hpp"
#include "ACTFW/EventData/SimParticle.hpp"
#include "ACTFW/EventData/SimVertex.hpp"
#include "ACTFW/Framework/ProcessCode.hpp"
#include "ACTFW/Framework/WhiteBoard.hpp"
#include "ACTFW/Random/RandomNumberDistributions.hpp"
#include "ACTFW/Random/RandomNumbersSvc.hpp"
#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackState.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/GeometryID.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"

namespace FW {

template <typename kalman_Fitter_t>
class FittingAlgorithm : public BareAlgorithm
{
public:
  // A few initialisations and definitionas
  using Identifier  = Acts::GeometryID;
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
    std::shared_ptr<RandomNumbersSvc> randomNumberSvc = nullptr;
  };

  /// Constructor of the fitting algorithm
  ///
  /// @param cfg is the config struct to configure the algorihtm
  /// @param level is the logging level
  FittingAlgorithm(const Config&        cfg,
                   Acts::Logging::Level level = Acts::Logging::INFO);

  /// Framework execute method of the fitting algorithm
  ///
  /// @param ctx is the algorithm context that holds event-wise information
  /// @return a process code to steer the algporithm flow
  FW::ProcessCode
  execute(FW::AlgorithmContext ctx) const final override;

private:
  Config m_cfg;  /// config struct
};

}  // namespace FW

template <typename kalman_Fitter_t>
FW::FittingAlgorithm<kalman_Fitter_t>::FittingAlgorithm(
    const Config&        cfg,
    Acts::Logging::Level level)
  : FW::BareAlgorithm("FittingAlgorithm", level), m_cfg(cfg)
{
  if (m_cfg.simulatedHitCollection.empty()) {
    throw std::invalid_argument("Missing input hits collection");
  } else if (m_cfg.simulatedEventCollection.empty()) {
    throw std::invalid_argument("Missing input particle collection");
  } else if (m_cfg.trackCollection.empty()) {
    throw std::invalid_argument("Missing output track collection");
  }
}

template <typename kalman_Fitter_t>
FW::ProcessCode
FW::FittingAlgorithm<kalman_Fitter_t>::execute(
    FW::AlgorithmContext context) const
{
  // Create a random number generator
  FW::RandomEngine generator = m_cfg.randomNumberSvc->spawnGenerator(context);
  FW::GaussDist    gauss(0, 1);

  // Read truth particles from input collection
  const std::vector<FW::Data::SimVertex<>>* simulatedEvent = nullptr;
  if (context.eventStore.get(m_cfg.simulatedEventCollection, simulatedEvent)
      == FW::ProcessCode::ABORT)
    return FW::ProcessCode::ABORT;

  ACTS_DEBUG("Read collection '" << m_cfg.simulatedEventCollection << "' with "
                                 << simulatedEvent->size()
                                 << " vertices");

  // Read truth hits from input collection
  const FW::DetectorData<geo_id_value, Data::SimHit<Data::SimParticle>>* simHits
      = nullptr;
  if (context.eventStore.get(m_cfg.simulatedHitCollection, simHits)
      == FW::ProcessCode::ABORT)
    return FW::ProcessCode::ABORT;

  ACTS_DEBUG("Retrieved hit data '" << m_cfg.simulatedHitCollection
                                    << "' from event store.");

  // Prepare the output data
  TrackMap fittedTracks;

  // Prepare the measurements for KalmanFitter
  ACTS_DEBUG("Prepare the measurements and then tracks");
  TrackMap tracks;
  for (auto& vData : (*simHits)) {
    for (auto& lData : vData.second) {
      for (auto& sData : lData.second) {
        // get the hit
        for (auto& hit : sData.second) {
          // get the barcode of the particle associated to the hit
          barcode_type barcode = hit.particle.barcode();
          // get the surface of hit
          auto hitSurface = hit.surface;
          // get the geoID and transform of the surface
          auto geoID = hitSurface->geoID();

          // transform global into local position
          Acts::Vector2D local(0, 0);
          Acts::Vector3D mom(1, 1, 1);
          hitSurface->globalToLocal(hit.position, mom, local);

          // smear the truth hit with a gaussian and set the covariance
          double                  resX = 30. * Acts::units::_um;
          double                  resY = 30. * Acts::units::_um;
          Acts::ActsSymMatrixD<2> cov2D;
          cov2D << resX * resX, 0., 0., resY * resY;

          double dx = resX * gauss(generator);
          double dy = resY * gauss(generator);

          // move a ,LOC_0, LOC_1 measurement
          Acts::Measurement<Identifier,
                            Acts::ParDef::eLOC_0,
                            Acts::ParDef::eLOC_1>
              m01(hitSurface->getSharedPtr(),
                  geoID,
                  cov2D,
                  local[Acts::ParDef::eLOC_0] + dx,
                  local[Acts::ParDef::eLOC_1] + dy);

          // find the truth particle this hit belongs to via the barcode
          auto trackStates = tracks.find(barcode);
          if (trackStates == tracks.end()) {
            tracks[barcode] = std::vector<TrackState>();
            trackStates     = tracks.find(barcode);
          }
          // insert the measurement for this truth particle
          (trackStates->second).push_back(TrackState(std::move(m01)));
        }  // hit loop
      }    // moudle loop
    }      // layer loop
  }        // volume loop

  ACTS_DEBUG("There are " << tracks.size() << " tracks for this event ");

  // Get the truth particle
  ACTS_DEBUG("Get truth particle.");
  ParticleMap particles;
  for (auto& vertex : *simulatedEvent) {
    for (auto& particle : vertex.outgoing()) {
      particles.insert(std::make_pair(particle.barcode(), particle));
    }
  }

  // Start to perform fit to the prepared tracks
  int itrack = 0;
  for (TrackMap::iterator it = tracks.begin(); it != tracks.end(); ++it) {
    if (it->first == 0) continue;
    itrack++;
    barcode_type barcode  = it->first;
    auto         particle = particles.find(barcode)->second;

    auto measurements = it->second;
    ACTS_DEBUG("Start processing itrack = " << itrack << " with nStates = "
                                            << measurements.size()
                                            << " and truth particle id = "
                                            << barcode);

    // get the truth particle info
    Acts::Vector3D pos = particle.position();
    Acts::Vector3D mom = particle.momentum();
    double         q   = particle.q();
    ACTS_DEBUG("truth position = " << pos[0] << " : " << pos[1] << " : "
                                   << pos[2]);
    ACTS_DEBUG("truth momentum = " << mom[0] << " : " << mom[1] << " : "
                                   << mom[2]);

    // smear the truth particle momentum and position
    Acts::Vector3D dir = mom.normalized();

    double fphi   = Acts::VectorHelpers::phi(mom);
    double ftheta = Acts::VectorHelpers::theta(mom);
    double fp     = mom.norm();
    double fqOp   = q / fp;

    // set the smearing error
    double loc0Res  = 10 * Acts::units::_um;
    double loc1Res  = 10 * Acts::units::_um;
    double phiRes   = 0.02;
    double thetaRes = 0.02;
    double qOpRes   = -q / (fp * fp) * 1.0 * Acts::units::_GeV;

    // prepare the covariance
    Acts::ActsSymMatrixD<5> cov;
    cov << pow(loc0Res, 2), 0., 0., 0., 0., 0., pow(loc1Res, 2), 0., 0., 0., 0.,
        0., pow(phiRes, 2), 0., 0., 0., 0., 0., pow(thetaRes, 2), 0., 0., 0.,
        0., 0., pow(qOpRes, 2);
    auto covPtr = std::make_unique<const Acts::ActsSymMatrixD<5>>(cov);

    // prepare the initial momentum
    double         rPhi   = fphi + phiRes * gauss(generator);
    double         rTheta = ftheta + thetaRes * gauss(generator);
    double         rP     = q / (fqOp + qOpRes * gauss(generator));
    Acts::Vector3D rMom(rP * sin(rTheta) * cos(rPhi),
                        rP * sin(rTheta) * sin(rPhi),
                        rP * cos(rTheta));

    // prepare the initial position
    // the rotation of the starting surface
    Acts::Vector3D T = rMom.normalized();
    Acts::Vector3D U = std::abs(T.dot(Acts::Vector3D::UnitZ())) < 0.99
        ? Acts::Vector3D::UnitZ().cross(T).normalized()
        : Acts::Vector3D::UnitX().cross(T).normalized();
    Acts::Vector3D V = T.cross(U);
    Acts::Vector3D displaced
        = U * loc0Res * gauss(generator) + V * loc1Res * gauss(generator);
    Acts::Vector3D rPos = pos + displaced;

    // then create the start parameters using the prepared momentum and position
    Acts::SingleCurvilinearTrackParameters<Acts::ChargedPolicy> rStart(
        std::move(covPtr), rPos, rMom, q);

    // set the target surface
    const Acts::Surface* rSurface = &rStart.referenceSurface();

    // perform the fit with KalmanFitter
    auto fittedResult = m_cfg.kFitter.fit(measurements, rStart, rSurface);
    ACTS_DEBUG("Finish the fitting.");

    // get the fitted parameters
    if (fittedResult.fittedParameters) {
      ACTS_DEBUG("Get the fitted parameters.");
      auto fittedParameters = fittedResult.fittedParameters.get();
      auto fittedPos        = fittedParameters.position();
      auto fittedMom        = fittedParameters.momentum();
      ACTS_DEBUG("Fitted Position at target = " << fittedPos[0] << " : "
                                                << fittedPos[1]
                                                << " : "
                                                << fittedPos[2]);
      ACTS_DEBUG("fitted Momentum at target = " << fittedMom[0] << " : "
                                                << fittedMom[1]
                                                << " : "
                                                << fittedMom[2]);
    } else
      ACTS_WARNING("No fittedParameter!");

    // get the fitted states
    if (!fittedResult.fittedStates.empty()) {
      ACTS_DEBUG("Get the fitted states.");
      fittedTracks.insert(std::make_pair(barcode, fittedResult.fittedStates));
    }

    // Make sure the fitting is deterministic
    // auto fittedAgainTrack      = m_cfg.kFitter.fit(measurements, rStart,
    // rSurface);
    // ACTS_DEBUG("Finish fitting again");
    // auto fittedAgainParameters = fittedAgainTrack.fittedParameters.get();

    ACTS_DEBUG("Finish processing the track with particle id = " << barcode);
  }

  // Write the fitted tracks to the EventStore
  if (context.eventStore.add(m_cfg.trackCollection, std::move(fittedTracks))
      == FW::ProcessCode::ABORT) {
    return FW::ProcessCode::ABORT;
  }

  return FW::ProcessCode::SUCCESS;
}