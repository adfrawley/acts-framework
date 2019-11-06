// This file is part of the Acts project.
//
// Copyright (C) 2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ACTFW/EventData/Barcode.hpp"
#include "ACTFW/EventData/SimHit.hpp"
#include "ACTFW/EventData/SimParticle.hpp"
#include "ACTFW/EventData/SimVertex.hpp"
#include "ACTFW/Fatras/FatrasAlgorithm.hpp"
#include "ACTFW/Fatras/FatrasOptions.hpp"
#include "ACTFW/Framework/RandomNumbers.hpp"
#include "ACTFW/Framework/Sequencer.hpp"
#include "ACTFW/Plugins/BField/BFieldOptions.hpp"
#include "ACTFW/Plugins/BField/ScalableBField.hpp"
#include "ACTFW/Plugins/Csv/CsvParticleWriter.hpp"
#include "ACTFW/Plugins/Root/RootParticleWriter.hpp"
#include "ACTFW/Plugins/Root/RootSimHitWriter.hpp"
#include "ACTFW/Utilities/Paths.hpp"
#include "Acts/Geometry/GeometryID.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/MagneticField/SharedBField.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Propagator/detail/DebugOutputActor.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Fatras/Kernel/Interactor.hpp"
#include "Fatras/Kernel/Process.hpp"
#include "Fatras/Kernel/SelectorList.hpp"
#include "Fatras/Kernel/Simulator.hpp"
#include "Fatras/Physics/EnergyLoss/BetheBloch.hpp"
#include "Fatras/Physics/Scattering/Highland.hpp"
#include "Fatras/Physics/Scattering/Scattering.hpp"
#include "Fatras/Selectors/ChargeSelectors.hpp"
#include "Fatras/Selectors/KinematicCasts.hpp"
#include "Fatras/Selectors/SelectorHelpers.hpp"

typedef FW::Data::SimHit                 FatrasHit;
typedef std::vector<FW::Data::SimVertex> FatrasEvent;

namespace po = boost::program_options;

/// Construct SimHits from Fatras.
struct SimHitCreator
{
  /// @param surface is the Surface where the hit is created
  /// @param position is the global hit position
  /// @param direction is the momentum direction at hit position
  /// @param value is the simulated value
  /// @param time is the timeStamp
  /// @param particle is the particle for the truth link
  FW::Data::SimHit
  operator()(const Acts::Surface&         surface,
             const Acts::Vector3D&        position,
             const Acts::Vector3D&        direction,
             double                       value,
             double                       time,
             const FW::Data::SimParticle& simParticle) const
  {
    FW::Data::SimHit simHit(surface);
    simHit.position  = position;
    simHit.time      = time;
    simHit.direction = direction;
    simHit.value     = value;
    simHit.particle  = simParticle;
    return simHit;
  }
};

/// Simple struct to select sensitive surfaces
struct SurfaceSelector
{
  bool selectSensitive = true;
  bool selectMaterial  = false;
  bool selectPassive   = false;

  /// SurfaceSelector with options
  ///
  /// @param sSensitive is the directive to select sensitive surfaces
  /// @param sMaterial is the directive to select material surfaces
  /// @param sPassive is the directive to select passive surfaces
  SurfaceSelector(bool sSensitive = true,
                  bool sMaterial  = false,
                  bool sPassive   = false)
    : selectSensitive(sSensitive)
    , selectMaterial(sMaterial)
    , selectPassive(sPassive)
  {
  }

  /// Call operator to check if a surface should be selected
  ///
  /// @param surface is the test surface
  bool
  operator()(const Acts::Surface& surface) const
  {
    if (selectSensitive && surface.associatedDetectorElement()) { return true; }
    if (selectMaterial && surface.surfaceMaterial()) { return true; }
    if (selectPassive) { return true; }
    return false;
  }
};

