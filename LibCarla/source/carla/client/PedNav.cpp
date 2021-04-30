// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/PedNav.h"
#include "carla/geom/Location.h"

#include <assert.h>

namespace carla {
namespace client {

  /// Load a local file for navigation of pedestrians
  bool PedNav::LoadNavigationFile(std::string filename) {
    _filename = filename;
    return _nav.Load(filename);
  }

  /// Returns if a location is reachable by a pedestrian
  bool PedNav::IsLocationReachableByPedestrian(carla::geom::Location from, carla::geom::Location to, float max_distance) {
    return _nav.IsLocationReachable(from, to, max_distance);
  }

  /// Get a random location from the pedestrians navigation mesh
  boost::optional<geom::Location> PedNav::GetRandomLocationFromNavigation() const {
      geom::Location random_location(0, 0, 0);
      if (_nav.GetRandomLocation(random_location))
        return boost::optional<geom::Location>(random_location);
      else
        return {};
  }

} // namespace client
} // namespace carla
