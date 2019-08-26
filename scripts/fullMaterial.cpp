// This file is part of the Acts project.
//
// Copyright (C) 2017 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/*
 * fullMaterial.cxx
 *
 *  Created on: 22 Aug 2016
 *      Author: jhrdinka
 */

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TROOT.h"
#include "TTree.h"

// This root script writes out the full material described in
// MaterialTracks. It sums up all the material accumulated along one track
// and averages the material over all tracks. It generates Histograms of the
// different MaterialProperties along phi, theta,z and eta.
// It is forseen to be used with an input file generated by the
// materialTrackWriter.

void
fullMaterial(std::string inFile,
             std::string treename,
             std::string outFile,
             int         binsZ,
             float       minZ,
             float       maxZ,
             int         binsPhi,
             float       minPhi,
             float       maxPhi,
             int         binsTheta,
             float       minTheta,
             float       maxTheta,
             int         binsEta,
             float       minEta,
             float       maxEta)
{
  std::cout << "Opening file: " << inFile << std::endl;
  TFile inputFile(inFile.c_str());
  std::cout << "Reading tree: " << treename << std::endl;
  TTreeReader reader(treename.c_str(), &inputFile);

  // get the MaterialTrack entities
  std::vector<Acts::MaterialTrack> mrecords;
  std::cout << "Accessing Branch 'MaterialTracks'" << std::endl;
  TTreeReaderValue<Acts::MaterialTrack> mtRecord(reader, "MaterialTracks");
  while (reader.Next()) { mrecords.push_back(*mtRecord); }
  inputFile.Close();

  std::cout << "Creating new output file: " << outFile
            << " and writing "
               "material maps"
            << std::endl;
  TFile outputFile(outFile.c_str(), "recreate");
  // thickness
  TProfile* t_phi = new TProfile("t_phi", "t_phi", binsPhi, minPhi, maxPhi);
  TProfile* t_eta = new TProfile("t_eta", "t_eta", binsEta, minEta, maxEta);
  TProfile* t_theta
      = new TProfile("t_theta", "t_theta", binsTheta, minTheta, maxTheta);

  // thickness in X0
  TProfile* tInX0_phi
      = new TProfile("tInX0_phi", "tInX0_phi", binsPhi, minPhi, maxPhi);
  TProfile* tInX0_eta
      = new TProfile("tInX0_eta", "tInX0_eta", binsEta, minEta, maxEta);
  TProfile* tInX0_theta = new TProfile(
      "tInX0_theta", "tInX0_theta", binsTheta, minTheta, maxTheta);
  // thickness in L0
  TProfile* tInL0_phi
      = new TProfile("tInL0_phi", "tInL0_phi", binsPhi, minPhi, maxPhi);
  TProfile* tInL0_eta
      = new TProfile("tInL0_eta", "tInL0_eta", binsEta, minEta, maxEta);
  TProfile* tInL0_theta = new TProfile(
      "tInL0_theta", "tInL0_theta", binsTheta, minTheta, maxTheta);
  // A
  TProfile* A_phi = new TProfile("A_phi", "A_phi", binsPhi, minPhi, maxPhi);
  TProfile* A_eta = new TProfile("A_eta", "A_eta", binsEta, minEta, maxEta);
  TProfile* A_theta
      = new TProfile("A_theta", "A_theta", binsTheta, minTheta, maxTheta);
  // Z
  TProfile* Z_phi = new TProfile("Z_phi", "Z_phi", binsPhi, minPhi, maxPhi);
  TProfile* Z_eta = new TProfile("Z_eta", "Z_eta", binsEta, minEta, maxEta);
  TProfile* Z_theta
      = new TProfile("Z_theta", "Z_theta", binsTheta, minTheta, maxTheta);
  // Rho
  TProfile* rho_phi
      = new TProfile("rho_phi", "rho_phi", binsPhi, minPhi, maxPhi);
  TProfile* rho_eta
      = new TProfile("rho_eta", "rho_eta", binsEta, minEta, maxEta);
  TProfile* rho_theta
      = new TProfile("rho_theta", "rho_theta", binsTheta, minTheta, maxTheta);
  // x0
  TProfile* x0_phi = new TProfile("x0_phi", "x0_phi", binsPhi, minPhi, maxPhi);
  TProfile* x0_eta = new TProfile("x0_eta", "x0_eta", binsEta, minEta, maxEta);
  TProfile* x0_theta
      = new TProfile("x0_theta", "x0_theta", binsTheta, minTheta, maxTheta);
  // l0
  TProfile* l0_phi = new TProfile("l0_phi", "l0_phi", binsPhi, minPhi, maxPhi);
  TProfile* l0_eta = new TProfile("l0_eta", "l0_eta", binsEta, minEta, maxEta);
  TProfile* l0_theta
      = new TProfile("l0_theta", "l0_theta", binsTheta, minTheta, maxTheta);

  for (auto& mrecord : mrecords) {
    std::vector<Acts::MaterialStep> steps = mrecord.materialSteps();
    float                           theta = mrecord.theta();
    float                           eta   = -log(tan(theta * 0.5));
    float                           phi   = VectorHelpers::phi(mrecord);

    float thickness = 0.;
    float rho       = 0.;
    float A         = 0.;
    float Z         = 0.;
    float tInX0     = 0.;
    float tInL0     = 0.;

    for (auto& step : steps) {
      //  std::cout << "t" << step.material().thickness() << std::endl;
      float t = step.material().thickness();
      float r = step.material().averageRho();
      thickness += step.material().thickness();
      rho += r * t;
      A += step.material().averageA() * r * t;
      Z += step.material().averageZ() * r * t;
      tInX0 += (t != 0. && step.material().x0() != 0.)
          ? t / step.material().x0()
          : 0.;

      tInL0 += (t != 0. && step.material().x0() != 0.)
          ? t / step.material().l0()
          : 0.;
    }
    if (rho != 0.) {
      A /= rho;
      Z /= rho;
    }
    if (thickness != 0.) rho /= thickness;

    t_phi->Fill(phi, thickness);
    t_theta->Fill(theta, thickness);
    t_eta->Fill(eta, thickness);

    if (tInX0 != 0.) {
      x0_phi->Fill(phi, thickness / tInX0);
      x0_theta->Fill(theta, thickness / tInX0);
      x0_eta->Fill(eta, thickness / tInX0);
    }

    if (tInL0 != 0.) {
      l0_phi->Fill(phi, thickness / tInL0);
      l0_theta->Fill(theta, thickness / tInL0);
      l0_eta->Fill(eta, thickness / tInL0);
    }

    tInX0_phi->Fill(phi, tInX0);
    tInX0_theta->Fill(theta, tInX0);
    tInX0_eta->Fill(eta, tInX0);

    tInL0_phi->Fill(phi, tInL0);
    tInL0_theta->Fill(theta, tInL0);
    tInL0_eta->Fill(eta, tInL0);

    A_phi->Fill(phi, A);
    A_theta->Fill(theta, A);
    A_eta->Fill(eta, A);

    Z_phi->Fill(phi, Z);
    Z_theta->Fill(theta, Z);
    Z_eta->Fill(eta, Z);

    rho_phi->Fill(phi, rho);
    rho_theta->Fill(theta, rho);
    rho_eta->Fill(eta, rho);
  }
  // thickness
  t_phi->Write();
  t_theta->Write();
  t_eta->Write();
  delete t_theta;
  delete t_eta;
  delete t_phi;
  // thickness in X0
  tInX0_phi->Write();
  tInX0_theta->Write();
  tInX0_eta->Write();
  delete tInX0_phi;
  delete tInX0_eta;
  delete tInX0_theta;
  // thickness in L0
  tInL0_phi->Write();
  tInL0_theta->Write();
  tInL0_eta->Write();
  delete tInL0_phi;
  delete tInL0_eta;
  delete tInL0_theta;
  // A
  A_phi->Write();
  A_eta->Write();
  A_theta->Write();
  delete A_phi;
  delete A_eta;
  delete A_theta;
  // Z
  Z_phi->Write();
  Z_eta->Write();
  Z_theta->Write();
  delete Z_phi;
  delete Z_eta;
  delete Z_theta;
  // rho
  rho_phi->Write();
  rho_eta->Write();
  rho_theta->Write();
  delete rho_phi;
  delete rho_eta;
  delete rho_theta;
  // x0
  x0_phi->Write();
  x0_eta->Write();
  x0_theta->Write();
  delete x0_phi;
  delete x0_eta;
  delete x0_theta;
  // l0
  l0_phi->Write();
  l0_eta->Write();
  l0_theta->Write();
  delete l0_phi;
  delete l0_eta;
  delete l0_theta;

  outputFile.Close();
}
