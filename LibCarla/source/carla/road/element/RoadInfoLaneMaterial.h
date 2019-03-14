// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include <string>

namespace carla {
namespace road {
namespace element {

  class RoadInfoLaneMaterial : public RoadInfo {

  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoLaneMaterial(
        double s,   // start position relative to the position of the preceding
                    // lane section
        std::string surface,
        double friction,
        double roughness)
      : RoadInfo(s),
        _surface(surface),
        _friction(friction),
        _roughness(roughness) {}

    std::string GetSurface() {
      return _surface;
    }

    double GetFriction() const {
      return _friction;
    }

    double GetRoughness() const {
      return _roughness;
    }

  private:

    std::string _surface;

    double _friction;

    double _roughness;
  };

} // namespace element
} // namespace road
} // namespace carla
