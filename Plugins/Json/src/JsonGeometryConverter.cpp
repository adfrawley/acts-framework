// This file is part of the Acts project.
//
// Copyright (C) 2017-2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Plugins/Json/JsonGeometryConverter.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include <Acts/Geometry/ApproachDescriptor.hpp>
#include <Acts/Geometry/GeometryID.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Material/BinnedSurfaceMaterial.hpp>
#include <Acts/Material/HomogeneousSurfaceMaterial.hpp>
#include <Acts/Material/ProtoSurfaceMaterial.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Utilities/BinUtility.hpp>
#include <Acts/Utilities/BinningType.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/iter_find.hpp>

FW::Json::JsonGeometryConverter::JsonGeometryConverter(
    const FW::Json::JsonGeometryConverter::Config& cfg)
  : m_cfg(std::move(cfg))
{
  // Validate the configuration
  if (!m_cfg.logger) { throw std::invalid_argument("Missing logger"); }
}

std::pair<Acts::SurfaceMaterialMap, Acts::VolumeMaterialMap>
FW::Json::JsonGeometryConverter::jsonToMaterialMaps(const json& materialmaps)
{

  auto& j = materialmaps;
  // The return maps
  std::pair<Acts::SurfaceMaterialMap, Acts::VolumeMaterialMap> maps;
  ACTS_VERBOSE("j2a: Reading material maps from json file.");
  ACTS_VERBOSE("j2a: Found entries for " << j.count(m_cfg.volkey)
                                         << " volume(s).");
  // Structured binding
  for (auto& [key, value] : j.items()) {
    // Check if this the volume key
    if (key == m_cfg.volkey) {
      // Get the volume json
      auto volj = value;
      for (auto& [vkey, vvalue] : volj.items()) {
        // Create the volume id
        int  vid      = std::stoi(vkey);
        auto volumeID = Acts::GeometryID().setVolume(vid);
        ACTS_VERBOSE("j2a: -> Found Volume " << vid);
        // Loop through the information in the volume
        for (auto& [vckey, vcvalue] : vvalue.items()) {
          if (vckey == m_cfg.boukey and m_cfg.processBoundaries
              and not vcvalue.empty()) {
            ACTS_VERBOSE("j2a: --> BoundarySurface(s) to be parsed");
            for (auto& [bkey, bvalue] : vcvalue.items()) {
              // Create the boundary id
              int  bid        = std::stoi(bkey);
              auto boundaryID = Acts::GeometryID(volumeID).setBoundary(bid);
              ACTS_VERBOSE("j2a: ---> Found boundary surface " << bid);
              auto boumat = jsonToSurfaceMaterial(bvalue);
              maps.first[boundaryID]
                  = std::shared_ptr<const Acts::ISurfaceMaterial>(boumat);
            }
          } else if (vckey == m_cfg.laykey) {
            ACTS_VERBOSE("j2a: --> Layer(s) to be parsed");
            // Loop over layers and repeat
            auto layj = vcvalue;
            for (auto& [lkey, lvalue] : layj.items()) {
              // Create the layer id
              int  lid     = std::stoi(lkey);
              auto layerID = Acts::GeometryID(volumeID).setLayer(lid);
              ACTS_VERBOSE("j2a: ---> Found Layer " << lid);
              // Finally loop over layer components
              for (auto& [lckey, lcvalue] : lvalue.items()) {
                if (lckey == m_cfg.repkey and m_cfg.processRepresenting
                    and not lcvalue.empty()) {
                  ACTS_VERBOSE("j2a: ----> Found representing surface");
                  auto repmat = jsonToSurfaceMaterial(lcvalue);
                  maps.first[layerID]
                      = std::shared_ptr<const Acts::ISurfaceMaterial>(repmat);
                } else if (lckey == m_cfg.appkey and m_cfg.processApproaches
                           and not lcvalue.empty()) {
                  ACTS_VERBOSE("j2a: ----> Found approach surface(s)");
                  // Loop over approach surfaces
                  for (auto& [askey, asvalue] : lcvalue.items()) {
                    // Create the layer id, todo set to max value
                    int  aid = (askey == "*") ? 0 : std::stoi(askey);
                    auto approachID
                        = Acts::GeometryID(layerID).setApproach(aid);
                    ACTS_VERBOSE("j2a: -----> Approach surface " << askey);
                    auto appmat = jsonToSurfaceMaterial(asvalue);
                    maps.first[approachID]
                        = std::shared_ptr<const Acts::ISurfaceMaterial>(appmat);
                  }
                } else if (lckey == m_cfg.senkey and m_cfg.processSensitives
                           and not lcvalue.empty()) {
                  ACTS_VERBOSE("j2a: ----> Found sensitive surface(s)");
                  // Loop over sensitive surfaces
                  for (auto& [sskey, ssvalue] : lcvalue.items()) {
                    // Create the layer id, todo set to max value
                    int  sid = (sskey == "*") ? 0 : std::stoi(sskey);
                    auto sensitiveID
                        = Acts::GeometryID(layerID).setSensitive(sid);
                    ACTS_VERBOSE("j2a: -----> Sensitive surface " << sskey);
                    auto senmat = jsonToSurfaceMaterial(ssvalue);
                    maps.first[sensitiveID]
                        = std::shared_ptr<const Acts::ISurfaceMaterial>(senmat);
                  }
                }
              }
            }

          } else if (vckey == m_cfg.matkey and not vcvalue.empty()) {
            ACTS_VERBOSE("--> VolumeMaterial to be parsed");
          }
        }
      }
    } else if (key == m_cfg.geoversion) {
      ACTS_VERBOSE("Detector version: " << m_cfg.geoversion);
    }
  }

  // Return the filled maps
  return maps;
}