/// @brief Simulation setup for the FatrasAlgorithm
///
/// @tparam bfield_t Type of the bfield for the simulation to be set up
///
/// @param fieldMap The field map for the simulation setup
/// @param sequencer The framework sequencer
/// @param vm The boost variable map to resolve
/// @param tGeometry The TrackingGeometry for the tracking setup
/// @param barcodesSvc The barcode service to be used for the simulation
/// @param randomNumberSvc The random number service to be used for the
/// simulation
template <typename bfield_t>
void
setupSimulationAlgorithm(
    bfield_t                                      fieldMap,
    FW::Sequencer&                                sequencer,
    po::variables_map&                            vm,
    std::shared_ptr<const Acts::TrackingGeometry> tGeometry,
    std::shared_ptr<FW::BarcodeSvc>               barcodeSvc,
    std::shared_ptr<FW::RandomNumbers>            randomNumberSvc)
{
  // Read the log level
  Acts::Logging::Level logLevel = FW::Options::readLogLevel(vm);

  /// Read the evgen particle collection
  std::string evgenCollection = "particles";

  // Create a navigator for this tracking geometry
  Acts::Navigator cNavigator(tGeometry);
  Acts::Navigator nNavigator(tGeometry);

  // using ChargedStepper     = Acts::AtlasStepper<bfield_t>;
  using ChargedStepper    = Acts::EigenStepper<bfield_t>;
  using ChargedPropagator = Acts::Propagator<ChargedStepper, Acts::Navigator>;
  using NeutralStepper    = Acts::StraightLineStepper;
  using NeutralPropagator = Acts::Propagator<NeutralStepper, Acts::Navigator>;

  ChargedStepper    cStepper(std::move(fieldMap));
  ChargedPropagator cPropagator(std::move(cStepper), std::move(cNavigator));
  NeutralStepper    nStepper;
  NeutralPropagator nPropagator(std::move(nStepper), std::move(nNavigator));

  // The Selector for charged particles, including kinematic cuts
  typedef Fatras::ChargedSelector            CSelector;
  typedef Fatras::Max<Fatras::casts::absEta> CMaxEtaAbs;
  typedef Fatras::Min<Fatras::casts::pT>     CMinPt;
  typedef Fatras::SelectorListAND<CSelector, CMinPt, CMaxEtaAbs>
      ChargedSelector;

  typedef Fatras::NeutralSelector                               NSelector;
  typedef Fatras::Max<Fatras::casts::absEta>                    NMaxEtaAbs;
  typedef Fatras::Min<Fatras::casts::E>                         NMinE;
  typedef Fatras::SelectorListAND<NSelector, NMinE, NMaxEtaAbs> NeutralSelector;

  // Accept everything
  typedef Selector All;
  // Define the processes with selectors
  typedef Fatras::Process<Fatras::BetheBloch, All, All, All> BetheBlochProcess;
  // typedef Process<BetheHeitler, All, All, All> BetheHeitlerProcess;
  typedef Fatras::Process<Fatras::Scattering<Fatras::Highland>, All, All, All>
      HighlandProcess;

  typedef Fatras::PhysicsList<BetheBlochProcess, HighlandProcess> PhysicsList;
  // typedef Fatras::PhysicsList<> PhysicsList;

  typedef Fatras::Interactor<FW::RandomEngine,
                             FW::Data::SimParticle,
                             FW::Data::SimHit,
                             SimHitCreator,
                             SurfaceSelector,
                             PhysicsList>
      ChargedInteractor;

  typedef Fatras::Interactor<FW::RandomEngine,
                             FW::Data::SimParticle,
                             FW::Data::SimHit,
                             SimHitCreator>
      NeutralInteractor;

  typedef Fatras::Simulator<ChargedPropagator,
                            ChargedSelector,
                            ChargedInteractor,
                            NeutralPropagator,
                            NeutralSelector,
                            NeutralInteractor>
      FatrasSimulator;

  FatrasSimulator fatrasSimulator(cPropagator, nPropagator);
  fatrasSimulator.debug = vm["fatras-debug-output"].template as<bool>();

  using FatrasAlgorithm = FW::FatrasAlgorithm<FatrasSimulator, FatrasEvent>;

  typename FatrasAlgorithm::Config fatrasConfig
      = FW::Options::readFatrasConfig<po::variables_map,
                                      FatrasSimulator,
                                      FatrasEvent>(vm, fatrasSimulator);
  fatrasConfig.randomNumberSvc      = randomNumberSvc;
  fatrasConfig.inputEventCollection = evgenCollection;

  // Finally the fatras algorithm
  sequencer.addAlgorithm(
      std::make_shared<FatrasAlgorithm>(fatrasConfig, logLevel));

  // Output directory
  std::string outputDir = vm["output-dir"].template as<std::string>();

  // Write simulation information as CSV files
  std::shared_ptr<FW::Csv::CsvParticleWriter> pWriterCsv = nullptr;
  if (vm["output-csv"].template as<bool>()) {
    FW::Csv::CsvParticleWriter::Config pWriterCsvConfig;
    pWriterCsvConfig.inputEvent = fatrasConfig.simulatedEventCollection;
    pWriterCsvConfig.outputDir  = outputDir;
    pWriterCsvConfig.outputStem = fatrasConfig.simulatedEventCollection;
    sequencer.addWriter(
        std::make_shared<FW::Csv::CsvParticleWriter>(pWriterCsvConfig));
  }

  // Write simulation information as ROOT files
  std::shared_ptr<FW::Root::RootParticleWriter> pWriterRoot = nullptr;
  if (vm["output-root"].template as<bool>()) {
    // Write particles as ROOT TTree
    FW::Root::RootParticleWriter::Config pWriterRootConfig;
    pWriterRootConfig.collection = fatrasConfig.simulatedEventCollection;
    pWriterRootConfig.filePath   = FW::joinPaths(
        outputDir, fatrasConfig.simulatedEventCollection + ".root");
    pWriterRootConfig.treeName   = fatrasConfig.simulatedEventCollection;
    pWriterRootConfig.barcodeSvc = barcodeSvc;
    sequencer.addWriter(
        std::make_shared<FW::Root::RootParticleWriter>(pWriterRootConfig));

    // Write simulated hits as ROOT TTree
    FW::Root::RootSimHitWriter::Config fhitWriterRootConfig;
    fhitWriterRootConfig.collection = fatrasConfig.simulatedHitCollection;
    fhitWriterRootConfig.filePath   = FW::joinPaths(
        outputDir, fatrasConfig.simulatedHitCollection + ".root");
    fhitWriterRootConfig.treeName = fatrasConfig.simulatedHitCollection;
    sequencer.addWriter(
        std::make_shared<FW::Root::RootSimHitWriter>(fhitWriterRootConfig));
  }
}

