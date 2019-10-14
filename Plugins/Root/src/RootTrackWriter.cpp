// This file is part of the Acts project.
//
// Copyright (C) 2017 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Plugins/Root/RootTrackWriter.hpp"
#include <TFile.h>
#include <TTree.h>
#include <ios>
#include <stdexcept>
#include "Acts/Utilities/Helpers.hpp"

using Acts::VectorHelpers::eta;
using Acts::VectorHelpers::theta;
using Acts::VectorHelpers::phi;
using Acts::VectorHelpers::perp;

FW::Root::RootTrackWriter::RootTrackWriter(
    const FW::Root::RootTrackWriter::Config& cfg,
    Acts::Logging::Level                     level)
  : Base(cfg.trackCollection, "RootTrackWriter", level)
  , m_cfg(cfg)
  , m_outputFile(cfg.rootFile)
{
  // An input collection name and tree name must be specified
  if (m_cfg.trackCollection.empty()) {
    throw std::invalid_argument("Missing input track collection");
  } else if (m_cfg.simulatedEventCollection.empty()) {
    throw std::invalid_argument("Missing input particle collection");
  } else if (m_cfg.simulatedHitCollection.empty()) {
    throw std::invalid_argument("Missing input hit collection");
  } else if (m_cfg.treeName.empty()) {
    throw std::invalid_argument("Missing tree name");
  }

  // Setup ROOT I/O
  if (m_outputFile == nullptr) {
    m_outputFile = TFile::Open(m_cfg.filePath.c_str(), m_cfg.fileMode.c_str());
    if (m_outputFile == nullptr) {
      throw std::ios_base::failure("Could not open '" + m_cfg.filePath);
    }
  }
  m_outputFile->cd();
  m_outputTree = new TTree(m_cfg.treeName.c_str(), m_cfg.treeName.c_str());
  if (m_outputTree == nullptr)
    throw std::bad_alloc();
  else {
    // I/O parameters
    m_outputTree->Branch("event_nr", &m_eventNr);
    m_outputTree->Branch("track_nr", &m_trackNr);
    m_outputTree->Branch("t_barcode", &m_t_barcode, "t_barcode/l");
    m_outputTree->Branch("t_charge", &m_t_charge);
    m_outputTree->Branch("t_vx", &m_t_vx);
    m_outputTree->Branch("t_vy", &m_t_vy);
    m_outputTree->Branch("t_vz", &m_t_vz);
    m_outputTree->Branch("t_px", &m_t_px);
    m_outputTree->Branch("t_py", &m_t_py);
    m_outputTree->Branch("t_pz", &m_t_pz);
    m_outputTree->Branch("t_theta", &m_t_theta);
    m_outputTree->Branch("t_phi", &m_t_phi);
    m_outputTree->Branch("t_eta", &m_t_eta);
    m_outputTree->Branch("t_pT", &m_t_pT);

    m_outputTree->Branch("t_x", &m_t_x);
    m_outputTree->Branch("t_y", &m_t_y);
    m_outputTree->Branch("t_z", &m_t_z);
    m_outputTree->Branch("t_r", &m_t_r);
    m_outputTree->Branch("t_dx", &m_t_dx);
    m_outputTree->Branch("t_dy", &m_t_dy);
    m_outputTree->Branch("t_dz", &m_t_dz);
    m_outputTree->Branch("t_eLOC0", &m_t_eLOC0);
    m_outputTree->Branch("t_eLOC1", &m_t_eLOC1);
    m_outputTree->Branch("t_ePHI", &m_t_ePHI);
    m_outputTree->Branch("t_eTHETA", &m_t_eTHETA);
    m_outputTree->Branch("t_eQOP", &m_t_eQOP);

    m_outputTree->Branch("nStates", &m_nStates);
    m_outputTree->Branch("volume_id", &m_volumeID);
    m_outputTree->Branch("layer_id", &m_layerID);
    m_outputTree->Branch("module_id", &m_moduleID);
    m_outputTree->Branch("l_x_hit", &m_lx_hit);
    m_outputTree->Branch("l_y_hit", &m_ly_hit);
    m_outputTree->Branch("g_x_hit", &m_x_hit);
    m_outputTree->Branch("g_y_hit", &m_y_hit);
    m_outputTree->Branch("g_z_hit", &m_z_hit);
    m_outputTree->Branch("res_x_hit", &m_res_x_hit);
    m_outputTree->Branch("res_y_hit", &m_res_y_hit);
    m_outputTree->Branch("err_x_hit", &m_err_x_hit);
    m_outputTree->Branch("err_y_hit", &m_err_y_hit);
    m_outputTree->Branch("pull_x_hit", &m_pull_x_hit);
    m_outputTree->Branch("pull_y_hit", &m_pull_y_hit);

    m_outputTree->Branch("nPredicted", &m_nPredicted);
    m_outputTree->Branch("predicted", &m_prt);
    m_outputTree->Branch("eLOC0_prt", &m_eLOC0_prt);
    m_outputTree->Branch("eLOC1_prt", &m_eLOC1_prt);
    m_outputTree->Branch("ePHI_prt", &m_ePHI_prt);
    m_outputTree->Branch("eTHETA_prt", &m_eTHETA_prt);
    m_outputTree->Branch("eQOP_prt", &m_eQOP_prt);
    m_outputTree->Branch("res_eLOC0_prt", &m_res_eLOC0_prt);
    m_outputTree->Branch("res_eLOC1_prt", &m_res_eLOC1_prt);
    m_outputTree->Branch("res_ePHI_prt", &m_res_ePHI_prt);
    m_outputTree->Branch("res_eTHETA_prt", &m_res_eTHETA_prt);
    m_outputTree->Branch("res_eQOP_prt", &m_res_eQOP_prt);
    m_outputTree->Branch("err_eLOC0_prt", &m_err_eLOC0_prt);
    m_outputTree->Branch("err_eLOC1_prt", &m_err_eLOC1_prt);
    m_outputTree->Branch("err_ePHI_prt", &m_err_ePHI_prt);
    m_outputTree->Branch("err_eTHETA_prt", &m_err_eTHETA_prt);
    m_outputTree->Branch("err_eQOP_prt", &m_err_eQOP_prt);
    m_outputTree->Branch("pull_eLOC0_prt", &m_pull_eLOC0_prt);
    m_outputTree->Branch("pull_eLOC1_prt", &m_pull_eLOC1_prt);
    m_outputTree->Branch("pull_ePHI_prt", &m_pull_ePHI_prt);
    m_outputTree->Branch("pull_eTHETA_prt", &m_pull_eTHETA_prt);
    m_outputTree->Branch("pull_eQOP_prt", &m_pull_eQOP_prt);
    m_outputTree->Branch("g_x_prt", &m_x_prt);
    m_outputTree->Branch("g_y_prt", &m_y_prt);
    m_outputTree->Branch("g_z_prt", &m_z_prt);
    m_outputTree->Branch("px_prt", &m_px_prt);
    m_outputTree->Branch("py_prt", &m_py_prt);
    m_outputTree->Branch("pz_prt", &m_pz_prt);
    m_outputTree->Branch("eta_prt", &m_eta_prt);
    m_outputTree->Branch("pT_prt", &m_pT_prt);

    m_outputTree->Branch("nFiltered", &m_nFiltered);
    m_outputTree->Branch("filtered", &m_flt);
    m_outputTree->Branch("eLOC0_flt", &m_eLOC0_flt);
    m_outputTree->Branch("eLOC1_flt", &m_eLOC1_flt);
    m_outputTree->Branch("ePHI_flt", &m_ePHI_flt);
    m_outputTree->Branch("eTHETA_flt", &m_eTHETA_flt);
    m_outputTree->Branch("eQOP_flt", &m_eQOP_flt);
    m_outputTree->Branch("res_eLOC0_flt", &m_res_eLOC0_flt);
    m_outputTree->Branch("res_eLOC1_flt", &m_res_eLOC1_flt);
    m_outputTree->Branch("res_ePHI_flt", &m_res_ePHI_flt);
    m_outputTree->Branch("res_eTHETA_flt", &m_res_eTHETA_flt);
    m_outputTree->Branch("res_eQOP_flt", &m_res_eQOP_flt);
    m_outputTree->Branch("err_eLOC0_flt", &m_err_eLOC0_flt);
    m_outputTree->Branch("err_eLOC1_flt", &m_err_eLOC1_flt);
    m_outputTree->Branch("err_ePHI_flt", &m_err_ePHI_flt);
    m_outputTree->Branch("err_eTHETA_flt", &m_err_eTHETA_flt);
    m_outputTree->Branch("err_eQOP_flt", &m_err_eQOP_flt);
    m_outputTree->Branch("pull_eLOC0_flt", &m_pull_eLOC0_flt);
    m_outputTree->Branch("pull_eLOC1_flt", &m_pull_eLOC1_flt);
    m_outputTree->Branch("pull_ePHI_flt", &m_pull_ePHI_flt);
    m_outputTree->Branch("pull_eTHETA_flt", &m_pull_eTHETA_flt);
    m_outputTree->Branch("pull_eQOP_flt", &m_pull_eQOP_flt);
    m_outputTree->Branch("g_x_flt", &m_x_flt);
    m_outputTree->Branch("g_y_flt", &m_y_flt);
    m_outputTree->Branch("g_z_flt", &m_z_flt);
    m_outputTree->Branch("px_flt", &m_px_flt);
    m_outputTree->Branch("py_flt", &m_py_flt);
    m_outputTree->Branch("pz_flt", &m_pz_flt);
    m_outputTree->Branch("eta_flt", &m_eta_flt);
    m_outputTree->Branch("pT_flt", &m_pT_flt);

    m_outputTree->Branch("nSmoothed", &m_nSmoothed);
    m_outputTree->Branch("smoothed", &m_smt);
    m_outputTree->Branch("eLOC0_smt", &m_eLOC0_smt);
    m_outputTree->Branch("eLOC1_smt", &m_eLOC1_smt);
    m_outputTree->Branch("ePHI_smt", &m_ePHI_smt);
    m_outputTree->Branch("eTHETA_smt", &m_eTHETA_smt);
    m_outputTree->Branch("eQOP_smt", &m_eQOP_smt);
    m_outputTree->Branch("res_eLOC0_smt", &m_res_eLOC0_smt);
    m_outputTree->Branch("res_eLOC1_smt", &m_res_eLOC1_smt);
    m_outputTree->Branch("res_ePHI_smt", &m_res_ePHI_smt);
    m_outputTree->Branch("res_eTHETA_smt", &m_res_eTHETA_smt);
    m_outputTree->Branch("res_eQOP_smt", &m_res_eQOP_smt);
    m_outputTree->Branch("err_eLOC0_smt", &m_err_eLOC0_smt);
    m_outputTree->Branch("err_eLOC1_smt", &m_err_eLOC1_smt);
    m_outputTree->Branch("err_ePHI_smt", &m_err_ePHI_smt);
    m_outputTree->Branch("err_eTHETA_smt", &m_err_eTHETA_smt);
    m_outputTree->Branch("err_eQOP_smt", &m_err_eQOP_smt);
    m_outputTree->Branch("pull_eLOC0_smt", &m_pull_eLOC0_smt);
    m_outputTree->Branch("pull_eLOC1_smt", &m_pull_eLOC1_smt);
    m_outputTree->Branch("pull_ePHI_smt", &m_pull_ePHI_smt);
    m_outputTree->Branch("pull_eTHETA_smt", &m_pull_eTHETA_smt);
    m_outputTree->Branch("pull_eQOP_smt", &m_pull_eQOP_smt);
    m_outputTree->Branch("g_x_smt", &m_x_smt);
    m_outputTree->Branch("g_y_smt", &m_y_smt);
    m_outputTree->Branch("g_z_smt", &m_z_smt);
    m_outputTree->Branch("px_smt", &m_px_smt);
    m_outputTree->Branch("py_smt", &m_py_smt);
    m_outputTree->Branch("pz_smt", &m_pz_smt);
    m_outputTree->Branch("eta_smt", &m_eta_smt);
    m_outputTree->Branch("pT_smt", &m_pT_smt);
  }
}