/// Convert method
///
json
FW::Json::JsonGeometryConverter::materialMapsToJson(
    const Acts::DetectorMaterialMaps& maps)
{
  DetectorRep detRep;
  // Collect all GeometryIDs per VolumeID for the formatted output
  for (auto& [key, value] : maps.first) {
    auto vid    = key.volume();
    auto volRep = detRep.volumes.find(vid);
    if (volRep == detRep.volumes.end()) {
      detRep.volumes.insert({vid, VolumeRep()});
      volRep                  = detRep.volumes.find(vid);
      volRep->second.volumeID = key;
    }
    auto lid = key.layer();
    if (lid != 0) {
      // we are on a layer, get the layer rep
      auto layRep = volRep->second.layers.find(lid);
      if (layRep == volRep->second.layers.end()) {
        volRep->second.layers.insert({lid, LayerRep()});
        layRep                 = volRep->second.layers.find(lid);
        layRep->second.layerID = key;
      }
      // now insert appropriately
      auto sid = key.sensitive();
      auto aid = key.approach();
      if (sid != 0) {
        layRep->second.sensitives.insert({sid, value.get()});
      } else if (aid != 0) {
        layRep->second.approaches.insert({aid, value.get()});
      } else {
        layRep->second.representing = value.get();
      }

    } else {
      // not on a layer can only be a boundary surface
      auto bid = key.boundary();
      volRep->second.boundaries.insert({bid, value.get()});
    }
  }
  for (auto& [key, value] : maps.second) {
    // find the volume representation
    auto vid    = key.volume();
    auto volRep = detRep.volumes.find(vid);
    if (volRep == detRep.volumes.end()) {
      detRep.volumes.insert({vid, VolumeRep()});
      volRep                  = detRep.volumes.find(vid);
      volRep->second.volumeID = key;
    }
    volRep->second.material = value.get();
  }
  // convert the detector representation to json format
  return detectorRepToJson(detRep);
}

