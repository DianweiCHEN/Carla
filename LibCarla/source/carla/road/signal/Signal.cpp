// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/signal/Signal.h"

namespace carla {
namespace road {
namespace signal {

bool Signal::IsLaneValid(road::LaneId laneId) {
  for(general::Validity& validity: _validities) {
    if(validity.IsValid(laneId)) {
      return true;
    }
  }
  return false;
}

} // signal
} // road
} // carla
