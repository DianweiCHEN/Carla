// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/RouteSegment.h"

namespace carla {
namespace client {

  RouteSegment::RouteSegment(SharedPtr<const Route> parent, road::RouteSegment route_segment) :
      _parent(parent), _route_segment(route_segment) {

  }

} // namespace client
} // namespace carla