/// Create Json from a detector represenation
json
FW::Json::JsonGeometryConverter::detectorRepToJson(const DetectorRep& detRep)
{
  json detectorj;
  ACTS_VERBOSE("a2j: Writing json from detector representation");
  ACTS_VERBOSE("a2j: Found entries for " << detRep.volumes.size()
                                         << " volume(s).");

  json volumesj;
  for (auto& [key, value] : detRep.volumes) {
    json volj;
    ACTS_VERBOSE("a2j: -> Writing Volume: " << key);
    volj[m_cfg.namekey]  = value.volumeName;
    volj[m_cfg.geoidkey] = std::to_string(value.volumeID.value());
    // Write the layers
    if (not value.layers.empty()) {
      ACTS_VERBOSE("a2j: ---> Found " << value.layers.size() << " layer(s) ");
      json layersj;
      for (auto& [lkey, lvalue] : value.layers) {
        ACTS_VERBOSE("a2j: ----> Convert layer " << lkey);
        json layj;
        layj[m_cfg.geoidkey] = std::to_string(lvalue.layerID.value());
        // First check for approaches
        if (not lvalue.approaches.empty() and m_cfg.processApproaches) {
          ACTS_VERBOSE("a2j: -----> Found " << lvalue.approaches.size()
                                            << " approach surface(s)");
          json approachesj;
          for (auto& [akey, avalue] : lvalue.approaches) {
            ACTS_VERBOSE("a2j: ------> Convert approach surface " << akey);
            approachesj[std::to_string(akey)] = surfaceMaterialToJson(*avalue);
          }
          // Add to the layer json
          layj[m_cfg.appkey] = approachesj;
        }
        // Then check for sensitive
        if (not lvalue.sensitives.empty() and m_cfg.processSensitives) {
          ACTS_VERBOSE("a2j: -----> Found " << lvalue.sensitives.size()
                                            << " sensitive surface(s)");
          json sensitivesj;
          for (auto& [skey, svalue] : lvalue.sensitives) {
            ACTS_VERBOSE("a2j: ------> Convert sensitive surface " << skey);
            sensitivesj[std::to_string(skey)] = surfaceMaterialToJson(*svalue);
          }
          // Add to the layer json
          layj[m_cfg.senkey] = sensitivesj;
        }
        // Finally check for representing
        if (lvalue.representing != nullptr and m_cfg.processRepresenting) {
          ACTS_VERBOSE("a2j: ------> Convert representing surface ");
          layj[m_cfg.repkey] = surfaceMaterialToJson(*lvalue.representing);
        }
        layersj[std::to_string(lkey)] = layj;
      }
      volj[m_cfg.laykey] = layersj;
    }
    // Write the boundary surfaces
    if (not value.boundaries.empty()) {
      ACTS_VERBOSE("a2j: ---> Found " << value.boundaries.size()
                                      << " boundary/ies ");
      json boundariesj;
      for (auto& [bkey, bvalue] : value.boundaries) {
        ACTS_VERBOSE("a2j: ----> Convert boundary " << bkey);
        boundariesj[std::to_string(bkey)] = surfaceMaterialToJson(*bvalue);
      }
      volj[m_cfg.boukey] = boundariesj;
    }

    volumesj[std::to_string(key)] = volj;
  }
  // Assign the volume json to the detector json
  detectorj[m_cfg.volkey] = volumesj;

  return detectorj;
}

/// Create the Surface Material
const Acts::ISurfaceMaterial*
FW::Json::JsonGeometryConverter::jsonToSurfaceMaterial(const json& material)
{

  Acts::ISurfaceMaterial* sMaterial = nullptr;
  // The bin utility for deescribing the data
  Acts::BinUtility bUtility;
  // Convert the material
  Acts::MaterialPropertiesMatrix mpMatrix;
  // Structured binding
  for (auto& [key, value] : material.items()) {
    // Check json keys
    if (key == m_cfg.bin0key and not value.empty()) {
      bUtility += jsonToBinUtility(value);
    } else if (key == m_cfg.bin1key and not value.empty()) {
      bUtility += jsonToBinUtility(value);
    }
    if (key == m_cfg.datakey and not value.empty()) {
      mpMatrix = jsonToMaterialMatrix(value);
    }
  }

  // We have protoMaterial
  if (mpMatrix.size() == 0) {
    sMaterial = new Acts::ProtoSurfaceMaterial(bUtility);
  } else if (bUtility.bins() == 1) {
    sMaterial = new Acts::HomogeneousSurfaceMaterial(mpMatrix[0][0]);
  } else {
    sMaterial = new Acts::BinnedSurfaceMaterial(bUtility, mpMatrix);
  }
  // return what you have
  return sMaterial;
}

json
FW::Json::JsonGeometryConverter::trackingGeometryToJson(
    const Acts::TrackingGeometry& tGeometry)
{
  DetectorRep detRep;
  convertToRep(detRep, *tGeometry.highestTrackingVolume());
  return detectorRepToJson(detRep);
}

