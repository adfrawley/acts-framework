// This file is part of the Acts project.
//
// Copyright (C) 2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ACTFW/Digitization/DigitizationAlgorithm.hpp"
#include "ACTFW/Digitization/DigitizationOptions.hpp"
#include "ACTFW/EventData/Barcode.hpp"
#include "ACTFW/Framework/RandomNumbers.hpp"
#include "ACTFW/Framework/Sequencer.hpp"
#include "ACTFW/Options/CommonOptions.hpp"
#include "ACTFW/Plugins/Csv/CsvPlanarClusterWriter.hpp"
#include "ACTFW/Plugins/Obj/ObjSpacePointWriter.hpp"
#include "ACTFW/Plugins/Root/RootPlanarClusterWriter.hpp"
#include "Acts/Plugins/Digitization/PlanarModuleStepper.hpp"

template <typename vmap_t>
void
setupDigitization(vmap_t&                            vm,
                  FW::Sequencer&                     sequencer,
                  std::shared_ptr<FW::BarcodeSvc>    barcodeSvc,
                  std::shared_ptr<FW::RandomNumbers> randomNumberSvc)
{
  // Read the standard options
  auto logLevel = FW::Options::readLogLevel(vm);

  // Set the module stepper
  auto pmStepper = std::make_shared<Acts::PlanarModuleStepper>(
      Acts::getDefaultLogger("PlanarModuleStepper", logLevel));

  // Read the digitization configuration
  auto digiConfig = FW::Options::readDigitizationConfig(vm);
  // Set the random number service
  digiConfig.randomNumberSvc     = randomNumberSvc;
  digiConfig.planarModuleStepper = pmStepper;

  // set te hit collection
  digiConfig.simulatedHitCollection
      = vm["fatras-sim-hits"].template as<std::string>();

  // Create the algorithm and add it to the sequencer
  sequencer.addAlgorithm(
      std::make_shared<FW::DigitizationAlgorithm>(digiConfig, logLevel));

  // Output directory
  std::string outputDir = vm["output-dir"].template as<std::string>();

  // Write digitsation output as OBJ files
  if (vm["output-obj"].template as<bool>()) {
    // space points as obj
    FW::Obj::ObjSpacePointWriter<Acts::Vector3D>::Config spWriterObjConfig;
    spWriterObjConfig.collection = digiConfig.spacePointCollection;
    spWriterObjConfig.outputDir  = outputDir;
    auto spWriterObj
        = std::make_shared<FW::Obj::ObjSpacePointWriter<Acts::Vector3D>>(
            spWriterObjConfig);
    // Add to the sequencer
    sequencer.addWriter(spWriterObj);
  }

  // Write digitsation output as Csv files
  if (vm["output-csv"].template as<bool>()) {
    // clusters as root
    FW::Csv::CsvPlanarClusterWriter::Config clusterWriterCsvConfig;
    clusterWriterCsvConfig.collection = digiConfig.clusterCollection;
    clusterWriterCsvConfig.outputDir  = outputDir;
    auto clusteWriterCsv = std::make_shared<FW::Csv::CsvPlanarClusterWriter>(
        clusterWriterCsvConfig);
    // Add to the sequencer
    sequencer.addWriter(clusteWriterCsv);
  }

  // Write digitsation output as OBJ files
  if (vm["output-root"].template as<bool>()) {
    // clusters as root
    FW::Root::RootPlanarClusterWriter::Config clusterWriterRootConfig;
    clusterWriterRootConfig.collection = digiConfig.clusterCollection;
    clusterWriterRootConfig.filePath
        = FW::joinPaths(outputDir, digiConfig.clusterCollection + ".root");
    clusterWriterRootConfig.treeName = digiConfig.clusterCollection;
    auto clusteWriterRoot = std::make_shared<FW::Root::RootPlanarClusterWriter>(
        clusterWriterRootConfig);
    // Add to the sequencer
    sequencer.addWriter(clusteWriterRoot);
  }
}
