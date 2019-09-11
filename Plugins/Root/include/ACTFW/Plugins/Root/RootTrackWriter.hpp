// This file is part of the Acts project.
//
// Copyright (C) 2017-2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include "ACTFW/Barcode/BarcodeSvc.hpp"
#include "ACTFW/EventData/DataContainers.hpp"
#include "ACTFW/EventData/SimHit.hpp"
#include "ACTFW/EventData/SimParticle.hpp"
#include "ACTFW/EventData/SimVertex.hpp"
#include "ACTFW/Framework/WriterT.hpp"
#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackState.hpp"
#include "Acts/Utilities/GeometryID.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"

class TFile;
class TTree;

namespace FW {
namespace Root {

  using Identifier  = Acts::GeometryID;
  using Measurement = Acts::
      Measurement<Identifier, Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_1>;
  using TrackState = Acts::TrackState<Identifier, Acts::BoundParameters>;
  using TrackMap   = std::map<barcode_type, std::vector<TrackState>>;
  /// Write out a track (i.e. a vector of trackState at the moment) into a TTree
  ///
  /// Safe to use from multiple writer threads - uses a std::mutex lock.
  ///
  /// Each entry in the TTree corresponds to one track for optimum writing
  /// speed. The event number is part of the written data.
  ///
  /// A common file can be provided for to the writer to attach his TTree,
  /// this is done by setting the Config::rootFile pointer to an existing file
  ///
  /// Safe to use from multiple writer threads - uses a std::mutex lock.
  class RootTrackWriter final : public WriterT<TrackMap>
  {
  public:
    using Base = WriterT<TrackMap>;
    /// @brief The nested configuration struct
    struct Config
    {
      std::string trackCollection;           ///< track collection to write
      std::string simulatedEventCollection;  ///< truth particle collection
      std::string simulatedHitCollection;    ///< truth hit collection
      std::string filePath;                  ///< path of the output file
      std::string fileMode = "RECREATE";     ///< file access mode
      std::string treeName = "tracks";       ///< name of the output tree
      TFile*      rootFile = nullptr;        ///< common root file
    };

    /// Constructor
    ///
    /// @param cfg Configuration struct
    /// @param level Message level declaration
    RootTrackWriter(const Config&        cfg,
                    Acts::Logging::Level level = Acts::Logging::INFO);

    /// Virtual destructor
    ~RootTrackWriter() override;

    /// End-of-run hook
    ProcessCode
    endRun() final override;

  protected:
    /// @brief Write method called by the base class
    /// @param [in] ctx is the algorithm context for event information
    /// @param [in] tracks are what to be written out
    ProcessCode
    writeT(const AlgorithmContext& ctx, const TrackMap& tracks) final override;

  private:
    Config     m_cfg;         ///< The config class
    std::mutex m_writeMutex;  ///< Mutex used to protect multi-threaded writes
    TFile*     m_outputFile{nullptr};  ///< The output file
    TTree*     m_outputTree{nullptr};  ///< The output tree
    int        m_eventNr{0};           ///< the event number

    unsigned long m_t_barcode{0};  ///< Truth particle barcode
    int           m_t_charge{0};   ///< Truth particle charge
    float         m_t_vx{0.};      ///< Truth particle vertex x
    float         m_t_vy{0.};      ///< Truth particle vertex y
    float         m_t_vz{0.};      ///< Truth particle vertex z
    float         m_t_px{0.};      ///< Truth particle initial momentum px
    float         m_t_py{0.};      ///< Truth particle initial momentum py
    float         m_t_pz{0.};      ///< Truth particle initial momentum pz
    float         m_t_theta{0.};   ///< Truth particle initial momentum theta
    float         m_t_phi{0.};     ///< Truth particle initial momentum phi
    float         m_t_eta{0.};     ///< Truth particle initial momentum eta
    float         m_t_pT{0.};      ///< Truth particle initial momentum pT

    std::vector<float> m_t_x;  ///< Global truth hit position x
    std::vector<float> m_t_y;  ///< Global truth hit position y
    std::vector<float> m_t_z;  ///< Global truth hit position z
    std::vector<float> m_t_r;  ///< Global truth hit position r
    std::vector<float>
        m_t_dx;  ///< Truth particle direction x at global hit position
    std::vector<float>
        m_t_dy;  ///< Truth particle direction y at global hit position
    std::vector<float>
                       m_t_dz;  ///< Truth particle direction z at global hit position
    std::vector<float> m_t_eLOC0;   ///< truth parameter eLOC_0
    std::vector<float> m_t_eLOC1;   ///< truth parameter eLOC_1
    std::vector<float> m_t_ePHI;    ///< truth parameter ePHI
    std::vector<float> m_t_eTHETA;  ///< truth parameter eTHETA
    std::vector<float> m_t_eQOP;    ///< truth parameter eQOP