FW::Root::RootTrackWriter::~RootTrackWriter()
{
  if (m_outputFile) {
    m_outputFile->Close();
  }
}

FW::ProcessCode
FW::Root::RootTrackWriter::endRun()
{
  if (m_outputFile) {
    m_outputFile->cd();
    m_outputTree->Write();
    ACTS_INFO("Wrote tracks to tree '" << m_cfg.treeName << "' in '"
                                       << m_cfg.filePath
                                       << "'");
  }
  return ProcessCode::SUCCESS;
}

FW::ProcessCode
FW::Root::RootTrackWriter::writeT(const AlgorithmContext& ctx,
                                  const TrackMap&         tracks)
{

  if (m_outputFile == nullptr) return ProcessCode::SUCCESS;

  // Read truth particles from input collection
  const std::vector<Data::SimVertex<>>* simulatedEvent = nullptr;
  simulatedEvent = &ctx.eventStore.get<std::vector<Data::SimVertex<>>>(
      m_cfg.simulatedEventCollection);
  if (!simulatedEvent) {
    throw std::ios_base::failure("Retrieve truth particle collection "
                                 + m_cfg.simulatedEventCollection
                                 + " failure!");
  }

  ACTS_DEBUG("Read collection '" << m_cfg.simulatedEventCollection << "' with "
                                 << simulatedEvent->size()
                                 << " vertices");

  // Read truth hits from input collection
  const FW::DetectorData<geo_id_value, Data::SimHit<Data::SimParticle>>* simHits
      = nullptr;
  simHits
      = &ctx.eventStore.get<FW::DetectorData<geo_id_value,
                                             Data::SimHit<Data::SimParticle>>>(
          m_cfg.simulatedHitCollection);
  if (!simHits) {
    throw std::ios_base::failure("Retrieve truth hit collection "
                                 + m_cfg.simulatedHitCollection
                                 + " failure!");
  }

  ACTS_DEBUG("Retrieved hit data '" << m_cfg.simulatedHitCollection
                                    << "' from event store.");

  // Get the map of truth particle
  ACTS_DEBUG("Get the truth particles.");
  std::map<barcode_type, Data::SimParticle> particles;
  for (auto& vertex : *simulatedEvent) {
    for (auto& particle : vertex.outgoing()) {
      particles.insert(std::make_pair(particle.barcode(), particle));
    }
  }

  // Get the map of truth hits on a module
  ACTS_DEBUG("Get the truth hits.");
  std::map<Acts::GeometryID, std::vector<Data::SimHit<Data::SimParticle>>>
      hitsOnModule;
  for (auto& vData : (*simHits)) {
    for (auto& lData : vData.second) {
      for (auto& sData : lData.second) {
        for (auto& hit : sData.second) {
          auto geoID = hit.surface->geoID();
          auto hits  = hitsOnModule.find(geoID);
          if (hits == hitsOnModule.end()) {
            hitsOnModule[geoID]
                = std::vector<Data::SimHit<Data::SimParticle>>();
            hits = hitsOnModule.find(geoID);
          }
          (hits->second).push_back(hit);
        }
      }
    }
  }

  // Exclusive access to the tree while writing
  std::lock_guard<std::mutex> lock(m_writeMutex);

  // Get the event number
  m_eventNr = ctx.eventNumber;

  // Loop over the tracks
  int iTrack = 0;
  for (auto& track : tracks) {
    /// collect the information
    m_trackNr = iTrack;
    // get the truth particle info
    m_t_barcode = track.first;
    if (particles.find(m_t_barcode) != particles.end()) {
      ACTS_DEBUG("Find the truth particle with barcode = " << m_t_barcode);
      auto           particle = particles.find(m_t_barcode)->second;
      Acts::Vector3D truthPos = particle.position();
      Acts::Vector3D truthMom = particle.momentum();
      m_t_charge              = particle.q();
      m_t_vx                  = truthPos.x();
      m_t_vy                  = truthPos.y();
      m_t_vz                  = truthPos.z();
      m_t_px                  = truthMom.x();
      m_t_py                  = truthMom.y();
      m_t_pz                  = truthMom.z();
      m_t_theta               = theta(truthMom);
      m_t_phi                 = phi(truthMom);
      m_t_pT                  = perp(truthMom);
      m_t_eta                 = eta(truthMom);
    } else {
      ACTS_WARNING("Truth particle with barcode = " << m_t_barcode
                                                    << " not found!");
    }

    // get the trackState info
    m_nStates    = track.second.size();
    m_nPredicted = 0;
    m_nFiltered  = 0;
    m_nSmoothed  = 0;
    for (auto& state : track.second) {

      // get the geometry ID
      auto geoID = state.referenceSurface().geoID();
      m_volumeID.push_back(geoID.value(Acts::GeometryID::volume_mask));
      m_layerID.push_back(geoID.value(Acts::GeometryID::layer_mask));
      m_moduleID.push_back(geoID.value(Acts::GeometryID::sensitive_mask));

      // get the uncalibrated measurement
      auto meas = std::get<Measurement>(*state.measurement.uncalibrated);
      // get local position
      Acts::Vector2D local(meas.parameters()[Acts::ParDef::eLOC_0],
                           meas.parameters()[Acts::ParDef::eLOC_1]);
      // get global position
      Acts::Vector3D global(0, 0, 0);
      Acts::Vector3D mom(1, 1, 1);
      meas.referenceSurface().localToGlobal(ctx.geoContext, local, mom, global);

      // get measurement covariance
      auto  cov  = meas.covariance();
      float resX = sqrt(cov(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0));
      float resY = sqrt(cov(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1));

      // push the measurement info
      m_lx_hit.push_back(local.x());
      m_ly_hit.push_back(local.y());
      m_x_hit.push_back(global.x());
      m_y_hit.push_back(global.y());
      m_z_hit.push_back(global.z());

      // get all truth hits on this module
      auto hitsOnThisModule = hitsOnModule.find(geoID)->second;

      // lambda to find the truth hit belonging to a given truth track
      barcode_type                    t_barcode = track.first;
      Data::SimHit<Data::SimParticle> truthHit;
      auto                            findTruthHit = [&t_barcode, &truthHit](
          std::vector<Data::SimHit<Data::SimParticle>> hits) -> bool {
        for (auto& hit : hits) {
          if (hit.particle.barcode() == t_barcode) {
            truthHit = hit;
            return true;
          }
        }
        return false;
      };

      // get the truth hit corresponding to this trackState
      float truthLOC0 = 0, truthLOC1 = 0, truthPHI = 0, truthTHETA = 0,
            truthQOP = 0;
      if (findTruthHit(hitsOnThisModule)) {
        ACTS_DEBUG("Find the truth hit for trackState on"
                   << " : volume = "
                   << geoID.value(Acts::GeometryID::volume_mask)
                   << " : layer = "
                   << geoID.value(Acts::GeometryID::layer_mask)
                   << " : module = "
                   << geoID.value(Acts::GeometryID::sensitive_mask));
        // get local truth position
        Acts::Vector2D hitlocal;
        meas.referenceSurface().globalToLocal(
            ctx.geoContext, truthHit.position, truthHit.direction, hitlocal);
        // push the truth hit info
        m_t_x.push_back(truthHit.position.x());
        m_t_y.push_back(truthHit.position.y());
        m_t_z.push_back(truthHit.position.z());
        m_t_r.push_back(perp(truthHit.position));
        m_t_dx.push_back(truthHit.direction.x());
        m_t_dy.push_back(truthHit.direction.y());
        m_t_dz.push_back(truthHit.direction.z());
        truthLOC0  = hitlocal.x();
        truthLOC1  = hitlocal.y();
        truthPHI   = phi(truthHit.particle.momentum());
        truthTHETA = theta(truthHit.particle.momentum());
        truthQOP   = m_t_charge / truthHit.particle.momentum().norm();
      } else {
        ACTS_WARNING("Truth hit for trackState on"
                     << " : volume = "
                     << geoID.value(Acts::GeometryID::volume_mask)
                     << " : layer = "
                     << geoID.value(Acts::GeometryID::layer_mask)
                     << " : module = "
                     << geoID.value(Acts::GeometryID::sensitive_mask)
                     << " not found!");
        // push default values if truth hit not found
        m_t_x.push_back(0.);
        m_t_y.push_back(0.);
        m_t_z.push_back(0.);
        m_t_r.push_back(0.);
        m_t_dx.push_back(0.);
        m_t_dy.push_back(0.);
        m_t_dz.push_back(0.);
      }
      // push the truth parameter at this track State
      m_t_eLOC0.push_back(truthLOC0);
      m_t_eLOC1.push_back(truthLOC1);
      m_t_ePHI.push_back(truthPHI);
      m_t_eTHETA.push_back(truthTHETA);
      m_t_eQOP.push_back(truthQOP);

      // get the predicted parameter
      bool predicted = false;
      if (state.parameter.predicted) {
        predicted = true;
        m_nPredicted++;
        auto parameter  = *state.parameter.predicted;
        auto covariance = *parameter.covariance();
        // local hit residual info
        auto H        = meas.projector();
        auto resCov   = cov + H * covariance * H.transpose();
        auto residual = meas.residual(parameter);
        m_res_x_hit.push_back(residual(Acts::ParDef::eLOC_0));
        m_res_y_hit.push_back(residual(Acts::ParDef::eLOC_1));
        m_err_x_hit.push_back(
            sqrt(resCov(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_err_y_hit.push_back(
            sqrt(resCov(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_pull_x_hit.push_back(
            residual(Acts::ParDef::eLOC_0)
            / sqrt(resCov(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_pull_y_hit.push_back(
            residual(Acts::ParDef::eLOC_1)
            / sqrt(resCov(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));

        // predicted parameter info
        m_eLOC0_prt.push_back(parameter.parameters()[Acts::ParDef::eLOC_0]);
        m_eLOC1_prt.push_back(parameter.parameters()[Acts::ParDef::eLOC_1]);
        m_ePHI_prt.push_back(parameter.parameters()[Acts::ParDef::ePHI]);
        m_eTHETA_prt.push_back(parameter.parameters()[Acts::ParDef::eTHETA]);
        m_eQOP_prt.push_back(parameter.parameters()[Acts::ParDef::eQOP]);
        m_res_eLOC0_prt.push_back(parameter.parameters()[Acts::ParDef::eLOC_0]
                                  - truthLOC0);
        m_res_eLOC1_prt.push_back(parameter.parameters()[Acts::ParDef::eLOC_1]
                                  - truthLOC1);
        m_res_ePHI_prt.push_back(parameter.parameters()[Acts::ParDef::ePHI]
                                 - truthPHI);
        m_res_eTHETA_prt.push_back(parameter.parameters()[Acts::ParDef::eTHETA]
                                   - truthTHETA);
        m_res_eQOP_prt.push_back(parameter.parameters()[Acts::ParDef::eQOP]
                                 - truthQOP);
        m_err_eLOC0_prt.push_back(
            sqrt(covariance(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_err_eLOC1_prt.push_back(
            sqrt(covariance(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_err_ePHI_prt.push_back(
            sqrt(covariance(Acts::ParDef::ePHI, Acts::ParDef::ePHI)));
        m_err_eTHETA_prt.push_back(
            sqrt(covariance(Acts::ParDef::eTHETA, Acts::ParDef::eTHETA)));
        m_err_eQOP_prt.push_back(
            sqrt(covariance(Acts::ParDef::eQOP, Acts::ParDef::eQOP)));
        m_pull_eLOC0_prt.push_back(
            (parameter.parameters()[Acts::ParDef::eLOC_0] - truthLOC0)
            / sqrt(covariance(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_pull_eLOC1_prt.push_back(
            (parameter.parameters()[Acts::ParDef::eLOC_1] - truthLOC1)
            / sqrt(covariance(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_pull_ePHI_prt.push_back(
            (parameter.parameters()[Acts::ParDef::ePHI] - truthPHI)
            / sqrt(covariance(Acts::ParDef::ePHI, Acts::ParDef::ePHI)));
        m_pull_eTHETA_prt.push_back(
            (parameter.parameters()[Acts::ParDef::eTHETA] - truthTHETA)
            / sqrt(covariance(Acts::ParDef::eTHETA, Acts::ParDef::eTHETA)));
        m_pull_eQOP_prt.push_back(
            (parameter.parameters()[Acts::ParDef::eQOP] - truthQOP)
            / sqrt(covariance(Acts::ParDef::eQOP, Acts::ParDef::eQOP)));
        m_x_prt.push_back(parameter.position().x());
        m_y_prt.push_back(parameter.position().y());
        m_z_prt.push_back(parameter.position().z());
        m_px_prt.push_back(parameter.momentum().x());
        m_py_prt.push_back(parameter.momentum().y());
        m_pz_prt.push_back(parameter.momentum().z());
        m_pT_prt.push_back(parameter.pT());
        m_eta_prt.push_back(eta(parameter.position()));
      } else {
        // push default values if no predicted parameter
        m_eLOC0_prt.push_back(-99.);
        m_eLOC1_prt.push_back(-99.);
        m_ePHI_prt.push_back(-99.);
        m_eTHETA_prt.push_back(-99.);
        m_eQOP_prt.push_back(-99.);
        m_res_eLOC0_prt.push_back(-99.);
        m_res_eLOC1_prt.push_back(-99.);
        m_res_ePHI_prt.push_back(-99.);
        m_res_eTHETA_prt.push_back(-99.);
        m_res_eQOP_prt.push_back(-99.);
        m_err_eLOC0_prt.push_back(-99);
        m_err_eLOC1_prt.push_back(-99);
        m_err_ePHI_prt.push_back(-99);
        m_err_eTHETA_prt.push_back(-99);
        m_err_eQOP_prt.push_back(-99);
        m_pull_eLOC0_prt.push_back(-99.);
        m_pull_eLOC1_prt.push_back(-99.);
        m_pull_ePHI_prt.push_back(-99.);
        m_pull_eTHETA_prt.push_back(-99.);
        m_pull_eQOP_prt.push_back(-99.);
        m_x_prt.push_back(-99.);
        m_y_prt.push_back(-99.);
        m_z_prt.push_back(-99.);
        m_px_prt.push_back(-99.);
        m_py_prt.push_back(-99.);
        m_pz_prt.push_back(-99.);
        m_pT_prt.push_back(-99.);
        m_eta_prt.push_back(-99.);
      }

      // get the filtered parameter
      bool filtered = false;
      if (state.parameter.filtered) {
        filtered = true;
        m_nFiltered++;
        auto parameter  = *state.parameter.filtered;
        auto covariance = *parameter.covariance();
        // filtered parameter info
        m_eLOC0_flt.push_back(parameter.parameters()[Acts::ParDef::eLOC_0]);
        m_eLOC1_flt.push_back(parameter.parameters()[Acts::ParDef::eLOC_1]);
        m_ePHI_flt.push_back(parameter.parameters()[Acts::ParDef::ePHI]);
        m_eTHETA_flt.push_back(parameter.parameters()[Acts::ParDef::eTHETA]);
        m_eQOP_flt.push_back(parameter.parameters()[Acts::ParDef::eQOP]);
        m_res_eLOC0_flt.push_back(parameter.parameters()[Acts::ParDef::eLOC_0]
                                  - truthLOC0);
        m_res_eLOC1_flt.push_back(parameter.parameters()[Acts::ParDef::eLOC_1]
                                  - truthLOC1);
        m_res_ePHI_flt.push_back(parameter.parameters()[Acts::ParDef::ePHI]
                                 - truthPHI);
        m_res_eTHETA_flt.push_back(parameter.parameters()[Acts::ParDef::eTHETA]
                                   - truthTHETA);
        m_res_eQOP_flt.push_back(parameter.parameters()[Acts::ParDef::eQOP]
                                 - truthQOP);
        m_err_eLOC0_flt.push_back(
            sqrt(covariance(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_err_eLOC1_flt.push_back(
            sqrt(covariance(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_err_ePHI_flt.push_back(
            sqrt(covariance(Acts::ParDef::ePHI, Acts::ParDef::ePHI)));
        m_err_eTHETA_flt.push_back(
            sqrt(covariance(Acts::ParDef::eTHETA, Acts::ParDef::eTHETA)));
        m_err_eQOP_flt.push_back(
            sqrt(covariance(Acts::ParDef::eQOP, Acts::ParDef::eQOP)));
        m_pull_eLOC0_flt.push_back(
            (parameter.parameters()[Acts::ParDef::eLOC_0] - truthLOC0)
            / sqrt(covariance(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_pull_eLOC1_flt.push_back(
            (parameter.parameters()[Acts::ParDef::eLOC_1] - truthLOC1)
            / sqrt(covariance(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_pull_ePHI_flt.push_back(
            (parameter.parameters()[Acts::ParDef::ePHI] - truthPHI)
            / sqrt(covariance(Acts::ParDef::ePHI, Acts::ParDef::ePHI)));
        m_pull_eTHETA_flt.push_back(
            (parameter.parameters()[Acts::ParDef::eTHETA] - truthTHETA)
            / sqrt(covariance(Acts::ParDef::eTHETA, Acts::ParDef::eTHETA)));
        m_pull_eQOP_flt.push_back(
            (parameter.parameters()[Acts::ParDef::eQOP] - truthQOP)
            / sqrt(covariance(Acts::ParDef::eQOP, Acts::ParDef::eQOP)));
        m_x_flt.push_back(parameter.position().x());
        m_y_flt.push_back(parameter.position().y());
        m_z_flt.push_back(parameter.position().z());
        m_px_flt.push_back(parameter.momentum().x());
        m_py_flt.push_back(parameter.momentum().y());
        m_pz_flt.push_back(parameter.momentum().z());
        m_pT_flt.push_back(parameter.pT());
        m_eta_flt.push_back(eta(parameter.position()));
      } else {
        // push default values if no filtered parameter
        m_eLOC0_flt.push_back(-99.);
        m_eLOC1_flt.push_back(-99.);
        m_ePHI_flt.push_back(-99.);
        m_eTHETA_flt.push_back(-99.);
        m_eQOP_flt.push_back(-99.);
        m_res_eLOC0_flt.push_back(-99.);
        m_res_eLOC1_flt.push_back(-99.);
        m_res_ePHI_flt.push_back(-99.);
        m_res_eTHETA_flt.push_back(-99.);
        m_res_eQOP_flt.push_back(-99.);
        m_err_eLOC0_flt.push_back(-99);
        m_err_eLOC1_flt.push_back(-99);
        m_err_ePHI_flt.push_back(-99);
        m_err_eTHETA_flt.push_back(-99);
        m_err_eQOP_flt.push_back(-99);
        m_pull_eLOC0_flt.push_back(-99.);
        m_pull_eLOC1_flt.push_back(-99.);
        m_pull_ePHI_flt.push_back(-99.);
        m_pull_eTHETA_flt.push_back(-99.);
        m_pull_eQOP_flt.push_back(-99.);
        m_x_flt.push_back(-99.);
        m_y_flt.push_back(-99.);
        m_z_flt.push_back(-99.);
        m_py_flt.push_back(-99.);
        m_pz_flt.push_back(-99.);
        m_pT_flt.push_back(-99.);
        m_eta_flt.push_back(-99.);
      }

      // get the smoothed parameter
      bool smoothed = false;
      if (state.parameter.smoothed) {
        smoothed = true;
        m_nSmoothed++;
        auto parameter  = *state.parameter.smoothed;
        auto covariance = *state.parameter.smoothed->covariance();
        // smoothed parameter info
        m_eLOC0_smt.push_back(parameter.parameters()[Acts::ParDef::eLOC_0]);
        m_eLOC1_smt.push_back(parameter.parameters()[Acts::ParDef::eLOC_1]);
        m_ePHI_smt.push_back(parameter.parameters()[Acts::ParDef::ePHI]);
        m_eTHETA_smt.push_back(parameter.parameters()[Acts::ParDef::eTHETA]);
        m_eQOP_smt.push_back(parameter.parameters()[Acts::ParDef::eQOP]);
        m_res_eLOC0_smt.push_back(parameter.parameters()[Acts::ParDef::eLOC_0]
                                  - truthLOC0);
        m_res_eLOC1_smt.push_back(parameter.parameters()[Acts::ParDef::eLOC_1]
                                  - truthLOC1);
        m_res_ePHI_smt.push_back(parameter.parameters()[Acts::ParDef::ePHI]
                                 - truthPHI);
        m_res_eTHETA_smt.push_back(parameter.parameters()[Acts::ParDef::eTHETA]
                                   - truthTHETA);
        m_res_eQOP_smt.push_back(parameter.parameters()[Acts::ParDef::eQOP]
                                 - truthQOP);
        m_err_eLOC0_smt.push_back(
            sqrt(covariance(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_err_eLOC1_smt.push_back(
            sqrt(covariance(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_err_ePHI_smt.push_back(
            sqrt(covariance(Acts::ParDef::ePHI, Acts::ParDef::ePHI)));
        m_err_eTHETA_smt.push_back(
            sqrt(covariance(Acts::ParDef::eTHETA, Acts::ParDef::eTHETA)));
        m_err_eQOP_smt.push_back(
            sqrt(covariance(Acts::ParDef::eQOP, Acts::ParDef::eQOP)));
        m_pull_eLOC0_smt.push_back(
            (parameter.parameters()[Acts::ParDef::eLOC_0] - truthLOC0)
            / sqrt(covariance(Acts::ParDef::eLOC_0, Acts::ParDef::eLOC_0)));
        m_pull_eLOC1_smt.push_back(
            (parameter.parameters()[Acts::ParDef::eLOC_1] - truthLOC1)
            / sqrt(covariance(Acts::ParDef::eLOC_1, Acts::ParDef::eLOC_1)));
        m_pull_ePHI_smt.push_back(
            (parameter.parameters()[Acts::ParDef::ePHI] - truthPHI)
            / sqrt(covariance(Acts::ParDef::ePHI, Acts::ParDef::ePHI)));
        m_pull_eTHETA_smt.push_back(
            (parameter.parameters()[Acts::ParDef::eTHETA] - truthTHETA)
            / sqrt(covariance(Acts::ParDef::eTHETA, Acts::ParDef::eTHETA)));
        m_pull_eQOP_smt.push_back(
            (parameter.parameters()[Acts::ParDef::eQOP] - truthQOP)
            / sqrt(covariance(Acts::ParDef::eQOP, Acts::ParDef::eQOP)));
        m_x_smt.push_back(parameter.position().x());
        m_y_smt.push_back(parameter.position().y());
        m_z_smt.push_back(parameter.position().z());
        m_px_smt.push_back(parameter.momentum().x());
        m_py_smt.push_back(parameter.momentum().y());
        m_pz_smt.push_back(parameter.momentum().z());
        m_pT_smt.push_back(parameter.pT());
        m_eta_smt.push_back(eta(parameter.position()));
      } else {
        // push default values if no smoothed parameter
        m_eLOC0_smt.push_back(-99.);
        m_eLOC1_smt.push_back(-99.);
        m_ePHI_smt.push_back(-99.);
        m_eTHETA_smt.push_back(-99.);
        m_eQOP_smt.push_back(-99.);
        m_res_eLOC0_smt.push_back(-99.);
        m_res_eLOC1_smt.push_back(-99.);
        m_res_ePHI_smt.push_back(-99.);
        m_res_eTHETA_smt.push_back(-99.);
        m_res_eQOP_smt.push_back(-99.);
        m_err_eLOC0_smt.push_back(-99);
        m_err_eLOC1_smt.push_back(-99);
        m_err_ePHI_smt.push_back(-99);
        m_err_eTHETA_smt.push_back(-99);
        m_err_eQOP_smt.push_back(-99);
        m_pull_eLOC0_smt.push_back(-99.);
        m_pull_eLOC1_smt.push_back(-99.);
        m_pull_ePHI_smt.push_back(-99.);
        m_pull_eTHETA_smt.push_back(-99.);
        m_pull_eQOP_smt.push_back(-99.);
        m_x_smt.push_back(-99.);
        m_y_smt.push_back(-99.);
        m_z_smt.push_back(-99.);
        m_px_smt.push_back(-99.);
        m_py_smt.push_back(-99.);
        m_pz_smt.push_back(-99.);
        m_pT_smt.push_back(-99.);
        m_eta_smt.push_back(-99.);
      }

      m_prt.push_back(predicted);
      m_flt.push_back(filtered);
      m_smt.push_back(smoothed);

    }  // all states

    // fill the variables for one track to tree
    m_outputTree->Fill();

    // now reset
    m_t_x.clear();
    m_t_y.clear();
    m_t_z.clear();
    m_t_r.clear();
    m_t_dx.clear();
    m_t_dy.clear();
    m_t_dz.clear();
    m_t_eLOC0.clear();
    m_t_eLOC1.clear();
    m_t_ePHI.clear();
    m_t_eTHETA.clear();
    m_t_eQOP.clear();

    m_volumeID.clear();
    m_layerID.clear();
    m_moduleID.clear();
    m_lx_hit.clear();
    m_ly_hit.clear();
    m_x_hit.clear();
    m_y_hit.clear();
    m_z_hit.clear();
    m_res_x_hit.clear();
    m_res_y_hit.clear();
    m_err_x_hit.clear();
    m_err_y_hit.clear();
    m_pull_x_hit.clear();
    m_pull_y_hit.clear();

    m_prt.clear();
    m_eLOC0_prt.clear();
    m_eLOC1_prt.clear();
    m_ePHI_prt.clear();
    m_eTHETA_prt.clear();
    m_eQOP_prt.clear();
    m_res_eLOC0_prt.clear();
    m_res_eLOC1_prt.clear();
    m_res_ePHI_prt.clear();
    m_res_eTHETA_prt.clear();
    m_res_eQOP_prt.clear();
    m_err_eLOC0_prt.clear();
    m_err_eLOC1_prt.clear();
    m_err_ePHI_prt.clear();
    m_err_eTHETA_prt.clear();
    m_err_eQOP_prt.clear();
    m_pull_eLOC0_prt.clear();
    m_pull_eLOC1_prt.clear();
    m_pull_ePHI_prt.clear();
    m_pull_eTHETA_prt.clear();
    m_pull_eQOP_prt.clear();
    m_x_prt.clear();
    m_y_prt.clear();
    m_z_prt.clear();
    m_px_prt.clear();
    m_py_prt.clear();
    m_pz_prt.clear();
    m_eta_prt.clear();
    m_pT_prt.clear();

    m_flt.clear();
    m_eLOC0_flt.clear();
    m_eLOC1_flt.clear();
    m_ePHI_flt.clear();
    m_eTHETA_flt.clear();
    m_eQOP_flt.clear();
    m_res_eLOC0_flt.clear();
    m_res_eLOC1_flt.clear();
    m_res_ePHI_flt.clear();
    m_res_eTHETA_flt.clear();
    m_res_eQOP_flt.clear();
    m_err_eLOC0_flt.clear();
    m_err_eLOC1_flt.clear();
    m_err_ePHI_flt.clear();
    m_err_eTHETA_flt.clear();
    m_err_eQOP_flt.clear();
    m_pull_eLOC0_flt.clear();
    m_pull_eLOC1_flt.clear();
    m_pull_ePHI_flt.clear();
    m_pull_eTHETA_flt.clear();
    m_pull_eQOP_flt.clear();
    m_x_flt.clear();
    m_y_flt.clear();
    m_z_flt.clear();
    m_px_flt.clear();
    m_py_flt.clear();
    m_pz_flt.clear();
    m_eta_flt.clear();
    m_pT_flt.clear();

    m_smt.clear();
    m_eLOC0_smt.clear();
    m_eLOC1_smt.clear();
    m_ePHI_smt.clear();
    m_eTHETA_smt.clear();
    m_eQOP_smt.clear();
    m_res_eLOC0_smt.clear();
    m_res_eLOC1_smt.clear();
    m_res_ePHI_smt.clear();
    m_res_eTHETA_smt.clear();
    m_res_eQOP_smt.clear();
    m_err_eLOC0_smt.clear();
    m_err_eLOC1_smt.clear();
    m_err_ePHI_smt.clear();
    m_err_eTHETA_smt.clear();
    m_err_eQOP_smt.clear();
    m_pull_eLOC0_smt.clear();
    m_pull_eLOC1_smt.clear();
    m_pull_ePHI_smt.clear();
    m_pull_eTHETA_smt.clear();
    m_pull_eQOP_smt.clear();
    m_x_smt.clear();
    m_y_smt.clear();
    m_z_smt.clear();
    m_px_smt.clear();
    m_py_smt.clear();
    m_pz_smt.clear();
    m_eta_smt.clear();
    m_pT_smt.clear();

    iTrack++;
  }  // all tracks

  return ProcessCode::SUCCESS;
}