void
FW::Json::JsonGeometryConverter::convertToRep(
    DetectorRep&                detRep,
    const Acts::TrackingVolume& tVolume)
{
  // The writer reader volume representation
  VolumeRep volRep;
  volRep.volumeName = tVolume.volumeName();
  // there are confined volumes
  if (tVolume.confinedVolumes()) {
    // get through the volumes
    auto& volumes = tVolume.confinedVolumes()->arrayObjects();
    // loop over the volumes
    for (auto& vol : volumes) {
      // recursive call
      convertToRep(detRep, *vol);
    }
  }
  // Get the volume Id
  auto vid = tVolume.geoID().volume();

  // Write the material if there's one
  if (tVolume.volumeMaterial() != nullptr) {
    volRep.material = tVolume.volumeMaterial();
  }
  // there are confied layers
  if (tVolume.confinedLayers()) {
    // get the layers
    auto& layers = tVolume.confinedLayers()->arrayObjects();
    // loop of the volumes
    for (auto& lay : layers) {
      auto layRep = convertToRep(*lay);
      if (layRep) {
        // it's a valid representation so let's go with it
        auto lid = lay->geoID().layer();
        volRep.layers.insert({lid, std::move(layRep)});
      }
    }
  }
  // Let's finally check the boundaries
  for (auto& bsurf : tVolume.boundarySurfaces()) {
    // the surface representation
    auto& bssfRep = bsurf->surfaceRepresentation();
    if (bssfRep.surfaceMaterial()) {
      auto bid = bssfRep.geoID().boundary();
      // Ignore if the volumeID is not correct (i.e. shared boundary)
      // if (boundaryID.value(Acts::GeometryID::volume_mask) == vid){
      volRep.boundaries[bid] = bssfRep.surfaceMaterial();
      //}
    }
  }
  // Write if it's good
  if (volRep) {
    volRep.volumeName = tVolume.volumeName();
    volRep.volumeID   = tVolume.geoID();
    detRep.volumes.insert({vid, std::move(volRep)});
  }
  return;
}

FW::Json::LayerRep
FW::Json::JsonGeometryConverter::convertToRep(const Acts::Layer& layer)
{
  LayerRep layRep;
  // fill layer ID information
  layRep.layerID = layer.geoID();
  if (m_cfg.processSensitives and layer.surfaceArray() != nullptr) {
    for (auto& ssf : layer.surfaceArray()->surfaces()) {
      if (ssf != nullptr && ssf->surfaceMaterial()) {
        auto sid = ssf->geoID().sensitive();
        layRep.sensitives.insert({sid, ssf->surfaceMaterial()});
      }
    }
  }
  // the representing
  if (layer.surfaceRepresentation().surfaceMaterial() != nullptr) {
    layRep.representing = layer.surfaceRepresentation().surfaceMaterial();
  }
  // the approach
  if (layer.approachDescriptor() != nullptr) {
    for (auto& asf : layer.approachDescriptor()->containedSurfaces()) {
      // get the surface and check for material
      if (asf->surfaceMaterial() != nullptr) {
        auto aid = asf->geoID().approach();
        layRep.approaches.insert({aid, asf->surfaceMaterial()});
      }
    }
  }
  // return the layer representation
  return layRep;
}

