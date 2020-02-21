// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/Waypoint.h"
#include "carla/client/Map.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/Route.h"

#include <vector>

namespace carla {
namespace client {

  class RouteSegment;

  class Route :
      public EnableSharedFromThis<Route>,
      private NonCopyable {

  public:

    double GetLength() const {
      return _route.length;
    }

    size_t GetNumberOfSegments() const {
      return _route.route_segments.size();
    }

    SharedPtr<RouteSegment> GetSegment(uint32_t segment_id);

    double EstimateRoadTime() const;

    std::vector<SharedPtr<Waypoint>> GenerateWaypoints(double separation) const;

  private:

    friend Map;

    Route(SharedPtr<const Map> parent, road::Route route);

    SharedPtr<const Map> _parent;

    road::Route _route;

  };

} // namespace client
} // namespace carla
