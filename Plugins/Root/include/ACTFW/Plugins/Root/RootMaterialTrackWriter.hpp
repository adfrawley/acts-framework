// This file is part of the Acts project.
//
// Copyright (C) 2017-2019 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include "ACTFW/Framework/IService.hpp"
#include "ACTFW/Framework/ProcessCode.hpp"
#include "ACTFW/Framework/WriterT.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "TTree.h"

namespace Acts {
// Using some short hands for Recorded Material
using RecordedMaterial = MaterialInteractor::result_type;

// And recorded material track
// - this is start:  position, start momentum
//   and the Recorded material
using RecordedMaterialTrack
    = std::pair<std::pair<Acts::Vector3D, Acts::Vector3D>, RecordedMaterial>;
}  // namespace Acts

namespace FW {

namespace Root {

  /// @class RootMaterialTrackWriter
  ///
  /// @brief Writes out MaterialTrack collections from a root file
  ///
  /// This service is the root implementation of the IWriterT.
  /// It writes out a MaterialTrack which is usually generated from
  /// Geant4 material mapping
  class RootMaterialTrackWriter
    : public WriterT<std::vector<Acts::RecordedMaterialTrack>>
  {
  public:
    using Base = WriterT<std::vector<Acts::RecordedMaterialTrack>>;

    /// @brief The nested configuration struct
    struct Config
    {
      std::string collection
          = "material-tracks";            ///< material collection to write
      std::string filePath = "";          ///< path of the output file
      std::string fileMode = "RECREATE";  ///< file access mode
      std::string treeName = "material-tracks";  ///< name of the output tree
      TFile*      rootFile = nullptr;            ///< common root file

      /// Re-calculate total values from individual steps (for cross-checks)
      bool recalculateTotals = false;
    };

    /// Constructor with
    /// @param cfg configuration struct
    /// @param output logging level
    RootMaterialTrackWriter(const Config&        cfg,
                            Acts::Logging::Level level = Acts::Logging::INFO);

    /// Virtual destructor
    ~RootMaterialTrackWriter() override;

    /// Framework intialize method
    FW::ProcessCode
    endRun() final override;

  protected:
    // This implementation holds the actual writing method
    /// and is called by the WriterT<>::write interface
    ///
    /// @param ctx The Algorithm context with per event information
    /// @param clusters is the data to be written out
    ProcessCode
    writeT(const AlgorithmContext&                         ctx,
           const std::vector<Acts::RecordedMaterialTrack>& materialtracks)
        final override;

  private:
    /// The config class
    Config m_cfg;
    /// mutex used to protect multi-threaded writes
    std::mutex m_writeMutex;
    /// The output file name
    TFile* m_outputFile;
    /// The output tree name
    TTree* m_outputTree;

    float m_v_x;    ///< start global x
    float m_v_y;    ///< start global y
    float m_v_z;    ///< start global z
    float m_v_px;   ///< start global momentum x
    float m_v_py;   ///< start global momentum y
    float m_v_pz;   ///< start global momentum z
    float m_v_phi;  ///< start phi direction
    float m_v_eta;  ///< start eta direction
    float m_tX0;    ///< thickness in X0/L0
    float m_tL0;    ///< thickness in X0/L0

    std::vector<float> m_step_x;       ///< step x position
    std::vector<float> m_step_y;       ///< step y position
    std::vector<float> m_step_z;       ///< step z position
    std::vector<float> m_step_length;  ///< step length
    std::vector<float> m_step_X0;      ///< step material x0
    std::vector<float> m_step_L0;      ///< step material l0
    std::vector<float> m_step_A;       ///< step material A
    std::vector<float> m_step_Z;       ///< step material Z
    std::vector<float> m_step_rho;     ///< step material rho
  };

}  // namespace Root
}  // namespace FW
