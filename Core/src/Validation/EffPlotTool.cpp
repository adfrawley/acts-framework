// This file is part of the Acts project.
//
// Copyright (C) 2017 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Validation/EffPlotTool.hpp"

using Acts::VectorHelpers::eta;
using Acts::VectorHelpers::theta;
using Acts::VectorHelpers::phi;
using Acts::VectorHelpers::perp;

FW::EffPlotTool::EffPlotTool(const FW::EffPlotTool::Config& cfg,
                             Acts::Logging::Level           level)
  : m_cfg(cfg), m_logger(Acts::getDefaultLogger("EffPlotTool", level))
{
}

void
FW::EffPlotTool::book(EffPlotTool::EffPlotCache& effPlotCache) const
{
  PlotHelpers::Binning bPhi = m_cfg.varBinning.at("Phi");
  PlotHelpers::Binning bEta = m_cfg.varBinning.at("Eta");
  PlotHelpers::Binning bPt  = m_cfg.varBinning.at("Pt");
  ACTS_DEBUG("Initialize the histograms for efficiency plots");
  // efficiency vs pT
  effPlotCache.trackeff_vs_pT
      = PlotHelpers::bookEff("trackeff_vs_pT",
                             "Fraction of smoothed track;pT [GeV/c];Efficiency",
                             bPt);
  // efficiency vs eta
  effPlotCache.trackeff_vs_eta = PlotHelpers::bookEff(
      "trackeff_vs_eta", "Fraction of smoothed track;#eta;Efficiency", bEta);
  // efficiency vs phi
  effPlotCache.trackeff_vs_phi = PlotHelpers::bookEff(
      "trackeff_vs_phi", "Fraction of smoothed track;#phi;Efficiency", bPhi);
}

void
FW::EffPlotTool::clear(EffPlotCache& effPlotCache) const
{
  delete effPlotCache.trackeff_vs_pT;
  delete effPlotCache.trackeff_vs_eta;
  delete effPlotCache.trackeff_vs_phi;
}

void
FW::EffPlotTool::write(const EffPlotTool::EffPlotCache& effPlotCache) const
{
  ACTS_DEBUG("Write the plots to output file.");
  effPlotCache.trackeff_vs_pT->Write();
  effPlotCache.trackeff_vs_eta->Write();
  effPlotCache.trackeff_vs_phi->Write();
}

void
FW::EffPlotTool::fill(EffPlotTool::EffPlotCache& effPlotCache,
                      const TrackStateVector&    trackStates,
                      const Data::SimParticle&   truthParticle) const
{
  int nSmoothed = 0;
  for (auto& state : trackStates) {
    if (state.parameter.smoothed) nSmoothed++;
  }
  ACTS_DEBUG("There are " << trackStates.size() << " states in total and "
                          << nSmoothed
                          << " of them are processed.");

  Acts::Vector3D truthMom = truthParticle.momentum();

  double t_phi = phi(truthMom);
  double t_eta = eta(truthMom);
  double t_pT  = perp(truthMom);

  bool status = nSmoothed > 0 ? true : false;

  PlotHelpers::fillEff(effPlotCache.trackeff_vs_pT, t_pT, status);
  PlotHelpers::fillEff(effPlotCache.trackeff_vs_eta, t_eta, status);
  PlotHelpers::fillEff(effPlotCache.trackeff_vs_phi, t_phi, status);
}
