// This file is part of the Acts project.
//
// Copyright (C) 2017-2019 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <mutex>
#include "ACTFW/Framework/IService.hpp"
#include "ACTFW/Framework/ProcessCode.hpp"
#include "ACTFW/Readers/IReaderT.hpp"
#include "Acts/Material/IMaterialDecorator.hpp"
#include "Acts/Material/ISurfaceMaterial.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/GeometryID.hpp"
#include "Acts/Utilities/Logger.hpp"

class TFile;

namespace Acts {

using SurfaceMaterialMap
    = std::map<GeometryID, std::shared_ptr<const ISurfaceMaterial>>;
}

namespace FW {

namespace Root {

  /// @class RootIndexedMaterialReader
  ///
  /// @brief Read the collection of SurfaceMaterial from a file in order to
  /// load it onto the TrackingGeometry
  class RootIndexedMaterialReader
      : public FW::IReaderT<Acts::SurfaceMaterialMap>
  {
  public:
    /// @class Config
    /// Configuration of the Reader
    class Config
    {
    public:
      /// The name of the output tree
      std::string folderNameBase = "Material";
      /// The volume identification string
      std::string voltag = "_vol";
      /// The layer identification string
      std::string laytag = "_lay";
      /// The approach identification string
      std::string apptag = "_app";
      /// The sensitive identification string
      std::string sentag = "_sen";
      /// The bin number tag
      std::string ntag = "n";
      /// The value tag -> binning values: binZ, binR, binPhi, etc.
      std::string vtag = "v";
      /// The option tag -> binning options: open, closed
      std::string otag = "o";
      /// The range min tag: min value
      std::string mintag = "min";
      /// The range max tag: max value
      std::string maxtag = "max";
      /// The thickness tag
      std::string ttag = "t";
      /// The x0 tag
      std::string x0tag = "x0";
      /// The l0 tag
      std::string l0tag = "l0";
      /// The A tag
      std::string atag = "A";
      /// The Z tag
      std::string ztag = "Z";
      /// The rho tag
      std::string rhotag = "rho";
      /// The name of the output file
      std::string fileName = "material-maps.root";
      /// The default logger
      std::shared_ptr<const Acts::Logger> logger;
      // The name of the writer
      std::string name = "";

      /// Constructor
      ///
      /// @param lname Name of the writer tool
      /// @param lvl The output logging level
      Config(const std::string&   lname = "MaterialReader",
             Acts::Logging::Level lvl   = Acts::Logging::INFO)
        : logger(Acts::getDefaultLogger(lname, lvl)), name(lname)
      {
      }
    };

    /// Constructor
    ///
    /// @param cfg configuration struct for the reader
    RootIndexedMaterialReader(const Config& cfg);

    /// Virtual destructor
    ~RootIndexedMaterialReader() override;

    /// Framework name() method
    std::string
    name() const final override;

    /// Read method
    ///
    /// @param surfaceMaterialMap The indexed material map collection
    /// @param skip is the number of skip reads (0 for this reader)
    /// @param is the AlgorithmContext pointer in case the reader would need
    /// information about the event context (not true in this case)
    FW::ProcessCode
    read(Acts::SurfaceMaterialMap&   sMaterialMap,
         size_t                      skip = 0,
         const FW::AlgorithmContext* ctx  = nullptr) final override;

  private:
    /// The config class
    Config m_cfg;

    /// The input file
    TFile* m_inputFile;

    /// mutex used to protect multi-threaded reads
    std::mutex m_read_mutex;

    /// Private access to the logging instance
    const Acts::Logger&
    logger() const
    {
      return *m_cfg.logger;
    }
  };

  inline std::string
  RootIndexedMaterialReader::name() const
  {
    return m_cfg.name;
  }

  ///@brief Material decorator from ROOT
  class RootMaterialDecorator : public Acts::IMaterialDecorator
  {
  public:
    RootMaterialDecorator(RootIndexedMaterialReader::Config rConfig)
      : m_readerConfig(rConfig)
    {
      // Create the reader with the config
      RootIndexedMaterialReader reader(m_readerConfig);
      // Read the map & return it
      reader.read(m_surfaceMaterialMap);
    }

    /// Decorate a surface
    ///
    /// @param surface the non-cost surface that is decorated
    void
    decorate(Acts::Surface& surface) const final
    {
      // Try to find the surface in the map
      auto sMaterial = m_surfaceMaterialMap.find(surface.geoID());
      if (sMaterial != m_surfaceMaterialMap.end()) {
        surface.assignSurfaceMaterial(sMaterial->second);
      }
    }

    /// Decorate a TrackingVolume
    ///
    /// @param volume the non-cost volume that is decorated
    void
    decorate(Acts::TrackingVolume& /*volume*/) const final
    {
    }

  private:
    RootIndexedMaterialReader::Config m_readerConfig;
    Acts::SurfaceMaterialMap          m_surfaceMaterialMap;
  };

}  // namespace Root
}  // namespace FW
