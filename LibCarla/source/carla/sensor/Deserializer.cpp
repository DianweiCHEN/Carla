// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/Deserializer.h"

#include "carla/sensor/SensorRegistry.h"

#include "carla/profiler/Tracer.h"

namespace carla {
namespace sensor {

  SharedPtr<SensorData> Deserializer::Deserialize(Buffer &&buffer) {
    TRACE_SCOPE_FUNCTION("Deserialize");
    return SensorRegistry::Deserialize(std::move(buffer));
  }

} // namespace sensor
} // namespace carla
