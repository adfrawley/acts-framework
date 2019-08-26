// This file is part of the Acts project.
//
// Copyright (C) 2017-2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

///////////////////////////////////////////////////////////////////
// MMMaterialStepAction.hpp
///////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "G4UserSteppingAction.hh"
#include "globals.hh"

namespace FW {
namespace Geant4 {

  /// @class MMSteppingAction
  ///
  /// @brief Collects the RecordedMaterialProperties entities
  ///
  /// The MMSteppingAction class is the implementation of the
  /// Geant4 class SteppingAction. It extracts the weighted material
  /// of every step and collects all material steps.
  class MMSteppingAction : public G4UserSteppingAction
  {
  public:
    /// Constructor
    MMSteppingAction();

    /// Destructor
    ~MMSteppingAction() override;

    /// Static access method to the instance
    static MMSteppingAction*
    Instance();

    /// @brief Interface Method doing the step
    /// @note it creates and collects the MaterialInteraction entities
    /// @param step is the Geant4 step of the particle
    void
    UserSteppingAction(const G4Step* step) final override;

    /// Interface reset method
    /// @note it clears the collected step vector
    void
    Reset();

    /// Access to the collected Acts::MaterialInteraction entities
    std::vector<Acts::MaterialInteraction>
    materialSteps()
    {
      return m_steps;
    }

  private:
    /// Instance of the SteppingAction
    static MMSteppingAction* fgInstance;

    /// The collected Acts::MaterialInteraction entities
    std::vector<Acts::MaterialInteraction> m_steps = {};
  };

}  // namespace Geant4
}  // namespace FW
