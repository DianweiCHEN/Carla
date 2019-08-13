// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once


#include "carla/MsgPack.h"

#include <cmath>
#include <limits>


namespace carla {
namespace rpc {

  class IntensityPoint {
  public:
    float dx = 0.0f;

    float dy = 0.0f;

    float dz = 0.0f;

    float i = 0.0f;

    float id = 0.0f;

    float vx = 0.0f;

    float vy = 0.0f;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    IntensityPoint() = default;

    IntensityPoint(float _x, float _y, float _z, float _i, float _id, float _vx, float _vy)
      : dx(_x),
        dy(_y),
        dz(_z),
        i(_i),
        id(_id),
        vx(_vx),
        vy(_vy) {}

    MSGPACK_DEFINE_ARRAY(dx, dy, dz, i, id, vx, vy)
  };
} // namespace rpc
} // namespace carla
