// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdlib>
#include <memory>

#include <Acts/Utilities/Units.hpp>

#include "ACTFW/Framework/RandomNumbers.hpp"
#include "ACTFW/Framework/Sequencer.hpp"
#include "ACTFW/Generators/ParticleSelector.hpp"
#include "ACTFW/Io/Csv/CsvParticleWriter.hpp"
#include "ACTFW/Io/Root/RootParticleWriter.hpp"
#include "ACTFW/Options/CommonOptions.hpp"
#include "ACTFW/Options/Pythia8Options.hpp"
#include "ACTFW/Printers/PrintParticles.hpp"
#include "ACTFW/Utilities/Paths.hpp"

using namespace Acts::units;
using namespace FW;

int
main(int argc, char* argv[])
{
  // setup and parse options
  auto desc = Options::makeDefaultOptions();
  Options::addSequencerOptions(desc);
  Options::addRandomNumbersOptions(desc);
  Options::addPythia8Options(desc);
  Options::addOutputOptions(desc);
  auto vm = Options::parse(desc, argc, argv);
  if (vm.empty()) { return EXIT_FAILURE; }

  auto              logLevel     = Options::readLogLevel(vm);
  Sequencer::Config sequencerCfg = Options::readSequencerConfig(vm);
  Sequencer         sequencer(sequencerCfg);

  // basic services
  auto rnd
      = std::make_shared<RandomNumbers>(Options::readRandomNumbersConfig(vm));

  // event generation w/ internal pythia8 instance
  EventGenerator::Config evgenCfg = Options::readPythia8Options(vm, logLevel);
  evgenCfg.output                 = "generated_particles";
  evgenCfg.randomNumbers          = rnd;
  sequencer.addReader(std::make_shared<EventGenerator>(evgenCfg, logLevel));

  // event selection
  ParticleSelector::Config selectorCfg;
  selectorCfg.input  = evgenCfg.output;
  selectorCfg.output = "selected_particles";
  //  selectorCfg.absEtaMax = 2.0;
  //  selectorCfg.ptMin     = 0.5 * _GeV;
  selectorCfg.keepNeutral = false;  // retain only charged particles
  sequencer.addAlgorithm(
      std::make_shared<ParticleSelector>(selectorCfg, logLevel));

  // print generated particles
  PrintParticles::Config printCfg;
  printCfg.inputEvent = evgenCfg.output;
  sequencer.addAlgorithm(std::make_shared<PrintParticles>(printCfg, logLevel));

  // different output modes
  std::string outputDir = vm["output-dir"].as<std::string>();
  if (vm["output-csv"].as<bool>()) {
    CsvParticleWriter::Config csvWriterCfg;
    csvWriterCfg.inputEvent = selectorCfg.output;
    csvWriterCfg.outputDir  = outputDir;
    csvWriterCfg.outputStem = "particles";
    sequencer.addWriter(
        std::make_shared<CsvParticleWriter>(csvWriterCfg, logLevel));
  }
  if (vm["output-root"].as<bool>()) {
    RootParticleWriter::Config rootWriterCfg;
    rootWriterCfg.collection = selectorCfg.output;
    rootWriterCfg.filePath   = joinPaths(outputDir, "particles.root");
    sequencer.addWriter(
        std::make_shared<RootParticleWriter>(rootWriterCfg, logLevel));
  }

  return sequencer.run();
}
