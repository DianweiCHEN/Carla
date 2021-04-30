// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/nav/Navigation.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {

  class PedNav {
    public:
    
    /// Load a local file for navigation of pedestrians
    bool LoadNavigationFile(std::string filename);
    /// Returns if a location is reachable by a pedestrian
    bool IsLocationReachableByPedestrian(carla::geom::Location from, carla::geom::Location to, float max_distance);
    /// Get a random location from the pedestrians navigation mesh
    boost::optional<geom::Location> GetRandomLocationFromNavigation() const;
  
  private:
    carla::nav::Navigation _nav;    
    std::string _filename;
  };

} // namespace client
} // namespace carla
