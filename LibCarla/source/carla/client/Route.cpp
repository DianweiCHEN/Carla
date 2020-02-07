// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Route.h"
#include "carla/client/RouteSegment.h"
#include <boost/graph/astar_search.hpp>

#include <queue>

namespace carla {
namespace client {

  Route::Route(SharedPtr<const Map> parent, road::Route route) :
        _parent(parent), _route(route) {
    }

  double Route::EstimateRoadTime() const {
    return 0.0;
  }

  SharedPtr<RouteSegment> Route::GetSegment(uint32_t segment_id){
    return SharedPtr<RouteSegment>(new RouteSegment(shared_from_this(), _route.route_segments[segment_id]));
  }

} // namespace client
} // namespace carla
