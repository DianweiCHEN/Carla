// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/Waypoint.h"
#include <vector>

namespace carla {
namespace road {

  enum class ConnectionType: uint32_t {
    FORWARD = 0,
    LEFT,
    RIGHT
  };

  struct RouteSegment {
    RoadId road_id;
    bool direction;
  };

  struct Route {
    std::vector<RouteSegment> route_segments;
    double length;
    element::Waypoint start_waypoint;
    element::Waypoint end_waypoint;
  };

} // namespace road
} // namespace carla
