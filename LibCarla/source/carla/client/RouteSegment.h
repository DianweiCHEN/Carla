// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/Road.h"
#include "carla/road/Lane.h"
#include "carla/client/Waypoint.h"

#include "carla/client/Route.h"

namespace carla {
namespace client {

  class RouteSegment :
      public EnableSharedFromThis<RouteSegment>,
      private NonCopyable {
  public:
    using LaneId = road::LaneId;
    using RoadId = road::RoadId;

    RoadId GetRoadId() const {
      return _route_segment.road_id;
    }

  private:
    friend Route;

    RouteSegment(SharedPtr<const Route> parent, road::RouteSegment route_segment);

    SharedPtr<const Route> _parent;

    road::RouteSegment _route_segment;
  };

} // namespace client
} // namespace carla
