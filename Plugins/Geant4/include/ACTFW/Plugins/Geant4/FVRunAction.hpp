///////////////////////////////////////////////////////////////////
// FVRunAction.h
///////////////////////////////////////////////////////////////////

#ifndef ACTFW_PLUGINS_GEANT4_FVRUNACTION_H
#define ACTFW_PLUGINS_GEANT4_FVRUNACTION_H

#include <memory>
#include <vector>
#include "ACTS/EventData/ParticleDefinitions.hpp"
#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;

namespace FW {
namespace G4 {

  /// @class FVRunAction
  ///
  /// @brief Geant4 run action for Fatras validation
  ///
  /// This run action can be used to validate the Fatras simulation.
  /// If used with its corresponding event action (FW::G4::FVEventAction),
  /// stepping action (FW::G4::FVSteppingAction) and
  /// FW::G4::PrimaryGeneratorAction it returns the Acts::ParticleProperties of
  /// the
  /// first step and the last step (when the radial step limit is reached).

  class FVRunAction : public G4UserRunAction
  {
  public:
    /// Constructor
    FVRunAction();

    /// Destructor
    virtual ~FVRunAction();

    /// Static access method
    static FVRunAction*
    Instance();

    /// Interface method at the begin of the run
    /// @note resets the event action
    virtual void
    BeginOfRunAction(const G4Run*);

    /// Interface method at the end of the run
    virtual void
    EndOfRunAction(const G4Run*);

    /// @return A vector of all corresponding first and last
    /// Acts::ParticleProperties of this run
    const std::vector<std::pair<Acts::ParticleProperties,
                                Acts::ParticleProperties>>&
    firstAndLastProperties()
    {
      return m_particleProperties;
    }

  private:
    /// Instance of the FVRunAction
    static FVRunAction* fgInstance;
    /// To collect the first and last particle properties
    std::vector<std::pair<Acts::ParticleProperties, Acts::ParticleProperties>>
        m_particleProperties;
  };
}  // end of namespace G4
}  // end of namespace FW

#endif  // ACTFW_PLUGINS_GEANT4_FVRUNACTION_H