    int                m_nStates{0};  ///< number of states
    std::vector<int>   m_volumeID;    ///< volume identifier
    std::vector<int>   m_layerID;     ///< layer identifier
    std::vector<int>   m_moduleID;    ///< surface identifier
    std::vector<float> m_lx_uncalib;  ///< uncalibrated measurement local x
    std::vector<float> m_ly_uncalib;  ///< uncalibrated measurement local y
    std::vector<float>
        m_err_x_uncalib;  ///< uncalibrated measurement resolution x
    std::vector<float>
                       m_err_y_uncalib;  ///< uncalibrated measurement resolution y
    std::vector<float> m_x_uncalib;      ///< uncalibrated measurement global x
    std::vector<float> m_y_uncalib;      ///< uncalibrated measurement global y
    std::vector<float> m_z_uncalib;      ///< uncalibrated measurement global y

    int m_nPredicted{0};       ///< number of states with predicted parameter
    std::vector<bool>  m_prt;  ///< predicted status
    std::vector<float> m_eLOC0_prt;      ///< predicted parameter eLOC0
    std::vector<float> m_eLOC1_prt;      ///< predicted parameter eLOC1
    std::vector<float> m_ePHI_prt;       ///< predicted parameter ePHI
    std::vector<float> m_eTHETA_prt;     ///< predicted parameter eTHETA
    std::vector<float> m_eQOP_prt;       ///< predicted parameter eQOP
    std::vector<float> m_res_eLOC0_prt;  ///< predicted parameter eLOC0 residual
    std::vector<float> m_res_eLOC1_prt;  ///< predicted parameter eLOC1 residual
    std::vector<float> m_res_ePHI_prt;   ///< predicted parameter ePHI residual
    std::vector<float>
                       m_res_eTHETA_prt;  ///< predicted parameter eTHETA residual
    std::vector<float> m_res_eQOP_prt;    ///< predicted parameter eQOP residual
    std::vector<float> m_err_eLOC0_prt;    ///< predicted parameter eLOC0 error
    std::vector<float> m_err_eLOC1_prt;    ///< predicted parameter eLOC1 error
    std::vector<float> m_err_ePHI_prt;     ///< predicted parameter ePHI error
    std::vector<float> m_err_eTHETA_prt;   ///< predicted parameter eTHETA error
    std::vector<float> m_err_eQOP_prt;     ///< predicted parameter eQOP error
    std::vector<float> m_pull_eLOC0_prt;   ///< predicted parameter eLOC0 pull
    std::vector<float> m_pull_eLOC1_prt;   ///< predicted parameter eLOC1 pull
    std::vector<float> m_pull_ePHI_prt;    ///< predicted parameter ePHI pull
    std::vector<float> m_pull_eTHETA_prt;  ///< predicted parameter eTHETA pull
    std::vector<float> m_pull_eQOP_prt;    ///< predicted parameter eQOP pull
    std::vector<float> m_x_prt;            ///< predicted global x
    std::vector<float> m_y_prt;            ///< predicted global y
    std::vector<float> m_z_prt;            ///< predicted global z
    std::vector<float> m_px_prt;           ///< predicted momentum px
    std::vector<float> m_py_prt;           ///< predicted momentum py
    std::vector<float> m_pz_prt;           ///< predicted momentum pz
    std::vector<float> m_eta_prt;          ///< predicted momentum eta
    std::vector<float> m_pT_prt;           ///< predicted momentum pT

