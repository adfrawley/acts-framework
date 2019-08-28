// This file is part of the Acts project.
//
// Copyright (C) 2017-2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/MaterialMapping/OutcomeRecording.hpp"
#include <iostream>
#include <stdexcept>
#include "ACTFW/Framework/WhiteBoard.hpp"
#include "ACTFW/Plugins/Geant4/MMDetectorConstruction.hpp"
#include "ACTFW/Plugins/Geant4/OREventAction.hpp"
#include "ACTFW/Plugins/Geant4/ORPrimaryGeneratorAction.hpp"
#include "ACTFW/Plugins/Geant4/MMRunAction.hpp"
#include "ACTFW/Plugins/Geant4/ORSteppingAction.hpp"
#include "FTFP_BERT.hh"

FW::OutcomeRecording::OutcomeRecording(
    const FW::OutcomeRecording::Config& cnf,
    Acts::Logging::Level                 level)
  : FW::BareAlgorithm("OutcomeRecording", level)
  , m_cfg(cnf)
  , m_runManager(std::make_unique<G4RunManager>())
{
  /// Check if the geometry should be accessed over the geant4 service
  if (m_cfg.geant4Service) {
    m_runManager->SetUserInitialization(m_cfg.geant4Service->geant4Geometry());
  } else if (!m_cfg.gdmlFile.empty()) {
    /// Access the geometry from the gdml file
    ACTS_INFO(
        "received Geant4 geometry from GDML file: " << m_cfg.gdmlFile.c_str());
    FW::Geant4::MMDetectorConstruction* detConstruction
        = new FW::Geant4::MMDetectorConstruction();
    detConstruction->setGdmlInput(m_cfg.gdmlFile.c_str());
    m_runManager->SetUserInitialization(
        detConstruction);  // constructs detector (calls Construct in
                           // Geant4DetectorConstruction)
  } else {
    throw std::invalid_argument("Missing geometry input for Geant4");
  }

  /// Now set up the Geant4 simulation
  m_runManager->SetUserInitialization(new FTFP_BERT);
  m_runManager->SetUserAction(new FW::Geant4::ORPrimaryGeneratorAction(
      cnf.pdg, cnf.momentum, cnf.lockAngle, cnf.phi, cnf.theta, cnf.lockPosition, {cnf.pos.x(), cnf.pos.y(), cnf.pos.z()}, m_cfg.seed1, m_cfg.seed2));
  FW::Geant4::MMRunAction* runaction = new FW::Geant4::MMRunAction();
  m_runManager->SetUserAction(runaction);
  FW::Geant4::OREventAction* evtAct = new FW::Geant4::OREventAction();
  m_runManager->SetUserAction(evtAct);
  m_runManager->SetUserAction(new FW::Geant4::ORSteppingAction(evtAct));
  m_runManager->Initialize();
}

FW::ProcessCode
FW::OutcomeRecording::execute(const FW::AlgorithmContext& context) const
{
  // Begin with the simulation
  m_runManager->BeamOn(m_cfg.tracksPerEvent);
  // Retrieve the track material tracks from Geant4
  auto recordedParticles
      = FW::Geant4::OREventAction::Instance()->outcomingParticles();
  ACTS_INFO("Received " << recordedParticles.size()
                        << " particles. Writing them now onto file...");

  // Write the recorded material to the event store
  context.eventStore.add(m_cfg.particleCollection,
                         std::move(recordedParticles));

  return FW::ProcessCode::SUCCESS;
}