json
FW::Json::JsonGeometryConverter::surfaceMaterialToJson(
    const Acts::ISurfaceMaterial& sMaterial)
{

  json smj;

  // lemma 0 : accept the surface
  auto convertMaterialProperties
      = [](const Acts::MaterialProperties& mp) -> std::vector<float> {
    // convert when ready
    if (mp) {
      /// Return the thickness in mm
      return {
          mp.averageX0(),
          mp.averageL0(),
          mp.averageZ(),
          mp.averageA(),
          mp.averageRho(),
          mp.thickness(),
      };
    }
    return {};
  };

  // A bin utility needs to be written
  const Acts::BinUtility* bUtility = nullptr;
  // Check if we have a proto material
  auto psMaterial = dynamic_cast<const Acts::ProtoSurfaceMaterial*>(&sMaterial);
  if (psMaterial != nullptr) {
    // Type is proto material
    smj[m_cfg.typekey] = "proto";
    bUtility           = &(psMaterial->binUtility());
  } else {
    // Now check if we have a homogeneous material
    auto hsMaterial
        = dynamic_cast<const Acts::HomogeneousSurfaceMaterial*>(&sMaterial);
    if (hsMaterial != nullptr) {
      // type is homogeneous
      smj[m_cfg.typekey] = "homogeneous";
      if (m_cfg.writeData) {
        // write out the data, it's a [[[X0,L0,Z,A,rho,thickness]]]
        auto& mp = hsMaterial->materialProperties(0, 0);
        std::vector<std::vector<std::vector<float>>> mmat
            = {{convertMaterialProperties(mp)}};
        smj[m_cfg.datakey] = mmat;
      }
    } else {
      // Only option remaining: BinnedSurface material
      auto bsMaterial
          = dynamic_cast<const Acts::BinnedSurfaceMaterial*>(&sMaterial);
      if (bsMaterial != nullptr) {
        // type is binned
        smj[m_cfg.typekey] = "binned";
        bUtility           = &(bsMaterial->binUtility());
        // convert the data
        // get the material matrix
        if (m_cfg.writeData) {
          auto& mpMatrix = bsMaterial->fullMaterial();
          std::vector<std::vector<std::vector<float>>> mmat;
          mmat.reserve(mpMatrix.size());
          for (auto& mpVector : mpMatrix) {
            std::vector<std::vector<float>> mvec;
            mvec.reserve(mpVector.size());
            for (auto& mp : mpVector) {
              mvec.push_back(convertMaterialProperties(mp));
            }
            mmat.push_back(std::move(mvec));
          }
          smj[m_cfg.datakey] = mmat;
        }
      }
    }
  }
  // add the bin utility
  if (bUtility != nullptr) {
    std::vector<std::string> binkeys = {m_cfg.bin0key, m_cfg.bin1key};
    // loop over dimensions and write
    auto& binningData = bUtility->binningData();
    // loop over the dimensions
    for (size_t ibin = 0; ibin < binningData.size(); ++ibin) {
      json binj;
      auto cbData = binningData[ibin];
      binj.push_back(Acts::binningValueNames[cbData.binvalue]);
      if (cbData.option == Acts::closed) {
        binj.push_back("closed");
      } else {
        binj.push_back("open");
      }
      binj.push_back(cbData.bins());
      // If it's not a proto map, write min / max
      if (smj[m_cfg.typekey] != "proto") {
        std::pair<double, double> minMax = {cbData.min, cbData.max};
        binj.push_back(minMax);
      }
      smj[binkeys[ibin]] = binj;
    }
  }

  return smj;
}

/// Create the Material Matrix
Acts::MaterialPropertiesMatrix
FW::Json::JsonGeometryConverter::jsonToMaterialMatrix(const json& data)
{
  Acts::MaterialPropertiesMatrix mpMatrix;
  /// This is assumed to be an array or array of array[6]
  for (auto& outer : data) {
    Acts::MaterialPropertiesVector mpVector;
    for (auto& inner : outer) {
      if (inner.size() > 5) {
        mpVector.push_back(Acts::MaterialProperties(
            inner[0], inner[1], inner[2], inner[3], inner[4], inner[5]));
      } else {
        mpVector.push_back(Acts::MaterialProperties());
      }
    }
    mpMatrix.push_back(std::move(mpVector));
  }
  return mpMatrix;
}

/// Create the BinUtility for this
Acts::BinUtility
FW::Json::JsonGeometryConverter::jsonToBinUtility(const json& bin)
{
  // finding the iterator position to determine the binning value
  auto bit = std::find(
      Acts::binningValueNames.begin(), Acts::binningValueNames.end(), bin[0]);
  size_t             indx = std::distance(Acts::binningValueNames.begin(), bit);
  Acts::BinningValue bval = Acts::BinningValue(indx);
  Acts::BinningOption bopt = bin[1] == "open" ? Acts::open : Acts::closed;
  unsigned int        bins = bin[2];
  double              min, max = 0;
  if (bin[3].size() == 2) {
    min = bin[3][0];
    max = bin[3][1];
  }
  return Acts::BinUtility(bins, min, max, bopt, bval);
}