/// @brief Simulation setup
///
/// @tparam bfield_t Type of the bfield for the simulation to be set up
///
/// @param fieldMap The field map for the simulation setup
/// @param sequencer The framework sequencer
/// @param vm The boost variable map to resolve
/// @param tGeometry The TrackingGeometry for the tracking setup
/// @param barcodesSvc The barcode service to be used for the simulation
/// @param randomNumberSvc The random number service to be used for the
/// simulation
template <typename vmap_t>
void
setupSimulation(vmap_t&                                       vm,
                FW::Sequencer&                                sequencer,
                std::shared_ptr<const Acts::TrackingGeometry> tGeometry,
                std::shared_ptr<FW::BarcodeSvc>               barcodeSvc,
                std::shared_ptr<FW::RandomNumbers>            randomNumberSvc)
{
  // Create BField service
  auto bField  = FW::Options::readBField(vm);
  auto field2D = std::get<std::shared_ptr<InterpolatedBFieldMap2D>>(bField);
  auto field3D = std::get<std::shared_ptr<InterpolatedBFieldMap3D>>(bField);

  if (field2D) {
    // Define the interpolated b-field
    using BField = Acts::SharedBField<InterpolatedBFieldMap2D>;
    BField fieldMap(field2D);
    // now setup of the simulation and append it to the sequencer
    setupSimulationAlgorithm(std::move(fieldMap),
                             sequencer,
                             vm,
                             tGeometry,
                             barcodeSvc,
                             randomNumberSvc);
  } else if (field3D) {
    // Define the interpolated b-field
    using BField = Acts::SharedBField<InterpolatedBFieldMap3D>;
    BField fieldMap(field3D);
    // now setup of the simulation and append it to the sequencer
    setupSimulationAlgorithm(std::move(fieldMap),
                             sequencer,
                             vm,
                             tGeometry,
                             barcodeSvc,
                             randomNumberSvc);
  } else if (vm["bf-context-scalable"].template as<bool>()) {
    using SField = FW::BField::ScalableBField;
    SField fieldMap(*std::get<std::shared_ptr<SField>>(bField));
    // now setup of the simulation and append it to the sequencer
    setupSimulationAlgorithm(std::move(fieldMap),
                             sequencer,
                             vm,
                             tGeometry,
                             barcodeSvc,
                             randomNumberSvc);
  } else {
    // Create the constant  field
    using CField = Acts::ConstantBField;
    CField fieldMap(*std::get<std::shared_ptr<CField>>(bField));
    // now setup of the simulation and append it to the sequencer
    setupSimulationAlgorithm(std::move(fieldMap),
                             sequencer,
                             vm,
                             tGeometry,
                             barcodeSvc,
                             randomNumberSvc);
  }
}
