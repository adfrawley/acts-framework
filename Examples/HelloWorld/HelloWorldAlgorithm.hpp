// This file is part of the Acts project.
//
// Copyright (C) 2017,2019 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "ACTFW/Framework/BareAlgorithm.hpp"

namespace FW {

/// A simple algorithm that just prints hello world.
class HelloWorldAlgorithm : public FW::BareAlgorithm
{
public:
  HelloWorldAlgorithm(Acts::Logging::Level level);

  // Per-event method.
  FW::ProcessCode
  execute(const AlgorithmContext& context) const final override;
};

}  // namespace FW