    int m_nFiltered{0};        ///< number of states with filtered parameter
    std::vector<bool>  m_flt;  ///< filtered status
    std::vector<float> m_eLOC0_flt;      ///< filtered parameter eLOC0
    std::vector<float> m_eLOC1_flt;      ///< filtered parameter eLOC1
    std::vector<float> m_ePHI_flt;       ///< filtered parameter ePHI
    std::vector<float> m_eTHETA_flt;     ///< filtered parameter eTHETA
    std::vector<float> m_eQOP_flt;       ///< filtered parameter eQOP
    std::vector<float> m_res_eLOC0_flt;  ///< filtered parameter eLOC0 residual
    std::vector<float> m_res_eLOC1_flt;  ///< filtered parameter eLOC1 residual
    std::vector<float> m_res_ePHI_flt;   ///< filtered parameter ePHI residual
    std::vector<float>
                       m_res_eTHETA_flt;  ///< filtered parameter eTHETA residual
    std::vector<float> m_res_eQOP_flt;    ///< filtered parameter eQOP residual
    std::vector<float> m_err_eLOC0_flt;   ///< filtered parameter eLOC0 error
    std::vector<float> m_err_eLOC1_flt;   ///< filtered parameter eLOC1 error
    std::vector<float> m_err_ePHI_flt;    ///< filtered parameter ePHI error
    std::vector<float> m_err_eTHETA_flt;   ///< filtered parameter eTHETA error
    std::vector<float> m_err_eQOP_flt;     ///< filtered parameter eQOP error
    std::vector<float> m_pull_eLOC0_flt;   ///< filtered parameter eLOC0 pull
    std::vector<float> m_pull_eLOC1_flt;   ///< filtered parameter eLOC1 pull
    std::vector<float> m_pull_ePHI_flt;    ///< filtered parameter ePHI pull
    std::vector<float> m_pull_eTHETA_flt;  ///< filtered parameter eTHETA pull
    std::vector<float> m_pull_eQOP_flt;    ///< filtered parameter eQOP pull
    std::vector<float> m_x_flt;            ///< filtered global x
    std::vector<float> m_y_flt;            ///< filtered global y
    std::vector<float> m_z_flt;            ///< filtered global z
    std::vector<float> m_px_flt;           ///< filtered momentum px
    std::vector<float> m_py_flt;           ///< filtered momentum py
    std::vector<float> m_pz_flt;           ///< filtered momentum pz
    std::vector<float> m_eta_flt;          ///< filtered momentum eta
    std::vector<float> m_pT_flt;           ///< filtered momentum pT

    int m_nSmoothed{0};        ///< number of states with smoothed parameter
    std::vector<bool>  m_smt;  ///< smoothed status
    std::vector<float> m_eLOC0_smt;      ///< smoothed parameter eLOC0
    std::vector<float> m_eLOC1_smt;      ///< smoothed parameter eLOC1
    std::vector<float> m_ePHI_smt;       ///< smoothed parameter ePHI
    std::vector<float> m_eTHETA_smt;     ///< smoothed parameter eTHETA
    std::vector<float> m_eQOP_smt;       ///< smoothed parameter eQOP
    std::vector<float> m_res_eLOC0_smt;  ///< smoothed parameter eLOC0 residual
    std::vector<float> m_res_eLOC1_smt;  ///< smoothed parameter eLOC1 residual
    std::vector<float> m_res_ePHI_smt;   ///< smoothed parameter ePHI residual
    std::vector<float>
                       m_res_eTHETA_smt;  ///< smoothed parameter eTHETA residual
    std::vector<float> m_res_eQOP_smt;    ///< smoothed parameter eQOP residual
    std::vector<float> m_err_eLOC0_smt;   ///< smoothed parameter eLOC0 error
    std::vector<float> m_err_eLOC1_smt;   ///< smoothed parameter eLOC1 error
    std::vector<float> m_err_ePHI_smt;    ///< smoothed parameter ePHI error
    std::vector<float> m_err_eTHETA_smt;   ///< smoothed parameter eTHETA error
    std::vector<float> m_err_eQOP_smt;     ///< smoothed parameter eQOP error
    std::vector<float> m_pull_eLOC0_smt;   ///< smoothed parameter eLOC0 pull
    std::vector<float> m_pull_eLOC1_smt;   ///< smoothed parameter eLOC1 pull
    std::vector<float> m_pull_ePHI_smt;    ///< smoothed parameter ePHI pull
    std::vector<float> m_pull_eTHETA_smt;  ///< smoothed parameter eTHETA pull
    std::vector<float> m_pull_eQOP_smt;    ///< smoothed parameter eQOP pull
    std::vector<float> m_x_smt;            ///< smoothed global x
    std::vector<float> m_y_smt;            ///< smoothed global y
    std::vector<float> m_z_smt;            ///< smoothed global z
    std::vector<float> m_px_smt;           ///< smoothed momentum px
    std::vector<float> m_py_smt;           ///< smoothed momentum py
    std::vector<float> m_pz_smt;           ///< smoothed momentum pz
    std::vector<float> m_eta_smt;          ///< smoothed momentum eta
    std::vector<float> m_pT_smt;           ///< smoothed momentum pT
  };

}  // namespace Root
}  // namespace FW
